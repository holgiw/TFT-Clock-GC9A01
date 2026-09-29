# Änderungshistorie

Alle nennenswerten Änderungen an der Firmware uhr4 (TFT-Uhr mit GC9A01/GC9D01 auf ESP32-S2).
Neueste Einträge oben.

## 2026-09-29

### Behoben
- **Ruckler ca. 1 s nach dem Start der Zeigeranimation:** Die Uhr startet mit der über den Neustart
  erhaltenen, einige Sekunden nachgehenden Zeit; kurz darauf korrigiert NTP (z. B. +7 s). `animateHand()` rechnete
  die Position als Start + Anteil des Wegs zum *aktuellen* Ziel, sodass der Sekundenzeiger bei diesem Sprung
  in einem Bild um ~18° mitsprang. Jetzt wird der Abstand zum Ziel geführt: Zielbewegungen werden in den
  Restweg eingerechnet (max. ~1,3° pro Bild), ein großer Sprung spät in der Animation startet sie neu.

### Geändert
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
- Screenshots (`screenshots/de`, `screenshots/en`) neu von uhr4 aufgenommen (vorher uhr3): 800 × 600, die Designer 800 × 1000 (Desktop-Ansicht verkleinert); englische Bilder mit englischer Browsersprache.
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
