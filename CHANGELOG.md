# Änderungshistorie

Alle nennenswerten Änderungen an der Firmware uhr3 (TFT-Uhr mit GC9A01/GC9D01 auf ESP32-S2).
Neueste Einträge oben. Die Einträge ab 2026-09-22 sind ausführlich, ältere Zeiträume sind aus den
Commit-Betreffen zusammengefasst.

## 2026-09-28

### Hinzugefügt
- Zeiger-Designer: „Transparent“ ist eine wählbare Farbe (Schachbrett-Feld bei den Standardfarben
  oder „Transparent“ im Hex-Feld). Stift, Linie, Rahmen, Rechteck, Ellipse, Kreis, Polygon, Füllen
  und Formgenerator zeichnen damit durchsichtige Pixel. Pipette, Rechtsklick und „Aufnehmen“
  übernehmen auch einen transparenten Pixel. (Im Zifferblatt-Designer bewusst nicht: das
  Zifferblatt ist die unterste Ebene, darunter scheint nichts durch.)
- Zeigerbreite 25 px statt 21 (160er-Display: 15 statt 13), Drehpunkt-Spalte 12 (160er: 7).
  Gültig sind alle Kombinationen aus alter und neuer Breite bzw. Höhe: 21 oder 25 × 131 oder 151
  (160er: 13 oder 15 × 86 oder 100). Alte Zeiger werden beim Laden waagerecht mittig und unten
  bündig eingesetzt (`placeHand()`), der Drehpunkt bleibt dadurch unverändert.
- Zeiger-Designer: Werkzeug „Weichzeichnen“ und Button „Ganzen Zeiger weichzeichnen“ (3×3-Mittel
  nur über deckende Pixel, die Außenkante bleibt scharf – die glättet die Uhr beim Zeichnen).
- Zeiger-Designer: Werkzeuge „Kreis“ und „Kreis gefüllt“ (exakt rund, Radius = Abstand zur Maus).
- Build-Marker `UHR3_BUILD_DISPLAY=<Display>` aus den Display-Defines in `config.h`, sichtbar auf den
  Info-Seiten („Build: …“) und damit in jeder `.bin` – jedes Build ist eindeutig einem Display
  zuzuordnen.
- Dateimanager: Formatangabe hinter Zeigerdateien („altes Format“, „neues Format“, „neue Breite“,
  „neue Länge“, „ungültige Größe – Standardzeiger wird verwendet“).
- Beide Designer: vollständige französische Übersetzung (bisher Englisch bei Sprache Französisch).

### Geändert
- Zeiger- und Zifferblatt-Seite: unter jedem Zeigersatz bzw. Zifferblatt (auch dem eingebauten)
  ein Link „Designer“. Er aktiviert den Satz bzw. das Zifferblatt und öffnet direkt den Designer,
  der immer auf dem aktiven Stand aufbaut (`/sethandset?…&designer=1`,
  `/setbackground?…&designer=1`). Der bisherige Abschnitt „Zeiger-/Zifferblatt-Designer öffnen“
  unten auf den Seiten entfällt.
- Zifferblatt-Designer: „Erzeugen“ ersetzt das Zifferblatt ohne Rückfrage (mit „Rückgängig“
  zurückholbar).
- Zifferblatt-Designer: „Name:“ und das Eingabefeld bleiben in einer Zeile.
- Kontaktadresse auf Status- und Info-Seite: howl-clock@gmx.de.
- Neue Screenshots der Weboberfläche, getrennt nach Sprache in `screenshots/de/` und
  `screenshots/en/` (PNG, englische Dateinamen im Ordner `en/`): Uhr-Einstellungen, Helligkeit,
  Vorschau, Uhren-Sets, Zifferblatt, Zeiger, Zeiger- und Zifferblatt-Designer, Dateimanager.
  Alle bisherigen JPG-Screenshots entfernt.
- Zeiger-Designer: Ellipsen und Kreise werden vom Mittelpunkt aus aufgezogen; die Anzeige unter
  der Zeichenfläche nennt den Radius.
- Zeiger-Designer speichert jeden Zeiger so klein wie möglich (Breite und Höhe im alten Maß,
  solange der Zeiger nicht darüber hinausragt) – normale Zeiger laufen weiter auf älterer Firmware.
