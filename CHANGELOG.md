# Changelog

*English version below.*

## 2026-10-06

### Geändert
- Startpaket einer neuen Uhr: Das Standard-Zifferblatt `face_default.bmp` hat jetzt die Ziffern 12, 3, 6 und 9.
  Ohne Zifferblätter und Zeigersätze erzeugt die Uhr dazu `face_numbers.bmp` (1–12) und `face_roman.bmp`
  (I–XII), die Zeigersätze 1 (Balken, schwarzer Sekundenzeiger) und 2 (geschwungen) und – ohne vorhandene
  Presets – die Presets „Standard“, „1-12“ und „I-XII“ (ohne Sekundenzeiger). Die Werksreset-Aktionen für
  Zifferblätter, Zeigersätze und Presets stellen diesen Zustand wieder her. Passen die erzeugten Dateien nach
  einem Wechsel des Displaytyps nicht mehr zur Uhrgröße, erzeugt die Uhr sie beim Start neu. Auf Displays mit
  Streifen (ILI9341, ST7789 172 × 320) haben die drei Presets einen weißen Streifen mit schwarzer Schrift in
  FreeSans Bold, DejaVu bzw. der Designer-Schrift „Sans“ (falls auf der Uhr vorhanden, sonst GLCD); eine neue
  Uhr startet gleich mit dem Streifen wie „Standard“.
- Wiederherstellen einer Sicherung mit anderem Displaytyp: Die Uhr weist darauf hin und übernimmt nur
  Zifferblätter, Zeiger (auf ihre Größe skaliert), Uhren Sets und allgemeine Einstellungen; Displaytyp,
  Rotation, Hintergrundbeleuchtung, Helligkeit und Nabengröße bleiben. Sind die Zifferblätter der Sicherung
  nicht 240 × 240 groß, übernimmt sie nichts und meldet das. Stammt die Sicherung von einer Uhr ohne Streifen,
  bekommt eine Uhr mit Streifen den Standard-Streifen (weiß, schwarze Schrift, FreeSans Bold). Bisher wurde eine andere Uhrgröße abgelehnt und bei
  gleicher Größe (z. B. GC9A01 und ILI9341) alles übernommen.
- Sicherung: Der Displaytyp steht jetzt immer darin, auch wenn er nie gespeichert wurde (Werkseinstellung) -
  sonst hätte die Zieluhr beim Wiederherstellen ihre eigene Werkseinstellung angenommen. Ältere Sicherungen
  ohne diesen Eintrag gelten als GC9A01 (Werkseinstellung des ESP32-S2).
- Sicherung: Der Dateiname enthält jetzt auch die Uhrzeit mit Sekunden (z. B.
  `uhr4-backup-clock_23F6A4-ST7789-20261006-175812.tar`), mehrere Sicherungen eines Tages überschreiben sich
  so nicht mehr.
- `flashESP.sh` (Linux) kennt jetzt auch den ESP32-C6: Es erkennt ihn am USB-Port (303a:1001), bietet die
  Displaytypen ST7789 und ST7789_240 an und flasht den Build aus `esp32c6` (ohne Umschalten in den
  Download-Modus, mit `boot_app0.bin`).
- ST7789 172 × 320: Start- und Statusmeldungen stehen quer (90° gedreht) in doppelt so großer Schrift.
- Hochladen: Fehlt einem Zifferblatt das `face_` oder einem Zeiger das `hand_set` im Dateinamen, ergänzt die
  Uhr es (z. B. `bahnhof.bmp` → `face_bahnhof.bmp`, `set3_hour.bmp` oder `3_hour.bmp` → `hand_set3_hour.bmp`);
  `.BMP` wird klein geschrieben. Die Hinweise unter den Upload-Formularen nennen jetzt, was die Uhr annimmt:
  BMP mit 16, 24 oder 32 Bit, höchstens so groß wie der freie Speicher (angezeigt); die Uhr skaliert und wandelt
  in RGB565 um.
- Presets enthalten jetzt auch, ob die Rocrail-Modellzeit an ist und ob die Uhr das WLAN neu verbindet
  (`rocrail=` und `wifiReconnect=true/false` in `/api/setMode`); Serveradressen und WLAN-Daten bleiben
  Einstellung der Uhr.
