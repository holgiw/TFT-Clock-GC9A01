#pragma once
    // ### Zeit: RTC, DCF77, NTP-Client & -Server, Zeitzone ################
    // ### Time: RTC, DCF77, NTP client & server, timezone ################
    // Benoetigt globals.h, config.h, prefs_keys.h und declarations.h (werden
    // zentral in uhr3.ino VOR dieser Datei eingebunden).

    // Requires globals.h, config.h, prefs_keys.h and declarations.h (these are
    // included centrally in uhr3.ino BEFORE this file).

    // Interrupt-Handler fuer den DCF77-Eingang.
    //
    // Hier darf NICHTS stehen, was im Flash liegt: waehrend der Flash-Cache
    // deaktiviert ist (jeder LittleFS-Schreibvorgang - also auch jede Logzeile
    // ueber logToFile() - und jedes nvs_commit() aus preferences.putXxx())
    // fuehrt ein solcher Zugriff zu einem "Cache disabled but cached memory
    // region accessed"-Panic-Reset. Deshalb wird die LED hier NICHT mehr
    // direkt geschaltet (toggleLED() -> setLedOn()/setLedOff() -> pinMode()/
    // digitalWrite(), alle Flash-resident), sondern nur ein Flag gesetzt, das
    // loop() abarbeitet.
    //
    // HINWEIS: DCF77::int0handler() liegt in der Bibliothek und damit ebenfalls
    // im Flash - das laesst sich von hier aus nicht aendern, ohne die
    // Bibliothek zu patchen. Ein Restrisiko bleibt daher bestehen, ist aber
    // deutlich kleiner als vorher (ein Aufruf statt vier).

    // Interrupt handler for the DCF77 input.
    //
    // NOTHING that lives in flash may run here: while the flash cache is
    // disabled (every LittleFS write - so also every log line via logToFile()
    // - and every nvs_commit() from preferences.putXxx()) such an access
    // causes a "Cache disabled but cached memory region accessed" panic reset.
    // That's why the LED is no longer switched directly here (toggleLED() ->
    // setLedOn()/setLedOff() -> pinMode()/digitalWrite(), all flash-resident);
    // only a flag is set, which loop() processes.
    //
    // NOTE: DCF77::int0handler() lives in the library and therefore also in
    // flash - that can't be changed from here without patching the library. A
    // residual risk remains, but it's much smaller than before (one call
    // instead of four).

    void IRAM_ATTR isr() {
#if defined DCF77_DATAPIN && defined DCF77_INTERRUPT
        DCF77::int0handler();
        if (!dcfTimeFound) dcfLedTogglePending = true;
        dcf77Count++;
        if (dcf77Count > 120) dcf77Count = 1;
#endif
    }


    // Lädt die Zeit vom RTC-Modul und setzt die Systemzeit entsprechend
    // Loads the time from the RTC module and sets the system time accordingly

    void loadTimeFromRTC() {
        if (rtcOk == RTC_AVAILABLE) {

            DateTime now = rtc.now(); // DS3231 lesen
                                      // read DS3231

            // RTClib liefert Year/Month/Day usw. "normal" (2026, 2, 11, ...)
            // RTClib returns Year/Month/Day etc. "normally" (2026, 2, 11, ...)
            struct tm tm_rtc = {};
            tm_rtc.tm_year = now.year() - 1900;
            tm_rtc.tm_mon = now.month() - 1;
            tm_rtc.tm_mday = now.day();
            tm_rtc.tm_hour = now.hour();
            tm_rtc.tm_min = now.minute();
            tm_rtc.tm_sec = now.second();
            tm_rtc.tm_isdst = -1; // Sommerzeit automatisch
                                  // DST automatic

            time_t t = mktime(&tm_rtc); // -> Unix-Zeit (lokale Interpretation je nach TZ!)
                                        // -> Unix time (local interpretation depending on TZ!)

            // ESP32-Systemzeit setzen
            // Set ESP32 system time
            struct timeval tv;
            tv.tv_sec = t;
            tv.tv_usec = 0;
            settimeofday(&tv, nullptr);

            // Optional: globales timeinfo aktualisieren
            // Optional: update the global timeinfo
            time_t now_esp = time(nullptr);
            localtime_r(&now_esp, &timeinfo);

            DEBUG_PRINTLN("[RTC] Time loaded from RTC and system time set");
        }
    }


    // Initialisiere die NTP-Server
    // Initialize the NTP servers

    void initializeNtpServers() {
        for (int i = 0; i < MAX_WLAN; i++) {
            String ntpServerKey = pkNtpServer(i);
            String ntpServerValue = preferences.getString(ntpServerKey.c_str(), "");
            if (ntpServerValue.isEmpty()) {
                // Standardwerte setzen, falls keine gespeicherten Werte vorhanden sind
                // Set default values if no saved values exist
                if (i == 0) {
                    strncpy(ntpServers[i], NTP_SERVER_1, sizeof(ntpServers[i]) - 1);
                }
                else if (i == 1) {
                    strncpy(ntpServers[i], NTP_SERVER_2, sizeof(ntpServers[i]) - 1);
                }
                else {
                    ntpServers[i][0] = '\0'; // Leerer Eintrag
                                             // Empty entry
                }
            }
            else {
                // Gespeicherte Werte laden
                // Load saved values
                strncpy(ntpServers[i], ntpServerValue.c_str(), sizeof(ntpServers[i]) - 1);
                DEBUG_PRINTLN("[NTP] Loaded NTP server " + String(i + 1) + ": " + String(ntpServers[i]));
            }
            ntpServers[i][sizeof(ntpServers[i]) - 1] = '\0'; // Null-terminieren
                                                             // Null-terminate
        }
    }


    // Funktion, um die DCF77-Zeit abzurufen und die Systemzeit zu setzen
    // Function to fetch the DCF77 time and set the system time

    bool getDCF77Time() {
#if defined DCF77_DATAPIN && defined DCF77_INTERRUPT
        time_t DCFtime;
        if (millis() - lastDCFUpdate > WAIT_1s) {
            // Serial.println(dcf77Count);

            lastDCFUpdate = millis(); // Timer zurücksetzen
                                      // reset timer
             // DEBUG_PRINTLN("[DCF77] checking DCF77 time... ");

            DCFtime = dcf.getUTCTime(); // DCF-Zeit abrufen
                                        // fetch DCF time

            if (DCFtime != 0) {
                setLedOff(); // LED ausschalten, wenn Zeit gefunden wurde
                             // turn off LED once time is found

                // NTP gilt als "aktuell verfuegbar", wenn WLAN verbunden ist UND die letzte
                // erfolgreiche Synchronisation nicht laenger als DCF77_NTP_GRACE_PERIOD
                // zurueckliegt - nur sonst wird DCF77 zur Zeituebernahme herangezogen.

                // NTP counts as "currently available" if WiFi is connected AND the last
                // successful sync is not older than DCF77_NTP_GRACE_PERIOD - only if not
                // is DCF77 used to set the time.
                bool ntpCurrentlyAvailable = (WiFi.getMode() == WIFI_STA && WiFi.isConnected() &&
                    lastNtpSuccessMillis != 0 &&
                    (millis() - lastNtpSuccessMillis) < DCF77_NTP_GRACE_PERIOD);

                if (!ntpCurrentlyAvailable && (millis() - lastRTCUpdate > WAIT_6h || dcfTimeFound == false)) {

                    lastRTCUpdate = millis();

                    localtime_r(&DCFtime, &timeinfo); // Konvertiere time_t in struct tm
                                                      // convert time_t to struct tm
                    setTimeStruct(timeinfo, "[DCF77] set time");         // Übergabe der struct tm an die Funktion
                                                                         // pass struct tm to the function

                    if (rtcOk == RTC_AVAILABLE) {

                        // Setze die RTC mit der synchronisierten Zeit
                        // Set the RTC to the synchronized time
                        rtc.adjust(DateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                            timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));
                        DEBUG_PRINTLN("[DCF77] RTC updated with DCF77 time");
                    }
                }

                dcfTimeFound = true;
                lastDcfSyncTime = DCFtime;
            }
        }
        return dcfTimeFound;
