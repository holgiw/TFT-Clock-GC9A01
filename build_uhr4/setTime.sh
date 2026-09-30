#!/usr/bin/env bash
# Setzt nur die Uhrzeit der laufenden Uhr auf die des PCs - per USB, ohne
# Flashen und ohne WLAN (z.B. fuer eine Uhr ohne WLAN, DCF77 und RTC).
# Aufruf: ./setTime.sh  (sucht die Uhr selbst)  oder  ./setTime.sh 0  (/dev/ttyACM0)
# Only sets the time of the running clock to the PC's - via USB, without
# flashing and without WiFi (e.g. for a clock without WiFi, DCF77 and RTC).
# Usage: ./setTime.sh  (finds the clock by itself)  or  ./setTime.sh 0  (/dev/ttyACM0)
exec bash "$(dirname "$0")/flashESP.sh" --time "$@"
