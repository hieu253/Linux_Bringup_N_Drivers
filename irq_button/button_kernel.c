#include <linux/module.h>          /* Defines functions such as module_init/module_exit */
#include <linux/gpio.h>            /* Defines functions such as gpio_request/gpio_free */
#include <linux/platform_device.h> /* For platform devices */
#include <linux/gpio/consumer.h>   /* For GPIO Descriptor */
#include <linux/of.h>              
#include <linux/interrupt.h>
#include <asm/uaccess.h>  
#include <linux/device.h> 
#include <linux/cdev.h>  
#include <linux/slab.h>   
#include <linux/uaccess.h>
#include <linux/fs.h>     
#include <linux/kthread.h>
#include <linux/atomic.h>

#define DEVICE_PATH "/dev/my_device_button"
#define DRIVER_AUTHOR "hieunt <hieutrung250302@gmail.com>"
#define DRIVER_DESC "Button driver for tictactoe game on BBB"
#define DRIVER_VERS "1.0"

#define LOW 0
#define HIGH 1

#define BUTTON_01 gpio0_30
#define BUTTON_02 gpio0_31
#define BUTTON_03 gpio0_3
#define BUTTON_04 gpio1_18
#define BUTTON_05 gpio1_28
#define BUTTON_06 gpio1_19
#define BUTTON_07 gpio2_3
#define BUTTON_08 gpio2_4
#define BUTTON_09 gpio1_12

struct gpio_desc *gpio0_30;
struct gpio_desc *gpio0_31;
struct gpio_desc *gpio0_3;

struct gpio_desc *gpio1_18;
struct gpio_desc *gpio1_28;
struct gpio_desc *gpio1_19;

struct gpio_desc *gpio2_3;
struct gpio_desc *gpio2_4;
struct gpio_desc *gpio1_12;

static atomic_t flag1 = ATOMIC_INIT(0);
static atomic_t flag2 = ATOMIC_INIT(0);
static atomic_t flag3 = ATOMIC_INIT(0);
static atomic_t flag4 = ATOMIC_INIT(0);
static atomic_t flag5 = ATOMIC_INIT(0);
static atomic_t flag6 = ATOMIC_INIT(0);
static atomic_t flag7 = ATOMIC_INIT(0);
static atomic_t flag8 = ATOMIC_INIT(0);
static atomic_t flag9 = ATOMIC_INIT(0);

int irq1, irq2, irq3, irq4, irq5, irq6, irq7, irq8, irq9;
int check;

static struct task_struct *thread1;

typedef struct
{
    dev_t dev_num;
    struct class *m_class;
    struct cdev m_cdev;
    int size;
} m_foo_dev;

char kernel_buff[50];

m_foo_dev mdev;

static int my_pdrv_remove(struct platform_device *pdev);
static int my_pdrv_probe(struct platform_device *pdev);

static irqreturn_t handler1(int irq, void *dev_id);
static irqreturn_t handler2(int irq, void *dev_id);
static irqreturn_t handler3(int irq, void *dev_id);
static irqreturn_t handler4(int irq, void *dev_id);
static irqreturn_t handler5(int irq, void *dev_id);
static irqreturn_t handler6(int irq, void *dev_id);
static irqreturn_t handler7(int irq, void *dev_id);
static irqreturn_t handler8(int irq, void *dev_id);
static irqreturn_t handler9(int irq, void *dev_id);

static int m_open(struct inode *inode, struct file *file);
static int m_release(struct inode *inode, struct file *file);
static ssize_t m_read(struct file *filp, char __user *user_buf, size_t size, loff_t *offset);
static ssize_t m_write(struct file *filp, const char __user *user_buf, size_t size, loff_t *offset);
static int thr_func(void *para);
static int write_to_device(const char *message);

static struct file_operations fops = {
        .owner = THIS_MODULE,
        .read = m_read,
        .write = m_write,
        .open = m_open,
        .release = m_release,
};

