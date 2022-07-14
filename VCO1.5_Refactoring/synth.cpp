#include "adc.h"
#include "synth.h"
#include "TR_HH_AT.h"
#include <avr/interrupt.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

//Thresholds for mode selection
#define LOW_THRES 150
#define HIGH_THRES 162

// Set this define if you want the easter egg mode to be triggered only by knobs (mode port is ignored)
// #define NOMODE_EASTER_EGG
// Set this define if you want the easter egg to check only the WAVESHAPE knob
// #define WS_EASTER_EGG

// Sine wavetable in progmem and RAM
const volatile unsigned char PROGMEM sinetable[128] = {
  0, 0, 0, 0, 1, 1, 1, 2, 2, 3, 4, 5, 5, 6, 7, 9, 10, 11, 12, 14, 15, 17, 18, 20, 21, 23, 25, 27, 29, 31, 33, 35, 37, 40, 42, 44, 47, 49, 52, 54, 57, 59, 62, 65, 67, 70, 73, 76, 79, 82, 85, 88, 90, 93, 97, 100, 103, 106, 109, 112, 115, 118, 121, 124,
  128, 131, 134, 137, 140, 143, 146, 149, 152, 155, 158, 162, 165, 167, 170, 173, 176, 179, 182, 185, 188, 190, 193, 196, 198, 201, 203, 206, 208, 211, 213, 215, 218, 220, 222, 224, 226, 228, 230, 232, 234, 235, 237, 238, 240, 241, 243, 244, 245, 246, 248, 249, 250, 250, 251, 252, 253, 253, 254, 254, 254, 255, 255, 255,
};

//EVERYTHING that is shared between an ISR and other functions MUST be volatile
//Otherwise LTO breaks the the code (like cutting away whole synthesis() function)
volatile uint8_t wavetable[256];

// Two outputs
volatile uint8_t sample1 = 0;
volatile uint8_t sample2 = 0;

// Internal oscillators
volatile uint16_t phase1 = 0, frequency1 = 0;
volatile uint16_t phase2 = 0, frequency2 = 0;
volatile uint16_t phase4 = 0, frequency4 = 0;
volatile uint16_t phase5 = 0, frequency5 = 0;
volatile uint16_t phase6 = 0, frequency6 = 0;
volatile uint8_t  phase3 = 0;

// Easter egg switch: volatile, because used in the ISR
volatile bool easterEgg_mode = false;
// Easter egg timer, increased in the ADC ISR
volatile uint8_t easterEgg_timer = 0;

//Synthesis mode
volatile uint8_t mode = MODE_TAH;

ISR(TIMER1_COMPA_vect)
{
  //Internal oscillators 1, 2, 4, 5, 6
  phase1 += frequency1;
  phase2 += frequency2;
  phase4 += frequency4;
  phase5 += frequency5;
  
  if(mode == MODE_FM) {
    // == FM MODE OUT1 == //
    // Frequency1 - main osc
    // Frequency2 - modulator
    // Waveshape - FM amount
    uint8_t phs=(phase1+(analogValues[ADC_WS2]*wavetable[phase2 >> 8])) >> 6;
    sample1 = wavetable[phs];
    if(easterEgg_mode) {
      uint8_t phs90 = phs + 64; // To be sure that phase value will, in fact, wrap around
      sample2 = wavetable[phs90];
    }
  } else {
    phase3 = phase2 >> 5; // No idea why this was done in the ISR() before
    phase6 += frequency6;
  }

  OCR0A = sample1;
  OCR0B = sample2;
}


