#ifndef TIMERS_H
#define TIMERS_H


  // Probably a way to improve quality: use timer0 for timing the interrupt,
  // use timer1 for PWM output, overclock timer1
  // Only one output will be available then, though
  // I didn't check it, but probably that's how the original kastle worked?
  // === TIMERS SETUP === //

inline void timers_init() {
  //Set up the TIM0 timer
  TCCR0A = bit(WGM00) | bit(WGM01); //Mode: 011, fast PWM. IMHO regular CTC PWM is superior
  TCCR0B = 0; //WGM02 is zero
  TCCR0A |= bit(COM0A1); // Channel A: Non-inverting PWM mode (10)
  TCCR0A |= bit(COM0B1); // Channel B: Non-inverting PWM mode (10)
  bitSet(TCCR0B, CS00); // Timer frequency: CLKio/1 = 8MHz

  //Set up the TIM1 timer
  TCCR1 = 0; TCNT1 = 0; // Stop and clear the timer
  GTCCR = bit(PSR1); //Reset the timer
  OCR1A = 250;
  // OCR1C = 0xFF; // Not initialized?!
  TIMSK = bit(OCIE1A); // Interrupt enable
  TCCR1 = bit(CTC1);  // Clock source: CPU Clock (8Mhz)
                      // Prescaler is changed dynamically in setFrequency() of lfo.cpp
                      // Currently, the prescaler is 0 (timer disabled)
                      // Timer setup is reset on OCR1C compare, interrupt OCR1A
                      // Interrupt resets the timer as well.
}								 

#endif
