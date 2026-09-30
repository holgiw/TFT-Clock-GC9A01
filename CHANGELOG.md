# Änderungshistorie / Change Log

*English version below.*

---

Alle nennenswerten Änderungen an der Firmware uhr4 (TFT-Uhr mit GC9A01/GC9D01 auf ESP32-S2).
Neueste Einträge oben.

## 2026-09-30

### Neu
- **ILI9341 (240x320) wieder unterstützt** – als vierter Eintrag der Displayauswahl (Web, `flashESP` 1–4,
  `UHR4 DISPLAY ILI9341`), wie früher in uhr3: Uhr (240x240, dieselben Zifferblätter und Zeiger wie beim GC9A01)
  oben, darunter Uhrzeit („H:MM“ mit blinkendem Doppelpunkt, ohne Sekundenzeiger „H:MM:SS“) und Datum. Quer
  (90°/270°) steht der Streifen rechts neben der Uhr. Feste Hintergrundbeleuchtung, keine Kreismaskierung der
  Zifferblätter – im Zifferblatt-Designer und in der Vorschau sind die Ecken sichtbar und editierbar.
  Sicherungen lassen sich zwischen GC9A01 und ILI9341 wiederherstellen (gleiche Uhrgröße).
  Farbreihenfolge (BGR) und Invertierung sind LovyanGFX-Standard – am echten Modul prüfen.
- **Eigene Schriften für Zifferblatt und Streifen** (VLW, kantengeglättet): Im Zifferblatt-Designer geladene
  Schriften (.ttf/.otf/.woff) werden als `font_<Name>.*` auf der Uhr gespeichert und stehen für Text, die Ziffern im
  Generator und den Streifen zur Verfügung – so passen Zifferblatt und Uhrzeit/Datum zusammen. Für den Streifen
  erzeugt der Browser daraus VLW-Schriften in der gewählten Größe (Uhrzeit/Datum, optional fett;
  `stripfont_<Name>_<Größe>.vlw`, jede Schrift nur einmal), die die Uhr weich über die Streifen-Grafik zeichnet.
  Kein Programmspeicher nötig, die Komplettsicherung enthält die Schriften.
- **Presets speichern den Streifen** (ILI9341): Lage, Farben, Schrift (auch die eigene VLW-Schrift mit Größen),
  Zeit-/Datumsformat, Sekunden, Blinken und Positionen. Die Streifen-Grafik gehört zum Zifferblatt und wechselt
  mit ihm. Ältere Presets lassen den Streifen unverändert. Die Preset-Vorschau zeigt beim ILI9341 das ganze
  Display samt Streifen (Demo-Zeit 10:10:30, heutiges Datum) in den Einstellungen des Presets.
- Entfernt: das Weiterschalten der Presets per Taste (wurde nicht mehr aufgerufen) samt der Einstellung
  `currentPreset`, die beim Start gelöscht wird. Ebenso unbenutzter Code: `toggleLED()`, `waitForWifiScan()` und die
  Makros `BACKLIGHT_CHANNEL`, `DCF77_INTERRUPT`, `HAND_SIDE_PAD`, `LIVE_PREVIEW_SIZE`, `WAIT_30m`.
- **Zeit- und Datumsformat** im Streifen: 24 h, 12 h mit AM/PM (klein rechts oben neben der Uhrzeit) oder 12 h,
  jeweils mit oder ohne Sekunden (oder automatisch: Sekunden nur ohne Sekundenzeiger);
  Datum als T.MM.JJJJ, TT.MM.JJJJ, TT.MM.JJ, MM/TT/JJJJ, JJJJ-MM-TT oder TT.MM.
- Build: Compilerschalter `-mtext-section-literals` (in `board.txt` für Visual Micro) – ohne ihn scheitert der
  Linker an der Größe der einen Übersetzungseinheit ("l32r: literal target out of range").
- **Streifen als Teil der Zeichenfläche** (ILI9341): Der Zifferblatt-Designer bearbeitet das ganze Display
  (240x320); der Bereich über bzw. unter der Uhr wird wie das Zifferblatt gemalt und mit ihm als `strip_<Name>.bmp`
  gespeichert (RLE, je Zifferblatt). Uhrzeit und Datum liegen darüber, quer wird die Grafik um 90° gedreht.
  Zifferblätter ohne Streifen-Grafik nutzen die Hintergrundfarbe. Löschen/Umbenennen eines Zifferblatts zieht die
  Streifen-Grafik mit, die Komplettsicherung enthält sie. Senkrechtes Spiegeln spiegelt an der Uhrmitte.
