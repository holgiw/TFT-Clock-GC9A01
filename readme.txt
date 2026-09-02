# ENGLISH VERSION BELOW

#######################################################################################
# Flashen unter Windows (10,11) getestet


Nach dem Download die .zip Datei in ein Verzeichnis auspacken.
DOS Fenster öffnen und mit cd in das Verzeichnis stellen.


Wenn der ESP bereits eine Vorgängerversion der Uhr hatte diesen Block ausführen:

########################################################
	ESP32-S2 am PC per USB anstecken..
	In der start.bat Schnittstelle auf COM4 stellen

########################################################

ansonsten

########################################################
Der ESP in den Bootmodus bringen:


	Am ESP32-S2 die Boot Taste drücken und halten.
	Erst DANACH den USB am Rechner anschließen!

ODER

	ESP32-S2 am PC per USB anstecken.
	Reset drücken und halten, Boot drücken und halten, Reset loslassen, danach Boot loslassen.
	Am PC sollte jetzt die COM Schnittstelle des ESP auftauchen, es ist meist COM3.
########################################################


In der Batch Datei start.bat muss evtl. die serielle COM Schnittstelle angepasst und gespeichert werden (COM3/COM4).
Danach ist im DOS Fenster die start.bat auszuführen und flasht den ESP32 S2.

SSID einstellen:
	Am WLan Router WPS einschalten (Menü, Taster)
	Den ESP32-S2 mit USB Stromversorgung verbinden.
	Wenn der ESP32-S2 WPS findet, übernimmt er die Daten vom Router.

ODER

	Findet er keinen WPS Router, geht er in den Accesspoint Mode.
	Dann bitte mit dem WLAN Netzwerk SSID clock123 (PASS clock123) verbinden - es öffnet sich meist automatisch ein Browserfenster (Captive Portal), ansonsten im Browser die angezeigte IP mit HTTP aufrufen, z.b. http://192.168.4.1
	Achtung, nur HTTP verwendenden, HTTPs funktioniert nicht!
	Mit SAVE werden die geänderten Werte für das WLAN übermittelt und gespeichert.
	RESET startet die Uhr neu.

Wird der Taster (BUTTON, siehe Pinbelegung) beim Einschalten gedrückt gehalten (ca. 10 Sekunden), werden ALLE gespeicherten WLAN-Zugangsdaten gelöscht und die Uhr geht in den AccessPoint Mode. Alternativ funktioniert dafür auch der eingebaute Boot-Taster (BOOT_BUTTON).
Findet er anschließend einen Router im WPS Mode übernimmt er die WLAN Daten, bootet neu und versucht diese Verbindung herzustellen.

Für die Helligkeitssteuerung ist ein Photowiderstand mit 10-15 K Ohm notwendig, der externe Widerstand hat einen Wert von 10KOhm.
Die Portpins ADC_3V und ADC_GND versorgen den Spannungsteiler (Photowiderstand / 10kOhm Widerstand) mit der nötigen Versorgungsspannung.
Der ESP prüft beim Start automatisch durch Anlegen verschiedener Potentiale ob die Bauteile vorhanden sind.
Siehe unbedingt auch den Schaltplan und den Platinenentwurf, die TFTs sind nicht pinkompatibel in der Reihenfolge der PINs.
Unbedingt auf die Beschriftung achten (VCC, GND usw.)


Pinbelegung ESP32

TFT (Display 1, Pflicht):
	TFT_SCLK: 4
	TFT_MOSI: 6
	TFT_DC: 10
	TFT_RST: 0
	TFT_BL: 5 (Hintergrundbeleuchtung, nur bei GC9D01 bzw. GC9A01_WITH_BACKLIGHT relevant)

Chip-Select (wird vom Sketch manuell angesteuert, NICHT von der TFT_eSPI-Bibliothek - dort muss TFT_CS in der User_Setup.h auf -1 gesetzt sein):
	CS_1 (Display 1): 12
	CS_2 (Display 2, optional - "Display 2 aktivieren" in der Weboberfläche): 18

BUTTON: 16
BOOT_BUTTON: 0 (eingebauter Boot-Taster, siehe oben)
LED_BOARD: 15 (eingebaut)

ADC_3V: 1
ADC(photoresistor): 2
ADC_GND: 4

I2C / RTC (optional, DS3231 - hält die Uhrzeit auch ohne WLAN/DCF77 über Stromausfälle hinweg):
	SDA: 39
	SCL: 37

DCF77-Empfänger (optional, Funkuhr-Zeitsynchronisation ohne Internet):
	DATA: 35


#######################################################################################
# Flashen unter Linux (LMDE)

# einmalige Vorbereitungen:
# User zur dialout Gruppe hinzufügen
sudo usermod -a -G dialout $USER

# python3 installieren
sudo apt install python3-pip

# esptool installieren
sudo pip3 install esptool --break-system-packages

# udev installieren
sudo apt update && sudo apt upgrade udev

# Linux neu booten oder User abmelden/anmelden !!!

# ESP anstecken und serielle Schnittstelle suchen mit:
# z.b ttyACM0
dmesg | grep ttyUSB
dmesg | tail -n 20


