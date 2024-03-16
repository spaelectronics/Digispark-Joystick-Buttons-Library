## Digispark Joystick Buttons Arduino Library
#Joystick Buttons Only Arduino Library for Digispark


This library is for the attiny85 running tiny core Arduino (e.g. the Digispark)

This implements a USB HID joystick (buttons only) device (upto 8 digital buttons)
The code was borrowed mostly from the Digispark Joystick library

The library can be used for push-button, momentary switches, toggle switch, magnetic contact switch (door sensor)... It handle upto 8 buttons.
As to the use of this code in Arduino, include DigiButtons.h as you would any other library. See the included sample for use of the functions.


Features
----------------------------
* Supports upto 8 Buttons
* All functions are non-blocking
* Uses watchdog feature to help prevent crashes.

Available Examples
----------------------------
* Examples/DigisparkJoystickButtons: (https://github.com/spaelectronics/Digispark-Joystick-Buttons-Library/blob/main/DigisparkJoystickButtons/examples/JoystickButtons/JoystickButtons.ino)

Available Functions
----------------------------
* DigiButtons.setButtons(Byte)
* DigiButtons.delay()
* DigiButtons.update()

Additional Information
----------------------------
Ability to use upto 8 Buttons.
However, The DigiSpark only has 4 available pins to use for buttons/switches.

We have a few issues we have to workaround. 
Pin 1 is attached to a build-in LED, which acts like a pulldown resistor, which means we cannot use the internal pullup resistor on this pin.
To workaround this issue, we can simply wire this switch/button to +5v instead of GND. 
All the other switches can be wired to GND.

Some DigiSpark clone boards keep P5 as reset pin and not yet enable P5 as a I/O pin in efuse. You may test the blink program on P5 to test if it can blink first.
To work around this issue, please see: https://thetoivonen.blogspot.com/2015/12/fixing-pin-p5-or-6-on-digispark-clones.html