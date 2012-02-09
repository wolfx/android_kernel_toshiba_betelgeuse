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
	int gpio_pwr, gpio_rst;
	
	//if (!machine_is_harmony())
	//	return 0;

	/* WLAN - Power up (low) and Reset (low) */
	gpio_pwr = gpio_request(TEGRA_GPIO_WLAN_PWR_LOW, "wlan_pwr");
	gpio_rst = gpio_request(TEGRA_GPIO_WLAN_RST_LOW, "wlan_rst");
	if (gpio_pwr < 0 || gpio_rst < 0) {
		pr_warning("Unable to get gpio for WLAN Power and Reset\n");
	} else {
		tegra_gpio_enable(TEGRA_GPIO_WLAN_PWR_LOW);
		tegra_gpio_enable(TEGRA_GPIO_WLAN_RST_LOW);
		/* toggle in this order as per spec */
		gpio_direction_output(TEGRA_GPIO_WLAN_PWR_LOW, 0);
		gpio_direction_output(TEGRA_GPIO_WLAN_RST_LOW, 0);
		udelay(5);
		gpio_direction_output(TEGRA_GPIO_WLAN_PWR_LOW, 1);
		gpio_direction_output(TEGRA_GPIO_WLAN_RST_LOW, 1);
	}
	
	return 0;
}

/*
 * subsys_initcall_sync is good synch point to call harmony_wifi_init
 * This makes sure that the required regulators (LDO3
 * supply of external PMU and 1.2V regulator) are properly enabled,
 * and mmc driver has not yet probed for a device on SDIO bus.
 */
subsys_initcall_sync(betelgeuse_wifi_init);

extern int betelgeuse_wlan_register_devices(void)
{
	pr_info("%s: WIFI init start\n", __func__);
	platform_device_register(&betelgeuse_wifi_device);
	return 0;
}
