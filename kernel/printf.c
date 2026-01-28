#include "printf.h"
#include "../drivers/uart/uart.h"

typedef __builtin_va_list va_list;
#define va_start(ap, last) __builtin_va_start(ap, last)
#define va_arg(ap, type) __builtin_va_arg(ap, type)
#define va_end(ap) __builtin_va_end(ap)

/* Helper to print a number in a given base */
static void print_num(uint64_t num, int base, int width, char pad) {
    char buf[32];
    int i = 0;
    const char *digits = "0123456789abcdef";
    
    if (num == 0) {
        buf[i++] = '0';
    } else {
        while (num > 0) {
            buf[i++] = digits[num % base];
            num /= base;
        }
    }
    
    /* Padding */
    while (i < width) {
        buf[i++] = pad;
    }
    
    /* Print in reverse order */
    while (i > 0) {
        uart_putc(buf[--i]);
    }
}

void printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            int width = 0;
            char pad = ' ';
            
            /* Parse width and padding */
            if (*fmt == '0') {
                pad = '0';
                fmt++;
            }
            while (*fmt >= '0' && *fmt <= '9') {
                width = width * 10 + (*fmt - '0');
                fmt++;
            }
            
            switch (*fmt) {
                case 'd': {
                    int64_t num = va_arg(args, int64_t);
                    if (num < 0) {
                        uart_putc('-');
                        num = -num;
                    }
                    print_num(num, 10, width, pad);
                    break;
                }
                case 'u': {
                    uint64_t num = va_arg(args, uint64_t);
                    print_num(num, 10, width, pad);
                    break;
                }
                case 'x': {
                    uint64_t num = va_arg(args, uint64_t);
                    print_num(num, 16, width, pad);
                    break;
                }
                case 'p': {
                    uart_puts("0x");
                    uint64_t ptr = (uint64_t)va_arg(args, void*);
                    print_num(ptr, 16, 16, '0');
                    break;
                }
                case 's': {
                    const char *s = va_arg(args, const char*);
                    if (s) {
                        uart_puts(s);
                    } else {
                        uart_puts("(null)");
                    }
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    uart_putc(c);
                    break;
                }
                case '%': {
                    uart_putc('%');
                    break;
                }
                default: {
                    uart_putc('%');
                    uart_putc(*fmt);
                    break;
                }
            }
        } else {
            if (*fmt == '\n') {
                uart_putc('\r');
            }
            uart_putc(*fmt);
        }
        fmt++;
    }
    
    va_end(args);
}

void panic(const char *msg) {
    printf("\n\n*** KERNEL PANIC ***\n");
    printf("%s\n", msg);
    printf("System halted.\n");
    
    /* Halt the system */
    while (1) {
        asm volatile("wfi");
    }
}
