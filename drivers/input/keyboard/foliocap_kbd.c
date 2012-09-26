/*
 * Athor       : Artem Makhutov <artem@makhutov.org>
 * Date        : 2012/09/26
 * 
 * Based on NvODM driver by Jn Hung <pegatraon.com>
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/slab.h>

#define DRIVER_NAME	"foliocap_kbd"
#define DRIVER_AUTHOR	"Artem Makhutov <artem@makhutov.org>"

#define FOLIOCAP_I2C_TRY_COUNT			3
#define FOLIOCAP_TIMER_INTERVAL			2000
#define TAG             "foliocap_kdb: "
#define logd(x...)      do { printk(x); } while(0)

/* PIC16F722 register address */
#define BUTTONS_STATUS_REG      0x00
#define BUTTONS_CHANGE_REG      0x01
#define LED_STATUS_REG          0x02
#define LED_ONOFF_REG           0x03
#define LED_BLINK_REG           0x04
#define WAKEUP_REG              0x05
#define LED_CTRL_REG            0x06
#define CHIP_ID_REG             0x07
#define INT_REG                 0x08
#define PCTRL_REG               0x09
#define FW_REV0_REG             0x0a
#define FW_REV1_REG             0x0b
#define FW_REV2_REG             0x0c
#define LED_MANUAL_MODE_REG     0x0d
#define LED_BLINK_FREQ_REG      0x0e
#define LED_BLINK_DUTY_REG      0x0f
#define LED_SET_T1_REG          0x10
#define LED_SET_T2_REG          0x11
#define LED_SET_T3_REG          0x12
#define BTN0_SENSITIVITY_REG    0x13
#define BTN1_SENSITIVITY_REG    0x14
#define BTN2_SENSITIVITY_REG    0x15
#define BTN3_SENSITIVITY_REG    0x16
#define BTN4_SENSITIVITY_REG    0x17
#define BTN5_SENSITIVITY_REG    0x18
#define BTN6_SENSITIVITY_REG    0x19
#define BTN7_SENSITIVITY_REG    0x1a

#define NVODM_QUERY_I2C_EEPROM_ADDRESS   0xA0   // I2C device base address for EEPROM (7'h50)
#define NVODM_QUERY_SENSITIVITY_START    0x7C   // 0x7C,0x7D,0x7E,0x7F

/* mode settings */
#define CAPSENSOR_MODE_NORMAL         0
#define CAPSENSOR_MODE_SLEEP          1

#define CAPSENSOR_CHIP_ID      0x01        // RO - device identification
#define FW_REV_ID_1            0x01000200  // FW reversion identification
#define FW_REV_ID_0            0x01000108  // FW reversion identification

static unsigned int cap_sensitivity_v1 = 0x08151515;	// Default Sensitivity [08] [21] [21] [21] for Cap Sensor board V1.0 FW(CSM + difital filter)
static unsigned int cap_sensitivity_v2 = 0x646E8C78;	// Default Sensitivity [100][110][140][120]for Cap Sensor board V2.0 FW (CVD + difital filter)
static volatile unsigned int keyEvent = 0;

struct foliocap_kbd_dev {
	/* i2c client for convinience */
	struct i2c_client	*client;

	/* input dev */
	struct input_dev *input_dev;
};

struct foliocap_kbd_info {
	unsigned int 	key_mask;
	int 		key_code; 
};

static struct foliocap_kbd_info key_table[] = {
	{ 0x08, KEY_SEARCH },
	{ 0x10, KEY_BACK },
	{ 0x20, KEY_HOMEPAGE },
	{ 0x80, KEY_MENU },
};

static int key_num = sizeof(key_table)/sizeof(key_table[0]);

/*
    Write Cap Sensor Register to the PIC16F722
*/
static int writeReg(struct foliocap_kbd_dev *dev, unsigned char reg, unsigned char *buffer, unsigned int len) {
	int i;
	int ret;
        unsigned char *writeBuffer;
        struct i2c_msg msgs[1];

        writeBuffer = kmalloc(len + 1, GFP_KERNEL);
        if (writeBuffer == NULL) {
                logd(TAG "writeReg kmalloc nomem");
                return -ENOMEM;
        }

	writeBuffer[0] = reg;
	memcpy(&writeBuffer[1], buffer, len);

	msgs[0].addr = dev->client->addr;
	msgs[0].len = len + 1;
	msgs[0].buf = writeBuffer;
	msgs[0].flags = 0;

	for (i = 0; i < FOLIOCAP_I2C_TRY_COUNT; i++) {
		ret = i2c_transfer(dev->client->adapter, msgs, 1);
		if (ret==1) {
			kfree(writeBuffer);
			return 0;
		}
	}

	if(ret != 1)
	{
		logd(TAG "i2c_write failed(%d)\r\n", ret);
		kfree(writeBuffer);
		return -EINVAL;
	}

	kfree(writeBuffer);
        return 0;
}

