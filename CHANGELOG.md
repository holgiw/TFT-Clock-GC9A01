# Changelog

*English version below.*

## 2026-10-08

### Hinzugefügt
- Eigener Build für den Waveshare ESP32-S3-LCD-1.28 (rundes GC9A01 fest verbaut, 16 MB Flash, 2 MB PSRAM),
  Ordner `build_uhr4/esp32s3`; noch nicht am Gerät erprobt:
  - Board „ESP32S3 Dev Module“, Flash Size 16MB, PSRAM „QSPI PSRAM“, Partition Scheme „16M Flash (3MB APP/9.9MB
    FATFS)“, USB CDC On Boot „Disabled“ (USB läuft über den Wandler CH343P). Falsche Einstellungen brechen den
    Build mit einer Meldung ab.
  - Zwei App-Partitionen, LittleFS mit 9,9 MB auf der Partition „ffat“.
  - Firmware-Update über WLAN: Seite „Sicherung“ → „Firmware-Update“ (`uhr4.ino.bin` hochladen, mit
    Fortschrittsbalken) oder ArduinoOTA aus Visual Micro/Arduino IDE (Port 3232, ohne Passwort). Die Uhr
    prüft Kopf und Chip der Datei, lehnt merged-, Bootloader- und Partitionsdateien ab und startet die neue
    Firmware erst nach vollständiger Prüfung. Während des Updates zeigt das Display den Fortschritt, die
    übrige Uhr pausiert, WLAN-Scans unterbleiben, das Funk-Energiesparen ist aus; hängt ein Update, startet
    die Uhr nach 3 Minuten neu.
  - Beleuchtung an GPIO 40, ab Werk geregelt. Fotowiderstand an GPIO 2/4/5, Taster 16, DCF77 17, RTC am I2C
    6/7 des eingebauten Lagesensors. Das Board hat keine LED, die Option „DCF77 Sync LED Blink“ entfällt.
  - Rotation „automatisch“ (Tab „Uhr Einstellungen“, nur dieses Board): Der Lagesensor QMI8658 dreht das
    Zifferblatt aufrecht, sobald eine neue Lage 1,5 s anliegt; flach liegend bleibt die Rotation. Beim
    Einschalten nimmt die Uhr die aktuelle Lage als Bezug für die gerade eingestellte Rotation. Die
    Statusseite zeigt Lage und Bezug.
  - `flashESP.bat`/`flashESP.sh` erkennen das Board am CH343P (USB-Kennung 1A86:55D3), fragen keinen
    Displaytyp ab und flashen den Build aus `esp32s3`. Antwortet am Wandler keine uhr4, fragen sie vorher
    nach.
- Uhrzeit aus offenen WLANs (alle Builds, Tab „NTP Zeitzone“, ab Werk an): Verbindet sich die Uhr beim Start
  mit keinem gespeicherten WLAN (außer Reichweite oder kein Zugang, z. B. falsches Passwort), verbindet sie sich
  ohne RTC kurz mit bis zu vier offenen WLANs (ab -85 dBm, gleiche Namen nur einmal, je 15 s) und holt die
  Zeit: NTP, dann der `Date:`-Kopf einer HTTP-Antwort (erst ohne DNS: Gateway, 1.1.1.1; dann per Name),
  zuletzt einmal der Weiterleitung zur Anmeldeseite folgen, per HTTPS (TLS ohne Zertifikatsprüfung) und HTTP.
  Danach trennt sie wieder und geht wie mit RTC in den normalen Uhrenbetrieb, ohne WPS und Access Point; das
  gespeicherte WLAN bleibt und wird mit „WLAN neu verbinden“ stündlich wieder versucht. Das Log nennt jeden
  Schritt: Grund, Netz, IP, Weg und Ergebnis. Kostet etwa 100 KB Flash (TLS).
