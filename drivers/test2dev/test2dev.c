#include "test2dev.h"

#include "../../kernel/mm/mm.h"
#include "../../kernel/printf.h"

/* Test device buffer */
#define test2dev_SIZE 1024

static char test2dev_buffer[test2dev_SIZE];
static uint32_t test2dev_len = 0;

/* Device open */
static int test2dev_open(inode_t *inode, file_t *file) {
  printf("[test2dev] Device opened\n");
  return 0;
}

/* Device close */
static int test2dev_close(file_t *file) {
  printf("[test2dev] Device closed\n");
  return 0;
}

/* Device read */
static int test2dev_read(file_t *file, void *buf, size_t count) {
  /* Calculate available data */
  uint32_t available = test2dev_len - file->offset;
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
    cbuf[i] = test2dev_buffer[file->offset + i];
  }

  /* Update offset */
  file->offset += count;

  printf("[test2dev] Read %u bytes (offset now %u)\n", (uint32_t)count,
         file->offset);
  return count;
}

/* Device write */
static int test2dev_write(file_t *file, const void *buf, size_t count) {
  /* Calculate space available */
  uint32_t space = test2dev_SIZE - file->offset;
  if (space == 0) {
    printf("[test2dev] Buffer full\n");
    return -1;
  }

  /* Write at most 'count' bytes */
  if (count > space) {
    count = space;
  }

  /* Copy data from user buffer */
  const char *cbuf = (const char *)buf;
  for (size_t i = 0; i < count; i++) {
    test2dev_buffer[file->offset + i] = cbuf[i];
  }

  /* Update offset and length */
  file->offset += count;
  if (file->offset > test2dev_len) {
    test2dev_len = file->offset;
  }

  printf("[test2dev] Wrote %u bytes (offset now %u, total %u)\n",
         (uint32_t)count, file->offset, test2dev_len);
  return count;
}

/* Device seek */
static int test2dev_seek(file_t *file, uint32_t offset) {
  if (offset > test2dev_SIZE) {
    printf("[test2dev] Seek offset too large\n");
    return -1;
  }

  file->offset = offset;
  printf("[test2dev] Seek to offset %u\n", offset);
  return 0;
}

/* File operations table */
static file_ops_t test2dev_ops = {.open = test2dev_open,
                                  .close = test2dev_close,
                                  .read = test2dev_read,
                                  .write = test2dev_write,
                                  .seek = test2dev_seek};

/* Initialize test device */
void test2dev_init(void) {
  printf("[test2dev] Initializing test device\n");

  /* Clear buffer */
  for (int i = 0; i < test2dev_SIZE; i++) {
    test2dev_buffer[i] = 0;
  }
  test2dev_len = 0;

  printf("[test2dev] Test device initialized (buffer size: %d bytes)\n",
         test2dev_SIZE);
}

/* Register with VFS */
int test2dev_register(void) {
  int ret = vfs_register_device("test2dev", &test2dev_ops);
  if (ret == 0) {
    printf("[test2dev] Registered as /test2dev\n");
  }
  return ret;
}
