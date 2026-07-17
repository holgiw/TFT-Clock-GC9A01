#pragma once
// ####################################################################
// ### Webinterface: alle HTTP-Routen & HTML-Generierung
// ####################################################################
// Benoetigt globals.h, config.h, prefs_keys.h und declarations.h (werden
// zentral in uhr3.ino VOR dieser Datei eingebunden).

// Generiert den HTML-Header für die Weboberfläche
String generateHtmlHeader() {
    String html = "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
    html.reserve(512);  // Header: klein, wird auf jeder Seite einmal aufgerufen
    html += "<style>body{font-family:Arial;text-align:center;}input,select,button{margin:10px;padding:10px;width:80%;}";
    html += "h1 { color: #333333; } ";
    html += "hr { border: 0; height: 1px; background-color: #cccccc; margin: 20px 0; } ";
    html += "table { margin: auto; border-collapse: collapse; } "; // Tabellen zentrieren
    html += "th, td { padding: 10px; text-align: center; border: 1px solid #cccccc; } "; // Tabellenzellen 
    html += "li { text-align: left; } "; // <li> linksbündig formatieren
    html += "</style></head><body>";
    // Seite benötigt JavaScript
    html += "<noscript><div style='color:red;font-weight:bold;margin:20px;'>" + 
            translate("JavaScript is disabled.This page requires JavaScript to work properly!") + "</div></noscript>";
    return html;
}


/// Generiert den HTML-Statusabschnitt für die Weboberfläche
String generateHtmlStatus() {
    setLedOn();
    size_t total = LittleFS.totalBytes();
    size_t used = LittleFS.usedBytes();
    String html;
    html.reserve(512);  // Statusleiste: klein
    if (WiFi.getMode() == WIFI_STA) {
        html = translate("Connected to") + ": <strong>" + WiFi.SSID() + "</strong>";
        html += "<br>" + translate("IP Address") + ": <strong>" + "<a href='http://" +  + "'>http://" + ipAddress +"</a></strong> ";
        if (pingHostname)  html += "<br>" + translate("Hostname") + ": <strong>" + "<a href='http://" + hostname + ".local'>http://" + hostname + ".local</a>" + "</strong>";
    }
    else html = "<br>Access Point: <strong>" + String(WiFi.softAPSSID()) + "</strong> (" + WiFi.softAPIP().toString() + ")";

    html += "<br>" + translate("Storage used") + ": " + String(used / 1024) + " KB / " + String(total / 1024) + " KB";
    html += " (" + translate("Free") + ": " + String((total - used) / 1024) + " KB)<hr>";
    setLedOff();
    return html;
}


// Navigationsleiste generieren
String generateNavigation() {
 /*   if (WiFi.getMode() != WIFI_STA) {
        DEBUG_PRINTLN("[HTML] Skipping HTML navigation");
        return "";
    }
    */

    String nav = "<style>";
    nav += "a { text-decoration: underline; color: blue; font-weight: bold; }";
    nav += "a:hover { text-decoration: underline; }";
    nav += "</style>";
    nav += "<div style='text-align:center; margin-bottom:20px;'>";

    const struct NavItem {
        String path;
        String label;
        String confirmMessage; // Optional: Bestätigungsnachricht
    } navItems[] = {
        {"/", translate("Main"), ""},
        {"/presets", translate("Presets"), ""},
        {"/listfilesFaces", translate("Clock&nbsp;Face"), ""},
        {"/handsets", translate("Hand&nbsp;Set"), ""},        
        {"/timezone_form", translate("NTP&nbsp;Timezone"), ""},
        {"/brightness", translate("Brightness"), ""},
        {"/status", "Status", ""},
        {"/files", translate("<br>File&nbsp;Manager"), ""},
        {"/reboot", translate("Reboot"), translate("Are you sure you want to reboot?")},
        {"/factoryReset", translate("Factory&nbsp;Reset"), translate("Are you sure you want to reset to factory settings?")}
    };

    String currentPath = webserver.uri(); // Aktueller Pfad der Seite

    for (const auto& item : navItems) {
        if (item.path == currentPath) {
            // Wenn der aktuelle Pfad mit dem Navigationseintrag übereinstimmt, nur Text anzeigen
            nav += "<span style=\"margin-right:15px; font-weight:bold;\">" + item.label + "</span> ";
        }
        else {
            // Andernfalls als Link anzeigen
            nav += "<a href=\"" + item.path + "\" style=\"margin-right:15px;\"";
            if (!item.confirmMessage.isEmpty()) {
                nav += " onclick=\"return confirm('" + item.confirmMessage + "')\"";
            }
            nav += ">" + item.label + "</a> ";
        }
    }

    nav += "</div>";
    
    return nav;
}


// Sprachselector generieren
String generateLanguageSelector() {
    String html = "<form method='POST' action='/setLanguage'>";
    html.reserve(512);  // Sprachauswahl: klein
    html += "<label for='lang'>Language/Sprache:</label>";
    html += "<select name='lang'>";
    html += "<option value='en'" + String(currentLanguage == "en" ? " selected" : "") + ">Englisch / English</option>";
    html += "<option value='de'" + String(currentLanguage == "de" ? " selected" : "") + ">Deutsch / German</option>";
    html += "<option value='fr'" + String(currentLanguage == "fr" ? " selected" : "") + ">Franz&ouml;sisch / Fran&ccedil;ais</option>";
    html += "</select>";
    html += "<button type='submit'>" + translate("save") + "</button>";
    html += "</form><hr>";
    return html;
}


// Vergleicht zwei Dateinamen "natuerlich": zusammenhaengende Ziffernfolgen
// werden als Zahl verglichen statt zeichenweise-alphabetisch, damit z.B.
// "hand_set2_hour.bmp" vor "hand_set10_hour.bmp" einsortiert wird (bei
// reinem String-Vergleich waere "10" faelschlich vor "2").
bool naturalLess(const String& a, const String& b) {
    unsigned int i = 0, j = 0;
    while (i < a.length() && j < b.length()) {
        char ca = a[i], cb = b[j];
        if (isDigit(ca) && isDigit(cb)) {
            unsigned int si = i, sj = j;
            while (i < a.length() && isDigit(a[i])) i++;
            while (j < b.length() && isDigit(b[j])) j++;
            String numA = a.substring(si, i);
            String numB = b.substring(sj, j);
            long valA = numA.toInt();
            long valB = numB.toInt();
            if (valA != valB) return valA < valB;
            if (numA != numB) return numA < numB; // z.B. fuehrende Nullen als Tiebreaker
            continue;
        }
        if (ca != cb) {
            char lca = tolower(ca);
            char lcb = tolower(cb);
            if (lca != lcb) return lca < lcb;
            return ca < cb; // bei gleichem Buchstaben unterschiedlicher Groesse: Grossbuchstabe zuerst (stabiler Tiebreaker)
        }
        i++; j++;
    }
    return (a.length() - i) < (b.length() - j);
}


// Sortiert eine Liste von Dateinamen "natuerlich" (siehe naturalLess()).
// Einfacher Insertion-Sort statt std::sort, um keine zusaetzliche
// <algorithm>-Abhaengigkeit zu benoetigen - bei der ueberschaubaren
// Dateianzahl auf LittleFS ist die O(n^2)-Laufzeit unerheblich.
void naturalSortNames(std::vector<String>& names) {
    for (size_t i = 1; i < names.size(); i++) {
        String key = names[i];
        long j = (long)i - 1;
        while (j >= 0 && naturalLess(key, names[j])) {
            names[j + 1] = names[j];
            j--;
        }
        names[j + 1] = key;
    }
}


