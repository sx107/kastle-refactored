/*
  == KASTLE LFO refactoring ==
  Original code by Vaclav Pelousek @ Bastl Instruments (c) 2017  http://www.bastl-instruments.com
  Code refactored by sx107 (Aleksandr Kurganov) (c) 2020

  == File structure ==
  This file: initializing and stuff
  lfo.h, lfo.cpp: LFO core, Timer 1 interrupt
  adc.h, adc.cpp: ADC functions, ADC interrupt

  == Compiling ==
  This code is intentded for Attiny85 running @ 8Mhz

  Compile options (with AttinyCore 1.3.3, Arduino IDE 1.8.10):
  Attiny85 @ 8MHz internal
  Millis/micros: disabled (we don't use those)
  Timer1 clock: 32 MHz (though I don't know if it changes anything, since the timer clock source is set in timer initialization)
  
  LTO: Up to you, I'd set it to enabled. It can sometimes break the code, though,
  but I hope that I've made everything that's needed volatile

  == Notes ==

  I've tried to stick to the original code as much as possible to keep the original functionality,
  but later I'll refactor this code even more as a separate project.
  Although I've tried to keep the original functionality, LFO speed is approx. half the original one.
  Uploading the original LFO code in the bastl instruments repo sets the LFO speed to half of the original one as well.

  There are tonns of potential bugs here which I'll fix in a separate, "remastering", project.

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
#include "adc.h"
#include "lfo.h"
#include "timers.h"

void setup() {
  adc_createLookup();
  lfo_init();

  // Pins setup
  // 0 - Rungler out, 1 - Tri out, 2 - Square output
  // 3 - LFO Reset, 4 - Rate input, 5 - uC reset pin & rungler mode
  
  digitalWrite(5, HIGH); // Pull reset pin to VCC
  
  pinMode(0, OUTPUT);
  pinMode(1, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  pinMode(5, INPUT);

  cli();
  timers_init();
  adc_init();
  lfo_set_frequency(128);
  sei();

  

  adc_initialConversion();
}

void loop() {
  // Nothing here, see timer1 interrupt in lfo.cpp
}
