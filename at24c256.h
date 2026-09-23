#ifndef AT24C256_H
#define AT24C256_H

#include <stdint.h>


/*
 * AT24C256 7-bit I2C address.
 *
 * A2 = 0
 * A1 = 0
 * A0 = 0
 *
 * Therefore:
 *
 * 1010 A2 A1 A0
 * 1010 0  0  0
 *
 * = 0x50
 */
#define AT24C256_ADDRESS 0x50


/*
 * AT24C256 memory size
 *
 * 256 Kbit = 32 KB
 */
#define AT24C256_SIZE 32768UL


/*
 * AT24C256 page size.
 *
 * One page = 64 bytes.
 */
#define AT24C256_PAGE_SIZE 64


/*
 * Initialize/check AT24C256.
 *
 * Returns:
 *
 * 0 -> EEPROM detected
 * 1 -> EEPROM not detected
 */
uint8_t AT24C256_Init(void);


/*
 * Write one byte to EEPROM.
 *
 * address:
 *     EEPROM memory address
 *
 * data:
 *     byte to be written
 */
uint8_t AT24C256_WriteByte(uint16_t address,
                           uint8_t data);


/*
 * Read one byte from EEPROM.
 *
 * address:
 *     EEPROM memory address
 *
 * data:
 *     pointer where received byte
 *     will be stored
 */
uint8_t AT24C256_ReadByte(uint16_t address,
                          uint8_t *data);


/*
 * Write multiple bytes to EEPROM.
 *
 * The function automatically handles
 * 64-byte page boundaries.
 */
uint8_t AT24C256_Write(uint16_t address,
                       const uint8_t *data,
                       uint16_t length);


/*
 * Read multiple bytes from EEPROM.
 */
uint8_t AT24C256_Read(uint16_t address,
                      uint8_t *data,
                      uint16_t length);

#endif