- Zifferblatt-Designer, Streifen Uhrzeit/Datum: Die Einstellungen stehen in drei Gruppen Uhrzeit, Datum und
  Wochentag. Jede Gruppe hat „anzeigen“, einen Regler „Größe“ und die Position (automatisch oder X/Y); dazu
  Zeitformat, Sekunden und Blinken bei der Uhrzeit, das Datumsformat beim Datum.
  - Neu: Wochentag ausgeschrieben in der Sprache der Weboberfläche (Deutsch, sonst Englisch), ab Werk aus.
    Automatisch stehen die eingeschalteten Zeilen in der Reihenfolge Uhrzeit, Wochentag, Datum mit gleichen
    Abständen; quer steht der Wochentag vor dem Datum.
  - Größe bei eingebauten Schriften 50–200 %, bei Designer-Schriften (VLW) 8–120 px (bisher zwei Zahlenfelder,
    nur VLW). Eine VLW-Größe geht erst beim Loslassen an die Uhr, weil jede Größe eine eigene Datei ist. Der
    Wochentag bekommt eine eigene VLW-Datei mit Buchstaben (`stripfont_<Name>_<Größe>_wd.vlw`); fehlt sie, nimmt
    er DejaVu.
  - Neues Datumsformat T.MM.JJ.
  - Der Streifen bleibt leer, bis eine aktuelle Zeit vorliegt (NTP, RTC, DCF77, offenes WLAN oder
    Rocrail-Modellzeit) – vorher stand dort „0:00“.
- Bis eine Zeit vorliegt, stehen Stunden-, Minuten- und Sekundenzeiger auf 12 (bisher liefen sie ab 10:10:30).
  Kommt die Zeit, laufen sie zur Uhrzeit.
- Helligkeit mit PWM (Hintergrundbeleuchtung): Solange die Uhr beim Start auf Daten wartet (noch keine Zeit,
  WPS, Access Point), leuchtet sie mit 50 % statt voll.
- Die Gamma-Korrektur (Eingabe und Kurve) erscheint nur noch mit Lichtsensor - ohne ihn wirkt sie nicht.
- ESP32-C6: Liegt die maximale Helligkeit über 128, warnt die Helligkeitsseite vor Überhitzung der
  Beleuchtung (Hinweis von Waveshare), schon beim Eintippen.
- Der Streifen gehört zum Zifferblatt: Neben der Grafik (`strip_<Name>.bmp`) hat jedes Zifferblatt seine
  Streifen-Einstellungen in `stripcfg_<Name>.txt` (Lage, Farben, Schrift, Größen, Formate, Positionen,
  ein/aus). „Streifen speichern“ legt sie zum aktiven Zifferblatt ab, „Speichern“ im Designer zum
  gespeicherten. Ein Zifferblatt ohne eigene Einstellungen bekommt den Standard (weiß, schwarze Schrift
  FreeSans Bold, Uhrzeit und Datum automatisch); „Standard“ im Designer setzt dieselben Werte.
  - Löschen und Umbenennen eines Zifferblatts nehmen Grafik und Einstellungen mit. Beim Start löscht die Uhr
    Streifen ohne Zifferblatt.
  - Die Sicherung enthält die Einstellungen. Von einem anderen Displaytyp mit Streifen rechnet die Uhr sie um:
    Positionen im Verhältnis von Breite bzw. Höhe, Größen mit dem kleineren Verhältnis, VLW-Größen auf die
    nächste vorhandene Datei. Die Grafiken lässt sie dabei aus (anderes Seitenverhältnis). Displays ohne
    Streifen übernehmen keine Streifen-Dateien, auch keine Schriften.
  - Uhren Sets speichern keine Streifen-Werte mehr, sie kommen über das Zifferblatt. Beim ersten Start
    übernimmt die Uhr die bisherigen Einstellungen einmal zum aktiven Zifferblatt und die Streifen-Werte
    vorhandener Uhren Sets zu deren Zifferblättern.

### Geändert
- Jeder Start beginnt ohne Uhrzeit: Die Systemzeit überstand bisher einen Neustart per Reset-Taste oder
  Software und wird jetzt am Anfang verworfen. Die Uhr holt sie wie nach einem Stromausfall neu (NTP, RTC,
  DCF77, offenes WLAN, USB).
- Displayauswahl (Tab „Uhr Einstellungen“): ILI9341 und ST7789 172 × 320 heißen „mit Streifen für Uhrzeit und
  Datum“ statt „mit Uhrzeit und Datum unter der Uhr“ – der Streifen kann auch über der Uhr liegen.
- Taster (oder Boot-Taster) bekommt eine Stufe für das WLAN: ab 10 s läuft ein gelber Countdown „WiFi Reset“
  über 10 s (Loslassen bricht ab), ab 20 s ein roter Countdown „Factory Reset“ über 10 s. Wer in dieser Zeit
  loslässt, löscht alle gespeicherten WLANs und startet neu; der vollständige Werksreset kommt erst nach
  30 s (bisher 15 s). Jede Stufe steht im Log.

