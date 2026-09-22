*English version below.*

---

## 1. Unterstützung mehrerer TFT-Displays

- Unterstützte Displays: GC9A01, GC9D01, ILI9341 (veraltet).
- Zweites, baugleiches Display optional ansteuerbar, mit eigener Rotationseinstellung (0°, 90°, 180°, 270°) - ein nicht angeschlossenes Display wird auf "nicht angeschlossen (n.a.)" gestellt: dann bleibt es schwarz, Zifferblatt und Zeiger werden dafür weder gezeichnet noch berechnet. Standard: Display 1 mit 0°, Display 2 n.a. Status- und Startmeldungen (Start, Access-Point-Modus, Bestätigungscodes) erscheinen bis zum Uhrstart trotzdem auf beiden Displays (Hinweis dazu auf der Einstellungsseite).

---

## 2. Anpassbare Zeiger und Zifferblätter

- Eigene Stunden-, Minuten- und Sekundenzeiger sowie eigene Zifferblätter lassen sich als BMP-Dateien hochladen; Standardsätze sind als Fallback enthalten.
- Stunden- und Minutenzeiger werden kantengeglättet dargestellt.

---

## 3. Sanfter Minutenzeiger und Bahnhofsuhr-Modus

- Sanfter Minutenzeiger: bewegt sich gleichmäßig statt in 1-Minuten-Schritten zu springen.
- Bahnhofsuhr-Modus: der Sekundenzeiger läuft in 58,5 Sekunden um und pausiert kurz oben auf der 12, wie bei einer klassischen Bahnhofsuhr.

---

## 4. Helligkeitssteuerung

- Automatische Helligkeit über einen Fotowiderstand mit einstellbaren Schwellwerten, alternativ manuell einstellbar.
- Bei Rocrail-Verbindung kann die Helligkeit stattdessen vom Server übernommen werden - siehe Abschnitt 11.

---

## 5. WLAN- und NTP-Integration

- Bis zu 15 WLAN-Netzwerke, Einrichtung auch per WPS, automatischer Reconnect, anpassbarer Hostname.
- Überschreiben oder Wechseln des aktuell verbundenen WLAN-Netzwerks wird bei Zugriff aus dem privaten (Heim-)Netzwerk direkt ausgeführt; bei Zugriff aus einem nicht-privaten Netzwerk (z. B. über eine Port-Weiterleitung/DMZ) ist stattdessen die Bestätigung eines auf dem Display angezeigten Codes nötig. Löschen eines WLAN-Netzwerks (aktiv oder nicht) sowie alle sonstigen Änderungen an nicht aktiven Netzwerken sind dagegen grundsätzlich nur aus einem privaten Netzwerk möglich, ganz ohne Code-Option.
- NTP mit DCF77 als Fallback für die Zeitsynchronisation; die Uhr agiert selbst auch als NTP-Server für andere Geräte im Netzwerk, antwortet dabei aber nur auf Anfragen aus einem privaten Netzwerk und erst, sobald eine gültige Uhrzeit ermittelt wurde.
- Bis zu 15 eigene NTP-Server hinterlegbar; ist keiner konfiguriert (oder werden alle gelöscht), fällt die Uhr automatisch auf `pool.ntp.org` und `ptbtime1.ptb.de` zurück. Sind die konfigurierten Server nicht erreichbar, werden diese beiden zusätzlich als letzter Fallback versucht.

---

## 6. Weboberfläche

- Dunkel gestaltete Einstellungszentrale mit Tabs (Status, WLAN, Uhr, Helligkeit, Zeit); dateilastige Seiten (Presets, Zifferblatt, Zeiger, Dateiverwaltung, Vorschau, Werksreset) separat über die Navigation erreichbar.
- Mehrsprachig (Deutsch, Englisch, Französisch); Info-Seite mit Projektbeschreibung und Kontakt.
- Zusätzliche Zifferblätter, Zeigersätze und Presets lassen sich direkt von GitHub herunterladen.
- Rocrail-Tab für die Modellzeit-Anbindung - siehe Abschnitt 11.

---

## 7. Dateiverwaltung mit LittleFS

- Zifferblätter und Zeiger werden komprimiert gespeichert; Dateien lassen sich über die Weboberfläche hoch-/herunterladen, umbenennen und löschen. Hochladen, Umbenennen und Löschen sind dabei nur bei Zugriff aus einem privaten Netzwerk möglich (Schutz vor Fernzugriff, z. B. über eine Portweiterleitung).
- Optionales Logging, einsehbar im Log-Tab mit Dateiauswahl (Dropdown zeigt alle vorhandenen Logdateien, neueste vorausgewählt) und Auto-Refresh.

---

## 8. Zeitzonen-Anpassung

- Automatische Sommerzeitumstellung oder dauerhaft Sommer-/Winterzeit einstellbar.
- Ist keine Zeitzone hinterlegt (leeres Feld) oder ist der eingetragene Wert kein gültiger POSIX-TZ-String, fällt die Uhr automatisch auf `CET-1CEST,M3.5.0,M10.5.0/3` (Mitteleuropäische Zeit) zurück.

