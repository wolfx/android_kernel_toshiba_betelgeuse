/*
 * drivers/misc/tegra-vibrator.c
 *
 * Miscellaneous timed output driver for vibrators implemented using i2c
 *
 * Copyright (c) 2012 Nicolas PINOT <npinot@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/hrtimer.h>
#include <linux/delay.h>
#include <linux/mfd/tps6586x.h>

#include "../../drivers/staging/android/timed_output.h"

static int s_timeout;

struct device *tps_dev ;

static void vibrator_start(void) 
{
 	int err = tps6586x_write(tps_dev, 0x5b, 0x8f );
	if (err < 0) {
		printk( "failed to program new time\n");
	} 
}

static void vibrator_stop(void) 
{
  	int err = tps6586x_write(tps_dev, 0x5b, 0x00 );
	if (err < 0) {
		printk( "failed to program new time\n");
	}
}

static void vibrator_enable(struct timed_output_dev *dev, int value)
{
	int mode;
	s_timeout = value;

	if (value > 0) {
		vibrator_start() ;
		msleep(value);
		vibrator_stop() ;
	} else {
		vibrator_stop() ;
	}
}

static inline struct device *to_tps6586x_dev(struct device *dev)
{
	return dev->parent;
}

static int vibrator_get_time(struct timed_output_dev *dev)
{
	return s_timeout;
}

static struct timed_output_dev tegra_vibrator = {
	.name		= "vibrator",
	.get_time	= vibrator_get_time,
	.enable		= vibrator_enable,
};

static int __devinit tps6586x_vibrator_probe(struct platform_device *pdev)
{
	struct tps6586x_vibrator_platform_data *pdata = pdev->dev.platform_data;
	tps_dev = to_tps6586x_dev(&pdev->dev);
	
	// Add init information 
	printk("tps6586x_vibrator_probe vibrator init\n");

	s_timeout = 0;
	
	dev_set_drvdata(&pdev->dev, &tegra_vibrator );
	device_init_wakeup(&pdev->dev, 1);
	
	int status = timed_output_dev_register(&tegra_vibrator);

	return status;
}

static int __devexit tps6586x_vibrator_remove(void)
{
	printk("tps6586x_vibrator_remove\n");
	timed_output_dev_unregister(&tegra_vibrator);
	tps_dev = NULL;
	return 0;
}

static struct platform_driver tps6586x_vibrator_driver = {
	.driver	= {
		.name	= "tps6586x-vibrator",
		.owner	= THIS_MODULE,
	},
	.probe	= tps6586x_vibrator_probe,
	.remove	= __devexit_p(tps6586x_vibrator_remove),
};

static int __init tps6586x_vibrator_init(void)
{
	return platform_driver_register(&tps6586x_vibrator_driver);
}
module_init(tps6586x_vibrator_init);

static void __exit tps6586x_vibrator_exit(void)
{
	platform_driver_unregister(&tps6586x_vibrator_driver);
}
module_exit(tps6586x_vibrator_exit);

MODULE_DESCRIPTION("timed output vibrator device for tps6586x");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:tps6586x-vibrator");
MODULE_AUTHOR("Nicolas Pinot <npinot@gmail.com>");
