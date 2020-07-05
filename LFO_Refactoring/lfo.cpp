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
volatile uint8_t runglerMode = 1; // 0 - connected to GND, 1 - floating, 2 - VCC

void lfo_init() {
  runglerByte = random(255); // It sets the runglerByte to the same value every time, actually
}

// Detect rungler mode (much like modeSelect() in VCO code)

void lfo_mode_select(uint8_t val) {
  if(val < LOW_THRES) {runglerMode = 0;}        // GND
  else if (val > HIGH_THRES) {runglerMode = 2;} // VCC
  else {runglerMode = 1;}                                   // Floating
}

// LFO frequency

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


// Rungler ("random") generation, generates next step and returns next byte state
uint8_t lfo_process_rungler(uint8_t in) {
  bool newBit = bitRead(in, 7);
  in <<= 1;
  
  switch(runglerMode) {
    case 1: newBit = !newBit; break;
    case 0: newBit = newBit; break; // Excuse me?
    case 2: newBit = TCNT0 >> 7; break; // Pseudo-random, though imho TCNT0 & 0b1 would be better
  }
  
  bitWrite(in, 0, newBit);
  return in;
}

// Returns rungler output dependant on it's current byte state
uint8_t lfo_output_rungler(uint8_t in) {
  const uint8_t runglerMap[8] = {0, 80, 120, 150, 180, 200, 220, 255};
  
  uint8_t out = 0;
  bitWrite(out, 0, bitRead(in, 0));
  bitWrite(out, 1, bitRead(in, 3));
  bitWrite(out, 2, bitRead(in, 5));
  return runglerMap[out];
}


ISR(TIMER1_COMPA_vect) {
  static uint8_t lfoValue = 0;
  static bool lfoFlop = false; // lfoFlop inverts the LFO value in the end, making the output falling instead of rising
  static bool doReset = false;
  static bool lastDoReset = false;
  static uint8_t runglerOut = 0; // runglerByte is globally defined

  //Increase LFO value
  lfoValue++;

  // Square output, slighly assymetric (probably to make it possible for the LFO to self-reset to produce a saw wave instead)
  if (lfoFlop && lfoValue < 200) {bitWrite(PORTB, PINB2, 1);}
  else {bitWrite(PORTB, PINB2, 0);}

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
    runglerByte = lfo_process_rungler(runglerByte);
    runglerOut = lfo_output_rungler(runglerByte);
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
