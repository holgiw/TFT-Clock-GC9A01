@echo off
rem Flasht die Uhr. Doppelklick: sucht zuerst die Uhr, fragt Displaytyp und WLAN
rem ab, flasht sie und sendet danach die Einstellungen und die Uhrzeit des PCs
rem per USB. Nur die Uhrzeit setzen: setTime.bat.
rem Aufruf mit Parametern ohne Rueckfragen: flashESP.bat 3  (fuer COM3),
rem optional mit Displaytyp: flashESP.bat 3 GC9D01  (ESP32-S2: 1/GC9A01,
rem 2/GC9A01_WITH_BACKLIGHT, 3/GC9D01, 4/ILI9341; ESP32-C6: 1/ST7789, 2/ST7789_240).
rem Das Board erkennt das Skript am USB-Port, die Builds liegen in esp32s2 bzw. esp32c6.
rem Ablauf in clocksetup.ps1, Portsuche und Umschalten einer laufenden Uhr in den
rem Download-Modus in port.ps1.
rem Flashes the clock. Double-click: first finds the clock, asks for the display
rem type and WiFi, flashes it and afterwards sends the settings and the PC's time
rem via USB. Only set the time: setTime.bat.
rem Usage with parameters without questions: flashESP.bat 3  (for COM3),
rem optionally with display type: flashESP.bat 3 GC9D01  (ESP32-S2: 1/GC9A01,
rem 2/GC9A01_WITH_BACKLIGHT, 3/GC9D01, 4/ILI9341; ESP32-C6: 1/ST7789, 2/ST7789_240).
rem The script recognizes the board by the USB port, the builds are in esp32s2 or esp32c6.
rem Flow in clocksetup.ps1, port search and switching a running clock into download
rem mode in port.ps1.
setlocal
cd /d "%~dp0"
if not exist "%~dp0clocksetup.ps1" (
    echo clocksetup.ps1 fehlt - das Zip zuerst komplett in einen Ordner auspacken und dort starten.
    echo clocksetup.ps1 is missing - first unpack the whole zip into a folder and start it there.
    pause
    exit /b 1
)
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0clocksetup.ps1" -Flash -Port "%~1" -Display "%~2"
if "%~1"=="" pause
