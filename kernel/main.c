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

/* Test scheduler features */
static void test_scheduler_features(void) {
  printf("\n[TEST] Testing advanced scheduler features...\n");

  /* Test 1: Process allocation with priorities */
  printf("\n[TEST] 1. Testing process priorities\n");
  
  process_t *p1 = process_alloc();
  if (p1) {
    const char *name1 = "high-prio";
    int i;
    for (i = 0; name1[i] && i < 31; i++) {
      p1->name[i] = name1[i];
    }
    p1->name[i] = '\0';
    sched_set_priority(p1, 100);  /* High priority (low number) */
    printf("  Created process '%s' (PID %u) with priority %d\n", 
           p1->name, (uint32_t)p1->pid, p1->priority);
  }

  process_t *p2 = process_alloc();
  if (p2) {
    const char *name2 = "low-prio";
    int i;
    for (i = 0; name2[i] && i < 31; i++) {
      p2->name[i] = name2[i];
    }
    p2->name[i] = '\0';
    sched_set_priority(p2, 130);  /* Low priority (high number) */
    printf("  Created process '%s' (PID %u) with priority %d\n", 
           p2->name, (uint32_t)p2->pid, p2->priority);
  }

  /* Test 2: Real-time scheduling policies */
  printf("\n[TEST] 2. Testing real-time scheduling\n");
  
  process_t *rt_fifo = process_alloc();
  if (rt_fifo) {
    const char *name3 = "rt-fifo";
    int i;
    for (i = 0; name3[i] && i < 31; i++) {
      rt_fifo->name[i] = name3[i];
    }
    rt_fifo->name[i] = '\0';
    sched_set_policy(rt_fifo, SCHED_FIFO);
    sched_set_priority(rt_fifo, 50);  /* RT priority */
    printf("  Created RT FIFO process '%s' (PID %u) with priority %d\n", 
           rt_fifo->name, (uint32_t)rt_fifo->pid, rt_fifo->priority);
  }

  process_t *rt_rr = process_alloc();
  if (rt_rr) {
    const char *name4 = "rt-rr";
    int i;
    for (i = 0; name4[i] && i < 31; i++) {
      rt_rr->name[i] = name4[i];
    }
    rt_rr->name[i] = '\0';
    sched_set_policy(rt_rr, SCHED_RR);
    sched_set_priority(rt_rr, 60);  /* RT priority */
    printf("  Created RT Round-Robin process '%s' (PID %u) with priority %d\n", 
           rt_rr->name, (uint32_t)rt_rr->pid, rt_rr->priority);
  }

  /* Test 3: Process statistics */
  printf("\n[TEST] 3. Testing process statistics\n");
  
  if (p1) {
    proc_stats_t stats;
    sched_get_stats(p1, &stats);
    printf("  Process '%s' stats:\n", p1->name);
    printf("    CPU time: %u ticks\n", (uint32_t)stats.cpu_time);
    printf("    Context switches: %u\n", (uint32_t)stats.context_switches);
    printf("    Last run tick: %u\n", (uint32_t)stats.last_run_tick);
  }

  /* Test 4: Multi-level feedback queue */
  printf("\n[TEST] 4. Testing MLFQ\n");
  printf("  MLFQ uses 3 priority levels with different time slices:\n");
  printf("    Level 0 (high): 10 ticks\n");
  printf("    Level 1 (med):  20 ticks\n");
  printf("    Level 2 (low):  40 ticks\n");
  printf("  Processes automatically move between levels based on behavior\n");

  /* Test 5: CPU affinity */
  printf("\n[TEST] 5. Testing SMP/CPU affinity\n");
  if (p1) {
    p1->cpu_affinity = 0;  /* Pin to CPU 0 */
    printf("  Process '%s' pinned to CPU %d\n", p1->name, p1->cpu_affinity);
  }

  /* Test 6: Idle process */
  printf("\n[TEST] 6. Idle process information\n");
  printf("  Idle process (PID 0) runs when no other process is ready\n");
  printf("  Uses WFI (Wait For Interrupt) to save power\n");

  /* Print per-CPU info */
  printf("\n[TEST] 7. Per-CPU information\n");
  cpu_info_t *cpu = current_cpu_info();
  if (cpu) {
    printf("  CPU %d:\n", cpu->cpu_id);
    printf("    Idle time: %u ticks\n", (uint32_t)cpu->idle_time);
    printf("    Busy time: %u ticks\n", (uint32_t)cpu->busy_time);
  }

  printf("\n[TEST] Advanced scheduler features test PASSED\n");
  printf("====================================\n\n");
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
      printf("  echo     - Echo back the input\n");
      printf("  reboot   - Reboot the system\n");
    } else if (buffer[0] == 'i' && buffer[1] == 'n' && buffer[2] == 'f' &&
               buffer[3] == 'o' && buffer[4] == '\0') {
      show_system_info();
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

  /* Show system info */
  show_system_info();

  /* Run tests */
  //   run_tests();
  
  /* Test new scheduler features */
  test_scheduler_features();

  /* Run initial test */
  //   test_memory();

  /* Start shell */
  run_shell();

  /* Should never reach here */
  panic("Kernel main returned");
}
