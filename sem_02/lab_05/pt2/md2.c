#include <linux/module.h>
#include <linux/kernel.h>

#include "md.h"

MODULE_LICENSE("GPL");

static int __init init_md(void)
{
    printk("+ module md2 start\n");
    printk("+ export md2 data: %s\n", md1_data);
    printk("+ export md2 proc: %s\n", md1_proc());

    // printk("+ call md1 local: %s\n", md1_local());
    printk("+ call md1 noexport: %s\n", md1_noexport());

    return 0;
}

static void __exit exit_md(void)
{
    printk("+ module md2 end\n");
}

module_init(init_md);
module_exit(exit_md);

