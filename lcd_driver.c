/***************************************************************************//**
*  \file       driver.c
*
*  \details    Simple I2C driver (Grove RGB LCD V4.0  Display Interface)
*
*  \author     Divyesh Kapadiya
*
*  \Tested with Linux Beaglebone Black
*
* *******************************************************************************/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/sysfs.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/kobject.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>

#define I2C_BUS_AVAILABLE   (2)              // I2C Bus available in our Beaglebone Black

#define LCD_ADDRESS 0x3E
#define RGB_ADDRESS 0x62

#define DEVICE_NAME "rgb_lcd"
#define CLASS_NAME "rgb_lcd_class"

#define COMMAND_MODE 0x00
#define DATA_MODE 0x40

#define LCD_MAX_CHARS_PER_LINE 16


/********************************-> RGB LCD start <-****************************************/


static uint8_t rgb_reg_reset = 0x00;
static uint8_t rgb_reg_clock = 0x01;
static uint8_t rgb_reg_led = 0x08;
static uint8_t rgb_reg_red = 0x04;
static uint8_t rgb_reg_green = 0x03;
static uint8_t rgb_reg_blue = 0x02;
static uint8_t rgb_val_reset = 0x00;
static uint8_t rgb_val_clock = 0x00;
static uint8_t rgb_val_led = 0xAA;
static uint8_t rgb_val_red = 0xFF;
static uint8_t rgb_val_green = 0xFF;
static uint8_t rgb_val_blue = 0xFF;

static char lcd_text[256] = "Hello, World!";

/* Array of LCD initialization commands:
** 0x28: Function set - 2 line, 8-bit mode, 5x8 dots
** 0x08: Display off
** 0x01: Clear display
** 0x06: Entry mode set - increment cursor, no display shift
** 0x0C: Display on, cursor off, blink off
*/
static uint8_t lcd_init_cmds[] = {0x28, 0x08, 0x01, 0x06, 0x0C};

// Corresponding delays in milliseconds after each command
static unsigned int lcd_init_delays[] = {50, 50, 50, 50, 50};

uint8_t i = 0;

struct i2c_adapter *adapter = NULL;

static struct i2c_client *lcd_client;
static struct i2c_client *rgb_client;

static struct i2c_board_info lcd_board_info = {
    I2C_BOARD_INFO("lcd", LCD_ADDRESS)
};

static struct i2c_board_info rgb_board_info = {
    I2C_BOARD_INFO("rgb", RGB_ADDRESS)
};

/*
** This function writes the data into the I2C client
**
**  Arguments:
**      client -> address of client rgb or lcd
**	reg    -> control byte
**      len    -> Length of the data
**   
*/
static int i2c_write_data(struct i2c_client *client, uint8_t reg, uint8_t data) {
    int ret;
    uint8_t buf[2] = {reg, data};

    pr_info("Sending data to I2C device: register=0x%02X, data=0x%02X\n", buf[0], buf[1]);

    ret = i2c_master_send(client, buf, 2);
    if (ret < 0) {  
        pr_info("Failed to write to the i2c device\n");
    }
    pr_info("send bytes %d", ret);
    return ret;
}

/*
** Initialize the LCD
*/
static void initialize_lcd(struct i2c_client *client) {
    pr_info("Initilize of LCD starting...\n");

    msleep(50);
    i = 0; 
    while(i < sizeof(lcd_init_cmds)) {
        i2c_write_data(client, COMMAND_MODE, lcd_init_cmds[i]);
        msleep(lcd_init_delays[i]);
	i++;
    }
    
    pr_info("Initilize of LCD Completed\n");
}

/*
** Initialize the RGB backlight
*/
static void initialize_rgb(struct i2c_client *client) {
    pr_info("Initilize of RGB starting...\n");
    
    i2c_write_data(client, rgb_reg_reset, rgb_val_reset);
    i2c_write_data(client, rgb_reg_clock, rgb_val_clock);
    i2c_write_data(client, rgb_reg_led, rgb_val_led);
    i2c_write_data(client, rgb_reg_red, rgb_val_red);
    i2c_write_data(client, rgb_reg_green, rgb_val_green);
    i2c_write_data(client, rgb_reg_blue, rgb_val_blue);

    pr_info("Initilize of RGB Completed\n");
}

