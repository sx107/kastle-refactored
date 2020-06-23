#ifndef LFO_H
#define LFO_H

#include <avr/io.h>
#include <stdlib.h>

void lfo_mode_select(uint8_t val); // Called in ADC ISR
void lfo_set_frequency(uint16_t freq); // Called in ADC ISR
void lfo_init(); // Called in setup()

#endif
