/////////////////////////////////////////////////////////////////////////
//              V-USB Joystick Library for Buttons Only.               //
//                     Tested on DigiSpark Only.                       //
/////////////////////////////////////////////////////////////////////////

// Ability to use upto 8 Buttons.
// However, The DigiSpark only has 4 available pins to use for buttons/switches.

// We have a few issues we have to workaround. Pin 1 is attached to a build-in LED, 
// which acts like a pulldown resistor, which means we cannot use the internal pullup resistor on this pin.
// To workaround this issue, we can simply wire this switch/button to +5v instead of GND. 
// All the other switches can be wired to GND.

// Some DigiSpark clone boards keep P5 as reset pin and not yet enable P5 as a I/O pin in efuse. You may test the blink program on P5 to test if it can blink first.
// To work around this issue, please see: https://thetoivonen.blogspot.com/2015/12/fixing-pin-p5-or-6-on-digispark-clones.html

// Sets how many joystick buttons you want to show up on the pc.
#define ButtonQty 4  // Range: 1-8


#include "DigiButtons.h"

void setup() {
  pinMode(0, INPUT_PULLUP);  // Button 1
  pinMode(1, INPUT);         // Button 2  // This pin is attached to the Built-in LED, so we have to connect your switch to the +5v line, instead of GND. We cannot use the internal pullup here.
  pinMode(2, INPUT_PULLUP);  // Button 3
  pinMode(5, INPUT_PULLUP);  // Button 4  // Some DigiSpark clone boards keep P5 as reset pin and not yet enable P5 as a I/O pin in efuse. You may test the blink program on P5 to test if it can blink first.
}


void loop() {
  // If not using plentiful DigiButtons.delay() calls, make sure to
  // DigiButtons.update(); // call this at least every 50ms
  // calling more often than that is fine
  // this will actually only send the data every once in a while unless the data is different


  // The USB Protocol required buttons to be reported as a 1 byte (8 bits) value.
  // Think of each bit, as a button. The bits should look like the examples below.

  // If button 1 is pressed, it look like this:    [00000001].
  // If button 2 is pressed, it should look like:  [00000010].
  // If button 1 & 2 are pressed at the same time: [00000011].
  // If button 1 & 3 are pressed at the same time: [00000101].
  
  // Now we use << (bitshift left) to convert the button values to Hex, like this:
  unsigned char report = (!digitalRead(0)) + (digitalRead(1) << 1) + (!digitalRead(2) << 2) + (!digitalRead(5) << 3) + (0 << 4) + (0 << 5) + (0 << 6) + (0 << 7);    // Each button uses 1 bit, however, USB Protocol requires 8 bits, so we must pass 0's for the unused bits.
  DigiButtons.setButtons(report); // Now we send the value to the USBButton Library.

  // it's best to use DigiButtons.delay() because it knows how to talk to
  // the connected computer - otherwise the USB link can crash with the
  // regular arduino delay() function
  DigiButtons.delay(50); // wait 50 milliseconds


  
  // We can also set buttons like this: DigiButtons.setButtons(Byte)
  //DigiButtons.setButtons((char)0x00);                               // You can use a Binary to Hex converter to get the Hex value.

  // If your only using 1 button, you can use the following:
  //DigiButtons.setButtons((char)digitalRead(0) ? 0x00 : 0x01);
}