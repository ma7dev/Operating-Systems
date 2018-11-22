/*
 * LED Heartbeat Trigger
 *
 * Copyright (C) 2006 Atsushi Nemoto <anemo@mba.ocn.ne.jp>
 *
 * Based on Richard Purdie's ledtrig-timer.c and some arch's
 * CONFIG_HEARTBEAT code.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/sched.h>
#include <linux/sched/loadavg.h>
#include <linux/leds.h>
#include <linux/reboot.h>
#include <linux/uaccess.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/version.h>
#include "../leds.h"

#define TIME_UNIT 40
#define WORD_LENGTH 10
#define FIRST_MINOR 0
#define MINOR_CNT 1


static dev_t dev;
static struct cdev c_dev;
static struct class *cl;

//dot = 0
//dash = 1
//between letters = 2
//between words = 3

int morse_a[3] = {1,0,2};
int morse_b[5] = {1,0,0,0,2};
int morse_c[5] = {1,0,1,0,2};
int morse_d[4] = {1,0,0,2};
int morse_e[2] = {0,2};
int morse_f[6] = {0,0,1,0,2};
int morse_g[5] = {1,1,0,2};
int morse_h[6] = {0,0,0,0,2};
int morse_i[4] = {0,0,2};
int morse_j[6] = {0,1,1,1,2};
int morse_k[5] = {1,0,1,2};
int morse_l[6] = {0,1,0,0,2};
int morse_m[4] = {1,1,2};
int morse_n[4] = {1,0,2};
int morse_o[5] = {1,1,1,2};
int morse_p[6] = {0,1,1,0,2};
int morse_q[6] = {1,1,0,1,2};
int morse_r[5] = {0,1,0,2};
int morse_s[5] = {0,0,0,2};
int morse_t[3] = {1,2};
int morse_u[5] = {0,0,1,2};
int morse_v[6] = {0,0,0,1,2};
int morse_w[5] = {0,1,1,2};
int morse_x[6] = {1,0,0,1,2};
int morse_y[6] = {1,0,1,1,2};
int morse_z[6] = {1,1,0,0,2};
int morse_1[7] = {0,1,1,1,1,2};
int morse_2[7] = {0,0,1,1,1,2};
int morse_3[7] = {0,0,0,1,1,2};
int morse_4[7] = {0,0,0,0,1,2};
int morse_5[7] = {0,0,0,0,0,2};
int morse_6[7] = {1,0,0,0,0,2};
int morse_7[7] = {1,1,0,0,0,2};
int morse_8[7] = {1,1,1,0,0,2};
int morse_9[7] = {1,1,1,1,0,2};
int morse_0[7] = {1,1,1,1,1,2};


int word_data[WORD_LENGTH] = {1,0,1,0,2,0,1,2,1,3};

static int panic_heartbeats;

struct heartbeat_trig_data {
	unsigned int index;
	unsigned int period;
	struct timer_list timer;
	unsigned int invert;
	unsigned int is_on;
};

 
static int my_open(struct inode *i, struct file *f)
{
    printk("CS444 Dummy driver open\r\n");
    return 0;
}
static int my_close(struct inode *i, struct file *f)
{
    printk("CS444 Dummy driver close\r\n");
    return 0;
}

static ssize_t dummy_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
    printk("CS444 Dummy driver read\r\n");
    snprintf(buf, size, "Hey there, I'm a dummy!\r\n");
    return strlen(buf);
}

static ssize_t dummy_write(struct file *file, const char __user *buf, size_t size, loff_t *ppos)
{
    char lcl_buf[64];

    memset(lcl_buf, 0, sizeof(lcl_buf));

    if (copy_from_user(lcl_buf, buf, min(size, sizeof(lcl_buf))))
        {
            return -EACCES;
        }

    printk("CS444 Dummy driver write %ld bytes: %s\r\n", size, lcl_buf);

    return size;
}

static void led_heartbeat_function(unsigned long data)
{	struct led_classdev *led_cdev = (struct led_classdev *) data;
	struct heartbeat_trig_data *heartbeat_data = led_cdev->trigger_data;
	unsigned long brightness = LED_OFF;
	unsigned long delay = 0;
	int current_action = -1;

	//ptr = fopen("/dev/cs444_dummy","r");
	

	if (test_and_clear_bit(LED_BLINK_BRIGHTNESS_CHANGE, &led_cdev->work_flags))
		led_cdev->blink_brightness = led_cdev->new_blink_brightness;

	//If the LED is on, we should turn it off and rest for one TIME_UNIT
	if(heartbeat_data->is_on){
		current_action = 4;
	}
	else{
		//Find the next action to perform in the word
		current_action = word_data[heartbeat_data->index % WORD_LENGTH];
	}

	switch (current_action) {
		//dot 
		case 0:
			delay = TIME_UNIT;
			heartbeat_data->index++;
			brightness = led_cdev->blink_brightness;
			heartbeat_data->is_on = 1;
			break;
		//dash
		case 1:
			delay = TIME_UNIT * 3;
			heartbeat_data->index++;
			brightness = led_cdev->blink_brightness;
			heartbeat_data->is_on = 1;
			break;
		//medium gap (between letters)
		case 2:
			delay = TIME_UNIT * 2;
			heartbeat_data->index++;
			brightness = LED_OFF;
			heartbeat_data->is_on = 0;
			break;
		//big gap (between words)
		case 3:
			delay = TIME_UNIT * 6;
			heartbeat_data->index++;
			brightness = LED_OFF;
			heartbeat_data->is_on = 0;
			break;
		//the LED is already on meaning we have to wait
		//for the minimum time (between dots and dashes)
		case 4:
			delay = TIME_UNIT;
			brightness = LED_OFF;
			heartbeat_data->is_on = 0;
			break;
		//the LED is already on meaning we have to wait
		//for the minimum time (between dots and dashes)
		default:
			delay = TIME_UNIT;
			brightness = LED_OFF;
			heartbeat_data->is_on = 0;
			break;
	}
	
	//Change the speed based in invert 
	delay = delay * (heartbeat_data->invert + 1);

	led_set_brightness_nosleep(led_cdev, brightness);
	mod_timer(&heartbeat_data->timer, jiffies + msecs_to_jiffies(delay));
}

static ssize_t led_invert_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct heartbeat_trig_data *heartbeat_data = led_cdev->trigger_data;

	return sprintf(buf, "Speedy: %u\n", heartbeat_data->invert);
}

static ssize_t led_invert_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct heartbeat_trig_data *heartbeat_data = led_cdev->trigger_data;
	unsigned long state;
	int ret;

	ret = kstrtoul(buf, 0, &state);
	if (ret)
		return ret;

	//heartbeat_data->invert = !!state;
	heartbeat_data->invert = state;

	return size;
}

static DEVICE_ATTR(invert, 0644, led_invert_show, led_invert_store);

static struct file_operations dummy_fops =
{
    .owner = THIS_MODULE,
    .open = my_open,
    .read = dummy_read,
    .write = dummy_write,
    .release = my_close
};

static int __init dummy_init(void)
{
    int ret;
    struct device *dev_ret;
 
    // Allocate the device
    if ((ret = alloc_chrdev_region(&dev, FIRST_MINOR, MINOR_CNT, "cs444_dummy")) < 0)
    {
        return ret;
    }
 
    cdev_init(&c_dev, &dummy_fops);
 
    if ((ret = cdev_add(&c_dev, dev, MINOR_CNT)) < 0)
    {
        return ret;
    }
     
    // Allocate the /dev device (/dev/cs444_dummy)
    if (IS_ERR(cl = class_create(THIS_MODULE, "char")))
    {
        cdev_del(&c_dev);
        unregister_chrdev_region(dev, MINOR_CNT);
        return PTR_ERR(cl);
    }
    if (IS_ERR(dev_ret = device_create(cl, NULL, dev, NULL, "cs444_dummy")))
    {
        class_destroy(cl);
        cdev_del(&c_dev);
        unregister_chrdev_region(dev, MINOR_CNT);
        return PTR_ERR(dev_ret);
    }

    printk("CS444 Dummy Driver has been loaded!\r\n");
 
    return 0;
}
 
static void __exit dummy_exit(void)
{
    device_destroy(cl, dev);
    class_destroy(cl);
    cdev_del(&c_dev);
    unregister_chrdev_region(dev, MINOR_CNT);
}

static void heartbeat_trig_activate(struct led_classdev *led_cdev)
{
	struct heartbeat_trig_data *heartbeat_data;
	int rc;

	heartbeat_data = kzalloc(sizeof(*heartbeat_data), GFP_KERNEL);
	if (!heartbeat_data)
		return;

	led_cdev->trigger_data = heartbeat_data;
	rc = device_create_file(led_cdev->dev, &dev_attr_invert);
	if (rc) {
		kfree(led_cdev->trigger_data);
		return;
	}

	//Character Device
	int status = dummy_init();
	printk("Status activate: %d", status);


	setup_timer(&heartbeat_data->timer,
		    led_heartbeat_function, (unsigned long) led_cdev);
	heartbeat_data->index = 0;
	heartbeat_data->is_on = 0;
	if (!led_cdev->blink_brightness)
		led_cdev->blink_brightness = led_cdev->max_brightness;
	led_heartbeat_function(heartbeat_data->timer.data);
	set_bit(LED_BLINK_SW, &led_cdev->work_flags);
	led_cdev->activated = true;
}

static void heartbeat_trig_deactivate(struct led_classdev *led_cdev)
{
	struct heartbeat_trig_data *heartbeat_data = led_cdev->trigger_data;

	//Character Device
	dummy_exit();
	printk("Status deactivate");
	
	if (led_cdev->activated) {
		del_timer_sync(&heartbeat_data->timer);
		device_remove_file(led_cdev->dev, &dev_attr_invert);
		kfree(heartbeat_data);
		clear_bit(LED_BLINK_SW, &led_cdev->work_flags);
		led_cdev->activated = false;
	}
}

static struct led_trigger heartbeat_led_trigger = {
	.name     = "morse",
	.activate = heartbeat_trig_activate,
	.deactivate = heartbeat_trig_deactivate,
};

static int heartbeat_reboot_notifier(struct notifier_block *nb,
				     unsigned long code, void *unused)
{
	led_trigger_unregister(&heartbeat_led_trigger);
	return NOTIFY_DONE;
}

static int heartbeat_panic_notifier(struct notifier_block *nb,
				     unsigned long code, void *unused)
{
	panic_heartbeats = 1;
	return NOTIFY_DONE;
}

static struct notifier_block heartbeat_reboot_nb = {
	.notifier_call = heartbeat_reboot_notifier,
};

static struct notifier_block heartbeat_panic_nb = {
	.notifier_call = heartbeat_panic_notifier,
};

static int __init heartbeat_trig_init(void)
{
	int rc = led_trigger_register(&heartbeat_led_trigger);

	if (!rc) {
		atomic_notifier_chain_register(&panic_notifier_list,
					       &heartbeat_panic_nb);
		register_reboot_notifier(&heartbeat_reboot_nb);
	}
	return rc;
}

static void __exit heartbeat_trig_exit(void)
{
	unregister_reboot_notifier(&heartbeat_reboot_nb);
	atomic_notifier_chain_unregister(&panic_notifier_list,
					 &heartbeat_panic_nb);
	led_trigger_unregister(&heartbeat_led_trigger);
}

module_init(heartbeat_trig_init);
module_exit(heartbeat_trig_exit);

MODULE_AUTHOR("Atsushi Nemoto <anemo@mba.ocn.ne.jp>");
MODULE_DESCRIPTION("Heartbeat LED trigger");
MODULE_LICENSE("GPL");
