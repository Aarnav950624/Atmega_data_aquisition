/*
 * COPYRIGHT (C) 2026 Aarnav Patel
 *
 * @file adc.h
 * @brief ADC driver for the ATmega16 multiprotocol embedded data acquisition system
 *
 * provides functions for initializing and using the ADC
 *
 * @author Aarnav Patel
 * @date September 2026
 *
 */

#ifndef ADC_H
#define ADC_H

#include <stdint.h>

/* Initialize ADC peripheral */
void ADC_Init(void);

/* Read ADC value from selected channel */
uint16_t ADC_Read(uint8_t channel);

#endif