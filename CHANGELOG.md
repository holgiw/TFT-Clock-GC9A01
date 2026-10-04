# Changelog

*English version below.*

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