static const struct of_device_id gpiod_dt_ids[] = {
    {
        .compatible = "gpio_based",
    },
    {}
};

static struct platform_driver mypdrv = {
    .probe = my_pdrv_probe,
    .remove = my_pdrv_remove,
    .driver = {
        .name = "descriptor-based",
        .of_match_table = of_match_ptr(gpiod_dt_ids),
        .owner = THIS_MODULE,
    },
};

static int my_pdrv_probe(struct platform_device *pdev)
{
    pr_info("Probe start\n");
    struct device *dev = &pdev->dev;

    BUTTON_01 = gpiod_get(dev, "button1", GPIOD_IN);
    BUTTON_02 = gpiod_get(dev, "button2", GPIOD_IN);
    BUTTON_03 = gpiod_get(dev, "button3", GPIOD_IN);
    BUTTON_04 = gpiod_get(dev, "button4", GPIOD_IN);
    BUTTON_05 = gpiod_get(dev, "button5", GPIOD_IN);
    BUTTON_06 = gpiod_get(dev, "button6", GPIOD_IN);
    BUTTON_07 = gpiod_get(dev, "button7", GPIOD_IN);
    BUTTON_08 = gpiod_get(dev, "button8", GPIOD_IN);
    BUTTON_09 = gpiod_get(dev, "button9", GPIOD_IN);

    gpiod_set_debounce(BUTTON_01, 200);
    gpiod_set_debounce(BUTTON_02, 200);
    gpiod_set_debounce(BUTTON_03, 200);
    gpiod_set_debounce(BUTTON_04, 200);
    gpiod_set_debounce(BUTTON_05, 200);
    gpiod_set_debounce(BUTTON_06, 200);
    gpiod_set_debounce(BUTTON_07, 200);
    gpiod_set_debounce(BUTTON_08, 200);
    gpiod_set_debounce(BUTTON_09, 200);

    irq1 = gpiod_to_irq(BUTTON_01);
    irq2 = gpiod_to_irq(BUTTON_02);
    irq3 = gpiod_to_irq(BUTTON_03);
    irq4 = gpiod_to_irq(BUTTON_04);
    irq5 = gpiod_to_irq(BUTTON_05);
    irq6 = gpiod_to_irq(BUTTON_06);
    irq7 = gpiod_to_irq(BUTTON_07);
    irq8 = gpiod_to_irq(BUTTON_08);
    irq9 = gpiod_to_irq(BUTTON_09);

    check = request_irq(irq1, handler1, IRQF_TRIGGER_FALLING, "button_irq1", NULL);
    if (check)
    {
        pr_err("Failed to request IRQ for Button GPIO\n");
        gpiod_put(BUTTON_01);
        return check;
    }
    else
    {
        pr_info("Button1 -  OK\n");
    }

    check = request_irq(irq2, handler2, IRQF_TRIGGER_FALLING, "button_irq2", NULL);
    if (check)
    {
        pr_err("Failed to request IRQ for Button GPIO\n");
        gpiod_put(BUTTON_02);
        return check;
    }
    else
    {
        pr_info("Button2 -  OK\n");
    }

    check = request_irq(irq3, handler3, IRQF_TRIGGER_FALLING, "button_irq3", NULL);
    if (check)
    {
        pr_err("Failed to request IRQ for Button GPIO\n");
        gpiod_put(BUTTON_03);
        return check;
    }
    else
    {
        pr_info("Button3 -  OK\n");
    }

    check = request_irq(irq4, handler4, IRQF_TRIGGER_FALLING, "button_irq4", NULL);
    if (check)
    {
        pr_err("Failed to request IRQ for Button GPIO\n");
        gpiod_put(BUTTON_04);
        return check;
    }
    else
    {
        pr_info("Button4 -  OK\n");
    }

    check = request_irq(irq5, handler5, IRQF_TRIGGER_FALLING, "button_irq5", NULL);
    if (check)
    {
        pr_err("Failed to request IRQ for Button GPIO\n");
        gpiod_put(BUTTON_05);
        return check;
    }
    else
    {
        pr_info("Button5 -  OK\n");
    }

    check = request_irq(irq6, handler6, IRQF_TRIGGER_FALLING, "button_irq6", NULL);
    if (check)
    {
        pr_err("Failed to request IRQ for Button GPIO\n");
        gpiod_put(BUTTON_06);
        return check;
    }
    else
    {
        pr_info("Button6 - OK\n");
    }

    check = request_irq(irq7, handler7, IRQF_TRIGGER_FALLING, "button_irq7", NULL);
    if (check)
    {
        pr_err("Failed to request IRQ for Button GPIO\n");
        gpiod_put(BUTTON_07);
        return check;
    }
    else
    {
        pr_info("Button7 - OK\n");
    }

    check = request_irq(irq8, handler8, IRQF_TRIGGER_FALLING, "button_irq8", NULL);
    if (check)
    {
        pr_err("Failed to request IRQ for Button GPIO\n");
        gpiod_put(BUTTON_08);
        return check;
    }
    else
    {
        pr_info("Button8 - OK\n");
    }

    check = request_irq(irq9, handler9, IRQF_TRIGGER_FALLING, "button_irq9", NULL);
    if (check)
    {
        pr_err("Failed to request IRQ for Button GPIO\n");
        gpiod_put(BUTTON_09);
        return check;
    }
    else
    {
        pr_info("Button9 - OK\n");
    }
    pr_info("Init GPIO IRQ - OK\n");

    pr_info("Probe Start - success\n");
    return 0;
}

