# uhr4 (Version 4) – Uhr auf rundem Display

uhr4 ist ein digitales Uhrenprojekt auf rundem Display: Ein ESP32-S2 (Lolin S2 Pico) zeigt die Uhrzeit als Zifferblatt mit Zeigern auf einem runden TFT-Display (GC9A01 oder GC9D01). Die Uhrzeit kommt per NTP (WLAN), DCF77 oder RTC; Zifferblätter, Zeiger und alle Einstellungen lassen sich über die Weboberfläche anpassen.

**Download:** Firmware mit Flash-Tool unter [Releases](https://github.com/holgiw/TFT-Clock-GC9A01/releases/latest) bzw. im Ordner `build_uhr4` (Anleitung: `build_uhr4/readme.txt`). Die Vorgängerversion uhr3 (Version 3, mit TFT_eSPI) gibt es als ZIP im Release [Version 3](https://github.com/holgiw/TFT-Clock-GC9A01/releases/tag/v3); ein Update von uhr3 auf uhr4 übernimmt den Displaytyp automatisch.

*English version below.*

---

## 1. Unterstützung mehrerer TFT-Displays

- Unterstützte Displays: GC9A01 (240 × 240) und GC9D01 (160 × 160) – eine Firmware für beide, der Displaytyp ist eine Einstellung (Tab „Uhr Einstellungen“: GC9A01 ohne / mit Hintergrundbeleuchtung (BL) an Pin 3, GC9D01 – wie in `flashESP`; beim Flashen per `flashESP` oder automatisch beim Update von uhr3). Grafikbibliothek: LovyanGFX.
- Zweites, baugleiches Display optional ansteuerbar, mit eigener Rotationseinstellung (0°, 90°, 180°, 270°) - ein nicht angeschlossenes Display wird auf "nicht angeschlossen (n.a.)" gestellt: dann bleibt es schwarz, Zifferblatt und Zeiger werden dafür weder gezeichnet noch berechnet. Standard: Display 1 mit 0°, Display 2 n.a. Status- und Startmeldungen (Start, Access-Point-Modus) erscheinen bis zum Uhrstart trotzdem auf beiden Displays (Hinweis dazu auf der Einstellungsseite).

---

## 2. Anpassbare Zeiger und Zifferblätter

- Eigene Stunden-, Minuten- und Sekundenzeiger sowie eigene Zifferblätter lassen sich als BMP-Dateien hochladen; Standardsätze sind als Fallback enthalten.
- Alle Zeiger werden kantengeglättet dargestellt (LovyanGFX); Stunden- und Minutenzeiger liegen in einem zwischengespeicherten Bild, zum Display wird nur der Bereich um den Sekundenzeiger übertragen – der schwingende Sekundenzeiger läuft dadurch flüssig.
- Zeiger dürfen vom Drehpunkt bis zum Displayrand reichen und breiter sein: neues Zeigerformat 25 × 151 px mit Drehpunkt 12 / 120 (160er-Display: 15 × 100, Drehpunkt 7 / 80). Zeigersätze im bisherigen Format 21 × 131 (13 × 86) funktionieren unverändert weiter – sie werden beim Laden oben und seitlich transparent aufgefüllt. Gültig sind alle Kombinationen aus alter und neuer Breite bzw. Höhe (21 oder 25 × 131 oder 151); beim Hochladen bleiben genau diese Größen unverändert, alles andere wird wie bisher auf 21 × 131 (13 × 86) skaliert. Der Zeiger-Designer speichert Breite und Höhe im alten Maß, solange ein Zeiger nicht darüber hinausragt – solche Zeigersätze laufen auch auf älterer Firmware.
- Zeiger-Designer im Browser (Seite „Zeiger“ → Link „Designer“ unter dem gewünschten Zeigersatz; der Satz wird dabei aktiviert): ausgehend vom gerade aktiven Zeigersatz neue Zeiger gestalten; Änderungen werden als neuer Zeigersatz gespeichert (optional gleich aktiviert) oder überschreiben den aktiven Satz und werden sofort angewendet – Pixel-Editor mit Stift, Linie, Rechteck, Ellipse, Polygon, Füllen, Pipette, freier Farbwahl (Farbpalette, Farbwähler, Hex-Eingabe), Spiegelung an der Mittelachse, Rückgängig und Formgenerator (Länge, Breiten, Gegengewicht, Scheibe), dazu eine Live-Vorschau auf dem aktuellen Zifferblatt. Die Uhr übernimmt dabei automatisch die passende Zeigergröße und den Drehpunkt ihres Displays.
- Zifferblatt-Designer im Browser (Seite „Zifferblatt“ → Link „Designer“ unter dem gewünschten Zifferblatt; es wird dabei aktiviert): ausgehend vom aktiven Zifferblatt ein neues gestalten oder das aktive überschreiben und sofort anwenden – Zifferblatt-Generator (Hintergrund, Rand, Stunden- und Minutenstriche, Ziffern 1–12, 12/3/6/9 oder römisch), Pixel-Werkzeuge mit Stiftbreite, Text (gerade, gedreht oder im Bogen, auch mit installierten oder geladenen Schriften), Logo als Stempel, Spiegel- und Dreh-Symmetrie (4-, 12- oder 60-fach), freie Farbwahl, Bild laden (PNG, JPG, BMP …), Zoom und Live-Vorschau mit den aktiven Zeigern. Bei runden Displays ist der unsichtbare Bereich markiert.

---

## 3. Sanfter Minutenzeiger und Bahnhofsuhr-Modus

- Sanfter Minutenzeiger: bewegt sich gleichmäßig statt in 1-Minuten-Schritten zu springen.
- Bahnhofsuhr-Modus: der Sekundenzeiger läuft in 58,5 Sekunden um und pausiert kurz oben auf der 12, wie bei einer klassischen Bahnhofsuhr.
- Zeitsprünge (Start, erste gültige Uhrzeit, Zeitumstellung, Rocrail): die Zeiger laufen in 3 Sekunden sanft auf dem kürzesten Weg zur neuen Zeit, auch rückwärts, statt zu springen.

---

## 4. Helligkeitssteuerung

- Automatische Helligkeit über einen Fotowiderstand mit einstellbaren Schwellwerten, alternativ manuell einstellbar.
- Optionale Hintergrundbeleuchtung per PWM an Pin 3 (Displaytyp „GC9A01 mit BL“ oder GC9D01; der Haken „Hintergrundbeleuchtung regeln“ im Helligkeits-Tab erscheint nur bei diesen Displays): gedimmt wird dann über die Beleuchtung statt über dunklere Pixel.
- Während der Einrichtung (noch keine gültige Uhrzeit, Access Point oder WPS aktiv) leuchtet das Display immer mit voller Helligkeit.
- Bei Rocrail-Verbindung kann die Helligkeit stattdessen vom Server übernommen werden - siehe Abschnitt 11.

---

## 5. WLAN- und NTP-Integration

- Bis zu 15 WLAN-Netzwerke, Einrichtung auch per WPS, automatischer Reconnect, anpassbarer Hostname.
- Ersteinrichtung direkt beim Flashen: `flashESP.bat`/`flashESP.sh` fragen Displaytyp und WLAN ab (alle sichtbaren 2,4-GHz-Netze zur Auswahl, Passwort verdeckt) und senden beides per USB an die Uhr, zuletzt die Uhrzeit des PCs. Nur die Uhrzeit setzen (ohne Flashen, z.B. für eine Uhr ohne WLAN, DCF77 und RTC): `setTime.bat`/`setTime.sh`. Alternativ per WPS oder über den Einrichtungs-Access-Point (SSID `clock123`, Passwort `clocksetup`).
- Hinzufügen, Überschreiben, Wechseln und Löschen von WLAN-Netzwerken wird direkt ausgeführt. Die Weboberfläche unterscheidet nicht zwischen Zugriff aus dem Heimnetz und von außen – die Uhr daher nicht per Port-Weiterleitung/DMZ aus dem Internet erreichbar machen.
- NTP mit DCF77 als Fallback für die Zeitsynchronisation; die Uhr agiert selbst auch als NTP-Server für andere Geräte im Netzwerk, antwortet dabei, sobald eine gültige Uhrzeit ermittelt wurde.
- Bis zu 15 eigene NTP-Server hinterlegbar; ist keiner konfiguriert (oder werden alle gelöscht), fällt die Uhr automatisch auf `pool.ntp.org` und `ptbtime1.ptb.de` zurück. Sind die konfigurierten Server nicht erreichbar, werden diese beiden zusätzlich als letzter Fallback versucht.

---

## 6. Weboberfläche

- Dunkel gestaltete Einstellungszentrale mit Tabs (WLAN, Uhr Einstellungen, Helligkeit, NTP Zeitzone, Status, Log; Rocrail bei aktivierter Anbindung); weitere Seiten (Vorschau, Uhren Sets, Zifferblatt, Zeiger, Dateimanager, DCF77, Sicherung, Werkseinstellungen) separat über die Navigation erreichbar.
- Mehrsprachig (Deutsch, Englisch).
- Zusätzliche Zifferblätter, Zeigersätze und Presets lassen sich direkt von GitHub herunterladen.
- Rocrail-Tab für die Modellzeit-Anbindung - siehe Abschnitt 11.

---

## 7. Dateiverwaltung mit LittleFS

- Zifferblätter und Zeiger werden komprimiert gespeichert; Dateien lassen sich über die Weboberfläche hoch-/herunterladen, umbenennen und löschen.
- Optionales Logging, einsehbar im Log-Tab mit Dateiauswahl (Dropdown zeigt alle vorhandenen Logdateien, neueste vorausgewählt) und Auto-Refresh.

---

## 8. Zeitzonen-Anpassung

- Automatische Sommerzeitumstellung oder dauerhaft Sommer-/Winterzeit einstellbar.
- Ist keine Zeitzone hinterlegt (leeres Feld) oder ist der eingetragene Wert kein gültiger POSIX-TZ-String, fällt die Uhr automatisch auf `CET-1CEST,M3.5.0,M10.5.0/3` (Mitteleuropäische Zeit) zurück.

---

## 9. Hardware-Integration

- ESP32-S2 (Lolin S2 Pico) mit rundem TFT-Display (GC9A01 oder GC9D01); Pinbelegung siehe `build_uhr4/readme.txt`, Platine im Ordner `PCB`.
- Fotowiderstand zur Helligkeitsmessung (wird beim Start automatisch erkannt).
- Optional: zweites, baugleiches Display (eigener Chip-Select), RTC DS3231 (hält die Uhrzeit auch ohne WLAN über Stromausfälle hinweg), DCF77-Empfänger (Funkuhr-Zeit ohne Internet).
- Taster (zusätzlich auch der eingebaute Boot-Taster): kurz gedrückt zeigt die Uhr das verbundene WLAN, länger als 15 Sekunden gehalten löst er einen vollständigen Werksreset aus (ab 10 Sekunden erscheint ein Countdown, Loslassen bricht ab).

---

## 10. Erweiterte Funktionen

- Laufzeit-Anzeige: zeigt die Laufzeit der Uhr seit dem letzten Neustart.
- Neustart-Funktion: erlaubt einen Neustart der Uhr über die Weboberfläche.
- BMP-Skalierung: hochgeladene BMP-Dateien können auf die Displaygröße skaliert werden.
- API-Schnittstelle
- Bis zu 50 Presets (Zifferblatt, Zeigersatz, Nabenfarbe/-größe, Sekundenzeiger sichtbar und Stil, Bahnhofsmodus, sanfter Minutenzeiger, Zeitzone und Helligkeitseinstellungen) - einzeln umbenenn- und löschbar, alphabetisch sortiert in der Liste; alle Presets lassen sich in eine Datei sichern und später wiederherstellen; sind alle 50 Plätze belegt, erscheint eine Warnung.
- Komplettsicherung (Seite „Sicherung“): alle Einstellungen, Presets, Zifferblätter und Zeigersätze in einer Datei, wiederherstellbar auf dieser oder einer anderen Uhr mit gleichem Displaytyp. WLAN-Zugangsdaten nur auf Wunsch und dann verschlüsselt – mit einem Schlüssel, der in jeder uhr4-Firmware gleich ist, also nicht sicher. Displaytyp, Rotation, Hintergrundbeleuchtung und Lichtsensor der Ziel-Uhr bleiben unverändert; eine falsche oder beschädigte Datei ändert nichts.
- Das Löschen eines Zifferblatts oder Zeigersatzes entfernt automatisch alle Presets, die darauf verwiesen haben; wird der gerade aktive Zeigersatz gelöscht, fällt die Uhr automatisch auf den eingebauten Standard zurück.
- DCF77 wird unterstützt: robuster Empfang auch bei schwachem oder gestörtem Signal (Impulse werden über ein Sekundenraster statt reiner Zählung platziert, sodass fehlende Impulse nicht die folgenden Bits verschieben), funktioniert unabhängig von der Signalpolarität; ein gestörtes Telegramm kann nie eine falsche Zeit setzen.
- Live-Seite (/dcf77) zeigt den Bit-Fortschritt des aktuellen Telegramms und das letzte dekodierte Telegramm zur Diagnose.
- Optionales LED-Blinken während der Synchronisation, nur sichtbar, wenn tatsächlich ein DCF77-Empfänger erkannt wurde.

---

## 11. Rocrail-Modellzeit

- Optionale Verbindung zu einem [Rocrail](https://wiki.rocrail.net/)-Server (Modelleisenbahn-Steuerungssoftware): die Zeiger können statt der echten Zeit die "Fast Clock" (Modellzeit) des Servers anzeigen, aktiviert über den Haken „Rocrail“ im Tab „Uhr Einstellungen“.
- Bis zu 15 Serveradressen können hinterlegt werden (wie bei den NTP-Servern erscheint nach dem letzten Eintrag immer automatisch ein neuer, leerer Platz); ein Radio-Button je Zeile legt den aktiven Server fest. Verbindungsversuche starten sofort (beim Speichern, Aktivieren oder Neustart), statt auf das reguläre, einmal pro Minute wiederkehrende Zeitfenster zu warten.
- Bleiben Updates länger als 2 Minuten aus, läuft die Uhr mit der normalen NTP-/RTC-/DCF77-Zeit weiter, statt bei einer veralteten Modellzeit hängen zu bleiben. Die Modellzeit läuft mit Rocrails eigenem Beschleunigungsfaktor (dem "Divider"); die Bahnhofsuhr-Sekundenzeiger-Animation skaliert entsprechend mit, statt sich abzuschalten (oberhalb eines Schwellwerts wird der Sekundenzeiger ganz ausgeblendet).
- Der Tab zeigt zu Diagnosezwecken einen Live-Verbindungsstatus, den Divider und die aktuelle Modellzeit; für jeden Server lässt sich zusätzlich ein Anlagenname zur eigenen Orientierung eintragen.
- Meldet der Server zusätzlich einen Helligkeitswert, übernimmt die Uhr auch die Display-Helligkeit von dort (siehe Abschnitt 4); die Web-Vorschau ("Vorschau") spiegelt Modellzeit und Divider auf dieselbe Weise.

---
---

# English Version

uhr4 is a digital clock project on a round display: an ESP32-S2 (Lolin S2 Pico) shows the time as a clock face with hands on a round TFT display (GC9A01 or GC9D01). The time comes via NTP (WiFi), DCF77 or RTC; clock faces, hands and all settings can be customized via the web interface.

**Download:** firmware with flash tool under [Releases](https://github.com/holgiw/TFT-Clock-GC9A01/releases/latest) or in the folder `build_uhr4` (instructions: `build_uhr4/readme.txt`). The previous version uhr3 (version 3, with TFT_eSPI) is available as a ZIP in the release [Version 3](https://github.com/holgiw/TFT-Clock-GC9A01/releases/tag/v3); updating from uhr3 to uhr4 takes over the display type automatically.

## 1. Support for Multiple TFT Displays

- Supported displays: GC9A01 (240 × 240) and GC9D01 (160 × 160) – one firmware for both, the display type is a setting ("Clock Setup" tab: GC9A01 without / with backlight (BL) on pin 3, GC9D01 – as in `flashESP`; when flashing via `flashESP`, or automatically when updating from uhr3). Graphics library: LovyanGFX.
- A second, identical display can optionally be driven, with its own rotation setting (0°, 90°, 180°, 270°) - a display that is not connected is set to "not connected (n.a.)": it then stays black and face/hands are neither drawn nor calculated for it. Default: display 1 at 0°, display 2 n.a. Status and boot messages (boot, access point mode) still appear on both displays until the clock takes over (the settings page notes this).

---

## 2. Customizable Clock Hands and Faces

- Custom hour, minute, and second hands, as well as custom clock faces, can be uploaded as BMP files; default sets are included as a fallback.
- All hands are rendered anti-aliased (LovyanGFX); hour and minute hands live in a cached composite image, and only the area around the second hand is sent to the display – so the sweeping second hand runs smoothly.
- Hands may reach from the pivot to the display edge and be wider: new hand format 25 x 151 px with pivot 12 / 120 (160 display: 15 x 100, pivot 7 / 80). Hand sets in the previous format 21 x 131 (13 x 86) keep working unchanged - they are padded transparent at the top and the sides when loaded. All combinations of old and new width and height are valid (21 or 25 x 131 or 151); on upload exactly these sizes stay unchanged, everything else is scaled to 21 x 131 (13 x 86) as before. The hand designer keeps width and height at the old size as long as a hand does not extend beyond it - such hand sets also run on older firmware.
- In-browser hand designer ("Hand Set" page → "Designer" link below the desired hand set, which also activates it): design new hands starting from the currently active hand set; changes are saved as a new hand set (optionally activated right away) or overwrite the active set and are applied immediately - pixel editor with pen, line, rectangle, ellipse, polygon, fill, picker, free colour choice (palette, colour picker, hex input), mirroring at the centre axis, undo and a shape generator (length, widths, counterweight, disc), plus a live preview on the current clock face. The clock automatically applies its display's hand size and pivot point.
- In-browser clock face designer ("Clock Face" page → "Designer" link below the desired clock face, which also activates it): design a new clock face starting from the active one, or overwrite the active one and apply it right away - face generator (background, rim, hour and minute marks, numerals 1-12, 12/3/6/9 or Roman), pixel tools with pen width, text (straight, rotated or on an arc, also with installed or loaded fonts), logo stamp, mirror and rotation symmetry (4, 12 or 60 times), free colour choice, image import (PNG, JPG, BMP ...), zoom and a live preview with the active hands. On round displays the invisible area is marked.

---

## 3. Smooth Minute and Train Station Modes

- Smooth Minute Mode: the minute hand moves smoothly instead of jumping in 1-minute increments.
- Train Station Mode: the second hand completes its round in 58.5 seconds and briefly pauses at the top on the 12, like a classic railway clock.
- Time jumps (boot, first valid time, DST change, Rocrail): the hands move smoothly within 3 seconds along the shortest path to the new time, backwards too, instead of jumping.

---

## 4. Brightness Control

- Automatic brightness via a photoresistor with configurable thresholds, or manual control.
- Optional backlight control via PWM on pin 3 (display type "GC9A01 with BL" or GC9D01; the "Backlight control" checkbox in the Brightness tab only appears for these displays): dimming then happens via the backlight instead of darker pixels.
- During setup (no valid time yet, access point or WPS active) the display always runs at full brightness.
- When connected to Rocrail, brightness can be taken over from the server instead - see section 11.

---

## 5. WiFi and NTP Integration

- Up to 15 WiFi networks, WPS setup, automatic reconnect, customizable hostname.
- First-time setup directly when flashing: `flashESP.bat`/`flashESP.sh` ask for the display type and a WiFi network (all visible 2.4 GHz networks for selection, password entered hidden) and send both to the clock via USB, finally the PC's time. To only set the time (without flashing, e.g. for a clock without WiFi, DCF77 and RTC): `setTime.bat`/`setTime.sh`. Alternatively via WPS or the setup access point (SSID `clock123`, password `clocksetup`).
- Adding, overwriting, switching and deleting WiFi networks is executed directly. The web interface does not distinguish between access from the home network and from outside - so do not expose the clock to the internet via a port forward/DMZ.
- NTP with DCF77 as a fallback for time sync; the clock also acts as an NTP server for other devices on the network, answering once a valid time has been determined.
- Up to 15 custom NTP servers can be stored; if none are configured (or all are deleted), the clock automatically falls back to `pool.ntp.org` and `ptbtime1.ptb.de`. If the configured servers are unreachable, these two are additionally tried as a last-resort fallback.

---

## 6. Web Interface

- Dark-themed settings hub with tabs (WiFi Settings, Clock Setup, Brightness, NTP Timezone, Status, Log; Rocrail when the connection is enabled); further pages (Preview, Presets, Clock Face, Hand Set, File Manager, DCF77, Backup, Factory Reset) reachable separately via the navigation bar.
- Multi-language (German, English).
- Additional clock faces, hand sets, and presets can be downloaded directly from GitHub.
- Rocrail tab for the model-time connection - see section 11.

---

## 7. File Management with LittleFS

- Clock faces and hands are stored compressed; files can be uploaded, downloaded, renamed, and deleted via the web interface.
- Optional logging, viewable in the Log tab with a file selector (dropdown shows all existing log files, newest preselected) and auto-refresh.

---

## 8. Time Zone Customization

- Automatic daylight saving time or permanent summer/winter time can be configured.
- If no timezone is stored (empty field) or the stored value is not a valid POSIX TZ string, the clock automatically falls back to `CET-1CEST,M3.5.0,M10.5.0/3` (Central European Time).

---

## 9. Hardware Integration

- ESP32-S2 (Lolin S2 Pico) with a round TFT display (GC9A01 or GC9D01); pinout see `build_uhr4/readme.txt`, PCB in the `PCB` folder.
- Photoresistor for brightness measurement (detected automatically at startup).
- Optional: a second, identical display (own chip select), RTC DS3231 (keeps the time across power loss even without WiFi), DCF77 receiver (radio clock time without internet).
- Button (the built-in Boot button works too): a short press shows the connected WiFi, holding it for more than 15 seconds triggers a full factory reset (a countdown appears from 10 seconds, releasing aborts).

---

## 10. Advanced Features

- Uptime Display: Shows the clock's runtime since the last restart.
- Reboot Function: Allows restarting the clock via the web interface.
- BMP Scaling: Uploaded BMP files can be scaled to fit the display size.
- API Interface
- Up to 50 presets (face, hand set, hub color/size, second hand visibility and style, station mode, smooth minute hand, time zone and brightness settings) - individually renameable and deletable, sorted alphabetically in the list; back up all presets to a file and restore them later; a warning is shown once all 50 slots are full.
- Full backup (Backup page): all settings, presets, clock faces and hand sets in one file, restorable on this or another clock with the same display type. WiFi credentials only on request and then encrypted – with a key that is the same in every uhr4 firmware, so not secure. Display type, rotation, backlight and light sensor of the target clock stay unchanged; a wrong or damaged file changes nothing.
- Deleting a clock face or hand set automatically removes any presets that referenced it; deleting the currently active hand set automatically falls back to the built-in default.
- DCF77 supported: robust reception even with a weak or disturbed signal (pulses are placed on a one-second grid instead of relying on pure counting, so missing pulses don't shift the following bits), works regardless of signal polarity; a disturbed telegram can never set a wrong time.
- Live page (/dcf77) shows the bit progress of the current telegram and the last decoded telegram for diagnostics.
- Optional LED blink during synchronization, only shown once a DCF77 receiver has actually been detected.

---

## 11. Rocrail Model Time

- Optional connection to a [Rocrail](https://wiki.rocrail.net/) server (model-railroad control software): the hands can display the server's "fast clock" (model time) instead of the real time, enabled via the "Rocrail" checkbox in the "Clock Setup" tab.
- Up to 15 server addresses can be stored (like the NTP servers, a new empty slot always appears automatically after the last entry); a radio button per row selects the active server. Connection attempts start immediately (on save, enable, or restart) instead of waiting for the regular once-a-minute retry window.
- If updates stop coming in for more than 2 minutes, the clock falls back to the normal NTP/RTC/DCF77 time instead of getting stuck on a stale model time. Model time runs at Rocrail's own acceleration factor (the "divider"); the station-clock second-hand animation scales with it instead of switching off (above a threshold the second hand is hidden entirely).
- The tab shows a live connection status, the divider and the current model time for diagnostics; each server can also be given a layout name for your own reference.
- If the server also reports a brightness value, the display takes its brightness from there too (see section 4); the web preview (Preview page) mirrors the model time and divider the same way.
