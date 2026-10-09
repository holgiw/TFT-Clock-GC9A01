# ENGLISH VERSION BELOW

#######################################################################################
# Flashen unter Windows (10, 11) getestet

Für ESP32-S2 (Lolin S2 Pico), ESP32-C6 und ESP32-S3 (Waveshare). Windows 7/8 haben keinen passenden
USB-Treiber und werden nicht unterstützt. macOS: flashESP.sh läuft dort nicht - von Hand mit esptool
flashen (siehe Linux-Abschnitt, Port /dev/cu.usbmodem...).

1. Die .zip Datei in ein Verzeichnis auspacken, nicht aus dem Zip heraus starten.
   flashESP.bat, setTime.bat, clocksetup.ps1, port.ps1, esptool.exe und die Unterordner esp32s2,
   esp32c6 und esp32s3 müssen zusammen liegen.
   Meldet der Virenscanner esptool.exe, ist das ein bekannter Fehlalarm (gepacktes Python-Programm):
   für diesen Ordner eine Ausnahme einrichten und das Zip erneut auspacken.

2. Die Uhr per USB am PC anstecken - mit einem Datenkabel, nicht nur einem Ladekabel.
   Möglichst direkt am PC, nicht über einen USB-Hub oder Frontanschluss: Beim Wechsel in den
   Download-Modus meldet sich der ESP32-S2 neu an, das klappt dort nicht immer.

3. flashESP.bat per Doppelklick starten.

	Uhr suchen: flashESP.bat listet alle COM Schnittstellen und markiert angeschlossene Uhren
	(Download-Modus bzw. laufend). Bei genau einer Uhr nimmt es deren Schnittstelle ohne Rückfrage,
	sonst fragt es nach der Nummer. Das Board erkennt es am USB-Port und flasht den Build aus
	esp32s2, esp32c6 bzw. esp32s3. Den ESP32-S3 erkennt es am USB-Seriell-Wandler CH343P; den gibt
	es auch auf fremden Geräten, antwortet dort keine uhr4, fragt flashESP.bat vor dem Flashen nach.

	Displaytyp:
	  ESP32-S2: 1 = GC9A01 (240x240) ohne Hintergrundbeleuchtung (BL), 2 = GC9A01 mit BL an Pin 3,
	            3 = GC9D01 (160x160), 4 = ILI9341 (240x320, Uhr mit Streifen für Uhrzeit und Datum)
	  ESP32-C6: 1 = ST7789 (172x320, ESP32-C6-LCD-1.47), 2 = ST7789_240 (240x240, ESP32-C6-LCD-1.3)
	  ESP32-S3 (ESP32-S3-LCD-1.28): keine Abfrage, das GC9A01 ist fest verbaut
	  Enter = unverändert (Update einer eingerichteten Uhr).
	Läuft auf der Uhr schon uhr4, fragt flashESP.bat ihren Displaytyp ab und wählt ihn vor (Enter
	behält ihn). Hat sie noch keinen, muss er angegeben werden. Mit falschem Displaytyp zeigt das
	Display nichts Lesbares an.

	WLAN: flashESP.bat sucht neu und listet alle sichtbaren 2,4-GHz-Netze nach Signalstärke (die
	Uhr kann nur 2,4 GHz; das Netz des PCs ist markiert). Nummer oder Name eingeben, dann das
	Passwort zweimal (verdeckt, wird nirgends gespeichert). Ist der PC mit einem 2,4-GHz-Netz
	verbunden, bietet flashESP.bat an, Name und gespeichertes Passwort zu übernehmen (Enter = ja).
	Enter überspringt - die Uhr lässt sich dann später per WPS oder Access Point einrichten.

	Flashen: Erst nach diesen Fragen flasht flashESP.bat die Uhr. Einen laufenden ESP32-S2 bringt
	es selbst in den Download-Modus (1200 Baud, wie die Arduino IDE) und flasht über die neu
	erscheinende COM Schnittstelle (meist COM4 laufend, COM3 im Download-Modus). ESP32-C6 und
	ESP32-S3 schaltet esptool selbst um.
	Danach sendet es Displaytyp und WLAN per USB an die Uhr; sie startet neu und verbindet sich
	mit dem WLAN. Zuletzt bekommt die Uhr die Uhrzeit des PCs und läuft damit sofort richtig,
	auch ohne WLAN, DCF77 und RTC (NTP/DCF77 korrigieren später).
	Das Fenster bleibt am Ende offen und wird zum seriellen Monitor: Er zeigt die Ausgabe der Uhr
	(WLAN, Zeit, Fehler) und verbindet sich nach einem Neustart der Uhr selbst wieder. Beenden mit Q
	oder Esc. Die Uhr schreibt nur dann, wenn "Logging aktivieren" in ihren Einstellungen an ist.
	Ohne Monitor: clocksetup.ps1 -Flash -NoMonitor.

	Im DOS Fenster im ausgepackten Verzeichnis geht es auch mit der COM-Nummer, z.B.
	"flashESP.bat 3" für COM3, optional mit Displaytyp: "flashESP.bat 3 GC9D01".

	Findet flashESP.bat keine Uhr, fragt es nach: Taucht die Uhr im Gerätemanager mit einem
	COM Port auf? Ist das USB-Kabel ein Datenkabel?

	Neuer ESP32-S2 ohne Programm (oder mit fremdem Programm): Er bekommt erst im Bootmodus einen
	COM Port. Boot-Taste halten, dann USB einstecken (siehe 4.) und flashESP.bat starten - es
	erkennt den ESP im Download-Modus und nimmt ihn ohne Rückfrage. Startet die Uhr nach dem
	Flashen nicht von selbst, einmal Reset drücken.

	Nur die Uhrzeit setzen (ohne Flashen): setTime.bat per Doppelklick sendet die Uhrzeit des PCs
	per USB an die laufende Uhr (auch mit COM-Nummer: "setTime.bat 4"). Für eine Uhr ohne WLAN,
	DCF77 und RTC; eine vorhandene RTC wird mitgestellt.

