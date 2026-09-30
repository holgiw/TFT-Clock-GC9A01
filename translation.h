#ifndef TRANSLATION_H
#define TRANSLATION_H


    // Uebersetzungen liegen als Tabelle direkt im Flash statt in einer std::map (deren Initialisierung
    // verursachte einen Stack-Overflow). availableLanguages dient nur der Pruefung in /setLanguage.

    // Translations live as a table directly in flash instead of a std::map (its initialization caused a stack
    // overflow). availableLanguages is only used for validation in /setLanguage.

    const std::set<String> availableLanguages = {"de"};


    // Uebersetzungstabelle (Erklaerung siehe Dateianfang)
    // Translation table (see top of file for explanation)

    struct TranslationEntry {
        const char* key;
        const char* de;
    };

    static const TranslationEntry translationTable[] = {
        { "Main", "Start" },
        { "Connected to", "Verbunden mit" },
        { "Connecting to", "Verbinde mit" },
        { "Connection failed", "Verbindung fehlgeschlagen" },
        { "Not connected", "Nicht verbunden" },
        { "IP Address", "IP Adresse" },
        { "Password", "Passwort" },
        { "WiFi Settings", "WLAN Einstellungen" },
        { "Save Settings", "Einstellungen speichern" },
        { "Scan", "Scannen" },
        { "Refresh", "Aktualisieren" },
        { "Signal Strength", "Signalst&auml;rke" },
        { "Encryption", "Verschl&uuml;sselung" },
        { "Open", "Offen" },
        { "Secured", "Gesichert" },
        { "Manage", "Verwalten" },
        { "No WiFi network configured yet, or the last known network is unavailable - the clock created its own WiFi network. Enter your home WiFi details below, save, and the clock will restart and try to connect", "Es ist noch kein WLAN eingerichtet, oder das zuletzt bekannte WLAN ist gerade nicht erreichbar - die Uhr hat ein eigenes WLAN erstellt. Trage unten dein Heim-WLAN ein, speichere, und die Uhr startet neu und versucht sich zu verbinden" },
        { "Syncing", "Synchronisiere" },
        { "Error", "Fehler" },

        // Bewusst OHNE Umlaut-Entity (&uuml;): wird vom Live-Status-Skript
        // per JS direkt in "title"/"aria-label" geschrieben (setStatusDot()
        // in generateTopBar()) - JS dekodiert Entities dabei nicht.

        // Deliberately WITHOUT the umlaut entity (&uuml;): written into
        // "title"/"aria-label" directly via JS by the live-status script
        // (setStatusDot() in generateTopBar()) - JS doesn't decode entities there.

        { "Not available", "Nicht verfuegbar" },
        { "Connection lost", "Verbindung unterbrochen" },

        // Kurzes Label fuer den Live-Helligkeitswert in der Topbar -
        // bewusst kurz wie die anderen Topbar-Labels, anders als das
        // ausfuehrlichere "Light (for Threshold)" weiter unten.

        // Short label for the live brightness value in the topbar -
        // deliberately kept short like the other topbar labels, unlike the
        // more verbose "Light (for Threshold)" further below.

        { "Light", "Licht" },
        { "Storage used", "Speicher belegt" },
        { "Presets used", "Presets belegt" },
        { "Free", "Frei" },
        { "Not applicable to this file type", "Nicht anwendbar auf diesen Dateityp" },
        { "Test", "Testen" },
        { "Testing", "Teste" },
        { "Server not reachable", "Server nicht erreichbar" },
        { "The second hand completes its lap in about 58.5 seconds and then waits at 60 until the minute changes, like a classic train station clock", "Der Sekundenzeiger legt seine Runde in rund 58,5 Sekunden zur&uuml;ck und wartet dann auf der 60, bis die Minute wechselt, wie bei einer klassischen Bahnhofsuhr" },
        { "Shows or hides the second hand on the clock face", "Zeigt oder verbirgt den Sekundenzeiger auf dem Zifferblatt" },
        { "The minute hand moves smoothly instead of jumping in 1-minute steps", "Der Minutenzeiger bewegt sich gleichm&auml;&szlig;ig, statt in 1-Minuten-Schritten zu springen" },
        { "The second hand moves smoothly instead of jumping in 1-second steps", "Der Sekundenzeiger bewegt sich gleichm&auml;&szlig;ig, statt in 1-Sekunden-Schritten zu springen" },
        { "Automatically tries to reconnect if the WiFi connection is lost", "Versucht automatisch, die WLAN-Verbindung bei Verbindungsverlust wiederherzustellen" },
        { "Writes up to 9 log files to LittleFS for troubleshooting", "Schreibt bis zu 9 Logdateien zur Fehlersuche in LittleFS" },
        { "DCF77 Sync LED Blink", "DCF77-Sync-LED-Blinken" },
        { "Flashes the LED for every received DCF77 pulse while the clock is still acquiring the time signal", "L&auml;sst die LED bei jedem empfangenen DCF77-Impuls aufblitzen, solange die Uhr das Zeitsignal noch sucht" },
        { "Rotates the clock face by the selected number of degrees, useful if the display is mounted rotated in its housing", "Dreht das Zifferblatt um den gew&auml;hlten Winkel - n&uuml;tzlich, falls das Display gedreht im Geh&auml;use verbaut ist" },
        { "Rotates Display 2's (CS2) clock face independently of Display 1", "Dreht das Zifferblatt von Display 2 (CS2) unabh&auml;ngig von Display 1" },
        { "not connected (n.a.)", "nicht angeschlossen (n.a.)" },
        { "Set a display that is not physically connected to n.a. - it then stays black and the clock face is neither drawn nor calculated for it. Boot, access point and code messages still appear on both displays until the clock takes over", "Ein nicht tats&auml;chlich angeschlossenes Display auf n.a. stellen - es bleibt dann schwarz und das Zifferblatt wird daf&uuml;r weder gezeichnet noch berechnet. Start-, Access-Point- und Code-Meldungen erscheinen bis zum Uhrstart weiterhin auf beiden Displays" },

        // "2 Minuten" statt vormals "3 Minuten": muss zum tatsaechlichen WPS-
        // Timeout passen (2 * WAIT_1m in startAP()/loop(), siehe uhr4.ino/
        // wifi_manager.h) - der Text war seit dessen Aenderung veraltet.

        // "2 minutes" instead of the former "3 minutes": must match the actual
        // WPS timeout (2 * WAIT_1m in startAP()/loop(), see uhr4.ino/
        // wifi_manager.h) - the text was stale since that value was changed.

        { "Adds a new network via WPS - press the WPS button on your router when prompted. The clock's connection may be lost for about 2 minutes while this happens", "F&uuml;gt ein neues Netzwerk per WPS hinzu - dr&uuml;cken Sie bei Aufforderung die WPS-Taste an Ihrem Router. Die Verbindung zur Uhr kann dabei f&uuml;r ca. 2 Minuten verloren gehen" },
        { "Scans for available WiFi networks again and refreshes the dropdown lists below", "Sucht erneut nach verf&uuml;gbaren WLAN-Netzwerken und aktualisiert die Auswahllisten darunter" },
        { "The clock can also be reached at http://&quot;hostname&quot;.local instead of its IP address, e.g.", "Die Uhr ist statt &uuml;ber die IP-Adresse auch &uuml;ber http://&quot;hostname&quot;.local erreichbar, z.B." },
        { "Up to", "Bis zu" },
        { "WiFi networks can be stored", "WLAN-Netzwerke k&ouml;nnen hinterlegt werden" },
        { "A restart is required for a changed hostname to take effect. Not all routers support hostname resolution", "Ein Neustart ist erforderlich, damit ein ge&auml;nderter Hostname wirksam wird. Nicht alle Router unterst&uuml;tzen die Aufl&ouml;sung von Hostnamen" },
        { "Automatically adjusts brightness based on ambient light measured by the photoresistor", "Passt die Helligkeit automatisch anhand des vom Fotowiderstand gemessenen Umgebungslichts an" },
        { "Reverses the brightness sensor reading - use if the display gets darker in bright light instead of brighter", "Kehrt den Messwert des Helligkeitssensors um - verwenden, falls das Display bei hellem Licht dunkler statt heller wird" },
        { "Below this ambient light percentage, the display uses minimum brightness", "Unterhalb dieses Umgebungslicht-Prozentwerts verwendet das Display die Mindesthelligkeit" },
        { "Above this ambient light percentage, the display uses maximum brightness", "Oberhalb dieses Umgebungslicht-Prozentwerts verwendet das Display die Maximalhelligkeit" },
        { "Display brightness used at or below the low threshold", "Displayhelligkeit, die bei oder unterhalb der unteren Schwelle verwendet wird" },
        { "Display brightness used at or above the high threshold", "Displayhelligkeit, die bei oder oberhalb der oberen Schwelle verwendet wird" },
        { "Start of the daily time window during which the display always uses full brightness, regardless of ambient light", "Beginn des t&auml;glichen Zeitfensters, in dem das Display unabh&auml;ngig vom Umgebungslicht immer volle Helligkeit nutzt" },
        { "End of the daily time window during which the display always uses full brightness, regardless of ambient light", "Ende des t&auml;glichen Zeitfensters, in dem das Display unabh&auml;ngig vom Umgebungslicht immer volle Helligkeit nutzt" },
        { "Adjusts how brightness ramps between minimum and maximum - higher values keep the display darker for longer before brightening", "Passt an, wie die Helligkeit zwischen Minimum und Maximum ansteigt - h&ouml;here Werte halten das Display l&auml;nger dunkler, bevor es heller wird" },
        { "Total", "Gesamt" },
        { "Used", "Belegt" },
        { "Save", "speichern" },
        { "Brightness", "Helligkeit" },
        { "Language", "Sprache" },
        { "Timezone", "Zeitzone" },
        { "Handset", "Zeigersatz" },
        { "Background", "Hintergrund" },
        { "Station Mode", "Station Modus" },
        { "Smooth Minute Hand", "Sanfter Minutenzeiger" },
        { "Smooth Second Hand", "Sanfter Sekundenzeiger" },
        { "Min Brightness", "Minimale Helligkeit" },
        { "Max Brightness", "Maximale Helligkeit" },
        { "Rebooting...", "Starte neu..." },
        { "Reboot", "Neustart" },
        { "Factory&nbsp;Reset", "Werkseinstellungen" },
        { "File&nbsp;Manager", "Dateimanager" },
        { "Upload", "Hochladen" },
        { "Delete", "L&ouml;schen" },
        { "active", "aktiv" },
        { "pixels", "Pixel" },
        { "Backup / Restore Presets", "Presets sichern / wiederherstellen" },
        { "Save Presets to File", "Presets als Datei speichern" },
        { "Load Presets from File", "Presets aus Datei laden" },
        { "Presets imported successfully", "Presets erfolgreich importiert" },
        { "Import failed - please check the file", "Import fehlgeschlagen - bitte Datei pr&uuml;fen" },
        { "File deleted", "Datei gel&ouml;scht" },
        { "Files deleted", "Dateien gel&ouml;scht" },
        { "No files selected", "Keine Dateien ausgew&auml;hlt" },
        { "Delete selected", "Ausgew&auml;hlte l&ouml;schen" },
        { "Delete the selected files?", "Die ausgew&auml;hlten Dateien l&ouml;schen?" },
        { "Select all", "Alle ausw&auml;hlen" },
        { "Modified", "Ge&auml;ndert" },
        { "Clock faces", "Zifferbl&auml;tter" },
        { "Strip graphics", "Streifen-Grafiken" },
        { "Hand sets", "Zeiger" },
        { "Fonts", "Schriften" },
        { "Log files", "Logdateien" },
        { "Other files", "Sonstige Dateien" },
        { "rename", "Umbenennen" },
        { "Disconnected", "Getrennt" },
        { "Settings", "Einstellungen" },
        { "settings", "Einstellungen" },
        { "save", "Speichern" },
        { "brightness", "Helligkeit" },
        { "timezone", "Zeitzone" },
        { "NTP&nbsp;Timezone", "NTP&nbsp;Zeitzone" },
        { "Are you sure you want to delete", "M&ouml;chten Sie wirklich l&ouml;schen" },
        { "Cancel", "Abbrechen" },
        { "Confirm", "Best&auml;tigen" },
        { "Clock&nbsp;Face", "Zifferblatt" },
        { "Hand&nbsp;Set", "Zeiger" },
        { "Presets", "Uhren Sets" },
        { "Default", "Standard" },
        { "Rescan Networks", "WLan Netzwerke neu scannen" },
        { "Connect", "Verbinden" },
        { "Password is hidden. Leave empty to keep current", "Das Passwort ist ausgeblendet. Lassen Sie das Feld leer, um das aktuelle Passwort beizubehalten" },
        { "You can also enter an SSID manually", "Sie k&ouml;nnen auch eine SSID manuell eingeben" },

        // Platzhalter in den SSID-Auswahllisten, solange /api/scanwifi laeuft
        // bzw. wenn die Abfrage fehlschlaegt (siehe panel-wlan).

        // Placeholders in the SSID dropdowns while /api/scanwifi is running
        // resp. when the request fails (see panel-wlan).

        { "WLAN scan in progress", "WLAN-Scan l&auml;uft" },
        { "Scan failed", "Scan fehlgeschlagen" },
        { "Save WiFi settings", "WLAN Einstellungen speichern" },
        { "Train Station Mode", "Bahnhof Modus" },
        { "Show Seconds", "Sekundenzeiger anzeigen" },
        { "Apply", "Anwenden" },
        { "select network", "Netzwerk ausw&auml;hlen" },
        { "Manage Clock Face Files", "Zifferbl&auml;tter verwalten" },
        { "Rename", "Umbenennen" },
        { "New Name", "Neuer Name" },
        { "Download Additional Clock Faces", "Zus&auml;tzliche Zifferbl&auml;tter herunterladen" },
        { "You can download a ZIP file containing additional clock faces and hand sets from the following link: (use 'view raw')", "Sie k&ouml;nnen eine ZIP-Datei mit zus&auml;tzlichen Zifferbl&auml;ttern und Zeigers&auml;tzen von folgendem Link herunterladen: (verwenden Sie &#39;view raw&#39;)" },
        { "After downloading, upload the extracted BMP files using the form below", "Nach dem Herunterladen k&ouml;nnen Sie die extrahierten BMP-Dateien mit dem untenstehenden Formular hochladen" },
        { "Upload New Clock Face", "neue Zifferbl&auml;tter hochladen" },
        { "Requirements", "Anforderungen" },
        { "instead of the IP address for better reliability", "anstelle der IP Adresse f&uuml;r bessere Erreichbarkeit" },
        { "Clock Setup", "Uhr Einstellungen" },
        { "name must start with", "Name muss beginnen mit" },
        { "Manage Clock Hand Sets", "Zeigers&auml;tze verwalten" },
        { "Upload New Hand Set", "Neuen Zeigersatz hochladen" },
        { "Hand Designer", "Zeiger-Designer" },
        { "Clock Face Designer", "Zifferblatt-Designer" },
        { "old format", "altes Format" },
        { "new format", "neues Format" },
        { "Size (pixels)", "Gr&ouml;&szlig;e (Pixel)" },
        { "File", "Datei" },
        { "Uploading... please wait", "Wird hochgeladen... bitte warten" },
        { "no.", "Nr." },
        { "e.g.", "z. B." },
        { "new width", "neue Breite" },
        { "new length", "neue L&auml;nge" },
        { "unsupported size - default hand is used", "ung&uuml;ltige Gr&ouml;&szlig;e - Standardzeiger wird verwendet" },
        { "Hands: keep the current size, other sizes distort the hand and shift the pivot", "Zeiger: aktuelle Gr&ouml;&szlig;e beibehalten, andere Gr&ouml;&szlig;en verzerren den Zeiger und verschieben den Drehpunkt" },
        { "Upload to Set", "Set hochladen" },
        { "Color (RGB hex, e.g. FF0000 = Red, 000000 = Black, EC0016 = DB red)", "Farbe (RGB hex, z.B. FF0000 = Rot, 000000 = Schwarz, EC0016 = DB rot)" },
        { "Centre point", "Mittelpunkt" },
        { "Size", "Gr&ouml;&szlig;e" },
        { "Warning: Not enough free space to upload new hand sets! Free up some space first", "Warnung: Nicht gen&uuml;gend Speicherplatz zum Hochladen neuer Zeiger! Bitte zuerst Speicherplatz freigeben" },
        { "Pivot point", "Drehpunkt bei" },
        { "Manage Presets", "Uhren Sets verwalten" },
        { "Create New Preset", "Erstelle neues Set" },
        { "Create Preset from Current Settings", "Erzeuge ein Set aus den aktuellen Einstellungen" },
        { "For custom timezones, select a preset or enter your own value above", "F&uuml;r benutzerdefinierte Zeitzonen w&auml;hlen Sie eine Voreinstellung aus oder geben Sie oben Ihren eigenen Wert ein" },
        { "Save Timezone", "Zeitzone speichern" },
        { "NTP Server / Timezone (DST String)", "NTP Server / Zeitzone (DST)" },
        { "Low Threshold", "untere Helligkeitsschwelle" },
        { "High Threshold", "obere Helligkeitsschwelle" },
        { "Full brightness from (hour, 0-23)", "volle Helligkeit ab Stunde (0-23)" },
        { "Full brightness until (hour, 0-23)", "volle Helligkeit bis Stunde (0-23)" },
        { "Current ADC Value", "aktueller ADC Wert" },
        { "Current Brightness", "aktuelle Helligkeit" },
        { "Light (for Threshold)", "Licht (f&uuml;r Helligkeitsschwelle)" },
        { "Brightness is currently taken over from Rocrail - the photoresistor and time-window settings below are inactive while connected", "Die Helligkeit wird derzeit von Rocrail &uuml;bernommen - die Fotowiderstand- und Zeitfenster-Einstellungen unten sind w&auml;hrend der Verbindung inaktiv" },
        { "Brightness Settings", "Helligkeit Einstellungen" },
        { "Enable Auto Brightness", "automatische Einstellung" },
        { "Invert ADC Reading", "invertiere ADC Werte" },
        { "All Files on LittleFS", "alle Dateien im FileSystem" },
        { "Scale and Save BMP", "skaliere und speichere das BMP" },
        { "Filename", "Dateiname" },
        { "Preview", "Vorschau" },
        { "Preview Size", "Vorschaugr&ouml;&szlig;e" },
        { "No BMP files found in /", "Keine BMP-Dateien im Dateisystem gefunden" },
        { "Size(bytes)", "Gr&ouml;&szlig;e (Bytes)" },
        { "Action", "Aktion" },
        { "Scale", "skalieren" },
        { "Source", "Quelle" },
        { "Target", "Ziel" },
        { "Width", "Breite" },
        { "Height", "H&ouml;he" },
        { "Scale and Save", "skalieren und speichern" },
        { "Are you sure you want to reboot?", "Sind Sie sicher, das Sie die Uhr neu starten wollen?" },
        { "Are you sure you want to reset to factory settings?", "Sind Sie absolut sicher, das Sie die Uhr auf Werkseinstellung setzen wollen?" },
        { "A 3-digit code now appears on the clock's display. Enter it below to confirm - this cannot be undone", "Auf dem Display der Uhr erscheint jetzt ein 3-stelliger Code. Zur Best&auml;tigung unten eingeben - dies kann nicht r&uuml;ckg&auml;ngig gemacht werden" },
        { "Confirm Reset", "Zur&uuml;cksetzen best&auml;tigen" },
        { "Reset Everything", "Alles zur&uuml;cksetzen" },
        { "Resets WiFi, all settings and deletes all files - the clock restarts afterwards", "Setzt WLAN, alle Einstellungen zur&uuml;ck und l&ouml;scht alle Dateien - die Uhr startet danach neu" },
        { "Deletes all saved WiFi networks - other settings remain unchanged", "L&ouml;scht alle gespeicherten WLAN-Netzwerke - andere Einstellungen bleiben unver&auml;ndert" },
        { "Delete Clock Faces (except default)", "Zifferbl&auml;tter l&ouml;schen (au&szlig;er Standard)" },
        { "Deletes all uploaded clock faces - the built-in default remains", "L&ouml;scht alle hochgeladenen Zifferbl&auml;tter - der eingebaute Standard bleibt erhalten" },
        { "Delete Hand Sets (except default)", "Zeigers&auml;tze l&ouml;schen (au&szlig;er Standard)" },
        { "Deletes all uploaded hand sets - the built-in default remains", "L&ouml;scht alle hochgeladenen Zeigers&auml;tze - der eingebaute Standard bleibt erhalten" },
        { "Delete Presets", "Uhren Sets l&ouml;schen" },
        { "Deletes all saved presets", "L&ouml;scht alle gespeicherten Uhren Sets" },
        { "Are you sure you want to delete all clock faces except the default one?", "Sind Sie sicher, dass Sie alle Zifferbl&auml;tter au&szlig;er dem Standard l&ouml;schen wollen?" },
        { "Are you sure you want to delete all hand sets except the default one?", "Sind Sie sicher, dass Sie alle Zeigers&auml;tze au&szlig;er dem Standard l&ouml;schen wollen?" },
        { "Are you sure you want to delete all presets?", "Sind Sie sicher, dass Sie alle Uhren Sets l&ouml;schen wollen?" },
        { "Clock faces deleted", "Zifferbl&auml;tter gel&ouml;scht" },
        { "Hand sets deleted", "Zeigers&auml;tze gel&ouml;scht" },
        { "Presets deleted", "Presets gel&ouml;scht" },
        { "Code expired, please try again", "Code abgelaufen, bitte erneut versuchen" },
        { "Wrong code, please try again", "Falscher Code, bitte erneut versuchen" },
        { "Too many wrong attempts, please try again", "Zu viele Fehlversuche, bitte erneut versuchen" },
        { "Status information is only shown when accessing the clock from a private network", "Statusinformationen werden nur angezeigt, wenn auf die Uhr aus einem privaten Netzwerk zugegriffen wird" },
        { "This action is only available when accessing the clock from a private network", "Diese Aktion ist nur verf&uuml;gbar, wenn auf die Uhr aus einem privaten Netzwerk zugegriffen wird" },
        { "Another confirmation is already pending. Please complete it or wait a moment and try again", "Es steht bereits eine andere Best&auml;tigung aus. Bitte diese abschlie&szlig;en oder kurz warten und es erneut versuchen" },
        { "Network deleted", "Netzwerk gel&ouml;scht" },
        { "Rename File", "Datei umbenennen" },
        { "Rename Preset", "Preset umbenennen" },
        { "Maximum number of presets reached - delete an existing preset first", "Maximale Anzahl an Presets erreicht - bitte zuerst ein bestehendes Preset l&ouml;schen" },
        { "Copy link", "Link kopieren" },
        { "Download Additional Clock Faces from GitHub", "Weitere Zifferbl&auml;tter von GitHub laden" },
        { "Download Additional Hand Sets from GitHub", "Weitere Zeigers&auml;tze von GitHub laden" },
        { "Checking GitHub for new files", "Suche nach neuen Dateien auf GitHub" },
        { "Scanning for WiFi networks - the page will reload automatically in 10 seconds", "Suche nach WLAN-Netzwerken - die Seite l&auml;dt sich in 10 Sekunden automatisch neu" },
        { "All files already up to date", "Alle Dateien sind bereits aktuell" },
        { "All presets already up to date", "Alle Presets sind bereits aktuell" },
        { "No presets found. Load recommended presets from GitHub?", "Keine Presets vorhanden. Empfohlene Presets von GitHub laden?" },
        { "Downloading", "Lade herunter" },
        { "Converting", "Konvertiere" },
        { "Done - reloading", "Fertig - lade neu" },
        { "Failed to reach GitHub - check your internet connection", "GitHub nicht erreichbar - Internetverbindung pr&uuml;fen" },
        { "Load Presets from GitHub", "Presets von GitHub laden" },
        { "Language updated", "Sprache ge&auml;ndert" },
        { "Preset created", "Preset erstellt" },
        { "Preset applied", "Preset angewendet" },
        { "File renamed", "Datei umbenannt" },
        { "Clock face uploaded", "Zifferblatt hochgeladen" },
        { "Clock face selected", "Zifferblatt ausgew&auml;hlt" },
        { "Preset deleted", "Preset gel&ouml;scht" },
        { "Preset renamed", "Preset umbenannt" },
        { "Hand set uploaded", "Zeigersatz hochgeladen" },
        { "Hand set selected", "Zeigersatz ausgew&auml;hlt" },
        { "Hand set deleted", "Zeigersatz gel&ouml;scht" },
        { "Timezone updated", "Zeitzone aktualisiert" },
        { "Settings saved", "Einstellungen gespeichert" },
        { "Please connect to your home network and go to the ESP website at", "Bitte mit dem Heimnetzwerk verbinden und die ESP-Webseite aufrufen unter" },
        { "Time sync started", "Zeitsynchronisation gestartet" },
        { "Returning to main page in 3 seconds", "Zur&uuml;ck zur Hauptseite in 3 Sekunden" },
        { "System Status", "Systemstatus" },
        { "Upload failed", "Upload fehlgeschlagen" },
        { "Only .bmp files starting with", "Nur .bmp-Dateien, die beginnen mit" },
        { "or", "oder" },
        { "are accepted", "werden akzeptiert" },
        { "are accepted for handset upload", "werden f&uuml;r den Zeigersatz-Upload akzeptiert" },
        { "Please also check the available space", "Bitte auch den verf&uuml;gbaren Speicherplatz pr&uuml;fen" },
        { "Try again", "Erneut versuchen" },
        { "Return to the main page in 10 seconds or refresh the website when the ESP is online again", "Kehren Sie in 10 Sekunden zur Hauptseite zur&uuml;ck oder aktualisieren Sie die Website, wenn der ESP wieder online ist" },
        { "Enable Logging", "Logging aktivieren" },
        { "Log file", "Log-Datei" },
        { "Auto-refresh (10s)", "Automatisch aktualisieren (10s)" },
        { "Logging is disabled.", "Logging ist deaktiviert." },
        { "No log entries yet.", "Noch keine Log-Eintr&auml;ge vorhanden." },
        { "Log file could not be opened.", "Log-Datei konnte nicht ge&ouml;ffnet werden." },
        { "Loading&hellip;", "L&auml;dt&hellip;" },
        { "Refresh now", "Jetzt aktualisieren" },
        { "Refresh failed - showing last known content", "Aktualisierung fehlgeschlagen - zeige zuletzt bekannten Inhalt" },
        { "Reconnect WiFi", "Wifi neu verbinden" },
        { "JavaScript is disabled. This page requires JavaScript to work properly!", "JavaScript ist deaktiviert. Diese Seite ben&ouml;tigt JavaScript, um richtig zu funktionieren!" },
        { "Are you sure you want to reset all saved WiFi networks?", "Sind Sie sicher, dass Sie alle gespeicherten WLAN Netzwerke zur&uuml;cksetzen m&ouml;chten?" },
        { "DCF77 detected", "DCF77 erkannt" },
        { "Waiting", "warte" },
        { "Back", "Zur&uuml;ck" },
        { "Check RTC", "RTC pr&uuml;fen" },
        { "Download", "Herunterladen" },
        { "Failed to scale BMP", "Skalierung des BMP fehlgeschlagen" },
        { "Gamma Correction", "Gamma-Korrektur" },

        // Komplettsicherung (backup.h, /backup)
        // Full backup (backup.h, /backup)

        { "Backup", "Sicherung" },
        { "Create Backup", "Sicherung erstellen" },
        { "Saves all settings, presets, clock faces and hand sets in one file", "Speichert alle Einstellungen, Presets, Zifferbl&auml;tter und Zeigers&auml;tze in einer Datei" },
        { "Include WiFi credentials (network names, passwords, hostname)", "WLAN-Zugangsdaten einschlie&szlig;en (Netzwerknamen, Passw&ouml;rter, Hostname)" },
        { "Saving the WiFi credentials is not secure: they are encrypted in the file, but with a key that is the same in every uhr4 firmware - anyone with the firmware or its source code can decrypt them. Keep the file safe and do not pass it on", "Das Sichern der WLAN-Zugangsdaten ist nicht sicher: Sie stehen zwar verschl&uuml;sselt in der Datei, aber mit einem Schl&uuml;ssel, der in jeder uhr4-Firmware gleich ist - wer Firmware oder Quelltext hat, kann sie entschl&uuml;sseln. Datei sicher aufbewahren und nicht weitergeben" },
        { "Restore WiFi credentials (network names, passwords, hostname)", "WLAN-Zugangsdaten wiederherstellen (Netzwerknamen, Passw&ouml;rter, Hostname)" },
        { "Without this option the clock keeps its own WiFi and hostname - recommended when transferring the settings to another clock", "Ohne diese Option beh&auml;lt die Uhr ihr eigenes WLAN und ihren Hostnamen - empfohlen, wenn die Einstellungen auf eine andere Uhr &uuml;bertragen werden" },
        { "Display type, rotation, backlight and light sensor of this clock stay unchanged. The backup must come from a clock with the same clock size", "Displaytyp, Rotation, Hintergrundbeleuchtung und Lichtsensor dieser Uhr bleiben unver&auml;ndert. Die Sicherung muss von einer Uhr mit gleicher Uhrgr&ouml;&szlig;e stammen" },
        { "Nothing was changed on the clock", "An der Uhr wurde nichts ge&auml;ndert" },
        { "Download Backup", "Sicherung herunterladen" },
        { "Restore Backup", "Sicherung wiederherstellen" },
        { "Replaces all settings, presets, clock faces and hand sets with the contents of the backup - the clock restarts afterwards", "Ersetzt alle Einstellungen, Presets, Zifferbl&auml;tter und Zeigers&auml;tze durch den Inhalt der Sicherung - die Uhr startet danach neu" },

        // Steht in einem JavaScript-confirm() mit einfachen Anfuehrungszeichen - daher ohne Apostroph
        // Used inside a JavaScript confirm() with single quotes - hence no apostrophe

        { "Replace all current settings, clock faces and hand sets with the backup?", "Alle aktuellen Einstellungen, Zifferbl&auml;tter und Zeigers&auml;tze durch die Sicherung ersetzen?" },
        { "The backup could not be restored", "Die Sicherung konnte nicht wiederhergestellt werden" },
        { "Backup restored", "Sicherung wiederhergestellt" },
        { "This backup is from a {b} clock, this clock is a {c}", "Diese Sicherung stammt von einer {b}-Uhr, diese Uhr ist eine {c}" },
        { "Clock faces and hands fit (same clock size); display type, rotation and backlight of this clock stay unchanged", "Zifferbl&auml;tter und Zeiger passen (gleiche Uhrgr&ouml;&szlig;e); Displaytyp, Rotation und Hintergrundbeleuchtung dieser Uhr bleiben unver&auml;ndert" },
        { "The clock size differs - the clock will reject the backup", "Die Uhrgr&ouml;&szlig;e ist anders - die Uhr wird die Sicherung ablehnen" },
        { "The file name says {n}", "Der Dateiname nennt {n}" },
        { "Restore anyway?", "Trotzdem wiederherstellen?" },
        { "The backup is from a {b} clock - the display type of this clock ({c}) was kept", "Die Sicherung stammt von einer {b}-Uhr - der Displaytyp dieser Uhr ({c}) bleibt" },
        { "files", "Dateien" },

        { "Display type", "Display-Typ" },

        // Steht in einem JavaScript-confirm() mit einfachen Anfuehrungszeichen - daher ohne Apostroph
        // Used inside a JavaScript confirm() with single quotes - hence no apostrophe

        { "The clock restarts to apply the display type", "Die Uhr startet neu, um den Display-Typ zu &uuml;bernehmen" },
        { "Change the display type to", "Display-Typ &auml;ndern auf" },
        { "Type of the connected display - applies to both displays. BL = backlight: with BL the brightness is controlled via PWM on pin 3 (same as the backlight checkbox in the brightness tab), without BL by darkening the pixels. Switching to another display type restarts the clock and resets backlight, brightness and hub size to the defaults of the new type; uploaded clock faces and hands only fit the size they were made for (GC9A01 and ILI9341 share the 240 size)", "Typ des angeschlossenen Displays - gilt f&uuml;r beide Displays. BL = Hintergrundbeleuchtung: mit BL wird die Helligkeit per PWM an Pin 3 geregelt (wie der Haken Hintergrundbeleuchtung im Helligkeits-Tab), ohne BL durch Abdunkeln der Pixel. Beim Wechsel auf einen anderen Displaytyp startet die Uhr neu, Hintergrundbeleuchtung, Helligkeit und Nabengr&ouml;&szlig;e werden auf die Standardwerte des neuen Typs gesetzt; hochgeladene Zifferbl&auml;tter und Zeiger passen nur zu der Gr&ouml;&szlig;e, f&uuml;r die sie erstellt wurden (GC9A01 und ILI9341 teilen sich die 240er Gr&ouml;&szlig;e)" },
        { "ILI9341 (240x320) with time and date below the clock", "ILI9341 (240x320) mit Uhrzeit und Datum unter der Uhr" },
        { "GC9A01 (240x240) without backlight (BL)", "GC9A01 (240x240) ohne Hintergrundbeleuchtung (BL)" },
        { "GC9A01 (240x240) with backlight (BL) on pin 3", "GC9A01 (240x240) mit Hintergrundbeleuchtung (BL) an Pin 3" },
        { "Backlight control (pin 3)", "Hintergrundbeleuchtung regeln (Pin 3)" },
        { "Dims the display via the backlight PWM on pin 3 instead of darkening the pixels - only if the backlight is wired to pin 3 (always on GC9D01). Switching resets min. brightness and thresholds to the matching defaults", "Dimmt das Display &uuml;ber die PWM der Hintergrundbeleuchtung an Pin 3 statt die Pixel abzudunkeln - nur wenn die Beleuchtung an Pin 3 angeschlossen ist (beim GC9D01 immer). Beim Umschalten werden min. Helligkeit und Schwellwerte auf die passenden Standardwerte gesetzt" },

        // Diagrammtitel der Gamma-Kurve - wird per decodeHtml() an Plotly
        // uebergeben, Entities sind hier also erlaubt (siehe plotGamma()).

        // Chart title of the gamma curve - passed to Plotly via decodeHtml(),
        // so entities are fine here (see plotGamma()).

        { "Gamma correction curve", "Gamma-Korrektur-Kurve" },
        { "Hostname saved - requires a reboot to take effect", "Hostname gespeichert - Neustart erforderlich, damit die &Auml;nderung wirksam wird" },
        { "No valid hostname could be derived from the input - falling back to the automatic name based on the MAC address", "Aus der Eingabe konnte kein g&uuml;ltiger Hostname gebildet werden - R&uuml;ckfall auf den automatischen, aus der MAC-Adresse gebildeten Namen" },
        { "Reset Saved Networks", "Gespeicherte Netzwerke zur&uuml;cksetzen" },
        { "Delete Active WiFi", "Aktives WLAN l&ouml;schen" },
        { "Change Active WiFi", "Aktives WLAN &auml;ndern" },
        { "Add Network via WPS", "Netzwerk per WPS hinzuf&uuml;gen" },

        // Mit Trennungshinweis direkt als Flash-Meldung beim WPS-Start. Der englische Schluessel wird in
        // webserver_routes.h exakt verglichen (erkennt den WPS-Start) - bei einer Aenderung dieses Textes
        // dort mitaendern.

        // With the disconnect notice shown directly as a flash message when WPS starts. The English key is
        // compared exactly in webserver_routes.h (detects the WPS start) - if this text changes, update it
        // there too.

        { "WPS active - press the WPS button on your router now. Connection to the clock may be lost for about 2 minutes while this happens", "WPS aktiv - jetzt die WPS-Taste am Router dr&uuml;cken. Die Verbindung zur Uhr kann dabei f&uuml;r ca. 2 Minuten unterbrochen werden" },
        { "Reset WLan...", "WLAN zur&uuml;cksetzen..." },
        { "Saved as", "Gespeichert als" },
        { "Scaling successful", "Skalierung erfolgreich" },
        { "View", "Anzeigen" },
        { "Warning: Not enough free space to upload new clock faces! Free up some space first", "Warnung: Nicht gen&uuml;gend Speicherplatz zum Hochladen neuer Zifferbl&auml;tter! Bitte zuerst Speicherplatz freigeben" },
        { "Use the host name", "Benutze den Hostnamen" },

        // Neu fuer die Statuszeile (Topbar, siehe generateTopBar() in webserver_routes.h)
        // New for the status bar (topbar, see generateTopBar() in webserver_routes.h)

        { "Time", "Zeit" },

        // Neu fuer die /dcf77-Live-Seite (Bit-Fortschritt + dekodiertes
        // Telegramm, siehe webserver_routes.h)

        // New for the /dcf77 live page (bit progress + decoded telegram,
        // see webserver_routes.h)

        { "Bit progress", "Bit-Fortschritt" },
        { "Decoded telegram", "Dekodiertes Telegramm" },
        { "Waiting for first complete telegram", "Warte auf erstes vollst&auml;ndiges Telegramm" },
        { "Hour", "Stunde" },
        { "Day", "Tag" },
        { "Month", "Monat" },
        { "Year", "Jahr" },
        { "Weekday", "Wochentag" },
        { "Summer time", "Sommerzeit" },
        { "Winter time", "Winterzeit" },
        { "Call bit", "Anrufbit" },
        { "Parity", "Parit&auml;t" },
        { "Last decoded", "Zuletzt dekodiert" },
        { "Reconstructed bits", "Rekonstruierte Bits" },
        { "Dropped edges (buffer overflow)", "Verlorene Flanken (Puffer&uuml;berlauf)" },
        { "Telegram sync", "Telegramm-Synchronisation" },
        { "Pulses seen / missed / grid losses", "Impulse gesehen / fehlend / Rasterverluste" },
        { "Last pulses (width / gap in ms, expected ~100 or ~200 / ~n&times;1000)", "Letzte Impulse (Breite / Abstand in ms, erwartet ~100 oder ~200 / ~n&times;1000)" },
        { "Minute marker not identified yet - the boxes show raw grid positions, not telegram bit numbers.", "Minutenmarke noch nicht erkannt - die K&auml;stchen zeigen rohe Rasterpositionen, keine Telegramm-Bitnummern." },
        { "(DCF77: 1=Mon..7=Sun)", "(DCF77: 1=Mo..7=So)" },
        { "next", "n&auml;chstes" },
        { "lost", "verloren" },

        // Bit-Tooltips im /dcf77-Bitraster (title-Attribut je Kaestchen) -
        // Bit-Kennzahlen (BCD-Bit N, Paritaeten) werden im Code an den
        // uebersetzten Text angehaengt, nicht Teil des Schluessels.

        // Bit tooltips in the /dcf77 bit grid (title attribute per box) - bit
        // numbers (BCD bit N, parities) are appended to the translated text
        // in code, not part of the key.

        { "Start of minute (always 0)", "Minutenanfang (immer 0)" },
        { "Weather broadcast / special function (unused)", "Wetterdurchsage / Sonderfunktion (ungenutzt)" },
        { "DST change announcement", "Ank&uuml;ndigung Zeitumstellung" },
        { "Summer time (CEST) in effect", "Sommerzeit (MESZ) aktiv" },
        { "Winter time (CET) in effect", "Winterzeit (MEZ) aktiv" },
        { "Leap second announcement", "Ank&uuml;ndigung Schaltsekunde" },
        { "Start of time (always 1)", "Zeitanfang (immer 1)" },
        { "Minute BCD bit", "Minuten-BCD-Bit" },
        { "Minute parity", "Minuten-Parit&auml;t" },
        { "Hour BCD bit", "Stunden-BCD-Bit" },
        { "Hour parity", "Stunden-Parit&auml;t" },
        { "Day of month BCD bit", "Tag-BCD-Bit" },
        { "Day of week BCD bit", "Wochentag-BCD-Bit" },
        { "Month BCD bit", "Monats-BCD-Bit" },
        { "Year BCD bit", "Jahres-BCD-Bit" },
        { "Date parity (day+weekday+month+year)", "Datums-Parit&auml;t (Tag+Wochentag+Monat+Jahr)" },

        // Nur ASCII, OHNE HTML-Entities: per JS als .textContent gesetzt (dcfSynced-Anzeige), Entities
        // erschienen dort woertlich. {pos} wird per JS ersetzt.

        // ASCII only, WITHOUT HTML entities: set via JS as .textContent (dcfSynced display), entities would
        // show up literally there. {pos} is substituted in JS.

        { "yes (marker at grid position {pos})", "ja (Marke bei Rasterposition {pos})" },
        { "no (collecting - the minute marker needs a few minutes)", "nein (sammle - die Minutenmarke braucht ein paar Minuten)" },

        // Neu fuer den Rocrail-Tab (siehe webserver_routes.h und
        // rocrail_client.h) - Modellzeit-Anbindung an einen Rocrail-Server

        // New for the Rocrail tab (see webserver_routes.h and
        // rocrail_client.h) - model time connection to a Rocrail server

        { "Rocrail model time can run much faster than real time (the divider) - the station-clock second-hand animation speeds up by the same factor instead of switching off, so it stays in sync with the model minutes", "Rocrails Modellzeit kann viel schneller laufen als die reale Zeit (der Divider) - die Bahnhofsuhr-Sekundenzeiger-Animation wird dabei um denselben Faktor beschleunigt statt abgeschaltet, damit sie mit den Modell-Minuten synchron bleibt" },
        { "Above a divider of", "Ab einem Divider von" },
        { "it is hidden entirely, since it would no longer be meaningfully readable", "wird er ganz ausgeblendet, da er ohnehin nicht mehr sinnvoll ablesbar w&auml;re" },
        { "Showing Rocrail model time ({divider}&times; speed)", "Zeigt die Rocrail-Modellzeit ({divider}-fache Geschwindigkeit)" },
        { "Use this server", "Diesen Server verwenden" },
        { "IP address or hostname", "IP-Adresse oder Hostname" },
        { "Take over the model time from a Rocrail server (model railroad control software) for the hands - unlocks the Rocrail tab, where the server address can then be entered", "Modellzeit von einem Rocrail-Server (Modelleisenbahn-Steuerungssoftware) f&uuml;r die Zeiger &uuml;bernehmen - schaltet den Rocrail-Tab frei, in dem dann die Serveradresse eingetragen werden kann" },
        { "Layout name", "Anlagenname" },
        { "Model time", "Modellzeit" },
        { "Disabled", "Deaktiviert" },
        { "Connecting", "Verbindung wird aufgebaut" },
        { "Connected", "Verbunden" },
        { "paused", "angehalten" },
    };
    static const size_t translationTableSize = sizeof(translationTable) / sizeof(translationTable[0]);


    // Laedt die zuletzt gespeicherte Spracheinstellung aus den Preferences (einmalig in setup()) -
    // translate() liest direkt aus der flash-residenten Tabelle.

    // Loads the last saved language setting from Preferences (once in setup()) - translate() reads directly
    // from the flash-resident table.

    void loadLanguage() {
        currentLanguage = preferences.getString(PK_LANGUAGE, "en");

        // Nicht (mehr) unterstuetzte gespeicherte Sprache -> Englisch
        // Stored language not (or no longer) supported -> English

        if (currentLanguage != "en" && !availableLanguages.count(currentLanguage)) currentLanguage = "en";
    }


    // Setzt die aktive Sprache und speichert sie dauerhaft in den Preferences
    // English: sets the active language and persists it in Preferences

    void saveLanguage(String lang) {
        currentLanguage = lang;
        preferences.putString(PK_LANGUAGE, lang);
    }


    // Uebersetzt einen englischen Schluessel-String in die aktuell aktive Sprache
    // (Fallback: Schluessel selbst bei Englisch/fehlendem Eintrag). Durchsucht
    // direkt die flash-residente translationTable - kein Heap-Umkopieren noetig.

    // English: translates an English key string into the currently active
    // language (fallback: the key itself for English/missing entry). Searches
    // the flash-resident translationTable directly - no heap copying needed.

    String translate(const String& key) {

        if (currentLanguage == "en") return key;  // Englisch: Schlüssel zurückgeben
                                                  // English: return the key

        for (size_t i = 0; i < translationTableSize; i++) {
            if (key == translationTable[i].key) {
                if (currentLanguage == "de") return String(translationTable[i].de);
                return key;
            }
        }

        return key; // Fallback: Schlüssel zurückgeben
                    // English: fallback - return the key
    }


#endif // TRANSLATION_H

