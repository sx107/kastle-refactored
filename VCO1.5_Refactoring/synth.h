#ifndef SYNTH_H
#define SYNTH_H

#define F_CPU 8000000UL
#include "Arduino.h"

//Synth modes
#define MODE_FM 0 //aka phase modulation
#define MODE_NOISE 1 // Noise sample
#define MODE_TAH 2 //aka track & hold modulation (downsampling with T&H)

extern volatile uint8_t mode;
extern volatile uint8_t easterEgg_timer;

void synthesis();
void synthesis_init();
void modeSelect();

// Replaces setFrequency(), setFrequency2()
// Executed in the ADC interrupt, imho it's not the best solution:
// these could be relatively long functions due to multiplications
volatile void parameterModified(uint8_t parameterId, uint16_t parameterValue);
void synthesis_easterEggInit();

#endif
