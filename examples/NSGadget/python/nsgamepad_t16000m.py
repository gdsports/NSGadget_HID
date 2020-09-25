#!/usr/bin/python3
"""
Read from Thrustmaster T.16000M FCS and write to NSGadget.

T16K -> Raspberry Pi -> NSGadget -> Nintendo Switch
"""
import os
from sys import exit
from struct import unpack
import array
from fcntl import ioctl
import serial
from nsgpadserial import NSGamepadSerial, NSButton, NSDPad

Nsg = NSGamepadSerial()
Nsg.begin(serial.Serial('/dev/ttyUSB0', 8*115200, timeout=0))

# Open the T16K
# joystick code based on https://gist.github.com/rdb/8864666
T16K = False
for fn in os.listdir('/dev/input'):
    if fn.startswith('js'):
        print('/dev/input/%s' % (fn))
        jsdev = open('/dev/input/' + fn, 'rb')
        buf = array.array('B', [0] * 64)
        ioctl(jsdev, 0x80006a13 + (0x10000 * len(buf)), buf) # JSIOCGNAME(len)
        js_name = buf.tobytes().rstrip(b'\x00').decode('utf-8').upper()
        print('Device name: %s' % js_name)
        if 'THRUSTMASTER T.16000M' in js_name:
            T16K = True
            break
        else:
            jsdev.close()

if not T16K:
    print('THRUSTMASTER T.16000M not found')
    exit(1)

print('THRUSTMASTER T.16000M found')

# Get number of axes and buttons
buf = array.array('B', [0])
ioctl(jsdev, 0x80016a11, buf) # JSIOCGAXES
num_axes = buf[0]
print('num_axes = %s' % num_axes)

buf = array.array('B', [0])
ioctl(jsdev, 0x80016a12, buf) # JSIOCGBUTTONS
num_buttons = buf[0]
print('num_buttons = %s' % num_buttons)

# Map T16K button numbers to NS gamepad buttons
# T16K buttons
# 0 = front trigger
# 1 = side thumb rest button
# 2 = top large left
# 3 = top large right
# 4 = top small left
# 5 = top small right
#
# Button array (2 rows, 3 columns) on base
#
# 7 9 11
# 6 8 10
#
BUTTON_MAP = array.array('B', [
    NSButton.A,
    NSButton.B,
    NSButton.X,
    NSButton.Y,
    NSButton.LEFT_TRIGGER,
    NSButton.RIGHT_TRIGGER,
    NSButton.MINUS,
    NSButton.PLUS,
    NSButton.CAPTURE,
    NSButton.HOME,
    NSButton.LEFT_THROTTLE,
    NSButton.RIGHT_THROTTLE,
    NSButton.LEFT_THROTTLE,
    NSButton.RIGHT_THROTTLE,
    NSButton.LEFT_THROTTLE,
    NSButton.RIGHT_THROTTLE])

left_stick_pressed = False
right_stick_pressed = False

while True:
    evbuf = jsdev.read(8)
    if evbuf:
        time, value, type, number = unpack('IhBB', evbuf)
        if type & 0x01: # button event
            button_out = BUTTON_MAP[number]
            if value:
                Nsg.press(button_out)
            else:
                Nsg.release(button_out)

        if type & 0x02: # axis event
            # NS wants values 0..128..255 where 128 is center position
            axis = ((value + 32767) >> 8)
            if axis == 127:
                axis = 128
            # Axes 0,1 left stick X,Y
            if number == 0:
                Nsg.leftXAxis(axis)
            elif number == 1:
                Nsg.leftYAxis(axis)
            # Axis 2 twist
            elif number == 2:
                if axis == 0:
                    Nsg.press(NSButton.LEFT_STICK)
                    left_stick_pressed = True
                elif axis == 255:
                    Nsg.press(NSButton.RIGHT_STICK)
                    right_stick_pressed = True
                else:
                    if left_stick_pressed:
                        Nsg.release(NSButton.LEFT_STICK)
                        left_stick_pressed = False
                    if right_stick_pressed:
                        Nsg.release(NSButton.RIGHT_STICK)
                        right_stick_pressed = False
            # Axis 3 throttle lever
#            elif number == 3:
#                Find a use for this!
            # Axes 4,5 directional pad X,Y
            elif number == 4:
                Nsg.rightXAxis(axis)
            elif number == 5:
                Nsg.rightYAxis(axis)