/*
    Read Cap Sensor Register of the PIC16F722
*/
static int readReg(struct foliocap_kbd_dev *dev, unsigned char reg, unsigned char *buffer, unsigned int read_num)
{
        int i;
        int ret;
        struct i2c_msg msgs[2];

        msgs[0].addr = dev->client->addr;
        msgs[0].len = 1;
        msgs[0].buf = &reg;
        msgs[0].flags = 0;

        msgs[1].addr = dev->client->addr | 0x1;
        msgs[1].len= read_num;
        msgs[1].buf = buffer;
        msgs[1].flags = I2C_M_RD;

        for (i = 0; i < FOLIOCAP_I2C_TRY_COUNT; i++) {
		ret = i2c_transfer(dev->client->adapter, msgs, 2);
		if (ret==2) {
			return 0;
		}
        }

        if(ret != 2)
        {
                logd(TAG "i2c_read failed(%d)\r\n", ret);
                return -EINVAL;
        }

        return 0;
}

/*
    Read Sensitivity from DMI EEPROM
*/
static int readRegDMI(struct foliocap_kbd_dev *dev, __u8 i2cAddr, unsigned char reg, unsigned char *buffer) {
        int i; 
        int ret;
        struct i2c_msg msgs[2];

        msgs[0].addr = (i2cAddr >> 1);
        msgs[0].len = 1;
        msgs[0].buf = &reg;
        msgs[0].flags = 0;

        msgs[1].addr = (i2cAddr >> 1 ) | 0x1;
        msgs[1].len= 1;
        msgs[1].buf = buffer;
        msgs[1].flags = I2C_M_RD;

        for (i = 0; i < FOLIOCAP_I2C_TRY_COUNT; i++) {
                ret = i2c_transfer(dev->client->adapter, msgs, 2);
                if (ret==2) {
                        return 0;
                }
        }

        if(ret != 2)
        {
                logd(TAG "i2c_read failed(%d)\r\n", ret);
                return -EINVAL;
        }

        return 0;
}

/*
    Write Sensitivity into DMI EEPROM
*/
static int writeRegDMI(struct foliocap_kbd_dev *dev, __u8 i2cAddr, unsigned char reg, __u32 pData) {
	int i;
	int ret;
	__u8 writeBuffer[5];
	struct i2c_msg msgs[1];

	writeBuffer[0] = reg;

	writeBuffer[1] = (pData&0xff);
	writeBuffer[2] = ((pData>>8)&0xff);
	writeBuffer[3] = ((pData>>16)&0xff);
	writeBuffer[4] = ((pData>>24)&0xff);

	msgs[0].addr = (i2cAddr >> 1);
	msgs[0].len = 5;
	msgs[0].buf = writeBuffer;
	msgs[0].flags = 0;

	for (i = 0; i < FOLIOCAP_I2C_TRY_COUNT; i++) {
		ret = i2c_transfer(dev->client->adapter, msgs, 1);
		if (ret==1) {
			return 0;
		}
	}

	if(ret != 1)
	{
		logd(TAG "i2c_write failed(%d)\r\n", ret);
		return -EINVAL;
	}

	return 0;
}

__u32 readSensitivityFromDMI(struct foliocap_kbd_dev *dev) {
	unsigned char retVal = 0;
	__u32 sensitivity = 0;

	if (readRegDMI(dev, NVODM_QUERY_I2C_EEPROM_ADDRESS, NVODM_QUERY_SENSITIVITY_START, &retVal))
		goto error;
	logd(TAG "Read sensitivity 1: 0x%x\n", retVal);
	sensitivity |=retVal;

	if (readRegDMI(dev, NVODM_QUERY_I2C_EEPROM_ADDRESS, NVODM_QUERY_SENSITIVITY_START+1, &retVal))
		goto error;
	logd(TAG "Read sensitivity 2: 0x%x\n", retVal);
	sensitivity |=(retVal<<8);
	
	if (readRegDMI(dev, NVODM_QUERY_I2C_EEPROM_ADDRESS, NVODM_QUERY_SENSITIVITY_START+2, &retVal))
		goto error;
	logd(TAG "Read sensitivity 3: 0x%x\n", retVal);
	sensitivity |=(retVal<<16);
	
	if (readRegDMI(dev, NVODM_QUERY_I2C_EEPROM_ADDRESS, NVODM_QUERY_SENSITIVITY_START+3, &retVal))
		goto error;
	logd(TAG "Read sensitivity 4: 0x%x\n", retVal);
	sensitivity |=(retVal<<24);

	logd(TAG "sensitivity=0x%x\n", sensitivity);
	return sensitivity;

error:
	logd(TAG "Error readSensitivityFromDMI\n");
	return 0xffffffff;
}

