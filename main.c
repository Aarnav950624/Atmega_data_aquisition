#include "uart.h"

int main(void)
{
    UART_Init();

    while (1)
    {
        UART_SendString("Multiprotocol Data Acquisition System\r\n");
    }

    return 0;
}