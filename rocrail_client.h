#pragma once
    // Rocrail-Modellzeit: verbindet sich mit einem Rocrail-Server (TCP-Port
    // 8051, RCP-Protokoll) und uebernimmt dessen Fast-Clock als Zeitquelle
    // fuer die Zeiger. Serveradresse (Rocrail-Tab) ist verpflichtend.

    // Rocrail model time: connects to a Rocrail server (TCP port 8051, RCP
    // protocol) and adopts its fast clock as the time source for the
    // hands. Server address (Rocrail tab) is mandatory.

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


    // Laedt die Liste moeglicher Rocrail-Server (bis zu MAX_WLAN, siehe
    // globals.h) aus den Preferences und stellt sie in rocrailServerList[]/
    // rocrailServerPortList[]/rocrailActiveServerIndex wieder her - analog
    // zum Laden der WLAN-/NTP-Listen in uhr3.ino. Wird einmal in setup()
    // aufgerufen, direkt nach dem Laden von rocrailServerHost/-Port (die
    // weiterhin den "aktuell aktiven" Server tragen, siehe globals.h).

    // Einmalige Migration: ist die Liste noch komplett leer, aber
    // rocrailServerHost (aus der alten Einzel-Server-Preference) bereits
    // gesetzt, wird dieser Wert als Listenplatz 1 uebernommen und dort als
    // aktiv markiert - so bleibt ein vor diesem Update konfigurierter
    // Server nach einem Firmware-Update in der neuen Mehrfach-Server-
    // Uebersicht sichtbar, statt dort leer zu erscheinen.

    // Loads the list of possible Rocrail servers (up to MAX_WLAN, see
    // globals.h) from preferences and restores it into rocrailServerList[]/
    // rocrailServerPortList[]/rocrailActiveServerIndex - analogous to
    // loading the WiFi/NTP lists in uhr3.ino. Called once in setup(),
    // right after loading rocrailServerHost/-Port (which continue to carry
    // the "currently active" server, see globals.h).

    // One-time migration: if the list is still completely empty but
    // rocrailServerHost (from the old single-server preference) is already
    // set, that value is taken over as list slot 1 and marked active there -
    // so a server configured before this update stays visible in the new
    // multi-server list after a firmware update, instead of appearing empty.

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


    // Sucht das oeffnende <plan .../>-Tag (bzw. <plan ...> bei Kindelementen)
    // im Rohstrom und uebernimmt dessen "title"-Attribut als Anlagenname fuer
    // den aktuell aktiven Server - aber nur, solange dessen Namensfeld noch
    // leer ist (siehe rocrailServerNameList[] in globals.h). Rein passiv:
    // es wird NICHT aktiv angefragt (ein frueherer Versuch mit
    // <model cmd="plan"/> wurde wieder entfernt - die Antwort kann den
    // kompletten Anlagenplan mit allen Bloecken/Routen umfassen und damit
    // den begrenzten Speicher des ESP32 sprengen bzw. die Verbindung
    // stoeren). Sendet ein Rocrail-Server den Plan von sich aus (z.B. manche
    // Versionen/Konfigurationen direkt nach dem Verbindungsaufbau), wird der
    // Titel opportunistisch mitgenommen - ansonsten bleibt das Feld leer und
    // ist manuell einzutragen. Bewusst VOR der Clock-Tag-Verarbeitung in
    // processRocrailBuffer() aufgerufen: die verwirft beim Finden eines
    // <clock/>-Tags alles davor im Puffer (siehe dort) - ein dazwischen
    // angekommenes <plan>-Tag ginge sonst unbemerkt verloren. Der grosse
    // Rest eines <plan>-Elements (Bloecke, Routen, ...) wird bewusst nicht
    // ausgewertet, nur die oeffnenden Attribute interessieren hier - er wird
    // spaeter ganz normal vom Puffer-Deckel in pollRocrailClient() verworfen.

    // Finds the opening <plan .../> tag (or <plan ...> if it has child
    // elements) in the raw stream and takes over its "title" attribute as
    // the layout name for the currently active server - but only while that
    // server's name field is still empty (see rocrailServerNameList[] in
    // globals.h). Purely passive: it is NOT actively requested (an earlier
    // attempt using <model cmd="plan"/> was removed again - the reply can
    // include the entire layout plan with all blocks/routes, which can
    // exceed the ESP32's limited memory or disrupt the connection). If a
    // Rocrail server sends the plan on its own (e.g. some versions/
    // configurations do so right after connecting), the title is picked up
    // opportunistically - otherwise the field stays empty and needs to be
    // entered manually. Deliberately called BEFORE the clock-tag processing
    // in processRocrailBuffer(): that discards everything before a found
    // <clock/> tag in the buffer (see there) - a <plan> tag that arrived in
    // between would otherwise be silently lost. The large remainder of a
    // <plan> element (blocks, routes, ...) is deliberately not evaluated,
    // only the opening attributes matter here - it later gets discarded as
    // usual by the buffer cap in pollRocrailClient().

    void processRocrailPlanTag() {
        if (rocrailActiveServerIndex < 0 || rocrailActiveServerIndex >= MAX_WLAN) return;
        if (strlen(rocrailServerNameList[rocrailActiveServerIndex]) > 0) return; // schon gesetzt (Nutzer oder frueher per RCP) - nicht ueberschreiben
                                                                                 // already set (by the user or earlier via RCP) - don't overwrite

        int planPos = rocrailRxBuffer.indexOf("<plan ");
        if (planPos == -1) return;

        int endPos = rocrailRxBuffer.indexOf(">", planPos);
        if (endPos == -1) return; // Oeffnendes Tag noch nicht vollstaendig angekommen
                                  // opening tag hasn't fully arrived yet

        String title = rocrailXmlAttrString(rocrailRxBuffer.substring(planPos, endPos + 1), "title");
        title.trim();
        if (title.isEmpty()) return;

        strncpy(rocrailServerNameList[rocrailActiveServerIndex], title.c_str(), sizeof(rocrailServerNameList[rocrailActiveServerIndex]) - 1);
        rocrailServerNameList[rocrailActiveServerIndex][sizeof(rocrailServerNameList[rocrailActiveServerIndex]) - 1] = '\0';
        preferences.putString(pkRocrailServerName(rocrailActiveServerIndex).c_str(), rocrailServerNameList[rocrailActiveServerIndex]);

        DEBUG_PRINTLN("[ROCRAIL] Layout name from RCP for server " + String(rocrailActiveServerIndex + 1) + ": " + title);
    }


    // Sucht das letzte vollstaendige <clock .../>-Tag im Rohstrom (kein
    // <xmlh>-Header, siehe Dateikopf) und wertet es aus.

    // Finds the last complete <clock .../> tag in the raw stream (no
    // <xmlh> header, see file header) and evaluates it.

    void processRocrailBuffer() {
        processRocrailPlanTag();

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
    // Clock-Objekt, "time" liefert die Sekunde (time%60). Eine Abweichung
    // wird sanft angeglichen statt gesprungen (siehe advanceRocrailTime()).

    // Evaluates a <clock .../> tag: state ("go"/"freeze") belongs to the
    // clock object, "time" gives the second (time%60). A deviation is
    // eased in smoothly instead of snapped (see advanceRocrailTime()).

    void processRocrailClockPayload(const String& payload) {
        int hour = rocrailXmlAttrInt(payload, "hour", -1);
        int minute = rocrailXmlAttrInt(payload, "minute", -1);
        int divider = rocrailXmlAttrInt(payload, "divider", rocrailDivider);
        long timeValue = rocrailXmlAttrInt(payload, "time", 0);
        String state = rocrailXmlAttrString(payload, "state");

        // Helligkeit: optionales "bri"-Attribut (0-255), das Rocrail nur
        // sendet, wenn serverseitig eine Tag/Nacht-Lichtsteuerung konfiguriert
        // ist. Fallback -1 heisst "in diesem Telegramm nicht enthalten" -
        // rocrailBrightness/-Known bleiben dann unveraendert, statt auf einen
        // falschen Wert wie 0 zurueckzufallen. Die eigentliche Anwendung
        // (Deaktivieren der lokalen Fotowiderstand-/Zeitsteuerung) geschieht
        // in updateBrightness() (display.h), nicht hier.

        // Brightness: optional "bri" attribute (0-255) that Rocrail only
        // sends when a day/night lighting scheme is configured server-side.
        // Fallback -1 means "not present in this telegram" - rocrailBrightness/
        // -Known then stay unchanged, instead of falling back to a wrong
        // value like 0. The actual application (disabling the local
        // photoresistor/time-of-day control) happens in updateBrightness()
        // (display.h), not here.
        int bri = rocrailXmlAttrInt(payload, "bri", -1);
        if (bri >= 0) {
            rocrailBrightness = (uint8_t)constrain(bri, 0, 255);
            rocrailBrightnessKnown = true;
        }

        if (state == "go") rocrailFrozen = false;
        else if (state == "freeze") rocrailFrozen = true;

        if (hour >= 0 && hour < 24 && minute >= 0 && minute < 60) {
            int seconds = (timeValue > 0) ? (int)(timeValue % 60) : 0;
            long newModelSeconds = (long)hour * 3600 + (long)minute * 60 + seconds;

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
                    DEBUG_PRINTLN("[ROCRAIL] Large drift (" + String(drift, 1) + "s) - snapping directly");
                }
                else {
                    rocrailDriftSeconds = drift;
                }
            }

            rocrailLastClockMillis = millis();
            if (divider > 0) rocrailDivider = (uint8_t)divider;

            DEBUG_PRINTLN("[ROCRAIL] clock " + String(hour) + ":" +
                          (minute < 10 ? "0" : "") + String(minute) + ":" +
                          (seconds < 10 ? "0" : "") + String(seconds) +
                          " (divider " + String(rocrailDivider) +
                          (rocrailFrozen ? ", angehalten)" : ")"));
        }
    }


    // Eigene FreeRTOS-Task fuer den (weiterhin blockierenden) connect(),
    // damit loop() nicht blockiert. Loescht sich am Ende selbst.

    // Own FreeRTOS task for the (still blocking) connect(), so loop()
    // doesn't block. Deletes itself at the end.

    void rocrailConnectTaskFunc(void* param) {
        rocrailConnectTaskResult = rocrailClient.connect(rocrailServerHost.c_str(), rocrailServerPort, ROCRAIL_CONNECT_TIMEOUT_MS);
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
            DEBUG_PRINTLN("[ROCRAIL] Connected");
            rocrailRxBuffer = "";
            rocrailFrozen = false; // Zustand unbekannt bis zum naechsten state-Attribut
                                   // state unknown until the next state attribute
            // Naechstes <clock>-Update startet die Modellzeit neu.
            // Next <clock> update restarts the model time.
            rocrailLastClockMillis = 0;
        }
        else if (!rocrailConnectTaskResult) {
            DEBUG_PRINTLN("[ROCRAIL] Server did not respond on port " + String(rocrailServerPort) +
                          " - falling back to real time, retrying in a minute");
        }
    }


    // Gemeinsamer Kern von connectRocrailClient() (periodisch, Sekunde 59)
    // und triggerRocrailConnectNow() (sofort nach einer Einstellungs-
    // Aenderung im Webinterface) - startet die eigentliche Connect-Task.
    // Geht davon aus, dass der Aufrufer bereits geprueft hat, dass gerade
    // keine Task laeuft und keine Verbindung besteht.

    // Shared core of connectRocrailClient() (periodic, second 59) and
    // triggerRocrailConnectNow() (immediate, right after a settings change
    // in the web interface) - starts the actual connect task. Assumes the
    // caller has already checked that no task is currently running and no
    // connection is open.

    void startRocrailConnectTask() {
        rocrailLastConnectAttemptMillis = millis();

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

        DEBUG_PRINTLN("[ROCRAIL] Connecting to " + rocrailServerHost + ":" + String(rocrailServerPort));
        startRocrailConnectTask();
    }


    // Stoesst sofort einen Verbindungsversuch an, ohne auf das naechste
    // reguläre Zeitfenster (Sekunde 59, hoechstens 1x pro Minute, siehe
    // connectRocrailClient()) zu warten - aufgerufen direkt nachdem im
    // Webinterface Rocrail aktiviert oder die Serveradresse gespeichert
    // wurde (siehe webserver_routes.h /applydisplaysettings, /save_rocrail).
    // Kein Effekt, wenn Rocrail nicht aktiv ist, keine Serveradresse
    // hinterlegt ist, bereits eine Verbindung besteht oder schon eine
    // Connect-Task laeuft - dann uebernehmen ohnehin die naechsten
    // pollRocrailClient()/pollRocrailConnectTask()-Aufrufe aus loop().

    // Immediately kicks off a connection attempt, without waiting for the
    // next regular window (second 59, at most once a minute, see
    // connectRocrailClient()) - called right after Rocrail was enabled or
    // the server address was saved in the web interface (see
    // webserver_routes.h /applydisplaysettings, /save_rocrail). No effect
    // if Rocrail isn't enabled, no server address is configured, a
    // connection is already open, or a connect task is already running -
    // the next pollRocrailClient()/pollRocrailConnectTask() calls from
    // loop() handle that case anyway.

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
                if (rocrailLogChunkCount < 5) {
                    rocrailLogChunkCount++;
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
