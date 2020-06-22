#include "lfo.h"
#include "adc.h"
#define F_CPU 8000000UL

#include "Arduino.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdlib.h>

#define LOW_THRES 150
#define HIGH_THRES 162

volatile uint8_t runglerByte = 0; // Will be set to a random value in lfo_init()

void lfo_init() {
  runglerByte = random(255); // It sets the runglerByte to the same value every time, actually
}

void lfo_set_frequency(uint16_t freq) {
  // This code in the original firmware makes me sexually identify as a measure of speed
  // Because I really want to KM/S because of it
  
  // (2048 - freq*8) + 20, freq now is mapped to (2068-20) 12-bit
  // Highest bit is neglected, though
  // Probably a bug: freq can be larger than 2047 (11 bits), which breaks prescaler and compare value settings
  freq = (2048 - (freq << 3)) + 20; 

  // freq / 128 + 1, high 5 bits + 1
  // +1, because preScaler = 0 means timer stop
  // preScaler here can be higher than 0b1111, which breaks everything (probably a bug)
  // If (freq >> 7 & 0b1111) = 0b1111, timer essentially stops (since preScaler is then set to 0)
  uint8_t preScaler = (freq >> 7); 
  preScaler += 1;

  TCCR1 &= ~(0b1111);
  TCCR1 |= preScaler & (0b1111); // Use low 4 bits of preScaler as prescaler
  OCR1A = freq | 0b10000000; // Was: uint8_t comp = freq; bitWrite(comp, 7, 0); OCR1A = comp + 128;

  // In the end: this code makes the ISR frequency from 1.937Hz to 51.28kHz
  // Input values 3 <= freq <= 18 make the LFO completely stop (timer stops due to prescaler being equal to zero)
  // Input values 0 <= freq <= 2 break the code and make the ISR frequency 54.1kHz-60.6kHz
  // LFO Frequency is approx. ISR frequency / (256 * 2) = (excluding broken cases) 3.7mHz-100.1Hz
}

ISR(TIMER1_COMPA_vect) {
  const uint8_t runglerMap[8] = {0, 80, 120, 150, 180, 200, 220, 255};
  
  static uint8_t lfoValue = 0;
  static bool lfoFlop = false; // lfoFlop inverts the LFO value in the end, making the output falling instead of rising
  static bool doReset = false;
  static bool lastDoReset = false;
  static uint8_t runglerOut = 0; // runglerByte is globally defined

  //Increase LFO value
  lfoValue++;

  // Square output, slighly assymetric (probably to make it possible for the LFO to self-reset to produce a saw wave instead)
  if (lfoFlop && lfoValue < 200) {digitalWrite(2, HIGH);}
  else {digitalWrite(2, LOW);}

  // Reset at pin 3: make lfo value zero, rising
  doReset = digitalRead(3);

  if(!lastDoReset && doReset) {
    lfoValue = 0;
    lfoFlop = false;
  }

  lastDoReset = doReset;

  // If lfoValue overflowed
  if(lfoValue == 0) { 
    lfoFlop = !lfoFlop; // Invert the LFO output, we don't need to reset the LFO value (it resets itselfs by overflowing)

    // Rungler
    bool newBit = bitRead(runglerByte, 7);
    runglerByte <<= 1; // Shift the rungler

    // Three rungler modes: set first bit to the last one before shift, set first bit to a random one, set first bit to inverted last one before shift
    if(analogValues[0] < LOW_THRES) // Connected to GND
    {
      newBit = newBit; // Excuse me?
    }
    else if (analogValues[0] > HIGH_THRES) // Connected to VCC
    { // Connected to VCC
      newBit = TCNT0 >> 7; // Timer0 current value used as random source
    }
    else { // Floating
      newBit = !newBit;
    }

    bitWrite(runglerByte, 0, newBit);
    runglerOut = 0;
    bitWrite(runglerOut, 0, bitRead(runglerByte, 0));
    bitWrite(runglerOut, 1, bitRead(runglerByte, 3));
    bitWrite(runglerOut, 2, bitRead(runglerByte, 5));
    runglerOut = runglerMap[runglerOut];
  }

  // Flip the LFO output if we need to
  uint8_t out = lfoValue;
  if(lfoFlop) {out = 255 - lfoValue;}

  // Output
  OCR0A = runglerOut;
  OCR0B = constrain(out, 0, 255); // constrain?!

  // Reset the TIMER1
  TCNT1 = 0;
}
