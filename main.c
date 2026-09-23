/*
 * ============================================================
 * ATmega16 Multiprotocol Embedded Data Acquisition System
 * ============================================================
 *
 * Peripherals:
 *
 *   ADC       -> Potentiometer
 *   I2C/TWI   -> DS3231 RTC
 *   I2C/TWI   -> AT24C256 EEPROM
 *   SPI       -> BMP280
 *   UART      -> PC / Serial Terminal
 *
 * Bare-metal C implementation
 * ============================================================
 */

#include <avr/io.h>
#include <stdint.h>

#include "config.h"
#include "uart.h"
#include "adc.h"
#include "i2c.h"
#include "spi.h"
#include "ds3231.h"
#include "bmp280.h"
#include "at24c256.h"


/* ============================================================
 * EEPROM CONFIGURATION
 * ============================================================
 *
 * EEPROM record:
 *
 * RTC:
 *   seconds  -> 1 byte
 *   minutes  -> 1 byte
 *   hours    -> 1 byte
 *   day      -> 1 byte
 *   date     -> 1 byte
 *   month    -> 1 byte
 *   year     -> 1 byte
 *
 * BMP280:
 *   temperature -> 4 bytes
 *   pressure    -> 4 bytes
 *
 * ADC:
 *   ADC value   -> 2 bytes
 *
 * Total = 17 bytes
 * ============================================================
 */

#define RECORD_SIZE          17U
#define EEPROM_START_ADDRESS 0x0000U


/* ============================================================
 * DATA RECORD STRUCTURE
 * ============================================================
 */

typedef struct
{
    DS3231_Time rtc;

    int32_t temperature;

    uint32_t pressure;

    uint16_t adc_value;

} DataRecord;


/* ============================================================
 * GLOBAL DATA RECORD
 * ============================================================
 */

static DataRecord record;


/* ============================================================
 * SIMPLE DELAY FUNCTION
 * ============================================================
 */

static void Delay_ms(uint16_t milliseconds)
{
    uint16_t i;
    uint16_t j;

    for (i = 0; i < milliseconds; i++)
    {
        for (j = 0; j < 1000; j++)
        {
            asm volatile ("nop");
        }
    }
}


/* ============================================================
 * SEND SIGNED 32-BIT NUMBER THROUGH UART
 * ============================================================
 */

static void UART_SendSigned32(int32_t number)
{
    uint32_t magnitude;

    if (number < 0)
    {
        UART_Transmit('-');

        magnitude = (uint32_t)(-number);
    }
    else
    {
        magnitude = (uint32_t)number;
    }

    UART_SendNumber32(magnitude);
}


/* ============================================================
 * SEND BMP280 TEMPERATURE THROUGH UART
 *
 * BMP280 temperature is in hundredths of degree Celsius.
 *
 * Example:
 *
 * 2534 -> 25.34 C
 * ============================================================
 */

static void UART_SendTemperature(int32_t temperature)
{
    int32_t integer_part;
    uint16_t fractional_part;

    if (temperature < 0)
    {
        UART_Transmit('-');

        temperature = -temperature;
    }

    integer_part = temperature / 100;

    fractional_part = (uint16_t)(temperature % 100);

    UART_SendNumber32((uint32_t)integer_part);

    UART_Transmit('.');

    if (fractional_part < 10)
    {
        UART_Transmit('0');
    }

    UART_SendNumber(fractional_part);

    UART_SendString(" C");
}


/* ============================================================
 * PACK 16-BIT VALUE INTO BYTE ARRAY
 * ============================================================
 */

static void Pack_Uint16(uint16_t value, uint8_t *buffer)
{
    buffer[0] = (uint8_t)(value >> 8);
    buffer[1] = (uint8_t)(value & 0xFF);
}


/* ============================================================
 * PACK 32-BIT VALUE INTO BYTE ARRAY
 * ============================================================
 */

static void Pack_Uint32(uint32_t value, uint8_t *buffer)
{
    buffer[0] = (uint8_t)(value >> 24);
    buffer[1] = (uint8_t)(value >> 16);
    buffer[2] = (uint8_t)(value >> 8);
    buffer[3] = (uint8_t)(value & 0xFF);
}


