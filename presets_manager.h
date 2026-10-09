#pragma once

    // Presets: Laden/Speichern/Wechseln der Anzeigekonfigurationen (benoetigt globals.h, config.h,
    // prefs_keys.h, declarations.h). Entfernt einen alten "rotation="-Parameter aus einer Preset-URL (Altlast
    // frueherer Versionen).

    // Presets: load/save/switch display configurations (requires globals.h, config.h, prefs_keys.h,
    // declarations.h). Removes a legacy "rotation=" parameter from a preset URL (leftover from older
    // versions).

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


    // Preset-Werte URL-kodieren/-dekodieren: /api/setMode dekodiert per webserver.arg(), die Preset-Vorschau per
    // presetUrlDecode() - so kommt z.B. die Zeitzone "<+09>-9" unveraendert an. Kodiert wird alles ausser
    // A-Z a-z 0-9 - _ . , / :

    // URL-encode/-decode preset values: /api/setMode decodes via webserver.arg(), the preset preview via
    // presetUrlDecode() - so e.g. the time zone "<+09>-9" arrives unchanged. Everything except
    // A-Z a-z 0-9 - _ . , / : is encoded.

    String presetUrlEncode(const String& value) {
        static const char hex[] = "0123456789ABCDEF";
        String out;
        out.reserve(value.length() * 3);
        for (size_t i = 0; i < value.length(); i++) {
            uint8_t c = (uint8_t)value[i];
            if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == ',' || c == '/' || c == ':') {
                out += (char)c;
            }
            else {
                out += '%';
                out += hex[c >> 4];
                out += hex[c & 0x0F];
            }
        }
        return out;
    }


    String presetUrlDecode(const String& value) {
        String out;
        out.reserve(value.length());
        for (size_t i = 0; i < value.length(); i++) {
            char c = value[i];
            if (c == '+') {
                out += ' ';
            }
            else if (c == '%' && i + 2 < value.length() && isxdigit((uint8_t)value[i + 1]) && isxdigit((uint8_t)value[i + 2])) {
                char hexPair[3] = { value[i + 1], value[i + 2], 0 };
                out += (char)strtol(hexPair, nullptr, 16);
                i += 2;
            }
            else {
                out += c;
            }
        }
        return out;
    }


    // Preset-URL: Zifferblatt, Zeiger, Modi und Nabe wie uebergeben, Zeitzone, Rocrail, WLAN neu verbinden,
    // Helligkeit aus den aktuellen Einstellungen - gemeinsam fuer createPresetFromPreferences() und
    // addStarterPresets(). Der Streifen gehoert zum Zifferblatt (stripcfg_*.txt), nicht zum Preset.

    // Preset URL: clock face, hands, modes and hub as passed, time zone, Rocrail, reconnect WiFi, brightness
    // from the current settings - shared by createPresetFromPreferences() and addStarterPresets(). The strip
    // belongs to the clock face (stripcfg_*.txt), not to the preset.

    String buildPresetUrl(const String& face, const String& handSet, bool stationMode, bool showSecondHand,
                          bool smoothMinute, bool smoothSecond, uint8_t hubSize, uint32_t hubColor) {

        // URL bewusst OHNE "rotation": geraeteweite HW-Einstellung, bleibt beim Laden unveraendert.
        // Build URL deliberately WITHOUT "rotation": device-wide HW setting, stays unchanged when loading.

        String url = "http://" + ipAddress + "/api/setMode?";
        url += "face=" + (face.startsWith("/") ? face.substring(1) : face);
        url += "&handSet=" + handSet;

        url += "&stationMode=" + String(stationMode ? "true" : "false");
        url += "&showSecondHand=" + String(showSecondHand ? "true" : "false");
        url += "&smoothMinute=" + String(smoothMinute ? "true" : "false");
        url += "&smoothSecond=" + String(smoothSecond ? "true" : "false");
        url += "&hubSize=" + String(hubSize);
        url += "&hubColor=" + String(hubColor, HEX);

        // Zeitzone - z.B. fuer Weltzeit-Presets (eigenes Zifferblatt + eigene
        // Zone). Kodiert, da POSIX-Zonen '+', '<', '>' enthalten koennen.

        // Time zone - e.g. for world-time presets (own clock face + own zone).
        // Encoded, since POSIX zones may contain '+', '<', '>'.

        url += "&timeZone=" + presetUrlEncode(preferences.getString(PK_TIMEZONE, TIMEZONE_DEFAULT));

        // Rocrail-Modellzeit und "WLAN neu verbinden" wie gerade eingestellt - ohne Serveradressen und
        // WLAN-Daten (Geraeteeinstellungen)

        // Rocrail model time and "Reconnect WiFi" as currently set - without server addresses and WiFi
        // credentials (device settings)

        url += "&rocrail=" + String(rocrailEnabled ? "true" : "false");
        url += "&wifiReconnect=" + String(wifiActive ? "true" : "false");

        // Helligkeit (siehe applyBrightnessPresetValue() in display.h) - bewusst OHNE useBacklight, das
        // haengt an der Verdrahtung. Aeltere Presets ohne diese Werte lassen die Helligkeit unveraendert.

        // Brightness (see applyBrightnessPresetValue() in display.h) - deliberately WITHOUT useBacklight,
        // that depends on the wiring. Older presets without these values leave brightness unchanged.

        url += "&minBrightness=" + String(preferences.getUChar(PK_MIN_BRIGHTNESS, 100));
        url += "&maxBrightness=" + String(preferences.getUChar(PK_MAX_BRIGHTNESS, 255));
        url += "&brightStart=" + String(preferences.getUChar(PK_BRIGHT_START_HOUR, 7));
        url += "&brightEnd=" + String(preferences.getUChar(PK_BRIGHT_END_HOUR, 21));
        url += "&lowThreshold=" + String(preferences.getInt(PK_LOW_THRESHOLD, 40));
        url += "&highThreshold=" + String(preferences.getInt(PK_HIGH_THRESHOLD, 60));
        url += "&gamma=" + String(preferences.getFloat(PK_GAMMA_BRIGHTNESS, 2.2f), 1);
        url += "&autoBrightness=" + String(useAdc ? "true" : "false");

        return url;
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
        // Kommentar bei der smoothSecond-Ladezeile in uhr4.ino.

        // Fallback deliberately stationMode instead of a fixed literal - see
        // the comment at the smoothSecond load line in uhr4.ino.

        bool smoothSecond = getSmoothSecondPref(stationMode);
        uint8_t hubSize = preferences.getUInt(PK_CENTER_SIZE, 6);
        uint32_t hubColor = preferences.getLong(PK_CENTER_COLOR, 0xEC0016);

        String url = buildPresetUrl(background, handset, stationMode, showSecondHand, smoothMinute, smoothSecond, hubSize, hubColor);

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

        // Namen eindeutig halten - sonst waeren zwei Presets in der Liste nicht zu unterscheiden, Suffix anhaengen
        // Keep names unique - otherwise two presets would be indistinguishable in the list, append a suffix

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


    // Setzt in einer Preset-URL den Wert eines Parameters (haengt ihn an, falls er fehlt)
    // Sets a parameter's value in a preset URL (appends it if missing)

    String setPresetUrlParam(const String& url, const String& key, const String& value) {
        int q = url.indexOf('?');
        if (q < 0) return url + "?" + key + "=" + value;
        int pos = url.indexOf("?" + key + "=", q);
        if (pos < 0) pos = url.indexOf("&" + key + "=", q);
        if (pos < 0) return url + "&" + key + "=" + value;
        int start = pos + 1 + key.length() + 1;
        int end = url.indexOf('&', start);
        return url.substring(0, start) + value + (end < 0 ? String("") : url.substring(end));
    }

    // Zeigersatz-Nummern in allen Uhren Sets (Parameter handSet) und im aktiven Satz durch fn ersetzen. Gibt die
    // Zahl der geaenderten Uhren Sets zurueck.

    // Replace hand set numbers in all presets (parameter handSet) and in the active set through fn. Returns the
    // number of changed presets.

    int rewriteHandSetRefs(const std::function<String(const String&)>& fn) {
        int changed = 0;
        for (int i = 0; i < MAX_PRESETS; i++) {
            String url = preferences.getString(pkPresetUrl(i).c_str(), "");
            int q = url.indexOf('?');
            if (q < 0) continue;
            int pos = url.indexOf("?handSet=", q);
            if (pos < 0) pos = url.indexOf("&handSet=", q);
            if (pos < 0) continue;
            int start = pos + 9, end = url.indexOf('&', start);
            String value = url.substring(start, end < 0 ? url.length() : end);
            String moved = fn(value);
            if (moved != value) {
                preferences.putString(pkPresetUrl(i).c_str(), setPresetUrlParam(url, "handSet", moved));
                changed++;
            }
        }
        String active = preferences.getString(PK_HANDSET, "");
        String movedActive = fn(active);
        if (movedActive != active) preferences.putString(PK_HANDSET, movedActive);
        return changed;
    }

    // Neunummerierung zu Ende fuehren. Plan "Stufe|alt>neu,...": 1 Uhren Sets und Sekundenfeld-Nummer auf "~neu",
    // 2 Dateien auf "hand_set^neu_...", 3 auf den neuen Namen, 4 "~" entfernen. Jede Stufe ist wiederholbar.

    // Finish the renumbering. Plan "stage|old>new,...": 1 presets and subdial number to "~new", 2 files to
    // "hand_set^new_...", 3 to the new name, 4 remove "~". Every stage can be repeated.

    void finishHandSetRenumber(String plan) {
        int stage = plan.substring(0, 1).toInt();
        std::vector<std::pair<String, String>> pairs;
        for (int p = 2; p < (int)plan.length();) {
            int comma = plan.indexOf(',', p);
            String item = plan.substring(p, comma < 0 ? plan.length() : comma);
            int arrow = item.indexOf('>');
            if (arrow > 0) pairs.push_back({ item.substring(0, arrow), item.substring(arrow + 1) });
            if (comma < 0) break;
            p = comma + 1;
        }
        auto setStage = [&](int s) {
            plan = String(s) + plan.substring(1);
            preferences.putString(PK_RENUM_PLAN, plan);
        };
        const char* parts[] = { "hour", "minute", "second" };

        if (stage <= 1) {
            rewriteHandSetRefs([&](const String& v) -> String {
                for (const auto& pr : pairs) if (pr.first == v) return String("~" + pr.second);
                return v;
            });
            String sub = preferences.getString(PK_SUBDIAL_SET, "");
            for (const auto& pr : pairs) {
                if (pr.first == sub) { preferences.putString(PK_SUBDIAL_SET, String("~") + pr.second); break; }
            }
            setStage(2);
        }
        if (stage <= 2) {
            for (const auto& pr : pairs) {
                for (const char* part : parts) {
                    String from = "/hand_set" + pr.first + "_" + part + ".bmp";
                    String temp = "/hand_set^" + pr.second + "_" + part + ".bmp";
                    if (!LittleFS.exists(from)) continue;
                    LittleFS.remove(temp);
                    LittleFS.rename(from, temp);
                }
            }
            setStage(3);
        }
        if (stage <= 3) {
            for (const auto& pr : pairs) {
                for (const char* part : parts) {
                    String temp = "/hand_set^" + pr.second + "_" + part + ".bmp";
                    String to = "/hand_set" + pr.second + "_" + part + ".bmp";
                    if (!LittleFS.exists(temp)) continue;
                    LittleFS.remove(to);
                    LittleFS.rename(temp, to);
                }
            }
            setStage(4);
        }
        rewriteHandSetRefs([](const String& v) -> String { return v.startsWith("~") ? v.substring(1) : v; });
        String sub = preferences.getString(PK_SUBDIAL_SET, "");
        if (sub.startsWith("~")) preferences.putString(PK_SUBDIAL_SET, sub.substring(1));
        preferences.remove(PK_RENUM_PLAN);
        loadPresets();
        DEBUG_PRINTLN("[Starter] Hand sets renumbered: " + String(pairs.size()) + " moved");
    }

    // Beim Start VOR ensureStarterSet(): eine unterbrochene Neunummerierung (Stromausfall) zu Ende fuehren
    // At boot BEFORE ensureStarterSet(): finish an interrupted renumbering (power cut)

    void recoverHandSetRenumber() {
        String plan = preferences.getString(PK_RENUM_PLAN, "");
        if (plan.length() > 2) finishHandSetRenumber(plan);
        else if (plan.length()) preferences.remove(PK_RENUM_PLAN);
    }

    // Beim Start NACH ensureStarterSet(): Zeigersaetze fortlaufend nummerieren. 0, 1, 2 bleiben, der Sekundenfeld-
    // Satz wird 3, die uebrigen folgen in ihrer Reihenfolge ab 4; Uhren Sets und aktiver Satz ziehen mit.

    // At boot AFTER ensureStarterSet(): number the hand sets consecutively. 0, 1, 2 stay, the subdial set
    // becomes 3, the others follow in their order from 4; presets and active set follow.

    void renumberHandSets() {
        std::set<int> numbers;
        File root = LittleFS.open("/");
        for (File f = root.openNextFile(); f; f = root.openNextFile()) {
            String n = f.name();
            if (f.isDirectory() || !n.startsWith("hand_set") || !n.endsWith(".bmp")) continue;
            int us = n.indexOf('_', 8);
            if (us <= 8) continue;
            String id = n.substring(8, us);
            int num = id.toInt();
            if (num >= 3 && String(num) == id) numbers.insert(num); // nur gewoehnliche Zahlen, Namen und "07" bleiben
                                                                    // only plain numbers, names and "07" stay
        }
        root.close();

        int sub = subdialHandSet().toInt();
        std::vector<int> order;
        if (numbers.count(sub)) order.push_back(sub);
        for (int n : numbers) if (n != sub) order.push_back(n);

        String plan;
        int target = 3;
        for (int n : order) {
            if (n != target) plan += String(plan.length() ? "," : "") + String(n) + ">" + String(target);
            target++;
        }
        if (plan.isEmpty()) return;
        plan = "1|" + plan;
        preferences.putString(PK_RENUM_PLAN, plan);
        finishHandSetRenumber(plan);
    }

    // Kleinste und groesste vorhandene Groesse einer Streifen-Schrift aus dem Designer (stripfont_<Name>_<Groesse>.vlw)
    // Smallest and largest available size of a strip font from the designer (stripfont_<name>_<size>.vlw)

    bool stripVlwSizes(const char* name, int& minSize, int& maxSize) {
        String prefix = "stripfont_" + String(name) + "_";
        minSize = 0;
        maxSize = 0;
        File root = LittleFS.open("/");
        for (File f = root.openNextFile(); f; f = root.openNextFile()) {
            String n = f.name();
            if (f.isDirectory() || !n.startsWith(prefix) || !n.endsWith(".vlw") || n.endsWith("_wd.vlw")) continue;
            int size = n.substring(prefix.length(), n.length() - 4).toInt();
            if (size <= 0) continue;
            if (!minSize || size < minSize) minSize = size;
            if (size > maxSize) maxSize = size;
        }
        root.close();
        return maxSize > 0;
    }

    // Einmalig: Streifen-Einstellungen von vor 2026-10-08 (Preferences fuer alle Zifferblaetter, Streifen-Werte
    // in Presets) als stripcfg_*.txt zu den Zifferblaettern - erst das aktive aus den Preferences, dann je Preset
    // sein Zifferblatt, solange es noch keine Datei hat. Laeuft in setup() nach loadPresets().

    // Once: strip settings from before 2026-10-08 (preferences for all clock faces, strip values in presets) as
    // stripcfg_*.txt for the clock faces - first the active one from the preferences, then per preset its clock
    // face as long as it has no file yet. Runs in setup() after loadPresets().

    void migrateStripSettings() {
        if (TFT_HEIGHT <= CLOCK_HEIGHT || preferences.getBool(PK_STRIP_CFG_DONE, false)) return;
        String active = stripConfigPath(selectedBackground);
        if (active.length() && !LittleFS.exists(active) && loadLegacyStripPrefs()) saveStripSettingsForFace(selectedBackground);
        for (int i = 0; i < MAX_PRESETS; i++) {
            const String& url = presets[i].url;
            int f = url.indexOf("face=");
            if (f < 0 || url.indexOf("stripBg=") < 0) continue;
            int end = url.indexOf('&', f);
            String face = "/" + presetUrlDecode(url.substring(f + 5, end < 0 ? url.length() : end));
            String cfg = stripConfigPath(face);
            if (!cfg.length() || LittleFS.exists(cfg) || !LittleFS.exists(face)) continue;
            loadLegacyStripPrefs();
            applyStripQuery(url);
            saveStripSettingsForFace(face);
        }
        preferences.putBool(PK_STRIP_CFG_DONE, true);
        stripSettingsFor = "?";
        DEBUG_PRINTLN("[Strip] Settings moved to the clock faces (stripcfg_*.txt)");
    }

    // Verwaiste Dateien eines Zifferblatts loeschen: strip_<Name>.bmp, stripcfg_<Name>.txt und facecfg_<Name>.txt
    // ohne face_<Name>.bmp (z.B. nach Loeschen ueber den Dateimanager). Laeuft in setup() nach migrateStripSettings().

    // Delete orphaned files of a clock face: strip_<name>.bmp, stripcfg_<name>.txt and facecfg_<name>.txt without
    // face_<name>.bmp (e.g. after deleting via the file manager). Runs in setup() after migrateStripSettings().

    void removeOrphanedStrips() {
        std::vector<String> orphans;
        File root = LittleFS.open("/");
        for (File f = root.openNextFile(); f; f = root.openNextFile()) {
            String n = f.name();
            String face;
            if (n.startsWith("strip_") && n.endsWith(".bmp")) face = "/face_" + n.substring(6);
            else if (n.startsWith("stripcfg_") && n.endsWith(".txt")) face = "/face_" + n.substring(9, n.length() - 4) + ".bmp";
            else if (n.startsWith("facecfg_") && n.endsWith(".txt")) face = "/face_" + n.substring(8, n.length() - 4) + ".bmp";
            if (face.length() && !f.isDirectory()) orphans.push_back(face + "|/" + n);
        }
        root.close();
        for (const String& o : orphans) {
            int sep = o.indexOf('|');
            if (LittleFS.exists(o.substring(0, sep))) continue;
            LittleFS.remove(o.substring(sep + 1));
            DEBUG_PRINTLN("[Strip] Removed orphaned " + o.substring(sep + 1));
        }
    }

    // Legt die vier Start-Presets zu den erzeugten Zifferblaettern und Zeigersaetzen an - jedes nur, wenn seine
    // Dateien da sind und kein Preset gleichen Namens existiert. Bahnhofsmodus, Nabe rot bzw. schwarz, roemisch
    // ohne Sekundenzeiger, Sekundenfeld mit eigenem Satz, mit Streifen weiss/schwarz in eigener Schrift; sonst wie die
    // Einstellungen.

    // Creates the four starter presets for the generated clock faces and hand sets - each only if its files
    // exist and there is no preset with the same name. Station mode, hub red or black, roman without a second
    // hand, subdial with its own set, with a strip white/black in its own font; otherwise as the current settings.

    void addStarterPresets() {
        struct { const char* name; const char* face; int style; bool second; uint32_t hubColor; const char* stripFont; const char* vlw; } starter[] = {
            { "Standard", "face_default.bmp", 0, true, 0xEC0016, "FreeSans Bold", nullptr },
            { "1-12", "face_numbers.bmp", 1, true, 0xEC0016, "DejaVu", nullptr },
            { "I-XII", "face_roman.bmp", 2, false, 0x000000, "GLCD", "Sans" },
            { "Sekundenfeld", "face_subdial.bmp", 3, true, 0x000000, "FreeSans Bold", nullptr },
        };
        bool changed = false;
        for (const auto& s : starter) {
            String handSet = s.style == 3 ? subdialHandSet() : String(s.style);
            if (!LittleFS.exists("/" + String(s.face)) || !LittleFS.exists("/hand_set" + handSet + "_hour.bmp")) continue;
            int freeSlot = -1;
            bool exists = false;
            for (int i = 0; i < MAX_PRESETS && !exists; i++) {
                exists = presets[i].name == s.name;
                if (freeSlot < 0 && presets[i].name.isEmpty() && presets[i].url.isEmpty()) freeSlot = i;
            }
            if (exists || freeSlot < 0) continue;
            presets[freeSlot].name = s.name;
            String url = buildPresetUrl(s.face, handSet, true, s.second, false, true, displayGeom->centerSize, s.hubColor);

            // Displays mit Streifen (ILI9341, ST7789 172x320): Streifen-Einstellungen des Zifferblatts, falls es noch
            // keine hat - weisser Streifen, schwarze Schrift, je Zifferblatt eine eingebaute Schrift

            // Displays with a strip (ILI9341, ST7789 172x320): strip settings of the clock face if it has none
            // yet - white strip, black text, one built-in font per clock face

            String cfg = stripConfigPath("/" + String(s.face));
            if (TFT_HEIGHT > CLOCK_HEIGHT && cfg.length() && !LittleFS.exists(cfg)) {
                String saved = stripSettingsQuery();
                resetStripSettings();
                for (uint8_t f = 0; f < STRIP_FONT_COUNT; f++) {
                    if (strcmp(STRIP_FONTS[f].name, s.stripFont) == 0) stripFont = f;
                }

                // Eigene Designer-Schrift (VLW), falls auf der Uhr vorhanden: groesste Datei fuer die Uhrzeit,
                // kleinste fuers Datum - sonst bleibt die eingebaute Schrift

                // Own designer font (VLW) if present on the clock: largest file for the time, smallest for the
                // date - otherwise the built-in font stays

                int vlwMin = 0, vlwMax = 0;
                if (s.vlw && stripVlwSizes(s.vlw, vlwMin, vlwMax)) {
                    stripFont = STRIP_FONT_VLW;
                    stripVlwName = s.vlw;
                    stripVlwTimeSize = vlwMax;
                    stripVlwDateSize = vlwMin;
                }
                saveStripSettingsForFace("/" + String(s.face));
                resetStripSettings();
                applyStripQuery(saved);
                stripSettingsFor = "?";
            }
            presets[freeSlot].url = url;
            changed = true;
        }
        if (changed) savePresets();
    }


    // Startpaket: fehlende Zifferblaetter, Zeigersaetze und Uhren Sets des Startpakets bei jedem Start erzeugen -
    // vorhandene bleiben. Ein Zeigersatz gilt als vorhanden, sobald eine seiner Dateien da ist (eigene Saetze mit
    // gleicher Nummer werden nicht ergaenzt). Laeuft in setup() VOR loadClockFace() (laedt die Presets dafuer
    // schon einmal, setup() laedt sie spaeter mit der IP neu).

    // Starter set: create missing clock faces, hand sets and presets of the starter set at every start - existing
    // ones stay. A hand set counts as present as soon as one of its files is there (own sets with the same number
    // are not completed). Runs in setup() BEFORE loadClockFace() (loads the presets once for this, setup()
    // reloads them later with the IP).

    void ensureStarterSet() {
        for (const auto& f : STARTER_FACES) {
            if (!LittleFS.exists(f.path)) writeGeneratedFace(f.path, f.numerals);
        }
        ensureDefaultHands();
        for (int style = 1; style <= 3; style++) {
            String id = style == 3 ? subdialHandSet() : String(style);
            if (!handSetExists(id)) writeGeneratedHandSet(style, id);
        }
        loadPresets();
        addStarterPresets();
    }


    // Erzeugte Zifferblaetter und Zeigersaetze im Mass eines anderen Displaytyps neu erzeugen (z.B. erster Start
    // als 1,47" und danach auf 1,3" umgestellt) - in falscher Groesse kann die Uhr sie nicht anzeigen und zeichnet
    // sonst ersatzweise Satz 0. Dateien in passender Groesse bleiben unangetastet.

    // Regenerate generated clock faces and hand sets in the size of another display type (e.g. first start as
    // 1.47" and then switched to 1.3") - in the wrong size the clock cannot show them and otherwise draws set 0
    // as a substitute. Files of the right size stay untouched.

    void refreshGeneratedAssets() {
        for (const auto& f : STARTER_FACES) {
            int32_t w, h;
            if (!LittleFS.exists(f.path)) continue;
            if (readImageSize(f.path, w, h) && w == CLOCK_WIDTH && h == CLOCK_HEIGHT) continue;
            DEBUG_PRINTLN(String("[Starter] Wrong size, regenerating ") + f.path);
            LittleFS.remove(f.path);
            writeGeneratedFace(f.path, f.numerals);
        }
        for (int style = 0; style <= 3; style++) {
            String id = style == 3 ? subdialHandSet() : String(style);
            bool wrongSize = false;
            for (const char* part : { "hour", "minute", "second" }) {
                String path = "/hand_set" + id + "_" + part + ".bmp";
                int32_t w, h;
                if (LittleFS.exists(path) && !(readImageSize(path.c_str(), w, h) && isValidHandSize(w, h))) wrongSize = true;
            }
            if (!wrongSize) continue;
            DEBUG_PRINTLN("[Starter] Wrong size, regenerating hand set " + id);
            for (const char* part : { "hour", "minute", "second" }) LittleFS.remove("/hand_set" + id + "_" + part + ".bmp");
            writeGeneratedHandSet(style, id);
        }
    }