- **Streifen Uhrzeit/Datum einstellbar** (Zifferblatt-Designer, Karte „Streifen Uhrzeit/Datum“, nur ILI9341):
  Lage über oder unter der Uhr (quer: links oder rechts), Hintergrund- und Schriftfarbe, Schriftart (GLCD,
  7-Segment/DejaVu, FreeSans Bold, DejaVu, Orbitron) und Position von Uhrzeit und Datum (automatisch oder per
  Regler). Änderungen zeigt die Uhr sofort, „Streifen speichern“ legt sie ab. Der Streifen wird in einem Stück
  gesendet (kein Flackern), der Doppelpunkt blinkt wahlweise, ohne die Ziffern zu verschieben. Die Vorschau im
  Designer und die Live-Vorschau auf der Hauptseite zeigen das ganze Display samt Streifen so, wie die Uhr ihn
  zeichnet.
- Zifferblatt-Designer: neue Werkzeuge **Kreis** und **Kreis gefüllt**. Kreis und Ellipse werden jetzt vom
  Mittelpunkt aus aufgezogen (Abstand = Radius bzw. Halbachsen), Mitte und Radius stehen beim Ziehen unter dem Bild.

### Behoben
- Vorschau-Seite: Der Streifen lag im Zifferblatt statt darunter bzw. darüber – ein `</div>` fehlte, die Verschachtelung
  ist korrigiert. Zifferblatt und Streifen stoßen dort jetzt nahtlos aneinander (gemeinsamer Rahmen wie ein Display).

### Geändert
- Dateiname der Komplettsicherung enthält neben dem Hostnamen den Displaytyp, z.B.
  `uhr4-backup-clock-E405-ILI9341-20260930.tar`.
- Beim Drehen eines Displays wird es vorher einmal schwarz gelöscht – es bleiben keine Reste der alten Lage
  stehen (z. B. der Uhrzeit-/Datumsstreifen des ILI9341).
- Kommentare: Sind der deutsche und englische Teil jeweils nur eine Zeile, stehen sie jetzt direkt
  untereinander ohne Leerzeile dazwischen (nur Leerzeilen entfernt, Code unverändert, kein neuer Build nötig).
- `flashESP.bat`/`flashESP.sh`: Wird keine Uhr erkannt, nennen sie jetzt auch den Fall eines neuen ESP32-S2
  ohne Programm (oder mit fremdem Programm) - der meldet sich erst im Bootmodus (Boot halten, USB einstecken)
  und wird dann automatisch als Download-Modus erkannt. `readme.txt` entsprechend ergänzt.
- Flashen robuster: `flashESP.bat`/`setTime.bat`/`flashESP.sh` prüfen, ob alle Dateien da sind (Start direkt aus
  dem Zip, esptool.exe vom Virenscanner entfernt) und melden das klar. Ein ESP32-S3/C3/C6 (USB-Kennung 303A:1001)
  wird als „kein ESP32-S2“ erkannt. Schlägt das Flashen fehl, nennt die Meldung jetzt die häufigen Ursachen
  (Bootmodus, falscher Chip, Port belegt, USB-Hub, Virenscanner) statt nur den Bootmodus. `readme.txt`: Hinweise
  zu USB-Hub, Virenscanner, Windows 7/8 und macOS.
- Kommentare: Historie entfernt („kein … mehr“, „früher“, „bisher“, „Bugfix:“). Reine Historien-Kommentare
  sind gelöscht, bei den übrigen bleibt nur, was der Code heute tut und warum (nur Kommentare geändert).

## 2026-09-29

### Behoben
- **Ruckler ca. 1 s nach dem Start der Zeigeranimation:** Die Uhr startet mit der über den Neustart
  erhaltenen, einige Sekunden nachgehenden Zeit; kurz darauf korrigiert NTP (z. B. +7 s). `animateHand()` rechnete
  die Position als Start + Anteil des Wegs zum *aktuellen* Ziel, sodass der Sekundenzeiger bei diesem Sprung
  in einem Bild um ~18° mitsprang. Jetzt wird der Abstand zum Ziel geführt: Zielbewegungen werden in den
  Restweg eingerechnet (max. ~1,3° pro Bild), ein großer Sprung spät in der Animation startet sie neu.

