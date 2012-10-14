/*
 * Author       : Artem Makhutov <artem@makhutov.org>
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/delay.h>
#include <asm/uaccess.h>

#define DRIVER_NAME	"folio_eeprom"
#define DRIVER_AUTHOR	"Artem Makhutov <artem@makhutov.org>"

#define I2C_TRY_COUNT	3
#define TAG             "folio_eeprom: "
#define logd(x...)      do { printk(x); } while(0)
#define EEPROM_SIZE	0x100

static struct folio_eeprom_dev *dev;
static struct proc_dir_entry *proc_file_entry;

struct folio_eeprom_dev {
	/* i2c client for convinience */
	struct i2c_client	*client;
};

/*
    Write into DMI EEPROM
 */
static int readRegDMI(unsigned char reg, unsigned char *buffer) {
        int i; 
        int ret;
        struct i2c_msg msgs[2];

        msgs[0].addr = dev->client->addr;
        msgs[0].len = 1;
        msgs[0].buf = &reg;
        msgs[0].flags = 0;

        msgs[1].addr = dev->client->addr | 0x1;
        msgs[1].len= 1;
        msgs[1].buf = buffer;
        msgs[1].flags = I2C_M_RD;

        for (i = 0; i < I2C_TRY_COUNT; i++) {
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
    Write into DMI EEPROM
*/
static int writeRegDMI(unsigned char reg, unsigned char buffer, unsigned int len) {
	int i;
	int ret;
	unsigned char writeBuffer[2];
	struct i2c_msg msgs[1];

	if (writeBuffer == NULL) {
		logd(TAG "writeReg kmalloc nomem");
		return -ENOMEM;
	}

	writeBuffer[0] = reg;
	writeBuffer[1] = buffer;

	msgs[0].addr = dev->client->addr;
	msgs[0].len = 2;
	msgs[0].buf = writeBuffer;
	msgs[0].flags = 0;

	for (i = 0; i < I2C_TRY_COUNT; i++) {
		msleep(50);
		ret = i2c_transfer(dev->client->adapter, msgs, 1);
		if (ret==1) {
			return 0;
		}
	}

	if(ret != 1)
	{
		logd(TAG "i2c_write failed(%d), %d\r\n", ret, reg);
		return -EINVAL;
	}

	return 0;
}


static void dumpDMI(unsigned char *buffer) {
	int i;

	for (i=0;i<EEPROM_SIZE;i++) {
		readRegDMI(i, &buffer[i]);
	}

	buffer[EEPROM_SIZE] ='\0';

	return;
};

static int cmdline_proc_show(struct seq_file *m, void *v) {
	int i;
	unsigned char buffer[EEPROM_SIZE + 1];

	dumpDMI(buffer);

	for (i=0;i<EEPROM_SIZE;i++) {
		seq_printf(m, "%c", buffer[i]);
	}

	return 0;
}

static int cmdline_proc_open(struct inode *inode, struct file *file) {
	return single_open(file, cmdline_proc_show, NULL);
}

static ssize_t proc_procreadwrite_write (struct file *file, const char * buf, size_t size, loff_t * ppos) {
	int i;
	size_t len = EEPROM_SIZE;
	char buffer[EEPROM_SIZE + 1];

	// EEPROM must be EEPROM_SIZE byte in size
	if (size != EEPROM_SIZE) {
		logd(TAG "EEPROM size %i bytes and not %i bytes\n", size, EEPROM_SIZE);
		return -EFAULT;
	}

	if (len > size)
		len = size;

	if (copy_from_user (buffer, buf, len))
		return -EFAULT;

	buffer[len] = '\0';

	logd(TAG "proc_procreadwrite_write: %i\n", size);

	for (i=0;i<EEPROM_SIZE;i++) {
		writeRegDMI(i, buffer[i], EEPROM_SIZE);
	}

	return len;
}

static const struct file_operations folio_eeprom_proc_dump_fops = {
	.open		= cmdline_proc_open,
	.read		= seq_read,
	.write		= proc_procreadwrite_write,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int folio_eeprom_probe(struct i2c_client *client, const struct i2c_device_id *id) {
	logd(TAG "folio_eeprom_probe\r\n");

	dev = kzalloc(sizeof(struct folio_eeprom_dev), GFP_KERNEL);
	if (!dev) {
		logd(TAG "folio_eeprom_probe kmalloc fail \r\n");
		goto failed_alloc_dev;
	}
	i2c_set_clientdata(client, dev);
	// For convinience
	dev->client = client;

	proc_file_entry = proc_create("folio_eeprom_dump", 0, NULL, &folio_eeprom_proc_dump_fops);
	if(proc_file_entry == NULL) {
		logd(TAG "folio_eeprom_probe failed: could not create proc entry\r\n");
		return -ENOMEM;
	}

	return 0;

failed_alloc_dev:
	logd(TAG "folio_eeprom_probe failed\r\n");
	return -1;
}

static int folio_eeprom_remove(struct i2c_client *client) {
	kfree(dev);
	remove_proc_entry("folio_eeprom_dump", NULL);
	return 0;
}

static const struct i2c_device_id folio_eeprom_id[] = {
	{ DRIVER_NAME, 0 },
	{ }
};

static struct i2c_driver folio_eeprom_driver = {
	.probe		= folio_eeprom_probe,
	.remove		= folio_eeprom_remove,
	.id_table	= folio_eeprom_id,
	.driver		= {
		.name	= DRIVER_NAME,
	},
};

static int __init folio_eeprom_init(void) {
	int e;
	logd(TAG "folio_eeprom_init\r\n");

	e = i2c_add_driver(&folio_eeprom_driver);
	if (e != 0) {
		pr_err("%s: failed to register with I2C bus with "
		"error: 0x%x\n", __func__, e);
	}
	return e;
}

static void __exit folio_eeprom_exit(void) {
	i2c_del_driver(&folio_eeprom_driver);
}

module_init(folio_eeprom_init);
module_exit(folio_eeprom_exit);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Folio 100 EEPROM driver");
