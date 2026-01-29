#ifndef _TESTDEV_H
#define _TESTDEV_H

#include "../../kernel/fs/vfs.h"
#include "../../kernel/types.h"

/* Test device initialization */
void testdev_init(void);

/* Register test device with VFS */
int testdev_register(void);

#endif /* _TESTDEV_H */
