#include <linux/module.h>
#include <linux/kernel.h>

#include "md.h"

MODULE_LICENSE("GPL");

char *md1_data = "aaa";

extern char *md1_proc(void)
{
    return md1_data;
}

static char *md1_local(void)
{
    return md1_data;
}

extern char *md1_noexport(void)
{
    return md1_data;
}

EXPORT_SYMBOL(md1_data);
EXPORT_SYMBOL(md1_proc);

static int __init init_md(void)
{
    printk("+ module md1 start\n");
    return 0;
}

static void __exit exit_md(void)
{
    printk("+ module md1 end\n");
}

module_init(init_md);
module_exit(exit_md);

