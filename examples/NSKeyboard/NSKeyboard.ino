/*
 * MIT License
 *
 * Copyright (c) 2020 gdsports625@gmail.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
   Gamepad controller for Nintendo Switch.

   The gamepad has 14 buttons, 2 analog joysticks, 1 8-way direction pad.
   Compared to the Nintendo Switch Pro controller, the NSGamepad does
   not have rumble motors, motion sensors (accel/gyro), and Bluetooth.

   Do not try to run this on a Trinket M0 because it does not have enough
   pins for the buttons.

## Adafruit ItsyBitsy M0/M4 Express

NS Button   |Button Name    |ItsyBitsy Pin
------------|---------------|-------------
0           |Y              |0
1           |B              |1
2           |A              |2
3           |X              |3
4           |L              |4
5           |R              |7
6           |ZL             |9
7           |ZR             |10
8           |MINUS          |11
9           |PLUS           |12
10          |LSB#           |A3
11          |RSB#           |SCK
12          |HOME           |A4
13          |CAPTURE        |A5

# LSB/RSB = thumbsticks are also buttons

ItsyBitsy Pins Available  |Notes
--------------------------|-----
A0                        |No joysticks
A1                        |No joysticks
A2                        |No joysticks
5                         |Has 5V level shifter for WS2812 LEDs
13                        |Has LED and resistor

Direction pad requires 4 pins.

MOSI  button 1
MISO  button 0
SCL   button 2
SDA   button 3

Map QWERTY keyboard to Nintendo Switch compatible buttons.
In some games, the Playstation 4 button names are helpful.

QWERTY keyboard |NS,PS4 button
----------------|-------------
Row 1
1               |ZL,L2
2               |L,L1
3               |LSB,L3
4               |Minus
5               |Capture
6               |Home
7               |Plus
8               |RSB,R3
9               |R,R1
0               |ZR,R2
Backspace       |B,Cross
Row 2
E               |DPad Right
R               |DPad Down
U               |DPad Left
I               |DPad Up
Row 3
A               |Left stick tilted left
S               |Left stick tilted right
D               |X,Triangle
F               |Y,Square
J               |B,Cross
K               |A,Circle
L               |Right stick tilted left
;               |Right stick tilted right
Enter           |A,Circle
Row 4
V               |L,L1
N               |R,R1
Up              |DPad Up
Row 5
M1              |ZL,L2
M2              |ZR,R2
Left            |DPad Left
Down            |DPad Down
Right           |DPad Right

*/

#define HAS_DOTSTAR_LED (defined(ADAFRUIT_TRINKET_M0) || defined(ADAFRUIT_ITSYBITSY_M0) || defined(ADAFRUIT_ITSYBITSY_M4_EXPRESS))
#if HAS_DOTSTAR_LED
#include <Adafruit_DotStar.h>
#if defined(ADAFRUIT_ITSYBITSY_M4_EXPRESS)
#define DATAPIN    8
#define CLOCKPIN   6
#elif defined(ADAFRUIT_ITSYBITSY_M0)
#define DATAPIN    41
#define CLOCKPIN   40
#elif defined(ADAFRUIT_TRINKET_M0)
#define DATAPIN    7
#define CLOCKPIN   8
#endif
Adafruit_DotStar strip = Adafruit_DotStar(1, DATAPIN, CLOCKPIN, DOTSTAR_BRG);
#endif

#include "HID-Project.h"
#define BOUNCE_WITH_PROMPT_DETECTION
#include <Bounce2.h>

// BEWARE: Make sure all these pins are available on your board. Some pins
// may be connected to on board devices so cannot be used as inputs.
//#define NUM_BUTTONS 14
#define NUM_BUTTONS 10
//const uint8_t BUTTON_PINS[NUM_BUTTONS] = {0, 1, 2, 3, 4, 7, 9, 10, 11, 12, A3, SCK, A4, A5};
const uint8_t BUTTON_PINS[NUM_BUTTONS] = {2, MISO, MOSI, SCK, A5, A4, A3, A2, A1, A0};
#define NUM_DPAD 4
//const uint8_t DPAD_PINS[NUM_DPAD] = {MOSI, MISO, SCL, SDA};

