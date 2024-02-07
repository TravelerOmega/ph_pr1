#include<linux/module.h> /* printk y otras definiciones */
#include<linux/kernel.h> /* constantes KERN_XX */
#include <linux/cdev.h>
#include <linux/fs.h>

#define DRIVER_NAME "ECCDriver"
#define DRIVER_CLASS "ECCDriverClass"
#define NUM_DEVICES 3 /* Número de dispositivos a crear */

static int ECCopen(struct inode *inode, struct file *file) {
	pr_info("ECCopen");
	return 0;
}

static ssize_t ECCread(struct file *file, char __user *buffer, size_t count, loff_t *f_pos) {
	pr_info("ECCread");
	return 0;
}
static ssize_t ECCwrite(struct file *file, const char __user *buffer, size_t count, loff_t *f_pos) {
	pr_info("ECCwrite");
	return -EINVAL;
}
static int ECCrelease(struct inode *inode, struct file *file) {
	pr_info("ECCrelease");
	return 0;
}

static const struct file_operations ECC_fops = {
	.owner = THIS_MODULE,
	.open = ECCopen,
	.read = ECCread,
	.write = ECCwrite,
	.release = ECCrelease,
};

static dev_t major_minor = -1;
static struct cdev ECCcdev[NUM_DEVICES];
static struct class *ECCclass = NULL;

static int __init init_driver(void) {
	int n_device;
	dev_t id_device;
	if (alloc_chrdev_region(&major_minor, 0, NUM_DEVICES, DRIVER_NAME) < 0) {
		pr_err("Major number assignment failed");
		goto error;
	}
	pr_info("%s driver assigned %d major number\n", DRIVER_NAME, MAJOR(major_minor));
	if((ECCclass = class_create(THIS_MODULE, DRIVER_CLASS)) == NULL) {
		pr_err("Class device registering failed");
		goto error;
	}
	pr_info("/sys/class/%s class driver registered\n", DRIVER_CLASS);
	for (n_device = 0; n_device < NUM_DEVICES; n_device++) {
		cdev_init(&ECCcdev[n_device], &ECC_fops);
		id_device = MKDEV(MAJOR(major_minor), MINOR(major_minor) + n_device);
		if(cdev_add(&ECCcdev[n_device], id_device, 1) == -1) {
			pr_err("Device node creation failed");
			goto error;
		}
		if(device_create(ECCclass, NULL, id_device, NULL, DRIVER_NAME "%d", n_device) == NULL) {
			pr_err("Device node creation failed");
			goto error;
		}
		pr_info("Device node /dev/%s%d created\n", DRIVER_NAME, n_device);
	}
	pr_info("ECC driver initialized and loaded\n");
	return 0;


error:
	if(ECCclass)
		class_destroy(ECCclass);
	if(major_minor != -1)
		unregister_chrdev_region(major_minor, NUM_DEVICES);
return -1;
}

static void __exit exit_driver(void) {
	int n_device;
	for (n_device = 0; n_device < NUM_DEVICES; n_device++) {
		device_destroy(ECCclass, MKDEV(MAJOR(major_minor), MINOR(major_minor) + n_device));
		cdev_del(&ECCcdev[n_device]);
}
	class_destroy(ECCclass);
	unregister_chrdev_region(major_minor, NUM_DEVICES);
	pr_info("ECC driver unloaded\n");
}

MODULE_LICENSE("GPL"); /* Obligatorio */

module_init(init_driver)
module_exit(exit_driver)
