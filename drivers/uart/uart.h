#ifndef _UART_H
#define _UART_H

#include "../../kernel/types.h"

/* UART initialization */
void uart_init(void);

/* UART output */
void uart_putc(char c);
void uart_puts(const char *s);

/* UART input */
char uart_getc(void);
int uart_has_char(void);

#endif /* _UART_H */
