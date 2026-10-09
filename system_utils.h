#pragma once
    // Systemfunktionen: Tasten, Logging, Reset, Neustart, Hilfsfunktionen
    // Benoetigt globals.h, config.h, prefs_keys.h, declarations.h (vor dieser Datei eingebunden)

    // System functions: buttons, logging, reset, restart, helpers
    // Requires globals.h, config.h, prefs_keys.h, declarations.h (included before this file)


    // Zeilen der Tastermeldungen: Hoehe der Zeilenmitte als Anteil der Displayhoehe (Titel, Zeile 2, Hinweis) und
    // Schriften von gross nach klein - je Zeile gilt die erste, in der der Text auf dem Display Platz hat. Nur
    // Schriften, die ohnehin in der Firmware stecken (Streifen, Zifferblatt-Generator).

    // Rows of the button messages: height of the row centre as a share of the display height (title, line 2,
    // hint) and fonts from large to small - per row the first one in which the text fits on the display is used.
    // Only fonts that are in the firmware anyway (strip, clock face generator).

    const float BUTTON_ROW_Y[3] = { 0.30f, 0.53f, 0.76f };
    const lgfx::IFont* const TEXT_FONTS_BIG[] = { &fonts::FreeSansBold24pt7b, &fonts::FreeSansBold12pt7b, &fonts::DejaVu18, &fonts::Font0, nullptr };
    const lgfx::IFont* const TEXT_FONTS_TITLE[] = { &fonts::FreeSansBold12pt7b, &fonts::DejaVu18, &fonts::Font0, nullptr };
    const lgfx::IFont* const TEXT_FONTS_HINT[] = { &fonts::DejaVu18, &fonts::Font0, nullptr };

    // Eine Textzeile zentriert auf Hoehe yShare (Anteil der Displayhoehe) zeichnen, in der ersten Schrift aus
    // fontList (Ende = nullptr), in der der Text Platz hat: Displaybreite, beim runden Display die Sehne des
    // Kreises auf dieser Hoehe (mit Rand). clear = Zeile vorher schwarz loeschen (Countdown ohne Flackern).

    // Draw a text row centred at height yShare (share of the display height), in the first font from fontList
    // (end = nullptr) in which the text fits: display width, on a round display the chord of the circle at that
    // height (with a margin). clear = blank the row first (countdown without flicker).

    void drawTextRow(lgfx::LovyanGFX& g, float yShare, const String& text, uint16_t color, const lgfx::IFont* const* fontList, bool clear) {
        int w = statusWidth(), h = CLOCK_HEIGHT;
        int y = (int)(h * yShare);
        float usable = w * 0.92f;
        if (displayGeom->round) {
            float r = h / 2.0f, dy = fabsf(y - r) + h * 0.06f; // halbe Zeilenhoehe mitrechnen / count half a row height
            usable = dy < r ? 2.0f * sqrtf(r * r - dy * dy) * 0.9f : 0;
        }
        g.setTextSize(1);
        int chosen = 0;
        for (int i = 0; fontList[i]; i++) {
            chosen = i;
            g.setFont(fontList[i]);
            if (g.textWidth(text) <= usable) break;
        }

        // Passt nur noch Font0 (6 x 8 Pixel, sehr klein): lieber die vorige Schrift bis auf 75 % verkleinern, und wenn
        // auch das nicht reicht, Font0 in Halbschritten bis 3-fach vergroessern.

        // Only Font0 (6 x 8 pixels, very small) is left: rather shrink the previous font down to 75 %, and if that
        // is not enough either, enlarge Font0 in half steps up to 3x.

        if (chosen > 0 && fontList[chosen] == &fonts::Font0) {
            g.setFont(fontList[chosen - 1]);
            float wide = (float)g.textWidth(text);
            if (wide > 0 && usable / wide >= 0.75f) {
                g.setTextSize(usable / wide);
            }
            else {
                g.setFont(&fonts::Font0);
                float wide0 = (float)g.textWidth(text);
                g.setTextSize(wide0 > 0 ? constrain(floorf(usable / wide0 * 2.0f) / 2.0f, 1.0f, 3.0f) : 1.0f);
            }
        }
        if (clear) {
            int fh = g.fontHeight();
            g.fillRect(0, y - fh / 2 - 2, w, fh + 4, TFT_BLACK);
        }
        g.setTextDatum(lgfx::middle_center);
        g.setTextColor(color, TFT_BLACK);
        g.drawString(text, w / 2, y);

        // Zustand wie der restliche Code ihn erwartet / state as the rest of the code expects it

        g.setTextDatum(lgfx::top_left);
        g.setFont(&fonts::Font0);
        g.setTextSize(1);
    }

    // Meldung der Tasterstufen und des Werksresets: schwarzer Hintergrund, Titel und Zeile 2 zentriert in der
    // Farbe der Stufe (gelb = WLAN, rot = Werksreset), Zeile 3 kleiner als Hinweis in hintColor.

    // Message of the button stages and the factory reset: black background, title and line 2 centred in the
    // colour of the stage (yellow = WiFi, red = factory reset), line 3 smaller as a hint in hintColor.

    void showButtonMessage(uint16_t color, const String& title, const String& line2, const String& hint, uint16_t hintColor) {
        DRAW_ON_BOTH_DISPLAYS(
            tft.fillScreen(TFT_BLACK);
            drawTextRow(tft, BUTTON_ROW_Y[0], title, color, TEXT_FONTS_TITLE, false);
            drawTextRow(tft, BUTTON_ROW_Y[1], line2, color, TEXT_FONTS_BIG, false);
            if (hint.length()) drawTextRow(tft, BUTTON_ROW_Y[2], hint, hintColor, TEXT_FONTS_HINT, false);
        );
    }

    // Nur Zeile 2 neu zeichnen (Countdown) - ohne das ganze Bild zu loeschen, also ohne Flackern
    // Redraw only line 2 (countdown) - without clearing the whole screen, so without flicker

    void updateButtonLine(uint16_t color, const String& line2) {
        DRAW_ON_BOTH_DISPLAYS(
            drawTextRow(tft, BUTTON_ROW_Y[1], line2, color, TEXT_FONTS_BIG, true);
        );
    }


    // Taster (BUTTON1 oder Boot-Taster) in Stufen nach Haltedauer: unter 10 s WLAN anzeigen (gruen); 10-20 s
    // gelber Countdown "WiFi Reset", loslassen bricht ab; 20-30 s roter Countdown "Factory Reset", loslassen
    // loescht alle WLAN-Eintraege und startet neu; ab 30 s vollstaendiger Werksreset.

    // Button (BUTTON1 or boot button) in stages by hold time: below 10 s show the WiFi (green); 10-20 s yellow
    // countdown "WiFi Reset", releasing aborts; 20-30 s red countdown "Factory Reset", releasing deletes all WiFi
    // entries and restarts; from 30 s full factory reset.

    void checkButton() {
        if (digitalRead(BUTTON1) == HIGH || digitalRead(BOOT_BUTTON) == LOW) {

            const unsigned long WIFI_RESET_HOLD = 2 * WAIT_10s;
            const unsigned long FACTORY_RESET_HOLD = 3 * WAIT_10s;
            unsigned long pressStart = millis();
            int shownStage = 0;   // 0 = WLAN-Anzeige, 1 = WLAN-Countdown, 2 = Werksreset-Countdown
                                  // 0 = WiFi info, 1 = WiFi countdown, 2 = factory reset countdown
            int shownSecs = -1;
            DEBUG_PRINTLN(String("[BUTTON] Pressed (") + (digitalRead(BUTTON1) == HIGH ? "button" : "boot button") + ")");

            clearTFT();

            // Einmalig Anzeige zeichnen
            // Draw display once

            showWlanCredentials(WiFi.SSID());

            // Blockierender Loop während Button gedrückt
            // Blocking loop while button is pressed

            while (digitalRead(BUTTON1) == HIGH || digitalRead(BOOT_BUTTON) == LOW) {
                unsigned long held = millis() - pressStart;

                if (held >= FACTORY_RESET_HOLD) {
                    DEBUG_PRINTLN("[BUTTON] Held for 30 s - factory reset");
                    factoryReset();
                    return;
                }

                if (held >= WIFI_RESET_HOLD) {
                    int secs = (int)((FACTORY_RESET_HOLD - held + 999) / 1000);
                    if (shownStage != 2) {
                        DEBUG_PRINTLN("[BUTTON] Held for 20 s - WiFi reset armed, factory reset countdown");
                        shownStage = 2;
                        shownSecs = secs;
                        showButtonMessage(TFT_RED, "Factory Reset", "in " + String(secs) + " s", "release: WiFi Reset", TFT_YELLOW);
                    }
                    else if (secs != shownSecs) {
                        shownSecs = secs;
                        updateButtonLine(TFT_RED, "in " + String(secs) + " s");
                    }
                }
                else if (held >= WAIT_10s) {
                    int secs = (int)((WIFI_RESET_HOLD - held + 999) / 1000);
                    if (shownStage != 1) {
                        DEBUG_PRINTLN("[BUTTON] Held for 10 s - WiFi reset countdown");
                        shownStage = 1;
                        shownSecs = secs;
                        showButtonMessage(TFT_YELLOW, "WiFi Reset", "in " + String(secs) + " s", "release: cancel", TFT_DARKGREY);
                    }
                    else if (secs != shownSecs) {
                        shownSecs = secs;
                        updateButtonLine(TFT_YELLOW, "in " + String(secs) + " s");
                    }
                }
                delay(10);
            }

            // Zwischen 20 und 30 s losgelassen: alle WLAN-Eintraege loeschen (wie "Gespeicherte Netzwerke
            // zuruecksetzen" in der Weboberflaeche), "done" zeigen und neu starten

            // Released between 20 and 30 s: delete all WiFi entries (like "Reset Saved Networks" in the web
            // interface), show "done" and restart

            if (shownStage == 2) {
                DEBUG_PRINTLN("[BUTTON] Released - deleting all WiFi entries and restarting");
                showButtonMessage(TFT_YELLOW, "WiFi Reset", "...", "", TFT_YELLOW);
                eraseWiFiConfig();
                showButtonMessage(TFT_YELLOW, "WiFi Reset", "done", "restarting", TFT_DARKGREY);
                delay(WAIT_3s);
                espReboot();
                return;
            }
            if (shownStage == 1) {
                DEBUG_PRINTLN("[BUTTON] Released during the WiFi reset countdown - aborted");
                showButtonMessage(TFT_YELLOW, "WiFi Reset", "cancelled", "", TFT_YELLOW);
                delay(WAIT_1s + WAIT_1s / 2);
            }

            // Button wurde vor 10secs losgelassen → WLAN-Credentials für 3 Sekunden anzeigen
            // Button released before 10 secs → show WLAN credentials for 3 seconds

            if (shownStage == 0) {
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


    // Kurze englische Beschriftung einer per Code zu bestaetigenden Aktion - gemeinsam fuer Display und
    // Web-Eingabeseite (/factoryReset/enterCode), damit beide denselben Text zeigen.

    // Short English label for an action confirmed by code - shared by the display and the web entry page
    // (/factoryReset/enterCode), so both show the same text.

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


    // Erzeugt einen 3-stelligen Bestaetigungscode fuer `action`, merkt sich die Aktion und setzt Anzeige und
    // Fehlversuche zurueck (gezeichnet von checkFactoryResetCodePending()). Der Aufrufer setzt vorher die
    // Nutzlast (pendingWifiChangeIndex bzw. pendingWifiSsid[]/pendingWifiPass[]).

    // Generates a 3-digit confirmation code for `action`, remembers the action and resets display and
    // attempts (drawn by checkFactoryResetCodePending()). The caller sets the payload beforehand
    // (pendingWifiChangeIndex or pendingWifiSsid[]/pendingWifiPass[]).

    void requestConfirmationCode(const String& action) {

        // Noch gueltiger Code anhaengig? Dann nicht ueberschreiben und das Zeitfenster nicht verlaengern -
        // sonst koennten wiederholte Anfragen von aussen die Anzeige dauerhaft blockieren.

        // A still-valid code pending? Then do not overwrite it or extend its window - otherwise repeated
        // requests from outside could block the display permanently.

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


    // Liefert true (mit Hinweisseite), wenn schon ein anderer gueltiger Code aussteht. MUSS vor dem Setzen
    // der Nutzlast aufgerufen werden, sonst wuerde die Nutzlast der wartenden Anfrage ueberschrieben und mit
    // deren Code die FALSCHE Aktion ausgefuehrt. Bei true sofort zurueckkehren.

    // Returns true (with a hint page) if a different valid code is already pending. MUST be called before
    // setting the payload, otherwise the pending request's payload would be overwritten and its code would
    // run the WRONG action. Return immediately on true.

    bool rejectIfConfirmationPending() {
        if (!factoryResetCode.isEmpty() && millis() - factoryResetCodeStartMillis < FACTORY_RESET_CODE_TIMEOUT_MS) {
            DEBUG_PRINTLN("[SECURITY] Request from " + webserver.client().remoteIP().toString() + " ignored - a different confirmation (action: " + factoryResetPendingAction + ") is still pending");
            webserver.send(200, "text/html", simpleMessagePage(translate("Factory&nbsp;Reset"), "<p>" + translate("Another confirmation is already pending. Please complete it or wait a moment and try again") + ".</p>"));
            return true;
        }
        return false;
    }


    // Zeigt den 3-stelligen Bestaetigungscode fuer Werksreset-Aktionen und Aenderungen am aktiven WLAN -
    // Beweis physischen Zugriffs, da nur am Geraet ablesbar. Nicht blockierend: zeichnet einmal und verwirft
    // den Code nach FACTORY_RESET_CODE_TIMEOUT_MS, danach zeichnet updateClock() wieder normal.

    // Shows the 3-digit confirmation code for factory reset actions and changes to the active WiFi - proof of
    // physical access, since it can only be read on the device. Non-blocking: draws once and discards the
    // code after FACTORY_RESET_CODE_TIMEOUT_MS, then updateClock() draws normally again.

    bool checkFactoryResetCodePending() {
        if (factoryResetCode.isEmpty()) return false;

        if (millis() - factoryResetCodeStartMillis > FACTORY_RESET_CODE_TIMEOUT_MS) {
            DEBUG_PRINTLN("[SECURITY] Confirmation code expired unused (action: " + factoryResetPendingAction + ")");
            factoryResetCode = "";
            factoryResetCodeShown = false;
            factoryResetPendingAction = "";
            factoryResetCodeAttempts = 0;
            pendingWifiChangeIndex = -1;

            // Auch pendingWifiSsid[]/pendingWifiPass[] verwerfen, damit ein Klartext-Passwort eines
            // verfallenen Antrags nicht unnoetig im RAM bleibt.

            // Also discard pendingWifiSsid[]/pendingWifiPass[], so a plain-text password of an expired
            // request does not linger in RAM.

            for (int i = 0; i < MAX_WLAN; i++) {
                pendingWifiSsid[i] = "";
                pendingWifiPass[i] = "";
            }
            return false;
        }

        if (!factoryResetCodeShown) {
            factoryResetCodeShown = true;

            // Nur der nackte Code, gross und mittig; welche Aktion bestaetigt wird, zeigt die
            // Web-Eingabeseite. Die "_" davor und danach sind rein optisch - verglichen werden nur die 3
            // Ziffern.

            // Only the bare code, large and centered; the web entry page shows which action is being
            // confirmed. The "_" before and after are purely visual - only the 3 digits are compared.

            String displayCode = "_" + factoryResetCode + "_";

            // Textgroesse: groesstes ganzzahliges Vielfaches der 6x8-Schrift, bei dem der Code (5 Zeichen) in
            // 75 % der Meldungsbreite (statusWidth()) passt - skaliert so auch auf dem 160x160-Display passend.

            // Text size: the largest integer multiple of the 6x8 font at which the code (5 characters) fits
            // into 75 % of the message width (statusWidth()) - so it also scales correctly on the 160x160 display.

            showButtonMessage(TFT_YELLOW, "Code", displayCode, "", TFT_DARKGREY);
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

        showButtonMessage(TFT_RED, "Factory Reset", "...", "", TFT_RED);
        preferences.begin("clock", false);
        preferences.putInt(PK_FIRST_START, 0);
        preferences.end();
        delay(100);
        DEBUG_PRINTLN(">>> Factory reset started..");
        LittleFS.begin(false, "/littlefs", 10, LITTLEFS_PARTITION);
        LittleFS.format();
        LittleFS.end();
        eraseWiFiConfig();
        eraseAllNVS();
        showButtonMessage(TFT_RED, "Factory Reset", "done", "restarting", TFT_DARKGREY);
        delay(WAIT_3s);
        DEBUG_PRINTLN(">>> Restarting..");
        espReboot();
    }


    void espReboot() {

        // Zuerst eine laufende NTP-Sync-Task beenden - sie wuerde sonst nach preferences.end() ueber
        // logToFile() weiter auf Preferences zugreifen. Gilt fuer alle Neustarts (woechentlich, Web-Buttons),
        // die alle hier durchlaufen.

        // First stop a running NTP sync task - it would otherwise keep accessing Preferences via logToFile()
        // after preferences.end(). Applies to all restarts (weekly, web buttons), which all go through here.

        stopNtpSyncTaskIfRunning();

        // Generischer Log-Eintrag fuer jeden Software-Reboot, zentral hier statt
        // an jeder Aufrufstelle. Wird VOR den Display-Aktionen geloggt, damit er
        // sicher im Logfile landet, bevor der ESP neu startet.

        // Generic log entry for every software-triggered reboot, centralized here
        // instead of at each call site. Logged BEFORE the display actions, so it
        // reliably ends up in the log file before the ESP restarts.

        DEBUG_PRINTLN("[SYSTEM] Software-triggered reboot - restarting now..");

        // Gepufferte Log-Zeilen JETZT schreiben, nicht erst beim naechsten checkLogFlush() - sonst gingen sie
        // beim Neustart verloren.

        // Write buffered log lines NOW, not at the next checkLogFlush() - otherwise they would be lost on
        // restart.

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

        showButtonMessage(TFT_GREEN, "Restart", "...", "", TFT_DARKGREY);
        delay(WAIT_3s);
        DRAW_ON_BOTH_DISPLAYS(
            tft.fillScreen(TFT_BLACK);
        );
        delay(100);
        ESP.restart();
    }


    // Log-Funktionen. Aktuelle Logdatei wie in logToFile() bestimmen, aber ohne Seiteneffekte - fuer Log-Tab
    // und /api/currentLog, damit beide dieselbe Datei sehen.

    // Log functions. Determine the current log file like logToFile(), but without side effects - for the Log
    // tab and /api/currentLog, so both see the same file.

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

        // Eigene lokale Zeitstruktur wie in checkWeeklyRestart() (siehe dort). Timeout 0 und keine
        // WLAN-Bedingung - die Zeit ist auch ohne WLAN gueltig, wenn sie von RTC oder DCF77 stammt.

        // Own local time struct, same reason as in checkWeeklyRestart() (see there). Timeout 0 and no WiFi
        // condition - the time is valid without WiFi too when it comes from RTC or DCF77.

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


    // Schreibt den gesammelten Log-Puffer auf einen Rutsch (statt einer Datei-Runde je Zeile) - weniger
    // Flash-Zugriffe und Aussetzer. Aufruf regelmaessig per checkLogFlush() sowie vor
    // espReboot()/factoryReset().

    // Writes the collected log buffer in one go (instead of a file round per line) - fewer flash accesses and
    // stalls. Called regularly via checkLogFlush() and before espReboot()/factoryReset().

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

        if (!LittleFS.begin(false, "/littlefs", 10, LITTLEFS_PARTITION)) {
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
    // ist): leert den Log-Puffer, sobald er seit dem letzten Leeren
    // LOG_FLUSH_INTERVAL_MS alt ist oder LOG_FLUSH_MAX_BUFFER_BYTES erreicht.

    // Called from loop() on every tick (no-op as long as there's nothing to
    // do): flushes the log buffer once it's LOG_FLUSH_INTERVAL_MS old since
    // the last flush, or has reached LOG_FLUSH_MAX_BUFFER_BYTES.

    void checkLogFlush() {
        if (logBufferMutex == nullptr) return;

        // Laenge nur unter dem Mutex lesen - logLineBuffer wird auch von der NTP-/Rocrail-Task beschrieben.
        // Only read the length under the mutex - logLineBuffer is also written by the NTP/Rocrail task.

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


    // Liest PK_SMOOTH_SECOND mit Migrations-Fallback auf stationMode, statt
    // die Formel an mehreren Stellen zu wiederholen (siehe presets_manager.h,
    // uhr4.ino, webserver_routes.h).

    // Reads PK_SMOOTH_SECOND with a migration fallback to stationMode,
    // instead of repeating the formula at several places (see
    // presets_manager.h, uhr4.ino, webserver_routes.h).

    bool getSmoothSecondPref(bool stationModeFallback) {
        return preferences.getBool(PK_SMOOTH_SECOND, stationModeFallback);
    }


