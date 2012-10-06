/*
 * Copyright (C) 2011 Eduardo José Tagle <ejtagle@tutopia.com> 
	 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
	 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/adt7461.h>
//#include <linux/akm8975.h>
//#include <linux/i2c/ak8975.h>
#include <linux/lsm303dlh.h>

#include "board-betelgeuse.h"
#include "gpio-names.h"
#include "cpu-tegra.h"

/*
static struct akm8975_platform_data compass_platform_data = {
        .intr = TEGRA_GPIO_TO_IRQ(AKM8975_IRQ_GPIO),
        .init = NULL,
        .exit = NULL,
        .power_on = NULL,
        .power_off = NULL,
};
*/

/*
static struct akm8975_platform_data akm8975_pdata = {
	.gpio_data_ready_int = AKM8975_IRQ_GPIO,
};
*/

/*
static struct i2c_board_info __initdata ak8975_device = {
        I2C_BOARD_INFO("akm8975", 0x0c),
        .irq            = TEGRA_GPIO_TO_IRQ(AKM8975_IRQ_GPIO),
        .platform_data  = &compass_platform_data,
	//.platform_data = &akm8975_pdata,
};
*/

static struct i2c_board_info __initdata ak8975_device = {
	I2C_BOARD_INFO("mm_ak8975", 0x0c),
	.irq = TEGRA_GPIO_TO_IRQ(AKM8975_IRQ_GPIO),
};

static void betelgeuse_akm8975_init(void)
{
        //tegra_gpio_enable(AKM8975_IRQ_GPIO);
        //gpio_request(AKM8975_IRQ_GPIO, "ak8975");
        //gpio_direction_input(AKM8975_IRQ_GPIO);
        i2c_register_board_info(0, &ak8975_device, 1);
}

static struct adt7461_platform_data betelgeuse_adt7461_pdata = {
	.supported_hwrev = true,
	.ext_range = false,
	.therm2 = true,
	.conv_rate = 0x05,
	.offset = 0,
	.hysteresis = 0,
	.shutdown_ext_limit = 115,
	.shutdown_local_limit = 120,
	.throttling_ext_limit = 90,
	.alarm_fn = tegra_throttling_enable,
	.irq_gpio = ADT7461_IRQ_GPIO,
};


static struct i2c_board_info __initdata adt7461_device = {
	I2C_BOARD_INFO("adt7461", 0x4c),
	.irq = TEGRA_GPIO_TO_IRQ(ADT7461_IRQ_GPIO),
	.platform_data = &betelgeuse_adt7461_pdata,
};

static void betelgeuse_adt7461_init(void)
{
	tegra_gpio_enable(ADT7461_IRQ_GPIO);
	i2c_register_board_info(4, &adt7461_device, 1);
}

static struct lsm303dlh_platform_data betelgeuse_lsm303dlh_pdata = {
	.name_a = "acelerometer",
	.name_m = "magnetometer",
	.irq_a1 = TEGRA_GPIO_TO_IRQ(LSM303DHL_IRQ_GPIO),
	.irq_a2 = -1,
	.irq_m = -1,
	.axis_map_x = 1,
	.axis_map_y = 0,
	.axis_map_z = 2,
	.negative_x = 0,
	.negative_y = 1,
	.negative_z = 0,
};

static struct i2c_board_info __initdata lsm303dlh_device = {
        I2C_BOARD_INFO("lsm303dlh_a", 0x09),
        .irq = TEGRA_GPIO_TO_IRQ(LSM303DHL_IRQ_GPIO),
	.platform_data = &betelgeuse_lsm303dlh_pdata,
};

static void betelgeuse_lsm303dlh_init(void)
{
        i2c_register_board_info(0, &lsm303dlh_device, 1);
}

static struct i2c_board_info __initdata betelgeuse_cap_sensor_device = {
	I2C_BOARD_INFO("foliocap_kbd", 0x43),
	.irq = TEGRA_GPIO_TO_IRQ(FOLIOCAP_IRQ_GPIO),
};

static void betelgeuse_cap_sensor_init(void)
{
	i2c_register_board_info(4, &betelgeuse_cap_sensor_device, 1);
}

static struct i2c_board_info __initdata betelgeuse_eeprom_device = {
	I2C_BOARD_INFO("folio_eeprom", 0x50),
};

static void betelgeuse_eeprom_init(void)
{
	i2c_register_board_info(4, &betelgeuse_eeprom_device, 1);
}

int __init betelgeuse_sensors_register_devices(void)
{
	betelgeuse_cap_sensor_init();
	betelgeuse_akm8975_init();
	betelgeuse_adt7461_init();
	betelgeuse_lsm303dlh_init();
	betelgeuse_eeprom_init();
	return 0;
}
