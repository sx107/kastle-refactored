#ifndef ADC_H
#define ADC_H

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdlib.h>

// ADC Channels
#define ADC_MODE 0    // Mode selection on the reset pin
#define ADC_RATE 2

// Read values
extern volatile uint16_t analogValues[4];
extern volatile uint8_t atomicLock[4];

// ADC functions
void adc_init();
void adc_startConversion();
void adc_initialConversion();
uint16_t adc_getConversionResult();
void adc_connectChannel(uint8_t channel);

#endif
