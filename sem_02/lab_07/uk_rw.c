#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/vmalloc.h>
#include <linux/seq_file.h>

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

void *seq_start(struct seq_file *m, loff_t *pos)
{
    printk("+ UK Sequence start");

    if (0 == filled)
    {
        printk("+ UK Sequence buffer empty (start)");

        return NULL;
    }
    else if (filled <= *pos)
    {
        printk("+ UK Sequence pos overflow (start)");

        return NULL;
    }

    read_buf = buffer + *pos;

    return &read_buf;
}

void seq_stop(struct seq_file *m, void *v)
{
    if (!v)
        printk("+ UK Sequence stop: v nullpointer");
    else
        printk("+ UK Sequence stop: v (%s)", (*(struct buffer **)v)->string);
}

void *seq_next(struct seq_file *m, void *v, loff_t *pos)
{
    printk("+ UK Sequence next: v (%s)", (*(struct buffer **)v)->string);

    if (filled <= ++(*pos))
    {
        printk("+ UK Sequence pos overflow (next)");

        return NULL;
    }

    ++(*(struct buffer **)v);

    return v;
}

int seq_show(struct seq_file *m, void *v)
{
    struct buffer *buf = *(struct buffer**)v;
    printk("+ UK Sequence show: \"%s\" -- length: %zu", buf->string, buf->len);
    seq_printf(m, "%s", buf->string);

    return 0;
}

struct seq_operations ct_seq_ops =
{
    .start = seq_start,
    .stop = seq_stop,
    .next = seq_next,
    .show = seq_show
};

int proc_open(struct inode *inode, struct file *file)
{
    printk("+ UK Open func");

    return seq_open(file, &ct_seq_ops);
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
    printk("+ UK Read wrap");

    return seq_read(file, ubuffer, count, off);
}

struct proc_ops pops =
{
    .proc_open = proc_open,
    .proc_write = proc_write,
    .proc_read = proc_read,
    .proc_release = seq_release
};

void clear_res(void)
{
    if (proc_source_symlink)
    {
        remove_proc_entry("source", proc_dir);
        proc_source_symlink = NULL;
    }

    if (proc_file)
    {
        remove_proc_entry("latch", proc_dir);
        proc_file = NULL;
    }

    if (proc_dir)
    {
        remove_proc_entry("user_kernel", NULL);
        proc_dir = NULL;
    }

    if (buffer)
    {
        vfree(buffer);
        buffer = NULL;
    }
}

int __init init_md(void)
{
    printk("+ Start user kernel module");
    int rc = 0;
    buffer = vmalloc(BUFSIZE * sizeof(struct buffer));

    if (!buffer)
        rc = -ENOMEM;
    else
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

    if (0 == rc)
    {
        proc_source_symlink = proc_symlink("source", proc_dir, __FILE__);

        if (!proc_source_symlink)
            rc = -ENOMEM;
    }

    if (0 != rc)
        clear_res();
    else
    {
        buffer_end = buffer + BUFSIZE;
        read_buf = buffer;
        write_buf = buffer;
        filled = 0;

        printk("+ Start success");
    }

    return rc;
}

void __exit exit_md(void)
{
    clear_res();
    printk("+ Exit success");
}

module_init(init_md);
module_exit(exit_md);

