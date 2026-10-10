#pragma once

    // Zeit: RTC, DCF77, NTP-Client und -Server, Zeitzone (benoetigt globals.h, config.h, prefs_keys.h,
    // declarations.h). Die ISR reagiert auf jede Flanke am DCF77-Pin und schreibt nur einen Zeitstempel;
    // Auswertung in processDcf77Bits().

    // Time: RTC, DCF77, NTP client and server, time zone (requires globals.h, config.h, prefs_keys.h,
    // declarations.h). The ISR fires on every edge on the DCF77 pin and only writes a timestamp; evaluated in
    // processDcf77Bits().

    void IRAM_ATTR isr() {

        // Die ISR ruft nichts im Flash auf (bei abgeschaltetem Flash-Cache drohen verlorene Flanken oder
        // Panic) - daher kein digitalRead() und kein LED-GPIO. Den LED-Blitz fordert erst processDcf77Bits()
        // an, sobald dcf77Confirmed gilt.

        // The ISR calls nothing in flash (with the flash cache off this risks lost edges or a panic) - hence
        // no digitalRead() and no LED GPIO. The LED flash is only requested by processDcf77Bits() once
        // dcf77Confirmed is true.

        dcf77Count++;
        if (dcf77Count > 120) dcf77Count = 1;

        // Bei vollem Puffer wird die Flanke verworfen, statt den noch
        // ungelesenen Tail zu ueberschreiben (Zaehler siehe dcf77EdgeDropped).

        // On a full buffer the edge is dropped instead of overwriting the
        // not-yet-read tail (counter: see dcf77EdgeDropped).

        uint8_t dcf77EdgeNextHead = (dcf77EdgeHead + 1) % DCF77_EDGE_BUFFER_SIZE;
        if (dcf77EdgeNextHead != dcf77EdgeTail) {
            dcf77EdgeMillis[dcf77EdgeHead] = millis();
            dcf77EdgeHead = dcf77EdgeNextHead;
        }
        else {
            dcf77EdgeDropped++;
        }
    }


    // Loggt die Differenz zwischen der Zeit vor einer Synchronisation (oldTime, gesichert zu oldTimeMillis)
    // und der neu gesetzten: positiv = Uhr ging nach, negativ = sie ging vor - macht Drift zwischen
    // NTP/DCF77/RTC sichtbar. Ohne Wirkung, wenn oldTime keine sinnvolle Zeit war.

    // Logs the difference between the time before a sync (oldTime, saved at oldTimeMillis) and the newly set
    // one: positive = clock was behind, negative = it was ahead - surfaces drift between NTP/DCF77/RTC. No-op
    // if oldTime was not a meaningful time.

    void logTimeSyncDifference(const String& source, const struct timeval& oldTime, unsigned long oldTimeMillis) {
        if (oldTime.tv_sec <= 1483228800L) return; // 2017-01-01 - keine sinnvolle vorherige Zeit
                                                   // 2017-01-01 - no meaningful previous time
        struct timeval newTime;
        gettimeofday(&newTime, nullptr);
        time_t extrapolatedOldTime = oldTime.tv_sec + (time_t)((millis() - oldTimeMillis) / 1000);
        long diffSeconds = (long)(newTime.tv_sec - extrapolatedOldTime);

        // Nur bei tatsaechlich nennenswerter Abweichung loggen (nicht bei
        // -1/0/1s) - kleinere Differenzen sind normale Rundung/Latenz, kein
        // Anzeichen fuer Drift.

        // Only log an actually notable difference (not -1/0/1s) - smaller
        // differences are normal rounding/latency, not a sign of drift.

        if (diffSeconds > 1 || diffSeconds < -1) {
            DEBUG_PRINTLN(source + " clock drift: " + String(diffSeconds) + "s");
        }
    }


    // Lädt die Zeit vom RTC-Modul und setzt die Systemzeit entsprechend
    // Loads the time from the RTC module and sets the system time accordingly

    void loadTimeFromRTC() {
        if (rtcOk == RTC_AVAILABLE) {

            struct timeval oldTime;
            gettimeofday(&oldTime, nullptr);
            unsigned long oldTimeMillis = millis();

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
            logTimeSyncDifference("[RTC]", oldTime, oldTimeMillis);
        }
    }


    // Sind ALLE NTP-Slots leer, werden nur Slot 0 und 1 mit den eingebauten Standardservern gefuellt; ein
    // eigener Server irgendwo verhindert das. Aufruf beim Start und vor jedem Sync - ein zur Laufzeit
    // geleertes ntpServers[] faellt sofort zurueck.

    // If ALL NTP slots are empty, only slot 0 and 1 are filled with the built-in default servers; a custom
    // server anywhere prevents that. Called at boot and before every sync - an ntpServers[] emptied at
    // runtime falls back immediately.

    void applyNtpServerDefaultsIfNoneConfigured() {
        for (int i = 0; i < MAX_WLAN; i++) {
            if (strlen(ntpServers[i]) > 0) return; // mindestens ein eigener Server konfiguriert
                                                   // at least one custom server configured
        }
        strncpy(ntpServers[0], NTP_SERVER_1, sizeof(ntpServers[0]) - 1);
        ntpServers[0][sizeof(ntpServers[0]) - 1] = '\0';
        strncpy(ntpServers[1], NTP_SERVER_2, sizeof(ntpServers[1]) - 1);
        ntpServers[1][sizeof(ntpServers[1]) - 1] = '\0';
        DEBUG_PRINTLN("[NTP] No NTP server configured, falling back to defaults: " + String(NTP_SERVER_1) + ", " + String(NTP_SERVER_2));
    }


    // Prueft einen POSIX-TZ-String (Name[Offset[Name[Offset]][,Regel]], z.B. "CET-1CEST,M3.5.0,M10.5.0/3")
    // auf Grammatik und Wertebereiche - faengt typische Tippfehler ab, bevor ein kaputter String still als
    // UTC endet.

    // Checks a POSIX TZ string (Name[Offset[Name[Offset]][,Rule]], e.g. "CET-1CEST,M3.5.0,M10.5.0/3") for
    // grammar and value ranges - catches typical typos before a broken string silently ends up as UTC.

    bool isValidPosixTimezone(const String& tz) {
        int len = tz.length();

        // Muss in timezoneSnapshot[] passen (siehe globals.h) - sonst wuerde
        // ein laengerer, aber gueltiger String hier durchgehen und beim
        // Kopieren stillschweigend abgeschnitten werden.

        // Must fit into timezoneSnapshot[] (see globals.h) - otherwise a
        // longer but valid string would pass here and get silently
        // truncated when copied.

        if (len == 0 || len >= (int)sizeof(timezoneSnapshot)) return false;

        // Name: entweder <...> (mind. 1 Zeichen) oder mind. 3 Buchstaben
        // name: either <...> (at least 1 char) or at least 3 letters

        auto parseName = [&](int& i) -> bool {
            if (i < len && tz[i] == '<') {
                int start = ++i;
                while (i < len && tz[i] != '>') i++;
                if (i >= len || i - start < 1) return false;
                i++; // '>'
                return true;
            }
            int start = i;
            while (i < len && isAlpha(tz[i])) i++;
            return (i - start) >= 3;
        };

        // Optionale Minuten und Sekunden ":mm[:ss]" (je 0-59) nach den Stunden
        // Optional minutes and seconds ":mm[:ss]" (0-59 each) after the hours

        auto parseMinSec = [&](int& i) -> bool {
            for (int part = 0; part < 2; part++) {
                if (i >= len || tz[i] != ':') break;
                int save = i;
                i++;
                int mstart = i;
                while (i < len && isDigit(tz[i])) i++;
                if (i == mstart) { i = save; break; }
                int mm = tz.substring(mstart, i).toInt();
                if (mm < 0 || mm > 59) return false;
            }
            return true;
        };

        // Offset: [+-]hh[:mm[:ss]], hh 0-24, mm/ss 0-59 falls vorhanden.
        // required=false: Abwesenheit ist ok (z.B. optionaler DST-Offset).

        // offset: [+-]hh[:mm[:ss]], hh 0-24, mm/ss 0-59 if present.
        // required=false: absence is ok (e.g. optional DST offset).

        auto parseOffset = [&](int& i, bool required) -> bool {
            int start = i;
            if (i < len && (tz[i] == '+' || tz[i] == '-')) i++;
            int digitsStart = i;
            while (i < len && isDigit(tz[i])) i++;
            if (i == digitsStart) { i = start; return !required; }
            int hh = tz.substring(digitsStart, i).toInt();
            if (hh < 0 || hh > 24) return false;
            return parseMinSec(i);
        };

        // Datum einer Regel: Jn (1-365), n (0-365) oder Mm.w.d (m 1-12, w
        // 1-5, d 0-6 [0=Sonntag]).

        // rule date: Jn (1-365), n (0-365) or Mm.w.d (m 1-12, w 1-5, d 0-6
        // [0=Sunday]).

        auto parseDate = [&](int& i) -> bool {
            if (i >= len) return false;
            if (tz[i] == 'J') {
                i++;
                int start = i;
                while (i < len && isDigit(tz[i])) i++;
                if (i == start) return false;
                int val = tz.substring(start, i).toInt();
                return val >= 1 && val <= 365;
            }
            if (tz[i] == 'M') {
                i++;
                int start = i;
                while (i < len && isDigit(tz[i])) i++;
                if (i == start) return false;
                int month = tz.substring(start, i).toInt();
                if (month < 1 || month > 12) return false;
                if (i >= len || tz[i] != '.') return false;
                i++;
                start = i;
                while (i < len && isDigit(tz[i])) i++;
                if (i == start) return false;
                int week = tz.substring(start, i).toInt();
                if (week < 1 || week > 5) return false;
                if (i >= len || tz[i] != '.') return false;
                i++;
                start = i;
                while (i < len && isDigit(tz[i])) i++;
                if (i == start) return false;
                int day = tz.substring(start, i).toInt();
                return day >= 0 && day <= 6;
            }
            if (isDigit(tz[i])) {
                int start = i;
                while (i < len && isDigit(tz[i])) i++;
                int val = tz.substring(start, i).toInt();
                return val >= 0 && val <= 365;
            }
            return false;
        };

        // Zeit einer Regel: /[+-]hh[:mm[:ss]] - grosszuegiger Bereich
        // (glibc-Erweiterung: -167..167), damit auch ungewoehnliche, aber
        // reale Uebergangszeiten (z.B. "/24") nicht faelschlich abgelehnt werden.

        // rule time: /[+-]hh[:mm[:ss]] - generous range (glibc extension:
        // -167..167), so unusual but real transition times (e.g. "/24")
        // aren't rejected incorrectly.

        auto parseRuleTime = [&](int& i) -> bool {
            if (i < len && tz[i] == '/') {
                i++;
                if (i < len && (tz[i] == '+' || tz[i] == '-')) i++;
                int digitsStart = i;
                while (i < len && isDigit(tz[i])) i++;
                if (i == digitsStart) return false;
                int hh = tz.substring(digitsStart, i).toInt();
                if (hh < -167 || hh > 167) return false;
                return parseMinSec(i);
            }
            return true;
        };

        int i = 0;
        if (!parseName(i)) return false;
        if (!parseOffset(i, true)) return false;
        if (i >= len) return true; // nur Standardzeit, keine Sommerzeit
                                   // just standard time, no DST

        if (!parseName(i)) return false; // DST-Name
                                         // DST name
        parseOffset(i, false); // optionaler DST-Offset
                               // optional DST offset

        if (i >= len) return true; // DST-Name (+Offset) ohne Regel - laut POSIX gueltig (Default-Regel)
                                   // DST name (+offset) without a rule - valid per POSIX (default rule)

        if (tz[i] != ',') return false;
        i++;
        if (!parseDate(i)) return false;
        if (!parseRuleTime(i)) return false;
        if (i >= len || tz[i] != ',') return false;
        i++;
        if (!parseDate(i)) return false;
        if (!parseRuleTime(i)) return false;

        return i == len; // gesamter String muss verbraucht sein - kein Muell am Ende
                         // entire string must be consumed - no trailing garbage
    }


    // Faellt auf TIMEZONE_DEFAULT zurueck, wenn `timezone` leer oder ungueltig ist - configTzTime() machte
    // daraus UTC. Kann durch einen fehlerhaften gespeicherten Wert entstehen (/set_timezone, /api/setMode,
    // Preset).

    // Falls back to TIMEZONE_DEFAULT if `timezone` is empty or invalid - configTzTime() would turn it into
    // UTC. Can result from a malformed stored value (/set_timezone, /api/setMode, preset).

    void applyTimezoneDefaultIfInvalid() {
        if (!isValidPosixTimezone(timezone)) {
            DEBUG_PRINTLN("[NTP] Timezone '" + timezone + "' is empty or invalid, falling back to default: " + String(TIMEZONE_DEFAULT));
            timezone = TIMEZONE_DEFAULT;
        }
    }


    // Wendet `timezone` sofort auf die Systemzeit an (TZ + tzset()) statt
    // erst beim naechsten NTP-Versuch (configTzTime()) - sonst wird die als
    // Ortszeit gespeicherte RTC-/DCF77-Zeit als UTC gelesen (1-2h Versatz).

    // Applies `timezone` to the system time right away (TZ + tzset())
    // instead of only on the next NTP attempt (configTzTime()) - otherwise
    // RTC/DCF77 time, stored as local time, is read as UTC (1-2h offset).

    void applyTimezoneToSystem() {
        applyTimezoneDefaultIfInvalid();
        setenv("TZ", timezone.c_str(), 1);
        tzset();
    }


    // Initialisiere die NTP-Server
    // Initialize the NTP servers

    void initializeNtpServers() {
        for (int i = 0; i < MAX_WLAN; i++) {
            String ntpServerKey = pkNtpServer(i);
            String ntpServerValue = preferences.getString(ntpServerKey.c_str(), "");
            strncpy(ntpServers[i], ntpServerValue.c_str(), sizeof(ntpServers[i]) - 1);
            ntpServers[i][sizeof(ntpServers[i]) - 1] = '\0'; // Null-terminieren
                                                             // Null-terminate
            if (ntpServerValue.length() > 0) {
                DEBUG_PRINTLN("[NTP] Loaded NTP server " + String(i + 1) + ": " + String(ntpServers[i]));
            }
        }

        // Falls kein einziger Server gespeichert ist (weder je konfiguriert
        // noch durch den Erststart-Block in uhr4.ino vorbelegt), hier auf die
        // eingebauten Standardserver zurueckfallen.

        // If not a single server is stored (neither ever configured nor
        // pre-filled by the first-start block in uhr4.ino), fall back to the
        // built-in default servers here.

        applyNtpServerDefaultsIfNoneConfigured();
    }


    // Baut aus dcf77LastDecoded eine LOKALE struct tm. DCF77 sendet bereits
    // Lokalzeit inkl. Sommerzeit-Bit, daher keine UTC-Umrechnung noetig.
    // Gemeinsam genutzt von updateDcf77Status() und applyDcf77DecodedTime().

    // Builds a LOCAL struct tm from dcf77LastDecoded. DCF77 already transmits
    // local time incl. summer-time bit, so no UTC conversion is needed.
    // Shared by updateDcf77Status() and applyDcf77DecodedTime().

    struct tm dcf77DecodedToLocalTm() {
        struct tm t = {};
        t.tm_year = dcf77LastDecoded.year - 1900;
        t.tm_mon  = dcf77LastDecoded.month - 1;
        t.tm_mday = dcf77LastDecoded.day;
        t.tm_hour = dcf77LastDecoded.hour;
        t.tm_min  = dcf77LastDecoded.minute;
        t.tm_sec  = 0;
        t.tm_isdst = dcf77LastDecoded.dst ? 1 : 0; // direkt aus dem DCF77-Sommerzeit-Bit, nicht raten lassen
                                                   // taken directly from the DCF77 summer-time bit, not guessed
        return t;
    }


    // Haelt dcfTimeFound/lastDcfSyncTime (Statusanzeige) auf dem Stand des
    // eigenen Dekoders - UNABHAENGIG davon, ob DCF77 gerade tatsaechlich die
    // Zeit stellt (das entscheidet applyDcf77DecodedTime()/uhr4.ino).

    // Keeps dcfTimeFound/lastDcfSyncTime (status display) in sync with the own
    // decoder - INDEPENDENT of whether DCF77 currently drives the system time
    // (that's decided by applyDcf77DecodedTime()/uhr4.ino).

    bool updateDcf77Status() {
        static unsigned long lastSeenDecodeMillis = 0; // welches dcf77LastDecoded wurde hier zuletzt gesehen
                                                       // which dcf77LastDecoded was last seen here

        if (millis() - lastDCFUpdate > WAIT_1s) {
            lastDCFUpdate = millis(); // Timer zurücksetzen
                                      // reset timer

            if (dcf77LastDecoded.valid && dcf77LastDecoded.decodedAtMillis != lastSeenDecodeMillis) {
                lastSeenDecodeMillis = dcf77LastDecoded.decodedAtMillis;

                struct tm t = dcf77DecodedToLocalTm();
                lastDcfSyncTime = mktime(&t);
                dcfTimeFound = true;
                setLedOff(); // LED ausschalten, wenn Zeit gefunden wurde
                             // turn off LED once time is found
            }
        }
        return dcfTimeFound;
    }


    // Abweichung der aktuellen RTC-Zeit von `newLocal` in Sekunden - fuer
    // das Diff-Logging und die 2s-Schwelle bei einem RTC-Update via NTP/
    // DCF77 (siehe pollNtpSyncTask()/applyDcf77DecodedTime()).

    // Deviation of the current RTC time from `newLocal` in seconds - for
    // the diff logging and the 2s threshold on an RTC update via NTP/
    // DCF77 (see pollNtpSyncTask()/applyDcf77DecodedTime()).

    long rtcDriftSec(struct tm newLocal) {
        DateTime oldRtcTime = rtc.now();
        struct tm oldRtcTm = {};
        oldRtcTm.tm_year = oldRtcTime.year() - 1900;
        oldRtcTm.tm_mon = oldRtcTime.month() - 1;
        oldRtcTm.tm_mday = oldRtcTime.day();
        oldRtcTm.tm_hour = oldRtcTime.hour();
        oldRtcTm.tm_min = oldRtcTime.minute();
        oldRtcTm.tm_sec = oldRtcTime.second();
        oldRtcTm.tm_isdst = -1;
        return (long)(mktime(&newLocal) - mktime(&oldRtcTm));
    }


    // Uebernimmt dcf77LastDecoded als Systemzeit (+RTC) - DCF77-Fallback des
    // periodischen Sync-Blocks, wenn NTP nicht verfuegbar ist. False, wenn
    // kein frisches (DCF77_DECODED_MAX_AGE), paritaets-korrektes Telegramm vorliegt.

    // Applies dcf77LastDecoded as the system time (+RTC) - DCF77 fallback of
    // the periodic sync block when NTP is unavailable. False when no fresh
    // (DCF77_DECODED_MAX_AGE), parity-correct telegram is available.

    bool applyDcf77DecodedTime(String source) {
        if (!dcf77LastDecoded.valid) {
            DEBUG_PRINTLN(source + " skipped: no valid DCF77 telegram decoded yet");
            return false;
        }
        if (millis() - dcf77LastDecoded.decodedAtMillis >= DCF77_DECODED_MAX_AGE) {
            DEBUG_PRINTLN(source + " skipped: last decoded DCF77 telegram is too old/stale");
            return false; // zu alt/veraltet
                          // too old/stale
        }

        struct tm dcfLocal = dcf77DecodedToLocalTm();

        // tm_sec ist immer 0 - seit decodedAtMillis verstrichene Zeit
        // ergaenzen, sonst stuende die Zeit bis zu ~59s zurueck.

        // tm_sec is always 0 - add the time elapsed since decodedAtMillis,
        // otherwise the time would read up to ~59s behind.

        time_t dcfEpoch = mktime(&dcfLocal);
        dcfEpoch += (time_t)((millis() - dcf77LastDecoded.decodedAtMillis) / 1000);
        localtime_r(&dcfEpoch, &dcfLocal);

        setTimeStruct(dcfLocal, source); // Übergabe der struct tm an die Funktion
                                         // pass struct tm to the function

        // RTC_AVAILABLE_BUT_INVALID bewusst mit zulassen (wie setupNTP()) -
        // DCF77 soll eine als "ungueltig" markierte RTC ebenso reparieren
        // koennen wie NTP.

        // Deliberately also allow RTC_AVAILABLE_BUT_INVALID (like setupNTP()) -
        // DCF77 should be able to repair an RTC flagged "invalid" just as
        // well as NTP.

        if (rtcOk == RTC_AVAILABLE || rtcOk == RTC_AVAILABLE_BUT_INVALID) {

            // Nur bei nennenswerter Abweichung tatsaechlich schreiben (siehe
            // RTC_UPDATE_MIN_DRIFT_SEC) - unnoetige I2C-Schreibzugriffe vermeiden.

            // Only actually write on a notable deviation (see
            // RTC_UPDATE_MIN_DRIFT_SEC) - avoid unnecessary I2C writes.

            long rtcDiffSec = rtcDriftSec(dcfLocal);
            if (labs(rtcDiffSec) >= RTC_UPDATE_MIN_DRIFT_SEC) {
                rtc.adjust(DateTime(dcfLocal.tm_year + 1900, dcfLocal.tm_mon + 1, dcfLocal.tm_mday,
                    dcfLocal.tm_hour, dcfLocal.tm_min, dcfLocal.tm_sec));
                DEBUG_PRINTLN("[RTC] RTC updated with DCF77 time (diff " + String(rtcDiffSec) + "s)");
            }
            rtcOk = RTC_AVAILABLE; // siehe Begruendung bei setupNTP() weiter oben
                                   // see the reasoning at setupNTP() further above
        }
        lastRTCUpdate = millis();

        DEBUG_PRINTLN(source + " succeeded");
        return true;
    }


    // Beobachtet dcf77Count auf echte Aenderungen (nie zurueckgesetzt, siehe
    // globals.h). Pflegt dcf77PlausiblePulseStreak/dcf77Confirmed: erst
    // DCF77_PRESENCE_MIN_STREAK plausibel getaktete Aenderungen gelten als Empfang.

    // Watches dcf77Count for real changes (never reset, see globals.h). Also
    // maintains dcf77PlausiblePulseStreak/dcf77Confirmed: only
    // DCF77_PRESENCE_MIN_STREAK plausibly timed changes count as reception.

    void checkDcf77Health() {
        static uint16_t lastSeenCount = 0;
        static unsigned long lastPlausibleChangeMillis = 0;
        uint16_t current = dcf77Count;
        if (current != lastSeenCount) {
            unsigned long now = millis();
            if (!dcf77Confirmed) {
                bool plausibleGap = (lastPlausibleChangeMillis != 0) &&
                                     (now - lastPlausibleChangeMillis) <= DCF77_PRESENCE_MAX_GAP_MS;
                dcf77PlausiblePulseStreak = plausibleGap ? (dcf77PlausiblePulseStreak + 1) : 1;
                if (dcf77PlausiblePulseStreak >= DCF77_PRESENCE_MIN_STREAK) {
                    dcf77Confirmed = true;
                }
            }
            lastPlausibleChangeMillis = now;
            lastDcf77PulseChangeMillis = now;
            lastSeenCount = current;
        }
    }


    // Dekodiert dcf77Bits[] nach DCF77-Format; bricht ab (valid=false) wenn
    // ein Bit fehlt. Bits: 0/20=Festbits, 17/18=Sommer/Winterzeit,
    // 21-58=Minute/Stunde/Datum (je inkl. Paritaet).

    // Decodes dcf77Bits[] per the DCF77 format; aborts (valid=false) if a bit
    // is missing. Bits: 0/20=fixed bits, 17/18=summer/winter time,
    // 21-58=minute/hour/date (each incl. parity).

    bool decodeDcf77Telegram(unsigned long decodedAtMillis) {

        // Strukturpruefung: Bit 0 muss 0, Bit 20 muss 1 sein, sonst steht der Dekoder auf falscher Sekunde
        // (false; nach mehreren Fehlern verwirft der Aufrufer die Marke). Danach von Rasterposition in
        // Sekundenfolge umsortieren.

        // Structure check: bit 0 must be 0, bit 20 must be 1, otherwise the decoder sits on the wrong second
        // (false; after repeated failures the caller discards the marker). Then reorder from grid position
        // into second order.

        if (dcf77MarkerPos < 0) return true; // ohne Marke ist keine Zuordnung moeglich
                                              // without the marker no mapping is possible

        int8_t bits[DCF77_TELEGRAM_BITS];
        for (uint8_t sec = 0; sec < DCF77_TELEGRAM_BITS; sec++) {
            bits[sec] = dcf77Bits[((uint8_t)dcf77MarkerPos + 1 + sec) % DCF77_GRID_SLOTS];
        }

        if (bits[0] == 1 || bits[20] == 0) {
            return false;
        }

        // Rekonstruktion fehlender Bits: ohne sie braucht es 42 lueckenlose
        // Sekunden. Rekonstruierbar: Bit 0/20 (Festwerte), Bit 17/18 (invers),
        // je ein fehlendes Bit pro Paritaetsgruppe (Minute/Stunde/Datum).

        // Reconstructing missing bits: without this a telegram needs 42
        // gapless seconds. Reconstructable: bit 0/20 (fixed), bit 17/18
        // (inverse), one missing bit per parity group (minute/hour/date).

        uint8_t repaired = 0;

        if (bits[0] < 0)  { bits[0] = 0;  repaired++; }
        if (bits[20] < 0) { bits[20] = 1; repaired++; }

        if (bits[17] < 0 && bits[18] >= 0) { bits[17] = bits[18] ? 0 : 1; repaired++; }
        else if (bits[18] < 0 && bits[17] >= 0) { bits[18] = bits[17] ? 0 : 1; repaired++; }

        // Genau ein fehlendes Bit je Gruppe aus der geraden Paritaet ergaenzen
        // (der Bereich schliesst das Paritaetsbit selbst mit ein).

        // Fill in exactly one missing bit per group from the even parity (the
        // range includes the parity bit itself).

        const uint8_t groupStart[3] = { 21, 29, 36 };
        const uint8_t groupEnd[3]   = { 28, 35, 58 };
        for (uint8_t g = 0; g < 3; g++) {
            int missingIndex = -1;
            uint8_t missingCount = 0;
            int sum = 0;
            for (uint8_t i = groupStart[g]; i <= groupEnd[g]; i++) {
                if (bits[i] < 0) { missingCount++; missingIndex = i; }
                else sum += bits[i];
            }
            if (missingCount == 1) {
                bits[missingIndex] = (int8_t)(sum % 2); // ergaenzt zur geraden Paritaet
                                                        // completes to even parity
                repaired++;
            }
        }

        // Vollstaendigkeit nur fuer Bits 17-58 verlangen (gehen in die Zeit
        // ein). Bits 1-15/19 (Wetter, Anruf, Schaltsekunde) sind nur Anzeige -
        // eine Luecke dort darf ein gueltiges Zeittelegramm nicht verwerfen.

        // Require completeness only for bits 17-58 (they feed into the time).
        // Bits 1-15/19 (weather, call, leap second) are display-only - a gap
        // there must not discard an otherwise valid time telegram.

        for (uint8_t i = 17; i < DCF77_TELEGRAM_BITS; i++) {
            if (i == 19) continue; // Schaltsekunden-Ankuendigung, hier nicht ausgewertet
                                    // leap second announcement, not evaluated here

            if (bits[i] < 0) {

                // Noch unvollstaendig - dcf77LastDecoded NICHT anfassen (letzte
                // gueltige Dekodierung bleibt sichtbar); true: eine Luecke
                // stellt die Minutenmarke nicht in Frage.

                // Still incomplete - do NOT touch dcf77LastDecoded (last valid
                // decoding stays visible); true: a gap doesn't call the minute
                // marker into question.

                return true;
            }
        }

        Dcf77Decoded result;

        // 0 bedeutet anderswo "nie dekodiert" (/api/dcf77status) - einen
        // echten Zeitstempel 0 daher auf 1 anheben.

        // 0 means "never decoded" elsewhere (/api/dcf77status) - so lift a
        // genuine timestamp of 0 to 1.

        result.decodedAtMillis = (decodedAtMillis == 0) ? 1 : decodedAtMillis;
        result.repairedBits = repaired;

        result.callBit = (bits[15] == 1);
        result.dst = (bits[17] == 1);

        // Minute (Bits 21-27, BCD), Paritaet Bit 28
        // Minute (bits 21-27, BCD), parity bit 28

        int minuteUnits = bits[21] + bits[22] * 2 + bits[23] * 4 + bits[24] * 8;
        int minuteTens = bits[25] + bits[26] * 2 + bits[27] * 4;
        result.minute = (uint8_t)(minuteTens * 10 + minuteUnits);
        int minuteParitySum = 0;
        for (int i = 21; i <= 28; i++) minuteParitySum += bits[i];
        result.parityMinOk = (minuteParitySum % 2) == 0;

        // Stunde (Bits 29-34, BCD), Paritaet Bit 35
        // Hour (bits 29-34, BCD), parity bit 35

        int hourUnits = bits[29] + bits[30] * 2 + bits[31] * 4 + bits[32] * 8;
        int hourTens = bits[33] + bits[34] * 2;
        result.hour = (uint8_t)(hourTens * 10 + hourUnits);
        int hourParitySum = 0;
        for (int i = 29; i <= 35; i++) hourParitySum += bits[i];
        result.parityHourOk = (hourParitySum % 2) == 0;

        // Datum: Tag (36-41), Wochentag (42-44), Monat (45-49), Jahr (50-57),
        // gemeinsame Paritaet Bit 58

        // Date: day (36-41), day of week (42-44), month (45-49), year
        // (50-57), combined parity bit 58

        int dayUnits = bits[36] + bits[37] * 2 + bits[38] * 4 + bits[39] * 8;
        int dayTens = bits[40] + bits[41] * 2;
        result.day = (uint8_t)(dayTens * 10 + dayUnits);

        result.weekday = (uint8_t)(bits[42] + bits[43] * 2 + bits[44] * 4);

        int monthUnits = bits[45] + bits[46] * 2 + bits[47] * 4 + bits[48] * 8;
        int monthTens = bits[49];
        result.month = (uint8_t)(monthTens * 10 + monthUnits);

        int yearUnits = bits[50] + bits[51] * 2 + bits[52] * 4 + bits[53] * 8;
        int yearTens = bits[54] + bits[55] * 2 + bits[56] * 4 + bits[57] * 8;
        result.year = (uint16_t)(2000 + yearTens * 10 + yearUnits);

        int dateParitySum = 0;
        for (int i = 36; i <= 58; i++) dateParitySum += bits[i];
        result.parityDateOk = (dateParitySum % 2) == 0;

        // Zusaetzlich Wertebereiche pruefen: Paritaet erkennt nur eine
        // UNGERADE Zahl gekippter Bits, zwei Fehler in einer Gruppe blieben
        // sonst unentdeckt (z.B. "Monat 15" haette die Zeit verstellt).

        // Also check value ranges: parity only detects an ODD number of
        // flipped bits, two errors in one group would otherwise go unnoticed
        // (e.g. "month 15" would have set a wrong time).

        bool rangesOk = (result.minute <= 59) &&
                        (result.hour <= 23) &&
                        (result.day >= 1 && result.day <= 31) &&
                        (result.month >= 1 && result.month <= 12) &&
                        (result.weekday >= 1 && result.weekday <= 7) &&
                        (bits[17] != bits[18]);

        bool selfConsistent = result.parityMinOk && result.parityHourOk && result.parityDateOk && rangesOk;

        // Zeitpunkt als Unix-Zeit - Bezugspunkt fuer Kohaerenzpruefung unten
        // und die naechste Minute.

        // This telegram's time as a Unix timestamp - the reference for the
        // coherence check below and the next minute.

        struct tm decodedTm = {};
        decodedTm.tm_year = result.year - 1900;
        decodedTm.tm_mon = result.month - 1;
        decodedTm.tm_mday = result.day;
        decodedTm.tm_hour = result.hour;
        decodedTm.tm_min = result.minute;
        decodedTm.tm_sec = 0;
        decodedTm.tm_isdst = result.dst ? 1 : 0;
        time_t decodedEpoch = selfConsistent ? mktime(&decodedTm) : 0;

        // Vollstaendig empfangen -> sofort gueltig. Rekonstruiert -> die
        // ergaenzte Gruppe ist ungeprueft (Paritaet stimmt per Konstruktion),
        // gilt erst, wenn es exakt zum Vorgaenger + verstrichenen Minuten passt.

        // Fully received -> valid immediately. Reconstructed -> the completed
        // group is unverified (parity holds by construction), so it counts
        // only once it matches the predecessor + elapsed minutes exactly.

        if (selfConsistent && repaired == 0) {
            result.valid = true;
        }
        else if (selfConsistent && dcf77PrevEpoch != 0) {
            unsigned long elapsedMs = result.decodedAtMillis - dcf77PrevAtMillis;
            long elapsedMinutes = (long)((elapsedMs + 30000UL) / 60000UL);
            time_t expected = dcf77PrevEpoch + (time_t)elapsedMinutes * 60;
            result.valid = (elapsedMinutes >= 1) && (decodedEpoch == expected);
            if (!result.valid) {
                DEBUG_PRINTLN("[DCF77] Reconstructed telegram does not match the previous one - waiting for confirmation");
            }
        }

        // Bezugspunkt merken, wenn das Telegramm stimmig ist - auch wenn
        // (noch) nicht gueltig. So bestaetigen sich zwei rekonstruierte
        // Telegramme gegenseitig.

        // Remember the reference when the telegram is self-consistent - even
        // if not (yet) valid. This is how two reconstructed telegrams confirm
        // each other.

        if (selfConsistent) {
            dcf77PrevEpoch = decodedEpoch;
            dcf77PrevAtMillis = result.decodedAtMillis;
        }

        if (!rangesOk) {
            DEBUG_PRINTLN("[DCF77] Telegram parity ok but values implausible - discarded");
        }

        dcf77LastDecoded = result;
        return true;
    }


    // Setzt die Minutenmarken-Statistik zurueck (dcf77MarkerMiss/-Hit).
    // Noetig bei Rasterverlust: dcf77Phase startet neu, alte Fehlstellen
    // zeigten dann auf nicht mehr gueltige Positionen.

    // Resets the minute-marker statistics (dcf77MarkerMiss/-Hit). Needed on
    // grid loss: dcf77Phase restarts, old miss counts would point at
    // positions that no longer apply.

    void resetDcf77MarkerStats() {
        for (uint8_t i = 0; i < 60; i++) {
            dcf77MarkerMiss[i] = 0;
            dcf77MarkerHit[i] = 0;
        }
        dcf77MarkerPos = -1;
        dcf77StructFails = 0;
        dcf77LastSecond = -1;
        dcf77Synced = false;
        dcf77BitIndex = 0;
        for (uint8_t i = 0; i < DCF77_GRID_SLOTS; i++) dcf77Bits[i] = -1;
    }


    // Sucht die Rasterposition der Minutenmarke (59. Sekunde, ohne Impuls):
    // nie getroffene Position mit den meisten Fehlstellen, mit klarem
    // Vorsprung vor dem Zweiten (DCF77_MARKER_MIN_LEAD).

    // Searches for the minute marker's grid position (59th second, no pulse):
    // the never-hit position with the most misses, with a clear lead over
    // the runner-up (DCF77_MARKER_MIN_LEAD).

    void evaluateDcf77Marker() {
        int8_t best = -1;
        uint8_t bestMiss = 0;
        uint8_t secondMiss = 0;

        for (uint8_t p = 0; p < 60; p++) {
            if (dcf77MarkerHit[p] != 0) continue; // dort kam schon ein Impuls an - keine Marke
                                                   // a pulse already arrived there - not the marker
            uint8_t miss = dcf77MarkerMiss[p];
            if (miss > bestMiss) {
                secondMiss = bestMiss;
                bestMiss = miss;
                best = (int8_t)p;
            }
            else if (miss > secondMiss) {
                secondMiss = miss;
            }
        }

        if (best >= 0 && bestMiss >= DCF77_MARKER_MIN_MISSES &&
            bestMiss >= (uint8_t)(secondMiss + DCF77_MARKER_MIN_LEAD)) {
            dcf77MarkerPos = best;
            dcf77StructFails = 0;
            DEBUG_PRINTLN("[DCF77] Minute marker found at grid position " + String(best) +
                          " (missing " + String(bestMiss) + "x, next candidate " + String(secondMiss) + "x)");
        }
    }


    // Wertet den ISR-Ringpuffer aus, baut den Bit-Fortschritt auf: Dauer ->
    // Bitwert, Abstand -> Sekundenraster (dcf77Phase), Marke statistisch
    // bestimmt (evaluateDcf77Marker()). Nur aus loop(), nicht aus der ISR.

    // Evaluates the ISR ring buffer, builds up the bit progress: duration ->
    // bit value, gap -> second grid (dcf77Phase), marker determined
    // statistically (evaluateDcf77Marker()). loop() only, not the ISR.

    void processDcf77Bits() {
        static unsigned long prevEdgeMillis = 0;
        static bool havePrevEdge = false;
        static unsigned long lastPulseStartMillis = 0;
        static bool havePulseStart = false;
        static unsigned long lastGridLossLogMillis = 0;

        while (dcf77EdgeTail != dcf77EdgeHead) {
            unsigned long edgeMillis = dcf77EdgeMillis[dcf77EdgeTail];
            dcf77EdgeTail = (dcf77EdgeTail + 1) % DCF77_EDGE_BUFFER_SIZE;

            if (!havePrevEdge) {
                prevEdgeMillis = edgeMillis;
                havePrevEdge = true;
                continue;
            }

            unsigned long duration = edgeMillis - prevEdgeMillis;

            // Rauschen/Prellen: kurze Flanke ignorieren, OHNE prevEdgeMillis
            // zu verschieben - wird beim naechsten echten Wechsel einfach
            // uebersprungen statt eine zu kurze Dauer zu erzeugen.

            // Noise/bounce: ignore a short edge WITHOUT moving prevEdgeMillis -
            // it is simply skipped at the next genuine change instead of
            // producing a too-short duration.

            if (duration < DCF77_BIT_NOISE_IGNORE_MS) {
                continue;
            }

            unsigned long pulseStart = prevEdgeMillis;
            prevEdgeMillis = edgeMillis;

            // Langes Intervall = Rest der Sekunde; die Anzahl vergangener
            // Sekunden folgt unten aus dem Abstand der IMPULSANFAENGE, bleibt
            // also auch bei fehlenden Impulsen richtig.

            // Long interval = rest of the second; the number of elapsed
            // seconds follows below from the distance between PULSE STARTS,
            // staying correct even when pulses are missing.

            if (duration > DCF77_PULSE_MAX_MS) {
                continue;
            }

            // Ab hier beendet diese Flanke einen Impuls
            // From here on this edge ends a pulse

            int8_t bitValue = (duration >= DCF77_PULSE_ONE_MIN_MS) ? 1 : 0;

            uint16_t steps = 1;
            bool gridOk = false;
            unsigned long gap = 0;

            if (havePulseStart) {
                gap = pulseStart - lastPulseStartMillis;
                unsigned long secondsElapsed = (gap + DCF77_SECOND_MS / 2) / DCF77_SECOND_MS;
                unsigned long expected = secondsElapsed * DCF77_SECOND_MS;
                unsigned long deviation = (gap > expected) ? (gap - expected) : (expected - gap);

                if (secondsElapsed < 1) {

                    // Zwei Impulse in derselben Sekunde - Stoerung. Verwerfen,
                    // Raster UND Bezugszeitpunkt aber behalten, damit der
                    // naechste echte Impuls den korrekten Abstand hat.

                    // Two pulses in the same second - interference. Discard,
                    // but keep both the grid AND the reference timestamp, so
                    // the next genuine pulse has the correct distance again.

                    continue;
                }

                gridOk = (secondsElapsed <= (unsigned long)DCF77_MAX_PHASE_GAP_SECONDS) &&
                         (deviation <= DCF77_STEP_TOLERANCE_MS);
                if (gridOk) steps = (uint16_t)secondsElapsed;
            }

            lastPulseStartMillis = pulseStart;
            havePulseStart = true;

            // Diagnose fuer die /dcf77-Seite: Impulsdauer und Abstand zum
            // vorherigen Impulsanfang mitschreiben, damit von aussen sichtbar
            // ist, was der Empfaenger tatsaechlich liefert.

            // Diagnostics for the /dcf77 page: record pulse width and distance
            // to the previous pulse start, so what the receiver actually
            // delivers is visible from the outside.

            dcf77DiagWidth[dcf77DiagIdx] = (uint16_t)duration;
            dcf77DiagGap[dcf77DiagIdx] = (gap > 65535UL) ? 65535 : (uint16_t)gap;
            dcf77DiagIdx = (dcf77DiagIdx + 1) % DCF77_DIAG_SLOTS;
            if (dcf77DiagCount < DCF77_DIAG_SLOTS) dcf77DiagCount++;
            dcf77PulsesSeen++;

            if (!gridOk) {

                // Raster verloren (erster Impuls, lange Luecke oder unpassender
                // Abstand) - Fehlstellen-Statistik zeigt sonst auf ungueltige
                // Positionen, deshalb zuruecksetzen und neue Phase beginnen.

                // Grid lost (first pulse, long gap, or a distance fitting no
                // grid) - the miss statistics would otherwise point at invalid
                // positions, so reset them and start a new phase.

                dcf77PhaseBreaks++;
                resetDcf77MarkerStats();
                dcf77Phase = 0;
                dcf77MarkerHit[0] = 1;

                // Ratenbegrenzt loggen: jede Logzeile schreibt auf LittleFS und
                // kostet dabei DCF77-Flanken (siehe isr()) - ungebremst wuerde
                // das den Empfang gerade bei schlechtem Empfang verschlechtern.

                // Rate-limited: every log line writes to LittleFS and costs
                // DCF77 edges while doing so (see isr()) - unthrottled this
                // would worsen reception exactly when it's already poor.

                if (millis() - lastGridLossLogMillis > WAIT_1m) {
                    lastGridLossLogMillis = millis();
                    DEBUG_PRINTLN("[DCF77] Second grid lost, resynchronizing (total: " +
                                  String(dcf77PhaseBreaks) + ")");
                }
                continue;
            }

            // LED-Blitz anfordern, sobald dcf77Confirmed gilt - deutlich vor
            // dem Markenfund. "&& !wpsPending": waehrend WPS blinkt die LED
            // schon eigenstaendig (loop()), sonst wuerden sich beide Signale ueberlagern.

            // Request an LED flash once dcf77Confirmed is true - well before
            // the marker is found. "&& !wpsPending": during WPS the LED
            // already blinks on its own (loop()), otherwise both signals would overlap.

            if (dcf77Confirmed && !wpsPending) {
                dcfLedTogglePending = true;
            }

            // Phase weiterschalten, uebersprungene Positionen als Fehlstelle
            // zaehlen - NUR bei kurzen Luecken, sonst traefe eine lange Pause
            // alle 60 Positionen gleich und die Marke ginge im Rauschen unter.

            // Advance the phase, count skipped positions as missing - ONLY
            // for short gaps, otherwise a long pause would hit all 60
            // positions equally and drown the marker in noise.

            uint8_t phaseBefore = dcf77Phase;

            if (steps <= DCF77_MISS_COUNT_MAX_GAP) {
                for (uint16_t k = 1; k < steps; k++) {
                    uint8_t p = (uint8_t)((dcf77Phase + k) % DCF77_GRID_SLOTS);
                    if (dcf77MarkerMiss[p] < DCF77_MARKER_COUNT_MAX) dcf77MarkerMiss[p]++;
                    dcf77PulsesMissed++;
                }
            }
            else {
                dcf77PulsesMissed += (steps - 1);
            }

            dcf77Phase = (uint8_t)((dcf77Phase + steps) % DCF77_GRID_SLOTS);
            if (dcf77MarkerHit[dcf77Phase] < DCF77_MARKER_COUNT_MAX) dcf77MarkerHit[dcf77Phase]++;

            // Saettigung: alle Zaehler halbieren, damit alte Ereignisse
            // ausduennen und ein behobener Dauerstoerer nicht ewig nachwirkt.

            // Saturation: halve all counters so old events thin out and a
            // resolved persistent interferer does not keep echoing forever.

            if (dcf77MarkerHit[dcf77Phase] >= DCF77_MARKER_COUNT_MAX) {
                for (uint8_t i = 0; i < DCF77_GRID_SLOTS; i++) {
                    dcf77MarkerMiss[i] /= 2;
                    dcf77MarkerHit[i] /= 2;
                }
            }

            // Bit IMMER ablegen (Index ist Rasterposition, braucht die Marke
            // nicht) - solange die Marke unbekannt ist, bei jedem Rasterumlauf
            // leeren, sonst mischen sich Bits verschiedener Minuten.

            // ALWAYS store the bit (indexed by grid position, not the marker)
            // - while the marker is unknown, clear on every grid wrap,
            // otherwise bits from different minutes would mix.

            if (dcf77MarkerPos < 0 && dcf77Phase <= phaseBefore) {
                for (uint8_t i = 0; i < DCF77_GRID_SLOTS; i++) dcf77Bits[i] = -1;
            }

            // Inhalt VOR dem Ueberschreiben sichern: bei Minutenwechsel ist
            // das genau das Bit der ABGELAUFENEN Minute an dieser Position
            // (60 Rasterplaetze = 60 Sekunden = ein Zyklus), noetig unten.

            // Save the content BEFORE overwriting: on a minute change this is
            // exactly the JUST-ELAPSED minute's bit at this position (60 grid
            // slots = 60 seconds = one cycle), needed below.

            int8_t previousGridValue = dcf77Bits[dcf77Phase];

            dcf77Bits[dcf77Phase] = bitValue;

            if (dcf77MarkerPos < 0) {
                evaluateDcf77Marker();

                if (dcf77MarkerPos >= 0) {

                    // Marke soeben gefunden: frisches Raster, da bisherige
                    // Bits aus der Suchphase aelter als die laufende Minute
                    // sein koennen. dcf77LastSecond=-1 fuer sauberen Neustart.

                    // Marker just found: fresh grid, since bits collected
                    // during the search phase may be older than the current
                    // minute. dcf77LastSecond=-1 for a clean restart.

                    for (uint8_t i = 0; i < DCF77_GRID_SLOTS; i++) dcf77Bits[i] = -1;
                    dcf77Bits[dcf77Phase] = bitValue;
                    dcf77LastSecond = -1;
                }
                else {

                    // Sekundenzuordnung fehlt noch, gesammelt wird trotzdem;
                    // der Fortschrittsbalken laeuft ueber die Rasterposition.

                    // Second mapping still missing, collecting continues
                    // anyway; the progress display runs on the grid position.

                    dcf77Synced = false;
                    dcf77BitIndex = (uint8_t)(dcf77Phase + 1);
                    continue;
                }
            }

            dcf77Synced = true;

            // Rasterposition -> Sekunde der Minute. Die Markenposition selbst
            // ist die 59. Sekunde, die Position danach die Sekunde 0.

            // Grid position -> second of the minute. The marker position
            // itself is the 59th second, the one after it is second 0.

            uint8_t sec = (uint8_t)((dcf77Phase + DCF77_GRID_SLOTS - (uint8_t)dcf77MarkerPos + 59) % DCF77_GRID_SLOTS);

            // Minutenwechsel: Sekundennummer kleiner als zuvor -> abgelaufene
            // Minute vollstaendig. Zeitstempel = Minutenanfang, ueber die
            // aktuelle Sekundennummer exakt zurueckrechenbar.

            // Minute change: second number lower than before -> the elapsed
            // minute is complete. Timestamp = start of the minute, computed
            // back exactly from the current second number.

            if (dcf77LastSecond >= 0 && sec < (uint8_t)dcf77LastSecond) {
                unsigned long minuteStart = pulseStart - (unsigned long)sec * DCF77_SECOND_MS;

                // Bit gehoert schon zur NEUEN Minute - vor dem Dekodieren durch
                // previousGridValue ersetzen, NICHT auf -1 setzen (sonst zeigte
                // "Reconstructed bits" faelschlich nie 0). Wird danach zurueckgeschrieben.

                // Bit already belongs to the NEW minute - replace with
                // previousGridValue before decoding, do NOT set -1 (otherwise
                // "Reconstructed bits" would falsely never read 0). Written back after.

                dcf77Bits[dcf77Phase] = previousGridValue;

                if (decodeDcf77Telegram(minuteStart)) {
                    dcf77StructFails = 0;
                }
                else {

                    // Festbits widersprechen: erst nach mehreren Telegrammen
                    // in Folge gilt die Marke als falsch (ein einzelner
                    // Stoerimpuls kann genau dort gelandet sein).

                    // Fixed bits contradict: only after several telegrams in a
                    // row does the marker count as wrong (a single spurious
                    // pulse may have landed exactly there).

                    dcf77StructFails++;
                    if (dcf77StructFails >= DCF77_STRUCT_FAIL_LIMIT) {
                        DEBUG_PRINTLN("[DCF77] Fixed bits keep contradicting - discarding minute marker, searching again");
                        resetDcf77MarkerStats();
                        continue;
                    }
                }

                for (uint8_t i = 0; i < DCF77_GRID_SLOTS; i++) dcf77Bits[i] = -1;
                dcf77Bits[dcf77Phase] = bitValue;
            }

            dcf77LastSecond = (int8_t)sec;
            dcf77BitIndex = (uint8_t)(sec + 1);
        }
    }

    // Bindet den eigenen NTP-Server (neu) an Port 123 - nach JEDEM Verbindungsaufbau, da connectWiFi() WLAN
    // zwischendurch abschaltet und der alte Socket sein Interface verliert. udp.stop() gibt einen noch
    // gebundenen Port frei.

    // (Re)binds the own NTP server to port 123 - after EVERY connection setup, since connectWiFi() shuts WiFi
    // down in between and the old socket loses its interface. udp.stop() releases a still-bound port.

    bool startNtpServer() {
        udp.stop();
        ntpServerRunning = (udp.begin(NTP_PORT) == 1);

        if (ntpServerRunning) {
            DEBUG_PRINTLN("[NTPD] NTP server listening on port " + String(NTP_PORT));
        }
        else {
            DEBUG_PRINTLN("[NTPD] Could not bind port " + String(NTP_PORT) + " - clock is not available as a time source");
        }
        return ntpServerRunning;
    }


    // Testet einen NTP-Server per direkter UDP-Anfrage (RFC 5905, minimales
    // Client-Paket), ohne die Systemzeit zu veraendern - nutzt eine eigene
    // lokale WiFiUDP-Instanz. Gibt bei Erfolg die UTC-Zeit als String zurueck.

    // Tests an NTP server via a direct UDP request (RFC 5905, minimal client
    // packet), without changing the system time - uses its own local WiFiUDP
    // instance. Returns the UTC time as a string on success, empty otherwise.

    String testNtpServer(const String& server) {
        if (WiFi.getMode() != WIFI_STA || !WiFi.isConnected()) return "";
        time_t epochTime = ntpQueryEpoch(server);
        if (epochTime == 0) return "";

        struct tm resultTime;
        gmtime_r(&epochTime, &resultTime);
        char buf[32];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &resultTime);
        return String(buf) + " UTC";
    }


    // Eine NTP-Abfrage per UDP (siehe testNtpServer(), fetchTimeFromOpenWifi()): Unix-Sekunden UTC, 0 ohne Antwort
    // One NTP query via UDP (see testNtpServer(), fetchTimeFromOpenWifi()): unix seconds UTC, 0 without a reply

    time_t ntpQueryEpoch(const String& server) {
        WiFiUDP testUdp;
        if (!testUdp.begin(0)) return 0; // beliebiger freier lokaler Port
                                         // any free local port

        IPAddress serverIp;
        if (!WiFi.hostByName(server.c_str(), serverIp)) {
            testUdp.stop();
            return 0;
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
            return 0;
        }

        testUdp.read(packet, 48);
        testUdp.stop();

        // Transmit-Timestamp: Sekunden seit 1900 in Byte 40-43 (big-endian)
        // Transmit timestamp: seconds since 1900 in bytes 40-43 (big-endian)

        uint32_t secsSince1900 = ((uint32_t)packet[40] << 24) | ((uint32_t)packet[41] << 16) |
                                 ((uint32_t)packet[42] << 8) | (uint32_t)packet[43];
        const uint32_t SEVENTY_YEARS = 2208988800UL; // Differenz 1900 -> 1970
                                                     // difference 1900 -> 1970
        if (secsSince1900 < SEVENTY_YEARS) return 0;
        return (time_t)(secsSince1900 - SEVENTY_YEARS);
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

        // Synchronisation mit GENAU einem Server (DNS + bis zu NTP_SYNC_ATTEMPTS Versuche) - als Lambda,
        // damit dieselbe Logik fuer die konfigurierten Server und den eingebauten Fallback dient.

        // Sync with EXACTLY one server (DNS + up to NTP_SYNC_ATTEMPTS attempts) - as a lambda, so the same
        // logic serves the configured servers and the built-in fallback.

        auto trySyncWithServer = [&](const char* ntpServerCStr) -> bool {
            String ntpServer = ntpServerCStr;

            // Diagnose: DNS-Aufloesung separat pruefen/loggen, damit im
            // Fehlerfall sichtbar ist, ob der Server ueberhaupt aufloesbar war.

            // Diagnostic: check/log DNS resolution separately, so on failure
            // it's visible whether the server was resolvable at all.

            IPAddress ntpServerIp;
            if (WiFi.hostByName(ntpServerCStr, ntpServerIp)) {
                DEBUG_PRINTLN("[NTP] Trying server: " + ntpServer + " (" + ntpServerIp.toString() + ")");
            }
            else {

                // Ohne aufloesbaren Namen kann der SNTP-Client den Server auch
                // nicht erreichen - direkt zum naechsten springen statt WAIT_3s
                // auf eine unmoegliche Antwort zu warten.

                // Without a resolvable name the SNTP client can't reach the
                // server either - skip straight to the next one instead of
                // waiting WAIT_3s for a response that cannot arrive.

                DEBUG_PRINTLN("[NTP] DNS lookup failed for server: " + ntpServer);
                return false;
            }

            // getLocalTime() prueft nur "Jahr > 2016" - da die RTC das schon
            // erfuellt, Zeit vorher sichern und auf 1970 (ungueltig) setzen,
            // sonst meldet der erste Server faelschlich sofort "Erfolg".

            // getLocalTime() only checks "year > 2016" - since the RTC already
            // satisfies that, save the time first and set it to 1970
            // (invalid), otherwise the first server falsely reports success.

            struct timeval savedTime;
            gettimeofday(&savedTime, nullptr);

            // Schwelle identisch zu getLocalTime() (Jahr > 2016), sonst wuerde
            // eine gueltige, aber aeltere Systemzeit nicht wiederhergestellt.

            // Threshold matches getLocalTime() (year > 2016), otherwise a
            // valid but older system time would not get restored.

            bool hadValidTime = (savedTime.tv_sec > 1483228800L); // 2017-01-01
            unsigned long syncStart = millis();

            struct timeval invalidTime = { 0, 0 };
            settimeofday(&invalidTime, nullptr);

            // WAIT_3s statt 500ms (oft zu knapp fuer DNS+Antwort). Mehrere
            // NTP_SYNC_ATTEMPTS pro Server, da ein einzelnes verlorenes Paket
            // normal ist; configTzTime() pro Versuch neu, fuer eine frische Anfrage.

            // WAIT_3s instead of 500ms (often too short for DNS+response).
            // Multiple NTP_SYNC_ATTEMPTS per server, since a single lost
            // packet is normal; configTzTime() reissued each time for a fresh request.

            bool ntpResponded = false;

            // Lokale struct tm statt der globalen timeinfo: setupNTP() laeuft in einer eigenen Task,
            // updateClock() nutzt timeinfo gleichzeitig auf dem Hauptthread. Die Systemzeit selbst ist die
            // Quelle, aus der updateClock() liest.

            // Local struct tm instead of the global timeinfo: setupNTP() runs in its own task, updateClock()
            // uses timeinfo on the main thread at the same time. The system time itself is the source
            // updateClock() reads from.

            struct tm ntpLocalTime;
            for (uint8_t attempt = 0; attempt < NTP_SYNC_ATTEMPTS; attempt++) {
                if (attempt > 0) {
                    DEBUG_PRINTLN("[NTP] No response from " + ntpServer + ", retrying (attempt " + String(attempt + 1) + "/" + String(NTP_SYNC_ATTEMPTS) + ")..");
                }
                configTzTime(timezoneSnapshot, ntpServerCStr);
                if (getLocalTime(&ntpLocalTime, WAIT_3s)) {
                    ntpResponded = true;
                    break;
                }
            }

            if (ntpResponded) {

                DEBUG_PRINTLN("[NTP] Time synchronized successfully with " + ntpServer);
                lastNtpSuccessMillis = millis();
                logTimeSyncDifference("[NTP]", savedTime, syncStart);

                // Das RTC-Update (I2C) passiert bewusst NICHT hier, sondern in pollNtpSyncTask() auf dem
                // Hauptthread - ein zweiter Busszugriff aus dieser Task waere ein Race.

                // The RTC update (I2C) deliberately does NOT happen here but in pollNtpSyncTask() on the main
                // thread - a second bus access from this task would race.

                return true;
            }

            // Keine Antwort: die ungueltig gesetzte Zeit auf den gesicherten Stand plus verstrichene Zeit
            // zuruecksetzen - MIT Millisekunden, sonst ging je Server ein Sekundenbruchteil verloren und der
            // Sekundenzeiger zuckte zurueck.

            // No response: restore the invalidated time to the saved value plus elapsed time - WITH
            // milliseconds, otherwise a fraction of a second was lost per server and the second hand twitched
            // backward.

            if (hadValidTime) {
                unsigned long elapsedMs = millis() - syncStart;
                struct timeval restoreTime;
                restoreTime.tv_sec = savedTime.tv_sec + (time_t)(elapsedMs / 1000);
                restoreTime.tv_usec = savedTime.tv_usec + (suseconds_t)(elapsedMs % 1000) * 1000;
                if (restoreTime.tv_usec >= 1000000) {
                    restoreTime.tv_usec -= 1000000;
                    restoreTime.tv_sec += 1;
                }
                settimeofday(&restoreTime, nullptr);
            }

            DEBUG_PRINTLN("[NTP] Failed to synchronize with server: " + ntpServer);
            return false;
        };

        bool anyServerConfigured = false;
        for (int i = 0; i < MAX_WLAN; i++) {
            String ntpServer = ntpServersSnapshot[i];
            if (ntpServer.length() == 0) continue;
            anyServerConfigured = true;
            if (trySyncWithServer(ntpServersSnapshot[i])) return true;
        }

        // Kein konfigurierter Server erreichbar - zusaetzlich die eingebauten Standardserver versuchen,
        // ausser sie standen schon (erfolglos) in der Liste.

        // No configured server reachable - additionally try the built-in default servers, unless they were
        // already (unsuccessfully) in the list.

        if (anyServerConfigured) {
            bool server1AlreadyTried = false;
            bool server2AlreadyTried = false;
            for (int i = 0; i < MAX_WLAN; i++) {
                if (String(ntpServersSnapshot[i]) == NTP_SERVER_1) server1AlreadyTried = true;
                if (String(ntpServersSnapshot[i]) == NTP_SERVER_2) server2AlreadyTried = true;
            }
            if (!server1AlreadyTried) {
                DEBUG_PRINTLN("[NTP] All configured servers unreachable, falling back to default: " + String(NTP_SERVER_1));
                if (trySyncWithServer(NTP_SERVER_1)) return true;
            }
            if (!server2AlreadyTried) {
                DEBUG_PRINTLN("[NTP] All configured servers unreachable, falling back to default: " + String(NTP_SERVER_2));
                if (trySyncWithServer(NTP_SERVER_2)) return true;
            }
        }

        handleNTPFailure();
        return false;
    }


    // Eigene FreeRTOS-Task fuer das blockierende setupNTP() (wie rocrailConnectTaskFunc()), damit die
    // DNS-/UDP- Wartezeiten weder loop() noch den Webserver blockieren.

    // Own FreeRTOS task for the blocking setupNTP() (like rocrailConnectTaskFunc()), so the DNS/UDP wait
    // times block neither loop() nor the web server.

    void ntpSyncTaskFunc(void* param) {
        ntpSyncTaskResult = setupNTP();
        ntpSyncTaskDone = true;
        vTaskDelete(NULL);
    }


    // Beendet eine laufende NTP-Sync-Task HART, fuer Neustart/Werksreset - sie kann bis ~90 s laufen und
    // dabei Preferences/LittleFS anfassen, die gleich geschlossen bzw. formatiert werden. Der ESP startet
    // danach ohnehin neu.

    // Stops a running NTP sync task FORCIBLY, for restart/factory reset - it can run up to ~90 s and touch
    // Preferences/LittleFS, which are about to be closed or formatted. The ESP restarts right afterward
    // anyway.

    void stopNtpSyncTaskIfRunning() {

        // Auch ntpSyncTaskDone pruefen: hat sich die Task schon selbst geloescht, ist das Handle ungueltig -
        // ein zweites vTaskDelete() waere ein Double-Delete.

        // Also check ntpSyncTaskDone: if the task already deleted itself, the handle is stale - a second
        // vTaskDelete() would be a double delete.

        if (ntpSyncTaskRunning && !ntpSyncTaskDone && ntpSyncTaskHandle != NULL) {
            vTaskDelete(ntpSyncTaskHandle);
        }
        ntpSyncTaskRunning = false;
        ntpSyncTaskDone = false;
        ntpSyncTaskHandle = NULL;
    }


    // Startet einen asynchronen NTP-Sync-Versuch (Boot, periodisch, oder per
    // "Jetzt synchronisieren"-Button). No-op, wenn bereits eine Sync-Task
    // laeuft - Ergebnis wird von pollNtpSyncTask() ausgewertet.

    // Starts an asynchronous NTP sync attempt (boot, periodic, or via the
    // "Sync now" button). No-op if a sync task is already running - result
    // is evaluated by pollNtpSyncTask().

    void startNtpSyncTask(String label) {
        if (ntpSyncTaskRunning) return;

        // Sicherheitsnetz zur Laufzeit: greift auch, wenn ntpServers[]/timezone erst ueber die Weboberflaeche
        // leer oder ungueltig wurden, ohne Neustart dazwischen.

        // Runtime safety net: also applies when ntpServers[]/timezone only became empty or invalid via the
        // web UI, without a restart in between.

        applyNtpServerDefaultsIfNoneConfigured();
        applyTimezoneDefaultIfInvalid();

        // Kopien JETZT auf dem Hauptthread anlegen (siehe ntpServersSnapshot/timezoneSnapshot in globals.h) -
        // setupNTP() liest nur noch diese, egal was sich an den Originalen aendert.

        // Take the copies NOW on the main thread (see ntpServersSnapshot/timezoneSnapshot in globals.h) -
        // setupNTP() only reads those, whatever happens to the originals.

        memcpy(ntpServersSnapshot, ntpServers, sizeof(ntpServersSnapshot));
        timezone.toCharArray(timezoneSnapshot, sizeof(timezoneSnapshot));

        ntpSyncCheckBeforeMillis = millis();
        ntpSyncCheckLabel = label;
        ntpSyncTaskDone = false;
        ntpSyncTaskRunning = true;
        xTaskCreate(ntpSyncTaskFunc, "ntpSync", 4096, NULL, 1, &ntpSyncTaskHandle);
    }


    // Wertet eine beendete NTP-Sync-Task aus - in jedem loop()-Durchlauf aufgerufen, no-op solange keine Task
    // fertig ist. Bei Fehlschlag greift der DCF77-Fallback.

    // Evaluates a finished NTP sync task - called on every loop() iteration, no-op as long as no task has
    // finished. On failure, the DCF77 fallback kicks in.

    void pollNtpSyncTask() {
        if (!ntpSyncTaskDone) return;

        ntpSyncTaskRunning = false;
        ntpSyncTaskDone = false;
        ntpSyncTaskHandle = NULL;

        // ntpSyncTaskResult bewusst NICHT auswerten (ohne WLAN irrefuehrend true) - stattdessen pruefen, ob
        // seit dem Start dieser Task ein echter Erfolg eingetragen wurde.

        // Deliberately do NOT evaluate ntpSyncTaskResult (misleadingly true without WiFi) - instead check
        // whether a real success was recorded since this task started.

        bool ntpJustSucceeded = (lastNtpSuccessMillis != 0 && lastNtpSuccessMillis >= ntpSyncCheckBeforeMillis);
        if (ntpJustSucceeded) {
            DEBUG_PRINTLN("[TIME SYNC] " + ntpSyncCheckLabel + ": NTP succeeded");

            // RTC-Update (I2C) bewusst HIER auf dem Hauptthread: der Bus wird auch von checkRtcHealth()
            // genutzt, und die Task ist hier schon beendet - kein paralleler Zugriff moeglich.

            // RTC update (I2C) deliberately HERE on the main thread: the bus is also used by
            // checkRtcHealth(), and the task has already finished here - no parallel access possible.

            if (rtcOk == RTC_AVAILABLE || rtcOk == RTC_AVAILABLE_BUT_INVALID) {
                struct tm nowTm;
                if (getLocalTime(&nowTm, 100)) {

                    // Nur bei nennenswerter Abweichung tatsaechlich schreiben
                    // (siehe RTC_UPDATE_MIN_DRIFT_SEC) - unnoetige I2C-
                    // Schreibzugriffe vermeiden.

                    // Only actually write on a notable deviation (see
                    // RTC_UPDATE_MIN_DRIFT_SEC) - avoid unnecessary I2C writes.

                    long rtcDiffSec = rtcDriftSec(nowTm);
                    if (labs(rtcDiffSec) >= RTC_UPDATE_MIN_DRIFT_SEC) {
                        rtc.adjust(DateTime(nowTm.tm_year + 1900, nowTm.tm_mon + 1, nowTm.tm_mday,
                            nowTm.tm_hour, nowTm.tm_min, nowTm.tm_sec));
                        DEBUG_PRINTLN("[RTC] RTC updated with NTP time (diff " + String(rtcDiffSec) + "s)");
                    }

                    // rtcOk zurueck auf RTC_AVAILABLE: eine zuvor "ungueltige"
                    // RTC ist jetzt physisch korrekt - sonst wuerde
                    // applyDcf77DecodedTime() (prueft strikt) sie nie updaten.

                    // rtcOk back to RTC_AVAILABLE: a previously "invalid" RTC
                    // is now physically correct - otherwise
                    // applyDcf77DecodedTime() (strict check) would never update it.

                    rtcOk = RTC_AVAILABLE;
                }
            }
        }
        else {
            DEBUG_PRINTLN("[TIME SYNC] " + ntpSyncCheckLabel + ": NTP unavailable, trying DCF77 fallback..");
            applyDcf77DecodedTime("[DCF77] " + ntpSyncCheckLabel + " (NTP unavailable)");
        }
    }


    // NTP-Sync-Fehler: die letzte bekannte Zeit weiter nutzen. Ohne jede Zeit bewusst KEINE Ersatzzeit setzen
    // - die Anzeige laeuft ab der Startzeit weiter, bis USB, NTP, DCF77 oder RTC eine echte Zeit liefern.

    // NTP sync failure: keep using the last known time. Without any time deliberately set NO substitute time
    // - the display keeps running from the start time until USB, NTP, DCF77 or RTC deliver a real time.

    void handleNTPFailure() {
        DEBUG_PRINTLN("[NTP] Handling NTP synchronization failure..");

        // Lokale struct tm statt der globalen timeinfo: laeuft in der NTP-Sync-Task, waehrend updateClock()
        // timeinfo gleichzeitig auf dem Hauptthread nutzt.

        // Local struct tm instead of the global timeinfo: runs in the NTP sync task while updateClock() uses
        // timeinfo on the main thread at the same time.

        struct tm localTime;

        // Versuche, die letzte bekannte Zeit zu verwenden
        // Try to use the last known time

        if (getLocalTime(&localTime, 100)) {
            char timeStr[32];
            strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &localTime);
            DEBUG_PRINTLN("[NTP] Using last known time: " + String(timeStr));
        }
        else {
            DEBUG_PRINTLN("[NTP] No valid time available - display keeps running from the start time");
        }

        // Wiederholung ergibt sich aus dem periodischen NTP-Aufruf in loop();
        // fallen alle Server aus, springt derselbe Aufrufer per
        // applyDcf77DecodedTime() auf DCF77 als Zeitquelle um.

        // Retry falls out of the periodic NTP call in loop(); if all servers
        // fail, that same caller falls back to DCF77 via
        // applyDcf77DecodedTime().

    }


    // Setzt die Systemzeit manuell anhand einer `tm`-Struktur.
    // Sets the system time manually from a `tm` struct.

    void setTimeStruct(const struct tm& timeinfo, String source) {

        struct timeval oldTime;
        gettimeofday(&oldTime, nullptr);
        unsigned long oldTimeMillis = millis();

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
        DEBUG_PRINTLN(source + " " + String(buffer)); // Gibt die lokale Zeit und die Zeitzone aus
                                                      // prints the local time and timezone
        logTimeSyncDifference(source, oldTime, oldTimeMillis);
    }


    // Setzt die Uhrzeit aus "<Unix-Sekunden UTC>[.<Bruchteil>]" (USB "UHR4 TIME", Weboberflaeche /api/setTime):
    // Systemzeit und ggf. RTC, nur 2024-2099. NTP/DCF77 korrigieren spaeter wie gewohnt. localText: gesetzte
    // Ortszeit "JJJJ-MM-TT hh:mm:ss". false bei ungueltigem Wert.

    // Sets the time from "<unix seconds UTC>[.<fraction>]" (USB "UHR4 TIME", web interface /api/setTime): system
    // time and the RTC if present, 2024-2099 only. NTP/DCF77 correct it later as usual. localText: the set local
    // time "YYYY-MM-DD hh:mm:ss". false on an invalid value.

    bool setClockTime(const String& arg, const char* source, String& localText) {
        const time_t TIME_MIN = 1704067200; // 2024-01-01 00:00:00 UTC
        const time_t TIME_MAX = 4102444799; // 2099-12-31 23:59:59 UTC
        const char* text = arg.c_str();
        char* end = nullptr;
        long long sec = strtoll(text, &end, 10);
        long usec = 0;
        if (end != text && *end == '.') {

            // Bruchteil auf Mikrosekunden (max. 6 Stellen) / fraction to microseconds (max. 6 digits)

            long scale = 100000;
            for (end++; isdigit((uint8_t)*end); end++) {
                usec += (*end - '0') * scale;
                scale /= 10;
            }
        }
        if (end == text || *end != '\0' || sec < TIME_MIN || sec > TIME_MAX) return false;

        struct timeval oldTime;
        gettimeofday(&oldTime, nullptr);
        unsigned long oldTimeMillis = millis();
        struct timeval tv = { (time_t)sec, (suseconds_t)usec };
        settimeofday(&tv, nullptr);
        timeSetByUser = true;

        struct tm local;
        localtime_r(&tv.tv_sec, &local);
        timeinfo = local;
        logTimeSyncDifference(source, oldTime, oldTimeMillis);

        // RTC mitstellen - auch eine als "ungueltig" markierte (wie NTP/DCF77)
        // Set the RTC too - also one flagged "invalid" (like NTP/DCF77)

        if (rtcOk == RTC_AVAILABLE || rtcOk == RTC_AVAILABLE_BUT_INVALID) {
            rtc.adjust(DateTime(local.tm_year + 1900, local.tm_mon + 1, local.tm_mday,
                local.tm_hour, local.tm_min, local.tm_sec));
            rtcOk = RTC_AVAILABLE;
            lastRTCUpdate = millis();
            DEBUG_PRINTLN(String("[RTC] RTC updated with time from ") + source);
        }
        serialTimeSet = true;

        char buffer[24];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &local);
        localText = buffer;
        DEBUG_PRINTLN(String(source) + " Time set to " + buffer);
        return true;
    }


    // "UHR4 TIME <Unix-Sekunden UTC>[.<Bruchteil>]" per USB (flashESP, setTime) - siehe setClockTime(). Antwort
    // "UHR4 OK TIME <JJJJ-MM-TT hh:mm:ss>" (Ortszeit).

    // "UHR4 TIME <unix seconds UTC>[.<fraction>]" via USB (flashESP, setTime) - see setClockTime(). Reply
    // "UHR4 OK TIME <YYYY-MM-DD hh:mm:ss>" (local).

    void handleSerialTime(const String& arg) {
        String localText;
        if (!setClockTime(arg, "[USB]", localText)) {
            serialReply("UHR4 ERROR TIME invalid '" + arg + "' (unix seconds UTC, 2024-2099)");
            return;
        }
        serialReply("UHR4 OK TIME " + localText);
    }


    // Scannt den I2C-Bus nach Geraeten und gibt die Anzahl zurueck.
    // Scans the I2C bus for devices and returns the number found.

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
            String addressHex = String(address, HEX);
            if (address < 16) addressHex = "0" + addressHex;

            // Eine Zeile je Adresse - mehrere DEBUG_PRINT ergaeben im Log getrennte Zeilen
            // One line per address - several DEBUG_PRINT calls would give separate lines in the log

            if (error == 0) {
                DEBUG_PRINTLN("[I2C]   I2C device found at address 0x" + addressHex);

                nDevices++;

                if (i2cAddr != "") i2cAddr += ", ";
                i2cAddr += "0x" + String(address, HEX);
            }
            else if (error == 4) {
                DEBUG_PRINTLN("[I2C] Unknown error at address 0x" + addressHex);
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


    // Prueft stuendlich, ob die RTC noch auf 0x68 antwortet, setzt rtcOk bei
    // Ausfall auf RTC_NOT_AVAILABLE. Erkennt NUR Ausfall, keine Wiederkehr -
    // eine wieder angeschlossene RTC braucht einen Neustart (Reinit in setup()).

    // Periodically checks whether the RTC still responds on 0x68, sets rtcOk
    // to RTC_NOT_AVAILABLE on failure. Detects failure ONLY, not recovery - a
    // reconnected RTC needs a restart (reinit in setup()).

    void checkRtcHealth() {
        if (rtcOk == RTC_NOT_AVAILABLE) return; // beim Boot nie gefunden - nichts zu ueberwachen
                                                // never found at boot - nothing to monitor
        static unsigned long lastRtcHealthCheck = 0;
        if (millis() - lastRtcHealthCheck < WAIT_1h) return;
        lastRtcHealthCheck = millis();

        Wire.beginTransmission(0x68);
        if (Wire.endTransmission() != 0) {
            DEBUG_PRINTLN("[RTC] Health check failed - RTC no longer responding on I2C bus");
            rtcOk = RTC_NOT_AVAILABLE;
        }
    }


    // Baut aus dem empfangenen Anfragepaket die NTP-Antwort (RFC 5905, Mode 4)
    // - IM SELBEN Puffer, den die Anfrage benutzt hat (siehe Aufrufstelle in
    // loop()). 'receivedAt' ist der Zeitpunkt, zu dem die Anfrage eintraf.

    // Builds the NTP reply (RFC 5905, mode 4) from the received request packet
    // - IN THE SAME buffer the request used (see the call site in loop()).
    // 'receivedAt' is the instant at which the request arrived.

    void createNtpResponse(byte* packet, const struct timeval& receivedAt) {

        // Originate Timestamp der ANFRAGE (Byte 40-47) muss unveraendert in
        // Byte 24-31 der Antwort zurueck, sonst verwerfen RFC-konforme Clients
        // sie als "bogus packet". Vor dem memset sichern (gleicher Puffer).

        // The REQUEST's Originate Timestamp (bytes 40-47) must be mirrored
        // back unchanged into bytes 24-31 of the reply, otherwise RFC-
        // compliant clients discard it as "bogus packet". Save before memset.

        byte originateTimestamp[8];
        memcpy(originateTimestamp, &packet[40], sizeof(originateTimestamp));

        memset(packet, 0, NTP_PACKET_SIZE);

        packet[0] = 0b00100100; // LI = 0, Version 4, Mode 4 (Server)
                                // LI = 0, version 4, mode 4 (server)
        packet[1] = 1;          // Stratum 1 (primaere Referenz)
                                // stratum 1 (primary reference)
        packet[2] = 6;          // Poll Interval
        packet[3] = 0xEC;       // Precision (2^-20 s)

        // Root Delay und Root Dispersion bleiben 0 (lokale Referenz).
        // Root delay and root dispersion stay 0 (local reference).

        // Reference Identifier: bei Stratum 1 die Quelle als vier ASCII-Zeichen.
        // Reference identifier: at stratum 1, the source as four ASCII chars.

        packet[12] = 'D'; packet[13] = 'C'; packet[14] = 'F'; packet[15] = ' ';

        // NTP zaehlt Sekunden seit 1900, Unix seit 1970.
        // NTP counts seconds since 1900, Unix since 1970.

        const uint32_t NTP_UNIX_OFFSET = 2208988800UL;

        // Sekundenbruchteile mitliefern (1/2^32s) - blieben sie 0, waere jede
        // Antwort auf die volle Sekunde gerundet, mit bis zu 1s Fehler.

        // Provide fractional seconds (1/2^32s) - if left 0, every reply would
        // round to the full second, with up to 1s of error.

        auto writeTimestamp = [&](uint8_t offset, const struct timeval& tv) {
            uint32_t seconds = htonl((uint32_t)(tv.tv_sec + NTP_UNIX_OFFSET));
            uint32_t fraction = htonl((uint32_t)(((uint64_t)tv.tv_usec << 32) / 1000000ULL));
            memcpy(&packet[offset], &seconds, 4);
            memcpy(&packet[offset + 4], &fraction, 4);
        };

        // Reference Timestamp: letzte Zeitstellung, hier vereinfacht
        // Empfangszeit minus einer Sekunde.

        // Reference timestamp: when last set, simplified here to the receive
        // instant minus one second.

        struct timeval referenceTime = receivedAt;
        referenceTime.tv_sec -= 1;
        writeTimestamp(16, referenceTime);

        // Originate Timestamp: gespiegelter Transmit-Timestamp der Anfrage.
        // Originate timestamp: mirrored transmit timestamp of the request.

        memcpy(&packet[24], originateTimestamp, sizeof(originateTimestamp));

        // Receive Timestamp: Eintreffen der Anfrage.
        // Receive timestamp: when the request arrived.

        writeTimestamp(32, receivedAt);

        // Transmit Timestamp: JETZT, nicht der Empfangszeitpunkt - der Client
        // rechnet aus Receive/Transmit die Serverzeit aus der Laufzeit heraus.

        // Transmit timestamp: NOW, not the receive instant - the client uses
        // receive/transmit to remove server processing time from round-trip delay.

        struct timeval transmitTime;
        gettimeofday(&transmitTime, nullptr);
        writeTimestamp(40, transmitTime);
    }

