#pragma once
    // Info-Seite: Projektbeschreibung in drei Sprachen als fertiges HTML
    // direkt aus dem Flash gestreamt, ohne Heap-Kopie und ohne Markdown-
    // Parser. Umlaute als HTML-Entities wie in translation.h.

    // Info page: project description in three languages as finished HTML,
    // streamed directly from flash without a heap copy or a Markdown
    // parser. Umlauts as HTML entities, as in translation.h.

static const char README_HTML_EN[] = R"rawliteral(
<h2>1. Support for Multiple TFT Displays</h2>
<ul>
<li>Supported displays: GC9A01, GC9D01, ILI9341 (deprecated).</li>
<li>An optional second, identical display can be driven on its own chip-select pin (CS2) on the shared SPI bus. Each display has its own rotation setting, so both can be mounted in different orientations. A display that is not connected is set to &quot;n.a.&quot; - it then stays black and the clock face is neither drawn nor calculated for it (default: display 1 at 0&deg;, display 2 n.a.). Status and boot messages (boot, access point mode, confirmation codes) appear on both displays until the clock takes over, each correctly rotated (n.a. counts as 0&deg;).</li>
</ul>

<h2>2. Customizable Hands and Clock Faces</h2>
<ul>
<li>Custom hour, minute and second hands can be uploaded as BMP files; a default set is built in.</li>
<li>Hour and minute hands are anti-aliased (3x3 supersampling). They are drawn into a cached composite image that is only rebuilt when an angle actually changes, so the sweeping second hand keeps full frame rate.</li>
<li>Custom clock faces can be uploaded and selected; a default face is built in.</li>
</ul>

<h2>3. Smooth Minute and Train Station Modes</h2>
<ul>
<li>Smooth minute mode: the minute hand moves continuously instead of jumping once a minute.</li>
<li>Train station mode: the second hand steps around in 60 steps, accelerating and braking within each step, completes its round in 58.5 s and then rests at the 12 until the minute changes - like the classic railway clock.</li>
</ul>

<h2>4. Brightness Control</h2>
<ul>
<li>Automatic brightness from a photoresistor, with configurable thresholds and gamma.</li>
<li>A daily time window can force full brightness regardless of ambient light.</li>
<li>Manual brightness control if no photoresistor is detected.</li>
</ul>

<h2>5. WiFi, mDNS and Time Synchronization</h2>
<ul>
<li>Up to 15 WiFi networks. Scanning shows signal strength and encryption; the strongest networks are kept and listed first.</li>
<li>Hostname and all WiFi settings live on the WLAN tab, which opens automatically when no known network is reachable.</li>
<li>Automatic reconnect, individual networks deletable, and WPS setup with a single button; overwriting or switching away from the currently connected network is executed directly when accessed from the private network, but requires confirming a code shown on the display when accessed from a non-private network (e.g. via a port forward/DMZ). Deleting a network (active or not), and any other changes to non-active networks, is only possible from a private network in general, with no code option at all.</li>
<li>The clock is reachable at http://&lt;hostname&gt;.local via mDNS; the link is only offered when mDNS actually started.</li>
<li>NTP has priority and is checked every 6 hours, with a retry per server before moving to the next; if none are configured, or the configured ones are unreachable, pool.ntp.org and ptbtime1.ptb.de are used as a built-in fallback.</li>
<li>If NTP is unavailable, the last valid DCF77 telegram is used, provided it is parity-correct and no older than 10 minutes - the elapsed time is added back on.</li>
<li>The RTC is set at boot and periodically from whichever source succeeded.</li>
<li>The clock also acts as an NTP server on port 123 for other devices. It only answers requests from a private network, and only once its own time is valid, no matter which source provided it.</li>
</ul>

<h2>6. DCF77 Reception</h2>
<ul>
<li>The receiver input is read on both edges and evaluated purely by the duration between them, so either signal polarity works without any configuration.</li>
<li>Pulses are placed on a one-second grid: a weakly received second only leaves a gap at its own position instead of shifting all following bits.</li>
<li>The minute marker (the 59th second, the only one without a pulse) is identified statistically over several minutes rather than from a single two-second gap - which is what makes reception work at all when individual seconds drop out.</li>
<li>Missing bits are reconstructed where the protocol allows it (fixed bits, the summer/winter time pair, one bit per parity group). Such a telegram is only accepted once it matches the previous one plus the minutes elapsed since.</li>
<li>A telegram is accepted only when fixed bits, all three parities and the value ranges fit, so a disturbed telegram cannot set a wrong time.</li>
<li>The live page /dcf77 shows the bit progress, the last decoded telegram and diagnostics down to the raw pulse widths.</li>
</ul>