/*
** Write string to the LCD
*/
static void lcd_write_string(struct i2c_client *client, const char *str) {
    
    pr_info("Writing is start on display\n");
    i2c_write_data(client, COMMAND_MODE, 0x80);  // Set DDRAM address to 0x00 (start of second line)
    
    i = 0;
    while ((i < LCD_MAX_CHARS_PER_LINE) && (*str)) {
        i2c_write_data(client, DATA_MODE, *str++);
	i++;
    }

    // Move the cursor to the beginning of the second line if there are more characters to display
    if (*str) {
        i2c_write_data(client, COMMAND_MODE, 0xC0);  // Set DDRAM address to 0x40 (start of second line)

        while (*str) {
            i2c_write_data(client, DATA_MODE, *str++);
        }
    }

    pr_info("Writing is completed on display\n");
}

static int i2c_driver_probe(struct i2c_client *client, const struct i2c_device_id *id) {
    pr_info("Grove LCD RGB Backlight V4.0 driver probe\n");

    if (client->addr == RGB_ADDRESS) {
        pr_info("client->addr is RGB\n");
        initialize_rgb(client);
        rgb_client = client;
    } else if (client->addr == LCD_ADDRESS) {
        pr_info("client->addr is LCD\n");
        initialize_lcd(client);
        lcd_write_string(client, lcd_text);
        lcd_client = client;
    }

    return 0;
}

static int i2c_driver_remove(struct i2c_client *client) {
    pr_info("Grove LCD RGB Backlight V4.0 driver remove\n");
    return 0;
}

static const struct i2c_device_id i2c_driver_id[] = {
    { "lcd", 0 },
    { "rgb", 0 },
    { }
};

MODULE_DEVICE_TABLE(i2c, i2c_driver_id);

static struct i2c_driver etx_i2c_driver = {
    .driver = {
        .name = "grove_lcd_rgb",
        .owner = THIS_MODULE,
    },
    .probe = i2c_driver_probe,
    .remove = i2c_driver_remove,
    .id_table = i2c_driver_id,
};


/********************************-> RGB LCD end<-****************************************/


/********************************-> sysfs start <-****************************************/


struct kobject *kobj_ref;


static ssize_t rgb_sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    printk(KERN_INFO "Sysfs - RGB Read!!!\n");
    return sprintf(buf, "R : %02d, G : %02d, B : %02d\n", rgb_val_red, rgb_val_green, rgb_val_blue);
}

static ssize_t rgb_sysfs_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
    unsigned int red, green, blue;
    printk(KERN_INFO "Sysfs - RGB Write!!!\n");
    if (sscanf(buf, "Red : %u, Green : %u, Blue : %u", &red, &green, &blue) == 3) {
        
	if(red > 255 || red < 0){
	    pr_info("Red value is out of range\nContinue with default(255)\n");
	    rgb_val_red = 255;
	} else {
	    rgb_val_red = red;
	}

	if(green > 255 || green < 0){
	    pr_info("Red value is out of range\nContinue with default(255)\n");
	    rgb_val_green = 255;
	} else {
	    rgb_val_green = green;
	}

	if(blue > 255 || blue < 0){
	    pr_info("Red value is out of range\nContinue with default(255)\n");
	    rgb_val_blue = 255;
	} else {
	    rgb_val_blue = blue;
	}

	i2c_write_data(rgb_client, rgb_reg_red, rgb_val_red);
        i2c_write_data(rgb_client, rgb_reg_green, rgb_val_green);
        i2c_write_data(rgb_client, rgb_reg_blue, rgb_val_blue);
        
	return count;
    } else {
	    pr_info("Invalid Write format\n format is -> Red : <value>, Green : <value>, Blue : <value>"); 
    }
    return -EINVAL;
}

static struct kobj_attribute rgb_attr = __ATTR(rgb_color, 0660, rgb_sysfs_show, rgb_sysfs_store);


static ssize_t lcd_sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    printk(KERN_INFO "Sysfs - LCD Read!!!\n");
    return sprintf(buf, "Displayed Text : %s\n", lcd_text);
}

static ssize_t lcd_sysfs_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
    printk(KERN_INFO "Sysfs - LCD Write!!!\n");
    strncpy(lcd_text, buf, sizeof(lcd_text));
    lcd_text[count - 1] = '\0';
    initialize_lcd(lcd_client);
    lcd_write_string(lcd_client, lcd_text);

    return count;
}

static struct kobj_attribute lcd_attr = __ATTR(lcd_text, 0660, lcd_sysfs_show, lcd_sysfs_store);


/********************************-> sysfs end <-****************************************/



/********************************-> procfs start <-****************************************/

struct proc_dir_entry *rgb_lcd_proc_file;

#define PROCFS_MAX_SIZE 1024

