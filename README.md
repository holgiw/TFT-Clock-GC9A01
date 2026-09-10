*English version below.*

---

## 1. Unterstützung mehrerer TFT-Displays

- Die Uhr unterstützt verschiedene TFT-Displays:
  - GC9A01
  - GC9D01
  - ILI9341 (veraltet - wird nicht mehr aktiv gepflegt)
- Zweites, baugleiches Display immer aktiv: steuert über einen eigenen Chip-Select-Pin (CS2) am gemeinsamen SPI-Bus ein zweites Display desselben Typs an - jedes Display hat seine eigene, unabhängig einstellbare Rotation (Rotation Display 1 / Rotation Display 2), sodass beide unterschiedlich eingebaut sein können. Status-/Boot-Meldungen (Start, Neustart, SSID, WPS, Werksreset, ...) erscheinen auf beiden Displays, jeweils passend zur eigenen Ausrichtung gedreht. Ist nur ein Display tatsächlich angeschlossen, sollten beide Rotationswerte auf denselben Wert gestellt werden - die Einstellungsseite weist direkt neben den beiden Rotations-Dropdowns darauf hin. Beim GC9D01 (Software-Rotation) spart ein gleicher Wert bei beiden zudem die erneute Berechnung eines zweiten, identischen Frames für Display 2 pro Takt - der für Display 1 bereits fertige Frame wird einfach erneut gesendet.

---

## 2. Anpassbare Zeiger und Zifferblätter

- Zeiger:
  - Eigene Stunden-, Minuten- und Sekundenzeiger können als BMP-Dateien hochgeladen werden.
  - Standardzeiger sind als Fallback enthalten.
  - Kantengeglättete Stunden- und Minutenzeiger: beide werden mit 3x3-Supersampling und pixelgenauer Coverage-Blendung statt der bibliothekseigenen Nearest-Neighbour-Rotation gezeichnet, was die Treppenstufen an ihren Kanten entfernt. Sie werden in ein zwischengespeichertes Verbundbild gezeichnet, das nur bei tatsächlicher Winkeländerung neu aufgebaut wird - das kostet in den Frames, in denen nur der Sekundenzeiger läuft, also keine Zeit; der Aufwand pro Frame ist damit sogar geringer als vorher.
- Zifferblätter:
  - Eigene Zifferblätter können hochgeladen und ausgewählt werden.
  - Ein eingebautes Standard-Zifferblatt ist verfügbar.

---

## 3. Sanfter Minutenzeiger und Bahnhofsuhr-Modus

- Sanfter Minutenzeiger-Modus:
  - Der Minutenzeiger bewegt sich gleichmäßig, statt in 1-Minuten-Schritten zu springen.
- Bahnhofsuhr-Modus:
  - Der Sekundenzeiger läuft in 60 Schritten, beschleunigt und bremst innerhalb jedes Schritts wie bei alten Bahnhofsuhren, vollendet seinen Umlauf in 58,5 Sekunden und verharrt anschließend für die restlichen ca. 1,5 Sekunden der realen Minute oben auf der 12. Beim Minutenwechsel springt der Minutenzeiger vor und der nächste Umlauf beginnt.

---

## 4. Helligkeitssteuerung

- Automatische Helligkeitsanpassung über einen Fotowiderstand.
- Einstellbare Schwellwerte für minimale und maximale Helligkeit.
- Manuelle Helligkeitssteuerung ist verfügbar, falls kein Fotowiderstand erkannt wird.
- Bei Verbindung zu einem Rocrail-Server, der über seine Fast-Clock-Updates einen Helligkeitswert (`bri`, 0-255, z.B. aus dessen Tag/Nacht-Lichtsteuerung) meldet, übernimmt das Display stattdessen dessen Helligkeit - die Fotowiderstand- und Zeitfenster-Logik wird währenddessen deaktiviert und greift automatisch wieder, sobald die Verbindung getrennt wird. Der Helligkeit-Tab zeigt währenddessen einen Hinweis an.