#######################################################################################
# ESP flashen mit neuer Firmware
# in das heruntergeladene Archiv stellen, z.b.
cd esp32.esp32.lolin_s2_pico/

# ESP im Bootmodus (Reset und Boot drücken, Reset loslassen und Boot kurz danach loslassen)

# hier serielle Schnittstelle anpassen
# optional: Chip löschen (nicht empfohlen)
esptool --port /dev/ttyACM0 erase_flash

# hier serielle Schnittstelle anpassen
# ESP flashen
esptool --chip esp32-s2 -p /dev/ttyACM0 -b 460800 write-flash 0x1000 uhr3.ino.bootloader.bin 0x8000 uhr3.ino.partitions.bin 0x10000 uhr3.ino.bin




#######################################################################################
#######################################################################################
# ENGLISH VERSION

#######################################################################################
# Flashing on Windows (10, 11) - tested


After downloading, unpack the .zip file into a directory.
Open a Command Prompt (DOS window) and cd into that directory.


If the ESP already had a previous version of the clock, run this block:

########################################################
	Connect the ESP32-S2 to the PC via USB..
	In start.bat, set the interface to COM4

########################################################

otherwise

########################################################
Put the ESP into boot mode:


	Press and hold the Boot button on the ESP32-S2.
	Only AFTER that, connect the USB to the computer!

OR

	Connect the ESP32-S2 to the PC via USB.
	Press and hold Reset, press and hold Boot, release Reset, then release Boot shortly after.
	The ESP's COM port should now appear on the PC, usually COM3.
########################################################


In the batch file start.bat, the serial COM port may need to be adjusted and saved (COM3/COM4).
Then run start.bat in the Command Prompt to flash the ESP32-S2.

Setting the SSID:
	Enable WPS on the WiFi router (via button or menu).
	Connect the ESP32-S2 to USB power.
	If the ESP32-S2 finds WPS, it takes over the credentials from the router.

OR

	If it does not find a WPS router, it switches to Access Point mode.
	In that case, please connect to the WiFi network SSID clock123 (PASSWORD clock123) - a browser window (captive portal) usually opens automatically, otherwise open the displayed IP address in your browser using HTTP, e.g. http://192.168.4.1
	Note: only use HTTP, HTTPS does not work!
	SAVE transmits and stores the changed WiFi settings.
	RESET restarts the clock.

If the button (BUTTON, see pinout) is held down while powering on (approx. 10 seconds), ALL stored WiFi credentials are erased and the clock enters Access Point mode. The built-in Boot button (BOOT_BUTTON) works the same way.
If it then finds a router in WPS mode, it takes over the WiFi credentials, reboots, and tries to establish that connection.

For brightness control, a photoresistor with 10-15 kOhm is required; the external resistor has a value of 10 kOhm.
Port pins ADC_3V and ADC_GND supply the voltage divider (photoresistor / 10 kOhm resistor) with the necessary supply voltage.
On startup, the ESP automatically checks whether the components are present by applying various potentials.
Be sure to also check the circuit diagram and PCB layout - the TFTs are not pin-compatible in terms of pin order.
Be sure to pay close attention to the labeling (VCC, GND, etc.)


ESP32 Pin Assignment

TFT (Display 1, required):
	TFT_SCLK: 4
	TFT_MOSI: 6
	TFT_DC: 10
	TFT_RST: 0
	TFT_BL: 5 (backlight, only relevant for GC9D01 / GC9A01_WITH_BACKLIGHT)

Chip select (driven manually by the sketch, NOT by the TFT_eSPI library - its User_Setup.h must have TFT_CS set to -1):
	CS_1 (Display 1): 12
	CS_2 (Display 2, optional - "Enable Display 2" in the web interface): 18

BUTTON: 16
BOOT_BUTTON: 0 (built-in Boot button, see above)
LED_BOARD: 15 (built-in)

ADC_3V: 1
ADC(photoresistor): 2
ADC_GND: 4

I2C / RTC (optional, DS3231 - keeps time across power loss even without WiFi/DCF77):
	SDA: 39
	SCL: 37

DCF77 receiver (optional, radio-clock time sync without internet):
	DATA: 35


#######################################################################################
# Flashing on Linux (LMDE)

# one-time preparations:
# add the user to the dialout group
sudo usermod -a -G dialout $USER

# install python3
sudo apt install python3-pip

# install esptool
sudo pip3 install esptool --break-system-packages

# install udev
sudo apt update && sudo apt upgrade udev

# reboot Linux, or log the user out and back in !!!

# connect the ESP and find the serial port with:
# e.g. ttyACM0
dmesg | grep ttyUSB
dmesg | tail -n 20


#######################################################################################
# Flashing the ESP with new firmware
# change into the downloaded archive directory, e.g.
cd esp32.esp32.lolin_s2_pico/

# put the ESP into boot mode (press Reset and Boot, release Reset, then release Boot shortly after)

# adjust the serial port here
# optional: erase the chip (not recommended)
esptool --port /dev/ttyACM0 erase_flash

# adjust the serial port here
# flash the ESP
esptool --chip esp32-s2 -p /dev/ttyACM0 -b 460800 write-flash 0x1000 uhr3.ino.bootloader.bin 0x8000 uhr3.ino.partitions.bin 0x10000 uhr3.ino.bin
