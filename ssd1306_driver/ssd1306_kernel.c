#include "ssd1306_lib.h"

#define DEVICE_PATH "/dev/my_device_ssd"
#define DRIVER_AUTHOR "hieunt <hieutrung250302@gmail.com>"
#define DRIVER_DESC "ssd1306 driver for tictactoe game on BBB"

static int ssd1306_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int ssd1306_remove(struct i2c_client *client);

static int __init chdev_init(void);
static void __exit chdev_exit(void);

static int m_open(struct inode *inode, struct file *file);
static int m_release(struct inode *inode, struct file *file);
static ssize_t m_read(struct file *filp, char __user *user_buf, size_t size, loff_t *offset);
static ssize_t m_write(struct file *filp, const char *user_buf, size_t size, loff_t *offset);

struct ssd1306_i2c_module *module_ssd;

uint8_t row = 0;

typedef struct
{
    dev_t dev_num;
    struct class *m_class;
    struct cdev m_cdev;
    int size;
    // char *k_buff;
} m_foo_dev;

char kernel_buff[256];

m_foo_dev mdev;

static const struct of_device_id ssd1306_of_match[] = {
    {.compatible = "ssd1306_driver"},
    {}};

MODULE_DEVICE_TABLE(of, ssd1306_of_match);

static struct i2c_driver ssd1306_driver = {
    .probe = ssd1306_probe,
    .remove = ssd1306_remove,
    .driver = {
        .name = "ssd1306",
        .of_match_table = ssd1306_of_match,
    },
};

static struct file_operations fops =
    {
        .owner = THIS_MODULE,
        .read = m_read,
        .write = m_write,
        .open = m_open,
        .release = m_release,
};

static int ssd1306_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    // struct ssd1306_i2c_module *module;

    pr_info("Start probe\n");
    module_ssd = kmalloc(sizeof(*module_ssd), GFP_KERNEL);
    if (!module_ssd)
    {
        pr_err("kmalloc failed\n");
        return -1;
    }

    module_ssd->client = client;
    module_ssd->line_num = 0;
    module_ssd->cursor_position = 0;
    module_ssd->font_size = SSD1306_DEF_FONT_SIZE;
    i2c_set_clientdata(client, module_ssd);

    ssd1306_display_init(module_ssd);
    ssd1306_set_cursor(module_ssd, 0, 0);
    ssd1306_print_string(module_ssd, "\n");
    ssd1306_set_cursor(module_ssd, 1, 0);
    ssd1306_print_string(module_ssd, "  1|2|3  Tic Tac Toe\n");
    ssd1306_set_cursor(module_ssd, 2, 0);
    ssd1306_print_string(module_ssd, " ------- \n");
    ssd1306_set_cursor(module_ssd, 3, 0);
    ssd1306_print_string(module_ssd, "  4|5|6  Made by Hieu\n");
    ssd1306_set_cursor(module_ssd, 4, 0);
    ssd1306_print_string(module_ssd, " ------- \n");
    ssd1306_set_cursor(module_ssd, 5, 0);
    ssd1306_print_string(module_ssd, "  7|8|9   \n");
    ssd1306_set_cursor(module_ssd, 6, 0);
    ssd1306_print_string(module_ssd, " \n");

    pr_info("Probe function done \n");

    return 0;
}

static int ssd1306_remove(struct i2c_client *client)
{
    pr_info("Starting remove process\n");
    module_ssd = i2c_get_clientdata(client);

    ssd1306_clear_full(module_ssd);
    ssd1306_set_cursor(module_ssd, 3, 13);
    ssd1306_print_string(module_ssd, "GOODBYE. SEE YOU !!\n");
    msleep(3000);
    ssd1306_clear_full(module_ssd);
    ssd1306_write(module_ssd, true, 0xAE); 
    kfree(module_ssd);

    pr_info("Remove process done\n");
    return 0;
}

/* This function will be called when we open the Device file */
static int m_open(struct inode *inode, struct file *file)
{
    pr_info("System call open() called...!!!\n");
    return 0;
}

/* This function will be called when we close the Device file */
static int m_release(struct inode *inode, struct file *file)
{
    pr_info("System call close() called...!!!\n");
    return 0;
}

/* This function will be called when we read the Device file */
static ssize_t m_read(struct file *filp, char __user *user_buf, size_t size, loff_t *offset)
{
    pr_info("System call read() called...!!!\n");

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
    pr_info("System call write() called...!!!\n");
    // mdev.k_buff = kmalloc(size, GFP_KERNEL);

    if (copy_from_user(kernel_buff, user_buf, size))
    {
        pr_err("Can not copy user to kernel\n");
        // kfree(mdev.k_buff);
        return -EFAULT;
    }

    pr_info("Data from user space: %s\n", kernel_buff);

    if (strcmp(kernel_buff, "clear") == 0)
    {
        ssd1306_clear_full(module_ssd);
    }

    ssd1306_set_cursor(module_ssd, row, 0);
    ssd1306_print_string(module_ssd, kernel_buff);
    row ++;
    if(row == 6)
    {
      row = 0;   
    }

    mdev.size = size;
    // kfree(mdev.k_buff);

    return size;
}

static int __init chdev_init(void)
{
    if (alloc_chrdev_region(&mdev.dev_num, 0, 1, "my-cdev"))
    {
        pr_err("ERROR: Can not make number device\n");
        return -1;
    }
    pr_info("Khoi tao : bat dau\n");

    pr_info("Major : %d    Minor : %d\n", MAJOR(mdev.dev_num), MINOR(mdev.dev_num));

    if ((mdev.m_class = class_create(THIS_MODULE, "my_class")) == NULL)
    {
        pr_err("ERROR: Can not create class\n");
        goto rm_dev_num;
    }
    pr_info("Init : Khoi tao class create\n");

    if (device_create(mdev.m_class, NULL, mdev.dev_num, NULL, "my_device_ssd") == NULL)
    {
        pr_err("ERROR: Can not create device\n");
        goto rm_class;
    }
    pr_info("Init : Khoi tao device create\n");

    cdev_init(&mdev.m_cdev, &fops);
    pr_info("Init : Khoi tao Cdev Init\n");
    if (cdev_add(&mdev.m_cdev, mdev.dev_num, 1) < 0)
    {
        pr_err("ERROR: Can not add device\n");
        goto rm_dev_num;
    }
    pr_info("Init : Khoi tao Cdev add\n");

    i2c_add_driver(&ssd1306_driver);
    pr_info("init: i2c add driver\n");

    pr_info("Khoi tao ket thuc\n");

    return 0;

rm_dev_num:
    unregister_chrdev_region(mdev.dev_num, 1);
rm_class:
    class_destroy(mdev.m_class);

    return -1;
}

static void __exit chdev_exit(void)
{
    pr_info("Start Huy\n");

    i2c_del_driver(&ssd1306_driver);
    pr_info("Exit: huy i2c_del_driver\n");

    cdev_del(&mdev.m_cdev);
    pr_info("Exit: huy cdev");

    device_destroy(mdev.m_class, mdev.dev_num);
    pr_info("Exit: huy device_destroy\n");

    class_destroy(mdev.m_class);
    pr_info("Exit: check huy class_destroy\n");

    unregister_chrdev_region(mdev.dev_num, 1);
    pr_info("Exit: check huy number\n");

    pr_info("End Huy\n");
}

// module_i2c_driver(ssd1306_driver);
module_init(chdev_init);
module_exit(chdev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);