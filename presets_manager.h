#pragma once
    // Presets: Laden/Speichern/Wechseln der Anzeigekonfigurationen.
    // Benoetigt globals.h, config.h, prefs_keys.h, declarations.h (vorher eingebunden).

    // Presets: load/save/switch display configurations.
    // Requires globals.h, config.h, prefs_keys.h, declarations.h (included before this file).


    // Entfernt einen alten "rotation="-Parameter aus einer Preset-URL
    // (Altlast frueherer Versionen, siehe switchToNextPreset()).

    // Removes a legacy "rotation=" parameter from a preset URL
    // (leftover from older versions, see switchToNextPreset()).

    String stripRotationParam(const String& url) {
        int qIdx = url.indexOf('?');
        if (qIdx == -1) return url;
        String base = url.substring(0, qIdx);
        String query = url.substring(qIdx + 1);
        String result = "";
        int start = 0;
        while (start <= (int)query.length()) {
            int amp = query.indexOf('&', start);
            String part = (amp == -1) ? query.substring(start) : query.substring(start, amp);
            if (!part.startsWith("rotation=")) {
                if (result.length() > 0) result += "&";
                result += part;
            }
            if (amp == -1) break;
            start = amp + 1;
        }
        return base + "?" + result;
    }


    // Presets laden und dabei die gespeicherte IP-Adresse durch die aktuelle IP des ESP ersetzen
    // Load presets, replacing the stored IP address with the ESP's current IP

    void loadPresets() {

        for (int i = 0; i < MAX_PRESETS; i++) {
            String nameKey = pkPresetName(i);
            String urlKey = pkPresetUrl(i);

            presets[i].name = preferences.getString(nameKey.c_str(), "");
            presets[i].url = preferences.getString(urlKey.c_str(), "");

            // Alte "rotation="-Parameter entfernen und dauerhaft fixen (nur bei
            // Aenderung schreiben, spart Flash-Verschleiss).

            // Remove legacy "rotation=" parameters and fix permanently (write
            // only on change, saves flash wear).
            if (presets[i].url.indexOf("rotation=") != -1) {
                String cleaned = stripRotationParam(presets[i].url);
                if (cleaned != presets[i].url) {
                    presets[i].url = cleaned;
                    preferences.putString(urlKey.c_str(), presets[i].url);
                }
            }

            // Ersetze die gespeicherte IP durch die aktuelle IP des ESP
            // Replace the stored IP with the ESP's current IP
            if (presets[i].url.startsWith("http://")) {
                int ipEnd = presets[i].url.indexOf('/', 7); // Suche Ende der IP-Adresse
                                                            // Find end of IP address
                if (ipEnd != -1) {
                    presets[i].url = "http://" + ipAddress + presets[i].url.substring(ipEnd); // Ersetze die IP
                                           // Replace the IP
                }
                else {
                    presets[i].url = "http://" + ipAddress; // Nur IP ohne Pfad
                                           // IP only, no path
                }
            }
        }
    }


    // Presets speichern und dabei die aktuelle IP-Adresse des ESP in der URL verwenden
    // Save presets, using the ESP's current IP address in the URL

    void savePresets() {

        for (int i = 0; i < MAX_PRESETS; i++) {
            String nameKey = pkPresetName(i);
            String urlKey = pkPresetUrl(i);

            // Ersetze eine vorhandene IP-Adresse durch die aktuelle IP des ESP
            // Replace an existing IP address with the ESP's current IP
            if (presets[i].url.startsWith("http://")) {
                int ipEnd = presets[i].url.indexOf('/', 7);
                if (ipEnd != -1) {
                    presets[i].url = "http://" + ipAddress + presets[i].url.substring(ipEnd);
                }
                else {
                    presets[i].url = "http://" + ipAddress;
                }
            }

            // Nur schreiben bei tatsaechlicher Aenderung - das Formular sendet
            // immer alle MAX_PRESETS Eintraege mit (spart Flash-Verschleiss).

            // Only write on actual change - the form always submits all
            // MAX_PRESETS entries (saves flash wear).
            if (preferences.getString(nameKey.c_str(), "") != presets[i].name) {
                preferences.putString(nameKey.c_str(), presets[i].name);
            }
            if (preferences.getString(urlKey.c_str(), "") != presets[i].url) {
                preferences.putString(urlKey.c_str(), presets[i].url);
            }
        }
    }


    // Erstellt ein neues Preset basierend auf den aktuellen Einstellungen in den Preferences
    // Creates a new preset based on the current settings in the preferences

    bool createPresetFromPreferences(const String& customName) {
        // Suche das erste leere Preset
        // Find the first empty preset
        int presetIndex = -1;
        for (int i = 0; i < MAX_PRESETS; i++) {
            if (presets[i].name.isEmpty() && presets[i].url.isEmpty()) {
                presetIndex = i;
                break;
            }
        }

        // Wenn kein leeres Preset gefunden wurde, abbrechen
        // Abort if no empty preset was found
        if (presetIndex == -1) {
            DEBUG_PRINTLN("[Preset] No empty preset slot available");
            return false;
        }

        // Lese die aktuellen Einstellungen aus den Preferences
        // Read the current settings from the preferences
        String background = preferences.getString(PK_BACKGROUND, "/face_default.bmp");
        String handset = preferences.getString(PK_HANDSET, "default");

        bool stationMode = preferences.getBool(PK_STATION_MODE, true);
        bool showSecondHand = preferences.getBool(PK_SHOW_SECOND_HAND, true);
        bool smoothMinute = preferences.getBool(PK_SMOOTH_MINUTE, false);
        // Fallback bewusst stationMode statt eines festen Literals - siehe
        // Kommentar bei der smoothSecond-Ladezeile in uhr3.ino.
        // Fallback deliberately stationMode instead of a fixed literal - see
        // the comment at the smoothSecond load line in uhr3.ino.
        bool smoothSecond = getSmoothSecondPref(stationMode);
        uint8_t hubSize = preferences.getUInt(PK_CENTER_SIZE, 6);
        uint32_t hubColor = preferences.getLong(PK_CENTER_COLOR, 0xEC0016);

        // URL bewusst OHNE "rotation": geraeteweite HW-Einstellung, bleibt beim Laden unveraendert.
        // Build URL deliberately WITHOUT "rotation": device-wide HW setting, stays unchanged when loading.
        String url = "http://" + ipAddress + "/api/setMode?";
        if (background.startsWith("/")) {
            background = background.substring(1); // Entferne führenden Slash
                                                  // Remove leading slash
        }
        url += "face=" + background;
        url += "&handSet=" + handset;

        url += "&stationMode=" + String(stationMode ? "true" : "false");
        url += "&showSecondHand=" + String(showSecondHand ? "true" : "false");
        url += "&smoothMinute=" + String(smoothMinute ? "true" : "false");
        url += "&smoothSecond=" + String(smoothSecond ? "true" : "false");
        url += "&hubSize=" + String(hubSize);
        url += "&hubColor=" + String(hubColor, HEX);

        // Speichere das Preset
        // Save the preset
        String presetName;
        if (!customName.isEmpty()) {
            presetName = customName;
            presetName.replace(" ", "_"); // Konsistent zu Anzeige/API-Links (siehe /presets)
                                          // Consistent with display/API links (see /presets)
        }
        else {
            presetName = String(presetIndex + 1) + "_Preset";
        }

        // Eindeutigkeit sicherstellen: switchToNextPreset() identifiziert das
        // aktuelle Preset ueber den NAMEN (nicht den Index). Bei Kollision
        // waeren zwei Presets nicht unterscheidbar - Suffix anhaengen.

        // Ensure uniqueness: switchToNextPreset() identifies the current
        // preset by NAME (not index). Colliding names would make presets
        // indistinguishable - append a numeric suffix.
        if (!customName.isEmpty()) {
            String baseName = presetName;
            int suffix = 2;
            bool collision = true;
            while (collision) {
                collision = false;
                for (int i = 0; i < MAX_PRESETS; i++) {
                    if (i != presetIndex && presets[i].name == presetName) {
                        collision = true;
                        break;
                    }
                }
                if (collision) {
                    presetName = baseName + "_" + String(suffix);
                    suffix++;
                }
            }
        }
        presets[presetIndex].name = presetName;
        presets[presetIndex].url = url;

        // Schreibe das Preset in die Preferences
        // Write the preset to the preferences
        String nameKey = pkPresetName(presetIndex);
        String urlKey = pkPresetUrl(presetIndex);
        preferences.putString(nameKey.c_str(), presetName);
        preferences.putString(urlKey.c_str(), url);

        DEBUG_PRINTLN("[Preset] Created preset: " + presetName);
        DEBUG_PRINTLN("[Preset] URL: " + url);
        return true;
    }


    // Parst die Preset-URL und extrahiert nur die fuer die Vorschau relevanten
    // Werte (face, handSet, hubColor, hubSize, showSecondHand) - reine Lesefunktion.

    // Parses the preset URL and extracts only the values needed for the
    // preview (face, handSet, hubColor, hubSize, showSecondHand) - read-only.

    void parsePresetForPreview(const String& url, String& faceOut, String& handSetOut,
        uint16_t& hubColorOut, uint8_t& hubSizeOut, bool& showSecondOut) {
        // Sinnvolle Standardwerte, falls ein Parameter im Preset fehlt
        // Sensible defaults in case a parameter is missing from the preset
        faceOut = "/face_default.bmp";
        handSetOut = "default";
        hubColorOut = 0xF800; // Rot in RGB565 (entspricht TFT_RED)
                              // Red in RGB565 (roughly TFT_RED)
        hubSizeOut = 6;
        showSecondOut = true;

        int queryStart = url.indexOf('?');
        if (queryStart == -1) return;
        String query = url.substring(queryStart + 1);

        while (query.length() > 0) {
            int amp = query.indexOf('&');
            String param = (amp == -1) ? query : query.substring(0, amp);
            query = (amp == -1) ? "" : query.substring(amp + 1);

            int eq = param.indexOf('=');
            if (eq == -1) continue;
            String key = param.substring(0, eq);
            String value = param.substring(eq + 1);

            if (key == "face") {
                if (!value.startsWith("/")) value = "/" + value;
                faceOut = value;
            }
            else if (key == "handSet") {
                handSetOut = value;
            }
            else if (key == "hubSize") {
                hubSizeOut = value.toInt();
            }
            else if (key == "hubColor") {
                uint32_t rgb = strtoul(value.c_str(), NULL, 16);
                uint8_t r = (rgb >> 16) & 0xFF;
                uint8_t g = (rgb >> 8) & 0xFF;
                uint8_t b = rgb & 0xFF;
                hubColorOut = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
            }
            else if (key == "showSecondHand") {
                showSecondOut = (value == "1" || value.equalsIgnoreCase("true"));
            }
        }
    }


    // Entfernt Presets, deren Zifferblatt/Zeigersatz dem angegebenen Wert
    // entspricht (leer = ignoriert) - verhindert Verwaisung nach Datei-Loeschung.

    // Removes presets whose face/hand set matches the given value (empty =
    // ignored) - prevents orphaned presets after a file is deleted.

    void removeOrphanedPresets(const String& deletedFace, const String& deletedHandSet) {
        bool anyRemoved = false;
        for (int i = 0; i < MAX_PRESETS; i++) {
            if (presets[i].name.isEmpty() || presets[i].url.isEmpty()) continue;

            String face, handSet;
            uint16_t hubColor;
            uint8_t hubSize;
            bool showSecond;
            parsePresetForPreview(presets[i].url, face, handSet, hubColor, hubSize, showSecond);

            bool matches = (!deletedFace.isEmpty() && face == deletedFace) ||
                (!deletedHandSet.isEmpty() && handSet == deletedHandSet);

            if (matches) {
                DEBUG_PRINTLN("[Preset] Removing orphaned preset '" + presets[i].name + "' (references a deleted file)");
                presets[i].name = "";
                presets[i].url = "";
                anyRemoved = true;
            }
        }
        if (anyRemoved) savePresets();
    }


    // Loescht alle gespeicherten Presets (leert alle Slots).
    // Deletes all saved presets (clears all slots).

    void resetAllPresets() {
        for (int i = 0; i < MAX_PRESETS; i++) {
            presets[i].name = "";
            presets[i].url = "";
        }
        savePresets();
    }


    void switchToNextPreset() {
        // Sammle alle gültigen Presets
        // Collect all valid presets
        std::vector<int> validPresets;
        for (int i = 0; i < MAX_PRESETS; i++) {
            if (!presets[i].name.isEmpty() && !presets[i].url.isEmpty()) {
                validPresets.push_back(i);
            }
        }

        if (validPresets.empty()) {
            DEBUG_PRINTLN("[PRESET] No valid presets found");
            return;
        }

        // Bestimme den aktuellen Preset-Index
        // Determine the current preset index
        String currentPresetName = preferences.getString(PK_CURRENT_PRESET, "");
        int currentIndex = -1;
        for (size_t i = 0; i < validPresets.size(); i++) {
            if (presets[validPresets[i]].name == currentPresetName) {
                currentIndex = (int)i;
                break;
            }
        }

        // Wähle das nächste Preset
        // Select the next preset
        int nextIndex = (currentIndex + 1) % validPresets.size();
        int nextPresetIndex = validPresets[nextIndex];

        // Lade das nächste Preset
        // Load the next preset
        String nextPresetUrl = presets[nextPresetIndex].url;

        // Sicherstellen, dass die URL ab "/api" beginnt
        // Ensure the URL starts with "/api"
        if (!nextPresetUrl.startsWith("/api")) {
            DEBUG_PRINTLN("[PRESET] Invalid URL format, adjusting..");
            int apiIndex = nextPresetUrl.indexOf("/api");
            if (apiIndex != -1) {
                nextPresetUrl = nextPresetUrl.substring(apiIndex);
            }
            else {
                DEBUG_PRINTLN("[PRESET] URL does not contain '/api', aborting..");
                return;
            }
        }

        DEBUG_PRINTLN("[PRESET] Switching to preset: " + presets[nextPresetIndex].name + " -> " + nextPresetUrl);

        // PK_CURRENT_PRESET wird bewusst erst am Ende gespeichert, nicht hier -
        // sonst wuerde bei vorzeitigem Return (z.B. fehlende Query-Parameter)
        // der Name auf ein Preset zeigen, dessen Einstellungen nie griffen.

        // PK_CURRENT_PRESET is deliberately saved only at the end, not here -
        // otherwise an early return (e.g. missing query parameters) would leave
        // the name pointing at a preset whose settings were never applied.

        // Entferne die Basis-URL, falls vorhanden
        // Remove the base URL, if present
        int queryStart = nextPresetUrl.indexOf('?');
        if (queryStart == -1) {
            DEBUG_PRINTLN("[PRESET] No query parameters found in URL");
            return;
        }
        String query = nextPresetUrl.substring(queryStart + 1);

        // Parse die Parameter
        // Parse the parameters
        bool sawSmoothSecond = false;
        while (query.length() > 0) {
            int ampersandIndex = query.indexOf('&');
            String param = query.substring(0, ampersandIndex);
            if (ampersandIndex == -1) {
                query = "";
            }
            else {
                query = query.substring(ampersandIndex + 1);
            }

            int equalsIndex = param.indexOf('=');
            if (equalsIndex == -1) continue;

            String key = param.substring(0, equalsIndex);
            String value = param.substring(equalsIndex + 1);

            // Wende die Einstellungen an
            // Apply the settings
            if (key == "face") {
                //value.replace(".", "");
                if (!value.startsWith("/")) value = "/" + value;
                if (value == "/face_default.bmp" || LittleFS.exists(value)) {
                    preferences.putString(PK_BACKGROUND, value);
                    selectedBackground = value;
                }
            }
            else if (key == "handSet") {
                preferences.putString(PK_HANDSET, value);
            }
            else if (key == "timeZone") {
                preferences.putString(PK_TIMEZONE, value);

                // Globale timezone-Variable aktualisieren + nur die Task
                // anstossen (siehe time_sync.h) statt hier zu blockieren -
                // switchToNextPreset() wird aus der Touch-Behandlung heraus
                // aufgerufen (display.h), ein blockierendes setupNTP() wuerde
                // die Touch-Reaktion und den Zeigerantrieb einfrieren.

                // Update the global timezone variable + only kick off the
                // task (see time_sync.h) instead of blocking here -
                // switchToNextPreset() is called from touch handling
                // (display.h), a blocking setupNTP() would freeze touch
                // response and the hand animation.
                timezone = value;
                startNtpSyncTask("Preset switch sync");
            }
            else if (key == "hubSize") {
                hubSize = value.toInt();
                preferences.putUInt(PK_CENTER_SIZE, hubSize);
            }
            else if (key == "hubColor") {
                uint32_t rgb = strtoul(value.c_str(), NULL, 16);
                uint8_t r = (rgb >> 16) & 0xFF;
                uint8_t g = (rgb >> 8) & 0xFF;
                uint8_t b = rgb & 0xFF;
                hubColor = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
                preferences.putLong(PK_CENTER_COLOR, rgb);
            }
            else if (key == "stationMode") {
                stationMode = (value == "1" || value.equalsIgnoreCase("true"));
                preferences.putBool(PK_STATION_MODE, stationMode);
            }
            else if (key == "rotation") {
                // Bewusst ignoriert: Rotation ist eine geraeteweite HW-Einstellung.
                // Deliberately ignored: rotation is a device-wide HW setting.
            }
            else if (key == "showSecondHand") {
                showSecondHand = (value == "1" || value.equalsIgnoreCase("true"));
                preferences.putBool(PK_SHOW_SECOND_HAND, showSecondHand);
            }
            else if (key == "smoothMinute") {
                smoothMinute = (value == "1" || value.equalsIgnoreCase("true"));
                preferences.putBool(PK_SMOOTH_MINUTE, smoothMinute);
            }
            else if (key == "smoothSecond") {
                sawSmoothSecond = true;
                smoothSecond = (value == "1" || value.equalsIgnoreCase("true"));
                preferences.putBool(PK_SMOOTH_SECOND, smoothSecond);
            }
        }

        // Altes Preset (von vor der Trennung von stationMode/smoothSecond,
        // siehe globals.h): enthaelt kein eigenes smoothSecond. Damals gab es
        // schwingend nur zusammen mit stationMode=true - smoothSecond daher
        // hier genau wie stationMode setzen, damit das Preset weiterhin exakt
        // so aussieht wie zu der Zeit, als es erstellt wurde, statt ploetzlich
        // eine (damals gar nicht waehlbare) neue Kombination zu zeigen. Die
        // Preset-URL wird dabei gleich um den jetzt expliziten Parameter
        // ergaenzt - einmalig "geheilt", ab jetzt ist das Preset vollstaendig
        // und braucht diese Herleitung nicht mehr.

        // Old preset (from before stationMode/smoothSecond were split, see
        // globals.h): has no smoothSecond of its own. Back then, smooth
        // motion only existed together with stationMode=true - so set
        // smoothSecond to match stationMode here, so the preset keeps looking
        // exactly like it did when it was created, instead of suddenly
        // showing a (back then not even selectable) new combination. The
        // preset's URL gets the now-explicit parameter added at the same time
        // - healed once, from now on the preset is complete and no longer
        // needs this derivation.
        if (!sawSmoothSecond) {
            smoothSecond = stationMode;
            preferences.putBool(PK_SMOOTH_SECOND, smoothSecond);

            String healedUrl = presets[nextPresetIndex].url + "&smoothSecond=" + String(smoothSecond ? "true" : "false");
            presets[nextPresetIndex].url = healedUrl;
            preferences.putString(pkPresetUrl(nextPresetIndex).c_str(), healedUrl);

            DEBUG_PRINTLN("[PRESET] " + presets[nextPresetIndex].name + " had no smoothSecond - derived " +
                          String(smoothSecond ? "true" : "false") + " from stationMode and healed the stored URL");
        }

        freeClockFaceBuffer();
        loadClockFace();
        loadHandSprites();
        updateClock();

        // Speichere den aktuellen Preset-Namen
        // Save the current preset name
        preferences.putString(PK_CURRENT_PRESET, presets[nextPresetIndex].name);

        DEBUG_PRINTLN("[PRESET] Switched to preset: " + presets[nextPresetIndex].name);
    }


