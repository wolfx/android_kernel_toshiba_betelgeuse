/* OK, info extracted from a running .32 kernel */
/*
 * Copyright (C) 2011 Eduardo José Tagle <ejtagle@tutopia.com>
 * Copyright (C) 2011 Artem Makhutov <artem@makhutov.org>
 * Copyright (C) 2010 Google, Inc.
 *
 * Authors:
 *	Colin Cross <ccross@google.com>
 *	Eduardo José Tagle <ejtagle@tutopia.com>
 *	Artem Makhutov <artem@makhutov.org>
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

/* Tegra2 external memory controller initialization */
 
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/init.h>

#include "board-betelgeuse.h"
#include "tegra2_emc.h"

// This is the table used by nvidia ODM
static const struct tegra_emc_table betelgeuse_emc_tables[] = {
{
.rate = 166000,   /* SDRAM frequency */
.regs = {
            0x0000000A,   /* RC */
            0x00000016,   /* RFC */
            0x00000008,   /* RAS */
            0x00000003,   /* RP */
            0x00000004,   /* R2W */
            0x00000004,   /* W2R */
            0x00000002,   /* R2P */
            0x0000000B,   /* W2P */
            0x00000003,   /* RD_RCD */
            0x00000003,   /* WR_RCD */
            0x00000002,   /* RRD */
            0x00000001,   /* REXT */
            0x00000003,   /* WDV */
            0x00000004,   /* QUSE */
            0x00000003,   /* QRST */
            0x00000009,   /* QSAFE */
            0x0000000C,   /* RDV */
            0x000004DF,   /* REFRESH */
            0x00000000,   /* BURST_REFRESH_NUM */
            0x00000003,   /* PDEX2WR */
            0x00000003,   /* PDEX2RD */
            0x00000003,   /* PCHG2PDEN */
            0x00000003,   /* ACT2PDEN */
            0x00000001,   /* AR2PDEN */
            0x00000009,   /* RW2PDEN */
            0x000000C8,   /* TXSR */
            0x00000003,   /* TCKE */
            0x00000006,   /* TFAW */
            0x00000004,   /* TRPAB */
            0x00000008,   /* TCLKSTABLE */
            0x00000002,   /* TCLKSTOP */
            0x00000000,   /* TREFBW */
            0x00000003,   /* QUSE_EXTRA */
            0x00000002,   /* FBIO_CFG6 */
            0x00000000,   /* ODT_WRITE */
            0x00000000,   /* ODT_READ */
            0x00000083,   /* FBIO_CFG5 */
            0xA05C04AE,   /* CFG_DIG_DLL */
            0x007FC010,   /* DLL_XFORM_DQS */
            0x0000C10D,   /* DLL_XFORM_QUSE */
            0x00000000,   /* ZCAL_REF_CNT */
            0x00000000,   /* ZCAL_WAIT_CNT */
            0x00000000,   /* AUTO_CAL_INTERVAL */
            0x00000000,   /* CFG_CLKTRIM_0 */
            0x00000000,   /* CFG_CLKTRIM_1 */
            0x00000000,   /* CFG_CLKTRIM_2 */
}
},
{
.rate = 333000,   /* SDRAM frequency */
.regs = {
            0x00000014,   /* RC */
            0x0000002B,   /* RFC */
            0x0000000F,   /* RAS */
            0x00000005,   /* RP */
            0x00000004,   /* R2W */
            0x00000005,   /* W2R */
            0x00000003,   /* R2P */
            0x0000000B,   /* W2P */
            0x00000005,   /* RD_RCD */
            0x00000005,   /* WR_RCD */
            0x00000003,   /* RRD */
            0x00000001,   /* REXT */
            0x00000003,   /* WDV */
            0x00000004,   /* QUSE */
            0x00000003,   /* QRST */
            0x00000009,   /* QSAFE */
            0x0000000C,   /* RDV */
            0x000009FF,   /* REFRESH */
            0x00000000,   /* BURST_REFRESH_NUM */
            0x00000003,   /* PDEX2WR */
            0x00000003,   /* PDEX2RD */
            0x00000005,   /* PCHG2PDEN */
            0x00000005,   /* ACT2PDEN */
            0x00000001,   /* AR2PDEN */
            0x0000000E,   /* RW2PDEN */
            0x000000C8,   /* TXSR */
            0x00000003,   /* TCKE */
            0x0000000C,   /* TFAW */
            0x00000006,   /* TRPAB */
            0x00000008,   /* TCLKSTABLE */
            0x00000002,   /* TCLKSTOP */
            0x00000000,   /* TREFBW */
            0x00000003,   /* QUSE_EXTRA */
            0x00000002,   /* FBIO_CFG6 */
            0x00000000,   /* ODT_WRITE */
            0x00000000,   /* ODT_READ */
            0x00000083,   /* FBIO_CFG5 */
            0xE044048B,   /* CFG_DIG_DLL */
            0x007FB010,   /* DLL_XFORM_DQS */
            0x00003C14,   /* DLL_XFORM_QUSE */
            0x00000000,   /* ZCAL_REF_CNT */
            0x00000000,   /* ZCAL_WAIT_CNT */
            0x00000000,   /* AUTO_CAL_INTERVAL */
            0x00000000,   /* CFG_CLKTRIM_0 */
            0x00000000,   /* CFG_CLKTRIM_1 */
            0x00000000,   /* CFG_CLKTRIM_2 */
}
}
};

static const struct tegra_emc_chip betelgeuse_emc_chips[] = {
	{
		.description = "Shuttle memory",
		.mem_manufacturer_id = -1,
		.mem_revision_id1 = -1,
		.mem_revision_id2 = -1,
		.mem_pid = -1,
		.table = betelgeuse_emc_tables,
		.table_size = ARRAY_SIZE(betelgeuse_emc_tables),
	},
};

void __init betelgeuse_init_emc(void)
{
	tegra_init_emc(betelgeuse_emc_chips, ARRAY_SIZE(betelgeuse_emc_chips));
}
