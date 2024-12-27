#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/time.h>
#include <linux/slab.h>

#define MYFS_MAGIC_NUMBER 0x58ae901cc

MODULE_LICENSE("GPL");

struct myfs_inode
{
    int i_mode;
    unsigned long i_ino;
};

struct kmem_cache *cache = NULL;

static void myfs_inode_ctor(void *data)
{
    struct myfs_inode *inode = data;

    inode->i_mode = 0;
    inode->i_ino = 0;
}

static struct inode *myfs_make_inode(struct super_block *sb, int mode)
{
    struct inode *ret = new_inode(sb);

    if (ret)
    {
        inode_init_owner(sb->s_user_ns, ret, NULL, mode);
        ret->i_size = PAGE_SIZE;
        ret->i_atime = ret->i_mtime = ret->i_ctime = current_time(ret);
        ret->i_private = kmem_cache_alloc(cache, GFP_KERNEL);

        if (ret->i_private)
            ((struct myfs_inode *)ret->i_private)->i_mode = mode;
    }

    return ret;
}

static void myfs_put_super(struct super_block *sb)
{
    printk("+ MYFS super block destroyed!\n" ) ;
}

static int myfs_delete_inode(struct inode *inode)
{
    kmem_cache_free(cache, inode->i_private);

    return generic_delete_inode(inode);
}

static const struct super_operations myfs_super_ops = {
    .put_super = myfs_put_super,
    .statfs = simple_statfs,
    .drop_inode = myfs_delete_inode
};

// sb описывает подмонтированную файловую систему
static int myfs_fill_sb(struct super_block *sb, void *data, int silent)
{
    sb->s_blocksize = PAGE_SIZE; // Память выделяется страницами
    sb->s_magic = MYFS_MAGIC_NUMBER;
    sb->s_op = &myfs_super_ops;

    struct inode *root = myfs_make_inode(sb, S_IFDIR | 0755);

    if (!root)
    {
        printk("+ MYFS inode allocation failed !\n") ;
        return -ENOMEM;
    }

    ((struct myfs_inode *)root->i_private)->i_ino = root->i_ino = 1;
    root->i_op = &simple_dir_inode_operations;
    root->i_fop = &simple_dir_operations;

    sb->s_root = d_make_root(root) ;

    if (!sb->s_root)
    {
        printk("+ MYFS root creation failed !\n") ;
        iput(root);
        return -ENOMEM;
    }

    return 0;
}

static struct dentry *myfs_mount(struct file_system_type *type, int flags,
                                 const char *dev, void *data)
{
    struct dentry *const entry = mount_nodev(type, flags, data, myfs_fill_sb);

    if (IS_ERR(entry))
        printk("+ MYFS mounting failed !\n") ;
    else
        printk("+ MYFS mounted!\n") ;

    return entry;
}

static struct file_system_type myfs_type = {
    .owner = THIS_MODULE,
    .name = "myfs",
    // Minimum
    .mount = myfs_mount,
    .kill_sb = kill_anon_super
};

static int __init init_md(void)
{
    cache = kmem_cache_create("myfs_slab", sizeof(struct myfs_inode), 0,
                              SLAB_HWCACHE_ALIGN, myfs_inode_ctor);

    if (!cache)
        return -ENOMEM;

    int ret = register_filesystem(&myfs_type);

    if (ret)
        printk("+ MYFS_MODULE cannot register filesystem!\n");
    else
        printk("+ MYFS_MODULE loaded !\n");

    return ret;
}

static void __exit exit_md(void)
{
    int ret = unregister_filesystem(&myfs_type);

    kmem_cache_destroy(cache);

    if (ret != 0)
        printk("+ MYFS_MODULE cannot unregister filesystem !\n");
    else
        printk("+ MYFS_MODULE unloaded !\n");
}

module_init(init_md);
module_exit(exit_md);


