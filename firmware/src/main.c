#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <lvgl.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/led.h>


#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app);

static const struct device *pwm_led0 = DEVICE_DT_GET(DT_COMPAT_GET_ANY_STATUS_OKAY(pwm_leds));

#define MIN_PERIOD PWM_SEC(1U) / 128U
#define MAX_PERIOD PWM_SEC(1U)

#define STEP_SIZE PWM_USEC(2000)

static lv_obj_t * meter;

// static const struct gpio_dt_spec flesh_cs = GPIO_DT_SPEC_GET(DT_NODELABEL(fleshcs), gpios);
static const struct gpio_dt_spec flesh_cs = GPIO_DT_SPEC_GET(DT_ALIAS(fleshcs), gpios);

static void set_value(void * indic, int32_t v)
{
    // printk("UPDATE\n");
    lv_meter_set_indicator_end_value(meter, indic, v);
}

int main(void)
{
	int ret;

    if (!gpio_is_ready_dt(&flesh_cs)) {
		return 0;
	}
    printk("FLESH READY\n");

	ret = gpio_pin_configure_dt(&flesh_cs, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		return 0;
	}
    printk("FLESH CONFIG");

	if (!device_is_ready(pwm_led0)) {
		LOG_ERR("Device %s is not ready", pwm_led0->name);
		return 0;
	}

	// ret = led_on(pwm_led0, 0);
	// if (ret < 0) {
	// 	LOG_ERR("led on err=%d", ret);
	// 	return 0;
	// }

	// ret = led_set_brightness(pwm_led0, 0, 10);
	// if (ret < 0) {
	// 	LOG_ERR("brightness err=%d", ret);
	// 	return 0;
	// }
	// uint32_t pulse = STEP_SIZE;
	// printk("PWM-based blinky \n");
    //     if (!device_is_ready(pwm_led0.dev)) {
	// 	printk("Error: PWM device %s is not ready\n",
	// 	       pwm_led0.dev->name);
	// 	return 0;
	// }
    //     ret = pwm_set_pulse_dt(&pwm_led0, pulse);
	// 	if (ret){
	// 		printk("ERROR\n");
	// 		return 0;
	// 	}
        // int err;
	// char count_str[11] = {0};
	const struct device *display_dev;
	// lv_obj_t *hello_world_label;
	// lv_obj_t *count_label;
	printk("HELLO\nI AM WORKING!!\n");
	display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
	if (!device_is_ready(display_dev)) {
		LOG_ERR("Device not ready, aborting test");
		return 0;
	}

    static lv_style_t style_screen;
	lv_style_init(&style_screen);
	lv_style_set_bg_color(&style_screen, lv_color_white());
	lv_obj_add_style(lv_scr_act(), &style_screen, 0);

	meter = lv_meter_create(lv_scr_act());
	if (meter == NULL) {
		printk("METER NULL\n");
	}
    lv_obj_set_size(meter, 220, 220);
    lv_obj_center(meter);

    /*Create a scale for the minutes*/
    /*61 ticks in a 360 degrees range (the last and the first line overlaps)*/
    lv_meter_scale_t * scale_min = lv_meter_add_scale(meter);
	if (scale_min == NULL) {
		printk("SCALE MIN NULL\n");
	}
    lv_meter_set_scale_ticks(meter, scale_min, 61, 1, 10, lv_palette_main(LV_PALETTE_GREY));
    lv_meter_set_scale_range(meter, scale_min, 0, 60, 360, 270);

    /*Create another scale for the hours. It's only visual and contains only major ticks*/
    lv_meter_scale_t * scale_hour = lv_meter_add_scale(meter);
	if (scale_hour == NULL) {
		printk("SCALE HOUR NULL\n");
	}
    lv_meter_set_scale_ticks(meter, scale_hour, 12, 0, 0, lv_palette_main(LV_PALETTE_GREY));               /*12 ticks*/
    lv_meter_set_scale_major_ticks(meter, scale_hour, 1, 2, 20, lv_color_black(), 10);    /*Every tick is major*/
    lv_meter_set_scale_range(meter, scale_hour, 1, 12, 330, 300);       /*[1..12] values in an almost full circle*/

    LV_IMG_DECLARE(hand)

    /*Add a the hands from images*/
    lv_meter_indicator_t * indic_min = lv_meter_add_needle_img(meter, scale_min, &hand, 5, 5);
	if (indic_min == NULL) {
		printk("INDIC MIN NULL\n");
	}
    lv_meter_indicator_t * indic_hour = lv_meter_add_needle_img(meter, scale_hour, &hand, 5, 5);
	if (indic_hour == NULL) {
		printk("INDIC HOUR NULL\n");
	}

    /*Create an animation to set the value*/
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_exec_cb(&a, set_value);
    lv_anim_set_values(&a, 0, 60);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_time(&a, 2000);     /*2 sec for 1 turn of the minute hand (1 hour)*/
    lv_anim_set_var(&a, indic_min);
    lv_anim_start(&a);

    lv_anim_set_var(&a, indic_hour);
    lv_anim_set_time(&a, 24000);    /*24 sec for 1 turn of the hour hand*/
    lv_anim_set_values(&a, 0, 60);
    lv_anim_start(&a);
        lv_task_handler();
        display_blanking_off(display_dev);

    while(1) {
        k_msleep(5);
        lv_task_handler();
    }
        return 0;
}
