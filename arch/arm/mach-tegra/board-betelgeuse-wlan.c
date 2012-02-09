/*
 * arch/arm/mach-tegra/board-betelgeuse-wlan.c
 *
 * Copyright (C) 2010 Google, Inc.
 * Copyright (C) 2011 Artem Makhutov
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
#include <linux/mmc/host.h>

#include <asm/mach-types.h>
#include <mach/irqs.h>
#include <mach/iomap.h>
#include <mach/sdhci.h>

#include "gpio-names.h"
#include "board.h"
#include "board-betelgeuse.h"

static struct platform_device betelgeuse_wifi_device = {
	.name           = "ath6kl",
	.id             = 1,
};

/* This function is called from ar6000.ko */
void wlan_setup_power(int on, int detect)
{
	pr_info("%s: on: %i, detect:%i\n", __func__, on, detect);
}
EXPORT_SYMBOL(wlan_setup_power);

static int __init betelgeuse_wifi_init(void)
{
	//gpio_request(BETELGEUSE_WLAN_POWER, "wifi_power");
	//gpio_request(BETELGEUSE_WLAN_RESET, "wifi_reset");
	//gpio_direction_output(BETELGEUSE_WLAN_POWER, 0);
	//gpio_direction_output(BETELGEUSE_WLAN_RESET, 0);
	//gpio_set_value(BETELGEUSE_WLAN_POWER, 1);
	//gpio_set_value(BETELGEUSE_WLAN_RESET, 1);
	
	platform_device_register(&betelgeuse_wifi_device);
	
	return 0;
}

extern int betelgeuse_wlan_register_devices(void)
{
	pr_info("%s: WIFI init start\n", __func__);
	betelgeuse_wifi_init();
	return 0;
}
