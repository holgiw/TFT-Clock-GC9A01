# ENGLISH VERSION BELOW

#######################################################################################
# Flashen unter Windows (10, 11) getestet

Nur für den ESP32-S2 (Lolin S2 Pico). Windows 7/8 haben keinen passenden USB-Treiber eingebaut und
werden nicht unterstützt. macOS: flashESP.sh läuft dort nicht - von Hand mit esptool flashen (siehe
Linux-Abschnitt, Port /dev/cu.usbmodem...).

1. Nach dem Download die .zip Datei in ein Verzeichnis auspacken.
   flashESP.bat, setTime.bat, clocksetup.ps1, port.ps1, esptool.exe und die .bin Dateien müssen im
   selben Verzeichnis liegen. Nicht direkt aus dem Zip heraus starten.
   Meldet der Virenscanner esptool.exe, ist das ein bekannter Fehlalarm (gepacktes Python-Programm):
   für diesen Ordner eine Ausnahme einrichten und das Zip erneut auspacken.

2. Die Uhr (ESP32-S2) per USB am PC anstecken - mit einem Datenkabel, nicht nur einem Ladekabel.
   Möglichst direkt am PC, nicht über einen USB-Hub oder Frontanschluss: beim Wechsel in den
   Download-Modus meldet sich der ESP neu an, das klappt dort nicht immer.

3. flashESP.bat per Doppelklick starten.
	flashESP.bat sucht zuerst die Uhr und fragt dann, welches Display sie hat:
	  1 = GC9A01 (240x240) ohne Hintergrundbeleuchtung (BL), 2 = GC9A01 mit BL an Pin 3, 3 = GC9D01 (160x160),
	  4 = ILI9341 (240x320, Uhr oben, darunter Uhrzeit und Datum),
	  Enter = unverändert (beim Update einer bereits eingerichteten Uhr).
	Läuft auf der Uhr schon uhr4, fragt flashESP.bat sie vorher nach ihrem Displaytyp und wählt ihn
	vor (Enter behält ihn); hat sie noch keinen, muss er angegeben werden.
	Danach fragt flashESP.bat das WLAN für die Uhr ab: es sucht neu und listet alle
	sichtbaren 2,4-GHz-Netze nach Signalstärke (die Uhr kann nur 2,4 GHz; das Netz, mit dem der PC
	verbunden ist, ist markiert), dann Nummer oder Name eingeben und das Passwort zweimal (verdeckt,
	wird nirgends gespeichert). Ist der PC selbst mit einem 2,4-GHz-Netz verbunden, bietet
	flashESP.bat an, dessen Namen und das im PC gespeicherte Passwort zu übernehmen (Enter = ja).
	Enter überspringt - die Uhr lässt sich dann später per WPS oder Access Point einrichten.
	Erst nach diesen Fragen flasht flashESP.bat die Uhr und sendet danach ohne weiteren
	Eingriff Displaytyp und WLAN per USB an die Uhr, die dann damit neu startet und
	sich direkt mit dem WLAN verbindet. Zuletzt bekommt die Uhr die Uhrzeit des PCs - sie läuft
	damit sofort richtig, auch ohne WLAN, DCF77 und RTC (NTP/DCF77 korrigieren später wie gewohnt).
	Wichtig bei einer neuen Uhr: mit falschem Displaytyp zeigt das Display nichts Lesbares an.
	flashESP.bat listet alle COM Schnittstellen auf und markiert angeschlossene Uhren
	(Download-Modus bzw. laufend). Bei genau einer gefundenen Uhr wird deren Schnittstelle
	ohne Rückfrage verwendet, sonst fragt es nach der Nummer.
	Eine laufende Uhr bringt flashESP.bat automatisch in den Download-Modus (wie die Arduino IDE
	mit 1200 Baud) und flasht über die dann neu erscheinende COM Schnittstelle
	(meist COM4 laufend / COM3 im Download-Modus).
	Das Fenster bleibt am Ende offen, damit das Ergebnis lesbar ist.

	Alternativ im DOS Fenster im ausgepackten Verzeichnis mit der Nummer der COM Schnittstelle
	aufrufen, z.B. "flashESP.bat 3" für COM3, optional mit Displaytyp: "flashESP.bat 3 GC9D01".

	Wird keine Uhr gefunden, gibt flashESP.bat Hinweise: Taucht die Uhr im Gerätemanager mit
	einem COM Port auf? Ist das USB-Kabel ein Datenkabel?

	Neuer ESP32-S2 ohne Programm (oder mit fremdem Programm): Er bekommt erst im Bootmodus einen
	COM Port. Boot-Taste halten, dann USB einstecken (siehe 4.) und flashESP.bat starten - der ESP
	wird als "Download-Modus" erkannt und ohne Rückfrage verwendet. Nach dem Flashen einmal Reset
	drücken, falls die Uhr nicht von selbst startet.

	Nur die Uhrzeit setzen (ohne Flashen): setTime.bat per Doppelklick - sendet die Uhrzeit des
	PCs per USB an die laufende Uhr (auch mit COM-Nummer: "setTime.bat 4"). Nützlich für eine Uhr
	ohne WLAN, DCF77 und RTC; eine vorhandene RTC wird mitgestellt.

