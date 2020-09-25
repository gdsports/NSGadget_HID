#!/usr/bin/python3
"""
Read from Dragon Rise Arcade Joystick and write to NSGadget.

DRAJ -> Raspberry Pi -> NSGadget -> Nintendo Switch
"""
import os
import time
from sys import exit
from struct import unpack
import threading
import array
from fcntl import ioctl
import serial
from nsgpadserial import NSGamepadSerial, NSButton, NSDPad

Nsg = NSGamepadSerial()
Nsg.begin(serial.Serial('/dev/ttyUSB0', 8*115200, timeout=0))

# Open the DRAJ
# joystick code based on https://gist.github.com/rdb/8864666
js_num = 0;
for fn in os.listdir('/dev/input'):
    if fn.startswith('js'):
        print('/dev/input/%s' % (fn))
        jsdev = open('/dev/input/' + fn, 'rb')
        buf = array.array('B', [0] * 64)
        ioctl(jsdev, 0x80006a13 + (0x10000 * len(buf)), buf) # JSIOCGNAME(len)
        js_name = buf.tobytes().rstrip(b'\x00').decode('utf-8').upper()
        print('Device name: %s' % js_name)
        if 'DRAGONRISE INC.   GENERIC   USB  JOYSTICK' in js_name:
            js_num += 1
            if js_num == 1:
                js_left = jsdev
            elif js_num == 2:
                js_right = jsdev
        else:
            jsdev.close()

if js_num < 2:
    print('DRAGONRISE joysticks not found')
    exit(1)

print('DRAGONRISE joysticks found')

# Get number of axes and buttons
buf = array.array('B', [0])
ioctl(js_left, 0x80016a11, buf) # JSIOCGAXES
num_axes = buf[0]

buf = array.array('B', [0])
ioctl(js_left, 0x80016a12, buf) # JSIOCGBUTTONS
num_buttons = buf[0]
print('left num_axes = %s num_buttons = %s' % (num_axes, num_buttons))

# Get number of axes and buttons
buf = array.array('B', [0])
ioctl(js_right, 0x80016a11, buf) # JSIOCGAXES
num_axes = buf[0]

buf = array.array('B', [0])
ioctl(js_right, 0x80016a12, buf) # JSIOCGBUTTONS
num_buttons = buf[0]
print('right num_axes = %s num_buttons = %s' % (num_axes, num_buttons))

# Map DRAJ button numbers to NS gamepad buttons
# DRAJ buttons
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
BUTTON_MAP_LEFT = array.array('B', [
    NSButton.LEFT_THROTTLE,
    NSButton.LEFT_TRIGGER,
    NSButton.MINUS,
    255,    # Up
    255,    # Right
    255,    # Down
    255,    # Left
    NSButton.LEFT_STICK,
    NSButton.CAPTURE,
    NSButton.CAPTURE,
    NSButton.CAPTURE,
    NSButton.CAPTURE])

BUTTON_MAP_RIGHT = array.array('B', [
    NSButton.RIGHT_THROTTLE,
    NSButton.RIGHT_TRIGGER,
    NSButton.PLUS,
    NSButton.A,
    NSButton.B,
    NSButton.X,
    NSButton.Y,
    NSButton.RIGHT_STICK,
    NSButton.HOME,
    NSButton.HOME,
    NSButton.HOME,
    NSButton.HOME])

BUTTONS_MAP_DPAD = array.array('B', [
    #                     LDRU
    NSDPad.CENTERED,    # 0000
    NSDPad.UP,          # 0001
    NSDPad.RIGHT,       # 0010
    NSDPad.UP_RIGHT,    # 0011
    NSDPad.DOWN,        # 0100
    NSDPad.CENTERED,    # 0101
    NSDPad.DOWN_RIGHT,  # 0110
    NSDPad.CENTERED,    # 0111
    NSDPad.LEFT,        # 1000
    NSDPad.UP_LEFT,     # 1001
    NSDPad.CENTERED,    # 1010
    NSDPad.CENTERED,    # 1011
    NSDPad.DOWN_LEFT,   # 1100
    NSDPad.CENTERED,    # 1101
    NSDPad.CENTERED,    # 1110
    NSDPad.CENTERED     # 1111
])

def read_js_left():
    dpad_bits = 0
    while True:
        evbuf_left = js_left.read(8)
        if evbuf_left:
            time, value, type, number = unpack('IhBB', evbuf_left)
            if type & 0x01: # button event
                button_out = BUTTON_MAP_LEFT[number]
                if button_out == 255:
                    if value:
                        dpad_bits |= (1 << (number - 3))
                    else:
                        dpad_bits &= ~(1 << (number - 3))
                    Nsg.dPad(BUTTONS_MAP_DPAD[dpad_bits])
                else:
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

def read_js_right():
    while True:
        evbuf_right = js_right.read(8)
        if evbuf_right:
            time, value, type, number = unpack('IhBB', evbuf_right)
            if type & 0x01: # button event
                button_out = BUTTON_MAP_RIGHT[number]
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
                    Nsg.rightXAxis(axis)
                elif number == 1:
                    Nsg.rightYAxis(axis)


while True:
    task_left = threading.Thread(target=read_js_left)
    task_right = threading.Thread(target=read_js_right)
    task_left.start()
    task_right.start()

    while True:
        time.sleep(60)