<h2>7. Web Interface</h2>
<ul>
<li>Dark-themed settings hub: Status, WLAN, Clock Setup, Brightness, Time/NTP and Log are tabs on one page.</li>
<li>Separate pages for Presets, Clock Face, Hand Set, File Manager, Live Preview, DCF77 and Factory Reset.</li>
<li>Multi-language interface: German, English, French.</li>
<li>Live status bar with clock and status dots for time, RTC, DCF77 and ambient light, updated every few seconds.</li>
<li>Additional clock faces, hand sets and presets can be downloaded from GitHub; missing dependencies are fetched automatically.</li>
<li>Factory reset with separate options: everything, clock faces, hand sets, presets or saved WiFi networks.</li>
</ul>

<h2>8. File Management with LittleFS</h2>
<ul>
<li>Clock faces and hands are stored RLE-compressed to save flash space.</li>
<li>Upload, download, rename and delete through a compact icon-based file manager; uploading, renaming and deleting are only available from a private network.</li>
<li>Optional logging to up to 9 rotating log files, viewable live in the Log tab.</li>
</ul>

<h2>9. Time Zones</h2>
<ul>
<li>Automatic daylight saving (e.g. CET/CEST) or permanent summer or winter time.</li>
<li>Custom timezone strings can be entered directly.</li>
<li>If no timezone is set, or the entered value isn't a valid POSIX TZ string, the clock falls back to CET-1CEST,M3.5.0,M10.5.0/3 (Central European Time).</li>
</ul>

<h2>10. Further Features</h2>
<ul>
<li>Up to 50 presets (face, hand set, hub color and size, second hand) - renameable, deletable (only from a private network), sorted alphabetically, with backup and restore.</li>
<li>Deleting a face or hand set removes any presets referring to it; deleting the active hand set falls back to the default.</li>
<li>Uploaded BMP files can be scaled to the display size.</li>
<li>Uptime display, reboot from the web interface (only from a private network), weekly preventive restart.</li>
<li>API interface for switching settings from outside.</li>
<li>Hardware: ESP32-S2, photoresistor for brightness, optional DS3231 real time clock, DCF77 receiver module.</li>
</ul>

<h2>11. Rocrail Model Time</h2>
<ul>
<li>Optional connection to a Rocrail server (model railroad control software): the hands can show its "fast clock" (model time) instead of the real time, enabled via a switch on the Time tab.</li>
<li>Enabling Rocrail, saving its server address, or restarting the clock all trigger an immediate connection attempt instead of waiting for the once-a-minute retry.</li>
<li>Model time runs at Rocrail's own acceleration factor (the "divider") rather than 1:1; the clock keeps advancing independently between server updates and falls back to real time if updates stop for more than 2 minutes.</li>
<li>The station-clock second hand scales its speed with the divider instead of switching off, and is hidden above a configurable divider threshold since it would no longer be readable.</li>
<li>If the server also reports a brightness value, the display takes its brightness from there instead of the photoresistor/time window, resuming automatically once the connection drops.</li>
<li>The Preview page mirrors the same model time and divider speed.</li>
<li>The Rocrail tab shows a live connection status, the divider, the plan name and the current model time.</li>
</ul>
)rawliteral";

static const char README_HTML_DE[] = R"rawliteral(
<h2>1. Unterst&uuml;tzte TFT-Displays</h2>
<ul>
<li>Unterst&uuml;tzt werden GC9A01, GC9D01 und ILI9341 (nicht mehr gepflegt).</li>
<li>Ein optionales zweites, baugleiches Display kann &uuml;ber einen eigenen Chip-Select-Pin (CS2) am gemeinsamen SPI-Bus angesteuert werden. Jedes Display hat seine eigene Rotationseinstellung, beide k&ouml;nnen also unterschiedlich eingebaut sein. Ein nicht angeschlossenes Display wird auf &quot;n.a.&quot; gestellt - es bleibt dann schwarz, Zifferblatt und Zeiger werden daf&uuml;r weder gezeichnet noch berechnet (Standard: Display 1 mit 0&deg;, Display 2 n.a.). Status- und Startmeldungen (Start, Access-Point-Modus, Best&auml;tigungscodes) erscheinen bis zum Uhrstart auf beiden Displays, jeweils korrekt gedreht (n.a. z&auml;hlt als 0&deg;).</li>
</ul>

