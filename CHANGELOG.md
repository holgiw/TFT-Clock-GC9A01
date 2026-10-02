# Changelog

*English version below.*

## 2026-10-02

### Geändert
- Sicherung: Zifferblätter, Zeiger und Streifen-Grafiken liegen darin als normale BMP-Bilder (vorher im
  komprimierten Format der Uhr) und lassen sich mit jedem Bildprogramm öffnen. Beim Wiederherstellen packt die
  Uhr sie wieder wie beim Hochladen; ältere Sicherungen lassen sich weiter einspielen.
- Screenshots (`screenshots/de`, `screenshots/en`) neu aufgenommen; die Seite DCF77 stammt von einer Uhr mit
  Empfänger, dazu wieder der Zifferblatt-Designer einer ILI9341-Uhr (`zifferblatt_designer_ili.png`).
- Auswahl der mitgelieferten Zifferblätter, Zeiger und Presets überarbeitet (`graphic/` mit den Zips
  `faces_handsets_160.zip` und `faces_handsets_240.zip`, `presets.txt`).

### Behoben
- `presets.txt`: Das Preset „Antik“ wird beim Herunterladen von GitHub wieder übernommen – Name und Adresse
  waren durch ein Leerzeichen statt eines Tabs getrennt, die Zeile wurde deshalb übersprungen.

---

# English Version

## 2026-10-02

### Changed
- Backup: clock faces, hands and strip graphics are stored as normal BMP images (previously in the clock's
  compressed format) and open in any image program. On restore the clock packs them again like on upload;
  older backups can still be restored.
- Retook the screenshots (`screenshots/de`, `screenshots/en`); the DCF77 page comes from a clock with a
  receiver, plus again the clock face designer of an ILI9341 clock (`clock_face_designer_ili.png`).
- Revised the selection of included clock faces, hands and presets (`graphic/` with the zips
  `faces_handsets_160.zip` and `faces_handsets_240.zip`, `presets.txt`).

### Fixed
- `presets.txt`: the preset "Antik" is taken over again when downloading from GitHub – name and address were
  separated by a space instead of a tab, so the line was skipped.
