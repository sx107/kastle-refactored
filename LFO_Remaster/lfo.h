#ifndef LFO_H
#define LFO_H

#include <avr/io.h>
#include <stdlib.h>



// Used externally
void lfo_mode_select(uint16_t val); // Called in loop(), sets mode based on ADC reading
void lfo_set_frequency(uint16_t freq); // Called in loop(), sets LFO frequency based on ADC reading
void lfo_init(); // Called in setup()

#endif