4. Bei einem neuen ESP ohne Programm oder falls das Flashen fehlschlägt (flashESP.bat meldet
   es): den ESP von Hand in den Bootmodus bringen und flashESP.bat erneut starten.

########################################################
	Am ESP32-S2 die Boot Taste drücken und halten.
	Erst DANACH den USB am Rechner anschließen!

ODER

	ESP32-S2 am PC per USB anstecken.
	Reset drücken und halten, Boot drücken und halten, Reset loslassen, danach Boot loslassen.
	Am PC sollte jetzt die COM Schnittstelle des ESP auftauchen, meist COM3.
########################################################


SSID einstellen:
	Am einfachsten gleich beim Flashen: flashESP.bat fragt vor dem Flashen das WLAN ab und
	überträgt es per USB (siehe oben, Schritt 3). Sonst:

	Am WLan Router WPS einschalten (Menü, Taster)
	Den ESP32-S2 mit USB Stromversorgung verbinden.
	Wenn der ESP32-S2 WPS findet, übernimmt er die Daten vom Router.

ODER

	Findet er keinen WPS Router, geht er in den Accesspoint Mode.
	Dann bitte mit dem WLAN Netzwerk SSID clock123 verbinden - Passwort clocksetup (steht auch auf dem Display der Uhr) - es öffnet sich meist automatisch ein Browserfenster (Captive Portal), ansonsten im Browser die angezeigte IP mit HTTP aufrufen, z.b. http://192.168.4.1
	Achtung, nur HTTP verwendenden, HTTPs funktioniert nicht!
	Mit "speichern" werden die WLAN-Daten übermittelt und gespeichert, danach startet die Uhr von
	selbst neu und verbindet sich mit dem WLAN.

Wird der Taster (BUTTON, siehe Pinbelegung) im laufenden Betrieb kurz gedrückt, zeigt die Uhr das aktuell verbundene WLAN an. Wird er länger als 10 Sekunden gehalten, startet auf dem Display ein roter "Factory Reset"-Countdown - bis zu diesem Punkt passiert noch nichts, Loslassen bricht harmlos ab. Erst wenn er länger als 15 Sekunden durchgehend gehalten wird, löst das einen vollständigen Werksreset aus: dabei werden WLAN-Zugangsdaten UND alle hochgeladenen Zifferblätter/Zeigersätze/Presets gelöscht (kein reines "nur WLAN löschen" mehr). Alternativ funktioniert dafür auch der eingebaute Boot-Taster (BOOT_BUTTON). Nach einem Werksreset geht die Uhr wieder in den WPS-/AccessPoint-Modus.
Für ein WLAN-Reset ohne Verlust der eigenen Zifferblätter/Zeigersätze/Presets stattdessen in der Weboberfläche die Seite "Werkseinstellungen" nutzen ("Gespeicherte Netzwerke zurücksetzen").

