#pragma once
#include <nvs.h>
#include <mbedtls/gcm.h>
#include <esp_random.h>
    // WLAN: Verbindungsaufbau, Access-Point, Scan, Reconnect. Benoetigt
    // globals.h, config.h, prefs_keys.h, declarations.h (vor dieser Datei
    // in uhr4.ino eingebunden).

    // WiFi: connection setup, access point, scan, reconnect. Requires
    // globals.h, config.h, prefs_keys.h, declarations.h (included in
    // uhr4.ino before this file).

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


    // Prueft die WLAN-Verbindung und versucht stuendlich, sie wiederherzustellen (nur mit "WLAN neu verbinden",
    // siehe loop()). In den AP-Modus geht die Uhr nur beim Start ohne erreichbares WLAN, nie von hier aus.

    // Checks the WiFi connection and tries hourly to restore it (only with "Reconnect WiFi", see loop()). The
    // clock only enters AP mode at boot without a reachable WiFi, never from here.

    bool checkWiFiReconnect() {
        static unsigned long lastAttempt = 0;
        static unsigned long interval = 0; // 0 = erster Aufruf nach dem Start / first call after boot

        // Erster Versuch 5 Minuten nach dem Start - der Start hat es gerade erst versucht, ein sofortiger Versuch
        // blockierte Tasten und Weboberflaeche eine weitere Minute. 5 Minuten reichen fuer einen Router, der nach
        // einem Stromausfall langsamer hochfaehrt als die Uhr. Danach stuendlich, im AP-Modus von Anfang an.

        // First attempt 5 minutes after boot - boot has just tried, an immediate attempt blocked buttons and web
        // interface for another minute. 5 minutes suffice for a router that comes up more slowly than the clock
        // after a power cut. Then hourly, in AP mode right from the start.

        unsigned long now = millis();
        if (interval == 0) {
            interval = softAPIP ? WAIT_1h : 5 * WAIT_1m;
            lastAttempt = now;
            return true;
        }
        if (now - lastAttempt < interval) return true;
        interval = WAIT_1h;
        lastAttempt = now;

        if (WiFi.status() == WL_CONNECTED) {
            return true; // Verbindung ist in Ordnung
                         // connection is OK
        }

        DEBUG_PRINTLN("[WiFi] Disconnected. Attempting reconnect..");
        WiFi.disconnect();
        int slot = preferences.getInt(PK_LAST_WLAN, 0);
        connectWiFiWithRetries(slot, wifiSsid[slot], false);
        bool connected = WiFi.status() == WL_CONNECTED;

        // AP-Modus nach dem Start: verbunden -> AP-Modus beenden; sonst den AP wieder starten, den
        // connectWiFi() mit WiFi.mode(WIFI_MODE_NULL) abgeschaltet hat - die Uhr bleibt per Handy erreichbar.

        // AP mode after boot: connected -> leave AP mode; otherwise restart the AP that connectWiFi()
        // switched off with WiFi.mode(WIFI_MODE_NULL) - the clock stays reachable by phone.

        if (softAPIP) {
            if (connected) {
                DEBUG_PRINTLN("[WiFi] Reconnected - leaving access point mode");
                dnsServer.stop();
                softAPIP = false;
            }
            else if (!(WiFi.getMode() & WIFI_AP)) {
                WiFi.disconnect(); // keine Verbindungsversuche neben dem AP / no connection attempts beside the AP
                WiFi.softAP(AP_SSID, apPassword);
                dnsServer.stop();
                dnsServer.start(53, "*", WiFi.softAPIP());
                DEBUG_PRINTLN("[WiFi] Reconnect failed - access point restarted: " + String(AP_SSID));
            }
        }
        return connected;
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


    // WLAN-Passwoerter liegen im NVS verschluesselt ("e1:" + Hex, AES-256-GCM, WIFI_STORE_KEY), Klartext wird
    // beim Start umgestellt; Zugriff nur ueber loadWifiPass()/storeWifiPass(). Die Klartext-Kopie des
    // WiFi-Treibers ("nvs.net80211") ist abgeschaltet (WiFi.persistent(false)) und wird hier geloescht.

    // WiFi passwords are stored encrypted in NVS ("e1:" + hex, AES-256-GCM, WIFI_STORE_KEY), plain text is
    // converted at boot; access only via loadWifiPass()/storeWifiPass(). The WiFi driver's plain-text copy
    // ("nvs.net80211") is switched off (WiFi.persistent(false)) and deleted here.

    static_assert(sizeof(WIFI_STORE_KEY) == 65, "WIFI_STORE_KEY (config.h) muss 64 Hex-Zeichen haben / must have 64 hex characters");

    bool wifiStoreCrypt(bool encrypt, const uint8_t* iv, uint8_t* tag, const uint8_t* in, uint8_t* out, size_t len) {
        uint8_t key[32];
        if (backupUnhex(String(WIFI_STORE_KEY), key, sizeof(key)) != (int)sizeof(key)) return false;
        mbedtls_gcm_context gcm;
        mbedtls_gcm_init(&gcm);
        int rc = mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, key, 256);
        if (rc == 0) {
            rc = encrypt
                ? mbedtls_gcm_crypt_and_tag(&gcm, MBEDTLS_GCM_ENCRYPT, len, iv, 12, nullptr, 0, in, out, 16, tag)
                : mbedtls_gcm_auth_decrypt(&gcm, len, iv, 12, nullptr, 0, tag, 16, in, out);
        }
        mbedtls_gcm_free(&gcm);
        memset(key, 0, sizeof(key));
        return rc == 0;
    }

    String wifiPassEncrypt(const String& plain) {
        if (plain.length() == 0 || plain.length() > 63) return plain; // leer = kein Passwort / empty = no password
        uint8_t buf[12 + 16 + 63];
        uint8_t* iv = buf;
        uint8_t* tag = buf + 12;
        uint8_t* data = buf + 28;
        esp_fill_random(iv, 12);
        if (!wifiStoreCrypt(true, iv, tag, (const uint8_t*)plain.c_str(), data, plain.length())) return plain;
        String out = "e1:" + backupHex(buf, 28 + plain.length());
        memset(buf, 0, sizeof(buf));
        return out;
    }

    String wifiPassDecrypt(const String& stored) {
        if (!stored.startsWith("e1:")) return stored; // Klartext (aelterer Wert) / plain text (older value)
        uint8_t buf[12 + 16 + 63];
        int n = backupUnhex(stored.substring(3), buf, sizeof(buf));
        if (n < 28) return stored; // kein gueltiger Wert - evtl. Klartext mit "e1:" / not valid - maybe plain text with "e1:"
        size_t len = n - 28;
        uint8_t clear[64];
        if (!wifiStoreCrypt(false, buf, buf + 12, buf + 28, clear, len)) return stored;
        String out;
        out.concat((const char*)clear, len);
        memset(clear, 0, sizeof(clear));
        return out;
    }

    String loadWifiPass(int i) {
        return wifiPassDecrypt(preferences.getString(pkPass(i).c_str(), ""));
    }

    void storeWifiPass(int i, const String& pass) {
        preferences.putString(pkPass(i).c_str(), wifiPassEncrypt(pass));
    }

    // Schreibt und prueft durch Zuruecklesen (wie putStringVerified())
    // Writes and verifies by reading back (like putStringVerified())

    bool storeWifiPassVerified(int i, const String& pass) {
        storeWifiPass(i, pass);
        if (loadWifiPass(i) != pass) {
            DEBUG_PRINTLN("[Preferences] Verifikation fehlgeschlagen fuer Key '" + pkPass(i) + "'");
            return false;
        }
        return true;
    }

    // Beim Start: Klartext-Passwoerter (aeltere Firmware, alte Sicherung) verschluesseln
    // At boot: encrypt plain-text passwords (older firmware, old backup)

    void migrateWifiPasswords() {
        for (int i = 0; i < MAX_WLAN; i++) {
            String raw = preferences.getString(pkPass(i).c_str(), "");
            if (raw.length() > 0 && !raw.startsWith("e1:")) {
                storeWifiPass(i, raw);
                DEBUG_PRINTLN("[WiFi] Password in slot " + String(i + 1) + " encrypted in settings");
            }
            for (size_t k = 0; k < raw.length(); k++) raw[k] = 0;
        }
    }


    void wipeWifiDriverStorage() {
        nvs_iterator_t it = nullptr;
        bool hasEntries = nvs_entry_find("nvs", "nvs.net80211", NVS_TYPE_ANY, &it) == ESP_OK;
        nvs_release_iterator(it);
        if (!hasEntries) return;
        nvs_handle_t handle;
        if (nvs_open("nvs.net80211", NVS_READWRITE, &handle) == ESP_OK) {
            nvs_erase_all(handle);
            nvs_commit(handle);
            nvs_close(handle);
            DEBUG_PRINTLN("[WiFi] Removed stored credential copy of the WiFi driver");
        }
    }


    // Zugangsdaten nach erfolgreichem WPS lesen: WiFi.SSID()/psk() sind dann unzuverlaessig und esp_wifi_get_config()
    // liefert direkt nach dem Event manchmal leere Daten (ESP-IDF#10339/#11705) - bis zu 20 Versuche.

    // Read the credentials after a successful WPS: WiFi.SSID()/psk() are unreliable then and esp_wifi_get_config()
    // sometimes returns empty data right after the event (ESP-IDF#10339/#11705) - up to 20 attempts.

    bool readWpsCredentials(String& ssid, String& pass) {
        ssid = "";
        pass = "";
        for (int attempt = 0; attempt < 20 && ssid == ""; attempt++) {
            wifi_config_t config;
            if (esp_wifi_get_config(WIFI_IF_STA, &config) == ESP_OK) {
                char ssidBuf[33] = { 0 };
                char passBuf[65] = { 0 };
                memcpy(ssidBuf, config.sta.ssid, sizeof(config.sta.ssid));
                memcpy(passBuf, config.sta.password, sizeof(config.sta.password));
                ssid = String(ssidBuf);
                pass = String(passBuf);
            }
            if (ssid == "") delay(100); // kurz warten, dann erneut versuchen
                                        // wait briefly, then retry
        }
        return ssid != "";
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
                String storedPass = loadWifiPass(i);
                if (storedPass != pass) {
                    storeWifiPass(i, pass);
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
        storeWifiPass(freeIdx, pass);

        // PK_LAST_WLAN bewusst NICHT setzen (siehe Kommentar oben).
        // PK_LAST_WLAN deliberately NOT set (see comment above).

        wifiSsid[freeIdx] = ssid;
        wifiPass[freeIdx] = pass;
        DEBUG_PRINTLN("[WPS] Saved new network " + ssid + " in slot " + String(freeIdx + 1));
        return freeIdx;
    }


    // Zugangsdaten des Access Points auf dem Display: nach dem Start fuer
    // AP_INFO_SHOW_MS (danach laeuft die Uhr, siehe loop() in uhr4.ino) und
    // erneut bei kurzem Tasterdruck (showWlanCredentials()).

    // Access point credentials on the display: after starting for
    // AP_INFO_SHOW_MS (the clock runs afterwards, see loop() in uhr4.ino) and
    // again on a short button press (showWlanCredentials()).

    void showApInfo() {
        static const lgfx::IFont* const fontsMain[] = { &fonts::FreeSansBold12pt7b, &fonts::DejaVu18, &fonts::Font0, nullptr };
        static const lgfx::IFont* const fontsSmall[] = { &fonts::DejaVu18, &fonts::Font0, nullptr };
        String url = "http://" + WiFi.softAPIP().toString();
        String pass = String("PW ") + apPassword;
        DRAW_ON_BOTH_DISPLAYS(
            tft.fillScreen(TFT_BLACK);
            drawTextRow(tft, 0.24f, "Access Point", TFT_YELLOW, fontsMain, false);
            drawTextRow(tft, 0.42f, AP_SSID, TFT_YELLOW, fontsMain, false);
            drawTextRow(tft, 0.58f, pass, TFT_YELLOW, fontsMain, false);
            drawTextRow(tft, 0.76f, url, TFT_DARKGREY, fontsSmall, false);
        );
    }


    void startAP() {
        if (useBacklight && backlightAttached) ledcWrite(TFT_Backlight, BACKLIGHT_SETUP_LEVEL); // Einrichtung: 50 %
                                                                                                // setup: 50 %


        // Station-Modus starten, aber NICHT verbinden - ein gleichzeitiger
        // WiFi.begin() wuerde die WPS-Aushandlung stoeren (geteilter Funk).

        // Start station mode, but do NOT connect - a concurrent WiFi.begin()
        // would interfere with WPS negotiation (shared radio).

        WiFi.mode(WIFI_MODE_STA);
        WiFi.disconnect();


        // WPS versuchen, wenn moeglich. Den Slot waehlt saveWpsCredentials() nach einem WPS-Erfolg.
        // Try WPS if possible. The slot is chosen by saveWpsCredentials() after a WPS success.

        startWPS(); // WPS starten
                    // start WPS

        // Beschriftung einmalig zeichnen; nur der Countdown wird danach pro Sekunde aktualisiert (ohne Flimmern)
        // Draw the labels once; only the countdown is updated afterwards each second (without flicker)

        int lastSecondsShown = -1;
        showButtonMessage(TFT_YELLOW, "WPS", "120 s", "then Access Point", TFT_DARKGREY);

        // 30s waren in der Praxis oft zu knapp fuer eine vollstaendige
        // WPS-Aushandlung - auf 2 Minuten verlaengert, wie beim Web-Button-
        // WPS-Weg (siehe loop() in uhr4.ino).

        // 30s was often too short in practice for a full WPS negotiation -
        // extended to 2 minutes, matching the web-button WPS path (see
        // loop() in uhr4.ino).

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
                if (secondsLeft % 30 == 0) DEBUG_PRINTLN("[WPS] waiting... " + String(secondsLeft) + "s left");
                updateButtonLine(TFT_YELLOW, String(secondsLeft) + " s");
            }
            handleSerialCommands(); // Displaytyp per USB auch waehrend dieser Wartezeit (siehe handleSerialCommands())
                                    // display type via USB also during this wait (see handleSerialCommands())
            checkButton();          // Taster/Boot-Taste auch waehrend dieser Wartezeit
                                    // button/boot button also during this wait
            delay(100);
        }
        DEBUG_PRINTLN("[WPS] wait loop exited, success=" + String(wpsSuccessEvent) + ", failed=" + String(wpsFailedEvent));

        if (wpsSuccessEvent) {
            wpsSuccessEvent = false;

            // WiFi.SSID()/psk() sind hier unzuverlaessig (ESP-IDF#10339, siehe
            // onWpsEvent()) - stattdessen per esp_wifi_get_config() mehrfach lesen.

            // WiFi.SSID()/psk() are unreliable here (ESP-IDF#10339, see
            // onWpsEvent()) - read via esp_wifi_get_config() with retries instead.

            String newSsid, newPass;
            readWpsCredentials(newSsid, newPass);

            if (newSsid != "") {
                DEBUG_PRINTLN("[WPS] Connected to the network!");
                DEBUG_PRINTLN("[WPS] SSID: " + newSsid);

                int savedSlot = saveWpsCredentials(newSsid, newPass);
                preferences.putInt(PK_LAST_WLAN, savedSlot);

                showButtonMessage(TFT_GREEN, "WPS", tftText(newSsid), "restarting", TFT_DARKGREY);

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

        // Access Point zuerst starten, damit die Uhr auch bei haengendem Scan per WPS-Retry oder
        // Weboberflaeche erreichbar ist. MAC hier selbst holen - ohne gespeichertes Netz lief connectWiFi()
        // nie, das sonst mac[] fuellt.

        // Start the access point first, so the clock is reachable via WPS retry or the web interface even if
        // the scan hangs. Fetch the MAC here - without a stored network connectWiFi(), which otherwise fills
        // mac[], never ran.

        WiFi.macAddress(mac);

        // Festes Passwort aus der Firmware (AP_PASSWORD in config.h)
        // Fixed password from the firmware (AP_PASSWORD in config.h)

        strlcpy(apPassword, AP_PASSWORD, sizeof(apPassword));

        WiFi.softAP(AP_SSID, apPassword);
        DEBUG_PRINTLN("[WiFi] Started Access Point: " + String(AP_SSID)); // Passwort bewusst nicht im Log / password deliberately not logged

        // Captive portal: leite alle DNS-Anfragen auf die AP-IP um
        // Captive portal: redirect all DNS requests to the AP IP

        dnsServer.start(53, "*", WiFi.softAPIP());

        // WLAN-Scan durchführen (asynchron, mit Zeitlimit statt blockierend -
        // ein haengender Scan darf die Uhr nicht dauerhaft aufhalten, der AP
        // laeuft ja bereits).

        // Perform the WiFi scan (asynchronous, with a time limit instead of
        // blocking - a stuck scan must not hold up the clock permanently,
        // the AP is already running).

        showButtonMessage(TFT_YELLOW, "WiFi scan", "...", "", TFT_DARKGREY);
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

        // Vom Treiber fuer die Scan-Ergebnisse belegten Speicher freigeben (wie in
        // checkWiFiScan()/scanAndCacheNetworks()).

        // Free the memory the driver allocated for the scan results (as in
        // checkWiFiScan()/scanAndCacheNetworks()).

        WiFi.scanDelete();

        clearTFT();
        showApInfo();

        softAPIP = true;
        softAPIPstart = millis();

        ipAddress = WiFi.softAPIP().toString();

    }


    // Graue Kopfzeile der WLAN-Anzeigen: "Build" + Version. Auf dem kleinen runden Display (160 px) ist die
    // oberste Zeile nur ~120 px breit - dort ohne Jahrhundert und Sekunden ("Build 26-10-08 15:27").

    // Grey header row of the WiFi screens: "Build" + version. On the small round display (160 px) the top
    // row is only ~120 px wide - there without century and seconds ("Build 26-10-08 15:27").

    String buildLabel() {
        String v = String(version);
        if (displayGeom->round && CLOCK_HEIGHT < 200 && v.length() >= 16) v = v.substring(2, 16);
        return "Build " + v;
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

        if (verboseMode && useBacklight && backlightAttached) {
            ledcWrite(TFT_Backlight, BACKLIGHT_SETUP_LEVEL);
        }
        if (wifiSsid[number] == "") {
           // DEBUG_PRINTLN("[WiFi] SSID " + String(number + 1) + " is empty, skipping");
            return NOT_CONNECTED;
        }




        // DEBUG_PRINTLN("[WiFi] Trying SSID" + String(number+1) + ": " + wifiSsid[number]);

        // Wenn verboseMode aktiviert ist, zeige die Verbindungsinformationen auf dem Display an
        // If verboseMode is enabled, show connection info on the display

        if (verboseMode) {
            clearTFT();

            // Erst umwandeln, dann kuerzen: ein Zeichen = ein Byte, die Kuerzung trennt so keine UTF-8-Folge
            // (Umlaut in der SSID). Zeilen wie showWlanCredentials(), darunter dreht sich animateCursor().

            // Convert first, then shorten: one character = one byte, so shortening doesn't split a UTF-8
            // sequence (umlaut in the SSID). Rows like showWlanCredentials(), animateCursor() spins below.

            static const lgfx::IFont* const fontsMain[] = { &fonts::FreeSansBold12pt7b, &fonts::DejaVu18, &fonts::Font0, nullptr };
            static const lgfx::IFont* const fontsSmall[] = { &fonts::DejaVu18, &fonts::Font0, nullptr };
            String ssidText = tftText(wifiSsid[number]);
            if (ssidText.length() > 24) ssidText = ssidText.substring(0, 22) + "..";
            String slotText = "Connect to WiFi " + String(number + 1);
            DRAW_ON_BOTH_DISPLAYS(
                tft.fillScreen(TFT_BLACK);
                drawTextRow(tft, 0.18f, buildLabel(), TFT_DARKGREY, fontsSmall, false);
                drawTextRow(tft, 0.34f, slotText, TFT_GREEN, fontsSmall, false);
                drawTextRow(tft, 0.50f, ssidText, TFT_GREEN, fontsMain, false);
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
            handleSerialCommands(); // Displaytyp per USB auch waehrend dieser Wartezeit (siehe handleSerialCommands())
                                    // display type via USB also during this wait (see handleSerialCommands())
            checkButton();          // Taster/Boot-Taste auch waehrend dieser Wartezeit
                                    // button/boot button also during this wait
            if (loggingEnabled) Serial.print("");
            if (verboseMode) {
                animateCursor(statusWidth() / 2 - 3 * TFT_TEXT_SIZE, (int)(CLOCK_HEIGHT * 0.70f), 100);
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

            // Firmware-Update ueber WLAN (nur ESP32-S3) ebenso neu starten - siehe ota_update.h
            // Restart the firmware update over WiFi (ESP32-S3 only) likewise - see ota_update.h

            startArduinoOta();

            // R2RNet-Multicast ebenfalls neu beitreten - wie beim NTP-Server ueberlebt der Socket den
            // WLAN-Neuaufbau nicht (siehe rocrail_client.h). Nur, wenn Rocrail aktiviert ist.

            // Rejoin the R2RNet multicast too - like the NTP server, the socket does not survive the WiFi
            // restart (see rocrail_client.h). Only when Rocrail is enabled.

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
                tft.setTextSize(TFT_TEXT_SIZE);
                tft.setTextColor(TFT_GREEN, TFT_BLACK);
                tft.setCursor(x, y);
                tft.print(frames[i]);
            );
            delay(delayMs);
        }
    }


    // Anzeige WLAN Parameter auf dem TFT
    // Display WiFi parameters on the TFT

    void showWlanCredentials(String wlan) {

        // Im Access-Point-Modus dessen Zugangsdaten zeigen (Taster, siehe checkButton())
        // In access point mode show its credentials (button, see checkButton())

        if (softAPIP && WiFi.status() != WL_CONNECTED) {
            showApInfo();
            return;
        }

        // Zentrierte Zeilen in der jeweils groessten passenden Schrift (drawTextRow() in system_utils.h), gruen;
        // die Firmware-Version klein und grau. Lange WLAN-Namen gekuerzt (erst umwandeln, dann kuerzen).

        // Centred rows in the largest fitting font each (drawTextRow() in system_utils.h), green; the firmware
        // version small and grey. Long WiFi names shortened (convert first, then shorten).

        static const lgfx::IFont* const fontsMain[] = { &fonts::FreeSansBold12pt7b, &fonts::DejaVu18, &fonts::Font0, nullptr };
        static const lgfx::IFont* const fontsSmall[] = { &fonts::DejaVu18, &fonts::Font0, nullptr };
        bool connected = WiFi.status() == WL_CONNECTED;
        String wlanText = tftText(wlan);
        if (wlanText.length() > 24) wlanText = wlanText.substring(0, 22) + "..";
        String slotText = "WiFi " + String(preferences.getInt(PK_LAST_WLAN, -1) + 1);
        String ipText = WiFi.localIP().toString();
        String hostText = String(hostname) + ".local";
        DRAW_ON_BOTH_DISPLAYS(
            tft.fillScreen(TFT_BLACK);
            drawTextRow(tft, 0.18f, buildLabel(), TFT_DARKGREY, fontsSmall, false);
            if (connected) {
                drawTextRow(tft, 0.34f, slotText, TFT_GREEN, fontsSmall, false);
                drawTextRow(tft, 0.50f, wlanText, TFT_GREEN, fontsMain, false);
                drawTextRow(tft, 0.66f, ipText, TFT_GREEN, fontsMain, false);
                if (pingHostname) drawTextRow(tft, 0.82f, hostText, TFT_GREEN, fontsSmall, false);
            }
            else {
                drawTextRow(tft, 0.50f, "Not connected", TFT_GREEN, fontsMain, false);
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


    // Startet asynchronen WiFi-Scan - nie waehrend eines Firmware-Updates, die Kanalsuche unterbricht es
    // Starts an asynchronous WiFi scan - never during a firmware update, the channel search breaks it

    void startWiFiScan() {
        if (otaInProgress) return;
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


    // Scannt WLANs und cached Ergebnisse - nicht waehrend eines Firmware-Updates (siehe startWiFiScan())
    // Scans WiFi networks and caches results - not during a firmware update (see startWiFiScan())

    void scanAndCacheNetworks() {
        if (otaInProgress) return;

        showButtonMessage(TFT_GREEN, "WiFi scan", "...", "", TFT_DARKGREY);

        DEBUG_PRINTLN("[WiFi] Scanning for WiFi networks..");

        int networkCount = WiFi.scanNetworks();
        DEBUG_PRINTLN("[WiFi] found " + String(networkCount) + " WiFi networks:");
        if (networkCount > MAX_WLAN) {
            DEBUG_PRINTLN("[WiFi] keeping the " + String(MAX_WLAN) + " strongest:");
        }

        collectStrongestNetworks(networkCount);

        WiFi.scanDelete();
        DEBUG_PRINTLN("[WiFi] done");

    }