- Sichern und Wiederherstellen zeigen einen Fortschrittsbalken. Solange die Uhr beschäftigt ist, pausiert die
  Statusleiste, statt „Verbindung verloren“ zu melden; nach dem Wiederherstellen wartet die Seite, bis die Uhr
  nach dem Neustart wieder antwortet.

### Entfernt
- Der Button „Presets von GitHub laden“ auf der Seite Uhren Sets samt der automatischen Nachfrage bei leerer
  Liste (und die nur dafür genutzte Route `/importpresetsmerge`).
- Der Link auf die ZIP-Dateien (`faces_handsets_240.zip` / `_160.zip`) auf der Seite Zifferblätter.

### Behoben
- Hochladen: 16-Bit-BMPs im Format RGB555 bekommen die richtigen Farben; nicht unterstützte BMPs (Palette,
  8/4/1 Bit, komprimiert) werden abgelehnt, statt als schwarzes Bild gespeichert zu werden. Eine abgelehnte
  Datei bleibt nicht mehr auf der Uhr liegen.
- Zeigersätze hochladen: Ein dort hochgeladenes Zifferblatt (`face_…`) landete trotz Fehlermeldung auf der
  Uhr; die Zeiger-Seite nimmt jetzt nur noch Zeiger an.
- Wiederherstellen: Die Warnung bei abweichendem Displaytyp im Dateinamen der Sicherung greift jetzt (das Datum
  mit Bindestrich wurde nicht erkannt) und kennt auch die ST7789-Typen.
- ESP32-C6: Nach einem Neustart durch die Software (Wiederherstellen, Neustart-Button, flashESP) blieb das
  Display schwarz, erst Strom aus/an half. CS liegt jetzt nicht mehr dauerhaft auf LOW, sondern LovyanGFX
  schaltet es je Übertragung.
- Uhren Sets: Vorschaubilder fehlten oft, weil der Browser alle auf einmal anforderte und der Uhr (vor allem
  ohne PSRAM) der Speicher ausging. Die Seite lädt sie jetzt nacheinander und versucht es bei einem Fehler bis zu
  dreimal; die Uhr sendet jedes Bild zeilenweise statt als zweite Kopie im RAM.
- Zifferblatt-Designer (Displays mit Streifen): Füllen bleibt im angeklickten Bereich – Zifferblatt oder
  Streifen. Bisher lief z. B. der Rand um ein rundes Zifferblatt in den Streifen weiter und färbte ihn mit ein.
- ESP32-C6 (ohne PSRAM): Die Streifen-Grafik eines Zifferblatts (`strip_….bmp`) wurde nicht angezeigt – für
  ihre Kopie im RAM fehlte der Speicher, der Streifen blieb einfarbig. Die Uhr liest sie jetzt zeilenweise aus
  der Datei, auf dem Display und in den Vorschauen.

## 2026-10-05

### Hinzugefügt
- Erste Unterstützung für die Waveshare ESP32-C6-LCD-1.47 (ST7789, 172 × 320) und ESP32-C6-LCD-1.3 (ST7789,
  240 × 240) als eigenes Build: neue Displaytypen ST7789 (Uhr 172 × 172, darunter Uhrzeit und Datum) und
  ST7789_240, Pinbelegung der Boards in `config.h` (`BOARD_WAVESHARE_C6_ST7789`, am Chip erkannt; bei
  beiden Boards dieselben Pins), kein zweites Display. Zifferblätter von GitHub kommen aus dem
  240er-Ordner und werden beim Hochladen verkleinert. Der Linker-Schalter `-mtext-section-literals` gilt nur
  noch für den ESP32-S2 (`board.txt`, `platform.local.txt`). Ohne PSRAM hält die Uhr das Zifferblatt nur
  komprimiert (oder liest es aus der Datei) und zeichnet jedes Bild daraus neu; Hochladen skaliert zeilenweise
  ohne Vollbild im RAM; Vorschauen (Uhren Sets, Vorschau-Seite) verkleinern bzw. streamen zeilenweise. `flashESP.bat` erkennt den C6 am USB-Port und flasht den Build aus `build_uhr4\esp32c6`;
  der Build braucht „USB CDC On Boot: Enabled“ (sonst bricht er mit einer Meldung ab).

## 2026-10-04

