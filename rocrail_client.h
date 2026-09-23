#pragma once
    // Rocrail-Modellzeit: verbindet sich mit einem konfigurierten Rocrail-
    // Server (TCP, RCP-Protokoll) und uebernimmt dessen Fast-Clock als
    // Zeitquelle fuer die Zeiger.

    // Rocrail model time: connects to a configured Rocrail server (TCP,
    // RCP protocol) and adopts its fast clock as the time source for the
    // hands.

    // connect() mit Timeout ist zugleich der "Ping" (prueft Port-, nicht nur
    // Host-Ebene) und laeuft in einer eigenen FreeRTOS-Task, damit loop()
    // dabei nicht blockiert - siehe connectRocrailClient().

    // connect() with a timeout is also the "ping" (checks port level, not
    // just host level) and runs in its own FreeRTOS task so loop() doesn't
    // block - see connectRocrailClient().

    // <clock>-Events kommen ungerahmt im Rohstrom an, nicht wie dokumentiert
    // per <xmlh>-Header - processRocrailBuffer() sucht das Tag daher per
    // Substring-Scan statt per Header-Parser.

    // <clock> events arrive unframed in the raw stream, not wrapped in the
    // documented <xmlh> header - processRocrailBuffer() therefore finds
    // the tag via a substring scan instead of a header parser.

    // R2RNet-Discovery (automatische Server-Suche) wurde entfernt: das
    // Antwortformat war nirgends spezifiziert und funktionierte nicht
    // zuverlaessig (siehe Git-Historie).

    // R2RNet discovery (automatic server search) was removed: the reply
    // format was never specified and didn't work reliably (see git
    // history).


    // Diagnose-Funktion: tritt der R2RNet-Multicast-Gruppe bei und loggt jedes
    // empfangene Paket unveraendert (Rohtext + Hex) - um zu klaeren, ob und in
    // welchem Format ueberhaupt etwas ankommt, BEVOR echte Discovery evtl.
    // wieder aufgebaut wird. Muss nach jedem WiFi-(Re-)Connect neu aufgerufen
    // werden (siehe Aufrufstellen in uhr3.ino/wifi_manager.h), genau wie
    // startNtpServer() - ein Reconnect reisst den Socket sonst mit runter.

    // Diagnostic function: joins the R2RNet multicast group and logs every
    // received packet unchanged (raw text + hex) - to find out whether and in
    // what format anything arrives at all, BEFORE real discovery might get
    // rebuilt. Must be called again after every WiFi (re)connect (see the
    // call sites in uhr3.ino/wifi_manager.h), exactly like startNtpServer() -
    // a reconnect otherwise takes the socket down with it.

    bool startR2rnetDebugListener() {
        r2rnetDebugUdp.stop();

        IPAddress multicastIp;
        if (!multicastIp.fromString(R2RNET_DEBUG_MULTICAST_IP)) {
            DEBUG_PRINTLN("[R2RNET-DEBUG] Invalid multicast address: " + String(R2RNET_DEBUG_MULTICAST_IP));
            r2rnetDebugListening = false;
            return false;
        }

        r2rnetDebugListening = (r2rnetDebugUdp.beginMulticast(multicastIp, R2RNET_DEBUG_MULTICAST_PORT) == 1);

        if (r2rnetDebugListening) {
            DEBUG_PRINTLN("[R2RNET-DEBUG] Listening on multicast " + String(R2RNET_DEBUG_MULTICAST_IP) + ":" + String(R2RNET_DEBUG_MULTICAST_PORT));
        }
        else {
            // Zusatzdaten fuers Debugging (WiFi-Modus/-Status, freier Heap) -
            // der ESP-IDF-Fehlercode (errno) landet nur auf dem rohen
            // Serial-Ausgang, nicht hier (siehe log_e() in NetworkUdp.cpp).

            // Extra data for debugging (WiFi mode/status, free heap) - the
            // ESP-IDF error code (errno) only reaches the raw serial output,
            // not here (see log_e() in NetworkUdp.cpp).
            DEBUG_PRINTLN("[R2RNET-DEBUG] Could not join multicast group " + String(R2RNET_DEBUG_MULTICAST_IP) + ":" + String(R2RNET_DEBUG_MULTICAST_PORT) +
                          " (WiFi mode " + String(WiFi.getMode()) + ", status " + String(WiFi.status()) +
                          ", free heap " + String(ESP.getFreeHeap()) + ")");
        }
        return r2rnetDebugListening;
    }


    // In loop() gepollt (siehe uhr3.ino) - liest eingehende Multicast-Pakete
    // non-blocking (parsePacket() liefert 0, wenn keins wartet) und loggt
    // Absender, Laenge, Rohtext sowie eine Hex-Ansicht der ersten Bytes -
    // Rohtext allein reicht bei evtl. binaeren Headern/Nicht-ASCII nicht aus.

    // Polled in loop() (see uhr3.ino) - reads incoming multicast packets
    // non-blocking (parsePacket() returns 0 when none is waiting) and logs
    // sender, length, raw text, and a hex view of the first bytes - raw text
    // alone isn't enough if there are binary headers/non-ASCII bytes.

    void pollR2rnetDebugListener() {
        if (!r2rnetDebugListening) return;

        int packetSize = r2rnetDebugUdp.parsePacket();
        if (packetSize <= 0) return;

        uint8_t buf[R2RNET_DEBUG_PACKET_BUFFER_SIZE];
        int len = r2rnetDebugUdp.read(buf, sizeof(buf) - 1);
        if (len < 0) len = 0;
        buf[len] = '\0';

        // Hex-Ansicht der ersten Bytes, damit auch nicht-druckbare/binaere
        // Anteile sichtbar werden statt im Log als Muell/leer zu erscheinen.

        // Hex view of the first bytes, so non-printable/binary portions show
        // up too instead of appearing as garbage/blank in the log.
        String hex;
        int hexBytes = min(len, 64);
        hex.reserve(hexBytes * 3 + 4); // "XX " je Byte + evtl. "..." - vermeidet
                                       // wiederholte Heap-Reallokation durch +=

                                       // "XX " per byte + optional "..." - avoids
                                       // repeated heap reallocation from +=
        for (int i = 0; i < hexBytes; i++) {
            if (buf[i] < 0x10) hex += "0";
            hex += String(buf[i], HEX);
            hex += " ";
        }
        if (len > hexBytes) hex += "...";

        DEBUG_PRINTLN("[R2RNET-DEBUG] " + String(packetSize) + " bytes from " +
            r2rnetDebugUdp.remoteIP().toString() + ":" + String(r2rnetDebugUdp.remotePort()) +
            " | text: " + String((char*)buf) + " | hex: " + hex);
    }


    // Laedt die Rocrail-Server-Liste (bis zu MAX_WLAN) aus den Preferences.
    // Migriert einmalig einen alten Einzel-Server als Listenplatz 1, falls
    // die Liste sonst leer waere.

    // Loads the Rocrail server list (up to MAX_WLAN) from preferences.
    // Migrates an old single server into list slot 1 once, if the list
    // would otherwise be empty.

    void loadRocrailServerList() {
        bool anyFound = false;
        for (int i = 0; i < MAX_WLAN; i++) {
            String host = preferences.getString(pkRocrailServerHost(i).c_str(), "");
            strncpy(rocrailServerList[i], host.c_str(), sizeof(rocrailServerList[i]) - 1);
            rocrailServerList[i][sizeof(rocrailServerList[i]) - 1] = '\0';
            rocrailServerPortList[i] = preferences.getUShort(pkRocrailServerPort(i).c_str(), ROCRAIL_DEFAULT_PORT);

            String name = preferences.getString(pkRocrailServerName(i).c_str(), "");
            strncpy(rocrailServerNameList[i], name.c_str(), sizeof(rocrailServerNameList[i]) - 1);
            rocrailServerNameList[i][sizeof(rocrailServerNameList[i]) - 1] = '\0';

            if (host.length() > 0) anyFound = true;
        }
        rocrailActiveServerIndex = preferences.getInt(PK_ROCRAIL_ACTIVE_SRV, -1);
        if (rocrailActiveServerIndex < -1 || rocrailActiveServerIndex >= MAX_WLAN) rocrailActiveServerIndex = -1;

        // Keiner angehakt, aber mindestens ein Server vorhanden - den
        // ersten befuellten Listenplatz nehmen statt Rocrail stillschweigend inaktiv zu lassen.

        // None checked but at least one server exists - use the first
        // filled list slot instead of silently leaving Rocrail inactive.
        if (rocrailActiveServerIndex < 0 && anyFound) {
            for (int i = 0; i < MAX_WLAN; i++) {
                if (strlen(rocrailServerList[i]) > 0) { rocrailActiveServerIndex = i; break; }
            }
            preferences.putInt(PK_ROCRAIL_ACTIVE_SRV, rocrailActiveServerIndex);
        }


        if (!anyFound && rocrailServerHost.length() > 0) {
            strncpy(rocrailServerList[0], rocrailServerHost.c_str(), sizeof(rocrailServerList[0]) - 1);
            rocrailServerList[0][sizeof(rocrailServerList[0]) - 1] = '\0';
            rocrailServerPortList[0] = rocrailServerPort;
            rocrailActiveServerIndex = 0;
            preferences.putString(pkRocrailServerHost(0).c_str(), rocrailServerList[0]);
            preferences.putUShort(pkRocrailServerPort(0).c_str(), rocrailServerPortList[0]);
            preferences.putInt(PK_ROCRAIL_ACTIVE_SRV, 0);
            DEBUG_PRINTLN("[ROCRAIL] Migrated existing single server into list slot 1: " + rocrailServerHost);
        }

        // rocrailServerHost/-Port an den aufgeloesten Index angleichen - noetig,
        // falls der Fallback oben gerade erst einen Index bestimmt hat. setup()
        // (uhr3.ino) ruft danach triggerRocrailConnectNow() auf und verbindet sich so noch beim Start.

        // Align rocrailServerHost/-Port with the resolved index - needed if
        // the fallback above just determined an index. setup() (uhr3.ino)
        // calls triggerRocrailConnectNow() afterwards, so it connects right at boot.
        if (rocrailActiveServerIndex >= 0) {
            rocrailServerHost = String(rocrailServerList[rocrailActiveServerIndex]);
            rocrailServerPort = rocrailServerPortList[rocrailActiveServerIndex];
            preferences.putString(PK_ROCRAIL_SERVER, rocrailServerHost);
            preferences.putUShort(PK_ROCRAIL_SRV_PORT, rocrailServerPort);
        }
    }


    // Liest ein Attribut attr="wert" aus einem XML-Tag. fallback bei
    // fehlendem oder nicht-numerischem Wert.

    // Reads an attr="value" attribute from an XML tag. fallback if missing
    // or not numeric.

    int rocrailXmlAttrInt(const String& tag, const char* attr, int fallback) {
        String needle = String(attr) + "=\"";
        int pos = tag.indexOf(needle);
        if (pos == -1) return fallback;
        pos += needle.length();
        int endPos = tag.indexOf('"', pos);
        if (endPos == -1) return fallback;
        String value = tag.substring(pos, endPos);
        if (value.isEmpty()) return fallback;
        return value.toInt();
    }


    // Wie rocrailXmlAttrInt(), aber fuer Strings. Leerer String bei
    // fehlendem Attribut.

    // Like rocrailXmlAttrInt(), but for strings. Empty string if the
    // attribute is missing.

    String rocrailXmlAttrString(const String& tag, const char* attr) {
        String needle = String(attr) + "=\"";
        int pos = tag.indexOf(needle);
        if (pos == -1) return "";
        pos += needle.length();
        int endPos = tag.indexOf('"', pos);
        if (endPos == -1) return "";
        return tag.substring(pos, endPos);
    }


    // Schreibt die Modellzeit unabhaengig von der echten Zeit fort
    // (Divider-Geschwindigkeit) und gleicht eine gemeldete Abweichung
    // sanft statt schlagartig aus. Schreibt nicht in "timeinfo".

    // Advances the model time independent of the real time (divider
    // speed) and eases in a reported deviation smoothly instead of
    // abruptly. Does not write to "timeinfo".

    void advanceRocrailTime() {
        if (!rocrailEnabled) return;
        if (rocrailLastClockMillis == 0) return; // noch kein erstes <clock>-Update erhalten
                                                  // no first <clock> update received yet

        unsigned long nowMillis = millis();
        float elapsedRealSec = (nowMillis - rocrailLastAdvanceMillis) / 1000.0f;
        rocrailLastAdvanceMillis = nowMillis;

        if (rocrailFrozen) return; // angehalten - Zeitstempel oben bleibt trotzdem aktuell
                                   // stopped - timestamp above still stays current

        rocrailDisplaySeconds += elapsedRealSec * rocrailDivider;

        // Nie mehr ausgleichen als noch aussteht (kein Ueberschwingen bei
        // grossem elapsedRealSec, z.B. nach einem verzoegerten Frame).

        // Never correct more than what's outstanding (no overshoot with a
        // large elapsedRealSec, e.g. after a delayed frame).
        if (rocrailDriftSeconds != 0.0f) {
            float correction = rocrailDriftSeconds * ROCRAIL_DRIFT_CORRECTION_RATE * elapsedRealSec;
            if (fabsf(correction) > fabsf(rocrailDriftSeconds)) correction = rocrailDriftSeconds;
            rocrailDisplaySeconds += correction;
            rocrailDriftSeconds -= correction;
        }

        // In den 24h-Bereich falten.
        // Fold into the 24h range.
        while (rocrailDisplaySeconds >= 86400.0f) rocrailDisplaySeconds -= 86400.0f;
        while (rocrailDisplaySeconds < 0.0f) rocrailDisplaySeconds += 86400.0f;

        long wholeSeconds = (long)rocrailDisplaySeconds;
        rocrailTimeinfo.tm_hour = wholeSeconds / 3600;
        rocrailTimeinfo.tm_min = (wholeSeconds % 3600) / 60;
        rocrailTimeinfo.tm_sec = wholeSeconds % 60;

        // Sekundenbruchteil fuer die glatte Zeigerbewegung (siehe renderClockFrame()).
        // Fractional second for the smooth hand motion (see renderClockFrame()).
        rocrailSecFrac = fmodf(rocrailDisplaySeconds, 60.0f);
    }


    // Sucht das letzte vollstaendige <clock .../>-Tag im Rohstrom (kein
    // <xmlh>-Header, siehe Dateikopf) und wertet es aus.

    // Finds the last complete <clock .../> tag in the raw stream (no
    // <xmlh> header, see file header) and evaluates it.

    void processRocrailBuffer() {
        int clockPos = rocrailRxBuffer.lastIndexOf("<clock ");
        if (clockPos == -1) return;

        int endPos = rocrailRxBuffer.indexOf("/>", clockPos);
        if (endPos == -1) return; // Tag noch nicht vollstaendig angekommen
                                  // tag hasn't fully arrived yet

        processRocrailClockPayload(rocrailRxBuffer.substring(clockPos, endPos + 2));

        // Verarbeiteten Teil verwerfen, Rest bleibt fuer die naechste Runde.
        // Discard the processed part, keep the rest for the next round.
        rocrailRxBuffer = rocrailRxBuffer.substring(endPos + 2);
    }


    // Wertet ein <clock .../>-Tag aus: state ("go"/"freeze") gehoert zum
    // Clock-Objekt. Das "time"-Attribut ist laut Rocrail-Wiki (digint:user-en,
    // Beispiel "time="1559803151"" passend zu "year="2019" month="6" mday="6"")
    // ein REALER Unix-Zeitstempel des Sendezeitpunkts, KEINE Modellzeit-
    // Sekunde - time%60 lieferte daher einen mit hour/minute komplett
    // unkorrelierten Wert (frueherer Bug, siehe Git-Historie: loeste sehr
    // haeufig faelschlich "Large drift"/Snapping aus). Rocrails <clock>-
    // Updates sind ohnehin minutengenau ("Client update frequency in model
    // minutes", siehe rocrailini-service-Doku) - jedes Update markiert daher
    // den Beginn einer neuen Modellminute (Sekunde 0); dazwischen laeuft die
    // Anzeige rein lokal weiter (siehe advanceRocrailTime()). Eine Abweichung
    // wird sanft angeglichen statt gesprungen.

    // Evaluates a <clock .../> tag: state ("go"/"freeze") belongs to the
    // clock object. Per the Rocrail wiki (digint:user-en, example
    // "time="1559803151"" matching "year="2019" month="6" mday="6""), the
    // "time" attribute is a REAL Unix timestamp of when it was sent, NOT a
    // model-time second - time%60 therefore produced a value completely
    // uncorrelated with hour/minute (earlier bug, see git history: falsely
    // triggered "Large drift"/snapping very often). Rocrail's <clock>
    // updates are minute-granular anyway ("Client update frequency in model
    // minutes", see the rocrailini-service docs) - each update therefore
    // marks the start of a new model minute (second 0); the display keeps
    // running purely locally in between (see advanceRocrailTime()). A
    // deviation is eased in smoothly instead of snapped.

    // Drosselung fuer wiederkehrende Logs: loggt nur die ersten `limit`
    // Aufrufe (erhoeht `counter`); `isLastLogged` markiert den letzten
    // geloggten Aufruf fuer einen "wird jetzt still"-Hinweis.

    // Throttles recurring logs: only logs the first `limit` calls
    // (increments `counter`); `isLastLogged` marks the last logged call
    // so the caller can note the log going quiet.

    bool shouldLogThrottled(uint8_t& counter, bool& isLastLogged, uint8_t limit) {
        isLastLogged = false;
        if (counter >= limit) return false;
        counter++;
        isLastLogged = (counter == limit);
        return true;
    }


    void processRocrailClockPayload(const String& payload) {
        int hour = rocrailXmlAttrInt(payload, "hour", -1);
        int minute = rocrailXmlAttrInt(payload, "minute", -1);
        int divider = rocrailXmlAttrInt(payload, "divider", rocrailDivider);
        String state = rocrailXmlAttrString(payload, "state");

        // Helligkeit: optionales "bri"-Attribut (0-255), nur gesetzt bei
        // serverseitiger Tag/Nacht-Lichtsteuerung. Fallback -1 = nicht
        // enthalten, dann bleibt der alte Wert statt auf 0 zu fallen.

        // Brightness: optional "bri" attribute (0-255), only set with
        // server-side day/night lighting control. Fallback -1 = not
        // present, then the old value stays instead of falling to 0.
        int bri = rocrailXmlAttrInt(payload, "bri", -1);
        if (bri >= 0) {
            rocrailBrightness = (uint8_t)constrain(bri, 0, 255);
            rocrailBrightnessKnown = true;
        }

        // Frueheren Zustand fuer den Vergleich unten sichern, BEVOR er durch
        // das aktuelle Update ueberschrieben wird (state -> rocrailFrozen,
        // divider -> rocrailDivider) - noetig, um einen echten Wechsel von
        // einem blossen "unveraendert"-Update zu unterscheiden.

        // Save the previous state for the comparison below, BEFORE this
        // update overwrites it (state -> rocrailFrozen, divider ->
        // rocrailDivider) - needed to tell a genuine change apart from a
        // plain "unchanged" update.
        bool wasFrozen = rocrailFrozen;
        uint8_t oldDivider = rocrailDivider;

        if (state == "go") rocrailFrozen = false;
        else if (state == "freeze") rocrailFrozen = true;

        if (hour >= 0 && hour < 24 && minute >= 0 && minute < 60) {
            long newModelSeconds = (long)hour * 3600 + (long)minute * 60;

            bool firstSync = (rocrailLastClockMillis == 0);

            if (firstSync) {
                rocrailDisplaySeconds = (float)newModelSeconds;
                rocrailDriftSeconds = 0.0f;
                rocrailLastAdvanceMillis = millis();
            }
            else {
                float drift = (float)newModelSeconds - rocrailDisplaySeconds;
                // Tagesgrenze beruecksichtigen (23:59:58->00:00:02 = +4s, nicht -86396s).
                // Account for the day boundary (23:59:58->00:00:02 = +4s, not -86396s).
                if (drift > 43200.0f) drift -= 86400.0f;
                else if (drift < -43200.0f) drift += 86400.0f;

                if (fabsf(drift) > ROCRAIL_DRIFT_SNAP_THRESHOLD_SECONDS) {
                    // Zu gross fuer sanftes Angleichen - direkt setzen.
                    // Too large for a smooth ease-in - set directly.
                    rocrailDisplaySeconds = (float)newModelSeconds;
                    rocrailDriftSeconds = 0.0f;
                    DEBUG_PRINTLN("[ROCRAIL] " + rocrailServerHost + ":" + String(rocrailServerPort) +
                                  " - large drift (" + String(drift, 1) + "s) - snapping directly");
                }
                else {
                    rocrailDriftSeconds = drift;
                }
            }

            rocrailLastClockMillis = millis();
            if (divider > 0) rocrailDivider = (uint8_t)divider;

            // Die routinemaessige Zeile nur fuer die ersten beiden <clock>-
            // Updates zeigen (Bestaetigung, dass die Verbindung/Zeituebernahme
            // funktioniert) - danach wuerde sie das Logfile bei jedem Update
            // (minuetlich x Divider) zulaufen lassen, ohne neue Information zu
            // liefern. Ab dann nur noch bei echten Aenderungen (Divider- oder
            // Pause/Lauf-Wechsel) melden - der grosse-Drift-Fall oben bleibt
            // davon unberuehrt und wird immer geloggt. Host:Port stehen mit in
            // der Zeile (siehe precise-log-messages-Konvention), da bei
            // mehreren konfigurierten Servern sonst nicht erkennbar waere,
            // von welchem die Meldung stammt.

            // Only show the routine line for the first two <clock> updates
            // (confirmation that the connection/time takeover works) - after
            // that it would fill the log file on every update (per model
            // minute x divider) without new information. From then on only
            // report genuine changes (divider or pause/run switch) - the
            // large-drift case above is unaffected and always logged. Host:
            // port are included in the line (see the precise-log-messages
            // convention), since with multiple configured servers it
            // otherwise wouldn't be clear which one the message is about.
            bool isLastLogged = false;
            if (shouldLogThrottled(rocrailClockLogCount, isLastLogged)) {

                // Beim letzten der beiden Routine-Updates einen Hinweis
                // anhaengen, WARUM danach Stille im Log herrscht - sonst liesse
                // sich das leicht mit einer abgebrochenen Verbindung verwechseln.

                // On the last of the two routine updates, append a note
                // explaining WHY the log goes quiet afterwards - otherwise
                // that could easily be mistaken for a dropped connection.
                String suffix = isLastLogged ?
                    " - Rocrail ok, no more messages until divider/pause changes" : "";

                DEBUG_PRINTLN("[ROCRAIL] " + rocrailServerHost + ":" + String(rocrailServerPort) +
                              " - clock " + String(hour) + ":" +
                              (minute < 10 ? "0" : "") + String(minute) +
                              " (divider " + String(rocrailDivider) +
                              (rocrailFrozen ? ", angehalten)" : ")") + suffix);
            }
            else if (rocrailFrozen != wasFrozen || rocrailDivider != oldDivider) {
                DEBUG_PRINTLN("[ROCRAIL] " + rocrailServerHost + ":" + String(rocrailServerPort) +
                              " - clock " + String(hour) + ":" +
                              (minute < 10 ? "0" : "") + String(minute) +
                              " - " +
                              (rocrailFrozen != wasFrozen ? String(rocrailFrozen ? "paused" : "running again") : "") +
                              (rocrailFrozen != wasFrozen && rocrailDivider != oldDivider ? ", " : "") +
                              (rocrailDivider != oldDivider ? "divider changed " + String(oldDivider) + " -> " + String(rocrailDivider) : ""));
            }
        }
    }


    // Eigene FreeRTOS-Task fuer den (weiterhin blockierenden) connect(),
    // damit loop() nicht blockiert. Loescht sich am Ende selbst.

    // Own FreeRTOS task for the (still blocking) connect(), so loop()
    // doesn't block. Deletes itself at the end.

    void rocrailConnectTaskFunc(void* param) {
        rocrailConnectTaskResult = rocrailClient.connect(rocrailServerHostSnapshot, rocrailServerPortSnapshot, ROCRAIL_CONNECT_TIMEOUT_MS);
        rocrailConnectTaskDone = true;
        vTaskDelete(NULL);
    }


    // Wertet eine beendete Connect-Task aus - auch aufgerufen, wenn Rocrail
    // zwischenzeitlich deaktiviert wurde (raeumt dann nur auf).

    // Evaluates a finished connect task - also called if Rocrail was
    // disabled in the meantime (just cleans up then).

    void pollRocrailConnectTask() {
        if (!rocrailConnectTaskDone) return;

        rocrailConnectTaskRunning = false;
        rocrailConnectTaskHandle = NULL;

        if (rocrailConnectTaskResult && rocrailEnabled) {
            DEBUG_PRINTLN("[ROCRAIL] Connected to " + String(rocrailServerHostSnapshot) + ":" + String(rocrailServerPortSnapshot));
            rocrailRxBuffer = "";
            rocrailFrozen = false; // Zustand unbekannt bis zum naechsten state-Attribut
                                   // state unknown until the next state attribute
            // Naechstes <clock>-Update startet die Modellzeit neu.
            // Next <clock> update restarts the model time.
            rocrailLastClockMillis = 0;

            // Beide Zaehler zuruecksetzen: eine neue Verbindung ist ein
            // Neuanfang - Fehlschlaege und die ersten <clock>-Updates sollen
            // danach wieder kurz geloggt werden, statt dauerhaft stumm zu bleiben.

            // Reset both counters: a new connection is a fresh start -
            // failures and the first <clock> updates should briefly log
            // again afterward, instead of staying silent forever.
            rocrailConnectFailLogCount = 0;
            rocrailClockLogCount = 0;
        }
        else if (!rocrailConnectTaskResult) {

            // Wie bei der Clock-Routinezeile (siehe processRocrailClockPayload()):
            // nur die ersten 2 Fehlschlaege loggen, danach im Hintergrund
            // weiter versuchen, aber das Log nicht mit derselben Meldung jede
            // Minute zuschreiben - mit Hinweis bei der letzten geloggten Zeile,
            // damit das nicht wie eine tote Verbindung ohne jede Rueckmeldung wirkt.

            // Same as the clock routine line (see processRocrailClockPayload()):
            // only log the first 2 failures, then keep retrying in the
            // background without filling the log with the same message every
            // minute - with a hint on the last logged line, so it doesn't look
            // like a dead connection with no feedback at all.
            bool isLastLogged = false;
            if (shouldLogThrottled(rocrailConnectFailLogCount, isLastLogged)) {
                String suffix = isLastLogged ?
                    " - retrying in background, no more messages until it succeeds" : "";
                DEBUG_PRINTLN("[ROCRAIL] Server " + String(rocrailServerHostSnapshot) + ":" + String(rocrailServerPortSnapshot) +
                              " did not respond - falling back to real time, retrying in a minute" + suffix);
            }
        }
    }


    // Gemeinsamer Kern von connectRocrailClient() (periodisch) und
    // triggerRocrailConnectNow() (sofort) - startet die Connect-Task.
    // Setzt voraus, dass keine Task laeuft und keine Verbindung besteht.

    // Shared core of connectRocrailClient() (periodic) and
    // triggerRocrailConnectNow() (immediate) - starts the connect task.
    // Assumes no task is running and no connection is open.

    void startRocrailConnectTask() {
        rocrailLastConnectAttemptMillis = millis();

        // Snapshot JETZT anlegen, auf dem Hauptthread - siehe Kommentar bei
        // rocrailServerHostSnapshot in globals.h. Ab hier liest die Task nur
        // noch aus der Kopie, unabhaengig davon, was am Original waehrend
        // die Task laeuft noch veraendert wird (z.B. Speichern neuer
        // Rocrail-Einstellungen im Webinterface).

        // Take the snapshot NOW, on the main thread - see the comment at
        // rocrailServerHostSnapshot in globals.h. From here on the task only
        // reads the copy, regardless of what happens to the original while
        // the task is running (e.g. saving new Rocrail settings in the web UI).
        rocrailServerHost.toCharArray(rocrailServerHostSnapshot, sizeof(rocrailServerHostSnapshot));
        rocrailServerPortSnapshot = rocrailServerPort;

        rocrailConnectTaskDone = false;
        rocrailConnectTaskRunning = true;
        xTaskCreate(rocrailConnectTaskFunc, "rocrailConnect", 4096, NULL, 1, &rocrailConnectTaskHandle);
    }


    // Startet die Connect-Task (Timeout = "Ping", siehe Dateikopf) einmal
    // pro Minute, ausgeloest bei Sekunde 59 (Zeiger dort bereits in Ruhe).

    // Starts the connect task (timeout = "ping", see file header) once a
    // minute, triggered at second 59 (hand already at rest there).

    void connectRocrailClient() {
        if (rocrailConnectTaskRunning) {
            pollRocrailConnectTask();
            return;
        }

        if (millis() - rocrailLastConnectAttemptMillis < ROCRAIL_RECONNECT_INTERVAL_MS) return;

        // Sekunde 59: im Bahnhofsuhr-Modus ruht der Zeiger dort schon seit
        // ~58,5s. timeinfo statt rocrailTimeinfo, da bis zur Verbindung
        // ohnehin die echte Zeit angezeigt wird.

        // Second 59: in station-clock mode the hand has been resting there
        // since ~58.5s. timeinfo instead of rocrailTimeinfo, since the real
        // time is shown anyway until connected.
        if (timeinfo.tm_sec != 59) return;

        // Gleiche Drosselung wie bei der Fehlschlag-Meldung unten
        // (pollRocrailConnectTask()): sonst wuerde diese Zeile weiter jede
        // Minute geloggt, auch wenn die Fehlschlag-Zeile schon still ist.

        // Same throttling as the failure message below
        // (pollRocrailConnectTask()): otherwise this line would keep getting
        // logged every minute even while the failure line has already gone quiet.
        if (rocrailConnectFailLogCount < ROCRAIL_LOG_THROTTLE_LIMIT) {
            DEBUG_PRINTLN("[ROCRAIL] Connecting to " + rocrailServerHost + ":" + String(rocrailServerPort));
        }
        startRocrailConnectTask();
    }


    // Stoesst sofort einen Verbindungsversuch an, statt auf das reguläre
    // Zeitfenster zu warten - aufgerufen nach Aktivieren/Speichern im
    // Webinterface. No-op ohne Server oder bei laufender Verbindung/Task.

    // Immediately kicks off a connection attempt, instead of waiting for
    // the regular window - called after enabling/saving in the web
    // interface. No-op without a server, or an existing connection/task.

    void triggerRocrailConnectNow() {
        if (!rocrailEnabled || rocrailServerHost.isEmpty()) return;
        if (rocrailConnectTaskRunning || rocrailClient.connected()) return;

        DEBUG_PRINTLN("[ROCRAIL] Immediate connect attempt (settings saved) to " + rocrailServerHost + ":" + String(rocrailServerPort));
        startRocrailConnectTask();
    }


    // Nicht-blockierender Poll fuer loop(): haelt die Verbindung, liest
    // Daten in Haeppchen statt in einer While-Schleife.

    // Non-blocking poll for loop(): maintains the connection, reads data
    // in chunks instead of a while loop.

    void pollRocrailClient() {
        // Laufende Connect-Task immer abfragen/aufraeumen, auch deaktiviert.
        // Always poll/clean up a running connect task, even if disabled.
        if (rocrailConnectTaskRunning) {
            pollRocrailConnectTask();
        }

        if (!rocrailEnabled) {
            if (rocrailClient.connected()) {
                rocrailClient.stop();
                DEBUG_PRINTLN("[ROCRAIL] Disabled - connection closed");
            }
            rocrailConnected = false;
            return;
        }

        if (rocrailServerHost.isEmpty()) return; // noch kein Server bekannt/gefunden
                                                  // no server known/found yet

        // Waehrend die Connect-Task laeuft, rocrailClient nicht anfassen
        // (WiFiClient ist nicht nebenlaeufig-sicher).

        // While the connect task is running, don't touch rocrailClient
        // (WiFiClient isn't safe for concurrent access).
        if (rocrailConnectTaskRunning) return;

        if (!rocrailClient.connected()) {
            if (rocrailConnected) DEBUG_PRINTLN("[ROCRAIL] Connection lost");
            rocrailConnected = false;
            connectRocrailClient();
            return;
        }

        rocrailConnected = true;

        if (rocrailClient.available()) {
            char buf[129];
            int n = rocrailClient.read((uint8_t*)buf, sizeof(buf) - 1);
            if (n > 0) {
                buf[n] = '\0';

                // Diagnose: nur die ersten 5 Rohdaten-Haeppchen loggen, um
                // das Logfile nicht mit haeufigen Statuszeilen zulaufen zu
                // lassen.

                // Diagnostic: only log the first 5 raw data chunks, so the
                // log file doesn't fill up with frequent status lines.
                bool isLastChunkLogged = false;
                if (shouldLogThrottled(rocrailLogChunkCount, isLastChunkLogged, 5)) {
                    DEBUG_PRINTLN("[ROCRAIL] RCP #" + String(rocrailLogChunkCount) +
                                  " (" + String(n) + " bytes): " + String(buf));
                }

                rocrailRxBuffer += buf;

                // Puffer deckeln: ein <clock/>-Tag ist immer kurz, ein
                // grosser Broadcast (z.B. <plan>) soll den Heap nicht
                // unbegrenzt wachsen lassen.

                // Cap the buffer: a <clock/> tag is always short, a large
                // broadcast (e.g. <plan>) shouldn't grow the heap without bound.
                if (rocrailRxBuffer.length() > 4096) {
                    rocrailRxBuffer = rocrailRxBuffer.substring(rocrailRxBuffer.length() - 512);
                }

                processRocrailBuffer();
            }
        }
    }

