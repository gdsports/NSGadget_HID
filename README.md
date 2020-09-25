# Nintendo Switch Gamepad

Create Nintendo Switch compatible gamepads using Arduino and Adafruit SAMD
boards. This project depends on the NicoHood Arduino HID library.

This project works on Arduino and Adafruit SAMD boards but has not been tested
in AVR boards.

This project is not a library. The code in the src directory must be copied
into the HID-Project library. Be sure to install the latest version of
HID-Project from github. The version installed using Library Manager is too old
for SAMD boards.

Copy the source code files in the src directory to directories with the same
names in HID-Project library installation directory. See NSGamepad/README.md
for details.

The Adafruit SAMD board package version 1.5.14 must be used for Adafruit SAMD
boards. Version 1.6.0 and newer do not work with this project.

If using a Linux system, acli.sh uses (Arduino
CLI)[https://arduino.github.io/arduino-cli/latest/] to build the code. It also
applies patches required to make NSGadget compatible as possible with a real NS
gamepad. acli.sh creates the build directory in /tmp to avoid changing the
default Arduino directories. This is done because patches to SAMD board files
are required.
