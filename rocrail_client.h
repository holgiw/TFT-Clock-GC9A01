#pragma once

    // Rocrail-Modellzeit: TCP-Verbindung (RCP) in eigener Task, die Fast-Clock des Servers treibt die Zeiger.
    // <clock>-Events kommen ungerahmt (Substring-Suche). Dazu nur ein R2RNet-Multicast-Diagnoselog, nach
    // jedem WLAN-(Re-)Connect neu zu starten.

    // Rocrail model time: TCP connection (RCP) in its own task, the server's fast clock drives the hands.
    // <clock> events arrive unframed (substring scan). Plus only an R2RNet multicast diagnostic log, to be
    // restarted after every WiFi (re)connect.

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
            r2rnetDebugLogCount = 0; // neuer Beitritt: wieder kurz loggen
                                     // new join: log briefly again
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


    // In loop() gepollt: liest Multicast-Pakete ohne zu blockieren und loggt Absender, Laenge, Rohtext und
    // die ersten Bytes als Hex (fuer binaere Header/Nicht-ASCII).

    // Polled in loop(): reads multicast packets without blocking and logs sender, length, raw text and the
    // first bytes as hex (for binary headers/non-ASCII).

    void pollR2rnetDebugListener() {
        if (!r2rnetDebugListening) return;

        int packetSize = r2rnetDebugUdp.parsePacket();
        if (packetSize <= 0) return;

        // Nur die ersten R2RNET_DEBUG_LOG_LIMIT Pakete je Beitritt loggen -
        // jede Logzeile schreibt in LittleFS (Flash-Verschleiss, kostet DCF77-
        // Flanken); parsePacket() hat das Paket bereits aus dem Puffer geholt.

        // Only log the first R2RNET_DEBUG_LOG_LIMIT packets per join - each
        // log line writes to LittleFS (flash wear, costs DCF77 edges);
        // parsePacket() has already pulled the packet out of the buffer.

        bool isLastLogged = false;
        if (!shouldLogThrottled(r2rnetDebugLogCount, isLastLogged, R2RNET_DEBUG_LOG_LIMIT)) return;

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
            " | text: " + String((char*)buf) + " | hex: " + hex +
            (isLastLogged ? " - no more R2RNet packets logged until the next reconnect" : ""));
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
        // (uhr4.ino) ruft danach triggerRocrailConnectNow() auf und verbindet sich so noch beim Start.

        // Align rocrailServerHost/-Port with the resolved index - needed if
        // the fallback above just determined an index. setup() (uhr4.ino)
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

    // Wertet ein <clock .../>-Tag aus. "time" ist der reale Sendezeitpunkt, keine Modellzeit-Sekunde; jedes
    // (minutengenaue) Update ist Sekunde 0 einer neuen Modellminute, dazwischen laeuft die Anzeige lokal
    // weiter. Abweichungen werden sanft angeglichen; wiederkehrende Logs nur fuer die ersten `limit` Aufrufe.

    // Evaluates a <clock .../> tag. "time" is the real send time, not a model-time second; each
    // (minute-granular) update is second 0 of a new model minute, the display runs locally in between.
    // Deviations are eased in; recurring logs only for the first `limit` calls.

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

        // Vorherigen Zustand (rocrailFrozen, rocrailDivider) sichern, BEVOR dieses Update ihn ueberschreibt -
        // um einen echten Wechsel von einem unveraenderten Update zu unterscheiden.

        // Save the previous state (rocrailFrozen, rocrailDivider) BEFORE this update overwrites it - to tell
        // a genuine change from an unchanged update.

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

            // Die Routinezeile nur fuer die ersten beiden Updates loggen, danach nur echte Aenderungen
            // (Divider, Pause/Lauf) - grosse Drift wird immer geloggt. Host:Port stehen dabei, damit bei
            // mehreren Servern die Quelle erkennbar ist.

            // Log the routine line only for the first two updates, afterwards only genuine changes (divider,
            // pause/run) - large drift is always logged. Host:port are included so the source is clear with
            // multiple servers.

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

            // Wie bei der Clock-Routinezeile nur die ersten 2 Fehlschlaege loggen und im Hintergrund weiter
            // versuchen - mit Hinweis in der letzten geloggten Zeile, damit es nicht wie eine tote Verbindung
            // wirkt.

            // Like the clock routine line, only log the first 2 failures and keep retrying in the background
            // - with a hint on the last logged line so it does not look like a dead connection.

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

        // Snapshot JETZT auf dem Hauptthread anlegen (siehe rocrailServerHostSnapshot in globals.h) - die
        // Task liest nur die Kopie, auch wenn waehrenddessen neue Rocrail-Einstellungen gespeichert werden.

        // Take the snapshot NOW on the main thread (see rocrailServerHostSnapshot in globals.h) - the task
        // only reads the copy, even if new Rocrail settings are saved meanwhile.

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


    // Rocrail-Modellzeit ein-/ausschalten und speichern (Einstellungsseite, /api/setMode, Presets). Aus:
    // Verbindung und R2RNet-Diagnose sofort beenden; an: sofort verbinden statt auf das naechste Zeitfenster
    // zu warten (no-op ohne Serveradresse) und die Diagnose mit starten.

    // Switch the Rocrail model time on/off and store it (settings page, /api/setMode, presets). Off: close
    // the connection and the R2RNet diagnostics right away; on: connect immediately instead of waiting for
    // the next window (no-op without a server address) and start the diagnostics too.

    void setRocrailEnabled(bool on) {
        rocrailEnabled = on;
        preferences.putBool(PK_ROCRAIL_ENABLED, on);
        if (!on) {
            if (rocrailClient.connected()) rocrailClient.stop();
            rocrailConnected = false;
            r2rnetDebugUdp.stop();
            r2rnetDebugListening = false;
        }
        else {
            triggerRocrailConnectNow();
            startR2rnetDebugListener();
        }
    }