4. ESP32-S2 ohne Programm, oder das Flashen schlägt fehl (flashESP.bat meldet es): den ESP von
   Hand in den Bootmodus bringen und flashESP.bat erneut starten.

########################################################
	Am ESP32-S2 die Boot Taste drücken und halten.
	Erst DANACH den USB am Rechner anschließen!

ODER

	ESP32-S2 am PC per USB anstecken.
	Reset drücken und halten, Boot drücken und halten, Reset loslassen, danach Boot loslassen.
	Am PC erscheint jetzt die COM Schnittstelle des ESP, meist COM3.
########################################################


SSID einstellen:
	Am einfachsten beim Flashen: flashESP.bat fragt das WLAN ab und überträgt es per USB (siehe
	Schritt 3). Sonst:

	Am WLan Router WPS einschalten (Menü, Taster).
	Die Uhr mit USB Stromversorgung verbinden.
	Findet sie WPS, übernimmt sie die Daten vom Router.

ODER

	Findet sie keinen WPS Router, geht sie in den Accesspoint Mode.
	Dann mit dem WLAN clock123 verbinden, Passwort clocksetup (steht auch auf dem Display). Meist
	öffnet sich ein Browserfenster (Captive Portal), sonst die angezeigte IP mit HTTP aufrufen,
	z.B. http://192.168.4.1
	Nur HTTP verwenden, HTTPS funktioniert nicht!
	"Speichern" überträgt die WLAN-Daten; die Uhr startet neu und verbindet sich mit dem WLAN.

Taster (BUTTON, siehe Pinbelegung, oder der eingebaute Boot-Taster BOOT_BUTTON):
	Kurz drücken: Die Uhr zeigt das verbundene WLAN.
	10-20 Sekunden halten: Auf dem Display läuft ein gelber "WiFi Reset"-Countdown, Loslassen bricht
	ab.
	20-30 Sekunden halten: Auf dem Display läuft ein roter "Factory Reset"-Countdown. Jetzt loslassen
	löscht alle gespeicherten WLANs, die Uhr startet neu und geht in den WPS-/AccessPoint-Modus.
	Zifferblätter, Zeigersätze, Presets und Einstellungen bleiben.
	Länger als 30 Sekunden halten: vollständiger Werksreset. Er löscht die WLAN-Zugangsdaten UND
	alle hochgeladenen Zifferblätter, Zeigersätze und Presets. Danach geht die Uhr in den
	WPS-/AccessPoint-Modus.
	Nur das WLAN zurücksetzen geht auch in der Weboberfläche: Seite "Werkseinstellungen" ->
	"Gespeicherte Netzwerke zurücksetzen".

