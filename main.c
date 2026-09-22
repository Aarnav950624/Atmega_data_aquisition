#include <avr/io.h>
#include <stdint.h>

#include "config.h"
#include "uart.h"
#include "adc.h"
#include "i2c.h"
#include "ds3231.h"
#include "spi.h"
#include "bmp280.h"


/*
 * Simple software delay.
 *
 * This will later be replaced by
 * a hardware timer.
 */
static void delay_ms(uint16_t ms)
{
    volatile uint32_t count;

    while (ms--)
    {
        count = F_CPU / 4000UL;

        while (count--)
        {
            __asm__ __volatile__("nop");
        }
    }
}


int main(void)
{
    uint16_t adc_value;

    DS3231_Time rtc_time;
    uint8_t rtc_status;

    int32_t temperature;
    uint32_t pressure;

    uint8_t bmp280_status;


    /*
     * Initialize peripherals.
     */
    UART_Init();

    ADC_Init();

    I2C_Init();

    SPI_Init();


    /*
     * Allow peripherals to stabilize.
     */
    delay_ms(100);


    /*
     * Startup message.
     */
    UART_SendString("\r\n");
    UART_SendString("ATmega16 Data Acquisition System\r\n");
    UART_SendString("--------------------------------\r\n");


    /*
     * Initialize DS3231 RTC.
     */
    rtc_time.seconds = 0;
    rtc_time.minutes = 45;
    rtc_time.hours   = 14;

    rtc_time.day      = 7;
    rtc_time.date     = 20;
    rtc_time.month    = 9;
    rtc_time.year     = 26;


    rtc_status =
        DS3231_SetTime(&rtc_time);


    if (rtc_status == I2C_OK)
    {
        UART_SendString(
            "RTC initialized successfully\r\n"
        );
    }
    else
    {
        UART_SendString(
            "RTC initialization FAILED\r\n"
        );
    }


    /*
     * Initialize BMP280.
     */
    bmp280_status =
        BMP280_Init();


    if (bmp280_status == 0)
    {
        UART_SendString(
            "BMP280 initialized successfully\r\n"
        );
    }
    else
    {
        UART_SendString(
            "BMP280 initialization FAILED\r\n"
        );
    }


    UART_SendString("\r\n");


    /*
     * Main data acquisition loop.
     */
    while (1)
    {
        /*
         * -------------------------
         * DS3231
         * -------------------------
         */
        rtc_status =
            DS3231_GetTime(&rtc_time);


        if (rtc_status == I2C_OK)
        {
            UART_SendString("DATE: ");

            UART_SendNumber(rtc_time.date);
            UART_Transmit('/');

            UART_SendNumber(rtc_time.month);
            UART_Transmit('/');

            UART_SendNumber(rtc_time.year);

            UART_SendString("\r\n");


            UART_SendString("TIME: ");

            UART_SendNumber(rtc_time.hours);
            UART_Transmit(':');

            UART_SendNumber(rtc_time.minutes);
            UART_Transmit(':');

            UART_SendNumber(rtc_time.seconds);

            UART_SendString("\r\n");
        }
        else
        {
            UART_SendString(
                "RTC READ ERROR\r\n"
            );
        }


        /*
         * -------------------------
         * Potentiometer ADC
         * -------------------------
         */
        adc_value =
            ADC_Read(0);


        UART_SendString("ADC: ");

        UART_SendNumber(adc_value);

        UART_SendString("\r\n");


        /*
         * -------------------------
         * BMP280
         * -------------------------
         */
        if (bmp280_status == 0)
        {
            if (BMP280_ReadData(
                    &temperature,
                    &pressure) == 0)
            {
                UART_SendString(
                    "TEMPERATURE: "
                );

                /*
                 * Temperature is stored
                 * as hundredths of °C.
                 *
                 * Example:
                 * 2534 = 25.34 °C
                 */
                if (temperature < 0)
                {
                    UART_Transmit('-');
                    temperature = -temperature;
                }

                UART_SendNumber(
                    (uint16_t)(temperature / 100)
                );

                UART_Transmit('.');

                UART_SendNumber(
                    (uint16_t)(temperature % 100)
                );

                UART_SendString(
                    " C\r\n"
                );


                UART_SendString(
                    "PRESSURE: "
                );

                UART_SendNumber(
                    (uint16_t)pressure
                );

                UART_SendString(
                    " Pa\r\n"
                );
            }
            else
            {
                UART_SendString(
                    "BMP280 READ ERROR\r\n"
                );
            }
        }


        UART_SendString("\r\n");


        /*
         * Wait approximately one second.
         */
        delay_ms(1000);
    }
}