#else
        return false;
#endif
    }


    //
    // NTP-Zeitsynchronisation um 02:00:05 und 03:00:05

    // NTP time sync at 02:00:05 and 03:00:05
    //

    void checkNightlyTimeSync() {
        static bool triggered2 = false;
        static bool triggered3 = false;

        if (timeinfo.tm_hour == 2 && timeinfo.tm_min == 0 && timeinfo.tm_sec == 5 && !triggered2) {
            DEBUG_PRINTLN("[TIME SYNC] Triggered at 02:00:05");
            lastCheck = millis();
            triggered2 = true;
            setupNTP();
        }

        if (timeinfo.tm_hour == 3 && timeinfo.tm_min == 0 && timeinfo.tm_sec == 5 && !triggered3) {
            DEBUG_PRINTLN("[TIME SYNC] Triggered at 03:00:05");
            lastCheck = millis();
            triggered3 = true;
            setupNTP();
        }

        if (timeinfo.tm_hour > 3 && (triggered2 || triggered3)) {
            triggered2 = false;
            triggered3 = false;
        }
    }


    // Testet einen NTP-Server per direkter UDP-Anfrage (RFC 5905, minimales
    // Client-Paket), ohne die Systemzeit zu veraendern - nutzt eine eigene
    // lokale WiFiUDP-Instanz. Gibt bei Erfolg die UTC-Zeit als String zurueck.

    // Tests an NTP server via a direct UDP request (RFC 5905, minimal client
    // packet), without changing the system time - uses its own local WiFiUDP
    // instance. Returns the UTC time as a string on success, empty otherwise.

    String testNtpServer(const String& server) {
        if (WiFi.getMode() != WIFI_STA || !WiFi.isConnected()) return "";

        WiFiUDP testUdp;
        if (!testUdp.begin(0)) return ""; // beliebiger freier lokaler Port
                                          // any free local port

        IPAddress serverIp;
        if (!WiFi.hostByName(server.c_str(), serverIp)) {
            testUdp.stop();
            return "";
        }

        uint8_t packet[48];
        memset(packet, 0, sizeof(packet));
        packet[0] = 0b11100011; // LI=3 (unbekannt), VN=4, Mode=3 (Client)
                                // LI=3 (unknown), VN=4, Mode=3 (client)

        testUdp.beginPacket(serverIp, 123);
        testUdp.write(packet, sizeof(packet));
        testUdp.endPacket();

        unsigned long waitStart = millis();
        int received = 0;
        while (millis() - waitStart < WAIT_3s) {
            received = testUdp.parsePacket();
            if (received >= 48) break;
            delay(20);
        }

        if (received < 48) {
            testUdp.stop();
            return "";
        }

        testUdp.read(packet, 48);
        testUdp.stop();

        // Transmit-Timestamp: Sekunden seit 1900 in Byte 40-43 (big-endian)
        // Transmit timestamp: seconds since 1900 in bytes 40-43 (big-endian)
        uint32_t secsSince1900 = ((uint32_t)packet[40] << 24) | ((uint32_t)packet[41] << 16) |
                                 ((uint32_t)packet[42] << 8) | (uint32_t)packet[43];
        const uint32_t SEVENTY_YEARS = 2208988800UL; // Differenz 1900 -> 1970
                                                     // difference 1900 -> 1970
        if (secsSince1900 < SEVENTY_YEARS) return "";
        time_t epochTime = secsSince1900 - SEVENTY_YEARS;

        struct tm resultTime;
        gmtime_r(&epochTime, &resultTime);
        char buf[32];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &resultTime);
        return String(buf) + " UTC";
    }


    // Initialisiert die Zeitsynchronisierung über NTP und stellt die Zeitzone ein.
    // Im Fehlerfall wird die zuletzt bekannte Zeit verwendet.

    // Initializes NTP time synchronization and sets the timezone.
    // On failure, the last known time is used.

    boolean setupNTP() {

        if (loggingEnabled) Serial.println("[NTP] Setting up NTP..");
        if (WiFi.getMode() != WIFI_STA || !WiFi.isConnected()) {
            //DEBUG_PRINTLN("[NTP] Skipping NTP setup: Not in STA mode or WiFi not connected");
            return true;
        }

        timezone = preferences.getString(PK_TIMEZONE, TIMEZONE_DEFAULT);

        for (int i = 0; i < MAX_WLAN; i++) {
            String ntpServer = ntpServers[i];
            if (ntpServer.length() == 0) continue;

            // Diagnose: DNS-Aufloesung separat pruefen und loggen, damit im
            // Fehlerfall im Log sichtbar wird, ob der Server ueberhaupt
            // erreichbar/aufloesbar war, statt nur "failed" ohne Ursache zu
            // sehen (siehe testNtpServer() weiter oben fuer denselben Ansatz).

            // Diagnostic: check and log DNS resolution separately, so on
            // failure the log shows whether the server was reachable/
            // resolvable at all, instead of just "failed" with no cause
            // (see testNtpServer() further above for the same approach).
            IPAddress ntpServerIp;
            if (WiFi.hostByName(ntpServers[i], ntpServerIp)) {
                DEBUG_PRINTLN("[NTP] Trying server: " + ntpServer + " (" + ntpServerIp.toString() + ")");
            }
            else {
                // Ohne aufloesbaren Namen kann auch der SNTP-Client den Server
                // nicht erreichen - direkt zum naechsten springen, statt unten
                // WAIT_3s auf eine Antwort zu warten, die nicht kommen kann.
                // Das haelt setupNTP() ohne Internet kurz: vorher hat die
                // Funktion (faelschlich) beim ersten Server sofort Erfolg
                // gemeldet, jetzt wird echt gewartet - ohne diesen Ausstieg
                // waeren das WAIT_3s pro konfiguriertem Server.

                // Without a resolvable name the SNTP client can't reach the
                // server either - skip straight to the next one instead of
                // waiting WAIT_3s below for a response that cannot arrive.
                // This keeps setupNTP() short when offline: previously the
                // function (wrongly) reported success on the first server
                // immediately, now it really waits - without this early exit
                // that would be WAIT_3s per configured server.
                DEBUG_PRINTLN("[NTP] DNS lookup failed for server: " + ntpServer);
                continue;
            }

            // getLocalTime() prueft NUR, ob das Jahr > 2016 ist - nicht, ob
            // tatsaechlich eine NTP-Antwort eingetroffen ist. Da die Systemzeit
            // beim Boot bereits von der RTC gesetzt wurde (loadTimeFromRTC()
            // laeuft in setup() VOR setupNTP()), meldete der erste Server
            // deshalb sofort Erfolg, ohne dass je ein Paket ankam. Folge:
            // lastNtpSuccessMillis wurde gesetzt und von checkNTPRetry()
            // staendig erneuert, wodurch ntpCurrentlyAvailable in
            // getDCF77Time() dauerhaft wahr blieb - DCF77 kam NIE zur
            // Zeituebernahme, und ein Geraet ohne Internet driftete mit der RTC
            // unkorrigiert weg.
            //
            // Loesung: die Systemzeit vorher sichern und bewusst auf einen
            // ungueltigen Wert (1970) setzen. getLocalTime() kann dann nur noch
            // true liefern, wenn der SNTP-Client die Zeit wirklich neu gesetzt
            // hat. Schlaegt der Server fehl, wird die gesicherte Zeit um die
            // verstrichene Wartezeit fortgeschrieben wieder eingesetzt, damit
            // die Uhr nicht auf 1970 stehen bleibt.

            // getLocalTime() ONLY checks whether the year is > 2016 - not
            // whether an NTP response actually arrived. Since the system time
            // was already set from the RTC at boot (loadTimeFromRTC() runs in
            // setup() BEFORE setupNTP()), the first server therefore reported
            // success immediately without a single packet arriving. Result:
            // lastNtpSuccessMillis was set and kept refreshed by
            // checkNTPRetry(), so ntpCurrentlyAvailable in getDCF77Time()
            // stayed permanently true - DCF77 NEVER got to set the time, and a
            // device without internet drifted along with the RTC uncorrected.
            //
            // Fix: save the system time beforehand and deliberately set it to
            // an invalid value (1970). getLocalTime() can then only return true
            // if the SNTP client really set the time anew. If the server fails,
            // the saved time is restored, advanced by the elapsed waiting time,
            // so the clock doesn't stay stuck at 1970.
            struct timeval savedTime;
            gettimeofday(&savedTime, nullptr);

            // Schwelle bewusst identisch zu der, die getLocalTime() intern
            // anlegt (Jahr > 2016) - sonst gaebe es ein Fenster, in dem eine
            // zwar gueltige, aber aeltere Systemzeit nicht wiederhergestellt
            // wuerde.
            // Threshold deliberately identical to the one getLocalTime() applies
            // internally (year > 2016) - otherwise there would be a window in
            // which a valid but older system time would not be restored.
            bool hadValidTime = (savedTime.tv_sec > 1483228800L); // 2017-01-01
            unsigned long syncStart = millis();

            struct timeval invalidTime = { 0, 0 };
            settimeofday(&invalidTime, nullptr);

            configTzTime(timezone.c_str(), ntpServers[i]);

            // 500ms waren in der Praxis oft zu knapp fuer DNS-Aufloesung plus
            // NTP-Antwort ueber das offene Internet - auf 3s verlaengert, wie
            // bei testNtpServer() weiter oben.

            // 500ms was often too short in practice for DNS resolution plus
            // the NTP response over the open internet - extended to 3s,
            // matching testNtpServer() further above.
            if (getLocalTime(&timeinfo, WAIT_3s)) {

                DEBUG_PRINTLN("[NTP] Time synchronized successfully with " + ntpServer);
                lastNtpSuccessMillis = millis();

                if (rtcOk == RTC_AVAILABLE || rtcOk == RTC_AVAILABLE_BUT_INVALID) {

                    // Setze die RTC mit der synchronisierten Zeit
                    // Set the RTC to the synchronized time
                    rtc.adjust(DateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                        timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));
                    DEBUG_PRINTLN("[RTC] RTC updated with NTP time");

                }
                return true;
            }

            // Keine Server-Antwort: die oben absichtlich ungueltig gemachte
            // Systemzeit wieder auf den gesicherten Stand setzen, fortgeschrieben
            // um die waehrend des Versuchs verstrichene Zeit.

            // No server response: restore the system time that was
            // deliberately invalidated above to its saved value, advanced by
            // the time elapsed during the attempt.
            if (hadValidTime) {
                struct timeval restoreTime;
                restoreTime.tv_sec = savedTime.tv_sec + (time_t)((millis() - syncStart) / 1000);
                restoreTime.tv_usec = savedTime.tv_usec;
                settimeofday(&restoreTime, nullptr);
            }

            DEBUG_PRINTLN("[NTP] Failed to synchronize with server: " + ntpServer);
        }
        handleNTPFailure();
        return false;
    }


    // Behandelt NTP-Sync-Fehler: nutzt die letzte bekannte Zeit oder setzt 12:00 Uhr.
    // Handles NTP sync failure: uses the last known time or sets 12:00.

    void handleNTPFailure() {
        DEBUG_PRINTLN("[NTP] Handling NTP synchronization failure..");

        // Versuche, die letzte bekannte Zeit zu verwenden
        // Try to use the last known time
       // struct tm timeinfo;
        if (getLocalTime(&timeinfo, 100)) {
            char timeStr[32];
            strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
            DEBUG_PRINTLN("[NTP] Using last known time: " + String(timeStr));
        }
        else {
            // Wenn keine gültige Zeit verfügbar ist, auf 12:00 Uhr setzen
            // If no valid time is available, set it to 12:00
            DEBUG_PRINTLN("[NTP] No valid time available. Setting time to 12:00");
            timeinfo.tm_hour = 12;
            timeinfo.tm_min = 0;
            timeinfo.tm_sec = 0;
            timeinfo.tm_year = 126; // Jahr 2026 (1900 + 126)
                                    // year 2026 (1900 + 126)
            timeinfo.tm_mon = 0;    // Januar
                                    // January
            timeinfo.tm_mday = 1;   // 1. Tag des Monats
                                    // 1st day of the month
            setTimeStruct(timeinfo, "[NTP]"); // Funktion, um die Zeit zu setzen
                                              // function to set the time
        }
        // Optional: Wiederholung später einplanen
        // Optionally, schedule a retry later
        scheduleNTPRetry();
    }


    // Setzt die Systemzeit manuell anhand einer `tm`-Struktur.
    // Sets the system time manually from a `tm` struct.

    void setTimeStruct(const struct tm& timeinfo, String source) {

        // Konvertiere struct tm in time_t (unter Berücksichtigung der Zeitzone)
        // Convert struct tm to time_t (taking the timezone into account)
        time_t t = mktime(const_cast<struct tm*>(&timeinfo));

        // Setze die Systemzeit
        // Set the system time
        timeval tv = { t, 0 }; // Sekunden und Mikrosekunden
                               // seconds and microseconds
        settimeofday(&tv, nullptr);

        char buffer[64];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S %Z", &timeinfo);
        DEBUG_PRINT(source + " ");
        DEBUG_PRINTLN(buffer); // Gibt die lokale Zeit und die Zeitzone aus
                               // prints the local time and timezone
    }


    // Plant einen NTP-Wiederholungsversuch in 30 Minuten.
    // Schedules an NTP retry in 30 minutes.

    void scheduleNTPRetry() {
        lastNTPRetry = millis();
        DEBUG_PRINTLN("[NTP] Scheduled retry in 30 minutes");
    }


    // Prüft, ob ein NTP-Wiederholungsversuch fällig ist, und führt ihn ggf. aus.
    // Checks whether an NTP retry is due and runs it if so.

    void checkNTPRetry() {
        if (WiFi.status() != WL_CONNECTED) {
            //DEBUG_PRINTLN("[NTP] Skipping retry: Not connected to WiFi");
            lastNTPRetry = millis();
            return;
        }
        if (lastNTPRetry > 0 && millis() - lastNTPRetry >= WAIT_1h) {
            // DEBUG_PRINTLN("[NTP] Retrying NTP synchronization..");
            setupNTP();
            lastNTPRetry = millis();
        }
    }


    // --- Funktion: Scannt den I2C-Bus nach Geräten und gibt die Anzahl der gefundenen Geräte zurück ---
    // --- Function: scans the I2C bus for devices and returns the number found ---

    uint16_t i2cScan() {
        byte error, address;
        int nDevices = 0;

        int rtc3231Addr = 0x68;

        i2cAddr = "";

        // teste direkt auf 0x68
        // test directly at 0x68
        Wire.beginTransmission(rtc3231Addr);
        error = Wire.endTransmission();
        if (error == 0) {
            i2cAddr = "0x68 (RTC DS3231)";
            DEBUG_PRINTLN("[I2C] " + i2cAddr);
            return 1;
        }

        DEBUG_PRINTLN("[I2C] Scanning I2C..");

        for (address = 1; address < 127; address++) {
            Wire.beginTransmission(address);
            error = Wire.endTransmission();
            if (error == 0) {
                DEBUG_PRINT("[I2C]   I2C device found at address 0x");
                if (address < 16) {
                    DEBUG_PRINT("0");
                }
                DEBUG_PRINTLN(String(address, HEX) + " ");

                // Zaehler wurde vorher nie hochgezaehlt: die Funktion lieferte
                // deshalb IMMER 0 und loggte "No I2C devices found", auch wenn
                // direkt darueber Geraete ausgegeben wurden. Ein RTC-Modul auf
                // einer abweichenden Adresse (oder ein anderes I2C-Geraet)
                // wurde vom Aufrufer in uhr3.ino dadurch nie erkannt.

                // The counter was never incremented: the function therefore
                // ALWAYS returned 0 and logged "No I2C devices found", even
                // when devices were printed right above. An RTC module on a
                // different address (or any other I2C device) was therefore
                // never detected by the caller in uhr3.ino.
                nDevices++;

                if (i2cAddr != "") i2cAddr += ", ";
                i2cAddr += "0x" + String(address, HEX);
            }
            else if (error == 4) {
                DEBUG_PRINT("[I2C] Unknow error at address 0x");
                if (address < 16) {
                    DEBUG_PRINT("0");
                }
                DEBUG_PRINTLN(String(address, HEX) + " ");
            }
        }
        if (nDevices == 0) {
            DEBUG_PRINTLN("[I2C] No I2C devices found");
        }
        else {
            DEBUG_PRINTLN("[I2C] done");
        }

        return nDevices;
    }


    // Funktion, um ein NTP-Paket zu erstellen
    // Function to build an NTP packet

    void createNtpResponse(byte* packet, time_t currentTime) {

        // Originate Timestamp: der Transmit-Timestamp der ANFRAGE (Byte 40-47)
        // muss unveraendert in Byte 24-31 der Antwort zurueckgespiegelt werden.
        // RFC-konforme Clients (ntpd, chrony, systemd-timesyncd) vergleichen
        // dieses Feld mit dem Zeitstempel, den sie selbst gesendet haben, und
        // verwerfen die Antwort sonst als "bogus packet" - vorher blieb das
        // Feld durchgehend 0, die Antwort war also fuer echte NTP-Clients
        // unbrauchbar. Muss VOR dem memset gesichert werden, da Anfrage und
        // Antwort denselben Puffer benutzen (siehe Aufrufstelle in uhr3.ino).

        // Originate Timestamp: the REQUEST's transmit timestamp (bytes 40-47)
        // has to be mirrored back unchanged into bytes 24-31 of the response.
        // RFC-compliant clients (ntpd, chrony, systemd-timesyncd) compare this
        // field against the timestamp they sent themselves and otherwise
        // discard the response as a "bogus packet" - previously the field
        // stayed 0 throughout, making the response unusable for real NTP
        // clients. Has to be saved BEFORE the memset, since request and
        // response share the same buffer (see the call site in uhr3.ino).
        byte originateTimestamp[8];
        memcpy(originateTimestamp, &packet[40], sizeof(originateTimestamp));

        memset(packet, 0, NTP_PACKET_SIZE);

        // Flags und Stratum
        // Flags and stratum
        packet[0] = 0b00100100; // LI, Version, Mode
        packet[1] = 1;          // Stratum
        packet[2] = 6;          // Poll Interval
        packet[3] = 0xEC;       // Precision

        // Root Delay und Root Dispersion
        // Root Delay and Root Dispersion
        packet[4] = 0;
        packet[5] = 0;
        packet[6] = 0;
        packet[7] = 0;

        // Reference Identifier
        packet[12] = 'L';
        packet[13] = 'O';
        packet[14] = 'C';
        packet[15] = 'L';

        // Reference Timestamp
        time_t refTime = currentTime - 1; // Referenzzeit (1 Sekunde vorher)
                                          // reference time (1 second earlier)
        uint32_t refSeconds = htonl((uint32_t)(refTime + 2208988800UL));
        memcpy(&packet[16], &refSeconds, 4);

        // Originate Timestamp: gespiegelter Transmit-Timestamp der Anfrage
        // (oben vor dem memset gesichert), damit der Client die Antwort
        // seiner eigenen Anfrage zuordnen kann.
        // Originate Timestamp: mirrored transmit timestamp of the request
        // (saved above before the memset), so the client can match the
        // response to its own request.
        memcpy(&packet[24], originateTimestamp, sizeof(originateTimestamp));

        uint32_t nowSeconds = htonl((uint32_t)(currentTime + 2208988800UL));

        // Receive Timestamp: Zeitpunkt, zu dem die Anfrage eingetroffen ist.
        // Blieb vorher 0; Clients berechnen daraus Verzoegerung und Offset.
        // Receive Timestamp: the moment the request arrived. Previously stayed
        // 0; clients use it to compute delay and offset.
        memcpy(&packet[32], &nowSeconds, 4);

        // Transmit Timestamp
        memcpy(&packet[40], &nowSeconds, 4);
    }