// Translate 4 input pins to dpad direction values
const uint8_t DPAD_MAP[16] = {
                            // LDRU
  NSGAMEPAD_DPAD_CENTERED,  // 0000 All dpad buttons up
  NSGAMEPAD_DPAD_UP,        // 0001 direction UP
  NSGAMEPAD_DPAD_RIGHT,     // 0010 direction RIGHT
  NSGAMEPAD_DPAD_UP_RIGHT,  // 0011 direction UP RIGHT
  NSGAMEPAD_DPAD_DOWN,      // 0100 direction DOWN
  NSGAMEPAD_DPAD_CENTERED,  // 0101 invalid
  NSGAMEPAD_DPAD_DOWN_RIGHT,// 0110 direction DOWN RIGHT
  NSGAMEPAD_DPAD_CENTERED,  // 0111 invalid
  NSGAMEPAD_DPAD_LEFT,      // 1000 direction LEFT
  NSGAMEPAD_DPAD_UP_LEFT,   // 1001 direction UP LEFT
  NSGAMEPAD_DPAD_CENTERED,  // 1010 invalid
  NSGAMEPAD_DPAD_CENTERED,  // 1011 invalid
  NSGAMEPAD_DPAD_DOWN_LEFT, // 1100 direction DOWN LEFT
  NSGAMEPAD_DPAD_CENTERED,  // 1101 invalid
  NSGAMEPAD_DPAD_CENTERED,  // 1110 invalid
  NSGAMEPAD_DPAD_CENTERED,  // 1111 invalid
};

Bounce * buttons = new Bounce[NUM_BUTTONS];
#if 0
Bounce * dpad = new Bounce[NUM_DPAD];
#endif

uint8_t dpad_bits = 0;

#define DEBOUNCE  (3)

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttons[i].attach( BUTTON_PINS[i], INPUT_PULLUP );
    buttons[i].interval(DEBOUNCE);
  }
#if 0
  for (int i = 0; i < NUM_DPAD; i++) {
    dpad[i].attach( DPAD_PINS[i] , INPUT_PULLUP  );
    buttons[i].interval(DEBOUNCE);
  }
#endif

#if HAS_DOTSTAR_LED
  // Turn off built-in Dotstar RGB LED
  strip.begin();
  strip.clear();
  strip.show();
#endif

  // Sends a clean HID report to the host.
  NSGamepad.begin();
}

void loop() {
  for (int i = 0; i < NUM_BUTTONS; i++) {
    // Update the Bounce instance :
    buttons[i].update();
    // Button fell means button pressed
    if ( buttons[i].fell() ) {
      switch (i) {
        case 0:   // Semicolon
          NSGamepad.rightXAxis(255);
          break;
        case 1:   // L
          NSGamepad.rightXAxis(0);
          break;
        case 2:   // K
          NSGamepad.press(NSButton_A);  // PS4 Circle
          break;
        case 3:   // J
          NSGamepad.press(NSButton_B);  // PS4 Cross
          break;
        case 4:   // F
          NSGamepad.press(NSButton_Y);  // PS4 Square
          break;
        case 5:   // D
          NSGamepad.press(NSButton_X);  // PS4 Triangle
          break;
        case 6:   // S
          NSGamepad.leftXAxis(255);
          break;
        case 7:   // A
          NSGamepad.leftXAxis(0);
          break;
        case 8:   // Right Spacebar
          NSGamepad.press(NSButton_RightThrottle);
          break;
        case 9:   // Left Spacebar
          NSGamepad.press(NSButton_LeftThrottle);
          break;
        default:
          break;
      }
    }
    else if ( buttons[i].rose() ) {
      switch (i) {
        case 0:   // Semicolon
          NSGamepad.rightXAxis(128);
          break;
        case 1:   // L
          NSGamepad.rightXAxis(128);
          break;
        case 2:   // K
          NSGamepad.release(NSButton_A);  // PS4 Circle
          break;
        case 3:   // J
          NSGamepad.release(NSButton_B);  // PS4 Cross
          break;
        case 4:   // F
          NSGamepad.release(NSButton_Y);  // PS4 Square
          break;
        case 5:   // D
          NSGamepad.release(NSButton_X);  // PS4 Triangle
          break;
        case 6:   // S
          NSGamepad.leftXAxis(128);
          break;
        case 7:   // A
          NSGamepad.leftXAxis(128);
          break;
        case 8:   // Right Spacebar
          NSGamepad.release(NSButton_RightThrottle);
          break;
        case 9:   // Left Spacebar
          NSGamepad.release(NSButton_LeftThrottle);
          break;
        default:
          break;
      }
    }
  }
#if 0
  for (int i = 0; i < NUM_DPAD; i++) {
    // Update the Bounce instance
    dpad[i].update();
    // Button fell means button pressed
    if ( dpad[i].fell() ) {
      dpad_bits |= (1 << i);
    }
    else if ( dpad[i].rose() ) {
      dpad_bits &= ~(1 << i);
    }
  }
  NSGamepad.dPad(DPAD_MAP[dpad_bits]);
#endif

  unsigned long startTicks = millis(();
  NSGamepad.loop();
  unsigned long deltaTicks = millis() - startTicks;
  if (deltaTicks > 5) {
    digitalWrite(LED_BUILTIN, LOW);
  }
  else {
    digitalWrite(LED_BUILTIN, HIGH);
  }
}
