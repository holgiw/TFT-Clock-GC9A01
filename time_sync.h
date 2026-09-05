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

        // Eigener, von der DCF77-Bibliothek unabhaengiger Flanken-Mitschnitt
        // fuer die Bit-Fortschrittsanzeige auf /dcf77 (siehe dcf77Edge*-
        // Variablen in globals.h und processDcf77Bits() weiter unten) -
        // schreibt NUR in ein Ringpuffer-Array (reines RAM), keine
        // Flash-residenten Aufrufe, daher hier sicher (siehe Flash-Cache-
        // Warnung oben in dieser Funktion). digitalRead() selbst ist hier
        // unproblematisch - DCF77::int0handler() eine Zeile darueber macht
        // intern nichts anderes, um den Pegel auszuwerten.
        //
        // Bei vollem Puffer (z.B. waehrend eines blockierenden NTP-Sync-
        // Versuchs, siehe setupNTP() - WAIT_3s) wird die Flanke verworfen
        // statt den noch ungelesenen Tail zu ueberschreiben.

        // Own edge capture, independent of the DCF77 library, for the bit-
        // progress display on /dcf77 (see the dcf77Edge* variables in
        // globals.h and processDcf77Bits() further below) - ONLY writes to a
        // ring buffer array (plain RAM), no flash-resident calls, therefore
        // safe here (see the flash-cache warning above in this function).
        // digitalRead() itself is fine here - DCF77::int0handler() one line
        // above does nothing different internally to read the pin level.
        //
        // On a full buffer (e.g. during a blocking NTP sync attempt, see
        // setupNTP() - WAIT_3s) the edge is dropped instead of overwriting
        // the not-yet-read tail.
        uint8_t dcf77EdgeNextHead = (dcf77EdgeHead + 1) % DCF77_EDGE_BUFFER_SIZE;
        if (dcf77EdgeNextHead != dcf77EdgeTail) {
            dcf77EdgeMillis[dcf77EdgeHead] = millis();
            dcf77EdgeLevel[dcf77EdgeHead] = digitalRead(DCF77_DATAPIN);
            dcf77EdgeHead = dcf77EdgeNextHead;
        }
        else {
            dcf77EdgeDropped++;
        }
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


    // Baut aus dcf77LastDecoded (globals.h) eine LOKALE struct tm. DCF77
    // sendet bereits Lokalzeit (CET/CEST) inklusive explizitem Sommerzeit-Bit
    // - anders als dcf.getUTCTime() der DCF77-Bibliothek (nicht mehr benutzt,
    // siehe applyDcf77DecodedTime() unten) ist daher keine UTC-Umrechnung
    // noetig. Gemeinsam genutzt von updateDcf77Status() (fuer lastDcfSyncTime)
    // und applyDcf77DecodedTime() (fuer die eigentliche Zeituebernahme), damit
    // beide garantiert denselben Wert berechnen.

    // Builds a LOCAL struct tm from dcf77LastDecoded (globals.h). DCF77
    // already transmits local time (CET/CEST) including an explicit
    // summer-time bit - unlike the DCF77 library's dcf.getUTCTime() (no
    // longer used, see applyDcf77DecodedTime() below), no UTC conversion is
    // needed. Shared by updateDcf77Status() (for lastDcfSyncTime) and
    // applyDcf77DecodedTime() (for the actual time takeover), so both are
    // guaranteed to compute the same value.

    struct tm dcf77DecodedToLocalTm() {
        struct tm t = {};
#if defined DCF77_DATAPIN && defined DCF77_INTERRUPT
        t.tm_year = dcf77LastDecoded.year - 1900;
        t.tm_mon  = dcf77LastDecoded.month - 1;
        t.tm_mday = dcf77LastDecoded.day;
        t.tm_hour = dcf77LastDecoded.hour;
        t.tm_min  = dcf77LastDecoded.minute;
        t.tm_sec  = 0;
        t.tm_isdst = dcf77LastDecoded.dst ? 1 : 0; // direkt aus dem DCF77-Sommerzeit-Bit, nicht raten lassen
                                                   // taken directly from the DCF77 summer-time bit, not guessed
#endif
        return t;
    }


    // Haelt dcfTimeFound/lastDcfSyncTime (Status-Anzeige: Topbar-Punkt,
    // /status "DCF77 last sync") auf dem aktuellen Stand des eigenen Bit-
    // Dekoders (dcf77LastDecoded) - UNABHAENGIG davon, ob DCF77 gerade
    // tatsaechlich zur Zeituebernahme benutzt wird (das entscheidet der
    // stuendliche NTP-/DCF77-Sync-Block in uhr3.ino zusammen mit
    // applyDcf77DecodedTime() weiter unten, nach NTP-Vorrang). So zeigt der
    // Status auch dann "DCF77 empfaengt
    // einwandfrei", wenn gerade NTP die Zeit stellt - und faellt bei
    // laengeren Empfangsausfaellen trotzdem auf "bad" zurueck (siehe
    // getDcf77Status() in webserver_routes.h, DCF77_SYNC_STALE_AFTER in
    // config.h), unabhaengig davon welche Quelle die Systemzeit stellt.
    //
    // Frueher hiess diese Funktion getDCF77Time() und setzte hier auch
    // direkt die Systemzeit/RTC - das ist jetzt Aufgabe von
    // applyDcf77DecodedTime() und dem stuendlichen Sync-Block in uhr3.ino
    // (siehe dort fuer die Vorgeschichte: dcf.getUTCTime() der Original-
    // Bibliothek hat trotz nachweislich sauberem Empfang nie synchronisiert).

    // Keeps dcfTimeFound/lastDcfSyncTime (status display: topbar dot,
    // /status "DCF77 last sync") up to date with the own bit decoder
    // (dcf77LastDecoded) - INDEPENDENT of whether DCF77 is actually being
    // used to set the time right now (that's decided by the hourly
    // NTP/DCF77 sync block in uhr3.ino together with applyDcf77DecodedTime()
    // further below, after NTP priority). So the status still shows "DCF77 receiving fine" even while
    // NTP is currently driving the clock - and still falls back to "bad" on
    // a longer reception outage (see getDcf77Status() in webserver_routes.h,
    // DCF77_SYNC_STALE_AFTER in config.h), regardless of which source
    // actually sets the system time.
    //
    // This function used to be called getDCF77Time() and also set the system
    // time/RTC directly here - that is now applyDcf77DecodedTime()'s job,
    // together with the hourly sync block in uhr3.ino (see there for the
    // backstory: the original library's dcf.getUTCTime() never synchronized
    // despite demonstrably clean reception).

    bool updateDcf77Status() {
#if defined DCF77_DATAPIN && defined DCF77_INTERRUPT
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
#else
        return false;
#endif
    }


    // Uebernimmt das aktuellste, gueltige dcf77LastDecoded als Systemzeit
    // (+ RTC, falls vorhanden) - der DCF77-Fallback-Zweig des stuendlichen
    // NTP-/DCF77-Sync-Blocks in uhr3.ino (setup() und loop()), wenn NTP
    // gerade nicht verfuegbar ist. Auch direkt nutzbar fuer den Boot-
    // Sonderfall in connectWiFiAtBoot() (kein WLAN, keine RTC).
    // Liefert false (und tut sonst nichts), wenn kein ausreichend frisches
    // (< 1 Minute altes), Paritaets-korrektes Telegramm vorliegt - das ist
    // z.B. direkt nach dem Boot normal, solange DCF77 noch keine volle Minute
    // empfangen konnte.

    // Applies the most recent, valid dcf77LastDecoded as the system time
    // (+ RTC, if present) - the DCF77 fallback branch of the hourly NTP/DCF77
    // sync block in uhr3.ino (setup() and loop()) when NTP isn't currently
    // available. Also directly usable for the boot-time special case in
    // connectWiFiAtBoot() (no WiFi, no
    // RTC). Returns false (and does nothing else) when no sufficiently fresh
    // (< 1 minute old), parity-correct telegram is available - which is
    // normal e.g. right after boot, before DCF77 has had a chance to receive
    // a full minute yet.

    bool applyDcf77DecodedTime(String source) {
#if defined DCF77_DATAPIN && defined DCF77_INTERRUPT
        if (!dcf77LastDecoded.valid) {
            DEBUG_PRINTLN(source + " skipped: no valid DCF77 telegram decoded yet");
            return false;
        }
        if (millis() - dcf77LastDecoded.decodedAtMillis >= WAIT_1m) {
            DEBUG_PRINTLN(source + " skipped: last decoded DCF77 telegram is too old/stale");
            return false; // zu alt/veraltet
                          // too old/stale
        }

        struct tm dcfLocal = dcf77DecodedToLocalTm();

        // dcf77DecodedToLocalTm() setzt tm_sec IMMER auf 0, weil das nur genau
        // in dem Moment stimmt, in dem die Minutenmarke erkannt und das
        // Telegramm dekodiert wurde (decodedAtMillis) - applyDcf77DecodedTime()
        // erlaubt aber bewusst bis zu knapp WAIT_1m (60s) alte Telegramme
        // (siehe Kommentar oben). Ohne diese Korrektur wurde die Systemzeit/RTC
        // bei jedem Aufruf, der nicht zufaellig exakt auf die Minutenmarke
        // trifft (also praktisch immer), um bis zu ~59 Sekunden ZURUECK
        // gesetzt, weil die seit decodedAtMillis verstrichene Zeit schlicht
        // verworfen wurde. mktime()+Addition+localtime_r() lassen dabei auch
        // einen Minuten-/Stunden-/Tageswechsel korrekt ueberlaufen (z.B.
        // Telegramm um HH:MM:58 dekodiert -> 50s spaeter angewendet -> korrekt
        // HH:(MM+1):48, nicht HH:MM:48).

        // dcf77DecodedToLocalTm() ALWAYS sets tm_sec to 0, because that's only
        // correct in the exact instant the minute marker was recognized and
        // the telegram decoded (decodedAtMillis) - applyDcf77DecodedTime()
        // deliberately allows telegrams up to just under WAIT_1m (60s) old
        // (see comment above). Without this correction, the system time/RTC
        // was set up to ~59 seconds BACKWARDS on every call that doesn't
        // happen to land exactly on the minute marker (i.e. practically
        // always), because the time elapsed since decodedAtMillis was simply
        // discarded. mktime()+addition+localtime_r() also correctly roll over
        // a minute/hour/day boundary in the process (e.g. telegram decoded at
        // HH:MM:58, applied 50s later -> correctly HH:(MM+1):48, not HH:MM:48).
        time_t dcfEpoch = mktime(&dcfLocal);
        dcfEpoch += (time_t)((millis() - dcf77LastDecoded.decodedAtMillis) / 1000);
        localtime_r(&dcfEpoch, &dcfLocal);

        setTimeStruct(dcfLocal, source); // Übergabe der struct tm an die Funktion
                                         // pass struct tm to the function

        // RTC_AVAILABLE_BUT_INVALID (Batterie leer/Zeit vor Kompilierzeit,
        // siehe rtcOk-Zuweisung in setup()) bewusst mit zulassen, analog zu
        // setupNTP() - DCF77 soll eine als "ungueltig" markierte RTC genauso
        // reparieren koennen wie NTP, statt zu warten, bis zufaellig NTP
        // zuerst erfolgreich ist.
        // Deliberately also allow RTC_AVAILABLE_BUT_INVALID (dead battery/
        // time before compile time, see the rtcOk assignment in setup()),
        // matching setupNTP() - DCF77 should be able to repair an RTC flagged
        // as "invalid" just as well as NTP, instead of waiting for NTP to
        // happen to succeed first.
        if (rtcOk == RTC_AVAILABLE || rtcOk == RTC_AVAILABLE_BUT_INVALID) {

            // Setze die RTC mit der synchronisierten Zeit
            // Set the RTC to the synchronized time
            rtc.adjust(DateTime(dcfLocal.tm_year + 1900, dcfLocal.tm_mon + 1, dcfLocal.tm_mday,
                dcfLocal.tm_hour, dcfLocal.tm_min, dcfLocal.tm_sec));
            DEBUG_PRINTLN("[RTC] RTC updated with DCF77 time");
            rtcOk = RTC_AVAILABLE; // siehe Begruendung bei setupNTP() weiter oben
                                   // see the reasoning at setupNTP() further above
        }
        lastRTCUpdate = millis();

        DEBUG_PRINTLN(source + " succeeded");
        return true;
#else
        return false;
#endif
    }


    // Beobachtet dcf77Count auf tatsaechliche Aenderungen (echte Impulse) und
    // merkt sich den Zeitpunkt der letzten - dcf77Count selbst wird nirgends
    // auf 0 zurueckgesetzt (siehe Kommentar bei dcf77Count in globals.h), ist
    // also allein kein verlaessliches "empfaengt gerade noch"-Signal. Wird von
    // getDcf77Status() in webserver_routes.h benutzt, um einen kompletten
    // Empfangsausfall waehrend des Betriebs zu erkennen (DCF77_PULSE_STALE_AFTER
    // in config.h). Bewusst per Vergleich im Hauptthread statt in der ISR
    // (isr() in dieser Datei) erfasst - dort duerfen aus Flash-Cache-Gruenden
    // keine weiteren Operationen ergaenzt werden (siehe Kommentar dort).
    //
    // Pflegt zusaetzlich dcf77PlausiblePulseStreak/dcf77Confirmed (siehe
    // globals.h): eine EINZELNE dcf77Count-Aenderung reicht nicht als Beweis
    // fuer "Empfaenger wirklich angeschlossen" - der Datenpin haengt per
    // CHANGE-Interrupt am GPIO (siehe attachInterrupt() in uhr3.ino), und ein
    // floatender/nicht angeschlossener Pin kann durch Rauschen einzelne
    // Interrupts ausloesen, die sonst faelschlich als "erster Impuls" gezaehlt
    // wuerden. Erst eine Kette aus DCF77_PRESENCE_MIN_STREAK aufeinander-
    // folgenden Aenderungen mit jeweils plausiblem Abstand (siehe
    // DCF77_PRESENCE_MAX_GAP_MS in config.h) gilt als echter Empfang - siehe
    // dortigen Kommentar fuer die genaue Begruendung.

    // Watches dcf77Count for actual changes (real pulses) and remembers the
    // timestamp of the last one - dcf77Count itself is never reset to 0
    // anywhere (see the comment on dcf77Count in globals.h), so on its own
    // it's not a reliable "still receiving right now" signal. Used by
    // getDcf77Status() in webserver_routes.h to detect a complete reception
    // failure during operation (DCF77_PULSE_STALE_AFTER in config.h).
    // Deliberately observed via comparison on the main thread rather than in
    // the ISR (isr() in this file) - no further operations may be added there
    // for flash-cache reasons (see the comment there).
    //
    // Additionally maintains dcf77PlausiblePulseStreak/dcf77Confirmed (see
    // globals.h): a SINGLE dcf77Count change isn't proof that a receiver is
    // actually connected - the data pin sits on a CHANGE interrupt (see
    // attachInterrupt() in uhr3.ino), and a floating/unconnected pin can
    // trigger isolated interrupts from noise, which would otherwise be
    // falsely counted as the "first pulse". Only a chain of
    // DCF77_PRESENCE_MIN_STREAK consecutive changes, each with a plausible
    // gap (see DCF77_PRESENCE_MAX_GAP_MS in config.h), counts as genuine
    // reception - see the comment there for the full reasoning.

    void checkDcf77Health() {
#if defined DCF77_DATAPIN && defined DCF77_INTERRUPT
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
#endif
    }


    // Dekodiert das in dcf77Bits[] gesammelte 59-Bit-Telegramm (siehe
    // processDcf77Bits() unten) gemaess dem DCF77-Zeittelegramm-Format und
    // schreibt das Ergebnis nach dcf77LastDecoded (globals.h) - inklusive
    // der drei Paritaetsbits (Minute/Stunde/Datum), damit die Live-Anzeige
    // (/dcf77, webserver_routes.h) einen erkannten Uebertragungsfehler
    // sichtbar machen kann. Fehlt auch nur ein einzelnes Bit (noch nicht
    // erkannt, z.B. durch eine kurze Empfangsluecke), wird die Dekodierung
    // abgebrochen (dcf77LastDecoded.valid bleibt false), statt mit falschen
    // Nullwerten weiterzurechnen.
    //
    // Bitpositionen nach DCF77-Standard (0-indiziert):
    //  0        Start der Minute (immer 0)
    //  1-14     Wettermeldung/Sonderfunktion (hier ungenutzt)
    //  15       Anrufbit (unregelmaessige Aussendung)
    //  16       Ankuendigung Zeitzonenwechsel
    //  17/18    Sommerzeit/Winterzeit (genau eines von beiden ist 1)
    //  19       Ankuendigung Schaltsekunde
    //  20       Start der Zeitinformation (immer 1)
    //  21-27    Minute (BCD), 28 = Paritaet
    //  29-34    Stunde (BCD), 35 = Paritaet
    //  36-41    Tag, 42-44 Wochentag (1=Montag..7=Sonntag), 45-49 Monat,
    //           50-57 Jahr (2-stellig), 58 = gemeinsame Datums-Paritaet
    //
    // Liefert dcf77LastDecoded - inzwischen die tatsaechliche Grundlage der
    // Zeituebernahme (siehe applyDcf77DecodedTime()/updateDcf77Status() oben;
    // die DCF77-Bibliothek selbst wird dafuer nicht mehr benutzt, siehe
    // dortiger Kommentar zur Umstellung).

    // Decodes the 59-bit telegram collected in dcf77Bits[] (see
    // processDcf77Bits() below) per the DCF77 time telegram format and
    // writes the result to dcf77LastDecoded (globals.h) - including the
    // three parity bits (minute/hour/date), so the live display (/dcf77,
    // webserver_routes.h) can surface a detected transmission error. If even
    // a single bit is missing (not recognized yet, e.g. a brief reception
    // gap), the decode is aborted (dcf77LastDecoded.valid stays false)
    // instead of continuing with wrong zero values.
    //
    // Bit positions per the DCF77 standard (0-indexed):
    //  0        start of minute (always 0)
    //  1-14     weather broadcast/special function (unused here)
    //  15       call bit (irregular transmission)
    //  16       DST change announcement
    //  17/18    summer time/winter time (exactly one of the two is 1)
    //  19       leap second announcement
    //  20       start of time information (always 1)
    //  21-27    minute (BCD), 28 = parity
    //  29-34    hour (BCD), 35 = parity
    //  36-41    day, 42-44 day of week (1=Monday..7=Sunday), 45-49 month,
    //           50-57 year (2-digit), 58 = combined date parity
    //
    // Provides dcf77LastDecoded - by now the actual basis for the time
    // takeover (see applyDcf77DecodedTime()/updateDcf77Status() above; the
    // DCF77 library itself is no longer used for this, see the comment there
    // about the switch).

    void decodeDcf77Telegram() {
#if defined DCF77_DATAPIN && defined DCF77_INTERRUPT
        Dcf77Decoded result;
        result.decodedAtMillis = millis();

        for (uint8_t i = 0; i < DCF77_TELEGRAM_BITS; i++) {
            if (dcf77Bits[i] < 0) {
                // Unvollstaendig (z.B. weil die vermeintliche Minutenmarke in
                // Wirklichkeit nur eine durch Rauschen/eine verlorene Flanke
                // ausgeloeste Fehlklassifizierung war, siehe
                // DCF77_BIT_NOISE_IGNORE_MS in config.h und processDcf77Bits()
                // unten) - dcf77LastDecoded hier bewusst NICHT anfassen: sonst
                // wuerde ein zuvor erfolgreich dekodiertes Telegramm bei jedem
                // solchen Fehlalarm durch einen leeren/ungueltigen Datensatz
                // ueberschrieben, obwohl auf der Seite eigentlich weiter die
                // letzte tatsaechlich gueltige Dekodierung angezeigt werden
                // soll (siehe Bugreport: Live-Anzeige zeigte ein anderes
                // Ergebnis als die tatsaechlich von der DCF77-Bibliothek
                // uebernommene Zeit).
                // Incomplete (e.g. because the presumed minute mark was
                // actually just a misclassification caused by noise/a lost
                // edge, see DCF77_BIT_NOISE_IGNORE_MS in config.h and
                // processDcf77Bits() below) - deliberately do NOT touch
                // dcf77LastDecoded here: otherwise a previously successfully
                // decoded telegram would get overwritten with an empty/
                // invalid record on every such false alarm, even though the
                // page is supposed to keep showing the last actually valid
                // decode (see bug report: the live display showed a
                // different result than the time actually taken over by the
                // DCF77 library).
                return;
            }
        }

        result.callBit = (dcf77Bits[15] == 1);
        result.dst = (dcf77Bits[17] == 1);

        // Minute (Bits 21-27, BCD), Paritaet Bit 28
        // Minute (bits 21-27, BCD), parity bit 28
        int minuteUnits = dcf77Bits[21] + dcf77Bits[22] * 2 + dcf77Bits[23] * 4 + dcf77Bits[24] * 8;
        int minuteTens = dcf77Bits[25] + dcf77Bits[26] * 2 + dcf77Bits[27] * 4;
        result.minute = (uint8_t)(minuteTens * 10 + minuteUnits);
        int minuteParitySum = 0;
        for (int i = 21; i <= 28; i++) minuteParitySum += dcf77Bits[i];
        result.parityMinOk = (minuteParitySum % 2) == 0;

        // Stunde (Bits 29-34, BCD), Paritaet Bit 35
        // Hour (bits 29-34, BCD), parity bit 35
        int hourUnits = dcf77Bits[29] + dcf77Bits[30] * 2 + dcf77Bits[31] * 4 + dcf77Bits[32] * 8;
        int hourTens = dcf77Bits[33] + dcf77Bits[34] * 2;
        result.hour = (uint8_t)(hourTens * 10 + hourUnits);
        int hourParitySum = 0;
        for (int i = 29; i <= 35; i++) hourParitySum += dcf77Bits[i];
        result.parityHourOk = (hourParitySum % 2) == 0;

        // Datum: Tag (36-41), Wochentag (42-44), Monat (45-49), Jahr (50-57),
        // gemeinsame Paritaet Bit 58
        // Date: day (36-41), day of week (42-44), month (45-49), year
        // (50-57), combined parity bit 58
        int dayUnits = dcf77Bits[36] + dcf77Bits[37] * 2 + dcf77Bits[38] * 4 + dcf77Bits[39] * 8;
        int dayTens = dcf77Bits[40] + dcf77Bits[41] * 2;
        result.day = (uint8_t)(dayTens * 10 + dayUnits);

        result.weekday = (uint8_t)(dcf77Bits[42] + dcf77Bits[43] * 2 + dcf77Bits[44] * 4);

        int monthUnits = dcf77Bits[45] + dcf77Bits[46] * 2 + dcf77Bits[47] * 4 + dcf77Bits[48] * 8;
        int monthTens = dcf77Bits[49];
        result.month = (uint8_t)(monthTens * 10 + monthUnits);

        int yearUnits = dcf77Bits[50] + dcf77Bits[51] * 2 + dcf77Bits[52] * 4 + dcf77Bits[53] * 8;
        int yearTens = dcf77Bits[54] + dcf77Bits[55] * 2 + dcf77Bits[56] * 4 + dcf77Bits[57] * 8;
        result.year = (uint16_t)(2000 + yearTens * 10 + yearUnits);

        int dateParitySum = 0;
        for (int i = 36; i <= 58; i++) dateParitySum += dcf77Bits[i];
        result.parityDateOk = (dateParitySum % 2) == 0;

        result.valid = result.parityMinOk && result.parityHourOk && result.parityDateOk;

        dcf77LastDecoded = result;
#endif
    }


    // Wertet den von der ISR gefuellten Flanken-Ringpuffer aus (siehe isr()
    // oben und die dcf77Edge*-Variablen in globals.h) und baut daraus - rein
    // ueber die Dauer zwischen zwei Flanken, unabhaengig von der
    // tatsaechlichen Pegel-Polaritaet des jeweiligen Empfaengermoduls - live
    // den Bit-Fortschritt des laufenden Telegramms auf (dcf77Bits[]/
    // dcf77BitIndex): kurze Dauer (<350ms) = das Bit dieser Sekunde (0 bei
    // ~100ms, 1 bei ~200ms Sendedauer), mittlere Dauer (350-1299ms) =
    // normale Sekundenpause (nichts zu tun, das Bit kam bereits aus der
    // vorangegangenen kurzen Dauer), lange Dauer (>=1300ms) = die fehlende
    // 59. Sekundenmarke, also der Minutenwechsel: laufendes Telegramm
    // abschliessen und dekodieren (siehe decodeDcf77Telegram()), dann
    // dcf77BitIndex fuer die neue Minute auf 0 zuruecksetzen - UNABHAENGIG
    // von der aktuellen Position im Telegramm (siehe die ausfuehrliche
    // Begruendung direkt beim ">= 1300"-Zweig weiter unten: ein
    // positionsbasierter Plausibilitaetstest wurde testweise eingebaut und
    // wieder verworfen, weil er bei mehrsekuendigen Aussetzern die echte
    // Minutenmarke selbst verpasst/falsch einordnet - schlimmer als das
    // Problem, das er loesen sollte).
    //
    // Muss aus loop() gerufen werden (NICHT aus der ISR - siehe deren
    // Flash-Cache-Warnung bei isr()), damit Array-Operationen und
    // decodeDcf77Telegram() ohne Einschraenkung laufen koennen. Rein
    // informativ/zum Debugging, komplett unabhaengig von der DCF77-
    // Bibliothek und deren eigener Zeitdekodierung (applyDcf77DecodedTime()/
    // updateDcf77Status() oben).

    // Evaluates the edge ring buffer filled by the ISR (see isr() above and
    // the dcf77Edge* variables in globals.h) and builds, purely from the
    // duration between two edges and independent of the actual signal
    // polarity of the particular receiver module, the running telegram's
    // live bit progress (dcf77Bits[]/dcf77BitIndex): short duration (<350ms)
    // = this second's bit (0 for a ~100ms, 1 for a ~200ms transmit
    // duration), medium duration (350-1299ms) = the normal rest of the
    // second (nothing to do, the bit already came from the preceding short
    // duration), long duration (>=1300ms) = the missing 59th second mark,
    // i.e. the minute change: finalize and decode the running telegram (see
    // decodeDcf77Telegram()), then reset dcf77BitIndex to 0 for the new
    // minute - REGARDLESS of the current position within the telegram (see
    // the detailed reasoning right at the ">= 1300" branch further below: a
    // position-based plausibility check was tried and reverted, because it
    // caused the genuine minute marker itself to be missed/misjudged during
    // multi-second dropouts - worse than the problem it was meant to solve).
    //
    // Must be called from loop() (NOT from the ISR - see its flash-cache
    // warning at isr()), so array operations and decodeDcf77Telegram() can
    // run without restriction. Purely informational/for debugging,
    // completely independent of the DCF77 library and its own time decoding
    // (applyDcf77DecodedTime()/updateDcf77Status() above).

    void processDcf77Bits() {
#if defined DCF77_DATAPIN && defined DCF77_INTERRUPT
        static unsigned long prevEdgeMillis = 0;
        static bool havePrevEdge = false;

        while (dcf77EdgeTail != dcf77EdgeHead) {
            unsigned long edgeMillis = dcf77EdgeMillis[dcf77EdgeTail];
            // dcf77EdgeLevel[dcf77EdgeTail] wird hier bewusst nicht
            // ausgewertet (siehe Kommentar oben) - die Klassifizierung
            // arbeitet rein ueber die Dauer zwischen zwei Flanken.
            // dcf77EdgeLevel[dcf77EdgeTail] is deliberately not evaluated
            // here (see the comment above) - classification works purely
            // from the duration between two edges.
            dcf77EdgeTail = (dcf77EdgeTail + 1) % DCF77_EDGE_BUFFER_SIZE;

            if (!havePrevEdge) {
                prevEdgeMillis = edgeMillis;
                havePrevEdge = true;
                continue;
            }

            unsigned long duration = edgeMillis - prevEdgeMillis;

            // Rauschen/Kontaktprellen an der Flanke (elektrischer Jitter,
            // siehe DCF77_BIT_NOISE_IGNORE_MS in config.h sowie
            // DCF77_PRESENCE_MIN_STREAK/-MAX_GAP_MS weiter oben in derselben
            // Datei fuer denselben Effekt bei der Anwesenheitserkennung):
            // eine derart kurze Flanke ignorieren, OHNE prevEdgeMillis auf
            // sie zu verschieben - so wird sie beim naechsten, echten
            // Flankenwechsel einfach "uebersprungen" statt eine viel zu
            // kurze Dauer zu erzeugen, die faelschlich als eigenes Bit
            // gezaehlt wuerde (siehe Bugreport: Bit-Fortschritt sprang
            // sichtbar mehrere Sekunden auf einmal vor, Werte stimmten
            // nicht).
            // Noise/contact bounce on the edge (electrical jitter, see
            // DCF77_BIT_NOISE_IGNORE_MS in config.h and
            // DCF77_PRESENCE_MIN_STREAK/-MAX_GAP_MS further up in this same
            // file for the same effect on presence detection): ignore such
            // a short edge WITHOUT shifting prevEdgeMillis to it - that way
            // it's simply "skipped over" at the next, genuine edge change
            // instead of producing a far-too-short duration that would be
            // wrongly counted as its own bit (see bug report: bit progress
            // visibly jumped ahead by several seconds at once, values were
            // wrong).
            if (duration < DCF77_BIT_NOISE_IGNORE_MS) {
                continue;
            }

            prevEdgeMillis = edgeMillis;

            if (duration >= 1300) {
                // Zurueckgebaut: hier stand testweise eine bitIndex-basierte
                // Plausibilitaetspruefung (DCF77_MINUTE_MARKER_MIN_BIT_INDEX),
                // die eine Pause >=1300ms nur nahe dem Telegrammende als
                // echte Minutenmarke akzeptiert hat, sonst nur 1 Position als
                // verloren markiert. Das ging von genau EINER verlorenen
                // Sekunde pro Pause aus - bei zwei oder mehr AUFEINANDER-
                // FOLGENDEN schwachen/verlorenen Sekunden (laengere Pause,
                // z.B. ~2900ms statt ~1900ms) wurde bitIndex trotzdem nur um 1
                // erhoeht, blieb also hinter der Wanduhr zurueck - die dann
                // tatsaechlich folgende echte Minutenmarke traf dadurch auf
                // einen zu niedrigen bitIndex und wurde entweder verworfen
                // (Bugreport: "59. Sekunde wird nicht erkannt") oder erst bei
                // einer spaeteren, eigentlich falschen Position akzeptiert
                // ("...oder an falscher Stelle") - je nachdem, wie weit
                // bitIndex zu diesem Zeitpunkt bereits hinterherhing. Die
                // Grundannahme (bitIndex verlaesslich synchron zur echten
                // Sekunde-der-Minute) haelt bei mehrsekuendigen Aussetzern
                // also nicht.
                //
                // Zurueck zur einfachen, dafuer aber IMMER selbstkorrigierenden
                // Variante: jede Pause >=1300ms gilt als Minutenmarke, egal an
                // welcher Position - dadurch kann die Synchronisation nie
                // laenger als eine Minute verloren gehen. Der einzige "Preis"
                // eines einzelnen Aussetzers ist ein verworfenes, weil
                // unvollstaendiges Telegramm fuer genau diese eine Minute -
                // das betrifft nur die Live-Anzeige auf /dcf77, NICHT
                // dcf77LastDecoded: decodeDcf77Telegram() ueberschreibt dieses
                // ohnehin nur bei einem vollstaendigen UND Paritaets-korrekten
                // Telegramm (siehe dortiger Vollstaendigkeits-Check) - haeufige
                // Resets bei schwachem Empfang sind also ein sichtbares, aber
                // unschaedliches Symptom, keine Datenkorruption.

                // Reverted: this used to have a bitIndex-based plausibility
                // check (DCF77_MINUTE_MARKER_MIN_BIT_INDEX) that only
                // accepted a >=1300ms gap as the genuine minute marker near
                // the end of a telegram, otherwise just marked 1 position as
                // lost. That assumed exactly ONE lost second per gap - with
                // two or more CONSECUTIVE weak/lost seconds (a longer gap,
                // e.g. ~2900ms instead of ~1900ms), bitIndex was still only
                // advanced by 1, so it fell behind the wall clock - the
                // genuine minute marker that then actually followed hit an
                // implausibly low bitIndex and was either discarded (bug
                // report: "59th second not recognized") or only accepted
                // later at what was actually the wrong position ("...or at
                // the wrong position") - depending how far bitIndex had
                // already drifted by then. The underlying assumption
                // (bitIndex reliably in sync with the real second-of-minute)
                // doesn't hold across multi-second dropouts.
                //
                // Back to the simple variant, which is ALWAYS self-
                // correcting instead: any gap >=1300ms counts as the minute
                // marker, regardless of position - so sync can never be lost
                // for more than one minute. The only "cost" of a single
                // dropout is one discarded, incomplete telegram for that one
                // minute - this only affects the live display on /dcf77, NOT
                // dcf77LastDecoded: decodeDcf77Telegram() only ever
                // overwrites that on a complete AND parity-correct telegram
                // (see its completeness check) - frequent resets during weak
                // reception are a visible but harmless symptom, not data
                // corruption.
                decodeDcf77Telegram();
                for (uint8_t i = 0; i < DCF77_TELEGRAM_BITS; i++) dcf77Bits[i] = -1;
                dcf77BitIndex = 0;
            }
            else if (duration < 350) {
                // Impulsdauer dieser Sekunde -> Bitwert
                // this second's pulse duration -> bit value
                if (dcf77BitIndex < DCF77_TELEGRAM_BITS) {
                    dcf77Bits[dcf77BitIndex] = (duration >= 150) ? 1 : 0;
                    dcf77BitIndex++;
                }
            }
            // 350-1299ms: normale Sekundenpause, nichts zu tun
            // 350-1299ms: normal rest of the second, nothing to do
        }
#endif
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

            // 500ms waren in der Praxis oft zu knapp fuer DNS-Aufloesung plus
            // NTP-Antwort ueber das offene Internet - auf 3s verlaengert, wie
            // bei testNtpServer() weiter oben. Zusaetzlich NTP_SYNC_ATTEMPTS
            // Versuche pro Server (siehe config.h): ein einzelnes verlorenes
            // UDP-Antwortpaket von einem oeffentlichen Pool-Server ist
            // gelegentlich normal und soll nicht sofort als Fehlschlag des
            // ganzen Servers gewertet werden. configTzTime() wird pro Versuch
            // neu aufgerufen, damit der SNTP-Client auch wirklich eine neue
            // Anfrage verschickt (siehe Begruendung bei NTP_SYNC_ATTEMPTS).

            // 500ms was often too short in practice for DNS resolution plus
            // the NTP response over the open internet - extended to 3s,
            // matching testNtpServer() further above. Additionally,
            // NTP_SYNC_ATTEMPTS attempts per server (see config.h): a single
            // lost UDP response packet from a public pool server is
            // occasionally normal and shouldn't immediately be treated as
            // that whole server failing. configTzTime() is re-issued for
            // each attempt so the SNTP client actually sends a fresh request
            // (see the reasoning at NTP_SYNC_ATTEMPTS).
            bool ntpResponded = false;
            for (uint8_t attempt = 0; attempt < NTP_SYNC_ATTEMPTS; attempt++) {
                if (attempt > 0) {
                    DEBUG_PRINTLN("[NTP] No response from " + ntpServer + ", retrying (attempt " + String(attempt + 1) + "/" + String(NTP_SYNC_ATTEMPTS) + ")..");
                }
                configTzTime(timezone.c_str(), ntpServers[i]);
                if (getLocalTime(&timeinfo, WAIT_3s)) {
                    ntpResponded = true;
                    break;
                }
            }

            if (ntpResponded) {

                DEBUG_PRINTLN("[NTP] Time synchronized successfully with " + ntpServer);
                lastNtpSuccessMillis = millis();

                if (rtcOk == RTC_AVAILABLE || rtcOk == RTC_AVAILABLE_BUT_INVALID) {

                    // Setze die RTC mit der synchronisierten Zeit
                    // Set the RTC to the synchronized time
                    rtc.adjust(DateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                        timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));
                    DEBUG_PRINTLN("[RTC] RTC updated with NTP time");

                    // War die RTC bisher als "verfuegbar, aber Zeit ungueltig"
                    // markiert (leere Batterie/Zeit vor Kompilierzeit, siehe
                    // rtcOk-Zuweisung in setup()), ist sie ab jetzt wieder
                    // vertrauenswuerdig - rtcOk wurde vorher NIRGENDS wieder
                    // zurueckgesetzt, wodurch /status dauerhaft "invalid"
                    // anzeigte UND applyDcf77DecodedTime() (die strikt auf
                    // RTC_AVAILABLE prueft) die physisch bereits korrekt
                    // gestellte RTC nie wieder aktualisiert haette, sobald NTP
                    // spaeter ausfaellt und DCF77 uebernimmt.

                    // If the RTC was previously flagged as "available but time
                    // invalid" (dead battery/time before compile time, see the
                    // rtcOk assignment in setup()), it is trustworthy again
                    // from here on - rtcOk was previously NEVER reset back,
                    // which made /status show "invalid" forever AND meant
                    // applyDcf77DecodedTime() (which strictly checks for
                    // RTC_AVAILABLE) would never again update the by-now
                    // physically correct RTC once NTP later failed and DCF77
                    // took over.
                    rtcOk = RTC_AVAILABLE;
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
        // Ein Wiederholungsversuch wird nicht mehr hier eingeplant, sondern
        // ergibt sich automatisch aus dem stuendlichen NTP-Aufruf in loop()
        // (siehe checkHourlyTimeSync-Logik dort) - fallen alle NTP-Server
        // aus, springt derselbe Aufrufer zusaetzlich per
        // applyDcf77DecodedTime() auf DCF77 als Zeitquelle um.
        // A retry is no longer scheduled here - it falls out automatically
        // from the hourly NTP call in loop() (see the checkHourlyTimeSync
        // logic there); if all NTP servers fail, that same caller also falls
        // back to DCF77 as the time source via applyDcf77DecodedTime().
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


    // Prueft periodisch (stuendlich, alle WAIT_1h), ob die beim Boot erkannte
    // RTC noch auf dem I2C-Bus antwortet, und setzt rtcOk bei einem Ausfall auf
    // RTC_NOT_AVAILABLE - vorher wurde rtcOk nur EINMAL beim Boot in setup()
    // ermittelt und danach nie wieder geprueft, sodass ein Ausfall der RTC
    // waehrend des Betriebs (Chip getauscht/abgeklemmt, I2C-Fehler) vom
    // RTC-Punkt in der Topbar nie angezeigt wurde. Nutzt denselben minimalen
    // I2C-Ping wie i2cScan() (nur Adresse 0x68) statt eines vollen Bus-Scans.
    // Erkennt bewusst NUR den Ausfall, keine Wiederkehr: eine (wieder)
    // angeschlossene RTC wird erst nach einem Neustart erneut vollstaendig
    // initialisiert (rtc.begin(), lostPower()-Pruefung etc., siehe setup()) -
    // das hier nachzubilden waere fehleranfaellig und wuerde die Zeitquelle
    // nach einem Wackelkontakt unbeaufsichtigt umschalten.

    // Periodically checks (hourly, every WAIT_1h) whether the RTC detected at boot
    // still responds on the I2C bus, and sets rtcOk to RTC_NOT_AVAILABLE on a
    // failure - previously rtcOk was only ever determined ONCE at boot in
    // setup() and never rechecked, so an RTC failure during operation (chip
    // swapped/disconnected, I2C error) was never reflected by the RTC dot in
    // the topbar. Uses the same minimal I2C ping as i2cScan() (address 0x68
    // only) instead of a full bus scan. Deliberately detects only failure, not
    // recovery: an RTC that comes back (or is reconnected) is only fully
    // reinitialized after a restart (rtc.begin(), lostPower() check etc., see
    // setup()) - replicating that here would be error-prone and would switch
    // the time source unattended after a loose connection.

    void checkRtcHealth() {
#if defined SDA_PIN && defined SCL_PIN
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
#endif
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