### Behoben
- Taster und Boot-Taste reagierten nicht, solange die Uhr auf eine WLAN-Verbindung wartete (je 30 s, beim
  Start und bei jedem Reconnect), offene WLANs abfragte, im WPS-Countdown stand oder auf DCF77 wartete. Sie
  werden jetzt auch dort abgefragt. Außerdem versuchte die Uhr direkt nach einem gescheiterten Start sofort
  noch einmal 60 s lang, sich zu verbinden; der erste Reconnect-Versuch kommt jetzt nach 5 Minuten, danach
  stündlich. Neuer USB-Befehl `UHR4 PINS` (Diagnose: Pegel von Taster und Boot-Taste).
- Access-Point-Modus nach dem Start (gespeichertes WLAN nicht erreichbar): 2 Minuten nach Beginn schaltete ein
  Verbindungsversuch den AP ab – scheiterte er, war die Uhr bis zum nächsten stündlichen Versuch weder im
  WLAN noch per AP erreichbar. Jetzt prüft die Uhr auch im AP-Modus nur stündlich (mit „WLAN neu
  verbinden“) und startet den AP nach einem gescheiterten Versuch sofort wieder; klappt die Verbindung,
  beendet sie den AP-Modus. Im laufenden Betrieb geht die Uhr weiterhin nie in den AP-Modus.

## 2026-10-07

### Geändert
- Die Nabe (Mittelpunkt über den Zeigern) wird im Zifferblatt-Designer eingestellt: Radius und Farbe
  (Farbwähler) mit Live-Vorschau. Die Uhr zeigt Änderungen sofort, „Nabe speichern“ legt sie ab. Das Formular
  „Mittelpunkt“ auf der Seite „Zeiger“ entfällt.

## 2026-10-06

### Hinzugefügt
- Knopf „Uhrzeit übernehmen“: Hat die Uhr keine Uhrzeit (kein WLAN/NTP, keine RTC, kein DCF77, z. B. im
  Access-Point-Modus), zeigt ihn die Statusleiste. Er stellt die Uhr auf die Zeit des Handys bzw. PCs, eine
  vorhandene RTC gleich mit. Derselbe Knopf steht im Tab „NTP Zeitzone“.

### Geändert
- GitHub-Repository umbenannt in `holgiw/ESP32-Station-Clock` (vorher `TFT-Clock-GC9A01`; der Name passte nicht
  mehr zu den verschiedenen Displays und Boards). GitHub leitet alte Links weiter, Firmware und Doku verweisen
  auf den neuen Namen.
- Startpaket einer neuen Uhr:
  - `face_default.bmp` hat die Ziffern 12, 3, 6 und 9.
  - Ohne Zifferblätter und Zeigersätze erzeugt die Uhr dazu `face_numbers.bmp` (1–12), `face_roman.bmp`
    (I–XII), die Zeigersätze 1 (Balken, schwarzer Sekundenzeiger) und 2 (geschwungen) und, wenn keine Presets
    da sind, die Presets „Standard“, „1-12“ und „I-XII“ (ohne Sekundenzeiger).
  - Die Werksreset-Aktionen für Zifferblätter, Zeigersätze und Presets stellen diesen Zustand wieder her.
  - Passen die erzeugten Dateien nach einem Wechsel des Displaytyps nicht zur Uhrgröße, erzeugt die Uhr sie
    beim Start neu.
  - Displays mit Streifen (ILI9341, ST7789 172 × 320): Die drei Presets haben einen weißen Streifen mit
    schwarzer Schrift in FreeSans Bold, DejaVu bzw. der Designer-Schrift „Sans“ (falls auf der Uhr, sonst
    GLCD). Eine neue Uhr startet mit dem Streifen von „Standard“.
- Sicherung von einem anderen Displaytyp wiederherstellen: Die Uhr weist darauf hin und übernimmt nur
  Zifferblätter, Zeiger (auf ihre Größe skaliert), Uhren Sets und allgemeine Einstellungen. Die Nabengröße
  (auch in den Uhren Sets) rechnet sie um, die Nabenfarbe kommt mit. Displaytyp, Rotation,
  Hintergrundbeleuchtung und Helligkeit bleiben. Sind die Zifferblätter der Sicherung nicht 240 × 240 groß,
  übernimmt sie nichts und meldet das. Hat die Sicherung keinen Streifen, bekommt eine Uhr mit Streifen den
  Standard-Streifen (weiß, schwarze Schrift, FreeSans Bold). Bisher lehnte die Uhr eine andere Uhrgröße ab und
  übernahm bei gleicher Größe (z. B. GC9A01 und ILI9341) alles.
