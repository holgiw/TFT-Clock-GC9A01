@echo off
rem Setzt nur die Uhrzeit der laufenden Uhr auf die des PCs - per USB, ohne
rem Flashen und ohne WLAN (z.B. fuer eine Uhr ohne WLAN, DCF77 und RTC).
rem Doppelklick: sucht die Uhr selbst. Aufruf mit COM-Nummer: setTime.bat 4
rem Ablauf in clocksetup.ps1 (-Time).
rem Only sets the time of the running clock to the PC's - via USB, without
rem flashing and without WiFi (e.g. for a clock without WiFi, DCF77 and RTC).
rem Double-click: finds the clock by itself. Usage with COM number: setTime.bat 4
rem Flow in clocksetup.ps1 (-Time).
setlocal
cd /d "%~dp0"
if not exist "%~dp0clocksetup.ps1" (
    echo clocksetup.ps1 fehlt - das Zip zuerst komplett in einen Ordner auspacken und dort starten.
    echo clocksetup.ps1 is missing - first unpack the whole zip into a folder and start it there.
    pause
    exit /b 1
)
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0clocksetup.ps1" -Time -Port "%~1"
if "%~1"=="" pause
