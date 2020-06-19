#ifndef ADC_H
#define ADC_H

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdlib.h>

//ADC Channels
#define ADC_MODE 0    // Mode selection on the reset pin
#define ADC_PITCH 2   // Aka pitch
#define ADC_WS1 3     // Aka timbre
#define ADC_WS2 1     // Aka waveshape

//Read values
extern volatile uint8_t analogValues[4];

void adc_init();
void adc_startConversion();
void adc_initialConversion();
bool adc_isConversionFinished();  // Not used
bool adc_isConversionRunning();   // Not used
uint16_t adc_getConversionResult();
void adc_connectChannel(uint8_t channel);

#endif