<h2>2. Eigene Zeiger und Zifferbl&auml;tter</h2>
<ul>
<li>Stunden-, Minuten- und Sekundenzeiger lassen sich als BMP-Dateien hochladen; ein Standardsatz ist eingebaut.</li>
<li>Stunden- und Minutenzeiger werden kantengegl&auml;ttet gezeichnet (3x3-&Uuml;berabtastung). Sie landen in einem zwischengespeicherten Bild, das nur neu aufgebaut wird, wenn sich ein Winkel wirklich &auml;ndert - der schleichende Sekundenzeiger beh&auml;lt so seine volle Bildrate.</li>
<li>Eigene Zifferbl&auml;tter k&ouml;nnen hochgeladen und ausgew&auml;hlt werden; ein Standard-Zifferblatt ist eingebaut.</li>
</ul>

<h2>3. Sanfter Minutenzeiger und Bahnhofsuhr-Modus</h2>
<ul>
<li>Sanfter Minutenzeiger: die Minute l&auml;uft gleichm&auml;&szlig;ig durch, statt einmal pro Minute zu springen.</li>
<li>Bahnhofsuhr-Modus: der Sekundenzeiger l&auml;uft in 60 Schritten um, beschleunigt und bremst innerhalb jedes Schritts, braucht 58,5 s f&uuml;r einen Umlauf und wartet dann oben auf der 12 auf den Minutenwechsel - wie das Original.</li>
</ul>

<h2>4. Helligkeitssteuerung</h2>
<ul>
<li>Automatische Helligkeit &uuml;ber einen Fotowiderstand, mit einstellbaren Schwellen und Gamma-Korrektur.</li>
<li>Ein t&auml;gliches Zeitfenster kann unabh&auml;ngig vom Umgebungslicht volle Helligkeit erzwingen.</li>
<li>Ohne erkannten Fotowiderstand l&auml;sst sich die Helligkeit von Hand einstellen.</li>
</ul>

<h2>5. WLAN, mDNS und Zeitsynchronisation</h2>
<ul>
<li>Bis zu 15 WLAN-Netzwerke. Der Scan zeigt Signalst&auml;rke und Verschl&uuml;sselung; die st&auml;rksten Netze werden behalten und zuerst gelistet.</li>
<li>Hostname und alle WLAN-Einstellungen liegen auf dem WLAN-Tab, das automatisch &ouml;ffnet, wenn kein bekanntes Netz erreichbar ist.</li>
<li>Automatischer Reconnect, einzeln l&ouml;schbare Netzwerke und WPS-Einrichtung per Knopfdruck; das &Uuml;berschreiben oder Wechseln weg vom aktuell verbundenen Netzwerk wird bei Zugriff aus dem privaten Netzwerk direkt ausgefuehrt, verlangt bei Zugriff aus einem nicht-privaten Netzwerk (z.B. ueber eine Port-Weiterleitung/DMZ) aber die Best&auml;tigung eines auf dem Display angezeigten Codes. Das L&ouml;schen eines Netzwerks (aktiv oder nicht) sowie alle sonstigen &Auml;nderungen an nicht aktiven Netzwerken sind dagegen grunds&auml;tzlich nur aus einem privaten Netzwerk m&ouml;glich, ganz ohne Code-Option.</li>
<li>Die Uhr ist per mDNS unter http://&lt;hostname&gt;.local erreichbar; der Link wird nur angeboten, wenn mDNS tats&auml;chlich gestartet ist.</li>
<li>NTP hat Vorrang und wird alle 6 Stunden gepr&uuml;ft, mit einem Wiederholungsversuch je Server, bevor der n&auml;chste an die Reihe kommt; ist keiner konfiguriert oder sind die konfigurierten nicht erreichbar, dienen pool.ntp.org und ptbtime1.ptb.de als eingebauter R&uuml;ckfall.</li>
<li>Ist NTP nicht verf&uuml;gbar, springt das zuletzt empfangene DCF77-Telegramm ein, sofern seine Parit&auml;t stimmt und es h&ouml;chstens 10 Minuten alt ist - die seitdem vergangene Zeit wird aufgerechnet.</li>
<li>Die RTC wird beim Start und periodisch von der Quelle gestellt, die tats&auml;chlich Erfolg hatte.</li>
<li>Die Uhr ist selbst NTP-Server auf Port 123 f&uuml;r andere Ger&auml;te. Sie antwortet nur auf Anfragen aus einem privaten Netzwerk, und erst, sobald ihre eigene Zeit g&uuml;ltig ist - unabh&auml;ngig davon, woher sie stammt.</li>
</ul>

