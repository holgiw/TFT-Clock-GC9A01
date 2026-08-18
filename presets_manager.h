#pragma once
    // ### Presets: Laden/Speichern/Wechseln vordefinierter Anzeigekonfigurationen
    // Benoetigt globals.h, config.h, prefs_keys.h und declarations.h (werden
    // zentral in uhr3.ino VOR dieser Datei eingebunden).

    // ### Presets: load/save/switch predefined display configurations
    // Requires globals.h, config.h, prefs_keys.h and declarations.h (included
    // centrally in uhr3.ino BEFORE this file).


    // Entfernt ein "rotation=..."-Query-Parameter aus einer Preset-URL (Altlast aus
    // frueheren Versionen, die die Display-Rotation faelschlich mit im Preset
    // gespeichert haben - siehe switchToNextPreset() und createPresetFromPreferences()).

    // Removes a "rotation=..." query parameter from a preset URL (legacy from
    // older versions that mistakenly stored the display rotation in the preset -
    // see switchToNextPreset() and createPresetFromPreferences()).

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

            // Altlast bereinigen: frueher gespeicherte "rotation="-Parameter entfernen
            // und dauerhaft korrigieren (nur bei Aenderung schreiben, Flash-Verschleiss).

            // Clean up legacy data: remove previously stored "rotation=" parameters
            // and fix permanently (only write on change, to limit flash wear).
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

            // Nur schreiben, wenn sich der Wert tatsaechlich geaendert hat - das
            // Formular sendet immer alle MAX_PRESETS Eintraege mit, auch wenn nur
            // einer davon bearbeitet wurde (vermeidet unnoetigen Flash-Verschleiss).

            // Only write if the value actually changed - the form always submits
            // all MAX_PRESETS entries, even if only one was edited (avoids
            // unnecessary flash wear).
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
        uint8_t hubSize = preferences.getUInt(PK_CENTER_SIZE, 6);
        uint32_t hubColor = preferences.getLong(PK_CENTER_COLOR, 0xEC0016);

        // Hole die aktuelle IP-Adresse des ESP
        // Get the ESP's current IP address


        // Erstelle die URL mit den aktuellen Einstellungen (bewusst OHNE "rotation":
        // geraeteweite Hardware-Einstellung, soll beim Laden unveraendert bleiben).

        // Build the URL with the current settings (deliberately WITHOUT "rotation":
        // a device-wide hardware setting that must stay unchanged when loading).
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


    // Parst die in einem Preset gespeicherte Query-String-URL und extrahiert NUR
    // die fuer eine Vorschau relevanten Werte (face, handSet, hubColor, hubSize,
    // showSecondHand) - reine Lesefunktion, wendet nichts auf die Preferences an.

    // Parses the query-string URL stored in a preset and extracts ONLY the
    // values relevant for a preview (face, handSet, hubColor, hubSize,
    // showSecondHand) - read-only, does not apply anything to preferences.

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


    // Entfernt alle Presets, deren Zifferblatt bzw. Zeigersatz mit dem angegebenen
    // Wert uebereinstimmt (leer = nicht geprueft) - wird nach dem Loeschen einer
    // Zifferblatt-/Zeigersatz-Datei aufgerufen, damit keine Presets verwaisen.

    // Removes all presets whose face or hand set matches the given value
    // (empty = not checked) - called after deleting a face/hand-set file so
    // no presets are left orphaned.

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


    // --- Funktion: Wechselt zum nächsten Preset ---
    // --- Function: switches to the next preset ---

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

        // Speichere den aktuellen Preset-Namen
        // Save the current preset name
        preferences.putString(PK_CURRENT_PRESET, presets[nextPresetIndex].name);

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
                setupNTP();
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
                // Bewusst NICHT angewendet: Display-Rotation soll beim Laden eines
                // Presets unveraendert bleiben (aeltere Presets koennen den Wert
                // noch enthalten - wird hier absichtlich ignoriert).

                // Deliberately NOT applied: display rotation must stay unchanged when
                // loading a preset (older presets may still contain the value -
                // intentionally ignored here).
            }
            else if (key == "showSecondHand") {
                showSecondHand = (value == "1" || value.equalsIgnoreCase("true"));
                preferences.putBool(PK_SHOW_SECOND_HAND, showSecondHand);
            }
            else if (key == "smoothMinute") {
                smoothMinute = (value == "1" || value.equalsIgnoreCase("true"));
                preferences.putBool(PK_SMOOTH_MINUTE, smoothMinute);
            }
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


