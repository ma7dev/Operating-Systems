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
#define MAX_WORD_LENGTH 65536
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
int morse_f[5] = {0,0,1,0,2};
int morse_g[4] = {1,1,0,2};
int morse_h[5] = {0,0,0,0,2};
int morse_i[3] = {0,0,2};
int morse_j[5] = {0,1,1,1,2};
int morse_k[4] = {1,0,1,2};
int morse_l[5] = {0,1,0,0,2};
int morse_m[3] = {1,1,2};
int morse_n[3] = {1,0,2};
int morse_o[4] = {1,1,1,2};
int morse_p[5] = {0,1,1,0,2};
int morse_q[5] = {1,1,0,1,2};
int morse_r[4] = {0,1,0,2};
int morse_s[4] = {0,0,0,2};
int morse_t[3] = {1,2};
int morse_u[4] = {0,0,1,2};
int morse_v[5] = {0,0,0,1,2};
int morse_w[4] = {0,1,1,2};
int morse_x[5] = {1,0,0,1,2};
int morse_y[5] = {1,0,1,1,2};
int morse_z[5] = {1,1,0,0,2};
int morse_1[6] = {0,1,1,1,1,2};
int morse_2[6] = {0,0,1,1,1,2};
int morse_3[6] = {0,0,0,1,1,2};
int morse_4[6] = {0,0,0,0,1,2};
int morse_5[6] = {0,0,0,0,0,2};
int morse_6[6] = {1,0,0,0,0,2};
int morse_7[6] = {1,1,0,0,0,2};
int morse_8[6] = {1,1,1,0,0,2};
int morse_9[6] = {1,1,1,1,0,2};
int morse_0[6] = {1,1,1,1,1,2};

char* word_buffer;
size_t word_size = 0;

int* morse_data_buffer;
//int word_data[WORD_LENGTH] = {1,0,1,0,2,0,1,2,1,3};
int morse_data_buffer_size = 0;

static int panic_heartbeats;

struct heartbeat_trig_data {
	unsigned int index;
	unsigned int period;
	struct timer_list timer;
	unsigned int speed;
	unsigned int onetimedisplay;
	unsigned int is_on;
};

int is_first_time_display = 1;
 
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
    //snprintf(buf, size, "Hey there, I'm a dummy!\r\n");
    //size_t remainingBuffer = sizeof(word_buffer) - *ppos; 
    //size_t bytes = min(size, remainingBuffer);
    size_t bytes = min(size, MAX_WORD_LENGTH*sizeof(char));
    //snprintf(buf, bytes, word_buffer);
    //if (copy_to_user(buf+*ppos, word_buffer+*ppos, bytes))
    
    printk("Read:   size: %ld, word_buffer_size: %ld\n", size, MAX_WORD_LENGTH*sizeof(char));
    if(copy_to_user(buf, word_buffer, bytes)) 
       {
            return -EACCES;
       }
    printk("CS444 Dummy driver read %ld bytes: %s\r\n", word_size, word_buffer);
    //return strlen(buf);
    //if(bytes < size){
    //    (*ppos) += bytes;
    //}
    //return size;
    return 0;
}

