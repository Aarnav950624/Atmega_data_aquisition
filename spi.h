/******************************************************************************
 * COPYRIGHT (C) 2026 Aarnav Patel
 *
 * @file spi.h
 * @brief spi driver for the ATmega16 multiprotocol embedded data acquisition system
 *
 * provides functions for initializing and using the SPI interface
 *
 * @author Aarnav Patel
 * @date September 2026
 *
 *****************************************************************************/

#ifndef SPI_H
#define SPI_H

#include <stdint.h>

/*
 * Initialize ATmega16 SPI peripheral
 *
 * ATmega16 operates as SPI Master.
 */
void SPI_Init(void);


/*
 * Transfer one byte over SPI.
 *
 * The function:
 * - transmits 'data'
 * - simultaneously receives one byte
 *
 * Returns the received byte.
 */
uint8_t SPI_Transfer(uint8_t data);


/*
 * Select SPI slave.
 *
 * CS/SS is driven LOW.
 */
void SPI_Select(void);


/*
 * Deselect SPI slave.
 *
 * CS/SS is driven HIGH.
 */
void SPI_Deselect(void);

#endif