static ssize_t rgb_lcd_proc_read(struct file *file, char __user *buffer, size_t len, loff_t *offset) {
    char *proc_buf;
    int proc_buf_len;
    ssize_t ret;

    if (*offset > 0) {
        return 0;
    }

    proc_buf = kmalloc(PROCFS_MAX_SIZE, GFP_KERNEL);
    if (!proc_buf) {
        return -ENOMEM;
    }

    proc_buf_len = snprintf(proc_buf, PROCFS_MAX_SIZE, "R : %02X, G : %02X, B : %02X\nDisplayed Text : %s\n",
                            rgb_val_red, rgb_val_green, rgb_val_blue, lcd_text);

    if (proc_buf_len < 0) {
        kfree(proc_buf);
        return -EFAULT;
    }

    ret = simple_read_from_buffer(buffer, len, offset, proc_buf, proc_buf_len);

    kfree(proc_buf);
    return ret;
}

static const struct file_operations rgb_lcd_proc_fops = {
    .owner = THIS_MODULE,
    .read = rgb_lcd_proc_read,
};

/********************************-> procfs end <-****************************************/


/********************************-> IOCTL start <-****************************************/

struct rgb_value {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

#define WR_RGB_VALUE _IOW('a', 1, struct rgb_value)
#define RD_RGB_VALUE _IOR('a', 2, struct rgb_value)
#define WR_LCD_TEXT _IOW('a', 3, char*)
#define RD_LCD_TEXT _IOR('a', 4, char*)

int32_t value = 0;

dev_t dev = 0;
static struct class *dev_class;
static struct cdev etx_cdev;

static int rgb_lcd_open(struct inode *inode, struct file *file);
static int rgb_lcd_release(struct inode *inode, struct file *file);
static ssize_t rgb_lcd_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t rgb_lcd_write(struct file *filp, const char *buf, size_t len, loff_t * off);
static long rgb_lcd_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

static struct file_operations fops =
{
	.owner          = THIS_MODULE,
	.read           = rgb_lcd_read,
	.write          = rgb_lcd_write,
	.open           = rgb_lcd_open,
	.unlocked_ioctl = rgb_lcd_ioctl,
	.release        = rgb_lcd_release,
};

static int rgb_lcd_open(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "Device File Opened...!!!\n");
	return 0;
}

static int rgb_lcd_release(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "Device File Closed...!!!\n");
	return 0;
}

static ssize_t rgb_lcd_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
	printk(KERN_INFO "Read Function\n");
	return 0;
}
static ssize_t rgb_lcd_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
	printk(KERN_INFO "Write function\n");
	return 0;
}

static long rgb_lcd_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    switch (cmd) {

	case WR_RGB_VALUE:
	{ 
            struct rgb_value rgb;
	    
	    if (copy_from_user(&rgb, (struct rgb_value *)arg, sizeof(struct rgb_value))) {
                return -EFAULT;
            }

            rgb_val_red = rgb.red;
            rgb_val_green = rgb.green;
            rgb_val_blue = rgb.blue;

            initialize_rgb(rgb_client);
        }
	    break;

	case RD_RGB_VALUE:
	{   
            struct rgb_value rgb;
	    
	    rgb.red = rgb_val_red;
    	    rgb.green = rgb_val_green;
    	    rgb.blue = rgb_val_blue;

    	    if (copy_to_user((struct rgb_value *)arg, &rgb, sizeof(struct rgb_value))) {
                return -EFAULT;
    	    }
    	}
	    break;
	
	case WR_LCD_TEXT:
	{    
	    size_t text_len;
	    text_len = strnlen_user((char *)arg, sizeof(lcd_text));
            if (text_len == 0 || text_len > sizeof(lcd_text)) {
                return -EINVAL;
            }
            if (copy_from_user(lcd_text, (char *)arg, text_len)) {
                return -EFAULT;
            }
            lcd_text[text_len - 1] = '\0'; // Ensure null termination
            
	    initialize_lcd(lcd_client);
            lcd_write_string(lcd_client, lcd_text);
	}   
	    break;

        case RD_LCD_TEXT:
	{    
	    if (copy_to_user((char *)arg, lcd_text, strlen(lcd_text) + 1)) {
                return -EFAULT;
            }
	}
            break;

        default:
            return -ENOTTY;
    }
	return 0;
}

/********************************-> IOCTL end <-****************************************/