static int my_pdrv_remove(struct platform_device *pdev)
{
    pr_info("Remove start\n");

    free_irq(irq1, NULL);
    gpiod_put(BUTTON_01);

    free_irq(irq2, NULL);
    gpiod_put(BUTTON_02);

    free_irq(irq3, NULL);
    gpiod_put(BUTTON_03);

    free_irq(irq4, NULL);
    gpiod_put(BUTTON_04);

    free_irq(irq5, NULL);
    gpiod_put(BUTTON_05);

    free_irq(irq6, NULL);
    gpiod_put(BUTTON_06);

    free_irq(irq7, NULL);
    gpiod_put(BUTTON_07);

    free_irq(irq8, NULL);
    gpiod_put(BUTTON_08);

    free_irq(irq9, NULL);
    gpiod_put(BUTTON_09);

    pr_info("Remove successful\n");

    return 0;
}

static irqreturn_t handler1(int irq, void *dev_id)
{
    bool pressed = gpiod_get_value(BUTTON_01);
    if (pressed)
    {
        atomic_set(&flag1, 1);
        pr_info("Button1 Pressed\n");
    }

    else
        pr_info("Button1 Released\n");
    return IRQ_HANDLED;
}

static irqreturn_t handler2(int irq, void *dev_id)
{
    bool pressed = gpiod_get_value(BUTTON_02);

    if (pressed)
    {
        atomic_set(&flag2, 1);
        pr_info("Button2 Pressed\n");
    }
    else
        pr_info("Button2 Released\n");

    return IRQ_HANDLED;
}

static irqreturn_t handler3(int irq, void *dev_id)
{
    bool pressed = gpiod_get_value(BUTTON_03);

    if (pressed)
    {
        atomic_set(&flag3, 1);
        pr_info("Button3 Pressed\n");
    }
    else
        pr_info("Button3 Released\n");

    return IRQ_HANDLED;
}

static irqreturn_t handler4(int irq, void *dev_id)
{
    bool pressed = gpiod_get_value(BUTTON_04);

    if (pressed)
    {
        atomic_set(&flag4, 1);
        pr_info("Button4 Pressed\n");
    }
    else
        pr_info("Button4 Released\n");

    return IRQ_HANDLED;
}

