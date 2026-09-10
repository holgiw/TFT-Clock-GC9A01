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
- DCF77 wird unterstützt
- Der DCF77-Decoder arbeitet auf einem Ein-Sekunden-Raster: Impulse werden anhand des Abstands ihrer Startflanken platziert (immer eine ganze Anzahl Sekunden), nicht durch Zählen der Impulse. Eine zu schwach empfangene Sekunde hinterlässt daher nur an ihrer eigenen Position eine Lücke, statt alle folgenden Bits zu verschieben, und störungsbedingte Störflanken werden verworfen statt als zusätzliche Bits gezählt zu werden. Das Raster wird über Aussetzer von bis zu einer Minute hinweg fortgeführt.
- Der Bit-Fortschritt wird nach Rasterposition statt nach Sekunde der Minute erfasst, sodass er ab dem allerersten Impuls läuft - die Live-Seite bleibt während der Minuten, die zur Identifikation der Minutenmarke nötig sind, nicht mehr leer (sie weist darauf hin, solange diese Zuordnung noch fehlt). Das Raster wird bei jedem Überlauf geleert, solange die Marke unbekannt ist, sodass die Anzeige die letzte Minute tatsächlichen Empfangs zeigt statt Bits aus mehreren Minuten anzuhäufen.
- Die 59. Sekunde (die Minutenmarke, die einzige Sekunde ohne Impuls) wird statistisch statt anhand einer einzelnen Zwei-Sekunden-Lücke identifiziert: bei unvollkommenem Empfang erzeugt jede verlorene Sekunde genau dieselbe Lücke, sodass der Decoder sich früher immer wieder auf die falsche Position einrastete. Stattdessen zählt der Decoder für jede der 60 Rasterpositionen, wie oft dort ein Impuls fehlte - die Marke ist die eine Position, die in jeder einzelnen Minute fehlt, während Aussetzer zufällig streuen. Sie wird meist innerhalb von etwa drei Minuten identifiziert.
- Fehlende Bits werden rekonstruiert, wo es das Protokoll erlaubt: die festen Bits 0 und 20, die sich gegenseitig ausschließenden Sommer-/Winterzeit-Bits, und ein fehlendes Bit je Paritätsgruppe. Da die Parität eine so vervollständigte Gruppe nicht mehr verifizieren kann, wird ein solches Telegramm erst akzeptiert, wenn es zum vorigen Telegramm plus der seither vergangenen Minuten passt - zwei unabhängig voneinander empfangene Minuten, die exakt eine Minute auseinanderliegen. Vollständig empfangene Telegramme werden wie bisher sofort akzeptiert. Ein Fehler wurde behoben, bei dem der Rasterplatz von Bit 0 stets kurz vor der Dekodierung geleert wurde (er wird vom eigenen Bit 0 der folgenden Minute wiederverwendet, das genau in dem Moment eintrifft, in dem ein Telegramm vervollständigt wird) - jedes einzelne Telegramm zählte dadurch als mit 1 rekonstruiertem Bit, selbst bei perfektem Empfang. Der tatsächliche Wert dieses Platzes wird jetzt kurz bevor er überschrieben wird gesichert und für die Dekodierung wiederhergestellt, sodass "Rekonstruierte Bits" bei tatsächlich vollständigem Empfang wieder korrekt 0 anzeigt.
- Ein Telegramm wird nur akzeptiert, wenn die festen Bits des Protokolls (Bit 0 = 0, Bit 20 = 1), alle drei Paritäten und die Wertebereiche (Monat 1-12, Stunde 0-23, ...) passen - ein gestörtes Telegramm kann also keine falsche Zeit mehr setzen.
- Die 14 ungenutzten Wetter-/Sonderfunktions-Bits müssen nicht mehr empfangen werden: eine Lücke dort verwarf früher ein ansonsten vollständiges und korrektes Zeittelegramm.
- Der DCF77-Eingang wird auf beiden Flanken gelesen (Interrupt bei CHANGE) und rein anhand der Dauer zwischen zwei Flanken ausgewertet: von zwei aufeinanderfolgenden Intervallen ist das kurze immer der Impuls, das lange die Lücke. Der Empfang funktioniert dadurch mit beiden Signalpolaritäten, ohne einstellbare oder automatisch erkannte Flankenrichtung - der Pin-Pegel wird nie gelesen. Die DCF77-Bibliothek wird gar nicht mehr genutzt und muss nicht installiert werden (der dafür früher nötige Patch an DCF77.cpp entfällt damit ebenfalls).
- Der DCF77-Interrupt-Handler ruft nichts mehr auf, was im Flash liegt (weder den Handler der DCF77-Bibliothek noch digitalRead), und sein Ringpuffer fasst 64 statt 16 Flanken. Während der Flash-Cache deaktiviert ist - bei jedem LittleFS-Schreibvorgang, also auch jeder Log-Zeile - kosten solche Aufrufe eingehende Flanken; der vergrößerte Puffer überbrückt zusätzlich blockierende Vorgänge wie einen NTP-Versuch oder eine große Web-Anfrage.
- DCF77-Live-Seite (/dcf77): zeigt den bitweisen Empfangsfortschritt des aktuellen Telegramms (59-Kästchen-Raster, sekündlich aktualisiert) sowie das letzte vollständig dekodierte Telegramm (Datum, Uhrzeit, Wochentag, Sommer-/Winterzeit, Rufbit, Paritätsprüfungen, Anzahl rekonstruierter Bits). Verlorene Sekunden werden mit gestricheltem Rahmen dargestellt. Die Diagnose umfasst verworfene Flanken (Ringpuffer-Überlauf), ob die Minutenmarke identifiziert wurde und an welcher Rasterposition, Zähler für gesehene/fehlende Impulse/Rasterverluste, sowie Breite und Abstand der letzten zwölf Impulse in Millisekunden - was ohne Oszilloskop zeigt, ob der Empfänger überhaupt ein brauchbares Signal liefert (erwartet: Breiten um 100 oder 200 ms, Abstände nahe einem Vielfachen von 1000 ms). Vollständig in alle drei Interface-Sprachen übersetzt (Deutsch, Englisch, Französisch), einschließlich der Tooltip-Beschreibung jedes Bits und des vom Poll-Skript aktualisierten Live-Sync-Status-Texts.
- Das DCF77-Sync-LED-Blinken lässt sich über eine Checkbox abschalten (Standard: an) - solange die Uhr das Zeitsignal noch einliest, blitzt die eingebaute LED kurz bei jedem empfangenen Bit auf, allerdings erst, sobald DCF77 tatsächlich erkannt wurde (mehrere aufeinanderfolgende, plausibel getaktete Pegelwechsel - dieselbe Prüfung wie beim Topbar-Punkt und Navigationseintrag), nicht schon bei einer einzelnen rohen Flanke/einem Störimpuls. Das ist bewusst deutlich früher als das Finden der Minutenmarke (was bis zu 3 Minuten dauern kann). Die Checkbox selbst wird ebenfalls erst angezeigt, sobald DCF77 auf diese Weise erkannt wurde - ohne angeschlossenen Empfänger (oder mit toter Antenne/totem Kabel) erscheint die Option gar nicht erst, statt eine Einstellung für ein sichtbar nicht funktionierendes Feature anzubieten. Das Blinken schaltet sich nach einer festen Zeit von selbst wieder ab, und die LED wird zusätzlich einmal pro Minute als Sicherheitsnetz ausgeschaltet, damit sie nie dauerhaft leuchten bleiben kann.