Displaytyp:
	Eine Firmware je ESP, der Displaytyp ist eine Einstellung. Festlegen:
	- beim Flashen: flashESP.bat fragt ihn ab (siehe Schritt 3),
	- in der Weboberfläche: Tab "Uhr Einstellungen", Auswahl "Display-Typ" mit denselben Einträgen
	  wie flashESP; bei einem Wechsel startet die Uhr neu,
	- beim Umstieg von uhr3: uhr3 (ab 2026-09-29) vermerkt, für welches Display es kompiliert wurde,
	  und uhr4 übernimmt das beim ersten Start - nur solange uhr4 noch keinen Displaytyp
	  gespeichert hat.
	Ohne Einstellung startet der ESP32-S2 als GC9A01. Mit falschem Typ zeigt das Display nichts
	Lesbares an.

Helligkeitssteuerung: Photowiderstand mit 10-15 kOhm und ein Widerstand mit 10 kOhm als
Spannungsteiler, versorgt über die Pins ADC_3V und ADC_GND. Beim Start prüft der ESP über
verschiedene Potentiale, ob die Bauteile vorhanden sind. Ohne Photowiderstand gilt die manuelle
Helligkeit.
	ESP32-S2: ADC_3V 1, ADC (Messung) 2, ADC_GND 4 (siehe Pinbelegung unten)
	ESP32-C6: ADC_3V 1, ADC (Messung) 2, ADC_GND 3 (Stiftleiste)
	ESP32-S3: ADC_3V 2, ADC (Messung) 4, ADC_GND 5 (Stiftleiste)
Hintergrundbeleuchtung per PWM (Haken "Hintergrundbeleuchtung regeln" im Helligkeits-Tab):
	ESP32-S2: Pin 3, bei GC9D01 und Displaytyp "GC9A01 mit BL" ab Werk an
	ESP32-C6: Pin 22, auf dem Waveshare-Board fest verdrahtet, ab Werk an
	ESP32-S3: Pin 40, auf dem Waveshare-Board fest verdrahtet, ab Werk an
Unbedingt Schaltplan und Platinenentwurf beachten: Die TFTs haben unterschiedliche Pin-Reihenfolgen.
Auf die Beschriftung achten (VCC, GND usw.).


Pinbelegung ESP32-S2 (Lolin S2 Pico)

TFT (Display 1, Pflicht):
	TFT_SCLK: 7
	TFT_MOSI: 11
	TFT_DC: 33
	TFT_RST: 5
	TFT_Backlight: 3 (Hintergrundbeleuchtung, nur wenn "Hintergrundbeleuchtung regeln (Pin 3)" im Helligkeits-Tab eingeschaltet ist - beim GC9D01 und Displaytyp "GC9A01 mit BL" ab Werk an)

Chip-Select (steuert der Sketch selbst, nicht LovyanGFX - pin_cs ist in lgfx_config.h -1):
	CS_1 (Display 1): 12
	CS_2 (Display 2, optional - in der Weboberfläche die Rotation von Display 2 von "nicht angeschlossen (n.a.)" auf einen Winkel stellen; Standard ist n.a.): 18

BUTTON: 16
BOOT_BUTTON: 0 (eingebauter Boot-Taster)
LED_BOARD: 15 (eingebaut)

ADC_3V: 1
ADC(photoresistor): 2
ADC_GND: 4

I2C / RTC (optional, DS3231 - hält die Uhrzeit über Stromausfälle, auch ohne WLAN/DCF77):
	SDA: 39
	SCL: 37

DCF77-Empfänger (optional, Funkzeit ohne Internet):
	DATA: 35


Pinbelegung ESP32-C6 (Waveshare ESP32-C6-LCD-1.47 und -1.3, beide gleich belegt)

TFT (fest verbaut, nur zur Information):
	TFT_SCLK: 7
	TFT_MOSI: 6
	TFT_DC: 15
	TFT_RST: 21
	CS_1: 14 (schaltet LovyanGFX)
	TFT_Backlight: 22
Kein zweites Display möglich (Displayleitungen nicht an der Stiftleiste).