- Hochladen/Umwandlung: nur die vier gültigen Zeigergrößen bleiben unverändert, alles andere wird
  wie bisher auf 21 × 131 (13 × 86) skaliert (`handTargetSize()` statt `handTargetHeight()`).
- Skalieren-Formular im Dateimanager schlägt für Zeigerdateien deren aktuelle Größe vor (statt
  240 × 240) und warnt, dass andere Größen den Drehpunkt verschieben.
- Beide Designer: deutsche Texte mit echten Umlauten (als `ä` usw., da die Seiten ohne
  Zeichensatz ausgeliefert werden und HTML-Entities in per JavaScript gesetzten Texten nicht wirken).
- Freier-Speicher-Prüfung auf der Zeiger-Seite aus `HAND_WIDTH`/`HAND_HEIGHT` berechnet (vorher fest
  5818 Byte, eine unkomprimierte Datei im neuen Format braucht mehr).
- README und Einrichtungsanleitung auf 25 × 151 aktualisiert.

### Behoben
- Status- und Info-Seite: „daywindow“ zeigte das Ende eine Stunde zu spät (z. B. 7:00 - 23:00 bei
  eingestelltem 7–22). Die Helligkeitslogik rechnet mit Ende exklusiv (Stunde < Ende), die Anzeige
  addierte aber 1 und las die Preferences mit anderen Standardwerten (8/20 statt 7/21). Jetzt aus
  den Laufzeitwerten, mit Hinweis bei Fenster über Mitternacht bzw. „off“ bei Start = Ende.
- Fest englische Texte übersetzt: „Size (Pixel)“ und „File“ auf der Zeiger-Seite, „Uploading...
  please wait“ und „Upload BMP“ beim Hochladen, Namensschema der Zeigerdateien („Nr.“, „oder“,
  „z. B.“). Die Überschrift der Uhren-Sets hieß „Manage Uhren Sets“, jetzt „Uhren Sets verwalten“.
  Status- und Info-Seite bleiben bewusst englisch (`forceEnglish`).
- Bahnhofsuhr-Modus: Sekundenzeiger sprang ab ca. 4,7 h Laufzeit gelegentlich für ein Bild eine
  Sekunde vor und wieder zurück. Ursache: `stationLastMillis` (uint32) wurde per float fortgeschrieben;
  ab 2^24 ms rundete die 24-Bit-Mantisse, der Zeitstempel lag dann 1 ms in der Zukunft und die
  unsigned-Differenz lief über. Jetzt ganzzahlig und vorzeichenbehaftet gerechnet. (Ohne Fix wäre
  der Schritt ab ca. 12 Tagen Laufzeit dauerhaft falsch gewesen, 1024 statt 975 ms.)
- Anleitungen in den `build_*`-Ordnern (`readme.txt`, `liesmich.txt`) nannten noch das feste
  AP-Passwort `clock123` und den alten 10-s-WLAN-Reset. Jetzt wie die `readme.txt` im Hauptordner:
  Passwort pro Gerät auf dem Display, Werksreset nach 15 s.
- Alle drei Builds neu erstellt (GC9A01, GC9D01, GC9A01_WITH_BACKLIGHT; ESP32-Core 3.3.12). Das
  Backlight-Build war vorher vom 2026-09-10 und hatte keinen Build-Marker; seine `readme.txt`
  entspricht jetzt wieder der im Hauptordner.

### Repository
- Die Flash-Batch in den `build_*`-Ordnern heißt jetzt `flashESP.bat` (bisher `start.bat`) und
  nimmt die COM-Schnittstelle als Parameter (`flashESP.bat 3` für COM3, auch `flashESP.bat COM3`).
  Ohne Parameter listet sie alle COM-Schnittstellen auf und markiert angeschlossene Uhren an der
  USB-Kennung (Espressif 303A; 0002 = Download-Modus); bei genau einer Uhr wird deren Port ohne Rückfrage
  verwendet, sonst wird nach der Nummer gefragt. Ohne gefundene Uhr erscheinen Prüffragen
  (Uhr im Gerätemanager mit COM-Port? Datenkabel statt reinem Ladekabel?).
