#include "../drivers/testdev/testdev.h"
#include "../drivers/uart/uart.h"
#include "fs/simplefs.h"
#include "fs/vfs.h"
#include "mm/mm.h"
#include "mm/vm.h"
#include "printf.h"
#include "process/scheduler.h"
#include "riscv.h"
#include "trap/trap.h"
#include "types.h"

#define SHELL_BUFFER_SIZE 128
#define QEMU_VIRT_TEST 0x100000 /* QEMU virt test device for poweroff */

/* Test virtual memory */
static void test_vm(void) {
  printf("[TEST] Testing virtual memory...\n");

  // Test user page table creation
  pagetable_t pt = vm_create_user_pagetable();
  if (pt == NULL) {
    printf("[TEST] Failed to create user page table\n");
    return;
  }
  printf("[TEST] Created user page table at %p\n", pt);

  // Test mapping
  uint64_t pa = 0x80100000;
  int ret = mappages(pt, 0x1000, PAGE_SIZE, pa, PTE_R | PTE_W | PTE_U);
  if (ret != 0) {
    printf("[TEST] Failed to map page\n");
    return;
  }
  printf("[TEST] Mapped VA 0x1000 -> PA %p\n", (void *)pa);

  // Test address translation
  uint64_t pa2 = walkaddr(pt, 0x1000);
  if (pa2 != pa) {
    printf("[TEST] Address translation failed: got %p, expected %p\n",
           (void *)pa2, (void *)pa);
    return;
  }
  printf("[TEST] Address translation verified\n");

  // Clean up
  vm_free(pt);

  printf("[TEST] Virtual memory test PASSED\n");
}

/* Test scheduler */
static void test_scheduler(void) {
  printf("[TEST] Testing scheduler...\n");

  // Test process allocation
  process_t *p1 = process_alloc();
  if (p1 == NULL) {
    printf("[TEST] Failed to allocate process 1\n");
    return;
  }

  int i;
  for (i = 0; i < 5 && (p1->name[i] = "test1"[i]); i++)
    ;
  p1->name[i] = '\0';
  printf("[TEST] Allocated process: %s (PID %lu)\n", p1->name, p1->pid);

  process_t *p2 = process_alloc();
  if (p2 == NULL) {
    printf("[TEST] Failed to allocate process 2\n");
    return;
  }

  for (i = 0; i < 5 && (p2->name[i] = "test2"[i]); i++)
    ;
  p2->name[i] = '\0';
  printf("[TEST] Allocated process: %s (PID %lu)\n", p2->name, p2->pid);

  // Add to scheduler
  sched_add(p1);
  sched_add(p2);
  printf("[TEST] Added processes to scheduler\n");

  printf("[TEST] Scheduler test PASSED\n");
}

/* Test file system */
static void test_filesystem(void) {
  printf("[TEST] Testing file system...\n");

  // Test file creation
  int ino = sfs_create("testfile", VFS_FILE);
  if (ino <= 0) {
    printf("[TEST] Failed to create file\n");
    return;
  }
  printf("[TEST] Created file 'testfile' with inode %d\n", ino);

  // Test file deletion
  int ret = sfs_delete("testfile");
  if (ret != 0) {
    printf("[TEST] Failed to delete file\n");
    return;
  }
  printf("[TEST] Deleted file 'testfile'\n");

  printf("[TEST] File system test PASSED\n");
}

/* Run all tests */
static void run_tests(void) {
  printf("\n========================================\n");
  printf("  Running System Tests\n");
  printf("========================================\n\n");

  test_vm();
  printf("\n");

  test_scheduler();
  printf("\n");

  test_filesystem();
  printf("\n");

  printf("========================================\n");
  printf("  All Tests Completed\n");
  printf("========================================\n\n");
}

/* Banner */
static void print_banner(void) {
  printf("\n");
  printf("====================================\n");
  printf("  RISC-V 64-bit Embedded OS\n");
  printf("  Version 1.0\n");
  printf("====================================\n");
  printf("\n");
}

/* Test memory allocation */
static void test_memory(void) {
  printf("[TEST] Testing memory allocation...\n");

  /* Test page allocation */
  void *page1 = alloc_page();
  void *page2 = alloc_page();
  printf("[TEST] Allocated pages: %p, %p\n", page1, page2);

  /* Test heap allocation */
  void *ptr1 = kmalloc(256);
  void *ptr2 = kmalloc(512);
  printf("[TEST] Allocated heap: %p, %p\n", ptr1, ptr2);

  /* Free pages */
  free_page(page1);
  free_page(page2);
  printf("[TEST] Memory test completed\n");
}

/* Display system information */
static void show_system_info(void) {
  printf("\n[INFO] System Information:\n");
  printf("  Architecture: RISC-V 64-bit (RV64IMAC)\n");
  printf("  Privilege Mode: Supervisor (S-mode)\n");
  printf("  Page Size: %d bytes\n", PAGE_SIZE);

  /* Read CSR registers */
  uint64_t sstatus = r_sstatus();
  uint64_t sie = r_sie();
  uint64_t stvec = r_stvec();

  printf("  sstatus: %p\n", (void *)sstatus);
  printf("  sie:     %p\n", (void *)sie);
  printf("  stvec:   %p\n", (void *)stvec);
  printf("\n");
}