<h2>6. DCF77-Empfang</h2>
<ul>
<li>Der Empf&auml;ngereingang wird auf beiden Flanken ausgewertet, und zwar rein &uuml;ber die Dauer dazwischen - beide Signalpolarit&auml;ten funktionieren also ohne jede Einstellung.</li>
<li>Die Impulse liegen auf einem Sekundenraster: eine schwach empfangene Sekunde hinterl&auml;sst nur an ihrer eigenen Stelle eine L&uuml;cke, statt alle folgenden Bits zu verschieben.</li>
<li>Die Minutenmarke (die 59. Sekunde, die einzige ohne Impuls) wird statistisch &uuml;ber mehrere Minuten bestimmt statt aus einer einzelnen Zwei-Sekunden-Pause - erst dadurch funktioniert der Empfang &uuml;berhaupt, wenn einzelne Sekunden ausfallen.</li>
<li>Fehlende Bits werden rekonstruiert, wo das Protokoll es zul&auml;sst (Festbits, das Sommer-/Winterzeit-Paar, ein Bit je Parit&auml;tsgruppe). Ein so erg&auml;nztes Telegramm gilt erst als g&uuml;ltig, wenn es zur Vorminute plus der verstrichenen Zeit passt.</li>
<li>Angenommen wird ein Telegramm nur, wenn Festbits, alle drei Parit&auml;ten und die Wertebereiche stimmen - ein gest&ouml;rtes Telegramm kann also keine falsche Zeit stellen.</li>
<li>Die Live-Seite /dcf77 zeigt den Bit-Fortschritt, das zuletzt dekodierte Telegramm und Diagnosewerte bis hin zu den rohen Impulsl&auml;ngen.</li>
</ul>

<h2>7. Weboberfl&auml;che</h2>
<ul>
<li>Dunkel gehaltene Einstellungsseite: Status, WLAN, Uhr-Einstellungen, Helligkeit, Zeit/NTP und Log sind Tabs auf einer Seite.</li>
<li>Eigene Seiten f&uuml;r Presets, Zifferblatt, Zeigersatz, Dateimanager, Live-Vorschau, DCF77 und Werkseinstellungen.</li>
<li>Mehrsprachige Oberfl&auml;che: Deutsch, Englisch, Franz&ouml;sisch.</li>
<li>Statusleiste mit Uhrzeit und Statuspunkten f&uuml;r Zeit, RTC, DCF77 und Umgebungslicht, die sich alle paar Sekunden aktualisieren.</li>
<li>Weitere Zifferbl&auml;tter, Zeigers&auml;tze und Presets lassen sich von GitHub laden; fehlende Abh&auml;ngigkeiten werden automatisch mitgeholt.</li>
<li>Werkseinstellungen mit getrennten Optionen: alles, Zifferbl&auml;tter, Zeigers&auml;tze, Presets oder gespeicherte WLAN-Netzwerke.</li>
</ul>

<h2>8. Dateiverwaltung mit LittleFS</h2>
<ul>
<li>Zifferbl&auml;tter und Zeiger werden RLE-komprimiert gespeichert, das spart Flash-Speicher.</li>
<li>Hochladen, Herunterladen, Umbenennen und L&ouml;schen &uuml;ber einen kompakten Dateimanager mit Symbolen; Hochladen, Umbenennen und L&ouml;schen sind nur aus einem privaten Netzwerk m&ouml;glich.</li>
<li>Optionales Logging in bis zu 9 rotierende Logdateien, live im Log-Tab einsehbar.</li>
</ul>

<h2>9. Zeitzonen</h2>
<ul>
<li>Automatische Sommer-/Winterzeit (z.B. MEZ/MESZ) oder dauerhaft Sommer- bzw. Winterzeit.</li>
<li>Eigene Zeitzonen-Zeichenketten k&ouml;nnen direkt eingegeben werden.</li>
<li>Ist keine Zeitzone eingetragen oder der eingetragene Wert kein g&uuml;ltiger POSIX-TZ-String, f&auml;llt die Uhr automatisch auf CET-1CEST,M3.5.0,M10.5.0/3 (Mitteleurop&auml;ische Zeit) zur&uuml;ck.</li>
</ul>

