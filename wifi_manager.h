#pragma once
    // ### WLAN: Verbindungsaufbau, Access-Point, Scan, Reconnect ##########
    // ### WiFi: connection setup, access point, scan, reconnect ##########
    // Benoetigt globals.h, config.h, prefs_keys.h und declarations.h (werden
    // zentral in uhr3.ino VOR dieser Datei eingebunden).

    // Requires globals.h, config.h, prefs_keys.h and declarations.h
    // (included centrally in uhr3.ino BEFORE this file).

    // WPS-Typ definieren (Push-Button-Methode)
    // Define WPS type (push-button method)
#define ESP_WPS_MODE WPS_TYPE_PBC

    // WPS-Initialisierung
    // WPS initialization
    esp_wps_config_t wps_config = WPS_CONFIG_INIT_DEFAULT(ESP_WPS_MODE);


    // Aktiviert WPS (Push-Button-Methode) am ESP32 und startet den Verbindungsversuch
    // Activates WPS (push-button method) on the ESP32 and starts the connection attempt

    void startWPS() {
        if (esp_wifi_wps_enable(&wps_config) == ESP_OK) {
            if (esp_wifi_wps_start(0) == ESP_OK) {
                DEBUG_PRINTLN("[WPS] WPS started. Please press the WPS button on the router");
            }
            else {
                DEBUG_PRINTLN("[WPS] WPS could not be started");
            }
        }
        else {
            DEBUG_PRINTLN("[WPS] WPS could not be activated");
        }
    }


    // ### Verifiziertes Preferences-Schreiben #############################
    // ### Verified preferences write #############################
    // Schreibt einen String in die Preferences und liest ihn sofort wieder aus,
    // um einen (z.B. durch vollen NVS-Namespace) fehlgeschlagenen Schreibvorgang
    // zu erkennen, statt ihn erst nach einem Neustart als "Eintrag verschwunden"
    // zu bemerken. Gibt true zurueck, wenn der zurueckgelesene Wert dem
    // geschriebenen Wert entspricht.
    // (Hier statt in prefs_keys.h implementiert, da dort weder "preferences"
    // aus globals.h noch das Makro DEBUG_PRINTLN aus config.h bekannt sind -
    // prefs_keys.h wird in uhr3.ino vor beiden eingebunden.)

    // Writes a string to preferences and immediately reads it back to detect a
    // failed write (e.g. full NVS namespace) instead of only noticing a
    // "missing entry" after a reboot. Returns true if the value read back
    // matches the value written.
    // (Implemented here instead of in prefs_keys.h, since neither "preferences"
    // from globals.h nor the DEBUG_PRINTLN macro from config.h are known there -
    // prefs_keys.h is included in uhr3.ino before both.)

    bool putStringVerified(const char* key, const String& value) {
        preferences.putString(key, value);
        String readBack = preferences.getString(key, "");
        if (readBack != value) {
            DEBUG_PRINTLN("[Preferences] Verifikation fehlgeschlagen fuer Key '" + String(key) + "'");
            return false;
        }
        return true;
    }


    // überpüft die WiFi-Verbindung und versucht, sie alle x Minuten wiederherzustellen, wenn sie getrennt ist.
    // Checks the WiFi connection and tries to restore it every x minutes if disconnected.

    bool checkWiFiReconnect() {
        static unsigned long lastAttempt = 0;

        unsigned long now = millis();
        if (now - lastAttempt < WAIT_1h) return true;
        lastAttempt = now;

        String pingServer = preferences.getString(PK_PING_SERVER, "");

        if (WiFi.status() == WL_CONNECTED) {
            if (pingServer == "") {
                return true; // Kein Ping-Server konfiguriert, Verbindung als in Ordnung betrachten
                             // no ping server configured, treat connection as OK
            }
            if (isInternetReachable(pingServer)) {
                return true; // Verbindung ist in Ordnung
                             // connection is OK
            }
            else {
                // DEBUG_PRINTLN("[WiFi] Connected to WiFi but no internet. Attempting reconnect..");
                // return false; // Verbindung hat kein Internet, Reconnect versuchen
            }
        }

        DEBUG_PRINTLN("[WiFi] Disconnected. Attempting reconnect..");
        WiFi.disconnect();
        connectWiFi(preferences.getInt(PK_LAST_WLAN,0), false);
        return WiFi.status() == WL_CONNECTED;
    }


    // Startet einen WLAN-Access-Point mit dem Namen und Passwort 'clock123' und zeigt Verbindungsinformationen auf dem Display an.
    // Speichert die aktuell per WPS verbundenen Zugangsdaten (SSID/Passwort) in
    // einem freien oder passenden Slot. Wird von der asynchronen Web-Button-WPS-
    // Anfrage genutzt (siehe /api/startWPS und loop() in uhr3.ino) - eigenstaendig,
    // um den bestehenden Boot-Zeit-WPS-Code in setup() nicht anzufassen.
    // WiFi-Event-Callback fuer die per Web-Button gestartete WPS-Anfrage. Laeuft
    // laut Arduino-ESP32-Doku in einem ANDEREN Kontext/Thread als loop() - daher
    // hier NUR einfache Flags/Variablen setzen, keine schwereren Operationen wie
    // Preferences-Zugriffe oder Reconnects (die passieren in loop()).

    // Starts a WiFi access point named/password 'clock123' and shows connection info on the display.
    // Saves the credentials (SSID/password) just connected via WPS into a free
    // or matching slot. Used by the asynchronous web-button WPS request (see
    // /api/startWPS and loop() in uhr3.ino) - standalone, so the existing
    // boot-time WPS code in setup() stays untouched.
    // WiFi event callback for the WPS request started via the web button. Per
    // the Arduino-ESP32 docs it runs in a DIFFERENT context/thread than loop() -
    // so only set simple flags/variables here, no heavier operations like
    // preferences access or reconnects (those happen in loop()).

    void onWpsEvent(WiFiEvent_t event) {
        // Weder WiFi.SSID()/WiFi.psk() noch esp_wifi_get_config() liefern an
        // dieser Stelle zuverlaessig die neuen Zugangsdaten (bestaetigter
        // ESP-IDF-Bug, siehe espressif/esp-idf#10339, sowie fehlendes memcpy
        // fuer WPS_ER_SUCCESS im aktuellen arduino-esp32-Core). Nur ein Flag
        // setzen - die eigentliche Verbindung (und das zuverlaessige Auslesen
        // von SSID/Passwort NACH erfolgreicher Verbindung) passiert in loop().

        // Neither WiFi.SSID()/WiFi.psk() nor esp_wifi_get_config() reliably
        // return the new credentials at this point (confirmed ESP-IDF bug, see
        // espressif/esp-idf#10339, plus a missing memcpy for WPS_ER_SUCCESS in
        // the current arduino-esp32 core). Just set a flag - the actual
        // connection (and reliably reading SSID/password AFTER a successful
        // connection) happens in loop().
        if (event == ARDUINO_EVENT_WPS_ER_SUCCESS) {
            wpsSuccessEvent = true;
        }
        else if (event == ARDUINO_EVENT_WPS_ER_FAILED || event == ARDUINO_EVENT_WPS_ER_TIMEOUT) {
            wpsFailedEvent = true;
        }
    }


    // Stellt nach einem fehlgeschlagenen/abgebrochenen WPS-Versuch die zuvor
    // bestehende Verbindung wieder her, falls diese dadurch getrennt wurde -
    // gemeinsame Logik der beiden Fehlerpfade (Fail-Event/Timeout) in loop().

    // Restores the previously existing connection after a failed/aborted WPS
    // attempt if it got disconnected - shared logic for the two error paths
    // (fail event/timeout) in loop().

    void restorePreviousWpsConnection() {
        if (wpsPreviousSsid != "" && !WiFi.isConnected()) {
            for (int i = 0; i < MAX_WLAN; i++) {
                if (wifiSsid[i] == wpsPreviousSsid) {
                    connectWiFi(i, false);
                    break;
                }
            }
        }
        wpsPreviousSsid = "";
    }


    int saveWpsCredentials(const String& ssid, const String& pass) {
        // Bewusst frisch aus den Preferences lesen statt dem In-Memory-Array
        // wifiSsid[] zu vertrauen: dieses wird nur beim Booten befuellt und
        // koennte zum Zeitpunkt eines WPS-Erfolgs (Geraet laeuft ggf. schon
        // laenger) nicht mehr exakt mit den tatsaechlich gespeicherten
        // Netzwerken uebereinstimmen - das koennte sonst dazu fuehren, dass
        // ein bereits belegter Slot faelschlich als frei erkannt und
        // ueberschrieben wird.

        // Deliberately read fresh from preferences instead of trusting the
        // in-memory array wifiSsid[]: it's only filled at boot and might no
        // longer match the actually stored networks by the time WPS succeeds
        // (device may have been running for a while) - otherwise an already
        // used slot could be wrongly detected as free and overwritten.
        for (int i = 0; i < MAX_WLAN; i++) {
            String storedSsid = preferences.getString(pkSsid(i).c_str(), "");
            if (storedSsid == ssid) {
                String storedPass = preferences.getString(pkPass(i).c_str(), "");
                if (storedPass != pass) {
                    preferences.putString(pkPass(i).c_str(), pass);
                    wifiPass[i] = pass;
                    DEBUG_PRINTLN("[WPS] Password for " + ssid + " differed from stored value - updated");
                }
                else {
                    DEBUG_PRINTLN("[WPS] Password for " + ssid + " unchanged");
                }
                wifiSsid[i] = ssid; // In-Memory-Array synchron halten
                                    // keep in-memory array in sync
                // PK_LAST_WLAN bewusst NICHT setzen: diese Funktion soll das
                // Netzwerk nur speichern/aktualisieren, nicht als naechstes
                // beim Boot bevorzugt versucht werden lassen (siehe /api/startWPS
                // in webserver_routes.h - nur ein Eintrag hinzufuegen, nicht
                // automatisch dorthin wechseln).

                // Deliberately NOT setting PK_LAST_WLAN: this function should only
                // save/update the network, not make it the preferred one tried
                // next at boot (see /api/startWPS in webserver_routes.h - only
                // add an entry, don't switch to it automatically).
                DEBUG_PRINTLN("[WPS] SSID " + ssid + " already known, using slot " + String(i + 1));
                return i;
            }
        }

        // Neue SSID: ersten WIRKLICH freien Slot suchen (frisch aus den
        // Preferences gelesen), sonst letzten Slot ueberschreiben.

        // New SSID: look for the first REALLY free slot (read fresh from
        // preferences), otherwise overwrite the last slot.
        int freeIdx = -1;
        for (int i = 0; i < MAX_WLAN; i++) {
            if (preferences.getString(pkSsid(i).c_str(), "") == "") { freeIdx = i; break; }
        }
        if (freeIdx == -1) freeIdx = MAX_WLAN - 1;

        preferences.putString(pkSsid(freeIdx).c_str(), ssid);
        preferences.putString(pkPass(freeIdx).c_str(), pass);
        // PK_LAST_WLAN bewusst NICHT setzen (siehe Kommentar oben).
        // PK_LAST_WLAN deliberately NOT set (see comment above).
        wifiSsid[freeIdx] = ssid;
        wifiPass[freeIdx] = pass;
        DEBUG_PRINTLN("[WPS] Saved new network " + ssid + " in slot " + String(freeIdx + 1));
        return freeIdx;
    }


    void startAP() {
#ifdef TFT_Backlight
        ledcWrite(TFT_Backlight, 255);
#endif


        // WLAN im Station-Modus starten, aber NICHT verbinden - ein
        // gleichzeitiger Verbindungsversuch (WiFi.begin()) kann die WPS-
        // Aushandlung stoeren/verzoegern, da beide sich den Funk teilen.

        // Start WiFi in station mode, but do NOT connect - a concurrent
        // connection attempt (WiFi.begin()) can interfere with/delay the
        // WPS negotiation, since both compete for the radio.
        WiFi.mode(WIFI_MODE_STA);
        WiFi.disconnect();


        // WPS versuchen, wenn möglich. Die Slot-Suche (leer/zuletzt benutzt)
        // uebernimmt jetzt saveWpsCredentials() nach einem WPS-Erfolg -
        // hier vorab keine mehr noetig.

        // Try WPS if possible. Slot lookup (empty/last used) is now handled
        // by saveWpsCredentials() after a WPS success - no longer needed
        // here beforehand.

        //setCS1(LOW);
        tft.fillRect(0, 0, CLOCK_WIDTH, CLOCK_HEIGHT, TFT_BLACK);
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.setTextSize(TFT_TEXT_SIZE);
        tft.setCursor(10, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 8));
        tft.println("check for WPS..");

        startWPS(); // WPS starten
                    // start WPS

        // Statische Beschriftung einmalig zeichnen; nur die Zahl wird danach
        // pro Sekunde aktualisiert (verhindert Flimmern durch volles Neuzeichnen).

        // Draw the static label once; only the number is updated afterwards
        // each second (prevents flicker from a full redraw).
        int countdownY = CLOCK_HEIGHT / 2;
        int lastSecondsShown = -1;

        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.setTextSize(TFT_TEXT_SIZE);
        tft.setCursor(10, countdownY);
        tft.print("AP mode in ");
        int countdownNumX = tft.getCursorX();

        // 30s waren in der Praxis oft zu knapp fuer eine vollstaendige
        // WPS-Aushandlung - auf 2 Minuten verlaengert, wie beim Web-Button-
        // WPS-Weg (siehe loop() in uhr3.ino).

        // 30s was often too short in practice for a full WPS negotiation -
        // extended to 2 minutes, matching the web-button WPS path (see
        // loop() in uhr3.ino).
        unsigned long wpsTimeoutMs = 2 * WAIT_1m;
        long wpsWaitMillis = millis();

        // WICHTIG: WiFi.status() wechselt bei erfolgreichem WPS in aktuellen
        // arduino-esp32-Versionen NICHT automatisch auf WL_CONNECTED (bestaetigte
        // Regression, siehe espressif/arduino-esp32#11705) - der Kern verbindet
        // nach dem WPS_ER_SUCCESS-Event nicht mehr von selbst. Deshalb hier auf
        // das von onWpsEvent() gesetzte Flag reagieren statt auf WiFi.status()
        // zu pollen (identisches Prinzip wie im Web-Button-WPS-Weg in loop()).

        // IMPORTANT: WiFi.status() does NOT automatically switch to WL_CONNECTED
        // on WPS success in current arduino-esp32 versions (confirmed regression,
        // see espressif/arduino-esp32#11705) - the core no longer auto-connects
        // after the WPS_ER_SUCCESS event. So react to the flag set by onWpsEvent()
        // instead of polling WiFi.status() (same principle as the web-button WPS
        // path in loop()).
        while (!wpsSuccessEvent && !wpsFailedEvent && (millis() - wpsWaitMillis) <= wpsTimeoutMs) {
            int secondsLeft = (wpsTimeoutMs - (millis() - wpsWaitMillis)) / 1000;
            if (secondsLeft != lastSecondsShown) {
                lastSecondsShown = secondsLeft;
                DEBUG_PRINTLN("[WPS] waiting... " + String(secondsLeft) + "s left");
                tft.fillRect(countdownNumX, countdownY, CLOCK_WIDTH - countdownNumX, CLOCK_HEIGHT / 8, TFT_BLACK);
                tft.setCursor(countdownNumX, countdownY);
                tft.print(secondsLeft);
                tft.println("s");
            }
            delay(100);
        }
        DEBUG_PRINTLN("[WPS] wait loop exited, success=" + String(wpsSuccessEvent) + ", failed=" + String(wpsFailedEvent));

        if (wpsSuccessEvent) {
            wpsSuccessEvent = false;

            // WiFi.SSID()/WiFi.psk() liefern an dieser Stelle unzuverlaessig die
            // neuen Zugangsdaten (ESP-IDF#10339) - stattdessen wie im Web-Button-
            // Weg mehrfach per esp_wifi_get_config() versuchen.

            // WiFi.SSID()/WiFi.psk() unreliably return the new credentials here
            // (ESP-IDF#10339) - instead, retry via esp_wifi_get_config() several
            // times, same as the web-button path.
            String newSsid = "";
            String newPass = "";
            for (int wpsReadAttempt = 0; wpsReadAttempt < 20 && newSsid == ""; wpsReadAttempt++) {
                wifi_config_t wpsResultConfig;
                if (esp_wifi_get_config(WIFI_IF_STA, &wpsResultConfig) == ESP_OK) {
                    char ssidBuf[33] = { 0 };
                    char passBuf[65] = { 0 };
                    memcpy(ssidBuf, wpsResultConfig.sta.ssid, sizeof(wpsResultConfig.sta.ssid));
                    memcpy(passBuf, wpsResultConfig.sta.password, sizeof(wpsResultConfig.sta.password));
                    newSsid = String(ssidBuf);
                    newPass = String(passBuf);
                }
                if (newSsid == "") delay(100);
            }

            if (newSsid != "") {
                DEBUG_PRINTLN("[WPS] Connected to the network!");
                DEBUG_PRINTLN("[WPS] SSID: " + newSsid);

                int savedSlot = saveWpsCredentials(newSsid, newPass);
                preferences.putInt(PK_LAST_WLAN, savedSlot);

                tft.setCursor(10, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 4));
                tft.println(newSsid);

                tft.setCursor(10, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 8));
                tft.println("found WPS... reboot");

                delay(WAIT_5s);
                esp_wifi_wps_disable();
                espReboot();
            }
            else {
                DEBUG_PRINTLN("[WPS] Success event received, but SSID could not be read back - falling back to AP mode");
            }
        }
        else if (wpsFailedEvent) {
            wpsFailedEvent = false;
            DEBUG_PRINTLN("[WPS] WPS failed or timed out (event)");
        }

        // WPS deaktivieren und der WLAN-Firmware kurz Zeit geben, sich nach
        // dem abgebrochenen Handshake zu beruhigen, bevor der naechste
        // WLAN-Befehl (Scan) folgt - sonst kann der Scan haengen bleiben.

        // Disable WPS and give the WiFi firmware a brief moment to settle
        // after the aborted handshake before issuing the next WiFi command
        // (scan) - otherwise the scan can hang.
        esp_wifi_wps_disable();
        delay(WAIT_1s);

        // Access Point zuerst starten, damit die Uhr in jedem Fall (auch
        // falls der anschliessende Scan haengt/fehlschlaegt) per WPS-Retry
        // oder Weboberflaeche erreichbar wird.

        // Start the access point first, so the clock becomes reachable via
        // WPS retry or the web interface in any case (even if the scan
        // below hangs/fails).
        WiFi.softAP("clock123", "clock123");
        DEBUG_PRINTLN("[WiFi] Started Access Point: clock123");

        // Captive portal: leite alle DNS-Anfragen auf die AP-IP um
        // Captive portal: redirect all DNS requests to the AP IP
        dnsServer.start(53, "*", WiFi.softAPIP());

        // WLAN-Scan durchführen (asynchron, mit Zeitlimit statt blockierend -
        // ein haengender Scan darf die Uhr nicht dauerhaft aufhalten, der AP
        // laeuft ja bereits).

        // Perform the WiFi scan (asynchronous, with a time limit instead of
        // blocking - a stuck scan must not hold up the clock permanently,
        // the AP is already running).
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.setTextSize(TFT_TEXT_SIZE);
        tft.setCursor(10, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 8));
        tft.println("WLAN-Scan..");
        WiFi.scanNetworks(true);
        int networkCount = WIFI_SCAN_RUNNING;
        unsigned long scanStartMillis = millis();
        while (networkCount == WIFI_SCAN_RUNNING && (millis() - scanStartMillis) < WAIT_10s) {
            delay(100);
            networkCount = WiFi.scanComplete();
        }
        if (networkCount < 0) networkCount = 0; // Zeitlimit/Fehler - als "0 gefunden" behandeln
                                                 // timeout/error - treat as "0 found"

        // availableNetworks füllen
        // Fill availableNetworks
        foundNetworkCount = 0;
        for (int i = 0; i < networkCount && i < MAX_WLAN; i++) {
            availableNetworks[i].ssid = WiFi.SSID(i);
            availableNetworks[i].rssi = WiFi.RSSI(i);
            availableNetworks[i].enc = WiFi.encryptionType(i);
            foundNetworkCount++;
        }

        clearTFT();

        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.setTextSize(TFT_TEXT_SIZE);
        tft.setCursor(10, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 8)) ;
        tft.println("AccessPoint active");
        tft.setCursor(10, (CLOCK_HEIGHT / 2));
        tft.println("clock123 clock123");
        tft.setCursor(10, (CLOCK_HEIGHT / 2 ) + (CLOCK_HEIGHT / 8));

        tft.print("http://");
        tft.println(WiFi.softAPIP());

        softAPIP = true;
        softAPIPstart = millis();

        ipAddress = WiFi.softAPIP().toString();

    }


    // Versucht, eine Verbindung zum WLAN herzustellen, basierend auf den gespeicherten SSID- und Passwort-Paaren.
    // Zeigt während des Verbindungsversuchs Informationen auf dem Display an und überprüft die
    // Internet-Konnektivität nach erfolgreicher Verbindung.

    // Tries to connect to WiFi using the stored SSID/password pairs. Shows
    // connection info on the display while connecting and checks internet
    // connectivity after a successful connection.

    int connectWiFi(int number, bool verboseMode) {
#ifdef TFT_Backlight
        if (verboseMode) {
            ledcWrite(TFT_Backlight, 255);
        }
#endif
        if (wifiSsid[number] == "") {
           // DEBUG_PRINTLN("[WiFi] SSID " + String(number + 1) + " is empty, skipping");
            return NOT_CONNECTED;
        }




        // DEBUG_PRINTLN("[WiFi] Trying SSID" + String(number+1) + ": " + wifiSsid[number]);

        // Wenn verboseMode aktiviert ist, zeige die Verbindungsinformationen auf dem Display an
        // If verboseMode is enabled, show connection info on the display
        if (verboseMode) {
            clearTFT();
            tft.setTextColor(TFT_GREEN, TFT_BLACK);

            tft.setTextSize(TFT_TEXT_SIZE / 2);
#if defined GC9D01
            tft.setCursor(20, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 3));
#else
            tft.setCursor(60, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 3));