BUTTON: 23
BOOT_BUTTON: 9 (eingebauter Boot-Taster)
LED_BOARD: 8 (eingebaute RGB-LED)

ADC_3V: 1
ADC(photoresistor): 2
ADC_GND: 3

I2C / RTC (optional, DS3231, an TX/RX der Stiftleiste):
	SDA: 16
	SCL: 17

DCF77-Empfänger (optional):
	DATA: 20


Pinbelegung ESP32-S3 (Waveshare ESP32-S3-LCD-1.28)

TFT GC9A01 (fest verbaut, nur zur Information):
	TFT_SCLK: 10
	TFT_MOSI: 11
	TFT_DC: 8
	TFT_RST: 12
	CS_1: 9 (schaltet LovyanGFX)
	TFT_Backlight: 40
Ebenfalls fest verbaut: Lagesensor QMI8658 (I2C 6/7, INT 47/48, für die Rotation "automatisch"),
Akku-Messung an GPIO 1. Keine Board-LED.

BUTTON: 16
BOOT_BUTTON: 0 (eingebauter Boot-Taster)

ADC_3V: 2
ADC(photoresistor): 4
ADC_GND: 5

I2C / RTC (optional, DS3231, am selben Bus wie der Lagesensor, Pull-ups auf dem Board):
	SDA: 6
	SCL: 7

DCF77-Empfänger (optional):
	DATA: 17

Firmware-Update über WLAN (nur ESP32-S3): Weboberfläche, Seite "Sicherung" -> "Firmware-Update",
dort esp32s3/uhr4.ino.bin hochladen (nicht die merged-, Bootloader- oder Partitionsdatei). Einstellungen,
Zifferblätter und Zeigersätze bleiben. Aus Visual Micro/Arduino IDE geht es auch über den Netzwerk-Port
der Uhr (ArduinoOTA, Port 3232).


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

# Linux neu booten oder User abmelden/anmelden!

# ESP anstecken und serielle Schnittstelle suchen, z.B. ttyACM0:
dmesg | grep tty
dmesg | tail -n 20

# für die WLAN-Auswahl in flashESP.sh: NetworkManager (nmcli), auf den meisten Desktops vorhanden


#######################################################################################
# ESP flashen mit neuer Firmware
# in das heruntergeladene Archiv wechseln, z.B.
cd build_uhr4/

# flashESP.sh sucht die Uhr selbst (USB-Kennung 303a) und nimmt sie ohne Rückfrage, wenn
# genau eine da ist. Einen laufenden ESP32-S2 bringt es in den Download-Modus (er hat dann eine
# andere Schnittstelle). Den ESP32-C6 (303a:1001) und den ESP32-S3 (CH343P, 1a86:55d3) erkennt es und
# flasht den Build aus esp32c6 bzw. esp32s3.
# Findet es keine Uhr, fragt es nach (dmesg/lsusb, Datenkabel statt Ladekabel).
bash flashESP.sh

# Ablauf wie unter Windows: Uhr suchen, Displaytyp
# (ESP32-S2: 1 = GC9A01 ohne Hintergrundbeleuchtung (BL), 2 = GC9A01 mit BL an Pin 3, 3 = GC9D01,
# 4 = ILI9341; ESP32-C6: 1 = ST7789 (1,47"), 2 = ST7789_240 (1,3"); ESP32-S3: keine Abfrage;
# Enter = unverändert; bei einer laufenden uhr4 ist ihr Typ vorgewählt), dann WLAN
# (2,4-GHz-Netze per nmcli, Passwort verdeckt, Enter = überspringen; ist der PC mit einem
# 2,4-GHz-Netz verbunden, bietet es Name und Passwort an). Nach dem Flashen sendet es beides
# per USB an die Uhr, zuletzt die Uhrzeit des PCs.

# nur die Uhrzeit des PCs an die laufende Uhr senden (ohne Flashen)
bash setTime.sh
bash setTime.sh 0

# mit fester Schnittstelle: 0 = /dev/ttyACM0 (auch ttyACM0 oder /dev/ttyACM0),
# optional mit Displaytyp
bash flashESP.sh 0
bash flashESP.sh 0 GC9D01

