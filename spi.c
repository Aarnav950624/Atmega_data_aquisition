#include <avr/io.h>
#include <stdint.h>

#include "spi.h"


/*
 * ATmega16 SPI pin definitions
 *
 * PB4 -> SS
 * PB5 -> MOSI
 * PB6 -> MISO
 * PB7 -> SCK
 */


/*
 * Initialize SPI
 */
void SPI_Init(void)
{
    /*
     * Configure SPI pins.
     *
     * PB4 = SS   -> Output
     * PB5 = MOSI -> Output
     * PB6 = MISO -> Input
     * PB7 = SCK  -> Output
     */
    DDRB |= (1 << PB4) |
            (1 << PB5) |
            (1 << PB7);

    DDRB &= ~(1 << PB6);


    /*
     * Keep chip select HIGH.
     *
     * HIGH = BMP280 not selected.
     */
    PORTB |= (1 << PB4);


    /*
     * Configure SPI:
     *
     * SPE  = SPI Enable
     * MSTR = Master mode
     *
     * CPOL = 0
     * CPHA = 0
     *
     * Therefore:
     * SPI Mode 0
     *
     * SPR0 = 1
     *
     * SPI clock = F_CPU / 16
     *
     * At F_CPU = 8 MHz:
     *
     * SPI clock = 500 kHz
     */
    SPCR = (1 << SPE) |
           (1 << MSTR) |
           (1 << SPR0);


    /*
     * SPI Double Speed disabled.
     */
    SPSR &= ~(1 << SPI2X);
}


/*
 * Transfer one byte over SPI
 */
uint8_t SPI_Transfer(uint8_t data)
{
    /*
     * Put data into SPI Data Register.
     *
     * This starts the SPI transfer.
     */
    SPDR = data;


    /*
     * Wait until transfer is complete.
     *
     * SPIF becomes 1 when
     * the transfer finishes.
     */
    while (!(SPSR & (1 << SPIF)))
    {
    }


    /*
     * Return received byte.
     *
     * SPI is full duplex:
     *
     * transmit -> SPDR
     * receive  <- SPDR
     */
    return SPDR;
}


/*
 * Select BMP280
 */
void SPI_Select(void)
{
    /*
     * CSB LOW
     *
     * This tells BMP280 that
     * an SPI transaction is starting.
     */
    PORTB &= ~(1 << PB4);
}


/*
 * Deselect BMP280
 */
void SPI_Deselect(void)
{
    /*
     * CSB HIGH
     *
     * This ends the SPI transaction.
     */
    PORTB |= (1 << PB4);
}