<h2>10. Weitere Funktionen</h2>
<ul>
<li>Bis zu 50 Presets (Zifferblatt, Zeigersatz, Nabenfarbe und -gr&ouml;&szlig;e, Sekundenzeiger) - umbenennbar, l&ouml;schbar (nur aus einem privaten Netzwerk), alphabetisch sortiert, mit Sicherung und Wiederherstellung.</li>
<li>Wird ein Zifferblatt oder Zeigersatz gel&ouml;scht, verschwinden die Presets, die darauf verweisen; beim aktiven Zeigersatz wird auf den Standard zur&uuml;ckgeschaltet.</li>
<li>Hochgeladene BMP-Dateien k&ouml;nnen auf die Displaygr&ouml;&szlig;e skaliert werden.</li>
<li>Anzeige der Laufzeit, Neustart &uuml;ber die Weboberfl&auml;che (nur aus einem privaten Netzwerk), w&ouml;chentlicher vorbeugender Neustart.</li>
<li>API-Schnittstelle zum Umschalten der Einstellungen von au&szlig;en.</li>
<li>Hardware: ESP32-S2, Fotowiderstand f&uuml;r die Helligkeit, optionale DS3231-Echtzeituhr, DCF77-Empfangsmodul.</li>
</ul>

<h2>11. Rocrail-Modellzeit</h2>
<ul>
<li>Optionale Verbindung zu einem Rocrail-Server (Modelleisenbahn-Steuerungssoftware): die Zeiger k&ouml;nnen statt der echten Zeit dessen "Fast Clock" (Modellzeit) anzeigen, aktiviert &uuml;ber einen Schalter auf dem Zeit-Tab.</li>
<li>Rocrail aktivieren, die Serveradresse speichern oder ein Neustart der Uhr l&ouml;sen jeweils sofort einen Verbindungsversuch aus, statt auf den min&uuml;tlichen Wiederholungsversuch zu warten.</li>
<li>Die Modellzeit l&auml;uft mit Rocrails eigenem Beschleunigungsfaktor (dem "Divider") statt 1:1; die Uhr schreibt zwischen den Server-Updates eigenst&auml;ndig fort und f&auml;llt auf die echte Zeit zur&uuml;ck, wenn l&auml;nger als 2 Minuten keine Updates mehr eintreffen.</li>
<li>Der Sekundenzeiger der Bahnhofsuhr skaliert seine Geschwindigkeit mit dem Divider statt sich abzuschalten, und wird oberhalb eines einstellbaren Divider-Schwellwerts ausgeblendet, da er sonst nicht mehr ablesbar w&auml;re.</li>
<li>Meldet der Server zus&auml;tzlich einen Helligkeitswert, &uuml;bernimmt das Display dessen Helligkeit statt Fotowiderstand/Zeitfenster und schaltet automatisch zur&uuml;ck, sobald die Verbindung endet.</li>
<li>Die Vorschau-Seite spiegelt dieselbe Modellzeit und Divider-Geschwindigkeit.</li>
<li>Der Rocrail-Tab zeigt Live-Verbindungsstatus, Divider, Anlagenname und aktuelle Modellzeit.</li>
</ul>
)rawliteral";

static const char README_HTML_FR[] = R"rawliteral(
<h2>1. &Eacute;crans TFT pris en charge</h2>
<ul>
<li>&Eacute;crans pris en charge&nbsp;: GC9A01, GC9D01, ILI9341 (obsol&egrave;te).</li>
<li>Un second &eacute;cran identique peut &ecirc;tre pilot&eacute; via sa propre broche de s&eacute;lection (CS2) du bus SPI partag&eacute;. Chaque &eacute;cran a sa propre rotation, les deux peuvent donc &ecirc;tre mont&eacute;s diff&eacute;remment. Un &eacute;cran non raccord&eacute; se r&egrave;gle sur &quot;n.a.&quot; - il reste alors noir et le cadran et les aiguilles ne sont ni dessin&eacute;s ni calcul&eacute;s pour lui (par d&eacute;faut : &eacute;cran 1 &agrave; 0&deg;, &eacute;cran 2 n.a.). Les messages d&#39;&eacute;tat et de d&eacute;marrage (d&eacute;marrage, mode point d&#39;acc&egrave;s, codes de confirmation) s&#39;affichent sur les deux &eacute;crans jusqu&#39;&agrave; ce que l&#39;horloge prenne le relais, correctement orient&eacute;s (n.a. compte comme 0&deg;).</li>
</ul>

