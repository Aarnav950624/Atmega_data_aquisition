#ifndef ADC_H
#define ADC_H

#include <stdint.h>

/* Initialize ADC peripheral */
void ADC_Init(void);

/* Read ADC value from selected channel */
uint16_t ADC_Read(uint8_t channel);

#endif