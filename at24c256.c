#include <stdint.h>

#include "at24c256.h"
#include "i2c.h"


/*
 * Maximum EEPROM address.
 *
 * 32 KB = addresses:
 *
 * 0x0000 to 0x7FFF
 */
#define AT24C256_MAX_ADDRESS 0x7FFF


/*
 * Wait until the EEPROM has completed
 * its internal write cycle.
 *
 * After a write, the EEPROM temporarily
 * does not acknowledge its address while
 * programming the data internally.
 *
 * We repeatedly attempt an I2C START
 * followed by the EEPROM address.
 */
static uint8_t AT24C256_WaitForWriteComplete(void)
{
    uint8_t status;


    while (1)
    {
        /*
         * Try to generate START.
         */
        status = I2C_Start();

        if (status != I2C_OK)
        {
            continue;
        }


        /*
         * Try to address EEPROM in WRITE mode.
         */
        status =
            I2C_SendAddress(
                AT24C256_ADDRESS,
                0
            );


        /*
         * EEPROM acknowledged.
         *
         * Therefore its internal write
         * cycle has completed.
         */
        if (status == I2C_OK)
        {
            I2C_Stop();

            return 0;
        }


        /*
         * EEPROM has not finished yet.
         */
        I2C_Stop();
    }
}


/*
 * Initialize/check AT24C256.
 */
uint8_t AT24C256_Init(void)
{
    uint8_t status;


    /*
     * Generate START.
     */
    status = I2C_Start();

    if (status != I2C_OK)
    {
        return 1;
    }


    /*
     * Send EEPROM address in WRITE mode.
     */
    status =
        I2C_SendAddress(
            AT24C256_ADDRESS,
            0
        );


    /*
     * Finish transaction.
     */
    I2C_Stop();


    if (status != I2C_OK)
    {
        return 1;
    }


    return 0;
}


/*
 * Write one byte.
 */
uint8_t AT24C256_WriteByte(uint16_t address,
                           uint8_t data)
{
    uint8_t status;


    /*
     * Check memory address.
     *
     * AT24C256 addresses:
     *
     * 0x0000 - 0x7FFF
     */
    if (address > AT24C256_MAX_ADDRESS)
    {
        return 1;
    }


    /*
     * Generate START.
     */
    status = I2C_Start();

    if (status != I2C_OK)
    {
        return 1;
    }


    /*
     * Address EEPROM in WRITE mode.
     */
    status =
        I2C_SendAddress(
            AT24C256_ADDRESS,
            0
        );

    if (status != I2C_OK)
    {
        I2C_Stop();
        return 1;
    }


    /*
     * Send high byte of
     * 16-bit memory address.
     */
    status =
        I2C_WriteByte(
            (uint8_t)(address >> 8)
        );

    if (status != I2C_OK)
    {
        I2C_Stop();
        return 1;
    }


    /*
     * Send low byte of
     * 16-bit memory address.
     */
    status =
        I2C_WriteByte(
            (uint8_t)(address & 0xFF)
        );

    if (status != I2C_OK)
    {
        I2C_Stop();
        return 1;
    }


    /*
     * Send data byte.
     */
    status =
        I2C_WriteByte(data);

    if (status != I2C_OK)
    {
        I2C_Stop();
        return 1;
    }


    /*
     * End write transaction.
     */
    I2C_Stop();


    /*
     * Wait for EEPROM internal
     * write cycle to complete.
     */
    return AT24C256_WaitForWriteComplete();
}


/*
 * Read one byte.
 */
uint8_t AT24C256_ReadByte(uint16_t address,
                          uint8_t *data)
{
    uint8_t status;


    /*
     * Check address.
     */
    if (address > AT24C256_MAX_ADDRESS)
    {
        return 1;
    }


    /*
     * Check data pointer.
     */
    if (data == 0)
    {
        return 1;
    }


    /*
     * START.
     */
    status = I2C_Start();

    if (status != I2C_OK)
    {
        return 1;
    }


    /*
     * Address EEPROM in WRITE mode.
     *
     * We first have to tell the EEPROM
     * which memory address we want.
     */
    status =
        I2C_SendAddress(
            AT24C256_ADDRESS,
            0
        );

    if (status != I2C_OK)
    {
        I2C_Stop();
        return 1;
    }


    /*
     * Send high byte of memory address.
     */
    status =
        I2C_WriteByte(
            (uint8_t)(address >> 8)
        );

    if (status != I2C_OK)
    {
        I2C_Stop();
        return 1;
    }


    /*
     * Send low byte of memory address.
     */
    status =
        I2C_WriteByte(
            (uint8_t)(address & 0xFF)
        );

    if (status != I2C_OK)
    {
        I2C_Stop();
        return 1;
    }


    /*
     * Repeated START.
     *
     * Now switch from WRITE to READ.
     */
    status = I2C_Start();

    if (status != I2C_OK)
    {
        I2C_Stop();
        return 1;
    }


    /*
     * Address EEPROM in READ mode.
     */
    status =
        I2C_SendAddress(
            AT24C256_ADDRESS,
            1
        );

    if (status != I2C_OK)
    {
        I2C_Stop();
        return 1;
    }


    /*
     * Read one byte.
     *
     * NACK because this is the
     * final byte of the transaction.
     */
    status =
        I2C_ReadByte(
            data,
            0
        );

    if (status != I2C_OK)
    {
        I2C_Stop();
        return 1;
    }


    /*
     * Finish transaction.
     */
    I2C_Stop();

    return 0;
}


