# uhr4 (Version 4) – Uhr auf runden und eckigen Displays

*English version below.*

uhr4 zeigt die Uhrzeit als Zifferblatt mit Zeigern auf einem TFT-Display. Die Hardware ist ein ESP32-S2 (Lolin S2 Pico) mit rundem (GC9A01, GC9D01) oder rechteckigem Display (ILI9341), ein ESP32-C6 von Waveshare mit eingebautem ST7789 oder ein ESP32-S3 von Waveshare mit eingebautem rundem GC9A01. Auf den rechteckigen Displays (ILI9341, ST7789 172 × 320) zeigt ein Streifen neben der Uhr Uhrzeit und Datum, wahlweise über oder unter der Uhr (quer: links oder rechts). Die Zeit kommt per NTP (WLAN), DCF77 oder RTC. Zifferblätter, Zeiger und alle Einstellungen werden über die Weboberfläche angepasst.

**Download:** Firmware mit Flash-Tool unter [Releases](https://github.com/holgiw/ESP32-Station-Clock/releases/latest) oder im Ordner `build_uhr4` (Anleitung: `build_uhr4/readme.txt`). Beim Update von uhr3 (Version 3) übernimmt uhr4 den Displaytyp.

---

## 1. Unterstützung mehrerer TFT-Displays

Eine Firmware je ESP. Den Displaytyp wählt man im Tab „Uhr Einstellungen“ oder beim Flashen mit `flashESP`; beim Update von uhr3 wird er übernommen. Grafikbibliothek: LovyanGFX.

- **ESP32-S2** (Lolin S2 Pico, Build `build_uhr4/esp32s2`):
  - GC9A01 – rund, 240 × 240; ohne oder mit geregelter Hintergrundbeleuchtung (BL)
  - GC9D01 – rund, 160 × 160; BL geregelt
  - ILI9341 – rechteckig, 240 × 320; Uhr 240 × 240 (Zifferblätter und Zeiger wie beim GC9A01) und Streifen mit Uhrzeit und Datum; feste Beleuchtung
- **ESP32-C6** (Waveshare, Display fest verbaut, Build `build_uhr4/esp32c6`, ohne PSRAM; BL geregelt):
  - ESP32-C6-LCD-1.47: ST7789 – rechteckig, 172 × 320; Uhr 172 × 172 und Streifen mit Uhrzeit und Datum
  - ESP32-C6-LCD-1.3: ST7789_240 – quadratisch, 240 × 240
- **ESP32-S3** (Waveshare ESP32-S3-LCD-1.28, Display fest verbaut, Build `build_uhr4/esp32s3`, 16 MB Flash, 2 MB PSRAM; BL geregelt; noch nicht am Gerät erprobt):
  - GC9A01 – rund, 240 × 240
  - Flash mit zwei App-Partitionen und 9,9 MB für Zifferblätter, Zeiger und Schriften
  - Firmware-Update über WLAN (siehe Abschnitt 10)
  - Rotation von Hand oder „automatisch“ über den eingebauten Lagesensor (QMI8658): Das Zifferblatt steht aufrecht, egal wie die Uhr hängt; liegt sie flach, bleibt die Lage. Vorher die passende Rotation von Hand wählen und speichern, dann auf „automatisch“ stellen – die Uhr nimmt diese Lage als Bezug.
- Zweites, baugleiches Display (nur ESP32-S2) mit eigener Rotation (0°, 90°, 180°, 270°). Steht ein Display auf „nicht angeschlossen (n.a.)“, bleibt es schwarz, und die Uhr zeichnet und berechnet nichts dafür. Standard: Display 1 mit 0°, Display 2 n.a. Status- und Startmeldungen (Start, Access-Point-Modus) erscheinen bis zum Uhrstart trotzdem auf beiden Displays; die Einstellungsseite weist darauf hin.

---

## 2. Anpassbare Zeiger und Zifferblätter

- Stunden-, Minuten- und Sekundenzeiger und Zifferblätter lassen sich als BMP hochladen, z. B. im Layout der Deutschen Bahn oder anderer Gesellschaften, oder im Designer erstellen.
- Startpaket: Hat die Uhr keine Zifferblätter und Zeigersätze (nach dem ersten Flashen oder einem Werksreset), zeichnet sie sich selbst
  - drei Zifferblätter, weiß mit Rand, Stunden- und Minutenstrichen: `face_default.bmp` (12, 3, 6, 9), `face_numbers.bmp` (1–12), `face_roman.bmp` (I–XII),
  - drei Zeigersätze: Satz 0 mit schwarzen Balken und rotem Sekundenzeiger, Satz 1 ebenso mit schwarzem Sekundenzeiger, Satz 2 geschwungen,
  - ohne vorhandene Presets die Uhren Sets „Standard“, „1-12“ und „I-XII“ (I-XII ohne Sekundenzeiger; auf Displays mit Streifen – ILI9341, ST7789 172 × 320 – mit weißem Streifen und schwarzer Schrift).

  Die Werksreset-Aktionen „Zifferblätter löschen“, „Zeigersätze löschen“ und „Uhren Sets löschen“ stellen diesen Zustand wieder her. Fehlen nur `face_default.bmp` oder Satz 0, erzeugt die Uhr nur diese neu. Fehlt einem Zeigersatz ein Zeiger, nimmt sie den aus Satz 0.
- Alle Zeiger sind kantengeglättet (LovyanGFX). Stunden- und Minutenzeiger liegen in einem zwischengespeicherten Bild, zum Display geht nur der Bereich um den Sekundenzeiger. Deshalb läuft der schwingende Sekundenzeiger flüssig.
- Zeigerformat 25 × 151 px mit Drehpunkt 12 / 120 (160er-Display: 15 × 100, Drehpunkt 7 / 80): Zeiger reichen vom Drehpunkt bis zum Displayrand. Zeigersätze im älteren Format 21 × 131 (13 × 86) funktionieren weiter; die Uhr füllt sie beim Laden oben und seitlich transparent auf. Gültig sind alle Kombinationen aus Breite 21 oder 25 und Höhe 131 oder 151. Diese Größen bleiben beim Hochladen unverändert, alle anderen skaliert die Uhr auf 21 × 131 (13 × 86). Der Zeiger-Designer speichert im alten Maß, solange kein Zeiger darüber hinausragt – solche Sätze laufen auch auf älterer Firmware.
- Zeiger-Designer im Browser (Seite „Zeiger“ → Link „Designer“ unter dem Zeigersatz; der Satz wird dabei aktiviert): neue Zeiger auf Basis des aktiven Satzes gestalten. Speichern als neuer Zeigersatz (wahlweise gleich aktiviert) oder über den aktiven Satz, der dann sofort gilt. Werkzeuge: Stift, Linie, Rechteck, Ellipse, Polygon, Füllen, Pipette, Farbwahl (Palette, Farbwähler, Hex-Eingabe), Spiegeln an der Mittelachse, Rückgängig, Formgenerator (Länge, Breiten, Gegengewicht, Scheibe) und Live-Vorschau auf dem aktuellen Zifferblatt. Zeigergröße und Drehpunkt passt die Uhr an ihr Display an.
- Zifferblatt-Designer im Browser (Seite „Zifferblatt“ → Link „Designer“ unter dem Zifferblatt; es wird dabei aktiviert): ein neues Zifferblatt auf Basis des aktiven gestalten oder das aktive überschreiben und sofort anwenden. Werkzeuge: Generator (Hintergrund, Rand, Stunden- und Minutenstriche, Ziffern 1–12, 12/3/6/9 oder römisch), Pixel-Werkzeuge mit Stiftbreite, Text (gerade, gedreht oder im Bogen, mit installierten oder geladenen Schriften), Logo-Stempel, Spiegel- und Dreh-Symmetrie (4-, 12- oder 60-fach), Farbwahl, Bild laden (PNG, JPG, BMP …), Zoom und Live-Vorschau mit den aktiven Zeigern. Auf runden Displays ist der unsichtbare Bereich markiert.
- Die Nabe (Mittelpunkt über den Zeigern) wird ebenfalls im Zifferblatt-Designer eingestellt: Radius und Farbe; die Uhr zeigt Änderungen sofort.

---

## 3. Sanfter Minutenzeiger und Bahnhofsuhr-Modus

- Sanfter Minutenzeiger: läuft gleichmäßig, statt jede Minute zu springen.
- Bahnhofsuhr-Modus: Der Sekundenzeiger umrundet das Zifferblatt in 58,5 Sekunden und hält kurz auf der 12.
- Zeitsprünge (Start, erste gültige Uhrzeit, Zeitumstellung, Rocrail): Die Zeiger laufen in 3 Sekunden auf dem kürzesten Weg zur neuen Zeit, auch rückwärts.

---

## 4. Helligkeitssteuerung

- Automatische Helligkeit über einen Fotowiderstand mit einstellbaren Schwellwerten, oder manuell.
- Hintergrundbeleuchtung per PWM: Die Uhr dimmt dann die Beleuchtung statt die Pixel. Der Helligkeits-Tab zeigt dafür den Haken „Hintergrundbeleuchtung regeln“ mit der Pinnummer.
  - ESP32-S2: Pin 3, bei den Displaytypen „GC9A01 mit BL“ und GC9D01 ab Werk an.
  - ESP32-C6: Pin 22, auf dem Waveshare-Board fest verdrahtet, ab Werk an.
  - ESP32-S3: Pin 40, auf dem Waveshare-Board fest verdrahtet, ab Werk an.
- Fotowiderstand als Spannungsteiler mit 10 kΩ: ESP32-S2 an GPIO 1 (3 V), 2 (Messung), 4 (GND); ESP32-C6 an GPIO 1 (3 V), 2 (Messung), 3 (GND) der Stiftleiste; ESP32-S3 an GPIO 2 (3 V), 4 (Messung), 5 (GND) der Stiftleiste. Ohne Fotowiderstand gilt die manuelle Helligkeit.
- Während der Einrichtung (noch keine gültige Uhrzeit, Access Point oder WPS aktiv) leuchtet das Display mit PWM-Beleuchtung mit 50 %, sonst mit voller Helligkeit.
- Die Gamma-Korrektur gibt es nur mit Lichtsensor und PWM-Beleuchtung. ESP32-C6: Über 128 warnt die Helligkeitsseite vor Überhitzung der Beleuchtung (Hinweis von Waveshare).
- Mit Rocrail kann der Server die Helligkeit vorgeben (Abschnitt 11).

---

## 5. WLAN- und NTP-Integration

- Bis zu 15 WLAN-Netze, Einrichtung auch per WPS, automatischer Reconnect, eigener Hostname.
- Einrichtung beim Flashen: `flashESP.bat`/`flashESP.sh` erkennen ESP32-S2, ESP32-C6 und ESP32-S3 am USB-Port, fragen Displaytyp (nicht beim ESP32-S3) und WLAN ab (Auswahl aus allen sichtbaren 2,4-GHz-Netzen, Passwort verdeckt) und senden beides per USB an die Uhr, zuletzt die Uhrzeit des PCs.
- Nur die Uhrzeit setzen, z. B. bei einer Uhr ohne WLAN, DCF77 und RTC: `setTime.bat`/`setTime.sh` oder der Knopf „Uhrzeit übernehmen“ in der Weboberfläche (Zeit des Handys bzw. PCs, auch im Access-Point-Modus).
- Weitere Wege ins WLAN: WPS oder der Einrichtungs-Access-Point (SSID `clock123`, Passwort `clocksetup`). Den Access Point startet die Uhr nur beim Start, wenn kein WLAN gespeichert oder keins erreichbar ist (mit RTC oder mit Zeit aus einem offenen WLAN nur ohne gespeichertes WLAN). Bricht das WLAN im Betrieb weg, bleibt die Uhr im WLAN-Modus und versucht es mit „WLAN neu verbinden“ erneut – zuerst 5 Minuten nach dem Start, danach stündlich, im AP-Modus nur stündlich – der AP bleibt dabei erreichbar.
- Änderungen an den WLAN-Netzen (hinzufügen, überschreiben, wechseln, löschen) gelten sofort. Die Weboberfläche unterscheidet nicht zwischen Zugriff aus dem Heimnetz und von außen – die Uhr deshalb nicht per Port-Weiterleitung oder DMZ aus dem Internet erreichbar machen.
- NTP, bei Ausfall DCF77. Sobald die Uhr eine gültige Uhrzeit hat, ist sie selbst NTP-Server für andere Geräte im Netz.
- Uhrzeit aus offenen WLANs (Tab „NTP Zeitzone“, ab Werk an): Kann sich die Uhr beim Start mit keinem gespeicherten WLAN verbinden (außer Reichweite oder kein Zugang, z. B. falsches Passwort) und ist keine RTC eingebaut, verbindet sich die Uhr kurz mit bis zu vier offenen WLANs (gleiche Namen nur einmal, je 15 s) und holt die Zeit per NTP oder aus dem `Date:`-Kopf einer HTTP- bzw. HTTPS-Antwort, auch der Anmeldeseite eines Hotspots. Danach trennt sie wieder und läuft mit dieser Zeit normal weiter, ohne WPS und Access Point. Jeder Schritt steht im Log (Logging einschalten). Das sind fremde Netze – nur nutzen, wenn erlaubt.
- Bis zu 15 eigene NTP-Server. Ohne eigenen Server nutzt die Uhr `pool.ntp.org` und `ptbtime1.ptb.de`; sind die eigenen Server nicht erreichbar, versucht sie diese beiden zuletzt.

---

## 6. Weboberfläche

- Einstellungen in Tabs mit dunklem Design: WLAN, Uhr Einstellungen, Helligkeit, NTP Zeitzone, Status, Log und Rocrail (bei aktivierter Anbindung). Weitere Seiten über die Navigation: Vorschau, Uhren Sets, Zifferblatt, Zeiger, Dateimanager, DCF77, Sicherung, Werkseinstellungen.
- Deutsch und Englisch.
- Weitere Zifferblätter und Zeigersätze lassen sich direkt von GitHub laden.
- Rocrail-Tab für die Modellzeit (Abschnitt 11).

---

## 7. Dateiverwaltung mit LittleFS

- Zifferblätter und Zeiger liegen komprimiert im Dateisystem. Dateien lassen sich in der Weboberfläche hoch- und herunterladen, umbenennen und löschen.
- Logging optional, im Log-Tab mit Auswahl der Logdatei (neueste vorausgewählt) und Auto-Refresh.

---

## 8. Zeitzonen-Anpassung

- Automatische Sommerzeit oder dauerhaft Sommer- bzw. Winterzeit.
- Ohne Zeitzone (leeres Feld) oder bei einem ungültigen POSIX-TZ-String nutzt die Uhr `CET-1CEST,M3.5.0,M10.5.0/3` (Mitteleuropäische Zeit).

---

## 9. Hardware-Integration

- **ESP32-S2** (Lolin S2 Pico):
  - GC9A01 (rund, 240 × 240)
  - GC9D01 (rund, 160 × 160)
  - ILI9341 (rechteckig, 240 × 320)
  - Verdrahtung siehe `build_uhr4/readme.txt`, Platine im Ordner `PCB`
- **ESP32-C6** (Waveshare, Display fest verbaut):
  - [ESP32-C6-LCD-1.47](https://docs.waveshare.com/ESP32-C6-LCD-1.47/Resources-And-Documents) mit ST7789 (172 × 320)
  - [ESP32-C6-LCD-1.3](https://docs.waveshare.com/ESP32-C6-LCD-1.3/Resources-And-Documents) mit ST7789 (240 × 240)
- **ESP32-S3** (Waveshare, Display fest verbaut):
  - [ESP32-S3-LCD-1.28](https://docs.waveshare.com/ESP32-S3-LCD-1.28) mit GC9A01 (240 × 240, rund)
- Fotowiderstand für die Helligkeit; die Uhr erkennt ihn beim Start.
- Optional: zweites, baugleiches Display (nur ESP32-S2, eigener Chip-Select), RTC DS3231 (hält die Uhrzeit über Stromausfälle, auch ohne WLAN), DCF77-Empfänger (Funkzeit ohne Internet).
- Taster (oder der eingebaute Boot-Taster), je nach Haltedauer:
  - kurz: zeigt das verbundene WLAN,
  - 10–20 Sekunden: gelber Countdown „WiFi Reset“, Loslassen bricht ab,
  - 20–30 Sekunden: roter Countdown „Factory Reset“; jetzt loslassen löscht alle gespeicherten WLANs und startet neu – die Uhr geht in WPS/Access Point, alles andere bleibt,
  - ab 30 Sekunden: vollständiger Werksreset.

---

## 10. Weitere Funktionen

- Anzeige der Laufzeit seit dem letzten Neustart.
- Neustart über die Weboberfläche.
- Firmware-Update über WLAN (nur ESP32-S3): auf der Seite „Sicherung“ die Datei `uhr4.ino.bin` aus `esp32s3` hochladen, mit Fortschrittsbalken; oder aus Visual Micro bzw. der Arduino IDE über den Netzwerk-Port der Uhr (ArduinoOTA, Port 3232, ohne Passwort). Die neue Firmware landet in der zweiten App-Partition und startet erst, wenn sie vollständig und für den ESP32-S3 ist; Einstellungen, Zifferblätter und Zeigersätze bleiben. Hängt ein Update, startet die Uhr nach 3 Minuten mit der alten Firmware neu.
- Hochgeladene BMP-Dateien lassen sich auf die Displaygröße skalieren.
- HTTP-API, z. B. `/api/setMode` (Zifferblatt, Zeigersatz, Zeigermodus, Nabe); die Uhren Sets sind solche URLs.
- Bis zu 50 Uhren Sets (Presets) mit Zifferblatt, Zeigersatz, Nabenfarbe und -größe, Sekundenzeiger (sichtbar, Stil), Bahnhofsmodus, sanftem Minutenzeiger, Zeitzone und Helligkeit. Einzeln umbenennen und löschen, alphabetisch sortiert; alle zusammen in eine Datei sichern und wiederherstellen. Sind alle 50 Plätze belegt, erscheint eine Warnung.
- Komplettsicherung (Seite „Sicherung“):
  - Eine TAR-Datei mit allen Einstellungen, Uhren Sets, Zifferblättern, Zeigersätzen und Schriften; Zifferblätter und Zeiger darin als normale BMP-Bilder. Der Dateiname enthält Hostname, Displaytyp, Datum und Uhrzeit.
  - Wiederherstellen auf dieser oder einer anderen Uhr; Sichern und Wiederherstellen zeigen einen Fortschrittsbalken. Vorhandene Zifferblätter, Zeigersätze und Uhren Sets bleiben; gleichnamige aus der Sicherung ersetzen sie.
  - WLAN-Zugangsdaten nur auf Wunsch, verschlüsselt mit einem Schlüssel, der in jeder uhr4-Firmware gleich ist – also nicht sicher.
  - Displaytyp, Rotation, Hintergrundbeleuchtung und Lichtsensor der Ziel-Uhr bleiben. Eine falsche oder beschädigte Datei ändert nichts.
  - Sicherung von einem anderen Displaytyp: Die Uhr weist darauf hin und übernimmt nur Zifferblätter, Zeiger (auf ihre Größe skaliert), Uhren Sets und allgemeine Einstellungen. Die Nabengröße (auch in den Uhren Sets) rechnet sie um, die Helligkeit bleibt. Das geht nur mit Zifferblättern in 240 × 240, sonst übernimmt sie nichts. Hat die Sicherung keinen Streifen, bekommt eine Uhr mit Streifen den Standard-Streifen.
- Wird ein Zifferblatt oder Zeigersatz gelöscht, löscht die Uhr auch die Uhren Sets, die darauf verweisen. War der gelöschte Zeigersatz aktiv, schaltet sie auf Satz 0.
- DCF77: Empfang auch bei schwachem oder gestörtem Signal. Die Uhr ordnet die Impulse einem Sekundenraster zu, ein fehlender Impuls verschiebt also nicht die folgenden Bits. Die Signalpolarität spielt keine Rolle. Ein gestörtes Telegramm setzt nie eine falsche Zeit.
- Live-Seite `/dcf77`: Bit-Fortschritt des aktuellen und das zuletzt dekodierte Telegramm.
- Optional blinkt eine LED während der Synchronisation, aber nur, wenn ein DCF77-Empfänger erkannt wurde.

---

## 11. Rocrail-Modellzeit

- Optionale Verbindung zu einem [Rocrail](https://wiki.rocrail.net/)-Server (Modellbahn-Steuerung): Die Zeiger zeigen dann die „Fast Clock“ (Modellzeit) des Servers. Einschalten mit dem Haken „Rocrail“ im Tab „Uhr Einstellungen“.
- Bis zu 15 Serveradressen; wie bei den NTP-Servern erscheint nach dem letzten Eintrag ein leerer Platz. Ein Radio-Button je Zeile wählt den aktiven Server. Die Uhr verbindet sich sofort beim Speichern, Aktivieren oder Neustart, nicht erst im minütlichen Zeitfenster.
- Kommen länger als 2 Minuten keine Updates, läuft die Uhr mit der NTP-/RTC-/DCF77-Zeit weiter. Die Modellzeit läuft mit dem Beschleunigungsfaktor von Rocrail („Divider“), die Bahnhofsuhr-Animation des Sekundenzeigers entsprechend schneller. Oberhalb eines Schwellwerts blendet die Uhr den Sekundenzeiger aus.
- Der Tab zeigt Verbindungsstatus, Divider und Modellzeit live. Zu jedem Server lässt sich ein Anlagenname eintragen.
- Meldet der Server einen Helligkeitswert, übernimmt die Uhr ihn (Abschnitt 4). Die Seite „Vorschau“ zeigt Modellzeit und Divider ebenso.

---

## Lizenz

uhr4 steht unter der GNU General Public License v3.0 (siehe [LICENSE](LICENSE)): Du darfst es nutzen, ändern und
weitergeben – geänderte Versionen, auch als fertige Firmware, nur zusammen mit ihrem Quellcode und unter derselben
Lizenz. Die Lizenzen der enthaltenen Bibliotheken und Schriften (LovyanGFX, RTClib, arduino-esp32, DejaVu,
FreeSans, Orbitron) stehen in [THIRD_PARTY_LICENSES.md](THIRD_PARTY_LICENSES.md).

---
---

# English Version

uhr4 shows the time as a clock face with hands on a TFT display. The hardware is an ESP32-S2 (Lolin S2 Pico) with a round (GC9A01, GC9D01) or rectangular display (ILI9341), a Waveshare ESP32-C6 with a built-in ST7789, or a Waveshare ESP32-S3 with a built-in round GC9A01. On the rectangular displays (ILI9341, ST7789 172 × 320) a strip next to the clock shows time and date, either above or below the clock (landscape: left or right). The time comes via NTP (WiFi), DCF77 or RTC. Clock faces, hands and all settings are adjusted in the web interface.

**Download:** firmware with flash tool under [Releases](https://github.com/holgiw/ESP32-Station-Clock/releases/latest) or in the folder `build_uhr4` (instructions: `build_uhr4/readme.txt`). When updating from uhr3 (version 3), uhr4 keeps the display type.

## 1. Support for Multiple TFT Displays

One firmware per ESP. The display type is chosen in the "Clock Setup" tab or when flashing with `flashESP`; an update from uhr3 keeps it. Graphics library: LovyanGFX.

- **ESP32-S2** (Lolin S2 Pico, build `build_uhr4/esp32s2`):
  - GC9A01 – round, 240 × 240; without or with controlled backlight (BL)
  - GC9D01 – round, 160 × 160; BL controlled
  - ILI9341 – rectangular, 240 × 320; clock 240 × 240 (clock faces and hands as for the GC9A01) and a strip with time and date; fixed backlight
- **ESP32-C6** (Waveshare, display built in, build `build_uhr4/esp32c6`, without PSRAM; BL controlled):
  - ESP32-C6-LCD-1.47: ST7789 – rectangular, 172 × 320; clock 172 × 172 and a strip with time and date
  - ESP32-C6-LCD-1.3: ST7789_240 – square, 240 × 240
- **ESP32-S3** (Waveshare ESP32-S3-LCD-1.28, display built in, build `build_uhr4/esp32s3`, 16 MB flash, 2 MB PSRAM; BL controlled; not yet tried on the device):
  - GC9A01 – round, 240 × 240
  - flash with two app partitions and 9.9 MB for clock faces, hands and fonts
  - firmware update over WiFi (see section 10)
  - rotation by hand or "automatic" via the built-in motion sensor (QMI8658): the clock face stays upright however the clock hangs; lying flat, the orientation stays. First select and save the fitting rotation by hand, then switch to "automatic" – the clock takes this position as its reference.
- A second, identical display (ESP32-S2 only) with its own rotation (0°, 90°, 180°, 270°). A display set to "not connected (n.a.)" stays black, and the clock neither draws nor calculates anything for it. Default: display 1 at 0°, display 2 n.a. Status and boot messages (boot, access point mode) still appear on both displays until the clock starts; the settings page notes this.

---

## 2. Customizable Clock Hands and Faces

- Hour, minute and second hands and clock faces can be uploaded as BMP files, e.g. in the layout of Deutsche Bahn or other companies, or created in the designer.
- Starter set: if the clock has no clock faces and hand sets (after the first flash or a factory reset), it draws itself
  - three clock faces, white with a rim, hour and minute marks: `face_default.bmp` (12, 3, 6, 9), `face_numbers.bmp` (1–12), `face_roman.bmp` (I–XII),
  - three hand sets: set 0 with black bars and a red second hand, set 1 the same with a black second hand, set 2 curved,
  - if there are no presets yet, the presets "Standard", "1-12" and "I-XII" (I-XII without a second hand; on displays with a strip – ILI9341, ST7789 172 × 320 – with a white strip and black text).

  The factory reset actions "Delete Clock Faces", "Delete Hand Sets" and "Delete Presets" restore this state. If only `face_default.bmp` or set 0 are missing, the clock only recreates those. If a hand set lacks a hand, it uses the one from set 0.
- All hands are anti-aliased (LovyanGFX). Hour and minute hands live in a cached image, and only the area around the second hand goes to the display. That keeps the sweeping second hand smooth.
- Hand format 25 × 151 px with pivot 12 / 120 (160 display: 15 × 100, pivot 7 / 80): hands reach from the pivot to the display edge. Hand sets in the older format 21 × 131 (13 × 86) keep working; the clock pads them transparent at the top and the sides when loading. All combinations of width 21 or 25 and height 131 or 151 are valid. These sizes stay unchanged on upload, the clock scales all others to 21 × 131 (13 × 86). The hand designer saves at the old size as long as no hand extends beyond it – such sets also run on older firmware.
- In-browser hand designer ("Hand Set" page → "Designer" link below the hand set, which also activates it): design new hands based on the active set. Save as a new hand set (optionally activated right away) or over the active set, which then applies immediately. Tools: pen, line, rectangle, ellipse, polygon, fill, picker, colour choice (palette, colour picker, hex input), mirroring at the centre axis, undo, a shape generator (length, widths, counterweight, disc) and a live preview on the current clock face. The clock adapts hand size and pivot to its display.
- In-browser clock face designer ("Clock Face" page → "Designer" link below the clock face, which also activates it): design a new clock face based on the active one, or overwrite the active one and apply it right away. Tools: generator (background, rim, hour and minute marks, numerals 1-12, 12/3/6/9 or Roman), pixel tools with pen width, text (straight, rotated or on an arc, with installed or loaded fonts), logo stamp, mirror and rotation symmetry (4, 12 or 60 times), colour choice, image import (PNG, JPG, BMP ...), zoom and a live preview with the active hands. On round displays the invisible area is marked.
- The hub (centre over the hands) is also set in the clock face designer: radius and colour; the clock shows changes right away.

---

## 3. Smooth Minute and Train Station Modes

- Smooth minute hand: moves evenly instead of jumping every minute.
- Train station mode: the second hand goes round in 58.5 seconds and pauses briefly at the 12.
- Time jumps (boot, first valid time, DST change, Rocrail): the hands move to the new time in 3 seconds along the shortest path, backwards too.

---

## 4. Brightness Control

- Automatic brightness via a photoresistor with configurable thresholds, or manual.
- Backlight via PWM: the clock then dims the backlight instead of the pixels. The Brightness tab shows the "Backlight control" checkbox with the pin number for this.
  - ESP32-S2: pin 3, on by default for the display types "GC9A01 with BL" and GC9D01.
  - ESP32-C6: pin 22, hard-wired on the Waveshare board, on by default.
  - ESP32-S3: pin 40, hard-wired on the Waveshare board, on by default.
- Photoresistor as a voltage divider with 10 kΩ: ESP32-S2 on GPIO 1 (3 V), 2 (measurement), 4 (GND); ESP32-C6 on GPIO 1 (3 V), 2 (measurement), 3 (GND) of the pin header; ESP32-S3 on GPIO 2 (3 V), 4 (measurement), 5 (GND) of the pin header. Without a photoresistor the manual brightness applies.
- During setup (no valid time yet, access point or WPS active) the display runs at 50 % with PWM backlight, otherwise at full brightness.
- Gamma correction only exists with a light sensor and PWM backlight. ESP32-C6: above 128 the brightness page warns of the backlight overheating (note from Waveshare).
- With Rocrail, the server can set the brightness (section 11).

---

## 5. WiFi and NTP Integration

- Up to 15 WiFi networks, setup via WPS as well, automatic reconnect, custom hostname.
- Setup while flashing: `flashESP.bat`/`flashESP.sh` recognize the ESP32-S2, ESP32-C6 and ESP32-S3 by the USB port, ask for the display type (not on the ESP32-S3) and a WiFi network (choice of all visible 2.4 GHz networks, password hidden) and send both to the clock via USB, finally the PC's time.
- To only set the time, e.g. on a clock without WiFi, DCF77 and RTC: `setTime.bat`/`setTime.sh` or the "Use device time" button in the web interface (time of the phone or PC, also in access point mode).
- Other ways into WiFi: WPS or the setup access point (SSID `clock123`, password `clocksetup`). The clock only starts the access point at boot when no WiFi is stored or none is reachable (with an RTC or with a time from an open WiFi only without a stored WiFi). If WiFi drops during operation, the clock stays in WiFi mode and retries with "Reconnect WiFi" – first 5 minutes after boot, then hourly, in AP mode only hourly – the AP stays reachable meanwhile.
- Changes to the WiFi networks (add, overwrite, switch, delete) apply immediately. The web interface does not distinguish between access from the home network and from outside – so do not expose the clock to the internet via a port forward or DMZ.
- NTP, with DCF77 as a fallback. Once the clock has a valid time, it is an NTP server for other devices on the network.
- Time from open WiFis ("NTP Timezone" tab, on by default): if the clock cannot connect to any stored WiFi at boot (out of range or no access, e.g. a wrong password) and there is no RTC, the clock briefly connects to up to four open WiFis (each name once, 15 s each) and gets the time via NTP or from the `Date:` header of an HTTP or HTTPS reply, including the login page of a hotspot. Then it disconnects again and runs normally with that time, without WPS and access point. Every step is in the log (enable logging). These are networks of others – only use it if allowed.
- Up to 15 custom NTP servers. Without a custom server the clock uses `pool.ntp.org` and `ptbtime1.ptb.de`; if the custom servers are unreachable, it tries these two last.

---

## 6. Web Interface

- Settings in tabs with a dark theme: WiFi Settings, Clock Setup, Brightness, NTP Timezone, Status, Log and Rocrail (when the connection is enabled). Further pages via the navigation bar: Preview, Presets, Clock Face, Hand Set, File Manager, DCF77, Backup, Factory Reset.
- German and English.
- Additional clock faces and hand sets can be downloaded directly from GitHub.
- Rocrail tab for the model time (section 11).

---

## 7. File Management with LittleFS

- Clock faces and hands are stored compressed in the file system. Files can be uploaded, downloaded, renamed and deleted in the web interface.
- Optional logging, in the Log tab with a log file selector (newest preselected) and auto-refresh.

---

## 8. Time Zone Customization

- Automatic daylight saving time, or permanent summer or winter time.
- Without a time zone (empty field) or with an invalid POSIX TZ string the clock uses `CET-1CEST,M3.5.0,M10.5.0/3` (Central European Time).

---

## 9. Hardware Integration

- **ESP32-S2** (Lolin S2 Pico):
  - GC9A01 (round, 240 × 240)
  - GC9D01 (round, 160 × 160)
  - ILI9341 (rectangular, 240 × 320)
  - Wiring see `build_uhr4/readme.txt`, PCB in the `PCB` folder
- **ESP32-C6** (Waveshare, display built in):
  - [ESP32-C6-LCD-1.47](https://docs.waveshare.com/ESP32-C6-LCD-1.47/Resources-And-Documents) with ST7789 (172 × 320)
  - [ESP32-C6-LCD-1.3](https://docs.waveshare.com/ESP32-C6-LCD-1.3/Resources-And-Documents) with ST7789 (240 × 240)
- **ESP32-S3** (Waveshare, display built in):
  - [ESP32-S3-LCD-1.28](https://docs.waveshare.com/ESP32-S3-LCD-1.28) with GC9A01 (240 × 240, round)
- Photoresistor for brightness; the clock detects it at startup.
- Optional: a second, identical display (ESP32-S2 only, own chip select), RTC DS3231 (keeps the time across power loss, also without WiFi), DCF77 receiver (radio time without internet).
- Button (or the built-in Boot button), depending on how long it is held:
  - short: shows the connected WiFi,
  - 10–20 seconds: yellow "WiFi Reset" countdown, releasing aborts,
  - 20–30 seconds: red "Factory Reset" countdown; releasing now deletes all stored WiFi networks and restarts – the clock goes into WPS/access point, everything else stays,
  - from 30 seconds: full factory reset.

---

## 10. More Features

- Uptime since the last restart.
- Restart via the web interface.
- Firmware update over WiFi (ESP32-S3 only): upload the file `uhr4.ino.bin` from `esp32s3` on the "Backup" page, with a progress bar; or from Visual Micro or the Arduino IDE via the clock's network port (ArduinoOTA, port 3232, without password). The new firmware goes to the second app partition and only starts if it is complete and for the ESP32-S3; settings, clock faces and hand sets stay. If an update hangs, the clock restarts with the old firmware after 3 minutes.
- Uploaded BMP files can be scaled to the display size.
- HTTP API, e.g. `/api/setMode` (clock face, hand set, hand mode, hub); presets are such URLs.
- Up to 50 presets with clock face, hand set, hub colour and size, second hand (visible, style), station mode, smooth minute hand, time zone and brightness. Rename and delete them individually, sorted alphabetically; back up all of them to one file and restore them. A warning appears once all 50 slots are full.
- Full backup (Backup page):
  - One TAR file with all settings, presets, clock faces, hand sets and fonts; clock faces and hands inside as normal BMP images. The file name contains host name, display type, date and time.
  - Restore on this or another clock; backup and restore show a progress bar. Existing clock faces, hand sets and presets stay; same-named ones from the backup replace them.
  - WiFi credentials only on request, encrypted with a key that is the same in every uhr4 firmware – so not secure.
  - Display type, rotation, backlight and light sensor of the target clock stay. A wrong or damaged file changes nothing.
  - Backup from another display type: the clock points this out and only takes over clock faces, hands (scaled to its size), presets and general settings. It converts the hub size (also in the presets), the brightness stays. This only works with clock faces in 240 × 240, otherwise it takes over nothing. If the backup has no strip, a clock with a strip gets the standard strip.
- Deleting a clock face or hand set also deletes the presets that refer to it. If the deleted hand set was active, the clock switches to set 0.
- DCF77: reception even with a weak or disturbed signal. The clock places the pulses on a one-second grid, so a missing pulse does not shift the following bits. The signal polarity does not matter. A disturbed telegram never sets a wrong time.
- Live page `/dcf77`: bit progress of the current telegram and the last decoded one.
- Optional LED blink during synchronization, only if a DCF77 receiver was detected.

---

## 11. Rocrail Model Time

- Optional connection to a [Rocrail](https://wiki.rocrail.net/) server (model railway control): the hands then show the server's "fast clock" (model time). Enable it with the "Rocrail" checkbox in the "Clock Setup" tab.
- Up to 15 server addresses; as with the NTP servers, an empty slot appears after the last entry. A radio button per row selects the active server. The clock connects right away on save, enable or restart, not only in the once-a-minute window.
- If no updates arrive for more than 2 minutes, the clock continues with the NTP/RTC/DCF77 time. Model time runs at Rocrail's acceleration factor (the "divider"), the station-clock second-hand animation correspondingly faster. Above a threshold the clock hides the second hand.
- The tab shows connection status, divider and model time live. Each server can get a layout name.
- If the server reports a brightness value, the clock takes it over (section 4). The Preview page shows model time and divider the same way.

---

## License

uhr4 is licensed under the GNU General Public License v3.0 (see [LICENSE](LICENSE)): you may use, modify and
share it – modified versions, including ready-made firmware, only together with their source code and under the
same license. The licenses of the included libraries and fonts (LovyanGFX, RTClib, arduino-esp32, DejaVu,
FreeSans, Orbitron) are listed in [THIRD_PARTY_LICENSES.md](THIRD_PARTY_LICENSES.md).
