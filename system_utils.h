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


    // Kurze, englische Beschriftung fuer eine per Code zu bestaetigende
    // Aktion - gemeinsam genutzt vom Display (checkFactoryResetCodePending()
    // direkt darunter) und von der Web-Eingabeseite (/factoryReset/enterCode
    // in webserver_routes.h), damit beide immer denselben Text zeigen.

    // Short, English label for an action that requires code confirmation -
    // shared by the display (checkFactoryResetCodePending() right below) and
    // the web entry page (/factoryReset/enterCode in webserver_routes.h), so
    // both always show the same text.

    String factoryResetActionLabel(const String& action) {
        if (action == "all") return "Factory Reset";
        if (action == "wifi") return "Reset WiFi";
        if (action == "faces") return "Delete Faces";
        if (action == "hands") return "Delete Hands";
        if (action == "presets") return "Delete Presets";
        if (action == "wlanDeleteActive") return "Delete Active WiFi";
        if (action == "wlanOverwriteActive") return "Change Active WiFi";
        if (action == "wlanSwitchActive") return "Switch Active WiFi";
        return "Confirm Action";
    }


    // Erzeugt einen neuen 3-stelligen Bestaetigungscode fuer `action`, merkt
    // sich die angeforderte Aktion (factoryResetPendingAction, siehe
    // globals.h) und setzt den Anzeige-/Fehlversuchszustand zurueck - der
    // Code selbst wird von checkFactoryResetCodePending() im naechsten
    // loop()-Durchlauf auf dem Display gezeichnet. Gemeinsame Logik von
    // /factoryReset/requestCode sowie /deletewifi und /save (aktives
    // WLAN-Netzwerk betroffen) in webserver_routes.h - der Aufrufer muss
    // vorher die zur jeweiligen Aktion gehoerende Nutzlast setzen
    // (pendingWifiChangeIndex bzw. pendingWifiSsid[]/pendingWifiPass[], siehe
    // globals.h).

    // Generates a new 3-digit confirmation code for `action`, remembers the
    // requested action (factoryResetPendingAction, see globals.h), and
    // resets the display/attempt state - the code itself is drawn on the
    // display by checkFactoryResetCodePending() on the next loop() pass.
    // Shared logic between /factoryReset/requestCode as well as /deletewifi
    // and /save (active WiFi network affected) in webserver_routes.h - the
    // caller must set that action's payload beforehand
    // (pendingWifiChangeIndex or pendingWifiSsid[]/pendingWifiPass[], see
    // globals.h).

    void requestConfirmationCode(const String& action) {

        // Bereits ein noch gueltiger Code anhaengig? Dann nicht
        // ueberschreiben/das Zeitfenster nicht verlaengern - sonst koennte
        // eine Anfrage kurz vor Ablauf des aktuellen Fensters dieses immer
        // wieder von Neuem starten und die Anzeige so dauerhaft blockieren.
        // Relevant, seit /factoryReset/requestCode, /deletewifi, /save und
        // /api/connectWifi dies bei Zugriff aus einem NICHT-privaten Netz
        // ungeprueft aufrufen (aus einem privaten Netz wird die Aktion
        // stattdessen direkt ausgefuehrt, kein Code noetig - siehe dort).

        // Already a still-valid code pending? Then don't overwrite it/
        // extend its window - otherwise a request sent shortly before the
        // current window expires could keep restarting it indefinitely,
        // permanently blocking the display. Relevant since
        // /factoryReset/requestCode, /deletewifi, /save and
        // /api/connectWifi call this unconditionally on access from a
        // NON-private network (from a private network the action is
        // executed directly instead, no code needed - see there).
        if (!factoryResetCode.isEmpty() && millis() - factoryResetCodeStartMillis < FACTORY_RESET_CODE_TIMEOUT_MS) {
            return;
        }

        int code = random(100, 1000); // 3-stellig: 100-999
                                      // 3-digit: 100-999
        factoryResetCode = String(code);
        factoryResetCodeStartMillis = millis();
        factoryResetCodeShown = false; // loop() zeichnet ihn beim naechsten Durchlauf
                                       // loop() draws it on the next pass
        factoryResetPendingAction = action;
        factoryResetCodeAttempts = 0; // frischer Code, frisches Kontingent an Versuchen
                                      // fresh code, fresh attempt budget
    }


    // Zeigt eine Hinweisseite und liefert true, wenn bereits ein anderer,
    // noch gueltiger Bestaetigungscode aussteht. MUSS von /deletewifi, /save
    // und /api/connectWifi aufgerufen werden, BEVOR sie pendingWifiChangeIndex
    // bzw. pendingWifiSsid[]/pendingWifiPass[] setzen: requestConfirmationCode()
    // ignoriert eine Anfrage still, waehrend ein Code noch aussteht (siehe
    // dort) - ohne diese Pruefung VORHER wuerden die Aufrufer die Payload
    // einer laengst noch ausstehenden, anderen Anfrage ueberschreiben, sodass
    // bei Eingabe von deren (weiterhin gueltigem) Code die FALSCHE Aktion
    // ausgefuehrt wuerde (z.B. das falsche WLAN-Netzwerk betroffen waere).
    // Der Aufrufer muss bei true sofort zurueckkehren, ohne die
    // Payload-Variablen vorher zu setzen.

    // Shows a hint page and returns true if a different, still-valid
    // confirmation code is already pending. MUST be called by /deletewifi,
    // /save and /api/connectWifi BEFORE they set pendingWifiChangeIndex or
    // pendingWifiSsid[]/pendingWifiPass[]: requestConfirmationCode() silently
    // ignores a request while a code is still pending (see there) - without
    // this check FIRST, the callers would overwrite the payload of a still-
    // pending, different request, so entering that request's (still valid)
    // code would end up executing the WRONG action (e.g. affecting the
    // wrong WiFi network). The caller must return immediately on true,
    // without setting the payload variables first.

    bool rejectIfConfirmationPending() {
        if (!factoryResetCode.isEmpty() && millis() - factoryResetCodeStartMillis < FACTORY_RESET_CODE_TIMEOUT_MS) {
            DEBUG_PRINTLN("[SECURITY] Request from " + webserver.client().remoteIP().toString() + " ignored - a different confirmation (action: " + factoryResetPendingAction + ") is still pending");
            webserver.send(200, "text/html", simpleMessagePage(translate("Factory&nbsp;Reset"), "<p>" + translate("Another confirmation is already pending. Please complete it or wait a moment and try again") + ".</p>"));
            return true;
        }
        return false;
    }


    // Zeigt einen zufaelligen 3-stelligen Bestaetigungscode auf dem Display,
    // solange eine Aktion angefordert wurde, die physischen Zugriff auf die
    // Uhr voraussetzen soll (factoryResetCode != "", siehe globals.h) - das
    // betrifft die fuenf Factory-Reset-Aktionen UND das Loeschen/
    // Ueberschreiben des aktuell verbundenen WLAN-Netzwerks. Die Aktion
    // selbst (/factoryReset/confirm) verlangt diesen Code als Beweis
    // physischen Zugriffs, da er nur auf dem tatsaechlichen Geraet ablesbar
    // ist (schuetzt z.B. vor einer Aktion aus der Ferne ueber eine DMZ/Port-
    // Weiterleitung). Nicht-blockierend: wird in jedem loop()-Durchlauf
    // aufgerufen, zeichnet den Code nur einmal (factoryResetCodeShown) und
    // verwirft ihn nach Ablauf von FACTORY_RESET_CODE_TIMEOUT_MS -
    // updateClock() zeichnet das Display dann im naechsten Durchlauf von
    // selbst wieder normal.

    // Shows a random 3-digit confirmation code on the display for as long as
    // an action requiring physical access to the clock has been requested
    // (factoryResetCode != "", see globals.h) - this covers the five
    // factory-reset actions AND deleting/overwriting the currently connected
    // WiFi network. The action itself (/factoryReset/confirm) requires this
    // code as proof of physical access, since it can only be read on the
    // actual device (protects against, e.g., an action triggered remotely
    // via a DMZ/port forward). Non-blocking: called on every loop() pass,
    // draws the code only once (factoryResetCodeShown) and discards it once
    // FACTORY_RESET_CODE_TIMEOUT_MS has elapsed - updateClock() then resumes
    // drawing the display normally on the next pass by itself.

    bool checkFactoryResetCodePending() {
        if (factoryResetCode.isEmpty()) return false;

        if (millis() - factoryResetCodeStartMillis > FACTORY_RESET_CODE_TIMEOUT_MS) {
            DEBUG_PRINTLN("[SECURITY] Confirmation code expired unused (action: " + factoryResetPendingAction + ")");
            factoryResetCode = "";
            factoryResetCodeShown = false;
            factoryResetPendingAction = "";
            factoryResetCodeAttempts = 0;
            pendingWifiChangeIndex = -1;

            // pendingWifiSsid[]/pendingWifiPass[] ebenfalls verwerfen, damit
            // ein evtl. eingetragenes Klartext-Passwort aus einem
            // unbestaetigt verfallenen "wlanOverwriteActive"-Antrag nicht
            // unnoetig lange im RAM stehen bleibt.

            // Also discard pendingWifiSsid[]/pendingWifiPass[], so a
            // plaintext password from an unconfirmed, expired
            // "wlanOverwriteActive" request doesn't linger in RAM longer
            // than necessary.
            for (int i = 0; i < MAX_WLAN; i++) {
                pendingWifiSsid[i] = "";
                pendingWifiPass[i] = "";
            }
            return false;
        }

        if (!factoryResetCodeShown) {
            factoryResetCodeShown = true;

            // Nur noch der nackte Code, gross und mittig - keine
            // Aktionsbeschriftung und kein Hinweistext mehr auf dem Display.
            // factoryResetActionLabel() bleibt fuer die Web-Eingabeseite
            // erhalten (/factoryReset/enterCode in webserver_routes.h), die
            // dort weiterhin anzeigt, WELCHE Aktion gerade bestaetigt wird.

            // Only the bare code now, large and centered - no more action
            // label and no more hint text on the display.
            // factoryResetActionLabel() is kept for the web entry page
            // (/factoryReset/enterCode in webserver_routes.h), which still
            // shows WHICH action is currently being confirmed there.

            // Vor und nach dem Code je ein "_" - rein optisch auf dem
            // Display, NICHT Teil von factoryResetCode selbst (der Vergleich
            // in /factoryReset/confirm bleibt bei den nackten 3 Ziffern).

            // An "_" before and after the code - purely visual on the
            // display, NOT part of factoryResetCode itself (the comparison
            // in /factoryReset/confirm still checks the bare 3 digits).
            String displayCode = "_" + factoryResetCode + "_";

            // Textgroesse: groesstmoegliche ganzzahlige Vervielfachung der
            // eingebauten 6x8-Basisschrift, die den Anzeigetext (displayCode,
            // 5 Zeichen inkl. der beiden "_") noch mit Marge (75% von
            // CLOCK_WIDTH) auf dem jeweiligen Display unterbringt - skaliert
            // dadurch automatisch mit auf dem kleineren, quadratischen
            // 160x160-Display (siehe config.h), statt einen fixen Wert zu
            // riskieren, der dort ueberliefe.

            // Text size: the largest integer multiple of the built-in 6x8
            // base font that still fits the display text (displayCode, 5
            // characters including the two "_") with margin (75% of
            // CLOCK_WIDTH) on whichever display - this scales down
            // automatically on the smaller, square 160x160 display (see
            // config.h) instead of risking a fixed value that would overflow
            // there.
            int codeTextSize = (int)((CLOCK_WIDTH * 0.75f) / (displayCode.length() * 6));
            if (codeTextSize < 1) codeTextSize = 1;

            DRAW_ON_BOTH_DISPLAYS(
                tft.fillScreen(TFT_BLACK);
                tft.setTextColor(TFT_YELLOW, TFT_BLACK);
                tft.setTextSize(codeTextSize);
                int codeWidth = tft.textWidth(displayCode);
                int codeHeight = codeTextSize * 8; // Zeilenhoehe der Basisschrift bei Groesse 1 ist 8px
                                                   // base font's line height at size 1 is 8px
                tft.setCursor((CLOCK_WIDTH - codeWidth) / 2, (CLOCK_HEIGHT - codeHeight) / 2);
                tft.println(displayCode);
            );
        }
        return true;
    }


    void factoryReset() {

        // Zuerst hier: eine evtl. laufende NTP-Sync-Task (siehe time_sync.h)
        // wuerde sonst waehrend LittleFS.format()/eraseAllNVS()/dem folgenden
        // preferences.end()/begin() weiter auf genau diese Subsysteme zugreifen.

        // First, here: an NTP sync task still running (see time_sync.h) would
        // otherwise keep accessing exactly these subsystems during
        // LittleFS.format()/eraseAllNVS()/the preferences.end()/begin() below.
        stopNtpSyncTaskIfRunning();

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

        // Zuerst hier: eine evtl. laufende NTP-Sync-Task (siehe time_sync.h)
        // wuerde sonst waehrend/nach dem folgenden preferences.end() ueber
        // logToFile() weiter auf Preferences zugreifen (geschlossener/neu
        // geoeffneter Handle aus einem zweiten Thread). Deckt auch den
        // woechentlichen Neustart (checkWeeklyRestart()) und alle Restart-
        // Buttons der Weboberflaeche ab, die alle hier durchlaufen.

        // First, here: an NTP sync task still running (see time_sync.h) would
        // otherwise keep accessing Preferences via logToFile() during/after
        // the preferences.end() below (a closed/reopened handle from a second
        // thread). Also covers the weekly restart (checkWeeklyRestart()) and
        // every restart button in the web UI, since they all go through here.
        stopNtpSyncTaskIfRunning();

        // Generischer Log-Eintrag fuer jeden Software-Reboot, zentral hier statt
        // an jeder Aufrufstelle. Wird VOR den Display-Aktionen geloggt, damit er
        // sicher im Logfile landet, bevor der ESP neu startet.

        // Generic log entry for every software-triggered reboot, centralized here
        // instead of at each call site. Logged BEFORE the display actions, so it
        // reliably ends up in the log file before the ESP restarts.
        DEBUG_PRINTLN("[SYSTEM] Software-triggered reboot - restarting now..");

        // Gepufferte Log-Zeilen (siehe logToFile()/globals.h) JETZT erzwingen,
        // nicht erst beim naechsten checkLogFlush() aus loop() - sonst gingen
        // dieser und alle seit dem letzten Flush gesammelten Eintraege beim
        // Neustart verloren.

        // Force any buffered log lines (see logToFile()/globals.h) to flash
        // NOW, not at the next checkLogFlush() from loop() - otherwise this
        // and every entry collected since the last flush would be lost on restart.
        flushLogBuffer();

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


    // Schreibt eine Lognachricht in den RAM-Puffer, wenn Logging aktiviert ist -
    // der tatsaechliche Flash-Zugriff passiert erst in flushLogBuffer(), siehe dort.
    // Writes a log message into the RAM buffer if logging is enabled - the
    // actual flash access happens only in flushLogBuffer(), see there.

    void logToFile(const String& message) {
        if (!loggingEnabled) {
            return;
        }

        String trimmedMessage = message;
        trimmedMessage.trim(); // Entfernt auch \n, \r
                               // Also removes \n, \r
        if (trimmedMessage.isEmpty()) {
            return;
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

        // logLineBuffer wird sowohl vom Haupt-Loop als auch von der NTP-/
        // Rocrail-Sync-Task beschrieben (siehe globals.h) - ohne den Mutex
        // waere das ein Data Race auf den internen String-Speicher.

        // logLineBuffer is written from both the main loop and the NTP/
        // Rocrail sync task (see globals.h) - without the mutex this would
        // be a data race on the String's internal storage.
        if (logBufferMutex != nullptr && xSemaphoreTake(logBufferMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            logLineBuffer += String(timestamp) + trimmedMessage + "\n";
            xSemaphoreGive(logBufferMutex);
        }
    }


    // Schreibt den gesammelten Log-Puffer auf einen Rutsch auf Flash (statt
    // einer Datei-oeffnen/schreiben/schliessen-Runde PRO Zeile, siehe
    // logToFile()) - buendelt so mehrere Flash-Zugriffe (und die damit
    // verbundenen kurzen Aussetzer, siehe Kommentar bei logLineBuffer in
    // globals.h) zu einem einzigen. Wird ueber checkLogFlush() regelmaessig
    // aus loop() aufgerufen, sowie gezielt vor espReboot()/factoryReset(),
    // damit kein Log-Eintrag beim Neustart verloren geht.

    // Writes the collected log buffer to flash in one go (instead of an
    // open/write/close round PER LINE, see logToFile()) - this coalesces
    // several flash accesses (and the brief stalls that come with them, see
    // the comment at logLineBuffer in globals.h) into a single one. Called
    // regularly from loop() via checkLogFlush(), and explicitly before
    // espReboot()/factoryReset(), so no log entry is lost on restart.

    void flushLogBuffer() {
        if (logBufferMutex == nullptr) return;

        String textToWrite;
        if (xSemaphoreTake(logBufferMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            textToWrite = logLineBuffer;
            logLineBuffer = "";
            xSemaphoreGive(logBufferMutex);
        }
        lastLogFlushMillis = millis();

        if (textToWrite.isEmpty()) return;

        if (!LittleFS.begin()) {
            if (loggingEnabled) Serial.println("[LOG] LittleFS is not mounted. Log will not be written");
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

        File logFile = LittleFS.open(logFileName, FILE_APPEND);
        if (!logFile) {
            if (loggingEnabled) Serial.println("[LOG] Error opening log file: " + logFileName);
            return;
        }
        logFile.print(textToWrite);
        logFile.close();
    }


    // Aus loop() bei jedem Tick aufgerufen (no-op, solange nichts zu tun
    // ist): spuelt den Log-Puffer, sobald er seit dem letzten Flush
    // LOG_FLUSH_INTERVAL_MS alt ist oder LOG_FLUSH_MAX_BUFFER_BYTES erreicht.

    // Called from loop() on every tick (no-op as long as there's nothing to
    // do): flushes the log buffer once it's LOG_FLUSH_INTERVAL_MS old since
    // the last flush, or has reached LOG_FLUSH_MAX_BUFFER_BYTES.

    void checkLogFlush() {
        if (logBufferMutex == nullptr) return;

        // Laenge nur unter dem Mutex lesen - logLineBuffer wird auch von der
        // NTP-/Rocrail-Sync-Task beschrieben (siehe logToFile()), ein
        // ungeschuetzter Zugriff hier waere derselbe Data Race, den der
        // Mutex eigentlich verhindern soll.

        // Only read the length under the mutex - logLineBuffer is also
        // written from the NTP/Rocrail sync task (see logToFile()), an
        // unprotected access here would be exactly the data race the mutex
        // is meant to prevent.
        size_t bufferedBytes = 0;
        if (xSemaphoreTake(logBufferMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            bufferedBytes = logLineBuffer.length();
            xSemaphoreGive(logBufferMutex);
        }
        if (bufferedBytes == 0) return;

        if (bufferedBytes >= LOG_FLUSH_MAX_BUFFER_BYTES ||
            millis() - lastLogFlushMillis >= LOG_FLUSH_INTERVAL_MS) {
            flushLogBuffer();
        }
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


    // Liest PK_SMOOTH_SECOND mit Migrations-Fallback: wurde es noch nie
    // explizit gespeichert, uebernimmt es den Wert von `stationModeFallback`
    // (klassischer Bahnhofsuhr-Look: schwingender Sekundenzeiger passend zum
    // "wartet auf 12"-Verhalten von stationMode) statt eines festen
    // Literals. Ein Aufruf statt der wiederholten Inline-Formel an mehreren
    // Stellen (siehe presets_manager.h, uhr3.ino, webserver_routes.h), damit
    // die Migrationsregel sich nur an einer Stelle aendern muss.

    // Reads PK_SMOOTH_SECOND with a migration fallback: if it was never
    // explicitly saved, it defaults to the value of `stationModeFallback`
    // (classic station-clock look: a sweeping second hand matching
    // stationMode's "waits at 12" behaviour) instead of a fixed literal. One
    // call instead of the repeated inline formula at several places (see
    // presets_manager.h, uhr3.ino, webserver_routes.h), so the migration
    // rule only needs to change in one spot.

    bool getSmoothSecondPref(bool stationModeFallback) {
        return preferences.getBool(PK_SMOOTH_SECOND, stationModeFallback);
    }

