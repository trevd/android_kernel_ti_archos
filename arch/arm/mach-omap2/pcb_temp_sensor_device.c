/*
 * PCB Temperature sensor device file
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#include <linux/err.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <plat/omap_device.h>
#include "control.h"
#include "pm.h"
#include <plat/pcb_temperature_sensor.h>

#define TWL6030_ADC_START_VALUE 126
#define TWL6030_ADC_END_VALUE   978
#define TWL6030_GPADC_CHANNEL     4

/*
 * Temperature values in milli degrees celsius ADC code values from 978 to 126
 */
static int adc_to_temp[] = {
	-40000, -40000, -40000, -40000, -40000, -37000, -35000, -33000, -31000,
	-29000, -28000, -27000, -25000, -24000, -23000, -22000, -21000, -20000,
	-19000, -18000, -17000, -17000, -16000, -15000, -14000, -14000, -13000,
	-12000, -12000, -11000, -11000, -10000, -10000, -9000, -8000, -8000,
	-7000, -7000, -6000, -6000, -6000, -5000, -5000, -4000, -4000, -3000,
	-3000, -3000, -2000, -2000, -1000, -1000, -1000, 0, 0, 0, 1000, 1000,
	1000, 2000, 2000, 2000, 3000, 3000, 3000, 4000, 4000, 4000, 5000, 5000,
	5000, 6000, 6000, 6000, 6000, 7000, 7000, 7000, 8000, 8000, 8000, 8000,
	9000, 9000, 9000, 9000, 10000, 10000, 10000, 10000, 11000, 11000,
	11000, 11000, 12000, 12000, 12000, 12000, 12000, 13000, 13000, 13000,
	13000, 14000, 14000, 14000, 14000, 14000, 15000, 15000, 15000, 15000,
	15000, 16000, 16000, 16000, 16000, 16000, 17000, 17000, 17000, 17000,
	17000, 18000, 18000, 18000, 18000, 18000, 19000, 19000, 19000, 19000,
	19000, 20000, 20000, 20000, 20000, 20000, 20000, 21000, 21000, 21000,
	21000, 21000, 22000, 22000, 22000, 22000, 22000, 22000, 23000, 23000,
	23000, 23000, 23000, 23000, 24000, 24000, 24000, 24000, 24000, 24000,
	25000, 25000, 25000, 25000, 25000, 25000, 25000, 26000, 26000, 26000,
	26000, 26000, 26000, 27000, 27000, 27000, 27000, 27000, 27000, 27000,
	28000, 28000, 28000, 28000, 28000, 28000, 29000, 29000, 29000, 29000,
	29000, 29000, 29000, 30000, 30000, 30000, 30000, 30000, 30000, 30000,
	31000, 31000, 31000, 31000, 31000, 31000, 31000, 32000, 32000, 32000,
	32000, 32000, 32000, 32000, 33000, 33000, 33000, 33000, 33000, 33000,
	33000, 33000, 34000, 34000, 34000, 34000, 34000, 34000, 34000, 35000,
	35000, 35000, 35000, 35000, 35000, 35000, 35000, 36000, 36000, 36000,
	36000, 36000, 36000, 36000, 36000, 37000, 37000, 37000, 37000, 37000,
	37000, 37000, 38000, 38000, 38000, 38000, 38000, 38000, 38000, 38000,
	39000, 39000, 39000, 39000, 39000, 39000, 39000, 39000, 39000, 40000,
	40000, 40000, 40000, 40000, 40000, 40000, 40000, 41000, 41000, 41000,
	41000, 41000, 41000, 41000, 41000, 42000, 42000, 42000, 42000, 42000,
	42000, 42000, 42000, 43000, 43000, 43000, 43000, 43000, 43000, 43000,
	43000, 43000, 44000, 44000, 44000, 44000, 44000, 44000, 44000, 44000,
	45000, 45000, 45000, 45000, 45000, 45000, 45000, 45000, 45000, 46000,
	46000, 46000, 46000, 46000, 46000, 46000, 46000, 47000, 47000, 47000,
	47000, 47000, 47000, 47000, 47000, 47000, 48000, 48000, 48000, 48000,
	48000, 48000, 48000, 48000, 48000, 49000, 49000, 49000, 49000, 49000,
	49000, 49000, 49000, 49000, 50000, 50000, 50000, 50000, 50000, 50000,
	50000, 50000, 51000, 51000, 51000, 51000, 51000, 51000, 51000, 51000,
	51000, 52000, 52000, 52000, 52000, 52000, 52000, 52000, 52000, 52000,
	53000, 53000, 53000, 53000, 53000, 53000, 53000, 53000, 53000, 54000,
	54000, 54000, 54000, 54000, 54000, 54000, 54000, 54000, 55000, 55000,
	55000, 55000, 55000, 55000, 55000, 55000, 55000, 56000, 56000, 56000,
	56000, 56000, 56000, 56000, 56000, 56000, 57000, 57000, 57000, 57000,
	57000, 57000, 57000, 57000, 57000, 58000, 58000, 58000, 58000, 58000,
	58000, 58000, 58000, 58000, 59000, 59000, 59000, 59000, 59000, 59000,
	59000, 59000, 59000, 60000, 60000, 60000, 60000, 60000, 60000, 60000,
	60000, 61000, 61000, 61000, 61000, 61000, 61000, 61000, 61000, 61000,
	62000, 62000, 62000, 62000, 62000, 62000, 62000, 62000, 62000, 63000,
	63000, 63000, 63000, 63000, 63000, 63000, 63000, 63000, 64000, 64000,
	64000, 64000, 64000, 64000, 64000, 64000, 64000, 65000, 65000, 65000,
	65000, 65000, 65000, 65000, 65000, 66000, 66000, 66000, 66000, 66000,
	66000, 66000, 66000, 66000, 67000, 67000, 67000, 67000, 67000, 67000,
	67000, 67000, 68000, 68000, 68000, 68000, 68000, 68000, 68000, 68000,
	68000, 69000, 69000, 69000, 69000, 69000, 69000, 69000, 69000, 70000,
	70000, 70000, 70000, 70000, 70000, 70000, 70000, 70000, 71000, 71000,
	71000, 71000, 71000, 71000, 71000, 71000, 72000, 72000, 72000, 72000,
	72000, 72000, 72000, 72000, 73000, 73000, 73000, 73000, 73000, 73000,
	73000, 73000, 74000, 74000, 74000, 74000, 74000, 74000, 74000, 74000,
	75000, 75000, 75000, 75000, 75000, 75000, 75000, 75000, 76000, 76000,
	76000, 76000, 76000, 76000, 76000, 76000, 77000, 77000, 77000, 77000,
	77000, 77000, 77000, 77000, 78000, 78000, 78000, 78000, 78000, 78000,
	78000, 79000, 79000, 79000, 79000, 79000, 79000, 79000, 79000, 80000,
	80000, 80000, 80000, 80000, 80000, 80000, 81000, 81000, 81000, 81000,
	81000, 81000, 81000, 82000, 82000, 82000, 82000, 82000, 82000, 82000,
	82000, 83000, 83000, 83000, 83000, 83000, 83000, 83000, 84000, 84000,
	84000, 84000, 84000, 84000, 84000, 85000, 85000, 85000, 85000, 85000,
	85000, 85000, 86000, 86000, 86000, 86000, 86000, 86000, 87000, 87000,
	87000, 87000, 87000, 87000, 87000, 88000, 88000, 88000, 88000, 88000,
	88000, 88000, 89000, 89000, 89000, 89000, 89000, 89000, 90000, 90000,
	90000, 90000, 90000, 90000, 91000, 91000, 91000, 91000, 91000, 91000,
	91000, 92000, 92000, 92000, 92000, 92000, 92000, 93000, 93000, 93000,
	93000, 93000, 93000, 94000, 94000, 94000, 94000, 94000, 94000, 95000,
	95000, 95000, 95000, 95000, 96000, 96000, 96000, 96000, 96000, 96000,
	97000, 97000, 97000, 97000, 97000, 97000, 98000, 98000, 98000, 98000,
	98000, 99000, 99000, 99000, 99000, 99000, 100000, 100000, 100000,
	100000, 100000, 100000, 101000, 101000, 101000, 101000, 101000, 102000,
	102000, 102000, 102000, 102000, 103000, 103000, 103000, 103000, 103000,
	104000, 104000, 104000, 104000, 104000, 105000, 105000, 105000, 105000,
	106000, 106000, 106000, 106000, 106000, 107000, 107000, 107000, 107000,
	108000, 108000, 108000, 108000, 108000, 109000, 109000, 109000, 109000,
	110000, 110000, 110000, 110000, 111000, 111000, 111000, 111000, 112000,
	112000, 112000, 112000, 112000, 113000, 113000, 113000, 114000, 114000,
	114000, 114000, 115000, 115000, 115000, 115000, 116000, 116000, 116000,
	116000, 117000, 117000, 117000, 118000, 118000, 118000, 118000, 119000,
	119000, 119000, 120000, 120000, 120000, 120000, 121000, 121000, 121000,
	122000, 122000, 122000, 123000, 123000, 123000, 124000, 124000, 124000,
	124000, 125000, 125000, 125000, 125000, 125000, 125000, 125000, 125000,
	125000, 125000, 125000, 125000,
};