- Sicherung: Der Displaytyp steht immer darin, auch wenn er nie gespeichert wurde (Werkseinstellung). Sonst
  nahm die Zieluhr beim Wiederherstellen ihre eigene Werkseinstellung an. Ältere Sicherungen ohne diesen
  Eintrag gelten als GC9A01 (Werkseinstellung des ESP32-S2).
- Sicherung: Der Dateiname enthält die Uhrzeit mit Sekunden (z. B.
  `uhr4-backup-clock_23F6A4-ST7789-20261006-175812.tar`), damit sich mehrere Sicherungen eines Tages nicht
  überschreiben.
- `flashESP.sh` (Linux) kennt den ESP32-C6: erkennt ihn am USB-Port (303a:1001), bietet ST7789 und ST7789_240
  an und flasht den Build aus `esp32c6` mit `boot_app0.bin`, ohne Download-Modus.
- ST7789 172 × 320: Start- und Statusmeldungen stehen quer (90° gedreht) in doppelter Schriftgröße.
- Hochladen:
  - Fehlt im Dateinamen `face_` (Zifferblatt) oder `hand_set` (Zeiger), ergänzt die Uhr es, z. B.
    `bahnhof.bmp` → `face_bahnhof.bmp`, `set3_hour.bmp` oder `3_hour.bmp` → `hand_set3_hour.bmp`. `.BMP` wird
    klein geschrieben.
  - Die Hinweise unter den Upload-Formularen nennen, was die Uhr annimmt: BMP mit 16, 24 oder 32 Bit,
    höchstens so groß wie der freie Speicher (angezeigt). Die Uhr skaliert und wandelt in RGB565 um.
- Presets speichern, ob die Rocrail-Modellzeit an ist und ob die Uhr das WLAN neu verbindet (`rocrail=` und
  `wifiReconnect=true/false` in `/api/setMode`). Serveradressen und WLAN-Daten bleiben Einstellungen der Uhr.
- Sichern und Wiederherstellen zeigen einen Fortschrittsbalken. Solange die Uhr beschäftigt ist, pausiert die
  Statusleiste, statt „Verbindung verloren“ zu melden. Nach dem Wiederherstellen wartet die Seite, bis die Uhr
  nach dem Neustart wieder antwortet.

### Entfernt
- Knopf „Presets von GitHub laden“ auf der Seite Uhren Sets, die Nachfrage bei leerer Liste und die nur
  dafür genutzte Route `/importpresetsmerge`.
- Link auf die ZIP-Dateien (`faces_handsets_240.zip` / `_160.zip`) auf der Seite Zifferblätter.

### Behoben
- Hochladen: 16-Bit-BMPs im Format RGB555 haben die richtigen Farben. Nicht unterstützte BMPs (Palette,
  8/4/1 Bit, komprimiert) lehnt die Uhr ab, statt sie als schwarzes Bild zu speichern; die abgelehnte Datei
  bleibt nicht liegen.
- Zeigersätze hochladen: Ein dort hochgeladenes Zifferblatt (`face_…`) landete trotz Fehlermeldung auf der
  Uhr. Die Zeiger-Seite nimmt nur noch Zeiger an.
- Wiederherstellen: Die Warnung bei anderem Displaytyp im Dateinamen der Sicherung greift (das Datum mit
  Bindestrich wurde nicht erkannt) und kennt die ST7789-Typen.
- ESP32-C6: Nach einem Neustart durch die Software (Wiederherstellen, Neustart-Knopf, flashESP) blieb das
  Display schwarz, nur Strom aus/an half. CS liegt nicht mehr dauerhaft auf LOW, LovyanGFX schaltet es je
  Übertragung.
- Uhren Sets: Vorschaubilder fehlten oft. Der Browser forderte alle gleichzeitig an, und der Uhr (vor allem
  ohne PSRAM) ging der Speicher aus. Die Seite lädt sie nacheinander mit bis zu drei Versuchen; die Uhr sendet
  jedes Bild zeilenweise statt als zweite Kopie im RAM.
- Zifferblatt-Designer (Displays mit Streifen): Füllen bleibt im angeklickten Bereich, Zifferblatt oder
  Streifen. Bisher lief z. B. der Rand um ein rundes Zifferblatt in den Streifen weiter und färbte ihn mit.
