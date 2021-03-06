# kastle-refactored
Refactored code of the original Bastl instruments Kastle synth firmware.


This is a refactored vesrion of the original firmware for the @bastl-instruments Kastle. I will also upload here my experiments with bastl kastle firmware (and schematics, probably).

The purpose of this project is to make original firmware code understandable, to make it easier to develop custom firmwares for the Kastle synth and make this amazing simple platform more accessible.

Original repo: https://github.com/bastl-instruments/kastle

License: CC-BY-SA

Arduino and [AttinyCore](https://github.com/SpenceKonde/ATTinyCore) were used in making this project.

### Folders description
- Hex - hex files of original Kastle VCO and LFO firmware
- VCO1.5_Refactoring - refactored version of the original Kastle 1.5 VCO
- LFO_Refactoring - refactored version of the original Kastle 1.5 LFO
- LFO_Remaster - remastered (improved and bugfixed) version of the original Kastle LFO firmware. See Readme.md for further details

### Compiling and uploading
See .ino files for any particular firmwares for additional instructions.
Use [Arduino IDE](https://www.arduino.cc/en/Main/Software) and [AttinyCore](https://github.com/SpenceKonde/ATTinyCore) to compile and upload the firmwares.
I'd recommend to not use the optiboot bootloader, since you will have to buy some sort of programmer to upload the bootloader anyway.
The cheapest possible programmer you can buy is [USBAsp](https://www.fischl.de/usbasp/), which can be easily bought in most electronic stores, including [aliexpress](aliexpress.com).

### To be done
See .ino file comments.

- [x] LFO code refactoring
- [x] LFO code remastering (__in progress__, see LFO_Remaster/README.md)
- [ ] Further VCO code bugfixes, making it easier to develop custom synths
- [ ] Custom VCO firmwares refactoring, developing some of my own
- [ ] Custom PCB gerbers
- [ ] Probably a 3d-printed case
- [ ] Eurorack PCB

Added 02.12.2020:

- [ ] Refactor Kastle drum firmware
- [ ] Remaster it and mod it :)

### Do refactored versions sound exactly the same?
Almost. I did not modify the synthesis code much in them, though for some reason these firmwares do sound a tad bit different from the original one.

- ~~The LFO speed, though, is halved compared to the original one. Compiling and uploading the LFO firmware from the original bastl instruments repo has the same effect. Probably, somewhere in the process they have changed it and forgot to update the code in repo, or I'm missing something major here.~~  __FIXED__

- VCO FM output is a bit more noisy

### Special thanks
- @bastl-instruments - for developing original Kastle synth and making it's schematics and firmware publically available under an open-source license (CC-BY-SA).
- Václav Peloušek from @bastl-instruments - for being awesome and adding a link to this repo on the bastl instruments website.
- @SpenceKonde - for developing and updating [AttinyCore](https://github.com/SpenceKonde/ATTinyCore) and helping me a little with some issues.