---

## 11. Rocrail-Modellzeit

- Optionale Verbindung zu einem [Rocrail](https://wiki.rocrail.net/)-Server (Modelleisenbahn-Steuerungssoftware): die Zeiger können statt der echten Zeit die "Fast Clock" (Modellzeit) des Servers anzeigen.
- Standardmäßig aus: ein "Rocrail"-Schalter auf dem Zeit-Tab aktiviert das Feature und schaltet den separaten "Rocrail"-Tab frei, in dem die Serveradresse(n) eingetragen werden - der Tab bleibt verborgen, bis der Schalter eingeschaltet wird, damit er die Einstellungen für alle ohne Anlage nicht überfrachtet.
- Bis zu 15 Serveradressen können hinterlegt werden (wie bei den NTP-Servern erscheint nach dem letzten befüllten Eintrag immer automatisch ein neuer, leerer Platz); ein Radio-Button je Zeile legt fest, welcher davon gerade der aktive Server ist. Bleibt beim Speichern ein Eintrag mittendrin leer, rücken die folgenden Einträge automatisch nach.
- Vor dem Verbindungsaufbau prüft ein ICMP-Ping, ob die Serveradresse überhaupt erreichbar ist, damit die Uhr nicht auf den (längeren) TCP-Verbindungs-Timeout wartet, wenn der Server aus oder im Netzwerk nicht erreichbar ist.
- Das Aktivieren des Rocrail-Schalters oder das Speichern einer Serveradresse (auf dem jeweiligen Tab) löst sofort einen Verbindungsversuch aus, wenn eine Serveradresse hinterlegt ist, statt auf das reguläre, einmal pro Minute wiederkehrende Zeitfenster zu warten.
- Derselbe sofortige Verbindungsversuch erfolgt auch direkt nach einem Neustart, wenn Rocrail bereits aktiviert und eine Serveradresse hinterlegt ist, statt auf das erste reguläre Zeitfenster zu warten.
- Bis das erste Modellzeit-Update tatsächlich eintrifft - oder wenn länger als 2 Minuten keine Updates mehr eintreffen - läuft die Uhr weiter mit der normalen NTP-/RTC-/DCF77-Zeit, sodass sie nie bei einer veralteten oder leeren Modellzeit hängen bleibt.
- Die Modellzeit läuft mit Rocrails eigenem Beschleunigungsfaktor (dem "Divider", z.B. 10-fache Realzeit) statt 1:1 zur echten Zeit - die Uhr schreibt zwischen den Server-Updates mit derselben Rate eigenständig fort, friert also bei einem kurzen Netzwerk-Aussetzer nicht ein und gleicht sich beim nächsten Update wieder an. Pausiert die Automatik der Anlage, pausieren die Zeiger direkt mit, statt weiterzulaufen.
- Die Sekundenzeiger-Animation der Bahnhofsuhr skaliert mit dem Divider (schnellerer Umlauf, kürzere Pause oben) statt sich abzuschalten, und läuft dabei weiterhin gleichmäßig statt einmal pro Sekunde zu ticken; oberhalb eines einstellbaren Divider-Schwellwerts wird der Sekundenzeiger ganz ausgeblendet, da er ohnehin nicht mehr sinnvoll ablesbar wäre.
- Der Tab zeigt zu Diagnosezwecken einen Live-Verbindungsstatus, den Divider (erst sobald das erste Modellzeit-Update tatsächlich eingetroffen ist, davor "-", genau wie bei der Modellzeit selbst) und die aktuelle Modellzeit. Für jeden gespeicherten Server lässt sich außerdem frei ein Anlagenname eintragen (rein zur eigenen Orientierung, z.B. bei mehreren hinterlegten Servern) - eine aktive Anfrage danach bei Rocrail ist aus Stabilitätsgründen bewusst nicht vorgesehen (das würde den vollständigen Anlagenplan anfordern und könnte den begrenzten Speicher des ESP32 sprengen); sendet der Server ihn zufällig unaufgefordert, wird er übernommen, darauf ist aber kein Verlass.
- Meldet der Server über seine Fast-Clock-Updates zusätzlich einen Helligkeitswert, übernimmt die Uhr auch die Display-Helligkeit von dort - siehe Abschnitt 4.
- Die Web-Vorschau (Seite "Vorschau") spiegelt die Rocrail-Modellzeit auf dieselbe Weise: solange verbunden, zeigt sie die Modellzeit statt der echten Zeit, tickt mit der Geschwindigkeit des Dividers, und auch ihre Bahnhofsuhr-Sweep-Animation skaliert mit dem Divider (passend zum physischen Display); ein Hinweis über der Uhr weist darauf hin und zeigt die aktuelle Geschwindigkeit.

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
- DCF77 supported
- DCF77 decoder works on a one-second grid: pulses are placed by the distance between their start edges (always a whole number of seconds), not by counting pulses. A second that is received too weakly therefore only leaves a hole at its own position instead of shifting every following bit, and spurious edges caused by interference are discarded instead of being counted as extra bits. The grid is carried across dropouts of up to a minute.
- The bit progress is collected by grid position rather than by second of the minute, so it runs from the very first pulse - the live page no longer stays empty during the minutes it takes to identify the minute marker (it says so while that mapping is still missing). The grid is cleared on every wrap while the marker is unknown, so the display shows the last minute of actual reception rather than accumulating bits from several minutes.
- The 59th second (the minute marker, the only second without a pulse) is identified statistically rather than from a single two-second gap: with imperfect reception every lost second produces exactly the same gap, so the decoder used to lock onto the wrong position again and again. Instead, the decoder counts for each of the 60 grid positions how often a pulse was missing there - the marker is the one position that is missing in every single minute, while dropouts scatter randomly. It is usually identified within about three minutes.
- Missing bits are reconstructed where the protocol allows it: the fixed bits 0 and 20, the mutually inverse summer/winter time bits, and one missing bit per parity group. Since parity can no longer verify a group completed this way, such a telegram is only accepted once it matches the previous telegram plus the minutes elapsed since - two independently received minutes exactly one minute apart. Fully received telegrams are accepted immediately, as before. Fixed a bug where bit 0's grid slot was always cleared right before decoding (it gets reused by the following minute's own bit 0, which arrives at exactly the moment a telegram is completed) - every single telegram counted as having 1 reconstructed bit even under perfect reception. The slot's genuine value is now saved just before it's overwritten and restored for decoding, so "Reconstructed bits" correctly reads 0 again when nothing was actually missing.
- A telegram is only accepted when the protocol's fixed bits (bit 0 = 0, bit 20 = 1), all three parities and the value ranges (month 1-12, hour 0-23, ...) fit - so a disturbed telegram can no longer set a wrong time.
- The 14 unused weather/special-function bits no longer have to be received: a gap there used to discard an otherwise complete and correct time telegram.
- The DCF77 input is read on both edges (interrupt on CHANGE) and evaluated purely by the duration between two edges: of two consecutive intervals the short one is always the pulse, the long one the gap. Reception therefore works with either signal polarity, without a configurable or auto-detected edge direction - the pin level is never read. The DCF77 library is no longer used at all and does not need to be installed (the patch to DCF77.cpp it used to require is gone with it).
- The DCF77 interrupt handler no longer calls anything that lives in flash (neither the DCF77 library's handler nor digitalRead), and its ring buffer holds 64 instead of 16 edges. While the flash cache is disabled - during every LittleFS write, so also every log line - such calls cost incoming edges; the enlarged buffer additionally bridges blocking operations such as an NTP attempt or a large web request.
- DCF77 live page (/dcf77): shows the bit-by-bit reception progress of the current telegram (59-box grid, updated every second) plus the last fully decoded telegram (date, time, weekday, summer/winter time, call bit, parity checks, number of reconstructed bits). Seconds that were lost are shown with a dashed border. Diagnostics cover dropped edges (ring buffer overflow), whether the minute marker has been identified and at which grid position, counters for pulses seen/missing/grid losses, and the raw width and spacing in milliseconds of the last twelve pulses - which shows without an oscilloscope whether the receiver delivers a usable signal at all (expected: widths around 100 or 200 ms, spacing close to a multiple of 1000 ms). Fully translated into all three interface languages (German, English, French), including every bit's tooltip description and the live sync-status text updated by the poll script.
- DCF77 sync LED blink can be turned off via a checkbox (default: on) - while the clock is still acquiring the time signal, the onboard LED flashes briefly for every received bit, but only once DCF77 has actually been recognized (several consecutive, plausibly timed level changes - the same check used for the topbar dot and navigation entry), not for a single raw edge/noise spike. This is deliberately much earlier than the minute marker being found (which can take up to 3 minutes). The checkbox itself is also only shown once DCF77 has been recognized this way - without a receiver wired up (or with a dead antenna/cable), the option doesn't appear at all instead of offering a setting for a feature that's visibly not working. The flash switches itself off again after a fixed time, and the LED is additionally switched off once per minute as a safety net, so it can never stay lit permanently

---

## 11. Rocrail Model Time

- Optional connection to a [Rocrail](https://wiki.rocrail.net/) server (model-railroad control software): the hands can display the server's "fast clock" (model time) instead of the real time.
- Off by default: a "Rocrail" switch on the Time tab enables the feature and unlocks the separate "Rocrail" tab, where the server address(es) are entered - the tab stays hidden until the switch is turned on, so it doesn't clutter the settings for anyone without a layout.
- Up to 15 server addresses can be stored (like the NTP servers, a new empty slot always appears automatically after the last filled entry); a radio button per row selects which one is currently the active server. If an entry in the middle is left empty when saving, the following entries automatically move up.
- Before connecting, an ICMP ping checks whether the server address is reachable at all, so the clock doesn't hang waiting on the (longer) TCP connection timeout when the server is off or unreachable on the network.
- Enabling the Rocrail switch or saving a server address (both on their respective tabs) triggers an immediate connection attempt if a server address is configured, instead of waiting for the regular once-a-minute retry window.
- The same immediate connection attempt happens right after a restart if Rocrail was already enabled with a server address configured, instead of waiting for the first regular retry window.
- Until the first model-time update actually arrives - or if updates stop coming in for more than 2 minutes - the clock keeps running on the normal NTP/RTC/DCF77 time, so it never gets stuck showing a stale or blank model time.
- Model time runs at Rocrail's own acceleration factor (the "divider", e.g. 10x real speed) rather than 1:1 with real time - the clock keeps advancing independently between server updates at that same rate, so it doesn't freeze during a brief network hiccup, and corrects itself on the next update. If the layout's automation is paused, the hands pause right along with it instead of drifting ahead.
- The station-clock second-hand animation scales with the divider (faster lap, shorter pause at the top) instead of switching off, and runs smoothly rather than ticking once per second while in this mode; above a configurable divider threshold the second hand is hidden entirely, since it would no longer be meaningfully readable.
- The tab shows a live connection status, the divider (only once the first model-time update has actually arrived, "-" before that, exactly like the model time itself) and the current model time for diagnostics. Each stored server also has a freely editable layout name (purely for your own reference, e.g. with several servers configured) - actively requesting it from Rocrail is deliberately not implemented for stability reasons (that would request the complete layout plan and could exceed the ESP32's limited memory); if the server happens to send it unprompted, it's picked up, but this can't be relied on.
- If the server also reports a brightness value on its fast-clock updates, the display's brightness is taken over from there too - see section 4.
- The web preview (Preview page) reflects Rocrail model time the same way: while connected, it shows the model time instead of real time, ticks at the divider's speed, and its station-clock sweep animation scales with the divider too (matching the physical display); a hint above the clock notes this and the current speed.
