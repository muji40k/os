#include <linux/module.h>
#include <linux/kernel.h>

#include "md.h"

MODULE_LICENSE("GPL");

static int __init init_md(void)
{
    printk("+ module md3 start\n");
    printk("+ export md3 data: %s\n", md1_data);
    printk("+ export md3 proc: %s\n", md1_proc());
    return 0;
    // return -1;
}

static void __exit exit_md(void)
{
    printk("+ module md3 end\n");
}

module_init(init_md);
module_exit(exit_md);