// Webserver-API-Endpunkte einrichten
void setupWebServer() {

    // API to set clock face, hand set, timezone, hub size/color, station mode, rotation, second hand visibility, and smooth minute hand

    // DB
    // http://192.168.0.214/api/setMode?face=face_db_uhr.bmp&handSet=0&hubSize=6&hubColor=ff0000showSecondHand=1&stationMode=true&smoothMinute=false&rotation=2

    // Irish Pub
    // http://192.168.0.214/api/setMode?face=face_irish_pub.bmp&handSet=0&hubSize=2&hubColor=aaaaaa&showSecondHand=false&stationMode=false&smoothMinute=true&rotation=2



    // API zum Zurücksetzen der WiFi-Einstellungen
    // http://192.168.0.214/api/resetWiFi  
   
    webserver.on("/setLanguage", HTTP_POST, []() {
        if (webserver.hasArg("lang")) {
            String lang = webserver.arg("lang");
            if (lang == "en") {
                saveLanguage(lang);
                // webserver.send(200, "text/plain", "Language updated to " + lang);
                webserver.sendHeader("Location", "/", true);
                webserver.send(302, "text/plain", "");
                return;
            }

            if (availableLanguages.count(lang)) {
                saveLanguage(lang);
                // webserver.send(200, "text/plain", "Language updated to " + lang);
                webserver.sendHeader("Location", "/", true);
                webserver.send(302, "text/plain", "");
                return;
            }
            else {
                webserver.send(400, "text/plain", "Invalid language");
            }
        }
        else {
            webserver.send(400, "text/plain", "Missing 'lang' parameter");
        }
        });

    webserver.on("/api/resetWiFi", HTTP_GET, []() {

        DEBUG_PRINTLN("[API] Received GET request to /api/resetWiFi, resetting WiFi settings..");
               
        eraseWiFiConfig();

        // Sende eine Bestätigung zurück
        webserver.send(200, "application/json", "{\"status\":\"WiFi settings reset successfully\"}");
        DEBUG_PRINTLN("[API] WiFi settings reset via /api/resetWiFi");

        delay(WAIT_1s);
        // Neustart des ESP
        espReboot();

        });

    // resetWifi POST API, um WiFi-Einstellungen zurückzusetzen
    webserver.on("/api/resetWiFi", HTTP_POST, []() {
        DEBUG_PRINTLN("[API] Received POST request to /api/resetWiFi, resetting WiFi settings..");
       
        eraseWiFiConfig();
        // Sende eine Bestätigung zurück
        webserver.send(200, "application/json", "{\"status\":\"WiFi settings reset successfully\"}");
        DEBUG_PRINTLN("[API] WiFi settings reset via /api/resetWiFi");

        preferences.end(); // Schließe die Preferences, um sicherzustellen, dass alle Änderungen gespeichert werden
        delay(WAIT_1s);
        // Neustart des ESP
        espReboot();
        });

    webserver.on("/api/createPreset", HTTP_POST, []() {
        createPresetFromPreferences(); // Ruft die Funktion auf, um ein neues Preset zu erstellen

        // Weiterleitung zur Presets-Seite
        webserver.sendHeader("Location", "/presets", true);
        webserver.send(302, "text/plain", "Redirecting to /presets..");
        });

    // API zum Setzen von Uhrmodus und anderen Einstellungen
    webserver.on("/api/setMode", HTTP_GET, []() {
        Serial.println("[API] Received GET request to /api/setMode with arguments");
        if (webserver.hasArg("face")) {
            String face = webserver.arg("face");
           // face.replace(".", "");
            if (!face.startsWith("/")) face = "/" + face;
            if (face == "/face_default.bmp" || LittleFS.exists(face)) {
                preferences.putString(PK_BACKGROUND, face);
                selectedBackground = face;
            }
        }

        if (webserver.hasArg("handSet")) {
            String handSet = webserver.arg("handSet");
            preferences.putString(PK_HANDSET, handSet);
        }

        if (webserver.hasArg("timeZone")) {
            String tz = webserver.arg("timeZone");
            preferences.putString(PK_TIMEZONE, tz);
            timezone = tz;
            setupNTP();
        }

        for (int i = 0; i < MAX_WLAN; i++) {
            String argName = pkNtpServer(i);
            if (webserver.hasArg(argName)) {
                strncpy(ntpServers[i], webserver.arg(argName).c_str(), sizeof(ntpServers[i]) - 1);
                ntpServers[i][sizeof(ntpServers[i]) - 1] = '\0'; // Null-terminieren
                if (preferences.getString(argName.c_str(), "") != String(ntpServers[i])) {
                    preferences.putString(argName.c_str(), ntpServers[i]);
                }
                // DEBUG_PRINTLN("[API] Received " + argName + ": " + String(ntpServers[i]));  
            }
        }


        if (webserver.hasArg("hubSize")) {
            hubSize = webserver.arg("hubSize").toInt();
            preferences.putUInt(PK_CENTER_SIZE, hubSize);
        }
        if (webserver.hasArg("hubColor")) {
            uint32_t rgb = strtoul(webserver.arg("hubColor").c_str(), NULL, 16); // 24-Bit RGB
            // DEBUG_PRINTLN("[API] Received hubColor: " + webserver.arg("hubColor") + " -> " + String(rgb, HEX));
            uint8_t r = (rgb >> 16) & 0xFF; // Rot extrahieren
            uint8_t g = (rgb >> 8) & 0xFF;  // Grün extrahieren
            uint8_t b = rgb & 0xFF;         // Blau extrahieren

            // Konvertiere RGB888 zu RGB565
            hubColor = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
            preferences.putLong(PK_CENTER_COLOR, rgb);
        }


        if (webserver.hasArg("stationMode")) {
            String stationModeArg = webserver.arg("stationMode");
            stationMode = (stationModeArg == "1" || stationModeArg.equalsIgnoreCase("true")); // Konvertiere zu bool            
            preferences.putBool(PK_STATION_MODE, stationMode);
        }

        if (webserver.hasArg("rotation")) {
            String rotationArg = webserver.arg("rotation");
            uint8_t tftRotation = 0;

            // Prüfe, ob der Wert in Grad angegeben ist
            if (rotationArg == "0" || rotationArg == "90" || rotationArg == "180" || rotationArg == "270") {
                if (rotationArg == "0") tftRotation = 0;
                else if (rotationArg == "90") tftRotation = 1;
                else if (rotationArg == "180") tftRotation = 2;
                else if (rotationArg == "270") tftRotation = 3;
            }
            // Prüfe, ob der Wert als Index (0-3) angegeben ist
            else {
                tftRotation = rotationArg.toInt();
            }

            // Validierung des Wertes
            if (tftRotation >= 0 && tftRotation <= 3) {
                preferences.putUChar(PK_TFT_ROTATION, tftRotation);
                firstRun = true;
                if (!psramAvailable) {
                    tft.setRotation(tftRotation); // sofort anwenden
                }
            }
        }

   
        if (webserver.hasArg("showSecondHand")) {
            String showSecondHandArg = webserver.arg("showSecondHand");
            showSecondHand = (showSecondHandArg == "1" || showSecondHandArg.equalsIgnoreCase("true")); // Konvertiere zu bool
            preferences.putBool(PK_SHOW_SECOND_HAND, showSecondHand);
        }

        if (webserver.hasArg("smoothMinute")) {
            String smoothMinuteArg = webserver.arg("smoothMinute");
            smoothMinute = (smoothMinuteArg == "1" || smoothMinuteArg.equalsIgnoreCase("true")); // Konvertiere zu bool 
            preferences.putBool(PK_SMOOTH_MINUTE, smoothMinute);
        }

        if (webserver.hasArg("pingServer")) {
            String pingServerArg = webserver.arg("pingServer");
            preferences.putString(PK_PING_SERVER, pingServerArg);
        }

        freeClockFaceBuffer();
        loadClockFace();
        loadHandSprites();
        updateClock();

        if (webserver.hasArg("source")) {
            String sourceArg = webserver.arg("source");
            if (sourceArg == "preset") {
                // DEBUG_PRINTLN("[API] Request source: preset");
                webserver.sendHeader("Location", "/presets", true);
                webserver.send(302, "text/plain", "Redirecting to /presets..");
                return;
            }
        }

        webserver.send(200, "text/plain", "ok");
        });

    // Preset-Verwaltung
    webserver.on("/presets", HTTP_GET, []() {
        webserver.setContentLength(CONTENT_LENGTH_UNKNOWN);
        webserver.send(200, "text/html", "");

        String chunk = generateHtmlHeader();
        chunk.reserve(1024);
        chunk += generateHtmlStatus();
        chunk += generateNavigation();
        chunk += "<h2>" + translate("Manage Presets") + "</h2>";

        // Links oben anzeigen
        chunk += "<div style='text-align:center;'>";

        if (pingHostname) {
            chunk += "<p>" + translate("Use the host name") + " <strong>" + String(hostname) + ".local</strong> " + translate("instead of the IP address for better reliability") + ".</p>";
        }

        chunk += "<ul style='list-style-type:none; padding:0; display:inline-block; text-align:left;'>";

         
        String espHost = "http://" + String(hostname) + ".local"; // Aktueller Hostname des ESP

        chunk += "<table style='width:100%; border-collapse:collapse;'>";
        chunk += "<tr><th style='border:1px solid #ccc; padding:8px;'>" + translate("Preset Name") +
            "</th><th style = 'border:1px solid #ccc; padding:8px;'>Link</th></tr>";

        webserver.sendContent(chunk);
        chunk = "";

        int rowCount = 0;
        for (int i = 0; i < MAX_PRESETS; i++) {
            if (!presets[i].name.isEmpty() && !presets[i].url.isEmpty()) {
                String displayUrl = presets[i].url;

                // Ersetze die gespeicherte IP durch die aktuelle IP des ESP
                if (displayUrl.startsWith("http://")) {
                    int ipEnd = displayUrl.indexOf('/', 7); // Suche nach dem Ende der IP-Adresse
                    if (ipEnd != -1) {
                        displayUrl = "http://" + ipAddress + displayUrl.substring(ipEnd); // Ersetze die IP
                    }
                    else {
                        displayUrl = "http://" + ipAddress; // Nur die IP ohne Pfad
                    }
                }
                displayUrl += "&source=preset";
                presets[i].name.replace(" ", "_"); // Ersetze Leerzeichen durch Unterstriche

                chunk += "<tr>";
                chunk += "<td style='border:1px solid #ccc; padding:8px; text-align: left;'><a href='" + displayUrl + "'>" + presets[i].name + "</a></td>";
                String presetName = presets[i].name;
                presetName.replace(" ", "_"); // Ersetze Leerzeichen durch Unterstriche
                chunk += "<td style='border:1px solid #ccc; padding:8px; text-align: left;'>http://" + ipAddress + "/api/setPreset?name=" + presetName;
                if (pingHostname) {
                    chunk += "<br>" + espHost + "/api/setPreset?name=" + presetName;
                }
                chunk += "</td></tr>";

                rowCount++;
                if (rowCount % 5 == 0) {
                    webserver.sendContent(chunk);
                    chunk = "";
                }
            }
        }
        chunk += "</table>";
        chunk += "</ul>";
        chunk += "</div><hr>";

        chunk += "<hr>";
        chunk += "<h3>" + translate("Create New Preset") + "</h3>";
        chunk += "<form method='POST' action='/api/createPreset'>";
        chunk += "<button type='submit'>" + translate("Create Preset from Current Settings") + "</button>";
        chunk += "</form>";
        chunk += "<hr>";

        // Gefüllte Presets anzeigen
        chunk += "<h3>" + translate("Edit Presets") + "</h3>";
        chunk += "<form method='POST' action='/save_presets'>";

        webserver.sendContent(chunk);
        chunk = "";

        for (int i = 0; i < MAX_PRESETS; i++) {

            String displayUrl = presets[i].url;

            // Ersetze die gespeicherte IP durch die aktuelle IP des ESP
            if (displayUrl.startsWith("http://")) {
                int ipEnd = displayUrl.indexOf('/', 7); // Suche nach dem Ende der IP-Adresse
                if (ipEnd != -1) {
                    displayUrl = "http://" + ipAddress + displayUrl.substring(ipEnd); // Ersetze die IP
                   // DEBUG_PRINTLN("[HTML] 1 Replaced preset URL for display: " + displayUrl);
                }
                else {
                    displayUrl = "http://" + ipAddress; // Nur die IP ohne Pfad
                   // DEBUG_PRINTLN("[HTML] 2 Replaced preset URL for display: " + displayUrl);
                }
            }
            Serial.println(displayUrl);
            // falschen Backslash entfernen, der sich manchmal in die URL einschleicht
            int index = displayUrl.indexOf("/face_");
            if (index > 1) {
                Serial.println(displayUrl);
                Serial.println("found /face_ at position " + String(index));
                displayUrl = displayUrl.substring(0, index) + displayUrl.substring(index + 1);      
                Serial.println(displayUrl);   
            }

            presets[i].name.replace(" ", "_"); // Ersetze Leerzeichen durch Unterstriche
            chunk += "<h4>" + translate("Preset") + " " + String(i + 1) + "</h4>";
            chunk += "Name: <input type='text' name='name" + String(i) + "' value='" + presets[i].name + "'><br>";
            chunk += "URL: <input type='text' name='url" + String(i) + "' value='" + displayUrl + "'><br><br>";

            if (i % 5 == 4) {
                webserver.sendContent(chunk);
                chunk = "";
            }

            if (presets[i].name.isEmpty() && presets[i].url.isEmpty()) {
                break;
            }
        }


        chunk += "<button type='submit'>" + translate("Save Presets") + "</button>";
        chunk += "</form><hr>";

        chunk += "</body></html>";
        webserver.sendContent(chunk);
        webserver.sendContent(""); // Ende der Chunked-Uebertragung signalisieren
        });

    // Presets speichern    
    webserver.on("/save_presets", HTTP_POST, []() {
        for (int i = 0; i < MAX_PRESETS; i++) {
            String nameArg = "name" + String(i);
            String urlArg = "url" + String(i);
            presets[i].name.replace(" ", "_"); // Ersetze Leerzeichen durch Unterstriche
            presets[i].name = webserver.hasArg(nameArg) ? webserver.arg(nameArg) : "";
            presets[i].url = webserver.hasArg(urlArg) ? webserver.arg(urlArg) : "";
        }

        savePresets();

        webserver.send(200, "text/html", "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='0; url=/presets'><title>Saved</title></head><body><h2>Presets saved</h2><p>Redirecting...</p></body></html>");
        });

    // API zum Restart des ESP
    webserver.on("/api/reboot", HTTP_GET, []() {
        webserver.send(200, "text/html", "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='0; url=/status'><title>Rebooting</title></head><body><h2>Rebooting...</h2></body></html>");
        delay(WAIT_1s);
        espReboot();
        });


    // API zum Setzen eines Presets
    webserver.on("/api/setPreset", HTTP_GET, []() {
        Serial.println("[API] Received request to /api/setPreset with args: " + webserver.arg("name"));
        if (!webserver.hasArg("name")) {
            webserver.send(400, "text/plain", "Missing 'name' parameter");
            return;
        }

        String presetName = webserver.arg("name");
        presetName.replace(" ", "_"); // Ersetze Leerzeichen durch Unterstriche

        // Suche das Preset mit dem angegebenen Namen
        for (int i = 0; i < MAX_PRESETS; i++) {
            if (presets[i].name.equalsIgnoreCase(presetName)) {
                if (!presets[i].url.isEmpty()) {
                    // Redirect zur URL des Presets
                    webserver.sendHeader("Location", presets[i].url, true);
                    webserver.send(302, "text/plain", "Redirecting to preset URL..");
                    //DEBUG_PRINTLN("[setPreset] Redirecting to preset: " + presetName + " -> " + presets[i].url);
                    return;
                }
                else {
                    webserver.send(404, "text/plain", "Preset URL is empty");
                    return;
                }
            }
        }

        // Preset nicht gefunden
        webserver.send(404, "text/plain", "Preset not found");
        DEBUG_PRINTLN("[setPreset] Preset not found: " + presetName);
        });

    // NTP Server und Zeitzone setzen
    webserver.on("/set_timezone", HTTP_POST, []() {

        for (int i = 0; i < MAX_WLAN; i++) {
            String argName = pkNtpServer(i);
            if (webserver.hasArg(argName)) {
                strncpy(ntpServers[i], webserver.arg(argName).c_str(), sizeof(ntpServers[i]) - 1);
                ntpServers[i][sizeof(ntpServers[i]) - 1] = '\0'; // Ensure null-termination
                if (preferences.getString(argName.c_str(), "") != String(ntpServers[i])) {
                    preferences.putString(argName.c_str(), ntpServers[i]);
                }
                //DEBUG_PRINTLN("[NTP] " + argName + " set to: " + String(ntpServers[i]));
            }
        }

        int writeIndex = 0; // Index, an den die nächste gültige NTP-Server-Adresse geschrieben wird

        for (int readIndex = 0; readIndex < MAX_WLAN; readIndex++) {
            if (strlen(ntpServers[readIndex]) > 0) { // Nur nicht-leere Einträge berücksichtigen
                if (writeIndex != readIndex) {
                    strncpy(ntpServers[writeIndex], ntpServers[readIndex], sizeof(ntpServers[writeIndex]) - 1);
                    ntpServers[writeIndex][sizeof(ntpServers[writeIndex]) - 1] = '\0'; // Null-terminieren
                    memset(ntpServers[readIndex], 0, sizeof(ntpServers[readIndex])); // Ursprünglichen Eintrag löschen
                }
                writeIndex++;
            }
        }

        // Leere Einträge am Ende sicherstellen
        for (int i = writeIndex; i < MAX_WLAN; i++) {
            memset(ntpServers[i], 0, sizeof(ntpServers[i]));
        }




        if (webserver.hasArg("timezone")) {
            String tz = webserver.arg("timezone");
            preferences.putString(PK_TIMEZONE, tz);
            setupNTP();
        }


        webserver.send(200, "text/html",
            "<!DOCTYPE html><html><head>"
            "<meta http-equiv='refresh' content='0; url=/timezone_form' />"
            "<title>NTP / Timezone Updated</title></head>"
            "<body><h2>NTP / Timezone updated</h2>"
            "</body></html>");            
        }
        
        
        );


    // NTP Server und Zeitzone Formular
    webserver.on("/timezone_form", HTTP_GET, []() {
        String timezone = preferences.getString(PK_TIMEZONE, TIMEZONE_DEFAULT);

        struct TimezoneEntry {
            const char* label;
            const char* value;
        } tzList[] = {
            {"Germany (DST auto)", TIMEZONE_DEFAULT},
            {"Germany (fixed summer time)", "CEST-2"},
            {"Germany (fixed winter time)", "CET-1"},
            {"UK (DST auto)", "GMT0BST,M3.5.0/1,M10.5.0"},
            {"UK (fixed summer time)", "BST-1"},
            {"UK (fixed winter time)", "GMT0"},
            {"USA Pacific (DST auto)", "PST8PDT,M3.2.0,M11.1.0"},
            {"USA Central (DST auto)", "CST6CDT,M3.2.0,M11.1.0"},
            {"USA Mountain (DST auto)", "MST7MDT,M3.2.0,M11.1.0"},
            {"USA Eastern (DST auto)", "EST5EDT,M3.2.0,M11.1.0"},
            {"USA Eastern (fixed summer time)", "EDT-4"},
            {"USA Eastern (fixed winter time)", "EST-5"},
            {"Japan (JST)", "JST-9"},
            {"Australia Sydney (DST auto)", "AEST-10AEDT,M10.1.0,M4.1.0/3"},
            {"Australia Sydney (fixed summer time)", "AEDT-11"},
            {"Australia Sydney (fixed winter time)", "AEST-10"},
            {"India (IST)", "IST-5:30"},
            {"Brazil (BRT)", "BRT-3"},
            {"China (CST)", "CST-8"},
            {"Singapore (SGT)", "SGT-8"},
            {"Indonesia (WIB)", "WIB-7"},
            {"South Korea (KST)", "KST-9"},
            {"Argentina (ART)", "ART-3"},
            {"Chile (DST auto)", "CLT4CLST,M9.1.6/24,M4.1.6/24"},
            {"New Zealand (DST auto)", "NZST-12NZDT,M9.5.0,M4.1.0/3"},
            {"Fiji (FJT)", "FJT-12"},
            {"Nigeria (WAT)", "WAT-1"},
            {"South Africa (SAST)", "SAST-2"},
            {"Egypt (EET)", "EET-2"}
        };

        String html = generateHtmlHeader();
        html.reserve(6144);  // Zeitzonen-Formular: lange Dropdown-Liste
        html += generateHtmlStatus(); // Statusleiste einfügen
        html += generateNavigation(); // Navigation einfügen
        html += "<h2>" + translate("NTP Server / Timezone (DST String)") + "</h2>";
        html += "<form method='POST' action='/set_timezone'>";

        for (int i = 0; i < MAX_WLAN; i++) {
                html += "NTP Server" + String(i + 1) + " : <input type = 'text' name = 'ntpServer" + String(i + 1) + "' value = '" + String(ntpServers[i]) + "'><br>";
                if (trim(ntpServers[i]) == "") {
                    break;
            }
        }


        //html += "NTP Server1: <input type='text' name='ntpServer1' value='" + String(ntpServers[0]) + "'><br>";
        //html += "NTP Server2: <input type='text' name='ntpServer2' value='" + String(ntpServers[1]) + "'><br><br>";

        // Kombiniertes Select + Input
        html += translate("Timezone") + ": <br><select id = 'tz_select' style = 'width: 400px;' onchange = \"document.getElementById('tz_input').value=this.value\">";
        for (size_t i = 0; i < sizeof(tzList) / sizeof(tzList[0]); i++) {
            html += "<option value='" + String(tzList[i].value) + "'";
            if (timezone == tzList[i].value) html += " selected";
            html += ">" + String(tzList[i].label) + " (" + String(tzList[i].value) + ")</option>";
        }
        html += "</select><br><br>";

        html += "<input type='text' id='tz_input' name='timezone' style='width: 400px;' value='" + timezone + "'><br><br>";

        html += "<small>" + translate("For custom timezones, select a preset or enter your own value above") + "</small><br><br>";
        html += "<button type='submit'>" + translate("Save Timezone") + "</button><br><br>";
        //  html += generateNavigation(); // Navigation einfügen
        html += "<br><br>";
        html += "</form></body></html>";
        webserver.send(200, "text/html", html);
        });

    // Datei umbenennen Formular
    webserver.on("/rename_form", HTTP_GET, []() {
        if (!webserver.hasArg("file")) {
            webserver.send(400, "text/plain", "Missing file parameter");
            return;
        }

        String oldName = webserver.arg("file");
        String html = generateHtmlHeader();
        html.reserve(1024);  // Umbenennen-Formular: klein

        html += generateHtmlStatus(); // Statusleiste einfügen
        html += generateNavigation(); // Navigation einfügen
        html += "<h2>" + translate("Rename File") + "</h2>";
        html += "<form action='/rename' method='POST'>";
        html += "<input type='hidden' name='old' value='" + oldName + "'>";
        html += "<label>" + translate("New Name") + ":</label><br>";
        html += "<input name='new' value='" + oldName + "' required><br><br>";
        html += "<button type='submit'>" + translate("Rename") + "</button></form>";
        html += "<br><a href='/files'>" + translate("Cancel") + "</a></body></html>";
        webserver.send(200, "text/html", html);
        });

    // Datei umbenennen Aktion
    webserver.on("/rename", HTTP_POST, []() {
        if (webserver.hasArg("old") && webserver.hasArg("new")) {
            String oldName = webserver.arg("old");
            String newName = webserver.arg("new");

            //oldName.replace(".", ""); newName.replace(".", "");
            if (!oldName.startsWith("/")) oldName = "/" + oldName;
            if (!newName.startsWith("/")) newName = "/" + newName;

            if (LittleFS.exists(oldName)) {
                if (LittleFS.rename(oldName, newName)) {
                    webserver.sendHeader("Location", "/files", true);
                    webserver.send(302, "text/plain", "");
                }
                else {
                    webserver.send(500, "text/plain", "Rename failed");
                }
            }
            else {
                webserver.send(404, "text/plain", "Original file not found");
            }
        }
        else {
            webserver.send(400, "text/plain", "Missing parameters");
        }
        });


    // BMP skalieren Formular
    webserver.on("/scalebmp_form", HTTP_GET, []() {
        if (!webserver.hasArg("file")) {
            webserver.send(400, "text/plain", "Missing file name");
            return;
        }
        String src = webserver.arg("file");
        String html = generateHtmlHeader();
        html.reserve(1536);  // BMP-Skalieren-Formular: klein
        html += generateHtmlStatus(); // Statusleiste einfügen
        html += generateNavigation(); // Navigation einfügen
        html += "<h2>" + translate("Scale and Save BMP") + "</h2>";
        html += "<form action='/scalebmp_run' method='GET'>";
        html += translate("Source") + ": <input name = 'src' value = '/" + src + "' readonly><br>";
        html += translate("Target") + ": <input name = 'dst' value = '/scaled_" + src + "'><br>";
        html += translate("Width") + ": <input name='w' type='number' value='" + String(CLOCK_WIDTH) + "' required><br>";
        html += translate("Height") + ": <input name = 'h' type = 'number' value = '" + String(CLOCK_HEIGHT) + "' required><br>";
        html += "<button type='submit'>" + translate("Scale and Save") + "</button></form>";
        html += "<br><br>";
        // html += generateNavigation(); // Navigation einfügen
        html += "</body></html>";
        webserver.send(200, "text/html", html);
        });

    // BMP skalieren Aktion 
    webserver.on("/scalebmp_run", HTTP_GET, []() {
        if (!webserver.hasArg("src") || !webserver.hasArg("dst") || !webserver.hasArg("w") || !webserver.hasArg("h")) {
            webserver.send(400, "text/plain", "Missing parameters");
            return;
        }

        String src = webserver.arg("src");
        String dst = webserver.arg("dst");
        int w = webserver.arg("w").toInt();
        int h = webserver.arg("h").toInt();

        bool scaleSuccess = scaleAndSaveBmp(src.c_str(), dst.c_str(), w, h);
        if (scaleSuccess) {
            webserver.send(200, "text/html", "<html><body style='font-family:Arial;'><h3>" + translate("Scaling successful") + "!</h3><p>" + translate("Saved as") + ": " + dst + "</p><a href = '/files'>" + translate("Back") + " </a></body></html>");
        }
        else {
            webserver.send(500, "text/html", "<html><body style='font-family:Arial;'><h3>" + translate("Failed to scale BMP") + "</h3><p>Check source file and format.</p><a href='/files'>Back</a></body></html>");
        }
        });

    // Anzeigeeinstellungen speichern
    webserver.on("/applydisplaysettings", HTTP_POST, []() {
        // Save to Preferences

        if (webserver.hasArg("pingServer")) {
            preferences.putString(PK_PING_SERVER, webserver.arg("pingServer"));
        }

        stationMode = preferences.getBool(PK_STATION_MODE, false);
        showSecondHand = preferences.getBool(PK_SHOW_SECOND_HAND, true);
        smoothMinute = preferences.getBool(PK_SMOOTH_MINUTE, false);


        stationMode = webserver.hasArg("stationMode");
        showSecondHand = webserver.hasArg("showSecondHand");
        smoothMinute = webserver.hasArg("smoothMinute");


        useTouch = webserver.hasArg("useTouch");
        preferences.putBool(PK_USE_TOUCH, useTouch);

        if (useTouch) enableTouch();
        else disableTouch();

        // Logging-Einstellung speichern
        loggingEnabled = webserver.hasArg("loggingEnabled");
        preferences.putBool(PK_LOGGING_ENABLED, loggingEnabled);

        wifiActive = webserver.hasArg("wifiActive");
        preferences.putBool(PK_WIFI_ACTIVE, wifiActive);

        preferences.putBool(PK_STATION_MODE, stationMode);
        preferences.putBool(PK_SHOW_SECOND_HAND, showSecondHand);
        preferences.putBool(PK_SMOOTH_MINUTE, smoothMinute);

        if (webserver.hasArg("rotation")) {
            tftRotation = webserver.arg("rotation").toInt();
            if (tftRotation >= 0 && tftRotation <= 3) {
                preferences.putUChar(PK_TFT_ROTATION, tftRotation);
                firstRun = true;
                if (!psramAvailable) {
                    tft.setRotation(tftRotation); // sofort anwenden
                }
            }

            freeClockFaceBuffer();
            loadClockFace();      // neu zeichnen mit neuer Ausrichtung
            loadHandSprites();
        }

       
        webserver.send(200, "text/html",
            "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='0; url=/'><title>Saved</title></head>"
            "<body style='font-family:Arial;text-align:center;'><h2>Saved</h2><p>Back...</p></body></html>");
        });


    // Helligkeitseinstellungen Formular
    webserver.on("/brightness", HTTP_POST, []() {
        webserver.setContentLength(CONTENT_LENGTH_UNKNOWN);
        webserver.send(200, "text/html", "");

        String chunk = generateHtmlHeader();
        chunk.reserve(1024);
        chunk += generateHtmlStatus(); // Statusleiste einfügen
        chunk += generateNavigation(); // Navigation einfügen
        chunk += "<h2>" + translate("Brightness Settings") + "</h2><form method = 'POST' action = '/save_brightness'>";

        if (photoresistorFound) {
            chunk += "<table style='margin:auto;text-align:left;'><tr>";
            chunk += "<td><label><input type='checkbox' name='use_adc' value='1' " + String(useAdc ? "checked" : "") + "> " + translate("Enable Auto Brightness") + "</label></td>";

            chunk += "<td><label><input type='checkbox' name='adcInverted' value='1' " + String(adcInverted ? "checked" : "") + "> " + translate("Invert ADC Reading") + "</label></td>";
            chunk += "</tr></table><hr><br>";

            chunk += "<label>" + translate("Low Threshold") + " (0 - 100 %) :</label><br><input name = 'lowThreshold' type = 'number' min = '0' max = '100' value = '" + String(lowThreshold) + "'><br>";
            chunk += "<label>" + translate("High Threshold") + " (0 - 100 %) :</label><br><input name = 'highThreshold' type = 'number' min = '0' max = '100' value = '" + String(highThreshold) + "'><br>";
        }

        chunk += "<label>" + translate("Min Brightness") + " (0 - 255) : </label><br><input name = 'minBrightness' type = 'number' min = '0' max = '255' value = '" + String(minBrightness) + "'><br>";

        chunk += "<label>" + translate("Max Brightness") + " (0 - 255) : </label><br><input name = 'maxBrightness' type = 'number' min = '0' max = '255' value = '" + String(maxBrightness) + "'><br>";


        chunk += "<label>" + translate("Full brightness from (hour, 0-23)") + ":</label><br><input name = 'brightStart' type = 'number' min = '0' max = '23' value = '" + String(brightStartHour) + "'><br>";
        chunk += "<label>" + translate("Full brightness until (hour, 0-23)") + ":</label><br><input name = 'brightEnd' type = 'number' min = '0' max = '23' value = '" + String(brightEndHour) + "'><br>";

#if defined (GC9D01)  || defined(GC9A01_WITH_BACKLIGHT) 
        chunk += "<label>" + translate("Gamma Correction") + " (0.1 - 3.0) : </label><br>";
        chunk += "<input type='number' name='gamma' step='0.1' min='0.1' max='3.0' value='" + String(gammaBrightness) + "' required><br>";

#endif


        chunk += "<button type='submit'>" + translate("Save") + "</button></form>";

        webserver.sendContent(chunk);
        chunk = "";

        if (photoresistorFound) {
            chunk += "<br>";
            chunk += "<hr><strong>" + translate("Current ADC Value") + ":</strong> " + String(currentAdcAvg) + "<br>";
            chunk += "<strong>" + translate("Current Brightness") + ":</strong> " + String(currentBrightness) + " / 255<br>";
            chunk += "<strong>" + translate("Light (for Threshold)") + ":</strong> " + String(currentLightPercent) + " % <br>";

            chunk += "<br>";
            chunk += "<form method='GET' action='/brightness'><button type='submit'>" + translate("Refresh") + "</button></form>";
            chunk += "<br>"; chunk += "<br>";

            webserver.sendContent(chunk);
            chunk = "";

#if defined (GC9D01)  || defined(GC9A01_WITH_BACKLIGHT) 
            chunk += "<script src='https://cdn.plot.ly/plotly-latest.min.js'></script>\n";

            chunk += "<h2>Gamma-Korrektur: adc -> targetBrightness</h2>\n";
            chunk += "<label for='gammaSlider'>Gamma: <span id='gammaValue'>" + String(gammaBrightness) + "</span></label>\n";
            chunk += "<input type='range' id='gammaSlider' min='0.1' max='3.0' step='0.1' value='" + String(gammaBrightness) + "' style='width:300px;'><br><br>\n";
            chunk += "<div id='plot' style='width:100%; height:600px;'></div>\n";

            chunk += "<script>\n";
            chunk += "const minBrightness = " + String(minBrightness) + ";\n";
            chunk += "const maxBrightness = " + String(maxBrightness) + ";\n";
            chunk += "const avg = Array.from({length: 500}, (_, i) => i * (4095 / 499));\n\n";

            chunk += "function computeBrightness(gamma) {\n";
            chunk += "  return avg.map(val => {\n";
            chunk += "    let norm = Math.min(Math.max(val / 4095.0, 0.0), 1.0);\n";
            chunk += "    let gammaNorm = Math.pow(norm, gamma);\n";
            chunk += "    return minBrightness + Math.round((maxBrightness - minBrightness) * gammaNorm);\n";
            chunk += "  });\n";
            chunk += "}\n\n";

            webserver.sendContent(chunk);
            chunk = "";

            chunk += "function plotGamma(gamma) {\n";
            chunk += "  const y = computeBrightness(gamma);\n";
            chunk += "  Plotly.newPlot('plot', [{\n";
            chunk += "    x: avg,\n";
            chunk += "    y: y,\n";
            chunk += "    mode: 'lines',\n";
            chunk += "    name: `Gamma = ${gamma.toFixed(1)}`\n";
            chunk += "  }], {\n";
            chunk += "    title: 'Gamma-Korrektur-Kurve',\n";
            chunk += "    xaxis: { title: 'adc (0 - 4095)' },\n";
            chunk += "    yaxis: { title: 'targetBrightness (0 - 255)' }\n";
            chunk += "  });\n";
            chunk += "}\n\n";

            chunk += "const slider = document.getElementById('gammaSlider');\n";
            chunk += "const gammaValue = document.getElementById('gammaValue');\n";
            chunk += "slider.addEventListener('input', () => {\n";
            chunk += "  const gamma = parseFloat(slider.value);\n";
            chunk += "  gammaValue.textContent = gamma.toFixed(1);\n";
            chunk += "  plotGamma(gamma);\n";
            chunk += "});\n\n";

            chunk += "plotGamma(" + String(gammaBrightness) + ");\n";
            chunk += "</script>\n";
#endif
        }

        chunk += "<br><br>";
        chunk += generateNavigation(); // Navigation einfügen
        chunk += "</body></html>";
        webserver.sendContent(chunk);
        webserver.sendContent(""); // Ende der Chunked-Uebertragung signalisieren
        });

    // Helligkeitseinstellungen Formular
    webserver.on("/brightness", HTTP_GET, []() {
        webserver.setContentLength(CONTENT_LENGTH_UNKNOWN);
        webserver.send(200, "text/html", "");

        String chunk = generateHtmlHeader();
        chunk.reserve(1024);
        chunk += generateHtmlStatus(); // Statusleiste einfügen
        chunk += generateNavigation(); // Navigation einfügen
        chunk += "<h2>" + translate("Brightness Settings") + "</h2><form method = 'POST' action = '/save_brightness'>";

        if (photoresistorFound) {
            chunk += "<table style='margin:auto;text-align:left;'><tr>";
            chunk += "<td><label><input type='checkbox' name='use_adc' value='1' " + String(useAdc ? "checked" : "") + "> " + translate("Enable Auto Brightness") + "</label></td>";

            chunk += "<td><label><input type='checkbox' name='adcInverted' value='1' " + String(adcInverted ? "checked" : "") + "> " + translate("Invert ADC Reading") + "</label></td>";
            chunk += "</tr></table><hr><br>";

            chunk += "<label>" + translate("Low Threshold") + " (0 - 100) : </label><br><input name = 'lowThreshold' type = 'number' min = '0' max = '100' value = '" + String(lowThreshold) + "'><br>";
            chunk += "<label>" + translate("High Threshold") + " (0 - 100) : </label><br><input name = 'highThreshold' type = 'number' min = '0' max = '100' value = '" + String(highThreshold) + "'><br>";
        }

        chunk += "<label>" + translate("Min Brightness") + " (0 - 255) : </label><br><input name = 'minBrightness' type = 'number' min = '0' max = '255' value = '" + String(minBrightness) + "'><br>";

        chunk += "<label>" + translate("Max Brightness") + " (0 - 255) : </label><br><input name = 'maxBrightness' type = 'number' min = '0' max = '255' value = '" + String(maxBrightness) + "'><br>";


        chunk += "<label>" + translate("Full brightness from (hour, 0-23)") + ":</label><br><input name = 'brightStart' type = 'number' min = '0' max = '23' value = '" + String(brightStartHour) + "'><br>";
        chunk += "<label>" + translate("Full brightness until (hour, 0-23)") + ":</label><br><input name = 'brightEnd' type = 'number' min = '0' max = '23' value = '" + String(brightEndHour) + "'><br>";
#if defined (GC9D01)  || defined(GC9A01_WITH_BACKLIGHT) 
        chunk += "<label>" + translate("Gamma Correction") + " (0.1 - 3.0) : </label><br>";
        chunk += "<input type='number' name='gamma' step='0.1' min='0.1' max='3.0' value='" + String(gammaBrightness) + "' required><br>";
#endif


        chunk += "<button type='submit'>" + translate("Save") + "</button></form>";

        webserver.sendContent(chunk);
        chunk = "";

        if (photoresistorFound) {
            chunk += "<br>";
            chunk += "<hr><strong>" + translate("Current ADC Value") + ":</strong> " + String(currentAdcAvg) + "<br>";
            chunk += "<strong>" + translate("Current Brightness") + ":</strong> " + String(currentBrightness) + " / 255<br>";
            chunk += "<strong>" + translate("Light (for Threshold)") + ":</strong> " + String(currentLightPercent) + " % <br>";
            chunk += "<br>";
            chunk += "<form method='GET' action='/brightness'><button type='submit'>" + translate("Refresh") + "</button></form>";
            chunk += "<br>";

            webserver.sendContent(chunk);
            chunk = "";

#if defined (GC9D01)  || defined(GC9A01_WITH_BACKLIGHT) 
            chunk += "<script src='https://cdn.plot.ly/plotly-latest.min.js'></script>\n";

            chunk += "<h2>Gamma-Korrektur: adc -> targetBrightness</h2>\n";
            chunk += "<label for='gammaSlider'>Gamma: <span id='gammaValue'>" + String(gammaBrightness) + "</span></label>\n";
            chunk += "<input type='range' id='gammaSlider' min='0.1' max='3.0' step='0.1' value='" + String(gammaBrightness) + "' style='width:300px;'><br><br>\n";
            chunk += "<div id='plot' style='width:100%; height:600px;'></div>\n";

            chunk += "<script>\n";
            chunk += "const minBrightness = " + String(minBrightness) + ";\n";
            chunk += "const maxBrightness = " + String(maxBrightness) + ";\n";
            chunk += "const avg = Array.from({length: 500}, (_, i) => i * (4095 / 499));\n\n";

            chunk += "function computeBrightness(gamma) {\n";
            chunk += "  return avg.map(val => {\n";
            chunk += "    let norm = Math.min(Math.max(val / 4095.0, 0.0), 1.0);\n";
            chunk += "    let gammaNorm = Math.pow(norm, gamma);\n";
            chunk += "    return minBrightness + Math.round((maxBrightness - minBrightness) * gammaNorm);\n";
            chunk += "  });\n";
            chunk += "}\n\n";

            webserver.sendContent(chunk);
            chunk = "";

            chunk += "function plotGamma(gamma) {\n";
            chunk += "  const y = computeBrightness(gamma);\n";
            chunk += "  Plotly.newPlot('plot', [{\n";
            chunk += "    x: avg,\n";
            chunk += "    y: y,\n";
            chunk += "    mode: 'lines',\n";
            chunk += "    name: `Gamma = ${gamma.toFixed(1)}`\n";
            chunk += "  }], {\n";
            chunk += "    title: 'Gamma-Korrektur-Kurve',\n";
            chunk += "    xaxis: { title: 'adc (0 - 4095)' },\n";
            chunk += "    yaxis: { title: 'targetBrightness (0 - 255)' }\n";
            chunk += "  });\n";
            chunk += "}\n\n";

            chunk += "const slider = document.getElementById('gammaSlider');\n";
            chunk += "const gammaValue = document.getElementById('gammaValue');\n";
            chunk += "slider.addEventListener('input', () => {\n";
            chunk += "  const gamma = parseFloat(slider.value);\n";
            chunk += "  gammaValue.textContent = gamma.toFixed(1);\n";
            chunk += "  plotGamma(gamma);\n";
            chunk += "});\n\n";

            chunk += "plotGamma(" + String(gammaBrightness) + ");\n";
            chunk += "</script>\n";
#endif
        }

        chunk += generateNavigation(); // Navigation einfügen    
        chunk += "<br><br></body></html>";
        webserver.sendContent(chunk);
        webserver.sendContent(""); // Ende der Chunked-Uebertragung signalisieren
        });

    // Helligkeitseinstellungen speichern
    webserver.on("/save_brightness", HTTP_POST, []() {
        useAdc = webserver.hasArg("use_adc");
        adcInverted = webserver.hasArg("adcInverted");
        lowThreshold = webserver.arg("lowThreshold").toInt();
        highThreshold = webserver.arg("highThreshold").toInt();

        maxBrightness = (uint8_t)webserver.arg("maxBrightness").toInt();
        minBrightness = (uint8_t)webserver.arg("minBrightness").toInt();

        // neue: Zeitabhängige Helligkeit speichern


        brightStartHour = (uint8_t)constrain(webserver.arg("brightStart").toInt(), 0, 23);
        brightEndHour = (uint8_t)constrain(webserver.arg("brightEnd").toInt(), 0, 23);

#if defined (GC9D01)  || defined(GC9A01_WITH_BACKLIGHT) 
        gammaBrightness = webserver.arg("gamma").toFloat();
        preferences.putFloat(PK_GAMMA_BRIGHTNESS, gammaBrightness);
#endif

        preferences.putBool(PK_USE_ADC, useAdc);
        preferences.putBool(PK_ADC_INVERTED, adcInverted);
        preferences.putInt(PK_LOW_THRESHOLD, lowThreshold);
        preferences.putInt(PK_HIGH_THRESHOLD, highThreshold);

        preferences.putUChar(PK_MAX_BRIGHTNESS, maxBrightness);
        preferences.putUChar(PK_MIN_BRIGHTNESS, minBrightness);

        // persist time-based settings

        preferences.putUChar(PK_BRIGHT_START_HOUR, brightStartHour);
        preferences.putUChar(PK_BRIGHT_END_HOUR, brightEndHour);


        webserver.send(200, "text/html",
            "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='0; url=/brightness'><title>Saved</title></head>"
            "<body style='font-family:Arial;text-align:center;'><h2>Settings saved</h2><p>Returning...</p></body></html>");
        });


    // Alle Dateien auflisten
    webserver.on("/files", HTTP_GET, []() {
        webserver.setContentLength(CONTENT_LENGTH_UNKNOWN);
        webserver.send(200, "text/html", "");

        String chunk = generateHtmlHeader();
        chunk.reserve(1024);

        chunk += generateHtmlStatus(); // Statusleiste einfügen
        chunk += generateNavigation(); // Navigation einfügen

        chunk += "<h2>" + translate("All Files on LittleFS") + "</h2><table border = '1'><tr><th style='text-align:left;'>" + translate("Filename") + "</th><th>" + translate("Size(bytes)") + "</th><th>Info</th><th>" + translate("Action") + "</th></tr>";

        webserver.sendContent(chunk);
        chunk = "";

        // Erst alle Dateinamen sammeln und natuerlich sortieren (Zahlen im
        // Namen numerisch statt alphabetisch, z.B. hand_set2 vor hand_set10),
        // bevor die Tabelle daraus aufgebaut wird.
        std::vector<String> fileNames;
        File root = LittleFS.open("/");
        File file = root.openNextFile();
        while (file) {
            fileNames.push_back(String(file.name()));
            file = root.openNextFile();
        }
        naturalSortNames(fileNames);

        int rowCount = 0;
        for (const String& name : fileNames) {
            String openPath = name.startsWith("/") ? name : "/" + name;
            File f = LittleFS.open(openPath, "r");
            size_t fileSize = f ? f.size() : 0;
            if (f) f.close();
            String info = getBmpInfo(name);
            chunk += "<tr><td style='text-align:left;'>" + name + "</td><td align=right>" + String(fileSize) + "</td>";
            chunk += "<td align=right>" + String(info) + "</td>";
            chunk += " <td><a href = '/delete?file=" + name + "' onclick = 'return confirm(\"Delete " + name + "?\")'>" + translate("Delete") + "</a> ";
            // Scale-Option nur für .bmp-Dateien anzeigen
            if (name.endsWith(".bmp")) {
                chunk += "<a href = '/scalebmp_form?file=" + name + "'>" + translate("Scale") + "</a> ";
                chunk += "<a href='/rename_form?file=" + name + "'>" + translate("Rename") + "</a> ";
            }
            else {
                chunk += translate("Scale") + " ";
                chunk += translate("Rename") + " ";
            }
                       
            chunk += "<a href='/download?file=" + name + "'>" + translate("Download") + "</a> ";
            chunk += "<a href='/file?name=" + name + "'>" + translate("View") + "</a> "; // "View"-Link für Logdateien

            chunk += "</td></tr>";

            // Alle paar Zeilen zwischendurch senden, damit der Puffer auch
            // bei sehr vielen Dateien nicht unbegrenzt waechst.
            rowCount++;
            if (rowCount % 5 == 0) {
                webserver.sendContent(chunk);
                chunk = "";
            }
        }
        chunk += "</table><br><br>";
        chunk += generateNavigation(); // Navigation einfügen
        chunk += "</body></html>";
        webserver.sendContent(chunk);
        webserver.sendContent(""); // Ende der Chunked-Uebertragung signalisieren
        });

    webserver.on("/download", HTTP_GET, []() {
        if (webserver.hasArg("file")) {
            String path = webserver.arg("file");
            if (!path.startsWith("/")) path = "/" + path;

            if (LittleFS.exists(path)) {
                // Pruefen, ob es sich um eine RLE-komprimierte face_*.bmp
                // handelt - falls ja, vor dem Download zu einem echten,
                // eigenstaendigen Standard-BMP dekodieren, damit die Datei
                // auch ausserhalb der Uhr (Bildbetrachter etc.) nutzbar ist.
                bool isRle = false;
                if (path.endsWith(".bmp")) {
                    File probe = LittleFS.open(path, "r");
                    if (probe) {
                        uint8_t magic[4] = { 0 };
                        probe.read(magic, 4);
                        probe.close();
                        isRle = isRleFace(magic);
                    }
                }

                if (isRle) {
                    String downloadName = path.substring(path.lastIndexOf('/') + 1);
                    webserver.sendHeader("Content-Disposition", "attachment; filename=\"" + downloadName + "\"");
                    if (streamRleFaceAsStandardBmp(path, "application/octet-stream")) {
                        return;
                    }
                    // Streaming fehlgeschlagen (z.B. Datei-Lesefehler) - der
                    // Header wurde ggf. schon gesendet, daher kein sauberer
                    // 500-Statuscode mehr moeglich; das Streaming selbst
                    // braucht aber nur wenig RAM, Speicherknappheit ist hier
                    // kein realistischer Fehlschlaggrund mehr.
                    DEBUG_PRINTLN("[DOWNLOAD] RLE streaming failed for " + path);
                    return;
                }

                File file = LittleFS.open(path, "r");

                // Setze den Content-Disposition-Header, um den Dateinamen festzulegen
                webserver.sendHeader("Content-Disposition", "attachment; filename=\"" + String(file.name()) + "\"");
                webserver.streamFile(file, "application/octet-stream");
                file.close();
                return;
            }
        }
        webserver.send(404, "text/plain", "File not found");
        });

    // Systemstatus Seite
    webserver.on("/status", HTTP_GET, []() {

        webserver.setContentLength(CONTENT_LENGTH_UNKNOWN);
        webserver.send(200, "text/html", "");

        String chunk = generateHtmlHeader();
        chunk.reserve(1024);

        chunk += generateHtmlStatus(); // Statusleiste einfügen
        chunk += generateNavigation(); // Navigation einfügen

        webserver.sendContent(chunk);
        chunk = "";

        chunk += "<h2>System Status</h2><ul>";

        String tzLabel = preferences.getString(PK_TIMEZONE, "DE");
        String tzDesc;

        tzDesc = tzLabel;

       // struct tm timeinfo;
        if (getLocalTime(&timeinfo, 100)) {
            char nowStr[32];
            strftime(nowStr, sizeof(nowStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
            chunk += "<li>Current Time: " + String(nowStr) + "</li>";
            chunk += "<li>Timezone: " + tzDesc + "</li>";
            chunk += "<li>Current week: " + String(currentWeek) + "</li>";
            chunk += "<li>Last week reset: " + String(lastResetWeek) + "</li>";

            unsigned long seconds = millis() / 1000;
            unsigned long days = seconds / 86400;
            unsigned long hours = (seconds % 86400) / 3600;
            unsigned long minutes = (seconds % 3600) / 60;
            unsigned long secs = seconds % 60;
            chunk += "<li>Uptime: " + String(days) + "d " + String(hours) + "h " + String(minutes) + "m " + String(secs) + "s</li>";

            chunk += "<br>";
        }

        webserver.sendContent(chunk);
        chunk = "";

        chunk += "<li>Compiled on: <strong>" + (String)version + "</strong></li><br>";

        chunk += "<li>TFT Driver: " + tftType + "</li>";

        chunk += "<li>TFT Size: " + String(TFT_WIDTH) + " x " + String(TFT_HEIGHT) + "</li>";

        chunk += "<br>";

        webserver.sendContent(chunk);
        chunk = "";

        chunk += "<li>ChipModel: " + String(ESP.getChipModel()) + "</li>";
        chunk += "<li>ChipRevision: " + String(ESP.getChipRevision()) + "</li>";
        chunk += "<li>ChipCores: " + String(ESP.getChipCores()) + "</li>";
        chunk += "<li>Chip ID: " + String((uint32_t)ESP.getEfuseMac(), HEX) + "</li>";
        chunk += "<li>CPU Frequency: " + String(getCpuFrequencyMhz()) + " MHz</li><br>";

        webserver.sendContent(chunk);
        chunk = "";

        chunk += "<li>Hostname: " + String(hostname) + ".local" + "</li>";
        chunk += "<li>IP Address: " + WiFi.localIP().toString() + "</li>";
        chunk += "<li>MAC Address: " + WiFi.macAddress() + "</li>";
        chunk += "<li>WiFi SSID: " + String(WiFi.SSID()) + "</li>";
        chunk += "<li>WiFi Mode: " + String(WiFi.getMode() == WIFI_AP ? "WIFI_AP" : (WiFi.getMode() == WIFI_STA ? "WIFI_STA" : "AP_STA")) + "</li>";
        chunk += "<li>WiFi Channel: " + String(WiFi.channel()) + "</li>";
        chunk += "<li>Signal Strength (RSSI): " + String(WiFi.RSSI()) + " dBm</li><br>";

        webserver.sendContent(chunk);
        chunk = "";

        chunk += "<li>SDK Version: " + String(ESP.getSdkVersion()) + "</li><br>";

        webserver.sendContent(chunk);
        chunk = "";

        chunk += "<li>Flash Size: " + String(ESP.getFlashChipSize() / 1024) + " KB</li>";
        chunk += "<li>Free Heap: " + String(ESP.getFreeHeap() / 1024) + " KB</li>";
        chunk += "<li>Min Free Heap (since boot): " + String(ESP.getMinFreeHeap() / 1024) + " KB</li>";
        chunk += "<li>Max Sketch Size: " + String(ESP.getFreeSketchSpace() / 1024) + " KB</li>";
        chunk += "<li>Sketch Size: " + String(ESP.getSketchSize() / 1024) + " KB</li>";
        chunk += "<li>Free Sketch Space: " + String((ESP.getFreeSketchSpace() / 1024) - (ESP.getSketchSize() / 1024)) + " KB</li><br>";

        webserver.sendContent(chunk);
        chunk = "";

        chunk += "<li>PSRam size: " + String(ESP.getPsramSize() / 1024) + " kB</li>";
        chunk += "<li>PSRam free: " + String(ESP.getFreePsram() / 1024) + " kB</li><br>";
        // chunk += "<li>PSRam used: " + String(psramAvailable == true ? "true" : "false") + "</li><br>";


        chunk += "<li>LittleFS Size: " + String(LittleFS.totalBytes() / 1024) + " KB</li>";
        chunk += "<li>LittleFS Used: " + String(LittleFS.usedBytes() / 1024) + " KB</li>";
        chunk += "<li>LittleFS Free: " + String((LittleFS.totalBytes() - LittleFS.usedBytes()) / 1024) + " KB</li><br>";

        webserver.sendContent(chunk);
        chunk = "";

#ifdef ADC_PIN
        if (photoresistorFound) {
            chunk += "<li>Photoresistor found on GPIO: " + String(ADC_PIN) + "</li>";
            chunk += "<li>Actual brightness (0-255): " + String(currentBrightness) + "</li><br>";
        }
        else {
            chunk += "<li>Photoresistor not found on GPIO: " + String(ADC_PIN) + "</li><br>";
        }
#endif

        webserver.sendContent(chunk);
        chunk = "";



        chunk += "<li>TFT_SCLK GPIO: " + String(TFT_SCLK) + "</li>";
        //chunk += "<li>TFT_MISO: " + String(TFT_MISO) + "</li>";  
        chunk += "<li>TFT_MOSI GPIO: " + String(TFT_MOSI) + "</li>";
        chunk += "<li>TFT_CS GPIO: " + String(TFT_CS) + "</li>";
#if defined CS_2
        chunk += "<li>TFT_CS2 GPIO: " + String(CS_2) + "</li>";
#endif


        chunk += "<li>TFT_DC GPIO: " + String(TFT_DC) + "</li>";
        chunk += "<li>TFT_RST GPIO: " + String(TFT_RST) + "</li><br>";

#if defined SDA_PIN && defined SCL_PIN
        if (!i2cAddr.isEmpty()) {
            chunk += "<li>I2C ADR: " + i2cAddr + "</li>";
            chunk += "<li>I2C SDA GPIO: " + String(SDA_PIN) + "</li>";
            chunk += "<li>I2C SDL GPIO: " + String(SCL_PIN) + "</li><br>";
        }
        else {
            chunk += "<li>I2C: no device found</li><br>";
        }
#endif

        webserver.sendContent(chunk);
        chunk = "";

#if defined DCF77_DATAPIN && defined DCF77_INTERRUPT
        if (dcf77Count == 0) {
            chunk += "<li>DCF77: No signal received so far</li>";
        }
        else {
            chunk += "<li>DCF77: Pulses received</li>";
        }
        if (lastDcfSyncTime == 0) {
            chunk += "<li>DCF77 last sync: never</li>";
        }
        else {
            struct tm syncInfo;
            localtime_r(&lastDcfSyncTime, &syncInfo);
            char syncBuf[24];
            snprintf(syncBuf, sizeof(syncBuf), "%04d-%02d-%02d %02d:%02d:%02d",
                syncInfo.tm_year + 1900, syncInfo.tm_mon + 1, syncInfo.tm_mday,
                syncInfo.tm_hour, syncInfo.tm_min, syncInfo.tm_sec);
            chunk += "<li>DCF77 last sync: " + String(syncBuf) + "</li>";
        }
        chunk += "<li>DCF77 Data GPIO: " + String(DCF77_DATAPIN) + "</li>";  
        chunk += "<li>DCF77 Edge: " + String(dcf77Flank ? "rising" : "falling") + "</li><br>";
#endif

        webserver.sendContent(chunk);
        chunk = "";

#ifdef BUTTON1
        chunk += "<li>BUTTON GPIO: " + String(BUTTON1) + "</li>";
#endif
        chunk += "<li>BUTTON_BOOT GPIO: " + String(BOOT_BUTTON) + "</li>";

#ifdef LED_BOARD
        chunk += "<li>LED_BOARD GPIO: " + String(LED_BOARD) + "</li>";
#endif
#ifdef TOUCH_PIN
        chunk += "<li>TOUCH_PIN GPIO: " + String(TOUCH_PIN) + "</li>";
        chunk += "<li>use Touch: " + String(useTouch ? "true" : "false") + "</li><br>";
#endif
#ifdef ADC_PIN
        chunk += "<li>ADC_VCC GPIO: " + String(ADC_3V) + "</li>";
        chunk += "<li>ADC (photoresistor) GPIO: " + String(ADC_PIN) + "</li>";
        chunk += "<li>ADC_GND GPIO: " + String(ADC_GND) + "</li>";
        if (photoresistorFound) {
            chunk += "<li>ADC val: " + String(getAdjustedAdcValue(analogRead(ADC_PIN))) + "</li><br>";
        }
#endif


#ifndef TFT_Backlight 
        chunk += "<li>TFT_Backlight: none</li>";
#else
        chunk += "<li>TFT_Backlight GPIO: " + String(TFT_Backlight) + "</li>";
#endif
        chunk += "<br>";

        webserver.sendContent(chunk);
        chunk = "";

        chunk += "<li><h3>Actual Preferences</h3></li><ul>";

        for (int i = 0; i < MAX_WLAN; i++) {
            // Dynamisch berechnete Schlüssel
            String ssidKey = pkSsid(i);

            if (preferences.getString(ssidKey.c_str(), "") != "") {
                if (preferences.getInt(PK_LAST_WLAN) != i) {
                    chunk += "<li><b>" + ssidKey + ":</b> " + preferences.getString(ssidKey.c_str(), "") + "</li>";
                }
                else {
                    chunk += "<li><b>" + ssidKey + ": " + preferences.getString(ssidKey.c_str(), "") + "</b></li>";
                }
            }
    
        }

        webserver.sendContent(chunk);
        chunk = "";

        for (int i = 0; i < MAX_WLAN; i++) {
            if (preferences.getString((pkNtpServer(i)).c_str(), "") != "") {
                chunk += "<li><b>ntpServer" + String(i + 1) + ":</b> " + preferences.getString((pkNtpServer(i)).c_str(), "") + "</li>";
            }
        }
       
        chunk += "<li><b>pingServer:port</b>: " + preferences.getString(PK_PING_SERVER, DEFAULT_PING_SERVER) + "</li>";

        chunk += "<li><b>timezone</b>: " + preferences.getString(PK_TIMEZONE, TIMEZONE_DEFAULT) + "</li>";
        chunk += "<li><b>background</b>: " + preferences.getString(PK_BACKGROUND, "/faces/default") + "</li>";
        chunk += "<li><b>handset</b>: " + preferences.getString(PK_HANDSET, "") + "</li>";
        chunk += "<li><b>centerColor (RGB565)</b>: " + String(preferences.getUInt(PK_CENTER_COLOR, TFT_RED), HEX) + "</li>";
        chunk += "<li><b>centerSize</b>: " + String(preferences.getUInt(PK_CENTER_SIZE, 6)) + "</li>";

        uint8_t rotation = preferences.getUChar(PK_TFT_ROTATION, 0);
        const char* rotationLabels[] = { "0&deg;", "90&deg;", "180&deg;", "270&deg;" };
        chunk += "<li><b>tftRotation</b>: " + String(rotationLabels[rotation]) + "</li>";

        webserver.sendContent(chunk);
        chunk = "";

        // Booleans als Text
        
        chunk += "<li><b>stationMode</b>: " + String(preferences.getBool(PK_STATION_MODE, true) ? "true" : "false") + "</li>";
        chunk += "<li><b>showSecondhand</b>: " + String(preferences.getBool(PK_SHOW_SECOND_HAND, true) ? "true" : "false") + "</li>";
        chunk += "<li><b>smoothMinute</b>: " + String(preferences.getBool(PK_SMOOTH_MINUTE, false) ? "true" : "false") + "</li>";

        chunk += "<li><b>minBrightness</b>: " + String(preferences.getUChar(PK_MIN_BRIGHTNESS, 100)) + "</li>";
        chunk += "<li><b>maxBrightness</b>: " + String(preferences.getUChar(PK_MAX_BRIGHTNESS, 255)) + "</li>";

        uint16_t brightEnd = preferences.getUChar(PK_BRIGHT_END_HOUR, 20);
        brightEnd += 1;
        if (brightEnd > 23) brightEnd = 0;

        chunk += "<li><b>daywindow</b>: " + String(preferences.getUChar(PK_BRIGHT_START_HOUR, 8)) + ":00 - " + String(brightEnd) + ":00</li>";

        if (preferences.getBool(PK_USE_ADC, true)) {
            chunk += "<li><b>use_adc</b>: " + String(preferences.getBool(PK_USE_ADC, true) ? "true" : "false") + "</li>";
            chunk += "<li><b>adc lowThreshold</b>: " + String(preferences.getInt(PK_LOW_THRESHOLD, 40)) + "</li>";
            chunk += "<li><b>adc highThreshold</b>: " + String(preferences.getInt(PK_HIGH_THRESHOLD, 60)) + "</li>";
            chunk += "<li><b>adc Inverted</b>: " + String(preferences.getBool(PK_ADC_INVERTED, false) ? "true" : "false") + "</li>";
        }
        if (preferences.getBool(PK_USE_TOUCH, false)) {
            chunk += "<li><b>use Touch</b>: " + String(preferences.getBool(PK_USE_TOUCH, false) ? "true" : "false") + "</li>";
        }
        chunk += "</ul>";
        chunk += "</br>";
        chunk += "<li>Contact: <a href='mailto:holger.wagenlehner@gmx.de'>holger.wagenlehner@gmx.de</a></li>";

        chunk += "<li>Project: <a href='https://github.com/holgiw/TFT-Clock-GC9A01' target='_blank'>GitHub</a></li>";

        chunk += "</ul>";
        chunk += generateNavigation(); // Navigation einfügen
        chunk += "</body></html>";

        webserver.sendContent(chunk);
        webserver.sendContent(""); // Ende der Chunked-Uebertragung signalisieren
        });

    // Kleine Vorschau (80x80) fuer hochgeladene Zifferblaetter - siehe
    // sendScaledBmpPreview() in display.h. Vermeidet, dass fuer ein
    // 80x80-<img> die volle Aufloesung (z.B. 240x240 = ~115 KB) uebertragen wird.
    webserver.on("/facepreview", HTTP_GET, []() {
        if (webserver.hasArg("file")) {
            String path = webserver.arg("file");
            if (!path.startsWith("/")) path = "/" + path;
            sendScaledBmpPreview(path, 80, 80);
        }
        else {
            webserver.send(400, "text/plain", "missing 'file' argument");
        }
        });

    webserver.on("/preview_defaultface", HTTP_GET, []() {
        // Kleine Vorschau statt voller Aufloesung (spart ~90% Uebertragungsgroesse
        // fuer ein 80x80-<img> - siehe sendScaledBmpPreview() fuer den
        // gleichwertigen Ansatz bei hochgeladenen Zifferblaettern).
        const int outW = 80;
        const int outH = 80;
        const float scaleX = (float)CLOCK_WIDTH / outW;
        const float scaleY = (float)CLOCK_HEIGHT / outH;

        const int headerSize = 54;
        const int rowSize = ((outW * 3 + 3) / 4) * 4; // 3 Bytes pro Pixel für RGB888
        const int dataSize = rowSize * outH;
        const int fileSize = headerSize + dataSize;

        uint8_t* bmpData = new uint8_t[fileSize];
        memset(bmpData, 0, fileSize);

        // BMP-Header
        bmpData[0] = 'B'; bmpData[1] = 'M';
        *(uint32_t*)&bmpData[2] = fileSize;
        *(uint32_t*)&bmpData[10] = headerSize;
        *(uint32_t*)&bmpData[14] = 40;
        *(int32_t*)&bmpData[18] = outW;
        *(int32_t*)&bmpData[22] = -outH; // Top-down BMP
        *(uint16_t*)&bmpData[26] = 1;
        *(uint16_t*)&bmpData[28] = 24; // 24-Bit Farbtiefe
        *(uint32_t*)&bmpData[34] = dataSize;

        // Pixel-Daten (RGB565 -> RGB888, mit Downscaling)
        for (int y = 0; y < outH; y++) {
            int srcY = int(y * scaleY);
            uint8_t* rowPtr = bmpData + headerSize + y * rowSize;
            for (int x = 0; x < outW; x++) {
                int srcX = int(x * scaleX);
                uint16_t px = clockFace[srcY * CLOCK_WIDTH + srcX];

                // Transparente Farbe ersetzen
                if (px == TRANSPARENT_COLOR) {
                    rowPtr[x * 3 + 0] = 255; // Blau
                    rowPtr[x * 3 + 1] = 255; // Grün
                    rowPtr[x * 3 + 2] = 255; // Rot
                    continue;
                }

                // RGB565 ? RGB888
                uint8_t r = (px >> 8) & 0xF8; // obere 5 Bits
                uint8_t g = (px >> 3) & 0xFC; // mittlere 6 Bits
                uint8_t b = (px << 3) & 0xF8; // untere 5 Bits

                rowPtr[x * 3 + 0] = b; // Blau
                rowPtr[x * 3 + 1] = g; // Grün
                rowPtr[x * 3 + 2] = r; // Rot
            }
        }

        webserver.send_P(200, "image/bmp", (const char*)bmpData, fileSize);
        delete[] bmpData;
        });

    // Uhr-Gesichter verwalten
    webserver.on("/listfilesFaces", HTTP_GET, []() {

        size_t total = LittleFS.totalBytes();
        size_t used = LittleFS.usedBytes();

        webserver.setContentLength(CONTENT_LENGTH_UNKNOWN);
        webserver.send(200, "text/html", "");

        String chunk = generateHtmlHeader();
        chunk.reserve(1024);


        chunk += generateHtmlStatus(); // Statusleiste einfügen
        chunk += generateNavigation(); // Navigation einfügen
        chunk += "<h2>" + translate("Manage Clock Face Files") + " " + String(CLOCK_WIDTH) + " x " + String(CLOCK_HEIGHT) + "</h2><table border='1'><tr><th>" + translate("Preview/Set") + "</th></tr>";

        // Add built-in default face
        chunk += "<tr><td>";
        chunk += "<a href='http://" + ipAddress + "/setbackground?file=face_default.bmp'>";
        chunk += "<img src='http://" + ipAddress + "/preview_defaultface' style='width:80px;height:80px;border:1px solid #ccc'>";
        chunk += "</a><br>face_default.bmp<br><small>" + (String)TFT_WIDTH + " x " + (String)TFT_HEIGHT + " / " + "16 bpp" + " </small> </td>";
        chunk += "</tr>";

        webserver.sendContent(chunk);
        chunk = "";

        File root = LittleFS.open("/");
        File file = root.openNextFile();

        // Erst alle passenden Zifferblatt-Dateinamen sammeln und sortieren
        // (der eingebaute Standard oben bleibt davon unberuehrt, da er bereits
        // separat und fest an erster Stelle ausgegeben wurde).
        std::vector<String> faceNames;
        while (file) {
            String name = file.name();
            if (!file.isDirectory() && name.startsWith("face_") && name.endsWith(".bmp")) {
                faceNames.push_back(name);
            }
            file = root.openNextFile();
        }
        naturalSortNames(faceNames);

        bool anyFile = !faceNames.empty();
        int rowCount = 0;
        for (const String& name : faceNames) {
            String shortName = name;
            String info = getBmpInfo(name);
            chunk += "<tr><td>";
            chunk += "<a href=http://" + ipAddress + "/setbackground?file=" + shortName + ">";
            chunk += "<img src='/facepreview?file=" + name + "' style='width:80px;height:80px;border:1px solid #ccc'>";
            chunk += "</a><br>" + shortName + "<br><small>" + String(info) + "</small></td></tr>";

            // Alle paar Zeilen zwischendurch senden, damit der Puffer auch
            // bei vielen Zifferblaettern nicht unbegrenzt waechst.
            rowCount++;
            if (rowCount % 5 == 0) {
                webserver.sendContent(chunk);
                chunk = "";
            }
        }

        if (!anyFile) chunk += "<tr><td colspan='3'>No BMP files found in /</td></tr>";
        chunk += "</table><hr>";

        // Hinweis und Download-Link für die ZIP-Datei
        if (TFT_WIDTH == 240) {
            chunk += "<h3>" + translate("Download Additional Clock Faces") + "</h3>";
            chunk += "<p>" + translate("You can download a ZIP file containing additional clock faces and hand sets from the following link: (use 'view raw')") + "</p>";
            chunk += "<a href='https://github.com/holgiw/TFT-Clock-GC9A01/blob/master/graphic/faces_handsets_240.zip' target='_blank'>Download faces_handsets_240.zip</a>";
            chunk += "<br><small>" + translate("After downloading, upload the extracted BMP files using the form below") + ".</small><hr>";
        }

        if (TFT_WIDTH == 160) {
            chunk += "<h3>" + translate("Download Additional Clock Faces") + "</h3>";
            chunk += "<p>" + translate("You can download a ZIP file containing additional clock faces and hand sets from the following link: (use 'view raw')") + "</p>";
            chunk += "<a href='https://github.com/holgiw/TFT-Clock-GC9A01/blob/master/graphic/faces_handsets_160.zip' target='_blank'>Download faces_handsets_160.zip</a>";
            chunk += "<br><small>" + translate("After downloading, upload the extracted BMP files using the form below") + ".</small><hr>";
        }

        webserver.sendContent(chunk);
        chunk = "";

        if (used + (TFT_WIDTH * TFT_HEIGHT * 2) + 54 > total) {
            chunk += "<div style='color:red;font-weight:bold;'>" + translate("Warning: Not enough free space to upload new clock faces! Free up some space first") + ".</div><br><br>";
        }
        else {

            chunk += "<h3>" + translate("Upload New Clock Face") + "</h3>";
            chunk += "<small>" + translate("Requirements") + ": " + String(CLOCK_WIDTH) + " x " + String(CLOCK_HEIGHT) + " pixels, 16 - bit BMP(RGB565), " + translate("name must start with") + "  <code>face_</code></small><br><br>";

            chunk += "<form method = 'POST' action = '/upload' enctype = 'multipart/form-data' onsubmit = 'showProgress()'>";
            chunk += "<input type='file' name='upload' accept='.bmp' multiple required><br>";

            chunk += "<button type='submit'>" + translate("Upload") + " BMP</button>";
            chunk += "<div id='progress' style='display:none;'>Uploading... please wait</div>";
            chunk += "<script>function showProgress(){document.getElementById('progress').style.display='block';}</script></form><br><br>";
        }


        chunk += generateNavigation(); // Navigation einfügen
        chunk += "</body> </html>";
        webserver.sendContent(chunk);
        webserver.sendContent(""); // Ende der Chunked-Uebertragung signalisieren
        });

    // WLAN Netzwerke scannen
    webserver.on("/api/scanwifi", HTTP_GET, []() {
        String json = "";
               
        // die letzten Scan-Ergebnisse zurückgeben
        json = "[";
        for (int i = 0; i < foundNetworkCount; ++i) {
            if (i > 0) json += ",";
            json += "{\"ssid\":\"" + availableNetworks[i].ssid + "\"";
            json += ",\"rssi\":" + String(availableNetworks[i].rssi);
            json += ",\"enc\":" + String(availableNetworks[i].enc);
            json += "}";
        }
        json += "]";
        
        webserver.send(200, "application/json", json);
        });

    webserver.on("/api/rescanwifi", HTTP_POST, []() {
        scanAndCacheNetworks();
        webserver.send(200, "application/json", "{\"status\":\"ok\"}");
        });



    // Hauptseite - WLAN Einstellungen
    webserver.on("/", HTTP_GET, []() {

        webserver.setContentLength(CONTENT_LENGTH_UNKNOWN);
        webserver.send(200, "text/html", "");

        String chunk = generateHtmlHeader();
        chunk.reserve(1536);

        chunk += generateHtmlStatus(); // Statusleiste einfügen
        chunk += generateNavigation(); // Navigation einfügen

        chunk += "<h2>" + translate("Clock Setup") + "</h2>";

        chunk += generateLanguageSelector();
        chunk += "<form method='POST' action='/api/resetWiFi' onsubmit='return confirm(\"" + translate("Are you sure you want to reset all saved WiFi networks?") + "\");'>";
        chunk += "<button type='submit'>" + translate("Reset Saved Networks") + "</button>";
        chunk += "</form><br><br>";

        webserver.sendContent(chunk);
        chunk = "";

        chunk += "<form action = '/save' method = 'POST'>";

        chunk += "<button id='rescanBtn' type='button'>" + translate("Rescan Networks") + "</button><br>";

        for (int i = 0; i < MAX_WLAN; i++) {
            // Dynamisch berechnete Schlüssel
            String ssidKey = pkSsid(i);
            String passKey = pkPass(i);
            String ssidSelectId = "ssid_select" + String(i + 1);
            wifiSsid[i] = preferences.getString(ssidKey.c_str(), "");

            String upperSsidKey = ssidKey; // Kopie erstellen
            upperSsidKey.toUpperCase();    // Kopie in Großbuchstaben umwandeln
            chunk += "<h3>" + upperSsidKey + "</h3>";

            chunk += "<select id='" + ssidSelectId + "' onchange=\"document.getElementById('" + ssidKey + "').value=this.value\">";
            
            chunk += "</select><br>";
            chunk += "<input name='" + ssidKey + "' id='" + ssidKey + "' placeholder='" + ssidKey + "' value='" + wifiSsid[i] + "'><br>";
            chunk += "<small>" + translate("You can also enter an SSID manually") + ".</small><br>";

            chunk += "<input name='" + passKey + "' id='" + passKey + "' placeholder='Password' type='password' value=''><br>";
            if (WiFi.getMode() == WIFI_STA && wifiSsid[i] != "") {
                chunk += "<small>" + translate("Password is hidden.Leave empty to keep current") + ".</small>";
            }
            chunk += "<br><hr><br>";

            // Alle paar Eintraege zwischendurch senden, damit auch bei vielen
            // gespeicherten Netzwerken (bis zu MAX_WLAN) kein grosser Puffer entsteht.
            if (i % 3 == 2) {
                webserver.sendContent(chunk);
                chunk = "";
            }

            if (wifiSsid[i] == "") {
                break; // Keine weiteren SSIDs, Schleife beenden
            }
        }

        
        chunk += "<br><br>";

        chunk += "<button type='submit'>" + translate("Save WiFi settings") + "</button></form><hr>";

        webserver.sendContent(chunk);
        chunk = "";

        //if (WiFi.getMode() == WIFI_STA) {


            chunk += "<form action='/applydisplaysettings' method='POST'>";

            chunk += "<table style='margin:auto;text-align:left;'><tr>";

            chunk += "<td><input type='checkbox' name='stationMode' value='1' ";
            chunk += preferences.getBool(PK_STATION_MODE, true) ? "checked" : "";
            chunk += "> " + translate("Train Station Mode") + "</td>";

            chunk += "<td><input type='checkbox' name='showSecondHand' value='1' ";
            chunk += preferences.getBool(PK_SHOW_SECOND_HAND, true) ? "checked" : "";
            chunk += "> " + translate("Show Seconds") + "</td>";

            chunk += "<td><input type='checkbox' name='smoothMinute' value='1' ";
            chunk += preferences.getBool(PK_SMOOTH_MINUTE, true) ? "checked" : "";
            chunk += "> " + translate("Smooth Minute Hand") + "</td>";

            String pingServer = preferences.getString(PK_PING_SERVER, DEFAULT_PING_SERVER);
            chunk += "<td>" + translate("Ping Server") +"<input type='text' name='pingServer' value='" + pingServer + "'>";
            chunk +="</td>";

            chunk += "</tr><tr>";

            // Neue Checkbox für Touch-Freigabe
            //chunk += "<td><input type='checkbox' name='useTouch' value='1' ";
            //chunk += preferences.getBool(PK_USE_TOUCH, false) ? "checked" : "";
            //chunk += "> " + translate("Enable Touch") + "</td>";

            chunk += "<td><input type='checkbox' name='wifiActive' value='1' ";
            chunk += wifiActive ? "checked" : "";
            chunk += "> " + translate("Reconnect WiFi") + "</td>";
            

            chunk += "<td><input type='checkbox' name='loggingEnabled' value='1' ";
            chunk += loggingEnabled ? "checked" : "";
            chunk += "> " + translate("Enable Logging") + "</td>";

            
            chunk += "<td>Rotation: <select name='rotation'>";
            const char* rotationLabels[] = { "0&deg;", "90&deg;", "180&deg;", "270&deg;" };
            for (int i = 0; i <= 3; i++) {
                chunk += "<option value='" + String(i) + "'";
                if (i == tftRotation) chunk += " selected";
                chunk += ">" + String(rotationLabels[i]) + "</option>";
            }
            chunk += "</select></td>";


            // chunk += "<td><input type='checkbox' name='loggingEnabled' value='1' ";
            // chunk += loggingEnabled ? "checked" : "";
            // chunk += "> Logging aktivieren</td>";

            chunk += "<td valign=bottom><button type='submit'>" + translate("Apply") + "</button></td>";
            chunk += "</tr></table></form>";

            chunk += "<hr>";

            /*
            chunk += "<a href='/timezone_form'><button>Set Timezone</button></a><br><br>";

            chunk += "<a href='/listfilesFaces'><button>Manage Clock Face Files</button></a><br><br>";
            chunk += "<a href='/handsets'><button>Manage Hand Sets</button></a><br><br>";

            chunk += "<form action='/syncnow' method='POST'><button type='submit'>Sync Time Now</button></form><br>";
            chunk += "<form action='/brightness' method='POST'><button type='submit'>Brightness Settings</button></form><br>";
               */

       // }

        webserver.sendContent(chunk);
        chunk = "";

        chunk += "<script>";
        chunk += "document.getElementById('rescanBtn').onclick = function() {";
        chunk += "  fetch('/api/rescanwifi', {method: 'POST'})";
        chunk += "    .then(() => {";
        chunk += "      select1.innerHTML = \"<option>WLAN scan in progress...</option>\";";
        chunk += "      select2.innerHTML = \"<option>WLAN scan in progress...</option>\";";
        chunk += "      setTimeout(function() {";
        chunk += "        fetch('/api/scanwifi').then(response => response.json()).then(data => {";
        chunk += "          select1.innerHTML = \"<option value=''>" + translate("select network") + "</option>\";";
        chunk += "          data.forEach(function(net) {";
        chunk += "            var opt = document.createElement('option');";
        chunk += "            opt.value = net.ssid;";
        chunk += "            opt.text = net.ssid + ' (' + net.rssi + ' dBm)';";

        for (int i = 0; i < MAX_WLAN; i++) {
            String ssidKey = pkSsid(i);
            String select = "select" + String(i + 1);
            chunk += "            " + select + ".appendChild(opt);";
            if (preferences.getString(ssidKey.c_str(), "") == "") {
                break; // Keine weiteren SSIDs, Schleife beenden
            }
        }

        chunk += "          });";        

        chunk += "        });";
        chunk += "      }, 2000);"; // 2 Sekunden warten für Scan
        chunk += "    });";
        chunk += "};";

        webserver.sendContent(chunk);
        chunk = "";

        chunk += "window.addEventListener('DOMContentLoaded', function() {";
        for (int i = 0; i < MAX_WLAN; i++) {

           

            // Dynamisch berechnete Schlüssel
            String ssidKey = pkSsid(i);
            String passKey = pkPass(i);
            String select = "select" + String(i + 1); 
            String input = "input" + String(i + 1);
            String current = "current" + String(i + 1); 
            String ssidSelectId = "ssid_select" + String(i + 1);
            
                        

            chunk += "  var " + select + " = document.getElementById('" + ssidSelectId + "');";
            chunk += "  var " + input + " = document.getElementById('ssid1');";
            chunk += "  var " + current + " = " + input + ".value;";
            chunk += "  " + select + ".innerHTML = \"<option>WLAN scan in progress...</option>\";";
            chunk += "  fetch('/api/scanwifi')";
            chunk += "    .then(response => response.json())";
            chunk += "    .then(data => {";
            chunk += "      " + select + ".innerHTML = \"<option value=''>" + translate("select network") + "</option>\";";
            chunk += "      data.forEach(function(net) {";
            chunk += "        var opt = document.createElement('option');";
            chunk += "        opt.value = net.ssid;";
            chunk += "        opt.text = net.ssid + ' (' + net.rssi + ' dBm)';";
            chunk += "        if(net.ssid === " + current + ") opt.selected = true;";
            chunk += "        " + select + ".appendChild(opt);";
            chunk += "      });";
            chunk += "    })";
            chunk += "    .catch(() => { " + select + ".innerHTML = \"<option>Scan failed</option>\"; });";

            // Alle paar Eintraege zwischendurch senden (siehe Kommentar oben).
            if (i % 3 == 2) {
                webserver.sendContent(chunk);
                chunk = "";
            }

            if (preferences.getString(ssidKey.c_str(), "") == "") {
                break; // Keine weiteren SSIDs, Schleife beenden
            }

        }
        chunk += "});";
        chunk += "</script>";

        chunk += generateNavigation(); // Navigation einfügen


        chunk += "</body></html>";
        webserver.sendContent(chunk);
        webserver.sendContent(""); // Ende der Chunked-Uebertragung signalisieren
        });

    // Speichern der WiFi-Einstellungen
    webserver.on("/save", HTTP_POST, []() {
        //if (webserver.hasArg("ssid1")) {

            for (int i = 0; i < MAX_WLAN; i++) {
                // Dynamisch berechnete Schlüssel
                String ssidKey = pkSsid(i);
                String passKey = pkPass(i);

                if (preferences.getString(ssidKey.c_str(), "") != webserver.arg(ssidKey)) {
                    preferences.putString(ssidKey.c_str(), webserver.arg(ssidKey));
                }
                if (webserver.arg(passKey) != "" && preferences.getString(passKey.c_str(), "") != webserver.arg(passKey)) {
                    preferences.putString(passKey.c_str(), webserver.arg(passKey));
                }
            }


            // leere Einträge aussortieren
            String tempSsid[MAX_WLAN];
            String tempPass[MAX_WLAN];


            int j = 0;
            for (int i = 0; i < MAX_WLAN; i++) {

                // Dynamisch berechnete Schlüssel
                String ssidKey = pkSsid(i);
                String passKey = pkPass(i);

                String ssid = trim(preferences.getString(ssidKey.c_str(), ""));                
                if (ssid.length() > 0) {
                    tempSsid[j] = preferences.getString(ssidKey.c_str(), "");
                    tempPass[j] = preferences.getString(passKey.c_str(), "");                    
                    j++;
                }
            }

            for (int i = 0; i < MAX_WLAN; i++) {
                // Dynamisch berechnete Schlüssel
                String ssidKey = pkSsid(i);
                String passKey = pkPass(i);

                if (preferences.getString(ssidKey.c_str(), "") != tempSsid[i]) {
                    preferences.putString(ssidKey.c_str(), tempSsid[i]);
                }
                if (preferences.getString(passKey.c_str(), "") != tempPass[i]) {
                    preferences.putString(passKey.c_str(), tempPass[i]);
                }

                wifiSsid[i] = tempSsid[i];
                wifiPass[i] = tempPass[i];
            }


            

            if (WiFi.getMode() == WIFI_STA) {
                webserver.send(200, "text/html", "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='0; url=/'>"
                    "<title>Settings saved</title></head><body style='font-family:Arial;text-align:center;'>"
                    "<h2>Settings saved</h2><p>.</p></body></html>");
            }
            else {
                webserver.send(200, "text/html", "<!DOCTYPE html><html><head>"
                    "<title>Settings saved</title></head><body style='font-family:Arial;text-align:center;'>"
                    "<h2>Settings saved</h2><p>Please connect to your home network and go to the ESP website at http:// IPADDRESS</p></body></html>");

                espReboot();
            }
       // }
        });

    // Upload-Formular anzeigen
    webserver.on("/upload", HTTP_GET, []() {
        webserver.send(200, "text/html", "<form method='POST' action='/upload' enctype='multipart/form-data' onsubmit='showProgress()'><input type='file' name='upload' accept='.bmp' multiple required><br><br><button type='submit'>Upload BMP</button><div id='progress' style='display:none;'>Uploading... please wait</div><script>function showProgress(){document.getElementById('progress').style.display='block';}</script></form><br><a href='/listfilesFaces'>Back to file list</a>");
        });

    // Datei-Upload verarbeiten
    webserver.on("/upload", HTTP_POST, []() {
        if (uploadSuccess) {
            webserver.sendHeader("Location", "/listfilesFaces", true);
            webserver.send(302, "text/plain", "");
        }
        else {
            String errorHtml = "<!DOCTYPE html><html><head><title>Upload Failed</title><meta name='viewport' content='width=device-width, initial-scale=1'></head><body style='font-family:Arial;text-align:center;'>";
            errorHtml += "<h2>Upload failed</h2>";
            errorHtml += "<p>Only .bmp files starting with <code>face_</code> or <code>hand_</code> are accepted.</p>";
            errorHtml += "<p>Please also check the available space</p>";
            errorHtml += "<a href='/upload'>Try again</a></body></html>";
            webserver.send(400, "text/html", errorHtml);
        }
        }, handleFileUpload);

    // Hintergrundbild setzen
    webserver.on("/setbackground", HTTP_GET, []() {
        Serial.println("setbackground");
        if (webserver.hasArg("file")) {
            String file = webserver.arg("file");
        //    file.replace(".", "");
            if (!file.startsWith("/")) file = "/" + file;

            if (file == "/face_default.bmp") {
                selectedBackground = file;
                preferences.putString(PK_BACKGROUND, file);
                freeClockFaceBuffer();
                loadClockFace();
                loadHandSprites();
                webserver.sendHeader("Location", "/listfilesFaces", true);
                webserver.send(302, "text/plain", "");
                return;
            }

            if (LittleFS.exists(file)) {
                selectedBackground = file;
                preferences.putString(PK_BACKGROUND, file);
                DEBUG_PRINTLN("set bg to: " + file);
                freeClockFaceBuffer();
                loadClockFace();
                loadHandSprites();
                webserver.sendHeader("Location", "/listfilesFaces", true);
                webserver.send(302, "text/plain", "");
                return;
            }
        }
        webserver.send(404, "text/plain", "404 Site not found");
        });

    // Datei löschen
    webserver.on("/delete", HTTP_GET, []() {
        if (webserver.hasArg("file")) {
            String path = webserver.arg("file");
            //path.replace(".", "");
            if (!path.startsWith("/")) path = "/" + path;
            if (LittleFS.exists(path)) {
                LittleFS.remove(path);
                webserver.sendHeader("Location", "/files", true);
                webserver.send(302, "text/plain", "");
            }
            else {
                webserver.send(404, "text/plain", "File not found");
            }
        }
        });

    // Datei anzeigen (BMP)
    webserver.on("/file", HTTP_GET, []() {
        if (webserver.hasArg("name")) {

            setLedOn();

            String path = webserver.arg("name");
            if (!path.startsWith("/")) path = "/" + path;

            if (LittleFS.exists(path)) {

                // Prüfe den Dateityp basierend auf der Dateiendung
                if (path.endsWith(".log") || path.endsWith(".txt")) {
                    File file = LittleFS.open(path, "r");
                    webserver.streamFile(file, "text/plain"); // Logdateien als Text senden
                    file.close();
                }
                else if (path.endsWith(".bmp")) {
                    // Pruefen, ob RLE-komprimiert - falls ja, vor der
                    // Auslieferung zu einem echten, eigenstaendigen
                    // Standard-BMP dekodieren (sonst waere die Datei fuer
                    // externe Betrachter/Tools nicht als Bild lesbar).
                    bool isRle = false;
                    File probe = LittleFS.open(path, "r");
                    if (probe) {
                        uint8_t magic[4] = { 0 };
                        probe.read(magic, 4);
                        probe.close();
                        isRle = isRleFace(magic);
                    }

                    if (isRle) {
                        if (streamRleFaceAsStandardBmp(path)) {
                            setLedOff();
                            return;
                        }
                        // Streaming fehlgeschlagen (z.B. Datei-Lesefehler) - NICHT
                        // stillschweigend die rohen, komprimierten Bytes ausliefern
                        // (waere kein gueltiges BMP mehr, der Browser zeigt sonst
                        // "Grafik enthaelt Fehler") - stattdessen klarer Fehler.
                        // Hinweis: Falls bereits Chunked-Header gesendet wurden,
                        // kann hier kein sauberer 500-Statuscode mehr folgen -
                        // das betrifft aber nur echte Lese-/Formatfehler, nicht
                        // Speicherknappheit (das Streaming braucht nur wenig RAM).
                        DEBUG_PRINTLN("[FILE] RLE streaming failed for " + path);
                        setLedOff();
                        return;
                    }

                    File file = LittleFS.open(path, "r");
                    webserver.streamFile(file, "image/bmp"); // BMP-Dateien als Bild senden
                    file.close();
                }
                else {
                    File file = LittleFS.open(path, "r");
                    webserver.streamFile(file, "application/octet-stream"); // Andere Dateien als Binärdaten senden
                    file.close();
                }

                setLedOff();
                return;
            }          
        }
        webserver.send(404, "text/plain", "File not found");
        setLedOff();
        });

    // Hand-Sets verwalten
    webserver.on("/handsets", HTTP_GET, []() {

        size_t total = LittleFS.totalBytes();
        size_t used = LittleFS.usedBytes();

        // Chunked-Response (Variante B): Diese Seite bettet Vorschaubilder als
        // Base64 EIN (encodeBmpToBase64) und kann dadurch - abhaengig von der
        // Anzahl vorhandener Zeigersaetze - sehr gross werden. Statt die
        // komplette Seite in einem String zu sammeln, wird hier Stueck fuer
        // Stueck direkt an den Client gesendet (webserver.sendContent()).
        // Der Speicherbedarf haengt so nur noch von der GROESSTEN EINZELNEN
        // Zeile ab (ein Zeigersatz, ~20 KB), nicht mehr von der Gesamtzahl
        // der Zeigersaetze.
        webserver.setContentLength(CONTENT_LENGTH_UNKNOWN);
        webserver.send(200, "text/html", "");

        String chunk = generateHtmlHeader();
        chunk += generateHtmlStatus(); // Statusleiste einfügen
        chunk += generateNavigation(); // Navigation einfügen
        chunk += "<h2>" + translate("Manage Clock Hand Sets") + " " + String(HAND_WIDTH) + " x " + String(HAND_HEIGHT) + "</h2><table border = '1'><tr><th colspan = 2>Preview / Set</th></tr>";
        webserver.sendContent(chunk);

        String activeSet = preferences.getString(PK_HANDSET, "");
        // Numerisch (nicht alphabetisch) sortieren, damit z.B. "10" nach "9" statt
        // zwischen "1" und "2" einsortiert wird. std::map<int,String> sortiert
        // automatisch nach dem numerischen Key. Nicht-numerische Zeigersatz-Namen
        // (unueblich, aber theoretisch moeglich) landen zusaetzlich unsortiert in
        // einer separaten Liste, damit sie nicht verloren gehen.
        std::set<String> seenSetIds;
        std::map<int, String> numericSets;
        std::vector<String> otherSets;

        File root = LittleFS.open("/");
        File file = root.openNextFile();
        while (file) {
            String name = file.name();

            if (!file.isDirectory() && name.startsWith("hand_set") && name.endsWith(".bmp")) {
                int start = 8;
                int end = name.indexOf('_', start);
                if (end > start) {
                    String setIdStr = name.substring(start, end);
                    if (seenSetIds.insert(setIdStr).second) { // true, wenn neu (noch nicht gesehen)
                        bool isNumeric = setIdStr.length() > 0;
                        for (unsigned int k = 0; k < setIdStr.length(); k++) {
                            if (!isDigit(setIdStr[k])) { isNumeric = false; break; }
                        }
                        if (isNumeric) numericSets[setIdStr.toInt()] = setIdStr;
                        else otherSets.push_back(setIdStr);
                    }
                }
            }
            file = root.openNextFile();
        }


        String handHourBase64 = encodeBmpToBase64(handHour, HAND_WIDTH, HAND_HEIGHT);
        String handMinuteBase64 = encodeBmpToBase64(handMinute, HAND_WIDTH, HAND_HEIGHT);
        String handSecondBase64 = encodeBmpToBase64(handSecond, HAND_WIDTH, HAND_HEIGHT);

        // Default-Zeigersatz (eingebaut) - eigener Chunk
        chunk = "<tr><td>0</td><td>";
        chunk += "<a href='/sethandset?set=default'>";
        chunk += "<img src='data:image/bmp;charset=utf-8;base64, " + handHourBase64 + "'> ";
        chunk += "<img src='data:image/bmp;charset=utf-8;base64, " + handMinuteBase64 + "'> ";
        chunk += "<img src='data:image/bmp;charset=utf-8;base64, " + handSecondBase64 + "'>";
        chunk += "</a>";
        chunk += "</td></tr>";
        webserver.sendContent(chunk);

        // Jeden gefundenen Zeigersatz SOFORT senden statt zu sammeln - so liegt
        // nie mehr als ein Zeigersatz gleichzeitig im Speicher.
        auto renderSetRow = [&](const String& setId) {
            chunk = "<tr><td>" + setId + (setId == activeSet ? " (active)" : "") + "</td><td>";
            String hourPath = "/hand_set" + setId + "_hour.bmp";
            String minutePath = "/hand_set" + setId + "_minute.bmp";
            String secondPath = "/hand_set" + setId + "_second.bmp";
            chunk += "<a href='/sethandset?set=" + setId + "'>";
            chunk += LittleFS.exists(hourPath) ? "<img src='/file?name=" + hourPath + "'> " : "<img src='data:image/bmp;charset=utf-8;base64, " + handHourBase64 + "'> ";
            chunk += LittleFS.exists(minutePath) ? "<img src='/file?name=" + minutePath + "'> " : "<img src='data:image/bmp;charset=utf-8;base64, " + handMinuteBase64 + "'> ";
            chunk += LittleFS.exists(secondPath) ? "<img src='/file?name=" + secondPath + "'> " : "<img src='data:image/bmp;charset=utf-8;base64," + handSecondBase64 + "'>";
            chunk += "</a>";
            chunk += "</td>";
            chunk += "</tr>";
            webserver.sendContent(chunk);
            checkHeapWarning("/handsets Zeigersatz " + setId);
            };

        // Zuerst alle numerisch benannten Zeigersaetze in aufsteigender Reihenfolge...
        for (auto& entry : numericSets) {
            renderSetRow(entry.second);
        }
        // ...danach eventuelle Sonderfaelle mit nicht-numerischem Namen (unsortiert)
        for (const String& setId : otherSets) {
            renderSetRow(setId);
        }

        // Ab hier sind die grossen Base64-Strings nicht mehr benoetigt.
        handHourBase64 = String();
        handMinuteBase64 = String();
        handSecondBase64 = String();

        chunk = "</table><hr>";

        uint8_t hubSize = preferences.getUInt(PK_CENTER_SIZE, 6);
        uint32_t hubColorRgb = preferences.getLong(PK_CENTER_COLOR, 0xEC0016);

        chunk += "<h2>" + translate("Centre point") + "</h2><form action = '/setcenter' method = 'POST'>";
        chunk += "<label>Size (Pixel):</label><br><input name='size' type='number' min='0' max='50' value='" + String(hubSize) + "'><br>";
        chunk += "<label>" + translate("Color (RGB hex, e.g. FF0000 = Red, 000000 = Black, EC0016 = DB red)") + ":</label><br><input name = 'color' value = '" + String(hubColorRgb, HEX) + "'><br>";
        chunk += "<button type='submit'>" + translate("Apply") + "</button></form><hr>";

        if (used + 5818 > total) {
            chunk += "<div style='color:red;font-weight:bold;'>" + translate("Warning: Not enough free space to upload new hand sets!Free up some space first") + ".</div><br><br>";
        }
        else {

            chunk += "<h3>" + translate("Upload New Hand Set") + "</h3>";
            chunk += "<small>" + translate("Requirements") + ": " + String(HAND_WIDTH) + " x " + String(HAND_HEIGHT) + " pixels, 16 - bit BMP(RGB565), <br>name must start with <code>hand_set + no + _hour, _minute or _second.bmp e.g.hand_set1_second.bmp</code><br>Pivot point : " + String(int(HAND_WIDTH / 2)) + " / " + String(int(HAND_HEIGHT * 0.77)) + "<br><br>";
            chunk += "<form method='POST' action='/uploadhandset' enctype='multipart/form-data'>";

            chunk += "File: <input type='file' name='upload' accept='.bmp' multiple required><br><br>";
            chunk += "<button type='submit'>" + translate("Upload to Set") + "</button></form>";
        }
        chunk += "<br><br>";
        chunk += generateNavigation(); // Navigation einfügen

        chunk += "<br><br>";
        chunk += "</body></html>";
        webserver.sendContent(chunk);
        webserver.sendContent(""); // Ende der Chunked-Uebertragung signalisieren
        });

    webserver.on("/setcenter", HTTP_POST, []() {
        if (webserver.hasArg("size") && webserver.hasArg("color")) {
            hubSize = webserver.arg("size").toInt();
            uint32_t rgb = (uint32_t)strtoul(webserver.arg("color").c_str(), nullptr, 16);

            // Convert 24-bit RGB888 to RGB565
            uint8_t r = (rgb >> 16) & 0xFF;
            uint8_t g = (rgb >> 8) & 0xFF;
            uint8_t b = rgb & 0xFF;
            uint16_t rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);

            preferences.putUInt(PK_CENTER_SIZE, hubSize);
            preferences.putLong(PK_CENTER_COLOR, rgb);

            hubColor = rgb565;

        }
        webserver.send(200, "text/html",
            "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='0; url=/handsets'>"
            "<title>Updated</title></head><body style='font-family:Arial;text-align:center;'>"
            "<h2>Centre point updated</h2><p>back to handsets in 3 seconds</p></body></html>");
        });

    //  Handsets Datei-Upload verarbeiten
    webserver.on("/uploadhandset", HTTP_POST, []() {
        if (uploadSuccess) {
            // Sicherheitsprüfung auf Dateinamenmuster
            if (!uploadFilePath.endsWith(".bmp") || !uploadFilePath.startsWith("/hand_set")) {
                String errorHtml = "<!DOCTYPE html><html><head><title>Upload Failed</title><meta name='viewport' content='width=device-width, initial-scale=1'></head><body style='font-family:Arial;text-align:center;'>";
                errorHtml += "<h2>Upload failed</h2>";
                errorHtml += "<p>Only .bmp files starting with <code>hand_</code> are accepted for handset upload.</p>";
                errorHtml += "<a href='/handsets'>Try again</a></body></html>";
                webserver.send(400, "text/html", errorHtml);
                return;
            }
            String setId = webserver.arg("set");
            //  String target = server.arg("target");
            String dir = "/";
            if (!LittleFS.exists(dir)) LittleFS.mkdir(dir);
            //  String finalPath = "/hand_set" + set + "_" + target + ".bmp";
            //  LittleFS.rename(uploadFilePath, finalPath);
            //  DEBUG_PRINTLN("[UPLOAD] Hand uploaded to: " + finalPath);
            DEBUG_PRINTLN("[UPLOAD] Hand uploaded to: " + uploadFilePath);
            webserver.sendHeader("Location", "/handsets", true);
            webserver.send(302, "text/plain", "");
        }
        else {
            webserver.send(500, "text/html", " Upload failed!<br><a href='/handsets'>Try again</a>");
        }
        }, handleFileUpload);


    // Handset setzen
    webserver.on("/sethandset", HTTP_GET, []() {
        if (webserver.hasArg("set")) {
            String chosen = webserver.arg("set");
            preferences.putString(PK_HANDSET, chosen);
            // DEBUG_PRINTLN("[HANDSET] Set to: " + chosen);
            freeClockFaceBuffer();
            loadClockFace();
            loadHandSprites();
            updateClock();
            webserver.sendHeader("Location", "/handsets", true);
            webserver.send(302, "text/plain", "");
        }
        else {
            webserver.send(400, "text/plain", "Missing set name");
        }
        });

    // Handset löschen
    webserver.on("/deletehandset", HTTP_GET, []() {
        if (webserver.hasArg("set")) {
            String setId = webserver.arg("set");
            String targets[] = { "hour", "minute", "second" };
            for (const String& target : targets) {
                String path = "/hand_set" + setId + "_" + target + ".bmp";
                DEBUG_PRINTLN("[DELETE] Looking for: " + path);
                if (LittleFS.exists(path)) {
                    LittleFS.remove(path);
                    DEBUG_PRINTLN("[DELETE] Removed: " + path);
                }
            }
            webserver.sendHeader("Location", "/handsets", true);
            webserver.send(302, "text/plain", "");
        }
        else {
            webserver.send(400, "text/plain", "Missing set name");
        }
        });

    // ESP neu starten
    webserver.on("/reboot", HTTP_GET, []() {
        webserver.send(200, "text/html", "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='10; url=/'><title>Rebooting</title></head><body style='font-family:Arial;text-align:center;'><h2>" + translate("Rebooting...") + "</h2><p>" + translate("Return to the main page in 10 seconds or refresh the website when the ESP is online again") + ".</p></body></html>");
        espReboot();
        });

    // Factory Reset    
    webserver.on("/factoryReset", HTTP_GET, []() {
        factoryReset();
        });

    // Sofortige Zeitsynchronisation
    webserver.on("/syncnow", HTTP_POST, []() {
        setupNTP();
       // struct tm timeinfo;
        getLocalTime(&timeinfo, 100);

        char timeStr[32];
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);

        String html = "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='0; url=/'>"
            "<title>Time Synced</title></head><body style='font-family:Arial;text-align:center;'>"
            "<h2>Time synced</h2><p>" + String(timeStr) + "</p><p>Returning to main page in 3 seconds.</p></body></html>";

        webserver.send(200, "text/html", html);
        });

}


