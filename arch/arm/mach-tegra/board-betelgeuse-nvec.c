/*
 * Copyright (C) 2010 NVIDIA, Inc.
 *               2011 Artem Makhutov <artem@makhutov.org>
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
#include <mach/io.h>
#include <mach/iomap.h>
#include <linux/mfd/core.h>

#include "board.h"
#include "board-betelgeuse.h"
#include "devices.h"
#include "gpio-names.h"

#include "../../../drivers/staging/nvec/nvec.h"

static struct mfd_cell betelgeuse_nvec_devices[] = {
        {
                .name = "nvec-kbd",
                .id = 1,
        },
        {
                .name = "nvec-mouse",
                .id = 1,
        },
        {
                .name = "nvec-power",
                .id = 1,
        },
        {
                .name   = "nvec-event",
                .id = 1,
        },
};

static struct nvec_platform_data nvec_pdata = {
        .adapter        = 3,
        .gpio           = BETELGEUSE_NVEC_REQ,
        .nvec_devices   = betelgeuse_nvec_devices,
        .nr_nvec_devs   = ARRAY_SIZE(betelgeuse_nvec_devices),
        .has_poweroff   = true,
};

static struct platform_device nvec_device = {
        .name   = "nvec",
        .id     = 0,
        .dev    = {
                .platform_data = &nvec_pdata,
        }
};

static struct platform_device *betelgeuse_nv_devices[] __initdata = {
        &nvec_device,
};

int __init betelgeuse_nvec_register_devices(void)
{
	platform_add_devices(betelgeuse_nv_devices, ARRAY_SIZE(betelgeuse_nv_devices));
	return 0;
}