static int adc_to_temp_conversion(int adc_val)
{
	if ((adc_val < TWL6030_ADC_START_VALUE) ||
		(adc_val > TWL6030_ADC_END_VALUE)) {
		pr_err("%s:Temp read is invalid %i\n", __func__, adc_val);
		return -EINVAL;
	}

	return adc_to_temp[TWL6030_ADC_END_VALUE - adc_val];
}

void pcb_temp_sensor_resume_idle(void)
{
}

void pcb_temp_sensor_prepare_idle(void)
{
}

static struct omap_device_pm_latency pcb_temp_sensor_latency[] = {
	{
	 .deactivate_func = omap_device_idle_hwmods,
	 .activate_func = omap_device_enable_hwmods,
	 .flags = OMAP_DEVICE_LATENCY_AUTO_ADJUST,
	}
};

static int pcb_temp_sensor_dev_init(struct omap_hwmod *oh, void *user)
{
	struct pcb_temp_sensor_pdata *temp_sensor_pdata;
	struct omap_device *od;
	static int i;
	int ret = 0;

	temp_sensor_pdata =
	    kzalloc(sizeof(struct pcb_temp_sensor_pdata), GFP_KERNEL);
	if (!temp_sensor_pdata) {
		pr_err
		    ("%s: Unable to allocate memory for %s.Error!\n",
			__func__, oh->name);
		return -ENOMEM;
	}

	temp_sensor_pdata->name = "pcb_temp_sensor";
	temp_sensor_pdata->gpadc_channel = TWL6030_GPADC_CHANNEL;
	temp_sensor_pdata->adc_to_temp_conversion = &adc_to_temp_conversion;

	od = omap_device_build(temp_sensor_pdata->name, i, oh,
			       temp_sensor_pdata,
			       sizeof(*temp_sensor_pdata),
			       pcb_temp_sensor_latency,
			       ARRAY_SIZE(pcb_temp_sensor_latency), 0);
	if (IS_ERR(od)) {
		pr_warning("%s: Could not build omap_device for %s: %s.\n\n",
			   __func__, temp_sensor_pdata->name, oh->name);
		ret = -EINVAL;
		goto done;
	}

	i++;
done:
	kfree(temp_sensor_pdata);
	return ret;
}

int __init pcb_devinit_temp_sensor(void)
{
	return omap_hwmod_for_each_by_class("thermal_sensor",
			pcb_temp_sensor_dev_init, NULL);
}

arch_initcall(pcb_devinit_temp_sensor);