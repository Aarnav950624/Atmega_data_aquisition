#include <avr/io.h>
#include <stdint.h>

#include "uart.h"
#include "config.h"

#define BAUD 9600UL

void UART_Init(void)
{
    uint16_t ubrr_value;

    /* Calculate baud-rate register value */
    ubrr_value = (F_CPU / (16UL * BAUD)) - 1;

    /* Load baud-rate value into UBRR */
    UBRRH = (uint8_t)(ubrr_value >> 8);
    UBRRL = (uint8_t)ubrr_value;

    /* Enable transmitter and receiver */
    UCSRB |= (1 << TXEN) | (1 << RXEN);

    /*
     * USART frame format:
     * 8 data bits
     * No parity
     * 1 stop bit
     */
    UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);
}


void UART_Transmit(uint8_t data)
{
    /* Wait until transmit buffer is empty */
    while (!(UCSRA & (1 << UDRE)))
    {
    }

    /* Load data into USART data register */
    UDR = data;
}


uint8_t UART_Receive(void)
{
    /* Wait until data has been received */
    while (!(UCSRA & (1 << RXC)))
    {
    }

    /* Return received data */
    return UDR;
}


void UART_SendString(const char *str)
{
    /* Transmit characters until null terminator */
    while (*str != '\0')
    {
        UART_Transmit((uint8_t)*str);
        str++;
    }
}

void UART_SendNumber(uint16_t number)
{
    char digits[5];
    uint8_t i = 0;

    /* Special case for zero */
    if (number == 0)
    {
        UART_Transmit('0');
        return;
    }

    /* Extract digits from right to left */
    while (number > 0)
    {
        digits[i] = (number % 10) + '0';
        number /= 10;
        i++;
    }

    /* Transmit digits in correct order */
    while (i > 0)
    {
        i--;
        UART_Transmit(digits[i]);
    }
}