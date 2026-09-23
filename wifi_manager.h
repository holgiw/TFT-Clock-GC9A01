#pragma once
    // WLAN: Verbindungsaufbau, Access-Point, Scan, Reconnect. Benoetigt
    // globals.h, config.h, prefs_keys.h, declarations.h (vor dieser Datei
    // in uhr3.ino eingebunden).

    // WiFi: connection setup, access point, scan, reconnect. Requires
    // globals.h, config.h, prefs_keys.h, declarations.h (included in
    // uhr3.ino before this file).

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


    // Schreibt String und liest ihn sofort zur Verifikation zurueck (erkennt
    // fehlgeschlagene NVS-Schreibvorgaenge). Hier statt in prefs_keys.h, da
    // dort DEBUG_PRINTLN/preferences noch nicht bekannt sind.

    // Writes a string and reads it back immediately to verify (catches
    // failed NVS writes). Implemented here instead of prefs_keys.h since
    // DEBUG_PRINTLN/preferences aren't known there yet.

    bool putStringVerified(const char* key, const String& value) {
        preferences.putString(key, value);
        String readBack = preferences.getString(key, "");
        if (readBack != value) {
            DEBUG_PRINTLN("[Preferences] Verifikation fehlgeschlagen fuer Key '" + String(key) + "'");
            return false;
        }
        return true;
    }


    // Uebernimmt Scan-Ergebnisse in availableNetworks[], behaelt die MAX_WLAN
    // staerksten (absteigend sortiert) - Treiber-Ergebnisse sind NICHT nach
    // Signalstaerke sortiert. 'totalFound' ist die volle Trefferzahl.

    // Takes scan results into availableNetworks[], keeping the MAX_WLAN
    // strongest (sorted descending) - driver results are NOT sorted by
    // signal strength. 'totalFound' is the full hit count.

    void collectStrongestNetworks(int totalFound) {
        for (int i = 0; i < MAX_WLAN; i++) {
            availableNetworks[i].ssid = "";
            availableNetworks[i].rssi = 0;
            availableNetworks[i].enc = 0;
        }
        foundNetworkCount = 0; // wichtig: sonst akkumuliert der Zaehler ueber Scans, Array-Overflow
                               // important: otherwise counter accumulates across scans, array overflow

        if (totalFound < 0) totalFound = 0;

        for (int i = 0; i < totalFound; i++) {
            String scanSsid = WiFi.SSID(i);
            int scanRssi = WiFi.RSSI(i);
            int scanEnc = WiFi.encryptionType(i);

            int slot = -1;
            if (foundNetworkCount < MAX_WLAN) {
                slot = foundNetworkCount++;
            }
            else {
                // schwaechsten Eintrag finden, nur ersetzen wenn staerker
                // find weakest entry, replace only if new one is stronger
                int weakest = 0;
                for (int k = 1; k < MAX_WLAN; k++) {
                    if (availableNetworks[k].rssi < availableNetworks[weakest].rssi) weakest = k;
                }
                if (scanRssi > availableNetworks[weakest].rssi) slot = weakest;
            }

            if (slot < 0) continue;

            availableNetworks[slot].ssid = scanSsid;
            availableNetworks[slot].rssi = scanRssi;
            availableNetworks[slot].enc = scanEnc;
        }

        // Einfuegesortierung reicht fuer maximal MAX_WLAN Eintraege
        // insertion sort is sufficient for at most MAX_WLAN entries
        for (int i = 1; i < foundNetworkCount; i++) {
            WifiNetwork key = availableNetworks[i];
            int j = i - 1;
            while (j >= 0 && availableNetworks[j].rssi < key.rssi) {
                availableNetworks[j + 1] = availableNetworks[j];
                j--;
            }
            availableNetworks[j + 1] = key;
        }

        if (loggingEnabled) {
            for (int i = 0; i < foundNetworkCount; i++) {
                DEBUG_PRINTLN("  [WiFi] " +
                    availableNetworks[i].ssid + " (" +
                    String(availableNetworks[i].rssi) + " dBm) " +
                    (availableNetworks[i].enc == WIFI_AUTH_OPEN ? "Open" : "Secured"));
            }
        }
    }


    // überpüft die WiFi-Verbindung und versucht, sie alle x Minuten wiederherzustellen, wenn sie getrennt ist.
    // Checks the WiFi connection and tries to restore it every x minutes if disconnected.

    bool checkWiFiReconnect() {
        static unsigned long lastAttempt = 0;
        static bool firstAttempt = true; // siehe Kommentar unten
                                         // see comment below

        // Ohne firstAttempt gilt die Bedingung in der ersten Stunde nach Boot
        // immer als "Verbindung OK", ohne je zu verbinden - ein Router-Neustart
        // direkt nach Stromausfall liess die Uhr so bis zu 60 Min offline.

        // Without firstAttempt the condition is always "connection OK" during
        // the first hour after boot, without ever connecting - a router
        // restart right after a power cut left the clock offline for 60 min.
        unsigned long now = millis();
        if (!firstAttempt && (now - lastAttempt < WAIT_1h)) return true;
        firstAttempt = false;
        lastAttempt = now;

        if (WiFi.status() == WL_CONNECTED) {
            return true; // Verbindung ist in Ordnung
                         // connection is OK
        }

        DEBUG_PRINTLN("[WiFi] Disconnected. Attempting reconnect..");
        WiFi.disconnect();
        int slot = preferences.getInt(PK_LAST_WLAN, 0);
        connectWiFiWithRetries(slot, wifiSsid[slot], false);
        return WiFi.status() == WL_CONNECTED;
    }


    // WiFi-Event-Callback fuer die per Web-Button gestartete WPS-Anfrage.
    // Laeuft in einem anderen Thread als loop() - nur Flags setzen, keine
    // Preferences-Zugriffe oder Reconnects hier.

    // WiFi event callback for the web-button WPS request. Runs in a
    // different thread than loop() - only set flags here, no preferences
    // access or reconnects.

    void onWpsEvent(WiFiEvent_t event) {
        // WiFi.SSID()/psk() liefern hier unzuverlaessig die neuen Zugangsdaten
        // (ESP-IDF-Bug #10339) - daher nur Flag setzen, Zugangsdaten werden
        // nach erfolgreicher Verbindung in loop() ausgelesen.

        // WiFi.SSID()/psk() unreliably return the new credentials here
        // (ESP-IDF bug #10339) - so just set a flag, credentials are read
        // in loop() after a successful connection.
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
                    connectWiFiWithRetries(i, wpsPreviousSsid, false);
                    break;
                }
            }
        }
        wpsPreviousSsid = "";
    }


    int saveWpsCredentials(const String& ssid, const String& pass) {
        // Bewusst frisch aus Preferences lesen statt wifiSsid[]: das Array wird
        // nur beim Booten befuellt und koennte bei spaetem WPS-Erfolg nicht mehr
        // aktuell sein - ein belegter Slot koennte faelschlich ueberschrieben werden.

        // Deliberately read fresh from preferences instead of wifiSsid[]: it's
        // only filled at boot and may be stale by the time WPS succeeds - an
        // occupied slot could be wrongly overwritten.
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
                // PK_LAST_WLAN bewusst NICHT setzen: nur speichern/aktualisieren,
                // nicht automatisch als naechstes beim Boot bevorzugen.

                // Deliberately NOT setting PK_LAST_WLAN: only save/update,
                // don't make it preferred at next boot.
                DEBUG_PRINTLN("[WPS] SSID " + ssid + " already known, using slot " + String(i + 1));
                return i;
            }
        }

        // neue SSID: ersten wirklich freien Slot suchen, sonst letzten ueberschreiben
        // new SSID: find first really free slot, otherwise overwrite the last one
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


        // Station-Modus starten, aber NICHT verbinden - ein gleichzeitiger
        // WiFi.begin() wuerde die WPS-Aushandlung stoeren (geteilter Funk).

        // Start station mode, but do NOT connect - a concurrent WiFi.begin()
        // would interfere with WPS negotiation (shared radio).
        WiFi.mode(WIFI_MODE_STA);
        WiFi.disconnect();


        // WPS versuchen, wenn möglich. Die Slot-Suche (leer/zuletzt benutzt)
        // uebernimmt jetzt saveWpsCredentials() nach einem WPS-Erfolg -
        // hier vorab keine mehr noetig.

        // Try WPS if possible. Slot lookup (empty/last used) is now handled
        // by saveWpsCredentials() after a WPS success - no longer needed
        // here beforehand.

        DRAW_ON_BOTH_DISPLAYS(
            tft.fillRect(0, 0, CLOCK_WIDTH, CLOCK_HEIGHT, TFT_BLACK);
            tft.fillScreen(TFT_BLACK);
            tft.setTextColor(TFT_YELLOW, TFT_BLACK);
            tft.setTextSize(TFT_TEXT_SIZE);
            tft.setCursor(10, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 8));
            tft.println("check for WPS..");
        );

        startWPS(); // WPS starten
                    // start WPS

        // Statische Beschriftung einmalig zeichnen; nur die Zahl wird danach
        // pro Sekunde aktualisiert (verhindert Flimmern durch volles Neuzeichnen).

        // Draw the static label once; only the number is updated afterwards
        // each second (prevents flicker from a full redraw).
        int countdownY = CLOCK_HEIGHT / 2;
        int lastSecondsShown = -1;

        int countdownNumX = 0;
        DRAW_ON_BOTH_DISPLAYS(
            tft.setTextColor(TFT_YELLOW, TFT_BLACK);
            tft.setTextSize(TFT_TEXT_SIZE);
            tft.setCursor(10, countdownY);
            tft.print("AP mode in ");
            countdownNumX = tft.getCursorX();
        );

        // 30s waren in der Praxis oft zu knapp fuer eine vollstaendige
        // WPS-Aushandlung - auf 2 Minuten verlaengert, wie beim Web-Button-
        // WPS-Weg (siehe loop() in uhr3.ino).

        // 30s was often too short in practice for a full WPS negotiation -
        // extended to 2 minutes, matching the web-button WPS path (see
        // loop() in uhr3.ino).
        unsigned long wpsTimeoutMs = 2 * WAIT_1m;
        long wpsWaitMillis = millis();

        // WICHTIG: WiFi.status() springt bei WPS-Erfolg in aktuellen
        // arduino-esp32-Versionen NICHT automatisch auf WL_CONNECTED (Regression,
        // siehe arduino-esp32#11705) - daher auf das onWpsEvent()-Flag reagieren.

        // IMPORTANT: WiFi.status() does NOT auto-switch to WL_CONNECTED on WPS
        // success in current arduino-esp32 versions (regression, see
        // arduino-esp32#11705) - so react to the flag set by onWpsEvent().
        while (!wpsSuccessEvent && !wpsFailedEvent && (millis() - wpsWaitMillis) <= wpsTimeoutMs) {
            int secondsLeft = (wpsTimeoutMs - (millis() - wpsWaitMillis)) / 1000;
            if (secondsLeft != lastSecondsShown) {
                lastSecondsShown = secondsLeft;
                DEBUG_PRINTLN("[WPS] waiting... " + String(secondsLeft) + "s left");
                DRAW_ON_BOTH_DISPLAYS(
                    tft.fillRect(countdownNumX, countdownY, CLOCK_WIDTH - countdownNumX, CLOCK_HEIGHT / 8, TFT_BLACK);
                    tft.setCursor(countdownNumX, countdownY);
                    tft.print(secondsLeft);
                    tft.println("s");
                );
            }
            delay(100);
        }
        DEBUG_PRINTLN("[WPS] wait loop exited, success=" + String(wpsSuccessEvent) + ", failed=" + String(wpsFailedEvent));

        if (wpsSuccessEvent) {
            wpsSuccessEvent = false;

            // WiFi.SSID()/psk() sind hier unzuverlaessig (ESP-IDF#10339, siehe
            // onWpsEvent()) - stattdessen per esp_wifi_get_config() mehrfach lesen.

            // WiFi.SSID()/psk() are unreliable here (ESP-IDF#10339, see
            // onWpsEvent()) - read via esp_wifi_get_config() with retries instead.
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

                DRAW_ON_BOTH_DISPLAYS(
                    tft.setCursor(10, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 4));
                    tft.println(newSsid);

                    tft.setCursor(10, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 8));
                    tft.println("found WPS... reboot");
                );

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

        // MAC hier selbst holen: startAP() kann erreicht werden, ohne dass
        // connectWiFi() je lief (kein gespeichertes Netz) - nur dort wurde
        // mac[] bisher befuellt, sonst waere das Passwort auf jeder Uhr gleich.

        // Fetch the MAC here: startAP() can be reached without connectWiFi()
        // ever running (no stored network) - that was the only place filling
        // mac[], otherwise every clock would get the same password.
        WiFi.macAddress(mac);

        // Passwort aus den letzten 4 MAC-Bytes (als Hex = 8 Zeichen, WPA2-Minimum)
        // - pro Geraet verschieden. SSID bleibt fest (AP_SSID); Passwort war
        // frueher mit ihr identisch und damit auf jeder Uhr gleich/bekannt.

        // Password from the last 4 MAC bytes (hex = 8 chars, WPA2 minimum) -
        // different per device. SSID stays fixed (AP_SSID); the password used
        // to be identical to it and thus the same/known on every clock.

        // Kleinbuchstaben (%02x): auf dem Display und beim Abtippen am Handy
        // eindeutiger zu lesen. Der Hostname weiter unten in connectWiFi()
        // bleibt bewusst bei Grossbuchstaben, der ist ein anderer Bezeichner.

        // Lower case (%02x): easier to read on the display and to type on a
        // phone. The hostname further below in connectWiFi() deliberately stays
        // upper case, that is a different identifier.
        snprintf(apPassword, sizeof(apPassword), "%02x%02x%02x%02x",
            mac[2], mac[3], mac[4], mac[5]);

        WiFi.softAP(AP_SSID, apPassword);
        DEBUG_PRINTLN("[WiFi] Started Access Point: " + String(AP_SSID) + " / " + String(apPassword));

        // Captive portal: leite alle DNS-Anfragen auf die AP-IP um
        // Captive portal: redirect all DNS requests to the AP IP
        dnsServer.start(53, "*", WiFi.softAPIP());

        // WLAN-Scan durchführen (asynchron, mit Zeitlimit statt blockierend -
        // ein haengender Scan darf die Uhr nicht dauerhaft aufhalten, der AP
        // laeuft ja bereits).

        // Perform the WiFi scan (asynchronous, with a time limit instead of
        // blocking - a stuck scan must not hold up the clock permanently,
        // the AP is already running).
        DRAW_ON_BOTH_DISPLAYS(
            tft.fillScreen(TFT_BLACK);
            tft.setTextColor(TFT_YELLOW, TFT_BLACK);
            tft.setTextSize(TFT_TEXT_SIZE);
            tft.setCursor(10, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 8));
            tft.println("WLAN-Scan..");
        );
        WiFi.scanNetworks(true);
        int networkCount = WIFI_SCAN_RUNNING;
        unsigned long scanStartMillis = millis();
        while (networkCount == WIFI_SCAN_RUNNING && (millis() - scanStartMillis) < WAIT_10s) {
            delay(100);
            networkCount = WiFi.scanComplete();
        }
        if (networkCount < 0) networkCount = 0; // Zeitlimit/Fehler - als "0 gefunden" behandeln
                                                 // timeout/error - treat as "0 found"

        // availableNetworks füllen - staerkste zuerst, siehe
        // collectStrongestNetworks() weiter oben.

        // Fill availableNetworks - strongest first, see
        // collectStrongestNetworks() further above.
        collectStrongestNetworks(networkCount);

        // Vom Treiber fuer die Scan-Ergebnisse belegten Speicher freigeben -
        // fehlte hier bisher, obwohl checkWiFiScan()/scanAndCacheNetworks()
        // das nach demselben collectStrongestNetworks()-Aufruf konsequent tun.

        // Free the memory the driver allocated for the scan results - this
        // was missing here even though checkWiFiScan()/scanAndCacheNetworks()
        // consistently do it right after the same collectStrongestNetworks() call.
        WiFi.scanDelete();

        clearTFT();

        DRAW_ON_BOTH_DISPLAYS(
            tft.fillScreen(TFT_BLACK);
            tft.setTextColor(TFT_YELLOW, TFT_BLACK);
            tft.setTextSize(TFT_TEXT_SIZE);
            tft.setCursor(10, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 8)) ;
            tft.println("AccessPoint active");
            tft.setCursor(10, (CLOCK_HEIGHT / 2));
            tft.println(String(AP_SSID) + " " + apPassword);
            tft.setCursor(10, (CLOCK_HEIGHT / 2 ) + (CLOCK_HEIGHT / 8));

            tft.print("http://");
            tft.println(WiFi.softAPIP());
        );

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

        // Bereichspruefung: 'number' indiziert wifiSsid[]/wifiPass[] direkt,
        // und mind. ein Aufrufer reicht ihn ungeprueft aus dem NVS durch -
        // ein beschaedigter Wert wuerde sonst hinter das Array-Ende greifen.

        // Range check: 'number' indexes wifiSsid[]/wifiPass[] directly, and at
        // least one caller passes it through unchecked from NVS - a corrupted
        // value would otherwise reach past the end of the array.
        if (number < 0 || number >= MAX_WLAN) {
            DEBUG_PRINTLN("[WiFi] connectWiFi: index out of range (" + String(number) + ")");
            return NOT_CONNECTED;
        }

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
            // Preprocessor-Bedingung vorab in eine Variable aufloesen - #if/#else
            // duerfen nicht innerhalb der Argumentliste von DRAW_ON_BOTH_DISPLAYS() stehen.

            // Resolve the preprocessor condition into a variable beforehand - #if/#else
            // are not allowed inside DRAW_ON_BOTH_DISPLAYS()'s argument list.
