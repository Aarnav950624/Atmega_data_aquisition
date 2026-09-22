#ifndef UART_H
#define UART_H

#include <stdint.h>


/*
 * Initialize ATmega16 USART.
 */
void UART_Init(void);


/*
 * Transmit one byte.
 */
void UART_Transmit(uint8_t data);


/*
 * Receive one byte.
 */
uint8_t UART_Receive(void);


/*
 * Transmit a null-terminated string.
 */
void UART_SendString(const char *str);


/*
 * Transmit an unsigned 16-bit integer.
 */
void UART_SendNumber(uint16_t number);


/*
 * Transmit an unsigned 32-bit integer.
 *
 * Used for values such as BMP280
 * pressure readings.
 */
void UART_SendNumber32(uint32_t number);

#endif