- Linux: neues `flashESP.sh` in allen `build_*`-Ordnern mit derselben Logik wie `flashESP.bat`
  (Uhr-Erkennung per USB-Kennung über `udevadm` bzw. `/sys`, 1200-Baud-Neustart per `stty`,
  Prüffragen ohne gefundene Uhr, Hinweis auf die Gruppe `dialout` bei fehlenden Rechten; unterstützt
  `esptool` ab v5 und `esptool.py`). Aufruf `bash flashESP.sh` oder `bash flashESP.sh 0` für
  `/dev/ttyACM0`; die manuellen esptool-Befehle stehen weiter in der Readme.
- `flashESP.bat`/`flashESP.sh`: Schlägt das Flashen fehl, erklären beide, wie der ESP32-S2 von Hand
  in den Bootmodus gebracht wird (Boot halten und dann USB anstecken bzw. Reset+Boot).
  Der ESP32-S2 hat zwei COM-Ports (laufend bzw. Download-Modus): Eine laufende Uhr startet
  `port.ps1` wie die Arduino IDE per 1200-Baud-Signal in den Download-Modus neu und flasht über
  den dann neu erscheinenden Port. Die Port-Logik liegt in `port.ps1` neben `flashESP.bat`. Die
  bisherigen `start_COM3.bat`/`start_COM4.bat` im GC9D01-Ordner entfallen, Readmes angepasst.
- Git-Historie bereinigt: alte Build-Artefakte (`build/` mit `.elf`/`.map`/`merged.bin`, alte
  `build_*.zip`, frühere Versionen der `.bin` in `build_*`), `uhr3.ino.lolin_s2_pico.bin` im
  Hauptordner, `__vm/`, `uhr3.vcxproj(.filters)` sowie `pictures/uhr3.mp4`, `uhr3.bmp` und alte
  `TFT_eSPI.zip`-Stände entfernt. Bestehende Klone müssen neu geklont werden.
- `__vm/` und `uhr3.vcxproj(.filters)` werden nicht mehr versioniert (bleiben lokal), ebenso
  `.map`, `.elf` und `*_flashed.bin`; `.gitattributes` markiert Binärdateien und `.bat` (CRLF).
- Zeigersätze 5–9, 12 und 13 aus `graphic/` und aus `faces_handsets_160/240.zip` entfernt
  (240er: Sätze 5–9, 12, 13; 160er: `hand_set9_minute.bmp`).

## 2026-09-27

### Hinzugefügt
- Zifferblatt-Designer (`/facedesigner`, `face_designer_html.h`): Zifferblatt-Generator (Hintergrund,
  Rand, Stunden-/Minutenstriche, Ziffern 1–12, 12/3/6/9 oder römisch), Pixel-Werkzeuge mit
  Stiftbreite, Spiegel- und Dreh-Symmetrie (4-, 12-, 60-fach), Text gerade, gedreht oder im Bogen,
  installierte oder geladene Schriften, Logo als Stempel, Bild laden, Zoom, Live-Vorschau mit den
  aktiven Zeigern. Speichert als neues Zifferblatt oder überschreibt und aktiviert das aktive.
  Bei runden Displays ist der unsichtbare Bereich markiert.
- Zeiger dürfen vom Drehpunkt bis zum Displayrand reichen: Höhe 151 (160er: 100), Drehpunkt-Zeile
  fest je Display auf dem Displayradius (120 bzw. 80). Alte Zeiger (131 bzw. 86) werden oben
  transparent aufgefüllt; `loadHandPixels()` ersetzt `loadHandBmp()` und `loadHandPixelsForPreview()`.
- Beide Designer: schnelle Farbaufnahme (Button „Aufnehmen“, Rechtsklick in die Zeichenfläche,
  Klick in die Vorschau), kurzer Hinweis je Werkzeug.
- Zeiger-Designer: Seitenleisten bleiben beim Scrollen sichtbar.
- Designer-Vorschau bewegt die Zeiger wie die Uhr: Bahnhofsuhr (Sekunde wartet auf 12),
  schleichende oder tickende Sekunde, schleichende oder springende Minute; Anzeige des aktiven Stils.

### Geändert
- Drehpunkte je Display fest in `config.h` (`HAND_LEGACY_PIVOT_Y`, `HAND_PIVOT_Y`) statt berechnet,
  mit `static_assert`-Prüfungen – sie sind Teil des Dateiformats.