---

## 5. WLAN- und NTP-Integration

- WLAN:
  - Unterstützt bis zu 15 WLAN-Netzwerke.
  - Das Scannen nach Netzwerken zeigt Signalstärke und ob ein Netzwerk offen oder gesichert ist.
  - Hostname und alle WLAN-Netzwerkeinstellungen liegen auf einem eigenen WLAN-Tab der Haupteinstellungsseite - beim ersten Start oder wenn das zuletzt bekannte Netzwerk nicht erreichbar ist, öffnet sich dieser Tab automatisch.
  - Verbindet sich bei Verbindungsverlust automatisch neu (einstellbar).
  - Einzelne Netzwerke lassen sich direkt aus der Weboberfläche löschen - verbleibende Einträge rücken automatisch nach, um die Lücke zu schließen.
  - Ein neues Netzwerk lässt sich per WPS mit einem einzigen Knopf in der Weboberfläche hinzufügen - SSID/Passwort müssen dafür nicht vorher bekannt sein.
  - Anpassbarer Hostname (ungültige Zeichen werden automatisch bereinigt); fällt bei leerem Feld auf einen automatisch aus der MAC-Adresse generierten Namen zurück.
- NTP (Network Time Protocol):
  - NTP hat Vorrang und wird stündlich geprüft; jeder konfigurierte Server erhält einen Wiederholungsversuch beim selben Server, bevor die Uhr zum nächsten übergeht, sodass eine einzelne verlorene UDP-Antwort nicht schon als fehlgeschlagener Server zählt.
  - Ist NTP nicht verfügbar, fällt die Uhr automatisch auf das letzte gültige DCF77-Telegramm zurück (sofern eines dekodiert wurde, dessen Parität stimmt und das nicht älter als 10 Minuten ist - die seit der Dekodierung vergangene Zeit wird wieder aufaddiert, sodass ein älteres Telegramm genauso genau ist).
  - Die RTC wird sowohl beim Start als auch stündlich von der jeweils erfolgreichen Quelle (NTP oder DCF77) gestellt.
  - Der DCF77-Empfang wird unabhängig davon überwacht, welche Quelle die Uhr gerade antreibt, und ein länger andauernder DCF77-Ausfall wird in der Statusleiste angezeigt, auch während gerade NTP die Führung hat.
  - Die Uhr fungiert außerdem selbst als NTP-Server (Port 123), sodass andere Geräte im Netzwerk ihre Zeit von ihr beziehen können. Eine Anfrage wird beantwortet, sobald die Uhr über eine gültige Systemzeit verfügt, unabhängig davon, ob NTP, DCF77 oder die RTC sie geliefert hat. Der Socket wird nach jedem WLAN-(Wieder-)Verbindungsaufbau neu gebunden - er übersteht den WLAN-Stack-Neustart eines Reconnects nicht - und die Statusseite zeigt, ob der Server lauscht, sowie wie viele Anfragen eingegangen und beantwortet wurden. Antworten tragen Sub-Sekunden-Genauigkeit statt auf die volle Sekunde gerundet zu sein.

---

## 6. Weboberfläche

- Neu gestaltete, dunkel gehaltene Einstellungszentrale auf einer Seite: Status, WLAN, Uhr-Einrichtung, Helligkeit und Zeit/NTP sind jetzt Tabs auf einer Seite (kein Durchklicken separater Seiten mehr für Grundeinstellungen).
- Dateilastige Verwaltungsseiten (Presets, Zifferblatt, Zeigersatz, Dateiverwaltung, Live-Vorschau, Werksreset) bleiben eigene Seiten, erreichbar über die Navigationsleiste.
- Eine eingebaute Weboberfläche ermöglicht:
  - Hochladen, Herunterladen und Verwalten von Zifferblättern und Zeigern.
  - Anpassen von Helligkeit, Zeitzone und Display-Rotation.
  - Ein-/Ausschalten von Sanftem Minutenzeiger und Bahnhofsuhr-Modus.
  - Einsicht in den Systemstatus (z.B. WLAN-Details, Speichernutzung, Laufzeit) - einschließlich ob PSRAM erkannt wurde und welcher Rotationsmodus aktiv ist (Hardware-MADCTL-Register vs. Software-Pixel-Remapping beim GC9D01 mit PSRAM), sodass von außen sichtbar ist, wie die eingestellten Rotationswerte tatsächlich angewendet werden.
