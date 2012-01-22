/*
 * arch/arm/mach-tegra/board-betelgeuse-clocks.c
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

#include <linux/console.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/serial_8250.h>
#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <linux/fsl_devices.h>
#include <linux/platform_data/tegra_usb.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/reboot.h>
#include <linux/i2c-tegra.h>
#include <linux/memblock.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/time.h>
#include <asm/setup.h>

#include <mach/io.h>
#include <mach/w1.h>
#include <mach/iomap.h>
#include <mach/irqs.h>
#include <mach/nand.h>
#include <mach/sdhci.h>
#include <mach/gpio.h>
#include <mach/clk.h>
#include <mach/usb_phy.h>
#include <mach/i2s.h>
#include <mach/system.h>
#include <mach/nvmap.h>

#include "board.h"
#include "board-betelgeuse.h"
#include "clock.h"
#include "gpio-names.h"
#include "devices.h"

/* Be careful here: Most clocks have restrictions on parent and on
   divider/multiplier ratios. Check tegra2clocks.c before modifying
   this table ! */
static __initdata struct tegra_clk_init_table betelgeuse_clk_init_table[] = {
	/* name			parent				rate	enabled */
	/* 32khz system clock */
	{ "clk_32k",		NULL,			32768,		true},		/* always on */
	{ "rtc",		"clk_32k",			32768,		true},		/* rtc-tegra : must be always on */
	{ "kbc",		"clk_32k",			32768,		true},		/* tegra-kbc */
	{ "blink",		"clk_32k",			32768,		false},		/* used for bluetooth */
	{ "pll_s",		"clk_32k",			32768,		true},		/* must be always on */
	/* Master clock */
	{ "clk_m",		NULL,					0,		true},	 	/* must be always on - Frequency will be autodetected */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38)
	{ "pcie_xclk",	"clk_m",		12000000,		false},		/* pcie controller */
	{ "afi",		"clk_m",		12000000,		false},		/* pcie controller */
	{ "pex",		"clk_m",		12000000,		false},		/* pcie controller */
#endif
	{ "csus",		"clk_m",		12000000,		false},		/* tegra_camera */
	{ "isp",		"clk_m",		12000000,		false},		/* tegra_camera */
	{ "usb3",		"clk_m",		12000000,		true},		/* tegra_ehci.2 */
	{ "usb2",		"clk_m",		12000000,		true},		/* tegra_ehci.1 */
	{ "usbd",		"clk_m",		12000000,		true},		/* fsl-tegra-udc , utmip-pad , tegra_ehci.0 , tegra_otg */
	{ "tvdac",		"clk_m",		12000000,		false},
	{ "hdmi",		"clk_m",		12000000,		false},		/* tegra_dc.0, tegra_dc.1 */
	{ "tvo",		"clk_m",		12000000,		false},
	{ "cve",		"clk_m",		12000000,		false},
	{ "mipi",		"clk_m",		12000000,	false},
	{ "nor",		"clk_m",		12000000,	false},
	{ "owr",		"clk_m",		12000000,	false},		/* tegra_w1 */
	{ "la",			"clk_m",		12000000,	false},
	{ "bsev",		"clk_m",		12000000,	true},		/* tegra_aes */
	{ "bsea",		"clk_m",		12000000,	false},		/* tegra_avp */
	{ "vcp",		"clk_m",		12000000,	false},		/* tegra_avp */
	{ "sdmmc3",		"clk_m",		12000000,	false},
	{ "vfir",		"clk_m",		12000000,		false},
	{ "ndflash",	"pll_p",		12000000,		false},
	{ "ide",		"clk_m",		12000000,		false},
	{ "sbc4",		"clk_m",		12000000,		false}, 	/* tegra_spi_slave.3 */
	{ "sbc3",		"clk_m",		12000000,		false}, 	/* tegra_spi_slave.2 */
	{ "sbc2",		"clk_m",		12000000,		false}, 	/* tegra_spi_slave.1 */
	{ "sbc1",		"clk_m",		12000000,		false}, 	/* tegra_spi_slave.0 */
	{ "twc",		"clk_m",		12000000,		false},
	{ "xio",		"clk_m",		12000000,		false},
	{ "spi",		"clk_m",		12000000,		false},
	{ "pwm",		"clk_m",		12000000,		true},		/* tegra-pwm.0 tegra-pwm.1 tegra-pwm.2 tegra-pwm.3*/
	{ "kfuse",		"clk_m",		12000000,		false},		/* kfuse-tegra */ /* always on - no init req */
	{ "timer",		"clk_m",		12000000,		true},		/* timer */ /* always on - no init req */
	{ "clk_d",		"clk_m",		24000000,		true},
	// Use default for now
	//{ "pll_e",		"clk_m",		1200000000,		false},
	{ "pll_x",		"clk_m",		1000000000,		true}, // check here
	{ "cclk",		"pll_x",		1000000000,		true},
	{ "cpu",		"cclk",			1000000000,		true},
	{ "pll_u",		"clk_m",		480000000,		true},		/* USB ulpi clock */
	{ "pll_d",		"clk_m",		1000000,		true},		/* hdmi clock */
	{ "pll_p",		"clk_m",		216000000,	true},		/* must be always on */
	{ "host1x",		"pll_p",		108000000,	false},		/* tegra_grhost */
	{ "sdmmc4",		"pll_p",		48000000,	false},		/* sdhci-tegra.3 */
	{ "sdmmc2",		"pll_p",		48000000,	false},		/* sdhci-tegra.1 */
	{ "sdmmc1",		"pll_p",		48000000,	true},		/* sdhci-tegra.0 */
	{ "spdif_in",		"pll_p",		36000000,	false},
	{ "pll_p_out4",		"pll_p",		24000000,	true},		/* must be always on - USB ulpi */
	{ "pll_p_out3",		"pll_p",		72000000,	true},		/* must be always on - i2c, camera */
	{ "pll_p_out2",		"pll_p",		108000000,	true},		/* must be always on */
	{ "sclk",		"pll_p_out2",		108000000,	true},		/* must be always on */
	//{ "avp.sclk",		"sclk",			108000000,	false},		/* must be always on */
	{ "avp.sclk",		NULL,			108000000,	false},         /* must be always on */
	{ "cop",		"sclk",			108000000,	false},		/* must be always on */
	{ "hclk",		"sclk",			108000000,	true},		/* must be always on */
	{ "pclk",		"hclk",			54000000,	true},		/* must be always on */
	{ "pll_m",		"clk_m",		666000000,	true},		/* always on - memory clocks */
	{ NULL,			NULL,			0,		0},
};

void __init betelgeuse_clks_init(void)
{
	tegra_clk_init_from_table(betelgeuse_clk_init_table);
}