Displaytyp (GC9A01 240x240, GC9D01 160x160 oder ILI9341 240x320):
	Eine Firmware für alle Displays, der Typ ist eine Einstellung. Festlegen:
	- beim Flashen: flashESP.bat fragt ihn ab (siehe oben, Schritt 3),
	- in der Weboberfläche: Tab "Uhr Einstellungen", Auswahl "Display-Typ" mit denselben vier Einträgen
	  wie flashESP (GC9A01 ohne / mit Hintergrundbeleuchtung (BL) an Pin 3, GC9D01, ILI9341); beim
	  Wechsel auf einen anderen Displaytyp startet die Uhr neu,
	- beim Umstieg von uhr3 automatisch: uhr3 (ab 2026-09-29) vermerkt, für welches Display es
	  kompiliert wurde, uhr4 übernimmt das beim ersten Start - nur solange uhr4 noch keinen
	  Displaytyp gespeichert hat; ein einmal eingestellter Typ bleibt erhalten.
	Ohne Einstellung startet die Uhr als GC9A01. Mit falschem Typ zeigt das Display nichts Lesbares an.

Für die Helligkeitssteuerung ist ein Photowiderstand mit 10-15 K Ohm notwendig, der externe Widerstand hat einen Wert von 10KOhm.
Die Portpins ADC_3V und ADC_GND versorgen den Spannungsteiler (Photowiderstand / 10kOhm Widerstand) mit der nötigen Versorgungsspannung.
Der ESP prüft beim Start automatisch durch Anlegen verschiedener Potentiale ob die Bauteile vorhanden sind.
Siehe unbedingt auch den Schaltplan und den Platinenentwurf, die TFTs sind nicht pinkompatibel in der Reihenfolge der PINs.
Unbedingt auf die Beschriftung achten (VCC, GND usw.)


Pinbelegung ESP32

TFT (Display 1, Pflicht):
	TFT_SCLK: 7
	TFT_MOSI: 11
	TFT_DC: 33
	TFT_RST: 5
	TFT_Backlight: 3 (Hintergrundbeleuchtung, nur wenn "Hintergrundbeleuchtung regeln (Pin 3)" im Helligkeits-Tab eingeschaltet ist - beim GC9D01 und Displaytyp "GC9A01 mit BL" ab Werk an)

Chip-Select (wird vom Sketch manuell angesteuert, NICHT von der LovyanGFX-Bibliothek - pin_cs ist in lgfx_config.h auf -1 gesetzt):
	CS_1 (Display 1): 12
	CS_2 (Display 2, optional - in der Weboberfläche die Rotation von Display 2 von "nicht angeschlossen (n.a.)" auf einen Winkel stellen; Standard ist n.a.): 18

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
dmesg | grep tty
dmesg | tail -n 20

# für die WLAN-Auswahl in flashESP.sh: NetworkManager (nmcli), auf den meisten Desktops vorhanden


#######################################################################################
# ESP flashen mit neuer Firmware
# in das heruntergeladene Archiv stellen, z.b.
cd build_uhr4/

# ESP flashen - flashESP.sh sucht die Uhr selbst (USB-Kennung 303a) und nimmt sie
# ohne Rückfrage, wenn genau eine gefunden wird. Eine laufende Uhr wird automatisch
# in den Download-Modus gebracht (der ESP32-S2 hat dann eine andere Schnittstelle).
# Den ESP32-C6 (303a:1001) erkennt es ebenfalls und flasht den Build aus esp32c6.
# Wird keine Uhr gefunden, gibt es Prüffragen (dmesg/lsusb, Datenkabel statt Ladekabel).
bash flashESP.sh

# Wie unter Windows: flashESP.sh sucht zuerst die Uhr, fragt dann den Displaytyp ab
# (ESP32-S2: 1 = GC9A01 ohne Hintergrundbeleuchtung (BL), 2 = GC9A01 mit BL an Pin 3, 3 = GC9D01,
# 4 = ILI9341; ESP32-C6: 1 = ST7789 (1,47"), 2 = ST7789_240 (1,3");
# Enter = unverändert; bei einer laufenden uhr4 ist ihr Typ vorgewählt), danach das WLAN
# (2,4-GHz-Netze per nmcli, Passwort verdeckt, Enter = überspringen; ist der PC mit einem
# 2,4-GHz-Netz verbunden, bietet es an, Name und Passwort zu übernehmen). Nach dem Flashen
# sendet es beides per USB an die Uhr, zuletzt die Uhrzeit des PCs.