- Mehrsprachige Oberfläche (Deutsch, Englisch, Französisch).
- Info-Seite (/info), direkt nach dem Log-Tab verlinkt: zeigt diese Projektbeschreibung in der aktuell gewählten Interface-Sprache, dazu die Build-Version und einen Projekt-/Kontaktabschnitt. Aufgebaut wie der Log-Tab - eine schmale Kopfkarte mit Build-Version und Repository, darunter die Beschreibung in einem eigenen, gleich hohen Scroll-Fenster, sodass der Seitenrahmen (Topbar, Navigation) stehen bleibt, während der Text scrollt. Die Einstellungs-Tableiste bleibt auch auf der Info-Seite sichtbar: ihre Einträge führen zurück zum jeweiligen Tab (Info selbst wird als aktiver Eintrag angezeigt), sodass die Leiste nicht unter dem Klick verschwindet, der die Seite geöffnet hat (Repository, Zifferblatt- und Zeigersatz-Ordner, Schaltplan/Platine, weitere Projekte des Autors, E-Mail). Alle diese Adressen stammen aus den zentralen Makros in config.h, ein Fork muss also nur dort etwas ändern. Die drei Sprachversionen liegen im Flash (readme_text.h) und werden direkt von dort gestreamt, die Seite braucht also keinen Dateisystem-Upload und keinen Heap für ihre ca. 10 KB Text.
- Werksreset-Seite mit getrennten Optionen: alles zurücksetzen, Zifferblätter löschen (außer Standard), Zeigersätze löschen (außer Standard), Presets löschen, oder gespeicherte WLAN-Netzwerke zurücksetzen - jede mit eigener Bestätigung.
- Zusätzliche Zifferblätter, Zeigersätze und Presets lassen sich direkt über die Weboberfläche von GitHub herunterladen - fehlende Abhängigkeiten (z.B. ein von einem Preset referenziertes Zifferblatt/Zeigersatz) werden automatisch nachgeladen.
- Rocrail-Tab: verbindet sich optional mit einem Rocrail-Modelleisenbahn-Server und treibt die Zeiger über dessen "Fast Clock" (Modellzeit) statt der echten Zeit an - siehe Abschnitt 11.

---

## 7. Dateiverwaltung mit LittleFS

- Nutzt das LittleFS-Dateisystem, um:
  - Eigene Zifferblätter und Zeiger RLE-komprimiert zu speichern und so Flash-Speicher zu sparen.
  - Dateien (z.B. hochladen, herunterladen, umbenennen, löschen) über eine kompakte, symbolbasierte Oberfläche zu verwalten.
- Logging

---

## 8. Zeitzonen-Anpassung

- Unterstützt verschiedene Zeitzonen:
  - Automatische Sommerzeitumstellung (z.B. MEZ/MESZ).
  - Dauerhafte Sommer- oder Winterzeit.

---

## 9. Hardware-Integration

- Nur mit dem ESP32-S2 kompatibel.
- Fotowiderstand zur Helligkeitsmessung.

---

## 10. Erweiterte Funktionen

