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
/*#define ALC5623_IS_MASTER */
 
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

static struct tegra_audio_platform_data tegra_spdif_pdata = {
        .dma_on                 = true,  /* use dma by default */
        .mask                   = TEGRA_AUDIO_ENABLE_TX | TEGRA_AUDIO_ENABLE_RX,
        .stereo_capture = true,
};

static struct tegra_audio_platform_data tegra_audio_pdata[] = {
        /* For I2S1 - Hifi */
        [0] = { 
                .i2s_master             = true,         /* CODEC is slave for audio */
                .dma_on                 = true,         /* use dma by default */
                .i2s_master_clk = 44100,
                .i2s_clk_rate   = 11289600,
                .dap_clk                = "cdev1",
                .audio_sync_clk = "audio_2x",
                .mode                   = I2S_BIT_FORMAT_I2S,
                .fifo_fmt               = I2S_FIFO_PACKED,
                .bit_size               = I2S_BIT_SIZE_16,
                .i2s_bus_width  = 32,
                .mask                   = TEGRA_AUDIO_ENABLE_TX | TEGRA_AUDIO_ENABLE_RX,
                .stereo_capture = true,
        },
        /* For I2S2 - Bluetooth */
        [1] = { 
                .i2s_master             = false,        /* bluetooth is master always */
                .dma_on                 = true,  /* use dma by default */
                .i2s_master_clk = 8000,
                .dsp_master_clk = 8000,
                .i2s_clk_rate   = 2000000,
                .dap_clk                = "cdev1",
                .audio_sync_clk = "audio_2x",
                .mode                   = I2S_BIT_FORMAT_DSP,
                .fifo_fmt               = I2S_FIFO_16_LSB,
                .bit_size               = I2S_BIT_SIZE_16,
                .i2s_bus_width  = 32,
                .dsp_bus_width  = 16,
                .mask                   = TEGRA_AUDIO_ENABLE_TX | TEGRA_AUDIO_ENABLE_RX,
                .stereo_capture = true,
        }
};

static struct wm8903_platform_data wm8903_pdata = {
	.irq_active_low = 0,
	.micdet_cfg = 0x00,           /* enable mic bias current */
	.micdet_delay = 100,
	.gpio_base = WM8903_GPIO_BASE,
	.gpio_cfg = {
		(WM8903_GPn_FN_DMIC_LR_CLK_OUTPUT << WM8903_GP1_FN_SHIFT),
		(WM8903_GPn_FN_DMIC_LR_CLK_OUTPUT << WM8903_GP2_FN_SHIFT) | WM8903_GP2_DIR,
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
		.irq = TEGRA_GPIO_TO_IRQ(TEGRA_GPIO_CDC_IRQ),
	},
};

static struct tegra_wm8903_platform_data betelgeuse_audio_pdata = {
	.gpio_spkr_en           = TEGRA_GPIO_SPKR_EN,
	.gpio_hp_det            = -1,
	.gpio_hp_mute           = -1,
	.gpio_int_mic_en        = -1,
	.gpio_ext_mic_en        = -1,
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
	&tegra_pcm_device,
	&betelgeuse_audio_device,
};

int __init betelgeuse_audio_register_devices(void)
{
	int ret;

	pr_info("Audio: betelgeuse_audio_init\n");

	/* Patch in the platform data */
	//tegra_i2s_device1.dev.platform_data = &tegra_audio_pdata[0];
	//tegra_i2s_device2.dev.platform_data = &tegra_audio_pdata[1];
	//tegra_spdif_device.dev.platform_data = &tegra_spdif_pdata;

	ret = i2c_register_board_info(0, wm8903_board_info, 1);
	if (ret)
		return ret;

	return platform_add_devices(betelgeuse_audio_devices, ARRAY_SIZE(betelgeuse_audio_devices));
}
