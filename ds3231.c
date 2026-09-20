#include "ds3231.h"
#include "i2c.h"


/*
 * DS3231 register addresses
 */
#define DS3231_REG_SECONDS   0x00
#define DS3231_REG_MINUTES   0x01
#define DS3231_REG_HOURS     0x02
#define DS3231_REG_DAY       0x03
#define DS3231_REG_DATE      0x04
#define DS3231_REG_MONTH     0x05
#define DS3231_REG_YEAR      0x06


/*
 * Convert BCD to decimal
 */
static uint8_t BCD_To_Decimal(uint8_t bcd)
{
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}


/*
 * Convert decimal to BCD
 */
static uint8_t Decimal_To_BCD(uint8_t decimal)
{
    return ((decimal / 10) << 4) | (decimal % 10);
}


/*
 * Set DS3231 time and date
 */
uint8_t DS3231_SetTime(DS3231_Time *time)
{
    uint8_t status;

    /*
     * Generate START condition
     */
    status = I2C_Start();

    if (status != I2C_OK)
    {
        return status;
    }


    /*
     * Send DS3231 address + WRITE
     */
    status = I2C_SendAddress(DS3231_ADDRESS, 0);

    if (status != I2C_OK)
    {
        I2C_Stop();
        return status;
    }


    /*
     * Start writing from seconds register
     */
    status = I2C_WriteByte(DS3231_REG_SECONDS);

    if (status != I2C_OK)
    {
        I2C_Stop();
        return status;
    }


    /*
     * Write seconds
     */
    status = I2C_WriteByte(
        Decimal_To_BCD(time->seconds)
    );

    if (status != I2C_OK)
    {
        I2C_Stop();
        return status;
    }


    /*
     * Write minutes
     */
    status = I2C_WriteByte(
        Decimal_To_BCD(time->minutes)
    );

    if (status != I2C_OK)
    {
        I2C_Stop();
        return status;
    }


    /*
     * Write hours
     */
    status = I2C_WriteByte(
        Decimal_To_BCD(time->hours)
    );

    if (status != I2C_OK)
    {
        I2C_Stop();
        return status;
    }


    /*
     * Write day of week
     */
    status = I2C_WriteByte(
        Decimal_To_BCD(time->day)
    );

    if (status != I2C_OK)
    {
        I2C_Stop();
        return status;
    }


    /*
     * Write date
     */
    status = I2C_WriteByte(
        Decimal_To_BCD(time->date)
    );

    if (status != I2C_OK)
    {
        I2C_Stop();
        return status;
    }


    /*
     * Write month
     */
    status = I2C_WriteByte(
        Decimal_To_BCD(time->month)
    );

    if (status != I2C_OK)
    {
        I2C_Stop();
        return status;
    }


    /*
     * Write year
     */
    status = I2C_WriteByte(
        Decimal_To_BCD(time->year)
    );

    if (status != I2C_OK)
    {
        I2C_Stop();
        return status;
    }


    /*
     * Generate STOP condition
     */
    I2C_Stop();

    return I2C_OK;
}


/*
 * Read DS3231 time and date
 */
uint8_t DS3231_GetTime(DS3231_Time *time)
{
    uint8_t status;
    uint8_t data;


    /*
     * Generate START condition
     */
    status = I2C_Start();

    if (status != I2C_OK)
    {
        return status;
    }


    /*
     * Send DS3231 address + WRITE
     *
     * We do this because we first need
     * to tell the DS3231 which register
     * we want to read.
     */
    status = I2C_SendAddress(DS3231_ADDRESS, 0);

    if (status != I2C_OK)
    {
        I2C_Stop();
        return status;
    }


    /*
     * Select seconds register (0x00)
     */
    status = I2C_WriteByte(DS3231_REG_SECONDS);

    if (status != I2C_OK)
    {
        I2C_Stop();
        return status;
    }


    /*
     * Generate repeated START
     */
    status = I2C_Start();

    if (status != I2C_OK)
    {
        I2C_Stop();
        return status;
    }


    /*
     * Send DS3231 address + READ
     */
    status = I2C_SendAddress(DS3231_ADDRESS, 1);

    if (status != I2C_OK)
    {
        I2C_Stop();
        return status;
    }


    /*
     * Read seconds
     *
     * ACK because more data follows.
     */
    status = I2C_ReadByte(&data, 1);

    if (status != I2C_OK)
    {
        I2C_Stop();
        return status;
    }

    time->seconds = BCD_To_Decimal(data);


    /*
     * Read minutes
     */
    status = I2C_ReadByte(&data, 1);

    if (status != I2C_OK)
    {
        I2C_Stop();
        return status;
    }

    time->minutes = BCD_To_Decimal(data);


    /*
     * Read hours
     */
    status = I2C_ReadByte(&data, 1);

    if (status != I2C_OK)
    {
        I2C_Stop();
        return status;
    }

    time->hours = BCD_To_Decimal(data);


    /*
     * Read day
     */
    status = I2C_ReadByte(&data, 1);

    if (status != I2C_OK)
    {
        I2C_Stop();
        return status;
    }

    time->day = BCD_To_Decimal(data);


    /*
     * Read date
     */
    status = I2C_ReadByte(&data, 1);

    if (status != I2C_OK)
    {
        I2C_Stop();
        return status;
    }

    time->date = BCD_To_Decimal(data);


    /*
     * Read month
     */
    status = I2C_ReadByte(&data, 1);

    if (status != I2C_OK)
    {
        I2C_Stop();
        return status;
    }

    time->month = BCD_To_Decimal(data);


    /*
     * Read year
     *
     * NACK because this is the final byte.
     */
    status = I2C_ReadByte(&data, 0);

    if (status != I2C_OK)
    {
        I2C_Stop();
        return status;
    }

    time->year = BCD_To_Decimal(data);


    /*
     * Finish I2C transaction
     */
    I2C_Stop();

    return I2C_OK;
}