/*
  == KASTLE VCO v1.5 refactoring ==
  Original code by Vaclav Pelousek @ Bastl Instruments (c) 2017  http://www.bastl-instruments.com
  Code refactored by sx107 (Aleksandr Kurganov) (c) 2020

  == File structure ==
  This file: initializing and stuff
  synthesis.h, synthesis.cpp: synthesis engine (both in loop and interrupt), mode selection
  adc.h, adc.cpp: ADC functions

  == Compiling ==
  This code is intentded for Attiny85 running @ 8Mhz,
  but can be modified to run at 16Mhz without much consequences 
  (except for low-voltage 2.7V operability, PLL 64MHz only works at higher voltages)

  Compile options (with AttinyCore 1.3.3, Arduino IDE 1.8.10):
  Attiny85 @ 8MHz internal
  Millis/micros: disabled (we don't use those)
  Timer1 clock: 32 MHz (though I don't know if it changes anything, since the timer clock source is set in timer initialization)
  
  LTO: !!! IMPORTANT !!!
  Apparently LTO can break the code (mostly because of ISRs, which, as much as the optimizator is concerned,
  are never called, and variables shared between ISR and other functions are technically undefined behaviour)
  I've

  == Notes ==

  I've tried to stick to the original code as much as possible to keep the original functionality,
  but later I'll refactor this code even more as a separate project.
  Although I've tried to keep the original functionality, this firmware sounds a tad bit different from the original one:
  1. FM is a tad bit more noisy;
  2. It just sounds a little different.
  In first case it is most definetely my bad coding,
  in second case maybe it's my bad coding, but it could be the bad original code as well.

  TODO later (as a separate project, probably):
  1. Rewrite whole synthesis code into the timer ISR
  2. Make atomics where needed
  3. Sharing 16-bit values between ISR and other functions is a bad idea
  4. Separate the synthesis code as much as possible so that it would be easy to implement own synthesis modes
  5. Find a better solution for zener diode's capacitance issue
  6. Replace multiplexing with look-up tables (like using a quater-logsine lookup table)

 == Special thanks ==

  Special thanks to:
  Whole bastl instruments team developing this amazing mini-synth and making it publically available
  Spence Konde for the development of AttinyCore and helping with the LTO problem
  Stack overflow for pointing it out that I'd need tons of volatiles

  == License ==
  Open source license: CC BY SA
  
*/

#define F_CPU 8000000UL
#include "Arduino.h"

#include <avr/interrupt.h>
#include <stdlib.h>
#include <avr/io.h>

#include "adc.h"
#include "synth.h"
#include "timers.h"

void setup() {
  synthesis_init();
  // === PINS SETUP === //
  // Pins are setup as input by default.
  digitalWrite(5, HIGH); // Immedeately turn on this pull-up resistor for the reset pin
  pinMode(0, OUTPUT);
  pinMode(1, OUTPUT);

  cli();
  timers_init();
  adc_init();
  sei();

  adc_initialConversion();
}

void loop() {
  synthesis();
  modeSelect();
}
