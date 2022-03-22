#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>

static int kenlex_init(void) {

        printk (KERN_INFO "Kenlex Enabled\n");

        return 0;
}

static void kenlex_exit(void) {
}

module_init(kenlex_init);
module_exit(kenlex_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Kenneth Sherman and Alex Myers");
MODULE_DESCRIPTION("Kenlex File Security Module");