static irqreturn_t handler5(int irq, void *dev_id)
{
    bool pressed = gpiod_get_value(BUTTON_05);

    if (pressed)
    {
        atomic_set(&flag5, 1);
        pr_info("Button5 Pressed\n");
    }
    else
        pr_info("Button5 Released\n");

    return IRQ_HANDLED;
}

static irqreturn_t handler6(int irq, void *dev_id)
{
    bool pressed = gpiod_get_value(BUTTON_06);

    if (pressed)
    {
        atomic_set(&flag6, 1);
        pr_info("Button6 Pressed\n");
    }
    else
        pr_info("Button6 Released\n");

    return IRQ_HANDLED;
}

static irqreturn_t handler7(int irq, void *dev_id)
{
    bool pressed = gpiod_get_value(BUTTON_07);
    if (pressed)
    {
        atomic_set(&flag7, 1);
        pr_info("Button7 Pressed\n");
    }
    else
        pr_info("Button7 Released\n");

    return IRQ_HANDLED;
}

static irqreturn_t handler8(int irq, void *dev_id)
{
    bool pressed = gpiod_get_value(BUTTON_08);

    if (pressed)
    {
        atomic_set(&flag8, 1);
        pr_info("Button8 Pressed\n");
    }
    else
        pr_info("Button8 Released\n");

    return IRQ_HANDLED;
}

static irqreturn_t handler9(int irq, void *dev_id)
{
    bool pressed = gpiod_get_value(BUTTON_09);

    if (pressed)
    {
        atomic_set(&flag9, 1);
        pr_info("Button9 Pressed\n");
    }
    else
        pr_info("Button9 Released\n");

    return IRQ_HANDLED;
}

/* This function will be called when we open the Device file */
static int m_open(struct inode *inode, struct file *file)
{
    return 0;
}

/* This function will be called when we close the Device file */
static int m_release(struct inode *inode, struct file *file)
{
    return 0;
}

/* This function will be called when we read the Device file */
static ssize_t m_read(struct file *filp, char __user *user_buf, size_t size, loff_t *offset)
{

    // Check if the offset is beyond the end of the buffer
    if (*offset >= mdev.size)
    {
        return 0; // End of file
    }

    // Determine how many bytes to read (up to the end of the buffer)
    size_t bytes_to_read = min(size, (size_t)(mdev.size - *offset));

    // Copy data from kernel space to user space
    if (copy_to_user(user_buf, kernel_buff + *offset, bytes_to_read) != 0)
    {
        pr_err("Can not copy kernel to user\n");
        return -EFAULT;
    }

    // Update the offset and return the number of bytes read
    *offset += bytes_to_read;

    // kfree(mdev.k_buff);
    return bytes_to_read;
}

/* This function will be called when we write the Device file */
static ssize_t m_write(struct file *filp, const char __user *user_buf, size_t size, loff_t *offset)
{

    if (copy_from_user(kernel_buff, user_buf, size))
    {
        pr_err("Can not copy user to kernel\n");
        return -EFAULT;
    }

    pr_info("Data from user space: %s\n", kernel_buff);

    mdev.size = size; 

    return size;
}

static int __init chdev_init(void)
{
    if (alloc_chrdev_region(&mdev.dev_num, 0, 1, "my-cdev-button"))
    {
        pr_err("ERROR: Can not make number device\n");
        return -1;
    }
    pr_info("Major: %d Minor: %d\n", MAJOR(mdev.dev_num), MINOR(mdev.dev_num));

    if ((mdev.m_class = class_create(THIS_MODULE, "my_class_button")) == NULL)
    {
        pr_err("ERROR: Can not create class\n");
        goto rm_dev_num;
    }
    pr_info("Init class create\n");

    if (device_create(mdev.m_class, NULL, mdev.dev_num, NULL, "my_device_button") == NULL)
    {
        pr_err("ERROR: Can not create device\n");
        goto rm_class;
    }
    pr_info("Init device create\n");

    cdev_init(&mdev.m_cdev, &fops);
    pr_info("Init Chacracter device\n");
    if (cdev_add(&mdev.m_cdev, mdev.dev_num, 1) < 0)
    {
        pr_err("ERROR: Can not add device\n");
        goto rm_dev_num;
    }
    pr_info("Init Character devivce add\n");

    int ret;
    ret = platform_driver_register(&mypdrv);
    if (ret)
    {
        pr_err("Failed to register platform driver: %d\n", ret);
        return ret;
    }

    pr_info("Platform driver registered successfully\n");

    thread1 = kthread_run(thr_func, NULL, "p_thread");

    pr_info("Init end\n");

    return 0;

rm_dev_num:
    unregister_chrdev_region(mdev.dev_num, 1);
rm_class:
    class_destroy(mdev.m_class);

    return -1;
}