# von Hand (ESP32-S2 im Bootmodus: Reset und Boot drücken, Reset loslassen, kurz danach Boot)
# serielle Schnittstelle anpassen
# optional: Chip löschen (nicht empfohlen)
esptool --port /dev/ttyACM0 erase_flash

# ESP32-S2 (Build im Unterordner esp32s2)
esptool --chip esp32-s2 -p /dev/ttyACM0 -b 460800 write-flash 0x1000 esp32s2/uhr4.ino.bootloader.bin 0x8000 esp32s2/uhr4.ino.partitions.bin 0x10000 esp32s2/uhr4.ino.bin
# ESP32-C6 (Waveshare, Build im Unterordner esp32c6) - kein Bootmodus nötig. Vorher bei einer vorhandenen Uhr eine
# Sicherung ziehen: die Partitionstabelle der Firmware ist neu (2,25 MB Programm), das Dateisystem wird neu angelegt.
esptool --chip esp32c6 -p /dev/ttyACM0 -b 460800 write-flash 0x0 esp32c6/uhr4.ino.bootloader.bin 0x8000 esp32c6/uhr4.ino.partitions.bin 0xe000 esp32c6/boot_app0.bin 0x10000 esp32c6/uhr4.ino.bin
# ESP32-S3 (Waveshare ESP32-S3-LCD-1.28, Build im Unterordner esp32s3) - kein Bootmodus nötig
esptool --chip esp32s3 -p /dev/ttyACM0 -b 460800 write-flash 0x0 esp32s3/uhr4.ino.bootloader.bin 0x8000 esp32s3/uhr4.ino.partitions.bin 0xe000 esp32s3/boot_app0.bin 0x10000 esp32s3/uhr4.ino.bin


#######################################################################################
# Lizenz

uhr4 steht unter der GNU General Public License v3.0 (Datei LICENSE). Die Firmware enthält
Bibliotheken und Schriften mit eigenen Lizenzen (LovyanGFX, RTClib, arduino-esp32, DejaVu,
FreeSans, Orbitron) - Hinweise und Lizenztexte in THIRD_PARTY_LICENSES.md.
Quellcode: https://github.com/holgiw/ESP32-Station-Clock




#######################################################################################
#######################################################################################
# ENGLISH VERSION

#######################################################################################
# Flashing on Windows (10, 11) - tested

For the ESP32-S2 (Lolin S2 Pico), ESP32-C6 and ESP32-S3 (Waveshare). Windows 7/8 have no suitable USB driver
and are not supported. macOS: flashESP.sh does not run there - flash manually with esptool (see the
Linux section, port /dev/cu.usbmodem...).

1. Unpack the .zip file into a directory; do not start anything from within the zip.
   flashESP.bat, setTime.bat, clocksetup.ps1, port.ps1, esptool.exe and the subfolders esp32s2,
   esp32c6 and esp32s3 must be together.
   If the virus scanner reports esptool.exe, that is a known false alarm (packed Python program):
   add an exception for this folder and unpack the zip again.

2. Connect the clock to the PC via USB - with a data cable, not just a charging cable.
   Preferably directly to the PC, not via a USB hub or front port: when switching into download
   mode the ESP32-S2 re-enumerates, which does not always work there.

