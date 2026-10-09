#pragma once

#include <NetworkClientSecure.h>

    // Uhrzeit aus einem offenen WLAN (fetchTimeFromOpenWifi()): beim Start, wenn kein gespeichertes WLAN verbunden
    // hat, keine RTC laeuft und die Uhr noch keine Zeit hat. Erst NTP, sonst der "Date:"-Kopf einer HTTP(S)-
    // Antwort - die liefert auch die Anmeldeseite eines Hotspots, der NTP sperrt. Jeder Schritt steht im Log.

    // Time from an open WiFi (fetchTimeFromOpenWifi()): at boot, if no stored WiFi connected, no RTC runs and
    // the clock has no time yet. NTP first, otherwise the "Date:" header of an HTTP(S) reply - the login
    // page of a hotspot that blocks NTP delivers it too. Every step is in the log.


    // Eine HTTP(S)-Anfrage HEAD <path>: Unix-Sekunden UTC aus "Date:", 0 ohne brauchbare Antwort; note sagt, woran
    // es lag, location die Weiterleitung. Template: connect() mit Zeitlimit ist in NetworkClient nicht virtuell.

    // One HTTP(S) request HEAD <path>: unix seconds UTC from "Date:", 0 without a usable reply; note says why,
    // location the redirect. Template: connect() with a timeout is not virtual in NetworkClient.

    template <typename C>
    time_t httpDateRequest(C& client, uint16_t port, const String& target, const String& path, String& note,
                           String& location) {
        IPAddress ip;
        location = "";
        uint32_t connectMs = (port == 443) ? 10000 : 4000; // TLS braucht laenger / TLS takes longer
        bool connected = ip.fromString(target) ? client.connect(ip, port, connectMs) : client.connect(target.c_str(), port, connectMs);
        if (!connected) {
            note = "no connection";
            return 0;
        }
        client.print("HEAD " + path + " HTTP/1.1\r\nHost: " + target + "\r\nConnection: close\r\n\r\n");
        unsigned long start = millis();
        time_t result = 0;
        String status;
        note = "no reply";

        // Auch nach dem Schliessen durch den Server noch die gepufferten Zeilen lesen
        // Keep reading the buffered lines even after the server closed the connection

        while ((client.connected() || client.available()) && millis() - start < WAIT_5s) {
            if (!client.available()) {
                delay(10);
                continue;
            }
            String line = client.readStringUntil('\n');
            line.trim();
            if (status.length() == 0) {
                status = line.substring(0, 40);
                note = "no Date header (" + status + ")";
            }
            if (line.length() == 0) break; // Ende der Kopfzeilen / end of the headers
            if (line.startsWith("Location:") || line.startsWith("location:")) {
                location = line.substring(9);
                location.trim();
                continue;
            }
            if (!line.startsWith("Date:") && !line.startsWith("date:")) continue;

            // "Date: Tue, 06 Oct 2026 18:20:00 GMT"
            note = "Date unreadable (" + line.substring(0, 40) + ")";
            int day, year, hh, mm, ss;
            char mon[4] = "";
            if (sscanf(line.c_str() + 5, " %*[A-Za-z], %d %3s %d %d:%d:%d", &day, mon, &year, &hh, &mm, &ss) != 6) break;
            const char* months = "JanFebMarAprMayJunJulAugSepOctNovDec";
            const char* p = strstr(months, mon);
            if (!p || strlen(mon) != 3) break;
            int m = (p - months) / 3 + 1;
            note = status;

            // Tage seit 1970 (Kalenderrechnung nach H. Hinnant, ohne Zeitzone)
            // days since 1970 (civil calendar after H. Hinnant, without a time zone)

            int y = year - (m <= 2);
            long era = (y >= 0 ? y : y - 399) / 400;
            long yoe = y - era * 400;
            long doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + day - 1;
            long doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
            long days = era * 146097 + doe - 719468;
            result = (time_t)days * 86400 + hh * 3600 + mm * 60 + ss;
            break;
        }
        client.stop();
        return result;
    }

    // HTTP (Port 80) oder HTTPS (Port 443, TLS ohne Zertifikatspruefung - ohne gueltige Uhrzeit liesse sich das
    // Zertifikat ohnehin nicht pruefen)

    // HTTP (port 80) or HTTPS (port 443, TLS without certificate check - without a valid time the certificate
    // could not be checked anyway)

    time_t httpDateEpoch(const String& target, const String& path, String& note, String& location, bool tls) {
        if (tls) {
            NetworkClientSecure client;
            client.setInsecure();
            client.setHandshakeTimeout(8);
            time_t result = httpDateRequest(client, 443, target, path, note, location);

            // Schlug die TLS-Verbindung fehl, den mbedTLS-Fehler fuers Log anhaengen
            // If the TLS connection failed, append the mbedTLS error to the log

            if (!result && note == "no connection") {
                char err[80] = "";
                int code = client.lastError(err, sizeof(err));
                if (code) note += " (TLS error " + String(code) + (err[0] ? String(": ") + err : String("")) + ")";
            }
            return result;
        }
        WiFiClient client;
        return httpDateRequest(client, 80, target, path, note, location);
    }


    // Uhrzeit ueber ein verbundenes offenes WLAN holen: NTP, dann HTTP ohne DNS (Gateway = meist die Anmeldeseite,
    // 1.1.1.1), dann per Name, zuletzt einmal der Weiterleitung folgen (https:// per TLS, danach per HTTP).
    // how = Weg fuers Log, 0 ohne Zeit.

    // Get the time via a connected open WiFi: NTP, then HTTP without DNS (gateway = usually the login page,
    // 1.1.1.1), then by name, finally follow the redirect once (https:// via TLS, then via HTTP).
    // how = way for the log, 0 without a time.

    time_t openWifiQueryTime(const String& ssid, String& how) {
        how = "NTP pool.ntp.org";
        time_t epoch = ntpQueryEpoch("pool.ntp.org");
        DEBUG_PRINTLN("[OPEN-WIFI] '" + ssid + "': " + how + (epoch ? " answered" : " no answer (blocked?)"));
        if (epoch) return epoch;

        String targets[] = { WiFi.gatewayIP().toString(), "1.1.1.1", "connectivitycheck.gstatic.com", "neverssl.com" };
        String redirect;
        for (const String& target : targets) {
            how = "HTTP Date " + target;
            String note, location;
            epoch = httpDateEpoch(target, "/", note, location, false);
            DEBUG_PRINTLN("[OPEN-WIFI] '" + ssid + "': " + how + (epoch ? " answered: " : " - ") + note +
                          (location.length() ? " -> " + location.substring(0, 80) : String("")));
            if (epoch) return epoch;
            if (redirect.isEmpty()) redirect = location;
        }

        int scheme = redirect.indexOf("://");
        if (scheme <= 0) return 0;
        bool https = redirect.startsWith("https");
        String rest = redirect.substring(scheme + 3);
        int slash = rest.indexOf('/');
        String host = slash < 0 ? rest : rest.substring(0, slash);
        String path = slash < 0 ? "/" : rest.substring(slash);
        int colon = host.indexOf(':');
        if (colon >= 0) host = host.substring(0, colon);
        for (int pass = https ? 0 : 1; pass < 2; pass++) {
            bool tls = (pass == 0);
            how = String(tls ? "HTTPS" : "HTTP") + " Date " + host + " (redirect)";
            String note, location;
            epoch = httpDateEpoch(host, path, note, location, tls);
            DEBUG_PRINTLN("[OPEN-WIFI] '" + ssid + "': " + how + (epoch ? " answered: " : " - ") + note);
            if (epoch) return epoch;
        }
        return 0;
    }

    // Beim Start (connectWiFiAtBoot()), wenn kein gespeichertes WLAN verbunden hat: mit den bis zu vier
    // staerksten offenen WLANs aus dem Scan (ab -85 dBm) kurz verbinden, Zeit holen, sofort trennen. Gespeicherte
    // WLANs bleiben unberuehrt (WiFi.persistent(false)). Abschaltbar im Tab "NTP Zeitzone". true = Zeit gesetzt.

    // At boot (connectWiFiAtBoot()), if no stored WiFi connected: briefly connect to the up to
    // four strongest open WiFis from the scan (from -85 dBm), get the time, disconnect right away. Stored WiFis
    // stay untouched (WiFi.persistent(false)). Can be switched off in the "NTP Timezone" tab. true = time set.

    bool fetchTimeFromOpenWifi(bool force) {
        if (!force && !preferences.getBool(PK_OPEN_WIFI_TIME, true)) {
            DEBUG_PRINTLN("[OPEN-WIFI] Skipped - switched off in the settings");
            return false;
        }
        if (!force && rtcOk == RTC_AVAILABLE) {
            DEBUG_PRINTLN("[OPEN-WIFI] Skipped - the RTC provides the time");
            return false;
        }

        // Schon eine Zeit da (z.B. waehrend der WLAN-Versuche per USB gesetzt) - dann nichts holen
        // Already a time (e.g. set via USB during the WiFi attempts) - then fetch nothing

        struct tm now;
        if (!force && getLocalTime(&now, 0)) {
            DEBUG_PRINTLN("[OPEN-WIFI] Skipped - the clock already has a valid time (e.g. set via USB)");
            return false;
        }

        // Offene Netze aus dem Start-Scan, staerkste zuerst (availableNetworks ist nach Signal sortiert)
        // Open networks from the boot scan, strongest first (availableNetworks is sorted by signal)

        const int MAX_TRIES = 4;
        String candidates[MAX_TRIES];
        int candidateCount = 0;
        for (int i = 0; i < MAX_WLAN && candidateCount < MAX_TRIES; i++) {
            if (availableNetworks[i].ssid.length() == 0 || availableNetworks[i].enc != WIFI_AUTH_OPEN) continue;
            if (availableNetworks[i].rssi < (force ? -95 : -85)) continue; // force (Diagnose): auch sehr schwache / also very weak ones

            // Gleicher Name von mehreren Sendern (z.B. Hotspot-Ketten) nur einmal - WiFi.begin() nimmt ohnehin
            // den staerksten Sender dieses Namens

            // Same name from several access points (e.g. hotspot chains) only once - WiFi.begin() takes the
            // strongest access point of that name anyway

            bool duplicate = false;
            for (int k = 0; k < candidateCount; k++) {
                if (candidates[k] == availableNetworks[i].ssid) { duplicate = true; break; }
            }
            if (!duplicate) candidates[candidateCount++] = availableNetworks[i].ssid;
        }
        if (candidateCount == 0) {
            DEBUG_PRINTLN("[OPEN-WIFI] No open WiFi (from -85 dBm) in the scan - no time from open WiFis");
            return false;
        }
        DEBUG_PRINTLN("[OPEN-WIFI] Trying to get the time from " + String(candidateCount) + " open WiFi(s)");

#ifdef WIFI_TX_AUTO

        // Offene Hotspots sind meist weit weg (niedrige Datenrate, keine Verzerrung): volle Leistung. Danach wieder die
        // Startstufe, damit Access Point und gespeicherte WLANs sicher laufen.

        // Open hotspots are usually far away (low data rate, no distortion): full power. Afterwards the start step
        // again, so the access point and stored WiFis run safely.

        struct TxGuard {
            TxGuard() { wifiTxPowerCurrent = WIFI_TX_OPEN; }
            ~TxGuard() { wifiTxPowerCurrent = WIFI_TX_FIRST; applyWifiTxPower(); }
        } txGuard;
#endif
        showButtonMessage(TFT_GREEN, tftText(translate("Time from open WiFi")), "...", "", TFT_DARKGREY);

        for (int c = 0; c < candidateCount; c++) {
            const String& ssid = candidates[c];
            DEBUG_PRINTLN("[OPEN-WIFI] '" + ssid + "': connecting..");
            WiFi.disconnect();
            WiFi.mode(WIFI_STA);
            applyWifiTxPower();
            WiFi.begin(ssid.c_str());
            unsigned long start = millis();
            while (WiFi.status() != WL_CONNECTED && millis() - start < 15 * WAIT_1s) {
                handleSerialCommands();
                checkButton(); // Taster/Boot-Taste auch hier / button/boot button here too
                delay(100);
            }
            if (WiFi.status() != WL_CONNECTED) {
                DEBUG_PRINTLN("[OPEN-WIFI] '" + ssid + "': no connection within 15 s");
                WiFi.disconnect();
                continue;
            }
            DEBUG_PRINTLN("[OPEN-WIFI] '" + ssid + "': connected, IP " + WiFi.localIP().toString() + ", gateway " +
                          WiFi.gatewayIP().toString());

            String how;
            time_t epoch = openWifiQueryTime(ssid, how);
            WiFi.disconnect();
            String localText;
            if (epoch > 0 && setClockTime(String((long long)epoch), "[OPEN-WIFI]", localText)) {
                DEBUG_PRINTLN("[OPEN-WIFI] Time set from open WiFi '" + ssid + "' via " + how + ": " + localText + " (disconnected again)");
                return true;
            }
            DEBUG_PRINTLN("[OPEN-WIFI] '" + ssid + "': no time (disconnected again)");
        }
        DEBUG_PRINTLN("[OPEN-WIFI] No time from open WiFis");
        return false;
    }
