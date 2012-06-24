/*
 * arch/arm/mach-tegra/board-betelgeuse-sdhci.c
 *
 * Copyright (C) 2011 Eduardo José Tagle <ejtagle@tutopia.com> 
 * Copyright (C) 2010 Google, Inc.
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
#include <linux/resource.h>
#include <linux/platform_device.h>
#include <linux/wlan_plat.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/version.h>

#include <asm/mach-types.h>
#include <mach/irqs.h>
#include <mach/iomap.h>
#include <mach/sdhci.h>
#include <mach/pinmux.h>

#include "gpio-names.h"
#include "devices.h"
#include "board-betelgeuse.h"

/* Make sure they are NOT trying to compile with a nonworking config */

// Wifi SD
struct tegra_sdhci_platform_data betelgeuse_wifi_data = {
	.pm_flags = MMC_PM_KEEP_POWER,
	.cd_gpio = -1,
	.wp_gpio = -1,
	.power_gpio = -1,
};

// External SD
static struct tegra_sdhci_platform_data tegra_sdhci_platform_data2 = {
	.cd_gpio = BETELGEUSE_SDHC_EXT_CD,
	.wp_gpio = BETELGEUSE_SDHC_EXT_WP,
	.power_gpio = BETELGEUSE_SDHC_EXT_POWER,
};

// Internal SD
static struct tegra_sdhci_platform_data tegra_sdhci_platform_data4 = {
	.mmc_data = {
		.built_in = 1,
	},
	.cd_gpio = BETELGEUSE_SDHC_INT_CD,
	.wp_gpio = BETELGEUSE_SDHC_INT_WP,
	.power_gpio = BETELGEUSE_SDHC_INT_POWER,
};

static struct platform_device *betelgeuse_sdhci_devices[] __initdata = {
	&tegra_sdhci_device4,
	&tegra_sdhci_device2,
	&tegra_sdhci_device1,
};

/* Register sdhci devices */
int __init betelgeuse_sdhci_register_devices(void)
{
	int ret=0;
	/* Plug in platform data */
	tegra_sdhci_device2.dev.platform_data = &tegra_sdhci_platform_data2;
	tegra_sdhci_device4.dev.platform_data = &tegra_sdhci_platform_data4;
	tegra_sdhci_device1.dev.platform_data = &betelgeuse_wifi_data;

	ret = platform_add_devices(betelgeuse_sdhci_devices, ARRAY_SIZE(betelgeuse_sdhci_devices));
	return ret;
}
