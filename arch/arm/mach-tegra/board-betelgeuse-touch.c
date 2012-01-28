/*
 * arch/arm/mach-tegra/board-betelgeuse-touch.c
 *
 * Copyright (C) 2011 Jens Andersen <jens.andersen@gmail.com
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
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <asm/mach-types.h>
#include <mach/irqs.h>
#include <mach/iomap.h>
#include <mach/pinmux.h>
#include <linux/interrupt.h>
#include <linux/input.h>

#include "board-betelgeuse.h"
#include "gpio-names.h"

static struct i2c_board_info __initdata betelgeuse_i2c_bus0_touch_info_egalax[] = {
	{
		I2C_BOARD_INFO("egalax_ts", 0x04),
		.irq = TEGRA_GPIO_TO_IRQ(BETELGEUSE_TS_IRQ),
	},
};


int __init betelgeuse_touch_register_devices(void)
{
	i2c_register_board_info(0, betelgeuse_i2c_bus0_touch_info_egalax, 1);

	return 0;
}
