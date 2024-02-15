#include <linux/module.h> /* printk y otras definiciones */
#include <linux/kernel.h> /* constantes KERN_XX */
#include <linux/cdev.h>
#include <linux/fs.h>
#include <asm/unistd.h>
#include <asm/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/moduleparam.h>
#include <linux/printk.h>
#include <linux/stat.h>

#define DRIVER_NAME "clipboardDriver"
#define DRIVER_CLASS "clipboardDriverClass"
#define NUM_DEVICES 3 /* Número de dispositivos a crear */

unsigned int minor_num;

static int defaultSize = 255;
static int size0 = 255;
static int size1 = 255;
static int size2 = 255;

module_param(size0, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
module_param(size1, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
module_param(size2, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

char pristineBuffer[255];
char buffer0[255];
char buffer1[255];
char buffer2[255];

static int do_open(struct inode *inode, struct file *file) {
	minor_num = MINOR(inode->i_rdev);
	pr_info("Clipboard Driver opening");
	return 0;
}

static ssize_t do_read(struct file *file, char __user *buf, size_t count, loff_t *f_pos) {
	loff_t real_read;
	int k;
	char* buffer;
	int sizeofbuffer;
	if (minor_num == 0) {
		buffer = buffer0;
		sizeofbuffer = size0;
		}
	else if (minor_num==1) {
		buffer = buffer1;
		sizeofbuffer = size1;
		}
	else if (minor_num == 2) {
		buffer = buffer2;
		sizeofbuffer = size2;
		}
	pr_info("Clipoard Driver reading");
	
  /* El número de bytes a leer ha de ser >= 0 */
	if (count < 0)
	    return -EINVAL;


  /* Calcula el número real de caracteres a leer para evitar sobrepasar el tamaño del buffer */
	if ((*f_pos + count) < sizeofbuffer)
	    real_read = count;
	else
	    real_read = sizeofbuffer - *f_pos;

  /* Se copia buffer al espacio de usuario */
	k = copy_to_user (buf, buffer + *f_pos, real_read);

  /* Actualización del puntero */
	*f_pos += real_read;

  /* Retornamos número de caracteres leídos */
	return real_read;
}
static ssize_t do_write(struct file *file, const char __user *buf, size_t count, loff_t *f_pos) {
	long result;
	char* buffer;
	int sizeofbuffer;
	int i;
	if (minor_num == 0) {
		buffer = buffer0;
		sizeofbuffer = size0;
		}
	else if (minor_num==1) {
		buffer = buffer1;
		sizeofbuffer = size1;
		}
	else if (minor_num == 2) {
		buffer = buffer2;
		sizeofbuffer = size2;
		}
	pr_info("Clipboard Driver writing");
  /* comprobamos el número de bytes a escribir para evitar desbordamientos del buffer    */
	if (*f_pos + count > sizeofbuffer)
	    return -EINVAL;

	if (minor_num == 1)
		{
		for (i = 0; i<defaultSize; i++)
			{ 
			buffer2[i] = buffer1[i];
			buffer1[i] = pristineBuffer[i];
			}
		}
	if (minor_num == 0)
		{
		for (i = 0; i<defaultSize; i++) 
			{
			buffer2[i] = buffer1[i];
			buffer1[i] = buffer0[i];
			buffer0[i] = pristineBuffer[i];
			}
		}

  /* se copian los bytes desde el espacio de usuario al buffer  */
	result = copy_from_user(buffer + *f_pos, buf, count);

  return count;
}

static int do_release(struct inode *inode, struct file *file) {
	pr_info("Clipboard Driver released");
	return 0;
}


static const struct file_operations ECC_fops = {
	.owner = THIS_MODULE,
	.open = do_open,
	.read = do_read,
	.write = do_write,
	.release = do_release
};

static dev_t major_minor = -1;
static struct cdev ECCcdev[NUM_DEVICES];
static struct class *ECCclass = NULL;

static int __init init_driver(void) {
	int n_device;
	dev_t id_device;
	if (size0 > 255 || size0 < 0 || size1 > 255 || size1 < 0 || size2 > 255 || size2 < 0) {
		pr_err("Buffer size cannot be negative or exceed 255");
		goto error;
	}
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
	pr_info("Clipboard driver initialized and loaded\n");
	pr_info("Tamaño del primer buffer: %d\n", size0);
	pr_info("Tamaño del segundo buffer: %d\n", size1);
	pr_info("Tamaño del tercer buffer: %d\n", size2);
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
	pr_info("Clipboard Driver unloaded\n");
}

MODULE_LICENSE("GPL"); /* Obligatorio */
MODULE_DESCRIPTION("Clipboard Driver");
MODULE_AUTHOR("José Manuel Martínez Ramírez");


module_init(init_driver)
module_exit(exit_driver)
