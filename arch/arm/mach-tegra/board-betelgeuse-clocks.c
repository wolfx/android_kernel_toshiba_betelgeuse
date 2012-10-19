/*
 * arch/arm/mach-tegra/board-betelgeuse-clocks.c
 *
 * Copyright (C) 2011 - 2012 Artem Makhutov <artem@makhutov.org>
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

#include "clock.h"

static __initdata struct tegra_clk_init_table betelgeuse_clk_init_table[] = {
	/* name			parent			rate		enabled */
	{ "pll_p_out4",		"pll_p",		24000000,	true},		/* must be always on - USB ulpi */
	{ "i2s1",		"pll_a_out0",		0,		false},
	{ "i2s2",		"pll_a_out0",		0,		false},
	{ "spdif_out",		"pll_a_out0",		0,		false},
	{ "sdmmc2",		"clk_m",		48000000,	true },
	{ "pwm",		"clk_m",		12000000,	false},
	{ "pll_u",		NULL,			480000000,	true },
	{ NULL,			NULL,			0,		0},
};

void __init betelgeuse_clks_init(void)
{
	tegra_clk_init_from_table(betelgeuse_clk_init_table);
}
