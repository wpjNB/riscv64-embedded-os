#include "testdev.h"

#include "../../kernel/printf.h"

/* Test device buffer */
#define TESTDEV_SIZE 1024

static char testdev_buffer[TESTDEV_SIZE];
static uint32_t testdev_len = 0;

/* Device open */
static int testdev_open(inode_t *inode, file_t *file) {
  (void)inode;
  (void)file;
  return 0;
}

/* Device close */
static int testdev_close(file_t *file) {
  (void)file;
  return 0;
}

/* Device read */
static int testdev_read(file_t *file, void *buf, size_t count) {
  /* Calculate available data */
  uint32_t available = testdev_len - file->offset;
  if (available == 0) {
    return 0; /* EOF */
  }

  /* Read at most 'count' bytes */
  if (count > available) {
    count = available;
  }

  /* Copy data to user buffer */
  char *cbuf = (char *)buf;
  for (size_t i = 0; i < count; i++) {
    cbuf[i] = testdev_buffer[file->offset + i];
  }

  /* Update offset */
  file->offset += count;

  return count;
}

/* Device write */
static int testdev_write(file_t *file, const void *buf, size_t count) {
  /* Calculate space available */
  uint32_t space = TESTDEV_SIZE - file->offset;
  if (space == 0) {
    printf("[TESTDEV] Buffer full\n");
    return -1;
  }

  /* Write at most 'count' bytes */
  if (count > space) {
    count = space;
  }

  /* Copy data from user buffer */
  const char *cbuf = (const char *)buf;
  for (size_t i = 0; i < count; i++) {
    testdev_buffer[file->offset + i] = cbuf[i];
  }

  /* Update offset and length */
  file->offset += count;
  if (file->offset > testdev_len) {
    testdev_len = file->offset;
  }

  return count;
}

/* Device seek */
static int testdev_seek(file_t *file, uint32_t offset) {
  if (offset > TESTDEV_SIZE) {
    return -1;
  }

  file->offset = offset;
  return 0;
}

/* File operations table */
static file_ops_t testdev_ops = {.open = testdev_open,
                                 .close = testdev_close,
                                 .read = testdev_read,
                                 .write = testdev_write,
                                 .seek = testdev_seek};

/* Initialize test device */
void testdev_init(void) {
  /* Clear buffer - NO PRINTF */
  // for (int i = 0; i < TESTDEV_SIZE; i++) {
  //   testdev_buffer[i] = 0;
  // }
  testdev_len = 0;
}

/* Register with VFS */
int testdev_register(void) {
  /* NO PRINTF - just register silently */
  return vfs_register_device("testdev", &testdev_ops);
}