- ESP32-C6 (ohne PSRAM): Die Streifen-Grafik eines Zifferblatts (`strip_….bmp`) fehlte, weil für ihre Kopie im
  RAM der Speicher nicht reichte; der Streifen blieb einfarbig. Die Uhr liest sie zeilenweise aus der Datei,
  auf dem Display und in den Vorschauen.

## 2026-10-05

### Hinzugefügt
- Erste Unterstützung für Waveshare ESP32-C6-LCD-1.47 (ST7789, 172 × 320) und ESP32-C6-LCD-1.3 (ST7789,
  240 × 240) als eigener Build:
  - Displaytypen ST7789 (Uhr 172 × 172 mit Streifen für Uhrzeit und Datum) und ST7789_240.
  - Pinbelegung in `config.h` (`BOARD_WAVESHARE_C6_ST7789`, am Chip erkannt; beide Boards gleich belegt), kein
    zweites Display.
  - Zifferblätter von GitHub kommen aus dem 240er-Ordner und werden beim Hochladen verkleinert.
  - Der Linker-Schalter `-mtext-section-literals` gilt nur für den ESP32-S2 (`board.txt`,
    `platform.local.txt`).
  - Ohne PSRAM hält die Uhr das Zifferblatt nur komprimiert (oder liest es aus der Datei) und zeichnet jedes
    Bild daraus neu. Hochladen skaliert zeilenweise ohne Vollbild im RAM; Vorschauen (Uhren Sets,
    Vorschau-Seite) verkleinern bzw. streamen zeilenweise.
  - `flashESP.bat` erkennt den C6 am USB-Port und flasht den Build aus `build_uhr4\esp32c6`.
  - Der Build braucht „USB CDC On Boot: Enabled“, sonst bricht er mit einer Meldung ab.

## 2026-10-04

### Behoben
- README: Hinweis auf die englische Fassung direkt unter dem Titel; den Verweis auf das nicht mehr vorhandene
  Release „Version 3“ gibt es nicht mehr.

## 2026-10-02

### Geändert
- Sicherung: Zifferblätter, Zeiger und Streifen-Grafiken liegen darin als normale BMP-Bilder (vorher im
  komprimierten Format der Uhr) und öffnen in jedem Bildprogramm. Beim Wiederherstellen packt die Uhr sie wie
  beim Hochladen; ältere Sicherungen lassen sich weiter einspielen.
- Screenshots (`screenshots/de`, `screenshots/en`) neu aufgenommen: die Seite DCF77 von einer Uhr mit
  Empfänger, wieder der Zifferblatt-Designer einer ILI9341-Uhr (`zifferblatt_designer_ili.png`) und neu deren
  Vorschau mit dem Uhrzeit-/Datumsstreifen (`vorschau_ili.png`).
- Auswahl der mitgelieferten Zifferblätter, Zeiger und Presets überarbeitet (`graphic/` mit
  `faces_handsets_160.zip` und `faces_handsets_240.zip`, `presets.txt`).

### Behoben
- Selbst gezeichnetes Standard-Zifferblatt: Auf runden Displays bleibt alles außerhalb des Kreises weiß wie bei
  hochgeladenen Zifferblättern. Es übersteht Sichern und Wiederherstellen Pixel für Pixel unverändert.
- `presets.txt`: Das Preset „Antik“ wird beim Herunterladen von GitHub wieder übernommen. Name und Adresse
  waren durch ein Leerzeichen statt eines Tabs getrennt, deshalb wurde die Zeile übersprungen.

---

# English Version

## 2026-10-08

