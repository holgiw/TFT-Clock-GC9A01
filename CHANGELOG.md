# Änderungshistorie

Alle nennenswerten Änderungen an der Firmware uhr4 (TFT-Uhr mit GC9A01/GC9D01 auf ESP32-S2).
Neueste Einträge oben.

## 2026-09-29

### Geändert
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
