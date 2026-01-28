#include "types.h"
#include "riscv.h"
#include "printf.h"
#include "mm/mm.h"
#include "trap/trap.h"
#include "../drivers/uart/uart.h"

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
    
    printf("  sstatus: %p\n", (void*)sstatus);
    printf("  sie:     %p\n", (void*)sie);
    printf("  stvec:   %p\n", (void*)stvec);
    printf("\n");
}

/* Simple shell */
static void run_shell(void) {
    printf("[SHELL] Starting simple shell\n");
    printf("Type 'help' for available commands\n");
    
    char buffer[128];
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
                   buffer[3] == 'o' && buffer[4] == ' ') {
            printf("%s\n", buffer + 5);
        } else if (buffer[0] == 'r' && buffer[1] == 'e' && buffer[2] == 'b' && 
                   buffer[3] == 'o' && buffer[4] == 'o' && buffer[5] == 't' && 
                   buffer[6] == '\0') {
            printf("Rebooting...\n");
            /* On QEMU, we can exit by writing to a special address */
            *(volatile uint32_t*)0x100000 = 0x5555; /* QEMU poweroff */
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
    
    /* Initialize trap handling */
    trap_init();
    
    /* Show system info */
    show_system_info();
    
    /* Run initial test */
    test_memory();
    
    /* Start shell */
    run_shell();
    
    /* Should never reach here */
    panic("Kernel main returned");
}