### Behoben
- README: Hinweis auf die englische Fassung direkt unter dem Titel; der Verweis auf das nicht mehr vorhandene
  Release „Version 3“ ist entfernt.

## 2026-10-02

### Geändert
- Sicherung: Zifferblätter, Zeiger und Streifen-Grafiken liegen darin als normale BMP-Bilder (vorher im
  komprimierten Format der Uhr) und lassen sich mit jedem Bildprogramm öffnen. Beim Wiederherstellen packt die
  Uhr sie wieder wie beim Hochladen; ältere Sicherungen lassen sich weiter einspielen.
- Screenshots (`screenshots/de`, `screenshots/en`) neu aufgenommen; die Seite DCF77 stammt von einer Uhr mit
  Empfänger, dazu wieder der Zifferblatt-Designer einer ILI9341-Uhr (`zifferblatt_designer_ili.png`) und neu deren
  Vorschau mit dem Uhrzeit-/Datumsstreifen (`vorschau_ili.png`).
- Auswahl der mitgelieferten Zifferblätter, Zeiger und Presets überarbeitet (`graphic/` mit den Zips
  `faces_handsets_160.zip` und `faces_handsets_240.zip`, `presets.txt`).

### Behoben
- Selbst gezeichnetes Standard-Zifferblatt: auf runden Displays bleibt alles außerhalb des Kreises weiß wie bei
  hochgeladenen Zifferblättern – es übersteht Sicherung und Wiederherstellen jetzt Pixel für Pixel unverändert.
- `presets.txt`: Das Preset „Antik“ wird beim Herunterladen von GitHub wieder übernommen – Name und Adresse
  waren durch ein Leerzeichen statt eines Tabs getrennt, die Zeile wurde deshalb übersprungen.

---

# English Version

## 2026-10-06

### Changed
- Starter set of a new clock: the default clock face `face_default.bmp` now has the numerals 12, 3, 6 and 9.
  Without clock faces and hand sets the clock also generates `face_numbers.bmp` (1–12) and `face_roman.bmp`
  (I–XII), hand sets 1 (bars, black second hand) and 2 (curved) and – without existing presets – the presets
  "Standard", "1-12" and "I-XII" (without a second hand). The factory reset actions for clock faces, hand sets
  and presets restore this state. If the generated files no longer fit the clock size after a display type
  change, the clock regenerates them at startup. On displays with a strip (ILI9341, ST7789 172 × 320) the three
  presets have a white strip with black text in FreeSans Bold, DejaVu or the designer font "Sans" (if present
  on the clock, otherwise GLCD); a new clock starts right away with the strip like "Standard".
- Restoring a backup of another display type: the clock points this out and only takes over clock faces,
  hands (scaled to its size), presets and general settings; display type, rotation, backlight, brightness and
  hub size stay. If the backup's clock faces are not 240 × 240, it takes over nothing and says so. If the
  backup comes from a clock without a strip, a clock with a strip gets the standard strip (white, black text,
  FreeSans Bold). Before, a
  different clock size was rejected and with the same size (e.g. GC9A01 and ILI9341) everything was restored.