void synthesis() {
  static volatile uint8_t saw = 0, lastSaw = 0;
  if (mode == MODE_FM)
  {
      // == FM MODE OUT2 == //
      lastSaw = saw;                                                                  
      saw = ( ( (255 - (phase1 >> 8) ) * ( analogValues[ADC_WS2] ) ) >> 8);           // Saw at frequency1, ADC_WS2 modulates the volume
      sample2 = ( ( saw * wavetable[phase4 >> 8] ) >> 8)                              // Sine at frequency4 = frequency2, AM-modulated by saw
        + ( ( wavetable[phase5 >> 8] * (255 - analogValues[ADC_WS2]) ) >> 8);         // Sine at frequency5 = frequency2, volume reverse-controlled by ADC_WS2
      if (lastSaw < saw) {phase4 = 64 << 8;}                                          // If saw overflows, hard-sync frequency4
      uint8_t s = abs(saw - lastSaw); if(s > 3) {phase5 += s << 8;}                   // Soft-sync frequency5?
  }
  else if (mode == MODE_TAH)
  {
      // == T&H MODE OUT1 == //
      // S&H frequency1, frequency2 is T&H frequency, waveshape is T&H amount
      uint8_t phs = phase1 >> 8;
      if ((phase2 >> 8)>analogValues[ADC_WS2]) {
        sample1 = (wavetable[phs]);
        if(easterEgg_mode) {
            uint8_t phs90 = phs + 64;
            sample2 = wavetable[phs90]; 
        }
      } 
      // == T&H MODE OUT2 == //
      // Summ of freq2, freq4, freq5, freq6
      if(!easterEgg_mode) {
        sample2 = (wavetable[phase2 >>8]+ wavetable[phase4 >>8] + wavetable[phase5 >>8]+ wavetable[phase6 >>8])>>2;
      }
      
  }
  else if (mode == MODE_NOISE)
  {
      // == NOISE MODE OUT1 == //
      if((phase1>>2)>=(analogValues[ADC_WS2]-100)<<5) {phase1 = 0;} // Essentially sample length is controlled by WS2 (waveshape)
      uint8_t smp = (char)pgm_read_byte_near(sampleTable+(phase1>>2)); //Read the noise sample (and anything beneath it?)
      sample1 = (smp*wavetable[phase2>>8])>>8; //Modify the sample by AM-modulating it with freq2
      // == NOISE MODE OUT2 == //
      sample2 = (wavetable[phase3+(phase1>>8)]); // Pseudo-noise: sin(phase >> 5 + phase >> 8) and phase is "cut" at some point (sample length)
                                                  //The phase will, though, go past the wavetable and grab some noise
  }
}


volatile void parameterModified(uint8_t parId, uint16_t parVal) {
  const uint8_t freq_multiplier[24]={2,2,2,3,1,2,4,4, 3,4,5,2,1,5,6,8, 3,8,7,8,7,6,8,16}; // Harmonic oscillators' multipliers
  
  if(parId == ADC_WS2 || parId == ADC_MODE) {return;}
  
  if(parId == ADC_PITCH) {
    if(mode == MODE_NOISE) {frequency1 = ((parVal-200)<<2)+1;}
    else {frequency1 = (parVal<<2)+1;}
  }
  else {
    // Switch-case does not work for some reason.
    if(mode == MODE_FM) {
        frequency2 = (parVal << 2) + 1;
        frequency4 = frequency2;
        frequency5 = frequency2;
    }
    else if(mode == MODE_TAH) {
        volatile uint8_t multiplierIndex = analogValues[ADC_WS2]>>5;
        frequency2 = (parVal<<2)+1;
        frequency4=(frequency2+1)*freq_multiplier[multiplierIndex];
        frequency5=(frequency2-3)*freq_multiplier[multiplierIndex+8];
        frequency6=(frequency2+7)*freq_multiplier[multiplierIndex+16];
    }
    else if (mode == MODE_NOISE) {
      frequency2 = (((parVal-300)<<2)+1)/2;
    }
  }
}

void synthesis_init() {
  //Sine wave loading. Other waveforms were implemented in the original code, but hey
  for (int i = 0; i < 128; ++i) {
    wavetable[i] = pgm_read_byte_near(sinetable + i);
  }
  wavetable[128] = 255;
  for (int i = 129; i < 256; ++i) {
    wavetable[i] = wavetable[256 - i] ;
  }
}

void synthesis_easterEggInit() {
  // Run synthesis while we are waiting
  // In the original code it's while(...) {loop();}
  // But due to refactoring here it's impossible, so...
  while(easterEgg_timer < 12) {
    synthesis();
    modeSelect();
  }

  //Easter mode check
  easterEgg_mode = true;
  #ifndef NOMODE_EASTER_EGG
  if(analogValues[0] < HIGH_THRES) {easterEgg_mode = false;}
  #endif

  #ifdef WS_EASTER_EGG
  if(analogValues[ADC_WS2] < 200) {easterEgg_mode = false;}
  #else
  for(uint8_t i = 1; i < 4; i++) {if(analogValues[i] < 200) {easterEgg_mode = false;}}
  #endif
}

void modeSelect() {
  if (analogValues[ADC_MODE]<LOW_THRES) {mode=MODE_NOISE;}      // GND is connected
  else if (analogValues[ADC_MODE]>HIGH_THRES) {mode=MODE_TAH;}  // VCC is connected
  else {mode = MODE_FM;}                                        // Pin floating
}
