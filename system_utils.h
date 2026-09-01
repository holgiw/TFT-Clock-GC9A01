#pragma once
    // ### Systemfunktionen: Tasten, Logging, Reset, Neustart, Hilfsfunktionen
    // Benoetigt globals.h, config.h, prefs_keys.h und declarations.h (werden
    // zentral in uhr3.ino VOR dieser Datei eingebunden).

    // ### System functions: buttons, logging, reset, restart, helpers
    // Requires globals.h, config.h, prefs_keys.h and declarations.h
    // (included centrally in uhr3.ino BEFORE this file).


    // Button prüfen und ggf. Anzeige oder Factory Reset auslösen
    // Check button and trigger display or factory reset if needed

    void checkButton() {
        bool resetStarted = false;
#ifdef BUTTON1
        if (digitalRead(BUTTON1) == HIGH || digitalRead(BOOT_BUTTON) == LOW) {

            uint8_t secs = 5;
            unsigned long pressStart = millis();

            clearTFT();

            // Einmalig Anzeige zeichnen
            // Draw display once
            showWlanCredentials(WiFi.SSID());

            // Blockierender Loop während Button gedrückt
            // Blocking loop while button is pressed
            while (digitalRead(BUTTON1) == HIGH || digitalRead(BOOT_BUTTON) == LOW) {
                if (millis() - pressStart > WAIT_10s && millis() - pressStart < WAIT_15s) {
                    resetStarted = true;
                    DRAW_ON_BOTH_DISPLAYS(
                        tft.fillScreen(TFT_RED);
                        tft.setTextColor(TFT_WHITE, TFT_RED);
                        tft.setTextSize(TFT_TEXT_SIZE);
                        tft.setCursor(20, CLOCK_HEIGHT / 2);
                        tft.printf("Factory Reset");

                        tft.setCursor(20, (CLOCK_HEIGHT / 2) + 20);
                        tft.printf("in %d secs", secs);
                    );
                    delay(WAIT_1s);
                    if (secs > 0) secs--;
                }

                if (millis() - pressStart > WAIT_15s) {
                    // 15 Sekunden überschritten → Factory Reset
                    // 15 seconds exceeded → factory reset
                    DRAW_ON_BOTH_DISPLAYS(
                        tft.fillScreen(TFT_RED);
                        tft.setTextColor(TFT_WHITE, TFT_RED);
                        tft.setTextSize(TFT_TEXT_SIZE);
                        tft.setCursor(20, CLOCK_HEIGHT / 2);
                        tft.println("Factory Reset..");
                    );
                    delay(WAIT_1s);
                    factoryReset();
                    return;
                }
                delay(10);
            }
            // Button wurde vor 10secs losgelassen → WLAN-Credentials für 3 Sekunden anzeigen
            // Button released before 10 secs → show WLAN credentials for 3 seconds
            if (!resetStarted) {
                delay(WAIT_3s);
            }

            // Diese Funktion malt direkt auf den TFT statt auf das
            // Sprite-Backbuffer. Der Text verschwindet zwar von selbst, weil
            // updateClock() ohnehin ein volles Bild ausgibt - firstRun=true
            // sorgt aber dafuer, dass die Zeiger danach direkt auf die aktuelle
            // Zeit springen, statt aus ihrer alten Position dorthin zu
            // schleichen (siehe Glaettung in renderClockFrame()).

            // This function draws directly to the TFT instead of the sprite
            // backbuffer. The text disappears by itself, since updateClock()
            // outputs a full frame anyway - but firstRun=true makes the hands
            // snap straight to the current time afterwards instead of easing
            // there from their old position (see the smoothing in
            // renderClockFrame()).
            firstRun = true;
        }
#endif
    }


    // Prueft, ob der woechentliche geplante Neustart faellig ist, und loest ihn
    // bei Bedarf aus (praeventiver Neustart gegen langsame Speicherfragmentierung)

    // Checks if the weekly scheduled restart is due and triggers it
    // if needed (preventive restart against slow memory fragmentation)

    void checkWeeklyRestart() {

        // Eigene lokale Zeitstruktur statt der globalen 'timeinfo': diese
        // Funktion laeuft in JEDEM loop()-Durchlauf, wuerde die globale Struktur
        // also permanent ueberschreiben, die Zifferblatt, DCF77 und der
        // NTP-Server ebenfalls benutzen. Ausserdem mit Timeout 0 statt des
        // Defaults (5000 ms!): ohne gueltige Systemzeit hat getLocalTime()
        // sonst pro Durchlauf volle 5 Sekunden blockiert - die Uhr stand
        // praktisch still und Webserver/Button reagierten nur noch traege.

        // Its own local time struct instead of the global 'timeinfo': this
        // function runs in EVERY loop() pass and would therefore constantly
        // overwrite the global struct that the clock face, DCF77 and the NTP
        // server use as well. Also with timeout 0 instead of the default
        // (5000 ms!): without a valid system time, getLocalTime() previously
        // blocked for a full 5 seconds per pass - the clock effectively stood
        // still and the web server/button became sluggish.
        struct tm restartTime;
        if (!getLocalTime(&restartTime, 0)) return;

        if (restartTime.tm_wday == 0 &&
            restartTime.tm_hour == 3 && restartTime.tm_min == 5 && restartTime.tm_sec == 5) {

            lastResetWeek = preferences.getInt(PK_LAST_RESET_WEEK, -1);

            char weekStr[3];
            strftime(weekStr, sizeof(weekStr), "%V", &restartTime); // ISO-Woche (01–53)
                                                                    // ISO week (01-53)
            currentWeek = atoi(weekStr);

            if (lastResetWeek == -1) {
                lastResetWeek = currentWeek;
                preferences.putInt(PK_LAST_RESET_WEEK, lastResetWeek);
            }


            if (currentWeek != lastResetWeek) {
                DEBUG_PRINTF("Woechentlicher Reboot in Woche %d\n", currentWeek);
                preferences.putInt(PK_LAST_RESET_WEEK, currentWeek);
                delay(WAIT_1s);
                DEBUG_PRINTLN("reboot now..");
                delay(WAIT_1s);

                // preferences.end() erst NACH dem letzten Log-Eintrag - logToFile()
                // liest selbst PK_LOG_FILE_NUMBER aus preferences, ein bereits
                // geschlossener Handle wuerde den Eintrag in die falsche Logdatei
                // schreiben lassen (siehe Kommentar in espReboot()).

                // preferences.end() only AFTER the last log entry - logToFile()
                // itself reads PK_LOG_FILE_NUMBER from preferences, an already
                // closed handle would cause the entry to be written to the
                // wrong log file (see comment in espReboot()).
                preferences.end();
                ESP.restart();
            }
        }
    }


    // --- Funktion: Löscht den gesamten NVS-Speicher ---
    // --- Function: Erases the entire NVS storage ---

    void eraseAllNVS() {
        // Löscht gesamten NVS-Speicher
        // Erases entire NVS storage
        esp_err_t result = nvs_flash_erase();
        if (result == ESP_OK) {
            DEBUG_PRINTLN("Complete NVS storage erased (incl. WiFi, Preferences)");
            nvs_flash_init();  // Wichtig: Danach wieder initialisieren!
                               // Important: must re-initialize afterward!
        }
        else {
            DEBUG_PRINTF("NVS erase failed: %s\n", esp_err_to_name(result));
        }
    }


    // --- Funktion: Führt einen Factory Reset durch ---
    // --- Function: Performs a factory reset ---

    void factoryReset() {
        DRAW_ON_BOTH_DISPLAYS(
            tft.fillScreen(TFT_BLACK);
        );
        preferences.begin("clock", false);
        preferences.putInt(PK_FIRST_START, 0);
        preferences.end();
        delay(100);
        DEBUG_PRINTLN(">>> Factory reset started..");
        LittleFS.begin();
        LittleFS.format();
        LittleFS.end();
        eraseWiFiConfig();
        eraseAllNVS();
        delay(WAIT_5s);
        DEBUG_PRINTLN(">>> Restarting..");
        espReboot();
    }


    // --- Funktion: führt einen Reboot durch mit Anzeige ---
    // --- Function: Performs a reboot with display ---

    void espReboot() {

        // Generischer Log-Eintrag fuer JEDEN per Software ausgeloesten Reboot
        // (WLAN-Wechsel, Einstellungen speichern, manueller Neustart-Button, etc.) -
        // zentral hier statt an jeder einzelnen Aufrufstelle, damit kein Aufrufer
        // vergessen werden kann. Wird VOR den Display-Aktionen geloggt, damit der
        // Eintrag sicher im aktuellen Logfile landet, bevor der ESP neu startet.

        // Generic log entry for EVERY software-triggered reboot (WiFi switch,
        // saving settings, manual restart button, etc.) - centralized here
        // instead of at each individual call site, so no caller can be missed.
        // Logged BEFORE the display actions, so the entry reliably ends up in
        // the current log file before the ESP restarts.
        DEBUG_PRINTLN("[SYSTEM] Software-triggered reboot - restarting now..");

        // preferences.end() wird bewusst ERST HIER (nach dem obigen Log-Eintrag)
        // aufgerufen, nicht schon vom Aufrufer davor: logToFile() liest selbst
        // preferences.getInt(PK_LOG_FILE_NUMBER, ...), um die aktuelle Logdatei
        // zu bestimmen - war der Handle bereits geschlossen, lieferte das nur
        // noch den Default-Wert zurueck und der Log-Eintrag landete in der
        // falschen Datei (dort fehlte er dann scheinbar). Ein expliziter
        // preferences.end() ist fuer die Datensicherheit ohnehin nicht noetig -
        // jedes putXxx()/remove() committet laut Preferences-Quellcode bereits
        // synchron per nvs_commit() - dient hier nur dem sauberen Schliessen
        // des Handles vor dem Neustart.

        // preferences.end() is deliberately called HERE (after the log entry
        // above), not already by the caller beforehand: logToFile() itself
        // calls preferences.getInt(PK_LOG_FILE_NUMBER, ...) to determine the
        // current log file - if the handle was already closed, that only
        // returned the default value and the log entry ended up in the wrong
        // file (appearing to be missing there). An explicit preferences.end()
        // isn't actually needed for data safety anyway - per the Preferences
        // source, every putXxx()/remove() already commits synchronously via
        // nvs_commit() - this only cleanly closes the handle before restarting.
        preferences.end();

        // Kleine zusaetzliche Verzoegerung nach dem Log-Eintrag: logToFile()
        // schliesst die Datei zwar bereits synchron (flusht auf den Flash),
        // dies gibt dem Flash-Subsystem aber noch etwas Luft, bevor unten
        // der eigentliche Neustart angestossen wird - reine Sicherheitsmarge.

        // Small extra delay after the log entry: logToFile() already closes
        // the file synchronously (flushes to flash), but this gives the flash
        // subsystem a little more breathing room before the actual restart
        // further down - purely an extra safety margin.
        delay(100);

        DRAW_ON_BOTH_DISPLAYS(
            tft.fillScreen(TFT_BLACK);
            tft.setTextColor(TFT_GREEN, TFT_BLACK);
            tft.setTextSize(TFT_TEXT_SIZE);
            tft.setCursor(20, (CLOCK_HEIGHT / 2));
            tft.println("Rebooting..");
        );
        delay(WAIT_3s);
        DRAW_ON_BOTH_DISPLAYS(
            tft.fillScreen(TFT_BLACK);
        );
        delay(100);
        ESP.restart();
    }


    // ####################################################################
    // ### LOG-FUNKTIONEN #################################################

    // ### LOG FUNCTIONS ###################################################
    // Löscht alle Logdateien aus dem LittleFS
    // Deletes all log files from LittleFS

    void deleteAllLogFiles() {

        File root = LittleFS.open("/");
        File file = root.openNextFile();

        while (file) {
            String fileName = file.name();
            file.close(); // Datei schließen, bevor sie gelöscht wird
                          // Close file before deleting it

            if (fileName.endsWith(".log")) {
                if (LittleFS.remove("/" + fileName)) {
                  //  DEBUG_PRINTLN("[LOG] Successfully deleted: " + fileName);
                }
            }
            file = root.openNextFile(); // Nächste Datei öffnen
                                        // Open next file
        }
        preferences.putInt(PK_LOG_FILE_NUMBER, 0); // Log-Dateinummer zurücksetzen
                                                   // Reset log file number
    }


    // Prueft den freien Heap und schreibt bei Unterschreiten von HEAP_WARNING_THRESHOLD
    // eine Log-Zeile mit Kontext + Minimum seit Boot - an speicherhungrigen Stellen
    // aufgerufen, damit sich knapper Heap einer Codestelle zuordnen laesst.

    // Checks free heap and logs a line with context + minimum since boot
    // when it drops below HEAP_WARNING_THRESHOLD; call this at memory-heavy
    // spots to trace low heap back to a specific place in the code.

    void checkHeapWarning(const String& context) {
        size_t freeHeap = ESP.getFreeHeap();
        if (freeHeap < HEAP_WARNING_THRESHOLD) {
            DEBUG_PRINTLN("[HEAP WARNING] " + context + ": only " + String(freeHeap) +
                " bytes free (minimum since boot: " + String(ESP.getMinFreeHeap()) + " bytes)");
        }
    }


    // Schreibt eine Lognachricht in die aktuelle Logdatei, wenn Logging aktiviert ist
    // Writes a log message to the current log file if logging is enabled

    void logToFile(const String& message) {
        if (!loggingEnabled) {
            return; // Logging ist deaktiviert
                    // Logging is disabled
        }

        // Überprüfe, ob LittleFS gemountet ist
        // Check whether LittleFS is mounted
        if (!LittleFS.begin()) {
            if (loggingEnabled) Serial.println("[LOG] LittleFS is not mounted. Log will not be written");
            return;
        }

        // Überprüfe, ob die Nachricht leer ist oder nur aus Leerzeichen/Zeilenumbrüchen besteht
        // Check whether the message is empty or only whitespace/newlines
        String trimmedMessage = message;
        trimmedMessage.trim(); // Entfernt führende und nachfolgende Leerzeichen sowie \n, \r
                               // Removes leading/trailing whitespace and \n, \r
        if (trimmedMessage.isEmpty()) {
            return; // Nachricht nicht schreiben
                    // Don't write the message
        }

        // Überprüfe, ob genügend Speicherplatz verfügbar ist
        // Check whether enough storage space is available
        size_t freeSpace = LittleFS.totalBytes() - LittleFS.usedBytes();
        if (freeSpace < 15 * 1024) { // Weniger als 15 KB frei
                                     // Less than 15 KB free
            deleteAllLogFiles(); // Alle Logdateien löschen, um Platz zu schaffen
                                 // Delete all log files to free up space
            freeSpace = LittleFS.totalBytes() - LittleFS.usedBytes();
            if (freeSpace < 10 * 1024) {
                if (loggingEnabled) Serial.println("[LOG] Not enough free space on LittleFS. Log will not be written");
                return;
            }
        }

        uint16_t logfileNumber = preferences.getInt(PK_LOG_FILE_NUMBER, 1);
        if (logfileNumber > 9) {
            logfileNumber = 1;
            preferences.putInt(PK_LOG_FILE_NUMBER, logfileNumber);
            preferences.putBool(PK_LOGGING_ENABLED, false); // Logging deaktivieren
                                                            // Disable logging
            deleteAllLogFiles();
            loggingEnabled = preferences.getBool(PK_LOGGING_ENABLED, false);
            return; // Kein Logfile schreiben, da Logging jetzt deaktiviert ist
                    // Don't write a log file since logging is now disabled
        }

        String logFileName = "/log_" + String(logfileNumber) + ".log";

        // Überprüfe die Größe des aktuellen Logfiles
        // Check the size of the current log file
        if (LittleFS.exists(logFileName)) {
            File currentLogFile = LittleFS.open(logFileName, FILE_READ);
            if (currentLogFile) {
                size_t fileSize = currentLogFile.size();
                currentLogFile.close();

                if (fileSize > 10 * 1024) { // Wenn die Datei größer als 10 KB ist
                                            // If the file is larger than 10 KB
                    logfileNumber++;
                    preferences.putInt(PK_LOG_FILE_NUMBER, logfileNumber);
                    logFileName = "/log_" + String(logfileNumber) + ".log";
                }
            }
        }

        // Zeitstempel generieren
        // Generate timestamp
        char timestamp[32];

        // Zeitzone NICHT hier erneut setzen: configTzTime() startet dabei
        // auch den SNTP-Client neu - bei jedem einzelnen Log-Eintrag (diese
        // Funktion wird von JEDEM DEBUG_PRINTLN() aufgerufen, siehe config.h)
        // waere das ein staendiger Neustart der Zeitsynchronisation und
        // Ursache fuer eine driftende/falsche Anzeige trotz erfolgreichem
        // NTP-Sync. Die Zeitzone wird bereits einmalig in setupNTP() beim
        // Boot gesetzt und bleibt fuer die gesamte Laufzeit gueltig -
        // getLocalTime() unten liest sie automatisch mit.

        // Do NOT set the timezone again here: configTzTime() also restarts
        // the SNTP client - since this function is called by EVERY single
        // DEBUG_PRINTLN() (see config.h), that would mean constantly
        // restarting time sync on every log line, causing the display to
        // drift/show a wrong time despite a successful NTP sync. The
        // timezone is already set once in setupNTP() at boot and stays
        // valid for the whole runtime - getLocalTime() below picks it up
        // automatically.

        // Berechne die Millisekunden relativ zur aktuellen Sekunde
        // Calculate milliseconds relative to the current second
        unsigned long currentMillis = millis();
        unsigned long millisInSecond = currentMillis % 1000;

        // Eigene lokale Zeitstruktur statt der globalen 'timeinfo': diese
        // Funktion wird von JEDEM DEBUG_PRINT/PRINTLN/PRINTF aufgerufen (siehe
        // config.h) und hat vorher bei jeder Logzeile die globale Struktur
        // ueberschrieben, die Zifferblatt, DCF77 und der NTP-Server benutzen.
        // Konkret hat das in getDCF77Time() zugeschlagen: setTimeStruct() loggt
        // intern und hat 'timeinfo' veraendert, BEVOR rtc.adjust(DateTime(
        // timeinfo...)) sie ausgelesen hat - die RTC konnte also mit einer
        // anderen Zeit gestellt werden als DCF77 geliefert hat.
        //
        // Ausserdem: Timeout 0 statt 500 ms (blockierte sonst jede Logzeile,
        // solange keine gueltige Zeit gesetzt war) und keine WiFi-Bedingung
        // mehr - die Systemzeit ist auch ohne WLAN gueltig, wenn sie von RTC
        // oder DCF77 kommt; vorher stand in diesen Faellen unnoetig nur die
        // Millisekunden-Ersatzform im Log.

        // Its own local time struct instead of the global 'timeinfo': this
        // function is called by EVERY DEBUG_PRINT/PRINTLN/PRINTF (see
        // config.h) and previously overwrote, on every log line, the global
        // struct used by the clock face, DCF77 and the NTP server. It bit
        // specifically in getDCF77Time(): setTimeStruct() logs internally and
        // modified 'timeinfo' BEFORE rtc.adjust(DateTime(timeinfo...)) read it
        // - so the RTC could be set to a different time than DCF77 delivered.
        //
        // Also: timeout 0 instead of 500 ms (previously blocked every log line
        // while no valid time was set) and no more WiFi condition - the system
        // time is valid without WiFi too when it comes from the RTC or DCF77;
        // previously those cases needlessly logged only the millisecond
        // fallback form.
        struct tm logTime;
        if (getLocalTime(&logTime, 0)) {
            strftime(timestamp, sizeof(timestamp), "[%Y-%m-%d %H:%M:%S", &logTime);
            snprintf(timestamp + strlen(timestamp), sizeof(timestamp) - strlen(timestamp), ".%03lu] ", millisInSecond);
        }
        else {
            snprintf(timestamp, sizeof(timestamp), "[%lu ms] ", currentMillis);
        }

        // Öffne die Datei im Anhängemodus (append)
        // Open the file in append mode
        File logFile = LittleFS.open(logFileName, FILE_APPEND);
        if (!logFile) {
            if (loggingEnabled) Serial.println("[LOG] Error opening log file: " + logFileName);
            return;
        }
        // Schreibe Zeitstempel und Nachricht in die Datei
        // Write timestamp and message to the file
        logFile.print(timestamp);
        logFile.println(message);
        logFile.close();
    }


    // eigenes trim function
    // Custom trim function

    String trim(const String& str) {
        int start = 0;
        int end = str.length() - 1;

        // Führende Leerzeichen entfernen
        // Remove leading whitespace
        while (start <= end && isspace(str[start])) {
            start++;
        }

        // Nachfolgende Leerzeichen entfernen
        // Remove trailing whitespace
        while (end >= start && isspace(str[end])) {
            end--;
        }

        // Substring zurückgeben, der keine führenden oder nachfolgenden Leerzeichen enthält
        // Return substring without leading or trailing whitespace
        return str.substring(start, end + 1);
    }

