# kastle-refactored
Refactored code of the original Bastl instruments Kastle synth firmware.


This is a refactored vesrion of the original firmware for the @bastl-instruments Kastle. I will also upload here my experiments with bastl kastle firmware.

The purpose of this project is to make original firmware code understandable, to make it easier to develop custom firmwares for the Kastle synth and make this amazing simple platform more accessible.

Original repo: https://github.com/bastl-instruments/kastle

License: CC-BY-SA

Arduino and [AttinyCore](https://github.com/SpenceKonde/ATTinyCore) were used in making this project.

### Description
- VCO1.5_Refactoring - refactored version of the original Kastle 1.5 VCO

### To be done
See .ino file comments.

- [ ] LFO code refactoring
- [ ] Further VCO code bugfixes, making it easier to develop custom synths
- [ ] Custom VCO firmwares refactoring, developing some of my own
- [ ] Custom PCB gerbers
- [ ] Probably a 3d-printed case

### Does it sound exactly the same?
Almost. I did not modify the synthesis code much, though for some reason this firmware does sound a tad bit different from the original one. 

### Special thanks
- @bastl-instruments - for developing original Kastle synth and making it's schematics and firmware publically available under an open-source license (CC-BY-SA);
- @SpenceKonde - for developing and updating [AttinyCore](https://github.com/SpenceKonde/ATTinyCore) and helping me a little with some issues.

