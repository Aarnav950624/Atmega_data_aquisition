#ifndef UART_H
#define UART_H

#include <stdint.h>

/* Initialize ATmega16 USART */
void UART_Init(void);

/* Transmit one byte */
void UART_Transmit(uint8_t data);

/* Receive one byte */
uint8_t UART_Receive(void);

/* Transmit a null-terminated string */
void UART_SendString(const char *str);

#endif