# nur die Uhrzeit des PCs an die laufende Uhr senden (ohne Flashen)
bash setTime.sh
bash setTime.sh 0

# oder mit fester Schnittstelle: 0 = /dev/ttyACM0 (auch ttyACM0 oder /dev/ttyACM0),
# optional mit Displaytyp
bash flashESP.sh 0
bash flashESP.sh 0 GC9D01

# manuell (ESP im Bootmodus: Reset und Boot drücken, Reset loslassen und Boot kurz danach loslassen)
# hier serielle Schnittstelle anpassen
# optional: Chip löschen (nicht empfohlen)
esptool --port /dev/ttyACM0 erase_flash

# hier serielle Schnittstelle anpassen
# ESP flashen - ESP32-S2 (Build im Unterordner esp32s2)
esptool --chip esp32-s2 -p /dev/ttyACM0 -b 460800 write-flash 0x1000 esp32s2/uhr4.ino.bootloader.bin 0x8000 esp32s2/uhr4.ino.partitions.bin 0x10000 esp32s2/uhr4.ino.bin
# ESP32-C6 (Waveshare, Build im Unterordner esp32c6) - kein Bootmodus noetig
esptool --chip esp32c6 -p /dev/ttyACM0 -b 460800 write-flash 0x0 esp32c6/uhr4.ino.bootloader.bin 0x8000 esp32c6/uhr4.ino.partitions.bin 0xe000 esp32c6/boot_app0.bin 0x10000 esp32c6/uhr4.ino.bin


#######################################################################################
# Lizenz

uhr4 steht unter der GNU General Public License v3.0 (Datei LICENSE). Die Firmware enthält
Bibliotheken und Schriften mit eigenen Lizenzen (LovyanGFX, RTClib, arduino-esp32, DejaVu,
FreeSans, Orbitron) - Hinweise und Lizenztexte in THIRD_PARTY_LICENSES.md.
Quellcode: https://github.com/holgiw/TFT-Clock-GC9A01




#######################################################################################
#######################################################################################
# ENGLISH VERSION

#######################################################################################
# Flashing on Windows (10, 11) - tested

Only for the ESP32-S2 (Lolin S2 Pico). Windows 7/8 have no suitable built-in USB driver and are not
supported. macOS: flashESP.sh does not run there - flash manually with esptool (see the Linux
section, port /dev/cu.usbmodem...).

1. After downloading, unpack the .zip file into a directory.
   flashESP.bat, setTime.bat, clocksetup.ps1, port.ps1, esptool.exe and the .bin files must be
   in the same directory. Do not start it directly from within the zip.
   If the virus scanner reports esptool.exe, that is a known false alarm (packed Python program):
   add an exception for this folder and unpack the zip again.

2. Connect the clock (ESP32-S2) to the PC via USB - with a data cable, not just a charging cable.
   Preferably directly to the PC, not via a USB hub or front port: when switching into download
   mode the ESP re-enumerates, which does not always work there.