### Geändert
- Neuer Build in `build_uhr4` (Aufraeumen `#if`/Touch, Kommentare); Release `v4`: `uhr4_flash.zip` erneuert.
- **Kommentare vereinheitlicht:** alle eigenstaendigen Kommentare auf hoechstens 3 Zeilen je Sprache gekuerzt,
  mehrfach gemischte deutsch/englische Abschnitte zu einem deutschen und einem englischen Teil zusammengefasst;
  je eine Leerzeile vor dem Kommentar, zwischen Deutsch und Englisch und danach. Zeilenend-Kommentare hinter Code
  und Kommentare direkt nach einer Direktive bleiben unveraendert. Nur Kommentare und Leerzeilen geaendert, der
  Code ist identisch (gut 1.270 Bloecke, 266 davon umformuliert).
- **Überflüssige `#if`-Abfragen entfernt:** Die Abfragen auf fest definierte Pins (`SDA_PIN`/`SCL_PIN`,
  `DCF77_DATAPIN`/`DCF77_INTERRUPT`, `ADC_PIN`, `ADC_3V`, `LED_BOARD`, `BUTTON1`, `CS_2`, `ESP32_S2`) stammten aus
  uhr3 mit mehreren Board-Varianten; ob RTC, DCF77-Empfänger, Fotowiderstand und Display 2 vorhanden sind,
  erkennt die Firmware ohnehin zur Laufzeit. 58 Blöcke entfernt, nie kompilierte Ersatzzweige gestrichen.
  Die tote Touch-Steuerung (`TOUCH_PIN` war auskommentiert) ist komplett entfernt; der alte NVS-Schlüssel
  `useTouch` wird beim Start gelöscht. `ROUND_DISPLAY` und die USB-Modus-Abfragen bleiben. Keine
  Funktionsänderung.
- Neuer Build in `build_uhr4` (Startzeit 10:10:30, Uhr im Access-Point-Modus); Release `v4`: `uhr4_flash.zip` erneuert.
- **Uhr läuft auch im Access-Point-Modus:** Ohne WLAN (nach dem 2-minütigen WPS-Versuch) zeigte das Display
  bisher dauerhaft nur die Access-Point-Daten – `updateClock()` lief nur mit WLAN, RTC oder DCF77. Jetzt
  erscheinen die AP-Daten 2 Minuten (`AP_INFO_SHOW_MS` in `config.h`), danach läuft die Uhr (ohne Zeitquelle
  ab 10:10:30). Der Access Point bleibt aktiv, ein kurzer Tasterdruck zeigt die AP-Daten erneut
  (`showApInfo()`). Der Neustart nach 15 Minuten im AP-Modus erfolgt nur noch, wenn WLAN-Netze gespeichert
  sind – sonst gibt es nichts neu zu versuchen.
- **Startzeit 10:10:30 statt 12:00:00, Uhr läuft ohne Zeitquelle weiter:** Solange noch keine Uhrzeit aus
  NTP/DCF77/RTC/USB vorliegt, startet die Anzeige bei 10:10:30 (klassische Uhrmacher-Stellung) und läuft ab dort
  weiter (vorher standen die Zeiger auf 12:00:00). Die Systemzeit wird dabei nicht gesetzt, die erste echte Zeit
  übernimmt sofort. `handleNTPFailure()` setzt ohne bekannte Zeit keine Ersatzzeit (12:00) mehr. Einstellbar über
  `START_TIME_HOUR/MIN/SEC` in `config.h`.
- README: Rocrail-Haken sitzt im Tab „Uhr Einstellungen“ (nicht Zeit-Tab), Navigationspunkte wie auf der
  Uhr (Uhren Sets, Dateimanager, DCF77, Werkseinstellungen), Abschnitt Hardware um zweites Display, RTC,
  DCF77-Empfänger und Taster ergänzt.
- `CHANGELOG.md` zweisprachig (deutsch/englisch). Kopfkommentar von `build_uhr4/port.ps1` korrigiert
  (wird von `clocksetup.ps1` aufgerufen, nicht von `flashESP.bat`).
