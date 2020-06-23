/*
  == KASTLE LFO refactoring ==
  Original code by Vaclav Pelousek @ Bastl Instruments (c) 2017  http://www.bastl-instruments.com
  Code refactored by sx107 (Aleksandr Kurganov) (c) 2020

  See README.md for additional notes

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
