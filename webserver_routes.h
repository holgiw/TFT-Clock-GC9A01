#pragma once
    // ### Webinterface: alle HTTP-Routen & HTML-Generierung ##############
    // Benoetigt globals.h, config.h, prefs_keys.h und declarations.h (werden
    // zentral in uhr3.ino VOR dieser Datei eingebunden).

    // Generiert den HTML-Header für die Weboberfläche
    String generateHtmlHeader() {
        String html = "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
        html.reserve(512);  // Header: klein, wird auf jeder Seite einmal aufgerufen
        html += "<style>body{font-family:Arial;text-align:center;padding-top:110px;}input,select,button{margin:10px;padding:10px;width:80%;}";
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
        html += " (" + translate("Free") + ": " + String((total - used) / 1024) + " KB)";
        html += "<br>" + translate("Version") + ": " + String(version);

        // Oben rechts fest positioniert, neben der Live-Vorschau (oben links) -
        // kompaktere Schrift, damit die paar Zeilen nicht mehr Hoehe brauchen
        // als das 90px hohe Vorschaubild gegenueber.
        String boxed = "<div style='position:fixed;top:0;left:110px;max-width:calc(55% + 70px);height:100px;box-sizing:border-box;background:#fff;border:2px solid #333;border-radius:8px;padding:4px 8px;font-size:0.78em;line-height:1.25;text-align:left;white-space:nowrap;overflow-x:auto;overflow-y:auto;z-index:1000;'>";
        boxed += html;
        boxed += "</div><hr>";

        setLedOff();
        return boxed;
    }


    // Navigationsleiste generieren
    String generateNavigation() {
     /*   if (WiFi.getMode() != WIFI_STA) {
            DEBUG_PRINTLN("[HTML] Skipping HTML navigation");
            return "";
        }
        */

        String nav = "<form id='previewSaveForm' method='POST' action='/api/createPreset' style='display:none;'><input type='hidden' id='previewSaveName' name='name'></form>";
        nav += "<img src='/currentpreview' onclick=\"var n=prompt('" + translate("Enter a name for the new preset (leave empty for automatic naming)") + "'); if(n !== null) { document.getElementById('previewSaveName').value = n; document.getElementById('previewSaveForm').submit(); }\" title='" + translate("Save the current clock settings as a new preset?") + "' style='position:fixed;top:0;left:0;width:100px;height:100px;box-sizing:border-box;border:2px solid #333;border-radius:8px;background:#fff;z-index:1000;cursor:pointer;'>";
        nav += "<style>";
        nav += "a { text-decoration: underline; color: blue; font-weight: bold; }";
        nav += "a:hover { text-decoration: underline; }";
        nav += ".navToggle { display: none; cursor: pointer; font-size: 1.8em; user-select: none; }";
        nav += "@media (max-width: 600px) {";
        nav += "  .navToggle { display: inline-block; }";
        nav += "  .navLinks { display: none; }";
        nav += "  .navLinks.navOpen { display: block; }";
        nav += "  .navLinks a, .navLinks span { display: block; margin: 8px 0 !important; }";
        nav += "}";
        nav += "</style>";
        nav += "<div style='text-align:center; margin-bottom:20px;'>";
        nav += "<span class='navToggle' onclick=\"document.querySelectorAll('.navLinks').forEach(function(e){e.classList.toggle('navOpen');})\">&#9776;</span>";
        nav += "<div class='navLinks'>";

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
            {"/status", translate("Status"), ""},
            {"/files", translate("File&nbsp;Manager"), ""},
            {"/reboot", translate("Reboot"), translate("Are you sure you want to reboot?")},
            {"/factoryReset", translate("Factory&nbsp;Reset"), ""}
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

            // Zeilenumbruch nach "Status" zur thematischen Trennung (Status/Betrieb
            // vs. Dateiverwaltung/Reset) - bewusst hier im Code statt in der
            // Übersetzung, da es reine Layout-Struktur ist, keine Textinhalt.
            if (item.path == "/status") {
                nav += "<br>";
            }
        }

        nav += "</div>"; // Ende .navLinks
        nav += "</div>";
    
        return nav;
    }


    // Zeigt eine einheitliche Erfolgsmeldung, wenn die Route per Weiterleitung
    // einen "msg"-Parameter mitgibt (translate()-Schluessel, automatisch uebersetzt) -
    // blendet sich nach ein paar Sekunden per JS aus, ein Muster fuer alle Aktionen.
    String generateFlashMessage() {
        if (!webserver.hasArg("msg")) return "";
        String message = translate(webserver.arg("msg"));
        String html = "<div id='flashMsg' style='background:#d4edda;color:#155724;border:1px solid #c3e6cb;border-radius:6px;padding:10px 15px;margin:10px auto;max-width:500px;'>";
        html += "&#9989; " + message;
        html += "</div>";
        html += "<script>setTimeout(function(){var e=document.getElementById('flashMsg'); if(e) e.style.display='none';}, 4000);</script>";
        return html;
    }


    // Sprachselector generieren
    String generateLanguageSelector() {
        String html = "<form method='POST' action='/setLanguage'>";
        html.reserve(512);  // Sprachauswahl: klein
        html += "<label for='lang'>Language/Sprache/Langue:</label>";
        html += "<select name='lang' onchange='this.form.submit()'>";
        html += "<option value='en'" + String(currentLanguage == "en" ? " selected" : "") + ">Englisch / English</option>";
        html += "<option value='de'" + String(currentLanguage == "de" ? " selected" : "") + ">Deutsch / German</option>";
        html += "<option value='fr'" + String(currentLanguage == "fr" ? " selected" : "") + ">Franz&ouml;sisch / Fran&ccedil;ais</option>";
        html += "</select>";
        html += "<noscript><button type='submit'>Save / Speichern / Enregistrer</button></noscript>";
        html += "</form><hr>";
        return html;
    }


    // Vergleicht zwei Dateinamen "natuerlich": Ziffernfolgen werden als Zahl
    // verglichen statt zeichenweise, damit z.B. "hand_set2..." vor "hand_set10..." einsortiert wird.
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


    // Sortiert eine Liste von Dateinamen "natuerlich" (siehe naturalLess()) - Insertion-Sort
    // statt std::sort, um keine <algorithm>-Abhaengigkeit zu benoetigen (Dateianzahl ueberschaubar).
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


    // Bereinigt eine Nutzereingabe zu einem gueltigen Hostnamen (RFC 952/1123):
    // nur Buchstaben, Ziffern und Bindestriche; Leerzeichen/Unterstriche werden
    // zu Bindestrichen, alle anderen ungueltigen Zeichen (Umlaute, Sonderzeichen,
    // etc.) werden entfernt; darf nicht mit Bindestrich beginnen/enden; max. 30 Zeichen.
    String sanitizeHostname(String input) {
        input.trim();
        String result;
        for (unsigned int i = 0; i < input.length(); i++) {
            char c = input[i];
            if (isAlphaNumeric(c) || c == '-') {
                result += c;
            }
            else if (c == ' ' || c == '_') {
                result += '-';
            }
            // alle anderen Zeichen werden stillschweigend entfernt
        }
        while (result.startsWith("-")) result = result.substring(1);
        while (result.endsWith("-")) result = result.substring(0, result.length() - 1);
        if (result.length() > 30) result = result.substring(0, 30);
        while (result.endsWith("-")) result = result.substring(0, result.length() - 1);
        return result;
    }


    // Sendet eine 302-Weiterleitung an location - buendelt das sonst ueberall
    // wiederholte sendHeader("Location", ...)/send(302, ...)-Paar.
    void redirectTo(const String& location, const String& body) {
        webserver.sendHeader("Location", location, true);
        webserver.send(302, "text/plain", body);
    }


    // Erzeugt den fuer fast jede Seite gleichen Seitenanfang (Header + Statusleiste
    // + Navigation) - Reihenfolge entspricht der bisherigen, wiederholten Aufrufkette.
    String beginPage() {
        String html = generateHtmlHeader();
        html += generateHtmlStatus();
        html += generateNavigation();
        return html;
    }


    // Liest fuer jeden konfigurierten WLAN-Slot einen evtl. mitgesendeten NTP-Server-
    // Parameter aus der Anfrage und speichert ihn (nur bei Aenderung) in ntpServers[]/
    // Preferences - gemeinsame Logik von /api/setMode und /set_timezone.
    void updateNtpServersFromRequest() {
        for (int i = 0; i < MAX_WLAN; i++) {
            String argName = pkNtpServer(i);
            if (webserver.hasArg(argName)) {
                strncpy(ntpServers[i], webserver.arg(argName).c_str(), sizeof(ntpServers[i]) - 1);
                ntpServers[i][sizeof(ntpServers[i]) - 1] = '\0'; // Null-terminieren
                if (preferences.getString(argName.c_str(), "") != String(ntpServers[i])) {
                    preferences.putString(argName.c_str(), ntpServers[i]);
                }
            }
        }
    }


    // Webserver-API-Endpunkte einrichten
    void setupWebServer() {

        // Captive-Portal-Erkennung: Android/iOS/Windows fragen beim Verbinden diese
        // bekannten URLs ab - Redirect (statt 404) auf die Konfigurationsseite oeffnet
        // beim AP ("clock123") automatisch ein Browserfenster, wirkt mit dnsServer.processNextRequest() zusammen.
        auto captivePortalRedirect = []() {
            redirectTo("http://" + ipAddress + "/");
            };

        webserver.on("/generate_204", HTTP_GET, captivePortalRedirect);       // Android
        webserver.on("/gen_204", HTTP_GET, captivePortalRedirect);            // Android (aeltere Versionen)
        webserver.on("/hotspot-detect.html", HTTP_GET, captivePortalRedirect); // iOS / macOS
        webserver.on("/library/test/success.html", HTTP_GET, captivePortalRedirect); // iOS / macOS (alternative)
        webserver.on("/ncsi.txt", HTTP_GET, captivePortalRedirect);           // Windows
        webserver.on("/connecttest.txt", HTTP_GET, captivePortalRedirect);    // Windows

        // Catch-all fuer alle sonstigen, unbekannten Anfragen (z.B. Varianten
        // der obigen URLs oder Domains, die nicht explizit registriert sind) -
        // statt eines 404 lieber ebenfalls auf die Konfigurationsseite leiten.
        webserver.onNotFound(captivePortalRedirect);

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
                    redirectTo("/?msg=Language%20updated");
                    return;
                }

                if (availableLanguages.count(lang)) {
                    saveLanguage(lang);
                    // webserver.send(200, "text/plain", "Language updated to " + lang);
                    redirectTo("/?msg=Language%20updated");
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

        // Startet eine WPS-Anfrage per Web-Button, um ein neues WLAN hinzuzufuegen.
        // Kehrt SOFORT zurueck (kein Blockieren des Webservers) - der eigentliche
        // Verbindungsversuch und das Speichern der Zugangsdaten laeuft asynchron
        // in loop() (siehe uhr3.ino), analog zur bestehenden WPS-Logik in setup().
        // GET zusaetzlich zu POST registriert (analog zu /api/resetWiFi), damit
        // die Route auch bei direkter Browser-Navigation erreichbar ist.
        webserver.on("/api/startWPS", HTTP_GET, []() {
            wpsPreviousSsid = WiFi.isConnected() ? WiFi.SSID() : "";
            redirectTo("/?msg=WPS%20active%20-%20press%20the%20WPS%20button%20on%20your%20router%20now%20(within%202%20minutes)");
            // Erst NACH dem Senden der Antwort WPS starten - startWPS() stoerte
            // sonst vermutlich die noch offene HTTP-Verbindung (Browser zeigte
            // "Seite nicht erreichbar" statt die Weiterleitung zu erhalten).
            startWPS();
            wpsPending = true;
            wpsStartMillis = millis();
            });

        webserver.on("/api/startWPS", HTTP_POST, []() {
            wpsPreviousSsid = WiFi.isConnected() ? WiFi.SSID() : "";
            redirectTo("/?msg=WPS%20active%20-%20press%20the%20WPS%20button%20on%20your%20router%20now%20(within%202%20minutes)");
            startWPS();
            wpsPending = true;
            wpsStartMillis = millis();
            });

        // Speichert einen benutzerdefinierten Hostnamen (wird erst nach einem
        // Neustart wirksam, da WiFi.setHostname()/MDNS.begin() nur einmalig
        // beim Verbindungsaufbau in connectWiFi() aufgerufen werden).
        // Bereinigt eine Nutzereingabe zu einem gueltigen Hostnamen (RFC 952/1123):
        // nur Buchstaben, Ziffern und Bindestriche; Leerzeichen/Unterstriche werden
        // zu Bindestrichen, alle anderen ungueltigen Zeichen (Umlaute, Sonderzeichen,
        // etc.) werden entfernt; darf nicht mit Bindestrich beginnen/enden; max. 30 Zeichen.

        webserver.on("/sethostname", HTTP_POST, []() {
            if (webserver.hasArg("hostname")) {
                String newHostname = sanitizeHostname(webserver.arg("hostname"));

                if (newHostname.isEmpty()) {
                    // Kein gueltiger Hostname aus der Eingabe extrahierbar - den
                    // gespeicherten Override entfernen, damit beim naechsten Boot
                    // wieder automatisch "clock_" + letzte MAC-Stellen generiert
                    // wird (siehe connectWiFi() in wifi_manager.h).
                    preferences.remove(PK_HOSTNAME);
                    redirectTo("/?msg=No%20valid%20hostname%20could%20be%20derived%20from%20the%20input%20-%20falling%20back%20to%20the%20automatic%20name%20based%20on%20the%20MAC%20address");
                    return;
                }

                preferences.putString(PK_HOSTNAME, newHostname);
                redirectTo("/?msg=Hostname%20saved%20-%20requires%20a%20reboot%20to%20take%20effect");
            }
            else {
                webserver.send(400, "text/plain", "Missing parameter");
            }
            });

        webserver.on("/api/createPreset", HTTP_POST, []() {
            String customName = webserver.hasArg("name") ? webserver.arg("name") : "";
            bool created = createPresetFromPreferences(customName); // Erstellt ein neues Preset, falls noch ein Slot frei ist

            if (!created) {
                String html = beginPage();
                html += "<h2 style='color:red;'>" + translate("Maximum number of presets reached - delete an existing preset first") + "</h2>";
                html += "<a href='/presets'><button type='button'>" + translate("Back") + "</button></a>";
                html += "</body></html>";
                webserver.send(200, "text/html", html);
                return;
            }

            // Weiterleitung zur Presets-Seite
            redirectTo("/presets?msg=Preset%20created", "Redirecting to /presets..");
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

            updateNtpServersFromRequest();

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
                    redirectTo("/presets?msg=Preset%20applied", "Redirecting to /presets..");
                    return;
                }
            }

            webserver.send(200, "text/plain", "ok");
            });

        // Preset-Verwaltung
        webserver.on("/presets", HTTP_GET, []() {
            webserver.setContentLength(CONTENT_LENGTH_UNKNOWN);
            webserver.send(200, "text/html", "");

            String chunk = beginPage();
            chunk.reserve(1024);
            chunk += generateFlashMessage();
            chunk += "<h2>" + translate("Manage Presets") + "</h2>";

            // Links oben anzeigen
            chunk += "<div style='text-align:center;'>";

            if (pingHostname) {
                chunk += "<p>" + translate("Use the host name") + " <strong>" + String(hostname) + ".local</strong> " + translate("instead of the IP address for better reliability") + ".</p>";
            }

            chunk += "<ul style='list-style-type:none; padding:0; display:inline-block; text-align:left;'>";

         
            String espHost = "http://" + String(hostname) + ".local"; // Aktueller Hostname des ESP

            chunk += "<script>";
            chunk += "function copyPresetLink(text, el) {";
            chunk += "  function showCopied() {";
            chunk += "    var original = el.innerHTML;";
            chunk += "    el.innerHTML = '&#9989;';";
            chunk += "    setTimeout(function() { el.innerHTML = original; }, 1200);";
            chunk += "  }";
            chunk += "  if (navigator.clipboard && navigator.clipboard.writeText) {";
            chunk += "    navigator.clipboard.writeText(text).then(showCopied);";
            chunk += "  } else {";
            chunk += "    var temp = document.createElement('textarea');";
            chunk += "    temp.value = text;";
            chunk += "    document.body.appendChild(temp);";
            chunk += "    temp.select();";
            chunk += "    document.execCommand('copy');";
            chunk += "    document.body.removeChild(temp);";
            chunk += "    showCopied();";
            chunk += "  }";
            chunk += "}";
            chunk += "</script>";

            chunk += "<div style='display:flex;flex-wrap:wrap;gap:18px;justify-content:center;align-items:flex-start;'>";

            webserver.sendContent(chunk);
            chunk = "";

            // Nur die Anzeige-Reihenfolge sortieren (Array-Indizes bleiben unveraendert,
            // damit Rename-/Delete-/Vorschau-Links stimmen) - Insertion-Sort wie bei
            // naturalSortNames(), um keine <algorithm>-Abhaengigkeit zu benoetigen.
            std::vector<int> sortedIndices;
            for (int i = 0; i < MAX_PRESETS; i++) {
                if (!presets[i].name.isEmpty() && !presets[i].url.isEmpty()) {
                    sortedIndices.push_back(i);
                }
            }
            for (size_t i = 1; i < sortedIndices.size(); i++) {
                int key = sortedIndices[i];
                long j = (long)i - 1;
                while (j >= 0 && naturalLess(presets[key].name, presets[sortedIndices[j]].name)) {
                    sortedIndices[j + 1] = sortedIndices[j];
                    j--;
                }
                sortedIndices[j + 1] = key;
            }

            int rowCount = 0;
            for (int i : sortedIndices) {
                {
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

                    chunk += "<div style='text-align:center;border:1px solid #ccc;border-radius:6px;padding:8px;width:140px;'>";
                    chunk += "<a href='" + displayUrl + "'><img src='/presetpreview?index=" + String(i) + "' style='width:90px;height:90px;'></a>";
                    chunk += "<br><a href='" + displayUrl + "'>" + presets[i].name + "</a>";
                    String presetName = presets[i].name;
                    presetName.replace(" ", "_"); // Ersetze Leerzeichen durch Unterstriche
                    String ipLink = "http://" + ipAddress + "/api/setPreset?name=" + presetName;
                    chunk += "<br><span onclick=\"copyPresetLink('" + ipLink + "', this)\" style='cursor:pointer;font-size:1.3em;' title='" + translate("Copy link") + "'>&#128203;</span>";
                    if (pingHostname) {
                        String hostLink = espHost + "/api/setPreset?name=" + presetName;
                        chunk += " <span onclick=\"copyPresetLink('" + hostLink + "', this)\" style='cursor:pointer;font-size:1.3em;' title='" + translate("Copy link") + " (" + espHost + ")'>&#128203;</span>";
                    }
                    chunk += "<br><a href='/renamepreset_form?index=" + String(i) + "'>" + translate("Rename") + "</a> ";
                    chunk += "<a href='/deletepreset?index=" + String(i) + "' onclick='return confirm(\"" + translate("Delete") + " " + presets[i].name + "?\")'>" + translate("Delete") + "</a>";
                    chunk += "</div>";

                    rowCount++;
                    if (rowCount % 5 == 0) {
                        webserver.sendContent(chunk);
                        chunk = "";
                    }
                }
            }
            chunk += "</div>";
            chunk += "<p>" + translate("Presets used") + ": " + String(rowCount) + " / " + String(MAX_PRESETS) + "</p>";
            chunk += "</div><hr>";

            chunk += "<button type='button' id='ghPresetBtn' onclick='loadPresetsFromGithub()'>" + translate("Load Presets from GitHub") + "</button>";
            chunk += "<div id='ghPresetStatus'></div>";

            // Fuer den JS-Merge-Abgleich: bereits vorhandene Preset-Namen sowie
            // vorhandene Zifferblatt-/Zeigersatz-Dateien bereitstellen, damit der
            // GitHub-Download nur wirklich Neues hinzufuegt und automatisch die
            // dafuer noch fehlenden Zifferblaetter/Zeigersaetze mitlaedt.
            std::vector<String> existingPresetNamesForJs;
            for (int gi = 0; gi < MAX_PRESETS; gi++) {
                if (!presets[gi].name.isEmpty() && !presets[gi].url.isEmpty()) {
                    existingPresetNamesForJs.push_back(presets[gi].name);
                }
            }
            std::vector<String> existingFacesForJs;
            std::vector<String> existingHandsForJs;
            File ghScanRoot = LittleFS.open("/");
            File ghScanFile = ghScanRoot.openNextFile();
            while (ghScanFile) {
                String n = ghScanFile.name();
                if (!ghScanFile.isDirectory()) {
                    if (n.startsWith("face_") && n.endsWith(".bmp")) existingFacesForJs.push_back(n);
                    else if (n.startsWith("hand_set") && n.endsWith(".bmp")) existingHandsForJs.push_back(n);
                }
                ghScanFile = ghScanRoot.openNextFile();
            }

            chunk += "<script>";
            chunk += "var existingPresetNames = [";
            for (size_t gi = 0; gi < existingPresetNamesForJs.size(); gi++) {
                if (gi > 0) chunk += ",";
                chunk += "\"" + existingPresetNamesForJs[gi] + "\"";
            }
            chunk += "];";
            chunk += "var existingFacesForPresets = [";
            for (size_t gi = 0; gi < existingFacesForJs.size(); gi++) {
                if (gi > 0) chunk += ",";
                chunk += "\"" + existingFacesForJs[gi] + "\"";
            }
            chunk += "];";
            chunk += "var existingHandsForPresets = [";
            for (size_t gi = 0; gi < existingHandsForJs.size(); gi++) {
                if (gi > 0) chunk += ",";
                chunk += "\"" + existingHandsForJs[gi] + "\"";
            }
            chunk += "];";
            chunk += "async function loadPresetsFromGithub() {";
            chunk += "  var btn = document.getElementById('ghPresetBtn');";
            chunk += "  var status = document.getElementById('ghPresetStatus');";
            chunk += "  btn.disabled = true;";
            chunk += "  try {";
            chunk += "    status.innerHTML = '" + translate("Checking GitHub for new files") + "...';";
            chunk += "    var text = null;";
            chunk += "    for (var attempt = 0; attempt < 3 && text === null; attempt++) {";
            chunk += "      try {";
            chunk += "        var r = await fetch('" GITHUB_RAW_BASE "presets.txt');";
            chunk += "        if (r.ok) text = await r.text();";
            chunk += "      } catch (e) {}";
            chunk += "      if (text === null && attempt < 2) await new Promise(function(resolve) { setTimeout(resolve, 1500); });";
            chunk += "    }";
            chunk += "    if (text === null) throw new Error('unreachable');";
            chunk += "    var lines = text.split('\\n').map(function(l){return l.trim();}).filter(function(l){return l.length > 0;});";
            chunk += "    var newLines = [];";
            chunk += "    var neededFiles = {};";
            chunk += "    for (var i = 0; i < lines.length; i++) {";
            chunk += "      var tabIdx = lines[i].indexOf('\\t');";
            chunk += "      if (tabIdx === -1) continue;";
            chunk += "      var name = lines[i].substring(0, tabIdx);";
            chunk += "      var url = lines[i].substring(tabIdx + 1);";
            chunk += "      if (existingPresetNames.indexOf(name) !== -1) continue;";
            chunk += "      newLines.push(lines[i]);";
            chunk += "      var qIdx = url.indexOf('?');";
            chunk += "      if (qIdx === -1) continue;";
            chunk += "      var params = new URLSearchParams(url.substring(qIdx + 1));";
            chunk += "      var face = params.get('face');";
            chunk += "      if (face) {";
            chunk += "        var faceName = face.charAt(0) === '/' ? face.substring(1) : face;";
            chunk += "        if (faceName !== 'face_default.bmp' && existingFacesForPresets.indexOf(faceName) === -1) neededFiles[faceName] = true;";
            chunk += "      }";
            chunk += "      var handSet = params.get('handSet');";
            chunk += "      if (handSet && handSet !== 'default') {";
            chunk += "        ['hour','minute','second'].forEach(function(part) {";
            chunk += "          var hn = 'hand_set' + handSet + '_' + part + '.bmp';";
            chunk += "          if (existingHandsForPresets.indexOf(hn) === -1) neededFiles[hn] = true;";
            chunk += "        });";
            chunk += "      }";
            chunk += "    }";
            chunk += "    if (newLines.length === 0) { status.innerHTML = '" + translate("All presets already up to date") + ".'; btn.disabled = false; return; }";
            chunk += "    var missingNames = Object.keys(neededFiles);";
            chunk += "    if (missingNames.length > 0) {";
            chunk += "      status.innerHTML = '" + translate("Checking GitHub for new files") + "...';";
            chunk += "      var listResp = await fetch('" GITHUB_API_CONTENTS_BASE + String(CLOCK_WIDTH) + "');";
            chunk += "      var files = await listResp.json();";
            chunk += "      var fileMap = {};";
            chunk += "      files.forEach(function(f) { fileMap[f.name] = f.download_url; });";
            chunk += "      for (var j = 0; j < missingNames.length; j++) {";
            chunk += "        var fn = missingNames[j];";
            chunk += "        if (!fileMap[fn]) continue;";
            chunk += "        status.innerHTML = '" + translate("Downloading") + " ' + fn + ' (' + (j + 1) + '/' + missingNames.length + ')...';";
            chunk += "        var blob = await (await fetch(fileMap[fn])).blob();";
            chunk += "        var fd = new FormData();";
            chunk += "        fd.append('upload', blob, fn);";
            chunk += "        status.innerHTML = '" + translate("Converting") + " ' + fn + ' (' + (j + 1) + '/' + missingNames.length + ')...';";
            chunk += "        var target = fn.indexOf('face_') === 0 ? '/upload' : '/uploadhandset';";
            chunk += "        await fetch(target, { method: 'POST', body: fd });";
            chunk += "      }";
            chunk += "    }";
            chunk += "    status.innerHTML = '" + translate("Downloading") + " presets.txt...';";
            chunk += "    var presetBlob = new Blob([newLines.join('\\n')], { type: 'text/plain' });";
            chunk += "    var presetFd = new FormData();";
            chunk += "    presetFd.append('presetfile', presetBlob, 'presets.txt');";
            chunk += "    await fetch('/importpresetsmerge', { method: 'POST', body: presetFd });";
            chunk += "    status.innerHTML = '" + translate("Done - reloading") + "...';";
            chunk += "    location.href = location.pathname;";
            chunk += "  } catch (e) {";
            chunk += "    status.innerHTML = '" + translate("Failed to reach GitHub - check your internet connection") + ".';";
            chunk += "    btn.disabled = false;";
            chunk += "  }";
            chunk += "}";
            chunk += "</script>";
            chunk += "<hr>";

            // Falls beim Laden der Seite keine Presets vorhanden sind, automatisch
            // fragen, ob welche von GitHub geladen werden sollen - relevant z.B.
            // nach einem Werksreset oder erstmaligem Einrichten. Prueft die
            // Erreichbarkeit von GitHub mehrfach (mit kurzer Pause dazwischen),
            // bevor gefragt wird, damit direkt nach einem Neustart (WLAN evtl.
            // noch nicht ganz stabil) keine Nachfrage fuer eine dann sowieso
            // fehlschlagende Aktion erscheint.
            if (rowCount == 0) {
                chunk += "<script>";
                chunk += "async function checkGithubReachable(retries) {";
                chunk += "  for (var i = 0; i < retries; i++) {";
                chunk += "    try {";
                chunk += "      var r = await fetch('" GITHUB_RAW_BASE "presets.txt');";
                chunk += "      if (r.ok) return true;";
                chunk += "    } catch (e) {";
                chunk += "    }";
                chunk += "    await new Promise(function(resolve) { setTimeout(resolve, 1500); });";
                chunk += "  }";
                chunk += "  return false;";
                chunk += "}";
                chunk += "(async function() {";
                chunk += "  var reachable = await checkGithubReachable(3);";
                chunk += "  if (!reachable) return;";
                chunk += "  if (confirm('" + translate("No presets found. Load recommended presets from GitHub?") + "')) {";
                chunk += "    loadPresetsFromGithub();";
                chunk += "  }";
                chunk += "})();";
                chunk += "</script>";
            }

            chunk += "<h3>" + translate("Create New Preset") + "</h3>";
            chunk += "<form method='POST' action='/api/createPreset'>";
            chunk += "<button type='submit'>" + translate("Create Preset from Current Settings") + "</button>";
            chunk += "</form>";
            chunk += "<hr>";

            // Presets als Datei sichern/wiederherstellen
            chunk += "<h3>" + translate("Backup / Restore Presets") + "</h3>";
            chunk += "<a href='/exportpresets'><button type='button'>" + translate("Save Presets to File") + "</button></a> ";
            chunk += "<form method='POST' action='/importpresets' enctype='multipart/form-data' style='display:inline;'>";
            chunk += "<input type='file' name='presetfile' accept='.txt' required>";
            chunk += "<button type='submit'>" + translate("Load Presets from File") + "</button>";
            chunk += "</form> ";

            chunk += "</body></html>";
            webserver.sendContent(chunk);
            webserver.sendContent(""); // Ende der Chunked-Uebertragung signalisieren
            });

        // Alle belegten Presets (Name + URL) als herunterladbare Textdatei
        // exportieren - eine Zeile pro Preset, Name und URL durch Tab getrennt.
        webserver.on("/exportpresets", HTTP_GET, []() {
            String content;
            for (int i = 0; i < MAX_PRESETS; i++) {
                if (!presets[i].name.isEmpty() && !presets[i].url.isEmpty()) {
                    String exportUrl = presets[i].url;
                    // Host-Teil (aktuelle IP dieses Geraets) durch einen Platzhalter
                    // ersetzen: die Datei soll nicht wie eine feste Adresse fuer EIN
                    // bestimmtes Geraet aussehen. Wird beim Laden/Import ohnehin
                    // durch die dann aktuelle IP ersetzt (siehe loadPresets() und
                    // savePresets() - beide schneiden alles zwischen "http://" und
                    // dem naechsten "/" ab und setzen dort die aktuelle IP ein,
                    // unabhaengig davon, was zuvor dort stand).
                    if (exportUrl.startsWith("http://")) {
                        int ipEnd = exportUrl.indexOf('/', 7);
                        if (ipEnd != -1) {
                            exportUrl = "http://<clock-ip>" + exportUrl.substring(ipEnd);
                        }
                        else {
                            exportUrl = "http://<clock-ip>";
                        }
                    }
                    content += presets[i].name + "\t" + exportUrl + "\n";
                }
            }
            webserver.sendHeader("Content-Disposition", "attachment; filename=presets.txt");
            webserver.send(200, "text/plain", content);
            });

        // Presets aus einer zuvor per /exportpresets erzeugten Textdatei
        // wiederherstellen - ersetzt ALLE aktuell gespeicherten Presets.
        webserver.on("/importpresets", HTTP_POST, []() {
            if (presetImportSuccess) {
                redirectTo("/presets?msg=Presets%20imported%20successfully");
            }
            else {
                redirectTo("/presets?msg=Import%20failed%20-%20please%20check%20the%20file");
            }
            }, handlePresetImportUpload);

        // Wie /importpresets, loescht dabei aber keine bestehenden Presets - wird
        // vom GitHub-Download-Button auf /presets genutzt (siehe handlePresetMergeUpload()).
        webserver.on("/importpresetsmerge", HTTP_POST, []() {
            redirectTo("/presets?msg=Presets%20imported%20successfully");
            }, handlePresetMergeUpload);

        // API zum Restart des ESP
        webserver.on("/api/reboot", HTTP_GET, []() {
            webserver.send(200, "text/html", "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='0; url=/status'><title>Rebooting</title></head><body><h2>" + translate("Rebooting...") + "</h2></body></html>");
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
                        redirectTo(presets[i].url, "Redirecting to preset URL..");
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
        // Testet einen NTP-Server per direkter UDP-Anfrage, ohne die aktuelle
        // Zeitkonfiguration zu veraendern - fuer den "Test"-Button je Server-Feld.
        webserver.on("/api/testNtp", HTTP_GET, []() {
            if (!webserver.hasArg("server") || trim(webserver.arg("server")) == "") {
                webserver.send(400, "text/plain", "Missing parameter");
                return;
            }
            String result = testNtpServer(webserver.arg("server"));
            if (result == "") {
                webserver.send(200, "text/plain", "FAILED");
            }
            else {
                webserver.send(200, "text/plain", "OK|" + result);
            }
            });

        webserver.on("/set_timezone", HTTP_POST, []() {

            updateNtpServersFromRequest();

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

            redirectTo("/timezone_form?msg=Timezone%20updated");
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

            String html = beginPage();
            html.reserve(6144);  // Zeitzonen-Formular: lange Dropdown-Liste
            html += generateFlashMessage();
            html += "<h2>" + translate("NTP Server / Timezone (DST String)") + "</h2>";
            html += "<form method='POST' action='/set_timezone'>";

            for (int i = 0; i < MAX_WLAN; i++) {
                    html += "<div style='display:flex;gap:6px;align-items:center;flex-wrap:wrap;justify-content:center;'>";
                    html += "NTP Server" + String(i + 1) + " : <input type = 'text' id='ntpServerInput" + String(i + 1) + "' name = 'ntpServer" + String(i + 1) + "' value = '" + String(ntpServers[i]) + "' style='width:180px;'>";
                    html += "<button type='button' onclick='testNtp(" + String(i + 1) + ")' style='width:180px;'>" + translate("Test") + "</button>";
                    html += "</div>";
                    html += "<div id='ntpTestResult" + String(i + 1) + "'></div>";
                    if (trim(ntpServers[i]) == "") {
                        break;
                }
            }

            html += "<script>";
            html += "async function testNtp(idx) {";
            html += "  var input = document.getElementById('ntpServerInput' + idx);";
            html += "  var result = document.getElementById('ntpTestResult' + idx);";
            html += "  result.innerHTML = '" + translate("Testing") + "...';";
            html += "  try {";
            html += "    var r = await fetch('/api/testNtp?server=' + encodeURIComponent(input.value));";
            html += "    var text = await r.text();";
            html += "    if (text.indexOf('OK|') === 0) {";
            html += "      result.innerHTML = '<div style=\\'background:#d4edda;color:#155724;border:1px solid #c3e6cb;border-radius:6px;padding:8px 12px;margin:10px auto;max-width:400px;\\'>&#10004; ' + text.substring(3) + '</div>';";
            html += "    } else {";
            html += "      result.innerHTML = '<div style=\\'background:#f8d7da;color:#721c24;border:1px solid #f5c6cb;border-radius:6px;padding:8px 12px;margin:10px auto;max-width:400px;\\'>&#10008; " + translate("Server not reachable") + "</div>';";
            html += "    }";
            html += "  } catch (e) {";
            html += "    result.innerHTML = '<div style=\\'background:#f8d7da;color:#721c24;border:1px solid #f5c6cb;border-radius:6px;padding:8px 12px;margin:10px auto;max-width:400px;\\'>&#10008; " + translate("Server not reachable") + "</div>';";
            html += "  }";
            html += "}";
            html += "</script>";


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
            String html = beginPage();
            html.reserve(1024);  // Umbenennen-Formular: klein
            html += "<h2>" + translate("Rename File") + "</h2>";
            html += "<form action='/rename' method='POST'>";
            html += "<input type='hidden' name='old' value='" + oldName + "'>";
            html += "<label>" + translate("New Name") + ":</label><br>";
            html += "<input name='new' value='" + oldName + "' required><br><br>";
            html += "<button type='submit'>" + translate("Rename") + "</button></form>";
            html += "<br><a href='/files'><button type='button'>" + translate("Cancel") + "</button></a></body></html>";
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
                        String baseName = newName.startsWith("/") ? newName.substring(1) : newName;
                        String redirectTarget = "/files";
                        if (baseName.startsWith("face_") && baseName.endsWith(".bmp")) {
                            redirectTarget = "/listfilesFaces";
                        }
                        else if (baseName.startsWith("hand_set") && baseName.endsWith(".bmp")) {
                            redirectTarget = "/handsets";
                        }
                        redirectTo(redirectTarget + "?msg=File%20renamed");
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
            String html = beginPage();
            html.reserve(1536);  // BMP-Skalieren-Formular: klein
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
                webserver.send(200, "text/html", "<html><body style='font-family:Arial;'><h3>" + translate("Scaling successful") + "!</h3><p>" + translate("Saved as") + ": " + dst + "</p><a href = '/files'><button type='button'>" + translate("Back") + "</button></a></body></html>");
            }
            else {
                webserver.send(500, "text/html", "<html><body style='font-family:Arial;'><h3>" + translate("Failed to scale BMP") + "</h3><p>Check source file and format.</p><a href='/files'><button type='button'>" + translate("Back") + "</button></a></body></html>");
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


            redirectTo("/?msg=Settings%20saved");
            });


        // Helligkeitseinstellungen Formular
        webserver.on("/brightness", HTTP_POST, []() {
            webserver.setContentLength(CONTENT_LENGTH_UNKNOWN);
            webserver.send(200, "text/html", "");

            String chunk = beginPage();
            chunk.reserve(1024);
            chunk += generateFlashMessage();
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

            String chunk = beginPage();
            chunk.reserve(1024);
            chunk += generateFlashMessage();
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


            redirectTo("/brightness?msg=Settings%20saved");
            });


        // Alle Dateien auflisten
        webserver.on("/files", HTTP_GET, []() {
            webserver.setContentLength(CONTENT_LENGTH_UNKNOWN);
            webserver.send(200, "text/html", "");

            String chunk = beginPage();
            chunk.reserve(1024);
            chunk += generateFlashMessage();

            chunk += "<h2>" + translate("All Files on LittleFS") + "</h2><table border = '1'><tr><th style='text-align:left;'>" + translate("Filename") + "</th><th>" + translate("Size(bytes)") + "</th><th>" + translate("Info") + "</th><th>" + translate("Action") + "</th></tr>";

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
                chunk += " <td><a href = '/delete?file=" + name + "' title='" + translate("Delete") + "' onclick = 'return confirm(\"" + translate("Delete") + " " + name + "?\")'>&#128465;</a> ";
                // Scale-Option nur für .bmp-Dateien anzeigen
                if (name.endsWith(".bmp")) {
                    chunk += "<a href = '/scalebmp_form?file=" + name + "' title='" + translate("Scale") + "'>&#128207;</a> ";
                    chunk += "<a href='/rename_form?file=" + name + "' title='" + translate("Rename") + "'>&#9999;</a> ";
                }
                else {
                    chunk += "&nbsp; &nbsp;";
                }
                       
                chunk += "<a href='/download?file=" + name + "' title='" + translate("Download") + "'>&#11015;</a> ";
                chunk += "<a href='/file?name=" + name + "' title='" + translate("View") + "'>&#128065;</a> "; // "View"-Link für Logdateien

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
                    // Pruefen, ob RLE-komprimierte face_*.bmp - falls ja, vor dem Download
                    // zu einem echten Standard-BMP dekodieren (nutzbar ausserhalb der Uhr).
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
                        // Streaming fehlgeschlagen (z.B. Lesefehler) - Header ggf. schon
                        // gesendet, daher kein sauberer 500-Code mehr moeglich.
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

            String chunk = beginPage();
            chunk.reserve(1024);

            webserver.sendContent(chunk);
            chunk = "";

            chunk += "<h2>" + translate("System Status") + "</h2><ul>";

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

            chunk += "<li>Project: <a href='" GITHUB_REPO_URL "' target='_blank'>GitHub</a></li>";

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

        // Erzeugt ein Vorschaubild aus den GERADE AKTIVEN Einstellungen (kein Parameter
        // noetig) - wird auf jeder Seite oben links eingeblendet (generateNavigation())
        // und zeigt so nach jeder Aenderung, sobald die Seite neu laedt, den aktuellen Stand.
        webserver.on("/currentpreview", HTTP_GET, []() {
            String face = preferences.getString(PK_BACKGROUND, "/face_default.bmp");
            String handSet = preferences.getString(PK_HANDSET, "");
            if (handSet.isEmpty()) handSet = "default";
            uint8_t curHubSize = preferences.getUInt(PK_CENTER_SIZE, 6);
            uint32_t curHubColor = preferences.getLong(PK_CENTER_COLOR, 0xEC0016);
            bool curShowSecond = preferences.getBool(PK_SHOW_SECOND_HAND, true);

            uint8_t r = (curHubColor >> 16) & 0xFF;
            uint8_t g = (curHubColor >> 8) & 0xFF;
            uint8_t b = curHubColor & 0xFF;
            uint16_t hubColorRgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);

            uint8_t* bmpBytes = nullptr;
            size_t bmpSize = 0;
            if (generatePresetPreviewBmp(face, handSet, hubColorRgb565, curHubSize, curShowSecond, &bmpBytes, bmpSize)) {
                webserver.sendHeader("Cache-Control", "no-store");
                webserver.send_P(200, "image/bmp", (const char*)bmpBytes, bmpSize);
                delete[] bmpBytes;
            }
            else {
                webserver.send(500, "text/plain", "Failed to generate preview (out of memory?)");
            }
            });

        // Vorschaubild fuer die Preset-Verwaltung: Zifferblatt + Zeiger (feste Demo-
        // Zeit) + Mittelpunkt-Farbe/-Groesse, komponiert aus den im Preset gespeicherten
        // Einstellungen (siehe parsePresetForPreview() und generatePresetPreviewBmp()).
        webserver.on("/presetpreview", HTTP_GET, []() {
            if (!webserver.hasArg("index")) {
                webserver.send(400, "text/plain", "missing 'index' argument");
                return;
            }
            int index = webserver.arg("index").toInt();
            if (index < 0 || index >= MAX_PRESETS || presets[index].url.isEmpty()) {
                webserver.send(404, "text/plain", "preset not found");
                return;
            }

            String face, handSet;
            uint16_t hubColorRgb565;
            uint8_t hubSize;
            bool showSecond;
            parsePresetForPreview(presets[index].url, face, handSet, hubColorRgb565, hubSize, showSecond);

            uint8_t* bmpBytes = nullptr;
            size_t bmpSize = 0;
            if (generatePresetPreviewBmp(face, handSet, hubColorRgb565, hubSize, showSecond, &bmpBytes, bmpSize)) {
                webserver.send_P(200, "image/bmp", (const char*)bmpBytes, bmpSize);
                delete[] bmpBytes;
            }
            else {
                webserver.send(500, "text/plain", "Failed to generate preview (out of memory?)");
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

            String chunk = beginPage();
            chunk.reserve(1024);
            chunk += generateFlashMessage();
            chunk += "<h2>" + translate("Manage Clock Face Files") + " " + String(CLOCK_WIDTH) + " x " + String(CLOCK_HEIGHT) + "</h2>";
            chunk += "<div style='display:flex;flex-wrap:wrap;gap:24px 18px;justify-content:center;align-items:flex-start;'>";

            String activeBackground = preferences.getString(PK_BACKGROUND, "/face_default.bmp");

            // Add built-in default face
            chunk += "<div style='text-align:center;width:100px;'>";
            chunk += "<a href='http://" + ipAddress + "/setbackground?file=face_default.bmp'>";
            chunk += "<img src='http://" + ipAddress + "/preview_defaultface' style='width:80px;height:80px;border:1px solid #ccc'>";
            chunk += "</a><br>default" + String(activeBackground == "/face_default.bmp" ? " (" + translate("active") + ")" : "");
            chunk += "</div>";

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
                String displayName = shortName;
                if (displayName.startsWith("face_")) displayName = displayName.substring(5);
                if (displayName.endsWith(".bmp")) displayName = displayName.substring(0, displayName.length() - 4);
                String normalizedName = name.startsWith("/") ? name : "/" + name;
                bool isActive = (normalizedName == activeBackground);
                chunk += "<div style='text-align:center;width:100px;'>";
                chunk += "<a href=http://" + ipAddress + "/setbackground?file=" + shortName + ">";
                chunk += "<img src='/facepreview?file=" + name + "' style='width:80px;height:80px;border:1px solid #ccc'>";
                chunk += "</a><br>" + displayName + String(isActive ? " (" + translate("active") + ")" : "");
                chunk += "<br><a href='/rename_form?file=" + name + "'>" + translate("Rename") + "</a> ";
                chunk += "<a href='/delete?file=" + name + "' onclick='return confirm(\"" + translate("Delete") + " " + displayName + "?\")'>" + translate("Delete") + "</a>";
                chunk += "</div>";

                // Alle paar Eintraege zwischendurch senden, damit der Puffer auch
                // bei vielen Zifferblaettern nicht unbegrenzt waechst.
                rowCount++;
                if (rowCount % 5 == 0) {
                    webserver.sendContent(chunk);
                    chunk = "";
                }
            }

            if (!anyFile) chunk += "<p>" + translate("No BMP files found in /") + "</p>";
            chunk += "</div><hr>";

            // Automatischer Download neuer Zifferblaetter direkt im Browser: der
            // Browser laedt (per CORS, von GitHub offiziell fuer api.github.com
            // und raw.githubusercontent.com unterstuetzt) die Dateien per HTTPS
            // herunter und laedt sie dann per normalem lokalem HTTP ueber /upload
            // hoch - die Uhr selbst muss dabei nie eine HTTPS-Verbindung aufbauen.
            chunk += "<h3>" + translate("Download Additional Clock Faces from GitHub") + "</h3>";
            chunk += "<button type='button' id='ghFaceBtn' onclick='loadFacesFromGithub()'>" + translate("Download Additional Clock Faces from GitHub") + "</button>";
            chunk += "<div id='ghFaceStatus'></div>";
            chunk += "<script>";
            chunk += "var existingFaces = [";
            for (size_t i = 0; i < faceNames.size(); i++) {
                if (i > 0) chunk += ",";
                chunk += "\"" + faceNames[i] + "\"";
            }
            chunk += "];";
            chunk += "async function loadFacesFromGithub() {";
            chunk += "  var btn = document.getElementById('ghFaceBtn');";
            chunk += "  var status = document.getElementById('ghFaceStatus');";
            chunk += "  btn.disabled = true;";
            chunk += "  status.innerHTML = '" + translate("Checking GitHub for new files") + "...';";
            chunk += "  try {";
            chunk += "    var resp = await fetch('" GITHUB_API_CONTENTS_BASE + String(CLOCK_WIDTH) + "');";
            chunk += "    var files = await resp.json();";
            chunk += "    var toGet = files.filter(function(f) { return f.name.indexOf('face_') === 0 && f.name.endsWith('.bmp') && existingFaces.indexOf(f.name) === -1; });";
            chunk += "    if (toGet.length === 0) { status.innerHTML = '" + translate("All files already up to date") + ".'; btn.disabled = false; return; }";
            chunk += "    for (var i = 0; i < toGet.length; i++) {";
            chunk += "      status.innerHTML = '" + translate("Downloading") + " ' + toGet[i].name + ' (' + (i + 1) + '/' + toGet.length + ')...';";
            chunk += "      var blob = await (await fetch(toGet[i].download_url)).blob();";
            chunk += "      var fd = new FormData();";
            chunk += "      fd.append('upload', blob, toGet[i].name);";
            chunk += "      status.innerHTML = '" + translate("Converting") + " ' + toGet[i].name + ' (' + (i + 1) + '/' + toGet.length + ')...';";
            chunk += "      await fetch('/upload', { method: 'POST', body: fd });";
            chunk += "    }";
            chunk += "    status.innerHTML = '" + translate("Done - reloading") + "...';";
            chunk += "    location.href = location.pathname;";
            chunk += "  } catch (e) {";
            chunk += "    status.innerHTML = '" + translate("Failed to reach GitHub - check your internet connection") + ".';";
            chunk += "    btn.disabled = false;";
            chunk += "  }";
            chunk += "}";
            chunk += "</script><hr>";

            // Hinweis und Download-Link für die ZIP-Datei
            if (TFT_WIDTH == 240) {
                chunk += "<h3>" + translate("Download Additional Clock Faces") + "</h3>";
                chunk += "<p>" + translate("You can download a ZIP file containing additional clock faces and hand sets from the following link: (use 'view raw')") + "</p>";
                chunk += "<a href='" GITHUB_ZIP_BASE "faces_handsets_240.zip' target='_blank'>Download faces_handsets_240.zip</a>";
                chunk += "<br><small>" + translate("After downloading, upload the extracted BMP files using the form below") + ".</small><hr>";
            }

            if (TFT_WIDTH == 160) {
                chunk += "<h3>" + translate("Download Additional Clock Faces") + "</h3>";
                chunk += "<p>" + translate("You can download a ZIP file containing additional clock faces and hand sets from the following link: (use 'view raw')") + "</p>";
                chunk += "<a href='" GITHUB_ZIP_BASE "faces_handsets_160.zip' target='_blank'>Download faces_handsets_160.zip</a>";
                chunk += "<br><small>" + translate("After downloading, upload the extracted BMP files using the form below") + ".</small><hr>";
            }

            webserver.sendContent(chunk);
            chunk = "";

            if (used + (CLOCK_WIDTH * CLOCK_HEIGHT * 2) + 54 > total) {
                chunk += "<div style='color:red;font-weight:bold;'>" + translate("Warning: Not enough free space to upload new clock faces! Free up some space first") + ".</div><br><br>";
            }
            else {

                chunk += "<h3>" + translate("Upload New Clock Face") + "</h3>";
                chunk += "<small>" + translate("Requirements") + ": " + String(CLOCK_WIDTH) + " x " + String(CLOCK_HEIGHT) + " " + translate("pixels") + ", 16-bit BMP(RGB565), " + translate("name must start with") + "  <code>face_</code></small><br><br>";

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

            String chunk = beginPage();
            chunk.reserve(1536);
            chunk += generateFlashMessage(); // Erfolgsmeldung, falls vorhanden

            // Waehrend WPS aktiv ist, trennt sich der ESP von seinem bisherigen WLAN,
            // um als WPS-Client nach dem Router zu suchen (technisch notwendig fuer
            // WPS) - er ist unter seiner bisherigen IP fuer bis zu 2 Minuten schlicht
            // nicht erreichbar. Ein einmaliger location.reload() nach fester Wartezeit
            // wuerde in diesem Fenster garantiert eine Browser-Fehlerseite zeigen.
            // Stattdessen im Hintergrund per fetch() mit kurzem Timeout periodisch
            // pruefen, ob der ESP schon wieder antwortet (verbunden, egal ob WPS
            // erfolgreich war oder zur vorherigen Verbindung zurueckgefallen ist -
            // siehe loop() in uhr3.ino) und erst dann neu laden. Zusaetzlich Fallback
            // auf hostname.local (mDNS), falls der ESP nach einem WPS-Erfolg + Reboot
            // eine ANDERE IP per DHCP bekommen hat - die alte IP waere dann dauerhaft
            // tot, aber der Hostname bleibt gueltig (siehe pingHostname/hostname,
            // bereits an anderer Stelle im Webinterface genutzt).
            if (webserver.arg("msg") == "WPS active - press the WPS button on your router now (within 2 minutes)") {
                String hostnameTargetJs = pingHostname ? ("'http://" + String(hostname) + ".local/'") : "null";
                chunk += "<script>";
                chunk += "(function() {";
                chunk += "  var attempts = 0;";
                chunk += "  var maxAttempts = 25;"; // ca. 25 * (5s Timeout + 3s Pause) = ~3.3 Minuten, etwas mehr als der 2-Minuten-WPS-Timeout plus Reboot/Reconnect-Zeit
                chunk += "  var hostnameTarget = " + hostnameTargetJs + ";";
                chunk += "  function tryFetch(url, opts) {";
                chunk += "    return new Promise(function(resolve, reject) {";
                chunk += "      var controller = new AbortController();";
                chunk += "      var timeoutId = setTimeout(function() { controller.abort(); }, 5000);";
                chunk += "      var fetchOpts = Object.assign({ cache: 'no-store', signal: controller.signal }, opts || {});";
                chunk += "      fetch(url, fetchOpts)";
                chunk += "        .then(function() { clearTimeout(timeoutId); resolve(); })";
                chunk += "        .catch(function(e) { clearTimeout(timeoutId); reject(e); });";
                chunk += "    });";
                chunk += "  }";
                chunk += "  function poll() {";
                chunk += "    attempts++;";
                // Zuerst dieselbe IP versuchen (schnellster Weg, falls sie unveraendert
                // geblieben ist) - Same-Origin, daher normaler fetch() ohne CORS-Huerden.
                chunk += "    tryFetch(location.pathname, {}).then(function() {";
                chunk += "      location.href = location.pathname;";
                chunk += "    }).catch(function() {";
                // IP nicht erreichbar - falls ein Hostname bekannt ist, zusaetzlich per
                // mDNS probieren. mode:'no-cors' liefert eine "opake" Antwort ohne
                // lesbaren Inhalt, aber die Promise loest bereits auf, sobald die
                // Verbindung ueberhaupt zustande kam - genau das reicht hier als
                // Erreichbarkeits-Check, ganz ohne dass der ESP CORS-Header senden muss.
                chunk += "      if (hostnameTarget) {";
                chunk += "        tryFetch(hostnameTarget, { mode: 'no-cors' }).then(function() {";
                chunk += "          location.href = hostnameTarget;";
                chunk += "        }).catch(function() {";
                chunk += "          if (attempts < maxAttempts) setTimeout(poll, 3000);";
                chunk += "        });";
                chunk += "      } else if (attempts < maxAttempts) {";
                chunk += "        setTimeout(poll, 3000);";
                chunk += "      }";
                chunk += "    });";
                chunk += "  }";
                chunk += "  setTimeout(poll, 3000);";
                chunk += "})();";
                chunk += "</script>";
            }

            chunk += "<h2>" + translate("Clock Setup") + "</h2>";

            chunk += generateLanguageSelector();

            // --- Hostname: Textfeld (170px) + Save-Button in einer Zeile, zentriert ---
            chunk += "<form method='POST' action='/sethostname'>";
            chunk += "<div style='display:flex;justify-content:center;align-items:center;gap:12px;flex-wrap:wrap;'>";
            chunk += "<label>" + translate("Hostname") + ":</label>";
            chunk += "<input name='hostname' maxlength='30' value='" + String(hostname) + "' style='width:170px;'>";
            chunk += "<button type='submit' style='width:140px;'>" + translate("Save") + "</button>";
            chunk += "</div>";
            chunk += "</form><br><br>";

            webserver.sendContent(chunk);
            chunk = "";

            // --- Add Network via WPS + Rescan Networks: je 170px, in einer Zeile, zentriert ---
            chunk += "<div style='display:flex;justify-content:center;align-items:center;gap:12px;flex-wrap:wrap;'>";
            chunk += "<form method='POST' action='/api/startWPS' style='margin:0;'>";
            chunk += "<button type='submit' style='width:170px;'>" + translate("Add Network via WPS") + "</button>";
            chunk += "</form>";
            chunk += "<button id='rescanBtn' type='button' style='width:170px;'>" + translate("Rescan Networks") + "</button>";
            chunk += "</div><br>";

            chunk += "<form action = '/save' method = 'POST'>";

            for (int i = 0; i < MAX_WLAN; i++) {
                // Dynamisch berechnete Schlüssel
                String ssidKey = pkSsid(i);
                String passKey = pkPass(i);
                String ssidSelectId = "ssid_select" + String(i + 1);
                wifiSsid[i] = preferences.getString(ssidKey.c_str(), "");

                String upperSsidKey = ssidKey; // Kopie erstellen
                upperSsidKey.toUpperCase();    // Kopie in Großbuchstaben umwandeln
                chunk += "<h3>" + upperSsidKey + "</h3>";

                chunk += "<div style='display:flex;gap:6px;align-items:center;flex-wrap:wrap;justify-content:center;'>";
                chunk += "<select id='" + ssidSelectId + "' onchange=\"document.getElementById('" + ssidKey + "').value=this.value\" style='max-width:180px;'>";
                chunk += "</select>";
                chunk += "<input name='" + ssidKey + "' id='" + ssidKey + "' placeholder='" + ssidKey + "' value='" + wifiSsid[i] + "' style='width:110px;'>";
                chunk += "<input name='" + passKey + "' id='" + passKey + "' placeholder='Password' type='password' value='' style='width:110px;'>";
                if (wifiSsid[i] != "") {
                    chunk += "<a href='/deletewifi?index=" + String(i) + "' onclick='return confirm(\"" + translate("Delete") + " " + wifiSsid[i] + "?\")'>" + translate("Delete") + "</a>";
                }
                chunk += "</div>";

                chunk += "<small>" + translate("You can also enter an SSID manually") + ".";
                if (WiFi.getMode() == WIFI_STA && wifiSsid[i] != "") {
                    chunk += " " + translate("Password is hidden.Leave empty to keep current") + ".";
                }
                chunk += "</small>";
                chunk += "<hr>";

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

            chunk += "<div style='display:flex;flex-wrap:wrap;gap:15px;justify-content:center;max-width:600px;margin:auto;'>";

            chunk += "<div><input type='checkbox' name='stationMode' value='1' ";
            chunk += preferences.getBool(PK_STATION_MODE, true) ? "checked" : "";
            chunk += "> " + translate("Train Station Mode") + "</div>";

            chunk += "<div><input type='checkbox' name='showSecondHand' value='1' ";
            chunk += preferences.getBool(PK_SHOW_SECOND_HAND, true) ? "checked" : "";
            chunk += "> " + translate("Show Seconds") + "</div>";

            chunk += "<div><input type='checkbox' name='smoothMinute' value='1' ";
            chunk += preferences.getBool(PK_SMOOTH_MINUTE, true) ? "checked" : "";
            chunk += "> " + translate("Smooth Minute Hand") + "</div>";

            String pingServer = preferences.getString(PK_PING_SERVER, DEFAULT_PING_SERVER);
            chunk += "<div>" + translate("Ping Server") + "<input type='text' name='pingServer' value='" + pingServer + "'>";
            chunk += "</div>";

            // Neue Checkbox für Touch-Freigabe
            //chunk += "<div><input type='checkbox' name='useTouch' value='1' ";
            //chunk += preferences.getBool(PK_USE_TOUCH, false) ? "checked" : "";
            //chunk += "> " + translate("Enable Touch") + "</div>";

            chunk += "<div><input type='checkbox' name='wifiActive' value='1' ";
            chunk += wifiActive ? "checked" : "";
            chunk += "> " + translate("Reconnect WiFi") + "</div>";


            chunk += "<div><input type='checkbox' name='loggingEnabled' value='1' ";
            chunk += loggingEnabled ? "checked" : "";
            chunk += "> " + translate("Enable Logging") + "</div>";


            chunk += "<div>Rotation: <select name='rotation'>";
            const char* rotationLabels[] = { "0&deg;", "90&deg;", "180&deg;", "270&deg;" };
            for (int i = 0; i <= 3; i++) {
                chunk += "<option value='" + String(i) + "'";
                if (i == tftRotation) chunk += " selected";
                chunk += ">" + String(rotationLabels[i]) + "</option>";
            }
            chunk += "</select></div>";


            // chunk += "<div><input type='checkbox' name='loggingEnabled' value='1' ";
            // chunk += loggingEnabled ? "checked" : "";
            // chunk += "> Logging aktivieren</div>";

            chunk += "</div>";

            chunk += "<div style='text-align:center;margin-top:15px;'><button type='submit'>" + translate("Save") + "</button></div>";
            chunk += "</form>";

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
            chunk += "  var btn = this;";
            chunk += "  btn.disabled = true;";
            chunk += "  var hint = document.createElement('div');";
            chunk += "  hint.id = 'rescanHint';";
            chunk += "  hint.style.cssText = 'background:#d4edda;color:#155724;border:1px solid #c3e6cb;border-radius:6px;padding:8px 12px;margin:10px auto;max-width:400px;';";
            chunk += "  hint.innerHTML = '" + translate("Scanning for WiFi networks - the page will reload automatically in 10 seconds") + "';";
            chunk += "  btn.parentNode.insertBefore(hint, btn.nextSibling);";
            chunk += "  fetch('/api/rescanwifi', {method: 'POST'})";
            chunk += "    .catch(function() {})";
            chunk += "    .finally(function() {";
            chunk += "      setTimeout(function() { location.href = location.pathname; }, 10000);";
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
                chunk += "  var " + input + " = document.getElementById('" + ssidKey + "');";
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
        // Loescht ein einzelnes gespeichertes WLAN-Netzwerk und rueckt die
        // nachfolgenden Netzwerke auf, damit keine Luecke entsteht (gleiche
        // Kompaktierung wie beim regulaeren Speichern in /save).
        webserver.on("/deletewifi", HTTP_GET, []() {
            if (webserver.hasArg("index")) {
                int idx = webserver.arg("index").toInt();
                if (idx >= 0 && idx < MAX_WLAN) {
                    preferences.putString(pkSsid(idx).c_str(), "");
                    preferences.putString(pkPass(idx).c_str(), "");

                    String tempSsid[MAX_WLAN];
                    String tempPass[MAX_WLAN];
                    int j = 0;
                    for (int i = 0; i < MAX_WLAN; i++) {
                        String ssid = trim(preferences.getString(pkSsid(i).c_str(), ""));
                        if (ssid.length() > 0) {
                            tempSsid[j] = preferences.getString(pkSsid(i).c_str(), "");
                            tempPass[j] = preferences.getString(pkPass(i).c_str(), "");
                            j++;
                        }
                    }
                    for (int i = 0; i < MAX_WLAN; i++) {
                        if (preferences.getString(pkSsid(i).c_str(), "") != tempSsid[i]) {
                            preferences.putString(pkSsid(i).c_str(), tempSsid[i]);
                        }
                        if (preferences.getString(pkPass(i).c_str(), "") != tempPass[i]) {
                            preferences.putString(pkPass(i).c_str(), tempPass[i]);
                        }
                        wifiSsid[i] = tempSsid[i];
                        wifiPass[i] = tempPass[i];
                    }
                }
                redirectTo("/?msg=Network%20deleted");
            }
            else {
                webserver.send(400, "text/plain", "Missing parameter");
            }
            });

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
                    redirectTo("/?msg=Settings%20saved");
                }
                else {
                    webserver.send(200, "text/html", "<!DOCTYPE html><html><head>"
                        "<title>Settings saved</title></head><body style='font-family:Arial;text-align:center;'>"
                        "<h2>" + translate("Settings saved") + "</h2><p>" + translate("Please connect to your home network and go to the ESP website at") + " http:// IPADDRESS</p></body></html>");

                    espReboot();
                }
           // }
            });

        // Upload-Formular anzeigen
        webserver.on("/upload", HTTP_GET, []() {
            webserver.send(200, "text/html", "<form method='POST' action='/upload' enctype='multipart/form-data' onsubmit='showProgress()'><input type='file' name='upload' accept='.bmp' multiple required><br><br><button type='submit'>Upload BMP</button><div id='progress' style='display:none;'>Uploading... please wait</div><script>function showProgress(){document.getElementById('progress').style.display='block';}</script></form><br><a href='/listfilesFaces'><button type='button'>" + translate("Back") + "</button></a>");
            });

        // Datei-Upload verarbeiten
        webserver.on("/upload", HTTP_POST, []() {
            if (uploadSuccess) {
                redirectTo("/listfilesFaces?msg=Clock%20face%20uploaded");
            }
            else {
                String errorHtml = "<!DOCTYPE html><html><head><title>" + translate("Upload Failed") + "</title><meta name='viewport' content='width=device-width, initial-scale=1'></head><body style='font-family:Arial;text-align:center;'>";
                errorHtml += "<h2>" + translate("Upload failed") + "</h2>";
                errorHtml += "<p>" + translate("Only .bmp files starting with") + " <code>face_</code> " + translate("or") + " <code>hand_</code> " + translate("are accepted") + ".</p>";
                errorHtml += "<p>" + translate("Please also check the available space") + ".</p>";
                errorHtml += "<a href='/upload'><button type='button'>" + translate("Try again") + "</button></a></body></html>";
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
                    redirectTo("/listfilesFaces?msg=Clock%20face%20selected");
                    return;
                }

                if (LittleFS.exists(file)) {
                    selectedBackground = file;
                    preferences.putString(PK_BACKGROUND, file);
                    DEBUG_PRINTLN("set bg to: " + file);
                    freeClockFaceBuffer();
                    loadClockFace();
                    loadHandSprites();
                    redirectTo("/listfilesFaces?msg=Clock%20face%20selected");
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

                    // Falls ein Zifferblatt oder Teil eines Zeigersatzes geloescht
                    // wurde, alle Presets entfernen, die darauf verweisen.
                    String name = path.substring(1); // fuehrenden Slash entfernen
                    if (name.startsWith("face_") && name.endsWith(".bmp")) {
                        removeOrphanedPresets(path, "");
                    }
                    else if (name.startsWith("hand_set") && name.endsWith(".bmp")) {
                        int start = 8; // Laenge von "hand_set"
                        int end = name.indexOf('_', start);
                        if (end > start) {
                            String setId = name.substring(start, end);
                            removeOrphanedPresets("", setId);

                            // Falls der betroffene Zeigersatz gerade aktiv war,
                            // sofort auf den eingebauten Standard zurueckschalten.
                            if (preferences.getString(PK_HANDSET, "") == setId) {
                                preferences.putString(PK_HANDSET, "default");
                                freeClockFaceBuffer();
                                loadClockFace();
                                loadHandSprites();
                                updateClock();
                            }
                        }
                    }

                    String redirectTarget = "/files";
                    if (name.startsWith("face_") && name.endsWith(".bmp")) {
                        redirectTarget = "/listfilesFaces";
                    }
                    else if (name.startsWith("hand_set") && name.endsWith(".bmp")) {
                        redirectTarget = "/handsets";
                    }
                    redirectTo(redirectTarget + "?msg=File%20deleted");
                }
                else {
                    webserver.send(404, "text/plain", "File not found");
                }
            }
            });

        // Einzelnes Preset loeschen (Slot wird dadurch wieder frei fuer
        // createPresetFromPreferences())
        webserver.on("/deletepreset", HTTP_GET, []() {
            if (webserver.hasArg("index")) {
                int idx = webserver.arg("index").toInt();
                if (idx >= 0 && idx < MAX_PRESETS) {
                    presets[idx].name = "";
                    presets[idx].url = "";
                    savePresets();
                }
            }
            redirectTo("/presets?msg=Preset%20deleted");
            });

        // Umbenennen-Formular fuer ein einzelnes Preset
        webserver.on("/renamepreset_form", HTTP_GET, []() {
            if (!webserver.hasArg("index")) {
                webserver.send(400, "text/plain", "Missing index parameter");
                return;
            }
            int idx = webserver.arg("index").toInt();
            if (idx < 0 || idx >= MAX_PRESETS || presets[idx].name.isEmpty()) {
                webserver.send(404, "text/plain", "Preset not found");
                return;
            }
            String html = beginPage();
            html.reserve(1024);
            html += "<h2>" + translate("Rename Preset") + "</h2>";
            html += "<form action='/renamepreset' method='POST'>";
            html += "<input type='hidden' name='index' value='" + String(idx) + "'>";
            html += "<label>" + translate("New Name") + ":</label><br>";
            html += "<input name='new' value='" + presets[idx].name + "' required><br><br>";
            html += "<button type='submit'>" + translate("Rename") + "</button></form>";
            html += "<br><a href='/presets'><button type='button'>" + translate("Cancel") + "</button></a></body></html>";
            webserver.send(200, "text/html", html);
            });

        // Preset umbenennen Aktion
        webserver.on("/renamepreset", HTTP_POST, []() {
            if (webserver.hasArg("index") && webserver.hasArg("new")) {
                int idx = webserver.arg("index").toInt();
                String newName = webserver.arg("new");
                newName.replace(" ", "_"); // Konsistent zur Anzeige/den API-Links (siehe /presets)

                if (idx < 0 || idx >= MAX_PRESETS || presets[idx].name.isEmpty()) {
                    webserver.send(404, "text/plain", "Preset not found");
                    return;
                }
                if (newName.isEmpty()) {
                    webserver.send(400, "text/plain", "Name must not be empty");
                    return;
                }

                presets[idx].name = newName;
                savePresets();

                redirectTo("/presets?msg=Preset%20renamed");
            }
            else {
                webserver.send(400, "text/plain", "Missing parameters");
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
                        // Pruefen, ob RLE-komprimiert - falls ja, vor der Auslieferung zu
                        // einem echten Standard-BMP dekodieren (sonst fuer externe Tools nicht lesbar).
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
                            // Streaming fehlgeschlagen (z.B. Lesefehler) - NICHT stillschweigend
                            // die rohen komprimierten Bytes ausliefern (kein gueltiges BMP mehr),
                            // stattdessen klarer Fehler (500-Code ggf. nicht mehr moeglich, falls Header schon gesendet).
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

            // Chunked-Response (Variante B): Seite bettet Vorschaubilder als Base64 ein und
            // kann dadurch sehr gross werden - wird daher Stueck fuer Stueck gesendet
            // (webserver.sendContent()), Speicherbedarf haengt nur von der groessten Zeile ab.
            webserver.setContentLength(CONTENT_LENGTH_UNKNOWN);
            webserver.send(200, "text/html", "");

            String chunk = beginPage();
            chunk += generateFlashMessage();
            chunk += "<h2>" + translate("Manage Clock Hand Sets") + " " + String(HAND_WIDTH) + " x " + String(HAND_HEIGHT) + "</h2>";
            chunk += "<div style='display:flex;flex-wrap:wrap;gap:24px 18px;justify-content:center;align-items:flex-start;'>";
            webserver.sendContent(chunk);

            String activeSet = preferences.getString(PK_HANDSET, "");
            // Numerisch sortieren (std::map<int,String>), damit z.B. "10" nach "9" statt
            // zwischen "1" und "2" landet. Nicht-numerische Namen (unueblich) landen
            // zusaetzlich unsortiert in einer separaten Liste, damit sie nicht verloren gehen.
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
            bool defaultSetActive = (activeSet == "default" || activeSet.isEmpty());
            chunk = "<div style='text-align:center;border:1px solid #ccc;border-radius:6px;padding:8px;'>";
            chunk += "<a href='/sethandset?set=default'>";
            chunk += "<img src='data:image/bmp;charset=utf-8;base64, " + handHourBase64 + "'> ";
            chunk += "<img src='data:image/bmp;charset=utf-8;base64, " + handMinuteBase64 + "'> ";
            chunk += "<img src='data:image/bmp;charset=utf-8;base64, " + handSecondBase64 + "'>";
            chunk += "</a><br>0" + String(defaultSetActive ? " (" + translate("active") + ")" : "");
            chunk += "</div>";
            webserver.sendContent(chunk);

            // Jeden gefundenen Zeigersatz SOFORT senden statt zu sammeln - so liegt
            // nie mehr als ein Zeigersatz gleichzeitig im Speicher.
            auto renderSetRow = [&](const String& setId) {
                chunk = "<div style='text-align:center;border:1px solid #ccc;border-radius:6px;padding:8px;'>";
                String hourPath = "/hand_set" + setId + "_hour.bmp";
                String minutePath = "/hand_set" + setId + "_minute.bmp";
                String secondPath = "/hand_set" + setId + "_second.bmp";
                chunk += "<a href='/sethandset?set=" + setId + "'>";
                chunk += LittleFS.exists(hourPath) ? "<img src='/file?name=" + hourPath + "'> " : "<img src='data:image/bmp;charset=utf-8;base64, " + handHourBase64 + "'> ";
                chunk += LittleFS.exists(minutePath) ? "<img src='/file?name=" + minutePath + "'> " : "<img src='data:image/bmp;charset=utf-8;base64, " + handMinuteBase64 + "'> ";
                chunk += LittleFS.exists(secondPath) ? "<img src='/file?name=" + secondPath + "'> " : "<img src='data:image/bmp;charset=utf-8;base64," + handSecondBase64 + "'>";
                chunk += "</a><br>" + setId + (setId == activeSet ? " (" + translate("active") + ")" : "");
                chunk += "<br><a href='/deletehandset?set=" + setId + "' onclick='return confirm(\"" + translate("Delete") + " " + setId + "?\")'>" + translate("Delete") + "</a>";
                chunk += "</div>";
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

            chunk = "</div><hr>";

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
                // Vorhandene Zeigersatz-Dateinamen fuer den Vergleich mit GitHub einsammeln
                std::vector<String> existingHandFiles;
                File handRootScan = LittleFS.open("/");
                File handFileScan = handRootScan.openNextFile();
                while (handFileScan) {
                    String hName = handFileScan.name();
                    if (!handFileScan.isDirectory() && hName.startsWith("hand_set") && hName.endsWith(".bmp")) {
                        existingHandFiles.push_back(hName);
                    }
                    handFileScan = handRootScan.openNextFile();
                }

                // Automatischer Download neuer Zeigersaetze direkt im Browser (siehe
                // ausfuehrlichen Kommentar bei /listfilesFaces - gleiches Prinzip:
                // Browser laedt per CORS von GitHub, laedt lokal per /uploadhandset hoch).
                chunk += "<h3>" + translate("Download Additional Hand Sets from GitHub") + "</h3>";
                chunk += "<button type='button' id='ghHandBtn' onclick='loadHandsFromGithub()'>" + translate("Download Additional Hand Sets from GitHub") + "</button>";
                chunk += "<div id='ghHandStatus'></div>";
                chunk += "<script>";
                chunk += "var existingHands = [";
                for (size_t i = 0; i < existingHandFiles.size(); i++) {
                    if (i > 0) chunk += ",";
                    chunk += "\"" + existingHandFiles[i] + "\"";
                }
                chunk += "];";
                chunk += "async function loadHandsFromGithub() {";
                chunk += "  var btn = document.getElementById('ghHandBtn');";
                chunk += "  var status = document.getElementById('ghHandStatus');";
                chunk += "  btn.disabled = true;";
                chunk += "  status.innerHTML = '" + translate("Checking GitHub for new files") + "...';";
                chunk += "  try {";
                chunk += "    var resp = await fetch('" GITHUB_API_CONTENTS_BASE + String(CLOCK_WIDTH) + "');";
                chunk += "    var files = await resp.json();";
                chunk += "    var toGet = files.filter(function(f) { return f.name.indexOf('hand_set') === 0 && f.name.endsWith('.bmp') && existingHands.indexOf(f.name) === -1; });";
                chunk += "    if (toGet.length === 0) { status.innerHTML = '" + translate("All files already up to date") + ".'; btn.disabled = false; return; }";
                chunk += "    for (var i = 0; i < toGet.length; i++) {";
                chunk += "      status.innerHTML = '" + translate("Downloading") + " ' + toGet[i].name + ' (' + (i + 1) + '/' + toGet.length + ')...';";
                chunk += "      var blob = await (await fetch(toGet[i].download_url)).blob();";
                chunk += "      var fd = new FormData();";
                chunk += "      fd.append('upload', blob, toGet[i].name);";
                chunk += "      status.innerHTML = '" + translate("Converting") + " ' + toGet[i].name + ' (' + (i + 1) + '/' + toGet.length + ')...';";
                chunk += "      await fetch('/uploadhandset', { method: 'POST', body: fd });";
                chunk += "    }";
                chunk += "    status.innerHTML = '" + translate("Done - reloading") + "...';";
                chunk += "    location.href = location.pathname;";
                chunk += "  } catch (e) {";
                chunk += "    status.innerHTML = '" + translate("Failed to reach GitHub - check your internet connection") + ".';";
                chunk += "    btn.disabled = false;";
                chunk += "  }";
                chunk += "}";
                chunk += "</script><hr>";

                chunk += "<h3>" + translate("Upload New Hand Set") + "</h3>";
                chunk += "<small>" + translate("Requirements") + ": " + String(HAND_WIDTH) + " x " + String(HAND_HEIGHT) + " " + translate("pixels") + ", 16-bit BMP(RGB565), <br>" + translate("name must start with") + " <code>hand_set + no + _hour, _minute or _second.bmp e.g.hand_set1_second.bmp</code><br>" + translate("Pivot point") + ": " + String(int(HAND_WIDTH / 2)) + " / " + String(int(HAND_HEIGHT * 0.77)) + "<br><br>";
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
            redirectTo("/handsets?msg=Settings%20saved");
            });

        //  Handsets Datei-Upload verarbeiten
        webserver.on("/uploadhandset", HTTP_POST, []() {
            if (uploadSuccess) {
                // Sicherheitsprüfung auf Dateinamenmuster
                if (!uploadFilePath.endsWith(".bmp") || !uploadFilePath.startsWith("/hand_set")) {
                    String errorHtml = "<!DOCTYPE html><html><head><title>" + translate("Upload Failed") + "</title><meta name='viewport' content='width=device-width, initial-scale=1'></head><body style='font-family:Arial;text-align:center;'>";
                    errorHtml += "<h2>" + translate("Upload failed") + "</h2>";
                    errorHtml += "<p>" + translate("Only .bmp files starting with") + " <code>hand_</code> " + translate("are accepted for handset upload") + ".</p>";
                    errorHtml += "<a href='/handsets'><button type='button'>" + translate("Try again") + "</button></a></body></html>";
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
                redirectTo("/handsets?msg=Hand%20set%20uploaded");
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
                redirectTo("/handsets?msg=Hand%20set%20selected");
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
                removeOrphanedPresets("", setId);

                // Falls der geloeschte Zeigersatz gerade aktiv war, sofort auf
                // den eingebauten Standard zurueckschalten - sonst wuerde die
                // Uhr versuchen, einen nicht mehr existierenden Zeigersatz zu laden.
                if (preferences.getString(PK_HANDSET, "") == setId) {
                    preferences.putString(PK_HANDSET, "default");
                    freeClockFaceBuffer();
                    loadClockFace();
                    loadHandSprites();
                    updateClock();
                }

                redirectTo("/handsets?msg=Hand%20set%20deleted");
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

        // Werkseinstellungen: Uebersichtsseite mit mehreren, einzeln
        // bestaetigten Reset-Optionen statt einer einzigen Alles-oder-nichts-Aktion.
        webserver.on("/factoryReset", HTTP_GET, []() {
            String html = beginPage();
            html += generateFlashMessage();
            html += "<h2>" + translate("Factory&nbsp;Reset") + "</h2>";

            html += "<h3>" + translate("Reset Everything") + "</h3>";
            html += "<p>" + translate("Resets WiFi, all settings and deletes all files - the clock restarts afterwards") + ".</p>";
            html += "<form method='POST' action='/factoryReset/all' onsubmit=\"return confirm('" + translate("Are you sure you want to reset to factory settings?") + "');\">";
            html += "<button type='submit'>" + translate("Reset Everything") + "</button></form><hr>";

            html += "<h3>" + translate("Reset Saved Networks") + "</h3>";
            html += "<p>" + translate("Deletes all saved WiFi networks - other settings remain unchanged") + ".</p>";
            html += "<form method='POST' action='/api/resetWiFi' onsubmit='return confirm(\"" + translate("Are you sure you want to reset all saved WiFi networks?") + "\");'>";
            html += "<button type='submit'>" + translate("Reset Saved Networks") + "</button></form><hr>";

            html += "<h3>" + translate("Delete Clock Faces (except default)") + "</h3>";
            html += "<p>" + translate("Deletes all uploaded clock faces - the built-in default remains") + ".</p>";
            html += "<form method='POST' action='/factoryReset/faces' onsubmit=\"return confirm('" + translate("Are you sure you want to delete all clock faces except the default one?") + "');\">";
            html += "<button type='submit'>" + translate("Delete Clock Faces (except default)") + "</button></form><hr>";

            html += "<h3>" + translate("Delete Hand Sets (except default)") + "</h3>";
            html += "<p>" + translate("Deletes all uploaded hand sets - the built-in default remains") + ".</p>";
            html += "<form method='POST' action='/factoryReset/hands' onsubmit=\"return confirm('" + translate("Are you sure you want to delete all hand sets except the default one?") + "');\">";
            html += "<button type='submit'>" + translate("Delete Hand Sets (except default)") + "</button></form><hr>";

            html += "<h3>" + translate("Delete Presets") + "</h3>";
            html += "<p>" + translate("Deletes all saved presets") + ".</p>";
            html += "<form method='POST' action='/factoryReset/presets' onsubmit=\"return confirm('" + translate("Are you sure you want to delete all presets?") + "');\">";
            html += "<button type='submit'>" + translate("Delete Presets") + "</button></form><hr>";

            html += "</body></html>";
            webserver.send(200, "text/html", html);
            });

        webserver.on("/factoryReset/all", HTTP_POST, []() {
            factoryReset();
            });

        webserver.on("/factoryReset/faces", HTTP_POST, []() {
            resetFacesToDefault();
            redirectTo("/factoryReset?msg=Clock%20faces%20deleted");
            });

        webserver.on("/factoryReset/hands", HTTP_POST, []() {
            resetHandsToDefault();
            redirectTo("/factoryReset?msg=Hand%20sets%20deleted");
            });

        webserver.on("/factoryReset/presets", HTTP_POST, []() {
            resetAllPresets();
            redirectTo("/factoryReset?msg=Presets%20deleted");
            });

        // Sofortige Zeitsynchronisation
        webserver.on("/syncnow", HTTP_POST, []() {
            setupNTP();
           // struct tm timeinfo;
            getLocalTime(&timeinfo, 100);

            char timeStr[32];
            strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);

            String html = "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='3; url=/'>"
                "<title>Time Synced</title></head><body style='font-family:Arial;text-align:center;'>"
                "<h2>" + translate("Time synced") + "</h2><p>" + String(timeStr) + "</p><p>" + translate("Returning to main page in 3 seconds") + ".</p></body></html>";

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

                            if (!scaleAndSaveBmp(uploadFilePath.c_str(), uploadFilePath.c_str(), CLOCK_WIDTH, CLOCK_HEIGHT)) {
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


    // Prueft, ob das in einer Preset-URL angegebene Zifferblatt existiert - case-
    // insensitiv (LittleFS ist case-sensitiv, Nutzer koennten abweichend schreiben),
    // korrigiert die URL bei Treffer. False = kein passendes Zifferblatt (face_default.bmp immer gueltig).
    bool validateAndFixPresetFace(String& url, const std::vector<String>& existingFaces) {
        int facePos = url.indexOf("face=");
        if (facePos == -1) return true; // kein face-Parameter, nichts zu pruefen

        int valueStart = facePos + 5; // Laenge von "face="
        int valueEnd = url.indexOf('&', valueStart);
        if (valueEnd == -1) valueEnd = url.length();

        String faceValue = url.substring(valueStart, valueEnd);
        String faceName = faceValue.startsWith("/") ? faceValue.substring(1) : faceValue;

        if (faceName.equalsIgnoreCase("face_default.bmp")) {
            return true; // eingebautes Standard-Zifferblatt ist immer gueltig
        }

        for (const String& existing : existingFaces) {
            if (faceName.equalsIgnoreCase(existing)) {
                String correctValue = "/" + existing;
                if (correctValue != faceValue) {
                    // Gross-/Kleinschreibung weicht ab - URL korrigieren
                    url = url.substring(0, valueStart) + correctValue + url.substring(valueEnd);
                }
                return true;
            }
        }

        return false; // kein passendes Zifferblatt gefunden
    }


    // Verarbeitet den Datei-Upload fuer /importpresets: schreibt die Datei temporaer,
    // liest sie zeilenweise ein (Format "Name<TAB>URL", wie von /exportpresets erzeugt)
    // und fuegt nur NEUE Presets in freie Slots ein. Bestehende Presets werden NICHT
    // geloescht - auch nicht, wenn eine Zeile aus der Importdatei denselben Namen
    // traegt (der bestehende Eintrag bleibt dann einfach unangetastet erhalten).
    void handlePresetImportUpload() {
        HTTPUpload& upload = webserver.upload();

        if (upload.status == UPLOAD_FILE_START) {
            DEBUG_PRINTLN("[PRESET-IMPORT] Start");
            presetImportFile = LittleFS.open(PRESET_IMPORT_TMP_PATH, FILE_WRITE);
            presetImportSuccess = presetImportFile ? true : false;
        }
        else if (upload.status == UPLOAD_FILE_WRITE) {
            if (presetImportSuccess && presetImportFile) {
                presetImportFile.write(upload.buf, upload.currentSize);
            }
        }
        else if (upload.status == UPLOAD_FILE_END) {
            if (presetImportSuccess && presetImportFile) {
                presetImportFile.close();

                File readFile = LittleFS.open(PRESET_IMPORT_TMP_PATH, FILE_READ);
                if (!readFile) {
                    DEBUG_PRINTLN("[PRESET-IMPORT] Could not read file");
                    presetImportSuccess = false;
                    return;
                }

                // Namen der bereits vorhandenen Presets einmalig einsammeln, um
                // importierte Zeilen mit gleichem Namen ueberspringen zu koennen -
                // das bestehende Preset bleibt dadurch unveraendert erhalten.
                std::vector<String> existingPresetNames;
                for (int i = 0; i < MAX_PRESETS; i++) {
                    if (!presets[i].name.isEmpty() && !presets[i].url.isEmpty()) {
                        existingPresetNames.push_back(presets[i].name);
                    }
                }

                // Vorhandene Zifferblaetter einmalig einlesen, um jede
                // importierte Preset-Zeile dagegen pruefen zu koennen.
                std::vector<String> existingFaces;
                File faceRoot = LittleFS.open("/");
                File faceEntry = faceRoot.openNextFile();
                while (faceEntry) {
                    String entryName = faceEntry.name();
                    if (!faceEntry.isDirectory() && entryName.startsWith("face_") && entryName.endsWith(".bmp")) {
                        existingFaces.push_back(entryName);
                    }
                    faceEntry = faceRoot.openNextFile();
                }

                int importedCount = 0;
                int skippedCount = 0;
                while (readFile.available()) {
                    String line = readFile.readStringUntil('\n');
                    line.trim();
                    if (line.isEmpty()) continue;

                    int tabPos = line.indexOf('\t');
                    if (tabPos == -1) {
                        DEBUG_PRINTLN("[PRESET-IMPORT] Ungueltige Zeile (kein Tab): " + line);
                        continue;
                    }

                    String name = line.substring(0, tabPos);
                    String url = line.substring(tabPos + 1);
                    if (name.isEmpty() || url.isEmpty()) continue;

                    // Preset mit gleichem Namen existiert bereits - ueberspringen,
                    // statt es zu ueberschreiben oder zu loeschen.
                    bool alreadyExists = false;
                    for (const String& existingName : existingPresetNames) {
                        if (existingName == name) { alreadyExists = true; break; }
                    }
                    if (alreadyExists) {
                        DEBUG_PRINTLN("[PRESET-IMPORT] Skipped (already exists): " + name);
                        skippedCount++;
                        continue;
                    }

                    if (!validateAndFixPresetFace(url, existingFaces)) {
                        DEBUG_PRINTLN("[PRESET-IMPORT] Skipped (clock face not found): " + name);
                        skippedCount++;
                        continue;
                    }

                    int freeIndex = -1;
                    for (int i = 0; i < MAX_PRESETS; i++) {
                        if (presets[i].name.isEmpty() && presets[i].url.isEmpty()) {
                            freeIndex = i;
                            break;
                        }
                    }
                    if (freeIndex == -1) {
                        DEBUG_PRINTLN("[PRESET-IMPORT] No free slot left - aborted");
                        break;
                    }

                    presets[freeIndex].name = name;
                    presets[freeIndex].url = url;
                    existingPresetNames.push_back(name); // schuetzt auch vor Duplikaten INNERHALB der Importdatei
                    importedCount++;
                }
                readFile.close();
                LittleFS.remove(PRESET_IMPORT_TMP_PATH);

                if (importedCount > 0) {
                    savePresets();
                }

                DEBUG_PRINTLN("[PRESET-IMPORT] " + String(importedCount) + " presets imported, " + String(skippedCount) + " skipped");
                presetImportSuccess = true; // auch 0 neue Presets ist kein Fehler (z.B. alles schon vorhanden)
            }
            else {
                DEBUG_PRINTLN("[PRESET-IMPORT] Failed while writing");
            }
        }
    }


    // Aehnlich wie handlePresetImportUpload() - fuegt nur neue Presets in freie Slots
    // ein, bestehende bleiben erhalten. Wird vom GitHub-Download-Button auf /presets
    // genutzt: das Herausfiltern bereits vorhandener Namen erledigt hier schon die
    // aufrufende JavaScript-Funktion, bevor die Datei hier ankommt (daher KEINE
    // serverseitige Namens-Dopplungspruefung wie in handlePresetImportUpload()).
    void handlePresetMergeUpload() {
        HTTPUpload& upload = webserver.upload();

        if (upload.status == UPLOAD_FILE_START) {
            DEBUG_PRINTLN("[PRESET-MERGE] Start");
            presetImportFile = LittleFS.open(PRESET_IMPORT_TMP_PATH, FILE_WRITE);
            presetImportSuccess = presetImportFile ? true : false;
        }
        else if (upload.status == UPLOAD_FILE_WRITE) {
            if (presetImportSuccess && presetImportFile) {
                presetImportFile.write(upload.buf, upload.currentSize);
            }
        }
        else if (upload.status == UPLOAD_FILE_END) {
            if (presetImportSuccess && presetImportFile) {
                presetImportFile.close();

                File readFile = LittleFS.open(PRESET_IMPORT_TMP_PATH, FILE_READ);
                if (!readFile) {
                    DEBUG_PRINTLN("[PRESET-MERGE] Could not read file");
                    presetImportSuccess = false;
                    return;
                }

                std::vector<String> existingFaces;
                File faceRoot = LittleFS.open("/");
                File faceEntry = faceRoot.openNextFile();
                while (faceEntry) {
                    String entryName = faceEntry.name();
                    if (!faceEntry.isDirectory() && entryName.startsWith("face_") && entryName.endsWith(".bmp")) {
                        existingFaces.push_back(entryName);
                    }
                    faceEntry = faceRoot.openNextFile();
                }

                int addedCount = 0;
                int skippedCount = 0;
                while (readFile.available()) {
                    String line = readFile.readStringUntil('\n');
                    line.trim();
                    if (line.isEmpty()) continue;

                    int tabPos = line.indexOf('\t');
                    if (tabPos == -1) continue;

                    String name = line.substring(0, tabPos);
                    String url = line.substring(tabPos + 1);
                    if (name.isEmpty() || url.isEmpty()) continue;

                    if (!validateAndFixPresetFace(url, existingFaces)) {
                        DEBUG_PRINTLN("[PRESET-MERGE] Skipped (clock face not found): " + name);
                        skippedCount++;
                        continue;
                    }

                    int freeIndex = -1;
                    for (int i = 0; i < MAX_PRESETS; i++) {
                        if (presets[i].name.isEmpty() && presets[i].url.isEmpty()) {
                            freeIndex = i;
                            break;
                        }
                    }
                    if (freeIndex == -1) {
                        DEBUG_PRINTLN("[PRESET-MERGE] No free slot left - aborted");
                        break;
                    }

                    presets[freeIndex].name = name;
                    presets[freeIndex].url = url;
                    addedCount++;
                }
                readFile.close();
                LittleFS.remove(PRESET_IMPORT_TMP_PATH);

                if (addedCount > 0) {
                    savePresets();
                }
                presetImportSuccess = true; // Auch bei 0 neuen Presets kein Fehler (z.B. alles schon vorhanden)
                DEBUG_PRINTLN("[PRESET-MERGE] " + String(addedCount) + " new presets added, " + String(skippedCount) + " skipped");
            }
            else {
                DEBUG_PRINTLN("[PRESET-MERGE] Failed while writing");
            }
        }
    }
