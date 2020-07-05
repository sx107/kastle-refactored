# LFO Refactoring
This is a refactored version of the LFO source code shared in the original repo.

I'm trying to stay as close to the original code as possible to provide the same functionality. No bugfixes will be provided in this refactoring code. See LFO_Remaster for an improved & bugfixed version of the firmware.

Original code Original code by Vaclav Pelousek @ [Bastl Instruments](http://www.bastl-instruments.com) (c) 2017  

## Files description
- LFO_Refactoring.ino: main file, initializing and stuff
- lfo.h, lfo.cpp: LFO core, Timer 1 rendering interrupt
- adc.h, adc.cpp : ADC functions, ADC interrupt, parameters update
- settings.h: Settings defines

## Compiling
This code is intentded for Attiny85 running @ 8Mhz.

Compile options (with AttinyCore 1.3.3, Arduino IDE 1.8.10):
- Attiny85 @ 8MHz internal
- Millis/micros: disabled (we don't use those)
- Timer1 clock: CPU (though I don't know if it changes anything, since the timer clock source is set in timer initialization)  
- LTO: Up to you, I'd set it to enabled. It can sometimes break the code, though, but I hope that I've made everything that's needed volatile. If the code does not work, try setting it to disabled first.
- B.O.D. : disabled
- Save EEPROM: set to "EEPROM Retained" according to the fuse bits in the original firmware, though it does not matter since EEPROM is not used.

## Known bugs transferred from the original code

- _setFrequency()_: Setting the frequency too low sets the Timer1 prescaler to 1 or 0, which makes it tick either way too fast or entierly stops. See _setFrequency()_ code in lfo.cpp. Fixed with __S_LUT_FIX__
- Square output is a bit noisy, most probably this is due to a bug in Timer1 ISR. Square output is set high not only in the end of the rising slope, but also at the start of rising slope briefly.
- Only 8 bits of ADC resolution are used by bit-shifting 10 bits of ADC conversion result, though it can be done much easier and faster by setting ADLAR bit in the ADC registers.
- The code that flushes the zener diodes stray voltage is time-crucial and is sensitive to compiler and optimization settings. Replaced with asm insertion


## TODO
- lfo.cpp, Timer1 ISR: replace _digitalRead(3)_ with a direct bitRead (for some reason _bitRead(PINB, PINB3)_ does not work)

## License
__CC-BY-SA__ according to the original license.