#if defined GC9D01
            int versionCursorX = 20;
#else
            int versionCursorX = 60;
#endif
            DRAW_ON_BOTH_DISPLAYS(
                tft.setTextColor(TFT_GREEN, TFT_BLACK);

                tft.setTextSize(TFT_TEXT_SIZE / 2);
                tft.setCursor(versionCursorX, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 3));

                tft.println(String(version));

                tft.setTextSize(TFT_TEXT_SIZE);
                tft.setCursor(20, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 8));
                tft.println("Connect to SSID" + String(number+1));
                tft.setCursor(20, (CLOCK_HEIGHT / 2));

                if (wifiSsid[number].length() > 15) {
                    tft.print(wifiSsid[number].substring(0,15));
                    tft.println("..");
                } else tft.println(wifiSsid[number]);
            );
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
                animateCursor(20, (CLOCK_HEIGHT / 2) + (CLOCK_HEIGHT / 8), 100);
            }
            else {
                updateClock();
                delay(10); // kurze Pause, damit der WLAN-Stack und andere Aufgaben Zeit bekommen
                           // brief pause so the WiFi stack and other tasks get time
            }

        }
        if (loggingEnabled) Serial.println();

        if (WiFi.status() != WL_CONNECTED) {
            DEBUG_PRINTLN("[WiFi] Connection to '" + wifiSsid[number] + "' failed or timed out");
        } else {
            DEBUG_PRINTLN("[WiFi] Connected successfully to '" + wifiSsid[number] + "'");
        }

        if (WiFi.status() == WL_CONNECTED) {

            if (preferences.getInt(PK_LAST_WLAN, -1) != number) {
                preferences.putInt(PK_LAST_WLAN, number);
                DEBUG_PRINTLN("[WiFi] set lastWLan: " +  (String)(number + 1));
            }

            // MDNS.end() zuerst: connectWiFi() laeuft bei jedem Reconnect erneut,
            // ein zweites MDNS.begin() ohne vorheriges end() schlaegt fehl bzw.
            // haengt den HTTP-Dienst doppelt ein. end() auf ungestartetem ist ok.

            // MDNS.end() first: connectWiFi() runs again on every reconnect, a
            // second MDNS.begin() without a prior end() fails resp. registers
            // the HTTP service twice. end() on a never-started instance is fine.
            MDNS.end();

            // Ergebnis von MDNS.begin() merken statt pingHostname fest auf true:
            // es steuert, ob der "hostname.local"-Link ueberhaupt angezeigt wird
            // (Topbar, Statusseite, Display) - sonst fuehrte er ggf. ins Leere.

            // Remember MDNS.begin()'s result instead of hard-coding pingHostname
            // true: it controls whether the "hostname.local" link is shown at
            // all (topbar, status page, display) - otherwise it could lead nowhere.
            pingHostname = MDNS.begin(hostname);
            if (pingHostname) {
                MDNS.addService("http", "tcp", 80); // HTTP-Dienst auf Port 80 bekanntgeben
                                                    // announce the HTTP service on port 80
                DEBUG_PRINTLN("[mDNS] Started, clock reachable at http://" + String(hostname) + ".local");
            }
            else {
                DEBUG_PRINTLN("[mDNS] Error starting mDNS - only the IP address will be offered");
            }

            // NTP-Server neu an Port 123 binden: WiFi.mode(WIFI_MODE_NULL) oben
            // hat den WLAN-Stack heruntergefahren, der in setup() gebundene Socket
            // verlor sein Interface - sonst bleibt der NTP-Server nach Reconnect stumm.

            // Rebind the NTP server to port 123: WiFi.mode(WIFI_MODE_NULL) above
            // shut down the WiFi stack, the socket bound in setup() lost its
            // interface - otherwise the NTP server stays silent after a reconnect.
            startNtpServer();

            // R2RNet-Multicast-Diagnose ebenfalls neu beitreten - derselbe
            // Grund wie bei startNtpServer() direkt darueber: der Socket
            // ueberlebt den WiFi-Neuaufbau nicht (siehe rocrail_client.h).
            // Nur, wenn Rocrail ueberhaupt aktiviert ist (siehe Begruendung
            // beim analogen Aufruf in uhr3.ino).

            // Rejoin the R2RNet multicast diagnostic listener too - same
            // reason as startNtpServer() right above: the socket doesn't
            // survive the WiFi restart (see rocrail_client.h). Only when
            // Rocrail is actually enabled (see the reasoning at the
            // analogous call in uhr3.ino).
            if (rocrailEnabled) {
                startR2rnetDebugListener();
            }

            DEBUG_PRINTLN("[WiFi] Connected to: " + wifiSsid[number]);
            DEBUG_PRINTLN("[WiFi] IP address: " + WiFi.localIP().toString());

            if (verboseMode) {
                showWlanCredentials(wifiSsid[number]);
            }

          //  if (!WiFi.softAPgetStationNum()) updateClock();

            return CONNECTED;
        }

        return NOT_CONNECTED;
    }


    // Versucht connectWiFi() bis zu WIFI_CONNECT_ATTEMPTS mal (config.h).
    // Gemeinsam genutzt von connectWiFiAtBoot() (verboseMode=true) sowie
    // checkWiFiReconnect()/restorePreviousWpsConnection() (beide false).

    // Tries connectWiFi() up to WIFI_CONNECT_ATTEMPTS times (config.h).
    // Shared by connectWiFiAtBoot() (verboseMode=true) and
    // checkWiFiReconnect()/restorePreviousWpsConnection() (both false).

    int connectWiFiWithRetries(int number, const String& label, bool verboseMode) {
        int result = NOT_CONNECTED;
        for (int attempt = 0; attempt < WIFI_CONNECT_ATTEMPTS && result == NOT_CONNECTED; attempt++) {
            if (attempt > 0) {
                DEBUG_PRINTLN("[WiFi] Retrying '" + label + "' (attempt " + String(attempt + 1) + "/" + String(WIFI_CONNECT_ATTEMPTS) + ")..");
            }
            result = connectWiFi(number, verboseMode);
        }
        return result;
    }


    // Animation waehrend Verbindungsversuchen. Kein 'tft'-Parameter, da
    // DRAW_ON_BOTH_DISPLAYS() es intern selbst umbiegt.

    // Animation during connection attempts. No 'tft' parameter, since
    // DRAW_ON_BOTH_DISPLAYS() redirects it internally itself.

    void animateCursor(int x, int y, int delayMs) {
        const char* frames[] = { "/", "-", "\\", "-" };
        for (int i = 0; i < 4; i++) {
            DRAW_ON_BOTH_DISPLAYS(
                tft.setCursor(x, y);
                tft.print(frames[i]);
            );
            delay(delayMs);
        }
    }


    // Anzeige WLAN Parameter auf dem TFT
    // Display WiFi parameters on the TFT

    void showWlanCredentials(String wlan) {
#if defined(GC9D01)
        int versionCursorX = 20;
#else
        int versionCursorX = 60;
#endif
        DRAW_ON_BOTH_DISPLAYS(
            tft.fillScreen(TFT_BLACK);
            tft.setTextColor(TFT_GREEN, TFT_BLACK);

            tft.setTextSize(TFT_TEXT_SIZE/2);
            tft.setCursor(versionCursorX, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 3));
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
        );
    }


    // Loescht gespeicherte WLAN-Zugangsdaten
    // Deletes saved WiFi credentials

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

        // Kein zusaetzliches nvs_erase_all("wifi") noetig: dieser Namespace wird
        // nirgends verwendet (alles laeuft unter "clock") - ein Aufraeumen dort
        // wuerde faelschlich den kompletten "clock"-Namespace treffen.

        // No extra nvs_erase_all("wifi") needed: that namespace is unused
        // (everything runs under "clock") - cleaning it up there would wrongly
        // wipe the entire "clock" namespace.
    }


    // Startet asynchronen WiFi-Scan
    // Starts an asynchronous WiFi scan

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


    // Prüft Scan-Status und verarbeitet Ergebnisse
    // Checks scan status and processes results

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

                DEBUG_PRINTLN("[WiFi] found " + String(scanStatus) + " WiFi networks");
                if (scanStatus > MAX_WLAN && loggingEnabled) {
                    DEBUG_PRINTLN("[WiFi] keeping the " + String(MAX_WLAN) + " strongest:");
                }
                collectStrongestNetworks(scanStatus);

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


    // Scannt WLANs und cached Ergebnisse
    // Scans WiFi networks and caches results

    void scanAndCacheNetworks() {

        DRAW_ON_BOTH_DISPLAYS(
            tft.fillScreen(TFT_BLACK);
            tft.setTextColor(TFT_GREEN, TFT_BLACK);
            tft.setTextSize(TFT_TEXT_SIZE);
            tft.setCursor(10, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 8));
            tft.println("WLAN-Scan..");
        );

        DEBUG_PRINTLN("[WiFi] Scanning for WiFi networks..");
#ifdef LED_BOARD

#endif
        int networkCount = WiFi.scanNetworks();
        DEBUG_PRINTLN("[WiFi] found " + String(networkCount) + " WiFi networks:");
        if (networkCount > MAX_WLAN) {
            DEBUG_PRINTLN("[WiFi] keeping the " + String(MAX_WLAN) + " strongest:");
        }

        collectStrongestNetworks(networkCount);

        WiFi.scanDelete();
        DEBUG_PRINTLN("[WiFi] done");

    }

