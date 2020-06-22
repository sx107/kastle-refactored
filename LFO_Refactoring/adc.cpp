#include "adc.h"
#include "lfo.h"
#define F_CPU 8000000UL

#include "Arduino.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdlib.h>

// "Calibration curve" look-up table
// 8-bit ADC readings are then mapped using it
// For example, 97-161 range is mapped to 100-210
#define CCURVE_NPOINTS 5
const uint8_t cCurve_from[] = {0,   30,   97,   161,  255};
const uint8_t cCurve_to[]   = {1,   30,  100,   210,  254};
uint8_t cCurve[256]; // cCurve isn't changed in the ISR, so volatile is not required here

// All of these aren't really required to be defined globally except for analogValues[]
// But they are defined globally to stick to the original code
// analogValues[] and lastAnalogValues[], though, are defined as 8-bit instead of 16-bit in the original code
// Code mostly copy-pasted from the VCO, so 4 values instead of required 2 values
volatile uint8_t analogValues[4] = {0, 0, 0, 0};
volatile uint8_t lastAnalogValues[4] = {255, 255, 255, 255};
volatile uint8_t analogChannelRead = ADC_RATE;
volatile uint8_t analogChannelReadIndex = 0;



// ADC calibration curve LUT generation

uint8_t adc_curveMap(uint8_t value) {
  uint8_t inMin = 0, inMax = 255, outMin = 0, outMax = 255;
  for(uint8_t i = 0; i < CCURVE_NPOINTS - 1; i++) {
    if(value >= cCurve_from[i] && value <= cCurve_from[i+1]) {
      inMin = cCurve_from[i];
      inMax = cCurve_from[i+1];
      outMin = cCurve_to[i];
      outMax = cCurve_to[i+1];
      break; // Original: i += 10;
    }
  }
  return map(value, inMin, inMax, outMin, outMax); // How slow is this function?
}


void adc_createLookup() {
  for (uint16_t i = 0; i < 256; i++) {
    cCurve[i] = adc_curveMap(i);
  }
}



// ADC ISR

ISR(ADC_vect) {  
  const uint8_t analogChannelSequence[6]={ADC_MODE, ADC_RATE};

  // Save previous value and read new one
  lastAnalogValues[analogChannelRead] = analogValues[analogChannelRead];
  analogValues[analogChannelRead] = (adc_getConversionResult() >> 2) & 0xFF;

  // Update the rate parameter
  if(analogChannelRead == ADC_RATE && lastAnalogValues[ADC_RATE] != analogValues[ADC_RATE]) {
    lfo_set_frequency(cCurve[analogValues[ADC_RATE]]);
  }

  // Switch to the next channel
  analogChannelReadIndex++;
  if(analogChannelReadIndex > 2) {analogChannelReadIndex = 0;} // TODO: Replace hard-coded sequence length
  analogChannelRead=analogChannelSequence[analogChannelReadIndex];

  // Next conversion
  adc_connectChannel(analogChannelRead);
  adc_startConversion();
}

// Different ADC functions: initialization, conversion running/stopping, etc.

void adc_init() {
  // ADC Clock Frequency is 8000000/128 = 62.5kHz
  // ADC Sample rate is 62.5kHz / 13.5 = 4.63 kHz
  ADMUX  = 0;
  bitWrite(ADCSRA,ADEN,1);                        // ADC enable
  ADCSRA |= bit(ADPS0) | bit(ADPS1) | bit(ADPS2); // Prescaler : 1/128
  bitWrite(ADCSRA,ADIE,1);                        // Enable conversion finished interupt

}

void adc_startConversion() {
  bitWrite(ADCSRA,ADSC,1);
}

void adc_initialConversion() {
  adc_connectChannel(analogChannelRead);
  adc_startConversion();
  delayMicroseconds(100);
}

bool adc_isConversionFinished() {
  return bitRead(ADCSRA, ADIF);
}
bool adc_isConversionRunning() {
  return !adc_isConversionFinished();
}
uint16_t adc_getConversionResult() {
  uint16_t result = ADCL;
  return result | (ADCH<<8);
}
void adc_connectChannel(uint8_t number) {
  ADMUX &= (11110000);
  ADMUX |= number;
}
