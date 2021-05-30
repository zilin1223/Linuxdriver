#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/string.h>
#include <linux/of_irq.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>

#define MISCLED_NAME "miscled"
#define MISCLED_CNT   1
#define MISCLED_MINOR 255

struct miscled_dev {
	int gpio;
	struct device_node *nd;
};
struct miscled_dev led;

static int miscled_open(struct inode *inode ,struct file *filp)
{
	filp->private_data = &led;
	return 0;
}
static ssize_t miscled_write(struct file *filp,char __user *buf,size_t cnt, off_t *off_t)
{
	int retvalue = 0;
	unsigned char databuf[1];
	unsigned char ledstate;
	retvalue = copy_from_user(&databuf,buf,cnt);
	ledstate = databuf[0];
	  if (ledstate == 1)
	  {
		  gpio_set_value(led.gpio,0);	/*拉低电平 开灯*/
	  }else if (ledstate == 0)
	  {
		  gpio_set_value(led.gpio,1);  /*拉高电平 关灯*/
	  }
	  return 0;
}
static int miscled_release(struct inode *inode ,struct file *filp)
{
	//filp->private_data = &led;
	return 0;
}

/*字符设备操作函数集*/
struct file_operations miscled_fops = {
	.owner = THIS_MODULE,
	.open = miscled_open,
	.write = miscled_write,
	.release = miscled_release,
};

/*杂项设备结构体*/
static struct miscdevice miscdev = {
	.name = MISCLED_NAME,
	.minor = MISCLED_MINOR,
	.fops = &miscled_fops
};


/*probe函数*/
static int miscled_probe(struct platform_device *dev)
{	
	int ret = 0;
	/*获取GPIO*/
	led.nd = dev->dev.of_node;
	if (led.nd<0)
	{
		ret = -EINVAL;
		goto fail_nd;
	}
	led.gpio = of_get_named_gpio(led.nd,"led-gpios",0);
	if (led.gpio < 0)
	{
		ret = -EINVAL;
		goto fail_getgpio;
	}
	printk("led gpio = %d\r\n",led.gpio);
	ret = gpio_request(led.gpio,"label");
	if (ret)
	{
		ret = -EINVAL;
		goto fail_request_gpio;
	}
	gpio_direction_output(led.gpio,1);/*设置gpio为输出 默认关灯*/
	/*初始化杂项设备*/
	ret = misc_register(&miscdev);
	if (ret < 0)
	{
		printk("misc register error\r\n");
		ret = -EINVAL;
		goto fail_misc;

	}
	return 0;
fail_misc:
	gpio_free(led.gpio);
fail_request_gpio:
fail_getgpio:
fail_nd:
	return ret;
}
/*remove函数*/
static int miscled_remove(struct platform_device *dev)
{
	/*注销GPIO*/
	gpio_free(led.gpio);
	/*删除杂项设备*/
	misc_deregister(&miscdev);
	return 0;
}
/*属性匹配结构体*/
struct of_device_id led_match[] = {
	{.compatible = "jssunny,gpioled"},
	{/*Sential*/},
};
/*平台驱动结构体*/
static struct platform_driver  miscled_driver = {
	.driver = {
		.name = "imx6ull-led",
		.of_match_table = led_match,
	},
	.probe = miscled_probe,
	.remove = miscled_remove,

};

/*驱动入口*/
static int __init miscled_init(void)
{
	return platform_driver_register(&miscled_driver);
}
/*驱动出口*/
static void __exit miscled_exit(void)
{
	platform_driver_unregister(&miscled_driver);
}
/*驱动注册函数*/
module_init(miscled_init);
module_exit(miscled_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("zhangzilin");