3. Start flashESP.bat by double-click.

	Finding the clock: flashESP.bat lists all COM ports and marks connected clocks (download mode
	or running). If exactly one clock is found, it uses its port without asking, otherwise it asks
	for the number. It recognizes the board by the USB port and flashes the build from esp32s2,
	esp32c6 or esp32s3. It recognizes the ESP32-S3 by the CH343P USB serial converter; as that is
	also found on other devices, flashESP.bat asks before flashing if no uhr4 replies there.

	Display type:
	  ESP32-S2: 1 = GC9A01 (240x240) without backlight (BL), 2 = GC9A01 with BL on pin 3,
	            3 = GC9D01 (160x160), 4 = ILI9341 (240x320, clock with a time and date strip)
	  ESP32-C6: 1 = ST7789 (172x320, ESP32-C6-LCD-1.47), 2 = ST7789_240 (240x240, ESP32-C6-LCD-1.3)
	  ESP32-S3 (ESP32-S3-LCD-1.28): no question, the GC9A01 is built in
	  Enter = unchanged (updating a clock that is already set up).
	If the clock already runs uhr4, flashESP.bat asks it for its display type and preselects it
	(Enter keeps it). If it has none yet, it must be chosen. With the wrong display type the
	display shows nothing readable.

	WiFi: flashESP.bat rescans and lists all visible 2.4 GHz networks by signal strength (the clock
	only supports 2.4 GHz; the PC's network is marked). Enter number or name, then the password
	twice (hidden, stored nowhere). If the PC is connected to a 2.4 GHz network, flashESP.bat
	offers to take over its name and stored password (Enter = yes).
	Enter skips - the clock can then be set up later via WPS or access point.

	Flashing: only after these questions does flashESP.bat flash the clock. It switches a running
	ESP32-S2 to download mode itself (1200 baud, like the Arduino IDE) and flashes via the COM port
	that then appears (usually COM4 running, COM3 in download mode). esptool switches the ESP32-C6
	and ESP32-S3 itself.
	Then it sends display type and WiFi to the clock via USB; the clock restarts and connects to
	the WiFi. Finally the clock gets the PC's time and runs correctly right away, even without
	WiFi, DCF77 and RTC (NTP/DCF77 correct it later).
	The window stays open at the end and becomes a serial monitor: it shows the clock's output
	(WiFi, time, errors) and reconnects by itself after the clock restarts. Quit with Q or Esc. The
	clock only writes if "Enable Logging" is on in its settings. Without monitor:
	clocksetup.ps1 -Flash -NoMonitor.

	In a Command Prompt in the unpacked folder it also works with the COM port number, e.g.
	"flashESP.bat 3" for COM3, optionally with display type: "flashESP.bat 3 GC9D01".

	If flashESP.bat finds no clock, it asks: does the clock show up in Device Manager with a
	COM port? Is the USB cable a data cable?

	New ESP32-S2 without a program (or with a foreign program): it only gets a COM port in boot
	mode. Hold the Boot button, then plug in USB (see 4.) and start flashESP.bat - it detects the
	ESP in download mode and uses it without asking. If the clock does not start by itself after
	flashing, press Reset once.

	Only set the time (without flashing): double-click setTime.bat to send the PC's time via USB to
	the running clock (also with COM number: "setTime.bat 4"). For a clock without WiFi, DCF77 and
	RTC; an existing RTC is set as well.

4. ESP32-S2 without a program, or flashing fails (flashESP.bat reports it): put the ESP into boot
   mode manually and run flashESP.bat again.

########################################################
	Press and hold the Boot button on the ESP32-S2.
	Only AFTER that, connect the USB to the computer!

OR

	Connect the ESP32-S2 to the PC via USB.
	Press and hold Reset, press and hold Boot, release Reset, then release Boot.
	The ESP's COM port now appears on the PC, usually COM3.
########################################################


Setting the SSID:
	Easiest when flashing: flashESP.bat asks for the WiFi and transfers it via USB (see step 3).
	Otherwise:

	Enable WPS on the WiFi router (button or menu).
	Connect the clock to USB power.
	If it finds WPS, it takes over the credentials from the router.

OR

	If it finds no WPS router, it switches to Access Point mode.
	Connect to the WiFi network clock123, password clocksetup (also shown on the display). A
	browser window (captive portal) usually opens, otherwise open the displayed IP address with
	HTTP, e.g. http://192.168.4.1
	Only use HTTP, HTTPS does not work!
	"Save" transmits the WiFi settings; the clock restarts and connects to the WiFi.

Button (BUTTON, see pinout, or the built-in Boot button BOOT_BUTTON):
	Short press: the clock shows the connected WiFi network.
	Hold for 10-20 seconds: a yellow "WiFi Reset" countdown runs on the display, releasing aborts.
	Hold for 20-30 seconds: a red "Factory Reset" countdown runs on the display. Releasing now deletes
	all stored WiFi networks, the clock restarts and goes into WPS/Access Point mode. Clock faces, hand
	sets, presets and settings stay.
	Hold for more than 30 seconds: full factory reset. It erases the WiFi credentials AND all
	uploaded clock faces, hand sets and presets. Afterwards the clock goes into WPS/Access Point
	mode.
	Resetting only the WiFi also works in the web interface: "Factory Reset" page -> "Reset Saved
	Networks".

Display type:
	One firmware per ESP, the display type is a setting. Set it:
	- when flashing: flashESP.bat asks for it (see step 3),
	- in the web interface: "Clock Setup" tab, "Display type" selection with the same entries as
	  flashESP; switching restarts the clock,
	- when switching from uhr3: uhr3 (from 2026-09-29) records which display it was compiled for,
	  and uhr4 takes that over at the first start - only as long as uhr4 has not stored a display
	  type yet.
	Without a setting the ESP32-S2 starts as GC9A01. With the wrong type the display shows nothing
	readable.

Brightness control: a 10-15 kOhm photoresistor and a 10 kOhm resistor as a voltage divider,
supplied via the pins ADC_3V and ADC_GND. On startup the ESP checks via different potentials
whether the components are present. Without a photoresistor the manual brightness applies.
	ESP32-S2: ADC_3V 1, ADC (measurement) 2, ADC_GND 4 (see pin assignment below)
	ESP32-C6: ADC_3V 1, ADC (measurement) 2, ADC_GND 3 (pin header)
	ESP32-S3: ADC_3V 2, ADC (measurement) 4, ADC_GND 5 (pin header)
Backlight via PWM ("Backlight control" checkbox in the Brightness tab):
	ESP32-S2: pin 3, on by default for GC9D01 and display type "GC9A01 with BL"
	ESP32-C6: pin 22, hard-wired on the Waveshare board, on by default
	ESP32-S3: pin 40, hard-wired on the Waveshare board, on by default
Be sure to follow the circuit diagram and PCB layout: the TFTs have different pin orders.
Pay attention to the labeling (VCC, GND, etc.).


ESP32-S2 (Lolin S2 Pico) Pin Assignment

TFT (Display 1, required):
	TFT_SCLK: 7
	TFT_MOSI: 11
	TFT_DC: 33
	TFT_RST: 5
	TFT_Backlight: 3 (backlight, only if "Backlight control (pin 3)" is enabled in the brightness tab - on by default for GC9D01 and display type "GC9A01 with BL")

Chip select (driven by the sketch itself, not LovyanGFX - pin_cs is -1 in lgfx_config.h):
	CS_1 (Display 1): 12
	CS_2 (Display 2, optional - in the web interface, change Display 2's rotation from "not connected (n.a.)" to an angle; default is n.a.): 18

BUTTON: 16
BOOT_BUTTON: 0 (built-in Boot button)
LED_BOARD: 15 (built-in)

ADC_3V: 1
ADC(photoresistor): 2
ADC_GND: 4

I2C / RTC (optional, DS3231 - keeps time across power loss, also without WiFi/DCF77):
	SDA: 39
	SCL: 37

DCF77 receiver (optional, radio time without internet):
	DATA: 35


ESP32-C6 (Waveshare ESP32-C6-LCD-1.47 and -1.3, both mapped alike) Pin Assignment

TFT (built in, for information only):
	TFT_SCLK: 7
	TFT_MOSI: 6
	TFT_DC: 15
	TFT_RST: 21
	CS_1: 14 (driven by LovyanGFX)
	TFT_Backlight: 22
No second display possible (display lines not on the pin header).

BUTTON: 23
BOOT_BUTTON: 9 (built-in Boot button)
LED_BOARD: 8 (built-in RGB LED)

ADC_3V: 1
ADC(photoresistor): 2
ADC_GND: 3

I2C / RTC (optional, DS3231, on TX/RX of the pin header):
	SDA: 16
	SCL: 17

DCF77 receiver (optional):
	DATA: 20


ESP32-S3 (Waveshare ESP32-S3-LCD-1.28) Pin Assignment

TFT GC9A01 (built in, for information only):
	TFT_SCLK: 10
	TFT_MOSI: 11
	TFT_DC: 8
	TFT_RST: 12
	CS_1: 9 (driven by LovyanGFX)
	TFT_Backlight: 40
Also built in: QMI8658 motion sensor (I2C 6/7, INT 47/48, for the "automatic" rotation), battery
measurement on GPIO 1. No board LED.

BUTTON: 16
BOOT_BUTTON: 0 (built-in Boot button)

ADC_3V: 2
ADC(photoresistor): 4
ADC_GND: 5

I2C / RTC (optional, DS3231, on the same bus as the motion sensor, pull-ups on the board):
	SDA: 6
	SCL: 7

DCF77 receiver (optional):
	DATA: 17

Firmware update over WiFi (ESP32-S3 only): web interface, "Backup" page -> "Firmware Update", upload
esp32s3/uhr4.ino.bin there (not the merged, bootloader or partition file). Settings, clock faces and hand
sets stay. From Visual Micro/Arduino IDE it also works via the clock's network port (ArduinoOTA, port
3232).


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

# reboot Linux, or log the user out and back in!

# connect the ESP and find the serial port, e.g. ttyACM0:
dmesg | grep tty
dmesg | tail -n 20

# for the WiFi selection in flashESP.sh: NetworkManager (nmcli), available on most desktops


#######################################################################################
# Flashing the ESP with new firmware
# change into the downloaded archive directory, e.g.
cd build_uhr4/

# flashESP.sh finds the clock itself (USB id 303a) and uses it without asking if exactly one
# is found. It switches a running ESP32-S2 to download mode (it then has a different port).
# It recognizes the ESP32-C6 (303a:1001) and the ESP32-S3 (CH343P, 1a86:55d3) and flashes the build
# from esp32c6 or esp32s3.
# If it finds no clock, it asks troubleshooting questions (dmesg/lsusb, data cable instead of charging cable).
bash flashESP.sh

# Same steps as on Windows: find the clock, display type
# (ESP32-S2: 1 = GC9A01 without backlight (BL), 2 = GC9A01 with BL on pin 3, 3 = GC9D01,
# 4 = ILI9341; ESP32-C6: 1 = ST7789 (1.47"), 2 = ST7789_240 (1.3"); ESP32-S3: no question;
# Enter = unchanged; for a running uhr4 its type is preselected), then WiFi
# (2.4 GHz networks via nmcli, password hidden, Enter = skip; if the PC is connected to a
# 2.4 GHz network, it offers its name and password). After flashing it sends both to the
# clock via USB, finally the PC's time.

# only send the PC's time to the running clock (without flashing)
bash setTime.sh
bash setTime.sh 0

# with a fixed port: 0 = /dev/ttyACM0 (also ttyACM0 or /dev/ttyACM0),
# optionally with display type
bash flashESP.sh 0
bash flashESP.sh 0 GC9D01

# manually (ESP32-S2 in boot mode: press Reset and Boot, release Reset, then Boot shortly after)
# adjust the serial port
# optional: erase the chip (not recommended)
esptool --port /dev/ttyACM0 erase_flash

# ESP32-S2 (build in the subfolder esp32s2)
esptool --chip esp32-s2 -p /dev/ttyACM0 -b 460800 write-flash 0x1000 esp32s2/uhr4.ino.bootloader.bin 0x8000 esp32s2/uhr4.ino.partitions.bin 0x10000 esp32s2/uhr4.ino.bin
# ESP32-C6 (Waveshare, build in the subfolder esp32c6) - no boot mode needed. Take a backup first on an existing
# clock: the firmware's partition table is new (2.25 MB program), the file system is created anew.
esptool --chip esp32c6 -p /dev/ttyACM0 -b 460800 write-flash 0x0 esp32c6/uhr4.ino.bootloader.bin 0x8000 esp32c6/uhr4.ino.partitions.bin 0xe000 esp32c6/boot_app0.bin 0x10000 esp32c6/uhr4.ino.bin
# ESP32-S3 (Waveshare ESP32-S3-LCD-1.28, build in the subfolder esp32s3) - no boot mode needed
esptool --chip esp32s3 -p /dev/ttyACM0 -b 460800 write-flash 0x0 esp32s3/uhr4.ino.bootloader.bin 0x8000 esp32s3/uhr4.ino.partitions.bin 0xe000 esp32s3/boot_app0.bin 0x10000 esp32s3/uhr4.ino.bin

#######################################################################################
# License

uhr4 is licensed under the GNU General Public License v3.0 (file LICENSE). The firmware contains
libraries and fonts under their own licenses (LovyanGFX, RTClib, arduino-esp32, DejaVu, FreeSans,
Orbitron) - notices and license texts in THIRD_PARTY_LICENSES.md.
Source code: https://github.com/holgiw/ESP32-Station-Clock
