# LFO Refactoring
This is a refactored version of the LFO source code shared in the original repo.
Later, I'll also add the DUAL_STEP firmware in this same source code by utilizing defines.

I'm trying to stay as close to the original code as possible to provide the same functionality. No bugfixes will be provided in this refactoring code - I'll make them in a separate "remastered" version of the firmware.

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

## Settings in settings.h

- __S_ADC_FIX__ In the VCO code and in DUAL_STEP code a software fix for the zener diodes holding some stray voltage is provided by briefly switching the ADC inputs to digital outputs pulled to GND every second ADC conversion. In the original LFO firmware, though, it is not done and it breaks the code at low LFO values (see bugs) and is not utilized in the original LFO code.
- __S_LUT_FIX__ As said before, the __S_ADC_FIX__ define can break the code at low values. In the DUAL_STEP firmware original code this is fixed by changing the look-up table of ADC readings, and it does help, since the mapped ADC output never reaches the "breaking" values.

In the original LFO code both __S_ADC_FIX__ and __S_LUT_FIX__ are "unset". Though, the LFO frequency is then almost half the frequency in the firmware directly read from uC at mid-values.

Setting both __S_ADC_FIX__ and __S_LUT_FIX__ seems to fix the LFO frequency issue, perhaps they just forgot to update the ADC ISR and LUT in the original code.

| Code | S_ADC_FIX | S_LUT_FIX |
| --- | --- | --- |
| Original LFO code | Disabled | Disabled |
| "Fixed" LFO code, frequency closer to the original | Enabled | Enabled |

## Known bugs transferred from the original code

- _setFrequency()_: Setting the frequency too low sets the Timer1 prescaler to 1 or 0, which makes it tick either way too fast or entierly stops. See _setFrequency()_ code in lfo.cpp. Fixed with __S_LUT_FIX__
- Square output is a bit noisy, most probably this is due to a bug in Timer1 ISR. Square output is set high not only in the end of the rising slope, but also at the start of rising slope briefly.
- In the original shared code LFO frequency is approx. half the frequency in the original firmware. Fixed by setting __S_ADC_FIX__ and __S_LUT_FIX__.
- Only 8 bits of ADC resolution are used by bit-shifting 10 bits of ADC conversion result, though it can be done much easier and faster by setting ADLAR bit in the ADC registers.

## TODO
- Add DUAL_STEP firmware capabilities in this one by utilizing defines.
- Re-check the frequency issue: maybe, __S_ADC_FIX__ and __S_LUT_FIX__ should be enabled by default?
- lfo.cpp, Timer1 ISR: replace _digitalRead(3)_ with a direct bitRead (for some reason _bitRead(PINB, PINB3)_ does not work)

## License
__CC-BY-SA__ according to the original license.