/* ============================================================
 * PACK SIGNED 32-BIT VALUE INTO BYTE ARRAY
 * ============================================================
 */

static void Pack_Int32(int32_t value, uint8_t *buffer)
{
    Pack_Uint32((uint32_t)value, buffer);
}


/* ============================================================
 * PACK COMPLETE DATA RECORD
 *
 * EEPROM RECORD FORMAT:
 *
 * Byte 0   : RTC seconds
 * Byte 1   : RTC minutes
 * Byte 2   : RTC hours
 * Byte 3   : RTC day
 * Byte 4   : RTC date
 * Byte 5   : RTC month
 * Byte 6   : RTC year
 *
 * Byte 7   : Temperature MSB
 * Byte 8   : Temperature
 * Byte 9   : Temperature
 * Byte 10  : Temperature LSB
 *
 * Byte 11  : Pressure MSB
 * Byte 12  : Pressure
 * Byte 13  : Pressure
 * Byte 14  : Pressure LSB
 *
 * Byte 15  : ADC MSB
 * Byte 16  : ADC LSB
 *
 * Total = 17 bytes
 * ============================================================
 */

static void Pack_Record(DataRecord *data, uint8_t *buffer)
{
    /* RTC */

    buffer[0] = data->rtc.seconds;
    buffer[1] = data->rtc.minutes;
    buffer[2] = data->rtc.hours;

    buffer[3] = data->rtc.day;
    buffer[4] = data->rtc.date;
    buffer[5] = data->rtc.month;
    buffer[6] = data->rtc.year;


    /* Temperature */

    Pack_Int32(data->temperature, &buffer[7]);


    /* Pressure */

    Pack_Uint32(data->pressure, &buffer[11]);


    /* ADC */

    Pack_Uint16(data->adc_value, &buffer[15]);
}


/* ============================================================
 * MAIN
 * ============================================================
 */

