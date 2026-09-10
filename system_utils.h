#pragma once
    // Systemfunktionen: Tasten, Logging, Reset, Neustart, Hilfsfunktionen
    // Benoetigt globals.h, config.h, prefs_keys.h, declarations.h (vor dieser Datei eingebunden)

    // System functions: buttons, logging, reset, restart, helpers
    // Requires globals.h, config.h, prefs_keys.h, declarations.h (included before this file)


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

            // Malt direkt auf TFT statt Sprite-Backbuffer; firstRun=true laesst
            // die Zeiger sofort zur aktuellen Zeit springen statt einzuschleichen
            // (siehe Glaettung in renderClockFrame()).

            // Draws directly to TFT instead of the sprite backbuffer; firstRun=true
            // makes the hands snap to the current time instead of easing in
            // (see smoothing in renderClockFrame()).
            firstRun = true;
        }
#endif
    }


    // Prueft, ob der woechentliche geplante Neustart faellig ist, und loest ihn
    // bei Bedarf aus (praeventiver Neustart gegen langsame Speicherfragmentierung)

    // Checks if the weekly scheduled restart is due and triggers it
    // if needed (preventive restart against slow memory fragmentation)

    void checkWeeklyRestart() {

        // Eigene lokale Zeitstruktur statt globaler 'timeinfo' (wird von
        // Zifferblatt/DCF77/NTP genutzt). Timeout 0 statt 5000ms, sonst
        // blockierte getLocalTime() ohne gueltige Zeit 5s pro Durchlauf.

        // Own local time struct instead of the global 'timeinfo' (used by the
        // clock face/DCF77/NTP). Timeout 0 instead of 5000ms, otherwise
        // getLocalTime() blocked for 5s per pass without a valid time.
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

                // espReboot() statt eigenem preferences.end()+ESP.restart(): erledigt
                // Log-Eintrag, "Rebooting.."-Anzeige und preferences.end() zentral.

                // espReboot() instead of a direct preferences.end()+ESP.restart(): handles
                // the log entry, "Rebooting.." screen and preferences.end() centrally.
                espReboot();
            }
        }
    }


    void eraseAllNVS() {
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


    void espReboot() {

        // Generischer Log-Eintrag fuer jeden Software-Reboot, zentral hier statt
        // an jeder Aufrufstelle. Wird VOR den Display-Aktionen geloggt, damit er
        // sicher im Logfile landet, bevor der ESP neu startet.

        // Generic log entry for every software-triggered reboot, centralized here
        // instead of at each call site. Logged BEFORE the display actions, so it
        // reliably ends up in the log file before the ESP restarts.
        DEBUG_PRINTLN("[SYSTEM] Software-triggered reboot - restarting now..");

        // Erst HIER geschlossen (nach dem Log-Eintrag): logToFile() liest
        // PK_LOG_FILE_NUMBER selbst aus preferences - waere der Handle schon zu,
        // laende der Eintrag in der falschen Datei.

        // Closed only HERE (after the log entry): logToFile() itself reads
        // PK_LOG_FILE_NUMBER from preferences - if the handle were already
        // closed, the entry would land in the wrong file.
        preferences.end();

        // Kurze Verzoegerung, gibt dem Flash-Subsystem Luft nach dem Log-Schreiben.
        // Short delay, gives the flash subsystem breathing room after the log write.
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


    // Log-Funktionen
    // Log functions

    // Gleiche Logik wie in logToFile() (siehe dort), aber ohne Seiteneffekte.
    // Genutzt vom Log-Tab und /api/currentLog, damit beide dieselbe
    // aktuelle Datei sehen.

    // Same logic as in logToFile() (see there), but without side effects.
    // Used by the Log tab and /api/currentLog so both see the same
    // current file.

    String getCurrentLogFileName() {
        uint16_t logfileNumber = preferences.getInt(PK_LOG_FILE_NUMBER, 1);
        return "/log_" + String(logfileNumber) + ".log";
    }


    void deleteAllLogFiles() {

        File root = LittleFS.open("/");
        File file = root.openNextFile();

        while (file) {
            String fileName = file.name();
            file.close();

            if (fileName.endsWith(".log")) {
                if (LittleFS.remove("/" + fileName)) {
                  //  DEBUG_PRINTLN("[LOG] Successfully deleted: " + fileName);
                }
            }
            file = root.openNextFile();
        }
        // Reset auf 1 statt 0: alle anderen Stellen (getCurrentLogFileName(),
        // logToFile()) nutzen 1 als Fallback/Rollover-Wert; 0 waere inkonsistent.

        // Reset to 1 instead of 0: every other spot (getCurrentLogFileName(),
        // logToFile()) uses 1 as the fallback/rollover value; 0 would be inconsistent.
        preferences.putInt(PK_LOG_FILE_NUMBER, 1);
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
            return;
        }

        if (!LittleFS.begin()) {
            if (loggingEnabled) Serial.println("[LOG] LittleFS is not mounted. Log will not be written");
            return;
        }

        String trimmedMessage = message;
        trimmedMessage.trim(); // Entfernt auch \n, \r
                               // Also removes \n, \r
        if (trimmedMessage.isEmpty()) {
            return;
        }

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
            preferences.putBool(PK_LOGGING_ENABLED, false);
            deleteAllLogFiles();
            loggingEnabled = preferences.getBool(PK_LOGGING_ENABLED, false);
            return; // Logging jetzt deaktiviert, kein Logfile schreiben
                    // Logging now disabled, don't write a log file
        }

        String logFileName = "/log_" + String(logfileNumber) + ".log";

        if (LittleFS.exists(logFileName)) {
            File currentLogFile = LittleFS.open(logFileName, FILE_READ);
            if (currentLogFile) {
                size_t fileSize = currentLogFile.size();
                currentLogFile.close();

                if (fileSize > 10 * 1024) {
                    logfileNumber++;
                    preferences.putInt(PK_LOG_FILE_NUMBER, logfileNumber);
                    logFileName = "/log_" + String(logfileNumber) + ".log";
                }
            }
        }

        char timestamp[32];

        // Zeitzone hier NICHT erneut setzen: configTzTime() wuerde bei JEDEM
        // Log-Eintrag (jeder DEBUG_PRINTLN, siehe config.h) den SNTP-Client
        // neustarten und die Zeitsynchronisation staendig unterbrechen.

        // Do NOT set the timezone again here: configTzTime() would restart the
        // SNTP client on EVERY log entry (every DEBUG_PRINTLN, see config.h),
        // constantly disrupting time sync.
        unsigned long currentMillis = millis();
        unsigned long millisInSecond = currentMillis % 1000;

        // Eigene lokale Zeitstruktur wie in checkWeeklyRestart() (siehe dort).
        // Timeout 0 statt 500ms, keine WiFi-Bedingung mehr - Zeit ist auch
        // ohne WLAN gueltig, wenn sie von RTC oder DCF77 stammt.

        // Own local time struct, same reason as in checkWeeklyRestart() (see
        // there). Timeout 0 instead of 500ms, no WiFi condition anymore - time
        // is valid without WiFi too when it comes from RTC or DCF77.
        struct tm logTime;
        if (getLocalTime(&logTime, 0)) {
            strftime(timestamp, sizeof(timestamp), "[%Y-%m-%d %H:%M:%S", &logTime);
            snprintf(timestamp + strlen(timestamp), sizeof(timestamp) - strlen(timestamp), ".%03lu] ", millisInSecond);
        }
        else {
            snprintf(timestamp, sizeof(timestamp), "[%lu ms] ", currentMillis);
        }

        File logFile = LittleFS.open(logFileName, FILE_APPEND);
        if (!logFile) {
            if (loggingEnabled) Serial.println("[LOG] Error opening log file: " + logFileName);
            return;
        }
        logFile.print(timestamp);
        logFile.println(message);
        logFile.close();
    }


    // eigenes trim function
    // Custom trim function

    String trim(const String& str) {
        int start = 0;
        int end = str.length() - 1;

        while (start <= end && isspace(str[start])) {
            start++;
        }

        while (end >= start && isspace(str[end])) {
            end--;
        }

        return str.substring(start, end + 1);
    }

