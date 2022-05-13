#include "settings.h"
#include "adc.h"
#include "lfo.h"
#define F_CPU 8000000UL

#include "Arduino.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdlib.h>

volatile uint16_t analogValues[4] = {0, 0, 0, 0};
volatile uint8_t analogChannelRead = ADC_RATE;
volatile uint8_t analogChannelReadIndex = 0;
volatile uint8_t atomicLock[4] = {false, false, false, false};

#ifdef S_ADC_FIX
  volatile bool skipConversion = false; // Every second ADC reading is skipped
#endif

// ADC ISR

ISR(ADC_vect) {  
  const uint8_t analogChannelSequence[6]={ADC_MODE, ADC_RATE};
  #ifdef S_USE_MOVAVG
    static uint16_t prevAnalogValues[4] = {0, 0, 0, 0};
  #endif

  #ifdef S_ADC_FIX
    //If we need - skip every second conversion
    if(skipConversion) {
      if(analogChannelRead == ADC_RATE && analogValues[ADC_RATE] < 750) {
        // 0x17 is DDRB, 0x18 is PORTB, 4 is PORTB4 = ADC_RATE pin
        // Assembler insertion to make sure it is done as fast as possible, since this code is very time-sensitive
        __asm__ __volatile__ (
          "sbi 0x17, 4\n"
          "cbi 0x17, 4\n"
          "cbi 0x18, 4\n"  
        );
      }
      skipConversion = !skipConversion;
      adc_startConversion();
      return;
    }
  #endif

  // Moving average using prevAnalogValues[]
  uint16_t val = adc_getConversionResult();
  #ifdef S_USE_MOVAVG
    uint16_t avgVal = (val + prevAnalogValues[analogChannelRead]) >> 1;
    prevAnalogValues[analogChannelRead] = val;
  #endif

  // Set atomic lock, set value, reset atomic lock
  atomicLock[analogChannelRead] = true;
  #ifdef S_USE_MOVAVG
    analogValues[analogChannelRead] = avgVal;
  #else
    analogValues[analogChannelRead] = val;
  #endif
  atomicLock[analogChannelRead] = false;

  // Switch to the next channel
  analogChannelReadIndex++;
  if(analogChannelReadIndex > 2) {analogChannelReadIndex = 0;} // TODO: Replace hard-coded sequence length
  analogChannelRead=analogChannelSequence[analogChannelReadIndex];

  //Switch skipConversion if we need to
  #ifdef S_ADC_FIX
    skipConversion = !skipConversion;
  #endif

  // Next conversion
  adc_connectChannel(analogChannelRead);
  adc_startConversion();
}

// Different ADC functions: initialization, conversion running/stopping, etc.

void adc_init() {  
  // ADC Clock Frequency is 8000000/8 = 1 MHz
  // ADC Sample rate is 1MHz / 13.5 = 74.1 kHz (remember that it is also limited by the ADC ISR execution time and should be divided by 2)
  ADMUX  = 0;
  bitWrite(ADCSRA,ADEN,1);                        // ADC enable
  // ADCSRA |= bit(ADPS0) | bit(ADPS1); // Prescaler : 1/8
  ADCSRA |= bit(ADPS2); // Prescaler : 1/16
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

uint16_t adc_getConversionResult() {
  uint16_t result = ADCL;
  return result | (ADCH<<8);
}
void adc_connectChannel(uint8_t number) {
  ADMUX &= (11110000);
  ADMUX |= number;
}
