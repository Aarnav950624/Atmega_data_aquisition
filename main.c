#include <avr/io.h>
#include <stdint.h>
#include "config.h"
#include "uart.h"
#include "adc.h"


int main(void)
{
    uint16_t adc_value;

    /* Initialize peripherals */
    UART_Init();
    ADC_Init();

    UART_SendString("Multiprotocol Data Acquisition System\r\n");
    UART_SendString("System Initialized\r\n");

    while (1)
    {
        /* Read potentiometer connected to ADC0 */
        adc_value = ADC_Read(0);

        /* Send ADC value through UART */
        UART_SendString("ADC Value: ");
        UART_SendNumber(adc_value);
        UART_SendString("\r\n");
    }

    return 0;
}