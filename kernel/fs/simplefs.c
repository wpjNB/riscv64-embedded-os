#include "simplefs.h"
#include "../printf.h"
#include "../mm/mm.h"

/* In-memory structures */
static sfs_superblock_t superblock;
static sfs_inode_t inodes[SFS_MAX_FILES];
static void *data_blocks = NULL;

/* Initialize simple file system */
void sfs_init(void) {
    printf("[SFS] Initializing Simple File System\n");
    
    /* Clear inode table */
    for (int i = 0; i < SFS_MAX_FILES; i++) {
        inodes[i].ino = 0;
        inodes[i].type = 0;
        inodes[i].size = 0;
    }
}

/* Format the file system */
int sfs_format(uint32_t num_blocks) {
    printf("[SFS] Formatting file system with %u blocks\n", num_blocks);
    
    /* Initialize superblock */
    superblock.magic = SFS_MAGIC;
    superblock.block_size = SFS_BLOCK_SIZE;
    superblock.num_blocks = num_blocks;
    superblock.num_inodes = SFS_MAX_FILES;
    superblock.num_free_blocks = num_blocks;
    superblock.num_free_inodes = SFS_MAX_FILES;
    
    /* Allocate data blocks (in memory for now) */
    data_blocks = kmalloc(num_blocks * SFS_BLOCK_SIZE);
    if (data_blocks == NULL) {
        printf("[SFS] Failed to allocate data blocks\n");
        return -1;
    }
    
    printf("[SFS] File system formatted successfully\n");
    return 0;
}

/* Find inode by name */
static sfs_inode_t* find_inode(const char *name) {
    for (int i = 0; i < SFS_MAX_FILES; i++) {
        if (inodes[i].ino != 0) {
            /* Compare names */
            int match = 1;
            for (int j = 0; j < SFS_MAX_FILENAME; j++) {
                if (inodes[i].name[j] != name[j]) {
                    match = 0;
                    break;
                }
                if (name[j] == '\0') {
                    break;
                }
            }
            
            if (match) {
                return &inodes[i];
            }
        }
    }
    
    return NULL;
}

/* Create a new file */
int sfs_create(const char *name, uint32_t type) {
    /* Check if file already exists */
    if (find_inode(name) != NULL) {
        printf("[SFS] File already exists: %s\n", name);
        return -1;
    }
    
    /* Find free inode */
    for (int i = 0; i < SFS_MAX_FILES; i++) {
        if (inodes[i].ino == 0) {
            /* Initialize inode */
            inodes[i].ino = i + 1;
            inodes[i].type = type;
            inodes[i].size = 0;
            
            /* Copy name */
            int j;
            for (j = 0; j < SFS_MAX_FILENAME - 1 && name[j] != '\0'; j++) {
                inodes[i].name[j] = name[j];
            }
            inodes[i].name[j] = '\0';
            
            /* Clear blocks */
            for (j = 0; j < 12; j++) {
                inodes[i].blocks[j] = 0;
            }
            
            superblock.num_free_inodes--;
            
            printf("[SFS] Created file: %s (inode %u)\n", name, inodes[i].ino);
            return inodes[i].ino;
        }
    }
    
    printf("[SFS] No free inodes\n");
    return -1;
}

/* Delete a file */
int sfs_delete(const char *name) {
    sfs_inode_t *inode = find_inode(name);
    if (inode == NULL) {
        printf("[SFS] File not found: %s\n", name);
        return -1;
    }
    
    /* Free blocks */
    for (int i = 0; i < 12; i++) {
        if (inode->blocks[i] != 0) {
            superblock.num_free_blocks++;
        }
    }
    
    /* Clear inode */
    inode->ino = 0;
    inode->type = 0;
    inode->size = 0;
    superblock.num_free_inodes++;
    
    printf("[SFS] Deleted file: %s\n", name);
    return 0;
}

/* Read from a file */
int sfs_read(uint32_t ino, void *buf, uint32_t offset, uint32_t size) {
    /* Find inode */
    if (ino == 0 || ino > SFS_MAX_FILES) {
        return -1;
    }
    
    sfs_inode_t *inode = &inodes[ino - 1];
    if (inode->ino != ino) {
        return -1;
    }
    
    /* Check bounds */
    if (offset >= inode->size) {
        return 0;
    }
    
    if (offset + size > inode->size) {
        size = inode->size - offset;
    }
    
    /* Read data (simplified - from memory buffer) */
    /* Full implementation would read from actual blocks */
    
    return size;
}

/* Write to a file */
int sfs_write(uint32_t ino, const void *buf, uint32_t offset, uint32_t size) {
    /* Find inode */
    if (ino == 0 || ino > SFS_MAX_FILES) {
        return -1;
    }
    
    sfs_inode_t *inode = &inodes[ino - 1];
    if (inode->ino != ino) {
        return -1;
    }
    
    /* Allocate blocks if needed */
    /* Full implementation would manage block allocation */
    
    /* Update size */
    if (offset + size > inode->size) {
        inode->size = offset + size;
    }
    
    return size;
}
