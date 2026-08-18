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
                    //setCS1(LOW);
                    resetStarted = true;
                    tft.fillScreen(TFT_RED);
                    tft.setTextColor(TFT_WHITE, TFT_RED);
                    tft.setTextSize(TFT_TEXT_SIZE);
                    tft.setCursor(20, CLOCK_HEIGHT / 2);
                    tft.printf("Factory Reset");

                    tft.setCursor(20, (CLOCK_HEIGHT / 2) + 20);
                    tft.printf("in %d secs", secs);
                    delay(WAIT_1s);
                    if (secs > 0) secs--;
                }

                if (millis() - pressStart > WAIT_15s) {
                    //setCS1(LOW);
                    // 15 Sekunden überschritten → Factory Reset
                    // 15 seconds exceeded → factory reset
                    tft.fillScreen(TFT_RED);
                    tft.setTextColor(TFT_WHITE, TFT_RED);
                    tft.setTextSize(TFT_TEXT_SIZE);
                    tft.setCursor(20, CLOCK_HEIGHT / 2);
                    tft.println("Factory Reset..");
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
            // Sprite-Backbuffer - ohne firstRun=true bleibt der Bildschirm
            // dauerhaft auf diesem Text stehen (updateClock() zeichnet
            // danach nur noch die Zeiger, nicht den Zifferblatt-Hintergrund).

            // This function draws directly to the TFT instead of the sprite
            // backbuffer - without firstRun=true the screen stays stuck on
            // this text (updateClock() afterwards only redraws the hands,
            // not the clock face background).
            firstRun = true;
        }
#endif
    }


    // Prueft, ob der woechentliche geplante Neustart faellig ist, und loest ihn
    // bei Bedarf aus (praeventiver Neustart gegen langsame Speicherfragmentierung)

    // Checks if the weekly scheduled restart is due and triggers it
    // if needed (preventive restart against slow memory fragmentation)

    void checkWeeklyRestart() {

       // struct tm timeinfo;
        if (!getLocalTime(&timeinfo)) return;

        if (timeinfo.tm_wday == 0 &&
            timeinfo.tm_hour == 3 && timeinfo.tm_min == 5 && timeinfo.tm_sec == 5) {

            lastResetWeek = preferences.getInt(PK_LAST_RESET_WEEK, -1);

            char weekStr[3];
            strftime(weekStr, sizeof(weekStr), "%V", &timeinfo); // ISO-Woche (01–53)
                                                                 // ISO week (01-53)
            currentWeek = atoi(weekStr);

            if (lastResetWeek == -1) {
                lastResetWeek = currentWeek;
                preferences.putInt(PK_LAST_RESET_WEEK, lastResetWeek);
            }


            if (currentWeek != lastResetWeek) {
                DEBUG_PRINTF("Woechentlicher Reboot in Woche %d\n", currentWeek);
                preferences.putInt(PK_LAST_RESET_WEEK, currentWeek);
                preferences.end();
                delay(WAIT_1s);
                DEBUG_PRINTLN("reboot now..");
                delay(WAIT_1s);
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
        tft.fillScreen(TFT_BLACK);
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
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.setTextSize(TFT_TEXT_SIZE);
        tft.setCursor(20, (CLOCK_HEIGHT / 2));
        tft.println("Rebooting..");
        delay(WAIT_3s);
        tft.fillScreen(TFT_BLACK);
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

        // Zeitzone aus den Preferences abrufen
        // Get timezone from preferences
        String timezone = preferences.getString(PK_TIMEZONE, TIMEZONE_DEFAULT);
        configTzTime(timezone.c_str(), ntpServers[0]); // Zeitzone anwenden
                                                       // Apply timezone

        // Berechne die Millisekunden relativ zur aktuellen Sekunde
        // Calculate milliseconds relative to the current second
        unsigned long currentMillis = millis();
        unsigned long millisInSecond = currentMillis % 1000;

        if (WiFi.status() == WL_CONNECTED && getLocalTime(&timeinfo, 500)) {
            strftime(timestamp, sizeof(timestamp), "[%Y-%m-%d %H:%M:%S", &timeinfo);
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

