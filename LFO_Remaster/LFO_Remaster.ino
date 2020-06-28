/*
  == KASTLE LFO remaster ==
  Original code by Vaclav Pelousek @ Bastl Instruments (c) 2017  http://www.bastl-instruments.com
  Code remastered by sx107 (Aleksandr Kurganov) (c) 2020

  See README.md for additional notes

  == License ==
  Open source license: CC BY SA
  
*/

#define F_CPU 8000000UL
#include "Arduino.h"
#include "adc.h"
#include "lfo.h"
#include "timers.h"

void setup() {
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
  lfo_init();
  timers_init();
  adc_init();
  lfo_set_frequency(512);
  sei();

  adc_initialConversion();
}

void loop() {
  static volatile uint8_t phs = 0;
  while(atomicLock[ADC_MODE]) {__asm__ __volatile__ ("nop\n\t");}
  lfo_mode_select(analogValues[ADC_MODE]);
  while(atomicLock[ADC_RATE]) {__asm__ __volatile__ ("nop\n\t");}
  lfo_set_frequency(analogValues[ADC_RATE]);
}