- `flashESP.sh`/`setTime.sh`: prüfen vor dem Senden per `fuser`, ob ein anderes Programm (z. B. serieller
  Monitor) die Schnittstelle offen hat, und melden es mit Programmname – unter Linux gibt es dafür keinen
  Fehler, der Monitor schnappt nur die Antworten weg. Solange belegt, läuft das Zeitlimit „keine Antwort“
  nicht ab; `--time` wartet wie `setTime.bat` bis 30 s. Release `v4`: `uhr4_flash.zip` erneuert.
- `build_uhr4/readme.txt` korrigiert: Neustart nach „speichern“ im Access Point (kein RESET-Knopf mehr),
  Taster jederzeit im Betrieb, Seitennamen „Werkseinstellungen“ → „Gespeicherte Netzwerke zurücksetzen“,
  Linux-Ablauf wie unter Windows (erst Uhr suchen, Vorwahl, PC-WLAN übernehmen); englisch: Dateiliste,
  WLAN-Abfrage vor dem Flashen. Release `v4`: `uhr4_flash.zip` erneuert.
- Neuer Build in `build_uhr4` (Zeigeranimation, Prüfung auf private Netze aus); Release `v4`: `uhr4_flash.zip` erneuert.
- **Prüfung auf private IP-Adressen abgeschaltet:** `isPrivateNetworkIp()` liefert immer `true`
  (`PRIVATE_NETWORK_CHECK = false` in `webserver_routes.h`, Aufrufstellen unverändert). Alle Aktionen
  (Neustart, Dateien, WLAN, Werkseinstellungen, Sicherung, Status, Log) sind damit von überall ohne
  Bestätigungscode erlaubt, der NTP-Server antwortet allen. README und Anleitung angepasst (Hinweis:
  Uhr nicht per Port-Weiterleitung/DMZ ins Internet stellen).
- `erster_ start_first_start.md` an Firmware und Flash-Tool angeglichen: Ablauf beim Flashen (erst Uhr
  suchen, Displaytyp vorgewählt, PC-WLAN übernehmen), Seiten- und Knopfnamen (Zeigersätze verwalten, Set
  hochladen, Presets sichern / wiederherstellen, Werkseinstellungen → Gespeicherte Netzwerke zurücksetzen),
  Komplettsicherung, Displaytyp ändern, `setTime` in der Kurzübersicht.
- `CONTRIBUTING.md` aktualisiert: Board/Core-Version, PSRAM fest an, Builds nach `build_uhr4/`, drei
  Displaytypen (ILI9341 gestrichen), Dateitabelle um Designer, Rocrail und `build_defs.h` ergänzt,
  Übersetzungsregel: identische deutsche Texte ohne Eintrag.
- Neuer Build in `build_uhr4` (bereinigte `translation.h`); Release `v4`: `uhr4_flash.zip` erneuert.
- Screenshots (`screenshots/de`, `screenshots/en`) neu von uhr4 aufgenommen (vorher uhr3): 800 × 600, die
  Designer 800 × 1000 (Desktop-Ansicht verkleinert); englische Bilder mit englischer Browsersprache.
- `translation.h`: 76 Einträge entfernt – 56 ungenutzte (z. B. Touch-Steuerung, alte Upload-/WLAN-Meldungen,
  frühere Rocrail-Texte, Tippfehler „Clock Seetup“) und 20 mit identischem deutschem Text (z. B. „Status“,
  „Log“, „NTP Server“); `translate()` liefert ohne Eintrag ohnehin den Schlüssel. Angezeigte Texte
  unverändert, 350 Einträge bleiben.
- README an die Weboberfläche angeglichen: Displaytyp im Tab „Uhr Einstellungen“ (nicht „Zifferblatt“),
  tatsächliche Tab-Liste, Info-Seite gestrichen (gibt es nicht mehr), Hintergrundbeleuchtung über den
  Displaytyp, volle Helligkeit bei der Einrichtung, sanfte Zeiger bei Zeitsprüngen. Tab-Namen auch in
  `build_uhr4/readme.txt` und `CONTRIBUTING.md` korrigiert.
- `readme_text.h` entfernt: wurde seit dem Wegfall der /info-Seite (uhr3, `4b735b7`) nirgends mehr
  eingebunden. `readme.txt` im Hauptordner entfernt (identische Kopie von `build_uhr4/readme.txt`,
  die als Flash-Anleitung bleibt).
