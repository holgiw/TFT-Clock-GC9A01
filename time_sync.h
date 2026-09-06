#pragma once
    // ### Zeit: RTC, DCF77, NTP-Client & -Server, Zeitzone ################
    // ### Time: RTC, DCF77, NTP client & server, timezone ################
    // Benoetigt globals.h, config.h, prefs_keys.h und declarations.h (werden
    // zentral in uhr3.ino VOR dieser Datei eingebunden).

    // Requires globals.h, config.h, prefs_keys.h and declarations.h (these are
    // included centrally in uhr3.ino BEFORE this file).


    // Interrupt-Handler fuer den DCF77-Eingang. Laeuft bei JEDER Flanke am
    // Datenpin (CHANGE, siehe attachInterrupt() in uhr3.ino) und schreibt
    // ausschliesslich einen Zeitstempel in einen RAM-Ringpuffer - die
    // eigentliche Auswertung passiert in processDcf77Bits() aus loop().
    // Begruendung im Detail siehe im Rumpf.

    // Interrupt handler for the DCF77 input. Runs on EVERY edge at the data
    // pin (CHANGE, see attachInterrupt() in uhr3.ino) and only ever writes a
    // timestamp into a RAM ring buffer - the actual evaluation happens in
    // processDcf77Bits() called from loop(). See the body for the detailed
    // reasoning.

    void IRAM_ATTR isr() {
#if defined DCF77_DATAPIN && defined DCF77_INTERRUPT

        // In dieser Funktion darf NICHTS aufgerufen werden, was im Flash
        // liegt. Waehrend der Flash-Cache deaktiviert ist - das ist bei JEDEM
        // LittleFS-Schreibvorgang (also auch bei jeder Logzeile ueber
        // logToFile()) und bei jedem nvs_commit() aus preferences.putXxx() der
        // Fall - fuehrt ein solcher Zugriff im guenstigen Fall dazu, dass die
        // Flanke verlorengeht, im unguenstigen zu einem "Cache disabled but
        // cached memory region accessed"-Panic-Reset.
        //
        // Entfernt wurden deshalb (beide lagen im Flash und waren nicht
        // ersetzbar, nur verzichtbar):
        //
        // 1. DCF77::int0handler() - die Bibliotheksfunktion. Ihr Ergebnis
        //    (dcf.getUTCTime()) wird seit der Umstellung auf den eigenen
        //    Dekoder nirgends mehr gelesen (siehe applyDcf77DecodedTime()
        //    weiter unten), der Aufruf war also reine Rechenzeit mit
        //    Absturzrisiko in der ISR.
        // 2. digitalRead(DCF77_DATAPIN) fuer dcf77EdgeLevel[] - der Pegel wird
        //    von processDcf77Bits() nicht ausgewertet (die Klassifizierung
        //    laeuft ausschliesslich ueber die Dauer zwischen zwei Flanken),
        //    das Array war reine Diagnose-Altlast.
        //
        // Uebrig bleiben nur millis() und Schreibzugriffe auf RAM-Arrays.
        // Genau das war die Ursache fuer verlorene Flanken ("lost") waehrend
        // aktivem Logging: jede geschriebene Logzeile deaktivierte den Cache,
        // und die in dieser Zeit anfallenden DCF77-Flanken gingen verloren -
        // wodurch der Dekoder Sekunden verlor, haeufiger die Synchronisation
        // verwarf und dadurch noch mehr Logzeilen erzeugte.

        // NOTHING that lives in flash may be called in this function. While
        // the flash cache is disabled - which is the case during EVERY
        // LittleFS write (so also every log line via logToFile()) and every
        // nvs_commit() from preferences.putXxx() - such an access at best
        // makes the edge get lost and at worst causes a "Cache disabled but
        // cached memory region accessed" panic reset.
        //
        // Removed for that reason (both lived in flash and were not
        // replaceable, merely dispensable):
        //
        // 1. DCF77::int0handler() - the library function. Its result
        //    (dcf.getUTCTime()) has not been read anywhere since the switch to
        //    the own decoder (see applyDcf77DecodedTime() further below), so
        //    the call was pure compute time with a crash risk inside the ISR.
        // 2. digitalRead(DCF77_DATAPIN) for dcf77EdgeLevel[] - the level is
        //    not evaluated by processDcf77Bits() (classification works purely
        //    from the duration between two edges), the array was a leftover
        //    diagnostic.
        //
        // What remains is millis() plus writes to RAM arrays. This was exactly
        // the cause of lost edges while logging was enabled: every log line
        // written disabled the cache, and the DCF77 edges occurring during
        // that window were lost - which made the decoder lose seconds, drop
        // synchronization more often and thereby produce even more log lines.
        if (!dcfTimeFound) dcfLedTogglePending = true;
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
    // (hoechstens DCF77_DECODED_MAX_AGE altes, siehe config.h),
    // Paritaets-korrektes Telegramm vorliegt - das ist
    // z.B. direkt nach dem Boot normal, solange DCF77 noch keine volle Minute
    // empfangen konnte.

    // Applies the most recent, valid dcf77LastDecoded as the system time
    // (+ RTC, if present) - the DCF77 fallback branch of the hourly NTP/DCF77
    // sync block in uhr3.ino (setup() and loop()) when NTP isn't currently
    // available. Also directly usable for the boot-time special case in
    // connectWiFiAtBoot() (no WiFi, no
    // RTC). Returns false (and does nothing else) when no sufficiently fresh
    // (at most DCF77_DECODED_MAX_AGE old, see config.h), parity-correct
    // telegram is available - which is
    // normal e.g. right after boot, before DCF77 has had a chance to receive
    // a full minute yet.

    bool applyDcf77DecodedTime(String source) {
#if defined DCF77_DATAPIN && defined DCF77_INTERRUPT
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

    bool decodeDcf77Telegram(unsigned long decodedAtMillis) {
#if defined DCF77_DATAPIN && defined DCF77_INTERRUPT

        // Strukturpruefung VOR allem anderen: Bit 0 ist im DCF77-Telegramm
        // immer 0 (Minutenbeginn), Bit 20 immer 1 (Start der Zeitinformation).
        // Widerspricht auch nur eines der beiden, kann das Telegramm nicht an
        // der angenommenen Position begonnen haben - der Dekoder steht also
        // auf der falschen Sekunde. Dann false zurueckgeben; der Aufrufer
        // verwirft nach mehreren solchen Telegrammen die erkannte Minutenmarke
        // und sucht sie neu (siehe DCF77_STRUCT_FAIL_LIMIT in config.h).
        // Geprueft wird nur gegen TATSAECHLICH empfangene Bits (-1 = Luecke).

        // Structure check before anything else: in a DCF77 telegram bit 0 is
        // always 0 (start of minute) and bit 20 always 1 (start of time
        // information). If either one contradicts that, the telegram cannot
        // have started at the assumed position - so the decoder sits on the
        // wrong second. Return false in that case; after several such
        // telegrams the caller discards the detected minute marker and
        // searches for it anew (see DCF77_STRUCT_FAIL_LIMIT in config.h).
        // Only checked against bits ACTUALLY received (-1 = gap).
        // dcf77Bits ist nach Rasterposition indiziert (siehe globals.h); die
        // Minutenmarke liegt auf dcf77MarkerPos und ist die 59. Sekunde, die
        // Position danach ist die Sekunde 0. Hier einmal in Sekunden-
        // reihenfolge umsortieren, danach arbeitet die ganze Funktion auf
        // 'bits' mit den vertrauten Bitnummern des DCF77-Telegramms.

        // dcf77Bits is indexed by grid position (see globals.h); the minute
        // marker sits at dcf77MarkerPos and is the 59th second, the position
        // after it is second 0. Reorder into second order once here; after
        // that the whole function works on 'bits' with the DCF77 telegram's
        // familiar bit numbers.
        if (dcf77MarkerPos < 0) return true; // ohne Marke ist keine Zuordnung moeglich
                                              // without the marker no mapping is possible

        int8_t bits[DCF77_TELEGRAM_BITS];
        for (uint8_t sec = 0; sec < DCF77_TELEGRAM_BITS; sec++) {
            bits[sec] = dcf77Bits[((uint8_t)dcf77MarkerPos + 1 + sec) % DCF77_GRID_SLOTS];
        }

        if (bits[0] == 1 || bits[20] == 0) {
            return false;
        }

        // --- Arbeitskopie mit Rekonstruktion fehlender Bits ----------------
        //
        // Ohne diesen Schritt braucht ein verwertbares Telegramm eine
        // lueckenlose Minute: 42 aufeinanderfolgende Sekunden muessen sauber
        // ankommen. Schon bei 10 % Ausfallquote passiert das rechnerisch nur
        // in gut einer von hundert Minuten - der Dekoder lief dann zwar
        // sauber, lieferte aber praktisch nie ein Ergebnis.
        //
        // Rekonstruierbar sind:
        //  - Bit 0 und Bit 20: im Protokoll fest 0 bzw. 1
        //  - Bit 17/18 (Sommer-/Winterzeit): zueinander invers, eines
        //    ergaenzt das andere
        //  - genau EIN fehlendes Bit je Paritaetsgruppe (Minute 21-28,
        //    Stunde 29-35, Datum 36-58): sein Wert ist der, der die gerade
        //    Paritaet der Gruppe herstellt
        //
        // Das ist die uebliche Einzel-Ausfall-Korrektur ueber die Paritaet.
        // Der Preis: fuer eine so ergaenzte Gruppe kann die Paritaet nichts
        // mehr pruefen (sie wurde ja gerade erfuellt). Deshalb wird ein
        // Telegramm mit rekonstruierten Bits weiter unten zusaetzlich gegen
        // die zuletzt bestaetigte Zeit geprueft.

        // --- Working copy with reconstruction of missing bits --------------
        //
        // Without this step a usable telegram needs a gapless minute: 42
        // consecutive seconds have to arrive cleanly. At a 10 % dropout rate
        // that happens, statistically, in only about one minute in a hundred -
        // the decoder then ran correctly but practically never produced a
        // result.
        //
        // Reconstructable are:
        //  - bit 0 and bit 20: fixed at 0 resp. 1 by the protocol
        //  - bits 17/18 (summer/winter time): inverse to each other, one
        //    completes the other
        //  - exactly ONE missing bit per parity group (minute 21-28, hour
        //    29-35, date 36-58): its value is the one that makes the group's
        //    parity even
        //
        // This is the usual single-erasure correction via parity. The price:
        // for a group completed this way, parity can no longer verify anything
        // (it was just satisfied by construction). A telegram with
        // reconstructed bits is therefore additionally checked against the
        // last confirmed time further below.
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

        // Vollstaendigkeit NUR fuer die Bits verlangen, die tatsaechlich in die
        // Zeit eingehen: Bit 17/18 (Sommer-/Winterzeit), Bit 20 (Startbit) und
        // 21..58 (Minute/Stunde/Datum inkl. Paritaeten). Die Bits 1..14
        // (Wettermeldung/Sonderfunktion), Bit 15 (Anrufbit) und Bit 19
        // (Schaltsekunde) wertet dieser Dekoder gar nicht aus bzw. nur zur
        // Anzeige - eine Luecke dort darf ein sonst vollstaendiges und
        // korrektes Zeittelegramm nicht verwerfen.

        // Require completeness ONLY for the bits that actually go into the
        // time: bits 17/18 (summer/winter time), bit 20 (start bit) and 21..58
        // (minute/hour/date incl. parities). Bits 1..14 (weather
        // broadcast/special function), bit 15 (call bit) and bit 19 (leap
        // second) are not evaluated by this decoder at all, or only for
        // display - a gap there must not discard an otherwise complete and
        // correct time telegram.
        for (uint8_t i = 17; i < DCF77_TELEGRAM_BITS; i++) {
            if (i == 19) continue; // Schaltsekunden-Ankuendigung, hier nicht ausgewertet
                                    // leap second announcement, not evaluated here

            if (bits[i] < 0) {
                // Immer noch unvollstaendig - dcf77LastDecoded bewusst NICHT
                // anfassen, damit die letzte tatsaechlich gueltige Dekodierung
                // auf der Live-Seite stehen bleibt. true zurueckgeben: eine
                // Luecke ist kein Grund, die Minutenmarke in Frage zu stellen.
                // Still incomplete - deliberately do NOT touch
                // dcf77LastDecoded, so the last actually valid decoding stays
                // on the live page. Return true: a gap is no reason to doubt
                // the minute marker.
                return true;
            }
        }

        Dcf77Decoded result;
        // 0 bedeutet an anderer Stelle "noch nie dekodiert" (siehe
        // /api/dcf77status in webserver_routes.h) - einen echten Zeitpunkt 0
        // (nur in der ersten Millisekunde nach dem Boot moeglich) deshalb auf 1
        // anheben, statt das Telegramm dort als "nicht vorhanden" erscheinen zu lassen.
        // 0 means "never decoded" elsewhere (see /api/dcf77status in
        // webserver_routes.h) - so lift a genuine timestamp of 0 (only possible
        // in the very first millisecond after boot) to 1, instead of making the
        // telegram appear "not present" there.
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

        // Zusaetzlich zu den drei Paritaeten auch die Wertebereiche pruefen:
        // die Paritaet erkennt nur eine UNGERADE Anzahl gekippter Bits - bei
        // zwei Fehlern innerhalb derselben Gruppe (bei gestoertem Empfang
        // durchaus moeglich) stimmt sie trotzdem, und ohne diese Pruefung
        // waere z.B. "Monat 15" oder "Stunde 29" als gueltige Zeit
        // durchgegangen und haette Systemzeit und RTC verstellt. Ebenso
        // muessen die beiden Zeitzonenbits 17/18 zueinander invers sein.

        // Besides the three parities, also check the value ranges: parity only
        // detects an ODD number of flipped bits - with two errors inside the
        // same group (entirely possible with disturbed reception) it still
        // matches, and without this check e.g. "month 15" or "hour 29" would
        // have passed as a valid time and adjusted the system time and RTC.
        // Likewise the two timezone bits 17/18 have to be inverse.
        bool rangesOk = (result.minute <= 59) &&
                        (result.hour <= 23) &&
                        (result.day >= 1 && result.day <= 31) &&
                        (result.month >= 1 && result.month <= 12) &&
                        (result.weekday >= 1 && result.weekday <= 7) &&
                        (bits[17] != bits[18]);

        bool selfConsistent = result.parityMinOk && result.parityHourOk && result.parityDateOk && rangesOk;

        // Zeitpunkt dieses Telegramms als Unix-Zeit - Bezugspunkt sowohl fuer
        // die Kohaerenzpruefung unten als auch fuer die naechste Minute.
        // This telegram's time as a Unix timestamp - the reference both for
        // the coherence check below and for the next minute.
        struct tm decodedTm = {};
        decodedTm.tm_year = result.year - 1900;
        decodedTm.tm_mon = result.month - 1;
        decodedTm.tm_mday = result.day;
        decodedTm.tm_hour = result.hour;
        decodedTm.tm_min = result.minute;
        decodedTm.tm_sec = 0;
        decodedTm.tm_isdst = result.dst ? 1 : 0;
        time_t decodedEpoch = selfConsistent ? mktime(&decodedTm) : 0;

        // Ein VOLLSTAENDIG empfangenes Telegramm ist mit korrekter Paritaet
        // und plausiblen Werten fertig geprueft und sofort gueltig.
        //
        // Ein REKONSTRUIERTES Telegramm nicht: die aus der Paritaet ergaenzten
        // Bits erfuellen die Paritaet ihrer Gruppe per Konstruktion, diese
        // Gruppe ist damit ungeprueft. Es gilt erst als gueltig, wenn es exakt
        // zum vorherigen Telegramm plus der seitdem verstrichenen Minutenzahl
        // passt. Als Bezug reicht dabei auch ein vorheriges rekonstruiertes
        // Telegramm: zwei unabhaengig empfangene Minuten, die genau eine
        // Minute auseinanderliegen, koennen praktisch nicht beide auf
        // dieselbe Weise falsch sein. Ohne diese zweite Instanz wuerde bei
        // schwachem Empfang - also genau dann, wenn die Rekonstruktion
        // gebraucht wird - die erste Zeituebernahme sehr lange auf eine
        // zufaellig einmal lueckenlose Minute warten.

        // A FULLY received telegram is completely verified by correct parity
        // and plausible values, and is valid immediately.
        //
        // A RECONSTRUCTED one is not: the bits filled in from parity satisfy
        // their group's parity by construction, so that group is unverified.
        // It only counts as valid once it matches the previous telegram plus
        // the number of minutes elapsed since, exactly. A previous
        // reconstructed telegram is good enough as the reference: two
        // independently received minutes that lie exactly one minute apart can
        // practically not both be wrong in the same way. Without this second
        // instance, with weak reception - i.e. exactly when reconstruction is
        // needed - the first time takeover would wait a very long time for a
        // minute that happens to arrive without gaps.
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

        // Bezugspunkt fuer die naechste Minute IMMER dann merken, wenn das
        // Telegramm in sich stimmig ist - auch wenn es (noch) nicht als
        // gueltig gilt. Genau so bestaetigen sich zwei aufeinanderfolgende
        // rekonstruierte Telegramme gegenseitig.
        // ALWAYS remember the reference for the next minute when the telegram
        // is self-consistent - even when it does not (yet) count as valid.
        // This is exactly how two consecutive reconstructed telegrams confirm
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
#else
        (void)decodedAtMillis;
        return true;
#endif
    }


    // Setzt die Statistik zur Erkennung der Minutenmarke zurueck (siehe
    // dcf77MarkerMiss/-Hit in globals.h). Noetig, sobald das Sekundenraster
    // verlorengeht: dcf77Phase startet danach an einer beliebigen neuen
    // Stelle, die bisher gesammelten Fehlstellen zeigen also auf Positionen,
    // die es so nicht mehr gibt.

    // Resets the statistics used to detect the minute marker (see
    // dcf77MarkerMiss/-Hit in globals.h). Needed as soon as the second grid is
    // lost: dcf77Phase then restarts at an arbitrary new place, so the missing
    // pulses collected so far point at positions that no longer exist.

    void resetDcf77MarkerStats() {
#if defined DCF77_DATAPIN && defined DCF77_INTERRUPT
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
#endif
    }


    // Sucht in der Fehlstellen-Statistik die Rasterposition der Minutenmarke
    // (die 59. Sekunde, in der DCF77 als einzige keinen Impuls sendet).
    //
    // Gesucht wird die Position, an der noch NIE ein Impuls ankam und die am
    // haeufigsten gefehlt hat - mit deutlichem Vorsprung vor dem naechstbesten
    // Kandidaten (DCF77_MARKER_MIN_LEAD, siehe config.h). Beides zusammen
    // trennt die Marke zuverlaessig von empfangsbedingten Ausfaellen: die
    // Marke fehlt in JEDER Minute, ein schwach empfangener Zeitschlitz nur
    // gelegentlich und bekommt frueher oder spaeter auch einmal einen Impuls
    // (womit er als Kandidat dauerhaft ausscheidet).

    // Searches the missing-pulse statistics for the grid position of the
    // minute marker (the 59th second, the only one in which DCF77 sends no
    // pulse).
    //
    // What is sought is the position where a pulse has NEVER arrived and which
    // has been missing most often - by a clear lead over the next best
    // candidate (DCF77_MARKER_MIN_LEAD, see config.h). Together those two
    // reliably separate the marker from reception dropouts: the marker is
    // missing in EVERY minute, a weakly received time slot only occasionally,
    // and sooner or later it does receive a pulse (which permanently
    // disqualifies it as a candidate).

    void evaluateDcf77Marker() {
#if defined DCF77_DATAPIN && defined DCF77_INTERRUPT
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
#endif
    }


    // Wertet den von der ISR gefuellten Flanken-Ringpuffer aus (siehe isr()
    // oben und die dcf77Edge*-Variablen in globals.h) und baut daraus den
    // Bit-Fortschritt des laufenden Telegramms auf (dcf77Bits[]/dcf77BitIndex)
    // - Grundlage sowohl fuer die Live-Anzeige auf /dcf77 als auch fuer die
    // tatsaechliche Zeituebernahme (dcf77LastDecoded, siehe
    // applyDcf77DecodedTime()/updateDcf77Status() oben).
    //
    // Arbeitsweise in drei Stufen:
    //
    // 1. IMPULSE ERKENNEN. Ein Intervall zwischen zwei Flanken, das kuerzer
    //    als DCF77_PULSE_MAX_MS ist, ist ein Impuls; seine Dauer ergibt den
    //    Bitwert (>=DCF77_PULSE_ONE_MIN_MS -> 1, sonst 0). Alles Laengere ist
    //    die Pause bis zum naechsten Sekundenbeginn. Das funktioniert
    //    unabhaengig von der Pegel-Polaritaet des Empfaengermoduls: in beiden
    //    Faellen ist das kurze der beiden Intervalle der Impuls.
    //
    // 2. SEKUNDENRASTER HALTEN. Der Abstand zweier IMPULSANFAENGE ist bei
    //    DCF77 immer ein ganzzahliges Vielfaches einer Sekunde. Daraus wird
    //    die freilaufende Rasterposition dcf77Phase (0..59) fortgeschrieben -
    //    auch ueber Empfangsluecken hinweg (bis
    //    DCF77_MAX_PHASE_GAP_SECONDS). Eine fehlende Sekunde verschiebt damit
    //    nichts, sie hinterlaesst nur eine Luecke an ihrer eigenen Stelle.
    //
    // 3. MINUTENMARKE BESTIMMEN. Welche Rasterposition die 59. Sekunde ist,
    //    entscheidet NICHT ein einzelner Impulsabstand, sondern die Statistik
    //    ueber mehrere Minuten (siehe evaluateDcf77Marker() oben): die Marke
    //    ist die einzige Position, an der IMMER ein Impuls fehlt.
    //
    // Warum Stufe 3 so und nicht einfacher: eine Pause von zwei Sekunden als
    // Minutenmarke zu werten, funktioniert nur bei praktisch perfektem
    // Empfang. Sobald einzelne Sekunden ausfallen - und genau dann kommt es
    // darauf an - erzeugt jeder Ausfall dieselbe Zwei-Sekunden-Pause wie die
    // echte Marke. Der Dekoder synchronisierte sich dann auf eine falsche
    // Position, verwarf sie beim naechsten Telegramm wieder (Festbits Bit 0 /
    // Bit 20), synchronisierte erneut falsch - und die 59. Sekunde wurde nie
    // stabil erkannt. Ueber die Haeufigkeit gemittelt verschwindet dieses
    // Problem: zufaellige Ausfaelle streuen ueber alle 60 Positionen, die
    // Marke trifft immer dieselbe.
    //
    // Muss aus loop() gerufen werden (NICHT aus der ISR - siehe deren
    // Flash-Cache-Warnung bei isr()), damit Array-Operationen und
    // decodeDcf77Telegram() ohne Einschraenkung laufen koennen.

    // Evaluates the edge ring buffer filled by the ISR (see isr() above and
    // the dcf77Edge* variables in globals.h) and builds the running telegram's
    // bit progress from it (dcf77Bits[]/dcf77BitIndex) - the basis both for
    // the live display on /dcf77 and for the actual time takeover
    // (dcf77LastDecoded, see applyDcf77DecodedTime()/updateDcf77Status()
    // above).
    //
    // How it works, in three stages:
    //
    // 1. DETECT PULSES. An interval between two edges shorter than
    //    DCF77_PULSE_MAX_MS is a pulse; its length gives the bit value
    //    (>=DCF77_PULSE_ONE_MIN_MS -> 1, otherwise 0). Anything longer is the
    //    rest of the second. This works regardless of the receiver module's
    //    signal polarity: in both cases the shorter of the two intervals is
    //    the pulse.
    //
    // 2. HOLD THE SECOND GRID. With DCF77 the distance between two PULSE
    //    STARTS is always a whole multiple of one second. From that, the
    //    free-running grid position dcf77Phase (0..59) is advanced - across
    //    reception gaps as well (up to DCF77_MAX_PHASE_GAP_SECONDS). A missing
    //    second therefore shifts nothing, it only leaves a hole at its own
    //    position.
    //
    // 3. DETERMINE THE MINUTE MARKER. Which grid position is the 59th second
    //    is NOT decided by a single pulse distance but by the statistics over
    //    several minutes (see evaluateDcf77Marker() above): the marker is the
    //    only position where a pulse is ALWAYS missing.
    //
    // Why stage 3 works this way and not more simply: treating a two-second
    // gap as the minute marker only works with practically perfect reception.
    // As soon as individual seconds drop out - and that is exactly when it
    // matters - every dropout produces the same two-second gap as the genuine
    // marker. The decoder then synchronized to a wrong position, discarded it
    // again on the next telegram (fixed bits 0 / 20), synchronized wrongly
    // again - and the 59th second was never stably detected. Averaged over
    // frequency that problem disappears: random dropouts scatter across all 60
    // positions, the marker always hits the same one.
    //
    // Must be called from loop() (NOT from the ISR - see its flash-cache
    // warning at isr()), so array operations and decodeDcf77Telegram() can run
    // without restriction.

    void processDcf77Bits() {
#if defined DCF77_DATAPIN && defined DCF77_INTERRUPT
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

            // Rauschen/Kontaktprellen: eine derart kurze Flanke ignorieren,
            // OHNE prevEdgeMillis auf sie zu verschieben - so wird sie beim
            // naechsten, echten Flankenwechsel einfach "uebersprungen", statt
            // eine viel zu kurze Dauer zu erzeugen (siehe
            // DCF77_BIT_NOISE_IGNORE_MS in config.h).
            // Noise/contact bounce: ignore such a short edge WITHOUT moving
            // prevEdgeMillis onto it - that way it is simply "skipped over" at
            // the next genuine edge change instead of producing a far too
            // short duration (see DCF77_BIT_NOISE_IGNORE_MS in config.h).
            if (duration < DCF77_BIT_NOISE_IGNORE_MS) {
                continue;
            }

            unsigned long pulseStart = prevEdgeMillis;
            prevEdgeMillis = edgeMillis;

            // Langes Intervall = Pause bis zum naechsten Sekundenbeginn; wie
            // viele Sekunden vergangen sind, ergibt sich unten aus dem Abstand
            // der IMPULSANFAENGE und bleibt damit auch dann richtig, wenn
            // ganze Impulse fehlen.
            // Long interval = the rest of the second; how many seconds have
            // passed follows below from the distance between PULSE STARTS and
            // therefore stays correct even when whole pulses are missing.
            if (duration > DCF77_PULSE_MAX_MS) {
                continue;
            }

            // --- Ab hier: diese Flanke beendet einen Impuls ---
            // --- From here on: this edge ends a pulse ---
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
                    // Zwei Impulse innerhalb derselben Sekunde - das kann
                    // DCF77 nicht senden, also eine Stoerung. Verwerfen und
                    // dabei Raster UND Bezugszeitpunkt behalten, damit der
                    // naechste echte Impuls wieder den korrekten
                    // Sekundenabstand zum letzten echten Impuls hat.
                    // Two pulses within the same second - DCF77 cannot send
                    // that, so it is interference. Discard it while keeping
                    // both the grid AND the reference timestamp, so the next
                    // genuine pulse has the correct second distance to the
                    // last genuine pulse again.
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
                // Raster verloren (erster Impuls, zu lange Luecke oder ein
                // Abstand, der in kein Sekundenraster passt). Die gesammelte
                // Fehlstellen-Statistik zeigt danach auf Positionen, die es so
                // nicht mehr gibt - deshalb zuruecksetzen und mit diesem
                // Impuls eine neue Phase beginnen.
                // Grid lost (first pulse, too long a gap, or a distance
                // fitting no second grid). The missing-pulse statistics
                // collected so far then point at positions that no longer
                // exist - so reset them and start a new phase with this pulse.
                dcf77PhaseBreaks++;
                resetDcf77MarkerStats();
                dcf77Phase = 0;
                dcf77MarkerHit[0] = 1;

                // Ratenbegrenzt loggen: bei schlechtem Empfang kann das
                // mehrmals pro Minute vorkommen, und JEDE Logzeile schreibt
                // auf LittleFS - waehrend dieses Schreibvorgangs ist der
                // Flash-Cache aus und die dabei anfallenden DCF77-Flanken
                // gehen verloren. Ungebremstes Loggen wuerde den Empfang also
                // genau dann zusaetzlich verschlechtern, wenn er ohnehin
                // schon schlecht ist.
                // Rate-limited logging: with poor reception this can happen
                // several times per minute, and EVERY log line writes to
                // LittleFS - during that write the flash cache is off and the
                // DCF77 edges arriving meanwhile are lost. Unthrottled logging
                // would therefore worsen reception exactly when it is already
                // poor.
                if (millis() - lastGridLossLogMillis > WAIT_1m) {
                    lastGridLossLogMillis = millis();
                    DEBUG_PRINTLN("[DCF77] Second grid lost, resynchronizing (total: " +
                                  String(dcf77PhaseBreaks) + ")");
                }
                continue;
            }

            // Phase weiterschalten und die uebersprungenen Rasterpositionen
            // als Fehlstelle zaehlen - aber NUR bei kurzen Luecken.
            //
            // Bei einer langen Empfangspause ist an den uebersprungenen
            // Positionen nicht "ein Impuls ausgefallen", sondern es kam
            // schlicht gar nichts an. Wuerde man sie mitzaehlen, bekaeme jede
            // der 60 Positionen gleichmaessig Fehlstellen aufaddiert und die
            // Minutenmarke - die sich ja gerade dadurch abheben soll, dass NUR
            // sie immer fehlt - ginge im Rauschen unter.

            // Advance the phase and count the skipped grid positions as
            // missing - but ONLY for short gaps.
            //
            // During a long reception pause, the skipped positions did not
            // "lose a pulse", nothing arrived at all. Counting them would add
            // missing pulses evenly to all 60 positions, and the minute marker
            // - which is supposed to stand out precisely because ONLY it is
            // always missing - would drown in the noise.
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

            // Das Bit IMMER ablegen - dcf77Bits ist nach Rasterposition
            // indiziert und braucht die Minutenmarke dafuer nicht (siehe
            // globals.h). Vorher wurde erst nach dem Markenfund gesammelt,
            // wodurch die /dcf77-Seite in den ersten Minuten (und bei nie
            // gefundener Marke dauerhaft) keinerlei Fortschritt zeigte,
            // obwohl der Empfang lief.

            // ALWAYS store the bit - dcf77Bits is indexed by grid position and
            // does not need the minute marker for that (see globals.h).
            // Previously collecting only started after the marker was found,
            // which left the /dcf77 page without any progress during the first
            // minutes (and permanently if the marker was never found) even
            // though reception was running.
            // Solange die Minutenmarke unbekannt ist, ist auch der
            // Minutenwechsel unbekannt - geleert wird deshalb bei jedem
            // Umlauf des Rasters. Ohne das wuerden sich ueber die Minuten
            // hinweg Bits AUS VERSCHIEDENEN MINUTEN im Raster ansammeln: die
            // Anzeige liefe voll, obwohl gerade nichts ankommt, und das erste
            // Telegramm nach dem Markenfund waere aus mehreren Minuten
            // zusammengesetzt - mit im schlimmsten Fall in sich stimmigen
            // Werten einer alten Minute.

            // While the minute marker is unknown, the minute change is unknown
            // too - so the grid is cleared on every wrap. Without that, bits
            // FROM DIFFERENT MINUTES would accumulate in the grid over time:
            // the display would fill up even though nothing is currently
            // arriving, and the first telegram after the marker was found
            // would be assembled from several minutes - in the worst case with
            // self-consistent values of an old minute.
            if (dcf77MarkerPos < 0 && dcf77Phase <= phaseBefore) {
                for (uint8_t i = 0; i < DCF77_GRID_SLOTS; i++) dcf77Bits[i] = -1;
            }

            dcf77Bits[dcf77Phase] = bitValue;

            if (dcf77MarkerPos < 0) {
                evaluateDcf77Marker();

                if (dcf77MarkerPos >= 0) {
                    // Marke soeben gefunden: mit einem frischen Raster
                    // beginnen. Die bis hierher gesammelten Bits stammen aus
                    // der Suchphase und koennen aelter als die laufende Minute
                    // sein - sie duerfen nicht in das erste Telegramm
                    // einfliessen. dcf77LastSecond bleibt -1, damit der erste
                    // Minutenwechsel danach sauber erkannt wird.
                    // Marker just found: start with a fresh grid. The bits
                    // collected up to here come from the search phase and may
                    // be older than the current minute - they must not feed
                    // into the first telegram. dcf77LastSecond stays -1 so the
                    // first minute change afterwards is detected cleanly.
                    for (uint8_t i = 0; i < DCF77_GRID_SLOTS; i++) dcf77Bits[i] = -1;
                    dcf77Bits[dcf77Phase] = bitValue;
                    dcf77LastSecond = -1;
                }
                else {
                    // Position im Telegramm noch unbekannt: gesammelt wird
                    // trotzdem, nur die Zuordnung zu einer Sekunde fehlt noch.
                    // Der Fortschrittsbalken laeuft ueber die Rasterposition.
                    // Position within the telegram still unknown: collecting
                    // happens anyway, only the mapping to a second is still
                    // missing. The progress display runs on the grid position.
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

            // Minutenwechsel: die Sekundennummer ist kleiner als bei der
            // vorherigen Ablage, das Telegramm der abgelaufenen Minute ist
            // also vollstaendig. Als Zeitstempel der Minutenanfang - auch wenn
            // die Sekunde 0 selbst nicht empfangen wurde, ist er ueber die
            // aktuelle Sekundennummer exakt zurueckrechenbar.
            // Minute change: the second number is lower than at the previous
            // store, so the elapsed minute's telegram is complete. Timestamp:
            // the start of the minute - even when second 0 itself was not
            // received, it can be computed back exactly from the current
            // second number.
            if (dcf77LastSecond >= 0 && sec < (uint8_t)dcf77LastSecond) {
                unsigned long minuteStart = pulseStart - (unsigned long)sec * DCF77_SECOND_MS;

                // Das gerade abgelegte Bit gehoert schon zur NEUEN Minute -
                // vor dem Dekodieren kurz herausnehmen und danach wieder
                // eintragen, damit es weder im alten Telegramm mitzaehlt noch
                // beim anschliessenden Loeschen verlorengeht.
                // The bit just stored already belongs to the NEW minute - take
                // it out before decoding and put it back afterwards, so it
                // neither counts towards the old telegram nor gets lost in the
                // clearing that follows.
                dcf77Bits[dcf77Phase] = -1;

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
#endif
    }


    // Bindet den EIGENEN NTP-Server der Uhr an Port 123 (bzw. bindet ihn neu)
    // und meldet, ob das geklappt hat. Die Uhr kann damit anderen Geraeten im
    // Netz als Zeitquelle dienen; beantwortet werden die Anfragen in loop().
    //
    // Muss nach JEDEM Verbindungsaufbau erneut aufgerufen werden, nicht nur
    // einmal beim Booten: connectWiFi() faehrt den WLAN-Stack zwischendurch
    // komplett herunter (WiFi.mode(WIFI_MODE_NULL), siehe dort). Der in
    // setup() gebundene UDP-Socket verliert dabei sein Netzwerk-Interface und
    // empfaengt danach nichts mehr. Da udp.parsePacket() in loop() einfach
    // dauerhaft 0 liefert, faellt das nirgends auf - der NTP-Server der Uhr
    // war nach dem ersten Reconnect (z.B. Router-Neustart) stillschweigend
    // tot, bis zum naechsten Neustart der Uhr.
    //
    // udp.stop() davor, damit ein noch gebundener Socket sauber freigegeben
    // wird, statt beim erneuten begin() auf einem belegten Port zu scheitern.

    // Binds (or rebinds) the clock's OWN NTP server to port 123 and reports
    // whether that worked. This lets the clock serve as a time source for
    // other devices on the network; the requests are answered in loop().
    //
    // Must be called after EVERY connection setup, not just once at boot:
    // connectWiFi() shuts the WiFi stack down completely in between
    // (WiFi.mode(WIFI_MODE_NULL), see there). The UDP socket bound in setup()
    // loses its network interface in the process and receives nothing
    // afterwards. Since udp.parsePacket() in loop() simply keeps returning 0,
    // this goes unnoticed anywhere - the clock's NTP server was silently dead
    // after the first reconnect (e.g. a router restart) until the clock was
    // restarted.
    //
    // udp.stop() beforehand so a still-bound socket is released cleanly
    // instead of failing on an occupied port at the next begin().

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


    // Baut aus dem empfangenen Anfragepaket die NTP-Antwort (RFC 5905, Mode 4)
    // - IM SELBEN Puffer, den die Anfrage benutzt hat (siehe Aufrufstelle in
    // loop()). 'receivedAt' ist der Zeitpunkt, zu dem die Anfrage eintraf.

    // Builds the NTP reply (RFC 5905, mode 4) from the received request packet
    // - IN THE SAME buffer the request used (see the call site in loop()).
    // 'receivedAt' is the instant at which the request arrived.

    void createNtpResponse(byte* packet, const struct timeval& receivedAt) {

        // Originate Timestamp: der Transmit-Timestamp der ANFRAGE (Byte 40-47)
        // muss unveraendert in Byte 24-31 der Antwort zurueckgespiegelt werden.
        // RFC-konforme Clients (ntpd, chrony, systemd-timesyncd, w32tm)
        // vergleichen dieses Feld mit dem Zeitstempel, den sie selbst gesendet
        // haben, und verwerfen die Antwort sonst als "bogus packet". Muss VOR
        // dem memset gesichert werden, da Anfrage und Antwort denselben Puffer
        // benutzen.

        // Originate Timestamp: the REQUEST's transmit timestamp (bytes 40-47)
        // has to be mirrored back unchanged into bytes 24-31 of the reply.
        // RFC-compliant clients (ntpd, chrony, systemd-timesyncd, w32tm)
        // compare this field against the timestamp they sent themselves and
        // otherwise discard the reply as a "bogus packet". Has to be saved
        // BEFORE the memset, since request and reply share the same buffer.
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

        // Sekundenbruchteile als 32-Bit-Bruch (Einheit: 1/2^32 Sekunde)
        // mitliefern. Blieben sie 0, waere jede Antwort auf die volle Sekunde
        // gerundet - der Client haette systematisch bis zu einer Sekunde
        // Fehler, obwohl die Uhr die Zeit deutlich genauer kennt.

        // Provide the fractional seconds as a 32-bit fraction (unit: 1/2^32 of
        // a second). If they stayed 0, every reply would be rounded to the full
        // second - the client would carry a systematic error of up to one
        // second, even though the clock knows the time far more precisely.
        auto writeTimestamp = [&](uint8_t offset, const struct timeval& tv) {
            uint32_t seconds = htonl((uint32_t)(tv.tv_sec + NTP_UNIX_OFFSET));
            uint32_t fraction = htonl((uint32_t)(((uint64_t)tv.tv_usec << 32) / 1000000ULL));
            memcpy(&packet[offset], &seconds, 4);
            memcpy(&packet[offset + 4], &fraction, 4);
        };

        // Reference Timestamp: Zeitpunkt, zu dem die eigene Uhr zuletzt
        // gestellt wurde - hier vereinfacht der Empfangszeitpunkt minus einer
        // Sekunde.
        // Reference timestamp: when the own clock was last set - simplified
        // here to the receive instant minus one second.
        struct timeval referenceTime = receivedAt;
        referenceTime.tv_sec -= 1;
        writeTimestamp(16, referenceTime);

        // Originate Timestamp: gespiegelter Transmit-Timestamp der Anfrage.
        // Originate timestamp: mirrored transmit timestamp of the request.
        memcpy(&packet[24], originateTimestamp, sizeof(originateTimestamp));

        // Receive Timestamp: Eintreffen der Anfrage.
        // Receive timestamp: when the request arrived.
        writeTimestamp(32, receivedAt);

        // Transmit Timestamp: JETZT, unmittelbar vor dem Senden - nicht der
        // Empfangszeitpunkt. Der Client bildet aus Receive und Transmit die
        // Bearbeitungszeit des Servers und rechnet sie aus der Laufzeit heraus;
        // beide gleich zu setzen unterschlaegt diese Zeit.
        // Transmit timestamp: NOW, immediately before sending - not the receive
        // instant. The client derives the server's processing time from receive
        // and transmit and removes it from the round-trip delay; setting both to
        // the same value hides that time.
        struct timeval transmitTime;
        gettimeofday(&transmitTime, nullptr);
        writeTimestamp(40, transmitTime);
    }