int main(void)
{
    uint8_t eeprom_buffer[RECORD_SIZE];

    uint8_t status;

    uint8_t eeprom_test_write = 0x55;
    uint8_t eeprom_test_read = 0x00;

    uint16_t eeprom_address = EEPROM_START_ADDRESS;


    /* ========================================================
     * INITIALIZE PERIPHERALS
     * ========================================================
     */

    UART_Init();

    UART_SendString("\r\n");
    UART_SendString("========================================\r\n");
    UART_SendString(" ATmega16 DATA ACQUISITION SYSTEM\r\n");
    UART_SendString("========================================\r\n");

    UART_SendString("Initializing peripherals...\r\n");


    /* ADC */

    ADC_Init();

    UART_SendString("ADC       : OK\r\n");


    /* I2C */

    I2C_Init();

    UART_SendString("I2C/TWI   : OK\r\n");


    /* SPI */

    SPI_Init();

    UART_SendString("SPI       : OK\r\n");


    /* ========================================================
     * INITIALIZE BMP280
     * ========================================================
     */

    status = BMP280_Init();

    if (status == 0)
    {
        UART_SendString("BMP280    : OK\r\n");
    }
    else
    {
        UART_SendString("BMP280    : ERROR\r\n");
    }


    /* ========================================================
     * INITIALIZE AT24C256
     * ========================================================
     */

    status = AT24C256_Init();

    if (status == 0)
    {
        UART_SendString("AT24C256  : OK\r\n");
    }
    else
    {
        UART_SendString("AT24C256  : ERROR\r\n");
    }


    /* ========================================================
     * EEPROM BASIC TEST
     * ========================================================
     */

    UART_SendString("EEPROM test...\r\n");


    /* Write test byte */

    status = AT24C256_WriteByte(
        0x0000,
        eeprom_test_write
    );

    if (status == 0)
    {
        UART_SendString("EEPROM write: OK\r\n");
    }
    else
    {
        UART_SendString("EEPROM write: ERROR\r\n");
    }


    /* Wait for EEPROM internal write cycle */

    Delay_ms(10);


    /* Read test byte */

    status = AT24C256_ReadByte(
        0x0000,
        &eeprom_test_read
    );

    if ((status == 0) &&
        (eeprom_test_read == eeprom_test_write))
    {
        UART_SendString("EEPROM read : OK\r\n");
    }
    else
    {
        UART_SendString("EEPROM read : ERROR\r\n");
    }


    UART_SendString("----------------------------------------\r\n");
    UART_SendString("System ready.\r\n");
    UART_SendString("----------------------------------------\r\n");


    /* ========================================================
     * MAIN DATA ACQUISITION LOOP
     * ========================================================
     */

    while (1)
    {

        /* ====================================================
         * READ RTC
         * ====================================================
         */

        status = DS3231_GetTime(&record.rtc);

        if (status != 0)
        {
            UART_SendString("RTC READ ERROR\r\n");
        }


        /* ====================================================
         * READ ADC
         *
         * ADC0 = PA0
         * ====================================================
         */

        record.adc_value = ADC_Read(0);


        /* ====================================================
         * READ BMP280
         * ====================================================
         *
         * Temperature:
         *     hundredths of degree Celsius
         *
         * Pressure:
         *     Pa
         *
         * Pressure is stored as uint32_t.
         * ====================================================
         */

        status = BMP280_ReadData(
            &record.temperature,
            &record.pressure
        );

        if (status != 0)
        {
            UART_SendString("BMP280 READ ERROR\r\n");
        }


        /* ====================================================
         * DISPLAY RTC DATA
         * ====================================================
         */

        UART_SendString("\r\n");

        UART_SendString("TIME: ");

        UART_SendNumber(record.rtc.hours);

        UART_Transmit(':');

        UART_SendNumber(record.rtc.minutes);

        UART_Transmit(':');

        UART_SendNumber(record.rtc.seconds);

        UART_SendString("\r\n");


        /* ====================================================
         * DISPLAY DATE
         * ====================================================
         */

        UART_SendString("DATE: ");

        UART_SendNumber(record.rtc.date);

        UART_Transmit('/');

        UART_SendNumber(record.rtc.month);

        UART_Transmit('/');

        UART_SendNumber(record.rtc.year);

        UART_SendString("\r\n");


        /* ====================================================
         * DISPLAY TEMPERATURE
         * ====================================================
         */

        UART_SendString("TEMPERATURE: ");

        UART_SendTemperature(record.temperature);

        UART_SendString("\r\n");


        /* ====================================================
         * DISPLAY PRESSURE
         * ====================================================
         */

        UART_SendString("PRESSURE: ");

        UART_SendNumber32(record.pressure);

        UART_SendString(" Pa\r\n");


        /* ====================================================
         * DISPLAY ADC
         * ====================================================
         */

        UART_SendString("ADC: ");

        UART_SendNumber(record.adc_value);

        UART_SendString("\r\n");


        /* ====================================================
         * PACK DATA FOR EEPROM
         * ====================================================
         */

        Pack_Record(
            &record,
            eeprom_buffer
        );


        /* ====================================================
         * STORE DATA IN EEPROM
         * ====================================================
         */

        status = AT24C256_Write(
            eeprom_address,
            eeprom_buffer,
            RECORD_SIZE
        );

        if (status == 0)
        {
            UART_SendString("EEPROM: DATA STORED\r\n");

            UART_SendString("ADDRESS: ");

            UART_SendNumber(eeprom_address);

            UART_SendString("\r\n");
        }
        else
        {
            UART_SendString("EEPROM: WRITE ERROR\r\n");
        }


        /* ====================================================
         * MOVE TO NEXT EEPROM RECORD
         * ====================================================
         */

        eeprom_address += RECORD_SIZE;


        /* ====================================================
         * CHECK EEPROM MEMORY LIMIT
         * ====================================================
         */

        if (eeprom_address >
            (AT24C256_SIZE - RECORD_SIZE))
        {
            eeprom_address = EEPROM_START_ADDRESS;

            UART_SendString(
                "EEPROM MEMORY FULL - WRAPPING TO START\r\n"
            );
        }


        UART_SendString("----------------------------------------\r\n");


        /* ====================================================
         * WAIT 1 SECOND BEFORE NEXT ACQUISITION
         * ====================================================
         */

        Delay_ms(1000);
    }


    return 0;
}