/*
 * Write multiple bytes.
 */
uint8_t AT24C256_Write(uint16_t address,
                       const uint8_t *data,
                       uint16_t length)
{
    uint16_t remaining;
    uint16_t page_space;
    uint16_t chunk;
    uint16_t i;

    uint8_t status;


    /*
     * Check parameters.
     */
    if (data == 0)
    {
        return 1;
    }

    if (length == 0)
    {
        return 0;
    }


    /*
     * Check address range.
     */
    if (address > AT24C256_MAX_ADDRESS)
    {
        return 1;
    }


    /*
     * Make sure the entire
     * operation fits inside EEPROM.
     */
    if ((uint32_t)address + length >
        AT24C256_SIZE)
    {
        return 1;
    }


    remaining = length;


    while (remaining > 0)
    {
        /*
         * Determine how many bytes remain
         * in the current 64-byte page.
         *
         * Example:
         *
         * address = 0x003E
         *
         * Page:
         *
         * 0x0000 - 0x003F
         *
         * Only 2 bytes remain.
         */
        page_space =
            AT24C256_PAGE_SIZE -
            (address % AT24C256_PAGE_SIZE);


        /*
         * Write only as much as can fit
         * into the current page.
         */
        if (remaining < page_space)
        {
            chunk = remaining;
        }
        else
        {
            chunk = page_space;
        }


        /*
         * Start EEPROM write transaction.
         */
        status = I2C_Start();

        if (status != I2C_OK)
        {
            return 1;
        }


        /*
         * EEPROM WRITE address.
         */
        status =
            I2C_SendAddress(
                AT24C256_ADDRESS,
                0
            );

        if (status != I2C_OK)
        {
            I2C_Stop();
            return 1;
        }


        /*
         * Send 16-bit memory address.
         */
        status =
            I2C_WriteByte(
                (uint8_t)(address >> 8)
            );

        if (status != I2C_OK)
        {
            I2C_Stop();
            return 1;
        }


        status =
            I2C_WriteByte(
                (uint8_t)(address & 0xFF)
            );

        if (status != I2C_OK)
        {
            I2C_Stop();
            return 1;
        }


        /*
         * Send page data.
         */
        for (i = 0; i < chunk; i++)
        {
            status =
                I2C_WriteByte(
                    data[i]
                );

            if (status != I2C_OK)
            {
                I2C_Stop();
                return 1;
            }
        }


        /*
         * Finish page write.
         */
        I2C_Stop();


        /*
         * Wait until EEPROM finishes
         * programming this page.
         */
        if (AT24C256_WaitForWriteComplete() != 0)
        {
            return 1;
        }


        /*
         * Move to next EEPROM address.
         */
        address += chunk;


        /*
         * Move data pointer.
         */
        data += chunk;


        /*
         * Reduce remaining data.
         */
        remaining -= chunk;
    }


    return 0;
}


/*
 * Read multiple bytes.
 */
uint8_t AT24C256_Read(uint16_t address,
                      uint8_t *data,
                      uint16_t length)
{
    uint16_t i;

    uint8_t status;


    /*
     * Check parameters.
     */
    if (data == 0)
    {
        return 1;
    }

    if (length == 0)
    {
        return 0;
    }


    /*
     * Check address range.
     */
    if (address > AT24C256_MAX_ADDRESS)
    {
        return 1;
    }


    /*
     * Make sure read fits inside EEPROM.
     */
    if ((uint32_t)address + length >
        AT24C256_SIZE)
    {
        return 1;
    }


    /*
     * START.
     */
    status = I2C_Start();

    if (status != I2C_OK)
    {
        return 1;
    }


    /*
     * EEPROM WRITE address.
     *
     * We first send the memory address
     * that we want to read from.
     */
    status =
        I2C_SendAddress(
            AT24C256_ADDRESS,
            0
        );

    if (status != I2C_OK)
    {
        I2C_Stop();
        return 1;
    }


    /*
     * Send high memory address byte.
     */
    status =
        I2C_WriteByte(
            (uint8_t)(address >> 8)
        );

    if (status != I2C_OK)
    {
        I2C_Stop();
        return 1;
    }


    /*
     * Send low memory address byte.
     */
    status =
        I2C_WriteByte(
            (uint8_t)(address & 0xFF)
        );

    if (status != I2C_OK)
    {
        I2C_Stop();
        return 1;
    }


    /*
     * Repeated START.
     */
    status = I2C_Start();

    if (status != I2C_OK)
    {
        I2C_Stop();
        return 1;
    }


    /*
     * EEPROM READ address.
     */
    status =
        I2C_SendAddress(
            AT24C256_ADDRESS,
            1
        );

    if (status != I2C_OK)
    {
        I2C_Stop();
        return 1;
    }


    /*
     * Read requested number of bytes.
     */
    for (i = 0; i < length; i++)
    {
        /*
         * ACK every byte except
         * the final byte.
         */
        if (i < (length - 1))
        {
            status =
                I2C_ReadByte(
                    &data[i],
                    1
                );
        }
        else
        {
            status =
                I2C_ReadByte(
                    &data[i],
                    0
                );
        }


        if (status != I2C_OK)
        {
            I2C_Stop();
            return 1;
        }
    }


    /*
     * End transaction.
     */
    I2C_Stop();

    return 0;
}