<h2>2. Aiguilles et cadrans personnalis&eacute;s</h2>
<ul>
<li>Les aiguilles des heures, des minutes et la trotteuse peuvent &ecirc;tre t&eacute;l&eacute;charg&eacute;es en BMP&nbsp;; un jeu par d&eacute;faut est int&eacute;gr&eacute;.</li>
<li>Les aiguilles des heures et des minutes sont liss&eacute;es (sur&eacute;chantillonnage 3x3). Elles sont dessin&eacute;es dans une image mise en cache, reconstruite uniquement lorsqu&#39;un angle change r&eacute;ellement - la trotteuse conserve ainsi sa fluidit&eacute;.</li>
<li>Des cadrans personnalis&eacute;s peuvent &ecirc;tre t&eacute;l&eacute;charg&eacute;s et s&eacute;lectionn&eacute;s&nbsp;; un cadran par d&eacute;faut est int&eacute;gr&eacute;.</li>
</ul>

<h2>3. Minute fluide et mode gare</h2>
<ul>
<li>Minute fluide&nbsp;: l&#39;aiguille des minutes avance en continu au lieu de sauter une fois par minute.</li>
<li>Mode gare&nbsp;: la trotteuse avance en 60 pas, acc&eacute;l&egrave;re et freine &agrave; chaque pas, effectue son tour en 58,5&nbsp;s puis attend le changement de minute sur le 12 - comme l&#39;horloge de gare d&#39;origine.</li>
</ul>

<h2>4. R&eacute;glage de la luminosit&eacute;</h2>
<ul>
<li>Luminosit&eacute; automatique par photor&eacute;sistance, avec seuils et correction gamma r&eacute;glables.</li>
<li>Une plage horaire quotidienne peut imposer la luminosit&eacute; maximale ind&eacute;pendamment de la lumi&egrave;re ambiante.</li>
<li>R&eacute;glage manuel si aucune photor&eacute;sistance n&#39;est d&eacute;tect&eacute;e.</li>
</ul>

<h2>5. WiFi, mDNS et synchronisation de l&#39;heure</h2>
<ul>
<li>Jusqu&#39;&agrave; 15 r&eacute;seaux WiFi. Le balayage indique la puissance du signal et le chiffrement&nbsp;; les r&eacute;seaux les plus forts sont conserv&eacute;s et list&eacute;s en premier.</li>
<li>Le nom d&#39;h&ocirc;te et tous les param&egrave;tres WiFi se trouvent sur l&#39;onglet WLAN, qui s&#39;ouvre automatiquement si aucun r&eacute;seau connu n&#39;est joignable.</li>
<li>Reconnexion automatique, suppression individuelle des r&eacute;seaux et configuration WPS en un bouton&nbsp;; le remplacement ou le changement depuis le r&eacute;seau actuellement connect&eacute; est ex&eacute;cut&eacute; directement en cas d&#39;acc&egrave;s depuis le r&eacute;seau priv&eacute;, mais exige la confirmation d&#39;un code affich&eacute; sur l&#39;&eacute;cran en cas d&#39;acc&egrave;s depuis un r&eacute;seau non priv&eacute; (par ex. via une redirection de port/DMZ). La suppression d&#39;un r&eacute;seau (actif ou non), ainsi que toute autre modification des r&eacute;seaux non actifs, n&#39;est possible que depuis un r&eacute;seau priv&eacute;, sans aucune option de code.</li>
<li>L&#39;horloge est joignable via mDNS &agrave; http://&lt;hostname&gt;.local&nbsp;; le lien n&#39;est propos&eacute; que si mDNS a r&eacute;ellement d&eacute;marr&eacute;.</li>
<li>NTP est prioritaire et v&eacute;rifi&eacute; toutes les 6 heures, avec une nouvelle tentative par serveur avant de passer au suivant&nbsp;; si aucun serveur n&#39;est configur&eacute;, ou si les serveurs configur&eacute;s sont inaccessibles, pool.ntp.org et ptbtime1.ptb.de servent de solution de repli int&eacute;gr&eacute;e.</li>
<li>Si NTP est indisponible, le dernier t&eacute;l&eacute;gramme DCF77 valide prend le relais, &agrave; condition que sa parit&eacute; soit correcte et qu&#39;il ait au plus 10 minutes - le temps &eacute;coul&eacute; depuis est ajout&eacute;.</li>
<li>Le RTC est r&eacute;gl&eacute; au d&eacute;marrage et p&eacute;riodiquement par la source qui a r&eacute;ellement abouti.</li>
<li>L&#39;horloge est elle-m&ecirc;me serveur NTP sur le port 123. Elle ne r&eacute;pond qu&#39;aux demandes provenant d&#39;un r&eacute;seau priv&eacute;, et seulement d&egrave;s que sa propre heure est valide, quelle qu&#39;en soit la source.</li>
</ul>