- Laufzeit-Anzeige: zeigt die Laufzeit der Uhr seit dem letzten Neustart.
- Neustart-Funktion: erlaubt einen Neustart der Uhr über die Weboberfläche.
- BMP-Skalierung: hochgeladene BMP-Dateien können auf die Displaygröße skaliert werden.
- API-Schnittstelle
- Bis zu 50 Presets (Zifferblatt, Zeigersatz, Nabenfarbe/-größe, Sekundenzeiger-Anzeige) - einzeln umbenenn- und löschbar, alphabetisch sortiert in der Liste; alle Presets lassen sich in eine Datei sichern und später wiederherstellen; sind alle 50 Plätze belegt, erscheint eine Warnung.
- Das Löschen eines Zifferblatts oder Zeigersatzes entfernt automatisch alle Presets, die darauf verwiesen haben; wird der gerade aktive Zeigersatz gelöscht, fällt die Uhr automatisch auf den eingebauten Standard zurück.
- DCF77 wird unterstützt: robuster Empfang auch bei schwachem oder gestörtem Signal (Impulse werden über ein Sekundenraster statt reiner Zählung platziert, sodass fehlende Impulse nicht die folgenden Bits verschieben), funktioniert unabhängig von der Signalpolarität; ein gestörtes Telegramm kann nie eine falsche Zeit setzen.
- Live-Seite (/dcf77) zeigt den Bit-Fortschritt des aktuellen Telegramms und das letzte dekodierte Telegramm zur Diagnose.
- Optionales LED-Blinken während der Synchronisation, nur sichtbar, wenn tatsächlich ein DCF77-Empfänger erkannt wurde.

---

## 11. Rocrail-Modellzeit