// Handhabt den Datei-Upload
void handleFileUpload() {
    HTTPUpload& upload = webserver.upload();

    if (upload.status == UPLOAD_FILE_START) {
        uploadFilePath = upload.filename;
       // uploadFilePath.replace(".", "");
       // uploadFilePath.replace("#", "_");

        if (!uploadFilePath.startsWith("/")) uploadFilePath = "/" + uploadFilePath;

        // Nur bestimmte Dateinamenmuster zulassen
        if (!uploadFilePath.endsWith(".bmp") ||
            !(uploadFilePath.startsWith("/face_") || uploadFilePath.startsWith("/hand_set"))) {
            DEBUG_PRINTLN("[UPLOAD] Invalid filename: must start with 'face_' or 'hand_set' and end with '.bmp' : " + uploadFilePath);
            uploadSuccess = false;
            return;
        }

        DEBUG_PRINTLN("[UPLOAD] Start: " + uploadFilePath);
        uploadFile = LittleFS.open(uploadFilePath, FILE_WRITE);
        uploadSuccess = uploadFile ? true : false;
    }
    else if (upload.status == UPLOAD_FILE_WRITE) {
        if (uploadSuccess && uploadFile) {
            uploadFile.write(upload.buf, upload.currentSize);
        }
    }
    else if (upload.status == UPLOAD_FILE_END) {
        if (uploadSuccess && uploadFile) {
            uploadFile.close();
            if (LittleFS.exists(uploadFilePath)) {
                DEBUG_PRINTLN("[UPLOAD] Finished OK: " + uploadFilePath);
                String lowerPath = uploadFilePath;
                lowerPath.toLowerCase();
                if (lowerPath.endsWith(".bmp")) {
                  //  bool isHand = uploadFilePath.indexOf("hour") > 0 || uploadFilePath.indexOf("minute") > 0 || uploadFilePath.indexOf("second") > 0;
                    if (uploadFilePath.startsWith("/face_")) {
                        DEBUG_PRINTLN("[UPLOAD] Detected Clock Face upload");

                        if (!scaleAndSaveBmp(uploadFilePath.c_str(), uploadFilePath.c_str(), TFT_WIDTH, TFT_HEIGHT)) {
                            DEBUG_PRINTLN("[UPLOAD] Scaling failed!");
                            uploadSuccess = false;
                            return;
                        }

                    }
                    else if (uploadFilePath.startsWith("/hand_set")) {
                        DEBUG_PRINTLN("[UPLOAD] Detected Clock Hand upload");

                        if (!scaleAndSaveBmp(uploadFilePath.c_str(), uploadFilePath.c_str(), HAND_WIDTH, HAND_HEIGHT)) {
                            DEBUG_PRINTLN("[UPLOAD] Scaling failed!");
                            uploadSuccess = false;
                            return;
                        }
                    }                    
                }
            }
            else {
                DEBUG_PRINTLN("[UPLOAD] Finished but file missing!");
                uploadSuccess = false;
            }
        }
        else {
            DEBUG_PRINTLN("[UPLOAD] Failed during writing");
        }
    }
}




