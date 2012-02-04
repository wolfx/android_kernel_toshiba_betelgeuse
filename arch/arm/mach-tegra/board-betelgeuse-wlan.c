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
#include "board-betelgeuse-sdhci.h"

static struct clk *wifi_32k_clk;

static int betelgeuse_wifi_reset(int on);
static int betelgeuse_wifi_power(int on);
static int betelgeuse_wifi_set_carddetect(int val);

static struct wifi_platform_data betelgeuse_wifi_control = {
	.set_power	= betelgeuse_wifi_power,
	.set_reset	= betelgeuse_wifi_reset,
	.set_carddetect	= betelgeuse_wifi_set_carddetect,
};

/* This is used for the ath6kl driver - not used for ar6000.ko */
static struct platform_device betelgeuse_wifi_device = {
	.name           = "ath6kl",
	.id             = 1,
	.dev            = {
		.platform_data = &betelgeuse_wifi_control,
	},
};

static int betelgeuse_wifi_set_carddetect(int val)
{
	pr_info("%s: %d\n", __func__, val);
	if (wifi_status_cb)
		wifi_status_cb(val, wifi_status_cb_devid);
	else
		pr_warning("%s: Nobody to notify\n", __func__);
	return 0;
}

static int betelgeuse_wifi_power(int on)
{
	pr_info("%s: %d\n", __func__, on);

	gpio_set_value(BETELGEUSE_WLAN_POWER, on);
	mdelay(100);
	gpio_set_value(BETELGEUSE_WLAN_RESET, on);

	return 0;
}

static int betelgeuse_wifi_reset(int on)
{
	pr_info("%s: do nothing\n", __func__);
	return 0;
}

/* This function is called from ar6000.ko */
void wlan_setup_power(int on, int detect)
{
	pr_info("%s: on: %i, detect:%i\n", __func__, on, detect);
	betelgeuse_wifi_power(on);
	betelgeuse_wifi_set_carddetect(detect);
	mdelay(200);
}
EXPORT_SYMBOL(wlan_setup_power);

/* ar6000.ko stuff */
static struct platform_device betelgeuse_ar6000_pm_device = {
	.name		= "wlan_ar6000_pm",
	.id		= 1,
	.num_resources	= 0,
	.resource	= NULL,
};

static int __init betelgeuse_wifi_init(void)
{
	wifi_32k_clk = clk_get_sys(NULL, "blink");
	if (IS_ERR(wifi_32k_clk)) {
		pr_err("%s: unable to get blink clock\n", __func__);
		return PTR_ERR(wifi_32k_clk);
	}
	
	gpio_request(BETELGEUSE_WLAN_POWER, "wifi_power");
	gpio_request(BETELGEUSE_WLAN_RESET, "wifi_reset");
	gpio_direction_output(BETELGEUSE_WLAN_POWER, 0);
	gpio_direction_output(BETELGEUSE_WLAN_RESET, 0);
	tegra_gpio_enable(BETELGEUSE_WLAN_POWER);
	tegra_gpio_enable(BETELGEUSE_WLAN_RESET);

	// Lets just power on wifi
	betelgeuse_wifi_power(1);
	betelgeuse_wifi_reset(1);
	betelgeuse_wifi_set_carddetect(1);
	
	platform_device_register(&betelgeuse_wifi_device);
	platform_device_register(&betelgeuse_ar6000_pm_device);
	
	device_init_wakeup(&betelgeuse_wifi_device.dev, 1);
	device_set_wakeup_enable(&betelgeuse_wifi_device.dev, 0);
	
	return 0;
}

extern int betelgeuse_wlan_register_devices(void)
{
	pr_info("%s: WIFI init start\n", __func__);
	betelgeuse_wifi_init();
	return 0;
}
