#ifndef _test2dev_H
#define _test2dev_H

#include "../../kernel/fs/vfs.h"
#include "../../kernel/types.h"

/* Test device initialization */
void test2dev_init(void);

/* Register test device with VFS */
int test2dev_register(void);

#endif /* _test2dev_H */