/* Simple shell */
static void run_shell(void) {
  printf("[SHELL] Starting simple shell\n");
  printf("Type 'help' for available commands\n");

  char buffer[SHELL_BUFFER_SIZE];
  int pos = 0;

  while (1) {
    printf("> ");

    /* Read line */
    pos = 0;
    while (1) {
      char c = uart_getc();

      if (c == '\r' || c == '\n') {
        buffer[pos] = '\0';
        printf("\n");
        break;
      } else if (c == '\b' || c == 127) { /* Backspace */
        if (pos > 0) {
          pos--;
          printf("\b \b");
        }
      } else if (c >= 32 && c < 127) { /* Printable */
        if (pos < sizeof(buffer) - 1) {
          buffer[pos++] = c;
          uart_putc(c);
        }
      }
    }

    /* Process command */
    if (pos == 0) {
      continue;
    }

    /* Simple command parser */
    if (buffer[0] == 'h' && buffer[1] == 'e' && buffer[2] == 'l' &&
        buffer[3] == 'p' && buffer[4] == '\0') {
      printf("Available commands:\n");
      printf("  help     - Show this help message\n");
      printf("  info     - Show system information\n");
      printf("  test     - Run memory test\n");
      printf("  testdev  - Test VFS device driver\n");
      printf("  echo     - Echo back the input\n");
      printf("  reboot   - Reboot the system\n");
    } else if (buffer[0] == 'i' && buffer[1] == 'n' && buffer[2] == 'f' &&
               buffer[3] == 'o' && buffer[4] == '\0') {
      show_system_info();
    } else if (buffer[0] == 't' && buffer[1] == 'e' && buffer[2] == 's' &&
               buffer[3] == 't' && buffer[4] == 'd' && buffer[5] == 'e' &&
               buffer[6] == 'v' && buffer[7] == '\0') {
      /* Test the test device */
      printf("[TEST] Testing /testdev device\n");

      /* Open device */
      file_t *file = vfs_open("/testdev", 0);
      if (file == NULL) {
        printf("[TEST] Failed to open /testdev\n");
        continue;
      }

      /* Write some data */
      const char *test_data = "Hello from VFS test!";
      int written = vfs_write(file, test_data, 20);
      printf("[TEST] Wrote %d bytes\n", written);

      /* Seek back to beginning */
      if (file->inode->ops->seek) {
        file->inode->ops->seek(file, 0);
      }

      /* Read back */
      char read_buf[64];
      int read_bytes = vfs_read(file, read_buf, sizeof(read_buf));
      if (read_bytes > 0) {
        read_buf[read_bytes] = '\0';
        printf("[TEST] Read %d bytes: %s\n", read_bytes, read_buf);
      }

      /* Close */
      vfs_close(file);
      printf("[TEST] Test completed\n");

    } else if (buffer[0] == 't' && buffer[1] == 'e' && buffer[2] == 's' &&
               buffer[3] == 't' && buffer[4] == '\0') {
      test_memory();
    } else if (buffer[0] == 'e' && buffer[1] == 'c' && buffer[2] == 'h' &&
               buffer[3] == 'o' && pos > 4 && buffer[4] == ' ') {
      printf("%s\n", buffer + 5);
    } else if (buffer[0] == 'r' && buffer[1] == 'e' && buffer[2] == 'b' &&
               buffer[3] == 'o' && buffer[4] == 'o' && buffer[5] == 't' &&
               buffer[6] == '\0') {
      printf("Rebooting...\n");
      /* QEMU virt test device - writing 0x5555 causes QEMU to exit */
      *(volatile uint32_t *)QEMU_VIRT_TEST = 0x5555;
    } else {
      printf("Unknown command: %s\n", buffer);
      printf("Type 'help' for available commands\n");
    }
  }
}

/* Kernel main entry */
void kernel_main(void) {
  /* Initialize UART for console output */
  uart_init();

  /* Print banner */
  print_banner();

  printf("[KERNEL] Starting RISC-V OS kernel...\n");
  printf("[KERNEL] Kernel loaded at 0x80000000\n");

  /* Initialize memory management */
  mm_init();

  /* Initialize virtual memory (SV39 paging) */
  vm_init();
  kvminithart();

  /* Initialize trap handling */
  trap_init();

  /* Initialize scheduler */
  scheduler_init();

  /* Initialize file systems */
  vfs_init();
  sfs_init();
  sfs_format(256); /* Format with 256 blocks (1MB) */

  /* Initialize and register test device */
  testdev_init();
  testdev_register();

  /* Show system info */
  show_system_info();

  /* Run tests */
  run_tests();

  /* Run initial test */
  test_memory();

  /* Start shell */
  run_shell();

  /* Should never reach here */
  panic("Kernel main returned");
}