#endif


            tft.println(String(version));

            tft.setTextSize(TFT_TEXT_SIZE);
            tft.setCursor(20, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 8));
            tft.println("Connect to SSID" + String(number+1));
            tft.setCursor(20, (CLOCK_HEIGHT / 2));

            if (wifiSsid[number].length() > 15) {
                tft.print(wifiSsid[number].substring(0,15));
                tft.println("..");
            } else tft.println(wifiSsid[number]);
        }

        DEBUG_PRINTLN("[WiFi] Connect to: " + wifiSsid[number]);

        WiFi.disconnect();
        WiFi.mode(WIFI_MODE_NULL);

        WiFi.mode(WIFI_STA);
        // MAC-Adresse holen
        // Get MAC address
        WiFi.macAddress(mac);

        String customHostname = preferences.getString(PK_HOSTNAME, "");
        if (customHostname.length() > 0) {
            customHostname.toCharArray(hostname, sizeof(hostname));
        }
        else {
            snprintf(hostname, sizeof(hostname), "clock_%02X%02X%02X",
                mac[3], mac[4], mac[5]);
        }
        WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
        WiFi.setHostname(hostname);
        DEBUG_PRINTLN("[WiFi] Hostname set to: " + String(hostname));

        uint16_t waitTime = WAIT_30s; // 30 Sekunden
                                      // 30 seconds
        if (rtcOk == RTC_AVAILABLE) {
            waitTime = WAIT_15s; // 15 Sekunden
                                 // 15 seconds
        }


        DEBUG_PRINTLN("[WiFi] Attempting connection with a timeout of " + String(waitTime / 1000) + " seconds..");
        WiFi.begin(wifiSsid[number].c_str(), wifiPass[number].c_str());
        unsigned long start = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - start < waitTime) {

            if (loggingEnabled) Serial.print("");
            if (verboseMode) {
                animateCursor(tft, 20, (CLOCK_HEIGHT / 2) + (CLOCK_HEIGHT / 8), 100);
            }
            else {
                updateClock();
                delay(10); // kurze Pause, damit der WLAN-Stack und andere Aufgaben Zeit bekommen
                           // brief pause so the WiFi stack and other tasks get time
            }

        }
        if (loggingEnabled) Serial.println();

        if (WiFi.status() != WL_CONNECTED) {
            DEBUG_PRINTLN("[WiFi] Connection failed or timed out");
        } else {
            DEBUG_PRINTLN("[WiFi] Connected successfully");
        }

        if (WiFi.status() == WL_CONNECTED) {

            // mDNS initialisieren
            // Initialize mDNS
            if (!MDNS.begin(hostname)) {
                DEBUG_PRINTLN("[WIFI] Error starting mDNS");
            }
            else {
               MDNS.addService("http", "tcp", 80); // Beispiel: HTTP-Dienst auf Port 80
                                                   // example: HTTP service on port 80
            }

            DEBUG_PRINTLN("[WiFi] Connected to: " + wifiSsid[number]);
            DEBUG_PRINTLN("[WiFi] IP address: " + WiFi.localIP().toString());
            String fullHostname = String(hostname) + ".local";
            pingHostname = true;
            //   pingHostname = Ping.ping(fullHostname.c_str(),3);
            //   DEBUG_PRINTF("[mDNS] Ping to %s: %s\n", fullHostname.c_str(), pingHostname ? "success" : "failed");


            String pingServer = preferences.getString(PK_PING_SERVER, "");
            if (pingServer == "") {
                DEBUG_PRINTLN("[WiFi] No ping server configured, skipping internet connectivity check");
                return CONNECTED; // Kein Ping-Server konfiguriert, Verbindung als in Ordnung betrachten
                                  // no ping server configured, treat connection as OK
            }
            if (!isInternetReachable(pingServer)) {
                // Setze eine Statusvariable oder führe eine Aktion aus, wenn das Internet nicht erreichbar ist
                // Set a status variable or take action if the internet is unreachable
                DEBUG_PRINTLN("[WiFi] Internet not reachable after connection");
                return CONNECTED_NO_INTERNET;
            }

            if (verboseMode) {
                showWlanCredentials(wifiSsid[number]);
            }

            if (preferences.getInt(PK_LAST_WLAN, -1) != number) {
                preferences.putInt(PK_LAST_WLAN, number);
                DEBUG_PRINTLN("[WiFi] set lastWLan: " +  (String)(number + 1));
            }

          //  if (!WiFi.softAPgetStationNum()) updateClock();

            return CONNECTED;
        }

        return NOT_CONNECTED;
    }


    // Prüft die Internet-Konnektivität durch Verbindungsversuch zu einem konfigurierten Server.
    // Checks internet connectivity by attempting to connect to a configured server.

    bool isInternetReachable(String pingServer) {

        if (WiFi.status() != WL_CONNECTED) {
            DEBUG_PRINTLN("[PING] WiFi not connected, skipping ping");
            return false;
        }

        WiFiClient client;

        int pingPort = 80; // Standardport
                           // default port

    if (pingServer.indexOf(':') != -1) {
        pingPort = pingServer.substring(pingServer.indexOf(':') + 1).toInt();
        pingServer = pingServer.substring(0, pingServer.indexOf(':'));
    }
    // DEBUG_PRINTLN("[PING] Checking internet connectivity by connect to " + pingServer + ":" + pingPort); client.setTimeout(1);
    bool connected = client.connect(pingServer.c_str(), pingPort);
        if (connected) {
            DEBUG_PRINTLN("[PING] Successfully connected to " + pingServer + ":" + pingPort + " internet is reachable");
        } else {
            DEBUG_PRINTLN("[PING] Failed to connect to " + pingServer + ":" + pingPort);
        }

        client.stop();
        return connected;
    }


    // Animation während Verbindungsversuchen
    // Animation during connection attempts

    void animateCursor(TFT_eSPI& tft, int x, int y, int delayMs) {
        tft.setCursor(x, y);    tft.print("/");    delay(delayMs);
        tft.setCursor(x, y);    tft.print("-");    delay(delayMs);
        tft.setCursor(x, y);    tft.print("\\");   delay(delayMs);
        tft.setCursor(x, y);    tft.print("-");    delay(delayMs);
    }


    // Anzeige WLAN Parameter auf dem TFT
    // Display WiFi parameters on the TFT

    void showWlanCredentials(String wlan) {
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_GREEN, TFT_BLACK);

        tft.setTextSize(TFT_TEXT_SIZE/2);
