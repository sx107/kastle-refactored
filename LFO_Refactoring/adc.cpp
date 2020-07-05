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

const uint8_t cCurve_from[] = {0,   60,   127,   191,  255};
const uint8_t cCurve_to[]   = {50,   127,  190,   230,  254};

uint8_t cCurve[256]; // cCurve isn't changed in the ISR, so volatile is not required here

// All of these aren't really required to be defined globally except for analogValues[]
// But they are defined globally to stick to the original code
// analogValues[] and lastAnalogValues[], though, are defined as 8-bit instead of 16-bit in the original code
// Code mostly copy-pasted from the VCO, so 4 values instead of required 2 values
volatile uint8_t analogValues[4] = {0, 0, 0, 0};
volatile uint8_t lastAnalogValues[4] = {255, 255, 255, 255};
volatile uint8_t analogChannelRead = ADC_RATE;
volatile uint8_t analogChannelReadIndex = 0;

volatile bool skipConversion = false; // Every second ADC reading is skipped

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
  // const uint8_t analogToDigitalPinMapping[4]={PORTB5,PORTB2,PORTB4,PORTB3}; // Not used, see note below
  const uint8_t analogChannelSequence[2]={ADC_MODE, ADC_RATE};

  //If we need - skip every second conversion
  //#ifdef S_ADC_FIX
    if(skipConversion) {
      if(analogChannelRead != ADC_MODE) {
        // Famous last words: "I've kept the code original here, though it clearly can be optimized"
        // This is done to flush the stray voltage on the zener diodes

        // Original code:
        /*
        if(analogValues[analogChannelRead]<187) bitWrite(DDRB,analogToDigitalPinMapping[analogChannelRead],1);
        bitWrite(DDRB,analogToDigitalPinMapping[analogChannelRead],0);
        bitWrite(PORTB,analogToDigitalPinMapping[analogChannelRead],0);
        */

        // Original code does not work in my compiling setup, since the optimizer does not recognize that this code can be shortened
        // to 3 instructions, since analogChannelRead is the same every time essentially because of "if(analogChannelRead != ADC_MODE)".
        // In the original .hex it is shortened to 3 instructions, though.
        // Since this code is time-crucial, I'll just make sure that it works by writing the asm instructions required. Port is B4
        // 0x17 is DDRB, 0x18 is PORTB

        if(analogValues[analogChannelRead]<187) {
          __asm__ __volatile__ (
            "sbi 0x17, 4\n"
            "cbi 0x17, 4\n"
            "cbi 0x18, 4\n"  
          );
        }
      }
      skipConversion = !skipConversion;
      adc_startConversion();
      return;
    }
  //#endif

  // Save previous value and read new one
  lastAnalogValues[analogChannelRead] = analogValues[analogChannelRead];
  analogValues[analogChannelRead] = (adc_getConversionResult() >> 2) & 0xFF;

  // Update the rate parameter
  if(lastAnalogValues[analogChannelRead] != analogValues[analogChannelRead]) {
    if(analogChannelRead == ADC_RATE) {lfo_set_frequency(cCurve[analogValues[ADC_RATE]]);}
    if(analogChannelRead == ADC_MODE) {lfo_mode_select(analogValues[ADC_MODE]);}
  }

  // Switch to the next channel
  analogChannelReadIndex++;
  if(analogChannelReadIndex > 2) {analogChannelReadIndex = 0;} // TODO: Replace hard-coded sequence length
  analogChannelRead=analogChannelSequence[analogChannelReadIndex];

  //Switch skipConversion if we need to
  //#ifdef S_ADC_FIX
    skipConversion = !skipConversion;
  //#endif

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
