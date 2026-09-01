#pragma once
    // ### Webinterface: alle HTTP-Routen & HTML-Generierung ##############
    // Benoetigt globals.h, config.h, prefs_keys.h und declarations.h (werden
    // zentral in uhr3.ino VOR dieser Datei eingebunden).

    // ### Web interface: all HTTP routes & HTML generation ##############
    // Requires globals.h, config.h, prefs_keys.h and declarations.h (included
    // centrally in uhr3.ino BEFORE this file).

    // Fuer "new (std::nothrow)" in /preview_defaultface: liefert bei
    // fehlgeschlagener Allokation garantiert nullptr, statt sich auf das
    // (implementierungsabhaengige) Verhalten von "new" ohne Exceptions zu
    // verlassen.
    // For "new (std::nothrow)" in /preview_defaultface: guarantees a nullptr on
    // a failed allocation instead of relying on the (implementation-defined)
    // behaviour of plain "new" without exceptions.
#include <new>

    // Generiert den HTML-Header für die Weboberfläche
    // Generates the HTML header for the web interface

    String generateHtmlHeader(String extraHead) {
        String html = "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
        html.reserve(2600);  // Header: jetzt mit Dark-Theme + Tab-CSS, einmal pro Seite aufgerufen
                             // Header: now includes dark theme + tab CSS, called once per page
        // Dunkles Theme (angelehnt an eine externe Referenzvorlage) + CSS-only
        // Tab-Mechanik (verstecktes radio-Input + Label + allgemeiner
        // Geschwister-Selektor "~") fuer die neue Tab-Hub-Startseite ("/").
        // Gilt sitenweit (jede Seite bindet generateHtmlHeader() ein), damit
        // auch die weiterhin eigenstaendigen Seiten (Presets, Dateiverwaltung,
        // etc.) optisch einheitlich bleiben.

        // Dark theme (inspired by an external reference) plus a CSS-only tab
        // mechanism (hidden radio input + label + general sibling selector "~")
        // for the new tab-hub home page ("/"). Applies site-wide (every page
        // includes generateHtmlHeader()) so standalone pages stay visually consistent.
        html += "<style>";
        html += ":root{--bg:#10151a;--panel:#1a2129;--panel-border:#2a333c;--text:#e8edf2;--muted:#8f9ba7;--accent:#f5a623;--accent-dim:#7a530f;--ok:#3ddc84;--bad:#ff5c5c;}";
        html += "body{font-family:Arial,Helvetica,sans-serif;text-align:center;padding-top:110px;background:var(--bg);color:var(--text);}";
        html += "input,select,button{margin:10px;padding:10px;width:80%;box-sizing:border-box;background:#0d1216;color:var(--text);border:1px solid var(--panel-border);border-radius:6px;}";
        html += "input[type=checkbox],input[type=radio]{width:auto;}";
        html += "button,button[type=submit],input[type=submit]{background:var(--accent);color:#1a1200;font-weight:bold;border:none;cursor:pointer;}";
        html += "button:hover,input[type=submit]:hover{background:#ffb84d;}";
        html += "h1,h2,h3{color:var(--text);}";
        html += "hr{border:0;height:1px;background-color:var(--panel-border);margin:20px 0;}";
        html += "table{margin:auto;border-collapse:collapse;}"; // Tabellen zentrieren
                                                                // center tables
        html += "th,td{padding:10px;text-align:center;border:1px solid var(--panel-border);}"; // Tabellenzellen
                                                                                               // table cells
        html += "li{text-align:left;color:var(--text);}"; // <li> linksbündig formatieren
                                                          // left-align <li>
        html += "a{color:var(--accent);}";
        html += "small{color:var(--muted);}";
        // --- CSS-only Tabs: Radios ausblenden, Panels standardmaessig
        // verstecken, per :checked ~ .panel-X wieder einblenden. Radios,
        // .tabnav und alle .panel-* muessen dafuer direkte Geschwister
        // sein (siehe Aufbau der neuen "/" Seite).

        // --- CSS-only tabs: hide radios, hide panels by default, show them
        // again via :checked ~ .panel-X. Radios, .tabnav and all .panel-* must
        // be direct siblings for this (see markup of the new "/" page).
        html += ".tabctrl{display:none;}";
        html += ".tabnav{margin:20px auto;max-width:900px;display:flex;flex-wrap:wrap;justify-content:center;gap:4px;}";
        html += ".tabnav label{background:var(--panel);border:1px solid var(--panel-border);color:var(--muted);padding:8px 16px;border-radius:8px 8px 0 0;cursor:pointer;font-weight:bold;}";
        html += ".tabpanel{display:none;}";
        html += ".card{background:var(--panel);border:1px solid var(--panel-border);border-radius:10px;max-width:500px;margin:15px auto;padding:12px 16px;text-align:left;}";
        for (const char* t : { "status", "wlan", "zifferblatt", "helligkeit", "zeit" }) {
            html += String("#tab-") + t + ":checked ~ .tabnav label[for='tab-" + t + "']{background:var(--accent);color:#1a1200;}";
            html += String("#tab-") + t + ":checked ~ .panel-" + t + "{display:block;}";
        }
        html += "</style>" + extraHead + "</head><body>";
        // Seite benötigt JavaScript
        // Page requires JavaScript
        html += "<noscript><div style='color:red;font-weight:bold;margin:20px;'>" + 
                translate("JavaScript is disabled.This page requires JavaScript to work properly!") + "</div></noscript>";
        
        return html;
    }


    // Einfache Hinweisseite (Erfolg/Fehler/Status) im dunklen Theme - fuer Seiten wie
    // "Settings saved", "Rebooting..." oder Upload-Fehlermeldungen, die vorher eigene,
    // unformatierte <body style='font-family:Arial'>-Seiten ohne Dark-Theme hatten.

    // Simple message page (success/error/status) in dark theme - for pages like
    // "Settings saved", "Rebooting..." or upload error messages that previously had
    // their own unstyled <body style='font-family:Arial'> pages without dark theme.

    String simpleMessagePage(String heading, String bodyHtml, String extraHead) {
        String html = generateHtmlHeader(extraHead);
        html += "<div class='card' style='max-width:480px;'>";
        html += "<h2>" + heading + "</h2>";
        html += bodyHtml;
        html += "</div></body></html>";
        return html;
    }


    /// Generiert den HTML-Statusabschnitt für die Weboberfläche
    // Generates the HTML status section for the web interface

    String generateHtmlStatus() {
        setLedOn();
        size_t total = LittleFS.totalBytes();
        size_t used = LittleFS.usedBytes();
        String html;
        html.reserve(512);  // Statusleiste: klein
                            // status bar: small
        if (WiFi.getMode() == WIFI_STA) {
            html = translate("Connected to") + ": <strong>" + WiFi.SSID() + "</strong>";
            // Bugfix: hier fehlte der Operand zwischen den beiden "+" ("http://" +  + "'>").
            // Das war gueltiges C++ (unaeres Plus auf einen const char*), lieferte aber
            // href='http://' ohne IP - der Link auf JEDER Seite fuehrte ins Leere.
            // Bugfix: the operand between the two "+" was missing ("http://" +  + "'>").
            // That was valid C++ (unary plus on a const char*) but produced
            // href='http://' without the IP - the link on EVERY page went nowhere.
            html += "<br>" + translate("IP Address") + ": <strong>" + "<a href='http://" + ipAddress + "'>http://" + ipAddress +"</a></strong> ";
            if (pingHostname)  html += "<br>" + translate("Hostname") + ": <strong>" + "<a href='http://" + hostname + ".local'>http://" + hostname + ".local</a>" + "</strong>";
        }
        else {
            // Im AP-Modus auch das Passwort mit anzeigen: es wird zur Laufzeit
            // aus der MAC-Adresse gebildet (siehe startAP() in wifi_manager.h)
            // und steht sonst nur waehrend des AP-Bildschirms auf dem Display.
            // Sichtbar ist die Zeile ausschliesslich hier im AP-Zweig, also nur
            // fuer jemanden, der ohnehin schon mit dem AP verbunden ist und das
            // Passwort damit kennt.

            // In AP mode also show the password: it is generated at runtime from
            // the MAC address (see startAP() in wifi_manager.h) and is otherwise
            // only readable while the AP screen is on the display. This line only
            // appears in the AP branch, i.e. only to someone already connected to
            // the AP who therefore knows the password anyway.
            html = "<br>Access Point: <strong>" + String(WiFi.softAPSSID()) + "</strong> (" + WiFi.softAPIP().toString() + ")";
            if (apPassword[0] != '\0') {
                html += "<br>" + translate("Password") + ": <strong>" + String(apPassword) + "</strong>";
            }
        }

        html += "<br>" + translate("Storage used") + ": " + String(used / 1024) + " KB / " + String(total / 1024) + " KB";
        html += " (" + translate("Free") + ": " + String((total - used) / 1024) + " KB)";
        html += "<br>" + translate("Version") + ": " + String(version);

        // Oben rechts fest positioniert, neben der Live-Vorschau (oben links) -
        // kompaktere Schrift, damit die paar Zeilen nicht mehr Hoehe brauchen
        // als das 90px hohe Vorschaubild gegenueber.

        // Fixed top-right, next to the live preview (top-left) - smaller font so
        // these few lines do not take up more height than the 90px-tall preview
        // image next to them.
        String boxed = "<div style='position:fixed;top:0;left:110px;max-width:calc(55% + 70px);height:100px;box-sizing:border-box;background:#1a2129;color:#e8edf2;border:2px solid #2a333c;border-radius:8px;padding:4px 8px;font-size:0.78em;line-height:1.25;text-align:left;white-space:nowrap;overflow-x:auto;overflow-y:auto;z-index:1000;'>";
        boxed += html;
        boxed += "</div><hr>";

        setLedOff();

        return boxed;
    }


    // Navigationsleiste generieren
    // Generate the navigation bar

    String generateNavigation() {
     /*   if (WiFi.getMode() != WIFI_STA) {
            DEBUG_PRINTLN("[HTML] Skipping HTML navigation");
            return "";
        }
        */
        String nav;
        nav.reserve(2048);
        nav = "<form id='previewSaveForm' method='POST' action='/api/createPreset' style='display:none;'><input type='hidden' id='previewSaveName' name='name'></form>";

        // Live rotierendes Zeiger-Widget oben links (ersetzt die vorherige
        // statische Rasterbild-Vorschau), auf jeder Seite (da hier in
        // generateNavigation()) - nutzt die tatsaechlich konfigurierten Zeiger-
        // Bitmaps, rotiert rein im Browser per Systemzeit, kein zusaetzlicher
        // Netzwerkverkehr zum ESP32 noetig ausser dem einmaligen Laden der Bilder.

        // Live rotating hand widget top-left (replaces the previous static raster
        // preview image), on every page (defined here in generateNavigation()) -
        // uses the actually configured hand bitmaps, rotates purely in the browser
        // via system time, no extra network traffic to the ESP32 besides the initial image load.
        {
            // WICHTIG: handHour/handMinute/handSecond enthalten nur den
            // eingebauten Standard-Zeigersatz. Ein benutzerdefinierter Satz
            // wird von loadHandSprites() direkt in TFT-Sprite-Objekte geladen,
            // NICHT in diese Arrays - daher hier bei aktivem Custom-Set die
            // Datei selbst einlesen, damit die Vorschau 1:1 dem tatsaechlich
            // aktiven Zeigersatz aus den Preferences entspricht.

            // IMPORTANT: handHour/handMinute/handSecond only contain the built-in
            // default hand set. A custom set is loaded by loadHandSprites() directly
            // into TFT sprite objects, NOT into these arrays - so with an active
            // custom set, read the file itself here so the preview matches the
            // actually active hand set from preferences exactly.
            String activeHandSet = preferences.getString(PK_HANDSET, "");
            uint16_t* previewHour = nullptr;
            uint16_t* previewMinute = nullptr;
            uint16_t* previewSecond = nullptr;
            const uint16_t* hourSrc = handHour;
            const uint16_t* minuteSrc = handMinute;
            const uint16_t* secondSrc = handSecond;
            const size_t handPixelCount = (size_t)HAND_WIDTH * HAND_HEIGHT;

            if (activeHandSet != "" && activeHandSet != "default") {
                String hourPath = "/hand_set" + activeHandSet + "_hour.bmp";
                String minutePath = "/hand_set" + activeHandSet + "_minute.bmp";
                String secondPath = "/hand_set" + activeHandSet + "_second.bmp";

                if (LittleFS.exists(hourPath)) {
                    previewHour = (uint16_t*)malloc(handPixelCount * 2);
                    if (previewHour && loadHandPixelsForPreview(hourPath.c_str(), previewHour, HAND_WIDTH, HAND_HEIGHT)) {
                        hourSrc = previewHour;
                    }
                }
                if (LittleFS.exists(minutePath)) {
                    previewMinute = (uint16_t*)malloc(handPixelCount * 2);
                    if (previewMinute && loadHandPixelsForPreview(minutePath.c_str(), previewMinute, HAND_WIDTH, HAND_HEIGHT)) {
                        minuteSrc = previewMinute;
                    }
                }
                if (LittleFS.exists(secondPath)) {
                    previewSecond = (uint16_t*)malloc(handPixelCount * 2);
                    if (previewSecond && loadHandPixelsForPreview(secondPath.c_str(), previewSecond, HAND_WIDTH, HAND_HEIGHT)) {
                        secondSrc = previewSecond;
                    }
                }
            }

            String hourB64 = encodePngToBase64(hourSrc, HAND_WIDTH, HAND_HEIGHT);
            String minuteB64 = encodePngToBase64(minuteSrc, HAND_WIDTH, HAND_HEIGHT);
            String secondB64 = encodePngToBase64(secondSrc, HAND_WIDTH, HAND_HEIGHT);

            // Temporaere Puffer sofort wieder freigeben - werden nur fuer die
            // obige Kodierung benoetigt, nicht dauerhaft.

            // Free temporary buffers right away - only needed for the encoding
            // above, not permanently.
            if (previewHour) free(previewHour);
            if (previewMinute) free(previewMinute);
            if (previewSecond) free(previewSecond);

            // hubColor liegt als RGB565 vor (Displayformat) - fuer CSS in RGB888 umrechnen
            // hubColor is in RGB565 (display format) - convert to RGB888 for CSS
            uint8_t hubR = ((hubColor >> 11) & 0x1F) * 255 / 31;
            uint8_t hubG = ((hubColor >> 5) & 0x3F) * 255 / 63;
            uint8_t hubB = (hubColor & 0x1F) * 255 / 31;
            char hubHex[8];
            snprintf(hubHex, sizeof(hubHex), "#%02x%02x%02x", hubR, hubG, hubB);

            bool showSecond = preferences.getBool(PK_SHOW_SECOND_HAND, true);
            bool stationModeActive = preferences.getBool(PK_STATION_MODE, true);
            bool smoothMinuteActive = preferences.getBool(PK_SMOOTH_MINUTE, true);

            // Alle Groessen/Positionen direkt VORSKALIERT berechnen (kein
            // transform:scale() mehr auf einem Eltern-Container - das
            // Zusammenspiel aus verschachteltem scale() (Eltern) + rotate()
            // (Kind) fuehrte dazu, dass die Rotation zwar im DOM korrekt
            // gesetzt wurde, aber optisch keinerlei sichtbare Wirkung zeigte.
            // Jetzt hat jedes Zeiger-Bild nur noch EINE Transform-Eigenschaft
            // (rotate), angewendet auf bereits korrekt skalierte Masse.

            // Compute all sizes/positions directly PRE-SCALED (no more
            // transform:scale() on a parent container - nesting scale() (parent)
            // with rotate() (child) meant the rotation was set correctly in the
            // DOM but had no visible effect). Now each hand image has only ONE
            // transform property (rotate), applied to already correctly scaled sizes.
            float scaleFactor = 90.0 / CLOCK_WIDTH;
            int scaledHandWidth = (int)(HAND_WIDTH * scaleFactor + 0.5);
            int scaledHandHeight = (int)(HAND_HEIGHT * scaleFactor + 0.5);
            int scaledPivotX = (int)((HAND_WIDTH / 2.0) * scaleFactor + 0.5);
            int scaledPivotY = (int)((HAND_HEIGHT * 0.77) * scaleFactor + 0.5);
            // hubSize ist ein RADIUS (wie bei fillCircle() auf dem echten Display,
            // siehe display.h, und bei generatePresetPreviewBmp()) - fuer den CSS-
            // Kreis unten wird aber der DURCHMESSER (width/height) gebraucht, daher
            // hier verdoppeln. Vorher fehlte die Verdopplung, wodurch der Punkt in
            // dieser Live-Vorschau nur halb so gross wie auf dem echten Display war.

            // hubSize is a RADIUS (like fillCircle() on the real display, see
            // display.h, and generatePresetPreviewBmp()) - but the CSS circle
            // below needs the DIAMETER (width/height), hence doubled here.
            // Previously missing this doubling made the hub in this live
            // preview only half as large as on the real display.
            int scaledHubSize = (int)(hubSize * 2 * scaleFactor + 0.5);
            if (scaledHubSize < 4) scaledHubSize = 4;

            nav += "<div onclick=\"var n=prompt('" + translate("Enter a name for the new preset (leave empty for automatic naming)") + "'); if(n !== null) { document.getElementById('previewSaveName').value = n; document.getElementById('previewSaveForm').submit(); }\" title='" + translate("Save the current clock settings as a new preset?") + "' style='position:fixed;top:0;left:0;width:100px;height:100px;box-sizing:border-box;border:2px solid #2a333c;border-radius:8px;background:#1a2129 url(/currentfacebg) center/cover no-repeat;z-index:1000;overflow:hidden;cursor:pointer;'>";
            nav += "<div id='liveHandsPivot' style='position:absolute;left:50%;top:50%;width:0;height:0;'>";
            nav += "<img id='liveHourHand' src='data:image/png;base64," + hourB64 + "' style='position:absolute;left:-" + String(scaledPivotX) + "px;top:-" + String(scaledPivotY) + "px;width:" + String(scaledHandWidth) + "px;height:" + String(scaledHandHeight) + "px;transform-origin:" + String(scaledPivotX) + "px " + String(scaledPivotY) + "px;'>";
            nav += "<img id='liveMinuteHand' src='data:image/png;base64," + minuteB64 + "' style='position:absolute;left:-" + String(scaledPivotX) + "px;top:-" + String(scaledPivotY) + "px;width:" + String(scaledHandWidth) + "px;height:" + String(scaledHandHeight) + "px;transform-origin:" + String(scaledPivotX) + "px " + String(scaledPivotY) + "px;'>";
            if (showSecond) {
                nav += "<img id='liveSecondHand' src='data:image/png;base64," + secondB64 + "' style='position:absolute;left:-" + String(scaledPivotX) + "px;top:-" + String(scaledPivotY) + "px;width:" + String(scaledHandWidth) + "px;height:" + String(scaledHandHeight) + "px;transform-origin:" + String(scaledPivotX) + "px " + String(scaledPivotY) + "px;'>";
            }
            nav += "<div style='position:absolute;left:-" + String(scaledHubSize / 2) + "px;top:-" + String(scaledHubSize / 2) + "px;width:" + String(scaledHubSize) + "px;height:" + String(scaledHubSize) + "px;border-radius:50%;background:" + String(hubHex) + ";'></div>";
            nav += "</div></div>";

            nav += "<script>";
            nav += "(function() {";
            nav += "  var hourEls = document.querySelectorAll('#liveHourHand');";
            nav += "  var minuteEls = document.querySelectorAll('#liveMinuteHand');";
            nav += "  var secondEls = document.querySelectorAll('#liveSecondHand');";
            nav += "  var stationMode = " + String(stationModeActive ? "true" : "false") + ";";
            nav += "  var smoothMinute = " + String(smoothMinuteActive ? "true" : "false") + ";";
            nav += "  var fastSecondMs = " + String((int)FAST_SECOND) + ";"; // aus der Firmware-Konstante FAST_SECOND uebernommen
                                                                             // taken from the firmware constant FAST_SECOND
            nav += "  var baseH = 0, baseM = 0, baseS = 0, baseAt = 0, haveBase = false;";
            nav += "  fetch('/api/currentTime').then(function(r) { return r.json(); }).then(function(t) {";
            nav += "    baseH = t.hour; baseM = t.minute; baseS = t.second; baseAt = performance.now(); haveBase = true;";
            nav += "  }).catch(function() {});";
            nav += "  function tick() {";
            nav += "    var h, m, s, ms;";
            nav += "    if (haveBase) {";
            nav += "      var elapsed = (performance.now() - baseAt) / 1000;"; // Sekunden seit dem einmaligen Abruf der ESP32-Zeit
                                                                               // seconds since the one-time fetch of the ESP32 time
            nav += "      var totalSec = baseH * 3600 + baseM * 60 + baseS + elapsed;";
            nav += "      h = Math.floor(totalSec / 3600) % 12;";
            nav += "      m = Math.floor(totalSec / 60) % 60;";
            nav += "      s = Math.floor(totalSec) % 60;";
            nav += "      ms = (totalSec - Math.floor(totalSec)) * 1000;";
            nav += "    } else {";
            nav += "      var now = new Date();"; // Fallback, solange die ESP32-Zeit noch nicht eingetroffen ist
                                                  // fallback until the ESP32 time has arrived
            nav += "      h = now.getHours() % 12; m = now.getMinutes(); s = now.getSeconds(); ms = now.getMilliseconds();";
            nav += "    }";
            nav += "    var minuteDeg = smoothMinute ? (m + s / 60) * 6 : m * 6;";
            nav += "    var hourDeg = (h + minuteDeg / 360) * 30;";
            nav += "    var secDeg;";
            nav += "    if (stationMode) {";
            nav += "      var elapsedMs = (s + ms / 1000) * 1000;";
            nav += "      var tickIndex = Math.floor(elapsedMs / fastSecondMs);";
            nav += "      var subTick = (elapsedMs % fastSecondMs) / fastSecondMs;";
            // Beschleunigen und Bremsen innerhalb jeder Sekundenteilung, exakt
            // wie easeInOutSine() in display.h - so liefen aeltere
            // Bahnhofsuhren. Muss mit der Firmware uebereinstimmen, sonst laufen
            // Vorschau und Uhr sichtbar auseinander.
            // Acceleration and braking within each second division, exactly like
            // easeInOutSine() in display.h - this is how older station clocks
            // ran. Has to match the firmware, otherwise the preview and the
            // clock visibly diverge.
            nav += "      var eased = -(Math.cos(Math.PI * Math.pow(subTick, 0.5)) - 1) / 2;";
            nav += "      var smoothSec = Math.min(tickIndex + eased, 60);";
            nav += "      secDeg = smoothSec * 6;";
            nav += "    } else {";
            nav += "      secDeg = s * 6;"; // springt zur vollen Sekunde, keine Millisekunden-Glaettung - entspricht der echten Firmware
                                            // jumps to the full second, no millisecond smoothing - matches the real firmware
            nav += "    }";
            nav += "    hourEls.forEach(function(el) { el.style.transform = 'rotate(' + hourDeg + 'deg)'; });";
            nav += "    minuteEls.forEach(function(el) { el.style.transform = 'rotate(' + minuteDeg + 'deg)'; });";
            nav += "    secondEls.forEach(function(el) { el.style.transform = 'rotate(' + secDeg + 'deg)'; });";
            nav += "    requestAnimationFrame(tick);";
            nav += "  }";
            nav += "  requestAnimationFrame(tick);";
            nav += "})();";
            nav += "</script>";
        }

        nav += "<style>";
        nav += "a { text-decoration: underline; font-weight: bold; }";
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
                                   // optional: confirmation message
        } navItems[] = {
            // WiFi/Zeit/Helligkeit/Status sind jetzt Tabs auf "/" (siehe dortiger
            // Tab-Hub) und daher hier bewusst NICHT mehr gelistet - die Seiten
            // selbst (/wifi, /timezone_form, /brightness, /status) bleiben aber
            // als eigenstaendige Routen bestehen (Rueckwaertskompatibilitaet fuer

            // Lesezeichen/direkte Aufrufe).
            // WiFi/time/brightness/status are now tabs on "/" (see the tab hub
            // there) and are therefore deliberately no longer listed here - the
            // pages themselves (/wifi, /timezone_form, /brightness, /status) still
            // exist as standalone routes (backward compatibility for bookmarks/direct calls).
            {"/", translate("Main"), ""},
            {"/preview", translate("Preview"), ""},
            {"/presets", translate("Presets"), ""},
            {"/listfilesFaces", translate("Clock&nbsp;Face"), ""},
            {"/handsets", translate("Hand&nbsp;Set"), ""},
            {"/files", translate("File&nbsp;Manager"), ""},
            {"/reboot", translate("Reboot"), translate("Are you sure you want to reboot?")},
            {"/factoryReset", translate("Factory&nbsp;Reset"), ""}
        };

        String currentPath = webserver.uri(); // Aktueller Pfad der Seite
                                              // current path of the page

        for (const auto& item : navItems) {
            if (item.path == currentPath) {
                // Wenn der aktuelle Pfad mit dem Navigationseintrag übereinstimmt, nur Text anzeigen
                // If the current path matches the nav entry, show plain text only
                nav += "<span style=\"margin-right:15px; font-weight:bold;\">" + item.label + "</span> ";
            }
            else {
                // Andernfalls als Link anzeigen
                // Otherwise show as a link
                nav += "<a href=\"" + item.path + "\" style=\"margin-right:15px;\"";
                if (!item.confirmMessage.isEmpty()) {
                    nav += " onclick=\"return confirm('" + item.confirmMessage + "')\"";
                }
                nav += ">" + item.label + "</a> ";
            }

            // Zeilenumbruch nach "Status" zur thematischen Trennung (Status/Betrieb
            // vs. Dateiverwaltung/Reset) - bewusst hier im Code statt in der
            // Übersetzung, da es reine Layout-Struktur ist, keine Textinhalt.

            // Line break after "Status" for thematic separation (status/operation
            // vs. file management/reset) - deliberately in code, not in the
            // translation, since it is pure layout structure, not text content.
            if (item.path == "/status") {
                nav += "<br>";
            }
        }

        nav += "</div>"; // Ende .navLinks
                         // end .navLinks
        nav += "</div>";
           
        return nav;
    }


    // Zeigt eine einheitliche Erfolgsmeldung, wenn die Route per Weiterleitung
    // einen "msg"-Parameter mitgibt (translate()-Schluessel, automatisch uebersetzt) -
    // blendet sich nach ein paar Sekunden per JS aus, ein Muster fuer alle Aktionen.

    // Shows a uniform success message when the route redirect passes a
    // "msg" parameter (translate() key, auto-translated) - fades out after
    // a few seconds via JS, a pattern used for all actions.

    String generateFlashMessage() {
        if (!webserver.hasArg("msg")) return "";
        String message = translate(webserver.arg("msg"));
        String html = "<div id='flashMsg' style='background:rgba(61,220,132,0.12);color:var(--ok);border:1px solid var(--ok);border-radius:6px;padding:10px 15px;margin:10px auto;max-width:500px;'>";
        html += "&#9989; " + message;
        html += "</div>";
        html += "<script>setTimeout(function(){var e=document.getElementById('flashMsg'); if(e) e.style.display='none';}, 4000);</script>";
        return html;
    }


    // Sprachselector generieren
    // Generate the language selector

    String generateLanguageSelector() {
        String html = "<form method='POST' action='/setLanguage'>";
        html.reserve(512);  // Sprachauswahl: klein
                            // language selector: small
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

    // Compares two filenames "naturally": digit sequences are compared as
    // numbers instead of character by character, so e.g. "hand_set2..." sorts before "hand_set10..."

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
                                                      // e.g. leading zeros as a tiebreaker
                continue;
            }
            if (ca != cb) {
                char lca = tolower(ca);
                char lcb = tolower(cb);
                if (lca != lcb) return lca < lcb;
                return ca < cb; // bei gleichem Buchstaben unterschiedlicher Groesse: Grossbuchstabe zuerst (stabiler Tiebreaker)
                                // for the same letter in different case: uppercase first (stable tiebreaker)
            }
            i++; j++;
        }
        return (a.length() - i) < (b.length() - j);
    }


    // Sortiert eine Liste von Dateinamen "natuerlich" (siehe naturalLess()) - Insertion-Sort
    // statt std::sort, um keine <algorithm>-Abhaengigkeit zu benoetigen (Dateianzahl ueberschaubar).

    // Sorts a list of filenames "naturally" (see naturalLess()) - insertion sort
    // instead of std::sort, to avoid an <algorithm> dependency (file counts are small).

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

    // Sanitizes user input into a valid hostname (RFC 952/1123): only letters,
    // digits and hyphens; spaces/underscores become hyphens, all other invalid
    // characters (umlauts, special characters, etc.) are removed; must not
    // start/end with a hyphen; max. 30 characters.

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
            // all other characters are silently removed
        }
        while (result.startsWith("-")) result = result.substring(1);
        while (result.endsWith("-")) result = result.substring(0, result.length() - 1);
        if (result.length() > 30) result = result.substring(0, 30);
        while (result.endsWith("-")) result = result.substring(0, result.length() - 1);
        return result;
    }


    // Sendet eine 302-Weiterleitung an location - buendelt das sonst ueberall
    // wiederholte sendHeader("Location", ...)/send(302, ...)-Paar.

    // Sends a 302 redirect to location - bundles the sendHeader("Location", ...)/
    // send(302, ...) pair that would otherwise be repeated everywhere.

    void redirectTo(const String& location, const String& body) {
        webserver.sendHeader("Location", location, true);
        webserver.send(302, "text/plain", body);
    }


    // Erzeugt den fuer fast jede Seite gleichen Seitenanfang (Header + Statusleiste
    // + Navigation) - Reihenfolge entspricht der bisherigen, wiederholten Aufrufkette.

    // Generates the page start common to almost every page (header + status bar
    // + navigation) - order matches the previous, repeated call chain.

    String beginPage() {
        String html = generateHtmlHeader();
        html += generateHtmlStatus();
        html += generateNavigation();
        return html;
    }


    // Liest fuer jeden konfigurierten WLAN-Slot einen evtl. mitgesendeten NTP-Server-
    // Parameter aus der Anfrage und speichert ihn (nur bei Aenderung) in ntpServers[]/
    // Preferences - gemeinsame Logik von /api/setMode und /set_timezone.

    // Reads a possibly submitted NTP server parameter for each configured WiFi
    // slot from the request and stores it (only on change) in ntpServers[]/
    // preferences - shared logic of /api/setMode and /set_timezone.

    void updateNtpServersFromRequest() {
        for (int i = 0; i < MAX_WLAN; i++) {
            String argName = pkNtpServer(i);
            if (webserver.hasArg(argName)) {
                strncpy(ntpServers[i], webserver.arg(argName).c_str(), sizeof(ntpServers[i]) - 1);
                ntpServers[i][sizeof(ntpServers[i]) - 1] = '\0'; // Null-terminieren
                                                                 // null-terminate
                if (preferences.getString(argName.c_str(), "") != String(ntpServers[i])) {
                    preferences.putString(argName.c_str(), ntpServers[i]);
                }
            }
        }
    }


    // Wandelt esp_reset_reason() in lesbaren Text um - fuer die Status-
    // Anzeige, hilfreich um z.B. einen Watchdog-Reset oder Panic von
    // einem normalen Neustart zu unterscheiden (siehe die WPS-Freeze-
    // Diagnose weiter oben in wifi_manager.h). Als eigenstaendige Funktion
    // (nicht verschachtelt in setupWebServer()) definiert, da C++ keine
    // Funktionsdefinitionen innerhalb einer anderen Funktion erlaubt.

    // Converts esp_reset_reason() into readable text - for the status
    // display, useful to distinguish e.g. a watchdog reset or panic from a
    // normal restart (see the WPS freeze diagnosis further up in
    // wifi_manager.h). Defined as a standalone function (not nested inside
    // setupWebServer()), since C++ does not allow function definitions
    // inside another function.

    String resetReasonToString(esp_reset_reason_t reason) {
        switch (reason) {
            case ESP_RST_POWERON: return "Power-on";
            case ESP_RST_EXT: return "External pin";
            case ESP_RST_SW: return "Software (esp_restart)";
            case ESP_RST_PANIC: return "Panic / exception";
            case ESP_RST_INT_WDT: return "Interrupt watchdog";
            case ESP_RST_TASK_WDT: return "Task watchdog";
            case ESP_RST_WDT: return "Other watchdog";
            case ESP_RST_DEEPSLEEP: return "Deep sleep wake";
            case ESP_RST_BROWNOUT: return "Brownout";
            case ESP_RST_SDIO: return "SDIO";
            case ESP_RST_USB: return "USB";
            case ESP_RST_JTAG: return "JTAG";
            case ESP_RST_EFUSE: return "Efuse error";
            case ESP_RST_PWR_GLITCH: return "Power glitch";
            case ESP_RST_CPU_LOCKUP: return "CPU lockup";
            default: return "Unknown";
        }
    }


    // Wandelt rtcOk (siehe globals.h) in lesbaren Text um.
    // Converts rtcOk (see globals.h) into readable text.

    String rtcStatusToString(int status) {
        if (status == RTC_AVAILABLE) return "OK";
        if (status == RTC_AVAILABLE_BUT_INVALID) return "found, but time invalid";
        return "not found";
    }


    // Formatiert eine Millis-Zeitspanne wie bei Uptime (Xd Xh Xm Xs) -
    // gemeinsam genutzt fuer "letzter NTP-Sync vor ...".

    // Formats a millis duration the same way as Uptime (Xd Xh Xm Xs) -
    // shared for "last NTP sync ... ago".

    String formatDurationMs(unsigned long ms) {
        unsigned long seconds = ms / 1000;
        unsigned long days = seconds / 86400;
        unsigned long hours = (seconds % 86400) / 3600;
        unsigned long minutes = (seconds % 3600) / 60;
        unsigned long secs = seconds % 60;
        return String(days) + "d " + String(hours) + "h " + String(minutes) + "m " + String(secs) + "s";
    }


    // Baut den wiederverwendeten inneren Teil des Helligkeits-Formulars (Auto-
    // Helligkeit-Checkboxen + alle Zahlenfelder) - war frueher an drei Stellen
    // dupliziert (eigenstaendige /brightness-Seite per GET/POST sowie das
    // Helligkeit-Tab-Panel auf "/"), was schon zu kleinen Inkonsistenzen
    // gefuehrt hat (z.B. abweichender "%"-Hinweistext beim Schwellwert).
    // Umschliessendes <form>/<div> und der Save-Button bleiben bewusst bei den
    // Aufrufern, da sich Action-URL, "returnTo"-Feld und Wrapper-Div dort
    // unterscheiden.

    // Builds the reused inner part of the brightness form (auto-brightness
    // checkboxes + all number fields) - used to be duplicated in three places
    // (standalone /brightness page via GET/POST, and the Brightness tab panel
    // on "/"), which had already led to small inconsistencies (e.g. a
    // differing "%" hint text on the threshold fields). The surrounding
    // <form>/<div> and the Save button stay with the callers on purpose,
    // since the action URL, "returnTo" field, and wrapper div differ there.

    String brightnessFormFieldsHtml() {
        String html = "";

        if (photoresistorFound) {
            html += "<table style='margin:auto;text-align:left;'><tr>";
            html += "<td><label><input type='checkbox' name='use_adc' value='1' " + String(useAdc ? "checked" : "") + "> " + translate("Enable Auto Brightness") + "</label> <span title='" + translate("Automatically adjusts brightness based on ambient light measured by the photoresistor") + ".' style='cursor:help;'>&#9432;</span></td>";
            html += "<td><label><input type='checkbox' name='adcInverted' value='1' " + String(adcInverted ? "checked" : "") + "> " + translate("Invert ADC Reading") + "</label> <span title='" + translate("Reverses the brightness sensor reading - use if the display gets darker in bright light instead of brighter") + ".' style='cursor:help;'>&#9432;</span></td>";
            html += "</tr></table><hr><br>";
        }

        html += "<div style='display:flex;align-items:center;gap:6px;white-space:nowrap;'><label style='width:280px;display:inline-block;white-space:normal;'>" + translate("Full brightness from (hour, 0-23)") + ":</label><input name = 'brightStart' type = 'number' min = '0' max = '23' value = '" + String(brightStartHour) + "' style='width:70px;'> <span title='" + translate("Start of the daily time window during which the display always uses full brightness, regardless of ambient light") + ".' style='cursor:help;'>&#9432;</span></div>";
        html += "<div style='display:flex;align-items:center;gap:6px;white-space:nowrap;'><label style='width:280px;display:inline-block;white-space:normal;'>" + translate("Full brightness until (hour, 0-23)") + ":</label><input name = 'brightEnd' type = 'number' min = '0' max = '23' value = '" + String(brightEndHour) + "' style='width:70px;'> <span title='" + translate("End of the daily time window during which the display always uses full brightness, regardless of ambient light") + ".' style='cursor:help;'>&#9432;</span></div><br>";

        html += "<div style='display:flex;align-items:center;gap:6px;white-space:nowrap;'><label style='width:280px;display:inline-block;white-space:normal;'>" + translate("Min Brightness") + " (0 - 255) : </label><input name = 'minBrightness' type = 'number' min = '0' max = '255' value = '" + String(minBrightness) + "' style='width:70px;'> <span title='" + translate("Display brightness used at or below the low threshold") + ".' style='cursor:help;'>&#9432;</span></div>";
        html += "<div style='display:flex;align-items:center;gap:6px;white-space:nowrap;'><label style='width:280px;display:inline-block;white-space:normal;'>" + translate("Max Brightness") + " (0 - 255) : </label><input name = 'maxBrightness' type = 'number' min = '0' max = '255' value = '" + String(maxBrightness) + "' style='width:70px;'> <span title='" + translate("Display brightness used at or above the high threshold") + ".' style='cursor:help;'>&#9432;</span></div><br>";

        if (photoresistorFound) {
            html += "<div style='display:flex;align-items:center;gap:6px;white-space:nowrap;'><label style='width:280px;display:inline-block;white-space:normal;'>" + translate("Low Threshold") + " (0 - 100 %) : </label><input name = 'lowThreshold' type = 'number' min = '0' max = '100' value = '" + String(lowThreshold) + "' style='width:70px;'> <span title='" + translate("Below this ambient light percentage, the display uses minimum brightness") + ".' style='cursor:help;'>&#9432;</span></div>";
            html += "<div style='display:flex;align-items:center;gap:6px;white-space:nowrap;'><label style='width:280px;display:inline-block;white-space:normal;'>" + translate("High Threshold") + " (0 - 100 %) : </label><input name = 'highThreshold' type = 'number' min = '0' max = '100' value = '" + String(highThreshold) + "' style='width:70px;'> <span title='" + translate("Above this ambient light percentage, the display uses maximum brightness") + ".' style='cursor:help;'>&#9432;</span></div>";
        }

#if defined (GC9D01)  || defined(GC9A01_WITH_BACKLIGHT)
        html += "<div style='display:flex;align-items:center;gap:6px;white-space:nowrap;'><label style='width:280px;display:inline-block;white-space:normal;'>" + translate("Gamma Correction") + " (0.1 - 3.0) : </label><input type='number' name='gamma' step='0.1' min='0.1' max='3.0' value='" + String(gammaBrightness) + "' required style='width:70px;'> <span title='" + translate("Adjusts how brightness ramps between minimum and maximum - higher values keep the display darker for longer before brightening") + ".' style='cursor:help;'>&#9432;</span></div>";
#endif

        return html;
    }


    // Liest ein GET/POST-Formularfeld als int, mit echter Validierung: fehlt
    // das Feld oder enthaelt es keine gueltige Ganzzahl (z.B. leer oder
    // Buchstaben), wird defaultValue zurueckgegeben statt stillschweigend 0
    // (so verhaelt sich String::toInt() bei ungueltiger Eingabe) - anschliessend
    // wird das Ergebnis auf [minVal, maxVal] begrenzt, damit fehlerhafte/boese
    // Werte (z.B. riesige oder negative Zahlen) nicht unbemerkt in Preferences
    // landen oder bei der Zuweisung an kleinere Typen (uint8_t etc.) ueberlaufen.

    // Reads a GET/POST form field as an int, with real validation: if the
    // field is missing or does not contain a valid integer (e.g. empty or
    // letters), defaultValue is returned instead of silently 0 (which is
    // String::toInt()'s behavior on invalid input) - the result is then
    // clamped to [minVal, maxVal] so malformed/malicious values (e.g. huge or
    // negative numbers) don't end up unnoticed in Preferences or overflow when
    // assigned to smaller types (uint8_t etc.).

    int argToIntClamped(const String& name, int defaultValue, int minVal, int maxVal) {
        // defaultValue selbst wird ebenfalls begrenzt, damit ein Aufrufer nicht
        // versehentlich einen Default ausserhalb von [minVal, maxVal] uebergeben
        // und so die Begrenzung fuer den "Feld fehlt/ungueltig"-Fall aushebeln kann.

        // defaultValue itself is also clamped, so a caller can't accidentally
        // pass a default outside [minVal, maxVal] and thereby bypass the bound
        // for the "field missing/invalid" case.
        if (defaultValue < minVal) defaultValue = minVal;
        if (defaultValue > maxVal) defaultValue = maxVal;

        if (!webserver.hasArg(name)) return defaultValue;
        String val = webserver.arg(name);
        val.trim();
        if (val.length() == 0) return defaultValue;
        for (size_t i = 0; i < val.length(); i++) {
            char c = val.charAt(i);
            bool isSign = (i == 0 && (c == '-' || c == '+'));
            if (!isDigit(c) && !isSign) return defaultValue;
        }
        long parsed = val.toInt();
        if (parsed < minVal) parsed = minVal;
        if (parsed > maxVal) parsed = maxVal;
        return (int)parsed;
    }


    // Webserver-API-Endpunkte einrichten
    // Set up the webserver API endpoints

    void setupWebServer() {

        // Captive-Portal-Erkennung: Android/iOS/Windows fragen beim Verbinden diese
        // bekannten URLs ab - Redirect (statt 404) auf die Konfigurationsseite oeffnet
        // beim AP ("clock123") automatisch ein Browserfenster, wirkt mit dnsServer.processNextRequest() zusammen.

        // Captive portal detection: Android/iOS/Windows probe these known URLs
        // when connecting - redirecting (instead of 404) to the config page opens
        // a browser window automatically on the AP ("clock123"), works together with dnsServer.processNextRequest().
        auto captivePortalRedirect = []() {
            redirectTo("http://" + ipAddress + "/");
            };

        webserver.on("/generate_204", HTTP_GET, captivePortalRedirect);       // Android
        webserver.on("/gen_204", HTTP_GET, captivePortalRedirect);            // Android (aeltere Versionen)
                                                                              // Android (older versions)
        webserver.on("/hotspot-detect.html", HTTP_GET, captivePortalRedirect); // iOS
                                                                               // macOS
        webserver.on("/library/test/success.html", HTTP_GET, captivePortalRedirect); // iOS
                                                                                     // macOS (alternative)
        webserver.on("/ncsi.txt", HTTP_GET, captivePortalRedirect);           // Windows
        webserver.on("/connecttest.txt", HTTP_GET, captivePortalRedirect);    // Windows
                                                                              // Catch-all fuer alle sonstigen, unbekannten Anfragen (z.B. Varianten
        // der obigen URLs oder Domains, die nicht explizit registriert sind) -
        // statt eines 404 lieber ebenfalls auf die Konfigurationsseite leiten.

        // Catch-all for all other, unknown requests (e.g. variants of the URLs
        // above or domains that are not explicitly registered) - redirect to the
        // config page instead of a 404.
        webserver.onNotFound(captivePortalRedirect);

        // API zum Setzen von Zifferblatt, Zeigersatz, Zeitzone, Mittelpunkt-Groesse/-Farbe, Bahnhofsmodus, Rotation, Sekundenzeiger-Sichtbarkeit und sanftem Minutenzeiger
        // API to set clock face, hand set, timezone, hub size/color, station mode, rotation, second hand visibility, and smooth minute hand

        // DB
        // http://192.168.0.214/api/setMode?face=face_db_uhr.bmp&handSet=0&hubSize=6&hubColor=ff0000showSecondHand=1&stationMode=true&smoothMinute=false&rotation=2

        // Irish Pub
        // http://192.168.0.214/api/setMode?face=face_irish_pub.bmp&handSet=0&hubSize=2&hubColor=aaaaaa&showSecondHand=false&stationMode=false&smoothMinute=true&rotation=2



        // API zum Zurücksetzen der WiFi-Einstellungen

        // API to reset the WiFi settings
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
            // Send back a confirmation
            webserver.send(200, "application/json", "{\"status\":\"WiFi settings reset successfully\"}");
            DEBUG_PRINTLN("[API] WiFi settings reset via /api/resetWiFi");

            delay(WAIT_1s);
            // Neustart des ESP
            // Restart the ESP
            espReboot();

            });

        // resetWifi POST API, um WiFi-Einstellungen zurückzusetzen
        // resetWifi POST API to reset WiFi settings
        webserver.on("/api/resetWiFi", HTTP_POST, []() {
            DEBUG_PRINTLN("[API] Received POST request to /api/resetWiFi, resetting WiFi settings..");
       
            eraseWiFiConfig();
            // Sende eine Bestätigung zurück
            // Send back a confirmation
            webserver.send(200, "application/json", "{\"status\":\"WiFi settings reset successfully\"}");
            DEBUG_PRINTLN("[API] WiFi settings reset via /api/resetWiFi");

            // preferences.end() nicht mehr hier, sondern in espReboot() selbst -
            // sonst faellt der dort geloggte Reboot-Eintrag auf die falsche
            // Logdatei zurueck (siehe Kommentar in espReboot()).

            // preferences.end() no longer called here, but inside espReboot()
            // itself - otherwise the reboot log entry logged there falls back
            // to the wrong log file (see comment in espReboot()).
            delay(WAIT_1s);
            // Neustart des ESP
            // Restart the ESP
            espReboot();
            });

        // Startet eine WPS-Anfrage per Web-Button, um ein neues WLAN hinzuzufuegen.
        // Kehrt SOFORT zurueck (kein Blockieren des Webservers) - der eigentliche
        // Verbindungsversuch und das Speichern der Zugangsdaten laeuft asynchron
        // in loop() (siehe uhr3.ino), analog zur bestehenden WPS-Logik in setup().
        // GET zusaetzlich zu POST registriert (analog zu /api/resetWiFi), damit
        // die Route auch bei direkter Browser-Navigation erreichbar ist.

        // Starts a WPS request via web button to add a new WiFi network. Returns
        // IMMEDIATELY (does not block the webserver) - the actual connection attempt
        // and saving credentials runs asynchronously in loop() (see uhr3.ino),
        // like the existing WPS logic in setup(). Also registered for GET (like
        // /api/resetWiFi) so the route is reachable via direct browser navigation.
        webserver.on("/api/startWPS", HTTP_GET, []() {
            wpsPreviousSsid = WiFi.isConnected() ? WiFi.SSID() : "";
            redirectTo("/?tab=wlan&msg=WPS%20active%20-%20press%20the%20WPS%20button%20on%20your%20router%20now%20(within%202%20minutes)");
            // Erst NACH dem Senden der Antwort WPS starten - startWPS() stoerte
            // sonst vermutlich die noch offene HTTP-Verbindung (Browser zeigte
            // "Seite nicht erreichbar" statt die Weiterleitung zu erhalten).

            // Only start WPS AFTER sending the response - startWPS() would otherwise
            // probably disturb the still-open HTTP connection (browser showed
            // "page not reachable" instead of receiving the redirect).
            startWPS();
            wpsPending = true;
            wpsStartMillis = millis();
            });

        webserver.on("/api/startWPS", HTTP_POST, []() {
            wpsPreviousSsid = WiFi.isConnected() ? WiFi.SSID() : "";
            redirectTo("/?tab=wlan&msg=WPS%20active%20-%20press%20the%20WPS%20button%20on%20your%20router%20now%20(within%202%20minutes)");
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

        // Saves a custom hostname (only takes effect after a reboot, since
        // WiFi.setHostname()/MDNS.begin() are only called once when connecting
        // in connectWiFi()). Sanitizes user input into a valid hostname (RFC
        // 952/1123): only letters, digits and hyphens; spaces/underscores become
        // hyphens, other invalid characters are removed; max. 30 characters, no leading/trailing hyphen.

        webserver.on("/sethostname", HTTP_POST, []() {
            if (webserver.hasArg("hostname")) {
                String newHostname = sanitizeHostname(webserver.arg("hostname"));

                if (newHostname.isEmpty()) {
                    // Kein gueltiger Hostname aus der Eingabe extrahierbar - den
                    // gespeicherten Override entfernen, damit beim naechsten Boot
                    // wieder automatisch "clock_" + letzte MAC-Stellen generiert
                    // wird (siehe connectWiFi() in wifi_manager.h).

                    // No valid hostname could be extracted from the input - remove the
                    // stored override so that on the next boot "clock_" + the last MAC
                    // digits is generated automatically again (see connectWiFi() in
                    // wifi_manager.h).
                    preferences.remove(PK_HOSTNAME);
                    redirectTo("/?tab=wlan&msg=No%20valid%20hostname%20could%20be%20derived%20from%20the%20input%20-%20falling%20back%20to%20the%20automatic%20name%20based%20on%20the%20MAC%20address");
                    return;
                }

                preferences.putString(PK_HOSTNAME, newHostname);
                redirectTo("/?tab=wlan&msg=Hostname%20saved%20-%20requires%20a%20reboot%20to%20take%20effect");
            }
            else {
                webserver.send(400, "text/plain", "Missing parameter");
            }
            });

        webserver.on("/api/createPreset", HTTP_POST, []() {
            String customName = webserver.hasArg("name") ? webserver.arg("name") : "";
            bool created = createPresetFromPreferences(customName); // Erstellt ein neues Preset, falls noch ein Slot frei ist
                                                                    // creates a new preset if a slot is still free

            if (!created) {
                String html = beginPage();
                html += "<h2 style='color:red;'>" + translate("Maximum number of presets reached - delete an existing preset first") + "</h2>";
                html += "<a href='/presets'><button type='button'>" + translate("Back") + "</button></a>";
                html += "</body></html>";
                webserver.send(200, "text/html", html);
                return;
            }

            // Weiterleitung zur Presets-Seite
            // Redirect to the presets page
            redirectTo("/presets?msg=Preset%20created", "Redirecting to /presets..");
            });

        // API zum Setzen von Uhrmodus und anderen Einstellungen
        // API to set clock mode and other settings
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
                hubSize = argToIntClamped("hubSize", hubSize, 0, 100);
                preferences.putUInt(PK_CENTER_SIZE, hubSize);
            }
            if (webserver.hasArg("hubColor")) {
                uint32_t rgb = strtoul(webserver.arg("hubColor").c_str(), NULL, 16); // 24-Bit RGB

                // 24-bit RGB
                // DEBUG_PRINTLN("[API] Received hubColor: " + webserver.arg("hubColor") + " -> " + String(rgb, HEX));
                uint8_t r = (rgb >> 16) & 0xFF; // Rot extrahieren
                                                // extract red
                uint8_t g = (rgb >> 8) & 0xFF;  // Grün extrahieren
                                                // extract green
                uint8_t b = rgb & 0xFF;         // Blau extrahieren
                                                // extract blue
                // Konvertiere RGB888 zu RGB565
                // Convert RGB888 to RGB565
                hubColor = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
                preferences.putLong(PK_CENTER_COLOR, rgb);
            }


            if (webserver.hasArg("stationMode")) {
                String stationModeArg = webserver.arg("stationMode");
                stationMode = (stationModeArg == "1" || stationModeArg.equalsIgnoreCase("true")); // Konvertiere zu bool
                                                                                                  // convert to bool
                preferences.putBool(PK_STATION_MODE, stationMode);
            }

            if (webserver.hasArg("rotation")) {
                String rotationArg = webserver.arg("rotation");
                // Bugfix: hiess vorher ebenfalls "tftRotation" (heute tftRotation1)
                // und ueberschattete damit die GLOBALE Variable - der
                // Preferences-Wert wurde zwar korrekt gespeichert und die Drehung
                // einmalig sofort angewendet, aber renderClockFrame() liest
                // weiterhin die globale Variable und fiel beim naechsten Tick auf
                // die alte Rotation zurueck.

                // Bugfix: this used to also be named "tftRotation" (today
                // tftRotation1) and therefore shadowed the GLOBAL variable - the
                // value was correctly saved to Preferences and the rotation
                // applied immediately once, but renderClockFrame() still reads
                // the global variable and fell back to the old rotation on the
                // next tick.
                // Als long fuehren, NICHT als uint8_t: toInt() liefert einen long,
                // und eine Zuweisung an uint8_t haette den Wert vorher auf 8 Bit
                // abgeschnitten - "rotation=256" wurde so zu 0 und "rotation=257"
                // zu 1, beides hat die Pruefung unten klaglos passiert und wurde
                // gespeichert. Die Validierung hat also Unsinn stillschweigend
                // akzeptiert statt ihn abzulehnen.

                // Keep it as a long, NOT a uint8_t: toInt() returns a long, and
                // assigning to uint8_t truncated the value to 8 bits beforehand -
                // "rotation=256" became 0 and "rotation=257" became 1, both of which
                // silently passed the check below and got saved. So the validation
                // quietly accepted nonsense instead of rejecting it.
                long requestedRotation = -1;

                // Prüfe, ob der Wert in Grad angegeben ist
                // Check whether the value is given in degrees
                if (rotationArg == "0" || rotationArg == "90" || rotationArg == "180" || rotationArg == "270") {
                    if (rotationArg == "0") requestedRotation = 0;
                    else if (rotationArg == "90") requestedRotation = 1;
                    else if (rotationArg == "180") requestedRotation = 2;
                    else if (rotationArg == "270") requestedRotation = 3;
                }
                // Prüfe, ob der Wert als Index (0-3) angegeben ist
                // Check whether the value is given as an index (0-3)
                else {
                    requestedRotation = rotationArg.toInt();
                }

                // Validierung des Wertes
                // Validate the value
                if (requestedRotation >= 0 && requestedRotation <= 3) {
                    uint8_t newRotation = (uint8_t)requestedRotation;
                    uint8_t previousRotation = preferences.getUChar(PK_TFT_ROTATION1, 0);
                    preferences.putUChar(PK_TFT_ROTATION1, newRotation);
                    tftRotation1 = newRotation; // globale Variable aktualisieren (siehe Bugfix-Kommentar oben)
                                               // update the global variable (see bugfix comment above)
                    // firstRun nur bei TATSAECHLICHER Aenderung zuruecksetzen -
                    // sonst wuerde jedes Speichern der Haupteinstellungen (die
                    // das Rotationsfeld immer mitsenden, auch unveraendert) die
                    // Bahnhofsmodus-Wartephase am Sekundenzeiger neu starten.

                    // Only reset firstRun on an ACTUAL change - otherwise every save of the
                    // main settings (which always submit the rotation field, even unchanged)
                    // would restart the station-mode wait phase on the second hand.
                    if (tftRotation1 != previousRotation) {
                        firstRun = true;
                    }
                    if (!gc9d01SwRotation) {
                        // Wie bei /applydisplaysettings: explizit auf Display 1 umschalten,
                        // bevor die Rotation gesetzt wird - tft.setRotation() wirkt nur auf
                        // den aktuell selektierten Chip.

                        // Same as in /applydisplaysettings: explicitly switch to Display 1
                        // before applying the rotation - tft.setRotation() only affects the
                        // currently selected chip.
                        setCS1(LOW);
                        tft.setRotation(tftRotation1); // sofort anwenden
                                                      // apply immediately
                    }
                }
            }

   
            if (webserver.hasArg("showSecondHand")) {
                String showSecondHandArg = webserver.arg("showSecondHand");
                showSecondHand = (showSecondHandArg == "1" || showSecondHandArg.equalsIgnoreCase("true")); // Konvertiere zu bool
                                                                                                           // convert to bool
                preferences.putBool(PK_SHOW_SECOND_HAND, showSecondHand);
            }

            if (webserver.hasArg("smoothMinute")) {
                String smoothMinuteArg = webserver.arg("smoothMinute");
                smoothMinute = (smoothMinuteArg == "1" || smoothMinuteArg.equalsIgnoreCase("true")); // Konvertiere zu bool
                                                                                                     // convert to bool
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
        // Preset management
        webserver.on("/presets", HTTP_GET, []() {
            webserver.setContentLength(CONTENT_LENGTH_UNKNOWN);
            webserver.send(200, "text/html", "");

            String chunk = beginPage();
            chunk.reserve(1024);
            chunk += generateFlashMessage();
            chunk += "<h2>" + translate("Manage Presets") + "</h2>";

            // Links oben anzeigen
            // Show links at the top
            chunk += "<div style='text-align:center;'>";

            if (pingHostname) {
                chunk += "<p>" + translate("Use the host name") + " <strong>" + String(hostname) + ".local</strong> " + translate("instead of the IP address for better reliability") + ".</p>";
            }

            chunk += "<ul style='list-style-type:none; padding:0; display:inline-block; text-align:left;'>";

         
            String espHost = "http://" + String(hostname) + ".local"; // Aktueller Hostname des ESP
                                                                      // current hostname of the ESP

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

            // Only sort the display order (array indices stay unchanged so
            // rename/delete/preview links remain correct) - insertion sort like
            // naturalSortNames(), to avoid an <algorithm> dependency.
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
                    // Replace the stored IP with the ESP's current IP
                    if (displayUrl.startsWith("http://")) {
                        int ipEnd = displayUrl.indexOf('/', 7); // Suche nach dem Ende der IP-Adresse
                                                                // find the end of the IP address
                        if (ipEnd != -1) {
                            displayUrl = "http://" + ipAddress + displayUrl.substring(ipEnd); // Ersetze die IP
                                                                                              // replace the IP
                        }
                        else {
                            displayUrl = "http://" + ipAddress; // Nur die IP ohne Pfad
                                                                // just the IP, without path
                        }
                    }
                    displayUrl += "&source=preset";
                    // Bugfix: hier stand vorher presets[i].name.replace(" ", "_") -
                    // ein reiner GET-Request hat damit den GLOBALEN Preset-Zustand im RAM
                    // veraendert. Folgen: switchToNextPreset() verglich den (aus NVS
                    // gelesenen) Namen mit Leerzeichen gegen den mutierten Namen und fand
                    // nie einen Treffer, und das naechste beliebige savePresets() (z.B.
                    // ueber /deletepreset) hat die Mutation dauerhaft ins NVS geschrieben.
                    // Der Aufruf war ohnehin ueberfluessig - der Block unten legt sich
                    // fuer die URL bereits eine eigene lokale Kopie an (presetName).

                    // Bugfix: this used to be presets[i].name.replace(" ", "_") - a plain
                    // GET request thus mutated the GLOBAL preset state in RAM.
                    // Consequences: switchToNextPreset() compared the name read from NVS
                    // (with spaces) against the mutated name and never found a match, and
                    // the next arbitrary savePresets() (e.g. via /deletepreset) persisted
                    // the mutation into NVS. It was redundant anyway - the block below
                    // already makes its own local copy (presetName) for the URL.

                    chunk += "<div style='text-align:center;border:1px solid #ccc;border-radius:6px;padding:8px;width:220px;'>";
                    chunk += "<a href='" + displayUrl + "'><img src='/presetpreview?index=" + String(i) + "' style='width:90px;height:90px;'></a>";
                    chunk += "<br><a href='" + displayUrl + "'>" + presets[i].name + "</a>";
                    String presetName = presets[i].name;
                    presetName.replace(" ", "_"); // Ersetze Leerzeichen durch Unterstriche
                                                  // replace spaces with underscores
                    String ipLink = "http://" + ipAddress + "/api/setPreset?name=" + presetName;
                    chunk += "<br><span onclick=\"copyPresetLink('" + ipLink + "', this)\" style='cursor:pointer;font-size:1.3em;' title='" + translate("Copy link") + "'>&#128203;</span>";
                    if (pingHostname) {
                        String hostLink = espHost + "/api/setPreset?name=" + presetName;
                        chunk += " <span onclick=\"copyPresetLink('" + hostLink + "', this)\" style='cursor:pointer;font-size:1.3em;' title='" + translate("Copy link") + " (" + espHost + ")'>&#128203;</span>";
                    }
                    chunk += "<br><a href='/renamepreset_form?index=" + String(i) + "'>" + translate("Rename") + "</a> ";
                    chunk += "<button type='button' onclick='if(confirm(\"" + translate("Delete") + " " + presets[i].name + "?\")){window.location.href=\"/deletepreset?index=" + String(i) + "\";}'>" + translate("Delete") + "</button>";
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

            // For the JS merge comparison: provide already existing preset names
            // and existing clock-face/hand-set files, so the GitHub download only
            // adds what is really new and automatically fetches the still-missing
            // faces/hand sets needed for it.
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
            // If no presets exist when the page loads, automatically ask whether
            // some should be loaded from GitHub - relevant e.g. after a factory
            // reset or initial setup. Checks GitHub reachability multiple times
            // (with a short pause in between) before asking, so that right after a
            // reboot (WiFi maybe not fully stable yet) there is no prompt for an action that would fail anyway.
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
            // Back up/restore presets as a file
            chunk += "<h3>" + translate("Backup / Restore Presets") + "</h3>";
            chunk += "<a href='/exportpresets'><button type='button'>" + translate("Save Presets to File") + "</button></a> ";
            chunk += "<form method='POST' action='/importpresets' enctype='multipart/form-data' style='display:inline;'>";
            chunk += "<input type='file' name='presetfile' accept='.txt' required>";
            chunk += "<button type='submit'>" + translate("Load Presets from File") + "</button>";
            chunk += "</form> ";

            chunk += "</body></html>";
            webserver.sendContent(chunk);
            webserver.sendContent(""); // Ende der Chunked-Uebertragung signalisieren
                                       // signal the end of the chunked transfer
            });

        // Alle belegten Presets (Name + URL) als herunterladbare Textdatei
        // exportieren - eine Zeile pro Preset, Name und URL durch Tab getrennt.

        // Export all occupied presets (name + URL) as a downloadable text file -
        // one line per preset, name and URL separated by a tab.
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
                    // Replace the host part (this device's current IP) with a placeholder:
                    // the file should not look like a fixed address for ONE specific device.
                    // It gets replaced by the then-current IP on load/import anyway (see
                    // loadPresets() and savePresets() - both cut everything between
                    // "http://" and the next "/" and insert the current IP there, regardless of what was there before).
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

        // Restore presets from a text file previously created via /exportpresets
        // - replaces ALL currently stored presets.
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

        // Like /importpresets, but does not delete existing presets - used by the
        // GitHub download button on /presets (see handlePresetMergeUpload()).
        webserver.on("/importpresetsmerge", HTTP_POST, []() {
            redirectTo("/presets?msg=Presets%20imported%20successfully");
            }, handlePresetMergeUpload);

        // API zum Restart des ESP
        // API to restart the ESP
        webserver.on("/api/reboot", HTTP_GET, []() {
            webserver.send(200, "text/html", simpleMessagePage(translate("Rebooting..."), "", "<meta http-equiv='refresh' content='0; url=/status'>"));
            delay(WAIT_1s);
            espReboot();
            });


        // API zum Setzen eines Presets
        // API to set a preset
        webserver.on("/api/setPreset", HTTP_GET, []() {
            Serial.println("[API] Received request to /api/setPreset with args: " + webserver.arg("name"));
            if (!webserver.hasArg("name")) {
                webserver.send(400, "text/plain", "Missing 'name' parameter");
                return;
            }

            String presetName = webserver.arg("name");
            presetName.replace(" ", "_"); // Ersetze Leerzeichen durch Unterstriche
                                          // replace spaces with underscores
            // Suche das Preset mit dem angegebenen Namen
            // Find the preset with the given name
            for (int i = 0; i < MAX_PRESETS; i++) {
                if (presets[i].name.equalsIgnoreCase(presetName)) {
                    if (!presets[i].url.isEmpty()) {
                        // Redirect zur URL des Presets
                        // Redirect to the preset's URL
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
            // Preset not found
            webserver.send(404, "text/plain", "Preset not found");
            DEBUG_PRINTLN("[setPreset] Preset not found: " + presetName);
            });

        // NTP Server und Zeitzone setzen
        // Testet einen NTP-Server per direkter UDP-Anfrage, ohne die aktuelle
        // Zeitkonfiguration zu veraendern - fuer den "Test"-Button je Server-Feld.

        // Set NTP server and timezone.
        // Tests an NTP server via a direct UDP request without changing the
        // current time configuration - for the "Test" button next to each server field.
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
                                // index where the next valid NTP server address is written

            for (int readIndex = 0; readIndex < MAX_WLAN; readIndex++) {
                if (strlen(ntpServers[readIndex]) > 0) { // Nur nicht-leere Einträge berücksichtigen
                                                         // only consider non-empty entries
                    if (writeIndex != readIndex) {
                        strncpy(ntpServers[writeIndex], ntpServers[readIndex], sizeof(ntpServers[writeIndex]) - 1);
                        ntpServers[writeIndex][sizeof(ntpServers[writeIndex]) - 1] = '\0'; // Null-terminieren
                                                                                           // null-terminate
                        memset(ntpServers[readIndex], 0, sizeof(ntpServers[readIndex])); // Ursprünglichen Eintrag löschen
                                                                                         // clear the original entry
                    }
                    writeIndex++;
                }
            }

            // Leere Einträge am Ende sicherstellen
            // Ensure empty entries at the end
            for (int i = writeIndex; i < MAX_WLAN; i++) {
                memset(ntpServers[i], 0, sizeof(ntpServers[i]));
            }




            if (webserver.hasArg("timezone")) {
                String tz = webserver.arg("timezone");
                preferences.putString(PK_TIMEZONE, tz);
                setupNTP();
            }

            redirectTo("/?tab=zeit&msg=Timezone%20updated");
            }
        
        
            );


        // NTP Server und Zeitzone Formular
        // NTP server and timezone form
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
                                 // timezone form: long dropdown list
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
            // Combined select + input
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
        // Rename file form
        webserver.on("/rename_form", HTTP_GET, []() {
            if (!webserver.hasArg("file")) {
                webserver.send(400, "text/plain", "Missing file parameter");
                return;
            }

            String oldName = webserver.arg("file");
            String html = beginPage();
            html.reserve(1024);  // Umbenennen-Formular: klein
                                 // rename form: small
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
        // Rename file action
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
        // Scale BMP form
        webserver.on("/scalebmp_form", HTTP_GET, []() {
            if (!webserver.hasArg("file")) {
                webserver.send(400, "text/plain", "Missing file name");
                return;
            }
            String src = webserver.arg("file");
            String html = beginPage();
            html.reserve(1536);  // BMP-Skalieren-Formular: klein
                                 // scale-BMP form: small
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
        // Scale BMP action
        webserver.on("/scalebmp_run", HTTP_GET, []() {
            if (!webserver.hasArg("src") || !webserver.hasArg("dst") || !webserver.hasArg("w") || !webserver.hasArg("h")) {
                webserver.send(400, "text/plain", "Missing parameters");
                return;
            }

            String src = webserver.arg("src");
            String dst = webserver.arg("dst");
            int w = argToIntClamped("w", 0, 1, 1000);
            int h = argToIntClamped("h", 0, 1, 1000);

            bool scaleSuccess = scaleAndSaveBmp(src.c_str(), dst.c_str(), w, h);
            if (scaleSuccess) {
                webserver.send(200, "text/html", simpleMessagePage(translate("Scaling successful") + "!", "<p>" + translate("Saved as") + ": " + dst + "</p><a href='/files'><button type='button'>" + translate("Back") + "</button></a>"));
            }
            else {
                webserver.send(500, "text/html", simpleMessagePage(translate("Failed to scale BMP"), "<p>Check source file and format.</p><a href='/files'><button type='button'>" + translate("Back") + "</button></a>"));
            }
            });

        // Anzeigeeinstellungen speichern
        // Save display settings
        webserver.on("/applydisplaysettings", HTTP_POST, []() {
            // In den Preferences speichern
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
            // Save logging setting
            loggingEnabled = webserver.hasArg("loggingEnabled");
            preferences.putBool(PK_LOGGING_ENABLED, loggingEnabled);

            wifiActive = webserver.hasArg("wifiActive");
            preferences.putBool(PK_WIFI_ACTIVE, wifiActive);

            preferences.putBool(PK_STATION_MODE, stationMode);
            preferences.putBool(PK_SHOW_SECOND_HAND, showSecondHand);
            preferences.putBool(PK_SMOOTH_MINUTE, smoothMinute);

            if (webserver.hasArg("rotation")) {

                // Erst validieren, DANN die globale Variable setzen: vorher wurde
                // tftRotation1 unbedingt zugewiesen und blieb bei einem ungueltigen
                // Wert (z.B. rotation=9) zur Laufzeit stehen, obwohl nichts
                // gespeichert wurde. renderClockFrame() gibt sie jeden Tick weiter -
                // bei aktiver Software-Rotation landete sie in rotatedAngle()
                // (+9*90 Grad) und im switch von loadClockFace() (default = 270
                // Grad), Zifferblatt und Zeiger standen dann bis zum Reboot 180 Grad
                // gegeneinander verdreht. Zusaetzlich war ">= 0" bei einer uint8_t-
                // Variablen immer wahr, die Pruefung also halb wirkungslos.

                // Validate FIRST, then set the global variable: previously
                // tftRotation1 was assigned unconditionally and kept an invalid value
                // (e.g. rotation=9) at runtime even though nothing was saved.
                // renderClockFrame() passes it on every tick - with software rotation
                // active it ended up in rotatedAngle() (+9*90 degrees) and in
                // loadClockFace()'s switch (default = 270 degrees), leaving clock face
                // and hands 180 degrees apart until the next reboot. On top of that,
                // ">= 0" on a uint8_t variable was always true, making the check
                // half-useless.
                long requestedRotation = webserver.arg("rotation").toInt();
                if (requestedRotation >= 0 && requestedRotation <= 3) {
                    tftRotation1 = (uint8_t)requestedRotation;
                    uint8_t previousRotation = preferences.getUChar(PK_TFT_ROTATION1, 0);
                    preferences.putUChar(PK_TFT_ROTATION1, tftRotation1);
                    // firstRun nur bei TATSAECHLICHER Aenderung zuruecksetzen -
                    // sonst wuerde jedes Speichern dieser Seite (das Rotationsfeld
                    // wird immer mitgesendet, auch unveraendert) die Bahnhofsmodus-
                    // Wartephase am Sekundenzeiger unnoetig neu starten.

                    // Only reset firstRun on an ACTUAL change - otherwise every save of
                    // this page (rotation field is always submitted, even unchanged) would
                    // needlessly restart the station-mode wait phase on the second hand.
                    if (tftRotation1 != previousRotation) {
                        firstRun = true;
                    }
                    if (!gc9d01SwRotation) {
                        // Explizit auf Display 1 umschalten, bevor die Rotation gesetzt wird -
                        // tft.setRotation() wirkt nur auf den aktuell gewaehlten Chip.
                        // updateClock() laesst nach jedem Tick zwar bereits Display 1 selektiert
                        // zurueck, aber das hier nochmal sicherzustellen macht diesen Code
                        // unabhaengig davon.

                        // Explicitly switch to Display 1 before applying the rotation -
                        // tft.setRotation() only affects the currently selected chip.
                        // updateClock() already leaves Display 1 selected after every tick, but
                        // making sure of it here too keeps this code independent of that.
                        setCS1(LOW);
                        tft.setRotation(tftRotation1); // sofort anwenden
                                                      // apply immediately
                    }
                }

                freeClockFaceBuffer();
                loadClockFace();      // neu zeichnen mit neuer Ausrichtung
                                      // redraw with new orientation
                loadHandSprites();
            }

            // Rotation von Display 2 (CS2) - eigener, unabhaengiger Wert.
            // Rotation of Display 2 (CS2) - own, independent value.
            if (webserver.hasArg("rotation2")) {

                // Erst validieren, dann zuweisen - siehe ausfuehrlichen Kommentar
                // beim Rotationsblock von Display 1 weiter oben.
                // Validate first, then assign - see the detailed comment on the
                // Display 1 rotation block further above.
                long requestedRotation2 = webserver.arg("rotation2").toInt();
                if (requestedRotation2 >= 0 && requestedRotation2 <= 3) {
                    tftRotation2 = (uint8_t)requestedRotation2;
                    uint8_t previousRotation2 = preferences.getUChar(PK_TFT_ROTATION2, 0);
                    preferences.putUChar(PK_TFT_ROTATION2, tftRotation2);

                    // firstRun2 nur bei TATSAECHLICHER Aenderung setzen (wie bei Display 1)
                    // - sorgt dafuer, dass Display 2 sofort auf die neue Ausrichtung
                    // "springt" statt sich erst dorthin einzupendeln.

                    // Only set firstRun2 on an ACTUAL change (same as for Display 1) -
                    // makes Display 2 "snap" to the new orientation immediately instead
                    // of gradually easing into it.
                    if (tftRotation2 != previousRotation2) {
                        firstRun2 = true;
                    }

                    if (!gc9d01SwRotation) {
                        // Hardware-Rotation: das MADCTL-Register muss explizit auf dem
                        // Display-2-Chip gesetzt werden. Bei GC9D01-Software-Rotation
                        // (gc9d01SwRotation) ist hier NICHTS zu tun - renderClockFrame()
                        // liest tftRotation2 direkt und wendet es beim naechsten Tick an.

                        // Hardware rotation: the MADCTL register has to be explicitly set
                        // on the Display 2 chip. With GC9D01 software rotation
                        // (gc9d01SwRotation) there is NOTHING to do here - renderClockFrame()
                        // reads tftRotation2 directly and applies it on the next tick.
                        setCS2(LOW);
                        tft.setRotation(tftRotation2); // sofort auf Display 2 anwenden
                                                       // apply immediately to Display 2
                        setCS1(LOW); // zurueck auf Display 1, damit loop() im gewohnten Zustand weiterlaeuft
                                    // back to Display 1, so loop() continues from its usual state
                    }
                }
            }


            redirectTo("/?tab=zifferblatt&msg=Settings%20saved");
            });


        // Helligkeitseinstellungen Formular
        // Brightness settings form
        webserver.on("/brightness", HTTP_POST, []() {
            webserver.setContentLength(CONTENT_LENGTH_UNKNOWN);
            webserver.send(200, "text/html", "");

            String chunk = beginPage();
            chunk.reserve(1024);
            chunk += generateFlashMessage();
            chunk += "<h2>" + translate("Brightness Settings") + "</h2><form method = 'POST' action = '/save_brightness'><input type='hidden' name='returnTo' value='/brightness'>";

            chunk += "<div style='max-width:500px;margin:auto;text-align:left;border:1px solid #ccc;border-radius:8px;padding:12px 16px;'>";
            chunk += brightnessFormFieldsHtml();
            chunk += "</div>";


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
                chunk += "  document.querySelector(\"input[name='gamma']\").value = gamma.toFixed(1);\n";
                chunk += "  plotGamma(gamma);\n";
                chunk += "});\n\n";

                chunk += "plotGamma(" + String(gammaBrightness) + ");\n";
                chunk += "</script>\n";
#endif
            }

            chunk += "<br><br>";
            chunk += "</body></html>";
            webserver.sendContent(chunk);
            webserver.sendContent(""); // Ende der Chunked-Uebertragung signalisieren
                                       // signal the end of the chunked transfer
            });

        // Helligkeitseinstellungen Formular
        // Brightness settings form
        webserver.on("/brightness", HTTP_GET, []() {
            webserver.setContentLength(CONTENT_LENGTH_UNKNOWN);
            webserver.send(200, "text/html", "");

            String chunk = beginPage();
            chunk.reserve(1024);
            chunk += generateFlashMessage();
            chunk += "<h2>" + translate("Brightness Settings") + "</h2><form method = 'POST' action = '/save_brightness'><input type='hidden' name='returnTo' value='/brightness'>";

            chunk += "<div style='max-width:500px;margin:auto;text-align:left;border:1px solid #ccc;border-radius:8px;padding:12px 16px;'>";
            chunk += brightnessFormFieldsHtml();
            chunk += "</div>";


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
                chunk += "  document.querySelector(\"input[name='gamma']\").value = gamma.toFixed(1);\n";
                chunk += "  plotGamma(gamma);\n";
                chunk += "});\n\n";

                chunk += "plotGamma(" + String(gammaBrightness) + ");\n";
                chunk += "</script>\n";
#endif
            }

            chunk += "<br><br></body></html>";
            webserver.sendContent(chunk);
            webserver.sendContent(""); // Ende der Chunked-Uebertragung signalisieren
                                       // signal the end of the chunked transfer
            });

        // Helligkeitseinstellungen speichern
        // Save brightness settings
        webserver.on("/save_brightness", HTTP_POST, []() {
            useAdc = webserver.hasArg("use_adc");
            adcInverted = webserver.hasArg("adcInverted");
            lowThreshold = argToIntClamped("lowThreshold", lowThreshold, 0, 100);
            highThreshold = argToIntClamped("highThreshold", highThreshold, 0, 100);

            maxBrightness = (uint8_t)argToIntClamped("maxBrightness", maxBrightness, 0, 255);
            minBrightness = (uint8_t)argToIntClamped("minBrightness", minBrightness, 0, 255);

            // neue: Zeitabhängige Helligkeit speichern
            // new: save time-based brightness


            brightStartHour = (uint8_t)argToIntClamped("brightStart", brightStartHour, 0, 23);
            brightEndHour = (uint8_t)argToIntClamped("brightEnd", brightEndHour, 0, 23);

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

            // Zeitabhängige Einstellungen dauerhaft speichern
            // persist time-based settings

            preferences.putUChar(PK_BRIGHT_START_HOUR, brightStartHour);
            preferences.putUChar(PK_BRIGHT_END_HOUR, brightEndHour);

            // Nach dem Speichern zur Seite zurueckkehren, von der aus
            // abgeschickt wurde (Helligkeit-Tab auf "/" oder die eigenstaendige
            // /brightness-Seite) - statt immer fest zum Tab-Hub umzuleiten.
            // Das Formular der eigenstaendigen Seite sendet dafuer ein
            // verstecktes "returnTo"-Feld mit; das Tab-Panel sendet keines und
            // faellt daher weiterhin auf den Tab-Hub zurueck.

            // After saving, return to the page the form was submitted from
            // (the Brightness tab on "/" or the standalone /brightness page)
            // instead of always redirecting to the tab hub. The standalone
            // page's form sends a hidden "returnTo" field for this; the tab
            // panel doesn't send one and therefore still falls back to the
            // tab hub.
            String returnTo = webserver.hasArg("returnTo") ? webserver.arg("returnTo") : "/?tab=helligkeit";
            String sep = (returnTo.indexOf('?') >= 0) ? "&" : "?";
            redirectTo(returnTo + sep + "msg=Settings%20saved");
            });


        // Alle Dateien auflisten
        // List all files
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

            // First collect all filenames and sort them naturally (numbers in the
            // name sorted numerically instead of alphabetically, e.g. hand_set2
            // before hand_set10), before building the table from them.
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
                chunk += " <td><a href = '/delete?file=" + name + "' title='" + translate("Delete") + "' onclick = 'return confirm(\"" + translate("Delete") + " " + name + "?\")'>&#128465;&#65039;</a> ";
                // Scale-Option nur für .bmp-Dateien anzeigen
                // Show the scale option only for .bmp files
                if (name.endsWith(".bmp")) {
                    chunk += "<a href = '/scalebmp_form?file=" + name + "' title='" + translate("Scale") + "'>&#128208;</a> ";
                    chunk += "<a href='/rename_form?file=" + name + "' title='" + translate("Rename") + "'>&#9999;&#65039;</a> ";
                }
                else {
                    chunk += "<span style='opacity:0.25;' title='" + translate("Not applicable to this file type") + "'>&#128208;</span> ";
                    chunk += "<span style='opacity:0.25;' title='" + translate("Not applicable to this file type") + "'>&#9999;&#65039;</span> ";
                }
                       
                chunk += "<a href='/download?file=" + name + "' title='" + translate("Download") + "'>&#11015;&#65039;</a> ";
                chunk += "<a href='/file?name=" + name + "' title='" + translate("View") + "'>&#128065;&#65039;</a> "; // "View"-Link für Logdateien
                                                                                                                       // "View" link for log files

                chunk += "</td></tr>";

                // Alle paar Zeilen zwischendurch senden, damit der Puffer auch
                // bei sehr vielen Dateien nicht unbegrenzt waechst.

                // Send every few lines in between so the buffer does not grow
                // unbounded even with very many files.
                rowCount++;
                if (rowCount % 5 == 0) {
                    webserver.sendContent(chunk);
                    chunk = "";
                }
            }
            chunk += "</table><br><br>";
            chunk += "</body></html>";
            webserver.sendContent(chunk);
            webserver.sendContent(""); // Ende der Chunked-Uebertragung signalisieren
                                       // signal the end of the chunked transfer
            });

        webserver.on("/download", HTTP_GET, []() {
            if (webserver.hasArg("file")) {
                String path = webserver.arg("file");
                if (!path.startsWith("/")) path = "/" + path;

                if (LittleFS.exists(path)) {
                    // Pruefen, ob RLE-komprimierte face_*.bmp - falls ja, vor dem Download
                    // zu einem echten Standard-BMP dekodieren (nutzbar ausserhalb der Uhr).

                    // Check whether it is an RLE-compressed face_*.bmp - if so, decode it to
                    // a real standard BMP before download (usable outside the clock).
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

                        // Streaming failed (e.g. read error) - headers may already have been
                        // sent, so a clean 500 status is no longer possible.
                        DEBUG_PRINTLN("[DOWNLOAD] RLE streaming failed for " + path);
                        return;
                    }

                    File file = LittleFS.open(path, "r");

                    // Setze den Content-Disposition-Header, um den Dateinamen festzulegen
                    // Set the Content-Disposition header to define the file name
                    webserver.sendHeader("Content-Disposition", "attachment; filename=\"" + String(file.name()) + "\"");
                    webserver.streamFile(file, "application/octet-stream");
                    file.close();
                    return;
                }
            }
            webserver.send(404, "text/plain", "File not found");
            });

        // Systemstatus Seite
        // System status page
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
                chunk += "<li>Last Reset Reason: " + resetReasonToString(esp_reset_reason()) + "</li>";
                chunk += "<li>RTC Status: " + rtcStatusToString(rtcOk) + "</li>";
                if (lastNtpSuccessMillis == 0) {
                    chunk += "<li>Last NTP Sync: never</li>";
                }
                else {
                    chunk += "<li>Last NTP Sync: " + formatDurationMs(millis() - lastNtpSuccessMillis) + " ago</li>";
                }

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
            chunk += "<li>Arduino Core Version: " ESP_ARDUINO_VERSION_STR "</li><br>";

            webserver.sendContent(chunk);
            chunk = "";

            chunk += "<li>Flash Size: " + String(ESP.getFlashChipSize() / 1024) + " KB</li>";
            chunk += "<li>Free Heap: " + String(ESP.getFreeHeap() / 1024) + " KB</li>";
            chunk += "<li>Max Allocatable Block: " + String(ESP.getMaxAllocHeap() / 1024) + " KB</li>";
            chunk += "<li>Min Free Heap (since boot): " + String(ESP.getMinFreeHeap() / 1024) + " KB</li>";
            chunk += "<li>Max Sketch Size: " + String(ESP.getFreeSketchSpace() / 1024) + " KB</li>";
            chunk += "<li>Sketch Size: " + String(ESP.getSketchSize() / 1024) + " KB</li>";
            chunk += "<li>Free Sketch Space: " + String((ESP.getFreeSketchSpace() / 1024) - (ESP.getSketchSize() / 1024)) + " KB</li><br>";

            webserver.sendContent(chunk);
            chunk = "";

            // Explizite Erkennung zusaetzlich zu Groesse/Frei: ohne PSRAM stehen
            // beide Werte auf 0, was sich nicht von "PSRAM da, aber voll"
            // unterscheiden laesst. Der Wert entscheidet beim GC9D01 ausserdem
            // darueber, ob die Software-Rotation aktiv ist (siehe "rotation mode"
            // weiter unten und gc9d01SwRotation in uhr3.ino).

            // Explicit detection in addition to size/free: without PSRAM both
            // values read 0, which is indistinguishable from "PSRAM present but
            // full". On the GC9D01 this value also decides whether software
            // rotation is active (see "rotation mode" further below and
            // gc9d01SwRotation in uhr3.ino).
            chunk += "<li>PSRam detected: " + String(psramFound() ? "yes" : "no") + "</li>";
            chunk += "<li>PSRam size: " + String(ESP.getPsramSize() / 1024) + " kB</li>";
            chunk += "<li>PSRam free: " + String(ESP.getFreePsram() / 1024) + " kB</li><br>";


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
            chunk += "<li>TFT_CS1 GPIO: " + String(CS_1) + " (Display 1)</li>"; // CS_1 = Display 1 (vormals TFT_CS, jetzt manuell angesteuert, siehe config.h)
                                                                     // CS_1 = display 1 (formerly TFT_CS, now driven manually, see config.h)
#if defined CS_2
            chunk += "<li>TFT_CS2 GPIO: " + String(CS_2) + " (Display 2)</li>";
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
                // Dynamically computed keys
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
            // Bugfix: getUInt() auf einen Key, der ueberall mit putLong() geschrieben wird -
            // NVS meldet dabei ESP_ERR_NVS_TYPE_MISMATCH und Preferences liefert
            // kommentarlos den Default zurueck. Die Statusseite zeigte deshalb nie die
            // echte Nabenfarbe. Ausserdem stimmte das Label nicht: gespeichert wird
            // 24-Bit RGB888, nicht RGB565.
            // Bugfix: getUInt() on a key that is written with putLong() everywhere -
            // NVS reports ESP_ERR_NVS_TYPE_MISMATCH and Preferences silently returns
            // the default. So the status page never showed the real hub color. The
            // label was wrong too: what is stored is 24-bit RGB888, not RGB565.
            chunk += "<li><b>centerColor (RGB888)</b>: " + String(preferences.getLong(PK_CENTER_COLOR, 0xEC0016), HEX) + "</li>";
            chunk += "<li><b>centerSize</b>: " + String(preferences.getUInt(PK_CENTER_SIZE, 6)) + "</li>";

            uint8_t rotation = preferences.getUChar(PK_TFT_ROTATION1, 0);
            const char* rotationLabels[] = { "0&deg;", "90&deg;", "180&deg;", "270&deg;" };
            chunk += "<li><b>tftRotation1</b>: " + String(rotationLabels[rotation]) + "</li>";
            // Direkt unter tftRotation1, damit beide Rotationswerte untereinander stehen -
            // Display 2 ist fest aktiviert, daher immer sichtbar.

            // Directly under tftRotation1, so both rotation values are listed one below
            // the other - Display 2 is permanently enabled, so this is always shown.
            {
                uint8_t rotation2 = preferences.getUChar(PK_TFT_ROTATION2, 0);
                chunk += "<li><b>tftRotation2</b>: " + String(rotationLabels[rotation2]) + "</li>";
            }

            // Rotationsmodus: macht sichtbar, WIE die beiden Rotationswerte oben
            // ueberhaupt angewendet werden - das war von aussen bisher nicht
            // erkennbar. Beim GC9D01 wird der GC9A01-Treiber wiederverwendet,
            // dessen Hardware-Rotation dort wirkungslos bleibt; nur mit genuegend
            // PSRAM wird deshalb auf Software-Rotation umgeschaltet (siehe
            // gc9d01SwRotation in uhr3.ino sowie rotatedAngle()/loadClockFace()
            // und beginStatusDraw()/endStatusDraw() in display.h).

            // Rotation mode: makes visible HOW the two rotation values above are
            // actually applied - which could not be told from the outside before.
            // On the GC9D01 the GC9A01 driver is reused, whose hardware rotation
            // has no effect there; only with enough PSRAM does it switch to
            // software rotation (see gc9d01SwRotation in uhr3.ino as well as
            // rotatedAngle()/loadClockFace() and beginStatusDraw()/
            // endStatusDraw() in display.h).
            chunk += "<li><b>rotation mode</b>: ";
            if (gc9d01SwRotation) {
                chunk += "software (pixel remap, GC9D01 with PSRAM)";
            }
            else {
                chunk += "hardware (display MADCTL register)";
#ifdef GC9D01
                chunk += " - <b>ineffective on GC9D01</b>, software rotation needs PSRAM";
#endif
            }
            chunk += "</li>";

            webserver.sendContent(chunk);
            chunk = "";

            // Booleans als Text
            // Booleans as text
        
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
            chunk += "</body></html>";

            webserver.sendContent(chunk);
            webserver.sendContent(""); // Ende der Chunked-Uebertragung signalisieren
                                       // signal the end of the chunked transfer
            });

        // Kleine Vorschau (80x80) fuer hochgeladene Zifferblaetter - siehe
        // sendScaledBmpPreview() in display.h. Vermeidet, dass fuer ein
        // 80x80-<img> die volle Aufloesung (z.B. 240x240 = ~115 KB) uebertragen wird.

        // Small preview (80x80) for uploaded clock faces - see
        // sendScaledBmpPreview() in display.h. Avoids transferring the full
        // resolution (e.g. 240x240 = ~115 KB) for an 80x80 <img>.
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
        // Liefert nur das aktuell aktive Zifferblatt (ohne Zeiger) als BMP - fuer
        // das Live-Zeiger-Widget auf der Startseite als Hintergrundbild, damit
        // nicht bei jedem Seitenaufruf ~150 KB Bilddaten inline eingebettet werden
        // muessen (wie es bei einem direkt einkodierten Base64-String der Fall
        // waere). clockFaceBuffer enthaelt die rohen Zifferblatt-Pixel ohne
        // Helligkeitsanpassung und ohne eingezeichnete Zeiger.
        // Liefert die tatsaechliche, aktuelle Uhrzeit des ESP32 (nicht die des
        // PCs/Browsers) - wird vom Live-Zeiger-Widget einmalig beim Laden der
        // Seite abgerufen, danach laeuft die Anzeige lokal im Browser weiter
        // (per performance.now()-Differenz), damit kein staendiges Nachfragen
        // noetig ist. So zeigt die Web-Vorschau exakt das, was die echte Uhr
        // gerade anzeigt, auch wenn PC und ESP32 unterschiedlich gehen sollten.

        // Generates a preview image from the CURRENTLY ACTIVE settings (no
        // parameter needed) - shown top-left on every page (generateNavigation())
        // so it reflects the current state after every change once the page reloads.
        // Returns only the currently active clock face (without hands) as a BMP -
        // used as the background image for the live hand widget on the home page,
        // so ~150 KB of image data does not have to be embedded inline on every
        // page load (as would happen with a directly encoded base64 string).
        // clockFaceBuffer holds the raw face pixels without brightness adjustment
        // and without drawn-in hands.
        // Returns the ESP32's actual current time (not the PC's/browser's) -
        // fetched once by the live hand widget when the page loads; afterwards
        // the display keeps running locally in the browser (via performance.now()
        // differences), so no constant polling is needed. This way the web preview
        // shows exactly what the real clock is currently displaying, even if the
        // PC and ESP32 clocks should drift apart.
        webserver.on("/api/currentTime", HTTP_GET, []() {
            webserver.sendHeader("Cache-Control", "no-store");
            String json = "{\"hour\":" + String(timeinfo.tm_hour) +
                          ",\"minute\":" + String(timeinfo.tm_min) +
                          ",\"second\":" + String(timeinfo.tm_sec) + "}";
            webserver.send(200, "application/json", json);
            });

        // Zeigt die Live-Zeiger-Uhr in voller Aufloesung (400x400) als eigene
        // Seite - nutzt dieselbe Zeiger-Lade-/Kodierlogik wie das kleine Eck-
        // Widget in generateNavigation(), nur groesser skaliert.

        // Shows the live hand clock at full resolution (400x400) as its own
        // page - uses the same hand loading/encoding logic as the small corner
        // widget in generateNavigation(), just scaled up.
        webserver.on("/preview", HTTP_GET, []() {
            webserver.setContentLength(CONTENT_LENGTH_UNKNOWN);
            webserver.send(200, "text/html", "");

            String chunk = beginPage();
            chunk.reserve(2048);
            chunk += "<h2>" + translate("Preview") + "</h2>";

            const int previewSize = 400;

            String activeHandSet = preferences.getString(PK_HANDSET, "");
            uint16_t* previewHour = nullptr;
            uint16_t* previewMinute = nullptr;
            uint16_t* previewSecond = nullptr;
            const uint16_t* hourSrc = handHour;
            const uint16_t* minuteSrc = handMinute;
            const uint16_t* secondSrc = handSecond;
            const size_t handPixelCount = (size_t)HAND_WIDTH * HAND_HEIGHT;

            if (activeHandSet != "" && activeHandSet != "default") {
                String hourPath = "/hand_set" + activeHandSet + "_hour.bmp";
                String minutePath = "/hand_set" + activeHandSet + "_minute.bmp";
                String secondPath = "/hand_set" + activeHandSet + "_second.bmp";

                if (LittleFS.exists(hourPath)) {
                    previewHour = (uint16_t*)malloc(handPixelCount * 2);
                    if (previewHour && loadHandPixelsForPreview(hourPath.c_str(), previewHour, HAND_WIDTH, HAND_HEIGHT)) {
                        hourSrc = previewHour;
                    }
                }
                if (LittleFS.exists(minutePath)) {
                    previewMinute = (uint16_t*)malloc(handPixelCount * 2);
                    if (previewMinute && loadHandPixelsForPreview(minutePath.c_str(), previewMinute, HAND_WIDTH, HAND_HEIGHT)) {
                        minuteSrc = previewMinute;
                    }
                }
                if (LittleFS.exists(secondPath)) {
                    previewSecond = (uint16_t*)malloc(handPixelCount * 2);
                    if (previewSecond && loadHandPixelsForPreview(secondPath.c_str(), previewSecond, HAND_WIDTH, HAND_HEIGHT)) {
                        secondSrc = previewSecond;
                    }
                }
            }

            String hourB64 = encodePngToBase64(hourSrc, HAND_WIDTH, HAND_HEIGHT);
            String minuteB64 = encodePngToBase64(minuteSrc, HAND_WIDTH, HAND_HEIGHT);
            String secondB64 = encodePngToBase64(secondSrc, HAND_WIDTH, HAND_HEIGHT);

            if (previewHour) free(previewHour);
            if (previewMinute) free(previewMinute);
            if (previewSecond) free(previewSecond);

            uint8_t hubR = ((hubColor >> 11) & 0x1F) * 255 / 31;
            uint8_t hubG = ((hubColor >> 5) & 0x3F) * 255 / 63;
            uint8_t hubB = (hubColor & 0x1F) * 255 / 31;
            char hubHex[8];
            snprintf(hubHex, sizeof(hubHex), "#%02x%02x%02x", hubR, hubG, hubB);

            bool showSecond = preferences.getBool(PK_SHOW_SECOND_HAND, true);
            bool stationModeActive = preferences.getBool(PK_STATION_MODE, true);
            bool smoothMinuteActive = preferences.getBool(PK_SMOOTH_MINUTE, true);

            float scaleFactor = (float)previewSize / CLOCK_WIDTH;
            int scaledHandWidth = (int)(HAND_WIDTH * scaleFactor + 0.5);
            int scaledHandHeight = (int)(HAND_HEIGHT * scaleFactor + 0.5);
            int scaledPivotX = (int)((HAND_WIDTH / 2.0) * scaleFactor + 0.5);
            int scaledPivotY = (int)((HAND_HEIGHT * 0.77) * scaleFactor + 0.5);
            // hubSize ist ein RADIUS (wie bei fillCircle() auf dem echten Display,
            // siehe display.h, und bei generatePresetPreviewBmp()) - fuer den CSS-
            // Kreis unten wird aber der DURCHMESSER (width/height) gebraucht, daher
            // hier verdoppeln. Vorher fehlte die Verdopplung, wodurch der Punkt in
            // dieser Live-Vorschau nur halb so gross wie auf dem echten Display war.

            // hubSize is a RADIUS (like fillCircle() on the real display, see
            // display.h, and generatePresetPreviewBmp()) - but the CSS circle
            // below needs the DIAMETER (width/height), hence doubled here.
            // Previously missing this doubling made the hub in this live
            // preview only half as large as on the real display.
            int scaledHubSize = (int)(hubSize * 2 * scaleFactor + 0.5);
            if (scaledHubSize < 4) scaledHubSize = 4;

            chunk += "<div style='width:" + String(previewSize) + "px;height:" + String(previewSize) + "px;margin:20px auto;box-sizing:border-box;border:3px solid #333;border-radius:50%;background:#fff url(/currentfacebg) center/cover no-repeat;overflow:hidden;position:relative;'>";
            chunk += "<div id='liveHandsPivotFull' style='position:absolute;left:50%;top:50%;width:0;height:0;'>";
            chunk += "<img id='liveHourHandFull' src='data:image/png;base64," + hourB64 + "' style='position:absolute;left:-" + String(scaledPivotX) + "px;top:-" + String(scaledPivotY) + "px;width:" + String(scaledHandWidth) + "px;height:" + String(scaledHandHeight) + "px;transform-origin:" + String(scaledPivotX) + "px " + String(scaledPivotY) + "px;'>";
            chunk += "<img id='liveMinuteHandFull' src='data:image/png;base64," + minuteB64 + "' style='position:absolute;left:-" + String(scaledPivotX) + "px;top:-" + String(scaledPivotY) + "px;width:" + String(scaledHandWidth) + "px;height:" + String(scaledHandHeight) + "px;transform-origin:" + String(scaledPivotX) + "px " + String(scaledPivotY) + "px;'>";
            if (showSecond) {
                chunk += "<img id='liveSecondHandFull' src='data:image/png;base64," + secondB64 + "' style='position:absolute;left:-" + String(scaledPivotX) + "px;top:-" + String(scaledPivotY) + "px;width:" + String(scaledHandWidth) + "px;height:" + String(scaledHandHeight) + "px;transform-origin:" + String(scaledPivotX) + "px " + String(scaledPivotY) + "px;'>";
            }
            chunk += "<div style='position:absolute;left:-" + String(scaledHubSize / 2) + "px;top:-" + String(scaledHubSize / 2) + "px;width:" + String(scaledHubSize) + "px;height:" + String(scaledHubSize) + "px;border-radius:50%;background:" + String(hubHex) + ";'></div>";
            chunk += "</div></div>";

            chunk += "<script>";
            chunk += "(function() {";
            chunk += "  var hourEl = document.getElementById('liveHourHandFull');";
            chunk += "  var minuteEl = document.getElementById('liveMinuteHandFull');";
            chunk += "  var secondEl = document.getElementById('liveSecondHandFull');";
            chunk += "  var stationMode = " + String(stationModeActive ? "true" : "false") + ";";
            chunk += "  var smoothMinute = " + String(smoothMinuteActive ? "true" : "false") + ";";
            chunk += "  var fastSecondMs = " + String((int)FAST_SECOND) + ";";
            chunk += "  var baseH = 0, baseM = 0, baseS = 0, baseAt = 0, haveBase = false;";
            chunk += "  fetch('/api/currentTime').then(function(r) { return r.json(); }).then(function(t) {";
            chunk += "    baseH = t.hour; baseM = t.minute; baseS = t.second; baseAt = performance.now(); haveBase = true;";
            chunk += "  }).catch(function() {});";
            chunk += "  function tick() {";
            chunk += "    var h, m, s, ms;";
            chunk += "    if (haveBase) {";
            chunk += "      var elapsed = (performance.now() - baseAt) / 1000;";
            chunk += "      var totalSec = baseH * 3600 + baseM * 60 + baseS + elapsed;";
            chunk += "      h = Math.floor(totalSec / 3600) % 12;";
            chunk += "      m = Math.floor(totalSec / 60) % 60;";
            chunk += "      s = Math.floor(totalSec) % 60;";
            chunk += "      ms = (totalSec - Math.floor(totalSec)) * 1000;";
            chunk += "    } else {";
            chunk += "      var now = new Date();";
            chunk += "      h = now.getHours() % 12; m = now.getMinutes(); s = now.getSeconds(); ms = now.getMilliseconds();";
            chunk += "    }";
            chunk += "    var minuteDeg = smoothMinute ? (m + s / 60) * 6 : m * 6;";
            chunk += "    var hourDeg = (h + minuteDeg / 360) * 30;";
            chunk += "    var secDeg;";
            chunk += "    if (stationMode) {";
            chunk += "      var elapsedMs = (s + ms / 1000) * 1000;";
            chunk += "      var tickIndex = Math.floor(elapsedMs / fastSecondMs);";
            chunk += "      var subTick = (elapsedMs % fastSecondMs) / fastSecondMs;";
            // Wie in der Navigations-Vorschau weiter oben und in
            // renderClockFrame() (display.h).
            // As in the navigation preview above and in renderClockFrame()
            // (display.h).
            chunk += "      var eased = -(Math.cos(Math.PI * Math.pow(subTick, 0.5)) - 1) / 2;";
            chunk += "      var smoothSec = Math.min(tickIndex + eased, 60);";
            chunk += "      secDeg = smoothSec * 6;";
            chunk += "    } else {";
            chunk += "      secDeg = s * 6;";
            chunk += "    }";
            chunk += "    hourEl.style.transform = 'rotate(' + hourDeg + 'deg)';";
            chunk += "    minuteEl.style.transform = 'rotate(' + minuteDeg + 'deg)';";
            chunk += "    if (secondEl) secondEl.style.transform = 'rotate(' + secDeg + 'deg)';";
            chunk += "    requestAnimationFrame(tick);";
            chunk += "  }";
            chunk += "  requestAnimationFrame(tick);";
            chunk += "})();";
            chunk += "</script>";

            chunk += "</body></html>";
            webserver.sendContent(chunk);
            webserver.sendContent("");
            });

        webserver.on("/currentfacebg", HTTP_GET, []() {
            if (!clockFaceBuffer) {
                webserver.send(500, "text/plain", "Face not loaded");
                return;
            }
            size_t bmpSize = 0;
            uint8_t* bmpBytes = encodeBmpToBytes(clockFaceBuffer, CLOCK_WIDTH, CLOCK_HEIGHT, &bmpSize);
            if (!bmpBytes) {
                webserver.send(500, "text/plain", "Failed to generate face background (out of memory?)");
                return;
            }
            webserver.sendHeader("Cache-Control", "no-store");
            webserver.send_P(200, "image/bmp", (const char*)bmpBytes, bmpSize);
            delete[] bmpBytes;
            });

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

        // Preview image for preset management: clock face + hands (fixed demo
        // time) + hub color/size, composed from the settings stored in the
        // preset (see parsePresetForPreview() and generatePresetPreviewBmp()).
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

            // Small preview instead of full resolution (saves ~90% transfer size
            // for an 80x80 <img> - see sendScaledBmpPreview() for the equivalent
            // approach used for uploaded clock faces).
            const int outW = 80;
            const int outH = 80;
            const float scaleX = (float)CLOCK_WIDTH / outW;
            const float scaleY = (float)CLOCK_HEIGHT / outH;

            const int headerSize = 54;
            const int rowSize = ((outW * 3 + 3) / 4) * 4; // 3 Bytes pro Pixel für RGB888
                                                          // 3 bytes per pixel for RGB888
            const int dataSize = rowSize * outH;
            const int fileSize = headerSize + dataSize;

            // Null-Check ergaenzt: ~19 KB Allokation, direkt gefolgt von memset und
            // den Header-Schreibzugriffen. Ohne Pruefung endete eine fehlgeschlagene
            // Allokation (der ESP32-S2-Heap ist knapp, und /listfilesFaces laedt
            // mehrere Vorschauen parallel) in einem Panic-Reset statt in einer
            // sauberen Fehlerantwort - die Nachbar-Handler machen das bereits richtig.
            // Null check added: ~19 KB allocation, immediately followed by memset and
            // the header writes. Without the check, a failed allocation (the ESP32-S2
            // heap is tight, and /listfilesFaces loads several previews in parallel)
            // ended in a panic reset instead of a clean error response - the
            // neighbouring handlers already do this correctly.
            uint8_t* bmpData = new (std::nothrow) uint8_t[fileSize];
            if (!bmpData) {
                DEBUG_PRINTLN("[Preview] Error: couldnt allocate preview buffer for /preview_defaultface");
                webserver.send(500, "text/plain", "Out of memory");
                return;
            }
            memset(bmpData, 0, fileSize);

            // BMP-Header
            // BMP header
            bmpData[0] = 'B'; bmpData[1] = 'M';
            *(uint32_t*)&bmpData[2] = fileSize;
            *(uint32_t*)&bmpData[10] = headerSize;
            *(uint32_t*)&bmpData[14] = 40;
            *(int32_t*)&bmpData[18] = outW;
            *(int32_t*)&bmpData[22] = -outH; // Top-down BMP
                                             // top-down BMP
            *(uint16_t*)&bmpData[26] = 1;
            *(uint16_t*)&bmpData[28] = 24; // 24-Bit Farbtiefe
                                           // 24-bit color depth
            *(uint32_t*)&bmpData[34] = dataSize;

            // Pixel-Daten (RGB565 -> RGB888, mit Downscaling)
            // Pixel data (RGB565 -> RGB888, with downscaling)
            for (int y = 0; y < outH; y++) {
                int srcY = int(y * scaleY);
                uint8_t* rowPtr = bmpData + headerSize + y * rowSize;
                for (int x = 0; x < outW; x++) {
                    int srcX = int(x * scaleX);
                    uint16_t px = clockFace[srcY * CLOCK_WIDTH + srcX];

                    // Transparente Farbe ersetzen
                    // Replace the transparent color
                    if (px == TRANSPARENT_COLOR) {
                        rowPtr[x * 3 + 0] = 255; // Blau
                                                 // blue
                        rowPtr[x * 3 + 1] = 255; // Grün
                                                 // green
                        rowPtr[x * 3 + 2] = 255; // Rot
                                                 // red
                        continue;
                    }

                    // RGB565 ? RGB888
                    // RGB565 to RGB888
                    uint8_t r = (px >> 8) & 0xF8; // obere 5 Bits
                                                  // upper 5 bits
                    uint8_t g = (px >> 3) & 0xFC; // mittlere 6 Bits
                                                  // middle 6 bits
                    uint8_t b = (px << 3) & 0xF8; // untere 5 Bits
                                                  // lower 5 bits

                    rowPtr[x * 3 + 0] = b; // Blau
                                           // blue
                    rowPtr[x * 3 + 1] = g; // Grün
                                           // green
                    rowPtr[x * 3 + 2] = r; // Rot
                                           // red
                }
            }

            webserver.send_P(200, "image/bmp", (const char*)bmpData, fileSize);
            delete[] bmpData;
            });

        // Uhr-Gesichter verwalten
        // Manage clock faces
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

            // Eingebautes Standard-Zifferblatt hinzufuegen
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

            // First collect and sort all matching clock-face filenames (the built-in
            // default above is unaffected by this, since it was already output
            // separately and fixed in first place).
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

                // Send every few entries in between so the buffer does not grow
                // unbounded even with many clock faces.
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

            // Automatic download of new clock faces directly in the browser: the
            // browser downloads the files via HTTPS (via CORS, officially supported
            // by GitHub for api.github.com and raw.githubusercontent.com) and then
            // uploads them via normal local HTTP through /upload - the clock itself never needs to open an HTTPS connection.
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
            // Notice and download link for the ZIP file
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


            chunk += "</body> </html>";
            webserver.sendContent(chunk);
            webserver.sendContent(""); // Ende der Chunked-Uebertragung signalisieren
                                       // signal the end of the chunked transfer
            });

        // WLAN Netzwerke scannen
        // Scan WiFi networks
        webserver.on("/api/scanwifi", HTTP_GET, []() {
            String json = "";
               
            // die letzten Scan-Ergebnisse zurückgeben
            // Return the last scan results
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
        // Main page - WiFi settings
        webserver.on("/", HTTP_GET, []() {

            webserver.setContentLength(CONTENT_LENGTH_UNKNOWN);
            webserver.send(200, "text/html", "");

            String chunk = beginPage();
            chunk.reserve(2048);
            chunk += generateFlashMessage(); // Erfolgsmeldung, falls vorhanden
                                             // success message, if present

            chunk += generateLanguageSelector();

            bool apMode = (WiFi.getMode() != WIFI_STA);

            // --- CSS-only Tabs: die 5 radio-Inputs (siehe Tab-CSS in
            // generateHtmlHeader()) - muessen direkte Geschwister von .tabnav
            // und allen .panel-* Divs weiter unten sein. Reihenfolge der
            // Labels weiter unten bestimmt die sichtbare Tab-Reihenfolge -
            // Status bewusst ganz nach rechts (ans Ende), die anderen zuerst.
            // Beim Aufruf wird immer der erste Tab (WLAN) vorausgewaehlt,
            // unabhaengig vom Verbindungsstatus - das deckt automatisch auch
            // das Captive-Portal-Popup ab (das auf "/" verweist, siehe
            // captivePortalRedirect oben).

            // --- CSS-only tabs: the 5 radio inputs (see tab CSS in
            // generateHtmlHeader()) - must be direct siblings of .tabnav and
            // all .panel-* divs further below. Order of the labels below
            // determines the visible tab order - Status is deliberately
            // moved to the far right (last), the others come first. The
            // first tab (WiFi) is always preselected on load, regardless of
            // connection status - this also automatically covers the
            // captive portal popup (which points to "/", see
            // captivePortalRedirect above).
            chunk += "<input type='radio' name='tabs' id='tab-wlan' class='tabctrl' checked>";
            chunk += "<input type='radio' name='tabs' id='tab-zifferblatt' class='tabctrl'>";
            chunk += "<input type='radio' name='tabs' id='tab-helligkeit' class='tabctrl'>";
            chunk += "<input type='radio' name='tabs' id='tab-zeit' class='tabctrl'>";
            chunk += "<input type='radio' name='tabs' id='tab-status' class='tabctrl'>";

            chunk += "<div class='tabnav'>";
            chunk += "<label for='tab-wlan'>" + translate("WiFi Settings") + "</label>";
            chunk += "<label for='tab-zifferblatt'>" + translate("Clock Setup") + "</label>";
            chunk += "<label for='tab-helligkeit'>" + translate("Brightness") + "</label>";
            chunk += "<label for='tab-zeit'>" + translate("NTP&nbsp;Timezone") + "</label>";
            chunk += "<label for='tab-status'>" + translate("Status") + "</label>";
            chunk += "</div>";

            webserver.sendContent(chunk);
            chunk = "";

            // #####################################################################
            // Panel: Status (read-only Systeminfos - unveraendert aus der
            // bisherigen eigenstaendigen /status-Seite uebernommen)
            // #####################################################################

            // #####################################################################
            // Panel: status (read-only system info - identisch zur bisherigen
            // eigenstaendigen /status-Seite, inklusive "Actual Preferences" -
            // es gibt bewusst nur noch EINE Status-Ansicht mit vollem Umfang,
            // nicht mehr eine gekuerzte Tab-Version plus separate volle Seite.

            // Panel: status (read-only system info - identical to the previous
            // standalone /status page, including "Actual Preferences" -
            // deliberately only ONE full-detail status view now, not a
            // shortened tab version plus a separate full page)
            // #####################################################################
            chunk += "<div class='tabpanel panel-status'>";
            chunk += "<ul>";

            String tzLabel = preferences.getString(PK_TIMEZONE, "DE");
            String tzDesc = tzLabel;

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
                chunk += "<li>Last Reset Reason: " + resetReasonToString(esp_reset_reason()) + "</li>";
                chunk += "<li>RTC Status: " + rtcStatusToString(rtcOk) + "</li>";
                if (lastNtpSuccessMillis == 0) {
                    chunk += "<li>Last NTP Sync: never</li>";
                }
                else {
                    chunk += "<li>Last NTP Sync: " + formatDurationMs(millis() - lastNtpSuccessMillis) + " ago</li>";
                }
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
            chunk += "<li>Arduino Core Version: " ESP_ARDUINO_VERSION_STR "</li><br>";

            webserver.sendContent(chunk);
            chunk = "";

            chunk += "<li>Flash Size: " + String(ESP.getFlashChipSize() / 1024) + " KB</li>";
            chunk += "<li>Free Heap: " + String(ESP.getFreeHeap() / 1024) + " KB</li>";
            chunk += "<li>Max Allocatable Block: " + String(ESP.getMaxAllocHeap() / 1024) + " KB</li>";
            chunk += "<li>Min Free Heap (since boot): " + String(ESP.getMinFreeHeap() / 1024) + " KB</li>";
            chunk += "<li>Max Sketch Size: " + String(ESP.getFreeSketchSpace() / 1024) + " KB</li>";
            chunk += "<li>Sketch Size: " + String(ESP.getSketchSize() / 1024) + " KB</li>";
            chunk += "<li>Free Sketch Space: " + String((ESP.getFreeSketchSpace() / 1024) - (ESP.getSketchSize() / 1024)) + " KB</li><br>";

            webserver.sendContent(chunk);
            chunk = "";

            // Explizite Erkennung zusaetzlich zu Groesse/Frei: ohne PSRAM stehen
            // beide Werte auf 0, was sich nicht von "PSRAM da, aber voll"
            // unterscheiden laesst. Der Wert entscheidet beim GC9D01 ausserdem
            // darueber, ob die Software-Rotation aktiv ist (siehe "rotation mode"
            // weiter unten und gc9d01SwRotation in uhr3.ino).

            // Explicit detection in addition to size/free: without PSRAM both
            // values read 0, which is indistinguishable from "PSRAM present but
            // full". On the GC9D01 this value also decides whether software
            // rotation is active (see "rotation mode" further below and
            // gc9d01SwRotation in uhr3.ino).
            chunk += "<li>PSRam detected: " + String(psramFound() ? "yes" : "no") + "</li>";
            chunk += "<li>PSRam size: " + String(ESP.getPsramSize() / 1024) + " kB</li>";
            chunk += "<li>PSRam free: " + String(ESP.getFreePsram() / 1024) + " kB</li><br>";
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

            chunk += "<li>TFT_SCLK GPIO: " + String(TFT_SCLK) + "</li>";
            chunk += "<li>TFT_MOSI GPIO: " + String(TFT_MOSI) + "</li>";
            chunk += "<li>TFT_CS1 GPIO: " + String(CS_1) + " (Display 1)</li>"; // CS_1 = Display 1 (vormals TFT_CS, jetzt manuell angesteuert, siehe config.h)
                                                                     // CS_1 = display 1 (formerly TFT_CS, now driven manually, see config.h)
#if defined CS_2
            chunk += "<li>TFT_CS2 GPIO: " + String(CS_2) + " (Display 2)</li>";
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
                // Dynamically computed keys
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
            // Bugfix: getUInt() auf einen Key, der ueberall mit putLong() geschrieben wird -
            // NVS meldet dabei ESP_ERR_NVS_TYPE_MISMATCH und Preferences liefert
            // kommentarlos den Default zurueck. Die Statusseite zeigte deshalb nie die
            // echte Nabenfarbe. Ausserdem stimmte das Label nicht: gespeichert wird
            // 24-Bit RGB888, nicht RGB565.
            // Bugfix: getUInt() on a key that is written with putLong() everywhere -
            // NVS reports ESP_ERR_NVS_TYPE_MISMATCH and Preferences silently returns
            // the default. So the status page never showed the real hub color. The
            // label was wrong too: what is stored is 24-bit RGB888, not RGB565.
            chunk += "<li><b>centerColor (RGB888)</b>: " + String(preferences.getLong(PK_CENTER_COLOR, 0xEC0016), HEX) + "</li>";
            chunk += "<li><b>centerSize</b>: " + String(preferences.getUInt(PK_CENTER_SIZE, 6)) + "</li>";

            uint8_t rotation = preferences.getUChar(PK_TFT_ROTATION1, 0);
            const char* statusRotationLabels[] = { "0&deg;", "90&deg;", "180&deg;", "270&deg;" };
            chunk += "<li><b>tftRotation1</b>: " + String(statusRotationLabels[rotation]) + "</li>";
            // Direkt unter tftRotation1, damit beide Rotationswerte untereinander stehen -
            // Display 2 ist fest aktiviert, daher immer sichtbar.

            // Directly under tftRotation1, so both rotation values are listed one below
            // the other - Display 2 is permanently enabled, so this is always shown.
            {
                uint8_t rotation2Panel = preferences.getUChar(PK_TFT_ROTATION2, 0);
                chunk += "<li><b>tftRotation2</b>: " + String(statusRotationLabels[rotation2Panel]) + "</li>";
            }

            // Rotationsmodus: macht sichtbar, WIE die beiden Rotationswerte oben
            // ueberhaupt angewendet werden - das war von aussen bisher nicht
            // erkennbar. Beim GC9D01 wird der GC9A01-Treiber wiederverwendet,
            // dessen Hardware-Rotation dort wirkungslos bleibt; nur mit genuegend
            // PSRAM wird deshalb auf Software-Rotation umgeschaltet (siehe
            // gc9d01SwRotation in uhr3.ino sowie rotatedAngle()/loadClockFace()
            // und beginStatusDraw()/endStatusDraw() in display.h).

            // Rotation mode: makes visible HOW the two rotation values above are
            // actually applied - which could not be told from the outside before.
            // On the GC9D01 the GC9A01 driver is reused, whose hardware rotation
            // has no effect there; only with enough PSRAM does it switch to
            // software rotation (see gc9d01SwRotation in uhr3.ino as well as
            // rotatedAngle()/loadClockFace() and beginStatusDraw()/
            // endStatusDraw() in display.h).
            chunk += "<li><b>rotation mode</b>: ";
            if (gc9d01SwRotation) {
                chunk += "software (pixel remap, GC9D01 with PSRAM)";
            }
            else {
                chunk += "hardware (display MADCTL register)";
#ifdef GC9D01
                chunk += " - <b>ineffective on GC9D01</b>, software rotation needs PSRAM";
#endif
            }
            chunk += "</li>";

            webserver.sendContent(chunk);
            chunk = "";

            // Booleans als Text
            // Booleans as text

            chunk += "<li><b>stationMode</b>: " + String(preferences.getBool(PK_STATION_MODE, true) ? "true" : "false") + "</li>";
            chunk += "<li><b>showSecondhand</b>: " + String(preferences.getBool(PK_SHOW_SECOND_HAND, true) ? "true" : "false") + "</li>";
            chunk += "<li><b>smoothMinute</b>: " + String(preferences.getBool(PK_SMOOTH_MINUTE, false) ? "true" : "false") + "</li>";

            chunk += "<li><b>minBrightness</b>: " + String(preferences.getUChar(PK_MIN_BRIGHTNESS, 100)) + "</li>";
            chunk += "<li><b>maxBrightness</b>: " + String(preferences.getUChar(PK_MAX_BRIGHTNESS, 255)) + "</li>";

            uint16_t brightEndPanel = preferences.getUChar(PK_BRIGHT_END_HOUR, 20);
            brightEndPanel += 1;
            if (brightEndPanel > 23) brightEndPanel = 0;

            chunk += "<li><b>daywindow</b>: " + String(preferences.getUChar(PK_BRIGHT_START_HOUR, 8)) + ":00 - " + String(brightEndPanel) + ":00</li>";

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
            chunk += "</div>"; // Ende panel-status
                               // end panel-status

            webserver.sendContent(chunk);
            chunk = "";

            // #####################################################################
            // Panel: WLAN (unveraendert aus der bisherigen eigenstaendigen
            // /wifi-Seite uebernommen - Hostname-Formular, WPS/Rescan, WLAN-Slots)
            // #####################################################################

            // #####################################################################
            // Panel: WiFi (unchanged, taken over from the previous standalone
            // /wifi page - hostname form, WPS/rescan, WiFi slots)
            // #####################################################################
            chunk += "<div class='tabpanel panel-wlan'>";

            if (webserver.arg("msg") == "WPS active - press the WPS button on your router now (within 2 minutes)") {
                String hostnameTargetJs = pingHostname ? ("'http://" + String(hostname) + ".local/'") : "null";
                chunk += "<script>";
                chunk += "(function() {";
                chunk += "  var attempts = 0;";
                chunk += "  var maxAttempts = 25;";
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
                chunk += "    tryFetch(location.pathname + location.search, {}).then(function() {";
                chunk += "      location.href = location.pathname + '?tab=wlan';";
                chunk += "    }).catch(function() {";
                chunk += "      if (hostnameTarget) {";
                chunk += "        tryFetch(hostnameTarget, { mode: 'no-cors' }).then(function() {";
                chunk += "          location.href = hostnameTarget + '?tab=wlan';";
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

            if (apMode) {
                chunk += "<div style='background:#fff3cd;color:#856404;border:1px solid #ffeeba;border-radius:6px;padding:10px 15px;margin:10px auto;max-width:500px;'>" +
                    translate("No WiFi network configured yet, or the last known network is unavailable - the clock created its own WiFi network. Enter your home WiFi details below, save, and the clock will restart and try to connect") + ".</div>";
            }

            // Zeigt an, mit welcher SSID die Uhr aktuell verbunden ist (oder
            // dass keine Verbindung besteht) - direkt oben im WLAN-Tab.

            // Shows which SSID the clock is currently connected to (or that
            // there is no connection) - right at the top of the WLAN tab.
            if (WiFi.status() == WL_CONNECTED) {
                chunk += "<div style='text-align:center;margin:10px auto;'>" + translate("Connected to") + ": <strong>" + WiFi.SSID() + "</strong></div>";
            }
            else if (!apMode) {
                chunk += "<div style='text-align:center;margin:10px auto;color:#856404;'>" + translate("Not connected") + "</div>";
            }

            chunk += "<form method='POST' action='/sethostname'>";
            chunk += "<div style='display:flex;justify-content:center;align-items:center;gap:12px;flex-wrap:wrap;'>";
            chunk += "<label>" + translate("Hostname") + ":</label>";
            chunk += "<span title='" + translate("The clock can also be reached at http://\"hostname\".local instead of its IP address, e.g.") + " http://" + String(hostname) + ".local. " + translate("A restart is required for a changed hostname to take effect. Not all routers support hostname resolution") + ".' style='cursor:help;'>&#9432;</span>";
            chunk += "<input name='hostname' maxlength='30' value='" + String(hostname) + "' style='width:170px;'>";
            chunk += "<button type='submit' style='width:140px;'>" + translate("Save") + "</button>";
            chunk += "</div>";
            chunk += "</form><br><br>";

            webserver.sendContent(chunk);
            chunk = "";

            chunk += "<div style='display:flex;justify-content:center;align-items:center;gap:12px;flex-wrap:wrap;'>";
            chunk += "<form method='POST' action='/api/startWPS' style='margin:0;'>";
            chunk += "<button type='submit' style='width:170px;'>" + translate("Add Network via WPS") + "</button>";
            chunk += "</form>";
            chunk += "<span title='" + translate("Adds a new network via WPS - press the WPS button on your router when prompted. The clock's connection may be lost for about 3 minutes while this happens") + ".' style='cursor:help;'>&#9432;</span>";
            chunk += "<button id='rescanBtn' type='button' style='width:170px;'>" + translate("Rescan Networks") + "</button>";
            chunk += "<span title='" + translate("Scans for available WiFi networks again and refreshes the dropdown lists below") + ".' style='cursor:help;'>&#9432;</span>";
            chunk += "</div><br>";

            // Zaehlt die bereits gespeicherten Netzwerke, um den "Verbinden"-Button
            // je Eintrag nur anzuzeigen, wenn es ueberhaupt eine Auswahl gibt
            // (mehr als 1 gespeichertes Netzwerk).

            // Counts the already saved networks, to only show the "Connect"
            // button per entry when there is actually a choice (more than
            // 1 saved network).
            int savedWifiCount = 0;
            for (int i = 0; i < MAX_WLAN; i++) {
                if (preferences.getString(pkSsid(i).c_str(), "") != "") savedWifiCount++;
            }

            chunk += "<form action = '/save' method = 'POST'>";

            for (int i = 0; i < MAX_WLAN; i++) {
                String ssidKey = pkSsid(i);
                String passKey = pkPass(i);
                String ssidSelectId = "ssid_select" + String(i + 1);
                wifiSsid[i] = preferences.getString(ssidKey.c_str(), "");

                chunk += "<hr>";

                String upperSsidKey = ssidKey;
                upperSsidKey.toUpperCase();
                chunk += "<h3 style='display:flex;align-items:center;justify-content:center;gap:6px;'>" + upperSsidKey;
                if (i == 0) {
                    chunk += " <span title='" + translate("Up to") + " " + String(MAX_WLAN) + " " + translate("WiFi networks can be stored") + ".' style='cursor:help;'>&#9432;</span>";
                }
                chunk += "</h3>";

                chunk += "<div style='display:flex;gap:6px;align-items:center;flex-wrap:wrap;justify-content:center;'>";
                chunk += "<select id='" + ssidSelectId + "' onchange=\"document.getElementById('" + ssidKey + "').value=this.value\" style='max-width:180px;'>";
                chunk += "</select>";
                chunk += "<input name='" + ssidKey + "' id='" + ssidKey + "' placeholder='" + ssidKey + "' value='" + wifiSsid[i] + "' style='width:110px;'>";
                chunk += "<input name='" + passKey + "' id='" + passKey + "' placeholder='Password' type='password' value='' style='width:110px;'>";
                if (wifiSsid[i] != "") {
                    // "Verbinden"-Button nur anzeigen, wenn mehr als ein Netzwerk
                    // gespeichert ist - bei nur einem Eintrag ist er bereits
                    // (oder wird beim Speichern) die aktive Verbindung.

                    // Only show the "Connect" button when more than one network
                    // is saved - with just one entry it's already (or will
                    // become, once saved) the active connection anyway.
                    if (savedWifiCount > 1) {
                        chunk += " <a href='/api/connectWifi?index=" + String(i) + "' onclick='return confirm(\"" + translate("Connect") + " " + wifiSsid[i] + "?\")'>" + translate("Connect") + "</a>";
                    }
                    chunk += " <a href='/deletewifi?index=" + String(i) + "' onclick='return confirm(\"" + translate("Delete") + " " + wifiSsid[i] + "?\")'>" + translate("Delete") + "</a>";
                }
                chunk += "</div>";

                chunk += "<small>" + translate("You can also enter an SSID manually") + ".";
                if (WiFi.getMode() == WIFI_STA && wifiSsid[i] != "") {
                    chunk += " " + translate("Password is hidden.Leave empty to keep current") + ".";
                }
                chunk += "</small>";

                if (i % 3 == 2) {
                    webserver.sendContent(chunk);
                    chunk = "";
                }

                if (wifiSsid[i] == "") {
                    break;
                }
            }

            chunk += "<br><br>";
            chunk += "<button type='submit'>" + translate("Save WiFi settings") + "</button></form><hr>";

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
            chunk += "      setTimeout(function() { location.href = location.pathname + '?tab=wlan'; }, 10000);";
            chunk += "    });";
            chunk += "};";

            webserver.sendContent(chunk);
            chunk = "";

            chunk += "window.addEventListener('DOMContentLoaded', function() {";
            chunk += "  function decodeHtml(s){var t=document.createElement('textarea');t.innerHTML=s;return t.value;}";
            chunk += "  var wifiOpenLabel = decodeHtml('" + translate("Open") + "');";
            chunk += "  var wifiSecuredLabel = decodeHtml('" + translate("Secured") + "');";
            for (int i = 0; i < MAX_WLAN; i++) {
                String ssidKey = pkSsid(i);
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
                chunk += "        var encLabel = (net.enc === 0) ? wifiOpenLabel : wifiSecuredLabel;";
                chunk += "        opt.text = net.ssid + ' (' + net.rssi + ' dBm, ' + encLabel + ')';";
                chunk += "        if(net.ssid === " + current + ") opt.selected = true;";
                chunk += "        " + select + ".appendChild(opt);";
                chunk += "      });";
                chunk += "    })";
                chunk += "    .catch(() => { " + select + ".innerHTML = \"<option>Scan failed</option>\"; });";

                if (i % 3 == 2) {
                    webserver.sendContent(chunk);
                    chunk = "";
                }

                if (preferences.getString(ssidKey.c_str(), "") == "") {
                    break;
                }
            }
            chunk += "});";
            chunk += "</script>";
            chunk += "</div>"; // Ende panel-wlan
                               // end panel-wlan

            webserver.sendContent(chunk);
            chunk = "";

            // #####################################################################
            // Panel: Zifferblatt (Anzeige-Einstellungen - unveraendert aus der
            // bisherigen "/"-Seite uebernommen)
            // #####################################################################

            // #####################################################################
            // Panel: clock face (display settings - unchanged, taken over from the
            // previous "/" page)
            // #####################################################################
            chunk += "<div class='tabpanel panel-zifferblatt'>";
            chunk += "<form action='/applydisplaysettings' method='POST'>";
            chunk += "<div class='card'>";

            chunk += "<div style='display:flex;align-items:center;gap:6px;white-space:nowrap;'><input type='checkbox' name='stationMode' value='1' ";
            chunk += preferences.getBool(PK_STATION_MODE, true) ? "checked" : "";
            chunk += " style='width:auto;margin:0;'>" + translate("Train Station Mode");
            chunk += " <span title='" + translate("The second hand rushes ahead slightly and briefly rests at 60, like a classic train station clock") + ".' style='cursor:help;'>&#9432;</span></div><br>";

            chunk += "<div style='display:flex;align-items:center;gap:6px;white-space:nowrap;'><input type='checkbox' name='showSecondHand' value='1' ";
            chunk += preferences.getBool(PK_SHOW_SECOND_HAND, true) ? "checked" : "";
            chunk += " style='width:auto;margin:0;'>" + translate("Show Seconds");
            chunk += " <span title='" + translate("Shows or hides the second hand on the clock face") + ".' style='cursor:help;'>&#9432;</span></div><br>";

            chunk += "<div style='display:flex;align-items:center;gap:6px;white-space:nowrap;'><input type='checkbox' name='smoothMinute' value='1' ";
            chunk += preferences.getBool(PK_SMOOTH_MINUTE, true) ? "checked" : "";
            chunk += " style='width:auto;margin:0;'>" + translate("Smooth Minute Hand");
            chunk += " <span title='" + translate("The minute hand moves smoothly instead of jumping in 1-minute steps") + ".' style='cursor:help;'>&#9432;</span></div><br>";

            chunk += "<div style='display:flex;align-items:center;gap:6px;white-space:nowrap;'><input type='checkbox' name='wifiActive' value='1' ";
            chunk += wifiActive ? "checked" : "";
            chunk += " style='width:auto;margin:0;'>" + translate("Reconnect WiFi");
            chunk += " <span title='" + translate("Automatically tries to reconnect if the WiFi connection is lost") + ".' style='cursor:help;'>&#9432;</span></div><br>";

            chunk += "<div style='display:flex;align-items:center;gap:6px;white-space:nowrap;'><input type='checkbox' name='loggingEnabled' value='1' ";
            chunk += loggingEnabled ? "checked" : "";
            chunk += " style='width:auto;margin:0;'>" + translate("Enable Logging");
            chunk += " <span title='" + translate("Writes up to 9 log files to LittleFS for troubleshooting") + ".' style='cursor:help;'>&#9432;</span></div><br>";

            String pingServer = preferences.getString(PK_PING_SERVER, DEFAULT_PING_SERVER);
            chunk += "<div style='display:flex;align-items:center;gap:6px;white-space:nowrap;'>" + translate("Ping Server");
            chunk += " <span title='" + translate("Server used to periodically check the internet connection") + ".' style='cursor:help;'>&#9432;</span>";
            chunk += "<input type='text' name='pingServer' value='" + pingServer + "' style='width:100px;'></div><br>";

            chunk += "<div style='display:flex;align-items:center;gap:6px;white-space:nowrap;'>" + translate("Rotation Display 1") + ": <span title='" + translate("Rotates the clock face by the selected number of degrees, useful if the display is mounted rotated in its housing") + ".' style='cursor:help;'>&#9432;</span> <select name='rotation' style='width:100px;'>";
            const char* rotationLabels[] = { "0&deg;", "90&deg;", "180&deg;", "270&deg;" };
            for (int i = 0; i <= 3; i++) {
                chunk += "<option value='" + String(i) + "'";
                if (i == tftRotation1) chunk += " selected";
                chunk += ">" + String(rotationLabels[i]) + "</option>";
            }
            chunk += "</select></div>";

            chunk += "<div style='display:flex;align-items:center;gap:6px;white-space:nowrap;'>" + translate("Rotation Display 2") + ": <span title='" + translate("Rotates Display 2's (CS2) clock face independently of Display 1") + ".' style='cursor:help;'>&#9432;</span> <select name='rotation2' style='width:100px;'>";
            for (int i = 0; i <= 3; i++) {
                chunk += "<option value='" + String(i) + "'";
                if (i == tftRotation2) chunk += " selected";
                chunk += ">" + String(rotationLabels[i]) + "</option>";
            }
            chunk += "</select></div>";

            chunk += "</div>";
            chunk += "<div style='text-align:center;margin-top:15px;'><button type='submit'>" + translate("Save") + "</button></div>";
            chunk += "</form>";
            chunk += "</div>"; // Ende panel-zifferblatt
                               // end panel-zifferblatt

            webserver.sendContent(chunk);
            chunk = "";

            // #####################################################################
            // Panel: Helligkeit (unveraendert aus der bisherigen /brightness-Seite
            // uebernommen, inkl. optionalem Plotly-Gamma-Chart)
            // #####################################################################

            // #####################################################################
            // Panel: brightness (unchanged, taken over from the previous
            // /brightness page, incl. optional Plotly gamma chart)
            // #####################################################################
            chunk += "<div class='tabpanel panel-helligkeit'>";
            chunk += "<form method='POST' action='/save_brightness'>";

            chunk += "<div class='card'>";
            chunk += brightnessFormFieldsHtml();
            chunk += "</div>";
            chunk += "<button type='submit'>" + translate("Save") + "</button></form>";

            webserver.sendContent(chunk);
            chunk = "";

            if (photoresistorFound) {
                chunk += "<br>";
                chunk += "<hr><strong>" + translate("Current ADC Value") + ":</strong> " + String(currentAdcAvg) + "<br>";
                chunk += "<strong>" + translate("Current Brightness") + ":</strong> " + String(currentBrightness) + " / 255<br>";
                chunk += "<strong>" + translate("Light (for Threshold)") + ":</strong> " + String(currentLightPercent) + " % <br>";
                chunk += "<br>";
                // Bei einem GET-Formular ersetzt der Browser die Query-String
                // des "action"-Attributs durch die serialisierten Formularfelder -
                // "?tab=helligkeit" in der Action-URL ginge also beim Abschicken
                // verloren (Seite landete danach auf dem Standard-Tab statt auf
                // Helligkeit). Daher als verstecktes Feld mitschicken, statt es
                // nur in der Action-URL zu haben.

                // With a GET form, the browser replaces the "action" attribute's
                // query string with the serialized form fields - "?tab=helligkeit"
                // in the action URL would therefore be lost on submit (page ended
                // up on the default tab instead of Brightness). So send it as a
                // hidden field instead of only having it in the action URL.
                chunk += "<form method='GET' action='/'><input type='hidden' name='tab' value='helligkeit'><button type='submit'>" + translate("Refresh") + "</button></form>";
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
                chunk += "const minBrightnessG = " + String(minBrightness) + ";\n";
                chunk += "const maxBrightnessG = " + String(maxBrightness) + ";\n";
                chunk += "const gammaAvg = Array.from({length: 500}, (_, i) => i * (4095 / 499));\n\n";

                chunk += "function computeBrightness(gamma) {\n";
                chunk += "  return gammaAvg.map(val => {\n";
                chunk += "    let norm = Math.min(Math.max(val / 4095.0, 0.0), 1.0);\n";
                chunk += "    let gammaNorm = Math.pow(norm, gamma);\n";
                chunk += "    return minBrightnessG + Math.round((maxBrightnessG - minBrightnessG) * gammaNorm);\n";
                chunk += "  });\n";
                chunk += "}\n\n";

                webserver.sendContent(chunk);
                chunk = "";

                chunk += "function plotGamma(gamma) {\n";
                chunk += "  const y = computeBrightness(gamma);\n";
                chunk += "  Plotly.newPlot('plot', [{\n";
                chunk += "    x: gammaAvg,\n";
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
                chunk += "  document.querySelector(\"input[name='gamma']\").value = gamma.toFixed(1);\n";
                chunk += "  plotGamma(gamma);\n";
                chunk += "});\n\n";

                // WICHTIG: Plotly kann in ein per CSS verstecktes (display:none)
                // Panel nicht sinnvoll zeichnen (Container hat dort Breite/Hoehe 0).
                // Daher hier NICHT sofort beim Laden zeichnen, sondern erst wenn
                // der Helligkeit-Tab tatsaechlich aktiviert wird (bzw. sofort, falls
                // er schon beim Laden aktiv ist).

                // IMPORTANT: Plotly cannot meaningfully draw into a panel hidden via CSS
                // (display:none) (the container has width/height 0 there). So do not
                // draw immediately on load here, only once the brightness tab is
                // actually activated (or immediately, if it is already active on load).
                chunk += "var gammaTab = document.getElementById('tab-helligkeit');\n";
                chunk += "function drawGammaIfVisible() { if (gammaTab.checked) plotGamma(parseFloat(slider.value)); }\n";
                chunk += "gammaTab.addEventListener('change', drawGammaIfVisible);\n";
                chunk += "drawGammaIfVisible();\n";
                chunk += "</script>\n";
#endif
            }

            chunk += "</div>"; // Ende panel-helligkeit
                               // end panel-helligkeit

            webserver.sendContent(chunk);
            chunk = "";

            // #####################################################################
            // Panel: Zeit / NTP / Timezone (unveraendert aus der bisherigen
            // /timezone_form-Seite uebernommen)
            // #####################################################################

            // #####################################################################
            // Panel: time / NTP / timezone (unchanged, taken over from the previous
            // /timezone_form page)
            // #####################################################################
            chunk += "<div class='tabpanel panel-zeit'>";
            {
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

                chunk += "<h2>" + translate("NTP Server / Timezone (DST String)") + "</h2>";
                chunk += "<form method='POST' action='/set_timezone'>";

                for (int i = 0; i < MAX_WLAN; i++) {
                    chunk += "<div style='display:flex;gap:6px;align-items:center;flex-wrap:wrap;justify-content:center;'>";
                    chunk += "NTP Server" + String(i + 1) + " : <input type = 'text' id='ntpServerInput" + String(i + 1) + "' name = 'ntpServer" + String(i + 1) + "' value = '" + String(ntpServers[i]) + "' style='width:180px;'>";
                    chunk += "<button type='button' onclick='testNtp(" + String(i + 1) + ")' style='width:180px;'>" + translate("Test") + "</button>";
                    chunk += "</div>";
                    chunk += "<div id='ntpTestResult" + String(i + 1) + "'></div>";
                    if (trim(ntpServers[i]) == "") {
                        break;
                    }
                }

                webserver.sendContent(chunk);
                chunk = "";

                chunk += "<script>";
                chunk += "async function testNtp(idx) {";
                chunk += "  var input = document.getElementById('ntpServerInput' + idx);";
                chunk += "  var result = document.getElementById('ntpTestResult' + idx);";
                chunk += "  result.innerHTML = '" + translate("Testing") + "...';";
                chunk += "  try {";
                chunk += "    var r = await fetch('/api/testNtp?server=' + encodeURIComponent(input.value));";
                chunk += "    var text = await r.text();";
                chunk += "    if (text.indexOf('OK|') === 0) {";
                chunk += "      result.innerHTML = '<div style=\\'background:#d4edda;color:#155724;border:1px solid #c3e6cb;border-radius:6px;padding:8px 12px;margin:10px auto;max-width:400px;\\'>&#10004; ' + text.substring(3) + '</div>';";
                chunk += "    } else {";
                chunk += "      result.innerHTML = '<div style=\\'background:#f8d7da;color:#721c24;border:1px solid #f5c6cb;border-radius:6px;padding:8px 12px;margin:10px auto;max-width:400px;\\'>&#10008; " + translate("Server not reachable") + "</div>';";
                chunk += "    }";
                chunk += "  } catch (e) {";
                chunk += "    result.innerHTML = '<div style=\\'background:#f8d7da;color:#721c24;border:1px solid #f5c6cb;border-radius:6px;padding:8px 12px;margin:10px auto;max-width:400px;\\'>&#10008; " + translate("Server not reachable") + "</div>';";
                chunk += "  }";
                chunk += "}";
                chunk += "</script>";

                chunk += translate("Timezone") + ": <br><select id = 'tz_select' style = 'width: 400px;' onchange = \"document.getElementById('tz_input').value=this.value\">";
                for (size_t i = 0; i < sizeof(tzList) / sizeof(tzList[0]); i++) {
                    chunk += "<option value='" + String(tzList[i].value) + "'";
                    if (timezone == tzList[i].value) chunk += " selected";
                    chunk += ">" + String(tzList[i].label) + " (" + String(tzList[i].value) + ")</option>";
                }
                chunk += "</select><br><br>";

                chunk += "<input type='text' id='tz_input' name='timezone' style='width: 400px;' value='" + timezone + "'><br><br>";
                chunk += "<small>" + translate("For custom timezones, select a preset or enter your own value above") + "</small><br><br>";
                chunk += "<button type='submit'>" + translate("Save Timezone") + "</button><br><br>";
                chunk += "</form>";
            }
            chunk += "</div>"; // Ende panel-zeit
                               // end panel-zeit

            webserver.sendContent(chunk);
            chunk = "";

            // #####################################################################
            // Panel: System (Reboot, Verwaltung, Factory Reset - neu, verweist auf
            // die weiterhin eigenstaendigen Seiten fuer die komplexeren, Datei-
            // lastigen Bereiche statt sie hier nochmal nachzubauen)

            // #####################################################################
            // #####################################################################
            // Panel: system (reboot, management, factory reset - new, links to the
            // still-standalone pages for the more complex, file-heavy areas instead of rebuilding them here)
            // #####################################################################
            // Erlaubt gezieltes Anspringen eines Tabs per ?tab=... (z.B. nach
            // einem POST-Redirect von /save, /sethostname, /save_brightness,
            // /set_timezone etc.) - rein clientseitig, da die Tab-Auswahl selbst
            // per CSS-radio erfolgt und serverseitig kein Zustand dafuer noetig ist.

            // Allows jumping directly to a tab via ?tab=... (e.g. after a POST
            // redirect from /save, /sethostname, /save_brightness, /set_timezone
            // etc.) - purely client-side, since tab selection itself works via CSS radio and needs no server-side state.
            chunk += "<script>";
            chunk += "(function() {";
            chunk += "  var m = location.search.match(/[?&]tab=([a-zA-Z]+)/);";
            chunk += "  if (m) { var el = document.getElementById('tab-' + m[1]); if (el) el.checked = true; }";
            chunk += "})();";
            chunk += "</script>";

            chunk += "</body></html>";
            webserver.sendContent(chunk);
            webserver.sendContent(""); // Ende der Chunked-Uebertragung signalisieren
                                       // signal the end of the chunked transfer
            });

        webserver.on("/deletewifi", HTTP_GET, []() {
            if (webserver.hasArg("index")) {
                int idx = webserver.arg("index").toInt();
                if (idx >= 0 && idx < MAX_WLAN) {

                    // Merkt sich vor dem Loeschen, ob es sich um das gerade aktiv
                    // verbundene Netzwerk handelt - wird nach dem Umsortieren der
                    // Liste (Indizes verschieben sich) fuer den Neustart gebraucht.

                    // Remembers, before deleting, whether this is the network the
                    // clock is currently actively connected to - needed after the
                    // list is reindexed (indices shift) to decide on a reboot.
                    bool deletingActiveNetwork = (WiFi.status() == WL_CONNECTED && wifiSsid[idx] != "" && WiFi.SSID() == wifiSsid[idx]);

                    putStringVerified(pkSsid(idx).c_str(), "");
                    putStringVerified(pkPass(idx).c_str(), "");

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
                            putStringVerified(pkSsid(i).c_str(), tempSsid[i]);
                        }
                        if (preferences.getString(pkPass(i).c_str(), "") != tempPass[i]) {
                            putStringVerified(pkPass(i).c_str(), tempPass[i]);
                        }
                        wifiSsid[i] = tempSsid[i];
                        wifiPass[i] = tempPass[i];
                    }

                    if (deletingActiveNetwork) {

                        // Das aktuell verbundene Netzwerk wurde geloescht - PK_LAST_WLAN
                        // wird komplett entfernt (statt auf einen Index gesetzt), damit
                        // beim Neustart kein veralteter/falscher Index verwendet wird.
                        // getInt() faellt dann auf den Default-Wert 0 zurueck und die
                        // Uhr versucht ueber die normale Boot-Sequenz ein anderes
                        // gespeichertes WLAN (oder startet den Access-Point-Modus,
                        // falls kein Eintrag mehr uebrig ist).

                        // The currently connected network was deleted - PK_LAST_WLAN is
                        // removed entirely (instead of set to an index), so a stale/wrong
                        // index isn't used after reboot. getInt() then falls back to its
                        // default value 0, and the clock tries a different saved WiFi
                        // network via the normal boot sequence (or starts access point
                        // mode if no entry remains).
                        DEBUG_PRINTLN("[WiFi] Active network was deleted via Web UI - rebooting to connect to a different saved network");
                        preferences.remove(PK_LAST_WLAN);
                        redirectTo("/?tab=wlan&msg=Network%20deleted%2C%20reconnecting...");

                        // preferences.end() nicht mehr hier, sondern in espReboot() selbst -
                        // sonst faellt der dort geloggte Reboot-Eintrag auf die falsche
                        // Logdatei zurueck (siehe Kommentar in espReboot()).

                        // preferences.end() no longer called here, but inside espReboot()
                        // itself - otherwise the reboot log entry logged there falls back
                        // to the wrong log file (see comment in espReboot()).
                        delay(WAIT_1s);
                        // Neustart des ESP
                        // Restart the ESP
                        espReboot();
                    }
                }
                redirectTo("/?tab=wlan&msg=Network%20deleted");
            }
            else {
                webserver.send(400, "text/plain", "Missing parameter");
            }
            });

        // Verbindet die Uhr sofort mit einem bestimmten gespeicherten WLAN-Netzwerk
        // (Button "Verbinden" in der WLAN-Uebersicht). connectWiFi() selbst blockiert
        // bis zu 30s und wuerde daher den Webserver waehrend der Antwort blockieren -
        // stattdessen wird PK_LAST_WLAN gesetzt (einzige Quelle der Wahrheit fuer das
        // beim Boot verwendete Netzwerk, siehe wifi_manager.h) und die Uhr neu
        // gestartet, die dann ueber die normale Boot-Sequenz sauber verbindet.

        // Connects the clock immediately to a specific saved WiFi network (the
        // "Connect" button in the WLAN overview). connectWiFi() itself blocks for
        // up to 30s, which would block the webserver while the response is being
        // sent - instead PK_LAST_WLAN is set (single source of truth for which
        // network is used at boot, see wifi_manager.h) and the clock reboots,
        // connecting cleanly via the normal boot sequence.
        webserver.on("/api/connectWifi", HTTP_GET, []() {
            if (webserver.hasArg("index")) {
                int idx = webserver.arg("index").toInt();
                if (idx >= 0 && idx < MAX_WLAN && preferences.getString(pkSsid(idx).c_str(), "") != "") {
                    DEBUG_PRINTLN("[WiFi] Web UI requested switch to saved network: " + preferences.getString(pkSsid(idx).c_str(), ""));
                    preferences.putInt(PK_LAST_WLAN, idx);
                    redirectTo("/?tab=wlan&msg=Connecting...");

                    // preferences.end() nicht mehr hier, sondern in espReboot() selbst -
                    // sonst faellt der dort geloggte Reboot-Eintrag auf die falsche
                    // Logdatei zurueck (siehe Kommentar in espReboot()).

                    // preferences.end() no longer called here, but inside espReboot()
                    // itself - otherwise the reboot log entry logged there falls back
                    // to the wrong log file (see comment in espReboot()).
                    delay(WAIT_1s);
                    // Neustart des ESP
                    // Restart the ESP
                    espReboot();
                }
                else {
                    webserver.send(400, "text/plain", "Invalid index");
                }
            }
            else {
                webserver.send(400, "text/plain", "Missing parameter");
            }
            });

        webserver.on("/save", HTTP_POST, []() {
            //if (webserver.hasArg("ssid1")) {

                for (int i = 0; i < MAX_WLAN; i++) {
                    // Dynamisch berechnete Schlüssel
                    // Dynamically computed keys
                    String ssidKey = pkSsid(i);
                    String passKey = pkPass(i);

                    if (preferences.getString(ssidKey.c_str(), "") != webserver.arg(ssidKey)) {
                        putStringVerified(ssidKey.c_str(), webserver.arg(ssidKey));
                    }
                    if (webserver.arg(passKey) != "" && preferences.getString(passKey.c_str(), "") != webserver.arg(passKey)) {
                        putStringVerified(passKey.c_str(), webserver.arg(passKey));
                    }
                }


                // leere Einträge aussortieren
                // Filter out empty entries
                String tempSsid[MAX_WLAN];
                String tempPass[MAX_WLAN];


                int j = 0;
                for (int i = 0; i < MAX_WLAN; i++) {

                    // Dynamisch berechnete Schlüssel
                    // Dynamically computed keys
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
                    // Dynamically computed keys
                    String ssidKey = pkSsid(i);
                    String passKey = pkPass(i);

                    if (preferences.getString(ssidKey.c_str(), "") != tempSsid[i]) {
                        putStringVerified(ssidKey.c_str(), tempSsid[i]);
                    }
                    if (preferences.getString(passKey.c_str(), "") != tempPass[i]) {
                        putStringVerified(passKey.c_str(), tempPass[i]);
                    }

                    wifiSsid[i] = tempSsid[i];
                    wifiPass[i] = tempPass[i];
                }


            

                if (WiFi.getMode() == WIFI_STA) {
                    redirectTo("/?tab=wlan&msg=Settings%20saved");
                }
                else {
                    webserver.send(200, "text/html", simpleMessagePage(translate("Settings saved"), "<p>" + translate("Please connect to your home network and go to the ESP website at") + " http:// IPADDRESS</p>"));

                    espReboot();
                }
           // }
            });

        // Upload-Formular anzeigen
        // Show upload form
        webserver.on("/upload", HTTP_GET, []() {
            String uploadFormHtml = "<form method='POST' action='/upload' enctype='multipart/form-data' onsubmit='showProgress()'><input type='file' name='upload' accept='.bmp' multiple required><br><br><button type='submit'>Upload BMP</button><div id='progress' style='display:none;'>Uploading... please wait</div><script>function showProgress(){document.getElementById('progress').style.display='block';}</script></form><br><a href='/listfilesFaces'><button type='button'>" + translate("Back") + "</button></a>";
            webserver.send(200, "text/html", simpleMessagePage(translate("Upload"), uploadFormHtml));
            });

        // Datei-Upload verarbeiten
        // Process file upload
        webserver.on("/upload", HTTP_POST, []() {
            if (uploadSuccess) {
                redirectTo("/listfilesFaces?msg=Clock%20face%20uploaded");
            }
            else {
                String errorHtml = "<p>" + translate("Only .bmp files starting with") + " <code>face_</code> " + translate("or") + " <code>hand_</code> " + translate("are accepted") + ".</p>";
                errorHtml += "<p>" + translate("Please also check the available space") + ".</p>";
                errorHtml += "<a href='/upload'><button type='button'>" + translate("Try again") + "</button></a>";
                webserver.send(400, "text/html", simpleMessagePage(translate("Upload failed"), errorHtml));
            }
            }, handleFileUpload);

        // Hintergrundbild setzen
        // Set background image
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
        // Delete file
        webserver.on("/delete", HTTP_GET, []() {
            if (webserver.hasArg("file")) {
                String path = webserver.arg("file");
                //path.replace(".", "");
                if (!path.startsWith("/")) path = "/" + path;
                if (LittleFS.exists(path)) {
                    LittleFS.remove(path);

                    // Falls ein Zifferblatt oder Teil eines Zeigersatzes geloescht
                    // wurde, alle Presets entfernen, die darauf verweisen.

                    // If a clock face or part of a hand set was deleted, remove all
                    // presets that reference it.
                    String name = path.substring(1); // fuehrenden Slash entfernen
                                                     // remove the leading slash
                    if (name.startsWith("face_") && name.endsWith(".bmp")) {
                        removeOrphanedPresets(path, "");
                    }
                    else if (name.startsWith("hand_set") && name.endsWith(".bmp")) {
                        int start = 8; // Laenge von "hand_set"
                                       // length of "hand_set"
                        int end = name.indexOf('_', start);
                        if (end > start) {
                            String setId = name.substring(start, end);
                            removeOrphanedPresets("", setId);

                            // Falls der betroffene Zeigersatz gerade aktiv war,
                            // sofort auf den eingebauten Standard zurueckschalten.

                            // If the affected hand set was currently active, switch back to the
                            // built-in default immediately.
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
            else {
                // Ohne diesen Zweig blieb ein "GET /delete" ganz ohne Parameter
                // unbeantwortet: der ESP32-WebServer erzeugt fuer eine bereits
                // gematchte Route kein automatisches 404, die Verbindung lief also
                // bis zum Client-Timeout und hat den single-threaded Handler-Loop
                // solange blockiert. Alle vergleichbaren Routen (/download, /file,
                // /rename, /deletewifi) haben diesen Zweig bereits.

                // Without this branch a "GET /delete" with no parameter at all went
                // unanswered: the ESP32 WebServer does not generate an automatic 404
                // for an already-matched route, so the connection stayed open until
                // the client timed out and blocked the single-threaded handler loop
                // for that long. All comparable routes (/download, /file, /rename,
                // /deletewifi) already have this branch.
                webserver.send(400, "text/plain", "Missing parameter: file");
            }
            });

        // Einzelnes Preset loeschen (Slot wird dadurch wieder frei fuer
        // createPresetFromPreferences())

        // Delete a single preset (frees up the slot again for
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
        // Rename form for a single preset
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
        // Rename preset action
        webserver.on("/renamepreset", HTTP_POST, []() {
            if (webserver.hasArg("index") && webserver.hasArg("new")) {
                int idx = webserver.arg("index").toInt();
                String newName = webserver.arg("new");
                newName.replace(" ", "_"); // Konsistent zur Anzeige/den API-Links (siehe /presets)
                                           // consistent with the display/API links (see /presets)

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
        // Show file (BMP)
        webserver.on("/file", HTTP_GET, []() {
            if (webserver.hasArg("name")) {

                setLedOn();

                String path = webserver.arg("name");
                if (!path.startsWith("/")) path = "/" + path;

                if (LittleFS.exists(path)) {

                    // Prüfe den Dateityp basierend auf der Dateiendung
                    // Check the file type based on its extension
                    if (path.endsWith(".log") || path.endsWith(".txt")) {
                        File file = LittleFS.open(path, "r");
                        webserver.streamFile(file, "text/plain"); // Logdateien als Text senden
                                                                  // send log files as text
                        file.close();
                    }
                    else if (path.endsWith(".bmp")) {
                        // Pruefen, ob RLE-komprimiert - falls ja, vor der Auslieferung zu
                        // einem echten Standard-BMP dekodieren (sonst fuer externe Tools nicht lesbar).

                        // Check whether it is RLE-compressed - if so, decode it to a real
                        // standard BMP before serving it (otherwise unreadable by external tools).
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

                            // Streaming failed (e.g. read error) - do NOT silently serve the raw
                            // compressed bytes (no longer a valid BMP), instead a clear error
                            // (500 status may no longer be possible if headers were already sent).
                            DEBUG_PRINTLN("[FILE] RLE streaming failed for " + path);
                            setLedOff();
                            return;
                        }

                        File file = LittleFS.open(path, "r");
                        webserver.streamFile(file, "image/bmp"); // BMP-Dateien als Bild senden
                                                                 // send BMP files as an image
                        file.close();
                    }
                    else {
                        File file = LittleFS.open(path, "r");
                        webserver.streamFile(file, "application/octet-stream"); // Andere Dateien als Binärdaten senden
                                                                                // send other files as binary data
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
        // Manage hand sets
        webserver.on("/handsets", HTTP_GET, []() {

            size_t total = LittleFS.totalBytes();
            size_t used = LittleFS.usedBytes();

            // Chunked-Response (Variante B): Seite bettet Vorschaubilder als Base64 ein und
            // kann dadurch sehr gross werden - wird daher Stueck fuer Stueck gesendet
            // (webserver.sendContent()), Speicherbedarf haengt nur von der groessten Zeile ab.

            // Chunked response (variant B): the page embeds preview images as base64
            // and can therefore get very large - sent piece by piece
            // (webserver.sendContent()), memory use only depends on the largest line.
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

            // Sort numerically (std::map<int,String>) so e.g. "10" ends up after "9"
            // instead of between "1" and "2". Non-numeric names (unusual) land
            // additionally, unsorted, in a separate list so they are not lost.
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
                                                                  // true if new (not seen yet)
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
            // Default hand set (built-in) - its own chunk
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

            // Send each found hand set IMMEDIATELY instead of collecting them - so
            // never more than one hand set is in memory at a time.
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
            // First all numerically named hand sets in ascending order...
            for (auto& entry : numericSets) {
                renderSetRow(entry.second);
            }
            // ...danach eventuelle Sonderfaelle mit nicht-numerischem Namen (unsortiert)
            // ...then any special cases with non-numeric names (unsorted)
            for (const String& setId : otherSets) {
                renderSetRow(setId);
            }

            // Ab hier sind die grossen Base64-Strings nicht mehr benoetigt.
            // From here on the large base64 strings are no longer needed.
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
                // Collect existing hand-set filenames to compare with GitHub
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

                // Automatic download of new hand sets directly in the browser (see the
                // detailed comment at /listfilesFaces - same principle: browser
                // downloads via CORS from GitHub, uploads locally via /uploadhandset).
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

            chunk += "<br><br>";
            chunk += "</body></html>";
            webserver.sendContent(chunk);
            webserver.sendContent(""); // Ende der Chunked-Uebertragung signalisieren
                                       // signal the end of the chunked transfer
            });

        webserver.on("/setcenter", HTTP_POST, []() {
            if (webserver.hasArg("size") && webserver.hasArg("color")) {
                hubSize = argToIntClamped("size", hubSize, 0, 100);
                uint32_t rgb = (uint32_t)strtoul(webserver.arg("color").c_str(), nullptr, 16);

                // 24-Bit RGB888 in RGB565 umwandeln
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
        // Process hand-set file upload
        webserver.on("/uploadhandset", HTTP_POST, []() {
            if (uploadSuccess) {
                // Sicherheitsprüfung auf Dateinamenmuster
                // Security check on the filename pattern
                if (!uploadFilePath.endsWith(".bmp") || !uploadFilePath.startsWith("/hand_set")) {
                    String errorHtml = "<p>" + translate("Only .bmp files starting with") + " <code>hand_</code> " + translate("are accepted for handset upload") + ".</p>";
                    errorHtml += "<a href='/handsets'><button type='button'>" + translate("Try again") + "</button></a>";
                    webserver.send(400, "text/html", simpleMessagePage(translate("Upload failed"), errorHtml));
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
                webserver.send(500, "text/html", simpleMessagePage(translate("Upload failed"), "<a href='/handsets'><button type='button'>" + translate("Try again") + "</button></a>"));
            }
            }, handleFileUpload);


        // Handset setzen
        // Set hand set
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
        // Delete hand set
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

                // If the deleted hand set was currently active, switch back to the
                // built-in default immediately - otherwise the clock would try to load
                // a hand set that no longer exists.
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
        // Restart the ESP
        webserver.on("/reboot", HTTP_GET, []() {
            webserver.send(200, "text/html", simpleMessagePage(translate("Rebooting..."), "<p>" + translate("Return to the main page in 10 seconds or refresh the website when the ESP is online again") + ".</p>", "<meta http-equiv='refresh' content='10; url=/'>"));
            espReboot();
            });

        // Werkseinstellungen: Uebersichtsseite mit mehreren, einzeln
        // bestaetigten Reset-Optionen statt einer einzigen Alles-oder-nichts-Aktion.

        // Factory settings: overview page with several individually confirmed
        // reset options instead of one single all-or-nothing action.
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
        // Immediate time synchronization
        webserver.on("/syncnow", HTTP_POST, []() {
            setupNTP();
           // struct tm timeinfo;
            getLocalTime(&timeinfo, 100);

            char timeStr[32];
            strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);

            String html = simpleMessagePage(translate("Time synced"), "<p>" + String(timeStr) + "</p><p>" + translate("Returning to main page in 3 seconds") + ".</p>", "<meta http-equiv='refresh' content='3; url=/'>");

            webserver.send(200, "text/html", html);
            });

    }


    // Handhabt den Datei-Upload
    // Handles the file upload

    void handleFileUpload() {
        HTTPUpload& upload = webserver.upload();

        if (upload.status == UPLOAD_FILE_START) {
            uploadFilePath = upload.filename;
           // uploadFilePath.replace(".", "");
           // uploadFilePath.replace("#", "_");

            if (!uploadFilePath.startsWith("/")) uploadFilePath = "/" + uploadFilePath;

            // Nur bestimmte Dateinamenmuster zulassen
            // Only allow certain filename patterns
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

    // Checks whether the clock face given in a preset URL exists - case-
    // insensitive (LittleFS is case-sensitive, users might type it differently),
    // corrects the URL on a match. False = no matching face (face_default.bmp is always valid).

    bool validateAndFixPresetFace(String& url, const std::vector<String>& existingFaces) {
        int facePos = url.indexOf("face=");
        if (facePos == -1) return true; // kein face-Parameter, nichts zu pruefen
                                        // no face parameter, nothing to check

        int valueStart = facePos + 5; // Laenge von "face="
                                      // length of "face="
        int valueEnd = url.indexOf('&', valueStart);
        if (valueEnd == -1) valueEnd = url.length();

        String faceValue = url.substring(valueStart, valueEnd);
        String faceName = faceValue.startsWith("/") ? faceValue.substring(1) : faceValue;

        if (faceName.equalsIgnoreCase("face_default.bmp")) {
            return true; // eingebautes Standard-Zifferblatt ist immer gueltig
                         // built-in default face is always valid
        }

        for (const String& existing : existingFaces) {
            if (faceName.equalsIgnoreCase(existing)) {
                String correctValue = "/" + existing;
                if (correctValue != faceValue) {
                    // Gross-/Kleinschreibung weicht ab - URL korrigieren
                    // Case differs - correct the URL
                    url = url.substring(0, valueStart) + correctValue + url.substring(valueEnd);
                }
                return true;
            }
        }

        return false; // kein passendes Zifferblatt gefunden
                      // no matching face found
    }


    // Verarbeitet den Datei-Upload fuer /importpresets: schreibt die Datei temporaer,
    // liest sie zeilenweise ein (Format "Name<TAB>URL", wie von /exportpresets erzeugt)
    // und fuegt nur NEUE Presets in freie Slots ein. Bestehende Presets werden NICHT
    // geloescht - auch nicht, wenn eine Zeile aus der Importdatei denselben Namen
    // traegt (der bestehende Eintrag bleibt dann einfach unangetastet erhalten).

    // Processes the file upload for /importpresets: writes the file
    // temporarily, reads it line by line (format "Name<TAB>URL", as produced
    // by /exportpresets) and inserts only NEW presets into free slots.
    // Existing presets are NOT deleted - not even if a line from the import
    // file has the same name (the existing entry is simply left untouched).

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

                // Collect the names of already existing presets once, so imported
                // lines with the same name can be skipped - the existing preset stays
                // unchanged as a result.
                std::vector<String> existingPresetNames;
                for (int i = 0; i < MAX_PRESETS; i++) {
                    if (!presets[i].name.isEmpty() && !presets[i].url.isEmpty()) {
                        existingPresetNames.push_back(presets[i].name);
                    }
                }

                // Vorhandene Zifferblaetter einmalig einlesen, um jede
                // importierte Preset-Zeile dagegen pruefen zu koennen.

                // Read existing clock faces once, so each imported preset line can
                // be checked against them.
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

                    // A preset with the same name already exists - skip it instead of
                    // overwriting or deleting it.
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
                                                         // also protects against duplicates WITHIN the import file
                    importedCount++;
                }
                readFile.close();
                LittleFS.remove(PRESET_IMPORT_TMP_PATH);

                if (importedCount > 0) {
                    savePresets();
                }

                DEBUG_PRINTLN("[PRESET-IMPORT] " + String(importedCount) + " presets imported, " + String(skippedCount) + " skipped");
                presetImportSuccess = true; // auch 0 neue Presets ist kein Fehler (z.B. alles schon vorhanden)
                                            // 0 new presets is also not an error (e.g. everything already existed)
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

    // Similar to handlePresetImportUpload() - only inserts new presets into
    // free slots, existing ones are kept. Used by the GitHub download button
    // on /presets: filtering out already-existing names is already done by
    // the calling JavaScript function before the file arrives here (so
    // there is NO server-side name-duplicate check like in handlePresetImportUpload()).

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
                                            // 0 new presets is also not an error (e.g. everything already existed)
                DEBUG_PRINTLN("[PRESET-MERGE] " + String(addedCount) + " new presets added, " + String(skippedCount) + " skipped");
            }
            else {
                DEBUG_PRINTLN("[PRESET-MERGE] Failed while writing");
            }
        }
    }
