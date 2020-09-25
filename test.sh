#!/bin/bash
IDEVER="1.8.13"
WORKDIR="/tmp/autobuild_$$"
mkdir -p ${WORKDIR}
# Install Ardino IDE in work directory
if [ -f ~/Downloads/arduino-${IDEVER}-linux64.tar.xz ]
then
    tar xf ~/Downloads/arduino-${IDEVER}-linux64.tar.xz -C ${WORKDIR}
else
    wget -O arduino.tar.xz https://downloads.arduino.cc/arduino-${IDEVER}-linux64.tar.xz
    tar xf arduino.tar.xz -C ${WORKDIR}
    rm arduino.tar.xz
fi
# Create portable sketchbook and library directories
IDEDIR="${WORKDIR}/arduino-${IDEVER}"
LIBDIR="${IDEDIR}/portable/sketchbook/libraries"
mkdir -p "${LIBDIR}"
export PATH="${IDEDIR}:${PATH}"
cd ${IDEDIR}
which arduino
# Install board package
arduino --pref "compiler.warning_level=default" --save-prefs
#arduino --install-boards "arduino:samd"
arduino --pref "boardsmanager.additional.urls=https://adafruit.github.io/arduino-board-index/package_adafruit_index.json" --save-prefs
arduino --install-boards "adafruit:samd"
BOARD="adafruit:samd:adafruit_trinket_m0"
arduino --board "${BOARD}" --save-prefs
CC="arduino --verify --board ${BOARD}"
# Button debouncer
arduino --install-library "Bounce2"
arduino --install-library "Adafruit DotStar"
git init
git add .
git commit -m "First draft"
# Install NicoHood HID from github
cd ${LIBDIR}
git clone https://github.com/NicoHood/HID HID-Project
ln -s ~/Sync/NSGadget_HID .
cp NSGadget_HID/src/HID-APIs/* HID-Project/src/HID-APIs
cp NSGadget_HID/src/SingleReport/* HID-Project/src/SingleReport