static ssize_t dummy_write(struct file *file, const char __user *buf, size_t size, loff_t *ppos)
{
    //char lcl_buf[64];
    //memset(lcl_buf, 0, sizeof(lcl_buf));

	memset(word_buffer, 0, MAX_WORD_LENGTH*sizeof(char));
	//size_t remainingBuffer = (MAX_WORD_LENGTH*sizeof(char)) - *ppos;
	size_t bytes = min(size, MAX_WORD_LENGTH*sizeof(char));
	//size_t bytes = min(size, remainingBuffer);	
	printk("Write:   size: %ld, word_buffer_size: %ld\n", size, MAX_WORD_LENGTH*sizeof(char));
	
	if(copy_from_user(word_buffer, buf, bytes)){
		return -EACCES;
	}
	word_size = bytes;

	int i = 0;
	int j = 0;

	morse_data_buffer_size = 0;

	while(j < word_size && i < (MAX_WORD_LENGTH*7)){
		if(word_buffer[j] == 'a' || word_buffer[j] == 'A'){
			morse_data_buffer[i] = morse_a[0];
			morse_data_buffer[i+1] = morse_a[1];
			morse_data_buffer[i+2] = morse_a[2];
			i+=3;
		}
		else if(word_buffer[j] == 'b' || word_buffer[j] == 'B'){
			morse_data_buffer[i] = morse_b[0];
			morse_data_buffer[i+1] = morse_b[1];
			morse_data_buffer[i+2] = morse_b[2];
			morse_data_buffer[i+3] = morse_b[3];
			morse_data_buffer[i+4] = morse_b[4];
			i+=5;
		}
		else if(word_buffer[j] == 'c' || word_buffer[j] == 'C'){
			morse_data_buffer[i] = morse_c[0];
			morse_data_buffer[i+1] = morse_c[1];
			morse_data_buffer[i+2] = morse_c[2];
			morse_data_buffer[i+3] = morse_c[3];
			morse_data_buffer[i+4] = morse_c[4];
			i+=5;
		}
		else if(word_buffer[j] == 'd' || word_buffer[j] == 'D'){
			morse_data_buffer[i] = morse_d[0];
			morse_data_buffer[i+1] = morse_d[1];
			morse_data_buffer[i+2] = morse_d[2];
			morse_data_buffer[i+3] = morse_d[3];
			i+=4;
		}
		else if(word_buffer[j] == 'e' || word_buffer[j] == 'E'){
			morse_data_buffer[i] = morse_e[0];
			morse_data_buffer[i+1] = morse_e[1];
			i+=2;
		}
		else if(word_buffer[j] == 'f' || word_buffer[j] == 'F'){
			morse_data_buffer[i] = morse_f[0];
			morse_data_buffer[i+1] = morse_f[1];
			morse_data_buffer[i+2] = morse_f[2];
			morse_data_buffer[i+3] = morse_f[3];
			morse_data_buffer[i+4] = morse_f[4];
			i+=5;
		}
		else if(word_buffer[j] == 'g' || word_buffer[j] == 'G'){
			morse_data_buffer[i] = morse_g[0];
			morse_data_buffer[i+1] = morse_g[1];
			morse_data_buffer[i+2] = morse_g[2];
			morse_data_buffer[i+3] = morse_g[3];
			i+=4;
		}
		else if(word_buffer[j] == 'h' || word_buffer[j] == 'H'){
			morse_data_buffer[i] = morse_h[0];
			morse_data_buffer[i+1] = morse_h[1];
			morse_data_buffer[i+2] = morse_h[2];
			morse_data_buffer[i+3] = morse_h[3];
			morse_data_buffer[i+4] = morse_h[4];
			i+=5;
		}
		else if(word_buffer[j] == 'i' || word_buffer[j] == 'I'){
			morse_data_buffer[i] = morse_i[0];
			morse_data_buffer[i+1] = morse_i[1];
			morse_data_buffer[i+2] = morse_i[2];
			i+=3;
		}
		else if(word_buffer[j] == 'j' || word_buffer[j] == 'J'){
			morse_data_buffer[i] = morse_j[0];
			morse_data_buffer[i+1] = morse_j[1];
			morse_data_buffer[i+2] = morse_j[2];
			morse_data_buffer[i+3] = morse_j[3];
			morse_data_buffer[i+4] = morse_j[4];
			i+=5;
		}
		else if(word_buffer[j] == 'k' || word_buffer[j] == 'K'){
			morse_data_buffer[i] = morse_k[0];
			morse_data_buffer[i+1] = morse_k[1];
			morse_data_buffer[i+2] = morse_k[2];
			morse_data_buffer[i+3] = morse_k[3];
			i+=4;
		}
		else if(word_buffer[j] == 'l' || word_buffer[j] == 'L'){
			morse_data_buffer[i] = morse_l[0];
			morse_data_buffer[i+1] = morse_l[1];
			morse_data_buffer[i+2] = morse_l[2];
			morse_data_buffer[i+3] = morse_l[3];
			morse_data_buffer[i+4] = morse_l[4];
			i+=5;
		}
		else if(word_buffer[j] == 'm' || word_buffer[j] == 'M'){
			morse_data_buffer[i] = morse_m[0];
			morse_data_buffer[i+1] = morse_m[1];
			morse_data_buffer[i+2] = morse_m[2];
			i+=3;
		}
		else if(word_buffer[j] == 'n' || word_buffer[j] == 'N'){
			morse_data_buffer[i] = morse_n[0];
			morse_data_buffer[i+1] = morse_n[1];
			morse_data_buffer[i+2] = morse_n[2];
			i+=3;
		}
		else if(word_buffer[j] == 'o' || word_buffer[j] == 'O'){
			morse_data_buffer[i] = morse_o[0];
			morse_data_buffer[i+1] = morse_o[1];
			morse_data_buffer[i+2] = morse_o[2];
			morse_data_buffer[i+3] = morse_o[3];
			i+=4;
		}
		else if(word_buffer[j] == 'p' || word_buffer[j] == 'P'){
			morse_data_buffer[i] = morse_p[0];
			morse_data_buffer[i+1] = morse_p[1];
			morse_data_buffer[i+2] = morse_p[2];
			morse_data_buffer[i+3] = morse_p[3];
			morse_data_buffer[i+4] = morse_p[4];
			i+=5;
		}
		else if(word_buffer[j] == 'q' || word_buffer[j] == 'Q'){
			morse_data_buffer[i] = morse_q[0];
			morse_data_buffer[i+1] = morse_q[1];
			morse_data_buffer[i+2] = morse_q[2];
			morse_data_buffer[i+3] = morse_q[3];
			morse_data_buffer[i+4] = morse_q[4];
			i+=5;
		}
		else if(word_buffer[j] == 'r' || word_buffer[j] == 'R'){
			morse_data_buffer[i] = morse_r[0];
			morse_data_buffer[i+1] = morse_r[1];
			morse_data_buffer[i+2] = morse_r[2];
			morse_data_buffer[i+3] = morse_r[3];
			i+=4;
		}
		else if(word_buffer[j] == 's' || word_buffer[j] == 'S'){
			morse_data_buffer[i] = morse_s[0];
			morse_data_buffer[i+1] = morse_s[1];
			morse_data_buffer[i+2] = morse_s[2];
			morse_data_buffer[i+3] = morse_s[3];
			i+=4;
		}
		else if(word_buffer[j] == 't' || word_buffer[j] == 'T'){
			morse_data_buffer[i] = morse_t[0];
			morse_data_buffer[i+1] = morse_t[1];
			i+=2;
		}
		else if(word_buffer[j] == 'u' || word_buffer[j] == 'U'){
			morse_data_buffer[i] = morse_u[0];
			morse_data_buffer[i+1] = morse_u[1];
			morse_data_buffer[i+2] = morse_u[2];
			morse_data_buffer[i+3] = morse_u[3];
			i+=4;
		}
		else if(word_buffer[j] == 'v' || word_buffer[j] == 'V'){
			morse_data_buffer[i] = morse_v[0];
			morse_data_buffer[i+1] = morse_v[1];
			morse_data_buffer[i+2] = morse_v[2];
			morse_data_buffer[i+3] = morse_v[3];
			morse_data_buffer[i+4] = morse_v[4];
			i+=5;
		}
		else if(word_buffer[j] == 'w' || word_buffer[j] == 'W'){
			morse_data_buffer[i] = morse_w[0];
			morse_data_buffer[i+1] = morse_w[1];
			morse_data_buffer[i+2] = morse_w[2];
			morse_data_buffer[i+3] = morse_w[3];
			i+=4;
		}
		else if(word_buffer[j] == 'X' || word_buffer[j] == 'X'){
			morse_data_buffer[i] = morse_x[0];
			morse_data_buffer[i+1] = morse_x[1];
			morse_data_buffer[i+2] = morse_x[2];
			morse_data_buffer[i+3] = morse_x[3];
			morse_data_buffer[i+4] = morse_x[4];
			i+=5;
		}
		else if(word_buffer[j] == 'Y' || word_buffer[j] == 'Y'){
			morse_data_buffer[i] = morse_y[0];
			morse_data_buffer[i+1] = morse_y[1];
			morse_data_buffer[i+2] = morse_y[2];
			morse_data_buffer[i+3] = morse_y[3];
			morse_data_buffer[i+4] = morse_y[4];
			i+=5;
		}
		else if(word_buffer[j] == 'Z' || word_buffer[j] == 'Z'){
			morse_data_buffer[i] = morse_z[0];
			morse_data_buffer[i+1] = morse_z[1];
			morse_data_buffer[i+2] = morse_z[2];
			morse_data_buffer[i+3] = morse_z[3];
			morse_data_buffer[i+4] = morse_z[4];
			i+=5;
		}
		else if(word_buffer[j] == '1'){
			morse_data_buffer[i] = morse_1[0];
			morse_data_buffer[i+1] = morse_1[1];
			morse_data_buffer[i+2] = morse_1[2];
			morse_data_buffer[i+3] = morse_1[3];
			morse_data_buffer[i+4] = morse_1[4];
			morse_data_buffer[i+5] = morse_1[5];
			i+=6;
		}
		else if(word_buffer[j] == '2'){
			morse_data_buffer[i] = morse_2[0];
			morse_data_buffer[i+1] = morse_2[1];
			morse_data_buffer[i+2] = morse_2[2];
			morse_data_buffer[i+3] = morse_2[3];
			morse_data_buffer[i+4] = morse_2[4];
			morse_data_buffer[i+5] = morse_2[5];
			i+=6;
		}
		else if(word_buffer[j] == '3'){
			morse_data_buffer[i] = morse_3[0];
			morse_data_buffer[i+1] = morse_3[1];
			morse_data_buffer[i+2] = morse_3[2];
			morse_data_buffer[i+3] = morse_3[3];
			morse_data_buffer[i+4] = morse_3[4];
			morse_data_buffer[i+5] = morse_3[5];
			i+=6;
		}
		else if(word_buffer[j] == '4'){
			morse_data_buffer[i] = morse_4[0];
			morse_data_buffer[i+1] = morse_4[1];
			morse_data_buffer[i+2] = morse_4[2];
			morse_data_buffer[i+3] = morse_4[3];
			morse_data_buffer[i+4] = morse_4[4];
			morse_data_buffer[i+5] = morse_4[5];
			i+=6;
		}
		else if(word_buffer[j] == '5'){
			morse_data_buffer[i] = morse_4[0];
			morse_data_buffer[i+1] = morse_5[1];
			morse_data_buffer[i+2] = morse_5[2];
			morse_data_buffer[i+3] = morse_5[3];
			morse_data_buffer[i+4] = morse_5[4];
			morse_data_buffer[i+5] = morse_5[5];
			i+=6;
		}
		else if(word_buffer[j] == '6'){
			morse_data_buffer[i] = morse_6[0];
			morse_data_buffer[i+1] = morse_6[1];
			morse_data_buffer[i+2] = morse_6[2];
			morse_data_buffer[i+3] = morse_6[3];
			morse_data_buffer[i+4] = morse_6[4];
			morse_data_buffer[i+5] = morse_6[5];
			i+=6;
		}
		else if(word_buffer[j] == '7'){
			morse_data_buffer[i] = morse_7[0];
			morse_data_buffer[i+1] = morse_7[1];
			morse_data_buffer[i+2] = morse_7[2];
			morse_data_buffer[i+3] = morse_7[3];
			morse_data_buffer[i+4] = morse_7[4];
			morse_data_buffer[i+5] = morse_7[5];
			i+=6;
		}
		else if(word_buffer[j] == '8'){
			morse_data_buffer[i] = morse_8[0];
			morse_data_buffer[i+1] = morse_8[1];
			morse_data_buffer[i+2] = morse_8[2];
			morse_data_buffer[i+3] = morse_8[3];
			morse_data_buffer[i+4] = morse_8[4];
			morse_data_buffer[i+5] = morse_8[5];
			i+=6;
		}
		else if(word_buffer[j] == '9'){
			morse_data_buffer[i] = morse_9[0];
			morse_data_buffer[i+1] = morse_9[1];
			morse_data_buffer[i+2] = morse_9[2];
			morse_data_buffer[i+3] = morse_9[3];
			morse_data_buffer[i+4] = morse_9[4];
			morse_data_buffer[i+5] = morse_9[5];
			i+=6;
		}
		else if(word_buffer[j] == '0'){
			morse_data_buffer[i] = morse_0[0];
			morse_data_buffer[i+1] = morse_0[1];
			morse_data_buffer[i+2] = morse_0[2];
			morse_data_buffer[i+3] = morse_0[3];
			morse_data_buffer[i+4] = morse_0[4];
			morse_data_buffer[i+5] = morse_0[5];
			i+=6;
		}
		j++;
	}
	morse_data_buffer_size = i;
	printk("Morse data buffer size: %d", morse_data_buffer_size);
    //if (copy_from_user(lcl_buf, buf, min(size, sizeof(lcl_buf))))
//	if (copy_from_user(word_buffer+*ppos, buf+*ppos, bytes))
//        {
//           return -EACCES;
//        }

//	word_size = *ppos + bytes;

//	if(bytes < size){
//		*ppos += bytes;
//	}
    //printk("CS444 Dummy driver write %ld bytes: %s\r\n", size, lcl_buf);
	
	//update index to print from start
	is_first_time_display = 1;
	
	printk("CS444 Dummy driver write %ld bytes: %s\r\n", word_size, word_buffer);

    return bytes;
}

