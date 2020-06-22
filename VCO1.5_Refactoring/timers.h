#ifndef TIMERS_H
#define TIMERS_H


  // Probably a way to improve quality: use timer0 for timing the interrupt,
  // use timer1 for PWM output, overclock timer1
  // Only one output will be available then, though
  // I didn't check it, but probably that's how the original kastle worked?
  // === TIMERS SETUP === //

inline void timers_init() {
  // Turn on PLL and wait while it stabilizes
  bitSet(PLLCSR, PLLE);
  delayMicroseconds(100);
  while(!bitRead(PLLCSR, PLOCK));
  bitSet(PLLCSR, PCKE);
  bitSet(PLLCSR, LSM);  //low speed mode, PLL @ 32MHz.

  cli();

  //Set up the TIM0 timer
  TCCR0A = bit(WGM00) | bit(WGM01); //Mode: 011, fast PWM. IMHO regular CTC PWM is superior
  TCCR0B = 0; //WGM02 is zero
  TCCR0A |= bit(COM0A1); // Channel A: Non-inverting PWM mode (10)
  TCCR0A |= bit(COM0B1); // Channel B: Non-inverting PWM mode (10)
  bitSet(TCCR0B, CS00); // Timer frequency: CLKio/1 = 8MHz

  //Set up the TIM1 timer
  TCCR1 = 0; TCNT1 = 0; // Stop and clear the timer
  GTCCR = bit(PSR1); //Reset the timer
  OCR1A = 255; OCR1C = 255;
  TIMSK = bit(OCIE1A); // Interrupt enable
  TCCR1 = bit(CTC1) | bit(CS12);  //Clock: PLL Clock / 8 = 32MHz / 8 = 4 MHz = CPU Clock / 2
                                  //Final Timer1 config: reset on OCR1C compare
                                  //Interrupt on OCR1A compare
                                  //Frequency: 4MHz / 256 = 15625 Hz
}

#endif
