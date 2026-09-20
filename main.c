#include <avr/io.h>
#include <stdint.h>

#include "config.h"
#include "uart.h"
#include "adc.h"
#include "i2c.h"
#include "ds3231.h"


/*
 * Simple software delay
 *
 * Used only for initial testing.
 * Later we can replace this with
 * a Timer-based delay.
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


    /*
     * --------------------------------
     * Initialize peripherals
     * --------------------------------
     */

    /* Initialize UART */
    UART_Init();

    /* Initialize ADC */
    ADC_Init();

    /* Initialize I2C/TWI */
    I2C_Init();


    /*
     * Give peripherals some time
     * after power-up.
     */
    delay_ms(100);


    /*
     * --------------------------------
     * Startup message
     * --------------------------------
     */

    UART_SendString("\r\n");
    UART_SendString("ATmega16 Data Acquisition System\r\n");
    UART_SendString("--------------------------------\r\n");


    /*
     * --------------------------------
     * Initial RTC time
     * --------------------------------
     *
     * Change these values before
     * programming the ATmega16.
     *
     * day:
     *
     * 1 = Sunday
     * 2 = Monday
     * 3 = Tuesday
     * 4 = Wednesday
     * 5 = Thursday
     * 6 = Friday
     * 7 = Saturday
     */

    rtc_time.seconds = 0;
    rtc_time.minutes = 45;
    rtc_time.hours   = 14;

    rtc_time.day     = 7;
    rtc_time.date    = 20;
    rtc_time.month   = 9;
    rtc_time.year    = 26;


    /*
     * --------------------------------
     * Write initial time to DS3231
     * --------------------------------
     */

    rtc_status = DS3231_SetTime(&rtc_time);


    /*
     * Check whether RTC communication
     * was successful.
     */

    if (rtc_status == I2C_OK)
    {
        UART_SendString("RTC initialized successfully\r\n");
    }
    else
    {
        UART_SendString("RTC initialization FAILED\r\n");
    }


    UART_SendString("\r\n");


    /*
     * --------------------------------
     * Main loop
     * --------------------------------
     */

    while (1)
    {
        /*
         * Read current time from DS3231
         */
        rtc_status = DS3231_GetTime(&rtc_time);


        /*
         * Check whether RTC read was
         * successful.
         */

        if (rtc_status == I2C_OK)
        {
            /*
             * -------------------------
             * Print DATE
             * -------------------------
             *
             * DATE: DD/MM/YY
             */

            UART_SendString("DATE: ");

            UART_SendNumber(rtc_time.date);

            UART_Transmit('/');

            UART_SendNumber(rtc_time.month);

            UART_Transmit('/');

            UART_SendNumber(rtc_time.year);

            UART_SendString("\r\n");


            /*
             * -------------------------
             * Print TIME
             * -------------------------
             *
             * TIME: HH:MM:SS
             */

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
            /*
             * RTC communication failed
             */
            UART_SendString("RTC READ ERROR\r\n");
        }


        /*
         * --------------------------------
         * Read ADC channel 0
         * --------------------------------
         */

        adc_value = ADC_Read(0);


        /*
         * Send ADC value through UART
         */

        UART_SendString("ADC: ");

        UART_SendNumber(adc_value);

        UART_SendString("\r\n");


        /*
         * Wait approximately 1 second
         */

        delay_ms(1000);


        UART_SendString("\r\n");
    }
}