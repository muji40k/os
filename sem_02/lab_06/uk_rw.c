#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/vmalloc.h>

#define BUFSIZE   10
#define BUFSTRING 200

MODULE_LICENSE("GPL");

struct buffer
{
    size_t len;
    char string[BUFSTRING + 1];
};

struct buffer *buffer = NULL;
struct buffer *buffer_end = NULL;

struct proc_dir_entry *proc_file = NULL;
struct proc_dir_entry *proc_dir = NULL;
struct proc_dir_entry *proc_source_symlink = NULL;

struct buffer *read_buf = NULL;
struct buffer *write_buf = NULL;
size_t filled = 0;

int proc_open(struct inode *inode, struct file *file);
int proc_release(struct inode *inode, struct file *file);
ssize_t proc_write(struct file *file, const char __user *ubuffer, size_t len,
                   loff_t *off);
ssize_t proc_read(struct file *file, char __user *ubuffer, size_t count,
                  loff_t *off);

struct proc_ops pops =
{
    .proc_open = proc_open,
    .proc_release = proc_release,
    .proc_write = proc_write,
    .proc_read = proc_read
};

int proc_open(struct inode *inode, struct file *file)
{
    printk("+ UK Open func");

    return 0;
}

int proc_release(struct inode *inode, struct file *file)
{
    printk("+ UK Release func");

    return 0;
}

ssize_t proc_write(struct file *file, const char __user *ubuffer, size_t len,
                   loff_t *off)
{
    printk("+ UK Write func");

    if (BUFSIZE == filled)
    {
        printk("+ UK Buffer is full");

        return len;
    }

    if (len > BUFSTRING)
        len = BUFSTRING;

    if (copy_from_user(write_buf->string, ubuffer, len))
    {
        printk("+ UK copy_from_user error");

        return -EFAULT;
    }

    write_buf->string[len] = '\0';
    write_buf->len = len;

    printk("+ UK Write completed: \"%s\" -- length: %zu",
           write_buf->string, write_buf->len);

    if (buffer_end == ++write_buf)
        write_buf = buffer;

    filled++;

    return len;
}

ssize_t proc_read(struct file *file, char __user *ubuffer, size_t count,
                  loff_t *off)
{
    printk("+ UK Read func");

    if (0 == filled)
    {
        printk("+ UK Buffer is empty");

        return 0;
    }

    ssize_t len = read_buf->len;

    if (copy_to_user(ubuffer, read_buf->string, len))
    {
        printk("+ UK copy_to_user error");

        return -EFAULT;
    }

    printk("+ UK Read completed: \"%s\" -- length: %zu",
           read_buf->string, read_buf->len);

    if (buffer_end == ++read_buf)
        read_buf = buffer;

    filled--;

    return len;
}

int __init init_md(void)
{
    printk("+ Start user kernel module");
    int rc = 0;
    buffer = vmalloc(BUFSIZE * sizeof(struct buffer));

    if (!buffer)
        rc = -ENOMEM;

    if (0 == rc)
    {
        proc_dir = proc_mkdir("user_kernel", NULL);

        if (!proc_dir)
            rc = -ENOMEM;
    }

    if (0 == rc)
    {
        proc_file = proc_create("latch", S_IWUGO | S_IRUGO,
                                proc_dir, &pops);
        if (!proc_file)
            rc = -ENOMEM;
    }
    else
        remove_proc_entry("user_kernel", NULL);

    if (0 == rc)
    {
        proc_source_symlink = proc_symlink("source", proc_dir, __FILE__);

        if (!proc_source_symlink)
            rc = -ENOMEM;
    }
    else
    {
        remove_proc_entry("latch", proc_dir);
        remove_proc_entry("user_kernel", NULL);
    }

    if (0 != rc)
        vfree(buffer);

    buffer_end = buffer + BUFSIZE;
    read_buf = buffer;
    write_buf = buffer;
    filled = 0;

    printk("+ Start success");

    return rc;
}

void __exit exit_md(void)
{
    remove_proc_entry("latch", proc_dir);
    remove_proc_entry("source", proc_dir);
    remove_proc_entry("user_kernel", NULL);
    vfree(buffer);
    printk("+ Exit success");
}

module_init(init_md);
module_exit(exit_md);