<h2>6. R&eacute;ception DCF77</h2>
<ul>
<li>L&#39;entr&eacute;e du r&eacute;cepteur est lue sur les deux fronts et &eacute;valu&eacute;e uniquement par la dur&eacute;e qui les s&eacute;pare&nbsp;: les deux polarit&eacute;s de signal fonctionnent sans aucun r&eacute;glage.</li>
<li>Les impulsions sont plac&eacute;es sur une grille d&#39;une seconde&nbsp;: une seconde mal re&ccedil;ue ne laisse un trou qu&#39;&agrave; sa propre place, sans d&eacute;caler les bits suivants.</li>
<li>La marque de minute (la 59e seconde, la seule sans impulsion) est d&eacute;termin&eacute;e statistiquement sur plusieurs minutes plut&ocirc;t qu&#39;&agrave; partir d&#39;une seule pause de deux secondes - c&#39;est ce qui rend la r&eacute;ception possible lorsque des secondes manquent.</li>
<li>Les bits manquants sont reconstruits l&agrave; o&ugrave; le protocole le permet (bits fixes, paire heure d&#39;&eacute;t&eacute;/hiver, un bit par groupe de parit&eacute;). Un tel t&eacute;l&eacute;gramme n&#39;est accept&eacute; que s&#39;il correspond au pr&eacute;c&eacute;dent plus les minutes &eacute;coul&eacute;es.</li>
<li>Un t&eacute;l&eacute;gramme n&#39;est accept&eacute; que si les bits fixes, les trois parit&eacute;s et les plages de valeurs concordent&nbsp;: un t&eacute;l&eacute;gramme perturb&eacute; ne peut donc pas r&eacute;gler une heure fausse.</li>
<li>La page /dcf77 affiche la progression des bits, le dernier t&eacute;l&eacute;gramme d&eacute;cod&eacute; et des valeurs de diagnostic jusqu&#39;aux dur&eacute;es d&#39;impulsion brutes.</li>
</ul>

<h2>7. Interface web</h2>
<ul>
<li>Interface sombre&nbsp;: &Eacute;tat, WLAN, r&eacute;glages de l&#39;horloge, luminosit&eacute;, heure/NTP et journal sont des onglets d&#39;une m&ecirc;me page.</li>
<li>Pages s&eacute;par&eacute;es pour les pr&eacute;r&eacute;glages, le cadran, les aiguilles, le gestionnaire de fichiers, l&#39;aper&ccedil;u en direct, DCF77 et la r&eacute;initialisation d&#39;usine.</li>
<li>Interface multilingue&nbsp;: allemand, anglais, fran&ccedil;ais.</li>
<li>Barre d&#39;&eacute;tat avec l&#39;heure et des points d&#39;&eacute;tat pour l&#39;heure, le RTC, DCF77 et la lumi&egrave;re ambiante, actualis&eacute;s toutes les quelques secondes.</li>
<li>D&#39;autres cadrans, jeux d&#39;aiguilles et pr&eacute;r&eacute;glages peuvent &ecirc;tre t&eacute;l&eacute;charg&eacute;s depuis GitHub&nbsp;; les d&eacute;pendances manquantes sont r&eacute;cup&eacute;r&eacute;es automatiquement.</li>
<li>R&eacute;initialisation d&#39;usine avec options s&eacute;par&eacute;es&nbsp;: tout, cadrans, jeux d&#39;aiguilles, pr&eacute;r&eacute;glages ou r&eacute;seaux WiFi enregistr&eacute;s.</li>
</ul>

<h2>8. Gestion des fichiers avec LittleFS</h2>
<ul>
<li>Les cadrans et les aiguilles sont stock&eacute;s compress&eacute;s en RLE pour &eacute;conomiser la m&eacute;moire flash.</li>
<li>Envoi, t&eacute;l&eacute;chargement, renommage et suppression via un gestionnaire de fichiers compact&nbsp;; l&#39;envoi, le renommage et la suppression ne sont possibles que depuis un r&eacute;seau priv&eacute;.</li>
<li>Journalisation optionnelle dans jusqu&#39;&agrave; 9 fichiers rotatifs, consultables en direct dans l&#39;onglet Journal.</li>
</ul>

<h2>9. Fuseaux horaires</h2>
<ul>
<li>Heure d&#39;&eacute;t&eacute; automatique (par ex. CET/CEST) ou heure d&#39;&eacute;t&eacute; / d&#39;hiver permanente.</li>
<li>Des cha&icirc;nes de fuseau horaire personnalis&eacute;es peuvent &ecirc;tre saisies directement.</li>
<li>Si aucun fuseau horaire n&#39;est renseign&eacute;, ou si la valeur saisie n&#39;est pas une cha&icirc;ne POSIX-TZ valide, l&#39;horloge revient automatiquement &agrave; CET-1CEST,M3.5.0,M10.5.0/3 (heure d&#39;Europe centrale).</li>
</ul>

<h2>10. Autres fonctions</h2>
<ul>
<li>Jusqu&#39;&agrave; 50 pr&eacute;r&eacute;glages (cadran, aiguilles, couleur et taille du moyeu, trotteuse) - renommables, supprimables (uniquement depuis un r&eacute;seau priv&eacute;), tri&eacute;s alphab&eacute;tiquement, avec sauvegarde et restauration.</li>
<li>Supprimer un cadran ou un jeu d&#39;aiguilles supprime les pr&eacute;r&eacute;glages qui s&#39;y r&eacute;f&egrave;rent&nbsp;; pour le jeu actif, le d&eacute;faut reprend la main.</li>
<li>Les fichiers BMP envoy&eacute;s peuvent &ecirc;tre redimensionn&eacute;s &agrave; la taille de l&#39;&eacute;cran.</li>
<li>Affichage de la dur&eacute;e de fonctionnement, red&eacute;marrage depuis l&#39;interface web (uniquement depuis un r&eacute;seau priv&eacute;), red&eacute;marrage pr&eacute;ventif hebdomadaire.</li>
<li>Interface API pour modifier les r&eacute;glages depuis l&#39;ext&eacute;rieur.</li>
<li>Mat&eacute;riel&nbsp;: ESP32-S2, photor&eacute;sistance pour la luminosit&eacute;, horloge temps r&eacute;el DS3231 en option, module de r&eacute;ception DCF77.</li>
</ul>

<h2>11. Heure mod&egrave;le Rocrail</h2>
<ul>
<li>Connexion optionnelle &agrave; un serveur Rocrail (logiciel de commande de train miniature)&nbsp;: les aiguilles peuvent afficher son &laquo;&nbsp;fast clock&nbsp;&raquo; (heure du mod&egrave;le) au lieu de l&#39;heure r&eacute;elle, activ&eacute;e via un interrupteur sur l&#39;onglet Heure.</li>
<li>Activer Rocrail, enregistrer l&#39;adresse du serveur ou red&eacute;marrer l&#39;horloge d&eacute;clenchent chacun une tentative de connexion imm&eacute;diate, au lieu d&#39;attendre la nouvelle tentative toutes les minutes.</li>
<li>L&#39;heure du mod&egrave;le s&#39;&eacute;coule selon le facteur d&#39;acc&eacute;l&eacute;ration propre &agrave; Rocrail (le &laquo;&nbsp;diviseur&nbsp;&raquo;) plut&ocirc;t que 1:1&nbsp;; l&#39;horloge continue d&#39;avancer seule entre deux mises &agrave; jour du serveur et revient &agrave; l&#39;heure r&eacute;elle si plus aucune mise &agrave; jour n&#39;arrive pendant plus de 2 minutes.</li>
<li>La trotteuse de l&#39;horloge de gare acc&eacute;l&egrave;re proportionnellement au diviseur au lieu de se d&eacute;sactiver, et se masque au-del&agrave; d&#39;un seuil de diviseur r&eacute;glable, car elle ne serait plus lisible.</li>
<li>Si le serveur indique aussi une valeur de luminosit&eacute;, l&#39;&eacute;cran reprend cette luminosit&eacute; au lieu de la photor&eacute;sistance/plage horaire, et revient automatiquement d&egrave;s que la connexion est coup&eacute;e.</li>
<li>La page Aper&ccedil;u refl&egrave;te la m&ecirc;me heure mod&egrave;le et la m&ecirc;me vitesse de diviseur.</li>
<li>L&#39;onglet Rocrail affiche l&#39;&eacute;tat de connexion en direct, le diviseur, le nom du plan et l&#39;heure mod&egrave;le actuelle.</li>
</ul>
)rawliteral";


    // Liefert den README-Block zur aktuell eingestellten Sprache; faellt auf
    // Englisch zurueck, wenn fuer eine Sprache keine Fassung vorliegt.

    // Returns the README block for the currently selected language; falls back
    // to English if no version exists for a language.

    static const char* readmeHtmlForCurrentLanguage() {
        if (currentLanguage == "de") return README_HTML_DE;
        if (currentLanguage == "fr") return README_HTML_FR;
        return README_HTML_EN;
    }
