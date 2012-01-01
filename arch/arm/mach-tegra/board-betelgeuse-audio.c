/*
 * Copyright (C) 2010 NVIDIA, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA
 */

#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/i2c-tegra.h>

#include <mach/iomap.h>
#include <mach/irqs.h>
#include <mach/iomap.h>
#include <mach/io.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <sound/wm8903.h>
#include <mach/i2s.h>
#include <mach/tegra_wm8903_pdata.h>
#include <mach/audio.h>
#include "devices.h"
#include "gpio-names.h"
#include "board-betelgeuse.h"

static struct wm8903_platform_data wm8903_pdata = {
	.irq_active_low = 0,
	.micdet_cfg = 0x00,           /* enable mic bias current */
	.micdet_delay = 100,
	.gpio_base = WM8903_GPIO_BASE,
	.gpio_cfg = {
		WM8903_GPIO_NO_CONFIG,
		WM8903_GPIO_NO_CONFIG,
		//(0x06<<WM8903_GP1_FN_SHIFT) | WM8903_GP1_LVL | WM8903_GP1_DB,
		//(0x06<<WM8903_GP1_FN_SHIFT) | WM8903_GP1_DIR | WM8903_GP1_IP_CFG,
		0,                     /* as output pin */
		WM8903_GPIO_NO_CONFIG,
		WM8903_GPIO_NO_CONFIG,
	},
};

static struct i2c_board_info __initdata wm8903_board_info[] = {
	{
		I2C_BOARD_INFO("wm8903", 0x1a),
		.platform_data = &wm8903_pdata,
		//.irq = TEGRA_GPIO_TO_IRQ(TEGRA_GPIO_PX3),
	},
};

static struct tegra_wm8903_platform_data ventana_audio_pdata = {
	.gpio_spkr_en           = TEGRA_GPIO_SPKR_EN,
	.gpio_hp_det            = TEGRA_GPIO_HP_DET,
	.gpio_hp_mute           = -1,
	.gpio_int_mic_en        = TEGRA_GPIO_INT_MIC_EN,
	.gpio_ext_mic_en        = TEGRA_GPIO_EXT_MIC_EN,
};

static struct platform_device betelgeuse_audio_device = {
	.name   = "tegra-snd-wm8903",
	.id     = 0,
	.dev    = {
	        .platform_data  = &ventana_audio_pdata,
	},
};

static struct platform_device *betelgeuse_audio_devices[] __initdata = {
	&tegra_i2s_device1,
	&tegra_spdif_device,
	&tegra_das_device,
	&spdif_dit_device,
	&tegra_pcm_device,
	&betelgeuse_audio_device,
};

int __init betelgeuse_audio_init(void)
{
	pr_info("Audio: betelgeuse_audio_init\n");
	platform_add_devices(betelgeuse_audio_devices, ARRAY_SIZE(betelgeuse_audio_devices));
	i2c_register_board_info(0, wm8903_board_info, 1);
	return 0;
}
