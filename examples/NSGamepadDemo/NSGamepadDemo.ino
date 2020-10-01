/*
  Copyright (c) 2014-2015 NicoHood
  See the readme for credit to other people.

  NSGamepadDemo example
  Press a button and demonstrate NSGamepad actions

  You can also use NSGamepad1,2,3 and 4 as single report.
  This will use 1 endpoint for each NSGamepad.
*/

#define HAS_DOTSTAR_LED (defined(ADAFRUIT_TRINKET_M0) || defined(ADAFRUIT_ITSYBITY_M0) || defined(ADAFRUIT_ITSYBITSY_M4_EXPRESS))

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

void setup() {
#if HAS_DOTSTAR_LED
  // Turn off built-in Dotstar RGB LED
  strip.begin();
  strip.clear();
  strip.show();
#endif

  // Sends a clean report to the host. This is important on any Arduino type.
  NSGamepad.begin();
}

void loop() {
  static uint8_t count = NSButton_Y;
  if (count > NSButton_Capture) {
    NSGamepad.releaseAll();
    count = NSButton_Y;
  }
  NSGamepad.press(count);
  count++;

  // Move x/y Axis to a new position (16bit)
  NSGamepad.leftXAxis(random(0xFF));
  NSGamepad.leftYAxis(random(0xFF));
  NSGamepad.rightXAxis(random(0xFF));
  NSGamepad.rightYAxis(random(0xFF));

  // Go through all dPad positions
  // values: 0-8 (0==centered)
  static uint8_t dpad = NSGAMEPAD_DPAD_UP;
  NSGamepad.dPad(dpad++);
  if (dpad > NSGAMEPAD_DPAD_UP_LEFT)
    dpad = NSGAMEPAD_DPAD_UP;

  // Functions above only set the values.
  // This writes the report to the host.
  NSGamepad.loop();
}