- Zeiger bleiben beim Speichern und Hochladen im alten Format, wo immer möglich.
- Breitere Zahlenfelder in den Designern (Werte überlappten mit den Pfeilen).

## 2026-09-27 (erster Stand Zeiger-Designer)

### Hinzugefügt
- Zeiger-Designer (`/handdesigner`, `hand_designer_html.h`, aus dem Flash gestreamt): Pixel-Editor
  mit Stift, Radierer, Linie, Rechteck, Ellipse, Polygon, Füllen, Pipette, Spiegelung, Rückgängig,
  Formgenerator, Farbpalette und Hex-Eingabe, Zoom, Live-Vorschau auf dem aktuellen Zifferblatt.
  Basis ist der aktive Zeigersatz; speichert als neuen Satz oder überschreibt und aktiviert den
  aktiven. `/api/defaulthand` und `/api/defaultface` liefern die eingebauten Grafiken.

### Behoben
- `/preview` schneidet die Zeiger auf die Breiten des Zifferblatts zu und rechnet den
  Stundenwinkel sekundengenau wie das Display.
- Displays ohne Backlight-Pin: beim Dimmen konnte ein reines Grün genau die Transparenzfarbe
  ergeben und verschwinden (`setPixelBrightness()` weicht jetzt aus).

## 2026-09-22 bis 2026-09-26

### Hinzugefügt
- Rocrail-Modellzeit (Rocrail-Server, `<clock>`-Events, beschleunigte Modellzeit), später
  R2RNet-Multicast-Diagnose.

### Geändert
- Unveränderte Uhr-Frames werden nicht erneut gezeichnet und gesendet.
- Zeitzone wird beim Start vor dem RTC-Lesen gesetzt.
- RTC-Drift im Log in Sekunden; die RTC wird erst ab 2 s Abweichung geschrieben.
- R2RNet-Debug-Logging auf die ersten 20 Pakete je Beitritt gedrosselt.
- SNTP im Smooth-Modus (Zeitkorrektur per adjtime statt Sprung).
- Kommentare zweisprachig vereinheitlicht (je höchstens drei Zeilen DE/EN).

### Behoben
- R2RNet-Multicast-Adresse (war die reservierte All-Hosts-Gruppe), Diagnose bei Beitrittsfehlern.
- Sekundenzeiger: auch Vorwärtssprünge durch verspätete Frames werden abgefedert, nicht nur
  Rücksprünge.
- Diverse Punkte aus Code-Reviews.

## 2026-09 (bis 2026-09-11)

- Rocrail-Integration (erste Version), mehrere Code-Reviews.
- DCF77-Tab, Info-Tab, UI-Überarbeitung, DCF77-Fehler behoben.
- GC9D01-Builds, readme.txt überarbeitet.
- Zweites Display mit eigener Rotation, Fehlerbehebungen.

## 2026-07 bis 2026-08

- Live-Vorschau der Uhr auf der Webseite (GC9A01, auch mit Backlight).
- Umfassende Modernisierung der Weboberfläche (Startseite, Helligkeit, NTP-Server, Dateimanager
  mit Icons), Übersetzungen, Presets, Hostname-Prüfung.
- Webserver auf Chunk-Übertragung umgestellt, Bitmap-Kompression (RLE) für Zifferblätter und Zeiger.
- Quellkommentare zweisprachig, WLAN-Accesspoint- und WPS-Fehler behoben.
- Vollständiges Code-Review, GPLv3-Lizenz.

## 2026-01 bis 2026-06

- Zweites Display (Dual-Display).
- Boot-Taster kann den externen Taster ersetzen.
- DCF77-Unterstützung, RTC-Unterstützung.
- WPS-Unterstützung, bis zu 15 WLAN-Netze, verbesserter WLAN-Scan und Reconnect.
- Presets, Übersetzungen in `translation.h`, Versionsanzeige auf dem Display, Log mit Zeitstempel.
- Fehler im Normalmodus (ohne Bahnhofsuhr) behoben, Flashen unter Linux.

## 2025-06 bis 2025-12

- Projektstart im Repository, PSRAM-Status.
- Touch-Pin, Web-UI-Überarbeitungen, Board- und Projektkonfiguration.
- API-Schnittstelle (bis zu 15 Presets), Presets.
- Verbessertes NTP-Handling, Übersetzungen.
