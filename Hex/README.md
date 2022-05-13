# Files description
- Original_LFO_1.2.hex - Original LFO firmware read directly from Kastle 1.2
- Original_VCO_1.2.hex - Original VCO firmware read directly from Kastle 1.2
- Original_LFO_1.5.hex - Original LFO firmware read directly from Kastle 1.5 bought in 2019
- Original_VCO_1.5.hex - Original VCO firmware read directly from Kastle 1.5 bought in 2019

Both of these firmwares are developed by [Bastl instruments](https://bastl-instruments.com/) and have nothing to do with this project. They are included since the shared under CC-BY-SA original firmware source code doesn't compile properly. Firmwares from the [production repo](https://github.com/bastl-instruments/production/tree/master/attirify succesfully with the current firmware and probably won't be updated anymore, so these firmwares are included just in case.

- Original_DrumLFO.hex - Bastle kastle drum LFO firmware
- Original_DrumVCO.hex - Bastle kastle drum VCO firmware

Copied directly from [Bastl instruments production repo](https://github.com/bastl-instruments/production/tree/master/attiny). Tested, both of these do upload succesfully.

## Fuses
Original fuses: lfuse __E2__, hfuse __DF__, efuse __FF__

## Uploading firmwares
For firmwares other than the original ones, I'd strongly recommed downloading arduino, installing attinycore and using arduino to compile and upload.
It is a much easier and more intuitive way to change the firmware, and that's why I don't and probably won't include any other firmwares in this folder.

If you want to upload the original firmware, the easiest way to install required software and config files is to, again, just to install the lastest version of arduino IDE and attinycore.

- Arduino IDE link: [Arduino](https://www.arduino.cc/en/Main/Software)
- AttinyCore: [AttinyCore github repo](https://github.com/SpenceKonde/ATTinyCore), see install guide there

Then you will have to find the avrdude and avrdude configuration files. In my case, they were located in these folders:
- Avrdude: C:\Users\$USERNAME$\AppData\Local\Arduino15\packages\arduino\tools\avrdude\6.3.0-arduino17/bin/avrdude
- Avrdude config: C:\Users\$USERNAME$\AppData\Local\Arduino15\packages\ATTinyCore\hardware\avr\1.3.3/avrdude.conf

The easiest way to find them is to enable full output during uploading in arduino settings (File->Settings) and then look at the last lines before orange ones in the output.

After you find both of these files, the hex firmware upload command with the usbasp programmer _(which I strongly recommed)_ is:

> path/to/avrdude -C path/to/avrdude.conf -v -pattiny85 -cusbasp -Uflash:w:path/to/some_firmware.hex:i

And don't forget to set the fuses first:

> path/to/avrdude -C path/to/avrdude.conf -v -pattiny85 -cusbasp -Ulfuse:w:0xE2:m -Uhfuse:w:0xDF:m  -Uefuse:w:0xFF:m 