- Backup: the display type is now always included, even if it was never stored (factory default) - otherwise
  the target clock would have assumed its own factory default when restoring. Older backups without this entry
  count as GC9A01 (the ESP32-S2's factory default).
- Backup: the file name now also contains the time with seconds (e.g.
  `uhr4-backup-clock_23F6A4-ST7789-20261006-175812.tar`), so several backups on one day no longer overwrite
  each other.
- `flashESP.sh` (Linux) now also knows the ESP32-C6: it recognizes it by the USB port (303a:1001), offers the
  display types ST7789 and ST7789_240 and flashes the build from `esp32c6` (without switching into download
  mode, with `boot_app0.bin`).
- ST7789 172 × 320: start and status messages are shown in landscape (rotated 90°) at double font size.
- Upload: if a clock face lacks `face_` or a hand lacks `hand_set` in its file name, the clock adds it (e.g.
  `station.bmp` → `face_station.bmp`, `set3_hour.bmp` or `3_hour.bmp` → `hand_set3_hour.bmp`); `.BMP` is
  lowercased. The hints below the upload forms now state what the clock accepts: BMP with 16, 24 or 32 bit, at
  most as large as the free space (shown); the clock scales and converts to RGB565.
- Presets now also store whether the Rocrail model time is on and whether the clock reconnects the WiFi
  (`rocrail=` and `wifiReconnect=true/false` in `/api/setMode`); server addresses and WiFi credentials stay
  settings of the clock.
- Backup and restore show a progress bar. While the clock is busy, the status bar pauses instead of reporting
  "Connection lost"; after restoring, the page waits until the clock answers again after the restart.

### Removed
- The "Load Presets from GitHub" button on the presets page together with the automatic prompt for an empty
  list (and the route `/importpresetsmerge` used only for it).
- The link to the ZIP files (`faces_handsets_240.zip` / `_160.zip`) on the clock faces page.

### Fixed
- Upload: 16-bit BMPs in RGB555 get the right colors; unsupported BMPs (palette, 8/4/1 bit, compressed) are
  rejected instead of being stored as a black image. A rejected file no longer stays on the clock.
- Uploading hand sets: a clock face (`face_…`) uploaded there ended up on the clock despite the error
  message; the hand set page now only accepts hands.
- Restore: the warning about a different display type in the backup's file name now works (the date with a
  hyphen was not recognized) and also knows the ST7789 types.
- ESP32-C6: after a software restart (restore, restart button, flashESP) the display stayed black, only a
  power cycle helped. CS is no longer held LOW permanently; LovyanGFX now drives it for each transfer.
- Presets: preview images were often missing because the browser requested all of them at once and the clock
  (especially without PSRAM) ran out of memory. The page now loads them one after another and retries up to
  three times on an error; the clock sends each image row by row instead of as a second copy in RAM.
- Clock face designer (displays with a strip): filling stays within the clicked area – clock face or strip.
  Before, e.g. the margin around a round clock face ran on into the strip and coloured it too.
- ESP32-C6 (without PSRAM): a clock face's strip graphic (`strip_….bmp`) was not shown – there was not enough
  memory for its copy in RAM, the strip stayed plain. The clock now reads it row by row from the file, on the
  display and in the previews.

## 2026-10-05

### Added
- First support for the Waveshare ESP32-C6-LCD-1.47 (ST7789, 172 × 320) and ESP32-C6-LCD-1.3 (ST7789,
  240 × 240) as a separate build: new display types ST7789 (clock 172 × 172, time and date below) and
  ST7789_240, the boards' pin mapping in `config.h` (`BOARD_WAVESHARE_C6_ST7789`, recognized by the chip;
  the same pins on both boards), no second display. Clock faces from GitHub come from the 240
  folder and are scaled down on upload. The linker switch `-mtext-section-literals` now only applies to the
  ESP32-S2 (`board.txt`, `platform.local.txt`). Without PSRAM the clock keeps the clock face only compressed
  (or reads it from the file) and redraws every frame from it; uploads scale row by row without a full frame
  in RAM; previews (presets, preview page) scale down or stream row by row. `flashESP.bat` recognizes the C6 by its USB port and flashes the build from `build_uhr4\esp32c6`; the
  build needs "USB CDC On Boot: Enabled" (otherwise it stops with a message).

## 2026-10-04

### Fixed
- README: note about the English version directly below the title; the link to the no longer existing release
  "Version 3" is removed.

## 2026-10-02

### Changed
- Backup: clock faces, hands and strip graphics are stored as normal BMP images (previously in the clock's
  compressed format) and open in any image program. On restore the clock packs them again like on upload;
  older backups can still be restored.
- Retook the screenshots (`screenshots/de`, `screenshots/en`); the DCF77 page comes from a clock with a
  receiver, plus again the clock face designer of an ILI9341 clock (`clock_face_designer_ili.png`) and new its
  preview with the time/date strip (`preview_ili.png`).
- Revised the selection of included clock faces, hands and presets (`graphic/` with the zips
  `faces_handsets_160.zip` and `faces_handsets_240.zip`, `presets.txt`).

### Fixed
- Self-drawn default clock face: on round displays everything outside the circle stays white like with uploaded
  clock faces – it now survives backup and restore unchanged pixel for pixel.
- `presets.txt`: the preset "Antik" is taken over again when downloading from GitHub – name and address were
  separated by a space instead of a tab, so the line was skipped.
