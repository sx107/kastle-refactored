#include "adc.h"
#include "synth.h"
#define F_CPU 8000000UL
#include "Arduino.h"

// ADC_TEST_MODE: Disregards all ADC readings and just scans through some values instead
//#define ADC_TEST_MODE


volatile uint8_t analogValues[4] = {0, 0, 0, 0};
volatile uint8_t analogChannelRead = 1;
volatile uint8_t analogChannelReadIndex = 0;
volatile uint8_t lastAnalogValues[4] = {255, 255, 255, 255};
volatile uint8_t lastAnalogChannelRead = 0;
volatile bool skipConversion = false; // Every second ADC reading is skipped


#ifdef ADC_TEST_MODE

uint8_t fv;
ISR(ADC_vect) {
  analogValues[ADC_MODE] = 155;
  analogValues[ADC_PITCH] = 125;
  analogValues[ADC_WS2] = 127;
  if(fv++ == 100) {
    analogValues[ADC_WS1]++;
    parameterModified(ADC_PITCH, analogValues[ADC_PITCH] << 2);
    parameterModified(ADC_WS1, analogValues[ADC_WS1] << 2);
  }
  modeSelect();
  adc_startConversion();
}

#else

ISR(ADC_vect) {  
  const uint8_t analogChannelSequence[6]={0,1,0,2,0,3};
  const uint8_t analogToDigitalPinMapping[4]={PORTB5,PORTB2,PORTB4,PORTB3};

  // Easter egg timer
  easterEgg_timer++;

  if(skipConversion) {
    if(mode != MODE_NOISE) {
      // If the mode pin is not pulled to ground. Though code will reach to this point at second conversion when mode is undefined.
      // I've kept the code original here, though it clearly can be optimized
      // Apparently this is done to flush the stray voltage on the zener diodes (?)
      // And to skip every second conversion due to multiplexer cross-talk (?!)
      if(analogValues[analogChannelRead]<200) bitWrite(DDRB,analogToDigitalPinMapping[analogChannelRead],1);
      bitWrite(DDRB,analogToDigitalPinMapping[analogChannelRead],0);
      bitWrite(PORTB,analogToDigitalPinMapping[analogChannelRead],0);
    }
  } else {
    //Save previous value and read new one
    lastAnalogValues[analogChannelRead] = analogValues[analogChannelRead];
    analogValues[analogChannelRead] = (adc_getConversionResult() >> 2) & 0xFF;

    //Switch to the next channel
    lastAnalogChannelRead = analogChannelRead;
    if (analogChannelRead == 0) {modeSelect();}
    analogChannelReadIndex++;
    if(analogChannelReadIndex > 5) {analogChannelReadIndex = 0;} // TODO: Replace hard-coded sequence length
    analogChannelRead=analogChannelSequence[analogChannelReadIndex];
    adc_connectChannel(analogChannelRead);    

    //Update variables
    if(lastAnalogValues[lastAnalogChannelRead] != analogValues[lastAnalogChannelRead]) {
      // I don't get it. Why bitshift to the right, and then bitshift to the left?
      parameterModified(lastAnalogChannelRead, (0xFF&(analogValues[lastAnalogChannelRead])) << 2);
    }
  }

  skipConversion = !skipConversion;
  adc_startConversion();
}

#endif

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