static void led_heartbeat_function(unsigned long data)
{	struct led_classdev *led_cdev = (struct led_classdev *) data;
	struct heartbeat_trig_data *heartbeat_data = led_cdev->trigger_data;
	unsigned long brightness = LED_OFF;
	unsigned long delay = 0;
	int current_action = -1;
	
	if (test_and_clear_bit(LED_BLINK_BRIGHTNESS_CHANGE, &led_cdev->work_flags))
		led_cdev->blink_brightness = led_cdev->new_blink_brightness;

	if(is_first_time_display){
		heartbeat_data->index = 0;
		is_first_time_display = 0;
	}

	//If the LED is on, we should turn it off and rest for one TIME_UNIT
	if(heartbeat_data->is_on){
		current_action = 4;
	}
	else{
		//Find the next action to perform in the word
		current_action = morse_data_buffer[heartbeat_data->index % morse_data_buffer_size];
	}

	if(heartbeat_data->index >= morse_data_buffer_size && heartbeat_data->onetimedisplay){
		current_action = -1;
	}
	if(heartbeat_data->index >= morse_data_buffer_size && !heartbeat_data->onetimedisplay){
		heartbeat_data->index = 0;
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
	
	//Change the speed
	delay = delay * (heartbeat_data->speed + 2);

	led_set_brightness_nosleep(led_cdev, brightness);
	mod_timer(&heartbeat_data->timer, jiffies + msecs_to_jiffies(delay));
}

static ssize_t led_onetimedisplay_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct heartbeat_trig_data *heartbeat_data = led_cdev->trigger_data;

	return sprintf(buf, "onetimedisplay: %u\n", heartbeat_data->onetimedisplay);
}

