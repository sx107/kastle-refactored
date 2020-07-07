# LFO Remastering
Improved & bugfixed LFO firmware.
If you are looking for a refactored firmware that is most similar to the original, see the LFO_Refactoring folder instead.

Original code Original code by Vaclav Pelousek @ [Bastl Instruments](http://www.bastl-instruments.com) (c) 2017  

## Main changes
- Whole 10 bits of ADC resolution are used for frequency setup instead of just 8
- Rungler is properly randomized in initialization (made using EEPROM)
- curveMap is cut out, LFO response to the frequency modulation is pseudo-logariphmic
- Square output is now completely symmetric and clean
- LFO reset done in a separate PCINT0 ISR and can react to short pulses even at low LFO frequencies
- LFO max frequency is now faster and it can be used as a sub-oscillator, which opens a lot of audiorate modulation creative possibilities
- Parameters are updated in loop(), not in ADC ISR, atomic locks added
- Moved "lastAnalogValues[i] != analogValues[i]" to lfo_set_frequency

## Files description
- LFO_Remaster.ino: main file, initializing and stuff, variables update in loop()
- lfo.h, lfo.cpp: LFO core, Timer 1 rendering interrupt
- adc.h, adc.cpp : ADC functions, ADC interrupt, parameters update
- settings.h: Settings defines

## Compiling
This code is intentded for Attiny85 running @ 8Mhz.

Compile options (with AttinyCore 1.3.3, Arduino IDE 1.8.10):
- Attiny85 @ 8MHz internal
- Millis/micros: disabled
- Timer1 clock: CPU
- LTO: Enabled
- B.O.D. : disabled
- Save EEPROM: EEPROM Retained

## Settings in settings.h

- __S_TRUE_RANDOM__ Trully randomizes the rungler byte every initialization
- __S_USE_8BIT__ Use only 8bits of ADC resolution
- __S_FASTER_LFO__ Increases the ADC conversion result for LFO frequency calculation by the amount set in this define. Requiered since the ADC for some reason does not read whole 0-1023 range. If you wish to return to the original frequnecy range, comment this define out (it's faster than setting it to 0)
- __S_USE_EXP_LOOKUP__ Makes the transition between timer1 prescalers exponential by using a look-up table

## Theoretical LFO frequency
With __S_USE_EXP_LOOKUP__

![Theoretical LFO frequency with exponential LUT](/readme_images/w_exp.png)

And without

![Theoretical LFO frequency without exponential LUT](/readme_images/no_exp.png)

## Known bugs & issues

Don't forget to add your own bugs during bugfixes to make it more interesting...

- At a certain audiorate frequency LFO output can become very noisy. Apparently, this happens at the transition between lfo_phase_increase = 1 and lfo_phase_increase = 2.
- ISR frequencies higher than ~50kHz are unreachable, which makes high audiorate frequencies lower than calculated. This is another reason to lower the ISR frequency, sample rates higher than 20kHz are imho unnecessary.
- Probably not an issue at all: if you connect the square output to the LFO rate modulation and set the LFO rate modulation knob to max, a rising saw that is produced on the LFO Tri output clicks every period.

## TODO
- Fix bugs
- Add DUAL_STEP firmware capabilities in this one by utilizing defines
- Make LFO react to reset almost instantly by resetting all prescalers and calling the Timer1 ISR in the reset ISR
- It would be much better if INT0 ISR would be used instead of the PCINT0 ISR for reset, though it requires the schematic to be changed (PINB2 and PINB3 have to be swapped)
- Lower the ISR frequency and gradualy loose the resolution by making the lfoValue 16-bit and making phase increase 0x100, and then controlling it more precise
- Make the ADC frequency higher, maybe add running mean noise smooting and/or hysteresis

## Bad ideas
- Replacing lfo flopping by simply computing (255-lfoValue) with PWM inversion bit. This is a bad idea, because such a solution causes glitches due to a race condition between setting the OCR0B and COM0B0 (do we invert the PWM output first or set it to the right value first?)
- Making the EEPROM a bit more redundant (CRC checks and stuff). Totaly unnecessary.

## License
__CC-BY-SA__ according to the original license.