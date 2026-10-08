# Changelog

*English version below.*

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