static ssize_t led_onetimedisplay_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct heartbeat_trig_data *heartbeat_data = led_cdev->trigger_data;
	unsigned long state;
	int ret;

	ret = kstrtoul(buf, 0, &state);
	if (ret)
		return ret;

	heartbeat_data->onetimedisplay = !!state;

	is_first_time_display = 1;

	return size;
}

static ssize_t led_speed_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct heartbeat_trig_data *heartbeat_data = led_cdev->trigger_data;

	return sprintf(buf, "Speedy: %u\n", heartbeat_data->speed);
}

static ssize_t led_speed_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct heartbeat_trig_data *heartbeat_data = led_cdev->trigger_data;
	unsigned long state;
	int ret;

	ret = kstrtoul(buf, 0, &state);
	if (ret)
		return ret;

	heartbeat_data->speed = state;

	return size;
}

static DEVICE_ATTR(speed, 0644, led_speed_show, led_speed_store);
static DEVICE_ATTR(onetimedisplay, 0644, led_onetimedisplay_show, led_onetimedisplay_store);

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
	rc = device_create_file(led_cdev->dev, &dev_attr_speed);
	if (rc) {
		kfree(led_cdev->trigger_data);
		return;
	}
	device_create_file(led_cdev->dev, &dev_attr_onetimedisplay);

	//Character Device
	dummy_init();
	//printk("Status activate: %d", status);

	//Allocate memory for word buffer
	word_buffer = (char*) kmalloc(MAX_WORD_LENGTH * sizeof(char), GFP_USER);
	memset(word_buffer, 0, MAX_WORD_LENGTH*sizeof(char));
	morse_data_buffer = (int*) kmalloc(MAX_WORD_LENGTH * 7 * sizeof(int), GFP_USER);
	memset(morse_data_buffer, 0, MAX_WORD_LENGTH  * 7 * sizeof(int));

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

	//Free memory for word buffer 
	kfree(word_buffer);
	
	if (led_cdev->activated) {
		del_timer_sync(&heartbeat_data->timer);
		device_remove_file(led_cdev->dev, &dev_attr_speed);
		device_remove_file(led_cdev->dev, &dev_attr_onetimedisplay);
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
