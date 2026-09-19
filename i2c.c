#include <avr/io.h>
#include <stdint.h>

#include "i2c.h"
#include "config.h"

/*
 * I2C clock frequency
 */
#define I2C_CLOCK 100000UL

/*
 * TWI status mask
 *
 * The lower three bits of TWSR contain
 * the prescaler configuration.
 *
 * Bits 7:3 contain the actual TWI status.
 */
#define TWI_STATUS_MASK 0xF8


void I2C_Init(void)
{
    uint8_t twbr_value;

    /*
     * Set TWI prescaler to 1.
     *
     * TWPS1 = 0
     * TWPS0 = 0
     */
    TWSR &= ~((1 << TWPS1) | (1 << TWPS0));

    /*
     * Calculate TWBR value for desired SCL frequency.
     *
     * SCL = F_CPU / (16 + 2*TWBR*Prescaler)
     *
     * With:
     * F_CPU = 8 MHz
     * SCL   = 100 kHz
     * Prescaler = 1
     *
     * TWBR = 32
     */
    twbr_value = (uint8_t)(((F_CPU / I2C_CLOCK) - 16UL) / 2UL);

    TWBR = twbr_value;

    /*
     * Enable TWI peripheral.
     */
    TWCR = (1 << TWEN);
}


uint8_t I2C_Start(void)
{
    /*
     * Generate START condition.
     *
     * TWINT = 1
     * Clear interrupt flag.
     *
     * TWSTA = 1
     * Request START condition.
     *
     * TWEN = 1
     * Enable TWI.
     */
    TWCR = (1 << TWINT) |
           (1 << TWSTA) |
           (1 << TWEN);

    /*
     * Wait until START condition has been transmitted.
     */
    while (!(TWCR & (1 << TWINT)))
    {
    }

    /*
     * Check TWI status.
     *
     * 0x08 = START transmitted
     * 0x10 = Repeated START transmitted
     */
    if ((TWSR & TWI_STATUS_MASK) == 0x08 ||
        (TWSR & TWI_STATUS_MASK) == 0x10)
    {
        return I2C_OK;
    }

    return I2C_START_ERROR;
}


void I2C_Stop(void)
{
    /*
     * Generate STOP condition.
     *
     * TWINT = 1
     * Clear interrupt flag.
     *
     * TWSTO = 1
     * Generate STOP condition.
     *
     * TWEN = 1
     * Enable TWI.
     */
    TWCR = (1 << TWINT) |
           (1 << TWSTO) |
           (1 << TWEN);
}


uint8_t I2C_SendAddress(uint8_t address, uint8_t read)
{
    /*
     * Convert 7-bit address to 8-bit
     * address byte.
     *
     * Bit 0:
     *     0 = Write
     *     1 = Read
     */
    TWDR = (address << 1) | (read & 0x01);

    /*
     * Start transmission of address.
     */
    TWCR = (1 << TWINT) |
           (1 << TWEN);

    /*
     * Wait until address transmission
     * has completed.
     */
    while (!(TWCR & (1 << TWINT)))
    {
    }

    /*
     * Check response from slave.
     *
     * 0x18 = SLA+W transmitted and ACK received
     * 0x40 = SLA+R transmitted and ACK received
     */
    if (!read)
    {
        if ((TWSR & TWI_STATUS_MASK) == 0x18)
        {
            return I2C_OK;
        }
    }
    else
    {
        if ((TWSR & TWI_STATUS_MASK) == 0x40)
        {
            return I2C_OK;
        }
    }

    return I2C_ADDRESS_ERROR;
}


uint8_t I2C_WriteByte(uint8_t data)
{
    /*
     * Load data into TWI data register.
     */
    TWDR = data;

    /*
     * Start data transmission.
     */
    TWCR = (1 << TWINT) |
           (1 << TWEN);

    /*
     * Wait until transmission is complete.
     */
    while (!(TWCR & (1 << TWINT)))
    {
    }

    /*
     * 0x28 = Data transmitted and ACK received.
     */
    if ((TWSR & TWI_STATUS_MASK) == 0x28)
    {
        return I2C_OK;
    }

    return I2C_WRITE_ERROR;
}


uint8_t I2C_ReadByte(uint8_t *data, uint8_t ack)
{
    /*
     * Receive byte.
     *
     * TWEA = 1:
     * Send ACK after receiving byte.
     *
     * TWEA = 0:
     * Send NACK after receiving byte.
     */
    if (ack)
    {
        TWCR = (1 << TWINT) |
               (1 << TWEA) |
               (1 << TWEN);
    }
    else
    {
        TWCR = (1 << TWINT) |
               (1 << TWEN);
    }

    /*
     * Wait until reception is complete.
     */
    while (!(TWCR & (1 << TWINT)))
    {
    }

    /*
     * Check TWI status.
     *
     * 0x50 = Data received and ACK returned.
     *
     * 0x58 = Data received and NACK returned.
     */
    if (ack)
    {
        if ((TWSR & TWI_STATUS_MASK) == 0x50)
        {
            *data = TWDR;
            return I2C_OK;
        }
    }
    else
    {
        if ((TWSR & TWI_STATUS_MASK) == 0x58)
        {
            *data = TWDR;
            return I2C_OK;
        }
    }

    return I2C_READ_ERROR;
}