- **uhr4 ersetzt uhr3 im Repository (Version 4).** uhr3 bleibt als Tag `v3` bzw. Release „Version 3“
  (ZIP mit Quellcode und den drei Builds) erhalten; im Repository liegen jetzt nur noch uhr4 und
  `build_uhr4`. `.gitignore` schließt uhr3-Reste (`TFT_eSPI.zip`, `uhr3.ino.*.bin`) und lokale
  Visual-Studio-Ordner aus. README mit Download-Hinweis und Verweis auf Version 3.
- `setTime.bat`/`flashESP.bat`: Ist der COM-Port belegt (z. B. serieller Monitor offen), meldet das Skript
  das sofort und wartet bis zu 30 s, bis der Port frei wird, statt nur „keine Antwort“ zu melden.

---
---

# English Version

All notable changes to the uhr4 firmware (TFT clock with GC9A01/GC9D01 on ESP32-S2).
Newest entries on top.

## 2026-09-30

### New
- **ILI9341 (240x320) supported again** – as the fourth entry of the display selection (web, `flashESP` 1–4,
  `UHR4 DISPLAY ILI9341`), as formerly in uhr3: clock (240x240, the same clock faces and hands as the GC9A01) on
  top, time below ("H:MM" with a blinking colon, without the second hand "H:MM:SS") and date. In landscape
  (90°/270°) the strip is to the right of the clock. Fixed backlight, no circular masking of the clock faces - the
  corners are visible and editable in the clock face designer and the preview.
  Backups can be restored between GC9A01 and ILI9341 (same clock size). Color order (BGR) and inversion are the
  LovyanGFX defaults – check on a real module.
- **Own fonts for clock face and strip** (VLW, anti-aliased): fonts loaded in the clock face designer
  (.ttf/.otf/.woff) are stored on the clock as `font_<name>.*` and are available for text, the generator numerals
  and the strip – so clock face and time/date match. For the strip the browser creates VLW fonts from them in the
  chosen size (time/date, optionally bold; `stripfont_<name>_<size>.vlw`, each font only once), which the clock
  draws smoothly over the strip graphic. No program memory needed, the full backup contains the fonts.