__u8 readButtonChange(struct foliocap_kbd_dev *dev) {
	__u8 data;
	if (readReg(dev, BUTTONS_CHANGE_REG, &data, 1))
		return 0xFF;

	return data;
}

__u8 readButtonStatus(struct foliocap_kbd_dev *dev) {
	__u8 data;
	if (readReg(dev, BUTTONS_STATUS_REG, &data, 1))
		return 0xFF;

	return data;
}

/*
    To write sensitivity into DMI EEPROM
*/
int capProgSensitivity(struct foliocap_kbd_dev *dev, __u32 value)
{
	logd(TAG "capProgSensitivity\n");
	return writeRegDMI(dev, NVODM_QUERY_I2C_EEPROM_ADDRESS, NVODM_QUERY_SENSITIVITY_START, value);
}


static int setLowestSensitivity(struct foliocap_kbd_dev *dev)
{
	// Init Hw    
	__u8 testVal = 0x5F;  //Set lowest sensitivity to 0x5F BTN0,BTN1,BTN2,BTN6  unused button

	if (writeReg(dev, BTN0_SENSITIVITY_REG, &testVal, 1))
		goto error;

	if (writeReg(dev, BTN1_SENSITIVITY_REG, &testVal, 1))
		goto error;

	if (writeReg(dev, BTN2_SENSITIVITY_REG, &testVal, 1))
		goto error;

	if (writeReg(dev, BTN6_SENSITIVITY_REG, &testVal, 1))
		goto error;

	return 0;

	error:
		logd(TAG "Error in setLowestSensitivity\n");
		return -EINVAL;
}

/*
    Adjust sensitivity of Cap sensor button
*/
int capAdjustSensitivity(struct foliocap_kbd_dev *dev, __u32 value)
{
	__u8 testVal;
	
	testVal = (value&0xff);
	if (writeReg(dev, BTN3_SENSITIVITY_REG, &testVal, 1))
		goto error;
	testVal = ((value>>8)&0xff);
	if (writeReg(dev, BTN4_SENSITIVITY_REG, &testVal, 1))
		goto error;
	testVal = ((value>>16)&0xff);
	if (writeReg(dev, BTN5_SENSITIVITY_REG, &testVal, 1))
		goto error;
	testVal = ((value>>24)&0xff);
	if (writeReg(dev, BTN7_SENSITIVITY_REG, &testVal, 1))
		goto error;

	return 0;

	error:
		logd(TAG "Cap Sensor_Init failed\n");
		return -EINVAL;
}

/*
    Read device ID and Firmware Info.
*/
static int foliocapReadDevInfo(struct foliocap_kbd_dev *dev, __u32 *value) {
	unsigned char reg_val;

	if (readReg(dev, CHIP_ID_REG, &reg_val, 1)) {
		goto failed;
	}
	logd(TAG "CHIP_ID_REG=0x%x\n", reg_val);
	*value |= (reg_val<<24);
	if (readReg(dev, FW_REV2_REG, &reg_val, 1)) {
                goto failed;
        }
	logd(TAG "FW_REV2_REG=0x%x\n", reg_val);
	*value |= (reg_val<<16);
	if (readReg(dev, FW_REV1_REG, &reg_val, 1)) {
		goto failed;
	}
	logd(TAG "FW_REV1_REG=0x%x\n", reg_val);
	*value |= (reg_val<<8);
	if (readReg(dev, FW_REV0_REG, &reg_val, 1)) {
                goto failed;
        }
	logd(TAG "FW_REV0_REG=0x%x\n", reg_val);
	*value |= reg_val;

	return 0;
failed:
	logd(TAG "foliocapReadDevInfo failed\r\n");	
	return -EINVAL;
}

int foliocapInit(struct foliocap_kbd_dev *dev) {
	__u32 fw_rev = 0;
	__u32 sensitivity;

        foliocapReadDevInfo(dev, &fw_rev);
        logd(TAG "fw_rev: 0x%x\n", fw_rev);

        if (fw_rev==FW_REV_ID_0) {
                logd(TAG "FW_REV_ID_0\n");
        }
        else {  
                logd(TAG "FW_REV_ID_1\n");
        }

        sensitivity = readSensitivityFromDMI(dev);

	if((sensitivity==0xffffffff)&&(fw_rev==FW_REV_ID_0)){	/* Use default sensitivity */
		sensitivity = cap_sensitivity_v1;		/* Default Sensitivity 0x08(Menu) 0x15(Home) 0x15(Back) 0x15(Serach) */
		capProgSensitivity(dev,cap_sensitivity_v1);
	}

	if((sensitivity==0xffffffff)&&(fw_rev==FW_REV_ID_1)){	/* Use default sensitivity */
		sensitivity = cap_sensitivity_v2;		/* Default Sensitivity 0x64(Menu) 0x6E(Home) 0x8C(Back) 0x78(Serach) */
		capProgSensitivity(dev,cap_sensitivity_v2);
	}

	capAdjustSensitivity(dev, sensitivity);

	if(fw_rev==FW_REV_ID_0) {
		setLowestSensitivity(dev);
	}

	return 0;
}