/*
** Module Init function
*/
static int __init etx_driver_init(void)
{
    int ret = -1;

    /********************************-> IOCLT init start <-****************************************/
    
    /* Allocating Major number */
	if((alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME)) <0) {
		printk(KERN_INFO "Cannot allocate major number\n");
		return -1;
	}
	printk(KERN_INFO "Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));

	/* Creating cdev structure */
	cdev_init(&etx_cdev, &fops);

	/* Adding character device to the system */
	if((cdev_add(&etx_cdev, dev, 1)) < 0) {
		printk(KERN_INFO "Cannot add the device to the system\n");
		goto r_cdev;
	}

	/* Creating struct class */
	if((dev_class = class_create(THIS_MODULE, CLASS_NAME)) == NULL) {
		printk(KERN_INFO "Cannot create the struct class\n");
		goto r_class;
	}

	/* Creating device */
	if((device_create(dev_class, NULL, dev, NULL, DEVICE_NAME)) == NULL) {
		printk(KERN_INFO "Cannot create the Device 1\n");
		goto r_device;
	}

    /********************************-> IOCTL init over <-****************************************/



    /********************************-> sysfs init start <-****************************************/

    kobj_ref = kobject_create_and_add("rgb_lcd_sysfs", kernel_kobj);
    if (!kobj_ref) {
        printk(KERN_INFO "Cannot create sysfs directory\n");
        return -ENOMEM;
    }

    if (sysfs_create_file(kobj_ref, &lcd_attr.attr) ||
        sysfs_create_file(kobj_ref, &rgb_attr.attr)) {
        printk(KERN_INFO "Cannot create sysfs files\n");
        kobject_put(kobj_ref);
        return -ENOMEM;
    }

    /********************************-> sysfs init over <-****************************************/


    /********************************-> rgb lcd init start <-****************************************/
    
    adapter = i2c_get_adapter(I2C_BUS_AVAILABLE);
    
    if (adapter != NULL) {
        lcd_client = i2c_new_device(adapter, &lcd_board_info);
        rgb_client = i2c_new_device(adapter, &rgb_board_info);
        
        if (lcd_client != NULL && rgb_client != NULL) {
            i2c_add_driver(&etx_i2c_driver);
            ret = 0;
        }
        
        i2c_put_adapter(adapter);
    }

    /********************************-> rgb lcd init over <-****************************************/
    

    /********************************-> procfs init start <-****************************************/
    
    rgb_lcd_proc_file = proc_create("rgb_lcd", 0444, NULL, &rgb_lcd_proc_fops);
    if (!rgb_lcd_proc_file) {
        printk(KERN_INFO "Cannot create proc file for RGB\n");
        sysfs_remove_file(kobj_ref, &lcd_attr.attr);
        sysfs_remove_file(kobj_ref, &rgb_attr.attr);
        kobject_put(kobj_ref);
        return -ENOMEM;
    }

    /********************************-> procfs init over <-****************************************/

    ret = 0;

    pr_info("Driver Added!!!\n");
    return ret;

r_device:
	class_destroy(dev_class);
r_class:
	cdev_del(&etx_cdev);
r_cdev:
	unregister_chrdev_region(dev,1);
	return ret;
}

/*
** Module Exit function
*/
static void __exit etx_driver_exit(void)
{
    // Turn off the RGB backlight
    if (rgb_client) {
        i2c_write_data(rgb_client, 0x00, 0x00);  // Reset
        i2c_write_data(rgb_client, 0x01, 0x00);  // Disable clock
        i2c_write_data(rgb_client, 0x08, 0x00);  // Turn off LEDs
    }
    
    // Highlighted: Clear the LCD display before removing the driver
    if (lcd_client) {
        i2c_write_data(lcd_client, COMMAND_MODE, 0x01);  // Clear display command
        msleep(50);
    }

    proc_remove(rgb_lcd_proc_file);

    sysfs_remove_file(kobj_ref, &rgb_attr.attr);
    sysfs_remove_file(kobj_ref, &lcd_attr.attr);
    
    kobject_put(kobj_ref);

    device_destroy(dev_class,dev);
    class_destroy(dev_class);
    cdev_del(&etx_cdev);
    unregister_chrdev_region(dev, 1);

    i2c_unregister_device(lcd_client);
    i2c_unregister_device(rgb_client);
    i2c_del_driver(&etx_i2c_driver);
    pr_info("Driver Removed!!!\n");
}

module_init(etx_driver_init);
module_exit(etx_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Divyesh Kapadiya <divyeshkapadiya25@gmail.com>");
MODULE_DESCRIPTION("I2C driver for Grove RGB LCD V4.0 Display Interface)");
MODULE_VERSION("1.0");

