#include <avr/io.h>
#include <stdint.h>

#include "uart.h"
#include "config.h"


#define BAUD 9600UL


/*
 * Initialize ATmega16 USART.
 */
void UART_Init(void)
{
    uint16_t ubrr_value;


    /*
     * Calculate baud-rate register value.
     *
     * Formula:
     *
     * UBRR = F_CPU / (16 × BAUD) - 1
     *
     * F_CPU = 8 MHz
     * BAUD  = 9600
     */
    ubrr_value =
        (F_CPU / (16UL * BAUD)) - 1;


    /*
     * Load baud-rate value.
     *
     * UBRRH = upper 8 bits
     * UBRRL = lower 8 bits
     */
    UBRRH =
        (uint8_t)(ubrr_value >> 8);

    UBRRL =
        (uint8_t)ubrr_value;


    /*
     * Enable transmitter and receiver.
     */
    UCSRB |=
        (1 << TXEN) |
        (1 << RXEN);


    /*
     * USART frame format:
     *
     * 8 data bits
     * No parity
     * 1 stop bit
     *
     * UCSZ1 = 1
     * UCSZ0 = 1
     */
    UCSRC =
        (1 << URSEL) |
        (1 << UCSZ1) |
        (1 << UCSZ0);
}


/*
 * Transmit one byte.
 */
void UART_Transmit(uint8_t data)
{
    /*
     * Wait until transmit buffer
     * is empty.
     */
    while (!(UCSRA & (1 << UDRE)))
    {
    }


    /*
     * Put data into USART
     * data register.
     */
    UDR = data;
}


/*
 * Receive one byte.
 */
uint8_t UART_Receive(void)
{
    /*
     * Wait until a byte has
     * been received.
     */
    while (!(UCSRA & (1 << RXC)))
    {
    }


    /*
     * Return received byte.
     */
    return UDR;
}


/*
 * Transmit a string.
 */
void UART_SendString(const char *str)
{
    /*
     * Continue until the
     * null terminator.
     */
    while (*str != '\0')
    {
        UART_Transmit(
            (uint8_t)*str
        );

        str++;
    }
}


/*
 * Transmit an unsigned 16-bit integer.
 */
void UART_SendNumber(uint16_t number)
{
    char digits[5];
    uint8_t i = 0;


    /*
     * Special case for zero.
     */
    if (number == 0)
    {
        UART_Transmit('0');
        return;
    }


    /*
     * Extract digits from
     * right to left.
     */
    while (number > 0)
    {
        digits[i] =
            (number % 10) + '0';

        number /= 10;

        i++;
    }


    /*
     * Transmit digits in
     * correct order.
     */
    while (i > 0)
    {
        i--;

        UART_Transmit(
            digits[i]
        );
    }
}


/*
 * Transmit an unsigned 32-bit integer.
 */
void UART_SendNumber32(uint32_t number)
{
    /*
     * Maximum unsigned 32-bit value:
     *
     * 4294967295
     *
     * Therefore 10 decimal digits
     * are sufficient.
     */
    char digits[10];

    uint8_t i = 0;


    /*
     * Special case for zero.
     */
    if (number == 0)
    {
        UART_Transmit('0');
        return;
    }


    /*
     * Extract digits from
     * right to left.
     */
    while (number > 0)
    {
        digits[i] =
            (number % 10) + '0';

        number /= 10;

        i++;
    }


    /*
     * Transmit digits in
     * correct order.
     */
    while (i > 0)
    {
        i--;

        UART_Transmit(
            digits[i]
        );
    }
}