#if defined(GC9D01)
        tft.setCursor(20, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 3));
#else
        tft.setCursor(60, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 3));
#endif
        tft.println(String(version));

        tft.setTextSize(TFT_TEXT_SIZE);
        tft.setCursor(14, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 4));
        if (WiFi.status() == WL_CONNECTED) {
            tft.println("Connected to SSID" + String(preferences.getInt(PK_LAST_WLAN, -1) + 1));
            tft.setCursor(20, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 8));
            if (wlan.length() > 15) {
                tft.print(wlan.substring(0, 15));
                tft.println("..");
            }
            else tft.println(wlan);
            tft.setCursor(20, (CLOCK_HEIGHT / 2));
            tft.println(WiFi.localIP());

            if (pingHostname) {
                tft.setCursor(20, (CLOCK_HEIGHT / 2) + (CLOCK_HEIGHT / 8));
                tft.println(String(hostname) + ".local");
            }
        }
        else {
            tft.println("Not connected");
        }
    }


    // --- Funktion: Löscht die gespeicherten WiFi-Konfigurationen ---
    // --- Function: deletes the saved WiFi configurations ---

    void eraseWiFiConfig() {
        // WLAN trennen und komplett deaktivieren
        // Disconnect WiFi and turn it off completely
        WiFi.disconnect(true, true);  // true,true => auch gespeicherte Daten löschen
                                      // true,true => also erase saved data
        delay(100);
        WiFi.mode(WIFI_OFF);
        delay(WAIT_1s);

        for (int i = 0; i < MAX_WLAN; i++) {
            // Dynamisch berechnete Schlüssel
            // Dynamically computed keys
            String ssidKey = pkSsid(i);
            String passKey = pkPass(i);

            preferences.remove(ssidKey.c_str());
            preferences.remove(passKey.c_str());
        }

        // Zusätzlich: Manuell NVS-Einträge für WiFi löschen
        // Additionally: manually erase NVS entries for WiFi
        nvs_handle_t nvsHandle;
        esp_err_t err = nvs_open("wifi", NVS_READWRITE, &nvsHandle);
        if (err == ESP_OK) {
            nvs_erase_all(nvsHandle);
            nvs_commit(nvsHandle);
            nvs_close(nvsHandle);
            DEBUG_PRINTLN("WiFi NVS entries erased");
        }
        else {
            DEBUG_PRINTF("Error opening the NVS WiFi handle: %s\n", esp_err_to_name(err));
        }
    }


    // --- Funktion: Startet einen asynchronen WiFi-Scan ---
    // --- Function: starts an asynchronous WiFi scan ---

    void startWiFiScan() {
        if (!isScanning) {

            isScanning = true;

            WiFi.mode(WIFI_STA);
            WiFi.disconnect();
            delay(10);
            DEBUG_PRINTLN("[WiFi] Starting asynchronous scan..");
            WiFi.scanNetworks(true); // Asynchroner Scan
                                     // asynchronous scan
            //delay(250);
        }
    }


    // --- Funktion: Überprüft den Status des WiFi-Scans und verarbeitet die Ergebnisse ---
    // --- Function: checks the WiFi scan status and processes the results ---

    void checkWiFiScan() {
        if (isScanning) {
            int scanStatus = WiFi.scanComplete();
            if (scanStatus == WIFI_SCAN_RUNNING) {
                // Scan läuft noch
                // Scan still running
                // DEBUG_PRINTLN("[WiFi] Scan in progress..");
            }
            else if (scanStatus >= 0) {
                // Scan abgeschlossen
                // Scan complete

                // Vorherige Ergebnisse löschen
                // Clear previous results
                for (int i = 0; i < MAX_WLAN; i++) {
                    availableNetworks[i].ssid = "";
                    availableNetworks[i].rssi = 0;
                    availableNetworks[i].enc = 0;
                }
                foundNetworkCount = 0; // WICHTIG: zuruecksetzen, sonst summiert sich der Zaehler ueber mehrere Scans auf und /api/scanwifi liest ueber das availableNetworks[MAX_WLAN]-Array hinaus (Absturz/leere Anzeige)
                // IMPORTANT: reset here, otherwise the counter accumulates across scans and /api/scanwifi reads past the availableNetworks[MAX_WLAN] array (crash/empty display)

                int16_t n = scanStatus;
                DEBUG_PRINTLN("[WiFi] found " + String(n) + " WiFi networks");
                if (n > MAX_WLAN) {
                    if (loggingEnabled) DEBUG_PRINTLN("[WiFi] here the best " + String(MAX_WLAN) + " networks:");
                    n = MAX_WLAN;
                }

                for (int i = 0; i < n; i++) {
                    availableNetworks[i].ssid = WiFi.SSID(i);
                    availableNetworks[i].rssi = WiFi.RSSI(i);
                    availableNetworks[i].enc = WiFi.encryptionType(i);
                    foundNetworkCount++;
                    if (loggingEnabled) {
                        DEBUG_PRINTLN("  [WiFi] " +
                            availableNetworks[i].ssid + " (" +
                            String(availableNetworks[i].rssi) + " dBm) " +
                            (availableNetworks[i].enc == WIFI_AUTH_OPEN ? "Open" : "Secured"));
                    }

                }
                WiFi.scanDelete(); // Ergebnisse löschen
                                   // clear results
                isScanning = false;
                DEBUG_PRINTLN("[WiFi] done");
            }
            else {
                // Fehler beim Scan
                // Error during scan
                DEBUG_PRINTLN("[WiFi] Scan failed with error: " + String(scanStatus));
                isScanning = false;
                //scanAndCacheNetworks();
            }
        }
    }


    // Blockiert, bis ein laufender WiFi-Scan abgeschlossen ist (siehe checkWiFiScan()) -
    // gemeinsame Warteschleife der beiden Scan-Aufrufe in setup().

    // Blocks until a running WiFi scan finishes (see checkWiFiScan()) - shared
    // wait loop for the two scan calls in setup().

    void waitForWifiScan(int delayMs) {
        while (isScanning) {
            checkWiFiScan();
            delay(delayMs);
            if (loggingEnabled) Serial.print("");
        }
        if (loggingEnabled) Serial.println("");
    }


    // --- Funktion: Scannt verfügbare WLANs und speichert sie im Cache ---
    // --- Function: scans available WiFi networks and caches them ---

    void scanAndCacheNetworks() {

        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.setTextSize(TFT_TEXT_SIZE);
        tft.setCursor(10, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 8));
        tft.println("WLAN-Scan..");

        DEBUG_PRINTLN("[WiFi] Scanning for WiFi networks..");
#ifdef LED_BOARD

#endif
        int networkCount = WiFi.scanNetworks();
        DEBUG_PRINTLN("[WiFi] found " + String(networkCount) + " WiFi networks:");
        if (networkCount > MAX_WLAN) {
            DEBUG_PRINTLN("[WiFi] here the best " + String(MAX_WLAN) + " networks:");
            networkCount = MAX_WLAN;
        }

        foundNetworkCount = 0;
        for (int i = 0; i < networkCount; i++) {
            availableNetworks[i].ssid = WiFi.SSID(i);
            availableNetworks[i].rssi = WiFi.RSSI(i);
            availableNetworks[i].enc = WiFi.encryptionType(i);
            foundNetworkCount++;

            DEBUG_PRINTLN("  [WiFi] " + availableNetworks[i].ssid +
                          " (" + String(availableNetworks[i].rssi) + " dBm) " +
                          (availableNetworks[i].enc == WIFI_AUTH_OPEN ? "Open" : "Secured"));
        }

        WiFi.scanDelete(); // Ergebnisse löschen
                           // clear results
        DEBUG_PRINTLN("[WiFi] done");

    }
