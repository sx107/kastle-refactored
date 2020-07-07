#include "settings.h"
#include "lfo.h"
#include "adc.h"
#define F_CPU 8000000UL

#include "Arduino.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdlib.h>
#include <avr/eeprom.h> // used for random generation

#define LOW_THRES 600
#define HIGH_THRES 648

#define RND_SEED_ADDR 0

volatile uint8_t runglerByte = 0; // Will be set to a random value in lfo_init()
volatile uint8_t runglerMode = 1; // 0 - connected to GND, 1 - floating, 2 - VCC

volatile uint8_t lfo_phase_increase = 1;

volatile uint8_t lfoValue = 0;    // global, since 
volatile bool lfoFlop = false;    // lfoFlop inverts the LFO value in the end, making the output falling instead of rising
volatile bool doneReset = false;  // Have the reset ISR just done a reset?

// A lookup table to make the transition between timer1 prescalers exponential
// Essentially, this is a function f(x) = 128 * 2^(x / 127.721) for x in [0, 127]
// 127.721 beause we want f(127) = 255
// This lookup table could be shortened by times of 2, since (see lfo_set_frequency code) compVal changes in steps of 2. I made it longer to make code more readable
// And to account in advance for an additional bit added by smoothing
#ifdef S_USE_EXP_LOOKUP
const uint8_t PROGMEM expLookup[] = {128, 129, 129, 130, 131, 132, 132, 133, 134, 134, 135, 136, 137, 137, 138, 139, 140, 140, 141, 142, 143, 143, 144, 145, 146, 147, 147, 148, 149, 150, 151, 151, 152, 153, 154, 155, 156, 156, 157, 158, 159, 160, 161, 162, 163, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 209, 210, 211, 212, 213, 214, 216, 217, 218, 219, 220, 221, 223, 224, 225, 226, 228, 229, 230, 231, 233, 234, 235, 236, 238, 239, 240, 242, 243, 244, 245, 247, 248, 250, 251, 252, 254, 255};
#endif

void lfo_init() {
  //Seed initialization
  #ifdef S_TRUE_RANDOM
    uint16_t seed = eeprom_read_word(RND_SEED_ADDR);
    seed++;
    eeprom_update_dword(RND_SEED_ADDR, seed);
    randomSeed(seed);
  #endif
  
  runglerByte = random(255);

  //Init the reset interrupt
  bitSet(GIMSK, PCIE); // Enabe pin change interrupt
  PCMSK = _BV(PINB3); // Mask: only PINB3
}

// Detect rungler mode (much like modeSelect() in VCO code)
void lfo_mode_select(uint16_t val) {
  if(val < LOW_THRES) {runglerMode = 0;}        // GND
  else if (val > HIGH_THRES) {runglerMode = 2;} // VCC
  else {runglerMode = 1;}                       // Floating
}

// LFO frequency

void lfo_set_frequency(uint16_t freq) {  
  // Return if we are already at this frequency
  static volatile uint16_t prevFreq = 0;
  if(freq == prevFreq) {return;}
  prevFreq = freq;

  #ifdef S_USE_8BIT
    freq = freq & ~(0b11);
  #endif

  #ifdef S_FASTER_LFO
    freq += S_FASTER_LFO;
    if(freq > 1023) {freq = 1023;}
  #endif

  // 10 bits are passed as input.
  // 11 bits are used in frequency setup
  freq = 2047 - (freq << 1);

  // Three things affect LFO frequency:
  // 1. Timer 1 prescaler (1/1 -- 1/16384), high 4 bits of 11 bits are used
  // 2. Timer 1 OCR1A compare value, low 7 bits are used (changes in steps of 2, because "freq << 1")
  // Timer1 OCR1A compare value is using expLookup LUT to make transition between timer1 prescalers exponential
  // 3. In the ISR: phase increment (x1 or x2), x2 essentially reduces the PWM output resolution to 7 bit
  // x2 is set when the prescaler is 0 (since prescaler = 0 means actually timer stop)
  uint8_t prescaler = (freq >> 7) & 0b1111;
  
  #ifdef S_USE_EXP_LOOKUP
    uint8_t compVal = pgm_read_byte_near(expLookup + (freq & 0x7F));
  #else
    uint8_t compVal = (freq & 0x7F) | 0x80;
  #endif
  
  uint8_t lfo_inc = 1;
  
  if(prescaler == 0) {
    prescaler = 1;
    lfo_inc = 2;
  }

  if((TCCR1 & 0b1111) != prescaler) {
    uint8_t tccr = TCCR1 & ~(0b1111);
    tccr |= prescaler;
    TCCR1 = tccr; // Use low 4 bits of preScaler as prescaler
  }
  
  OCR1A = compVal; // Low 7 bits of freq, high bit of OCR1A set to 1
  lfo_phase_increase = lfo_inc;

  // ISR frequency: 1.914 Hz - 62kHz (maybe the max ISR frequency should be lowered?)
  // LFO frequency: 3.739mHz - 242.2 Hz
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
  

// Reset interrupt
ISR(PCINT0_vect) {
  if(bitRead(PINB, PINB3)) {return;}
  lfoValue = 0;
  lfoFlop = true;
  doneReset = true;

  //TODO: Call Timer1 ISR here, reset the TCNT1, reset Timer1 prescaler
}


ISR(TIMER1_COMPA_vect) {
  static uint8_t runglerOut = 0; // runglerByte is globally defined

  // If lfoValue overflowed OR a reset was triggered
  // If reset was triggered, DO NOT increase the phase and DO NOT switch lfoFlop

  if(!doneReset) {lfoValue += lfo_phase_increase;}

  if(lfoValue < lfo_phase_increase) {
    // Invert the LFO output, we don't need to reset the LFO value (it resets itselfs by overflowing)
    // If this interrupt was triggered by a reset, then we DONT need to flop the value
    if(!doneReset) {lfoFlop = !lfoFlop;} 
    // Rungler
    runglerByte = lfo_process_rungler(runglerByte);
    runglerOut = lfo_output_rungler(runglerByte);
  }

  
  
  // Square output, slighly assymetric (probably to make it possible for the LFO to self-reset to produce a saw wave instead)
  
  if (lfoFlop) {bitWrite(PORTB, PINB2, 1);}
  else {bitWrite(PORTB, PINB2, 0);}

  // These NOPs are to wait a bit for the PCINT0 interrupt to react if square output is connected to the reset pin
  // PCINT0 is "more important" than Timer1 COMPA interrupt
  __asm__ __volatile__ ("nop\n\t"); 
  __asm__ __volatile__ ("nop\n\t"); 
  __asm__ __volatile__ ("nop\n\t");

  // Output
  OCR0A = runglerOut;
  if(lfoFlop) {OCR0B = ~lfoValue;} // ~lfoValue is essentially 255 - lfoValue, but faster
  else {OCR0B = lfoValue;}  

  doneReset = false;
  
  // Reset the TIMER1
  TCNT1 = 0;
}