### Added
- Separate build for the Waveshare ESP32-S3-LCD-1.28 (round GC9A01 built in, 16 MB flash, 2 MB PSRAM), folder
  `build_uhr4/esp32s3`; not yet tried on the device:
  - Board "ESP32S3 Dev Module", Flash Size 16MB, PSRAM "QSPI PSRAM", Partition Scheme "16M Flash (3MB APP/9.9MB
    FATFS)", USB CDC On Boot "Disabled" (USB runs via the CH343P converter). Wrong settings stop the build with
    a message.
  - Two app partitions, LittleFS with 9.9 MB on the "ffat" partition.
  - Firmware update over WiFi: "Backup" page → "Firmware Update" (upload `uhr4.ino.bin`, with a progress
    bar) or ArduinoOTA from Visual Micro/Arduino IDE (port 3232, without password). The clock checks header
    and chip of the file, rejects merged, bootloader and partition files and only starts the new firmware
    after a complete check. During the update the display shows the progress, the rest of the clock pauses,
    WiFi scans are skipped, radio power saving is off; if an update hangs, the clock restarts after 3
    minutes.
  - Backlight on GPIO 40, controlled by default. Photoresistor on GPIO 2/4/5, button 16, DCF77 17, RTC on the
    I2C 6/7 of the built-in motion sensor. The board has no LED, the "DCF77 Sync LED Blink" option is omitted.
  - Rotation "automatic" ("Clock Setup" tab, this board only): the QMI8658 motion sensor turns the clock
    face upright once a new position lasts 1.5 s; lying flat, the rotation stays. When switching it on, the
    clock takes the current position as the reference for the rotation set right now. The status page shows
    position and reference.
  - `flashESP.bat`/`flashESP.sh` recognize the board by the CH343P (USB id 1A86:55D3), do not ask for a display
    type and flash the build from `esp32s3`. If no uhr4 replies on the converter, they ask first.
- Time from open WiFis (all builds, "NTP Timezone" tab, on by default): if the clock connects to no stored
  WiFi at boot (out of range or no access, e.g. a wrong password), the clock without an RTC briefly connects
  to up to four open WiFis (from -85 dBm, each name once, 15 s each) and gets the time: NTP, then the `Date:`
  header of an HTTP reply (first without DNS: gateway, 1.1.1.1; then by name), finally follows the redirect to
  the login page once, via HTTPS (TLS without certificate check) and HTTP. Then it disconnects again and, as
  with an RTC, goes into normal clock operation without WPS and access point; the stored WiFi stays and is
  retried hourly with "Reconnect WiFi". The log names every step: reason, network, IP, way and result. Costs
  about 100 KB of flash (TLS).
- Clock face designer, time/date strip: the settings are in three groups time, date and weekday. Each group
  has "show", a "Size" slider and the position (automatic or X/Y); plus time format, seconds and blinking for
  the time, the date format for the date.
  - New: weekday written out in the language of the web interface (German, otherwise English), off by default.
    In automatic mode the switched-on lines are ordered time, weekday, date with equal gaps; in landscape the
    weekday comes before the date.
  - Size for built-in fonts 50–200 %, for designer fonts (VLW) 8–120 px (previously two number fields, VLW
    only). A VLW size is only sent when the slider is released, since each size is a file of its own on the
    clock. The weekday gets its own VLW file with letters (`stripfont_<name>_<size>_wd.vlw`); without it, it
    uses DejaVu.
  - New date format D.MM.YY.
  - The strip stays empty until a current time is available (NTP, RTC, DCF77, open WiFi or Rocrail model
    time) – previously it showed "0:00".
- Until a time is available, hour, minute and second hands stand at 12 (previously they ran from 10:10:30).
  When the time arrives, they run to the time.
- Brightness with PWM (backlight): while the clock waits for data at boot (no time yet, WPS, access point), it
  shines at 50 % instead of full.
- Gamma correction (input and curve) only appears with a light sensor - without one it has no effect.
- ESP32-C6: if the maximum brightness is above 128, the brightness page warns of the backlight overheating
  (note from Waveshare), already while typing.
- The strip belongs to the clock face: besides the graphic (`strip_<name>.bmp`) each clock face has its strip
  settings in `stripcfg_<name>.txt` (placement, colours, font, sizes, formats, positions, on/off). "Save
  strip" stores them for the active clock face, "Save" in the designer for the saved one. A clock face without
  its own settings gets the default (white, black text FreeSans Bold, time and date automatic); "Default" in
  the designer sets the same values.
  - Deleting and renaming a clock face takes graphic and settings along. At start the clock deletes strips
    without a clock face.
  - The backup contains the settings. From another display type with a strip the clock converts them:
    positions by the ratio of width or height, sizes by the smaller ratio, VLW sizes to the nearest available
    file. It leaves out the graphics (other aspect ratio). Displays without a strip take over no strip files,
    no fonts either.
  - Presets no longer store strip values, they come with the clock face. At the first start the clock takes
    over the previous settings once for the active clock face and the strip values of existing presets for
    their clock faces.

### Changed
- Every boot starts without a time: the system time used to survive a restart via the reset button or
  software and is now discarded at the start. The clock gets it anew as after a power cut (NTP, RTC, DCF77,
  open WiFi, USB).
