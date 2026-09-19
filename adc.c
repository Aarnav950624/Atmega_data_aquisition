#include <avr/io.h>
#include <stdint.h>

#include "adc.h"

void ADC_Init(void)
{
    /*
     * Reference voltage:
     * AVCC with external capacitor at AREF
     *
     * ADLAR = 0
     * Result is right-adjusted
     */
    ADMUX = (1 << REFS0);

    /*
     * Enable ADC
     *
     * ADC clock prescaler = 128
     */
    ADCSRA = (1 << ADEN) |
             (1 << ADPS2) |
             (1 << ADPS1) |
             (1 << ADPS0);
}


uint16_t ADC_Read(uint8_t channel)
{
    uint16_t result;

    /*
     * Select ADC channel.
     *
     * Keep the reference-voltage configuration
     * and change only MUX bits.
     */
    ADMUX = (ADMUX & 0xE0) | (channel & 0x1F);

    /*
     * Start ADC conversion
     */
    ADCSRA |= (1 << ADSC);

    /*
     * Wait until conversion is complete
     */
    while (ADCSRA & (1 << ADSC))
    {
    }

    /*
     * Read ADC result.
     *
     * ADCL must be read before ADCH.
     */
    result = ADCL;
    result |= ((uint16_t)ADCH << 8);

    return result;
}