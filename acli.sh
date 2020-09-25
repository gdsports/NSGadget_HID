#!/bin/bash
ARDDIR=/tmp/acli_nsgadget_hid_$$
export ARDUINO_BOARD_MANAGER_ADDITIONAL_URLS="https://adafruit.github.io/arduino-board-index/package_adafruit_index.json"
export ARDUINO_DIRECTORIES_DATA="${ARDDIR}/data"
export ARDUINO_DIRECTORIES_DOWNLOADS="${ARDDIR}/downloads"
export ARDUINO_DIRECTORIES_USER="${ARDDIR}/user"
export ARDUINO_BOARD_FQBN=adafruit:samd:adafruit_trinket_m0
export ARDUINO_BOARD_FQBN2=${ARDUINO_BOARD_FQBN//:/.}
export MYPROJECT_EXAMPLES="${PWD}/examples"
export MYPROJECT_SRC="${PWD}/src"
export MYPROJECT_TOOLS="${PWD}/tools"
arduino-cli core update-index
# The latest version does not work. Use this version
arduino-cli core install adafruit:samd@1.5.14
arduino-cli core list
arduino-cli lib install Bounce2
arduino-cli lib install "Adafruit DotStar"
# Use the latest version from github to make SAMD work.
git clone https://github.com/NicoHood/HID ${ARDDIR}/user/libraries/HID-Project
export HID_PROJECT=$ARDUINO_DIRECTORIES_USER/libraries/HID-Project/src
# Merge this project's files into HID-Project
cp $MYPROJECT_SRC/SingleReport/* $HID_PROJECT/SingleReport
cp $MYPROJECT_SRC/HID-APIs/* $HID_PROJECT/HID-APIs
sed -i '/^#include "SingleReport\/SingleGamepad.h"/a #include "SingleReport/SingleNSGamepad.h"' $HID_PROJECT/HID-Project.h
# Fix VID/PID
find ${ARDUINO_DIRECTORIES_DATA} -name boards.txt -print0 | xargs -0 sed -i 's/^adafruit_trinket_m0.build.vid=0x239A$/adafruit_trinket_m0.build.vid=0x0f0d/;s/^adafruit_trinket_m0.build.pid=0x801E$/adafruit_trinket_m0.build.pid=0x00c1/'
# Disable CDC ACM
find ${ARDUINO_DIRECTORIES_DATA} -name USBDesc.h -print0 | xargs -0 sed -i 's/^#define.*CDC_ENABLED.*$/\/\/#define CDC_ENABLED/'
# Fix USB class/subclass/protocol
find ${ARDUINO_DIRECTORIES_DATA} -name USBCore.cpp -print0 | xargs -0 sed -i -e 's/bool _cdcComposite.*;/bool _cdcComposite = false;/;/if (setup.wLength == 8)/i #ifdef CDC_ENABLED' -e '/_cdcComposite = 1/a #endif'
# Compile all examples
find ${MYPROJECT_EXAMPLES} -name '*.ino' -print0 | xargs -0 -n 1 arduino-cli compile --fqbn ${ARDUINO_BOARD_FQBN}
# Convert all BIN to UF2 for drag-and-drop burning on boards with UF2 boot loader
for MYSKETCH in ${MYPROJECT_EXAMPLES}/* ; do
    pushd ${MYSKETCH}/build/${ARDUINO_BOARD_FQBN2}
    for i in *.bin ; do
        if [[ -f $i ]] ; then
            if [[ $i == *"m4"* ]] ; then
                ${MYPROJECT_TOOLS}/uf2conv.py -c -b 0x4000 $i -o $i.uf2
            else
                ${MYPROJECT_TOOLS}/uf2conv.py -c $i -o $i.uf2
            fi
        fi
    done
    FIRMWARE=${MYSKETCH}/firmware/${ARDUINO_BOARD_FQBN2}
    if [[ ! -d ${FIRMWARE} ]] ; then
        mkdir -p ${FIRMWARE}
    fi
    mv *.ino.bin.uf2 ${FIRMWARE}
    popd
done