int printKey(int keyCode, int status)
{
#if 0
	if (keyCode == KEY_SEARCH) logd(TAG "KEY_SEARCH - %d\n", status);
	if (keyCode == KEY_BACK) logd(TAG "KEY_BACK - %d\n", status);
	if (keyCode == KEY_HOMEPAGE) logd(TAG "KEY_HOMEPAGE - %d\n", status);
	if (keyCode == KEY_MENU) logd(TAG "KEY_MENU - %d\n", status);
#endif

	return 0;
}

static irqreturn_t foliocap_irq_callback(int irq, void *args)
{
	int i;
	int keyButton = 0;
	int keyStatus = 0;

	struct foliocap_kbd_dev *dev;
	dev = (struct foliocap_kbd_dev*)args;

	keyButton = readButtonChange(dev);
	keyStatus = readButtonStatus(dev);

        for (i = 0; i < key_num; i++) {
		if (keyButton & key_table[i].key_mask) {
			if (keyStatus & key_table[i].key_mask) {
				printKey(key_table[i].key_code, 1);
				input_report_key(dev->input_dev, key_table[i].key_code, 1);
				input_sync(dev->input_dev);
			} else {
				printKey(key_table[i].key_code, 0);
				input_report_key(dev->input_dev, key_table[i].key_code, 0);
				input_sync(dev->input_dev);
			}
		}

        }

	return IRQ_HANDLED;
}

static int foliocap_kbd_probe(struct i2c_client *client, const struct i2c_device_id *id) {
	int i;
	struct foliocap_kbd_dev *dev;

	logd(TAG "foliocap_kbd_probe\r\n");

	dev = kzalloc(sizeof(struct foliocap_kbd_dev), GFP_KERNEL);
	if (!dev) {
		logd(TAG "foliocap_kbd_probe kmalloc fail \r\n");
		goto failed_alloc_dev;
	}
	i2c_set_clientdata(client, dev);
	// For convinience
	dev->client = client;
	
	/* register input device */
	dev->input_dev = input_allocate_device();
	if (!dev->input_dev) {
		logd(TAG "foliocap_kbd_probe input_allocate_device fail \r\n");
		goto failed_alloc_input;
	}
	dev->input_dev->name = "foliocap_kbd";
	set_bit(EV_KEY, dev->input_dev->evbit);
	for (i = 0; i < key_num; i++) {
		set_bit(key_table[i].key_code, dev->input_dev->keybit);
	}
	if (input_register_device(dev->input_dev)) {
		goto failed_register_input;
	}
	
	foliocapInit(dev);

	if(request_threaded_irq(client->irq, NULL, foliocap_irq_callback, IRQF_TRIGGER_FALLING,	"foliocap_kbd", dev))
		goto failed_enable_irq;

	return 0;
failed_enable_irq:
	input_unregister_device(dev->input_dev);
failed_register_input:
	input_free_device(dev->input_dev);	
failed_alloc_input:
	kfree(dev);
failed_alloc_dev:
	logd(TAG "foliocap_kbd_probe failed\r\n");
	return -1;
}

static int foliocap_kbd_remove(struct i2c_client *client)
{
	struct foliocap_kbd_dev *dev;
	dev = (struct foliocap_kbd_dev *)i2c_get_clientdata(client);
	
	free_irq(client->irq, dev);
	input_unregister_device(dev->input_dev);
	input_free_device(dev->input_dev);
	kfree(dev);
	return 0;
}

static const struct i2c_device_id foliocap_kbd_id[] = {
	{ DRIVER_NAME, 0 },
	{ }
};

static struct i2c_driver foliocap_kbd_driver = {
	.probe		= foliocap_kbd_probe,
	.remove		= foliocap_kbd_remove,
	.id_table	= foliocap_kbd_id,
	.driver		= {
		.name	= DRIVER_NAME,
	},
};

static int __init foliocap_kbd_init(void)
{
	int e;
	logd(TAG "foliocap_kbd_init\r\n");

	e = i2c_add_driver(&foliocap_kbd_driver);
	if (e != 0) {
		pr_err("%s: failed to register with I2C bus with "
		       "error: 0x%x\n", __func__, e);
	}
	return e;
}

static void __exit foliocap_kbd_exit(void) 
{
	i2c_del_driver(&foliocap_kbd_driver);
}

module_init(foliocap_kbd_init);
module_exit(foliocap_kbd_exit);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Folio 100 I2C capacitive keyboard controller driver");
