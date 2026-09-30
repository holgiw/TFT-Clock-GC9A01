# Änderungshistorie / Change Log

*English version below.*

---

Alle nennenswerten Änderungen an der Firmware uhr4 (TFT-Uhr mit GC9A01/GC9D01 auf ESP32-S2).
Neueste Einträge oben.

## 2026-09-30

### Geändert
- Kommentare: Sind der deutsche und englische Teil jeweils nur eine Zeile, stehen sie jetzt direkt
  untereinander ohne Leerzeile dazwischen (nur Leerzeilen entfernt, Code unverändert, kein neuer Build nötig).
- `flashESP.bat`/`flashESP.sh`: Wird keine Uhr erkannt, nennen sie jetzt auch den Fall eines neuen ESP32-S2
  ohne Programm (oder mit fremdem Programm) - der meldet sich erst im Bootmodus (Boot halten, USB einstecken)
  und wird dann automatisch als Download-Modus erkannt. `readme.txt` entsprechend ergänzt.

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

### Changed
- Comments: if the German and English part are one line each, they now sit directly below each other
  without a blank line in between (only blank lines removed, code unchanged, no new build needed).
- `flashESP.bat`/`flashESP.sh`: if no clock is detected, they now also mention a new ESP32-S2 without a
  program (or with a foreign program) - it only shows up in boot mode (hold Boot, plug in USB) and is then
  detected automatically as download mode. `readme.txt` extended accordingly.

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
