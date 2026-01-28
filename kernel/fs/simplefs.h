#ifndef _SIMPLEFS_H
#define _SIMPLEFS_H

#include "../types.h"

/* Simple FS constants */
#define SFS_MAGIC 0x53465331  /* "SFS1" */
#define SFS_BLOCK_SIZE 4096
#define SFS_MAX_FILES 64
#define SFS_MAX_FILENAME 28

/* Simple FS superblock */
typedef struct {
    uint32_t magic;           /* Magic number */
    uint32_t block_size;      /* Block size */
    uint32_t num_blocks;      /* Total blocks */
    uint32_t num_inodes;      /* Total inodes */
    uint32_t num_free_blocks; /* Free blocks */
    uint32_t num_free_inodes; /* Free inodes */
} sfs_superblock_t;

/* Simple FS inode */
typedef struct {
    uint32_t ino;             /* Inode number */
    uint32_t type;            /* File type (1=file, 2=dir) */
    uint32_t size;            /* File size */
    uint32_t blocks[12];      /* Direct blocks */
    char name[SFS_MAX_FILENAME]; /* File name */
} sfs_inode_t;

/* Simple FS functions */
void sfs_init(void);
int sfs_format(uint32_t num_blocks);
int sfs_create(const char *name, uint32_t type);
int sfs_delete(const char *name);
int sfs_read(uint32_t ino, void *buf, uint32_t offset, uint32_t size);
int sfs_write(uint32_t ino, const void *buf, uint32_t offset, uint32_t size);

#endif /* _SIMPLEFS_H */
