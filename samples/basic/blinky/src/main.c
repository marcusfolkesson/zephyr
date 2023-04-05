/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>


#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/dac.h>

#define ZEPHYR_USER_NODE DT_PATH(zephyr_user)

#if (DT_NODE_HAS_PROP(ZEPHYR_USER_NODE, dac))
#define DAC_NODE DT_PHANDLE(ZEPHYR_USER_NODE, dac)
#else
#error "Unsupported board: see README and check /zephyr,user node"
#define DAC_NODE DT_INVALID_NODE
#endif

#define DAC_CHANNEL_ID 0
#define DAC_RESOLUTION 8

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   50

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)



static const struct device *const dac_dev = DEVICE_DT_GET(DAC_NODE);

static const struct dac_channel_cfg dac_ch_cfg = {
	.channel_id  = DAC_CHANNEL_ID,
	.resolution  = DAC_RESOLUTION
};



/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

void main(void)
{
	int ret;

	if (!gpio_is_ready_dt(&led)) {
		return;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return;
	}


	if (!device_is_ready(dac_dev)) {
		printk("DAC device %s is not ready\n", dac_dev->name);
		return;
	}

	ret = dac_channel_setup(dac_dev, &dac_ch_cfg);

	if (ret != 0) {
		printk("Setting up of DAC channel failed with code %d\n", ret);
		return;
	}

	printk("Generating sawtooth signal at DAC channel %d.\n",
		DAC_CHANNEL_ID);
	while (1) {
		/* Number of valid DAC values, e.g. 4096 for 12-bit DAC */
		const int dac_values = 1U << DAC_RESOLUTION;

		/*
		 * 1 msec sleep leads to about 4 sec signal period for 12-bit
		 * DACs. For DACs with lower resolution, sleep time needs to
		 * be increased.
		 * Make sure to sleep at least 1 msec even for future 16-bit
		 * DACs (lowering signal frequency).
		 */
		for (uint16_t i = 0; i < 256; i++) {
			ret = dac_write_value(dac_dev, DAC_CHANNEL_ID, i);
			if (ret != 0) {
				printk("dac_write_value() failed with code %d\n", ret);
				return;
			}

			ret = gpio_pin_toggle_dt(&led);
			/*k_msleep(SLEEP_TIME_MS);*/
			//k_sleep(K_MSEC(sleep_time));
		}
	}


}
