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
#include <mach/kbc.h>

#include "board.h"
#include "board-betelgeuse.h"
#include "devices.h"

#define BETELGEUSE_ROW_COUNT	1
#define BETELGEUSE_COL_COUNT	7

static const u32 kbd_keymap[] = {
	KEY(0, 0, KEY_VOLUMEUP),
	KEY(0, 1, KEY_VOLUMEDOWN),
	KEY(0, 2, KEY_RESERVED),
	KEY(0, 3, KEY_RESERVED),
	KEY(0, 4, KEY_RESERVED),
	KEY(0, 5, KEY_RESERVED),
	KEY(0, 6, KEY_RESERVED),
};

static const struct matrix_keymap_data keymap_data = {
	.keymap		= kbd_keymap,
	.keymap_size	= ARRAY_SIZE(kbd_keymap),
};

static struct tegra_kbc_wake_key betelgeuse_wake_cfg[] = {
	[0] = {
		.row = 0,
		.col = 0,
	},
};

static struct tegra_kbc_platform_data betelgeuse_kbc_platform_data = {
	.debounce_cnt = 2,
	.repeat_cnt = 5 * 32,
	.wakeup = true,
	.keymap_data = &keymap_data,
	.use_fn_map = false,
	.wake_cnt = 1,
	.wake_cfg = &betelgeuse_wake_cfg[0],
};

int __init betelgeuse_keyboard_register_devices(void)
{
	struct tegra_kbc_platform_data *data = &betelgeuse_kbc_platform_data;
	int i;
	tegra_kbc_device.dev.platform_data = &betelgeuse_kbc_platform_data;
	pr_info("Registering tegra-kbc\n");

	BUG_ON((KBC_MAX_ROW + KBC_MAX_COL) > KBC_MAX_GPIO);
	for (i = 0; i < KBC_MAX_ROW; i++) {
		data->pin_cfg[i].num = i;
		data->pin_cfg[i].is_row = true;
	}

	for (i = 0; i < KBC_MAX_COL; i++)
		data->pin_cfg[i + KBC_MAX_ROW].num = i;

	platform_device_register(&tegra_kbc_device);
	pr_info("Registering successful tegra-kbc\n");
	return 0;
}