- Optionale Verbindung zu einem [Rocrail](https://wiki.rocrail.net/)-Server (Modelleisenbahn-Steuerungssoftware): die Zeiger können statt der echten Zeit die "Fast Clock" (Modellzeit) des Servers anzeigen, aktiviert über einen Schalter auf dem Zeit-Tab.
- Bis zu 15 Serveradressen können hinterlegt werden (wie bei den NTP-Servern erscheint nach dem letzten Eintrag immer automatisch ein neuer, leerer Platz); ein Radio-Button je Zeile legt den aktiven Server fest. Verbindungsversuche starten sofort (beim Speichern, Aktivieren oder Neustart), statt auf das reguläre, einmal pro Minute wiederkehrende Zeitfenster zu warten.
- Bleiben Updates länger als 2 Minuten aus, läuft die Uhr mit der normalen NTP-/RTC-/DCF77-Zeit weiter, statt bei einer veralteten Modellzeit hängen zu bleiben. Die Modellzeit läuft mit Rocrails eigenem Beschleunigungsfaktor (dem "Divider"); die Bahnhofsuhr-Sekundenzeiger-Animation skaliert entsprechend mit, statt sich abzuschalten (oberhalb eines Schwellwerts wird der Sekundenzeiger ganz ausgeblendet).
- Der Tab zeigt zu Diagnosezwecken einen Live-Verbindungsstatus, den Divider und die aktuelle Modellzeit; für jeden Server lässt sich zusätzlich ein Anlagenname zur eigenen Orientierung eintragen.
- Meldet der Server zusätzlich einen Helligkeitswert, übernimmt die Uhr auch die Display-Helligkeit von dort (siehe Abschnitt 4); die Web-Vorschau ("Vorschau") spiegelt Modellzeit und Divider auf dieselbe Weise.

---
---

# English Version

## 1. Support for Multiple TFT Displays

- The clock supports various TFT displays:
  - GC9A01
  - GC9D01
  - ILI9341 (deprecated - no longer actively maintained)
- Second, identical display always active: drives a second display of the same type via its own chip-select pin (CS2) on the shared SPI bus - each display gets its own, independently configurable rotation (Rotation Display 1 / Rotation Display 2), so the two can be mounted in different orientations. Status/boot messages (startup, reboot, SSID, WPS, factory reset, ...) are shown on both displays, each correctly rotated to match that display's own orientation. If only a single display is physically connected, both rotation settings should be set to the same value - the settings page notes this next to the two rotation dropdowns. On the GC9D01 (software rotation), setting both to the same value also skips recomputing a second, identical frame for Display 2 each tick - the frame already built for Display 1 is simply re-sent.

---

## 2. Customizable Clock Hands and Faces

- Clock Hands:
  - Custom hour, minute, and second hands can be uploaded as BMP files.
  - Default hands are included as a fallback.
  - Anti-aliased hour and minute hands: both are rendered with 3x3 supersampling and per-pixel coverage blending instead of the library's nearest-neighbour rotation, which removes the stair-stepping on their edges. They are drawn into a cached composite image that is only rebuilt when an angle actually changed, so this costs no time in the frames where only the sweeping second hand moves - the per-frame work is in fact lower than before.
- Clock Faces:
  - Custom clock faces can be uploaded and selected.
  - A built-in default clock face is available.

---

## 3. Smooth Minute and Train Station Modes

- Smooth Minute Mode:
  - The minute hand moves smoothly instead of jumping in 1-minute increments.
- Train Station Mode:
  - The second hand steps around in 60 steps, accelerating and braking within each step the way older station clocks did, completes its round in 58.5 seconds and then rests at the top on the 12 for the remaining ~1.5 seconds of the real minute. At the minute change the minute hand jumps forward and the next round starts.

---

## 4. Brightness Control

- Automatic brightness adjustment using a photoresistor.
- Configurable thresholds for minimum and maximum brightness.
- Manual brightness control is available if no photoresistor is detected.
- When connected to a Rocrail server that reports a brightness value (`bri`, 0-255, e.g. from its day/night lighting control) on its fast-clock updates, the display takes its brightness from there instead - the photoresistor and time-window logic are disabled while connected and resume automatically as soon as the connection is lost. The Brightness tab shows a notice while this is active.

---

## 5. WiFi and NTP Integration

- WiFi:
  - Supports up to 15 WiFi networks.
  - Scanning for networks shows signal strength and whether each network is open or secured.
  - Hostname and all WiFi network settings live on their own WLAN tab on the main settings page - on first boot or whenever the last known network is unavailable, that tab opens automatically.
  - Automatically reconnects if the connection is lost (configurable).
  - Individual networks can be deleted directly from the web interface - remaining entries automatically move up to close the gap.
  - Add a new network via WPS with a single button on the web interface - no need to know the SSID/password in advance.
  - Customizable hostname (invalid characters are automatically cleaned up); falls back to an automatically generated name based on the MAC address if left empty.
- NTP (Network Time Protocol):
  - NTP has priority and is checked hourly; each configured server gets a same-server retry before the clock moves on to the next one, so a single lost UDP response doesn't count as a failed server.
  - If NTP is unavailable, the clock automatically falls back to the last valid DCF77 telegram (if one has been decoded, its parity checks out and it is no older than 10 minutes - the time elapsed since it was decoded is added back on, so an older telegram is just as accurate).
  - The RTC is set both at boot and hourly from whichever source (NTP or DCF77) actually succeeded.
  - DCF77 reception is monitored independently of which source currently drives the clock, and a prolonged DCF77 outage is shown in the status bar even while NTP is currently in charge.
  - The clock also acts as an NTP server itself (port 123), so other devices on the network can take their time from it. A request is answered whenever the clock has a valid system time, regardless of whether NTP, DCF77 or the RTC provided it. The socket is rebound after every WiFi (re)connection - it does not survive the WiFi stack restart that a reconnect performs - and the status page shows whether the server is listening, plus how many requests came in and were answered. Replies carry sub-second precision rather than being rounded to the full second.

---

## 6. Web Interface

- Redesigned dark-themed, single-page settings hub: Status, WLAN, Clock Setup, Brightness and Time/NTP are now tabs on one page (no more clicking through separate pages for basic settings).
- File-heavy management screens (Presets, Clock Face, Hand Set, File Manager, Live Preview, Factory Reset) remain separate pages, reachable from the navigation bar.
- A built-in web interface allows:
  - Uploading, downloading, and managing clock faces and hands.
  - Adjusting brightness, time zone, and display rotation.
  - Enabling/disabling Smooth Minute and Train Station modes.
  - Viewing system status (e.g., WiFi details, storage usage, uptime) - including whether PSRAM was detected and which rotation mode is active (hardware MADCTL register vs. software pixel remapping on the GC9D01 with PSRAM), so it is visible from the outside how the configured rotation values are actually applied.
- Multi-language interface (German, English, French).
- Info page (/info), linked right after the Log tab: shows this project description in the currently selected interface language, plus the build version and a project/contact section. Laid out like the Log tab - a slim header card with build version and repository, then the description in its own scrollable window of the same height, so the page frame (top status bar, navigation) stays put while the text scrolls. The settings tab bar stays visible on the info page as well: its entries lead back to the matching tab (Info itself is shown as the active entry), so the bar does not disappear underneath the click that opened the page (repository, clock face and hand set folder, schematic/PCB, the author's other projects, e-mail). All those addresses come from the central macros in config.h, so a fork only needs changing there. The three language versions are held in flash (readme_text.h) and streamed straight from there, so the page needs no filesystem upload and no heap for its ~10 KB of text.
- Factory reset page with separate options: reset everything, delete clock faces (except default), delete hand sets (except default), delete presets, or reset saved WiFi networks - each with its own confirmation.
- Download additional clock faces, hand sets, and presets directly from GitHub via the web interface - missing dependencies (e.g. a face/hand set referenced by a preset) are fetched automatically.
- Rocrail tab: optionally connects to a Rocrail model-railroad server and drives the hands from its "fast clock" (model time) instead of the real time - see section 11.

---

## 7. File Management with LittleFS

- Uses the LittleFS filesystem to:
  - Store custom clock faces and hands, RLE-compressed to save flash space.
  - Manage files (e.g., upload, download, rename, delete) with a compact icon-based interface.
- Logging

---

## 8. Time Zone Customization

- Supports various time zones:
  - Automatic daylight saving time (e.g. CET/CEST).
  - Permanent summer or winter time.

---

## 9. Hardware Integration

- Compatible only with ESP32-S2.
- Photoresistor for brightness measurement.

---

## 10. Advanced Features

- Uptime Display: Shows the clock's runtime since the last restart.
- Reboot Function: Allows restarting the clock via the web interface.
- BMP Scaling: Uploaded BMP files can be scaled to fit the display size.
- API Interface
- Up to 50 presets (face, hand set, hub color/size, second-hand display) - individually renameable and deletable, sorted alphabetically in the list; back up all presets to a file and restore them later; a warning is shown once all 50 slots are full.
- Deleting a clock face or hand set automatically removes any presets that referenced it; deleting the currently active hand set automatically falls back to the built-in default.
- DCF77 supported: robust reception even with a weak or disturbed signal (pulses are placed on a one-second grid instead of relying on pure counting, so missing pulses don't shift the following bits), works regardless of signal polarity; a disturbed telegram can never set a wrong time.
- Live page (/dcf77) shows the bit progress of the current telegram and the last decoded telegram for diagnostics.
- Optional LED blink during synchronization, only shown once a DCF77 receiver has actually been detected.

---

## 11. Rocrail Model Time

- Optional connection to a [Rocrail](https://wiki.rocrail.net/) server (model-railroad control software): the hands can display the server's "fast clock" (model time) instead of the real time, enabled via a switch on the Time tab.
- Up to 15 server addresses can be stored (like the NTP servers, a new empty slot always appears automatically after the last entry); a radio button per row selects the active server. Connection attempts start immediately (on save, enable, or restart) instead of waiting for the regular once-a-minute retry window.
- If updates stop coming in for more than 2 minutes, the clock falls back to the normal NTP/RTC/DCF77 time instead of getting stuck on a stale model time. Model time runs at Rocrail's own acceleration factor (the "divider"); the station-clock second-hand animation scales with it instead of switching off (above a threshold the second hand is hidden entirely).
- The tab shows a live connection status, the divider and the current model time for diagnostics; each server can also be given a layout name for your own reference.
- If the server also reports a brightness value, the display takes its brightness from there too (see section 4); the web preview (Preview page) mirrors the model time and divider the same way.
