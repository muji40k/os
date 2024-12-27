#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init_task.h>
#include <linux/sched.h>

MODULE_LICENSE("GPL");

int __init init_md(void)
{
    struct task_struct *task = &init_task;

    do
        printk("+ self - %s, pid - %d, parent - %s, ppid - %d, "
               "state - %d, flags - %d, prio - %d, policy - %d\n",
               task->comm, task->pid, task->parent->comm, task->parent->pid,
               task->__state, task->flags, task->prio, task->policy);
    while ((task = next_task(task)) != &init_task);

    return -1;
}

void __exit exit_md(void) {}

module_init(init_md);
module_exit(exit_md);

