#ifndef _VFS_H
#define _VFS_H

#include "../types.h"

/* File types */
#define VFS_FILE 1
#define VFS_DIR  2
#define VFS_DEV  3

/* File operations */
struct file_operations;

/* inode structure */
typedef struct inode {
    uint32_t ino;              /* Inode number */
    uint32_t type;             /* File type */
    uint32_t size;             /* File size */
    uint32_t ref;              /* Reference count */
    struct file_operations *ops; /* File operations */
    void *private_data;        /* Private data for specific FS */
} inode_t;

/* File descriptor */
typedef struct file {
    inode_t *inode;            /* Associated inode */
    uint32_t offset;           /* Current file offset */
    uint32_t flags;            /* Open flags */
} file_t;

/* File operations */
typedef struct file_operations {
    int (*open)(inode_t *inode, file_t *file);
    int (*close)(file_t *file);
    int (*read)(file_t *file, void *buf, size_t count);
    int (*write)(file_t *file, const void *buf, size_t count);
    int (*seek)(file_t *file, uint32_t offset);
} file_ops_t;

/* VFS functions */
void vfs_init(void);
inode_t* vfs_create_inode(uint32_t type);
void vfs_destroy_inode(inode_t *inode);
file_t* vfs_open(const char *path, uint32_t flags);
int vfs_close(file_t *file);
int vfs_read(file_t *file, void *buf, size_t count);
int vfs_write(file_t *file, const void *buf, size_t count);
int vfs_mount(const char *path, const char *fs_type);

/* Device file registration */
int vfs_register_device(const char *name, file_ops_t *ops);

#endif /* _VFS_H */