static void __exit chdev_exit(void)
{
    pr_info("Start character device exit\n");

    cdev_del(&mdev.m_cdev);
    pr_info("Exit character device");

    device_destroy(mdev.m_class, mdev.dev_num);
    pr_info("Exit device_destroy\n");

    class_destroy(mdev.m_class);
    pr_info("Check exit class_destroy\n");

    unregister_chrdev_region(mdev.dev_num, 1);
    pr_info("Check exit number\n");
    platform_driver_unregister(&mypdrv);
    kthread_stop(thread1);
    pr_info("Exit end\n");
}

static int write_to_device(const char *message)
{
    struct file *filp;
    int ret = 0;

    filp = filp_open(DEVICE_PATH, O_WRONLY, 0);
    if (IS_ERR(filp))
    {
        pr_err("Failed to open device file\n");
        return PTR_ERR(filp);
    }

    ssize_t len = strlen(message);
    ssize_t bytes_written = kernel_write(filp, message, len, &filp->f_pos);
    if (bytes_written < 0)
    {
        pr_err("Failed to write to device file\n");
        ret = bytes_written;
    }

    filp_close(filp, NULL);

    return ret;
}

static int thr_func(void *para)
{

    while (!kthread_should_stop())
    {
        if (atomic_read(&flag1) == 1)
        {
            int ret = write_to_device("1");
            if (ret < 0)
                return ret;  
            atomic_set(&flag1, 0); 
            pr_info("Flag1 - set\n");
        }
        if (atomic_read(&flag2) == 1)
        {
            int ret = write_to_device("2");
            if (ret < 0)
                return ret;  
            atomic_set(&flag2, 0);
            pr_info("Flag2 - set\n");
        }
        if (atomic_read(&flag3) == 1)
        {
            int ret = write_to_device("3");
            if (ret < 0)
                return ret;  
            atomic_set(&flag3, 0); 
        }
        if (atomic_read(&flag4) == 1)
        {
            int ret = write_to_device("4");
            if (ret < 0)
                return ret;  
            atomic_set(&flag4, 0); 
        }
        if (atomic_read(&flag5) == 1)
        {
            int ret = write_to_device("5");
            if (ret < 0)
                return ret;  

            atomic_set(&flag5, 0); 
        }
        if (atomic_read(&flag6) == 1)
        {
        
            int ret = write_to_device("6");
            if (ret < 0)
                return ret; 
            atomic_set(&flag6, 0);
        }
        if (atomic_read(&flag7) == 1)
        {
            int ret = write_to_device("7");
            if (ret < 0)
                return ret; 
            atomic_set(&flag7, 0);
        }
        if (atomic_read(&flag8) == 1)
        {
            int ret = write_to_device("8");
            if (ret < 0)
                return ret; 
            atomic_set(&flag8, 0);
        }
        if (atomic_read(&flag9) == 1)
        {
            int ret = write_to_device("9");
            if (ret < 0)
                return ret; 
            atomic_set(&flag9, 0);
        }
        schedule_timeout_interruptible(msecs_to_jiffies(100));
    }
    return 0;
}

module_init(chdev_init);
module_exit(chdev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERS);