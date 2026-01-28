#include "vfs.h"
#include "../printf.h"
#include "../mm/mm.h"

#define MAX_DEVICES 16

/* Device registry */
typedef struct device {
    char name[32];
    file_ops_t *ops;
    int used;
} device_t;

static device_t devices[MAX_DEVICES];
static uint32_t next_ino = 1;

/* Initialize VFS layer */
void vfs_init(void) {
    printf("[VFS] Initializing Virtual File System\n");
    
    /* Clear device registry */
    for (int i = 0; i < MAX_DEVICES; i++) {
        devices[i].used = 0;
    }
    
    printf("[VFS] VFS initialized\n");
}

/* Create a new inode */
inode_t* vfs_create_inode(uint32_t type) {
    inode_t *inode = (inode_t*)kmalloc(sizeof(inode_t));
    if (inode == NULL) {
        return NULL;
    }
    
    inode->ino = next_ino++;
    inode->type = type;
    inode->size = 0;
    inode->ref = 1;
    inode->ops = NULL;
    inode->private_data = NULL;
    
    return inode;
}

/* Destroy an inode */
void vfs_destroy_inode(inode_t *inode) {
    if (inode == NULL) {
        return;
    }
    
    inode->ref--;
    if (inode->ref == 0) {
        kfree(inode);
    }
}

/* Register a device */
int vfs_register_device(const char *name, file_ops_t *ops) {
    /* Find free slot */
    for (int i = 0; i < MAX_DEVICES; i++) {
        if (!devices[i].used) {
            /* Copy name */
            int j;
            for (j = 0; j < 31 && name[j] != '\0'; j++) {
                devices[i].name[j] = name[j];
            }
            devices[i].name[j] = '\0';
            
            devices[i].ops = ops;
            devices[i].used = 1;
            
            printf("[VFS] Registered device: %s\n", name);
            return 0;
        }
    }
    
    printf("[VFS] Failed to register device: %s (no free slots)\n", name);
    return -1;
}

/* Find device by name */
static device_t* find_device(const char *name) {
    for (int i = 0; i < MAX_DEVICES; i++) {
        if (devices[i].used) {
            /* Compare names */
            int match = 1;
            for (int j = 0; j < 32; j++) {
                if (devices[i].name[j] != name[j]) {
                    match = 0;
                    break;
                }
                if (name[j] == '\0') {
                    break;
                }
            }
            
            if (match) {
                return &devices[i];
            }
        }
    }
    
    return NULL;
}

/* Open a file */
file_t* vfs_open(const char *path, uint32_t flags) {
    /* Simple implementation: check if it's a device */
    if (path[0] == '/') {
        path++;  /* Skip leading slash */
    }
    
    device_t *dev = find_device(path);
    if (dev == NULL) {
        printf("[VFS] File not found: %s\n", path);
        return NULL;
    }
    
    /* Create file descriptor */
    file_t *file = (file_t*)kmalloc(sizeof(file_t));
    if (file == NULL) {
        return NULL;
    }
    
    /* Create inode */
    file->inode = vfs_create_inode(VFS_DEV);
    if (file->inode == NULL) {
        kfree(file);
        return NULL;
    }
    
    file->inode->ops = dev->ops;
    file->offset = 0;
    file->flags = flags;
    
    /* Call device open */
    if (dev->ops->open) {
        if (dev->ops->open(file->inode, file) != 0) {
            vfs_destroy_inode(file->inode);
            kfree(file);
            return NULL;
        }
    }
    
    return file;
}

/* Close a file */
int vfs_close(file_t *file) {
    if (file == NULL) {
        return -1;
    }
    
    /* Call device close */
    if (file->inode->ops && file->inode->ops->close) {
        file->inode->ops->close(file);
    }
    
    vfs_destroy_inode(file->inode);
    kfree(file);
    
    return 0;
}

/* Read from a file */
int vfs_read(file_t *file, void *buf, size_t count) {
    if (file == NULL || file->inode->ops == NULL || file->inode->ops->read == NULL) {
        return -1;
    }
    
    return file->inode->ops->read(file, buf, count);
}

/* Write to a file */
int vfs_write(file_t *file, const void *buf, size_t count) {
    if (file == NULL || file->inode->ops == NULL || file->inode->ops->write == NULL) {
        return -1;
    }
    
    return file->inode->ops->write(file, buf, count);
}

/* Mount filesystem (stub) */
int vfs_mount(const char *path, const char *fs_type) {
    printf("[VFS] Mount %s at %s (not implemented)\n", fs_type, path);
    return 0;
}