- **Presets store the strip** (ILI9341): placement, colours, font (also the own VLW font with sizes), time/date
  format, seconds, blinking and positions. The strip graphic belongs to the clock face and changes with it. Older
  presets leave the strip unchanged. On the ILI9341 the preset preview shows the whole display including the strip
  (demo time 10:10:30, today's date) in the preset's settings.
- Removed: switching presets via the button (was no longer called) together with the `currentPreset` setting,
  which is deleted at boot. Likewise unused code: `toggleLED()`, `waitForWifiScan()` and the macros
  `BACKLIGHT_CHANNEL`, `DCF77_INTERRUPT`, `HAND_SIDE_PAD`, `LIVE_PREVIEW_SIZE`, `WAIT_30m`.
- **Time and date format** in the strip: 24 h, 12 h with AM/PM (small at the top right next to the time) or 12 h,
  each with or without seconds (or automatic: seconds only without the second hand);
  date as D.MM.YYYY, DD.MM.YYYY, DD.MM.YY, MM/DD/YYYY, YYYY-MM-DD or DD.MM.
- Build: compiler flag `-mtext-section-literals` (in `board.txt` for Visual Micro) – without it the linker fails
  on the size of the single translation unit ("l32r: literal target out of range").
- **Strip as part of the drawing area** (ILI9341): the clock face designer edits the whole display (240x320); the
  area above or below the clock is painted like the clock face and saved with it as `strip_<name>.bmp` (RLE, per
  clock face). Time and date lie on top, in landscape the graphic is rotated by 90°. Clock faces without a strip
  graphic use the background colour. Deleting/renaming a clock face takes the strip graphic along, the full backup
  contains it. Vertical mirroring mirrors at the clock centre.
- **Configurable time/date strip** (clock face designer, card "Time/date strip", ILI9341 only): placement above
  or below the clock (landscape: left or right), background and text colour, font (GLCD, 7-segment/DejaVu,
  FreeSans Bold, DejaVu, Orbitron) and position of time and date (automatic or via sliders). The clock shows
  changes right away, "Save strip" stores them. The strip is sent in one piece (no flicker), the blinking colon
  optionally blinks without shifting the digits. The designer preview and the live preview on the main page show the whole
  display including the strip exactly as the clock draws it.
- Clock face designer: new tools **Circle** and **Filled circle**. Circle and ellipse are now dragged out from the
  centre (distance = radius or semi-axes), centre and radius are shown below the image while dragging.

### Fixed
- Preview page: the strip lay inside the clock face instead of below or above it – a `</div>` was missing, the
  nesting is fixed. Clock face and strip now join seamlessly there (one shared frame like a display).

### Changed
- The full backup's file name contains the display type next to the host name, e.g.
  `uhr4-backup-clock-E405-ILI9341-20260930.tar`.
- When a display is rotated, it is cleared to black once beforehand – no remains of the old orientation stay
  (e.g. the ILI9341 time/date strip).
- Comments: if the German and English part are one line each, they now sit directly below each other
  without a blank line in between (only blank lines removed, code unchanged, no new build needed).
- `flashESP.bat`/`flashESP.sh`: if no clock is detected, they now also mention a new ESP32-S2 without a
  program (or with a foreign program) - it only shows up in boot mode (hold Boot, plug in USB) and is then
  detected automatically as download mode. `readme.txt` extended accordingly.
- More robust flashing: `flashESP.bat`/`setTime.bat`/`flashESP.sh` check that all files are present (started
  directly from the zip, esptool.exe removed by the virus scanner) and say so clearly. An ESP32-S3/C3/C6 (USB id
  303A:1001) is detected as "not an ESP32-S2". If flashing fails, the message now lists the common causes (boot
  mode, wrong chip, port in use, USB hub, virus scanner) instead of only the boot mode. `readme.txt`: notes on
  USB hubs, virus scanners, Windows 7/8 and macOS.
- Comments: history removed ("no more ...", "previously", "used to", "Bugfix:"). Pure history comments are
  deleted, the others only keep what the code does today and why (only comments changed).

## 2026-09-29

### Fixed
- **Jerk about 1 s after the start of the hand animation:** The clock boots with the time kept across
  the restart, which lags a few seconds; shortly after, NTP corrects it (e.g. +7 s). `animateHand()`
  computed the position as start + share of the way to the *current* target, so on this jump the second
  hand jumped ~18° within one frame. Now the distance to the target is tracked: target movements are folded
  into the remaining way (max. ~1.3° per frame), a large jump late in the animation restarts it.

### Changed
- New build in `build_uhr4` (`#if`/touch cleanup, comments); release `v4`: `uhr4_flash.zip` renewed.
- **Comments unified:** all standalone comments shortened to at most 3 lines per language, repeatedly mixed
  German/English sections merged into one German and one English part; one blank line before the comment,
  between German and English and after it. Trailing comments behind code and comments right after a directive
  stay unchanged. Only comments and blank lines changed, the code is identical (about 1,270 blocks, 266 of them
  reworded).
- **Redundant `#if` checks removed:** The checks for always-defined pins (`SDA_PIN`/`SCL_PIN`,
  `DCF77_DATAPIN`/`DCF77_INTERRUPT`, `ADC_PIN`, `ADC_3V`, `LED_BOARD`, `BUTTON1`, `CS_2`, `ESP32_S2`) came from
  uhr3 with several board variants; whether RTC, DCF77 receiver, photoresistor and display 2 are present is
  detected at runtime anyway. 58 blocks removed, never-compiled fallback branches dropped. The dead touch
  control (`TOUCH_PIN` was commented out) is removed completely; the old NVS key `useTouch` is deleted at
  boot. `ROUND_DISPLAY` and the USB mode checks stay. No functional change.
- New build in `build_uhr4` (start time 10:10:30, clock in access point mode); release `v4`: `uhr4_flash.zip` renewed.
- **Clock also runs in access point mode:** Without WiFi (after the 2-minute WPS attempt) the display used to
  show only the access point details permanently - `updateClock()` only ran with WiFi, RTC or DCF77. Now the AP
  details are shown for 2 minutes (`AP_INFO_SHOW_MS` in `config.h`), then the clock runs (from 10:10:30 without
  a time source). The access point stays active, a short button press shows the AP details again
  (`showApInfo()`). The restart after 15 minutes in AP mode now only happens when WiFi networks are stored -
  otherwise there is nothing to retry.
- **Start time 10:10:30 instead of 12:00:00, clock keeps running without a time source:** As long as no time
  from NTP/DCF77/RTC/USB is available yet, the display starts at 10:10:30 (classic watchmaker position) and keeps
  running from there (previously the hands stood at 12:00:00). The system time is not set, the first real time
  takes over right away. `handleNTPFailure()` no longer sets a substitute time (12:00) without a known time.
  Adjustable via `START_TIME_HOUR/MIN/SEC` in `config.h`.
- README: the Rocrail checkbox is in the "Clock Setup" tab (not the Time tab), navigation items as on the
  clock (Presets, File Manager, DCF77, Factory Reset), hardware section extended with second display, RTC,
  DCF77 receiver and button.
- `CHANGELOG.md` bilingual (German/English). Header comment of `build_uhr4/port.ps1` corrected (it is
  called by `clocksetup.ps1`, not by `flashESP.bat`).
- `flashESP.sh`/`setTime.sh`: before sending, check via `fuser` whether another program (e.g. a serial
  monitor) has the port open, and report it with the program name - on Linux this causes no error, the
  monitor just snatches the replies. While the port is in use, the "no reply" timeout does not run out;
  `--time` waits up to 30 s like `setTime.bat`. Release `v4`: `uhr4_flash.zip` renewed.
- `build_uhr4/readme.txt` corrected: restart after "Save" in the access point (no RESET button anymore),
  button works at any time during operation, page names "Factory Reset" → "Reset Saved Networks", Linux
  flow as on Windows (find the clock first, preselection, take over the PC's WiFi); English: file list,
  WiFi question before flashing. Release `v4`: `uhr4_flash.zip` renewed.
- New build in `build_uhr4` (hand animation, private network check off); release `v4`: `uhr4_flash.zip` renewed.
- **Private IP address check disabled:** `isPrivateNetworkIp()` always returns `true`
  (`PRIVATE_NETWORK_CHECK = false` in `webserver_routes.h`, call sites unchanged). All actions (restart,
  files, WiFi, factory reset, backup, status, log) are thus allowed from anywhere without a confirmation
  code, the NTP server answers everyone. README and guide adjusted (note: do not expose the clock to the
  internet via port forward/DMZ).
- `erster_ start_first_start.md` aligned with firmware and flash tool: flashing flow (find the clock
  first, display type preselected, take over the PC's WiFi), page and button names (Manage Clock Hand Sets,
  Upload to Set, Backup / Restore Presets, Factory Reset → Reset Saved Networks), full backup, changing the
  display type, `setTime` in the quick reference.
- `CONTRIBUTING.md` updated: board/core version, PSRAM always on, builds go to `build_uhr4/`, three
  display types (ILI9341 dropped), file table extended with the designers, Rocrail and `build_defs.h`,
  translation rule: identical German texts without an entry.
- New build in `build_uhr4` (cleaned-up `translation.h`); release `v4`: `uhr4_flash.zip` renewed.
- Screenshots (`screenshots/de`, `screenshots/en`) retaken from uhr4 (previously uhr3): 800 × 600, the
  designers 800 × 1000 (desktop layout scaled down); English pictures with English browser language.
- `translation.h`: 76 entries removed - 56 unused ones (e.g. touch control, old upload/WiFi messages,
  former Rocrail texts, typo "Clock Seetup") and 20 with an identical German text (e.g. "Status", "Log",
  "NTP Server"); without an entry `translate()` returns the key anyway. Displayed texts unchanged, 350
  entries remain.
- README aligned with the web interface: display type in the "Clock Setup" tab (not "Clock Face"), actual
  tab list, info page removed (no longer exists), backlight via the display type, full brightness during
  setup, smooth hands on time jumps. Tab names also corrected in `build_uhr4/readme.txt` and
  `CONTRIBUTING.md`.
- `readme_text.h` removed: no longer included anywhere since the /info page was dropped (uhr3, `4b735b7`).
  `readme.txt` in the main folder removed (identical copy of `build_uhr4/readme.txt`, which stays as the
  flashing guide).
- **uhr4 replaces uhr3 in the repository (version 4).** uhr3 is kept as tag `v3` / release "Version 3"
  (ZIP with source code and the three builds); the repository now only contains uhr4 and `build_uhr4`.
  `.gitignore` excludes uhr3 leftovers (`TFT_eSPI.zip`, `uhr3.ino.*.bin`) and local Visual Studio folders.
  README with a download note and a link to version 3.
- `setTime.bat`/`flashESP.bat`: if the COM port is in use (e.g. a serial monitor is open), the script
  reports it right away and waits up to 30 s for the port to become free, instead of only reporting
  "no reply".
