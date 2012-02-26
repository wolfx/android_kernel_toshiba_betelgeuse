/*
 * arch/arm/mach-tegra/board-betelgeuse-audio.c
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

/* All configurations related to audio */
#include <linux/console.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/i2c-tegra.h>
#include <linux/i2c.h>
#include <linux/version.h>
#include <sound/wm8903.h>
#include <mach/tegra_wm8903_pdata.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/time.h>
#include <asm/setup.h>
#include <asm/io.h>

#include <mach/io.h>
#include <mach/irqs.h>
#include <mach/iomap.h>
#include <mach/gpio.h>
#include <mach/spdif.h>
#include <mach/audio.h>  
#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,36)
#include <mach/tegra_das.h>
#endif

#include <mach/system.h>

#include "board.h"
#include "board-betelgeuse.h"
#include "gpio-names.h"
#include "devices.h"

static struct wm8903_platform_data betelgeuse_wm8903_pdata = {
	.irq_active_low = 0,
	.micdet_cfg = 0x00,           /* enable mic bias current */
	.micdet_delay = 100,
	.gpio_base = WM8903_GPIO_BASE,
	.gpio_cfg = {
		WM8903_GPIO_NO_CONFIG,
		WM8903_GPIO_NO_CONFIG,
		0,                     /* as output pin */
		WM8903_GPIO_NO_CONFIG,
		WM8903_GPIO_NO_CONFIG,
	},
};

static struct i2c_board_info __initdata wm8903_board_info = {
	I2C_BOARD_INFO("wm8903", 0x1a),
	.platform_data = &betelgeuse_wm8903_pdata,
	.irq = TEGRA_GPIO_TO_IRQ(TEGRA_GPIO_CDC_IRQ),
};

static struct tegra_wm8903_platform_data betelgeuse_audio_pdata = {
	.gpio_spkr_en           = TEGRA_GPIO_SPKR_EN,
	.gpio_hp_det		= -1,
	.gpio_hp_mute           = -1,
	.gpio_int_mic_en	= -1,
	.gpio_ext_mic_en	= -1,
};

static struct platform_device betelgeuse_audio_device = {
	.name   = "tegra-snd-wm8903",
	.id     = 0,
	.dev    = {
		.platform_data  = &betelgeuse_audio_pdata,
	},
};

static struct platform_device *betelgeuse_audio_devices[] __initdata = {
	&tegra_i2s_device1,
	&tegra_i2s_device2,
	&tegra_spdif_device,
	&tegra_das_device,
	&spdif_dit_device,
	&bluetooth_dit_device,
	&tegra_pcm_device,
	&betelgeuse_audio_device,
};

int __init betelgeuse_audio_register_devices(void)
{
	int ret;

	pr_info("Audio: betelgeuse_audio_init\n");

	ret = i2c_register_board_info(0, &wm8903_board_info, 1);
	if (ret)
		return ret;

	return platform_add_devices(betelgeuse_audio_devices, ARRAY_SIZE(betelgeuse_audio_devices));
}
