#include "uart.h"

/* UART registers for QEMU virt machine */
#define UART_BASE 0x10000000UL

#define UART_RBR (UART_BASE + 0) /* Receive Buffer Register */
#define UART_THR (UART_BASE + 0) /* Transmit Holding Register */
#define UART_IER (UART_BASE + 1) /* Interrupt Enable Register */
#define UART_FCR (UART_BASE + 2) /* FIFO Control Register */
#define UART_LCR (UART_BASE + 3) /* Line Control Register */
#define UART_MCR (UART_BASE + 4) /* Modem Control Register */
#define UART_LSR (UART_BASE + 5) /* Line Status Register */

#define UART_LSR_TX_IDLE (1 << 5) /* Transmitter empty */
#define UART_LSR_RX_READY (1 << 0) /* Data ready */

/* Read/Write register macros */
#define READ_REG(addr) (*(volatile uint8_t *)(addr))
#define WRITE_REG(addr, val) (*(volatile uint8_t *)(addr) = (val))

void uart_init(void) {
    /* UART is already initialized by QEMU, no additional setup needed */
}

void uart_putc(char c) {
    /* Wait for transmitter to be ready */
    while ((READ_REG(UART_LSR) & UART_LSR_TX_IDLE) == 0)
        ;
    
    /* Write character */
    WRITE_REG(UART_THR, c);
}

void uart_puts(const char *s) {
    while (*s) {
        if (*s == '\n') {
            uart_putc('\r');
        }
        uart_putc(*s);
        s++;
    }
}

char uart_getc(void) {
    /* Wait for data to be ready */
    while ((READ_REG(UART_LSR) & UART_LSR_RX_READY) == 0)
        ;
    
    /* Read character */
    return READ_REG(UART_RBR);
}

int uart_has_char(void) {
    return (READ_REG(UART_LSR) & UART_LSR_RX_READY) != 0;
}