- Display selection ("Clock Setup" tab): ILI9341 and ST7789 172 × 320 read "with time and date strip" instead
  of "with time and date below the clock" – the strip can also be above the clock.
- Button (or Boot button) gets a stage for the WiFi: from 10 s a yellow "WiFi Reset" countdown runs for 10 s
  (releasing aborts), from 20 s a red "Factory Reset" countdown for 10 s. Releasing during it deletes all
  stored WiFi networks and restarts; the full factory reset now only comes after 30 s (previously 15 s).
  Every stage is logged.

### Fixed
- The button and boot button did not react while the clock waited for a WiFi connection (30 s each, at boot
  and on every reconnect), queried open WiFis, was in the WPS countdown or waited for DCF77. They are now
  checked there too. Also, right after a failed boot the clock immediately tried again for 60 s; the first
  reconnect attempt now comes after 5 minutes, then hourly. New USB command `UHR4 PINS` (diagnostics: levels
  of the button and boot button).
- Access point mode after boot (stored WiFi not reachable): 2 minutes after the start a connection attempt
  switched the AP off – if it failed, the clock was reachable neither via WiFi nor via AP until the next
  hourly attempt. Now the clock also checks only hourly in AP mode (with "Reconnect WiFi") and restarts the
  AP right after a failed attempt; if the connection works, it leaves AP mode. During operation the clock
  still never enters AP mode.

## 2026-10-07

### Changed
- The hub (centre over the hands) is set in the clock face designer: radius and colour (colour picker) with
  live preview. The clock shows changes right away, "Save hub" stores them. The "Centre point" form on the
  "Hand Set" page is gone.

## 2026-10-06

### Added
- "Use device time" button: if the clock has no time (no WiFi/NTP, no RTC, no DCF77, e.g. in access point
  mode), the status bar shows it. It sets the clock to the time of the phone or PC, and an existing RTC as
  well. The same button is in the "NTP Timezone" tab.

### Changed
- GitHub repository renamed to `holgiw/ESP32-Station-Clock` (previously `TFT-Clock-GC9A01`; the name no longer
  fit the different displays and boards). GitHub redirects old links, firmware and docs point to the new name.
- Starter set of a new clock:
  - `face_default.bmp` has the numerals 12, 3, 6 and 9.
  - Without clock faces and hand sets the clock also generates `face_numbers.bmp` (1–12), `face_roman.bmp`
    (I–XII), hand sets 1 (bars, black second hand) and 2 (curved) and, if there are no presets, the presets
    "Standard", "1-12" and "I-XII" (without a second hand).
  - The factory reset actions for clock faces, hand sets and presets restore this state.
  - If the generated files do not fit the clock size after a display type change, the clock regenerates them
    at startup.
  - Displays with a strip (ILI9341, ST7789 172 × 320): the three presets have a white strip with black text
    in FreeSans Bold, DejaVu or the designer font "Sans" (if on the clock, otherwise GLCD). A new clock starts
    with the strip of "Standard".
- Restoring a backup of another display type: the clock points this out and only takes over clock faces,
  hands (scaled to its size), presets and general settings. It converts the hub size (also in the presets),
  the hub colour comes along. Display type, rotation, backlight and brightness stay. If the backup's clock
  faces are not 240 × 240, it takes over nothing and says so. If the backup has no strip, a clock with a strip
  gets the standard strip (white, black text, FreeSans Bold). Before, the clock rejected a different clock
  size and restored everything with the same size (e.g. GC9A01 and ILI9341).
