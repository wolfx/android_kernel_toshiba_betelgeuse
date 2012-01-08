/*
 * arch/arm/mach-tegra/board-betelgeuse.c
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

#include <linux/console.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/platform_device.h>
#include <linux/serial_8250.h>
#include <linux/clk.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/dma-mapping.h>
#include <linux/fsl_devices.h>
#include <linux/platform_data/tegra_usb.h>
#include <linux/pda_power.h>
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
#include <mach/iomap.h>
#include <mach/sdhci.h>
#include <mach/gpio.h>
#include <mach/clk.h>
#include <mach/usb_phy.h>
//#include <mach/tegra2_i2s.h>
#include <mach/system.h>
#include <mach/nvmap.h>

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,38)	
#include <mach/suspend.h>
#else
#include "pm.h"
#endif

#include <linux/usb/android_composite.h>
#include <linux/usb/f_accessory.h>

#include "board.h"
#include "board-betelgeuse.h"
#include "clock.h"
#include "gpio-names.h"
#include "devices.h"
#include "wakeups-t2.h"


/* NVidia bootloader tags */
#define ATAG_NVIDIA		0x41000801

#define ATAG_NVIDIA_RM			0x1
#define ATAG_NVIDIA_DISPLAY		0x2
#define ATAG_NVIDIA_FRAMEBUFFER		0x3
#define ATAG_NVIDIA_CHIPSHMOO		0x4
#define ATAG_NVIDIA_CHIPSHMOOPHYS	0x5
#define ATAG_NVIDIA_PRESERVED_MEM_0	0x10000
#define ATAG_NVIDIA_PRESERVED_MEM_N	2
#define ATAG_NVIDIA_FORCE_32		0x7fffffff

struct tag_tegra {
	__u32 bootarg_key;
	__u32 bootarg_len;
	char bootarg[1];
};

static int __init parse_tag_nvidia(const struct tag *tag)
{
	return 0;
}
__tagtable(ATAG_NVIDIA, parse_tag_nvidia);

static struct tegra_suspend_platform_data betelgeuse_suspend = {
	.cpu_timer = 5000,
	.cpu_off_timer = 5000,
	.core_timer = 0x7e7e,
	.core_off_timer = 0x7f,
	.corereq_high = false,
	.sysclkreq_high = true,
	.suspend_mode = TEGRA_SUSPEND_LP1,
};

static void __init tegra_betelgeuse_init(void)
{
	struct clk *clk;

	/* force consoles to stay enabled across suspend/resume */
	// console_suspend_enabled = 0;	

	/* Init the suspend information */
	tegra_init_suspend(&betelgeuse_suspend);

	/* Set the SDMMC1 (wifi) tap delay to 6.  This value is determined
	 * based on propagation delay on the PCB traces. */
	clk = clk_get_sys("sdhci-tegra.0", NULL);
	if (!IS_ERR(clk)) {
		tegra_sdmmc_tap_delay(clk, 6);
		clk_put(clk);
	} else {
		pr_err("Failed to set wifi sdmmc tap delay\n");
	}

	/* Initialize the pinmux */
	betelgeuse_pinmux_init();

	/* Initialize the clocks - clocks require the pinmux to be initialized first */
	betelgeuse_clks_init();

	/* Register i2c devices - required for Power management and MUST be done before the power register */
	betelgeuse_i2c_register_devices();

	/* Register the power subsystem - Including the poweroff handler - Required by all the others */
	betelgeuse_power_register_devices();

	/* Register GPU devices */
	betelgeuse_gpu_register_devices();
	
	/* Register the USB device */
	betelgeuse_usb_register_devices();

	/* Register UART devices */
	betelgeuse_uart_register_devices();
	
	/* Register SPI devices */
	betelgeuse_spi_register_devices();

	/* Register Audio devices */
	betelgeuse_audio_register_devices();

	/* Register Jack devices */
//	betelgeuse_jack_register_devices();

	/* Register AES encryption devices */
	betelgeuse_aes_register_devices();

	/* Register Watchdog devices */
	betelgeuse_wdt_register_devices();

	/* Register all the keyboard devices */
	betelgeuse_keyboard_register_devices();
	
	/* Register touchscreen devices */
	betelgeuse_touch_register_devices();
	
	/* Register SDHCI devices */
	betelgeuse_sdhci_register_devices();

	/* Register accelerometer device */
	betelgeuse_sensors_register_devices();
	
	/* Register wlan devices */
	betelgeuse_wlan_register_devices();
	
	/* Register Bluetooth powermanagement devices */
	//betelgeuse_bt_pm_register_devices();
//	betelgeuse_bt_rfkill();
//	betelgeuse_setup_bluesleep();

	/* Register Camera powermanagement devices */
//	betelgeuse_camera_register_devices();
#if 0
	/* Finally, init the external memory controller and memory frequency scaling
   	   NB: This is not working on BETELGEUSE. And seems there is no point in fixing it,
	   as the EMC clock is forced to the maximum speed as soon as the 2D/3D engine
	   starts.*/
	betelgeuse_init_emc();
#endif
	
}

static void __init tegra_betelgeuse_fixup(struct machine_desc *desc,
	struct tag *tags, char **cmdline, struct meminfo *mi)
{
	mi->nr_banks = BETELGEUSE_MEM_BANKS;
	mi->bank[0].start = PHYS_OFFSET;
	mi->bank[0].size  = BETELGEUSE_MEM_SIZE - BETELGEUSE_GPU_MEM_SIZE;
} 

MACHINE_START(LEGACY, "Toshiba Folio 100")
	.boot_params	= 0x00000100,
	.map_io         = tegra_map_common_io,
	.init_early     = tegra_init_early,
	.init_irq       = tegra_init_irq,
	.timer          = &tegra_timer,
	.init_machine	= tegra_betelgeuse_init,
	.fixup			= tegra_betelgeuse_fixup,
MACHINE_END

#if 0
#define PMC_WAKE_STATUS 0x14

static int betelgeuse_wakeup_key(void)
{
	unsigned long status = 
		readl(IO_ADDRESS(TEGRA_PMC_BASE) + PMC_WAKE_STATUS);
	return status & TEGRA_WAKE_GPIO_PV2 ? KEY_POWER : KEY_RESERVED;
}
#endif


