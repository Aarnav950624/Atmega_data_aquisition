#ifndef I2C_H
#define I2C_H

#include <stdint.h>

/*
 * I2C/TWI status codes
 */
#define I2C_OK              0x00
#define I2C_START_ERROR     0x01
#define I2C_ADDRESS_ERROR   0x02
#define I2C_WRITE_ERROR     0x03
#define I2C_READ_ERROR      0x04

/*
 * Initialize ATmega16 TWI peripheral
 */
void I2C_Init(void);

/*
 * Generate START condition
 */
uint8_t I2C_Start(void);

/*
 * Generate STOP condition
 */
void I2C_Stop(void);

/*
 * Send 7-bit slave address with R/W bit
 *
 * address = 7-bit I2C address
 * read    = 0 -> write
 *           1 -> read
 */
uint8_t I2C_SendAddress(uint8_t address, uint8_t read);

/*
 * Write one byte to the I2C bus
 */
uint8_t I2C_WriteByte(uint8_t data);

/*
 * Read one byte from the I2C bus
 *
 * ack = 1 -> send ACK after receiving byte
 * ack = 0 -> send NACK after receiving byte
 */
uint8_t I2C_ReadByte(uint8_t *data, uint8_t ack);

#endif