- Backup: the display type is always included, even if it was never stored (factory default). Otherwise the
  target clock assumed its own factory default when restoring. Older backups without this entry count as
  GC9A01 (the ESP32-S2's factory default).
- Backup: the file name contains the time with seconds (e.g.
  `uhr4-backup-clock_23F6A4-ST7789-20261006-175812.tar`), so several backups on one day do not overwrite each
  other.
- `flashESP.sh` (Linux) knows the ESP32-C6: recognizes it by the USB port (303a:1001), offers ST7789 and
  ST7789_240 and flashes the build from `esp32c6` with `boot_app0.bin`, without download mode.
- ST7789 172 × 320: start and status messages are shown in landscape (rotated 90°) at double font size.
- Upload:
  - If the file name lacks `face_` (clock face) or `hand_set` (hand), the clock adds it, e.g. `station.bmp` →
    `face_station.bmp`, `set3_hour.bmp` or `3_hour.bmp` → `hand_set3_hour.bmp`. `.BMP` is lowercased.
  - The hints below the upload forms state what the clock accepts: BMP with 16, 24 or 32 bit, at most as large
    as the free space (shown). The clock scales and converts to RGB565.
- Presets store whether the Rocrail model time is on and whether the clock reconnects the WiFi (`rocrail=` and
  `wifiReconnect=true/false` in `/api/setMode`). Server addresses and WiFi credentials stay settings of the
  clock.
- Backup and restore show a progress bar. While the clock is busy, the status bar pauses instead of reporting
  "Connection lost". After restoring, the page waits until the clock answers again after the restart.

### Removed
- "Load Presets from GitHub" button on the presets page, the prompt for an empty list and the route
  `/importpresetsmerge` used only for it.
- Link to the ZIP files (`faces_handsets_240.zip` / `_160.zip`) on the clock faces page.

### Fixed
- Upload: 16-bit BMPs in RGB555 get the right colours. The clock rejects unsupported BMPs (palette, 8/4/1 bit,
  compressed) instead of storing them as a black image; the rejected file does not stay behind.
- Uploading hand sets: a clock face (`face_…`) uploaded there ended up on the clock despite the error message.
  The hand set page only accepts hands now.
- Restore: the warning about a different display type in the backup's file name works (the date with a hyphen
  was not recognized) and knows the ST7789 types.
- ESP32-C6: after a software restart (restore, restart button, flashESP) the display stayed black, only a
  power cycle helped. CS is no longer held LOW permanently, LovyanGFX drives it for each transfer.
- Presets: preview images were often missing. The browser requested all of them at once, and the clock
  (especially without PSRAM) ran out of memory. The page loads them one after another with up to three tries;
  the clock sends each image row by row instead of as a second copy in RAM.
- Clock face designer (displays with a strip): filling stays within the clicked area, clock face or strip.
  Before, e.g. the margin around a round clock face ran on into the strip and coloured it too.
- ESP32-C6 (without PSRAM): a clock face's strip graphic (`strip_….bmp`) was missing because there was not
  enough memory for its copy in RAM; the strip stayed plain. The clock reads it row by row from the file, on
  the display and in the previews.

## 2026-10-05

### Added
- First support for the Waveshare ESP32-C6-LCD-1.47 (ST7789, 172 × 320) and ESP32-C6-LCD-1.3 (ST7789,
  240 × 240) as a separate build:
  - Display types ST7789 (clock 172 × 172 with a time and date strip) and ST7789_240.
  - Pin mapping in `config.h` (`BOARD_WAVESHARE_C6_ST7789`, recognized by the chip; both boards mapped alike),
    no second display.
  - Clock faces from GitHub come from the 240 folder and are scaled down on upload.
  - The linker switch `-mtext-section-literals` only applies to the ESP32-S2 (`board.txt`,
    `platform.local.txt`).
  - Without PSRAM the clock keeps the clock face only compressed (or reads it from the file) and redraws every
    frame from it. Uploads scale row by row without a full frame in RAM; previews (presets, preview page) scale
    down or stream row by row.
  - `flashESP.bat` recognizes the C6 by its USB port and flashes the build from `build_uhr4\esp32c6`.
  - The build needs "USB CDC On Boot: Enabled", otherwise it stops with a message.

## 2026-10-04

### Fixed
- README: note about the English version directly below the title; the link to the no longer existing release
  "Version 3" is gone.

## 2026-10-02

### Changed
- Backup: clock faces, hands and strip graphics are stored as normal BMP images (previously in the clock's
  compressed format) and open in any image program. On restore the clock packs them like on upload; older
  backups can still be restored.
- Retook the screenshots (`screenshots/de`, `screenshots/en`): the DCF77 page from a clock with a receiver,
  again the clock face designer of an ILI9341 clock (`clock_face_designer_ili.png`) and newly its preview with
  the time/date strip (`preview_ili.png`).
- Revised the selection of included clock faces, hands and presets (`graphic/` with `faces_handsets_160.zip`
  and `faces_handsets_240.zip`, `presets.txt`).

### Fixed
- Self-drawn default clock face: on round displays everything outside the circle stays white like with uploaded
  clock faces. It survives backup and restore unchanged pixel for pixel.
- `presets.txt`: the preset "Antik" is taken over again when downloading from GitHub. Name and address were
  separated by a space instead of a tab, so the line was skipped.
