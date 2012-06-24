/*
 * arch/arm/mach-tegra/board-betelgeuse-usb.c
 *
 * Copyright (C) 2011 Artem Makhutov <artem@makhutov.org>
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

/* All configurations related to USB */
 
#include <linux/console.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sysfs.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <linux/platform_data/tegra_usb.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/reboot.h>
#include <linux/i2c-tegra.h>
#include <linux/mfd/tps6586x.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/time.h>
#include <asm/setup.h>

#include <mach/io.h>
#include <mach/iomap.h>
#include <mach/irqs.h>
#include <mach/gpio.h>
#include <mach/clk.h>
#include <mach/usb_phy.h>
#include <mach/system.h>

#include <linux/usb/f_accessory.h>

#include "board.h"
#include "clock.h"
#include "board-betelgeuse.h"
#include "devices.h"
#include "gpio-names.h"
#include "fuse.h"

static struct tegra_utmip_config utmi_phy_config[] = {
	[0] = {
			.hssync_start_delay = 0,
			.idle_wait_delay = 17,
			.elastic_limit = 16,
			.term_range_adj = 6,
			.xcvr_setup = 15,
			.xcvr_lsfslew = 2,
			.xcvr_lsrslew = 2,
	},
	[1] = {
			.hssync_start_delay = 0,
			.idle_wait_delay = 17,
			.elastic_limit = 16,
			.term_range_adj = 6,
			.xcvr_setup = 9,
			.xcvr_lsfslew = 2,
			.xcvr_lsrslew = 2,
	},
};

static struct tegra_ulpi_config ulpi_phy_config = {
	.reset_gpio = BETELGEUSE_USB1_RESET,
	.clk = "cdev2",
};

static struct usb_phy_plat_data tegra_usb_phy_pdata[] = {
	[0] = {
			.instance = 0,
			.vbus_irq = TPS6586X_INT_BASE + TPS6586X_INT_USB_DET,
			.vbus_gpio = BETELGEUSE_USB0_VBUS,
	},
	[1] = {
			.instance = 1,
			.vbus_gpio = -1,
	},
	[2] = {
			.instance = 2,
			.vbus_gpio = BETELGEUSE_USB2_VBUS,
	},
};

static struct tegra_ehci_platform_data tegra_ehci_pdata[] = {
	[0] = {
			.phy_config = &utmi_phy_config[0],
			.operating_mode = TEGRA_USB_HOST,
			.power_down_on_bus_suspend = 1,
			.default_enable = true,
	},
	[1] = {
			.phy_config = &ulpi_phy_config,
			.operating_mode = TEGRA_USB_HOST,
			.power_down_on_bus_suspend = 1,
			.phy_type = TEGRA_USB_PHY_TYPE_LINK_ULPI,
			.default_enable = true,
	},
	[2] = {
			.phy_config = &utmi_phy_config[1],
			.operating_mode = TEGRA_USB_HOST,
			.power_down_on_bus_suspend = 1,
			.hotplug = 1,
			.default_enable = true,
	},
};

static struct tegra_otg_platform_data tegra_otg_pdata = {
	.ehci_device = &tegra_ehci1_device,
	.ehci_pdata = &tegra_ehci_pdata[0],
};

static void betelgeuse_usb_hub_ovcur_config(void)
{
	tegra_gpio_enable(TEGRA_GPIO_PU3);
	gpio_request(TEGRA_GPIO_PU3, "usb_hub_ovcur");
	gpio_direction_input(TEGRA_GPIO_PU3);
}

static void betelgeuse_usb_init(void)
{
	betelgeuse_usb_hub_ovcur_config();

	tegra_usb_phy_init(tegra_usb_phy_pdata, ARRAY_SIZE(tegra_usb_phy_pdata));
	/* OTG should be the first to be registered */
	tegra_otg_device.dev.platform_data = &tegra_otg_pdata;
	platform_device_register(&tegra_otg_device);

	platform_device_register(&tegra_udc_device);

	tegra_ehci2_device.dev.platform_data=&tegra_ehci_pdata[1];
	platform_device_register(&tegra_ehci2_device);

	tegra_ehci3_device.dev.platform_data=&tegra_ehci_pdata[2];
	platform_device_register(&tegra_ehci3_device);
}

int __init betelgeuse_usb_register_devices(void)
{
	betelgeuse_usb_init();
	return 0;
}
