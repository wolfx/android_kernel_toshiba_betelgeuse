/*
 * arch/arm/mach-tegra/board-betelgeuse.h
 *
 * Copyright (C) 2011 Eduardo José Tagle <ejtagle@tutopia.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _MACH_TEGRA_BOARD_BETELGEUSE_H
#define _MACH_TEGRA_BOARD_BETELGEUSE_H

#define TPS6586X_INT_BASE	TEGRA_NR_IRQS

#define BETELGEUSE_BT_RESET 		TEGRA_GPIO_PU0 	/* 0= reset asserted */

#define BETELGEUSE_KEY_VOLUMEUP 	TEGRA_GPIO_PD4 /* 0=pressed */
#define BETELGEUSE_KEY_VOLUMEDOWN 	TEGRA_GPIO_PV4 /* 0=pressed */
#define BETELGEUSE_KEY_POWER 		TEGRA_GPIO_PV2 /* 0=pressed */
#define BETELGEUSE_KEY_BACK		TEGRA_GPIO_PH0 /* 0=pressed */

#define BETELGEUSE_CAMERA_POWER 	TEGRA_GPIO_PV4 	/* 1=powered on */

#define BETELGEUSE_BL_ENB		TEGRA_GPIO_PB5
#define BETELGEUSE_LVDS_SHUTDOWN	TEGRA_GPIO_PB2		// Is this right?
#define BETELGEUSE_EN_VDD_PANEL		TEGRA_GPIO_PC6 
#define BETELGEUSE_BL_VDD		TEGRA_GPIO_PW0
#define BETELGEUSE_BL_PWM		TEGRA_GPIO_PB4 		/* PWM */
#define BETELGEUSE_HDMI_ENB		TEGRA_GPIO_PV5 		/* unconfirmed */ // Does betelgeuse have HDMI enbl?
#define BETELGEUSE_HDMI_HPD		TEGRA_GPIO_PN7		/* 1=HDMI plug detected */

#define BETELGEUSE_BL_PWM_ID		0			/* PWM0 controls backlight */

#define BETELGEUSE_FB_PAGES		2			/* At least, 2 video pages */
#define BETELGEUSE_FB_HDMI_PAGES	2			/* At least, 2 video pages for HDMI */

#define BETELGEUSE_MEM_SIZE 		SZ_512M			/* Total memory */
#define BETELGEUSE_MEM_BANKS		1

#define BETELGEUSE_GPU_MEM_SIZE 	SZ_128M			/* Memory reserved for GPU */

#define BETELGEUSE_FB1_MEM_SIZE 	SZ_8M			/* Memory reserved for Framebuffer 1: LCD */
#define BETELGEUSE_FB2_MEM_SIZE 	SZ_8M			/* Memory reserved for Framebuffer 2: HDMI out */

#define DYNAMIC_GPU_MEM			0			/* use dynamic memory for GPU */

/*#define BETELGEUSE_48KHZ_AUDIO*/ /* <- define this if you want 48khz audio sampling rate instead of 44100Hz */


// TPS6586x GPIOs as registered 
#define PMU_GPIO_BASE		(TEGRA_NR_GPIOS) 
#define PMU_GPIO0 		(PMU_GPIO_BASE)
#define PMU_GPIO1 		(PMU_GPIO_BASE + 1) 
#define PMU_GPIO2 		(PMU_GPIO_BASE + 2)
#define PMU_GPIO3 		(PMU_GPIO_BASE + 3)

#define ALC5623_GPIO_BASE	(TEGRA_NR_GPIOS + 16)
#define ALC5623_GP0		(ALC5623_GPIO_BASE)

#define PMU_IRQ_BASE		(TEGRA_NR_IRQS)
#define PMU_IRQ_RTC_ALM1 	(TPS6586X_INT_BASE + TPS6586X_INT_RTC_ALM1)

#define	BETELGEUSE_ENABLE_VDD_VID	TEGRA_GPIO_PD1	/* 1=enabled.  Powers HDMI. Wait 500uS to let it stabilize before returning */

// TODO: Find whether there are any definitions for these?
/*#define BETELGEUSE_SDIO0_CD		TEGRA_GPIO_PI5
#define BETELGEUSE_SDIO0_POWER	TEGRA_GPIO_PD0*/	/* SDIO0 and SDIO2 power */

#define BETELGEUSE_SDHC_CD		TEGRA_GPIO_PI5
#define BETELGEUSE_SDHC_WP		-1	/*1=Write Protected */
#define BETELGEUSE_SDHC_POWER	TEGRA_GPIO_PD0

#define BETELGEUSE_TS_IRQ		TEGRA_GPIO_PU4

//#define BETELGEUSE_FB_NONROTATE TEGRA_GPIO_PH1 /*1 = screen rotation locked */

#define BETELGEUSE_WLAN_POWER 	TEGRA_GPIO_PK5
#define BETELGEUSE_WLAN_RESET 	TEGRA_GPIO_PK6

#define BETELGEUSE_BT_RST		TEGRA_GPIO_PU0

#define BETELGEUSE_LOW_BATT	TEGRA_GPIO_PW3 /*(0=low battery)*/
#define BETELGEUSE_IN_S3		TEGRA_GPIO_PAA7 /*1 = in S3 */

#define BETELGEUSE_USB0_VBUS		TEGRA_GPIO_PB0		/* 1= VBUS usb0 */
#define BETELGEUSE_USB1_RESET		TEGRA_GPIO_PV1	/* 0= reset */

#define BETELGEUSE_HP_DETECT	TEGRA_GPIO_PW2 	/* HeadPhone detect for audio codec: 1=Hedphone plugged */

#define BETELGEUSE_NVEC_REQ	TEGRA_GPIO_PBB1	/* Set to 0 to send a command to the NVidia Embedded controller */
#define BETELGEUSE_NVEC_I2C_ADDR 0x8a 			/* I2C address of Tegra, when acting as I2C slave */

#define BETELGEUSE_WAKE_KEY_POWER  TEGRA_WAKE_GPIO_PV2
#define BETELGEUSE_WAKE_KEY_RESUME TEGRA_WAKE_GPIO_PV2

#define BETELGEUSE_TEMP_ALERT	TEGRA_GPIO_PN6

/* The switch used to indicate rotation lock */
//#define SW_ROTATION_LOCK 	(SW_MAX-1)

extern void betelgeuse_wifi_set_cd(int val);

extern void betelgeuse_init_emc(void);
extern void betelgeuse_pinmux_init(void);
extern void betelgeuse_clks_init(void);

extern int betelgeuse_usb_register_devices(void);
extern int betelgeuse_audio_register_devices(void);
extern int betelgeuse_jack_register_devices(void);
extern int betelgeuse_gpu_register_devices(void);
extern int betelgeuse_uart_register_devices(void);
extern int betelgeuse_spi_register_devices(void);
extern int betelgeuse_aes_register_devices(void);
extern int betelgeuse_wdt_register_devices(void);
extern int betelgeuse_i2c_register_devices(void);
extern int betelgeuse_power_register_devices(void);
extern int betelgeuse_keyboard_register_devices(void);
extern int betelgeuse_touch_register_devices(void);
extern int betelgeuse_sdhci_register_devices(void);
extern int betelgeuse_sensors_register_devices(void);
extern int betelgeuse_wlan_pm_register_devices(void);
extern void betelgeuse_setup_bluesleep(void);
extern void betelgeuse_bt_rfkill(void);
extern int betelgeuse_camera_register_devices(void);

#endif