---

## 9. Hardware-Integration

- Kompatibel mit dem ESP32-S2.
- Fotowiderstand zur Helligkeitsmessung.

---

## 10. Erweiterte Funktionen

- Laufzeit-Anzeige: zeigt die Laufzeit der Uhr seit dem letzten Neustart.
- Neustart-Funktion: erlaubt einen Neustart der Uhr über die Weboberfläche, nur bei Zugriff aus einem privaten Netzwerk.
- BMP-Skalierung: hochgeladene BMP-Dateien können auf die Displaygröße skaliert werden.
- API-Schnittstelle
- Bis zu 50 Presets (Zifferblatt, Zeigersatz, Nabenfarbe/-größe, Sekundenzeiger-Anzeige) - einzeln umbenenn- und löschbar (nur bei Zugriff aus einem privaten Netzwerk), alphabetisch sortiert in der Liste; alle Presets lassen sich in eine Datei sichern und später wiederherstellen; sind alle 50 Plätze belegt, erscheint eine Warnung.
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

- Supported displays: GC9A01, GC9D01, ILI9341 (deprecated).
- A second, identical display can optionally be driven, with its own rotation setting (0°, 90°, 180°, 270°) - a display that is not connected is set to "not connected (n.a.)": it then stays black and face/hands are neither drawn nor calculated for it. Default: display 1 at 0°, display 2 n.a. Status and boot messages (boot, access point mode, confirmation codes) still appear on both displays until the clock takes over (the settings page notes this).

---

## 2. Customizable Clock Hands and Faces

- Custom hour, minute, and second hands, as well as custom clock faces, can be uploaded as BMP files; default sets are included as a fallback.
- Hour and minute hands are rendered anti-aliased.

---

## 3. Smooth Minute and Train Station Modes

- Smooth Minute Mode: the minute hand moves smoothly instead of jumping in 1-minute increments.
- Train Station Mode: the second hand completes its round in 58.5 seconds and briefly pauses at the top on the 12, like a classic railway clock.

---

## 4. Brightness Control

- Automatic brightness via a photoresistor with configurable thresholds, or manual control.
- When connected to Rocrail, brightness can be taken over from the server instead - see section 11.

---

## 5. WiFi and NTP Integration

- Up to 15 WiFi networks, WPS setup, automatic reconnect, customizable hostname.
- Overwriting or switching away from the currently connected WiFi network is executed directly when accessed from the private (home) network; when accessed from a non-private network (e.g. via a port forward/DMZ), confirming a code shown on the display is required instead. Deleting a WiFi network (active or not), as well as any other changes to non-active networks, is only possible from a private network in general, with no code option at all.
- NTP with DCF77 as a fallback for time sync; the clock also acts as an NTP server for other devices on the network, but only answers requests from a private network and only once a valid time has been determined.
- Up to 15 custom NTP servers can be stored; if none are configured (or all are deleted), the clock automatically falls back to `pool.ntp.org` and `ptbtime1.ptb.de`. If the configured servers are unreachable, these two are additionally tried as a last-resort fallback.

---

## 6. Web Interface

- Dark-themed settings hub with tabs (Status, WLAN, Clock Setup, Brightness, Time); file-heavy pages (Presets, Clock Face, Hand Set, File Manager, Preview, Factory Reset) reachable separately via the navigation bar.
- Multi-language (German, English, French); Info page with project description and contact details.
- Additional clock faces, hand sets, and presets can be downloaded directly from GitHub.
- Rocrail tab for the model-time connection - see section 11.

---

## 7. File Management with LittleFS

- Clock faces and hands are stored compressed; files can be uploaded, downloaded, renamed, and deleted via the web interface. Uploading, renaming, and deleting are only possible when accessing the clock from a private network (protection against remote access, e.g. via a port forward).
- Optional logging, viewable in the Log tab with a file selector (dropdown shows all existing log files, newest preselected) and auto-refresh.

---

## 8. Time Zone Customization

- Automatic daylight saving time or permanent summer/winter time can be configured.
- If no timezone is stored (empty field) or the stored value is not a valid POSIX TZ string, the clock automatically falls back to `CET-1CEST,M3.5.0,M10.5.0/3` (Central European Time).

---

## 9. Hardware Integration

- Compatible with the ESP32-S2.
- Photoresistor for brightness measurement.

---

## 10. Advanced Features

- Uptime Display: Shows the clock's runtime since the last restart.
- Reboot Function: Allows restarting the clock via the web interface, only when accessing from a private network.
- BMP Scaling: Uploaded BMP files can be scaled to fit the display size.
- API Interface
- Up to 50 presets (face, hand set, hub color/size, second-hand display) - individually renameable and deletable (only when accessing from a private network), sorted alphabetically in the list; back up all presets to a file and restore them later; a warning is shown once all 50 slots are full.
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