3. Start flashESP.bat by double-click.
	flashESP.bat first finds the clock and then asks which display it has:
	  1 = GC9A01 (240x240) without backlight (BL), 2 = GC9A01 with BL on pin 3, 3 = GC9D01 (160x160),
	  4 = ILI9341 (240x320, clock on top, time and date below),
	  Enter = unchanged (when updating a clock that is already set up).
	If the clock already runs uhr4, flashESP.bat asks it for its display type beforehand and
	preselects it (Enter keeps it); if it has none yet, it must be chosen.
	Then flashESP.bat asks for the WiFi for the clock: it rescans and lists all visible
	2.4 GHz networks by signal strength (the clock only supports 2.4 GHz; the network the PC is
	connected to is marked), then enter number or name and the password twice (hidden, stored
	nowhere). If the PC itself is connected to a 2.4 GHz network, flashESP.bat offers to take
	over its name and the password stored on the PC (Enter = yes).
	Enter skips - the clock can then be set up later via WPS or access point.
	Only after these questions does flashESP.bat flash the clock and then, without further
	intervention, send display type and WiFi to the clock via USB, which then restarts with them
	and connects to the WiFi directly. Finally the clock gets the PC's time - so it runs correctly
	right away, even without WiFi, DCF77 and RTC (NTP/DCF77 correct it later as usual).
	Important for a new clock: with the wrong display type the display shows nothing readable.
	flashESP.bat lists all COM ports and marks connected clocks (download mode or running).
	If exactly one clock is found, its port is used without asking, otherwise it asks for
	the number.
	A running clock is switched to download mode automatically (like the Arduino IDE with
	1200 baud) and flashed via the COM port that then appears
	(usually COM4 running / COM3 in download mode).
	The window stays open at the end so the result can be read.

	Alternatively, in a Command Prompt in the unpacked folder, run it with the COM port
	number, e.g. "flashESP.bat 3" for COM3, optionally with display type: "flashESP.bat 3 GC9D01".

	If no clock is found, flashESP.bat gives hints: Does the clock show up in Device Manager
	with a COM port? Is the USB cable a data cable?

	New ESP32-S2 without a program (or with a foreign program): it only gets a COM port in boot
	mode. Hold the Boot button, then plug in USB (see 4.) and start flashESP.bat - the ESP is
	detected as "download mode" and used without asking. After flashing, press Reset once if the
	clock does not start by itself.

	Only set the time (without flashing): double-click setTime.bat - sends the PC's time via USB
	to the running clock (also with COM number: "setTime.bat 4"). Useful for a clock without
	WiFi, DCF77 and RTC; an existing RTC is set as well.

4. With a new ESP without a program or if flashing fails (flashESP.bat reports it): put the
   ESP into boot mode manually and run flashESP.bat again.

########################################################
	Press and hold the Boot button on the ESP32-S2.
	Only AFTER that, connect the USB to the computer!

OR

	Connect the ESP32-S2 to the PC via USB.
	Press and hold Reset, press and hold Boot, release Reset, then release Boot shortly after.
	The ESP's COM port should now appear on the PC, usually COM3.
########################################################


Setting the SSID:
	Easiest right when flashing: flashESP.bat asks for the WiFi before flashing and transfers it
	via USB (see above, step 3). Otherwise:

	Enable WPS on the WiFi router (via button or menu).
	Connect the ESP32-S2 to USB power.
	If the ESP32-S2 finds WPS, it takes over the credentials from the router.

OR

	If it does not find a WPS router, it switches to Access Point mode.
	In that case, please connect to the WiFi network SSID clock123 - password clocksetup (also shown on the clock's display) - a browser window (captive portal) usually opens automatically, otherwise open the displayed IP address in your browser using HTTP, e.g. http://192.168.4.1
	Note: only use HTTP, HTTPS does not work!
	"Save" transmits and stores the WiFi settings, then the clock restarts by itself and connects
	to the WiFi.

If the button (BUTTON, see pinout) is pressed briefly during operation, the clock shows the currently connected WiFi network. If held down for more than 10 seconds, a red "Factory Reset" countdown starts on the display - up to that point nothing happens yet, releasing it aborts harmlessly. Only holding it continuously for more than 15 seconds triggers a full factory reset: this erases the WiFi credentials AND all uploaded clock faces/hand sets/presets (there is no longer a "WiFi only" reset tier). The built-in Boot button (BOOT_BUTTON) works the same way. After a factory reset, the clock goes back into WPS/Access Point mode.
For a WiFi-only reset without losing your own clock faces/hand sets/presets, use the "Factory Reset" page in the web interface instead ("Reset Saved Networks").

Display type (GC9A01 240x240, GC9D01 160x160 or ILI9341 240x320):
	One firmware for all displays, the type is a setting. Set it:
	- when flashing: flashESP.bat asks for it (see above, step 3),
	- in the web interface: "Clock Setup" tab, "Display type" selection with the same four entries
	  as flashESP (GC9A01 without / with backlight (BL) on pin 3, GC9D01, ILI9341); switching to
	  another display type restarts the clock,
	- automatically when switching from uhr3: uhr3 (from 2026-09-29) records which display it was
	  compiled for, uhr4 takes that over at the first start - only as long as uhr4 has not stored a
	  display type yet; a type once set is kept.
	Without a setting the clock starts as GC9A01. With the wrong type the display shows nothing readable.

For brightness control, a photoresistor with 10-15 kOhm is required; the external resistor has a value of 10 kOhm.
Port pins ADC_3V and ADC_GND supply the voltage divider (photoresistor / 10 kOhm resistor) with the necessary supply voltage.
On startup, the ESP automatically checks whether the components are present by applying various potentials.
Be sure to also check the circuit diagram and PCB layout - the TFTs are not pin-compatible in terms of pin order.
Be sure to pay close attention to the labeling (VCC, GND, etc.)


ESP32 Pin Assignment

TFT (Display 1, required):
	TFT_SCLK: 7
	TFT_MOSI: 11
	TFT_DC: 33
	TFT_RST: 5
	TFT_Backlight: 3 (backlight, only if "Backlight control (pin 3)" is enabled in the brightness tab - on by default for GC9D01 and display type "GC9A01 with BL")

Chip select (driven manually by the sketch, NOT by the LovyanGFX library - pin_cs is set to -1 in lgfx_config.h):
	CS_1 (Display 1): 12
	CS_2 (Display 2, optional - in the web interface, change Display 2's rotation from "not connected (n.a.)" to an angle; default is n.a.): 18

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
dmesg | grep tty
dmesg | tail -n 20

# for the WiFi selection in flashESP.sh: NetworkManager (nmcli), available on most desktops


#######################################################################################
# Flashing the ESP with new firmware
# change into the downloaded archive directory, e.g.
cd build_uhr4/

# flash the ESP - flashESP.sh finds the clock itself (USB id 303a) and uses it
# without asking if exactly one is found. A running clock is switched to download
# mode automatically (the ESP32-S2 then has a different port).
# It also recognizes the ESP32-C6 (303a:1001) and flashes the build from esp32c6.
# If no clock is found, it asks troubleshooting questions (dmesg/lsusb, data cable instead of charging cable).
bash flashESP.sh

# As on Windows: flashESP.sh first finds the clock, then asks for the display type
# (ESP32-S2: 1 = GC9A01 without backlight (BL), 2 = GC9A01 with BL on pin 3, 3 = GC9D01,
# 4 = ILI9341; ESP32-C6: 1 = ST7789 (1.47"), 2 = ST7789_240 (1.3");
# Enter = unchanged; for a running uhr4 its type is preselected), then for the WiFi
# (2.4 GHz networks via nmcli, password hidden, Enter = skip; if the PC is connected to a
# 2.4 GHz network, it offers to take over its name and password). After flashing it sends
# both to the clock via USB, finally the PC's time.

# only send the PC's time to the running clock (without flashing)
bash setTime.sh
bash setTime.sh 0

# or with a fixed port: 0 = /dev/ttyACM0 (also ttyACM0 or /dev/ttyACM0),
# optionally with display type
bash flashESP.sh 0
bash flashESP.sh 0 GC9D01

# manually (ESP in boot mode: press Reset and Boot, release Reset, then release Boot shortly after)
# adjust the serial port here
# optional: erase the chip (not recommended)
esptool --port /dev/ttyACM0 erase_flash

# adjust the serial port here
# flash the ESP - ESP32-S2 (build in the subfolder esp32s2)
esptool --chip esp32-s2 -p /dev/ttyACM0 -b 460800 write-flash 0x1000 esp32s2/uhr4.ino.bootloader.bin 0x8000 esp32s2/uhr4.ino.partitions.bin 0x10000 esp32s2/uhr4.ino.bin
# ESP32-C6 (Waveshare, build in the subfolder esp32c6) - no boot mode needed
esptool --chip esp32c6 -p /dev/ttyACM0 -b 460800 write-flash 0x0 esp32c6/uhr4.ino.bootloader.bin 0x8000 esp32c6/uhr4.ino.partitions.bin 0xe000 esp32c6/boot_app0.bin 0x10000 esp32c6/uhr4.ino.bin

#######################################################################################
# License

uhr4 is licensed under the GNU General Public License v3.0 (file LICENSE). The firmware contains
libraries and fonts under their own licenses (LovyanGFX, RTClib, arduino-esp32, DejaVu, FreeSans,
Orbitron) - notices and license texts in THIRD_PARTY_LICENSES.md.
Source code: https://github.com/holgiw/TFT-Clock-GC9A01
