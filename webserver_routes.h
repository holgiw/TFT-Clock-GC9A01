#pragma once

    // Alle HTTP-Routen und HTML-Generierung (benoetigt globals.h, config.h, prefs_keys.h, declarations.h).
    // new (std::nothrow) liefert bei fehlgeschlagener Allokation sicher nullptr.

    // All HTTP routes and HTML generation (requires globals.h, config.h, prefs_keys.h, declarations.h). new
    // (std::nothrow) reliably returns nullptr on a failed allocation.

#include <new>

    // Tab-Leiste der Startseite an EINER Stelle: Reihenfolge = Anzeige-
    // reihenfolge, index-gleich zu SETTINGS_TAB_LABELS. Genutzt von
    // generateHtmlHeader(), den Radio-Inputs und generateSettingsTabNav().

    // The start page's tab bar in ONE place: order = display order, index-
    // aligned with SETTINGS_TAB_LABELS. Used by generateHtmlHeader(), the
    // start page's radio inputs, and generateSettingsTabNav().

    static const char* const SETTINGS_TAB_KEYS[] = { "wlan", "zifferblatt", "helligkeit", "zeit", "rocrail", "status", "log" };

    // Uebersetzungsschluessel je Tab (siehe translation.h). "Status"/"Log"
    // stehen bewusst am Ende (Diagnose nach dem Einrichtungsablauf).
    // "Rocrail" bleibt unuebersetzt, wie auch "DCF77" ein Protokollname bleibt.

    // Translation key per tab (see translation.h). "Status"/"Log" are
    // deliberately last (diagnostics after the setup flow). "Rocrail" stays
    // untranslated, the same way "DCF77" stays a protocol name.

    static const char* const SETTINGS_TAB_LABELS[] = { "WiFi Settings", "Clock Setup", "Brightness", "NTP&nbsp;Timezone", "Rocrail", "Status", "Log" };

    static const size_t SETTINGS_TAB_COUNT = sizeof(SETTINGS_TAB_KEYS) / sizeof(SETTINGS_TAB_KEYS[0]);


    // Generiert den HTML-Header für die Weboberfläche
    // Generates the HTML header for the web interface

    String generateHtmlHeader(String extraHead) {
        String html = "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
        html.reserve(5000);  // Vorab reservierter Speicher fuer CSS+HTML
                              // Pre-reserved capacity for CSS+HTML

        // Dunkles Theme mit CSS-only Tab-Mechanik (verstecktes radio-Input +
        // Label + Selektor "~") fuer die Tab-Hub-Startseite. Gilt sitenweit,
        // damit alle Seiten optisch einheitlich bleiben.

        // Dark theme with a CSS-only tab mechanism (hidden radio input +
        // label + "~" selector) for the tab-hub home page. Applies site-wide
        // so every page stays visually consistent.

        html += "<style>";
        html += ":root{--bg:#10151a;--panel:#1a2129;--panel-border:#2a333c;--text:#e8edf2;--muted:#8f9ba7;--accent:#f5a623;--accent-dim:#7a530f;--ok:#3ddc84;--bad:#ff5c5c;--mono:ui-monospace,\"SFMono-Regular\",Menlo,Consolas,monospace;}";

        // Meldungen einheitlich im dunklen Design: .msg.ok Erfolg, .msg.warn Hinweis, .msg.err Fehler - das Symbol
        // setzt das CSS, der Text bleibt ohne.

        // Messages uniform in the dark design: .msg.ok success, .msg.warn note, .msg.err error - the CSS sets the
        // symbol, the text stays without.

        html += ".msg{border:1px solid;border-radius:6px;padding:10px 15px;margin:10px auto;max-width:500px;}";
        html += ".msg::before{margin-right:.45em;}";
        html += ".msg.ok{background:rgba(61,220,132,.12);color:var(--ok);border-color:var(--ok);}.msg.ok::before{content:\"\\2705\";}";
        html += ".msg.warn{background:rgba(245,166,35,.12);color:var(--accent);border-color:var(--accent);}.msg.warn::before{content:\"\\26A0\";}";
        html += ".msg.err{background:rgba(255,92,92,.12);color:var(--bad);border-color:var(--bad);}.msg.err::before{content:\"\\274C\";}";

        // Bewusst kein padding-top auf body: die Topbar ist sticky
        // positioniert und wuerde sonst mitverschoben statt nur der Inhalt
        // darunter.

        // Deliberately no padding-top on body: the topbar is sticky-
        // positioned and would otherwise shift down too, not just the
        // content below it.

        html += "body{font-family:Arial,Helvetica,sans-serif;text-align:center;background:var(--bg);color:var(--text);}";
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

        // Statuszeile (Topbar), sitenweit oben auf jeder Seite - siehe
        // generateTopBar() fuer die Markup-Erzeugung.

        // Status bar (topbar), site-wide at the top of every page - see
        // generateTopBar() for the markup generation.

        html += ".topbar{display:flex;flex-wrap:wrap;align-items:center;gap:.6rem 1.2rem;padding:.9rem 1rem;border-bottom:1px solid var(--panel-border);position:sticky;top:0;background:var(--bg);z-index:5;}";
        html += ".brand{display:flex;align-items:baseline;gap:.5rem;margin-right:auto;}";
        html += ".brand-mark{font-family:var(--mono);color:var(--accent);font-size:.8rem;letter-spacing:.05em;}";
        html += ".topbar h1{font-size:1.15rem;margin:0;letter-spacing:.02em;}";
        html += ".topbar h1 a.host-link{color:inherit;text-decoration:none;border-bottom:1px dashed var(--muted);}";
        html += ".topbar h1 a.host-link:hover{color:var(--accent);border-bottom-color:var(--accent);}";
        html += ".ip-hint{font-family:var(--mono);font-size:.7rem;color:var(--muted);}";

        // font-size hier bewusst "inherit" statt weggelassen, um nicht von
        // Vererbung abhaengig zu sein, falls spaeter eine Regel dazwischenfunkt.

        // font-size deliberately "inherit" rather than omitted, to not rely
        // on inheritance in case another rule interferes later.

        html += ".ip-hint a{color:inherit;font-size:inherit;text-decoration:none;border-bottom:1px dashed var(--muted);}";
        html += ".ip-hint a:hover{color:var(--accent);border-bottom-color:var(--accent);}";
        html += ".status-strip{display:flex;gap:.9rem;flex-wrap:wrap;}";
        html += ".status{display:flex;align-items:center;gap:.4rem;font-size:.8rem;color:var(--muted);}";

        // .status setzt "display:flex", was das native 'hidden'-Attribut
        // ueberschreibt (Autoren-CSS gewinnt immer gegen User-Agent-CSS).
        // Diese Regel stellt das 'hidden'-Verhalten explizit wieder her.

        // .status sets "display:flex", which overrides the native 'hidden'
        // attribute (author CSS always wins over user-agent CSS). This rule
        // explicitly restores 'hidden' behavior.

        html += ".status[hidden]{display:none;}";

        // display:inline-block noetig, sonst ignorieren Browser width/height
        // bei einem leeren <span> ausserhalb eines Flex-Kontexts (.status) -
        // der Punkt wuerde eckig statt rund erscheinen.

        // display:inline-block needed, otherwise browsers ignore width/height
        // on an empty <span> outside a flex context (.status) - the dot
        // would render square instead of round.

        html += ".dot{display:inline-block;width:.55rem;height:.55rem;border-radius:50%;background:var(--bad);box-shadow:0 0 0 3px rgba(255,92,92,.15);}";
        html += ".dot.ok{background:var(--ok);box-shadow:0 0 0 3px rgba(61,220,132,.18);}";

        // "na" (grau) gilt nur fuer den Zeit-Punkt (Systemzeit noch nie gesetzt) - bei RTC/DCF77 blendet
        // fehlende Hardware den ganzen Eintrag aus.

        // "na" (gray) only applies to the Time dot (system time never set) - for RTC/DCF77, missing hardware
        // hides the whole entry.

        html += ".dot.na{background:var(--muted);box-shadow:0 0 0 3px rgba(143,155,167,.18);}";

        // Live-Wertanzeige (z.B. Fotowiderstand-Helligkeit) statt farbigem
        // Punkt - heller Monospace-Text, analog zum Kontrast von .datetime
        // gegenueber seinem Muted-Label.

        // Live value reading (e.g. photoresistor brightness) instead of a
        // colored dot - bright monospace text, mirroring the contrast of
        // .datetime against its muted label.

        html += ".statval{font-family:var(--mono);color:var(--text);}";

        // "syncing" (DCF77): Empfaenger bekommt Impulse, hat aber noch kein
        // gueltiges Zeittelegramm dekodiert - gelb blinkend statt rot.

        // "syncing" (DCF77): receiver gets pulses but hasn't decoded a valid
        // time telegram yet - blinking yellow instead of red.

        html += ".dot.syncing{background:var(--accent);box-shadow:0 0 0 3px rgba(245,166,35,.18);animation:dotBlink 1s ease-in-out infinite;}";
        html += "@keyframes dotBlink{0%,100%{opacity:1;}50%{opacity:.25;}}";

        // prefers-reduced-motion: feste gedimmte Opazitaet statt Blinken.
        // prefers-reduced-motion: fixed dimmed opacity instead of blinking.

        html += "@media (prefers-reduced-motion:reduce){.dot.syncing{animation:none;opacity:.6;}}";
        html += ".datetime{font-family:var(--mono);font-size:.8rem;color:var(--muted);}";

        // Zeigt an, dass /api/topbarStatus mehrfach fehlschlug und die
        // angezeigten Werte veraltet sein koennen.

        // Indicates /api/topbarStatus failed repeatedly and the displayed
        // values may be stale.

        html += ".offline-hint{display:none;color:var(--bad);font-size:.75rem;margin-left:.3rem;}";
        html += ".offline-hint.show{display:inline;}";

        // margin/padding hier explizit 0 - sonst greift die allgemeine
        // input/select/button-Regel und sprengt die kompakte Topbar.

        // margin/padding explicitly 0 here - otherwise the general
        // input/select/button rule applies and blows up the compact topbar.

        html += ".reset-btn{background:transparent;border:1px solid var(--panel-border);color:var(--bad);border-radius:.4rem;width:2rem;height:2rem;padding:0;margin:0;font-size:1rem;line-height:1;cursor:pointer;}";
        html += ".reset-btn:hover{border-color:var(--bad);background:rgba(255,92,92,.12);}";
        html += ".reset-btn:focus-visible{outline:2px solid var(--bad);outline-offset:1px;}";

        // Tabs nur mit CSS: Radios ausblenden, Panels per :checked ~ .panel-X einblenden. Radios, .tabnav und
        // .panel-* muessen direkte Geschwister sein.

        // CSS-only tabs: hide radios, show panels via :checked ~ .panel-X. Radios, .tabnav and .panel-* must
        // be direct siblings.

        html += ".tabctrl{display:none;}";
        html += ".tabnav{margin:20px auto;max-width:900px;display:flex;flex-wrap:wrap;justify-content:center;gap:4px;}";
        html += ".tabnav label{background:var(--panel);border:1px solid var(--panel-border);color:var(--muted);padding:8px 16px;border-radius:8px 8px 0 0;cursor:pointer;font-weight:bold;}";
        html += ".tabpanel{display:none;}";
        html += ".card{background:var(--panel);border:1px solid var(--panel-border);border-radius:10px;max-width:500px;margin:15px auto;padding:12px 16px;text-align:left;}";
        for (size_t i = 0; i < SETTINGS_TAB_COUNT; i++) {
            String t = String(SETTINGS_TAB_KEYS[i]);
            html += String("#tab-") + t + ":checked ~ .tabnav label[for='tab-" + t + "']{background:var(--accent);border-color:var(--accent);color:#1a1200;}";
            html += String("#tab-") + t + ":checked ~ .panel-" + t + "{display:block;}";
        }
        html += "</style>" + extraHead + "</head><body>";

        // Statuszeile vor der JS-Warnung, da generateHtmlHeader() von jeder Seite eingebunden wird
        // Status bar before the JS warning, since every page includes generateHtmlHeader()

        html += generateTopBar();

        // Seite benötigt JavaScript
        // Page requires JavaScript

        html += "<noscript><div style='color:red;font-weight:bold;margin:20px;'>" +
                translate("JavaScript is disabled. This page requires JavaScript to work properly!") + "</div></noscript>";

        return html;
    }

    // Status der Topbar-Punkte (fuer generateTopBar() und /api/topbarStatus): "ok" gruen, "syncing" gelb
    // blinkend, "bad" rot, "na" grau bzw. ausgeblendet. Die Systemzeit kennt nur "ok"/"na".

    // Topbar dot status (for generateTopBar() and /api/topbarStatus): "ok" green, "syncing" blinking yellow,
    // "bad" red, "na" gray or hidden. System time only knows "ok"/"na".

    String getTimeStatus() {
        return (timeinfo.tm_year > 0) ? "ok" : "na";
    }


    // Ermittelt den RTC-Status aus rtcOk (siehe globals.h/checkRtcHealth()):
    // AVAILABLE->"ok", AVAILABLE_BUT_INVALID->"bad", NOT_AVAILABLE->"na".
    // Bei "na" blendet generateTopBar() den RTC-Eintrag komplett aus.

    // Determines RTC status from rtcOk (see globals.h/checkRtcHealth()):
    // AVAILABLE->"ok", AVAILABLE_BUT_INVALID->"bad", NOT_AVAILABLE->"na".
    // generateTopBar() fully hides the RTC entry on "na".

    String getRtcStatus() {
        if (rtcOk == RTC_AVAILABLE) return "ok";
        if (rtcOk == RTC_AVAILABLE_BUT_INVALID) return "bad";
        return "na";
    }


    // Ermittelt den DCF77-Status: "ok" wenn zuletzt erfolgreich dekodiert
    // (DCF77_SYNC_STALE_AFTER), sonst "syncing" solange noch Impulse kommen,
    // "na" ohne bestaetigten Empfang (dcf77Confirmed), sonst "bad".

    // Determines DCF77 status: "ok" if last decoded within
    // DCF77_SYNC_STALE_AFTER, else "syncing" while pulses still arrive,
    // "na" without confirmed reception (dcf77Confirmed), else "bad".

    String getDcf77Status() {

        // Ein einzelner Impuls (auch Rauschen) reicht nicht als Nachweis - siehe
        // DCF77_PRESENCE_MIN_STREAK/MAX_GAP_MS.

        // A single pulse (noise too) is no proof - see DCF77_PRESENCE_MIN_STREAK/MAX_GAP_MS.

        if (!dcf77Confirmed) return "na"; // noch keine plausible Impulskette / no plausible pulse chain yet
        bool syncFresh = dcfTimeFound && lastDcfSyncTime != 0 &&
                          (time(nullptr) - lastDcfSyncTime) < (time_t)(DCF77_SYNC_STALE_AFTER / 1000);
        if (syncFresh) return "ok";
        bool pulsesFresh = (lastDcf77PulseChangeMillis != 0) &&
                            (millis() - lastDcf77PulseChangeMillis) < DCF77_PULSE_STALE_AFTER;
        if (pulsesFresh) return "syncing";
        return "bad";
    }


    // Baut den Tooltip-/aria-label-Text fuer einen Status-Punkt, gemeinsam
    // genutzt von generateTopBar() und /api/topbarStatus.

    // Builds the tooltip/aria-label text for a status dot, shared by
    // generateTopBar() and /api/topbarStatus.

    String dotStatusText(const String& label, const String& state) {
        String stateText = (state == "ok") ? translate("OK") :
                            (state == "syncing") ? translate("Syncing") :
                            (state == "na") ? translate("Not available") :
                            translate("Error");
        return label + ": " + stateText;
    }

    // Text sicher fuer HTML escapen, auch Anfuehrungszeichen (Werte stehen in value='...'/onclick='...'). In
    // onclick mit eingebettetem JS-String: aeusseres Zeichen als Entity, inneres per Backslash (HTML wird vor
    // JS dekodiert).

    // Escape text safely for HTML, quotes too (values sit in value='...'/onclick='...'). In onclick with an
    // embedded JS string: outer quote as an entity, inner one with a backslash (HTML is decoded before JS).

    String escapeForJsStringInAttr(const String& text, char jsStringQuote) {
        String out;
        out.reserve(text.length());
        char htmlAttrQuote = (jsStringQuote == '\'') ? '"' : '\'';
        for (size_t i = 0; i < text.length(); i++) {
            char c = text[i];
            if (c == '\\') { out += "\\\\"; }
            else if (c == jsStringQuote) { out += '\\'; out += c; }
            else if (c == htmlAttrQuote) { out += (htmlAttrQuote == '"') ? "&quot;" : "&#39;"; }
            else if (c == '<') { out += "&lt;"; }
            else if (c == '>') { out += "&gt;"; }
            else if (c == '&') { out += "&amp;"; }
            else { out += c; }
        }
        return out;
    }


    // Escaped fuer JS-String-Literale DIREKT in <script> (nicht in einem
    // Attribut, siehe escapeForJsStringInAttr). <script>-Inhalt wird nicht
    // HTML-entity-dekodiert, daher Backslash + "<" als "\x3C" escapen.

    // Escapes for JS string literals DIRECTLY inside <script> (not an
    // attribute, see escapeForJsStringInAttr). <script> content isn't HTML-
    // entity-decoded, so escape backslash plus "<" as "\x3C".

    String escapeForJsStringLiteral(const String& text, char jsStringQuote = '"') {
        String out;
        out.reserve(text.length());
        for (size_t i = 0; i < text.length(); i++) {
            char c = text[i];
            if (c == '\\') { out += "\\\\"; }
            else if (c == jsStringQuote) { out += '\\'; out += c; }
            else if (c == '<') { out += "\\x3C"; }
            else if (c == '\n') { out += "\\n"; }
            else if (c == '\r') { out += "\\r"; }
            else { out += c; }
        }
        return out;
    }


    // Liegt die anfragende IP im GLEICHEN Netz wie die Uhr (STA-Subnetz bzw. AP-Subnetz 192.168.4.0/24)? Nur
    // als Rueckfall die RFC1918-Bereiche. Schuetzt Status und Aktionen vor Zugriff von aussen
    // (DMZ/Port-Weiterleitung).

    // Is the requesting IP in the SAME network as the clock (STA subnet or AP subnet 192.168.4.0/24)? RFC1918
    // ranges only as a fallback. Protects status and actions from outside access (DMZ/port forward).

    bool isSameSubnet(IPAddress ip, IPAddress ownIp, IPAddress mask) {
        for (int i = 0; i < 4; i++) {
            if ((ip[i] & mask[i]) != (ownIp[i] & mask[i])) return false;
        }
        return true;
    }

    // Pruefung abgeschaltet: jeder Zugriff gilt als privat, alle Aktionen sind ohne Bestaetigungscode
    // erlaubt. Aufrufstellen unveraendert - zum Wiedereinschalten PRIVATE_NETWORK_CHECK auf true setzen.

    // Check disabled: every access counts as private, all actions are allowed without a confirmation code.
    // Call sites unchanged - to re-enable, set PRIVATE_NETWORK_CHECK to true.

    constexpr bool PRIVATE_NETWORK_CHECK = false;

    bool isPrivateNetworkIp(IPAddress ip) {
        if (!PRIVATE_NETWORK_CHECK) return true;

        wifi_mode_t mode = WiFi.getMode();

        if ((mode == WIFI_STA || mode == WIFI_AP_STA) && WiFi.status() == WL_CONNECTED) {
            IPAddress mask = WiFi.subnetMask();
            if (mask != IPAddress(0, 0, 0, 0) && isSameSubnet(ip, WiFi.localIP(), mask)) {
                return true;
            }
        }

        if (mode == WIFI_AP || mode == WIFI_AP_STA) {
            if (isSameSubnet(ip, WiFi.softAPIP(), IPAddress(255, 255, 255, 0))) {
                return true;
            }
        }

        if (ip[0] == 10) return true; // 10.0.0.0/8
        if (ip[0] == 192 && ip[1] == 168) return true; // 192.168.0.0/16
        if (ip[0] == 172 && ip[1] >= 16 && ip[1] <= 31) return true; // 172.16.0.0/12
        return false;
    }


    String escapeHtmlText(const String& text) {
        String out;
        out.reserve(text.length());
        for (size_t i = 0; i < text.length(); i++) {
            char c = text[i];
            switch (c) {
                case '&':  out += "&amp;";  break;
                case '<':  out += "&lt;";   break;
                case '>':  out += "&gt;";   break;
                case '\'': out += "&#39;";  break;
                case '"':  out += "&quot;"; break;
                default:   out += c;        break;
            }
        }
        return out;
    }


    // Loescht eine Datei samt Folgen: Presets, die auf ein Zifferblatt oder einen Zeigersatz verweisen, die
    // Streifen-Grafik eines Zifferblatts und neu zu ladende Streifen-Daten. false, wenn es die Datei nicht gibt.

    // Deletes a file including its consequences: presets referring to a clock face or hand set, a clock face's
    // strip graphic and strip data to reload. false if the file does not exist.

    bool deleteFileWithSideEffects(const String& path) {
        if (!LittleFS.exists(path) || isProtectedFile(path)) return false;
        LittleFS.remove(path);

        // Falls ein Zifferblatt oder Teil eines Zeigersatzes geloescht
        // wurde, alle Presets entfernen, die darauf verweisen.

        // If a clock face or part of a hand set was deleted, remove all
        // presets that reference it.

        String name = path.substring(1); // fuehrenden Slash entfernen
                                         // remove the leading slash
        if (name.startsWith("face_") && name.endsWith(".bmp")) {
            removeOrphanedPresets(path, "");

            // Streifen-Grafik und -Einstellungen sowie Zifferblatt-Einstellungen gleich mit loeschen
            // Delete the strip graphic and settings as well as the clock face settings along with it

            for (const String& side : { stripPathForFace(path), stripConfigPath(path), faceConfigPath(path) }) {
                if (side.length() && LittleFS.exists(side)) LittleFS.remove(side);
            }
            stripImageFor = "?";
            stripSettingsFor = "?";
            faceSettingsFor = "?";
            infoStripDirty[0] = infoStripDirty[1] = true;
        }
        else if (name.startsWith("strip_")) {
            stripImageFor = "?";
            infoStripDirty[0] = infoStripDirty[1] = true;
        }
        else if (name.startsWith("stripfont_")) {
            stripVlwStale = true; // VLW-Schrift weg - der Streifen faellt auf GLCD zurueck
                                  // VLW font gone - the strip falls back to GLCD
            infoStripDirty[0] = infoStripDirty[1] = true;
        }
        else if (name.startsWith("hand_set") && name.endsWith(".bmp")) {
            int start = 8; // Laenge von "hand_set"
                           // length of "hand_set"
            int end = name.indexOf('_', start);
            if (end > start) {
                String setId = name.substring(start, end);
                removeOrphanedPresets("", setId);

                // Falls der betroffene Zeigersatz gerade aktiv war, sofort auf den Standardsatz zurueckschalten
                // If the affected hand set was currently active, switch back to the default set immediately

                if (handSetFileId(preferences.getString(PK_HANDSET, "")) == setId) {
                    preferences.putString(PK_HANDSET, "default");
                    freeClockFaceBuffer();
                    loadClockFace();
                    loadHandSprites();
                    updateClock();
                }
            }
        }
        return true;
    }


    // Seite, zu der /delete und /rename zurueckspringen: /files und /listfilesFaces haengen ein explizites
    // "from" an. Unbekannt/fehlend -> allgemeiner Dateimanager.

    // Page /delete and /rename return to: /files and /listfilesFaces attach an explicit "from".
    // Unknown/missing -> general file manager.

    String fileManagerReturnTarget(const String& from) {
        if (from == "listfilesFaces") return "/listfilesFaces";
        if (from == "handsets") return "/handsets";
        return "/files";
    }


    // Wie escapeHtmlText(), aber fuer String-Werte in JSON-Antworten - noetig
    // bei Feldern mit Nutzereingaben (Rocrail-Hostname, Vorschau-Fingerabdruck),
    // da ein Anfuehrungszeichen sonst die JSON-Antwort zerbrechen koennte.

    // Like escapeHtmlText(), but for string values in JSON responses -
    // needed for fields with user input (Rocrail hostname, preview
    // fingerprint), since a quote character could otherwise break the JSON.

    String escapeJsonText(const String& text) {
        String out;
        out.reserve(text.length());
        for (size_t i = 0; i < text.length(); i++) {
            char c = text[i];
            switch (c) {
                case '"':  out += "\\\""; break;
                case '\\': out += "\\\\"; break;
                default:   out += c;      break;
            }
        }
        return out;
    }


    // Erzeugt die Statuszeile (Topbar): Geraetename, Hostname/IP, je ein
    // Status-Punkt pro Zeitquelle (Zeit/RTC/DCF77), Helligkeitswert, Uhrzeit,
    // Neu-Laden/Neustart. Kein WLAN-Punkt - ohne WLAN laedt die Seite gar nicht.

    // Generates the status bar (topbar): device name, hostname/IP, one
    // status dot per time source (time/RTC/DCF77), brightness, time,
    // reload/restart. No WiFi dot - the page can't load without WiFi.

    String generateTopBar() {
        String html;
        html.reserve(3600); // Platz fuer Punkte, Tooltips und Live-Status-Skript
                            // Room for dots, tooltips and the live-status script

        html += "<header class='topbar'>";
        html += "<div class='brand'>";
        html += "<span class='brand-mark'>UHR&middot;4</span>";

        bool staConnected = (WiFi.getMode() == WIFI_STA && WiFi.status() == WL_CONNECTED);

        if (staConnected && pingHostname) {
            html += "<h1><a class='host-link' href='http://" + String(hostname) + ".local/'>" + String(hostname) + "</a></h1>";
            html += "<span class='ip-hint'><a href='http://" + WiFi.localIP().toString() + "/'>" + WiFi.localIP().toString() + "</a></span>";
        }
        else if (staConnected) {

            // Hostname noch nicht bestaetigt - SSID als Platzhalter zeigen.
            // Hostname not confirmed yet - show the SSID as a placeholder.

            html += "<h1>" + WiFi.SSID() + "</h1>";
            html += "<span class='ip-hint'><a href='http://" + WiFi.localIP().toString() + "/'>" + WiFi.localIP().toString() + "</a></span>";
        }
        else {

            // AP-/Setup-Modus: kein mDNS-Hostname. AP-Passwort bewusst nicht
            // hier - steht nur auf dem Display, siehe startAP() in wifi_manager.h.

            // AP/setup mode: no mDNS hostname. AP password deliberately not
            // shown here - only on the display, see startAP() in wifi_manager.h.

            html += "<h1>Access Point</h1>";
            html += "<span class='ip-hint'><a href='http://" + WiFi.softAPIP().toString() + "/'>" + WiFi.softAPIP().toString() + "</a></span>";
        }

        // Chip und Displaytyp neben der Adresse (Schreibweise wie in flashESP und der Displayauswahl)
        // Chip and display type next to the address (spelling as in flashESP and the display selection)

        html += "<span class='ip-hint' title='Chip / " + translate("Display type") + "'>" + String(CHIP_SHORT_NAME) + " " + String(displayChoiceName(displayType, useBacklight)) + "</span>";
        html += "</div>";

        html += "<div class='status-strip'>";

        // IDs und title/aria-label an jedem Punkt - das Status-Skript aktualisiert Farbe und Text (auch fuer
        // Screenreader). "Zeit" steht zuletzt neben dem Datum; timeState wird hier schon fuer
        // #topbar-datetime berechnet.

        // IDs and title/aria-label on every dot - the status script updates color and text (for screen
        // readers too). "Time" comes last next to the date; timeState is computed here already for
        // #topbar-datetime.

        String timeState = getTimeStatus();
        bool timeOk = (timeState == "ok"); // fuer die Datumsanzeige weiter unten wiederverwendet / reused for the date display further below
        String timeTitle = dotStatusText(translate("Time"), timeState);

        // Live-Helligkeitswert statt Status-Punkt. Ohne useAdc (Spannungsteiler
        // unbestromt, siehe uhr4.ino) den Eintrag lieber weglassen statt einen
        // eingefrorenen Wert zu zeigen.

        // Live brightness value instead of a status dot. Without useAdc
        // (voltage divider unpowered, see uhr4.ino) omit the entry rather
        // than show a stale value.

        if (photoresistorFound && useAdc) {
            html += "<span class='status' id='status-light'>" + translate("Light") + ": <span id='value-light' class='statval'>" + String(currentLightPercent) + " %</span></span>";
        }

        String rtcState = getRtcStatus();

        // rtcPresent false nur bei RTC_NOT_AVAILABLE - Eintrag bleibt dann
        // per 'hidden' unsichtbar statt eines grauen "na"-Punkts.

        // rtcPresent false only on RTC_NOT_AVAILABLE - the entry then stays
        // invisible via 'hidden' instead of a gray "na" dot.

        bool rtcPresent = (rtcState != "na");
        String rtcTitle = dotStatusText("RTC", rtcState);
        html += "<span class='status' id='status-rtc'" + String(rtcPresent ? "" : " hidden") + "><i id='dot-rtc' role='img' aria-label='" + rtcTitle + "' title='" + rtcTitle + "' class='dot";
        if (rtcState == "ok") html += " ok";
        html += "'></i>RTC</span>";

        String dcfState = getDcf77Status();

        // dcfPresent false bis der erste Impuls beobachtet wurde (dcfState==
        // "na") - Eintrag bleibt per 'hidden' unsichtbar statt grauem Punkt,
        // wird dann per setPresent() live eingeblendet und bleibt sichtbar.

        // dcfPresent false until the first pulse is observed (dcfState==
        // "na") - the entry stays 'hidden' instead of a gray dot, then gets
        // revealed live via setPresent() and stays visible.

        bool dcfPresent = (dcfState != "na");
        String dcfTitle = dotStatusText("DCF77", dcfState);
        html += "<span class='status' id='status-dcf77'" + String(dcfPresent ? "" : " hidden") + "><i id='dot-dcf77' role='img' aria-label='" + dcfTitle + "' title='" + dcfTitle + "' class='dot";
        if (dcfState == "ok") html += " ok";
        else if (dcfState == "syncing") html += " syncing";
        html += "'></i>DCF77</span>";

        // Nur sichtbar, wenn der Rocrail-Master-Schalter (Zifferblatt-Tab)
        // aktiv ist - gruen bei bestehender Verbindung, sonst rot (Basis-
        // ".dot" ohne "ok"-Klasse ist bereits rot, siehe CSS oben).

        // Only visible when the Rocrail master switch (Clock Setup tab) is
        // on - green while connected, red otherwise (the base ".dot"
        // without the "ok" class is already red, see CSS above).

        String rocrailTitle = dotStatusText("Rocrail", rocrailConnected ? "ok" : "error");
        html += "<span class='status' id='status-rocrail'" + String(rocrailEnabled ? "" : " hidden") + "><i id='dot-rocrail' role='img' aria-label='" + rocrailTitle + "' title='" + rocrailTitle + "' class='dot";
        if (rocrailConnected) html += " ok";
        html += "'></i>Rocrail</span>";

        // "Zeit"-Punkt letzter Eintrag, direkt vor #topbar-datetime.
        // "Time" dot is the last entry, right before #topbar-datetime.

        html += "<span class='status' id='status-time'><i id='dot-time' role='img' aria-label='" + timeTitle + "' title='" + timeTitle + "' class='dot";
        if (timeState == "ok") html += " ok";
        else if (timeState == "na") html += " na";
        html += "'></i>" + translate("Time") + "</span>";

        html += "</div>";

        // Immer rendern (auch leer), damit das Live-Status-Skript den Text
        // per Poll aktualisieren kann, ohne die Seite neu zu laden.

        // Always rendered (even empty) so the live-status script can update
        // the text on each poll without reloading the page.

        html += "<div id='topbar-datetime' class='datetime'>";
        if (timeOk) {
            char nowStr[20];
            strftime(nowStr, sizeof(nowStr), "%d.%m.%Y %H:%M", &timeinfo);
            html += String(nowStr);
        }
        html += "</div>";

        // Ohne Uhrzeit (kein WLAN/NTP, keine RTC, kein DCF77): Knopf, der die Uhrzeit dieses Geraets uebernimmt -
        // auch im Access-Point-Modus. Verschwindet, sobald die Uhr eine Zeit hat (Status-Skript unten).

        // Without a time (no WiFi/NTP, no RTC, no DCF77): button that takes over this device's time - also in
        // access point mode. Disappears as soon as the clock has a time (status script below).

        if (isPrivateNetworkIp(webserver.client().remoteIP())) {
            html += "<button type='button' id='topbar-settime' onclick='setClockFromDevice(this)' style='padding:2px 8px;font-size:0.85em;' title='" +
                    translate("Sets the clock to the time of this device") + "'" + String(timeOk ? " hidden" : "") + ">&#128339; " +
                    translate("Use device time") + "</button>";
        }

        // Versteckter Hinweis, siehe ".offline-hint" in generateHtmlHeader().
        // Hidden hint, see ".offline-hint" in generateHtmlHeader().

        html += "<span id='topbar-offline-hint' class='offline-hint'>&#9888; " + translate("Connection lost") + "</span>";

        // Kein Refresh-Knopf/Auto-Refresh - alle Werte aktualisieren sich
        // ohnehin per JS. Neustart bleibt (echter Geraete-Neustart).

        // No refresh button/auto-refresh - all values already update via
        // JS. Restart stays (an actual device restart).

        html += "<button type='button' class='reset-btn' onclick='if(confirm(\"" + translate("Are you sure you want to reboot?") + "\")){location.href=\"/reboot\";}' title='" + translate("Reboot") + "'>&#9211;</button>";

        html += "</header>";

        // Live-Status: pollt /api/topbarStatus alle 5 s ohne Reload, Offline-Hinweis erst nach zwei
        // Fehlschlaegen, Pause im Hintergrund. Meldet ein Poll eine andere Build-Version (pageVersion), laedt
        // die Seite komplett neu.

        // Live status: polls /api/topbarStatus every 5 s without reload, offline hint only after two
        // failures, paused in the background. If a poll reports a different build version (pageVersion), the
        // page reloads completely.

        html += "<script>(function(){";

        // "version" ist ein reiner Build-Zeitstempel ohne Anfuehrungszeichen
        // o.ae. (siehe globals.h) - daher hier ohne Escaping direkt als
        // JS-String-Literal eingebettet, wie auch beim JSON oben.

        // "version" is a plain build timestamp with no quotes etc. (see
        // globals.h) - so it's embedded here directly as a JS string
        // literal without escaping, same as in the JSON above.

        html += "var pageVersion=\"" + String(version) + "\";";
        html += "function setStatusDot(id,state,title){var el=document.getElementById(id);if(!el)return;el.classList.toggle('ok',state==='ok');el.classList.toggle('syncing',state==='syncing');el.classList.toggle('na',state==='na');if(title){el.title=title;el.setAttribute('aria-label',title);}}";

        // setPresent(): blendet einen Eintrag der Statuszeile (RTC, DCF77, Rocrail) live ein oder aus,
        // je nachdem, was der Server meldet.

        // setPresent(): shows or hides a status bar entry (RTC, DCF77, Rocrail) live, depending on what
        // the server reports.

        html += "function setPresent(id,present){var el=document.getElementById(id);if(!el)return;el.hidden=!present;}";

        // setValue(): aktualisiert eine Live-Wertanzeige statt einer Punktfarbe.
        // setValue(): updates a live value reading instead of a dot color.

        html += "function setValue(id,text){var el=document.getElementById(id);if(!el)return;el.textContent=text;}";

        // Bei Verbindungsverlust wuerden Licht/Zeit/Datum/RTC/DCF77 sonst
        // eingefrorene, veraltete Werte zeigen - waehrend des Aussetzers
        // lieber ganz ausblenden statt einen falschen Eindruck von Aktualitaet zu erwecken.

        // On connection loss, Light/Time/Date/RTC/DCF77 would otherwise
        // keep showing frozen, stale values - better to hide them entirely
        // for the outage than give a false impression of freshness.

        html += "function setOnline(ok){var h=document.getElementById('topbar-offline-hint');if(h)h.classList.toggle('show',!ok);";
        html += "['status-light','status-time','topbar-datetime','status-rtc','status-dcf77','status-rocrail'].forEach(function(id){var el=document.getElementById(id);if(el)el.hidden=!ok;});}";
        html += "var failCount=0;";
        html += "function poll(){fetch('/api/topbarStatus').then(function(r){return r.json();}).then(function(s){";
        html += "failCount=0;setOnline(true);";
        html += "if(s.version&&pageVersion&&s.version!==pageVersion){location.reload();return;}";
        html += "setStatusDot('dot-time',s.time,s.timeTitle);";
        html += "setPresent('status-rtc',s.rtcPresent);setStatusDot('dot-rtc',s.rtc,s.rtcTitle);";
        html += "setPresent('status-dcf77',s.dcf77Present);setStatusDot('dot-dcf77',s.dcf77,s.dcf77Title);";
        html += "setPresent('status-rocrail',s.rocrailEnabled);setStatusDot('dot-rocrail',s.rocrailConnected?'ok':'error',s.rocrailTitle);";

        html += "setValue('value-light',s.lightValue);";
        html += "var dt=document.getElementById('topbar-datetime');if(dt)dt.textContent=s.datetime;";
        html += "var st=document.getElementById('topbar-settime');if(st)st.hidden=(s.time!=='na');";
        html += "}).catch(function(){failCount++;if(failCount>=2)setOnline(false);});}";

        // setClockFromDevice(): Uhrzeit dieses Geraets an die Uhr senden (/api/setTime) - Knopf in der
        // Statusleiste und im Tab NTP Zeitzone
        // setClockFromDevice(): send this device's time to the clock (/api/setTime) - button in the status bar
        // and in the NTP timezone tab

        html += "window.setClockFromDevice=function(b){if(b)b.disabled=true;";
        html += "fetch('/api/setTime',{method:'POST',body:new URLSearchParams({t:(Date.now()/1000).toFixed(3)})}).then(function(r){return r.json();})";
        html += ".then(function(j){if(!j.ok)throw 0;alert('" + translate("Time set") + ": '+j.time);poll();})";
        html += ".catch(function(){alert('" + translate("Time could not be set") + "');}).then(function(){if(b)b.disabled=false;});};";
        html += "var pollTimer=null,paused=false;";
        html += "function startPolling(){if(pollTimer)return;poll();pollTimer=setInterval(poll,5000);}";
        html += "function stopPolling(){if(!pollTimer)return;clearInterval(pollTimer);pollTimer=null;}";
        html += "document.addEventListener('visibilitychange',function(){if(document.hidden||paused){stopPolling();}else{startPolling();}});";

        // topbarPolling(false): Pause, solange die Uhr mit einer langen Anfrage beschaeftigt ist (Sichern,
        // Wiederherstellen) - die Abfragen kaemen nicht durch und meldeten "Verbindung verloren".

        // topbarPolling(false): pause while the clock is busy with a long request (backup, restore) - the
        // polls would not get through and report "Connection lost".

        html += "window.topbarPolling=function(on){paused=!on;failCount=0;if(paused)stopPolling();else if(!document.hidden)startPolling();};";
        html += "if(!document.hidden)startPolling();";
        html += "})();</script>";

        return html;
    }


    // Tagesfenster wie in updateBrightness() (display.h): Stunde >= Start und < Ende, also Ende exklusiv -
    // aus den Laufzeitwerten, damit die Anzeige zur tatsaechlich verwendeten Einstellung passt.

    // Day window like in updateBrightness() (display.h): hour >= start and < end, i.e. end exclusive - from
    // the runtime values, so the display matches the setting actually in use.

    String dayWindowText() {
        if (brightStartHour == brightEndHour) return "off (start = end)";
        String text = String(brightStartHour) + ":00 - " + String(brightEndHour) + ":00";
        if (brightStartHour > brightEndHour) text += " (over midnight)";
        return text;
    }

    // Kurze Zeile mit der LittleFS-Belegung, reiner Text - der Aufrufer bettet ihn ein. forceEnglish fuer die
    // immer englische Status-Seite.

    // Short line with LittleFS usage, plain text - the caller embeds it. forceEnglish for the always-English
    // status page.

    String generateStorageInfo(size_t used, size_t total, bool forceEnglish) {
        String usedLabel = forceEnglish ? "Storage used" : translate("Storage used");
        String freeLabel = forceEnglish ? "Free" : translate("Free");
        String html = usedLabel + ": " + String(used / 1024) + " KB / " + String(total / 1024) + " KB";
        html += " (" + freeLabel + ": " + String((total - used) / 1024) + " KB)";
        return html;
    }


    // Einfache Hinweisseite (Erfolg/Fehler/Status) im Dark-Theme.
    // Simple message page (success/error/status) in dark theme.

    String simpleMessagePage(String heading, String bodyHtml, String extraHead) {
        String html = generateHtmlHeader(extraHead);
        html += "<div class='card' style='max-width:480px;'>";
        html += "<h2>" + heading + "</h2>";
        html += bodyHtml;
        html += "</div></body></html>";
        return html;
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

            // WLAN/Zeit/Helligkeit/Status sind Tabs auf "/" und fehlen daher hier - die Routen bleiben fuer
            // Lesezeichen.

            // WiFi/time/brightness/status are tabs on "/" and therefore missing here - the routes stay for
            // bookmarks.

            {"/", translate("Main"), ""},
            {"/preview", translate("Preview"), ""},
            {"/presets", translate("Presets"), ""},
            {"/listfilesFaces", translate("Clock&nbsp;Face"), ""},
            {"/handsets", translate("Hand&nbsp;Set"), ""},
            {"/files", translate("File&nbsp;Manager"), ""},

            // "/reboot" fehlt bewusst (reset-btn in der Topbar), die Route bleibt fuer Lesezeichen. "DCF77"
            // bewusst unuebersetzt (Protokollname, wie in generateTopBar()).

            // "/reboot" deliberately missing (reset-btn in the topbar), the route stays for bookmarks.
            // "DCF77" deliberately untranslated (a protocol name, as in generateTopBar()).

            {"/dcf77", "DCF77", ""},
            {"/backup", translate("Backup"), ""},
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

            // Zeilenumbruch zur thematischen Trennung, bewusst im Code statt in der Uebersetzung
            // Line break for thematic separation, deliberately in code rather than the translation

            if (item.path == "/status") {
                nav += "<br>";
            }
        }

        nav += "</div>"; // Ende .navLinks
                         // end .navLinks
        nav += "</div>";
           
        return nav;
    }


    // Baut einen Fingerabdruck aus allen fuer /preview sichtbaren
    // Einstellungen (Zifferblatt, Zeigersatz, Nabe, Sekundenzeiger). Nicht
    // clockAssetGeneration verwendet - die zaehlt auch bei Helligkeits-Rampenschritten hoch.

    // Builds a fingerprint from every setting visible in /preview (face,
    // hand set, hub, second hand). Not using clockAssetGeneration - that
    // also increments on brightness ramp steps.

    String currentPreviewSignature() {
        String sig = preferences.getString(PK_BACKGROUND, "/face_default.bmp");
        sig += "|" + preferences.getString(PK_HANDSET, "");
        sig += "|" + String(hubColor);
        sig += "|" + String(hubSize);
        sig += "|" + String(preferences.getBool(PK_SHOW_SECOND_HAND, true) ? 1 : 0);
        return sig;
    }


    // Baut die Tab-Leiste der Einstellungen als <label>-Elemente der CSS-
    // Tab-Mechanik (siehe generateHtmlHeader()).

    // Builds the settings tab bar as <label> elements of the CSS tab
    // mechanism (see generateHtmlHeader()).

    String generateSettingsTabNav() {
        String nav = "<div class='tabnav'>";
        for (size_t i = 0; i < SETTINGS_TAB_COUNT; i++) {
            String key = String(SETTINGS_TAB_KEYS[i]);

            // Rocrail-Tab nur anzeigen, wenn der Master-Schalter aktiv ist -
            // kein Live-Nachladen noetig, da das Umlegen des Schalters
            // ohnehin einen vollen Seitenneuaufbau ausloest.

            // Only show the Rocrail tab when the master switch is on - no
            // live reload needed since flipping the switch already
            // triggers a full page reload anyway.

            if (key == "rocrail" && !rocrailEnabled) continue;

            String label = translate(SETTINGS_TAB_LABELS[i]);
            nav += "<label for='tab-" + key + "'>" + label + "</label>";
        }

        nav += "</div>";
        return nav;
    }


    // Setzt den Pin der Hintergrundbeleuchtung in einen uebersetzten Text ein ("{pin}", je Board verschieden)
    // Inserts the backlight pin into a translated text ("{pin}", different per board)

    String withBacklightPin(String text) {
        text.replace("{pin}", String(TFT_Backlight));
        return text;
    }

    // ⓘ-Hinweis mit Erklaerung als Tooltip (title) - der Punkt am Satzende kommt hier dazu
    // ⓘ note with an explanation as tooltip (title) - the full stop at the end is added here

    String infoTip(const String& text) {
        return "<span title='" + text + ".' style='cursor:help;'>&#9432;</span>";
    }


    // Checkbox-Zeile der Einstellungen: Haken, Beschriftung und ⓘ-Hinweis; id optional (z.B. fuer Skripte)
    // Settings checkbox row: tick, label and ⓘ note; id optional (e.g. for scripts)

    String checkboxRow(const char* name, bool checked, const String& label, const String& tip, const char* id) {
        String html = "<div style='display:flex;align-items:center;gap:6px;white-space:nowrap;'><input type='checkbox' name='";
        html += name;
        html += "'";
        if (*id) {
            html += " id='";
            html += id;
            html += "'";
        }
        html += " value='1' ";
        html += checked ? "checked" : "";
        html += " style='width:auto;margin:0;'>" + label + " " + infoTip(tip) + "</div><br>";
        return html;
    }


    // Zeigt nach einer Weiterleitung eine einheitliche Meldung (translate()-Schluessel): "msg" Erfolg, blendet
    // sich nach 4 s aus - "warn" Hinweis und "err" Fehler bleiben stehen.

    // Shows a uniform message after a redirect (translate() key): "msg" success, fades out after 4 s - "warn"
    // note and "err" error stay.

    String generateFlashMessage() {
        String kind = webserver.hasArg("err") ? "err" : (webserver.hasArg("warn") ? "warn" : "ok");
        String param = (kind == "ok") ? "msg" : kind;
        if (!webserver.hasArg(param)) return "";
        String rawMsg = webserver.arg(param);
        String message = translate(rawMsg);

        // escapeHtmlText() nur anwenden, wenn KEINE Uebersetzung stattfand -
        // "msg" kommt aus der URL, sonst ein XSS-Einfallstor. Eine echte
        // Uebersetzung enthaelt bewusst HTML-Entities, die sonst kaputt-escaped wuerden.

        // Apply escapeHtmlText() only when NO translation happened - "msg"
        // comes from the URL, otherwise an XSS entry point. A genuine
        // translation deliberately contains HTML entities, which would otherwise get escaped and broken.

        if (message == rawMsg) {
            message = escapeHtmlText(message);
        }

        // Der WPS-Banner bleibt sichtbar, solange WPS tatsaechlich aktiv ist
        // (bis 2 Min., per /api/wpsStatus-Polling statt fester Zeit) -
        // Notabschaltung nach 3 Min., falls das Polling selbst nicht mehr durchkommt.

        // The WPS banner stays visible as long as WPS is actually active (up
        // to 2 min, via /api/wpsStatus polling instead of a fixed time) -
        // 3-minute safety cutoff in case polling itself can't get through.

        bool isWpsBanner = (rawMsg == "WPS active - press the WPS button on your router now. Connection to the clock may be lost for about 2 minutes while this happens");

        String html = "<div id='flashMsg' class='msg " + kind + "'>" + message + "</div>";
        if (isWpsBanner) {

            // Waehrend WPS laeuft, ist die Uhr kurz nicht erreichbar - jeder
            // fetch() schlaegt erwartungsgemaess fehl und wird per catch()
            // wiederholt. AbortController begrenzt jeden Versuch auf 3s, damit ein haengender fetch() die Wiederholung nicht blockiert.

            // While WPS runs, the clock is briefly unreachable - every
            // fetch() is expected to fail and gets retried via catch().
            // AbortController caps each attempt at 3s, so a hanging fetch() doesn't block the retry.

            html += "<script>(function(){var e=document.getElementById('flashMsg');var startedAt=Date.now();function poll(){if(!e)return;if(Date.now()-startedAt>180000){e.style.display='none';return;}var ctrl=(typeof AbortController!=='undefined')?new AbortController():null;var timer=ctrl?setTimeout(function(){ctrl.abort();},3000):null;fetch('/api/wpsStatus',ctrl?{signal:ctrl.signal}:{}).then(function(r){if(timer)clearTimeout(timer);return r.json();}).then(function(d){if(!d.pending){e.style.display='none';}else{setTimeout(poll,1000);}}).catch(function(){if(timer)clearTimeout(timer);setTimeout(poll,1000);});}poll();})();</script>";
        }
        else if (kind == "ok") {
            html += "<script>setTimeout(function(){var e=document.getElementById('flashMsg'); if(e) e.style.display='none';}, 4000);</script>";
        }
        return html;
    }


    // Auswahl eines Zifferblatts oder Zeigersatzes ohne die Seite neu zu laden: pickItem() ruft den Link per fetch() mit
    // &ajax=1 auf, setzt den Hinweis "(aktiv)" im angeklickten Feld (Klasse cls) um und zeigt die Meldung. Schlaegt
    // der Aufruf fehl, folgt der Browser dem Link wie bisher.

    // Selecting a clock face or hand set without reloading the page: pickItem() calls the link via fetch() with
    // &ajax=1, moves the "(active)" note to the clicked item (class cls) and shows the message. If the call
    // fails, the browser follows the link as before.

    String selectWithoutReloadScript(const String& message, const String& activeText) {
        String html = "<div id='selMsg' class='msg ok' style='display:none' data-act='" + activeText + "'>" + message + "</div>";
        html += "<script>function pickItem(a,cls){fetch(a.href+'&ajax=1').then(function(r){if(!r.ok)throw 0;"
                "var m=document.getElementById('selMsg');"
                "document.querySelectorAll('.'+cls).forEach(function(e){e.textContent='';});"
                "a.parentNode.querySelector('.'+cls).textContent=' ('+m.dataset.act+')';"
                "m.style.display='block';clearTimeout(window.selT);window.selT=setTimeout(function(){m.style.display='none';},4000);"
                "}).catch(function(){location.href=a.href;});return false;}</script>";
        return html;
    }


    // Sprachselector generieren
    // Generate the language selector

    String generateLanguageSelector() {
        String html = "<form method='POST' action='/setLanguage'>";
        html.reserve(512);  // Sprachauswahl: klein
                            // language selector: small
        html += "<label for='lang'>Language/Sprache:</label>";
        html += "<select name='lang' onchange='this.form.submit()'>";
        html += "<option value='en'" + String(currentLanguage == "en" ? " selected" : "") + ">English / Englisch</option>";
        html += "<option value='de'" + String(currentLanguage == "de" ? " selected" : "") + ">German / Deutsch</option>";
        html += "</select>";
        html += "<noscript><button type='submit'>Save / Speichern</button></noscript>";
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


    // Bereinigt eine Eingabe zu einem gueltigen Hostnamen (RFC 952/1123): nur
    // Buchstaben/Ziffern/Bindestriche, kein Bindestrich am Rand, max. 30 Zeichen.

    // Sanitizes input into a valid hostname (RFC 952/1123): only
    // letters/digits/hyphens, no hyphen at the edges, max. 30 characters.

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

        // Im AP-Modus zaehlt jede Formular-Aktion als Nutzung und verlaengert die Frist bis zum
        // 15-Minuten-Neustart - er soll keine laufende Einrichtung unterbrechen.

        // In AP mode every form action counts as use and extends the deadline of the 15-minute restart - it
        // should not interrupt an ongoing setup.

        if (softAPIP) softAPIPstart = millis();

        // Zugriffs-IP mitloggen fuer Formular-Aktionen (beginPage() deckt Seitenaufrufe ab) - bewusst nicht
        // fuer die Hintergrund-Polls der APIs, die das Log nur fuellen wuerden.

        // Log the accessing IP for form actions (beginPage() covers page views) - deliberately not for the
        // APIs' background polls, which would only fill the log.

        DEBUG_PRINTLN("[WEB] " + webserver.client().remoteIP().toString() + " -> " + webserver.uri());

        webserver.sendHeader("Location", location, true);
        webserver.send(302, "text/plain", body);
    }


    // Erzeugt den fuer fast jede Seite gleichen Seitenanfang (Header inkl. Topbar + Navigation).
    // Generates the page start common to almost every page (header incl. topbar + navigation).

    String beginPage() {

        // Gleicher Grund wie in redirectTo() - deckt normale Seitenaufrufe ab
        // (GET), redirectTo() deckt Formular-Aktionen (POST->redirect) ab.

        // Same reason as in redirectTo() - covers normal page views (GET),
        // redirectTo() covers form actions (POST->redirect).

        if (softAPIP) softAPIPstart = millis();

        // Zugriffs-IP mitloggen - siehe ausfuehrlichen Kommentar in redirectTo().
        // Log the accessing IP too - see the detailed comment in redirectTo().

        DEBUG_PRINTLN("[WEB] " + webserver.client().remoteIP().toString() + " -> " + webserver.uri());

        String html = generateHtmlHeader();
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


    // Uebernimmt eine komplette WLAN-Liste (SSID + Passwort je Slot) in Preferences und
    // wifiSsid[]/wifiPass[]. Leeres Passwort = unveraendert, leere SSID = geloescht. Gemeinsam fuer /save und
    // die bestaetigte Aktion "wlanOverwriteActive".

    // Applies a complete WiFi list (SSID + password per slot) to Preferences and wifiSsid[]/wifiPass[]. Empty
    // password = unchanged, empty SSID = deleted. Shared by /save and the confirmed action
    // "wlanOverwriteActive".

    void applyWlanList(String newSsid[MAX_WLAN], String newPass[MAX_WLAN]) {

        // Effektiver SSID/Passwort-Wert je Slot, zunaechst nur im RAM (leeres
        // Passwortfeld behaelt das gespeicherte). Preferences erst unten, nach
        // dem Kompaktieren, in EINEM Durchlauf beschreiben.

        // Effective SSID/password per slot, computed in RAM first (empty
        // password field keeps the stored one). Preferences are written only
        // below, after compaction, in a SINGLE pass.

        String effectiveSsid[MAX_WLAN];
        String effectivePass[MAX_WLAN];

        for (int i = 0; i < MAX_WLAN; i++) {
            effectiveSsid[i] = newSsid[i];
            if (newPass[i] != "") {
                effectivePass[i] = newPass[i];
            } else {
                effectivePass[i] = loadWifiPass(i);
            }
        }

        // leere Einträge aussortieren
        // Filter out empty entries

        String tempSsid[MAX_WLAN];
        String tempPass[MAX_WLAN];

        int j = 0;
        for (int i = 0; i < MAX_WLAN; i++) {
            if (trim(effectiveSsid[i]).length() > 0) {
                tempSsid[j] = effectiveSsid[i];
                tempPass[j] = effectivePass[i];
                j++;
            }
        }

        // Einziger Schreibdurchlauf: erst hier, mit dem kompaktierten
        // Endergebnis, in Preferences uebernehmen - Lesen-vor-Schreiben spart
        // unveraenderten Slots einen Flash-Schreibvorgang (putStringVerified()).

        // Single write pass: only now, with the already-compacted final
        // result, commit to preferences - read-before-write saves an
        // unchanged slot a flash write (see putStringVerified()).

        for (int i = 0; i < MAX_WLAN; i++) {
            String ssidKey = pkSsid(i);

            if (preferences.getString(ssidKey.c_str(), "") != tempSsid[i]) {
                putStringVerified(ssidKey.c_str(), tempSsid[i]);
            }
            if (loadWifiPass(i) != tempPass[i]) {
                storeWifiPassVerified(i, tempPass[i]);
            }

            wifiSsid[i] = tempSsid[i];
            wifiPass[i] = tempPass[i];
        }
    }


    // Fuehrt eine der acht bestaetigten bzw. direkt erlaubten Aktionen aus - aus /factoryReset/confirm
    // ausgelagert, damit Anfragen aus dem privaten Netz sie ohne Code aufrufen koennen. Die Nutzlast setzt
    // der Aufrufer vorher.

    // Executes one of the eight confirmed or directly allowed actions - factored out of /factoryReset/confirm
    // so requests from the private network can call it without a code. The caller sets the payload
    // beforehand.

    void executePendingAction(String action) {

        // Einen noch anhaengigen fremden Bestaetigungscode verwerfen - sonst zeigte das Display weiter den
        // Code statt der Uhrzeit, obwohl bereits eine andere Aktion ausgefuehrt wurde (ueber
        // /factoryReset/confirm ohnehin leer).

        // Discard a still-pending unrelated confirmation code - otherwise the display would keep showing the
        // code instead of the time, although a different action has already been executed (already empty via
        // /factoryReset/confirm).

        factoryResetCode = "";
        factoryResetPendingAction = "";
        factoryResetCodeAttempts = 0;

        if (action == "all") {
            factoryReset();
        }
        else if (action == "wifi") {
            eraseWiFiConfig();
            delay(WAIT_1s);
            espReboot();
        }
        else if (action == "faces") {
            resetFacesToDefault();
            addStarterPresets(); // der Reset raeumt sie als verwaist mit ab / the reset removes them as orphaned
            redirectTo("/factoryReset?msg=Clock%20faces%20deleted");
        }
        else if (action == "hands") {
            resetHandsToDefault();
            addStarterPresets();
            redirectTo("/factoryReset?msg=Hand%20sets%20deleted");
        }
        else if (action == "presets") {
            resetAllPresets();
            addStarterPresets();
            redirectTo("/factoryReset?msg=Presets%20deleted");
        }
        else if (action == "wlanDeleteActive") {

            // Slot loeschen und die Liste kompaktieren (wie in /deletewifi, ueber applyWlanList()), danach
            // neu starten - es waren die Zugangsdaten der laufenden Verbindung.

            // Delete the slot and compact the list (as in /deletewifi, via applyWlanList()), then reboot -
            // these were the credentials of the running connection.

            int idx = pendingWifiChangeIndex;
            pendingWifiChangeIndex = -1;

            if (idx >= 0 && idx < MAX_WLAN) {
                String newSsid[MAX_WLAN];
                String newPass[MAX_WLAN];
                for (int i = 0; i < MAX_WLAN; i++) {
                    newSsid[i] = preferences.getString(pkSsid(i).c_str(), "");
                    newPass[i] = loadWifiPass(i);
                }
                newSsid[idx] = "";
                newPass[idx] = "";
                applyWlanList(newSsid, newPass);

                // PK_LAST_WLAN komplett entfernen statt auf einen Index zu
                // setzen - nach dem Kompaktieren durch applyWlanList() koennte
                // der Index sonst ein anderes, verschobenes Netzwerk bezeichnen.

                // Remove PK_LAST_WLAN entirely rather than leave it pointing
                // at an index - after applyWlanList()'s compaction that index
                // could now denote a different, shifted network.

                preferences.remove(PK_LAST_WLAN);

                delay(WAIT_1s);
                espReboot();
            }
            else {
                DEBUG_PRINTLN("[SECURITY] wlanDeleteActive but no pending slot - nothing to do (from " + webserver.client().remoteIP().toString() + ")");
                redirectTo("/?tab=wlan&err=Nothing%20to%20delete");
            }
        }
        else if (action == "wlanOverwriteActive") {

            // Die im Formular gesendete WLAN-Liste uebernehmen (applyWlanList(), wie in /save),
            // pendingWifiSsid[]/Pass[] danach loeschen (kein Klartext-Passwort im RAM) und neu starten - es
            // war die laufende Verbindung.

            // Apply the WiFi list sent by the form (applyWlanList(), as in /save), clear
            // pendingWifiSsid[]/Pass[] afterwards (no plain-text password in RAM) and reboot - it was the
            // running connection.

            String newSsid[MAX_WLAN];
            String newPass[MAX_WLAN];
            for (int i = 0; i < MAX_WLAN; i++) {
                newSsid[i] = pendingWifiSsid[i];
                newPass[i] = pendingWifiPass[i];
                pendingWifiSsid[i] = "";
                pendingWifiPass[i] = "";
            }
            applyWlanList(newSsid, newPass);

            delay(WAIT_1s);
            espReboot();
        }
        else if (action == "wlanSwitchActive") {

            // Auf den gemerkten Slot wechseln, sofern er noch existiert (pendingWifiChangeIndex, wie bei
            // "wlanDeleteActive").

            // Switch to the remembered slot if it still exists (pendingWifiChangeIndex, as with
            // "wlanDeleteActive").

            int idx = pendingWifiChangeIndex;
            pendingWifiChangeIndex = -1;

            if (idx >= 0 && idx < MAX_WLAN && preferences.getString(pkSsid(idx).c_str(), "") != "") {
                DEBUG_PRINTLN("[WiFi] Switching to saved network: " + preferences.getString(pkSsid(idx).c_str(), "") + " (from " + webserver.client().remoteIP().toString() + ")");
                preferences.putInt(PK_LAST_WLAN, idx);
                delay(WAIT_1s);
                espReboot();
            }
            else {
                DEBUG_PRINTLN("[SECURITY] wlanSwitchActive but slot no longer valid - nothing to do (from " + webserver.client().remoteIP().toString() + ")");
                redirectTo("/?tab=wlan&err=Network%20no%20longer%20available");
            }
        }
    }


    // Wandelt esp_reset_reason() in lesbaren Text um, fuer die Status-Anzeige.
    // Converts esp_reset_reason() into readable text, for the status display.

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


    // Baut den gemeinsamen inneren Teil des Helligkeits-Formulars - umschliessendes <form>/<div> und
    // Save-Button bleiben bei den Aufrufern.

    // Builds the shared inner part of the brightness form - the surrounding <form>/<div> and the save button
    // stay with the callers.

    String brightnessFormFieldsHtml() {
        String html = "";

        // Haken fuer die Hintergrundbeleuchtung nur bei regelbarer Beleuchtung (GC9D01 oder GC9A01 mit BL);
        // das versteckte Feld zeigt /save_brightness, dass er im Formular war. Alle Haken in einer Tabelle,
        // buendig untereinander.

        // Backlight checkbox only with a controllable backlight (GC9D01 or GC9A01 with BL); the hidden field
        // tells /save_brightness it was in the form. All checkboxes in one table, lined up below each other.

        bool showBacklight = displayType == DISPLAY_TYPE_GC9D01 || displayType == DISPLAY_TYPE_ST7789 ||
                             displayType == DISPLAY_TYPE_ST7789_240 || useBacklight;
        if (showBacklight || photoresistorFound) {
            html += "<table style='margin:auto;text-align:left;'>";
            if (showBacklight) {
                html += "<tr><td colspan='2'><input type='hidden' name='useBacklightField' value='1'><label><input type='checkbox' name='useBacklight' value='1' " + String(useBacklight ? "checked" : "") + "> " + withBacklightPin(translate("Backlight control (pin {pin})")) + "</label> " + infoTip(withBacklightPin(translate("Dims the display via the backlight PWM on pin {pin} instead of darkening the pixels - only if the backlight is wired to pin {pin} (always on GC9D01). Switching resets min. brightness and thresholds to the matching defaults"))) + "</td></tr>";
            }
            if (photoresistorFound) {
                html += "<tr><td><label><input type='checkbox' name='use_adc' value='1' " + String(useAdc ? "checked" : "") + "> " + translate("Enable Auto Brightness") + "</label> " + infoTip(translate("Automatically adjusts brightness based on ambient light measured by the photoresistor")) + "</td>";
                html += "<td><label><input type='checkbox' name='adcInverted' value='1' " + String(adcInverted ? "checked" : "") + "> " + translate("Invert ADC Reading") + "</label> " + infoTip(translate("Reverses the brightness sensor reading - use if the display gets darker in bright light instead of brighter")) + "</td></tr>";
            }
            html += "</table><hr><br>";
        }

        html += "<div style='display:flex;align-items:center;gap:6px;white-space:nowrap;'><label style='width:280px;display:inline-block;white-space:normal;'>" + translate("Full brightness from (hour, 0-23)") + ":</label><input name = 'brightStart' type = 'number' min = '0' max = '23' value = '" + String(brightStartHour) + "' style='width:70px;'> " + infoTip(translate("Start of the daily time window during which the display always uses full brightness, regardless of ambient light")) + "</div>";
        html += "<div style='display:flex;align-items:center;gap:6px;white-space:nowrap;'><label style='width:280px;display:inline-block;white-space:normal;'>" + translate("Full brightness until (hour, 0-23)") + ":</label><input name = 'brightEnd' type = 'number' min = '0' max = '23' value = '" + String(brightEndHour) + "' style='width:70px;'> " + infoTip(translate("End of the daily time window during which the display always uses full brightness, regardless of ambient light")) + "</div><br>";

        html += "<div style='display:flex;align-items:center;gap:6px;white-space:nowrap;'><label style='width:280px;display:inline-block;white-space:normal;'>" + translate("Min Brightness") + " (0 - 255) : </label><input name = 'minBrightness' type = 'number' min = '0' max = '255' value = '" + String(minBrightness) + "' style='width:70px;'> " + infoTip(translate("Display brightness used at or below the low threshold")) + "</div>";
        html += "<div style='display:flex;align-items:center;gap:6px;white-space:nowrap;'><label style='width:280px;display:inline-block;white-space:normal;'>" + translate("Max Brightness") + " (0 - 255) : </label><input name = 'maxBrightness' type = 'number' min = '0' max = '255' value = '" + String(maxBrightness) + "' style='width:70px;'" +
                (BOARD_WAVESHARE_C6_ST7789 ? String(" oninput=\"document.getElementById('maxBrightWarn').style.display=this.value>" + String(BACKLIGHT_C6_MAX_SAFE) + "?'':'none'\"") : String("")) +
                "> " + infoTip(translate("Display brightness used at or above the high threshold")) + "</div>";

        // ESP32-C6: Warnung ueber BACKLIGHT_C6_MAX_SAFE (Hinweis von Waveshare zur Ueberhitzung), live beim Tippen
        // ESP32-C6: warning above BACKLIGHT_C6_MAX_SAFE (Waveshare's note on overheating), live while typing

        if (BOARD_WAVESHARE_C6_ST7789) {
            html += "<div id='maxBrightWarn' style='color:#e67e22;margin:4px 0;" + String(maxBrightness > BACKLIGHT_C6_MAX_SAFE ? "" : "display:none;") + "'>&#9888; " +
                    translate("Above 128 the backlight of the ESP32-C6 board can overheat (note from Waveshare) - 128 or less is recommended") + "</div>";
        }
        html += "<br>";

        if (photoresistorFound) {
            html += "<div style='display:flex;align-items:center;gap:6px;white-space:nowrap;'><label style='width:280px;display:inline-block;white-space:normal;'>" + translate("Low Threshold") + " (0 - 100 %) : </label><input name = 'lowThreshold' type = 'number' min = '0' max = '100' value = '" + String(lowThreshold) + "' style='width:70px;'> " + infoTip(translate("Below this ambient light percentage, the display uses minimum brightness")) + "</div>";
            html += "<div style='display:flex;align-items:center;gap:6px;white-space:nowrap;'><label style='width:280px;display:inline-block;white-space:normal;'>" + translate("High Threshold") + " (0 - 100 %) : </label><input name = 'highThreshold' type = 'number' min = '0' max = '100' value = '" + String(highThreshold) + "' style='width:70px;'> " + infoTip(translate("Above this ambient light percentage, the display uses maximum brightness")) + "</div>";
        }

        if (useBacklight && photoresistorFound) { // Gamma formt die Kurve Lichtsensor -> PWM, gibt es also nur mit beidem
                                                  // gamma shapes the curve light sensor -> PWM, so only with both
        html += "<div style='display:flex;align-items:center;gap:6px;white-space:nowrap;'><label style='width:280px;display:inline-block;white-space:normal;'>" + translate("Gamma Correction") + " (0.1 - 3.0) : </label><input type='number' name='gamma' step='0.1' min='0.1' max='3.0' value='" + String(gammaBrightness) + "' required style='width:70px;'> " + infoTip(translate("Adjusts how brightness ramps between minimum and maximum - higher values keep the display darker for longer before brightening")) + "</div>";
        }

        return html;
    }


    // Liest ein GET/POST-Feld als int mit Validierung: fehlt/ungueltig ->
    // defaultValue statt stillschweigend 0, Ergebnis auf [minVal, maxVal] geklemmt.

    // Reads a GET/POST field as an int with validation: missing/invalid ->
    // defaultValue instead of silent 0, result clamped to [minVal, maxVal].

    int argToIntClamped(const String& name, int defaultValue, int minVal, int maxVal) {

        // defaultValue selbst wird ebenfalls geklemmt, sonst koennte ein Aufrufer
        // die Begrenzung fuer den "Feld fehlt"-Fall umgehen.

        // defaultValue itself is also clamped, otherwise a caller could bypass
        // the bound for the "field missing" case.

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


    // HTML-Label fuer einen Rotationswert - "n.a." bei TFT_ROTATION_NA (siehe
    // isDisplayConnected() in display.h), sonst 0/90/180/270 Grad. Gemeinsam
    // genutzt von /status und /api/status.

    // HTML label for a rotation value - "n.a." for TFT_ROTATION_NA (see
    // isDisplayConnected() in display.h), otherwise 0/90/180/270 degrees.
    // Shared by /status and /api/status.

    String rotationLabelHtml(uint8_t rotation) {
        static const char* labels[] = { "0&deg;", "90&deg;", "180&deg;", "270&deg;", "n.a." };
        return labels[rotation <= TFT_ROTATION_NA ? rotation : 0];
    }


    // Eintraege der Statusseite (<li>...) - gemeinsam fuer /status, den Tab Status und status.txt in der Sicherung.
    // flush() nimmt die Teilstuecke ab (Seiten: senden, Sicherung: sammeln), damit die Liste nicht als Ganzes im
    // Speicher liegt.

    // Entries of the status page (<li>...) - shared by /status, the Status tab and status.txt in the backup. flush()
    // takes the pieces (pages: send, backup: collect), so the list is never held in memory as a whole.

    void generateStatusItems(String& chunk, std::function<void(String&)> flush) {
        chunk += "<li>" + generateStorageInfo(LittleFS.usedBytes(), LittleFS.totalBytes(), true) + "</li>";

        String tzLabel = preferences.getString(PK_TIMEZONE, "DE");
        String tzDesc;

        tzDesc = tzLabel;

        // Lokale Kopie statt der globalen timeinfo - der Haupt-Loop nutzt sie ebenfalls, ein Fehlschlag
        // hier soll sie nicht mit einer ungueltigen Zwischenzeit ueberschreiben.

        // Local copy instead of the global timeinfo - the main loop uses it too, a failure here should
        // not overwrite it with an invalid intermediate time.

        struct tm statusTimeinfo;
        if (getLocalTime(&statusTimeinfo, 100)) {
            char nowStr[32];
            strftime(nowStr, sizeof(nowStr), "%Y-%m-%d %H:%M:%S", &statusTimeinfo);
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

        flush(chunk);

        chunk += "<li>Compiled on: <strong>" + (String)version + "</strong></li><br>";

        chunk += "<li>TFT Driver: " + tftType + "</li>";
        chunk += "<li>Graphics Library: LovyanGFX " + String(LGFX_VERSION_MAJOR) + "." + String(LGFX_VERSION_MINOR) + "." + String(LGFX_VERSION_PATCH) + "</li>";

        // Bildrate der Uhranzeige (recordRenderFrame() in display.h) plus Zeigerstil - zur Diagnose ruckelnder/tickender Zeiger
        // frame rate of the clock display (recordRenderFrame() in display.h) plus hand style - for diagnosing jerky/ticking hands

        chunk += "<li>Render: " + String(renderStats.fps, 1) + " fps, avg " + String(renderStats.avgMs, 1) + " ms, max " + String(renderStats.maxMs, 1) + " ms, partial " + String(renderStats.partialPercent, 0) + " % (last 5 s)</li>";
        chunk += "<li>Second hand: " + String(smoothSecond ? "smooth" : "ticking") + ", station mode " + String(stationMode ? "on" : "off") + "</li>";

        chunk += "<li>TFT Size: " + String(TFT_WIDTH) + " x " + String(TFT_HEIGHT) + "</li>";

        chunk += "<br>";

        flush(chunk);

        chunk += "<li>Chip Model: " + String(ESP.getChipModel()) + "</li>";
        chunk += "<li>Chip Revision: " + String(ESP.getChipRevision()) + "</li>";
        chunk += "<li>Chip Cores: " + String(ESP.getChipCores()) + "</li>";
        chunk += "<li>Chip ID: " + String((uint32_t)ESP.getEfuseMac(), HEX) + "</li>";
        chunk += "<li>CPU Frequency: " + String(getCpuFrequencyMhz()) + " MHz</li><br>";

        flush(chunk);

        chunk += "<li>Hostname: " + String(hostname) + ".local" + "</li>";
        chunk += "<li>IP Address: " + WiFi.localIP().toString() + "</li>";
        chunk += "<li>MAC Address: " + WiFi.macAddress() + "</li>";
        chunk += "<li>WiFi SSID: " + String(WiFi.SSID()) + "</li>";
        chunk += "<li>WiFi Mode: " + String(WiFi.getMode() == WIFI_AP ? "WIFI_AP" : (WiFi.getMode() == WIFI_STA ? "WIFI_STA" : "AP_STA")) + "</li>";
        chunk += "<li>WiFi Channel: " + String(WiFi.channel()) + "</li>";
        chunk += "<li>WiFi TX Power: " + String(WiFi.getTxPower() / 4.0f, 1) + " dBm</li>";
#if HAS_OTA
        if (fwRemoteBuild.length()) chunk += "<li>Firmware on GitHub: " + fwRemoteBuild + (firmwareUpdateState() > 0 ? " (newer)" : "") + "</li>";
#endif
        chunk += "<li>Signal Strength (RSSI): " + String(WiFi.RSSI()) + " dBm</li>";

        // ntpServerRunning kommt vom echten Rueckgabewert von udp.begin()
        // (siehe startNtpServer()), nicht nur aus einem blinden Log.

        // ntpServerRunning comes from udp.begin()'s real return value
        // (see startNtpServer()), not just a blind log entry.

        chunk += "<li>NTP Server (own): " + String(ntpServerRunning ? "running on port " + String(NTP_PORT) : "not running") +
                 " - requests: " + String(ntpRequestsReceived) + ", answered: " + String(ntpRepliesSent) + "</li><br>";

        flush(chunk);

        chunk += "<li>SDK Version: " + String(ESP.getSdkVersion()) + "</li><br>";
        chunk += "<li>Arduino Core Version: " ESP_ARDUINO_VERSION_STR "</li><br>";

        flush(chunk);

        chunk += "<li>Flash Size: " + String(ESP.getFlashChipSize() / 1024) + " KB</li>";
        chunk += "<li>Free Heap: " + String(ESP.getFreeHeap() / 1024) + " KB</li>";
        chunk += "<li>Max Allocatable Block: " + String(ESP.getMaxAllocHeap() / 1024) + " KB</li>";
        chunk += "<li>Min Free Heap (since boot): " + String(ESP.getMinFreeHeap() / 1024) + " KB</li>";
        chunk += "<li>Max Sketch Size: " + String(ESP.getFreeSketchSpace() / 1024) + " KB</li>";
        chunk += "<li>Sketch Size: " + String(ESP.getSketchSize() / 1024) + " KB</li>";
        chunk += "<li>Free Sketch Space: " + String((ESP.getFreeSketchSpace() / 1024) - (ESP.getSketchSize() / 1024)) + " KB</li><br>";

        flush(chunk);

        // Explizite Erkennung, da ohne PSRAM Groesse/Frei beide 0 waeren
        // (nicht unterscheidbar von "PSRAM da, aber voll").

        // Explicit detection, since without PSRAM size/free would both
        // read 0 (indistinguishable from "PSRAM present but full").

        chunk += "<li>PSRAM Detected: " + String(psramFound() ? "yes" : "no") + "</li>";
        chunk += "<li>PSRAM Size: " + String(ESP.getPsramSize() / 1024) + " kB</li>";
        chunk += "<li>PSRAM Free: " + String(ESP.getFreePsram() / 1024) + " kB</li><br>";


        chunk += "<li>LittleFS Size: " + String(LittleFS.totalBytes() / 1024) + " KB</li>";
        chunk += "<li>LittleFS Used: " + String(LittleFS.usedBytes() / 1024) + " KB</li>";
        chunk += "<li>LittleFS Free: " + String((LittleFS.totalBytes() - LittleFS.usedBytes()) / 1024) + " KB</li><br>";

        flush(chunk);

        if (photoresistorFound) {
            chunk += "<li>Photoresistor found on GPIO: " + String(ADC_PIN) + "</li>";
            chunk += "<li>Actual brightness (0-255): " + String(currentBrightness) + "</li><br>";
        }
        else {
            chunk += "<li>Photoresistor not found on GPIO: " + String(ADC_PIN) + "</li><br>";
        }

        flush(chunk);



        chunk += "<li>TFT_SCLK GPIO: " + String(TFT_SCLK) + "</li>";
        //chunk += "<li>TFT_MISO: " + String(TFT_MISO) + "</li>";
        chunk += "<li>TFT_MOSI GPIO: " + String(TFT_MOSI) + "</li>";
        chunk += "<li>TFT_CS1 GPIO: " + String(CS_1) + " (Display 1)</li>"; // CS_1 = Display 1 (vormals TFT_CS, jetzt manuell angesteuert, siehe config.h)
                                                                 // CS_1 = display 1 (formerly TFT_CS, now driven manually, see config.h)
        if (HAS_DISPLAY2) chunk += "<li>TFT_CS2 GPIO: " + String(CS_2) + " (Display 2)</li>";


        chunk += "<li>TFT_DC GPIO: " + String(TFT_DC) + "</li>";
        chunk += "<li>TFT_RST GPIO: " + String(TFT_RST) + "</li><br>";

        if (!i2cAddr.isEmpty()) {
            chunk += "<li>I2C ADR: " + i2cAddr + "</li>";
            chunk += "<li>I2C SDA GPIO: " + String(SDA_PIN) + "</li>";
            chunk += "<li>I2C SCL GPIO: " + String(SCL_PIN) + "</li><br>";
        }
        else {
            chunk += "<li>I2C: no device found</li><br>";
        }

        flush(chunk);

        if (dcf77Count == 0) {
            chunk += "<li>DCF77 Status: No signal received so far</li>";
        }
        else {
            chunk += "<li>DCF77 Status: Pulses received</li>";
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
        chunk += "<li>DCF77 Input: both edges (CHANGE), polarity-independent</li><br>";

        flush(chunk);

        chunk += "<li>BUTTON GPIO: " + String(BUTTON1) + "</li>";
        chunk += "<li>BUTTON_BOOT GPIO: " + String(BOOT_BUTTON) + "</li>";

        chunk += "<li>LED_BOARD GPIO: " + String(LED_BOARD_GPIO) + "</li>";
        chunk += "<li>ADC_VCC GPIO: " + String(ADC_3V) + "</li>";
        chunk += "<li>ADC (photoresistor) GPIO: " + String(ADC_PIN) + "</li>";
        chunk += "<li>ADC_GND GPIO: " + String(ADC_GND) + "</li>";
        if (photoresistorFound) {
            chunk += "<li>ADC Value: " + String(getAdjustedAdcValue(analogRead(ADC_PIN))) + "</li><br>";
        }


        chunk += "<li>Build: " BUILD_DISPLAY_MARKER "</li>";
        chunk += "<li>TFT_Backlight GPIO: " + String(TFT_Backlight) + (useBacklight ? " (PWM)" : " (off - pixel dimming)") + "</li>";
        chunk += "<br>";

        flush(chunk);

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

        flush(chunk);

        for (int i = 0; i < MAX_WLAN; i++) {
            if (preferences.getString((pkNtpServer(i)).c_str(), "") != "") {
                chunk += "<li><b>ntpServer" + String(i + 1) + ":</b> " + preferences.getString((pkNtpServer(i)).c_str(), "") + "</li>";
            }
        }
   

        chunk += "<li><b>timezone</b>: " + preferences.getString(PK_TIMEZONE, TIMEZONE_DEFAULT) + "</li>";
        chunk += "<li><b>background</b>: " + preferences.getString(PK_BACKGROUND, "/faces/default") + "</li>";
        chunk += "<li><b>handset</b>: " + preferences.getString(PK_HANDSET, "") + "</li>";

        // getLong() wie beim Schreiben (putLong()) - getUInt() lieferte wegen des NVS-Typs
        // stillschweigend den Default. Gespeichert ist RGB888.

        // getLong() as when writing (putLong()) - getUInt() would silently return the default due to the
        // NVS type. Stored is RGB888.

        chunk += "<li><b>centerColor (RGB888)</b>: " + String(preferences.getLong(PK_CENTER_COLOR, 0xEC0016), HEX) + "</li>";
        chunk += "<li><b>centerSize</b>: " + String(preferences.getUInt(PK_CENTER_SIZE, 6)) + "</li>";

        uint8_t rotation = preferences.getUChar(PK_TFT_ROTATION1, TFT_ROTATION1_DEFAULT);
        chunk += "<li><b>tftRotation1</b>: " + rotationLabelHtml(rotation) + (autoRotation ? " (auto)" : "") + "</li>";
        if (imuAvailable()) {
            int8_t q = imuReadQuadrant();
            chunk += "<li><b>IMU QMI8658</b>: " + (q < 0 ? String("flat/unclear") : "position " + String(q) + " -> " + rotationLabelHtml(imuRotationFor(q))) +
                     ", offset " + String(preferences.getUChar(PK_IMU_ROT_OFFSET, 0)) + "</li>";
        }
        {
            uint8_t rotation2 = preferences.getUChar(PK_TFT_ROTATION2, TFT_ROTATION2_DEFAULT);
            chunk += "<li><b>tftRotation2</b>: " + rotationLabelHtml(rotation2) + "</li>";
        }

        // Rotationsmodus zeigt, WIE die Werte angewendet werden: beim GC9D01
        // ist Hardware-Rotation wirkungslos, nur mit PSRAM wird auf
        // Software-Rotation umgeschaltet (siehe gc9d01SwRotation in uhr4.ino).

        // Rotation mode shows HOW the values are applied: on the GC9D01
        // hardware rotation has no effect, only with PSRAM does it switch to
        // software rotation (see gc9d01SwRotation in uhr4.ino).

        chunk += "<li><b>rotation mode</b>: ";
        if (gc9d01SwRotation) {
            chunk += "software (pixel remap, GC9D01 with PSRAM)";
        }
        else {
            chunk += "hardware (display MADCTL register)";
            if (displayGeom->swRotation) chunk += " - <b>ineffective on GC9D01</b>, software rotation needs PSRAM";
        }
        chunk += "</li>";

        flush(chunk);

        // Booleans als Text
        // Booleans as text
    
        bool stationModeStatus = preferences.getBool(PK_STATION_MODE, true);
        chunk += "<li><b>stationMode</b>: " + String(stationModeStatus ? "true" : "false") + "</li>";
        chunk += "<li><b>smoothSecond</b>: " + String(getSmoothSecondPref(stationModeStatus) ? "true" : "false") + "</li>";
        chunk += "<li><b>showSecondhand</b>: " + String(preferences.getBool(PK_SHOW_SECOND_HAND, true) ? "true" : "false") + "</li>";
        chunk += "<li><b>smoothMinute</b>: " + String(preferences.getBool(PK_SMOOTH_MINUTE, false) ? "true" : "false") + "</li>";

        chunk += "<li><b>minBrightness</b>: " + String(preferences.getUChar(PK_MIN_BRIGHTNESS, 100)) + "</li>";
        chunk += "<li><b>useBacklight</b>: " + String(preferences.getBool(PK_USE_BACKLIGHT, BACKLIGHT_DEFAULT) ? "true" : "false") + "</li>";
        chunk += "<li><b>maxBrightness</b>: " + String(preferences.getUChar(PK_MAX_BRIGHTNESS, 255)) + "</li>";

        chunk += "<li><b>daywindow</b>: " + dayWindowText() + "</li>";

        if (preferences.getBool(PK_USE_ADC, true)) {
            chunk += "<li><b>use_adc</b>: " + String(preferences.getBool(PK_USE_ADC, true) ? "true" : "false") + "</li>";
            chunk += "<li><b>adc lowThreshold</b>: " + String(preferences.getInt(PK_LOW_THRESHOLD, 40)) + "</li>";
            chunk += "<li><b>adc highThreshold</b>: " + String(preferences.getInt(PK_HIGH_THRESHOLD, 60)) + "</li>";
            chunk += "<li><b>adc Inverted</b>: " + String(preferences.getBool(PK_ADC_INVERTED, false) ? "true" : "false") + "</li>";
        }
        if (preferences.getBool(PK_ROCRAIL_ENABLED, false)) {
            chunk += "<li><b>rocrailEnabled</b>: " + String(preferences.getBool(PK_ROCRAIL_ENABLED, false) ? "true" : "false") + "</li>";
            chunk += "<li><b>rocrailServer</b>: " + preferences.getString(PK_ROCRAIL_SERVER, "") + "</li>";
            chunk += "<li><b>rocrailServerPort</b>: " + String(preferences.getUShort(PK_ROCRAIL_SRV_PORT, ROCRAIL_DEFAULT_PORT)) + "</li>";
        }
        flush(chunk);
    }


    // Webserver-API-Endpunkte einrichten
    // Set up the webserver API endpoints

    void setupWebServer() {

        // Captive-Portal-Erkennung: bekannte OS-URLs auf die Konfigseite umleiten
        // statt 404, damit sich am AP automatisch ein Browserfenster oeffnet.

        // Captive portal detection: redirect known OS URLs to the config page
        // instead of 404, so a browser window opens automatically on the AP.

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
                                                                              // Catch-all: alle sonstigen Anfragen ebenfalls auf die Konfigseite leiten

        // Catch-all: redirect all other requests to the config page too

        webserver.onNotFound(captivePortalRedirect);

        // API fuer Zifferblatt, Zeiger, Zeitzone, Nabe, Bahnhofsmodus, Rotation und Zeigerstil, z.B.
        // /api/setMode?face=face_default.bmp&handSet=0&stationMode=true - Helligkeit optional (minBrightness,
        // maxBrightness, brightStart, brightEnd, lowThreshold, highThreshold, gamma, autoBrightness).

        // API for clock face, hands, time zone, hub, station mode, rotation and hand style, e.g.
        // /api/setMode?face=face_default.bmp&handSet=0&stationMode=true - brightness optional (minBrightness,
        // maxBrightness, brightStart, brightEnd, lowThreshold, highThreshold, gamma, autoBrightness).



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

        // Startet WPS per Web-Button, kehrt SOFORT zurueck - kein delay(),
        // sonst koennte der Webserver die Redirect-Zielseite nicht
        // ausliefern. startWPS() selbst laeuft zeitversetzt in loop() (siehe wpsStartRequested in globals.h).

        // Starts WPS via web button, returns IMMEDIATELY - no delay(),
        // otherwise the web server couldn't serve the redirect's target
        // page. startWPS() itself runs deferred in loop() (see wpsStartRequested in globals.h).

        webserver.on("/api/startWPS", HTTP_GET, []() {
            wpsPreviousSsid = WiFi.isConnected() ? WiFi.SSID() : "";
            redirectTo("/?tab=wlan&msg=WPS%20active%20-%20press%20the%20WPS%20button%20on%20your%20router%20now.%20Connection%20to%20the%20clock%20may%20be%20lost%20for%20about%202%20minutes%20while%20this%20happens");
            wpsStartRequested = true;
            wpsStartRequestedAtMillis = millis();
            });

        webserver.on("/api/startWPS", HTTP_POST, []() {
            wpsPreviousSsid = WiFi.isConnected() ? WiFi.SSID() : "";
            redirectTo("/?tab=wlan&msg=WPS%20active%20-%20press%20the%20WPS%20button%20on%20your%20router%20now.%20Connection%20to%20the%20clock%20may%20be%20lost%20for%20about%202%20minutes%20while%20this%20happens");
            wpsStartRequested = true;
            wpsStartRequestedAtMillis = millis();
            });

        // Liefert, ob WPS noch aktiv/wartend ist (wpsPending oder ein
        // angeforderter, noch nicht gestarteter Versuch) - fuer das Banner-
        // Polling in generateFlashMessage(), das den Hinweis so lange zeigt, wie WPS tatsaechlich laeuft.

        // Reports whether WPS is still active/waiting (wpsPending, or a
        // requested but not yet started attempt) - used by the banner
        // polling in generateFlashMessage(), which shows the notice for as long as WPS is actually running.

        webserver.on("/api/wpsStatus", HTTP_GET, []() {
            webserver.sendHeader("Cache-Control", "no-store");
            bool active = wpsPending || wpsStartRequested;
            webserver.send(200, "application/json", String("{\"pending\":") + (active ? "true" : "false") + "}");
            });

        // Speichert einen Hostnamen (siehe sanitizeHostname()) - wirkt erst
        // nach einem Neustart, da connectWiFi() das nur einmalig aufruft.

        // Saves a hostname (see sanitizeHostname()) - only takes effect
        // after a reboot, since connectWiFi() only calls this once.

        webserver.on("/sethostname", HTTP_POST, []() {
            if (webserver.hasArg("hostname")) {
                String newHostname = sanitizeHostname(webserver.arg("hostname"));

                if (newHostname.isEmpty()) {

                    // Kein gueltiger Hostname extrahierbar - Override entfernen,
                    // damit wieder der automatische Name generiert wird.

                    // No valid hostname could be extracted - remove the override
                    // so the automatic name is generated again.

                    preferences.remove(PK_HOSTNAME);
                    redirectTo("/?tab=wlan&warn=No%20valid%20hostname%20could%20be%20derived%20from%20the%20input%20-%20falling%20back%20to%20the%20automatic%20name%20based%20on%20the%20MAC%20address");
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

            // Nur bei echter Aenderung - jedes Preset enthaelt die Zeitzone, sonst stiesse jeder Aufruf einen
            // NTP-Abgleich an.

            // Only on an actual change - every preset contains the time zone, otherwise every call would trigger
            // an NTP sync.

            if (webserver.hasArg("timeZone") && webserver.arg("timeZone") != timezone) {
                String tz = webserver.arg("timeZone");
                preferences.putString(PK_TIMEZONE, tz);
                timezone = tz;
                applyTimezoneToSystem(); // sofort wirksam, auch ohne erreichbaren NTP-Server
                                         // effective right away, even without a reachable NTP server

                // Nur die Task anstossen (siehe time_sync.h) statt hier auf
                // DNS/UDP zu warten - der Webserver darf dabei nicht blockieren.
                // pollNtpSyncTask() in loop() wertet das Ergebnis aus.

                // Only kick off the task (see time_sync.h) instead of waiting
                // on DNS/UDP here - the web server must not block on this.
                // pollNtpSyncTask() in loop() evaluates the result.

                startNtpSyncTask("Timezone change sync");
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

            // Rocrail-Modellzeit an/aus (nur bei Aenderung) - die Serveradressen bleiben Geraeteeinstellung
            // Rocrail model time on/off (only on a change) - the server addresses stay a device setting

            if (webserver.hasArg("rocrail")) {
                String rocrailArg = webserver.arg("rocrail");
                bool on = (rocrailArg == "1" || rocrailArg.equalsIgnoreCase("true"));
                if (on != rocrailEnabled) setRocrailEnabled(on);
            }

            // "WLAN neu verbinden" (wifiActive) - loop() prueft es bei jedem Durchlauf
            // "Reconnect WiFi" (wifiActive) - loop() checks it on every pass

            if (webserver.hasArg("wifiReconnect")) {
                String wifiArg = webserver.arg("wifiReconnect");
                bool on = (wifiArg == "1" || wifiArg.equalsIgnoreCase("true"));
                if (on != wifiActive) {
                    wifiActive = on;
                    preferences.putBool(PK_WIFI_ACTIVE, on);
                }
            }

            // Rotation NICHT aus Presets (source=preset) - sie ist eine Geraeteeinstellung; alte Preset-URLs
            // enthalten noch "rotation=". Direkte API-Aufrufe duerfen sie weiter setzen.

            // Rotation NOT from presets (source=preset) - it is a device setting; old preset URLs still
            // contain "rotation=". Direct API calls may still set it.

            if (webserver.hasArg("rotation") && webserver.arg("source") != "preset") {
                String rotationArg = webserver.arg("rotation");

                // Eigener Name statt "tftRotation" (wuerde die globale Variable verdecken). Als long, damit
                // z.B. "rotation=256" nicht vor der Pruefung zu 0 abgeschnitten wird.

                // Own name instead of "tftRotation" (would shadow the global variable). As long, so e.g.
                // "rotation=256" is not truncated to 0 before validation.

                long requestedRotation = -1;

                // Prüfe, ob der Wert in Grad angegeben ist
                // Check whether the value is given in degrees

                if (rotationArg == "0" || rotationArg == "90" || rotationArg == "180" || rotationArg == "270") {
                    if (rotationArg == "0") requestedRotation = 0;
                    else if (rotationArg == "90") requestedRotation = 1;
                    else if (rotationArg == "180") requestedRotation = 2;
                    else if (rotationArg == "270") requestedRotation = 3;
                }

                // "na" / "n.a." = Display 1 nicht angeschlossen
                // "na" / "n.a." = display 1 not connected

                else if (rotationArg.equalsIgnoreCase("na") || rotationArg.equalsIgnoreCase("n.a.")) {
                    requestedRotation = TFT_ROTATION_NA;
                }

                // Prüfe, ob der Wert als Index (0-3, 4 = n.a.) angegeben ist
                // Check whether the value is given as an index (0-3, 4 = n.a.)

                else {
                    requestedRotation = rotationArg.toInt();
                }

                // Validierung des Wertes
                // Validate the value

                if (requestedRotation >= 0 && requestedRotation <= TFT_ROTATION_NA) {
                    applyDisplayRotation(1, (uint8_t)requestedRotation);
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

            if (webserver.hasArg("smoothSecond")) {
                String smoothSecondArg = webserver.arg("smoothSecond");
                smoothSecond = (smoothSecondArg == "1" || smoothSecondArg.equalsIgnoreCase("true")); // Konvertiere zu bool
                                                                                                     // convert to bool
                preferences.putBool(PK_SMOOTH_SECOND, smoothSecond);
            }

            // Helligkeit aus Presets (applyBrightnessPresetValue() in display.h,
            // gleiche Klemmung wie beim Weiterschalten per Taste)

            // Brightness from presets (applyBrightnessPresetValue() in display.h,
            // same clamping as when switching via the button)

            static const char* const brightnessKeys[] = {
                "minBrightness", "maxBrightness", "brightStart", "brightEnd",
                "lowThreshold", "highThreshold", "gamma", "autoBrightness"
            };
            for (const char* key : brightnessKeys) {
                if (webserver.hasArg(key)) applyBrightnessPresetValue(key, webserver.arg(key));
            }

            // Der Streifen gehoert zum Zifferblatt (stripcfg_*.txt) - Streifen-Werte aelterer Presets bleiben unbeachtet
            // The strip belongs to the clock face (stripcfg_*.txt) - strip values of older presets are ignored

            freeClockFaceBuffer();
            loadClockFace();
            loadHandSprites();
            updateClock();

            if (webserver.hasArg("source")) {
                String sourceArg = webserver.arg("source");
                if (sourceArg == "preset") {
                    // DEBUG_PRINTLN("[API] Request source: preset");

                    // Mit ajax=1 (Seite /presets ohne Neuladen) genuegt die Bestaetigung
                    // With ajax=1 (page /presets without reloading) the confirmation is enough

                    if (webserver.arg("ajax") == "1") {
                        webserver.send(200, "text/plain", "ok");
                        return;
                    }
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

            // Nur die Anzeige-Reihenfolge sortieren, Array-Indizes bleiben
            // unveraendert, damit Rename-/Delete-Links stimmen.

            // Only sort the display order, array indices stay unchanged so
            // rename/delete links remain correct.

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

                    // Relativer Pfad statt der festen IP der gespeicherten Preset-URL - so fuehrt der Link
                    // immer zu der Adresse, ueber die die Seite gerade aufgerufen wurde.

                    // Relative path instead of the stored preset URL's fixed IP - so the link always leads to
                    // the address the page was just reached with.

                    if (displayUrl.startsWith("http://")) {
                        int pathStart = displayUrl.indexOf('/', 7); // Suche nach dem Beginn des Pfads (nach der IP)
                                                                    // find the start of the path (after the IP)
                        displayUrl = (pathStart != -1) ? displayUrl.substring(pathStart) : "/";
                    }
                    displayUrl += "&source=preset";

                    // presetName ist eine lokale Kopie - ein GET darf den globalen Zustand nicht aendern. Der
                    // Name kommt vom Nutzer - daher escapen (escapeHtmlText()/escapeForJsStringInAttr()),
                    // sonst gespeichertes XSS.

                    // presetName is a local copy - a GET must not change the global state. The name comes
                    // from the user - so escape it (escapeHtmlText()/escapeForJsStringInAttr()), otherwise
                    // stored XSS.

                    String safePresetNameText = escapeHtmlText(presets[i].name);
                    chunk += "<div style='text-align:center;border:1px solid #ccc;border-radius:6px;padding:8px;width:220px;'>";
                    chunk += "<a href='" + displayUrl + "' onclick='return pickPreset(this)'><img class='pv' data-src='/presetpreview?index=" + String(i) + "' style='width:90px;height:auto;'></a>";
                    chunk += "<br><a href='" + displayUrl + "' onclick='return pickPreset(this)'>" + safePresetNameText + "</a>";
                    String presetName = presets[i].name;
                    presetName.replace(" ", "_"); // Ersetze Leerzeichen durch Unterstriche
                                                  // replace spaces with underscores

                    // "Copy link" nutzt den Host-Header dieser Anfrage statt der lokalen IP - der kopierte
                    // Link passt so auch von aussen (DMZ/Port-Weiterleitung). Fallback auf ipAddress, falls
                    // der Header leer ist.

                    // "Copy link" uses this request's host header instead of the local IP - so the copied
                    // link also works from outside (DMZ/port forward). Falls back to ipAddress if the header
                    // is empty.

                    String currentHost = webserver.hostHeader();
                    if (currentHost.length() == 0) currentHost = ipAddress;

                    String ipLink = "http://" + currentHost + "/api/setPreset?name=" + presetName;

                    // Aeusseres Attribut doppelt gequotet, JS-String innen einfach.
                    // Outer attribute double-quoted, inner JS string single-quoted.

                    chunk += "<br><span onclick=\"copyPresetLink('" + escapeForJsStringInAttr(ipLink, '\'') + "', this)\" style='cursor:pointer;font-size:1.3em;' title='" + translate("Copy link") + "'>&#128203;</span>";
                    if (pingHostname) {
                        String hostLink = espHost + "/api/setPreset?name=" + presetName;
                        chunk += " <span onclick=\"copyPresetLink('" + escapeForJsStringInAttr(hostLink, '\'') + "', this)\" style='cursor:pointer;font-size:1.3em;' title='" + translate("Copy link") + " (" + espHost + ")'>&#128203;</span>";
                    }
                    chunk += "<br><a href='/renamepreset_form?index=" + String(i) + "'>" + translate("Rename") + "</a> ";

                    // Aeusseres Attribut einfach gequotet, confirm()-String innen doppelt.
                    // Outer attribute single-quoted, inner confirm() string double-quoted.

                    chunk += "<button type='button' onclick='if(confirm(\"" + translate("Delete") + " " + escapeForJsStringInAttr(presets[i].name, '"') + "?\")){window.location.href=\"/deletepreset?index=" + String(i) + "\";}'>" + translate("Delete") + "</button>";
                    chunk += "</div>";

                    rowCount++;
                    if (rowCount % 5 == 0) {
                        webserver.sendContent(chunk);
                        chunk = "";
                    }
                }
            }
            chunk += "</div>";

            // Preset anwenden ohne die Seite neu zu laden: pickPreset() ruft den Link per fetch() mit &ajax=1 auf und
            // zeigt die Meldung. Schlaegt der Aufruf fehl, folgt der Browser dem Link wie bisher.

            // Apply a preset without reloading the page: pickPreset() calls the link via fetch() with &ajax=1 and shows
            // the message. If the call fails, the browser follows the link as before.

            chunk += "<div id='presetMsg' class='msg ok' style='display:none'>" + translate("Preset applied") + "</div>";
            chunk += "<script>function pickPreset(a){var m=document.getElementById('presetMsg');fetch(a.href+'&ajax=1').then(function(r){if(!r.ok)throw 0;"
                     "m.style.display='block';clearTimeout(window.presetT);window.presetT=setTimeout(function(){m.style.display='none';},4000);"
                     "}).catch(function(){location.href=a.href;});return false;}</script>";

            // Vorschaubilder nacheinander laden, bei Fehler bis zu 3 Versuche - parallel angefragt fehlte der Uhr
            // (vor allem ohne PSRAM) der Speicher, viele Bilder blieben leer.

            // Load the preview images one after another, up to 3 tries on error - requested in parallel the clock
            // (especially without PSRAM) ran out of memory, many images stayed empty.

            chunk += "<script>(function(){var a=document.querySelectorAll('img.pv'),i=0;"
                     "function next(){if(i>=a.length)return;var m=a[i++],t=0;"
                     "m.onload=next;m.onerror=function(){if(++t<3)setTimeout(function(){m.src=m.dataset.src+'&try='+t;},700);else next();};"
                     "m.src=m.dataset.src;}next();})();</script>";
            chunk += "<p>" + translate("Presets used") + ": " + String(rowCount) + " / " + String(MAX_PRESETS) + "</p>";
            chunk += "</div><hr>";

            chunk += "<h3>" + translate("Create New Preset") + "</h3>";
            chunk += "<form method='POST' action='/api/createPreset'>";
            chunk += "<button type='submit'>" + translate("Create Preset from Current Settings") + "</button>";
            chunk += "</form>";
            chunk += "</body></html>";
            webserver.sendContent(chunk);
            webserver.sendContent(""); // Ende der Chunked-Uebertragung signalisieren
                                       // signal the end of the chunked transfer
            });

        // API zum Neustart des ESP. Diagnose: Teil-Aktualisierung des Displays ein-/ausschalten - nicht
        // gespeichert, nach dem Neustart wieder an.

        // API to restart the ESP. Diagnostic: switch the display partial update on/off - not stored, back on
        // after a restart.

        webserver.on("/api/partialUpdate", HTTP_GET, []() {
            if (webserver.hasArg("enabled")) {
                partialUpdateEnabled = (webserver.arg("enabled") == "1");
                clockFrameDirty[0] = clockFrameDirty[1] = true; // naechstes Bild voll senden / send the next frame in full
            }
            webserver.send(200, "text/plain", String("partial update ") + (partialUpdateEnabled ? "on" : "off"));
            });

        webserver.on("/api/reboot", HTTP_GET, []() {

            // Nur aus einem privaten Netz (isPrivateNetworkIp()) - ein von aussen ausloesbarer Neustart waere
            // ein DoS-Vektor und koennte laufende Schreibvorgaenge unterbrechen.

            // Only from a private network (isPrivateNetworkIp()) - a restart triggerable from outside would
            // be a DoS vector and could interrupt running writes.

            if (!isPrivateNetworkIp(webserver.client().remoteIP())) {
                webserver.send(200, "text/html", simpleMessagePage(translate("Rebooting..."), "<p>" + translate("This action is only available when accessing the clock from a private network") + ".</p>"));
                return;
            }

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
            DEBUG_PRINTLN("[setPreset] Preset not found: " + presetName + " (from " + webserver.client().remoteIP().toString() + ")");
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

            // Kompaktierte Liste auch in die Preferences schreiben, sonst tauchen Eintraege nach einem
            // Neustart an alter Stelle wieder auf - nur bei tatsaechlicher Aenderung.

            // Write the compacted list to Preferences too, otherwise entries reappear at their old position
            // after a restart - only on an actual change.

            for (int i = 0; i < MAX_WLAN; i++) {
                String ntpKey = pkNtpServer(i);
                if (preferences.getString(ntpKey.c_str(), "") != String(ntpServers[i])) {
                    preferences.putString(ntpKey.c_str(), ntpServers[i]);
                }
            }

            // Ist die Liste danach leer, sofort auf die eingebauten Standardserver zurueckfallen.
            // If the list is empty afterwards, fall back to the built-in default servers immediately.

            applyNtpServerDefaultsIfNoneConfigured();

            // Uhrzeit aus offenen WLANs beim Start (Kaestchen im selben Formular, siehe fetchTimeFromOpenWifi())
            // Time from open WiFis at boot (checkbox in the same form, see fetchTimeFromOpenWifi())

            preferences.putBool(PK_OPEN_WIFI_TIME, webserver.hasArg("openWifiTime"));

            if (webserver.hasArg("timezone")) {
                String tz = webserver.arg("timezone");
                preferences.putString(PK_TIMEZONE, tz);

                // Auch die globale timezone-Variable setzen - die NTP-Task liest nur eine Kopie, sonst zeigte
                // die Einstellungsseite weiter die alte Zeitzone.

                // Also set the global timezone variable - the NTP task only reads a copy, otherwise the
                // settings page would keep showing the old time zone.

                timezone = tz;
                applyTimezoneToSystem(); // sofort wirksam, auch ohne erreichbaren NTP-Server
                                         // effective right away, even without a reachable NTP server

                // Nur die Task anstossen (siehe time_sync.h) - der Webserver
                // darf dabei nicht auf DNS/UDP warten. pollNtpSyncTask() in
                // loop() wertet das Ergebnis aus.

                // Only kick off the task (see time_sync.h) - the web server
                // must not wait on DNS/UDP here. pollNtpSyncTask() in loop()
                // evaluates the result.

                startNtpSyncTask("Timezone change sync");
            }

            redirectTo("/?tab=zeit&msg=Timezone%20updated");
            }
        
        
            );


        // Datei umbenennen Formular
        // Rename file form

        webserver.on("/rename_form", HTTP_GET, []() {
            if (!webserver.hasArg("file")) {
                webserver.send(400, "text/plain", "Missing file parameter");
                return;
            }

            // escapeHtmlText(): "file" kommt aus der URL - ohne Escaping reflektiertes XSS.
            // escapeHtmlText(): "file" comes from the URL - without escaping, reflected XSS.

            String oldName = escapeHtmlText(webserver.arg("file"));

            // "from" an Formular (verstecktes Feld) und Abbrechen-Link weitergeben, damit beide zur
            // Ausgangsseite zurueckkehren.

            // Pass "from" on to the form (hidden field) and the Cancel link, so both return to the
            // originating page.

            String from = escapeHtmlText(webserver.arg("from"));
            String returnTarget = fileManagerReturnTarget(webserver.arg("from"));

            String html = beginPage();
            html.reserve(1024);  // Umbenennen-Formular: klein
                                 // rename form: small
            html += "<h2>" + translate("Rename File") + "</h2>";
            html += "<form action='/rename' method='POST'>";
            html += "<input type='hidden' name='old' value='" + oldName + "'>";
            html += "<input type='hidden' name='from' value='" + from + "'>";
            html += "<label>" + translate("New Name") + ":</label><br>";
            html += "<input name='new' value='" + oldName + "' required><br><br>";
            html += "<button type='submit'>" + translate("Rename") + "</button></form>";
            html += "<br><a href='" + returnTarget + "'><button type='button'>" + translate("Cancel") + "</button></a></body></html>";
            webserver.send(200, "text/html", html);
            });

        // Datei umbenennen Aktion
        // Rename file action

        webserver.on("/rename", HTTP_POST, []() {

            // Nur aus einem privaten Netz (isPrivateNetworkIp(), wie /delete) - /rename_form zeigt nur das
            // Formular und bleibt frei erreichbar.

            // Only from a private network (isPrivateNetworkIp(), like /delete) - /rename_form only shows the
            // form and stays freely reachable.

            if (!isPrivateNetworkIp(webserver.client().remoteIP())) {
                webserver.send(200, "text/html", simpleMessagePage(translate("Rename"), "<p>" + translate("This action is only available when accessing the clock from a private network") + ".</p>"));
                return;
            }

            if (webserver.hasArg("old") && webserver.hasArg("new")) {
                String oldName = webserver.arg("old");
                String newName = webserver.arg("new");

                //oldName.replace(".", ""); newName.replace(".", "");
                if (!oldName.startsWith("/")) oldName = "/" + oldName;
                if (!newName.startsWith("/")) newName = "/" + newName;

                // Neuen Dateinamen pruefen: Dateinamen landen ungeescaped in HTML-Attributen (/files,
                // /listfilesFaces, /handsets) - ein "'" oder "<" ermoeglichte dort gespeichertes XSS.

                // Validate the new filename: filenames get embedded unescaped into HTML attributes (/files,
                // /listfilesFaces, /handsets) - a "'" or "<" would enable stored XSS there.

                // Von der Firmware erzeugte Dateien weder umbenennen noch durch Umbenennen ueberschreiben
                // Neither rename files created by the firmware nor overwrite them by renaming

                if (isProtectedFile(oldName) || isProtectedFile(newName)) {
                    webserver.send(403, "text/html", simpleMessagePage(translate("Rename"), "<p>" + translate("Built-in file cannot be changed") + ".</p>"));
                    return;
                }

                bool newNameValid = (newName.length() > 1 && newName.length() < 96);
                for (size_t i = 1; newNameValid && i < newName.length(); i++) {
                    char c = newName[i];
                    if (!(isalnum((unsigned char)c) || c == '_' || c == '-' || c == '.')) {
                        newNameValid = false;
                    }
                }
                if (!newNameValid) {
                    webserver.send(400, "text/plain", "Invalid new file name");
                    return;
                }

                if (LittleFS.exists(oldName)) {

                    // Kollision ablehnen statt stillschweigend zu ueberschreiben.
                    // Reject a collision instead of silently overwriting it.

                    if (newName != oldName && LittleFS.exists(newName)) {
                        webserver.send(409, "text/plain", "A file with the new name already exists");
                        return;
                    }
                    if (LittleFS.rename(oldName, newName)) {

                        // Streifen-Grafik, Streifen- und Zifferblatt-Einstellungen eines Zifferblatts mit umbenennen
                        // Rename a clock face's strip graphic, strip and clock face settings as well

                        String oldStrip = stripPathForFace(oldName), newStrip = stripPathForFace(newName);
                        if (oldStrip.length() && newStrip.length() && LittleFS.exists(oldStrip) && !LittleFS.exists(newStrip)) {
                            LittleFS.rename(oldStrip, newStrip);
                        }
                        for (const char* prefix : { "stripcfg_", "facecfg_" }) {
                            String oldCfg = faceSidePath(oldName, prefix), newCfg = faceSidePath(newName, prefix);
                            if (oldCfg.length() && newCfg.length() && LittleFS.exists(oldCfg) && !LittleFS.exists(newCfg)) {
                                LittleFS.rename(oldCfg, newCfg);
                            }
                        }
                        stripImageFor = "?";
                        stripSettingsFor = "?";
                        faceSettingsFor = "?";
                        infoStripDirty[0] = infoStripDirty[1] = true;

                        // Aktives Zifferblatt: Preference mitziehen, sonst zeigt
                        // sie auf eine nicht mehr existierende Datei.

                        // Active clock face: update the preference too,
                        // otherwise it points at a file that no longer exists.

                        if (preferences.getString(PK_BACKGROUND, "") == oldName) {
                            preferences.putString(PK_BACKGROUND, newName);
                            selectedBackground = newName;
                            freeClockFaceBuffer();
                            loadClockFace();
                        }

                        String redirectTarget = fileManagerReturnTarget(webserver.arg("from"));
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

            // escapeHtmlText(): "file" kommt aus der URL - siehe identischer
            // Kommentar bei /rename_form weiter oben.

            // escapeHtmlText(): "file" comes from the URL - see the identical
            // comment at /rename_form further above.

            String src = escapeHtmlText(webserver.arg("file"));
            String html = beginPage();
            html.reserve(1536);  // BMP-Skalieren-Formular: klein
                                 // scale-BMP form: small
            html += "<h2>" + translate("Scale and Save BMP") + "</h2>";
            html += "<form action='/scalebmp_run' method='GET'>";
            html += translate("Source") + ": <input name = 'src' value = '/" + src + "' readonly><br>";
            html += translate("Target") + ": <input name = 'dst' value = '/scaled_" + src + "'><br>";

            // Zeigerdateien: aktuelle Groesse vorschlagen statt der Zifferblattgroesse -
            // eine andere Groesse verzerrt den Zeiger und verschiebt den Drehpunkt.

            // Hand files: suggest the current size instead of the clock-face size -
            // a different size distorts the hand and shifts the pivot.

            int32_t suggestW = CLOCK_WIDTH, suggestH = CLOCK_HEIGHT;
            String scalePath = webserver.arg("file");
            if (!scalePath.startsWith("/")) scalePath = "/" + scalePath;
            bool isHandFile = scalePath.startsWith("/hand_set");
            if (isHandFile) {
                int32_t fileW, fileH;
                if (readImageSize(scalePath.c_str(), fileW, fileH)) {
                    suggestW = fileW;
                    suggestH = fileH;
                }
                else {
                    suggestW = HAND_LEGACY_WIDTH;
                    suggestH = HAND_LEGACY_HEIGHT;
                }
            }
            html += translate("Width") + ": <input name='w' type='number' value='" + String(suggestW) + "' required><br>";
            html += translate("Height") + ": <input name = 'h' type = 'number' value = '" + String(suggestH) + "' required><br>";
            if (isHandFile) {
                html += "<small>" + translate("Hands: keep the current size, other sizes distort the hand and shift the pivot") + " (" +
                        String(HAND_LEGACY_WIDTH) + "/" + String(HAND_WIDTH) + " x " + String(HAND_LEGACY_HEIGHT) + "/" + String(HAND_HEIGHT) + ")</small><br>";
            }
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

            if (isProtectedFile(dst)) {
                webserver.send(403, "text/html", simpleMessagePage(translate("Failed to scale BMP"), "<p>" + translate("Built-in file cannot be changed") + ".</p><a href='/files'><button type='button'>" + translate("Back") + "</button></a>"));
                return;
            }
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

            stationMode = preferences.getBool(PK_STATION_MODE, false);
            showSecondHand = preferences.getBool(PK_SHOW_SECOND_HAND, true);
            smoothMinute = preferences.getBool(PK_SMOOTH_MINUTE, false);
            smoothSecond = preferences.getBool(PK_SMOOTH_SECOND, false);


            stationMode = webserver.hasArg("stationMode");
            showSecondHand = webserver.hasArg("showSecondHand");
            smoothMinute = webserver.hasArg("smoothMinute");
            smoothSecond = webserver.hasArg("smoothSecond");

            // Ohne Sekundenzeiger haben Bahnhofsmodus und sanfter Sekundenzeiger keine Bedeutung - aus, wie im Formular
            // Without a second hand, station mode and smooth second hand have no meaning - off, as in the form

            if (!showSecondHand) {
                stationMode = false;
                smoothSecond = false;
            }


            // Logging-Einstellung speichern - beim Ausschalten alle Logdateien loeschen und den Zaehler
            // zuruecksetzen (deleteAllLogFiles()).

            // Save the logging setting - when switching it off, delete all log files and reset the counter
            // (deleteAllLogFiles()).

            bool wasLoggingEnabled = preferences.getBool(PK_LOGGING_ENABLED, false);
            loggingEnabled = webserver.hasArg("loggingEnabled");
            preferences.putBool(PK_LOGGING_ENABLED, loggingEnabled);

            if (wasLoggingEnabled && !loggingEnabled) {
                deleteAllLogFiles();

                // Gepufferte Log-Zeilen verwerfen - ein spaeteres Leeren schriebe sie sonst in die gerade
                // geloeschte Datei.

                // Discard buffered log lines - a later flush would otherwise write them into the file just
                // deleted.

                if (logBufferMutex != nullptr && xSemaphoreTake(logBufferMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                    logLineBuffer = "";
                    xSemaphoreGive(logBufferMutex);
                }
            }

            // DCF77-LED nur speichern, wenn die Checkbox ueberhaupt gerendert
            // wurde (Hardware da + dcf77Confirmed) - sonst wuerde jedes Speichern
            // vor Erstsync die Einstellung unbemerkt auf "aus" ueberschreiben.

            // Only save the DCF77 LED setting when the checkbox was actually
            // rendered (hardware present + dcf77Confirmed) - otherwise every
            // save before first sync would silently overwrite it to "off".

            if (dcf77Confirmed && LED_BOARD >= 0) {
                dcfSyncLedEnabled = webserver.hasArg("dcfSyncLed");
                preferences.putBool(PK_DCF_SYNC_LED, dcfSyncLedEnabled);
            }

            wifiActive = webserver.hasArg("wifiActive");
            preferences.putBool(PK_WIFI_ACTIVE, wifiActive);

            // Rocrail-Master-Schalter: schaltet Nutzung und Sichtbarkeit des
            // Rocrail-Tabs frei (siehe generateSettingsTabNav()). Beim
            // Deaktivieren eine evtl. offene Verbindung sofort trennen.

            // Rocrail master switch: unlocks use and visibility of the
            // Rocrail tab (see generateSettingsTabNav()). When disabling,
            // immediately close any open connection.

            setRocrailEnabled(webserver.hasArg("rocrailEnabled"));

            preferences.putBool(PK_STATION_MODE, stationMode);
            preferences.putBool(PK_SHOW_SECOND_HAND, showSecondHand);
            preferences.putBool(PK_SMOOTH_MINUTE, smoothMinute);
            preferences.putBool(PK_SMOOTH_SECOND, smoothSecond);

            // Rotation validieren, dann gemeinsam uebernehmen. Bewusst NICHT
            // argToIntClamped() (klemmt ungueltige Werte statt sie zu verwerfen) -
            // ein Tippfehler wuerde sonst ein Display faelschlich deaktivieren.

            // Validate rotation, then apply together. Deliberately NOT
            // argToIntClamped() (clamps invalid values instead of rejecting
            // them) - a typo could otherwise wrongly disable a display.

            if (webserver.hasArg("rotation") || webserver.hasArg("rotation2")) {
                uint8_t newRotation1 = tftRotation1;
                uint8_t newRotation2 = tftRotation2;

                if (webserver.hasArg("rotation")) {
                    long requestedRotation = webserver.arg("rotation").toInt();

                    // "automatisch" (nur mit Lagesensor): Rotation bleibt; beim Einschalten nimmt imuCalibrate()
                    // die aktuelle Lage als Bezug fuer die gerade eingestellte Rotation

                    // "automatic" (only with a motion sensor): the rotation stays; when switching on,
                    // imuCalibrate() takes the current position as the reference for the rotation set right now

                    bool wantAuto = requestedRotation == TFT_ROTATION_AUTO && imuAvailable();
                    if (wantAuto && !autoRotation) imuCalibrate(tftRotation1 < TFT_ROTATION_NA ? tftRotation1 : 0);
                    if (wantAuto != autoRotation) {
                        autoRotation = wantAuto;
                        preferences.putBool(PK_AUTO_ROTATION, autoRotation);
                        DEBUG_PRINTLN(String("[IMU] Automatic rotation ") + (autoRotation ? "on" : "off"));
                    }
                    if (wantAuto && newRotation1 == TFT_ROTATION_NA) newRotation1 = 0;
                    if (!wantAuto && requestedRotation >= 0 && requestedRotation <= TFT_ROTATION_NA) {
                        newRotation1 = (uint8_t)requestedRotation;
                    }
                }
                if (webserver.hasArg("rotation2")) {
                    long requestedRotation2 = webserver.arg("rotation2").toInt();
                    if (requestedRotation2 >= 0 && requestedRotation2 <= TFT_ROTATION_NA) {
                        newRotation2 = (uint8_t)requestedRotation2;
                    }
                }

                applyDisplayRotation(1, newRotation1);
                applyDisplayRotation(2, newRotation2);

                freeClockFaceBuffer();
                loadClockFace();      // neu zeichnen mit neuer Ausrichtung
                                      // redraw with new orientation
                loadHandSprites();
            }


            redirectTo("/?tab=zifferblatt&msg=Settings%20saved");
            });


        // Alte Adresse der Helligkeitsseite - die Einstellungen stehen im Tab Helligkeit der Hauptseite
        // Old address of the brightness page - the settings are in the Brightness tab of the main page

        webserver.on("/brightness", HTTP_GET, []() {
            redirectTo("/?tab=helligkeit");
            });

        // Helligkeitseinstellungen speichern
        // Save brightness settings

        webserver.on("/save_brightness", HTTP_POST, []() {
            setAutoBrightness(webserver.hasArg("use_adc")); // schaltet auch die Teiler-Pins (display.h)
                                                            // also switches the divider pins (display.h)
            adcInverted = webserver.hasArg("adcInverted");
            lowThreshold = argToIntClamped("lowThreshold", lowThreshold, 0, 100);
            highThreshold = argToIntClamped("highThreshold", highThreshold, 0, 100);

            maxBrightness = (uint8_t)argToIntClamped("maxBrightness", maxBrightness, 0, 255);
            minBrightness = (uint8_t)argToIntClamped("minBrightness", minBrightness, 0, 255);

            // neue: Zeitabhängige Helligkeit speichern
            // new: save time-based brightness


            brightStartHour = (uint8_t)argToIntClamped("brightStart", brightStartHour, 0, 23);
            brightEndHour = (uint8_t)argToIntClamped("brightEnd", brightEndHour, 0, 23);

            // Gamma-Feld gibt es nur mit Backlight und Lichtsensor (brightnessFormFieldsHtml())
            // The gamma field only exists with a backlight and a light sensor (brightnessFormFieldsHtml())

            if (useBacklight && photoresistorFound && webserver.hasArg("gamma")) {
                gammaBrightness = constrain(webserver.arg("gamma").toFloat(), 0.1f, 3.0f);
                preferences.putFloat(PK_GAMMA_BRIGHTNESS, gammaBrightness);
            }

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

            // Backlight zuletzt: bei einem Wechsel setzt setBacklightMode() min.
            // Helligkeit/Schwellwerte auf die passenden Werksvorgaben - die eben
            // gespeicherten Werte gehoerten ja noch zum alten Verfahren.

            // Backlight last: on a change setBacklightMode() resets min.
            // brightness/thresholds to the matching factory defaults - the
            // values just saved still belonged to the old method.

            if (webserver.hasArg("useBacklightField")) {
                setBacklightMode(webserver.hasArg("useBacklight"));
            }

            // Nach dem Speichern zur aufrufenden Seite zurueckkehren (returnTo)
            // statt immer zum Tab-Hub. returnTo kommt als POST-Parameter an -
            // nur lokale Pfade akzeptieren, sonst waere ein offener Redirect moeglich.

            // After saving, return to the calling page (returnTo) instead of
            // always the tab hub. returnTo arrives as a POST parameter - only
            // accept local paths, otherwise an open redirect would be possible.

            String returnTo = webserver.hasArg("returnTo") ? webserver.arg("returnTo") : "/?tab=helligkeit";
            if (!returnTo.startsWith("/") || returnTo.indexOf("://") >= 0) {
                returnTo = "/?tab=helligkeit";
            }
            String sep = (returnTo.indexOf('?') >= 0) ? "&" : "?";
            redirectTo(returnTo + sep + "msg=Settings%20saved");
            });


        // Rocrail-Modellzeit: Liste moeglicher Server speichern - erst alle
        // eingereichten Host/Port-Paare uebernehmen, dann Luecken heraus-
        // ziehen ("reorganisieren"), in die Preferences schreiben und den angehakten Eintrag als aktiven Server uebernehmen.

        // Rocrail model time: save the list of possible servers - first
        // take over all submitted host/port pairs, then pull out gaps
        // ("reorganize"), write to preferences and take over the checked entry as the active server.

        webserver.on("/save_rocrail", HTTP_POST, []() {

            // Eingereichte Werte uebernehmen (nur veraenderte Felder werden
            // gesendet, hasArg() deckt trotzdem beide Faelle ab).

            // Take over submitted values (only changed fields are sent,
            // hasArg() covers both cases regardless).

            for (int i = 0; i < MAX_WLAN; i++) {
                String hostArg = pkRocrailServerHost(i);
                if (webserver.hasArg(hostArg)) {
                    String host = webserver.arg(hostArg);
                    host.trim();
                    strncpy(rocrailServerList[i], host.c_str(), sizeof(rocrailServerList[i]) - 1);
                    rocrailServerList[i][sizeof(rocrailServerList[i]) - 1] = '\0';
                }
                String portArg = pkRocrailServerPort(i);
                if (webserver.hasArg(portArg)) {
                    rocrailServerPortList[i] = (uint16_t)argToIntClamped(portArg, ROCRAIL_DEFAULT_PORT, 1, 65535);
                }

                // Anlagenname: rein manuell, dient nur der eigenen Orientierung.
                // Layout name: purely manual, only for the user's own reference.

                String nameArg = pkRocrailServerName(i);
                if (webserver.hasArg(nameArg)) {
                    String name = webserver.arg(nameArg);
                    name.trim();
                    strncpy(rocrailServerNameList[i], name.c_str(), sizeof(rocrailServerNameList[i]) - 1);
                    rocrailServerNameList[i][sizeof(rocrailServerNameList[i]) - 1] = '\0';
                }
            }

            // Welcher Eintrag wurde angehakt? (Radio-Button "activeServer" -
            // pro Definition hoechstens einer gleichzeitig ausgewaehlt.)

            // Which entry was checked? (Radio button "activeServer" - by
            // definition at most one selected at a time.)

            int requestedActive = -1;
            if (webserver.hasArg("activeServer")) {
                requestedActive = webserver.arg("activeServer").toInt();
                if (requestedActive < 0 || requestedActive >= MAX_WLAN) requestedActive = -1;
            }

            // Reorganisieren: nicht-leere Eintraege nach vorn ruecken lassen,
            // dabei verfolgen, an welchem neuen Platz der angehakte Eintrag landet (falls er verschoben wurde).

            // Reorganize: let non-empty entries move to the front, tracking
            // which new slot the checked entry ends up at (if it moved).

            int writeIndex = 0;
            int newActiveIndex = -1;
            for (int readIndex = 0; readIndex < MAX_WLAN; readIndex++) {
                if (strlen(rocrailServerList[readIndex]) > 0) {
                    if (writeIndex != readIndex) {
                        strncpy(rocrailServerList[writeIndex], rocrailServerList[readIndex], sizeof(rocrailServerList[writeIndex]) - 1);
                        rocrailServerList[writeIndex][sizeof(rocrailServerList[writeIndex]) - 1] = '\0';
                        rocrailServerPortList[writeIndex] = rocrailServerPortList[readIndex];
                        strncpy(rocrailServerNameList[writeIndex], rocrailServerNameList[readIndex], sizeof(rocrailServerNameList[writeIndex]) - 1);
                        rocrailServerNameList[writeIndex][sizeof(rocrailServerNameList[writeIndex]) - 1] = '\0';
                        memset(rocrailServerList[readIndex], 0, sizeof(rocrailServerList[readIndex]));
                        rocrailServerPortList[readIndex] = ROCRAIL_DEFAULT_PORT;
                        memset(rocrailServerNameList[readIndex], 0, sizeof(rocrailServerNameList[readIndex]));
                    }
                    if (readIndex == requestedActive) newActiveIndex = writeIndex;
                    writeIndex++;
                }
            }
            for (int i = writeIndex; i < MAX_WLAN; i++) {
                memset(rocrailServerList[i], 0, sizeof(rocrailServerList[i]));
                rocrailServerPortList[i] = ROCRAIL_DEFAULT_PORT;
                memset(rocrailServerNameList[i], 0, sizeof(rocrailServerNameList[i]));
            }

            // Ist keiner angehakt, aber mindestens ein Server vorhanden -
            // den ersten nehmen, statt Rocrail dann stillschweigend inaktiv zu lassen.

            // If none is checked but at least one server exists - use the
            // first one, instead of silently leaving Rocrail inactive.

            if (newActiveIndex < 0 && writeIndex > 0) newActiveIndex = 0;
            rocrailActiveServerIndex = newActiveIndex;

            // In den Preferences sichern - nur bei tatsaechlicher Aenderung
            // schreiben, wie bei updateNtpServersFromRequest().

            // Persist to preferences - only write on an actual change, as in
            // updateNtpServersFromRequest().

            for (int i = 0; i < MAX_WLAN; i++) {
                String hostKey = pkRocrailServerHost(i);
                if (preferences.getString(hostKey.c_str(), "") != String(rocrailServerList[i])) {
                    preferences.putString(hostKey.c_str(), rocrailServerList[i]);
                }
                String portKey = pkRocrailServerPort(i);
                if (preferences.getUShort(portKey.c_str(), ROCRAIL_DEFAULT_PORT) != rocrailServerPortList[i]) {
                    preferences.putUShort(portKey.c_str(), rocrailServerPortList[i]);
                }
                String nameKey = pkRocrailServerName(i);
                if (preferences.getString(nameKey.c_str(), "") != String(rocrailServerNameList[i])) {
                    preferences.putString(nameKey.c_str(), rocrailServerNameList[i]);
                }
            }
            if (preferences.getInt(PK_ROCRAIL_ACTIVE_SRV, -1) != rocrailActiveServerIndex) {
                preferences.putInt(PK_ROCRAIL_ACTIVE_SRV, rocrailActiveServerIndex);
            }

            // Angehakten Eintrag als aktuell aktiven Server uebernehmen -
            // rocrail_client.h kennt die Liste selbst nicht und liest
            // weiterhin nur rocrailServerHost/-Port.

            // Take over the checked entry as the currently active server -
            // rocrail_client.h doesn't know about the list itself and
            // continues to read only rocrailServerHost/-Port.

            String newServer = (rocrailActiveServerIndex >= 0) ? String(rocrailServerList[rocrailActiveServerIndex]) : "";
            uint16_t newServerPort = (rocrailActiveServerIndex >= 0) ? rocrailServerPortList[rocrailActiveServerIndex] : ROCRAIL_DEFAULT_PORT;

            bool serverChanged = (newServer != rocrailServerHost) || (newServerPort != rocrailServerPort);

            rocrailServerHost = newServer;
            rocrailServerPort = newServerPort;

            preferences.putString(PK_ROCRAIL_SERVER, rocrailServerHost);
            preferences.putUShort(PK_ROCRAIL_SRV_PORT, rocrailServerPort);

            if (serverChanged) {

                // Bestehende Verbindung trennen, damit connectRocrailClient() sofort den neuen Server
                // versucht. Der Anlagenname muss nicht geleert werden - er haengt am Listenplatz, nicht am
                // aktiven Server.

                // Disconnect the existing connection, so connectRocrailClient() tries the new server
                // immediately. The layout name doesn't need clearing - it lives per list slot, not on the
                // active server.

                if (rocrailClient.connected()) rocrailClient.stop();
                rocrailConnected = false;
                rocrailLastConnectAttemptMillis = 0;
            }

            // Sofort versuchen statt auf das naechste Zeitfenster zu warten -
            // no-op, falls Rocrail nicht aktiv ist oder bereits verbunden.

            // Try immediately instead of waiting for the next window - a
            // no-op if Rocrail isn't enabled or already connected.

            triggerRocrailConnectNow();

            redirectTo("/?tab=rocrail&msg=Settings%20saved");
            });


        // Live-Status fuer den Rocrail-Tab: pollt alle paar Sekunden per JS
        // (siehe panel-rocrail) - dieselbe Herangehensweise wie /api/topbarStatus.

        // Live status for the Rocrail tab: polled every few seconds via JS
        // (see panel-rocrail) - the same approach as /api/topbarStatus.

        webserver.on("/api/rocrailStatus", HTTP_GET, []() {
            webserver.sendHeader("Cache-Control", "no-store");

            // "-" bis das erste <clock>-Update tatsaechlich eingetroffen ist
            // (rocrailLastClockMillis != 0) - vorher waeren Modellzeit UND
            // Divider nur der Default-Wert, nicht vom Server bestaetigt.

            // "-" until the first <clock> update has actually arrived
            // (rocrailLastClockMillis != 0) - before that, both the model
            // time AND the divider would just be the default value, not confirmed by the server.

            char modelTimeBuf[9] = "-";
            String dividerStr = "-";
            if (rocrailEnabled && rocrailLastClockMillis != 0) {
                snprintf(modelTimeBuf, sizeof(modelTimeBuf), "%02d:%02d:%02d",
                         rocrailTimeinfo.tm_hour, rocrailTimeinfo.tm_min, rocrailTimeinfo.tm_sec);
                dividerStr = String(rocrailDivider);
            }

            String json = "{";
            json += "\"enabled\":" + String(rocrailEnabled ? "true" : "false") + ",";
            json += "\"connected\":" + String(rocrailConnected ? "true" : "false") + ",";

            // "ready": TCP steht UND mindestens ein <clock>-Update kam an -
            // erst dann sind Divider/Modellzeit echte Werte. Fuer "Verbunden"
            // statt nur rocrailConnected, damit der Status nicht gruen zeigt, waehrend Divider/Modellzeit noch "-" zeigen.

            // "ready": TCP is up AND at least one <clock> update has
            // arrived - only then are divider/model time real values. Used
            // for "Connected" instead of just rocrailConnected, so status isn't green while divider/model time still show "-".

            json += "\"ready\":" + String((rocrailEnabled && rocrailConnected && rocrailLastClockMillis != 0) ? "true" : "false") + ",";
            json += "\"frozen\":" + String(rocrailFrozen ? "true" : "false") + ",";
            json += "\"host\":\"" + escapeJsonText(rocrailServerHost) + "\",";
            json += "\"port\":" + String(rocrailServerPort) + ",";
            json += "\"divider\":\"" + dividerStr + "\",";
            json += "\"modelTime\":\"" + String(modelTimeBuf) + "\"";
            json += "}";

            webserver.send(200, "application/json", json);
            });


        // Alle Dateien auflisten
        // List all files

        webserver.on("/files", HTTP_GET, []() {
            webserver.setContentLength(CONTENT_LENGTH_UNKNOWN);
            webserver.send(200, "text/html", "");

            String chunk = beginPage();
            chunk.reserve(1024);
            chunk += generateFlashMessage();

            chunk += "<h2>" + translate("All Files on LittleFS") + "</h2>";
            chunk += "<p>" + generateStorageInfo(LittleFS.usedBytes(), LittleFS.totalBytes()) + "</p>"; // wie bei Zifferblaettern/Zeigern
                                                                                                        // as for faces/hands

            // Feste Breite, alle Spalten linksbuendig; die Tabellen aller Abschnitte teilen dasselbe Raster (Auswahl
            // schmal, die uebrigen fuenf gleich breit). Schmale Fenster scrollen nur die Liste, nicht die Seite.

            // Fixed width, all columns left-aligned; the tables of all sections share the same grid (selection narrow,
            // the other five equally wide). Narrow windows scroll only the list, not the page.

            chunk += "<style>.fm{width:960px;max-width:100%;overflow-x:auto;margin:0 auto;text-align:left;}";
            chunk += ".fm table{table-layout:fixed;width:960px;}.fm th,.fm td{text-align:left;overflow-wrap:anywhere;vertical-align:top;}</style>";
            chunk += "<div class='fm'>";
            chunk += "<label><input type='checkbox' id='fselAll'> " + translate("Select all") + "</label>";
            webserver.sendContent(chunk);
            chunk = "";

            // Name, Groesse und Datum beim Durchlaufen des Verzeichnisses lesen (ohne jede Datei einzeln zu oeffnen)
            // und natuerlich sortieren - Zahlen im Namen numerisch, z.B. hand_set2 vor hand_set10.

            // Read name, size and date while walking the directory (without opening every file separately) and sort
            // naturally - numbers in the name numerically, e.g. hand_set2 before hand_set10.

            struct FileEntry {
                String name;
                size_t size;
                time_t modified;
            };
            std::vector<FileEntry> entries;
            File root = LittleFS.open("/");
            File file = root.openNextFile();
            while (file) {
                entries.push_back({ String(file.name()), file.size(), file.getLastWrite() });
                file = root.openNextFile();
            }
            for (size_t i = 1; i < entries.size(); i++) { // Einfuegesortieren wie naturalSortNames()
                                                          // insertion sort like naturalSortNames()
                FileEntry current = entries[i];
                size_t j = i;
                while (j > 0 && naturalLess(current.name, entries[j - 1].name)) {
                    entries[j] = entries[j - 1];
                    j--;
                }
                entries[j] = current;
            }

            // Abschnitte je Dateiart, einklappbar (Zustand merkt sich der Browser, siehe Skript unten)
            // Sections per file type, collapsible (the browser remembers the state, see the script below)

            static const char* const groupKeys[] = { "faces", "strips", "hands", "fonts", "logs", "other" };
            static const char* const groupTitles[] = { "Clock faces", "Strip graphics", "Hand sets", "Fonts", "Log files", "Other files" };
            auto groupOf = [](const String& n) -> int {
                if (n.startsWith("face_")) return 0;
                if (n.startsWith("strip_")) return 1;
                if (n.startsWith("hand_set")) return 2;
                if (n.startsWith("font_") || n.startsWith("stripfont_")) return 3;
                if (n.endsWith(".log")) return 4;
                return 5;
            };

            int rowCount = 0;
            for (int g = 0; g < 6; g++) {
                size_t count = 0, bytes = 0;
                for (const FileEntry& e : entries) {
                    if (groupOf(e.name) == g) { count++; bytes += e.size; }
                }
                if (!count) continue;
                chunk += "<details open data-g='" + String(groupKeys[g]) + "' style='margin:12px 0;'>";
                chunk += "<summary style='cursor:pointer;font-weight:bold;'>" + translate(groupTitles[g]) + " (" + String(count) + ", " + String((bytes + 1023) / 1024) + " KB)</summary>";
                chunk += "<table border = '1'><colgroup><col style='width:36px'><col><col><col><col><col></colgroup>";
                chunk += "<tr><th><input type='checkbox' class='fselGroup' title='" + translate("Select all") + "'></th><th>" + translate("Filename") + "</th><th>" + translate("Size(bytes)") + "</th><th>" + translate("Modified") + "</th><th>" + translate("Info") + "</th><th>" + translate("Action") + "</th></tr>";

                for (const FileEntry& e : entries) {
                    if (groupOf(e.name) != g) continue;
                    const String& name = e.name;
                    String openPath = name.startsWith("/") ? name : "/" + name;
                    String info = getBmpInfo(name);

                    // Aenderungsdatum (LittleFS speichert es) - Dateien von vor der ersten Uhrzeit (um 1970) ohne Datum
                    // Modification date (LittleFS stores it) - files from before the first time (around 1970) without date

                    String modifiedText = "&ndash;";
                    if (e.modified > 1577836800) { // nach dem 1.1.2020 / after 1 Jan 2020
                        struct tm lt;
                        localtime_r(&e.modified, &lt);
                        char buf[24];
                        strftime(buf, sizeof(buf), "%d.%m.%Y %H:%M:%S", &lt);
                        modifiedText = buf;
                    }

                    // Zeigerdateien mit Formatangabe: in einem Satz sind verschiedene
                    // gueltige Groessen normal (der Designer speichert so klein wie moeglich).

                    // Hand files with a format label: different valid sizes within one set
                    // are normal (the designer saves as small as possible).

                    if (name.startsWith("hand_set") && name.endsWith(".bmp")) {
                        String label = handFormatLabel(openPath);
                        if (label.length()) info += " (" + label + ")";
                    }
                    chunk += "<tr><td><input type='checkbox' class='fsel' value='" + name + "'></td><td>" + name + "</td><td>" + String(e.size) + "</td>";
                    chunk += "<td>" + modifiedText + "</td>";
                    chunk += "<td>" + String(info) + "</td>";
                    chunk += " <td><a href = '/delete?file=" + name + "&from=files' title='" + translate("Delete") + "' onclick = 'return confirm(\"" + translate("Delete") + " " + name + "?\")'>&#128465;&#65039;</a> ";

                    // Skalieren und Umbenennen nur fuer .bmp-Dateien
                    // Scale and rename only for .bmp files

                    if (name.endsWith(".bmp")) {
                        chunk += "<a href = '/scalebmp_form?file=" + name + "' title='" + translate("Scale") + "'>&#128208;</a> ";
                        chunk += "<a href='/rename_form?file=" + name + "&from=files' title='" + translate("Rename") + "'>&#9999;&#65039;</a> ";
                    }
                    else {
                        chunk += "<span style='opacity:0.25;' title='" + translate("Not applicable to this file type") + "'>&#128208;</span> ";
                        chunk += "<span style='opacity:0.25;' title='" + translate("Not applicable to this file type") + "'>&#9999;&#65039;</span> ";
                    }

                    // Ansehen nur, wo der Browser etwas anzeigen kann: Bilder (/file liefert RLE als normales BMP) und Text
                    // View only where the browser can show something: images (/file serves RLE as a standard BMP) and text

                    if (name.endsWith(".bmp") || name.endsWith(".log") || name.endsWith(".txt")) {
                        chunk += "<a href='/file?name=" + name + "' title='" + translate("View") + "'>&#128065;&#65039;</a> ";
                    }
                    else {
                        chunk += "<span style='opacity:0.25;' title='" + translate("Not applicable to this file type") + "'>&#128065;&#65039;</span> ";
                    }

                    chunk += "<a href='/download?file=" + name + "' title='" + translate("Download") + "'>&#11015;&#65039;</a> ";

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
                chunk += "</table></details>";
            }

            // Mehrfachauswahl: Texte als data-Attribute, damit der Browser die HTML-Entities der Uebersetzung aufloest
            // Multi-selection: texts as data attributes, so the browser resolves the translation's HTML entities

            chunk += "<button type='button' id='fselDel' data-none='" + translate("No files selected") + "' data-ask='" + translate("Delete the selected files?") + "'>" + translate("Delete selected") + "</button></div><br><br>";
            chunk += "<script>(function(){";
            chunk += "var all=document.getElementById('fselAll'),btn=document.getElementById('fselDel');";
            chunk += "all.onchange=function(){document.querySelectorAll('.fsel,.fselGroup').forEach(function(c){c.checked=all.checked;});};";
            chunk += "document.querySelectorAll('.fselGroup').forEach(function(g){g.onchange=function(){g.closest('table').querySelectorAll('.fsel').forEach(function(c){c.checked=g.checked;});};});";
            chunk += "var closed=[];try{closed=JSON.parse(localStorage.getItem('uhr4FilesClosed')||'[]');}catch(e){}";
            chunk += "document.querySelectorAll('details[data-g]').forEach(function(d){if(closed.indexOf(d.dataset.g)>=0)d.open=false;";
            chunk += "d.addEventListener('toggle',function(){var k=d.dataset.g,i=closed.indexOf(k);if(d.open&&i>=0)closed.splice(i,1);if(!d.open&&i<0)closed.push(k);";
            chunk += "try{localStorage.setItem('uhr4FilesClosed',JSON.stringify(closed));}catch(e){}});});";
            chunk += "btn.onclick=function(){var s=[].map.call(document.querySelectorAll('.fsel:checked'),function(c){return c.value;});";
            chunk += "if(!s.length){alert(btn.dataset.none);return;}if(!confirm(btn.dataset.ask+'\\n\\n'+s.join('\\n')))return;";
            chunk += "var f=document.createElement('form');f.method='POST';f.action='/deletemulti';";
            chunk += "s.forEach(function(n){var i=document.createElement('input');i.type='hidden';i.name='file';i.value=n;f.appendChild(i);});";
            chunk += "document.body.appendChild(f);f.submit();};})();</script>";
            chunk += "</body></html>";
            webserver.sendContent(chunk);
            webserver.sendContent(""); // Ende der Chunked-Uebertragung signalisieren
                                       // signal the end of the chunked transfer
            });

        webserver.on("/download", HTTP_GET, []() {
            if (webserver.hasArg("file")) {
                String path = webserver.arg("file");
                if (!path.startsWith("/")) path = "/" + path;

                // An einem eingebetteten Nullbyte kappen: endsWith() prueft die logische String-Laenge,
                // LittleFS nutzt .c_str() - sonst bestuende ".../log_1.log\0.bmp" die Endungspruefung und
                // oeffnete doch die Logdatei.

                // Truncate at an embedded null byte: endsWith() checks the logical String length, LittleFS
                // uses .c_str() - otherwise ".../log_1.log\0.bmp" would pass the extension check yet open the
                // log file.

                path = String(path.c_str());

                // Logdateien nur aus einem privaten Netz herunterladbar (koennen IPs, SSIDs u.ae. enthalten);
                // andere Dateien von ueberall.

                // Log files only downloadable from a private network (may contain IPs, SSIDs etc.); other
                // files from anywhere.

                if (path.endsWith(".log") && !isPrivateNetworkIp(webserver.client().remoteIP())) {
                    webserver.send(200, "text/plain", "This action is only available when accessing the clock from a private network.");
                    return;
                }

                if (LittleFS.exists(path)) {

                    // RLE-komprimierte face_*.bmp vor dem Download zu Standard-BMP dekodieren.
                    // Decode an RLE-compressed face_*.bmp to a standard BMP before download.

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

                        // Streaming fehlgeschlagen - Header evtl. schon gesendet, kein
                        // sauberer 500-Code mehr moeglich.

                        // Streaming failed - headers may already be sent, a clean
                        // 500 status is no longer possible.

                        DEBUG_PRINTLN("[DOWNLOAD] RLE streaming failed for " + path + " (from " + webserver.client().remoteIP().toString() + ")");
                        return;
                    }

                    File file = LittleFS.open(path, "r");

                    // open() kann trotz exists() fehlschlagen (Race mit /delete,
                    // /rename), und exists() ist bei Verzeichnissen ebenfalls true.

                    // open() can fail despite exists() (race with /delete,
                    // /rename), and exists() is also true for directories.

                    if (!file || file.isDirectory()) {
                        if (file) file.close();
                        webserver.send(404, "text/plain", "File not found");
                        return;
                    }

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

            // Nur aus einem privaten Netz anzeigen - zeigt WLAN-Modus, Reset-Grund, Speicher und
            // Rocrail-Server. Frueher Ausstieg moeglich, da die Seite fuer sich allein steht.

            // Only shown from a private network - shows WiFi mode, reset reason, storage and Rocrail server.
            // An early exit is possible since the page stands on its own.

            if (!isPrivateNetworkIp(webserver.client().remoteIP())) {
                String html = beginPage();
                html += "<h2>" + translate("System Status") + "</h2>";
                html += "<p>" + translate("Status information is only shown when accessing the clock from a private network") + ".</p>";
                html += "</body></html>";
                webserver.send(200, "text/html", html);
                return;
            }

            webserver.setContentLength(CONTENT_LENGTH_UNKNOWN);
            webserver.send(200, "text/html", "");

            String chunk = beginPage();
            chunk.reserve(1024);

            webserver.sendContent(chunk);
            chunk = "";

            chunk += "<h2>" + translate("System Status") + "</h2><ul>";
            generateStatusItems(chunk, [](String& part) { webserver.sendContent(part); part = ""; });
            chunk += "</ul>";
            chunk += "</br>";
            chunk += "<li>Contact: <a href='mailto:howl-clock@gmx.de'>howl-clock@gmx.de</a></li>";

            chunk += "<li>Project: <a href='" GITHUB_REPO_URL "' target='_blank'>GitHub</a></li>";

            chunk += "</ul>";
            chunk += "</body></html>";

            webserver.sendContent(chunk);
            webserver.sendContent(""); // Ende der Chunked-Uebertragung signalisieren
                                       // signal the end of the chunked transfer
            });

        // Streifen eines Zifferblatts fuer die Zifferblatt-Uebersicht (nur Displays mit Streifen, ILI9341)
        // A clock face's strip for the clock face overview (only displays with a strip, ILI9341)

        webserver.on("/strippreview", HTTP_GET, []() {
            String path = webserver.arg("file");
            if (!path.startsWith("/")) path = "/" + path;
            uint8_t* bmpBytes = nullptr;
            size_t bmpSize = 0;
            if (generateFaceStripBmp(path, 80, &bmpBytes, bmpSize)) {
                webserver.send_P(200, "image/bmp", (const char*)bmpBytes, bmpSize);
                delete[] bmpBytes;
            }
            else {
                webserver.send(404, "text/plain", "no strip");
            }
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
                if (webserver.hasArg("v")) webserver.sendHeader("Cache-Control", "public, max-age=31536000, immutable"); // Adresse traegt die Dateiversion
                                                                                                                      // the address carries the file version
                sendScaledBmpPreview(path, 80, 80);
            }
            else {
                webserver.send(400, "text/plain", "missing 'file' argument");
            }
            });

        // Vorschaubild der aktiven Einstellungen ohne Zeiger als BMP (spart ~150 KB Base64); die ESP32-Zeit,
        // mit der die Anzeige im Browser lokal weiterlaeuft; ein Inline-SVG als Favicon (kein 404, lange
        // Cache-Zeit).

        // Preview image of the active settings without hands as a BMP (saves ~150 KB base64); the ESP32 time
        // the browser display keeps running from locally; an inline SVG as favicon (no 404, long cache
        // lifetime).

        webserver.on("/favicon.ico", HTTP_GET, []() {
            webserver.sendHeader("Cache-Control", "public, max-age=86400");
            String svg = "<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 32 32'>"
                         "<circle cx='16' cy='16' r='14' fill='#10151a' stroke='#f5a623' stroke-width='2.5'/>"
                         "<line x1='16' y1='16' x2='16' y2='7' stroke='#f5a623' stroke-width='2.5' stroke-linecap='round'/>"
                         "<line x1='16' y1='16' x2='21' y2='16' stroke='#f5a623' stroke-width='2.5' stroke-linecap='round'/>"
                         "</svg>";
            webserver.send(200, "image/svg+xml", svg);
            });

        // Weist Suchmaschinen an, die Oberflaeche nicht zu indexieren - kein Zugriffsschutz, verhindert aber
        // Suchtreffer bei versehentlich oeffentlicher Erreichbarkeit. Lange Cache-Zeit.

        // Tells search engines not to index the interface - no access control, but prevents search results if
        // the clock is ever publicly reachable by accident. Long cache lifetime.

        webserver.on("/robots.txt", HTTP_GET, []() {
            webserver.sendHeader("Cache-Control", "public, max-age=86400");
            webserver.send(200, "text/plain", "User-agent: *\nDisallow: /\n");
            });

        // Uhrzeit vom Browser uebernehmen (Knopf in der Statusleiste und im Tab NTP Zeitzone): t = Unix-Sekunden
        // UTC mit Bruchteil, wie "UHR4 TIME" per USB (setClockTime()). Nur aus dem eigenen Netz.

        // Take over the time from the browser (button in the status bar and in the NTP timezone tab): t = unix
        // seconds UTC with fraction, like "UHR4 TIME" via USB (setClockTime()). Only from the own network.

        webserver.on("/api/setTime", HTTP_POST, []() {
            if (!isPrivateNetworkIp(webserver.client().remoteIP())) {
                webserver.send(403, "application/json", "{\"ok\":false}");
                return;
            }
            String localText;
            if (!setClockTime(webserver.arg("t"), "[WEB]", localText)) {
                webserver.send(400, "application/json", "{\"ok\":false}");
                return;
            }
            DEBUG_PRINTLN("[WEB] Time taken over from the browser on " + webserver.client().remoteIP().toString() + ": " + localText);
            webserver.send(200, "application/json", "{\"ok\":true,\"time\":\"" + localText + "\"}");
            });

        webserver.on("/api/currentTime", HTTP_GET, []() {
            webserver.sendHeader("Cache-Control", "no-store");

            // Dieselbe Bedingung wie rocrailTimeReady in renderClockFrame()
            // (display.h) - die Web-Vorschau soll exakt dieselbe Quelle
            // anzeigen wie die Zeiger auf dem Display selbst.

            // Same condition as rocrailTimeReady in renderClockFrame()
            // (display.h) - the web preview should show exactly the same
            // source as the hands on the display itself.

            bool rocrailTimeReady = rocrailEnabled && rocrailConnected && rocrailLastClockMillis != 0 &&
                                     (millis() - rocrailLastClockMillis) < ROCRAIL_STALE_TIMEOUT_MS;
            struct tm t = rocrailTimeReady ? rocrailTimeinfo : timeinfo;

            // Millisekunden dazu, sonst setzt jeder Abgleich die Vorschau um bis zu 1 s daneben. Ohne Rocrail frisch
            // aus der Systemzeit, solange sie gueltig ist (sonst rechnet die Uhr selbst aus timeinfo weiter).

            // Plus milliseconds, otherwise every sync puts the preview up to 1 s off. Without Rocrail fresh from the
            // system time as long as it is valid (otherwise the clock itself keeps extrapolating from timeinfo).

            int ms = 0;
            if (rocrailTimeReady) {
                ms = constrain((int)((rocrailSecFrac - t.tm_sec) * 1000.0f), 0, 999);
            }
            else {
                struct timeval tv;
                gettimeofday(&tv, nullptr);
                time_t sec = tv.tv_sec;
                struct tm fresh;
                localtime_r(&sec, &fresh);
                if (fresh.tm_year >= 100) {
                    t = fresh;
                    ms = tv.tv_usec / 1000;
                }
            }

            String json = "{\"hour\":" + String(t.tm_hour) +
                          ",\"minute\":" + String(t.tm_min) +
                          ",\"second\":" + String(t.tm_sec) +
                          ",\"ms\":" + String(ms) +
                          ",\"rocrail\":" + String(rocrailTimeReady ? "true" : "false") +
                          ",\"divider\":" + String(rocrailTimeReady ? rocrailDivider : 1) +
                          ",\"frozen\":" + String((rocrailTimeReady && rocrailFrozen) ? "true" : "false") +
                          ",\"previewSig\":\"" + escapeJsonText(currentPreviewSignature()) + "\"}";
            webserver.send(200, "application/json", json);
            });

        // Zustand der Topbar-Punkte als JSON fuer das Live-Status-Skript - gleiche Bedingungen wie in
        // generateTopBar(), beide gemeinsam aendern. rtcPresent/dcf77Present/lightValue schalten live um.

        // State of the topbar dots as JSON for the live-status script - same conditions as in
        // generateTopBar(), change both together. rtcPresent/dcf77Present/lightValue toggle live.

        webserver.on("/api/topbarStatus", HTTP_GET, []() {
            webserver.sendHeader("Cache-Control", "no-store");

            // Echte Werte nur fuer ein privates Netz - das Polling laeuft auf jeder Seite und wuerde sonst
            // die Sperre des Status-Tabs umgehen. Sonst vollstaendiges, neutrales JSON, damit das Skript
            // keine "undefined" zeigt.

            // Real values only for a private network - the polling runs on every page and would otherwise
            // bypass the Status tab's block. Otherwise complete neutral JSON, so the script shows no
            // "undefined".

            if (!isPrivateNetworkIp(webserver.client().remoteIP())) {
                webserver.send(200, "application/json",
                    "{\"time\":\"na\",\"rtc\":\"na\",\"rtcPresent\":false,\"dcf77\":\"na\",\"dcf77Present\":false,"
                    "\"rocrailEnabled\":false,\"rocrailConnected\":false,\"rocrailTitle\":\"\",\"lightValue\":\"\","
                    "\"datetime\":\"\",\"timeTitle\":\"\",\"rtcTitle\":\"\",\"dcf77Title\":\"\",\"version\":\"\"}");
                return;
            }

            String timeState = getTimeStatus();
            String rtcState = "na";
            String dcfState = "na";
            rtcState = getRtcStatus();
            dcfState = getDcf77Status();
            bool rtcPresent = (rtcState != "na");
            bool dcf77Present = (dcfState != "na");
            String lightValue = String(currentLightPercent) + " %";
            String datetimeStr = "";
            if (timeState == "ok") {
                char nowStr[20];
                strftime(nowStr, sizeof(nowStr), "%d.%m.%Y %H:%M", &timeinfo);
                datetimeStr = String(nowStr);
            }
            String timeTitle = dotStatusText(translate("Time"), timeState);
            String rtcTitle = dotStatusText("RTC", rtcState);
            String dcf77Title = dotStatusText("DCF77", dcfState);
            String rocrailTitle = dotStatusText("Rocrail", rocrailConnected ? "ok" : "error");

            // "version" mitschicken, damit das Topbar-Polling eine neue
            // Firmware-Version (OTA/WPS-Neustart) erkennt und die Seite dann
            // neu laedt - reiner Build-Zeitstempel, daher ohne JSON-Escaping sicher.

            // Include "version" so the topbar polling can detect a new
            // firmware version (OTA/WPS reboot) and reload the page then -
            // a plain build timestamp, so it's safe without JSON escaping.

            String json = "{\"time\":\"" + timeState + "\"" +
                          ",\"rtc\":\"" + rtcState + "\"" +
                          ",\"rtcPresent\":" + String(rtcPresent ? "true" : "false") +
                          ",\"dcf77\":\"" + dcfState + "\"" +
                          ",\"dcf77Present\":" + String(dcf77Present ? "true" : "false") +
                          ",\"rocrailEnabled\":" + String(rocrailEnabled ? "true" : "false") +
                          ",\"rocrailConnected\":" + String(rocrailConnected ? "true" : "false") +
                          ",\"rocrailTitle\":\"" + rocrailTitle + "\"" +
                          ",\"lightValue\":\"" + lightValue + "\"" +
                          ",\"datetime\":\"" + datetimeStr + "\"" +
                          ",\"timeTitle\":\"" + timeTitle + "\"" +
                          ",\"rtcTitle\":\"" + rtcTitle + "\"" +
                          ",\"dcf77Title\":\"" + dcf77Title + "\"" +
                          ",\"version\":\"" + String(version) + "\"}";
            webserver.send(200, "application/json", json);
            });

        // Liefert den Live-Fortschritt des aktuellen DCF77-Telegramms sowie
        // das letzte vollstaendig dekodierte Telegramm als JSON, fuer das
        // sekuendliche Polling der /dcf77-Seite. "bits": '0'/'1'/'?' je Position.

        // Returns the live progress of the current DCF77 telegram as well
        // as the last fully decoded telegram as JSON, for the /dcf77
        // page's once-per-second polling. "bits": '0'/'1'/'?' per position.

        webserver.on("/api/dcf77status", HTTP_GET, []() {
            webserver.sendHeader("Cache-Control", "no-store");

            // dcf77Bits ist nach Rasterposition indiziert. Ist die
            // Minutenmarke bekannt, hier in die Sekundenreihenfolge des
            // Telegramms umsortieren, sonst werden die Rohpositionen ausgegeben.

            // dcf77Bits is indexed by grid position. When the minute
            // marker is known, reorder into the telegram's second order
            // here, otherwise the raw positions are output.

            String bits;
            bits.reserve(DCF77_TELEGRAM_BITS);
            for (uint8_t i = 0; i < DCF77_TELEGRAM_BITS; i++) {
                uint8_t slot = (dcf77MarkerPos >= 0)
                                   ? (uint8_t)(((uint8_t)dcf77MarkerPos + 1 + i) % DCF77_GRID_SLOTS)
                                   : i;
                bits += (dcf77Bits[slot] < 0) ? '?' : (char)('0' + dcf77Bits[slot]);
            }

            String json = "{\"present\":true";
            json += ",\"state\":\"" + getDcf77Status() + "\"";
            json += ",\"edges\":" + String(dcf77Count != 0 ? "true" : "false");
            json += ",\"confirmed\":" + String(dcf77Confirmed ? "true" : "false");
            json += ",\"bitIndex\":" + String(dcf77BitIndex);
            json += ",\"bits\":\"" + bits + "\"";
            json += ",\"synced\":" + String(dcf77Synced ? "true" : "false"); // Diagnose: ist die Position im Telegramm bekannt? (siehe dcf77Synced in globals.h)
                                                                             // diagnostic: is the position within the telegram known? (see dcf77Synced in globals.h)
            json += ",\"markerPos\":" + String(dcf77MarkerPos);   // Rasterposition der erkannten Minutenmarke, -1 = noch nicht gefunden
                                                                  // grid position of the detected minute marker, -1 = not found yet
            json += ",\"phase\":" + String(dcf77Phase);
            json += ",\"pulsesSeen\":" + String(dcf77PulsesSeen);
            json += ",\"pulsesMissed\":" + String(dcf77PulsesMissed);
            json += ",\"phaseBreaks\":" + String(dcf77PhaseBreaks);

            // Die letzten Impulse als Rohwerte (Dauer/Abstand in ms),
            // aeltester zuerst - zeigt ohne Oszilloskop, ob der Empfaenger
            // ueberhaupt ein brauchbares Signal liefert (erwartet: ~100/200ms, Abstaende nahe einem Vielfachen von 1000ms).

            // The most recent pulses as raw values (width/distance in ms),
            // oldest first - shows without an oscilloscope whether the
            // receiver delivers a usable signal (expected: ~100/200ms, distances close to a multiple of 1000ms).

            json += ",\"pulses\":[";
            for (uint8_t i = 0; i < dcf77DiagCount; i++) {
                uint8_t idx = (uint8_t)((dcf77DiagIdx + DCF77_DIAG_SLOTS - dcf77DiagCount + i) % DCF77_DIAG_SLOTS);
                if (i > 0) json += ",";
                json += "[" + String(dcf77DiagWidth[idx]) + "," + String(dcf77DiagGap[idx]) + "]";
            }
            json += "]";
            json += ",\"edgeDropped\":" + String(dcf77EdgeDropped); // Diagnose: Flanken, die wegen vollem Ringpuffer verworfen wurden (siehe isr() in time_sync.h)
                                                                     // diagnostic: edges dropped due to a full ring buffer (see isr() in time_sync.h)
            json += ",\"decoded\":{";
            if (dcf77LastDecoded.decodedAtMillis == 0) {
                json += "\"hasData\":false";
            } else {
                unsigned long ageMs = millis() - dcf77LastDecoded.decodedAtMillis; // Ueberlauf nach ~49 Tagen absichtlich nicht behandelt, da hier nur zur Anzeige verwendet und nach spaetestens 60s durch ein neues Telegramm ersetzt
                                                                                    // overflow after ~49 days deliberately not handled, since this is only used for display here and gets replaced by a new telegram after 60s at the latest
                json += "\"hasData\":true";
                json += ",\"valid\":" + String(dcf77LastDecoded.valid ? "true" : "false");
                json += ",\"minute\":" + String(dcf77LastDecoded.minute);
                json += ",\"hour\":" + String(dcf77LastDecoded.hour);
                json += ",\"day\":" + String(dcf77LastDecoded.day);
                json += ",\"month\":" + String(dcf77LastDecoded.month);
                json += ",\"year\":" + String(dcf77LastDecoded.year);
                json += ",\"weekday\":" + String(dcf77LastDecoded.weekday);
                json += ",\"dst\":" + String(dcf77LastDecoded.dst ? "true" : "false");
                json += ",\"callBit\":" + String(dcf77LastDecoded.callBit ? "true" : "false");
                json += ",\"parityMin\":" + String(dcf77LastDecoded.parityMinOk ? "true" : "false");
                json += ",\"parityHour\":" + String(dcf77LastDecoded.parityHourOk ? "true" : "false");
                json += ",\"parityDate\":" + String(dcf77LastDecoded.parityDateOk ? "true" : "false");
                json += ",\"repaired\":" + String(dcf77LastDecoded.repairedBits); // aus der Paritaet rekonstruierte Bits (siehe decodeDcf77Telegram())
                                                                                    // bits reconstructed from parity (see decodeDcf77Telegram())
                json += ",\"ageSeconds\":" + String(ageMs / 1000);
            }
            json += "}}";
            webserver.send(200, "application/json", json);
            });

        // Liefert den Inhalt der AKTUELL aktiven Logdatei als Klartext - fuer
        // das Auto-Refresh-Polling im Log-Tab. Loest die Dateinummer bei
        // JEDEM Aufruf frisch auf, damit nach einer Rotation (>10 KB) die neue Datei geliefert wird.

        // Returns the content of the CURRENTLY active log file as plain
        // text - for the auto-refresh polling in the Log tab. Resolves the
        // file number freshly on EVERY call, so after a rotation (>10 KB) the new file is served.

        webserver.on("/api/currentLog", HTTP_GET, []() {

            // Nur aus einem privaten Netz erlaubt (siehe isPrivateNetworkIp()
            // oben) - Logeintraege koennen IP-Adressen, SSIDs u.ae. enthalten.

            // Only allowed from a private network (see isPrivateNetworkIp()
            // above) - log entries can contain IP addresses, SSIDs, etc.

            if (!isPrivateNetworkIp(webserver.client().remoteIP())) {
                webserver.send(200, "text/plain; charset=utf-8", translate("This action is only available when accessing the clock from a private network") + ".");
                return;
            }

            webserver.sendHeader("Cache-Control", "no-store");
            if (!loggingEnabled) {
                webserver.sendHeader("X-Log-File", "-");
                webserver.send(200, "text/plain; charset=utf-8", translate("Logging is disabled."));
                return;
            }
            String logFileName = getCurrentLogFileName();

            // Optionaler "file"-Parameter fuers Dropdown (siehe panel-log) -
            // nur ein Name aus dem bekannten log_1..9.log-Namensraum wird
            // akzeptiert, alles andere faellt auf die aktive Datei zurueck.

            // Optional "file" parameter for the dropdown (see panel-log) -
            // only a name from the known log_1..9.log namespace is
            // accepted, anything else falls back to the active file.

            if (webserver.hasArg("file")) {
                String requested = webserver.arg("file");
                for (int i = 1; i <= 9; i++) {
                    if (requested == ("/log_" + String(i) + ".log")) { logFileName = requested; break; }
                }
            }

            // X-Log-File: Custom-Header, damit JS #logFileName aktualisieren
            // kann, auch wenn sich die Datei durch Rotation geaendert hat.

            // X-Log-File: custom header so JS can update #logFileName, even
            // if the file changed due to rotation.

            webserver.sendHeader("X-Log-File", logFileName);
            if (!LittleFS.exists(logFileName)) {
                webserver.send(200, "text/plain; charset=utf-8", translate("No log entries yet."));
                return;
            }
            File logFile = LittleFS.open(logFileName, "r");
            if (!logFile) {
                webserver.send(200, "text/plain; charset=utf-8", translate("Log file could not be opened."));
                return;
            }
            webserver.streamFile(logFile, "text/plain; charset=utf-8");
            logFile.close();
            });

        // Liefert alle existierenden Logdateien (log_1.log..log_9.log) als
        // JSON-Array, absteigend sortiert - die aktive (hoechste Nummer)
        // steht so immer zuerst und wird im Dropdown vorausgewaehlt.

        // Returns all existing log files (log_1.log..log_9.log) as a JSON
        // array, sorted descending - the active one (highest number)
        // always comes first and is preselected in the dropdown.

        webserver.on("/api/logFileList", HTTP_GET, []() {

            // Nur aus einem privaten Netz erlaubt - siehe Begruendung bei
            // /api/currentLog weiter oben. Leeres Array statt Fehlerseite,
            // damit das Dropdown im Log-Tab einfach leer bleibt.

            // Only allowed from a private network - see the reasoning at
            // /api/currentLog further above. Empty array instead of an
            // error page, so the dropdown in the Log tab simply stays empty.

            if (!isPrivateNetworkIp(webserver.client().remoteIP())) {
                webserver.send(200, "application/json", "[]");
                return;
            }

            webserver.sendHeader("Cache-Control", "no-store");
            String json = "[";
            bool first = true;
            for (int i = 9; i >= 1; i--) {
                String name = "/log_" + String(i) + ".log";
                if (LittleFS.exists(name)) {
                    if (!first) json += ",";
                    json += "\"" + name + "\"";
                    first = false;
                }
            }
            json += "]";
            webserver.send(200, "application/json", json);
            });

        // Zeigt die Live-Zeiger-Uhr als eigene Seite. Server-seitig wird bei
        // previewSize (Basisgroesse) gerendert; ein Schieberegler skaliert die
        // fertige Uhr per CSS transform:scale() clientseitig weiter hoch/runter.

        // Shows the live hand clock as its own page. Server-side rendering
        // happens at previewSize (base size); a slider then scales the
        // finished clock further up/down client-side via CSS transform:scale().

        webserver.on("/preview", HTTP_GET, []() {
            webserver.setContentLength(CONTENT_LENGTH_UNKNOWN);
            webserver.send(200, "text/html", "");

            String chunk = beginPage();
            chunk.reserve(2048);
            chunk += "<h2>" + translate("Preview") + "</h2>";

            // Zuletzt per Schieberegler gewaehlte Groesse laden (persistiert
            // ueber /api/setPreviewSize), statt bei jedem Laden auf
            // PREVIEW_SIZE_DEFAULT zu springen. constrain() faengt einen ungueltigen gespeicherten Wert ab.

            // Load the size last chosen via the slider (persisted via
            // /api/setPreviewSize), instead of jumping back to
            // PREVIEW_SIZE_DEFAULT on every load. constrain() catches an invalid stored value.

            int previewSize = preferences.getInt(PK_PREVIEW_SIZE, PREVIEW_SIZE_DEFAULT);
            previewSize = constrain(previewSize, PREVIEW_SIZE_MIN, PREVIEW_SIZE_MAX);

            String previewSig = currentPreviewSignature();

            // Zeiger als eigene Bilder ueber /previewhand (zeilenweise gestreamt) statt eingebettet - drei
            // eingebettete PNGs brauchten zusammen mehr Speicher, als ohne PSRAM frei ist. Breite wie auf dem Display.

            // Hands as separate images via /previewhand (streamed row by row) instead of embedded - three embedded
            // PNGs together needed more memory than is free without PSRAM. Width as on the display.

            auto previewHandWidth = [](int w) { return (w <= 0 || w > CLOCK_WIDTH) ? HAND_WIDTH : w; };
            int hourW = previewHandWidth(hourHandWidth);
            int minuteW = previewHandWidth(minuteHandWidth);
            int secondW = previewHandWidth(secondHandWidth);
            String cacheBuster = String(millis());

            uint8_t hubR = ((hubColor >> 11) & 0x1F) * 255 / 31;
            uint8_t hubG = ((hubColor >> 5) & 0x3F) * 255 / 63;
            uint8_t hubB = (hubColor & 0x1F) * 255 / 31;
            char hubHex[8];
            snprintf(hubHex, sizeof(hubHex), "#%02x%02x%02x", hubR, hubG, hubB);

            bool showSecond = preferences.getBool(PK_SHOW_SECOND_HAND, true);
            bool stationModeActive = preferences.getBool(PK_STATION_MODE, true);

            // Default false wie ueberall sonst im Projekt - sonst zeigte die Anzeige nach einem Werksreset
            // einen anderen Zustand als tatsaechlich angewendet.

            // Default false, matching everywhere else in the project - otherwise this display would claim a
            // different state after a factory reset than actually applied.

            bool smoothMinuteActive = preferences.getBool(PK_SMOOTH_MINUTE, false);

            // Fallback bewusst stationModeActive statt eines festen Literals -
            // siehe Kommentar bei der smoothSecond-Ladezeile in uhr4.ino.

            // Fallback deliberately stationModeActive instead of a fixed
            // literal - see the comment at the smoothSecond load line in uhr4.ino.

            bool smoothSecondActive = getSmoothSecondPref(stationModeActive);

            float scaleFactor = (float)previewSize / CLOCK_WIDTH;

            // Drehpunkt wie setPivot() im Geraet (Breite/2, HAND_PIVOT_Y), jeweils
            // auf die Pixelmitte - ungerundet (CSS kann Nachkommastellen), Breite
            // pro Zeiger (Zifferblatt kann sie vorgeben).

            // Pivot like setPivot() on the device (width/2, HAND_PIVOT_Y), each at
            // the pixel centre - unrounded (CSS accepts decimals), width per hand
            // (the clock face can specify it).

            String scaledHandHeight = String(HAND_HEIGHT * scaleFactor, 2);
            String scaledPivotY = String((HAND_PIVOT_Y + 0.5f) * scaleFactor, 2);
            auto handImg = [&](const char* id, const char* part, int w) -> String {
                String scaledW = String(w * scaleFactor, 2);
                String scaledPivotX = String(((w / 2) + 0.5f) * scaleFactor, 2);
                return "<img id='" + String(id) + "' src='/previewhand?part=" + String(part) + "&w=" + String(w) + "&v=" + cacheBuster +
                       "' style='position:absolute;left:-" + scaledPivotX + "px;top:-" + scaledPivotY +
                       "px;width:" + scaledW + "px;height:" + scaledHandHeight +
                       "px;transform-origin:" + scaledPivotX + "px " + scaledPivotY + "px;'>";
            };

            // hubSize ist ein Radius, der CSS-Kreis braucht den Durchmesser - daher verdoppeln.
            // hubSize is a radius, the CSS circle needs the diameter - hence doubled.

            int scaledHubSize = (int)(hubSize * 2 * scaleFactor + 0.5);
            if (scaledHubSize < 4) scaledHubSize = 4;

            // Versteckt per Default - JS blendet ihn ein, sobald Rocrail-
            // Modellzeit aktiv ist. Gleicher Banner-Stil wie im Helligkeit-
            // Tab. max-width bewusst fest, damit der Text beim Skalieren der Uhr lesbar bleibt.

            // Hidden by default - JS reveals it once Rocrail model time is
            // active. Same banner style as in the Brightness tab. max-width
            // deliberately fixed, so the text stays readable while the clock is resized.

            chunk += "<div id='rocrailPreviewHint' class='msg warn' hidden style='margin:0 auto 10px;text-align:center;'></div>";

            // Groessenregler: skaliert die fertige Uhr client-seitig per CSS
            // transform:scale() - kein Server-Request, kein Neuaufbau der Zeigerbilder noetig.

            // Size slider: scales the finished clock client-side via CSS
            // transform:scale() - no server request, no need to rebuild the hand images.

            chunk += "<div style='display:flex;align-items:center;justify-content:center;gap:8px;margin-bottom:10px;flex-wrap:wrap;'>";
            chunk += "<label for='previewSizeSlider'>" + translate("Preview Size") + ":</label>";
            chunk += "<input type='range' id='previewSizeSlider' min='" + String(PREVIEW_SIZE_MIN) + "' max='" + String(PREVIEW_SIZE_MAX) + "' step='10' value='" + String(previewSize) + "' style='width:200px;'>";
            chunk += "<span id='previewSizeValue'>" + String(previewSize) + "</span>&nbsp;px";
            chunk += "</div>";

            // Streifen fuer Uhrzeit/Datum (ILI9341) ueber oder unter der Uhr, so wie die Uhr ihn zeichnet (/api/stripimg).
            // Der Browser skaliert ihn geglaettet - pixelweise wuerden die Ziffern ungleichmaessig.

            // Time/date strip (ILI9341) above or below the clock, exactly as the clock draws it (/api/stripimg). The
            // browser scales it smoothed - pixel by pixel the digits would become uneven.

            int stripPrevH = (TFT_HEIGHT > CLOCK_HEIGHT) ? previewSize * (TFT_HEIGHT - CLOCK_HEIGHT) / CLOCK_WIDTH : 0;
            String stripCanvas = "";
            if (stripPrevH > 0) {
                stripCanvas = "<canvas id='liveStrip' width='" + String(TFT_WIDTH) + "' height='" + String(TFT_HEIGHT - CLOCK_HEIGHT) +
                              "' style='display:block;width:" + String(previewSize) + "px;height:" + String(stripPrevH) +
                              "px;background:#000;'></canvas>";
            }

            // previewSizer traegt die tatsaechliche Boxgroesse, previewInner
            // bleibt fest bei previewSize und wird nur per transform:scale()
            // skaliert - die Zeiger-Pixel-Offsets muessen so nicht neu ermittelt werden.

            // previewSizer carries the actual box size, previewInner stays
            // fixed at previewSize and is only scaled via transform:scale() -
            // the hands' pixel offsets don't need to be recalculated this way.

            chunk += "<div id='previewSizer' style='width:" + String(previewSize) + "px;height:" + String(previewSize + stripPrevH) + "px;margin:20px auto;'>";

            // Mit Streifen ein gemeinsamer Rahmen ums ganze Display (outline, aendert die Groesse nicht) - Zifferblatt und
            // Streifen stossen nahtlos aneinander wie auf dem Display.

            // With a strip one shared frame around the whole display (outline, doesn't change the size) - clock face
            // and strip join seamlessly as on the display.

            chunk += "<div id='previewInner' style='width:" + String(previewSize) + "px;height:" + String(previewSize + stripPrevH) + "px;transform-origin:top left;" +
                     String(stripPrevH > 0 ? "outline:3px solid #333;" : "") + "'>";
            if (stripBefore) chunk += stripCanvas;
            chunk += "<div style='width:" + String(previewSize) + "px;height:" + String(previewSize) + "px;box-sizing:border-box;" + String(stripPrevH > 0 ? "" : "border:3px solid #333;") + String(displayGeom->round ? "border-radius:50%;" : "") + "background:#fff url(/currentfacebg) center/cover no-repeat;overflow:hidden;position:relative;'>";

            // Sekundenfeld: Sekundenzeiger an seinem Drehpunkt und unter den grossen Zeigern (wie renderClockFrame())
            // Seconds subdial: second hand at its pivot and below the large hands (as in renderClockFrame())

            ensureFaceSettings();
            bool subdial = showSecond && secPivotX >= 0;
            if (subdial) {
                chunk += "<div style='position:absolute;left:" + String((secPivotX + 0.5f) * scaleFactor, 2) + "px;top:" +
                         String((secPivotY + 0.5f) * scaleFactor, 2) + "px;width:0;height:0;'>" + handImg("liveSecondHandFull", "second", secondW) + "</div>";
            }
            chunk += "<div id='liveHandsPivotFull' style='position:absolute;left:50%;top:50%;width:0;height:0;'>";
            chunk += handImg("liveHourHandFull", "hour", hourW);
            chunk += handImg("liveMinuteHandFull", "minute", minuteW);
            if (showSecond && !subdial) {
                chunk += handImg("liveSecondHandFull", "second", secondW);
            }
            chunk += "<div id='liveHubFull' style='position:absolute;left:-" + String(scaledHubSize / 2) + "px;top:-" + String(scaledHubSize / 2) + "px;width:" + String(scaledHubSize) + "px;height:" + String(scaledHubSize) + "px;border-radius:50%;background:" + String(hubHex) + ";'></div>";
            chunk += "</div>"; // Ende Zeiger-Drehpunkt
                               // end hand pivot
            chunk += "</div>"; // Ende Zifferblatt-Kreis
                               // end clock-face circle
            if (!stripBefore) chunk += stripCanvas;
            chunk += "</div>"; // Ende previewInner
                               // end previewInner
            chunk += "</div>"; // Ende previewSizer
                               // end previewSizer

            chunk += "<script>";
            chunk += "(function() {";
            chunk += "  var sizer = document.getElementById('previewSizer');";
            chunk += "  var inner = document.getElementById('previewInner');";
            chunk += "  var sizeSlider = document.getElementById('previewSizeSlider');";
            chunk += "  var sizeValueEl = document.getElementById('previewSizeValue');";
            chunk += "  var stripH = " + String(stripPrevH) + ";"; // Hoehe des Streifens bei baseSize (0 ohne Streifen)
                                                                   // height of the strip at baseSize (0 without a strip)
            chunk += "  var baseSize = " + String(previewSize) + ";"; // Basisgroesse, bei der previewInner serverseitig gerendert wurde
                                                                       // base size previewInner was rendered at server-side

            // Skaliert previewInner per CSS transform statt die Zeigerbilder
            // neu zu positionieren - transform-origin:top left verankert die
            // Box, waehrend previewSizer im Seitenfluss mitwaechst/-schrumpft.

            // Scales previewInner via CSS transform instead of repositioning
            // the hand images - transform-origin:top left anchors the box,
            // while previewSizer grows/shrinks in the page flow.

            chunk += "  function applySize(px) {";
            chunk += "    var scale = px / baseSize;";
            chunk += "    sizer.style.width = px + 'px';";
            chunk += "    sizer.style.height = Math.round(px * (baseSize + stripH) / baseSize) + 'px';";
            chunk += "    inner.style.transform = 'scale(' + scale + ')';";
            chunk += "    if (sizeValueEl) sizeValueEl.textContent = px;";
            chunk += "  }";
            chunk += "  if (sizeSlider) sizeSlider.addEventListener('input', function() { applySize(parseInt(sizeSlider.value, 10)); });";

            // Nur beim Loslassen speichern (change), nicht bei jedem Zwischen-
            // wert waehrend des Ziehens (input) - siehe /api/setPreviewSize.

            // Save only on release (change), not on every intermediate value
            // while dragging (input) - see /api/setPreviewSize.

            chunk += "  if (sizeSlider) sizeSlider.addEventListener('change', function() { fetch('/api/setPreviewSize?size=' + sizeSlider.value, {cache:'no-store'}).catch(function() {}); });";
            chunk += "  var hourEl = document.getElementById('liveHourHandFull');";
            chunk += "  var minuteEl = document.getElementById('liveMinuteHandFull');";
            chunk += "  var secondEl = document.getElementById('liveSecondHandFull');";
            chunk += "  var hubEl = document.getElementById('liveHubFull');";

            // Streifenbild (RGB565 big-endian) neu holen, sobald sich die angezeigte Uhrzeit aendert - ohne Sekunden
            // einmal pro Minute. Jeder Abruf haelt die Uhr kurz an (ca. 130 ms), daher hoechstens einmal pro Sekunde.

            // Fetch the strip image (RGB565 big-endian) again as soon as the displayed time changes - without seconds
            // once a minute. Each fetch briefly stalls the clock (about 130 ms), hence at most once per second.

            chunk += "  var stripCv = document.getElementById('liveStrip');";
            chunk += "  var stripSec = " + String(stripShowsSeconds() ? "true" : "false") + ", stripKey = null, stripBusy = false, stripLastAt = 0;";
            chunk += "  function syncStrip(m, s) {";
            chunk += "    var key = stripSec ? m * 60 + s : m;";
            chunk += "    if (!stripCv || key === stripKey || stripBusy || performance.now() - stripLastAt < 900) return;";
            chunk += "    stripKey = key; stripBusy = true; stripLastAt = performance.now();";
            chunk += "    setTimeout(function() { loadStrip().then(function() { stripBusy = false; }); }, 200);";
            chunk += "  }";
            chunk += "  function loadStrip() {";
            chunk += "    return fetch('/api/stripimg', {cache:'no-store'}).then(function(r) { if (!r.ok) throw new Error(r.status); return r.arrayBuffer(); }).then(function(ab) {";
            chunk += "      var b = new Uint8Array(ab), w = stripCv.width, h = stripCv.height;";
            chunk += "      if (b.length < w * h * 2) return;";
            chunk += "      var ctx = stripCv.getContext('2d'), id = ctx.createImageData(w, h);";
            chunk += "      for (var i = 0; i < w * h; i++) {";
            chunk += "        var v = (b[2 * i] << 8) | b[2 * i + 1];";
            chunk += "        id.data[i * 4] = (v >> 11) * 255 / 31; id.data[i * 4 + 1] = ((v >> 5) & 63) * 255 / 63; id.data[i * 4 + 2] = (v & 31) * 255 / 31; id.data[i * 4 + 3] = 255;";
            chunk += "      }";
            chunk += "      ctx.putImageData(id, 0, 0);";
            chunk += "    }).catch(function() {});";
            chunk += "  }";
            chunk += "  var hintEl = document.getElementById('rocrailPreviewHint');";
            chunk += "  var stationMode = " + String(stationModeActive ? "true" : "false") + ";";
            chunk += "  var smoothMinute = " + String(smoothMinuteActive ? "true" : "false") + ";";
            chunk += "  var smoothSecond = " + String(smoothSecondActive ? "true" : "false") + ";";
            chunk += "  var fastSecondMs = " + String((int)FAST_SECOND) + ";";
            chunk += "  var rocrailHideDetailsDivider = " + String(ROCRAIL_HIDE_DETAILS_DIVIDER) + ";";
            chunk += "  var rocrailHintTpl = '" + translate("Showing Rocrail model time ({divider}&times; speed)") + "';";
            chunk += "  var baseH = 0, baseM = 0, baseS = 0, baseAt = 0, haveBase = false, pendingBase = null;";
            chunk += "  function setBase(b) { baseH = b.h; baseM = b.m; baseS = b.s; baseAt = b.at; haveBase = true; }";
            chunk += "  var rocrailDivider = 1, rocrailFrozen = false;";

            // Beim Laden eingebetteter Fingerabdruck (siehe
            // currentPreviewSignature()) - jeder 15s-Poll vergleicht und
            // laedt bei Abweichung die Seite neu (siehe applyCurrentTime() unten).

            // Fingerprint embedded on load (see currentPreviewSignature()) -
            // every 15s poll compares it and reloads the page on a
            // mismatch (see applyCurrentTime() below).

            chunk += "  var lastPreviewSig = '" + escapeForJsStringLiteral(previewSig, '\'') + "';";
            chunk += "  var pollTimer = null;";

            // Holt Basis-Uhrzeit + Rocrail-Status alle 15s neu, damit die
            // Vorschau live umschaltet, sobald sich Verbindung/Divider/
            // Frozen-Zustand auf dem Geraet aendern, ohne Neuladen der Seite.

            // Re-fetches the base time + Rocrail status every 15s, so the
            // preview switches live as soon as connection/divider/frozen
            // state change on the device, without reloading the page.

            chunk += "  function applyCurrentTime() {";
            chunk += "    var fetchStart = performance.now();";
            chunk += "    fetch('/api/currentTime', {cache:'no-store'}).then(function(r) { return r.json(); }).then(function(t) {";

            // Zifferblatt/Zeigersatz/Nabe/Sekundenzeiger haben sich seit dem
            // Laden geaendert (z.B. anderswo) - Seite neu laden, damit die
            // Vorschau wieder eine aktuelle Kopie des Displays zeigt.

            // Clock face/hand set/hub/second hand have changed since the
            // page loaded (e.g. elsewhere) - reload so the preview shows an
            // up to date copy of the display again.

            chunk += "      if (t.previewSig !== lastPreviewSig) { location.reload(); return; }";

            // baseAt auf die MITTE der Anfrage legen (wie bei NTP) statt auf das Antwortende - sonst hinkte
            // die Vorschau um Netzwerk- und Serverzeit hinter dem Display her.

            // Place baseAt at the MIDPOINT of the request (like NTP) instead of the end of the response -
            // otherwise the preview lagged behind the display by the network and server time.

            chunk += "      var roundTrip = performance.now() - fetchStart;";
            chunk += "      var nb = { h: t.hour, m: t.minute, s: t.second + (t.ms || 0) / 1000, at: fetchStart + roundTrip / 2 };";
            chunk += "      var div = t.rocrail ? t.divider : 1, frz = t.rocrail && t.frozen;";

            // Erste Zeit und Rocrail-Wechsel sofort, sonst erst in der naechsten Sekunde 59 (tick()) - im
            // Bahnhofsmodus wartet der Sekundenzeiger dann auf der 12, die Korrektur bleibt unsichtbar. Langsame
            // Antworten (Uhr gerade beschaeftigt) verwerfen: ihre Mitte trifft den Antwortzeitpunkt nicht.

            // First time and Rocrail changes right away, otherwise only in the next second 59 (tick()) - in station
            // mode the second hand then waits at 12, the correction stays invisible. Discard slow answers (clock
            // busy): their midpoint doesn't match the moment of the answer.

            chunk += "      if (!haveBase || div !== rocrailDivider || frz !== rocrailFrozen) { pendingBase = null; setBase(nb); }";
            chunk += "      else if (roundTrip < 150) pendingBase = nb;";

            // rocrailDivider/-Frozen: siehe /api/currentTime - dieselbe
            // rocrailTimeReady-Bedingung wie in renderClockFrame() (display.h).
            // Ohne aktive Rocrail-Zeit bleibt divider=1, frozen=false.

            // rocrailDivider/-Frozen: see /api/currentTime - the same
            // rocrailTimeReady condition as in renderClockFrame() (display.h).
            // Without an active Rocrail time, divider stays 1, frozen stays false.

            chunk += "      rocrailDivider = div;";
            chunk += "      rocrailFrozen = frz;";
            chunk += "      if (hintEl) { hintEl.hidden = !t.rocrail; if (t.rocrail) hintEl.innerHTML = rocrailHintTpl.replace('{divider}', rocrailDivider); }";
            chunk += "    }).catch(function() {});";
            chunk += "  }";

            // Pausiert das Pollen, waehrend der Tab nicht sichtbar ist -
            // gleiches Muster wie die DCF77-Live-Seite (siehe dort).

            // Pauses polling while the tab isn't visible - same pattern as
            // the DCF77 live page (see there).

            chunk += "  function startPoll() { if (pollTimer) return; applyCurrentTime(); pollTimer = setInterval(applyCurrentTime, 15000); }";
            chunk += "  function stopPoll() { if (!pollTimer) return; clearInterval(pollTimer); pollTimer = null; }";
            chunk += "  document.addEventListener('visibilitychange', function() { if (document.hidden) stopPoll(); else startPoll(); });";
            chunk += "  if (!document.hidden) startPoll();";
            chunk += "  function tick() {";
            chunk += "    var h, m, s, ms;";
            chunk += "    if (pendingBase && (baseS + (performance.now() - baseAt) / 1000 * rocrailDivider) % 60 >= 59) { setBase(pendingBase); pendingBase = null; }";
            chunk += "    if (haveBase) {";

            // Bei angehaltener Rocrail-Modellzeit (frozen) bleiben die Zeiger
            // auf dem zuletzt geholten Stand stehen, statt weiterzulaufen -
            // wie advanceRocrailTime() (rocrail_client.h) bei rocrailFrozen.

            // While the Rocrail model time is paused (frozen), the hands stay
            // at the last fetched reading instead of advancing - like
            // advanceRocrailTime() (rocrail_client.h) does with rocrailFrozen.

            chunk += "      var elapsed = rocrailFrozen ? 0 : ((performance.now() - baseAt) / 1000) * rocrailDivider;";
            chunk += "      var totalSec = baseH * 3600 + baseM * 60 + baseS + elapsed;";
            chunk += "      h = Math.floor(totalSec / 3600) % 12;";
            chunk += "      m = Math.floor(totalSec / 60) % 60;";
            chunk += "      s = Math.floor(totalSec) % 60;";
            chunk += "      ms = (totalSec - Math.floor(totalSec)) * 1000;";
            chunk += "    } else {";
            chunk += "      var now = new Date();";
            chunk += "      h = now.getHours() % 12; m = now.getMinutes(); s = now.getSeconds(); ms = now.getMilliseconds();";
            chunk += "    }";

            // Wie renderClockFrame() (display.h): sanfter Minutenzeiger gilt
            // nur ausserhalb des Bahnhofsuhr-Modus - dort springt die Minute
            // immer beim Wechsel, unabhaengig von smoothMinute.

            // As in renderClockFrame() (display.h): smooth minute only
            // applies outside station-clock mode - there the minute always
            // jumps on change, regardless of smoothMinute.

            chunk += "    var minuteDeg = (smoothMinute && !stationMode) ? (m + s / 60) * 6 : m * 6;";
            chunk += "    var hourDeg = (h + m / 60 + s / 3600) * 30;"; // wie im Geraet: Stunde + Minute + Sekunde, unabhaengig vom Minutenstil
                                                                         // like on the device: hour + minute + second, independent of the minute style
            chunk += "    var secDeg;";

            // Wie renderClockFrame(): stationMode ("wartet auf 12") und smoothSecond (schwingend/tickend)
            // sind unabhaengig.

            // As in renderClockFrame(): stationMode ("waits at 12") and smoothSecond (smooth/ticking) are
            // independent.

            chunk += "    if (stationMode) {";
            chunk += "      var elapsedMs = (s + ms / 1000) * 1000;";

            // fastSecondMs NICHT zusaetzlich durch rocrailDivider teilen - elapsedMs ist ueber "elapsed" oben
            // schon mit demselben Divider beschleunigt (sonst doppelt so schnell).

            // Do NOT additionally divide fastSecondMs by rocrailDivider - elapsedMs is already accelerated by
            // that divider via "elapsed" above (otherwise twice as fast).

            chunk += "      if (rocrailDivider > 1) {";
            chunk += "        var smoothPos = elapsedMs / fastSecondMs;";
            chunk += "        if (smoothPos > 60) smoothPos = 60;";
            chunk += "        if (!smoothSecond) smoothPos = Math.floor(smoothPos);";
            chunk += "        secDeg = smoothPos * 6;";
            chunk += "      } else {";
            chunk += "        var tickIndex = Math.floor(elapsedMs / fastSecondMs);";
            chunk += "        var subTick = (elapsedMs % fastSecondMs) / fastSecondMs;";
            chunk += "        var smoothSec;";
            chunk += "        if (smoothSecond) {";
            chunk += "          var eased = -(Math.cos(Math.PI * Math.pow(subTick, 0.5)) - 1) / 2;";
            chunk += "          smoothSec = Math.min(tickIndex + eased, 60);";
            chunk += "        } else {";
            chunk += "          smoothSec = Math.min(tickIndex, 60);";
            chunk += "        }";
            chunk += "        secDeg = smoothSec * 6;";
            chunk += "      }";
            chunk += "    } else {";
            chunk += "      secDeg = smoothSecond ? (s + ms / 1000) * 6 : s * 6;";
            chunk += "    }";
            chunk += "    syncStrip(m, s);";
            chunk += "    hourEl.style.transform = 'rotate(' + hourDeg + 'deg)';";
            chunk += "    minuteEl.style.transform = 'rotate(' + minuteDeg + 'deg)';";

            // Wie renderClockFrame(): hideDetails (Divider ueber Schwelle) blendet Sekundenzeiger UND Nabe
            // aus, hideSecondTicking (Divider > 1 und tickend) nur den Sekundenzeiger.

            // As in renderClockFrame(): hideDetails (divider above threshold) hides the second hand AND the
            // hub, hideSecondTicking (divider > 1 and ticking) only the second hand.

            chunk += "    var hideDetails = rocrailDivider >= rocrailHideDetailsDivider;";
            chunk += "    var hideSecondTicking = rocrailDivider > 1 && !smoothSecond;";
            chunk += "    var hideSecond = hideDetails || hideSecondTicking;";
            chunk += "    if (secondEl) {";
            chunk += "      secondEl.style.display = hideSecond ? 'none' : '';";
            chunk += "      if (!hideSecond) secondEl.style.transform = 'rotate(' + secDeg + 'deg)';";
            chunk += "    }";
            chunk += "    if (hubEl) hubEl.style.display = hideDetails ? 'none' : '';";
            chunk += "    requestAnimationFrame(tick);";
            chunk += "  }";
            chunk += "  requestAnimationFrame(tick);";
            chunk += "})();";
            chunk += "</script>";

            chunk += "</body></html>";
            webserver.sendContent(chunk);
            webserver.sendContent("");
            });

        // Speichert die per Schieberegler gewaehlte Groesse in den
        // Preferences - nur beim "change"-Event (Loslassen), nicht bei
        // jedem "input" (Ziehen), sonst NVS-Schreibvorgang pro Bewegung.

        // Saves the size chosen via the slider to preferences - only on
        // the "change" event (release), not on every "input" (dragging),
        // otherwise an NVS write per movement.

        webserver.on("/api/setPreviewSize", HTTP_GET, []() {
            int size = argToIntClamped("size", PREVIEW_SIZE_DEFAULT, PREVIEW_SIZE_MIN, PREVIEW_SIZE_MAX);
            preferences.putInt(PK_PREVIEW_SIZE, size);
            webserver.send(200, "text/plain", "OK");
            });

        // DCF77-Live-Seite: Bit-Fortschritt der laufenden Minute plus letztes
        // dekodiertes Telegramm, sekuendlich gepollt (/api/dcf77status).
        // Bit-Tooltips/Legende bewusst Englisch (Diagnoseinhalt).

        // DCF77 live page: bit progress of the running minute plus the last
        // decoded telegram, polled every second (/api/dcf77status). Bit
        // tooltips/legend deliberately English (diagnostic content).

        webserver.on("/dcf77", HTTP_GET, []() {
            webserver.setContentLength(CONTENT_LENGTH_UNKNOWN);
            webserver.send(200, "text/html", "");

            String chunk = beginPage();
            chunk.reserve(6000);
            chunk += "<h2>DCF77</h2>";

            // Hinweis ohne erkannten Empfaenger: gar keine Pegelwechsel am Pin (nichts angeschlossen) oder
            // Wechsel ohne gleichmaessigen Sekundentakt (Stoerungen, schlechter Empfang). poll() schaltet um.

            // Note without a recognized receiver: no level changes on the pin at all (nothing connected) or
            // changes without a steady one-second rhythm (interference, poor reception). poll() toggles them.

            String dcfPin = String(DCF77_DATAPIN);
            String noEdges = translate("No level changes on GPIO {pin} yet - no DCF77 receiver detected.");
            String noSignal = translate("Level changes on GPIO {pin}, but no steady DCF77 signal yet - align the receiver and keep it away from interference.");
            noEdges.replace("{pin}", dcfPin);
            noSignal.replace("{pin}", dcfPin);
            chunk += "<div id='dcfNoEdges' class='msg warn'" + String(dcf77Count == 0 ? "" : " hidden") + ">" + noEdges + "</div>";
            chunk += "<div id='dcfNoSignal' class='msg warn'" + String(dcf77Count != 0 && !dcf77Confirmed ? "" : " hidden") + ">" + noSignal + "</div>";

            chunk += "<div class='card' style='max-width:900px;'>"; // 900px: breit genug fuer alle 59 Bit-Kaestchen in einer kompakten Rastergrid ohne unnoetige Zeilenumbrueche auf einem Desktop-Bildschirm
                                                                     // 900px: wide enough to fit all 59 bit boxes in a compact grid without unnecessary line wraps on a desktop screen
            chunk += "<h3>" + translate("Bit progress") + "</h3>";

            // Vor gefundener Minutenmarke zeigen die Kaestchen rohe Rasterpositionen,
            // nicht Bitnummern - dieser Hinweis wird vom Poll-Skript ein-/ausgeblendet.

            // Before the minute marker is found, boxes show raw grid positions,
            // not bit numbers - this note is shown/hidden by the poll script.

            chunk += "<div id='dcfRawNote' hidden style='margin-bottom:.6rem;padding:.4rem .6rem;border-radius:.3rem;"
                     "border:1px solid var(--panel-border);color:var(--muted);font-size:.75rem;'>"
                     + translate("Minute marker not identified yet - the boxes show raw grid positions, not telegram bit numbers.") + "</div>";
            chunk += "<div id='dcfBitGrid' style='display:flex;flex-wrap:wrap;gap:3px;justify-content:center;font-family:var(--mono);font-size:.7rem;'>";

            for (int i = 0; i < DCF77_TELEGRAM_BITS; i++) {
                String desc;
                if (i == 0) desc = translate("Start of minute (always 0)");
                else if (i >= 1 && i <= 14) desc = translate("Weather broadcast / special function (unused)");
                else if (i == 15) desc = translate("Call bit");
                else if (i == 16) desc = translate("DST change announcement");
                else if (i == 17) desc = translate("Summer time (CEST) in effect");
                else if (i == 18) desc = translate("Winter time (CET) in effect");
                else if (i == 19) desc = translate("Leap second announcement");
                else if (i == 20) desc = translate("Start of time (always 1)");
                else if (i >= 21 && i <= 27) desc = translate("Minute BCD bit") + " " + String(i - 20);
                else if (i == 28) desc = translate("Minute parity");
                else if (i >= 29 && i <= 34) desc = translate("Hour BCD bit") + " " + String(i - 28);
                else if (i == 35) desc = translate("Hour parity");
                else if (i >= 36 && i <= 41) desc = translate("Day of month BCD bit") + " " + String(i - 35);
                else if (i >= 42 && i <= 44) desc = translate("Day of week BCD bit") + " " + String(i - 41);
                else if (i >= 45 && i <= 49) desc = translate("Month BCD bit") + " " + String(i - 44);
                else if (i >= 50 && i <= 57) desc = translate("Year BCD bit") + " " + String(i - 49);
                else desc = translate("Date parity (day+weekday+month+year)");
                chunk += "<div id='bit-" + String(i) + "' title='" + translate("Bit") + " " + String(i) + ": " + desc +
                         "' style='width:1.7rem;height:1.7rem;line-height:1.7rem;display:inline-block;text-align:center;border-radius:.25rem;border:1px solid var(--panel-border);color:var(--muted);'>" +
                         String(i) + "</div>";
            }
            chunk += "</div>";
            chunk += "<p style='color:var(--muted);font-size:.75rem;'>"
                     "<span style='display:inline-block;width:.8rem;height:.8rem;background:var(--accent);border-radius:.2rem;vertical-align:middle;'></span> = 1 &nbsp; "
                     "<span style='display:inline-block;width:.8rem;height:.8rem;border:1px solid var(--panel-border);border-radius:.2rem;vertical-align:middle;'></span> = 0 &nbsp; "
                     "<i class='dot syncing' style='display:inline-block;position:static;'></i> = " + translate("next") + " &nbsp; "
                     "<span style='display:inline-block;width:.8rem;height:.8rem;border:1px dashed var(--muted);border-radius:.2rem;vertical-align:middle;'></span> = " + translate("lost") + "</p>";

            // dcf77EdgeDropped (globals.h): steigt bei vollem Ringpuffer, hilft
            // das von schwachem Empfang zu unterscheiden.

            // dcf77EdgeDropped (globals.h): rises on a full ring buffer, helps
            // tell that apart from weak reception.

            chunk += "<p style='color:var(--muted);font-size:.7rem;'>" + translate("Dropped edges (buffer overflow)") + ": <span id='dcfEdgeDropped'>-</span></p>";

            // dcf77Synced: "no" waehrend der Decoder auf die naechste Minutenmarke
            // wartet - das Raster bleibt dann bewusst leer, nichts ist defekt.

            // dcf77Synced: "no" while the decoder waits for the next minute
            // marker - the grid stays empty on purpose, nothing is broken.

            chunk += "<p style='color:var(--muted);font-size:.7rem;'>" + translate("Telegram sync") + ": <span id='dcfSynced'>-</span></p>";

            // Rohdaten des Empfaengers: beantwortet ohne Oszilloskop, ob das
            // Modul ein sauberes Signal liefert oder Impulse fehlen.

            // Receiver raw data: answers without an oscilloscope whether the
            // module delivers a clean signal or pulses are missing.

            chunk += "<p style='color:var(--muted);font-size:.7rem;'>" + translate("Pulses seen / missed / grid losses") + ": "
                     "<span id='dcfSeen'>-</span> / <span id='dcfMissed'>-</span> / <span id='dcfBreaks'>-</span></p>";
            chunk += "<p style='color:var(--muted);font-size:.7rem;'>" + translate("Last pulses (width / gap in ms, expected ~100 or ~200 / ~n&times;1000)") + ":<br>"
                     "<span id='dcfPulses' style='font-family:var(--mono);'>-</span></p>";
            chunk += "</div>";

            // JS-Vorlagen fuer den Sync-Status (per .textContent gesetzt, siehe
            // poll() unten) - ASCII-only Uebersetzungen ohne HTML-Entities,
            // {pos} wird per JS ersetzt (siehe Hinweis in translation.h).

            // JS templates for the sync status (set via .textContent, see
            // poll() below) - ASCII-only translations without HTML entities,
            // {pos} is substituted in JS (see note in translation.h).

            String dcfSyncYesTpl = translate("yes (marker at grid position {pos})");
            String dcfSyncNoTpl = translate("no (collecting - the minute marker needs a few minutes)");

            // Server rendert uebersetzte Labels + leere Platzhalter-<span>s;
            // das Poll-Skript befuellt nur Rohdaten (Ausnahme: SYNC_YES/NO
            // oben, als ASCII-Vorlage aus translate() ins JS gereicht).

            // Server renders translated labels + empty placeholder <span>s;
            // the poll script only fills raw data (exception: SYNC_YES/NO
            // above, passed into JS as an ASCII template from translate()).

            chunk += "<div class='card' id='dcfDecodedCard'>";
            chunk += "<h3>" + translate("Decoded telegram") + "</h3>";
            chunk += "<div id='dcfWaitingMsg'>" + translate("Waiting for first complete telegram") + "</div>";
            chunk += "<table id='dcfDecodedTable' hidden>";
            chunk += "<tr><td>" + translate("Day") + "/" + translate("Month") + "/" + translate("Year") + "</td><td><span id='dcfDate'>-</span></td></tr>";
            chunk += "<tr><td>" + translate("Hour") + "/" + translate("Minute") + "</td><td><span id='dcfTime'>-</span></td></tr>";
            chunk += "<tr><td>" + translate("Weekday") + "</td><td><span id='dcfWeekday'>-</span> <small>" + translate("(DCF77: 1=Mon..7=Sun)") + "</small></td></tr>";
            chunk += "<tr><td>" + translate("Summer time") + " / " + translate("Winter time") + "</td><td><span id='dcfDstOn' hidden>" + translate("Summer time") + "</span><span id='dcfDstOff' hidden>" + translate("Winter time") + "</span></td></tr>";
            chunk += "<tr><td>" + translate("Call bit") + "</td><td><span id='dcfCall'>-</span></td></tr>";
            chunk += "<tr><td>" + translate("Parity") + " (" + translate("Minute") + "/" + translate("Hour") + "/" + translate("Day") + ")</td><td><span id='dcfParityMin'>-</span> / <span id='dcfParityHour'>-</span> / <span id='dcfParityDate'>-</span></td></tr>";
            chunk += "<tr><td>" + translate("Last decoded") + "</td><td><span id='dcfAge'>-</span> s</td></tr>";

            // Bits aus Paritaet/Festwerten rekonstruiert statt empfangen; 0 = vollstaendig empfangen.
            // Bits reconstructed from parity/fixed values instead of received; 0 = fully received.

            chunk += "<tr><td>" + translate("Reconstructed bits") + "</td><td><span id='dcfRepaired'>-</span></td></tr>";
            chunk += "</table>";
            chunk += "</div>";

            chunk += "<script>";
            chunk += "(function(){";
            chunk += "var TOTAL=" + String(DCF77_TELEGRAM_BITS) + ";";
            chunk += "var SYNC_YES='" + dcfSyncYesTpl + "';var SYNC_NO='" + dcfSyncNoTpl + "';";
            chunk += "function pad(n){return (n<10?'0':'')+n;}";
            chunk += "function paintBits(bitIndex,bits){";
            chunk += "for(var i=0;i<TOTAL;i++){";
            chunk += "var el=document.getElementById('bit-'+i); if(!el) continue;";
            chunk += "el.classList.remove('syncing');";
            chunk += "el.style.borderStyle='solid';";
            chunk += "if(i<bitIndex){";
            chunk += "if(bits.charAt(i)==='1'){el.style.background='var(--accent)';el.style.borderColor='var(--accent)';el.style.color='#1a1200';}";

            // '?' = uebersprungene Sekunde (Luecke), gestrichelter Rahmen zur
            // Unterscheidung von einer echten 0.

            // '?' = skipped second (a hole), dashed border to distinguish
            // it from an actually received 0.

            chunk += "else if(bits.charAt(i)==='?'){el.style.background='';el.style.borderColor='var(--muted)';el.style.borderStyle='dashed';el.style.color='var(--muted)';}";
            chunk += "else{el.style.background='';el.style.borderColor='var(--panel-border)';el.style.color='var(--muted)';}";
            chunk += "}else if(i===bitIndex){";
            chunk += "el.style.background='';el.style.borderColor='var(--accent)';el.style.color='var(--accent)';el.classList.add('syncing');";
            chunk += "}else{";
            chunk += "el.style.background='';el.style.borderColor='var(--panel-border)';el.style.color='var(--muted)';";
            chunk += "}}}";
            chunk += "function renderDecoded(d){";
            chunk += "var waiting=document.getElementById('dcfWaitingMsg'), table=document.getElementById('dcfDecodedTable');";
            chunk += "if(!d||!d.hasData){waiting.hidden=false;table.hidden=true;return;}";
            chunk += "waiting.hidden=true;table.hidden=false;";
            chunk += "document.getElementById('dcfDate').textContent=pad(d.day)+'.'+pad(d.month)+'.'+d.year;";
            chunk += "document.getElementById('dcfTime').textContent=pad(d.hour)+':'+pad(d.minute);";
            chunk += "document.getElementById('dcfWeekday').textContent=d.weekday;";
            chunk += "document.getElementById('dcfDstOn').hidden=!d.dst;";
            chunk += "document.getElementById('dcfDstOff').hidden=d.dst;";
            chunk += "document.getElementById('dcfCall').textContent=d.callBit?'1':'0';";
            chunk += "document.getElementById('dcfParityMin').textContent=d.parityMin?'\\u2714':'\\u2716';";
            chunk += "document.getElementById('dcfParityHour').textContent=d.parityHour?'\\u2714':'\\u2716';";
            chunk += "document.getElementById('dcfParityDate').textContent=d.parityDate?'\\u2714':'\\u2716';";
            chunk += "document.getElementById('dcfAge').textContent=d.ageSeconds;";
            chunk += "var rp=document.getElementById('dcfRepaired');if(rp)rp.textContent=(d.repaired?d.repaired:0);";
            chunk += "}";
            chunk += "function showHw(s){var ne=document.getElementById('dcfNoEdges'),ns=document.getElementById('dcfNoSignal');"
                     "if(ne)ne.hidden=s.edges;if(ns)ns.hidden=!s.edges||s.confirmed;}";
            chunk += "var timer=null;";
            chunk += "function poll(){fetch('/api/dcf77status',{cache:'no-store'}).then(function(r){return r.json();}).then(function(s){showHw(s);paintBits(s.bitIndex,s.bits);renderDecoded(s.decoded);var ed=document.getElementById('dcfEdgeDropped');if(ed)ed.textContent=s.edgeDropped;var rn=document.getElementById('dcfRawNote');if(rn)rn.hidden=!!s.synced;var sy=document.getElementById('dcfSynced');if(sy)sy.textContent=s.synced?SYNC_YES.replace('{pos}',s.markerPos):SYNC_NO;var se=document.getElementById('dcfSeen');if(se)se.textContent=s.pulsesSeen;var mi=document.getElementById('dcfMissed');if(mi)mi.textContent=s.pulsesMissed;var br=document.getElementById('dcfBreaks');if(br)br.textContent=s.phaseBreaks;var pu=document.getElementById('dcfPulses');if(pu){if(!s.pulses||!s.pulses.length){pu.textContent='-';}else{pu.textContent=s.pulses.map(function(p){return p[0]+'/'+p[1];}).join('  ');}}}).catch(function(){});}";
            chunk += "function start(){if(timer)return;poll();timer=setInterval(poll,1000);}";
            chunk += "function stop(){if(!timer)return;clearInterval(timer);timer=null;}";
            chunk += "document.addEventListener('visibilitychange',function(){if(document.hidden)stop();else start();});";
            chunk += "if(!document.hidden)start();";
            chunk += "})();";
            chunk += "</script>";

            chunk += "</body></html>";
            webserver.sendContent(chunk);
            webserver.sendContent("");
            });

        // Ein Zeiger des aktiven Satzes fuer die Vorschau-Seite als PNG, w = Breite wie auf dem Display
        // One hand of the active set for the preview page as PNG, w = width as on the display

        webserver.on("/previewhand", HTTP_GET, []() {
            String part = webserver.arg("part");
            if (part != "hour" && part != "minute" && part != "second") {
                webserver.send(400, "text/plain", "part must be hour, minute or second");
                return;
            }
            int w = webserver.arg("w").toInt();
            if (w <= 0 || w > CLOCK_WIDTH) w = HAND_WIDTH;
            uint16_t* pix = (uint16_t*)malloc((size_t)HAND_WIDTH * HAND_HEIGHT * sizeof(uint16_t));
            if (!pix) {
                webserver.send(500, "text/plain", "out of memory");
                return;
            }
            loadHandOrDefault(preferences.getString(PK_HANDSET, ""), part.c_str(), pix);
            streamHandPng(pix, HAND_WIDTH, w, HAND_HEIGHT);
            free(pix);
            });

        webserver.on("/currentfacebg", HTTP_GET, []() {

            // Ohne PSRAM gibt es kein Rohbild - dann die Zifferblatt-Datei selbst als BMP senden
            // Without PSRAM there is no raw image - then send the clock face file itself as BMP

            if (!clockFaceBuffer) {
                String path = clockFaceRlePath.length() ? clockFaceRlePath : selectedBackground;
                webserver.sendHeader("Cache-Control", "no-store");
                if (!streamRleFaceAsStandardBmp(path, "image/bmp")) webserver.send(500, "text/plain", "Face not loaded");
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

            webserver.sendHeader("Cache-Control", "no-store");
            if (!sendPresetPreviewBmp(face, handSet, hubColorRgb565, curHubSize, curShowSecond)) {
                webserver.send(500, "text/plain", "Failed to generate preview (out of memory?)");
            }
            });

        // Vorschaubild fuer die Preset-Verwaltung: Zifferblatt + Zeiger (feste Demo-
        // Zeit) + Mittelpunkt-Farbe/-Groesse, komponiert aus den im Preset gespeicherten
        // Einstellungen (siehe parsePresetForPreview() und sendPresetPreviewBmp()).

        // Preview image for preset management: clock face + hands (fixed demo
        // time) + hub color/size, composed from the settings stored in the
        // preset (see parsePresetForPreview() and sendPresetPreviewBmp()).

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

            if (!sendPresetPreviewBmp(face, handSet, hubColorRgb565, hubSize, showSecond, presets[index].url)) {
                webserver.send(500, "text/plain", "Failed to generate preview (out of memory?)");
            }
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
            chunk += selectWithoutReloadScript(translate("Clock face selected"), translate("active"));
            chunk += "<h2>" + translate("Manage Clock Face Files") + " " + String(CLOCK_WIDTH) + " x " + String(CLOCK_HEIGHT) + "</h2>";
            chunk += "<p>" + generateStorageInfo(used, total) + "</p>";
            chunk += "<div style='display:flex;flex-wrap:wrap;gap:24px 18px;justify-content:center;align-items:flex-start;'>";

            String activeBackground = preferences.getString(PK_BACKGROUND, "/face_default.bmp");

            // Vorschaubild eines Zifferblatts; hat es eine Streifen-Grafik (ILI9341), haengt der Streifen nahtlos
            // darunter bzw. darueber, gemeinsamer Rahmen wie ein Display.

            // Preview image of a clock face; if it has a strip graphic (ILI9341), the strip hangs seamlessly below or
            // above it, one shared frame like a display.

            auto faceThumb = [](const String& previewUrl, const String& faceParam) -> String {
                String stripPath = stripPathForFace(faceParam);
                if (TFT_HEIGHT <= CLOCK_HEIGHT || !stripPath.length() || !LittleFS.exists(stripPath)) return "<img src='" + previewUrl + "' style='width:80px;height:80px;border:1px solid #ccc'>";
                String strip = "<img src='/strippreview?file=" + faceParam + "' style='display:block;width:80px;height:" +
                               String(80 * (TFT_HEIGHT - CLOCK_HEIGHT) / CLOCK_WIDTH) + "px'>";
                String face = "<img src='" + previewUrl + "' style='display:block;width:80px;height:80px'>";
                return "<span style='display:inline-block;border:1px solid #ccc;line-height:0;'>" + (stripBefore ? strip + face : face + strip) + "</span>";
            };

            webserver.sendContent(chunk);
            chunk = "";

            // Das Standard-Zifferblatt fehlt nie in der Liste (z.B. nach dem Loeschen neu erzeugt)
            // The default clock face is never missing from the list (e.g. recreated after deletion)

            ensureDefaultFace();
            File root = LittleFS.open("/");
            File file = root.openNextFile();

            // Erst alle passenden Zifferblatt-Dateinamen sammeln und sortieren
            // First collect and sort all matching clock-face filenames

            std::vector<String> faceNames;
            while (file) {
                String name = file.name();
                if (!file.isDirectory() && name.startsWith("face_") && name.endsWith(".bmp")) {
                    faceNames.push_back(name);
                }
                file = root.openNextFile();
            }
            naturalSortNames(faceNames);

            // Zifferblaetter des Startpakets zuerst, in fester Reihenfolge
            // Starter set clock faces first, in a fixed order

            {
                std::vector<String> ordered;
                for (const auto& f : STARTER_FACES) {
                    for (const String& n : faceNames) if (n == String(f.path + 1)) ordered.push_back(n);
                }
                for (const String& n : faceNames) if (!isProtectedFile(n)) ordered.push_back(n);
                faceNames = ordered;
            }

            bool anyFile = !faceNames.empty();
            int rowCount = 0;
            for (const String& name : faceNames) {
                String shortName = name;
                String displayName = shortName;
                if (displayName.startsWith("face_")) displayName = displayName.substring(5);
                if (displayName.endsWith(".bmp")) displayName = displayName.substring(0, displayName.length() - 4);
                String normalizedName = name.startsWith("/") ? name : "/" + name;
                bool isActive = (normalizedName == activeBackground);

                // escapeHtmlText(): Dateiname ist frei waehlbar (handleFileUpload()
                // prueft nur Praefix/Suffix), daher unbedingt escapen.

                // escapeHtmlText(): the filename is freely chosen (handleFileUpload()
                // only checks prefix/suffix), so it must be escaped.

                String safeName = escapeHtmlText(name);
                String safeShortName = escapeHtmlText(shortName);
                chunk += "<div style='text-align:center;width:100px;'>";
                chunk += "<a href='/setbackground?file=" + safeShortName + "' onclick='return pickItem(this,\"fa\")'>";
                chunk += faceThumb("/facepreview?file=" + safeName + "&v=" + fileVersion("/" + name), safeName);
                chunk += "</a><br>" + escapeHtmlText(displayName) + "<span class='fa'>" + String(isActive ? " (" + translate("active") + ")" : "") + "</span>";
                chunk += "<br><a href='/setbackground?file=" + safeShortName + "&designer=1'>" + translate("Designer") + "</a>";
                if (isProtectedFile(name)) {
                    chunk += "<br><small>" + translate("built-in") + "</small>";
                }
                else {
                    chunk += "<br><a href='/rename_form?file=" + safeName + "&from=listfilesFaces'>" + translate("Rename") + "</a> ";
                    chunk += "<a href='/delete?file=" + safeName + "&from=listfilesFaces' onclick='return confirm(\"" + translate("Delete") + " " + escapeHtmlText(displayName) + "?\")'>" + translate("Delete") + "</a>";
                }
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

            webserver.sendContent(chunk);
            chunk = "";

            if (used + (CLOCK_WIDTH * CLOCK_HEIGHT * 2) + 54 > total) {
                chunk += "<div style='color:red;font-weight:bold;'>" + translate("Warning: Not enough free space to upload new clock faces! Free up some space first") + ".</div><br><br>";
            }
            else {

                chunk += "<h3>" + translate("Upload New Clock Face") + "</h3>";
                // Die Datei liegt beim Hochladen erst ganz im Dateisystem, skaliert wird danach - Grenze ist der freie Speicher
                // The file is first stored completely in the file system, scaling happens afterwards - the limit is the free space

                chunk += "<small>" + translate("BMP file (16, 24 or 32 bit), at most about") + " " + String((total > used ? total - used : 0) / 1024) + " KB " + translate("(free space of the clock)") + ". " + translate("The clock scales it to") + " " + String(CLOCK_WIDTH) + " x " + String(CLOCK_HEIGHT) + " " + translate("pixels") + " " + translate("and converts it to RGB565") + ", " + translate("ideally square") + ".</small><br><br>";

                chunk += "<form method = 'POST' action = '/upload' enctype = 'multipart/form-data' onsubmit = 'showProgress()'>";
                chunk += "<input type='file' name='upload' accept='.bmp' multiple required><br>";

                chunk += "<button type='submit'>" + translate("Upload") + " BMP</button>";
                chunk += "<div id='progress' style='display:none;'>" + translate("Uploading... please wait") + "</div>";
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

            // Status-LED kurz aufblitzen lassen (setLedOff() am Ende dieses
            // Handlers) - Lebenszeichen wie beim Upload-Pattern, kein
            // zusaetzliches delay() noetig, der Seitenaufbau dauert selbst lang genug.

            // Briefly flash the status LED (setLedOff() at the end of this
            // handler) - a sign of life like the upload pattern, no extra
            // delay() needed since building the page itself takes long enough.

            setLedOn();

            // Einmal pro Aufruf ermitteln, ob der Status-Tab weiter unten
            // angezeigt werden darf (siehe isPrivateNetworkIp() oben).

            // Determined once per call whether the Status tab further below
            // may be shown (see isPrivateNetworkIp() above).

            bool statusAccessAllowed = isPrivateNetworkIp(webserver.client().remoteIP());

            webserver.setContentLength(CONTENT_LENGTH_UNKNOWN);
            webserver.send(200, "text/html", "");

            String chunk = beginPage();
            chunk.reserve(2048);
            chunk += generateFlashMessage(); // Erfolgsmeldung, falls vorhanden
                                             // success message, if present

            chunk += generateLanguageSelector();

            bool apMode = (WiFi.getMode() != WIFI_STA);

            // CSS-Tabs: die Radio-Inputs muessen direkte Geschwister von .tabnav/.panel-* sein. Der erste Tab
            // (WLAN, Reihenfolge aus SETTINGS_TAB_KEYS) ist vorausgewaehlt - deckt auch das
            // Captive-Portal-Popup ab.

            // CSS tabs: the radio inputs must be direct siblings of .tabnav/.panel-*. The first tab (WiFi,
            // order from SETTINGS_TAB_KEYS) is preselected - also covers the captive portal popup.

            for (size_t i = 0; i < SETTINGS_TAB_COUNT; i++) {

                // Rocrail-Radio nur rendern, wenn der Tab auch sichtbar ist
                // (siehe generateSettingsTabNav()) - ein verwaistes, nie
                // anklickbares Radio waere sonst nutzlos.

                // Only render the Rocrail radio when the tab is also
                // visible (see generateSettingsTabNav()) - an orphaned,
                // never-clickable radio would otherwise be pointless.

                if (String(SETTINGS_TAB_KEYS[i]) == "rocrail" && !rocrailEnabled) continue;

                chunk += "<input type='radio' name='tabs' id='tab-" + String(SETTINGS_TAB_KEYS[i]) +
                         "' class='tabctrl'" + (i == 0 ? " checked" : "") + ">";
            }

            chunk += generateSettingsTabNav();

            webserver.sendContent(chunk);
            chunk = "";

            // Panel Status in einer .card (900px breit, zentriert, scrollbar) statt voller Seitenbreite -
            // siehe Kommentar an der Kartenoeffnung unten.

            // Status panel in a .card (900px wide, centered, scrollable) instead of full page width - see the
            // comment at the card's opening below.

            chunk += "<div class='tabpanel panel-status'>";

            // Scrollbares Fenster wie beim Log- und Info-Tab (gleiche Hoehe
            // ueber INFO_LOG_WINDOW_HEIGHT_CSS, gleiche 900px-Breite) - vorher
            // wuchs diese Karte ueber die ganze Seite statt nur der Inhalt.

            // Scrollable window like the Log and Info tabs (same height via
            // INFO_LOG_WINDOW_HEIGHT_CSS, same 900px width) - before, this
            // card grew over the full page instead of just the content.

            chunk += "<div class='card' id='statusContent' style='max-width:900px;" INFO_LOG_WINDOW_HEIGHT_CSS "overflow-y:auto;'>";

            // Von ausserhalb eines privaten Netzes bleibt der Inhalt verborgen (WLAN-Modus, Reset-Grund,
            // Speicher, Rocrail-Server).

            // From outside a private network the content stays hidden (WiFi mode, reset reason, storage,
            // Rocrail server).

            if (!statusAccessAllowed) {
                chunk += "<p>" + translate("Status information is only shown when accessing the clock from a private network") + ".</p>";
                chunk += "</div>"; // Ende .card
                                   // end .card
                chunk += "</div>"; // Ende panel-status
                                   // end panel-status
            }
            else {
            chunk += "<ul>";
            generateStatusItems(chunk, [](String& part) { webserver.sendContent(part); part = ""; });
            chunk += "</ul>";
            chunk += "</br>";
            chunk += "<li>Contact: <a href='mailto:howl-clock@gmx.de'>howl-clock@gmx.de</a></li>";
            chunk += "<li>Project: <a href='" GITHUB_REPO_URL "' target='_blank'>GitHub</a></li>";
            chunk += "</ul>";
            chunk += "</div>"; // Ende .card
                               // end .card
            chunk += "</div>"; // Ende panel-status
                               // end panel-status
            } // Ende else (statusAccessAllowed)
              // end else (statusAccessAllowed)

            webserver.sendContent(chunk);
            chunk = "";

            // Panel Log: zeigt die aktuell aktive Logdatei, mit abschaltbarem
            // 10s-Auto-Refresh und manuellem Button ueber /api/currentLog.
            // Inhalt wird per JS lazy-geladen statt serverseitig mitgerendert.

            // Log panel: shows the currently active log file, with a
            // togglable 10s auto-refresh and a manual button via
            // /api/currentLog. Content is lazy-loaded via JS, not server-rendered.

            chunk += "<div class='tabpanel panel-log'>";

            // Nur aus einem privaten Netz sichtbar (wie der Status-Tab) - Logs koennen IPs, SSIDs u.ae.
            // enthalten.

            // Only visible from a private network (like the Status tab) - logs may contain IPs, SSIDs etc.

            if (!isPrivateNetworkIp(webserver.client().remoteIP())) {
                chunk += "<p>" + translate("This action is only available when accessing the clock from a private network") + ".</p>";
            }
            else {

            // Nur ein Preferences-Zugriff, kein Datei-Lesevorgang - unproblematisch
            // bei jedem Seitenaufruf. JS-Refresh haelt #logFileName aktuell.

            // Just a Preferences lookup, not a file read - unproblematic on
            // every page load. The JS refresh keeps #logFileName up to date.

            String currentLogFileName = loggingEnabled ? getCurrentLogFileName() : "-";

            // 900px statt der ueblichen 500px, damit die Karte mit dem Logfenster darunter fluchtet.
            // disabledAttr: ohne Logging haette ein Klick keine Wirkung.

            // 900px instead of the usual 500px, so the card aligns with the log window below. disabledAttr:
            // without logging a click would have no effect.

            String disabledAttr = loggingEnabled ? "" : " disabled";

            chunk += "<div class='card' style='max-width:900px;'>";
            chunk += "<div style='margin-bottom:8px;'>" + translate("Log file") + ": <select id='logFileSelect' style='width:auto;display:inline-block;margin:0;padding:4px 8px;'" + disabledAttr + "><option>" + escapeHtmlText(currentLogFileName) + "</option></select></div>";
            chunk += "<div style='display:flex;align-items:center;gap:14px;flex-wrap:wrap;'>";
            chunk += "<label style='display:flex;align-items:center;gap:6px;white-space:nowrap;cursor:pointer;'>";
            chunk += "<input type='checkbox' id='logAutoRefresh' style='width:auto;margin:0;'" + disabledAttr + ">";
            chunk += translate("Auto-refresh (10s)");
            chunk += "</label>";

            // Nicht .reset-btn wiederverwendet - dessen rote Warnfarbe waere
            // fuer ein harmloses "Jetzt aktualisieren" irrefuehrend.

            // Not reusing .reset-btn - its red warning color would be
            // misleading for a harmless "refresh now".

            chunk += "<button type='button' id='logRefreshNow' style='background:var(--panel);border:1px solid var(--panel-border);color:var(--text);border-radius:.4rem;padding:4px 12px;font-size:.8rem;cursor:pointer;'" + disabledAttr + ">" + translate("Refresh now") + "</button>";
            chunk += "<span id='logErrorHint' class='offline-hint'>&#9888; " + translate("Refresh failed - showing last known content") + "</span>";
            chunk += "</div>";
            chunk += "</div>";

            // Nur ein Platzhalter - der Inhalt kommt per JS von /api/currentLog (siehe "Lazy-Load" unten).
            // Just a placeholder - the content arrives via JS from /api/currentLog (see "lazy load" below).

            chunk += "<pre id='logContent' style='background:var(--panel);border:1px solid var(--panel-border);border-radius:10px;max-width:900px;" INFO_LOG_WINDOW_HEIGHT_CSS "overflow-y:auto;margin:15px auto;padding:12px 16px;text-align:left;white-space:pre-wrap;word-break:break-word;font-family:monospace;font-size:.85rem;'>";
            chunk += loggingEnabled ? translate("Loading&hellip;") : translate("Logging is disabled.");
            chunk += "</pre>";

            // Auto-Refresh: Haken-Status in localStorage, pollt alle 10 s. Die Dateiliste kommt per JS von
            // /api/logFileList, neueste zuerst.

            // Auto-refresh: checkbox state in localStorage, polls every 10 s. The file list comes via JS from
            // /api/logFileList, newest first.

            chunk += "<script>";
            chunk += "(function() {";
            chunk += "  var cb = document.getElementById('logAutoRefresh');";
            chunk += "  var pre = document.getElementById('logContent');";
            chunk += "  var select = document.getElementById('logFileSelect');";
            chunk += "  var errEl = document.getElementById('logErrorHint');";
            chunk += "  var refreshBtn = document.getElementById('logRefreshNow');";
            chunk += "  var timer = null;";
            chunk += "  var loggingEnabled = " + String(loggingEnabled ? "true" : "false") + ";";
            chunk += "  var stored = localStorage.getItem('uhr4LogAutoRefresh');";
            chunk += "  cb.checked = (stored === null) ? false : (stored === '1');";
            chunk += "  function scrollToBottom() { pre.scrollTop = pre.scrollHeight; }";
            chunk += "  function refreshLog() {";
            chunk += "    if (!loggingEnabled || !select.value) return;";
            chunk += "    fetch('/api/currentLog?file=' + encodeURIComponent(select.value), {cache:'no-store'}).then(function(r){";
            chunk += "      return r.text();";
            chunk += "    }).then(function(text){";
            chunk += "      errEl.classList.remove('show');";
            chunk += "      pre.textContent = text;";
            chunk += "      scrollToBottom();";
            chunk += "    }).catch(function(){ errEl.classList.add('show'); });";
            chunk += "  }";

            // Dateiliste laden und die neueste auswaehlen - ausser eine bewusst gewaehlte aeltere Datei
            // existiert noch. So erscheinen per Rotation neu angelegte Logs, ohne ein gerade gelesenes
            // aelteres wegzureissen.

            // Load the file list and select the newest - unless a deliberately chosen older file still
            // exists. So logs newly created by rotation appear without yanking away an older one being read.

            chunk += "  function loadFileList() {";
            chunk += "    fetch('/api/logFileList', {cache:'no-store'}).then(function(r){ return r.json(); }).then(function(list){";
            chunk += "      var previousValue = select.value;";
            chunk += "      var wasOnNewest = (select.options.length === 0) || (select.options[0] && previousValue === select.options[0].value);";
            chunk += "      select.innerHTML = '';";
            chunk += "      list.forEach(function(name){";
            chunk += "        var opt = document.createElement('option');";
            chunk += "        opt.value = name; opt.textContent = name;";
            chunk += "        select.appendChild(opt);";
            chunk += "      });";
            chunk += "      if (list.length > 0) {";
            chunk += "        select.value = (wasOnNewest || list.indexOf(previousValue) === -1) ? list[0] : previousValue;";
            chunk += "      }";
            chunk += "      refreshLog();";
            chunk += "    }).catch(function(){ refreshLog(); });";
            chunk += "  }";
            chunk += "  function stopTimer() { if (timer) { clearInterval(timer); timer = null; } }";
            chunk += "  function applyState() {";
            chunk += "    stopTimer();";
            chunk += "    if (cb.checked && !document.hidden) {";
            chunk += "      timer = setInterval(loadFileList, 10000);";
            chunk += "    }";
            chunk += "  }";
            chunk += "  refreshBtn.addEventListener('click', loadFileList);";
            chunk += "  select.addEventListener('change', function() {";

            // Wird eine aeltere Datei gewaehlt, das Auto-Refresh abschalten - fuer eine statische Datei
            // waeren die Anfragen unnoetig.

            // When an older file is selected, turn off auto-refresh - requests for a static file would be
            // needless.

            chunk += "    var isNewest = select.options.length > 0 && select.value === select.options[0].value;";
            chunk += "    if (!isNewest && cb.checked) {";
            chunk += "      cb.checked = false;";
            chunk += "      localStorage.setItem('uhr4LogAutoRefresh', '0');";
            chunk += "      applyState();";
            chunk += "    }";
            chunk += "    refreshLog();";
            chunk += "  });";
            chunk += "  cb.addEventListener('change', function() {";
            chunk += "    localStorage.setItem('uhr4LogAutoRefresh', cb.checked ? '1' : '0');";
            chunk += "    if (cb.checked) loadFileList();";
            chunk += "    applyState();";
            chunk += "  });";
            chunk += "  document.addEventListener('visibilitychange', function() {";
            chunk += "    if (document.hidden) { stopTimer(); }";
            chunk += "    else { if (cb.checked) loadFileList(); applyState(); }";
            chunk += "  });";
            chunk += "  loadFileList();";
            chunk += "  applyState();";
            chunk += "})();";
            chunk += "</script>";
            } // Ende else (private Netz)
              // end else (private network)

            chunk += "</div>"; // Ende panel-log
                               // end panel-log

            webserver.sendContent(chunk);
            chunk = "";

            // Panel: WLAN (Hostname-Formular, WPS/Rescan, WLAN-Slots).
            // Panel: WiFi (hostname form, WPS/rescan, WiFi slots).

            chunk += "<div class='tabpanel panel-wlan'>";

            if (webserver.arg("msg") == "WPS active - press the WPS button on your router now. Connection to the clock may be lost for about 2 minutes while this happens") {
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
                chunk += "<div class='msg warn'>" +
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

            // hostname kommt unfiltriert aus /sethostname - ohne escapeHtmlText()
            // koennte ein "'" darin dieses title-Attribut aufbrechen.

            // hostname comes unfiltered from /sethostname - without
            // escapeHtmlText(), a "'" in it could break out of this title attribute.

            chunk += infoTip(translate("The clock can also be reached at http://&quot;hostname&quot;.local instead of its IP address, e.g.") + " http://" + escapeHtmlText(String(hostname)) + ".local. " + translate("A restart is required for a changed hostname to take effect. Not all routers support hostname resolution"));
            chunk += "<input name='hostname' maxlength='30' value='" + escapeHtmlText(String(hostname)) + "' style='width:170px;'>";
            chunk += "<button type='submit' style='width:140px;'>" + translate("Save") + "</button>";
            chunk += "</div>";
            chunk += "</form><br><br>";

            webserver.sendContent(chunk);
            chunk = "";

            chunk += "<div style='display:flex;justify-content:center;align-items:center;gap:12px;flex-wrap:wrap;'>";
            chunk += "<form method='POST' action='/api/startWPS' style='margin:0;'>";
            chunk += "<button type='submit' style='width:170px;'>" + translate("Add Network via WPS") + "</button>";
            chunk += "</form>";
            chunk += infoTip(translate("Adds a new network via WPS - press the WPS button on your router when prompted. The clock's connection may be lost for about 2 minutes while this happens"));
            chunk += "<button id='rescanBtn' type='button' style='width:170px;'>" + translate("Rescan Networks") + "</button>";
            chunk += infoTip(translate("Scans for available WiFi networks again and refreshes the dropdown lists below"));
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
                    chunk += " " + infoTip(translate("Up to") + " " + String(MAX_WLAN) + " " + translate("WiFi networks can be stored"));
                }
                chunk += "</h3>";

                chunk += "<div style='display:flex;gap:6px;align-items:center;flex-wrap:wrap;justify-content:center;'>";
                chunk += "<select id='" + ssidSelectId + "' onchange=\"document.getElementById('" + ssidKey + "').value=this.value\" style='max-width:180px;'>";
                chunk += "</select>";

                // wifiSsid[i] stammt aus WLAN-Scan/Nutzereingabe (/savewifi) -
                // ohne escapeHtmlText() koennte ein "'" im SSID-Namen dieses
                // value-Attribut aufbrechen (stored XSS).

                // wifiSsid[i] comes from a WiFi scan/user input (/savewifi) -
                // without escapeHtmlText(), a "'" in the SSID name could break
                // out of this value attribute (stored XSS).

                chunk += "<input name='" + ssidKey + "' id='" + ssidKey + "' placeholder='" + ssidKey + "' value='" + escapeHtmlText(wifiSsid[i]) + "' style='width:110px;'>";
                chunk += "<input name='" + passKey + "' id='" + passKey + "' placeholder='Password' type='password' value='' style='width:110px;'>";
                if (wifiSsid[i] != "") {

                    // "Verbinden"-Button nur bei mehr als einem gespeicherten Netzwerk.
                    // "Connect" button only when more than one network is saved.

                    if (savedWifiCount > 1) {
                        chunk += " <a href='/api/connectWifi?index=" + String(i) + "' onclick='return confirm(\"" + translate("Connect") + " " + escapeForJsStringInAttr(wifiSsid[i], '"') + "?\")'>" + translate("Connect") + "</a>";
                    }
                    chunk += " <a href='/deletewifi?index=" + String(i) + "' onclick='return confirm(\"" + translate("Delete") + " " + escapeForJsStringInAttr(wifiSsid[i], '"') + "?\")'>" + translate("Delete") + "</a>";
                }
                chunk += "</div>";

                chunk += "<small>" + translate("You can also enter an SSID manually") + ".";
                if (WiFi.getMode() == WIFI_STA && wifiSsid[i] != "") {
                    chunk += " " + translate("Password is hidden. Leave empty to keep current") + ".";
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

            // Wie die Erfolgsmeldung (z.B. nach "WPS starten") oben ueber der Sprachauswahl, dorthin scrollen
            // Like the success message (e.g. after "Start WPS") at the top above the language selector, scroll there

            chunk += "  hint.className = 'msg ok';";
            chunk += "  hint.innerHTML = '" + translate("Scanning for WiFi networks - the page will reload automatically in 10 seconds") + "';";
            chunk += "  var lang = document.querySelector(\"form[action='/setLanguage']\");";
            chunk += "  if (lang) lang.parentNode.insertBefore(hint, lang); else btn.parentNode.insertBefore(hint, btn.nextSibling);";
            chunk += "  hint.scrollIntoView({behavior: 'smooth', block: 'center'});";
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
                chunk += "  " + select + ".innerHTML = \"<option>" + translate("WLAN scan in progress") + "...</option>\";";
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
                chunk += "    .catch(() => { " + select + ".innerHTML = \"<option>" + translate("Scan failed") + "</option>\"; });";

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

            // Panel Zifferblatt: Anzeige-Einstellungen.
            // Clock face panel: display settings.

            chunk += "<div class='tabpanel panel-zifferblatt'>";
            chunk += "<form action='/applydisplaysettings' method='POST'>";
            chunk += "<div class='card'>";

            chunk += checkboxRow("showSecondHand", preferences.getBool(PK_SHOW_SECOND_HAND, true), translate("Show Seconds"), translate("Shows or hides the second hand on the clock face"), "cbShowSec");

            chunk += checkboxRow("stationMode", preferences.getBool(PK_STATION_MODE, true), translate("Train Station Mode"), translate("The second hand completes its lap in about 58.5 seconds and then waits at 60 until the minute changes, like a classic train station clock"), "cbStation");

            // Fallback bewusst PK_STATION_MODE (siehe smoothSecond in uhr4.ino) - Uhren ohne gespeichertes
            // smoothSecond behalten ihr Aussehen.

            // Fallback deliberately PK_STATION_MODE (see smoothSecond in uhr4.ino) - clocks without a stored
            // smoothSecond keep their look.

            chunk += checkboxRow("smoothSecond", getSmoothSecondPref(preferences.getBool(PK_STATION_MODE, true)), translate("Smooth Second Hand"), translate("The second hand moves smoothly instead of jumping in 1-second steps"), "cbSmoothSec");

            // Default false - sonst zeigte die Checkbox nach einem Werksreset faelschlich "aktiviert".
            // Default false - otherwise the checkbox would falsely show "enabled" after a factory reset.

            chunk += checkboxRow("smoothMinute", preferences.getBool(PK_SMOOTH_MINUTE, false), translate("Smooth Minute Hand"), translate("The minute hand moves smoothly instead of jumping in 1-minute steps"));

            // Ohne Sekundenzeiger sind Bahnhofsmodus und sanfter Sekundenzeiger aus und ausgegraut (abgeschaltete
            // Felder werden nicht gesendet); wieder angehakt kommt ihr vorheriger Zustand zurueck.

            // Without a second hand, station mode and smooth second hand are off and greyed out (disabled fields
            // are not sent); checked again, their previous state returns.

            chunk += "<script>(function(){var show=document.getElementById('cbShowSec'),deps=['cbStation','cbSmoothSec'].map(function(i){return document.getElementById(i);});";
            chunk += "function upd(){deps.forEach(function(c){if(!show.checked){if(!c.disabled)c.dataset.was=c.checked?'1':'0';c.checked=false;}else if(c.disabled){c.checked=c.dataset.was==='1';}";
            chunk += "c.disabled=!show.checked;c.parentNode.style.opacity=show.checked?'':'0.45';});}show.addEventListener('change',upd);upd();})();</script>";

            chunk += checkboxRow("wifiActive", wifiActive, translate("Reconnect WiFi"), translate("Automatically tries to reconnect if the WiFi connection is lost"));

            // Default aus - schaltet Nutzung und Sichtbarkeit des separaten
            // Rocrail-Tabs frei (siehe generateSettingsTabNav()).

            // Default off - unlocks use and visibility of the separate
            // Rocrail tab (see generateSettingsTabNav()).

            chunk += checkboxRow("rocrailEnabled", rocrailEnabled, translate("Rocrail"), translate("Take over the model time from a Rocrail server (model railroad control software) for the hands - unlocks the Rocrail tab, where the server address can then be entered"));

            chunk += checkboxRow("loggingEnabled", loggingEnabled, translate("Enable Logging"), translate("Writes up to 9 log files to LittleFS for troubleshooting"));

            // Nur sichtbar mit DCF77-Hardware UND dcf77Confirmed - ohne je
            // erkanntes Signal soll die Option gar nicht erst auftauchen.
            // Boards ohne LED (LED_BOARD -1) zeigen sie ebenfalls nicht.

            // Only visible with DCF77 hardware AND dcf77Confirmed - without a
            // signal ever recognized, the option should not appear at all.
            // Boards without an LED (LED_BOARD -1) don't show it either.

            if (dcf77Confirmed && LED_BOARD >= 0) {
                chunk += checkboxRow("dcfSyncLed", dcfSyncLedEnabled, translate("DCF77 Sync LED Blink"), translate("Flashes the LED for every received DCF77 pulse while the clock is still acquiring the time signal"));
            }

            // Displaytyp wie in flashESP (Werte = parseDisplayName(), nur Typen dieses Boards), ohne
            // name-Attribut. Wirkt nach Sicherheitsabfrage sofort per eigenem POST an /save_displaytype; nur
            // ein Wechsel des Displaytyps startet neu (GC9A01 ohne/mit BL nicht).

            // Display type as in flashESP (values = parseDisplayName(), only types of this board), without a
            // name attribute. Takes effect after a confirmation via its own POST to /save_displaytype; only a
            // change of the display type restarts (GC9A01 without/with BL does not).

            {
                const char* curChoice = displayChoiceName(displayType, useBacklight);
                struct { const char* name; uint8_t type; const char* label; } choices[] = {
                    { "GC9A01", DISPLAY_TYPE_GC9A01, "GC9A01 (240x240) without backlight (BL)" },
                    { "GC9A01_WITH_BACKLIGHT", DISPLAY_TYPE_GC9A01, "GC9A01 (240x240) with backlight (BL) on pin {pin}" },
                    { "GC9D01", DISPLAY_TYPE_GC9D01, "GC9D01 (160x160)" },
                    { "ILI9341", DISPLAY_TYPE_ILI9341, "ILI9341 (240x320) with time and date strip" },
                    { "ST7789", DISPLAY_TYPE_ST7789, "ST7789 (172x320, ESP32-C6-LCD-1.47) with time and date strip" },
                    { "ST7789_240", DISPLAY_TYPE_ST7789_240, "ST7789 (240x240, ESP32-C6-LCD-1.3)" },
                };
                chunk += "<div style='display:flex;flex-wrap:wrap;align-items:center;gap:6px;'>" + translate("Display type") + ": " + infoTip(withBacklightPin(translate("Type of the connected display - applies to both displays. BL = backlight: with BL the brightness is controlled via PWM on pin {pin} (same as the backlight checkbox in the brightness tab), without BL by darkening the pixels. Switching to another display type restarts the clock and resets backlight, brightness and hub size to the defaults of the new type; uploaded clock faces and hands only fit the size they were made for (GC9A01 and ILI9341 share the 240 size)"))) + " ";
                chunk += "<select data-cur='" + String(curChoice) + "' data-t='" + String(displayType) + "' style='min-width:190px;max-width:100%;' onchange=\"var o=this.options[this.selectedIndex];if(confirm('" + translate("Change the display type to") + ": '+o.text+'?'+(o.dataset.t!=this.dataset.t?'\\n" + translate("The clock restarts to apply the display type") + ".':''))){var f=document.createElement('form');f.method='POST';f.action='/save_displaytype';var i=document.createElement('input');i.type='hidden';i.name='display';i.value=this.value;f.appendChild(i);document.body.appendChild(f);f.submit();}else{this.value=this.dataset.cur;}\">";
                for (const auto& c : choices) {
                    if (!displayTypeSupported(c.type)) continue;
                    chunk += "<option value='" + String(c.name) + "' data-t='" + String(c.type) + "'";
                    if (strcmp(c.name, curChoice) == 0) chunk += " selected";
                    chunk += ">" + withBacklightPin(translate(c.label)) + "</option>";
                }
                chunk += "</select></div>";
            }

            // Mit Lagesensor (nur S3) zusaetzlich "automatisch" - der Hinweis erklaert den Bezug (imuCalibrate())
            // With a motion sensor (S3 only) additionally "automatic" - the hint explains the reference (imuCalibrate())

            String rotationTip = translate("Rotates the clock face by the selected number of degrees, useful if the display is mounted rotated in its housing");
            if (imuAvailable()) rotationTip += ". " + translate("Automatic: the built-in motion sensor turns the clock face upright. First select and save the rotation that fits the current position, then switch to automatic - the clock takes this position as its reference");
            chunk += "<div style='display:flex;align-items:center;gap:6px;white-space:nowrap;'>" + translate("Rotation Display 1") + ": " + infoTip(rotationTip) + " <select name='rotation' style='width:190px;'>";
            const char* rotationLabels[] = { "0&deg;", "90&deg;", "180&deg;", "270&deg;" };
            String rotationNaLabel = translate("not connected (n.a.)");
            for (int i = 0; i <= TFT_ROTATION_NA; i++) {
                chunk += "<option value='" + String(i) + "'";
                if (i == tftRotation1 && !autoRotation) chunk += " selected";
                chunk += ">" + (i == TFT_ROTATION_NA ? rotationNaLabel : String(rotationLabels[i])) + "</option>";
            }
            if (imuAvailable()) {
                chunk += "<option value='" + String(TFT_ROTATION_AUTO) + "'" + String(autoRotation ? " selected" : "") + ">" +
                         translate("automatic") + " (" + rotationLabels[tftRotation1 < TFT_ROTATION_NA ? tftRotation1 : 0] + ")</option>";
            }
            chunk += "</select></div>";

            // Display 2 nur auf Boards mit zweitem CS-Pin (HAS_DISPLAY2 in config.h)
            // Display 2 only on boards with a second CS pin (HAS_DISPLAY2 in config.h)

            if (HAS_DISPLAY2) {
                chunk += "<div style='display:flex;align-items:center;gap:6px;white-space:nowrap;'>" + translate("Rotation Display 2") + ": " + infoTip(translate("Rotates Display 2's (CS2) clock face independently of Display 1")) + " <select name='rotation2' style='width:190px;'>";
                for (int i = 0; i <= TFT_ROTATION_NA; i++) {
                    chunk += "<option value='" + String(i) + "'";
                    if (i == tftRotation2) chunk += " selected";
                    chunk += ">" + (i == TFT_ROTATION_NA ? rotationNaLabel : String(rotationLabels[i])) + "</option>";
                }
                chunk += "</select></div>";

                // Hinweis: ein nicht angeschlossenes Display auf "n.a." stellen - dann
                // bleibt es schwarz, Zifferblatt/Zeiger werden nicht gezeichnet/berechnet.
                // Status-/Startmeldungen erscheinen bis zum Uhrstart weiterhin auf beiden.

                // Hint: set a display that is not connected to "n.a." - then it stays
                // black, face/hands are not drawn/calculated. Status/boot messages still
                // appear on both until the clock takes over.

                chunk += "<small>" + translate("Set a display that is not physically connected to n.a. - it then stays black and the clock face is neither drawn nor calculated for it. Boot, access point and code messages still appear on both displays until the clock takes over") + ".</small><br><br>";
            }

            chunk += "</div>";
            chunk += "<div style='text-align:center;margin-top:15px;'><button type='submit'>" + translate("Save") + "</button></div>";
            chunk += "</form>";

            chunk += "</div>"; // Ende panel-zifferblatt
                               // end panel-zifferblatt

            webserver.sendContent(chunk);
            chunk = "";

            // Panel: Helligkeit (inkl. optionalem Plotly-Gamma-Chart).
            // Panel: brightness (incl. optional Plotly gamma chart).

            chunk += "<div class='tabpanel panel-helligkeit'>";

            // Hinweis: solange Rocrail verbunden ist und mindestens einmal
            // einen bri-Wert gemeldet hat, uebernimmt es die Helligkeit
            // komplett - dieselbe Bedingung wie in updateBrightness() (display.h).

            // Hint: as long as Rocrail is connected and has reported at
            // least one bri value, it fully takes over the brightness -
            // same condition as in updateBrightness() (display.h).

            bool rocrailBrightnessActiveForDisplay = rocrailEnabled && rocrailConnected && rocrailBrightnessKnown &&
                                                      (millis() - rocrailLastClockMillis) < ROCRAIL_STALE_TIMEOUT_MS;
            if (rocrailBrightnessActiveForDisplay) {
                chunk += "<div class='msg warn'>" +
                    translate("Brightness is currently taken over from Rocrail - the photoresistor and time-window settings below are inactive while connected") + ".</div><br>";
            }

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

                // Bei GET-Formularen ersetzt der Browser den Query-String der
                // Action-URL durch die Formularfelder - "tab" daher als verstecktes Feld.

                // With GET forms the browser replaces the action URL's query
                // string with the form fields - so "tab" goes as a hidden field.

                chunk += "<form method='GET' action='/'><input type='hidden' name='tab' value='helligkeit'><button type='submit'>" + translate("Refresh") + "</button></form>";
                chunk += "<br>";

                webserver.sendContent(chunk);
                chunk = "";

                if (useBacklight && photoresistorFound) { // Gamma-Kurve nur mit Backlight und Lichtsensor
                                                          // gamma curve only with a backlight and a light sensor
                chunk += "<script src='https://cdn.plot.ly/plotly-latest.min.js'></script>\n";

                // "adc"/"targetBrightness" bleiben unuebersetzt: das sind die
                // Variablennamen aus dem Sketch, keine uebersetzbaren Woerter
                // (gleiche Begruendung wie bei den Achsentiteln weiter unten).

                // "adc"/"targetBrightness" stay untranslated: these are the
                // sketch's variable names, not translatable words (same
                // reasoning as for the axis titles further below).

                chunk += "<h2>" + translate("Gamma Correction") + ": adc &rarr; targetBrightness</h2>\n";
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

                // Plotly rendert den Diagrammtitel als SVG-Text und dekodiert
                // HTML-Entities dabei NICHT - ueber decodeHtml() aufloesen,
                // wie bei den WLAN-Labels weiter unten.

                // Plotly renders the chart title as SVG text and does NOT
                // decode HTML entities - resolve it via decodeHtml(), same
                // as the WiFi labels further below.

                chunk += "function decodeHtml(s){var t=document.createElement('textarea');t.innerHTML=s;return t.value;}\n";
                chunk += "const gammaCurveTitle = decodeHtml('" + translate("Gamma correction curve") + "');\n";

                chunk += "function plotGamma(gamma) {\n";
                chunk += "  const y = computeBrightness(gamma);\n";
                chunk += "  Plotly.newPlot('plot', [{\n";
                chunk += "    x: gammaAvg,\n";
                chunk += "    y: y,\n";
                chunk += "    mode: 'lines',\n";
                chunk += "    name: `Gamma = ${gamma.toFixed(1)}`\n";
                chunk += "  }], {\n";
                chunk += "    title: gammaCurveTitle,\n";
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

                // Plotly kann in ein per CSS verstecktes Panel nicht zeichnen
                // (Breite/Hoehe 0) - daher erst beim Aktivieren des Tabs zeichnen
                // (bzw. sofort, falls er schon aktiv ist).

                // Plotly cannot draw into a panel hidden via CSS (width/height 0) -
                // so draw only once the tab is activated (or immediately, if it
                // is already active).

                chunk += "var gammaTab = document.getElementById('tab-helligkeit');\n";
                chunk += "function drawGammaIfVisible() { if (gammaTab.checked) plotGamma(parseFloat(slider.value)); }\n";
                chunk += "gammaTab.addEventListener('change', drawGammaIfVisible);\n";
                chunk += "drawGammaIfVisible();\n";
                chunk += "</script>\n";
                }
            }

            chunk += "</div>"; // Ende panel-helligkeit
                               // end panel-helligkeit

            webserver.sendContent(chunk);
            chunk = "";

            // Panel: Zeit/NTP/Zeitzone.
            // Panel: time/NTP/time zone.

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

                    // Nummer getrennt angehaengt statt eigener Schluessel je Slot:
                    // die Liste laeuft bis MAX_WLAN, dafuer gaebe es sonst 15
                    // Tabelleneintraege ("NTP Server 1".."NTP Server 15").

                    // Number appended separately instead of a key per slot: the
                    // list runs up to MAX_WLAN, which would otherwise need 15
                    // table entries ("NTP Server 1".."NTP Server 15").

                    chunk += translate("NTP Server") + " " + String(i + 1) + " : <input type = 'text' id='ntpServerInput" + String(i + 1) + "' name = 'ntpServer" + String(i + 1) + "' value = '" + String(ntpServers[i]) + "' style='width:180px;'>";
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
                chunk += "      result.innerHTML = '<div class=\\'msg ok\\'>' + text.substring(3) + '</div>';";
                chunk += "    } else {";
                chunk += "      result.innerHTML = '<div class=\\'msg err\\'>" + translate("Server not reachable") + "</div>';";
                chunk += "    }";
                chunk += "  } catch (e) {";
                chunk += "    result.innerHTML = '<div class=\\'msg err\\'>" + translate("Server not reachable") + "</div>';";
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
                chunk += checkboxRow("openWifiTime", preferences.getBool(PK_OPEN_WIFI_TIME, true), translate("Get the time from open WiFis"),
                                     translate("Only at boot, if the clock cannot connect to any stored WiFi (out of range or no access, e.g. a wrong password) and there is no RTC: the clock briefly connects to up to four open WiFis nearby and gets the time via NTP or from the login page of the hotspot (also via HTTPS), then disconnects again. Every step is written to the log. These are networks of others - only use it if allowed"));
                chunk += "<button type='submit'>" + translate("Save Timezone") + "</button><br><br>";
                chunk += "</form>";

                // Uhrzeit dieses Geraets uebernehmen (setClockFromDevice() im Statusleisten-Skript)
                // Take over this device's time (setClockFromDevice() in the status bar script)

                chunk += "<h3>" + translate("Use device time") + "</h3>";
                chunk += "<p><small>" + translate("Sets the clock to the time of this device - useful without WiFi, RTC and DCF77, e.g. in access point mode. The time zone above applies; NTP and DCF77 correct the time later as usual") + ".</small></p>";
                chunk += "<button type='button' onclick='setClockFromDevice(this)'>&#128339; " + translate("Use device time") + "</button><br><br>";
            }
            chunk += "</div>"; // Ende panel-zeit
                               // end panel-zeit

            webserver.sendContent(chunk);
            chunk = "";

            // Panel Rocrail: Liste moeglicher Serveradressen (nur sichtbar,
            // wenn der Master-Schalter aktiv ist) - wie bei den NTP-Servern
            // immer ein leerer Platz nach dem letzten Eintrag. Radio-Button waehlt den aktiven Server.

            // Rocrail panel: list of possible server addresses (only visible
            // when the master switch is on) - like the NTP servers, always
            // one empty slot after the last entry. Radio button selects the active server.

            chunk += "<div class='tabpanel panel-rocrail'>";
            chunk += "<div class='card' style='max-width:900px;'>";
            chunk += "<form method='POST' action='/save_rocrail'>";

            for (int i = 0; i < MAX_WLAN; i++) {
                chunk += "<div style='display:flex;justify-content:center;align-items:center;gap:8px;flex-wrap:wrap;margin-bottom:6px;'>";
                chunk += "<input type='radio' name='activeServer' value='" + String(i) + "'" +
                         String(i == rocrailActiveServerIndex ? " checked" : "") +
                         " title='" + translate("Use this server") + "'>";
                chunk += "<label>" + translate("Server") + " " + String(i + 1) + ":</label>";
                chunk += "<input type='text' name='" + pkRocrailServerHost(i) + "' placeholder='" + translate("IP address or hostname") + "' style='width:180px;' value='" +
                         escapeHtmlText(String(rocrailServerList[i])) + "'>";
                chunk += "<label>" + translate("Port") + ":</label>";
                chunk += "<input type='number' name='" + pkRocrailServerPort(i) + "' min='1' max='65535' style='width:90px;' value='" +
                         String(rocrailServerPortList[i]) + "'>";
                chunk += "<label>" + translate("Layout name") + " (" + translate("optional") + "):</label>";
                chunk += "<input type='text' name='" + pkRocrailServerName(i) + "' placeholder='" + translate("optional") + "' style='width:150px;' value='" +
                         escapeHtmlText(String(rocrailServerNameList[i])) + "'>";
                chunk += "</div>";
                if (trim(String(rocrailServerList[i])) == "") break;
            }

            chunk += "<small>" + translate("Rocrail model time can run much faster than real time (the divider) - the station-clock second-hand animation speeds up by the same factor instead of switching off, so it stays in sync with the model minutes") +
                     ". " + translate("Above a divider of") + " " + String(ROCRAIL_HIDE_DETAILS_DIVIDER) + " " + translate("it is hidden entirely, since it would no longer be meaningfully readable") + ".</small><br><br>";
            chunk += "<button type='submit'>" + translate("Save Settings") + "</button>";
            chunk += "</form>";
            chunk += "</div>";

            // "ready" statt nur rocrailConnected: erst wenn auch echte
            // Modellzeit-Daten angekommen sind, zeigt der Status "Verbunden"
            // (gruen) - sonst wuerde er schon gruen zeigen, waehrend Divider/Modellzeit noch "-" zeigen.

            // "ready" instead of just rocrailConnected: only once real model-
            // time data has actually arrived does the status show
            // "Connected" (green) - otherwise it would show green while divider/model time still show "-".

            bool rocrailReady = rocrailEnabled && rocrailConnected && rocrailLastClockMillis != 0;

            chunk += "<div class='card' id='rocrailStatusCard' style='max-width:900px;'>";
            chunk += "<div>" + translate("Status") + ": <span class='dot" +
                     String(rocrailEnabled ? (rocrailReady ? " ok" : " syncing") : " na") +
                     "' id='dot-rocrail-panel'></span> ";

            // Drei vorgerenderte, uebersetzte Text-Spans statt JS textContent -
            // wie beim ".status[hidden]"-Muster der Topbar-Punkte dekodiert
            // der Browser Entities so korrekt; JS blendet nur per "hidden" um.

            // Three pre-rendered, translated text spans instead of JS
            // textContent - like the ".status[hidden]" pattern of the topbar
            // dots, the browser decodes entities correctly this way; JS only toggles "hidden".

            chunk += "<span id='rocrailStatusOff'" + String(rocrailEnabled ? " hidden" : "") + ">" + translate("Disabled") + "</span>";
            chunk += "<span id='rocrailStatusSearching'" + String((rocrailEnabled && !rocrailReady) ? "" : " hidden") + ">" + translate("Connecting") + "...</span>";
            chunk += "<span id='rocrailStatusConnected'" + String(rocrailReady ? "" : " hidden") + ">" + translate("Connected") + "</span>";
            chunk += "</div>";
            chunk += "<div id='rocrailDetails'" + String(rocrailEnabled ? "" : " hidden") + ">";
            chunk += "<div>" + translate("Server") + ": <code id='rocrailHost'>-</code></div>";
            chunk += "<div>" + translate("Divider") + ": <code id='rocrailDivider'>-</code></div>";
            chunk += "<div>" + translate("Model time") + ": <code id='rocrailModelTime'>-</code>";

            // Vorgerenderter Span statt JS textContent - gleicher Grund wie
            // bei rocrailStatusOff/-Searching/-Connected oben (Entities in
            // Uebersetzungen wuerden sonst doppelt escaped).

            // Pre-rendered span instead of JS textContent - same reason as
            // rocrailStatusOff/-Searching/-Connected above (entities in
            // translations would otherwise be double-escaped).

            chunk += " <span id='rocrailFrozenHint'" + String((rocrailReady && rocrailFrozen) ? "" : " hidden") + ">(" + translate("paused") + ")</span>";
            chunk += "</div>";
            chunk += "</div>";
            chunk += "</div>";

            // Live-Poll alle 3s, solange der Tab sichtbar ist - kein eigener
            // Sichtbarkeits-/Pause-Mechanismus wie beim Log-Tab noetig, da
            // die Antwort winzig ist (im Gegensatz zum ganzen Logfile).

            // Live poll every 3s while the tab is visible - no dedicated
            // visibility/pause mechanism like the Log tab needs, since the
            // response is tiny (unlike the whole log file).

            chunk += "<script>";
            chunk += "(function() {";
            chunk += "  var dot = document.getElementById('dot-rocrail-panel');";
            chunk += "  var off = document.getElementById('rocrailStatusOff');";
            chunk += "  var searching = document.getElementById('rocrailStatusSearching');";
            chunk += "  var connected = document.getElementById('rocrailStatusConnected');";
            chunk += "  var details = document.getElementById('rocrailDetails');";
            chunk += "  var frozenHint = document.getElementById('rocrailFrozenHint');";
            chunk += "  function poll() {";
            chunk += "    fetch('/api/rocrailStatus', {cache:'no-store'}).then(function(r){return r.json();}).then(function(s){";
            chunk += "      off.hidden = s.enabled; details.hidden = !s.enabled;";
            chunk += "      searching.hidden = !s.enabled || s.ready;";
            chunk += "      connected.hidden = !s.enabled || !s.ready;";
            chunk += "      if (!s.enabled) { dot.className='dot na'; return; }";
            chunk += "      dot.className = s.ready ? 'dot ok' : 'dot syncing';";
            chunk += "      document.getElementById('rocrailHost').textContent = s.host ? (s.host + ':' + s.port) : '-';";
            chunk += "      document.getElementById('rocrailDivider').textContent = s.divider;";
            chunk += "      document.getElementById('rocrailModelTime').textContent = s.modelTime;";
            chunk += "      frozenHint.hidden = !s.ready || !s.frozen;";
            chunk += "    }).catch(function(){});";
            chunk += "  }";
            chunk += "  poll();";
            chunk += "  setInterval(poll, 3000);";
            chunk += "})();";
            chunk += "</script>";

            chunk += "</div>"; // Ende panel-rocrail
                               // end panel-rocrail

            webserver.sendContent(chunk);
            chunk = "";

            // ?tab=... springt direkt zu einem Tab (z.B. nach einem POST-Redirect) -
            // rein clientseitig, Tab-Auswahl laeuft ueber CSS-radio.

            // ?tab=... jumps directly to a tab (e.g. after a POST redirect) -
            // purely client-side, tab selection works via CSS radio.

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

            setLedOff(); // Gegenstueck zu setLedOn() ganz oben in diesem Handler
                         // counterpart to setLedOn() at the very top of this handler
            });

        webserver.on("/deletewifi", HTTP_GET, []() {

            // WLAN-Loeschungen nur aus einem privaten Netz, auch nicht per Bestaetigungscode (anders als
            // Ueberschreiben in /save und Wechseln in /api/connectWifi).

            // WiFi deletions only from a private network, not even via a confirmation code (unlike
            // overwriting in /save and switching in /api/connectWifi).

            if (!isPrivateNetworkIp(webserver.client().remoteIP())) {
                webserver.send(200, "text/html", simpleMessagePage(translate("Delete"), "<p>" + translate("This action is only available when accessing the clock from a private network") + ".</p>"));
                return;
            }

            if (webserver.hasArg("index")) {
                int idx = webserver.arg("index").toInt();
                if (idx >= 0 && idx < MAX_WLAN) {

                    // Das aktive Netz ueber executePendingAction() loeschen (kompaktiert und startet neu) -
                    // der Zugriff ist hier schon als privat bestaetigt, daher ohne Code.

                    // Delete the active network via executePendingAction() (compacts and reboots) - access is
                    // already confirmed as private here, so no code.

                    bool deletingActiveNetwork = (WiFi.status() == WL_CONNECTED && wifiSsid[idx] != "" && WiFi.SSID() == wifiSsid[idx]);

                    if (deletingActiveNetwork) {
                        pendingWifiChangeIndex = idx;
                        executePendingAction("wlanDeleteActive");
                        return;
                    }

                    // Liste kompaktieren wie bei "wlanDeleteActive" oben -
                    // ueber applyWlanList() geteilt, statt dieselbe
                    // Kompaktierungslogik hier ein zweites Mal zu pflegen.

                    // Compact the list as with "wlanDeleteActive" above -
                    // shared via applyWlanList(), instead of maintaining the
                    // same compaction logic here a second time.

                    String newSsid[MAX_WLAN];
                    String newPass[MAX_WLAN];
                    for (int i = 0; i < MAX_WLAN; i++) {
                        newSsid[i] = preferences.getString(pkSsid(i).c_str(), "");
                        newPass[i] = loadWifiPass(i);
                    }
                    newSsid[idx] = "";
                    newPass[idx] = "";
                    applyWlanList(newSsid, newPass);
                }
                redirectTo("/?tab=wlan&msg=Network%20deleted");
            }
            else {
                webserver.send(400, "text/plain", "Missing parameter");
            }
            });

        // Verbindet sofort mit einem gespeicherten WLAN. connectWiFi() blockiert
        // bis zu 30s, daher stattdessen PK_LAST_WLAN setzen und neu starten.

        // Connects immediately to a saved WiFi network. connectWiFi() blocks
        // for up to 30s, so instead set PK_LAST_WLAN and reboot.

        webserver.on("/api/connectWifi", HTTP_GET, []() {
            if (webserver.hasArg("index")) {
                int idx = webserver.arg("index").toInt();
                if (idx >= 0 && idx < MAX_WLAN && preferences.getString(pkSsid(idx).c_str(), "") != "") {

                    // Wechsel auf ein ANDERES Netz kann die Verbindung dauerhaft kappen: aus einem privaten
                    // Netz direkt, sonst erst per Bestaetigungscode. Neustart ins schon aktive Netz ist
                    // risikolos, bleibt aber privat-only (DoS-Schutz).

                    // Switching to a DIFFERENT network can sever the connection for good: from a private
                    // network directly, otherwise via a confirmation code first. Restarting into the already
                    // active network is safe but stays private-only (DoS).

                    bool isAlreadyActive = (WiFi.status() == WL_CONNECTED && WiFi.SSID() == wifiSsid[idx]);

                    if (!isAlreadyActive) {
                        if (isPrivateNetworkIp(webserver.client().remoteIP())) {
                            pendingWifiChangeIndex = idx;
                            DEBUG_PRINTLN("[SECURITY] WiFi network switch executed directly (private network) from " + webserver.client().remoteIP().toString());
                            executePendingAction("wlanSwitchActive");
                            return;
                        }

                        // rejectIfConfirmationPending() ZUERST, vor pendingWifiChangeIndex - sonst wuerde die
                        // Nutzlast einer anderen wartenden Anfrage ueberschrieben.

                        // rejectIfConfirmationPending() FIRST, before pendingWifiChangeIndex - otherwise
                        // another pending request's payload would be overwritten.

                        if (rejectIfConfirmationPending()) return;

                        pendingWifiChangeIndex = idx;
                        requestConfirmationCode("wlanSwitchActive");
                        DEBUG_PRINTLN("[SECURITY] Confirmation code requested to switch the active WiFi network to slot " + String(idx + 1) + " from " + webserver.client().remoteIP().toString());
                        redirectTo("/factoryReset/enterCode");
                        return;
                    }

                    if (!isPrivateNetworkIp(webserver.client().remoteIP())) {
                        webserver.send(200, "text/html", simpleMessagePage(translate("Connect"), "<p>" + translate("This action is only available when accessing the clock from a private network") + ".</p>"));
                        return;
                    }

                    DEBUG_PRINTLN("[WiFi] Web UI requested switch to saved network: " + preferences.getString(pkSsid(idx).c_str(), "") + " (from " + webserver.client().remoteIP().toString() + ")");
                    preferences.putInt(PK_LAST_WLAN, idx);
                    redirectTo("/?tab=wlan&msg=Connecting...");

                    // preferences.end() erfolgt in espReboot(), siehe dort.
                    // preferences.end() happens in espReboot(), see there.

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

                String newSsid[MAX_WLAN];
                String newPass[MAX_WLAN];
                for (int i = 0; i < MAX_WLAN; i++) {
                    newSsid[i] = webserver.arg(pkSsid(i));
                    newPass[i] = webserver.arg(pkPass(i));
                }

                // Betrifft das Formular den AKTUELL VERBUNDENEN Slot? Dann aus einem privaten Netz direkt,
                // sonst erst per Code auf dem Display - schuetzt die aktive Verbindung vor Aenderung aus der
                // Ferne. Andere Slots bleiben privat-only.

                // Does the form touch the CURRENTLY CONNECTED slot? Then directly from a private network,
                // otherwise via a code on the display first - protects the active connection from remote
                // changes. Other slots stay private-only.

                int activeIdx = -1;
                if (WiFi.status() == WL_CONNECTED) {
                    for (int i = 0; i < MAX_WLAN; i++) {
                        if (wifiSsid[i] != "" && WiFi.SSID() == wifiSsid[i]) {
                            activeIdx = i;
                            break;
                        }
                    }
                }

                bool activeSlotChanged = false;
                if (activeIdx >= 0) {
                    if (newSsid[activeIdx] != preferences.getString(pkSsid(activeIdx).c_str(), "")) {
                        activeSlotChanged = true;
                    }
                    if (newPass[activeIdx] != "" && newPass[activeIdx] != loadWifiPass(activeIdx)) {
                        activeSlotChanged = true;
                    }
                }

                if (activeSlotChanged) {

                    // Eine leere SSID im aktiven Slot ist ein LOESCHEN - daher wie in /deletewifi nur aus
                    // einem privaten Netz, ohne Code-Alternative, sonst waere /save ein Umweg um diese
                    // Sperre.

                    // An empty SSID in the active slot is a DELETE - so, like /deletewifi, only from a
                    // private network without a code alternative, otherwise /save would be a detour around
                    // that block.

                    bool isActiveDeleteAttempt = (newSsid[activeIdx].length() == 0);

                    if (isActiveDeleteAttempt) {
                        if (!isPrivateNetworkIp(webserver.client().remoteIP())) {
                            webserver.send(200, "text/html", simpleMessagePage(translate("Delete"), "<p>" + translate("This action is only available when accessing the clock from a private network") + ".</p>"));
                            return;
                        }
                        pendingWifiChangeIndex = activeIdx;
                        executePendingAction("wlanDeleteActive");
                        return;
                    }

                    if (isPrivateNetworkIp(webserver.client().remoteIP())) {
                        for (int i = 0; i < MAX_WLAN; i++) {
                            pendingWifiSsid[i] = newSsid[i];
                            pendingWifiPass[i] = newPass[i];
                        }
                        DEBUG_PRINTLN("[SECURITY] Active WiFi network change executed directly (private network) from " + webserver.client().remoteIP().toString());
                        executePendingAction("wlanOverwriteActive");
                        return;
                    }

                    // rejectIfConfirmationPending() ZUERST, vor pendingWifiSsid[]/pendingWifiPass[] - sonst
                    // wuerde die Nutzlast einer anderen wartenden Anfrage ueberschrieben.

                    // rejectIfConfirmationPending() FIRST, before pendingWifiSsid[]/pendingWifiPass[] -
                    // otherwise another pending request's payload would be overwritten.

                    if (rejectIfConfirmationPending()) return;

                    for (int i = 0; i < MAX_WLAN; i++) {
                        pendingWifiSsid[i] = newSsid[i];
                        pendingWifiPass[i] = newPass[i];
                    }
                    requestConfirmationCode("wlanOverwriteActive");
                    DEBUG_PRINTLN("[SECURITY] Confirmation code requested to change the active WiFi network (slot " + String(activeIdx + 1) + ") from " + webserver.client().remoteIP().toString());
                    redirectTo("/factoryReset/enterCode");
                    return;
                }

                // Nur nicht-aktive Slots betroffen: nur aus einem privaten Netz erlaubt (siehe
                // isPrivateNetworkIp() oben).

                // Only non-active slots affected: only allowed from a private network (see
                // isPrivateNetworkIp() above).

                if (!isPrivateNetworkIp(webserver.client().remoteIP())) {
                    webserver.send(200, "text/html", simpleMessagePage(translate("Settings saved"), "<p>" + translate("This action is only available when accessing the clock from a private network") + ".</p>"));
                    return;
                }

                applyWlanList(newSsid, newPass);

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
            String uploadFormHtml = "<form method='POST' action='/upload' enctype='multipart/form-data' onsubmit='showProgress()'><input type='file' name='upload' accept='.bmp' multiple required><br><br><button type='submit'>" + translate("Upload") + " BMP</button><div id='progress' style='display:none;'>" + translate("Uploading... please wait") + "</div><script>function showProgress(){document.getElementById('progress').style.display='block';}</script></form><br><a href='/listfilesFaces'><button type='button'>" + translate("Back") + "</button></a>";
            webserver.send(200, "text/html", simpleMessagePage(translate("Upload"), uploadFormHtml));
            });

        // Datei-Upload verarbeiten
        // Process file upload

        webserver.on("/upload", HTTP_POST, []() {
            if (uploadSuccess) {
                redirectTo("/listfilesFaces?msg=Clock%20face%20uploaded");
            }
            else {
                String errorHtml = "<p>" + translate("Only BMP files with 16, 24 or 32 bit are accepted") + ".</p>";
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

                // designer=1: Link "Designer" - der Designer baut immer auf dem
                // aktiven Zifferblatt auf, daher erst aktivieren, dann oeffnen.

                // designer=1: "Designer" link - the designer always builds on the
                // active clock face, so activate first, then open it.

                String faceTarget = webserver.arg("designer") == "1" ? "/facedesigner" : "/listfilesFaces?msg=Clock%20face%20selected";

                if (file == "/face_default.bmp") {
                    selectedBackground = file;
                    preferences.putString(PK_BACKGROUND, file);
                    freeClockFaceBuffer();
                    loadClockFace();
                    loadHandSprites();
                    if (webserver.arg("ajax") == "1") webserver.send(200, "text/plain", "ok");
                    else redirectTo(faceTarget);
                    return;
                }

                if (LittleFS.exists(file)) {
                    selectedBackground = file;
                    preferences.putString(PK_BACKGROUND, file);
                    DEBUG_PRINTLN("set bg to: " + file + " (from " + webserver.client().remoteIP().toString() + ")");
                    freeClockFaceBuffer();
                    loadClockFace();
                    loadHandSprites();
                    if (webserver.arg("ajax") == "1") webserver.send(200, "text/plain", "ok");
                    else redirectTo(faceTarget);
                    return;
                }
            }
            webserver.send(404, "text/plain", "404 Site not found");
            });

        // Datei löschen
        // Delete file

        webserver.on("/delete", HTTP_GET, []() {

            // Nur aus einem privaten Netz erlaubt (siehe isPrivateNetworkIp()
            // oben) - loescht beliebige Dateien auf LittleFS, das soll aus
            // der Ferne (z.B. ueber eine DMZ/Port-Weiterleitung) nicht moeglich sein.

            // Only allowed from a private network (see isPrivateNetworkIp()
            // above) - deletes arbitrary files on LittleFS, which shouldn't
            // be possible remotely (e.g. via a DMZ/port forward).

            if (!isPrivateNetworkIp(webserver.client().remoteIP())) {
                webserver.send(200, "text/html", simpleMessagePage(translate("Delete"), "<p>" + translate("This action is only available when accessing the clock from a private network") + ".</p>"));
                return;
            }

            if (webserver.hasArg("file")) {
                String path = webserver.arg("file");
                //path.replace(".", "");
                if (!path.startsWith("/")) path = "/" + path;
                if (isProtectedFile(path)) {
                    redirectTo(fileManagerReturnTarget(webserver.arg("from")) + "?err=Built-in%20file%20cannot%20be%20deleted");
                }
                else if (deleteFileWithSideEffects(path)) {
                    String redirectTarget = fileManagerReturnTarget(webserver.arg("from"));
                    redirectTo(redirectTarget + "?msg=File%20deleted");
                }
                else {
                    webserver.send(404, "text/plain", "File not found");
                }
            }
            else {

                // Ohne diesen Zweig bliebe ein parameterloser Request unbeantwortet
                // und wuerde den single-threaded Handler-Loop bis zum Timeout blockieren.

                // Without this branch a parameterless request would go unanswered
                // and block the single-threaded handler loop until timeout.

                webserver.send(400, "text/plain", "Missing parameter: file");
            }
            });

        // Mehrere Dateien auf einmal loeschen (Auswahl im Dateimanager) - wie /delete, nur aus einem privaten Netz
        // Delete several files at once (selection in the file manager) - like /delete, only from a private network

        webserver.on("/deletemulti", HTTP_POST, []() {
            if (!isPrivateNetworkIp(webserver.client().remoteIP())) {
                webserver.send(200, "text/html", simpleMessagePage(translate("Delete"), "<p>" + translate("This action is only available when accessing the clock from a private network") + ".</p>"));
                return;
            }
            int deleted = 0, kept = 0;
            for (int i = 0; i < webserver.args(); i++) {
                if (webserver.argName(i) != "file") continue;
                String path = webserver.arg(i);
                if (!path.startsWith("/")) path = "/" + path;
                path = String(path.c_str()); // an einem eingebetteten Nullbyte kappen (wie /download)
                                             // truncate at an embedded null byte (like /download)
                if (path.indexOf("..") >= 0) continue;
                if (isProtectedFile(path)) { kept++; continue; }
                if (deleteFileWithSideEffects(path)) deleted++;
            }
            redirectTo(deleted ? (kept ? "/files?warn=Files%20deleted%20-%20built-in%20files%20stay" : "/files?msg=Files%20deleted")
                               : (kept ? "/files?err=Built-in%20file%20cannot%20be%20deleted" : "/files?err=No%20files%20selected"));
            });

        // Einzelnes Preset loeschen (Slot wird dadurch wieder frei fuer
        // createPresetFromPreferences())

        // Delete a single preset (frees up the slot again for
        // createPresetFromPreferences())

        webserver.on("/deletepreset", HTTP_GET, []() {

            // Nur aus einem privaten Netz erlaubt (siehe isPrivateNetworkIp() oben).
            // Only allowed from a private network (see isPrivateNetworkIp() above).

            if (!isPrivateNetworkIp(webserver.client().remoteIP())) {
                webserver.send(200, "text/html", simpleMessagePage(translate("Delete"), "<p>" + translate("This action is only available when accessing the clock from a private network") + ".</p>"));
                return;
            }

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

            // Nur aus einem privaten Netz erlaubt (siehe isPrivateNetworkIp() oben).
            // Only allowed from a private network (see isPrivateNetworkIp() above).

            if (!isPrivateNetworkIp(webserver.client().remoteIP())) {
                webserver.send(200, "text/html", simpleMessagePage(translate("Rename"), "<p>" + translate("This action is only available when accessing the clock from a private network") + ".</p>"));
                return;
            }

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

                String path = webserver.arg("name");
                if (!path.startsWith("/")) path = "/" + path;

                // Auf den Stand nach einem evtl. eingebetteten Nullbyte
                // kappen - siehe ausfuehrliche Begruendung bei /download.

                // Truncate at any embedded null byte - see the detailed
                // reasoning at /download.

                path = String(path.c_str());

                // Standard-Zifferblatt und Standard-Zeigersatz bei Bedarf erst erzeugen - die Designer laden sie von hier
                // Create the default clock face and default hand set first if needed - the designers load them from here

                if (path == "/face_default.bmp") ensureDefaultFace();
                else if (path.startsWith("/hand_set0_")) ensureDefaultHands();

                // Logdateien nur aus einem privaten Netz einsehbar (koennen IPs, SSIDs u.ae. enthalten);
                // andere Dateien von ueberall.

                // Log files only viewable from a private network (may contain IPs, SSIDs etc.); other files
                // from anywhere.

                if (path.endsWith(".log") && !isPrivateNetworkIp(webserver.client().remoteIP())) {
                    webserver.send(200, "text/plain", "This action is only available when accessing the clock from a private network.");
                    return;
                }

                setLedOn();

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

                        // Zeiger mit Weiss statt der internen Transparenzfarbe ausliefern (encodeBmpToBytes()) - in
                        // der Zeigeruebersicht sonst dunkelgruen; Uhr und Designer werten beides als transparent.

                        // Serve hands with white instead of the internal transparent colour (encodeBmpToBytes()) -
                        // otherwise dark green in the hand set overview; clock and designers treat both as transparent.

                        int32_t hw = 0, hh = 0;
                        if (path.startsWith("/hand_set") && readImageSize(path.c_str(), hw, hh) && isValidHandSize(hw, hh)) {
                            uint16_t* px = (uint16_t*)preferPsramMalloc((size_t)hw * hh * sizeof(uint16_t));
                            size_t size = 0;
                            uint8_t* bmp = (px && loadFaceBmpInto(path, px, hw, hh)) ? encodeBmpToBytes(px, hw, hh, &size) : nullptr;
                            free(px);
                            if (bmp) {
                                webserver.sendHeader("Cache-Control", webserver.hasArg("v") ? "public, max-age=31536000, immutable" : "no-store");
                                webserver.send_P(200, "image/bmp", (const char*)bmp, size);
                                delete[] bmp;
                                setLedOff();
                                return;
                            }
                        }

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

                            DEBUG_PRINTLN("[FILE] RLE streaming failed for " + path + " (from " + webserver.client().remoteIP().toString() + ")");
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
            chunk += selectWithoutReloadScript(translate("Hand set selected"), translate("active"));
            chunk += "<h2>" + translate("Manage Clock Hand Sets") + " " + String(HAND_WIDTH) + " x " + String(HAND_HEIGHT) + "</h2>";
            chunk += "<p>" + generateStorageInfo(used, total) + "</p>";
            chunk += "<div style='display:flex;flex-wrap:wrap;gap:24px 18px;justify-content:center;align-items:flex-start;'>";
            webserver.sendContent(chunk);

            String activeSet = handSetFileId(preferences.getString(PK_HANDSET, ""));

            // Der Standardsatz 0 fehlt nie in der Liste (z.B. nach dem Loeschen neu erzeugt)
            // The default set 0 is never missing from the list (e.g. recreated after deletion)

            ensureDefaultHands();
            completeHandSets();

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

            // Jeden gefundenen Zeigersatz SOFORT senden statt zu sammeln - so liegt
            // nie mehr als ein Zeigersatz gleichzeitig im Speicher.

            // Send each found hand set IMMEDIATELY instead of collecting them - so
            // never more than one hand set is in memory at a time.

            auto renderSetRow = [&](const String& setId) {

                // setId kommt aus Upload-Dateinamen und ist nicht auf harmlose Zeichen
                // eingeschraenkt (stored XSS) - daher ueberall escapen, ausser fuer
                // Dateisystem-Pfade, die den echten Namen brauchen.

                // setId comes from upload filenames and isn't restricted to harmless
                // characters (stored XSS) - so escape it everywhere except for
                // filesystem paths, which need the real name.

                String safeSetId = escapeHtmlText(setId);
                chunk = "<div style='text-align:center;border:1px solid #ccc;border-radius:6px;padding:8px;'>";
                chunk += "<a href='/sethandset?set=" + safeSetId + "' onclick='return pickItem(this,\"ha\")'>";

                // Fehlende Zeiger eines Satzes zeigt die Uhr aus dem Standardsatz 0 - hier ebenso
                // The clock shows missing hands of a set from the default set 0 - likewise here

                for (const char* part : { "hour", "minute", "second" }) {
                    String path = "/hand_set" + setId + "_" + part + ".bmp";
                    if (!LittleFS.exists(path)) path = String("/hand_set0_") + part + ".bmp";
                    chunk += "<img src='/file?name=" + escapeHtmlText(path) + "&v=" + fileVersion(path) + "'> ";
                }
                chunk += "</a><br>" + safeSetId + "<span class='ha'>" + (setId == activeSet ? " (" + translate("active") + ")" : "") + "</span>";
                chunk += "<br><a href='/sethandset?set=" + safeSetId + "&designer=1'>" + translate("Designer") + "</a>";
                if (isProtectedHandSetId(setId)) chunk += "<br><small>" + translate("built-in") + "</small>";
                else chunk += "<br><a href='/deletehandset?set=" + safeSetId + "' onclick='return confirm(\"" + translate("Delete") + " " + escapeForJsStringInAttr(setId, '"') + "?\")'>" + translate("Delete") + "</a>";
                chunk += "</div>";
                webserver.sendContent(chunk);
                checkHeapWarning("/handsets Zeigersatz " + setId);
                };

            // Zuerst alle numerisch benannten Zeigersaetze in aufsteigender Reihenfolge...
            // First all numerically named hand sets in ascending order...

            // Zeigersaetze des Startpakets vorweg: 0, 1, 2, dann der Sekundenfeld-Satz
            // Starter set hand sets up front: 0, 1, 2, then the subdial set

            for (const String& id : { String("0"), String("1"), String("2"), subdialHandSet() }) {
                if (seenSetIds.count(id)) renderSetRow(id);
            }
            for (auto& entry : numericSets) {
                if (!isProtectedHandSetId(entry.second)) renderSetRow(entry.second);
            }

            // ...danach eventuelle Sonderfaelle mit nicht-numerischem Namen (unsortiert)
            // ...then any special cases with non-numeric names (unsorted)

            for (const String& setId : otherSets) {
                renderSetRow(setId);
            }

            chunk = "</div><br><br>";
            chunk += "<br><br>";
            chunk += "</body></html>";
            webserver.sendContent(chunk);
            webserver.sendContent(""); // Ende der Chunked-Uebertragung signalisieren
                                       // signal the end of the chunked transfer
            });

        // Nabe aus dem Zifferblatt-Designer: size = Radius, color = RGB888 hex; save=0 zeigt nur an,
        // save=1 speichert zusaetzlich.

        // Hub from the clock face designer: size = radius, color = RGB888 hex; save=0 only displays,
        // save=1 also stores.

        webserver.on("/setcenter", HTTP_POST, []() {
            if (!isPrivateNetworkIp(webserver.client().remoteIP())) {
                webserver.send(403, "application/json", "{\"ok\":false}");
                return;
            }
            if (!webserver.hasArg("size") || !webserver.hasArg("color")) {
                webserver.send(400, "application/json", "{\"ok\":false}");
                return;
            }
            hubSize = argToIntClamped("size", hubSize, 0, 100);
            uint32_t rgb = (uint32_t)strtoul(webserver.arg("color").c_str(), nullptr, 16) & 0xFFFFFF;

            // 24-Bit RGB888 in RGB565 umwandeln
            // Convert 24-bit RGB888 to RGB565

            uint8_t r = (rgb >> 16) & 0xFF;
            uint8_t g = (rgb >> 8) & 0xFF;
            uint8_t b = rgb & 0xFF;
            hubColor = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);

            // Drehpunkt des Sekundenzeigers (secx/secy, -1 = Mitte) gehoert zum Zifferblatt: gespeichert in dessen
            // facecfg_*.txt - face = Zifferblatt, das der Designer gerade speichert, sonst das aktive

            // Pivot of the second hand (secx/secy, -1 = centre) belongs to the clock face: stored in its
            // facecfg_*.txt - face = clock face the designer is saving right now, otherwise the active one

            bool pivotArg = webserver.hasArg("secx") && webserver.hasArg("secy");
            if (pivotArg) {
                ensureFaceSettings();
                secPivotX = (int16_t)constrain(webserver.arg("secx").toInt(), -1L, (long)CLOCK_WIDTH - 1);
                secPivotY = (int16_t)constrain(webserver.arg("secy").toInt(), -1L, (long)CLOCK_HEIGHT - 1);
                if (secPivotX < 0 || secPivotY < 0) secPivotX = secPivotY = -1;
                clockFrameDirty[0] = clockFrameDirty[1] = true;
            }
            if (webserver.arg("save") == "1") {
                preferences.putUInt(PK_CENTER_SIZE, hubSize);
                preferences.putLong(PK_CENTER_COLOR, rgb);
                if (pivotArg) saveFaceSettings(webserver.hasArg("face") ? "/" + webserver.arg("face") : selectedBackground, secPivotX, secPivotY);
                DEBUG_PRINTLN("[WEB] Hub saved: radius " + String(hubSize) + ", color " + String(rgb, HEX));
            }
            webserver.send(200, "application/json", "{\"ok\":true}");
            });

        //  Handsets Datei-Upload verarbeiten
        // Process hand-set file upload

        webserver.on("/uploadhandset", HTTP_POST, []() {

            // Sicherheitspruefung auf Dateinamenmuster - auch wenn handleFileUpload() schon abgelehnt hat, damit
            // die Meldung die Namensregel nennt

            // Security check on the filename pattern - also when handleFileUpload() has already rejected, so the
            // message states the naming rule

            if (!uploadFilePath.endsWith(".bmp") || !uploadFilePath.startsWith("/hand_set")) {
                String errorHtml = "<p>" + translate("Only .bmp files starting with") + " <code>hand_</code> " + translate("are accepted for handset upload") + ".</p>";
                errorHtml += "<a href='/handsets'><button type='button'>" + translate("Try again") + "</button></a>";
                webserver.send(400, "text/html", simpleMessagePage(translate("Upload failed"), errorHtml));
                return;
            }
            if (uploadSuccess) {
                String setId = webserver.arg("set");

                //  String target = server.arg("target");

                String dir = "/";
                if (!LittleFS.exists(dir)) LittleFS.mkdir(dir);

                //  String finalPath = "/hand_set" + set + "_" + target + ".bmp";

                //  LittleFS.rename(uploadFilePath, finalPath);
                //  DEBUG_PRINTLN("[UPLOAD] Hand uploaded to: " + finalPath);

                DEBUG_PRINTLN("[UPLOAD] Hand uploaded to: " + uploadFilePath + " (from " + webserver.client().remoteIP().toString() + ")");
                redirectTo("/handsets?msg=Hand%20set%20uploaded");
            }
            else {
                String errorHtml = "<p>" + translate("Only BMP files with 16, 24 or 32 bit are accepted") + ".</p>";
                errorHtml += "<p>" + translate("Please also check the available space") + ".</p>";
                errorHtml += "<a href='/handsets'><button type='button'>" + translate("Try again") + "</button></a>";
                webserver.send(400, "text/html", simpleMessagePage(translate("Upload failed"), errorHtml));
            }
            }, handleFileUpload);


        // Handset setzen
        // Set hand set

        webserver.on("/sethandset", HTTP_GET, []() {
            if (webserver.hasArg("set")) {
                String chosen = webserver.arg("set");
                completeHandSet(handSetFileId(chosen));
                preferences.putString(PK_HANDSET, chosen);
                // DEBUG_PRINTLN("[HANDSET] Set to: " + chosen);
                freeClockFaceBuffer();
                loadClockFace();
                loadHandSprites();
                updateClock();

                // designer=1: Link "Designer" - der Designer baut
                // immer auf dem aktiven Satz auf, daher erst aktivieren, dann oeffnen.

                // designer=1: "Designer" link - the designer always builds
                // on the active set, so activate first, then open it.

                if (webserver.arg("designer") == "1") redirectTo("/handdesigner");
                else if (webserver.arg("ajax") == "1") webserver.send(200, "text/plain", "ok");
                else redirectTo("/handsets?msg=Hand%20set%20selected");
            }
            else {
                webserver.send(400, "text/plain", "Missing set name");
            }
            });

        // CSS und Skript der Designer gzip-komprimiert aus dem Flash (erzeugt von web/build_web.py) - der Browser
        // entpackt. ?v= in der Adresse wechselt mit dem Inhalt, daher darf er sie dauerhaft zwischenspeichern.

        // The designers' CSS and script gzip-compressed from flash (generated by web/build_web.py) - the browser
        // unpacks. ?v= in the address changes with the content, so it may cache them permanently.

        struct GzAsset {
            const char* path;
            const char* type;
            const uint8_t* data;
            size_t len;
        };
        static const GzAsset gzAssets[] = {
            { "/designer.js", "application/javascript; charset=utf-8", DESIGNER_COMMON_JS_GZ, sizeof(DESIGNER_COMMON_JS_GZ) },
            { "/facedesigner.css", "text/css; charset=utf-8", FACE_DESIGNER_CSS_GZ, sizeof(FACE_DESIGNER_CSS_GZ) },
            { "/facedesigner.js", "application/javascript; charset=utf-8", FACE_DESIGNER_JS_GZ, sizeof(FACE_DESIGNER_JS_GZ) },
            { "/handdesigner.css", "text/css; charset=utf-8", HAND_DESIGNER_CSS_GZ, sizeof(HAND_DESIGNER_CSS_GZ) },
            { "/handdesigner.js", "application/javascript; charset=utf-8", HAND_DESIGNER_JS_GZ, sizeof(HAND_DESIGNER_JS_GZ) },
        };
        for (const GzAsset& asset : gzAssets) {
            const GzAsset* a = &asset;
            webserver.on(a->path, HTTP_GET, [a]() {
                webserver.sendHeader("Cache-Control", "public, max-age=31536000, immutable");
                webserver.sendHeader("Content-Encoding", "gzip");
                webserver.send_P(200, a->type, (const char*)a->data, a->len);
                });
        }

        // Zeiger-Designer: gemeinsamer Seitenkopf + Markup aus dem Flash (HAND_DESIGNER_HTML in hand_designer_html.h),
        // das CSS und Skript von /handdesigner.css und .js nachlaedt.

        // Hand designer: shared page header + markup from flash (HAND_DESIGNER_HTML in hand_designer_html.h), which
        // loads CSS and script from /handdesigner.css and .js.

        webserver.on("/handdesigner", HTTP_GET, []() {
            webserver.setContentLength(CONTENT_LENGTH_UNKNOWN);
            webserver.send(200, "text/html", "");

            String chunk = beginPage();
            chunk += "<h2>" + translate("Hand Designer") + " " + String(HAND_WIDTH) + " x " + String(HAND_HEIGHT) + "</h2>";

            // Satz-IDs wie in /handsets ermitteln (numerisch sortiert) - nur
            // Zeichen uebernehmen, die in einem JS-String unkritisch sind.

            // Determine set IDs like /handsets does (sorted numerically) -
            // only keep characters that are harmless inside a JS string.

            auto jsSafe = [](const String& s) {
                String out;
                for (size_t i = 0; i < s.length(); i++) {
                    char c = s[i];
                    if (isalnum((unsigned char)c) || c == '-' || c == '.' || c == '/' || c == '_' || c == '!') out += c;
                }
                return out;
            };

            std::map<int, String> numericSets;
            std::vector<String> otherSets;
            std::set<String> seenSetIds;
            ensureDefaultHands();
            File root = LittleFS.open("/");
            File file = root.openNextFile();
            while (file) {
                String name = file.name();
                if (!file.isDirectory() && name.startsWith("hand_set") && name.endsWith(".bmp")) {
                    int end = name.indexOf('_', 8);
                    if (end > 8) {
                        String setIdStr = jsSafe(name.substring(8, end));
                        if (setIdStr.length() > 0 && seenSetIds.insert(setIdStr).second) {
                            bool isNumeric = true;
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

            String setsJs;
            for (auto& entry : numericSets) setsJs += (setsJs.length() ? ",'" : "'") + entry.second + "'";
            for (auto& id : otherSets) setsJs += (setsJs.length() ? ",'" : "'") + id + "'";

            char hubHex[8];
            snprintf(hubHex, sizeof(hubHex), "#%02x%02x%02x",
                     ((hubColor >> 11) & 0x1F) * 255 / 31, ((hubColor >> 5) & 0x3F) * 255 / 63, (hubColor & 0x1F) * 255 / 31);

            // Zeigerstil wie in /preview, damit die Vorschau sich wie die Uhr bewegt
            // Hand style like in /preview, so the preview moves like the clock

            bool modeStation = preferences.getBool(PK_STATION_MODE, true);
            String modeJs = ",mode:{station:" + String(modeStation ? "true" : "false") +
                            ",smoothMin:" + String(preferences.getBool(PK_SMOOTH_MINUTE, false) ? "true" : "false") +
                            ",smoothSec:" + String(getSmoothSecondPref(modeStation) ? "true" : "false") +
                            ",fastMs:" + String((int)FAST_SECOND) + "}";

            // Streifen (ILI9341) nur, wenn das aktive Zifferblatt eine Streifen-Grafik hat - wie in der Uebersicht
            // Strip (ILI9341) only if the active clock face has a strip graphic - as in the overview

            String stripPath = stripPathForFace(selectedBackground);
            if (TFT_HEIGHT > CLOCK_HEIGHT && stripPath.length() && LittleFS.exists(stripPath)) {
                modeJs += ",strip:{w:" + String(TFT_WIDTH) + ",h:" + String(TFT_HEIGHT - CLOCK_HEIGHT) + ",before:" + String(stripBefore ? "true" : "false") +
                          ",sec:" + String(stripShowsSeconds() ? "true" : "false") + "}";
            }

            ensureFaceSettings();
            chunk += "<script>var HD={w:" + String(HAND_WIDTH) + ",h:" + String(HAND_HEIGHT) + ",lh:" + String(HAND_LEGACY_HEIGHT) + ",lw:" + String(HAND_LEGACY_WIDTH) +
                     ",px:" + String(HAND_WIDTH / 2) + ",py:" + String(HAND_PIVOT_Y) +
                     ",cw:" + String(CLOCK_WIDTH) + ",active:'" + jsSafe(handSetFileId(preferences.getString(PK_HANDSET, ""))) +
                     "',face:'" + jsSafe(selectedBackground) + "',hub:" + String(hubSize) + ",hubColor:'" + String(hubHex) +
                     "',sec:[" + String(secPivotX) + "," + String(secPivotY) + "],prot:[" + protectedSetsJs() + "]" +
                     ",lang:'" + jsSafe(currentLanguage) + "',sets:[" + setsJs + "]" +
                     ",widths:{hour:" + String(hourHandWidth) + ",minute:" + String(minuteHandWidth) + ",second:" + String(secondHandWidth) + "}" + modeJs + "};</script>";
            webserver.sendContent(chunk);
            webserver.sendContent_P(HAND_DESIGNER_HTML);
            webserver.sendContent("</body></html>");
            webserver.sendContent("");
            });

        // Zifferblatt-Designer: gemeinsamer Seitenkopf + Markup aus dem Flash (FACE_DESIGNER_HTML in
        // face_designer_html.h), das CSS und Skript von /facedesigner.css und .js nachlaedt.

        // Clock face designer: shared page header + markup from flash (FACE_DESIGNER_HTML in face_designer_html.h),
        // which loads CSS and script from /facedesigner.css and .js.

        webserver.on("/facedesigner", HTTP_GET, []() {
            webserver.setContentLength(CONTENT_LENGTH_UNKNOWN);
            webserver.send(200, "text/html", "");

            String chunk = beginPage();
            chunk += "<h2>" + translate("Clock Face Designer") + " " + String(TFT_WIDTH) + " x " + String(TFT_HEIGHT) + "</h2>"; // mit Streifen das ganze Display
                                                                                                                                 // with a strip the whole display

            // Nur Zeichen uebernehmen, die in einem JS-String unkritisch sind
            // Only keep characters that are harmless inside a JS string

            auto jsSafe = [](const String& s) {
                String out;
                for (size_t i = 0; i < s.length(); i++) {
                    char c = s[i];
                    if (isalnum((unsigned char)c) || c == '-' || c == '.' || c == '/' || c == '_' || c == '!') out += c;
                }
                return out;
            };

            String facesJs;
            File root = LittleFS.open("/");
            File file = root.openNextFile();
            while (file) {
                String name = file.name();
                if (!file.isDirectory() && name.startsWith("face_") && name.endsWith(".bmp")) {
                    facesJs += (facesJs.length() ? ",'" : "'") + jsSafe(name) + "'";
                }
                file = root.openNextFile();
            }

            // Gespeicherte Nabenfarbe (RGB888), solange sie zur angezeigten passt - sonst aus RGB565 zurueckgerechnet
            // Stored hub colour (RGB888) as long as it matches the displayed one - otherwise converted back from RGB565

            uint32_t hubRgb = (uint32_t)preferences.getLong(PK_CENTER_COLOR, 0xEC0016) & 0xFFFFFF;
            if (tft.color565((hubRgb >> 16) & 0xFF, (hubRgb >> 8) & 0xFF, hubRgb & 0xFF) != hubColor) {
                hubRgb = ((uint32_t)(((hubColor >> 11) & 0x1F) * 255 / 31) << 16) | ((uint32_t)(((hubColor >> 5) & 0x3F) * 255 / 63) << 8) | ((hubColor & 0x1F) * 255 / 31);
            }
            char hubHex[8];
            snprintf(hubHex, sizeof(hubHex), "#%06lx", (unsigned long)hubRgb);

            const char* roundJs = displayGeom->round ? "true" : "false";

            // Streifen fuer Uhrzeit/Datum (nur Displays groesser als die Uhr, ILI9341) - Masse hochkant
            // Time/date strip (only displays larger than the clock, ILI9341) - portrait dimensions

            // Hochgeladene Schriften (font_*) fuer Generator, Text-Werkzeug und Streifen
            // Uploaded fonts (font_*) for the generator, text tool and strip

            String fontsJs = "";
            {
                File fr = LittleFS.open("/");
                File ff = fr ? fr.openNextFile() : File();
                while (ff) {
                    String fn = ff.name();
                    if (fn.startsWith("/")) fn = fn.substring(1);
                    if (!ff.isDirectory() && fn.startsWith("font_")) fontsJs += String(fontsJs.length() ? "," : "") + "'" + jsSafe(fn) + "'";
                    ff = fr.openNextFile();
                }
            }

            // Name der VLW-Schrift mit Leerzeichen (z.B. "Sans B" = fett) - jsSafe() wuerde sie entfernen
            // Name of the VLW font with spaces (e.g. "Sans B" = bold) - jsSafe() would remove them

            String vlwNameJs;
            for (size_t i = 0; i < stripVlwName.length(); i++) {
                char c = stripVlwName[i];
                if (isalnum((unsigned char)c) || c == ' ' || c == '-' || c == '_' || c == '.') vlwNameJs += c;
            }

            String stripJs = "null";
            ensureStripSettings();
            if (TFT_HEIGHT > CLOCK_HEIGHT) {
                char bgHex[8], fgHex[8];
                snprintf(bgHex, sizeof(bgHex), "#%06lx", (unsigned long)stripBgRgb);
                snprintf(fgHex, sizeof(fgHex), "#%06lx", (unsigned long)stripFgRgb);
                stripJs = "{w:" + String(TFT_WIDTH) + ",h:" + String(TFT_HEIGHT - CLOCK_HEIGHT) + ",bg:'" + String(bgHex) +
                          "',fg:'" + String(fgHex) + "',font:" + String(stripFont) + ",before:" + String(stripBefore ? 1 : 0) +
                          ",blink:" + String(stripBlink ? 1 : 0) + ",tfmt:" + String(stripTimeFmt) + ",sec:" + String(stripSeconds) + ",dfmt:" + String(stripDateFmt) +
                          ",vlw:'" + vlwNameJs + "',vt:" + String(stripVlwTimeSize) + ",vd:" + String(stripVlwDateSize) +
                          ",ts:" + String(stripTimeScale) + ",ds:" + String(stripDateScale) +
                          ",st:" + String(stripShowTime ? 1 : 0) + ",sd:" + String(stripShowDate ? 1 : 0) +
                          ",sw:" + String(stripShowWeekday ? 1 : 0) + ",ws:" + String(stripWeekdayScale) +
                          ",vw:" + String(stripVlwWeekdaySize) + ",wx:" + String(stripWeekdayX) + ",wy:" + String(stripWeekdayY) +
                          ",awy:" + String(stripAutoWeekdayY) +
                          ",vwok:" + String(LittleFS.exists(stripVlwPath(stripVlwWeekdaySize, true)) ? 1 : 0) +
                          ",tx:" + String(stripTimeX) +
                          ",ty:" + String(stripTimeY) + ",dx:" + String(stripDateX) + ",dy:" + String(stripDateY) +
                          ",aty:" + String(stripAutoTimeY) + ",ady:" + String(stripAutoDateY) + ",fonts:[";

                // Feste Namen aus der Firmware - ohne jsSafe(), das auch Leerzeichen entfernen wuerde
                // Fixed names from the firmware - without jsSafe(), which would also strip spaces

                for (uint8_t i = 0; i < STRIP_FONT_COUNT; i++) stripJs += String(i ? "," : "") + "'" + String(STRIP_FONTS[i].name) + "'";
                stripJs += "]}";
            }
            long freeBytes = (long)LittleFS.totalBytes() - (long)LittleFS.usedBytes();

            // Zeigerstil wie in /preview, damit die Vorschau sich wie die Uhr bewegt
            // Hand style like in /preview, so the preview moves like the clock

            bool modeStation = preferences.getBool(PK_STATION_MODE, true);
            String modeJs = ",mode:{station:" + String(modeStation ? "true" : "false") +
                            ",smoothMin:" + String(preferences.getBool(PK_SMOOTH_MINUTE, false) ? "true" : "false") +
                            ",smoothSec:" + String(getSmoothSecondPref(modeStation) ? "true" : "false") +
                            ",fastMs:" + String((int)FAST_SECOND) + "}";

            ensureFaceSettings();
            chunk += "<script>var FD={w:" + String(CLOCK_WIDTH) + ",h:" + String(CLOCK_HEIGHT) + ",round:" + String(roundJs) +
                     ",active:'" + jsSafe(selectedBackground) + "',faces:[" + facesJs + "],free:" + String(freeBytes) +
                     ",hand:{w:" + String(HAND_WIDTH) + ",h:" + String(HAND_HEIGHT) + ",lh:" + String(HAND_LEGACY_HEIGHT) + ",lw:" + String(HAND_LEGACY_WIDTH) + ",py:" + String(HAND_PIVOT_Y) +
                     ",set:'" + jsSafe(handSetFileId(preferences.getString(PK_HANDSET, ""))) + "'" +
                     ",widths:{hour:" + String(hourHandWidth) + ",minute:" + String(minuteHandWidth) + ",second:" + String(secondHandWidth) + "}}" +
                     ",hub:" + String(hubSize) + ",hubColor:'" + String(hubHex) + "',showSec:" + String(showSecondHand ? "true" : "false") +
                     ",sec:[" + String(secPivotX) + "," + String(secPivotY) + "],prot:[" + protectedFacesJs() + "]" +
                     ",lang:'" + jsSafe(currentLanguage) + "'" + modeJs + ",strip:" + stripJs + ",fonts:[" + fontsJs + "]};</script>";
            webserver.sendContent(chunk);
            webserver.sendContent_P(FACE_DESIGNER_HTML);
            webserver.sendContent("</body></html>");
            webserver.sendContent("");
            });

        // Handset löschen
        // Delete hand set

        webserver.on("/deletehandset", HTTP_GET, []() {

            // Nur aus einem privaten Netz erlaubt (siehe isPrivateNetworkIp() oben).
            // Only allowed from a private network (see isPrivateNetworkIp() above).

            if (!isPrivateNetworkIp(webserver.client().remoteIP())) {
                webserver.send(200, "text/html", simpleMessagePage(translate("Delete"), "<p>" + translate("This action is only available when accessing the clock from a private network") + ".</p>"));
                return;
            }

            if (webserver.hasArg("set") && isProtectedHandSetId(webserver.arg("set"))) {
                redirectTo("/handsets?err=Built-in%20file%20cannot%20be%20deleted");
            }
            else if (webserver.hasArg("set")) {
                String setId = webserver.arg("set");
                String targets[] = { "hour", "minute", "second" };
                for (const String& target : targets) {
                    String path = "/hand_set" + setId + "_" + target + ".bmp";
                    DEBUG_PRINTLN("[DELETE] Looking for: " + path + " (from " + webserver.client().remoteIP().toString() + ")");
                    if (LittleFS.exists(path)) {
                        LittleFS.remove(path);
                        DEBUG_PRINTLN("[DELETE] Removed: " + path + " (from " + webserver.client().remoteIP().toString() + ")");
                    }
                }
                removeOrphanedPresets("", setId);

                // Falls der geloeschte Zeigersatz gerade aktiv war, sofort auf den Standardsatz zurueckschalten
                // (der Standardsatz 0 selbst wird dabei neu erzeugt).

                // If the deleted hand set was currently active, switch back to the default set immediately (the
                // default set 0 itself is created again in the process).

                if (handSetFileId(preferences.getString(PK_HANDSET, "")) == setId) {
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

        // Streifen fuer Uhrzeit/Datum aus dem Zifferblatt-Designer: Farben (#rrggbb), Schriftart, Lage (before=1:
        // ueber bzw. links von der Uhr) und Positionen (-1 = automatisch). save=0 zeigt die Werte nur sofort an,
        // save=1 speichert sie zusaetzlich.

        // Time/date strip from the clock face designer: colors (#rrggbb), font, placement (before=1: above or left
        // of the clock) and positions (-1 = automatic). save=0 only shows the values right away, save=1 also
        // stores them.

        webserver.on("/save_strip", HTTP_POST, []() {
            if (!isPrivateNetworkIp(webserver.client().remoteIP())) {
                webserver.send(403, "application/json", "{\"ok\":false}");
                return;
            }
            auto colorArg = [](const char* name, uint32_t current) -> uint32_t {
                String v = webserver.arg(name);
                if (v.length() != 7 || v[0] != '#') return current;
                return strtoul(v.c_str() + 1, nullptr, 16) & 0xFFFFFF;
            };
            auto posArg = [](const char* name, int16_t current, int maxValue) -> int16_t {
                if (!webserver.hasArg(name)) return current;
                long v = webserver.arg(name).toInt();
                if (v < 0) return -1;
                return (int16_t)min(v, (long)maxValue);
            };
            int stripW = TFT_WIDTH;
            int stripH = max(0, TFT_HEIGHT - CLOCK_HEIGHT);
            ensureStripSettings();
            stripBgRgb = colorArg("bg", stripBgRgb);
            stripFgRgb = colorArg("fg", stripFgRgb);
            if (webserver.hasArg("font")) {
                long f = webserver.arg("font").toInt();
                if ((f >= 0 && f < STRIP_FONT_COUNT) || f == STRIP_FONT_VLW) stripFont = (uint8_t)f;
            }
            if (webserver.hasArg("tfmt")) stripTimeFmt = (uint8_t)constrain(webserver.arg("tfmt").toInt(), 0L, 2L);
            if (webserver.hasArg("sec")) stripSeconds = (uint8_t)constrain(webserver.arg("sec").toInt(), 0L, 2L);
            if (webserver.hasArg("dfmt")) stripDateFmt = (uint8_t)constrain(webserver.arg("dfmt").toInt(), 0L, (long)STRIP_DATE_FMT_COUNT - 1);
            if (webserver.hasArg("vlw")) stripVlwName = webserver.arg("vlw").substring(0, 40);
            if (webserver.hasArg("vt")) stripVlwTimeSize = (uint8_t)constrain(webserver.arg("vt").toInt(), 8L, 120L);
            if (webserver.hasArg("vd")) stripVlwDateSize = (uint8_t)constrain(webserver.arg("vd").toInt(), 8L, 120L);
            if (webserver.hasArg("ts")) stripTimeScale = (uint8_t)constrain(webserver.arg("ts").toInt(), STRIP_SCALE_MIN, STRIP_SCALE_MAX);
            if (webserver.hasArg("ds")) stripDateScale = (uint8_t)constrain(webserver.arg("ds").toInt(), STRIP_SCALE_MIN, STRIP_SCALE_MAX);
            if (webserver.hasArg("st")) stripShowTime = webserver.arg("st") == "1";
            if (webserver.hasArg("sd")) stripShowDate = webserver.arg("sd") == "1";
            if (webserver.hasArg("sw")) stripShowWeekday = webserver.arg("sw") == "1";
            if (webserver.hasArg("ws")) stripWeekdayScale = (uint8_t)constrain(webserver.arg("ws").toInt(), STRIP_SCALE_MIN, STRIP_SCALE_MAX);
            if (webserver.hasArg("vw")) stripVlwWeekdaySize = (uint8_t)constrain(webserver.arg("vw").toInt(), 8L, 120L);
            stripTimeX = posArg("tx", stripTimeX, stripW);
            stripTimeY = posArg("ty", stripTimeY, stripH);
            stripDateX = posArg("dx", stripDateX, stripW);
            stripDateY = posArg("dy", stripDateY, stripH);
            stripWeekdayX = posArg("wx", stripWeekdayX, stripW);
            stripWeekdayY = posArg("wy", stripWeekdayY, stripH);

            // Neue Lage: beide Displays einmal loeschen, danach Uhr und Streifen komplett neu senden
            // New placement: clear both displays once, then resend the clock and the strip completely

            if (webserver.hasArg("before")) setStripBefore(webserver.arg("before") == "1");
            if (webserver.hasArg("blink")) stripBlink = webserver.arg("blink") == "1";

            // Speichern zum aktiven Zifferblatt oder zu face (der Designer vor dem Aktivieren eines neuen Zifferblatts)
            // Store for the active clock face or for face (the designer before activating a new clock face)

            if (webserver.arg("save") == "1") {
                String face = webserver.hasArg("face") ? "/" + webserver.arg("face") : selectedBackground;
                if (!saveStripSettingsForFace(face)) {
                    webserver.send(400, "application/json", "{\"ok\":false}");
                    return;
                }
            }
            infoStripDirty[0] = infoStripDirty[1] = true;
            drawInfoStrips();
            setCSIdle();
            webserver.send(200, "application/json", "{\"ok\":true,\"aty\":" + String(stripAutoTimeY) + ",\"ady\":" + String(stripAutoDateY) + ",\"awy\":" + String(stripAutoWeekdayY) + "}");
            });

        // Vorschau des Streifens: hochkant, ungedimmt, RGB565 big-endian (w x h aus FD.strip) - so wie die Uhr ihn
        // zeichnet. text=1: nur die Schrift auf TRANSPARENT_COLOR, text=2: auf Schwarz und Weiss (Designer).
        // h/m/s: feste Uhrzeit wie die Zeiger der Designer-Vorschau ohne Live-Uhrzeit.

        // Strip preview: portrait, undimmed, RGB565 big-endian (w x h from FD.strip) - exactly as the clock draws
        // it. text=1: only the text on TRANSPARENT_COLOR, text=2: on black and white (designer). h/m/s: fixed time like
        // the hands of the designer preview without live time.

        webserver.on("/api/stripimg", HTTP_GET, []() {
            int w = TFT_WIDTH, h = TFT_HEIGHT - CLOCK_HEIGHT;
            ensureStripSettings();
            struct tm fixed = timeinfo;
            bool useFixed = webserver.hasArg("h");
            if (useFixed) {
                fixed.tm_hour = constrain(webserver.arg("h").toInt(), 0, 23);
                fixed.tm_min = constrain(webserver.arg("m").toInt(), 0, 59);
                fixed.tm_sec = constrain(webserver.arg("s").toInt(), 0, 59);
            }
            // text=2: nur die Schrift, einmal auf Schwarz und einmal auf Weiss hintereinander - aus dem Unterschied
            // berechnet der Designer die Deckkraft jedes Pixels, kantengeglaettete Schrift liegt so sauber ueber
            // seiner Zeichnung

            // text=2: only the text, once on black and once on white one after the other - from the difference
            // the designer computes each pixel's opacity, so anti-aliased text lies cleanly over its drawing

            const bool twoPass = webserver.arg("text") == "2";
            const bool textOnly = twoPass || webserver.arg("text") == "1";
            const struct tm* when = useFixed ? &fixed : nullptr;
            if (!renderStripPreview(w, h, textOnly, when, twoPass ? 0x0000 : TRANSPARENT_COLOR)) {
                webserver.send(404, "text/plain", "no strip");
                return;
            }
            size_t n = (size_t)w * h * 2;
            webserver.setContentLength(twoPass ? 2 * n : n);
            webserver.send(200, "application/octet-stream", "");
            webserver.sendContent((const char*)infoStripSprite.getBuffer(), n);
            if (twoPass && renderStripPreview(w, h, true, when, 0xFFFF)) {
                webserver.sendContent((const char*)infoStripSprite.getBuffer(), n);
            }
            });

        // Displaytyp speichern ("display" wie parseDisplayName()) und neu starten, da Puffer und Sprites nur
        // beim Start angelegt werden. Gleiche Groesse (GC9A01 ohne/mit BL): nur die Backlight-Regelung
        // umschalten, ohne Neustart.

        // Save the display type ("display" as in parseDisplayName()) and restart, since buffers and sprites
        // are only created at boot. Same size (GC9A01 without/with BL): only switch the backlight control,
        // without a restart.

        webserver.on("/save_displaytype", HTTP_POST, []() {
            if (!isPrivateNetworkIp(webserver.client().remoteIP())) {
                webserver.send(200, "text/html", simpleMessagePage(translate("Display type"), "<p>" + translate("This action is only available when accessing the clock from a private network") + ".</p>"));
                return;
            }
            uint8_t newType;
            bool newBacklight;
            if (!parseDisplayName(webserver.arg("display"), newType, newBacklight)) {
                redirectTo("/?tab=zifferblatt");
                return;
            }
            if (newType == displayType) {
                setBacklightMode(newBacklight);
                redirectTo("/?tab=zifferblatt&msg=Settings%20saved");
                return;
            }
            setDisplayType(newType, newBacklight);
            webserver.send(200, "text/html", simpleMessagePage(translate("Rebooting..."), "<p>" + translate("Return to the main page in 10 seconds or refresh the website when the ESP is online again") + ".</p>", "<meta http-equiv='refresh' content='10; url=/'>"));
            espReboot();
            });

        webserver.on("/reboot", HTTP_GET, []() {

            // Nur aus einem privaten Netz erlaubt - siehe Begruendung bei
            // /api/reboot weiter oben.

            // Only allowed from a private network - see the reasoning at
            // /api/reboot further above.

            if (!isPrivateNetworkIp(webserver.client().remoteIP())) {
                webserver.send(200, "text/html", simpleMessagePage(translate("Rebooting..."), "<p>" + translate("This action is only available when accessing the clock from a private network") + ".</p>"));
                return;
            }

            webserver.send(200, "text/html", simpleMessagePage(translate("Rebooting..."), "<p>" + translate("Return to the main page in 10 seconds or refresh the website when the ESP is online again") + ".</p>", "<meta http-equiv='refresh' content='10; url=/'>"));
            espReboot();
            });

        // Werkseinstellungen: Uebersicht mit einzeln bestaetigten Reset-Optionen. Komplettsicherung
        // (backup.h): Seite, Download und Wiederherstellung nur aus einem privaten Netz - sie kann
        // WLAN-Passwoerter enthalten.

        // Factory reset: overview with individually confirmed reset options. Full backup (backup.h): page,
        // download and restore only from a private network - it may contain WiFi passwords.

        webserver.on("/backup", HTTP_GET, []() {
            String html = beginPage();
            html += generateFlashMessage();
            html += "<h2>" + translate("Backup") + "</h2>";
            if (!isPrivateNetworkIp(webserver.client().remoteIP())) {
                html += "<p>" + translate("This action is only available when accessing the clock from a private network") + ".</p></body></html>";
                webserver.send(200, "text/html", html);
                return;
            }

            html += "<h3>" + translate("Create Backup") + "</h3>";
            html += "<p>" + translate("Saves all settings, presets, clock faces and hand sets in one file") + ".</p>";
            html += "<p><small>" + translate("For troubleshooting, the backup also contains the status page and all log files - they are not restored") + ".</small></p>";

            // POST statt GET: keine Sicherung per Link/Vorabruf ausloesbar. Der
            // Warnhinweis erscheint mit dem WLAN-Haken - die Daten sind nur mit
            // dem internen Firmware-Schluessel verschluesselt (BACKUP_WIFI_KEY).

            // POST instead of GET: no backup can be triggered via a link/
            // prefetch. The warning appears with the WiFi box - the data is only
            // encrypted with the internal firmware key (BACKUP_WIFI_KEY).

            html += "<form method='POST' action='/backup/download' id='backupForm'>";
            html += "<label><input type='checkbox' name='wifi' value='1' style='width:auto;margin:0 6px 0 0;' onchange=\"document.getElementById('bkWifiWarn').hidden=!this.checked;\">" + translate("Include WiFi credentials (network names, passwords, hostname)") + "</label>";
            html += "<div id='bkWifiWarn' class='msg warn' hidden>" + translate("Saving the WiFi credentials is not secure: they are encrypted in the file, but with a key that is the same in every uhr4 firmware - anyone with the firmware or its source code can decrypt them. Keep the file safe and do not pass it on") + ".</div>";
            html += "<button type='submit'>" + translate("Download Backup") + "</button></form>";
            html += "<div id='backupFormP' hidden><progress max='100' style='width:100%;'></progress><small></small></div><hr>";

            // Reihenfolge wichtig: die WLAN-Option VOR dem Dateifeld - nur so
            // liegt sie beim Upload-Start vor und wird geprueft, bevor
            // irgendetwas ueberschrieben wird (backup.h).

            // Order matters: the WiFi option BEFORE the file field - only then
            // is it available at upload start and checked before anything gets
            // overwritten (backup.h).

            html += "<h3>" + translate("Restore Backup") + "</h3>";
            html += "<p>" + translate("Takes over the settings of the backup and adds its presets, clock faces and hand sets - existing ones stay, same-named ones are replaced by the backup. The clock restarts afterwards") + ".</p>";
            html += "<p><small>" + translate("Display type, rotation, backlight and light sensor of this clock stay unchanged. From a clock with another display type only clock faces, hands, presets and general settings are restored, scaled to this clock - its clock faces must be 240x240") + ". " + translate("This clock") + ": " + String(displayChoiceName(displayType, useBacklight)) + ".</small></p>";
            html += "<form method='POST' action='/backup/restore' enctype='multipart/form-data' id='restoreForm' data-ask='" + translate("Take over the settings of the backup? Existing clock faces, hand sets and presets stay, same-named ones are replaced.") + "'>";
            html += "<label><input type='checkbox' name='restoreWifi' value='1' style='width:auto;margin:0 6px 0 0;'>" + translate("Restore WiFi credentials (network names, passwords, hostname)") + "</label>";
            html += "<p><small>" + translate("Without this option the clock keeps its own WiFi and hostname - recommended when transferring the settings to another clock") + ".</small></p>";
            html += "<input type='file' name='backupfile' accept='.tar' required> ";
            html += "<button type='submit'>" + translate("Restore Backup") + "</button></form>";
            html += "<div id='restoreFormP' hidden><progress max='100' style='width:100%;'></progress><small></small></div>";

            // Vor dem Hochladen den Displaytyp der Sicherung pruefen: aus settings.txt (erster Eintrag im TAR) und aus
            // dem Dateinamen. Weicht er ab, warnt die Seite - passt die Uhrgroesse nicht, lehnt die Uhr ohnehin ab.

            // Check the backup's display type before uploading: from settings.txt (first entry in the TAR) and from
            // the file name. If it differs, the page warns - if the clock size does not fit, the clock rejects it anyway.

            String names = "", namesBl = "", clocks = "", blDefaults = "";
            for (uint8_t i = 0; i < DISPLAY_TYPE_COUNT; i++) {
                String sep = i ? "," : "";
                names += sep + "'" + displayChoiceName(i, false) + "'";
                namesBl += sep + "'" + displayChoiceName(i, true) + "'";
                clocks += sep + String(DISPLAY_GEOMETRY[i].clock);
                blDefaults += sep + String(DISPLAY_GEOMETRY[i].backlightDefault ? 1 : 0);
            }
            html += "<div id='restoreTexts' hidden data-from='" + translate("This backup is from a {b} clock, this clock is a {c}") +
                    "' data-fits='" + translate("Clock faces, hands, presets and general settings are restored (scaled to this clock, also the hub size); display type, rotation, backlight and brightness of this clock stay unchanged") +
                    "' data-size='" + translate("Its clock faces are not 240x240 - the clock will not restore anything") +
                    "' data-fname='" + translate("The file name says {n}") + "' data-anyway='" + translate("Restore anyway?") +
                    "' data-dl='" + translate("Creating backup - the clock is busy, please wait") +
                    "' data-dldone='" + translate("Backup saved") +
                    "' data-up='" + translate("Sending backup - the clock restores it while receiving, keep this page open") +
                    "' data-busy='" + translate("Applying settings") +
                    "' data-wait='" + translate("Backup restored - waiting for the clock to restart") +
                    "' data-timeout='" + translate("The clock does not respond yet - refresh the page later") +
                    "' data-err='" + translate("Connection to the clock interrupted") + "'></div>";
            html += "<script>(function(){";
            html += "var names=[" + names + "],namesBl=[" + namesBl + "],clocks=[" + clocks + "],blDef=[" + blDefaults + "];";
            html += "var myType=" + String(displayType) + ",myName='" + String(displayChoiceName(displayType, useBacklight)) + "';";
            html += "var f=document.getElementById('restoreForm'),T=document.getElementById('restoreTexts').dataset;";
            html += "function readSettings(file){return file.slice(0,65536).arrayBuffer().then(function(ab){var b=new Uint8Array(ab);if(b.length<512)return null;";
            html += "var name=new TextDecoder().decode(b.subarray(0,100)).replace(/\\0.*$/,'');if(name!=='" BACKUP_SETTINGS_NAME "')return null;";
            html += "var size=parseInt(new TextDecoder().decode(b.subarray(124,136)).replace(/[^0-7]/g,''),8)||0;";
            html += "return new TextDecoder().decode(b.subarray(512,512+Math.min(size,b.length-512)));}).catch(function(){return null;});}";
            html += "f.addEventListener('submit',function(ev){ev.preventDefault();var file=f.backupfile.files[0];if(!file)return;";
            html += "readSettings(file).then(function(txt){var warn=[];";
            html += "var all=names.concat(namesBl).filter(function(n,i,a){return a.indexOf(n)===i;}).sort(function(a,b){return b.length-a.length;});";
            html += "var m=new RegExp('-('+all.join('|')+')(-\\\\d{8}(-\\\\d{6})?)?(\\\\s*\\\\(\\\\d+\\\\))?\\\\.tar$','i').exec(file.name),fromName=m?m[1].toUpperCase():null;";
            html += "if(txt){var t=/^u8\\tdisplayType\\t(\\d+)/m.exec(txt),bl=/^u8\\tuseBacklight\\t(\\d+)/m.exec(txt);";
            html += "var type=t?+t[1]:0;if(!(type>=0&&type<names.length))type=0;var useBl=bl?+bl[1]!==0:blDef[type]===1;var bName=useBl?namesBl[type]:names[type];";
            html += "if(type!==myType){warn.push(T.from.replace('{b}',bName).replace('{c}',myName)+'.');warn.push((clocks[type]===240?T.fits:T.size)+'.');}";
            html += "if(fromName&&fromName!==bName)warn.push(T.fname.replace('{n}',fromName)+'.');}";
            html += "else if(fromName&&fromName!==myName)warn.push(T.from.replace('{b}',fromName).replace('{c}',myName)+'.');";
            html += "if(confirm(warn.length?warn.join('\\n')+'\\n\\n'+T.anyway:f.dataset.ask))upload();});});";

            // Fortschritt statt stummer Wartezeit: Sichern per fetch (Laenge bekannt), Wiederherstellen per
            // XMLHttpRequest (Upload-Fortschritt, die Uhr verarbeitet beim Empfang), danach warten, bis die Uhr
            // nach dem Neustart wieder antwortet. Die Statusleiste pausiert solange.

            // Progress instead of a silent wait: backup via fetch (length known), restore via XMLHttpRequest
            // (upload progress, the clock processes while receiving), then wait until the clock answers again
            // after the restart. The status bar pauses meanwhile.

            html += "function pause(p){if(window.topbarPolling)window.topbarPolling(!p);}";
            html += "function kb(n){return Math.round(n/1024)+' KB';}";
            html += "function show(id,txt,pct){var b=document.getElementById(id),p=b.querySelector('progress');b.hidden=false;b.querySelector('small').textContent=txt;if(pct==null)p.removeAttribute('value');else p.value=pct;}";
            html += "var d=document.getElementById('backupForm');";
            html += "d.addEventListener('submit',function(ev){ev.preventDefault();var btn=d.querySelector('button');btn.disabled=true;pause(true);show('backupFormP',T.dl,null);";
            html += "fetch('/backup/download',{method:'POST',body:new URLSearchParams(new FormData(d))}).then(function(r){if(!r.ok)throw new Error('HTTP '+r.status);";
            html += "var total=+r.headers.get('Content-Length')||0,got=0,parts=[],rd=r.body.getReader();";
            html += "var cd=/filename=\"?([^\";]+)/.exec(r.headers.get('Content-Disposition')||''),name=cd?cd[1]:'uhr4-backup.tar';";
            html += "function step(){return rd.read().then(function(x){if(x.done)return;parts.push(x.value);got+=x.value.length;show('backupFormP',T.dl+' ('+kb(got)+(total?' / '+kb(total):'')+')',total?100*got/total:null);return step();});}";
            html += "return step().then(function(){if(total&&got<total)throw new Error(kb(got)+' / '+kb(total));";
            html += "var a=document.createElement('a');a.href=URL.createObjectURL(new Blob(parts,{type:'application/x-tar'}));a.download=name;document.body.appendChild(a);a.click();";
            html += "setTimeout(function(){URL.revokeObjectURL(a.href);a.remove();},2000);show('backupFormP',T.dldone+': '+name,100);});";
            html += "}).catch(function(e){show('backupFormP',T.err+' ('+e.message+')',0);}).then(function(){btn.disabled=false;pause(false);});});";
            html += "function waitRestart(){var t0=Date.now();function tryIt(){fetch('/api/topbarStatus',{cache:'no-store'}).then(function(r){if(!r.ok)throw 0;location.href='/';})";
            html += ".catch(function(){if(Date.now()-t0>120000){show('restoreFormP',T.timeout,0);}else{setTimeout(tryIt,2000);}});}setTimeout(tryIt,3000);}";
            html += "function upload(){var btn=f.querySelector('button');btn.disabled=true;pause(true);show('restoreFormP',T.up,0);var x=new XMLHttpRequest();x.open('POST','/backup/restore');";
            html += "x.upload.onprogress=function(e){if(e.lengthComputable)show('restoreFormP',T.up+' ('+kb(e.loaded)+' / '+kb(e.total)+')',100*e.loaded/e.total);};";
            html += "x.upload.onload=function(){show('restoreFormP',T.busy,null);};";
            html += "x.onload=function(){if(x.status!==200){document.open();document.write(x.responseText);document.close();return;}show('restoreFormP',T.wait,null);waitRestart();};";
            html += "x.onerror=function(){show('restoreFormP',T.err,0);btn.disabled=false;pause(false);};";
            html += "x.send(new FormData(f));}";
            html += "})();</script>";

#if HAS_OTA

            // Firmware-Update ueber WLAN (ota_update.h): Upload per XMLHttpRequest mit Fortschritt, danach warten,
            // bis die Uhr mit der neuen Firmware wieder antwortet. Nur uhr4.ino.bin aus esp32s3.

            // Firmware update over WiFi (ota_update.h): upload via XMLHttpRequest with progress, then wait until
            // the clock answers again with the new firmware. Only uhr4.ino.bin from esp32s3.

            html += "<hr><h3>" + translate("Firmware Update") + "</h3>";
            html += "<p>" + translate("Installs a new firmware over WiFi - settings, clock faces and hand sets stay. Use the file uhr4.ino.bin from the folder esp32s3 of the release") + ".</p>";
            html += "<p><small>" + translate("Installed version") + ": " + String(version) + ". " + translate("The clock checks the file and only starts the new firmware if it is complete") + ".</small></p>";
            html += "<form id='fwForm' data-ask='" + translate("Install the firmware and restart the clock?") + "'>";
            html += "<input type='file' name='firmware' accept='.bin' required> ";
            html += "<button type='submit'>" + translate("Install Firmware") + "</button></form>";
            html += "<div id='fwFormP' hidden><progress max='100' style='width:100%;'></progress><small></small></div>";
            html += "<div id='fwTexts' hidden data-up='" + translate("Sending firmware - keep this page open") +
                    "' data-check='" + translate("Checking firmware") +
                    "' data-wait='" + translate("Firmware installed - waiting for the clock to restart") +
                    "' data-timeout='" + translate("The clock does not respond yet - refresh the page later") +
                    "' data-err='" + translate("Connection to the clock interrupted") + "'></div>";
            html += "<script>(function(){var f=document.getElementById('fwForm'),T=document.getElementById('fwTexts').dataset;";
            html += "function show(txt,pct){var b=document.getElementById('fwFormP'),p=b.querySelector('progress');b.hidden=false;b.querySelector('small').textContent=txt;if(pct==null)p.removeAttribute('value');else p.value=pct;}";
            html += "function kb(n){return Math.round(n/1024)+' KB';}";
            html += "function done(){f.querySelector('button').disabled=false;if(window.topbarPolling)window.topbarPolling(true);}";
            html += "function waitRestart(){var t0=Date.now();function tryIt(){fetch('/api/topbarStatus',{cache:'no-store'}).then(function(r){if(!r.ok)throw 0;location.href='/';})";
            html += ".catch(function(){if(Date.now()-t0>120000){show(T.timeout,0);done();}else{setTimeout(tryIt,2000);}});}setTimeout(tryIt,5000);}";
            html += "f.addEventListener('submit',function(ev){ev.preventDefault();var file=f.firmware.files[0];if(!file||!confirm(f.dataset.ask))return;";
            html += "f.querySelector('button').disabled=true;if(window.topbarPolling)window.topbarPolling(false);show(T.up,0);";
            html += "var fd=new FormData();fd.append('firmware',file,file.name);var x=new XMLHttpRequest();x.open('POST','/firmware/update');";
            html += "x.upload.onprogress=function(e){if(e.lengthComputable)show(T.up+' ('+kb(e.loaded)+' / '+kb(e.total)+')',100*e.loaded/e.total);};";
            html += "x.upload.onload=function(){show(T.check,null);};";
            html += "x.onload=function(){if(x.status!==200){show(x.responseText||('HTTP '+x.status),0);done();return;}show(T.wait,null);waitRestart();};";
            html += "x.onerror=function(){show(T.err,0);done();};x.send(fd);});})();</script>";

            // Update von GitHub (ota_update.h): Stand abfragen, bei neuerer Version Knopf zum Einspielen. Die Uhr
            // laedt selbst (blockiert dabei, Fortschritt auf dem Display); die Seite wartet auf den Neustart.

            // Update from GitHub (ota_update.h): query the state, a button to install if a newer version exists. The
            // clock downloads by itself (blocking, progress on the display); the page waits for the restart.

            html += "<hr><h3>" + translate("Firmware from GitHub") + "</h3>";
            html += "<p>" + translate("The clock checks once a day (3 a.m.) and after the start whether the latest release on GitHub has a newer firmware. It is installed only on your click; the file is checked by certificate and checksum") + ".</p>";
            html += "<p><small id='ghInfo'></small></p>";
            html += "<button type='button' id='ghCheck'>" + translate("Check now") + "</button> ";
            html += "<button type='button' id='ghInstall' hidden>" + translate("Install update") + "</button>";
            html += "<div id='ghTexts' hidden data-installed='" + translate("Installed version") +
                    "' data-github='" + translate("Firmware on GitHub") +
                    "' data-newer='" + translate("newer - update available") +
                    "' data-same='" + translate("same as installed") +
                    "' data-older='" + translate("older than installed") +
                    "' data-unknown='" + translate("not checked yet") +
                    "' data-busy='" + translate("Checking GitHub") +
                    "' data-load='" + translate("The clock is loading the firmware (progress on its display) - keep this page open") +
                    "' data-wait='" + translate("Firmware installed - waiting for the clock to restart") +
                    "' data-timeout='" + translate("The clock does not respond yet - refresh the page later") +
                    "' data-ask='" + translate("Install the firmware and restart the clock?") + "'></div>";
            html += "<script>(function(){var T=document.getElementById('ghTexts').dataset,i=document.getElementById('ghInfo'),c=document.getElementById('ghCheck'),b=document.getElementById('ghInstall');";
            html += "function lock(on){c.disabled=on;b.disabled=on;if(window.topbarPolling)window.topbarPolling(!on);}";
            html += "function show(r){var s=r.state=='newer'?T.newer:r.state=='same'?T.same:r.state=='older'?T.older:T.unknown;";
            html += "i.textContent=T.installed+': '+r.installed+' - '+T.github+': '+(r.remote||'?')+' ('+s+')'+(r.error?' - '+r.error:'');b.hidden=r.state!='newer';}";
            html += "function query(force){i.textContent=T.busy+' ...';c.disabled=true;fetch('/firmware/check'+(force?'?force=1':''),{cache:'no-store'}).then(function(r){return r.json();}).then(show).catch(function(){i.textContent='?';}).then(function(){c.disabled=false;});}";
            html += "function waitRestart(){var t0=Date.now();function tryIt(){fetch('/api/topbarStatus',{cache:'no-store'}).then(function(r){if(!r.ok)throw 0;location.href='/';})";
            html += ".catch(function(){if(Date.now()-t0>120000){i.textContent=T.timeout;lock(false);}else{setTimeout(tryIt,2000);}});}setTimeout(tryIt,5000);}";
            html += "c.addEventListener('click',function(){query(true);});";
            html += "b.addEventListener('click',function(){if(!confirm(T.ask))return;lock(true);i.textContent=T.load;";
            html += "fetch('/firmware/github',{method:'POST'}).then(function(r){return r.text().then(function(t){if(r.status!==200)throw t||('HTTP '+r.status);});})";
            html += ".then(function(){i.textContent=T.wait;waitRestart();}).catch(function(e){i.textContent=(typeof e=='string'?e:'?');lock(false);});});";
            html += "query(false);})();</script>";
#endif

            html += "</body></html>";
            webserver.send(200, "text/html", html);
            });

        webserver.on("/backup/download", HTTP_POST, []() {
            if (!isPrivateNetworkIp(webserver.client().remoteIP())) {
                webserver.send(403, "text/plain", "Only available from a private network");
                return;
            }
            streamBackup(webserver.arg("wifi") == "1");
            });

        webserver.on("/backup/restore", HTTP_POST, []() {
            bool ok = backupRestore && backupRestore->phase == BackupRestoreState::END;
            bool changed = backupRestore && backupRestore->settingsChecked;
            String error = backupRestore ? backupRestore->error : String("no upload received");
            size_t files = backupRestore ? backupRestore->restoredFiles.size() : 0;
            String backupDisplay = backupRestore ? backupRestore->backupDisplay : String("");
            bool otherType = backupRestore && backupRestore->otherType;
            delete backupRestore;
            backupRestore = nullptr;

            if (!ok) {
                String body = "<p>" + translate("The backup could not be restored") + ": " + escapeHtmlText(error) + "</p>";
                if (!changed) body += "<p>" + translate("Nothing was changed on the clock") + ".</p>";
                body += "<a href='/backup'><button type='button'>" + translate("Back") + "</button></a>";
                webserver.send(400, "text/html", simpleMessagePage(translate("Restore Backup"), body));
                return;
            }
            String myDisplay = displayChoiceName(displayType, useBacklight);
            String note = "";
            if (otherType) {
                note = "<div class='msg warn'>" + translate("The backup is from a {b} clock: clock faces, hands and hub size (scaled to this clock), presets and general settings were restored; display type, rotation, backlight and brightness of this clock ({c}) were kept") + ".</div>";
                note.replace("{b}", backupDisplay);
                note.replace("{c}", myDisplay);
            }
            webserver.send(200, "text/html", simpleMessagePage(translate("Rebooting..."),
                "<p>" + translate("Backup restored") + " (" + String(files) + " " + translate("files") + "). " +
                translate("Return to the main page in 10 seconds or refresh the website when the ESP is online again") + ".</p>" + note,
                "<meta http-equiv='refresh' content='10; url=/'>"));
            espReboot();
            }, handleBackupRestoreUpload);

        // Firmware-Update ueber WLAN, nur ESP32-S3 (ota_update.h)
        // Firmware update over WiFi, ESP32-S3 only (ota_update.h)

        setupOtaRoutes();

        webserver.on("/factoryReset", HTTP_GET, []() {
            String html = beginPage();
            html += generateFlashMessage();
            html += "<h2>" + translate("Factory&nbsp;Reset") + "</h2>";

            // Keine Aktion direkt per Klick: von aussen fuehren alle fuenf erst zu einem Bestaetigungscode
            // auf dem Display. Das versteckte Feld "action" sagt /factoryReset/requestCode, welche Aktion
            // danach ausgefuehrt wird.

            // No action directly on click: from outside all five first lead to a confirmation code on the
            // display. The hidden "action" field tells /factoryReset/requestCode which action to run
            // afterwards.

            html += "<h3>" + translate("Reset Everything") + "</h3>";
            html += "<p>" + translate("Resets WiFi, all settings and deletes all files - the clock restarts afterwards") + ".</p>";
            html += "<form method='POST' action='/factoryReset/requestCode' onsubmit=\"return confirm('" + translate("Are you sure you want to reset to factory settings?") + "');\">";
            html += "<input type='hidden' name='action' value='all'>";
            html += "<button type='submit'>" + translate("Reset Everything") + "</button></form><hr>";

            html += "<h3>" + translate("Reset Saved Networks") + "</h3>";
            html += "<p>" + translate("Deletes all saved WiFi networks - other settings remain unchanged") + ".</p>";
            html += "<form method='POST' action='/factoryReset/requestCode' onsubmit='return confirm(\"" + translate("Are you sure you want to reset all saved WiFi networks?") + "\");'>";
            html += "<input type='hidden' name='action' value='wifi'>";
            html += "<button type='submit'>" + translate("Reset Saved Networks") + "</button></form><hr>";

            html += "<h3>" + translate("Delete Clock Faces (except default)") + "</h3>";
            html += "<p>" + translate("Deletes all clock faces except the built-in ones") + ".</p>";
            html += "<form method='POST' action='/factoryReset/requestCode' onsubmit=\"return confirm('" + translate("Are you sure you want to delete all clock faces except the default one?") + "');\">";
            html += "<input type='hidden' name='action' value='faces'>";
            html += "<button type='submit'>" + translate("Delete Clock Faces (except default)") + "</button></form><hr>";

            html += "<h3>" + translate("Delete Hand Sets (except default)") + "</h3>";
            html += "<p>" + translate("Deletes all hand sets except the built-in ones") + ".</p>";
            html += "<form method='POST' action='/factoryReset/requestCode' onsubmit=\"return confirm('" + translate("Are you sure you want to delete all hand sets except the default one?") + "');\">";
            html += "<input type='hidden' name='action' value='hands'>";
            html += "<button type='submit'>" + translate("Delete Hand Sets (except default)") + "</button></form><hr>";

            html += "<h3>" + translate("Delete Presets") + "</h3>";
            html += "<p>" + translate("Deletes all saved presets - the three starter presets are created again") + ".</p>";
            html += "<form method='POST' action='/factoryReset/requestCode' onsubmit=\"return confirm('" + translate("Are you sure you want to delete all presets?") + "');\">";
            html += "<input type='hidden' name='action' value='presets'>";
            html += "<button type='submit'>" + translate("Delete Presets") + "</button></form><hr>";

            html += "</body></html>";
            webserver.send(200, "text/html", html);
            });

        // Schritt 1: Bestaetigungscode erzeugen und anzeigen, die Aktion merken und zur Eingabeseite
        // weiterleiten.

        // Step 1: generate and show a confirmation code, remember the action and redirect to the entry page.

        webserver.on("/factoryReset/requestCode", HTTP_POST, []() {
            String action = webserver.arg("action");
            if (action != "all" && action != "wifi" && action != "faces" && action != "hands" && action != "presets") {
                redirectTo("/factoryReset");
                return;
            }

            // Aus einem privaten Netz ohne Code - der Code ist nur die zusaetzliche Huerde fuer Anfragen von
            // aussen.

            // From a private network without a code - the code is only the extra hurdle for requests from
            // outside.

            if (isPrivateNetworkIp(webserver.client().remoteIP())) {
                DEBUG_PRINTLN("[SECURITY] Action '" + action + "' executed directly (private network) from " + webserver.client().remoteIP().toString());
                executePendingAction(action);
                return;
            }

            // requestConfirmationCode() erzeugt den Code und merkt sich die
            // Aktion (siehe system_utils.h) - hier nur noch protokollieren
            // und weiterleiten.

            // requestConfirmationCode() generates the code and remembers the
            // action (see system_utils.h) - only logging and the redirect
            // happen here.

            requestConfirmationCode(action);
            DEBUG_PRINTLN("[SECURITY] Confirmation code requested for action '" + action + "' from " + webserver.client().remoteIP().toString());
            redirectTo("/factoryReset/enterCode");
            });

        // Schritt 2: Eingabeseite fuer den auf dem Display gezeigten Code.
        // Step 2: entry page for the code shown on the display.

        webserver.on("/factoryReset/enterCode", HTTP_GET, []() {
            if (factoryResetCode.isEmpty()) {
                redirectTo("/factoryReset");
                return;
            }
            String html = beginPage();
            html += generateFlashMessage();
            html += "<h2>" + translate("Factory&nbsp;Reset") + "</h2>";

            // Kurze englische Aktionsbeschriftung - die einzige Stelle, die zeigt, WELCHE Aktion bestaetigt
            // wird.

            // Short English action label - the only place showing WHICH action is being confirmed.

            html += "<p><strong>" + factoryResetActionLabel(factoryResetPendingAction) + "</strong></p>";
            html += "<p>" + translate("A 3-digit code now appears on the clock's display. Enter it below to confirm - this cannot be undone") + ".</p>";
            html += "<form method='POST' action='/factoryReset/confirm'>";
            html += "<input type='text' name='code' inputmode='numeric' pattern='[0-9]{3}' maxlength='3' autofocus autocomplete='off' style='width:100px;font-size:1.5rem;text-align:center;letter-spacing:.2em;'> ";
            html += "<button type='submit'>" + translate("Confirm Reset") + "</button>";
            html += "</form>";
            html += "<p><a href='/factoryReset'>" + translate("Cancel") + "</a></p>";
            html += "</body></html>";
            webserver.send(200, "text/html", html);
            });

        // Schritt 3: Code pruefen, erst dann die gemerkte Aktion per executePendingAction() ausfuehren -
        // gemeinsam fuer alle fuenf Aktionen.

        // Step 3: verify the code, only then run the remembered action via executePendingAction() - shared by
        // all five actions.

        webserver.on("/factoryReset/confirm", HTTP_POST, []() {
            String action = factoryResetPendingAction;

            if (factoryResetCode.isEmpty() || action.isEmpty() || millis() - factoryResetCodeStartMillis > FACTORY_RESET_CODE_TIMEOUT_MS) {
                factoryResetCode = "";
                factoryResetPendingAction = "";
                factoryResetCodeAttempts = 0;
                DEBUG_PRINTLN("[SECURITY] Confirmation attempted from " + webserver.client().remoteIP().toString() + " with no valid code pending");
                redirectTo("/factoryReset?err=Code%20expired%2C%20please%20try%20again");
                return;
            }
            if (!webserver.hasArg("code") || webserver.arg("code") != factoryResetCode) {
                factoryResetCodeAttempts++;
                DEBUG_PRINTLN("[SECURITY] Confirmation attempted from " + webserver.client().remoteIP().toString() + " with wrong code (action: " + action + ", attempt " + String(factoryResetCodeAttempts) + "/" + String(FACTORY_RESET_MAX_ATTEMPTS) + ")");

                // Zu viele Fehlversuche: den Code sofort ungueltig machen statt weiter raten zu lassen - dann
                // ist ein neuer faellig.

                // Too many wrong attempts: invalidate the code right away instead of allowing more guessing -
                // a new one is then due.

                if (factoryResetCodeAttempts >= FACTORY_RESET_MAX_ATTEMPTS) {
                    factoryResetCode = "";
                    factoryResetPendingAction = "";
                    DEBUG_PRINTLN("[SECURITY] Too many wrong attempts from " + webserver.client().remoteIP().toString() + " - code invalidated");
                    redirectTo("/factoryReset?err=Too%20many%20wrong%20attempts%2C%20please%20try%20again");
                    return;
                }

                redirectTo("/factoryReset/enterCode?err=Wrong%20code%2C%20please%20try%20again");
                return;
            }

            // Code verbraucht - vor der Aktion loeschen, damit die Code-Anzeige das Display nicht mehr
            // belegt.

            // Code consumed - clear it before the action, so the code screen no longer occupies the display.

            factoryResetCode = "";
            factoryResetPendingAction = "";
            factoryResetCodeAttempts = 0;
            DEBUG_PRINTLN("[SECURITY] Confirmed from " + webserver.client().remoteIP().toString() + " (action: " + action + ")");

            executePendingAction(action);
            });

        // Sofortige Zeitsynchronisation: startet nur die asynchrone Task und antwortet gleich - das Ergebnis
        // zeigen Statuszeile (alle 5 s) bzw. Zeit-Tab.

        // Immediate time sync: only starts the asynchronous task and responds right away - the result shows
        // in the status bar (every 5 s) or the Time tab.

        webserver.on("/syncnow", HTTP_POST, []() {
            startNtpSyncTask("Manual sync");

            String html = simpleMessagePage(translate("Time sync started"), "<p>" + translate("Returning to main page in 3 seconds") + ".</p>", "<meta http-equiv='refresh' content='3; url=/'>");

            webserver.send(200, "text/html", html);
            });

    }


    // Ergaenzt beim Hochladen ein fehlendes Praefix: Zifferblaetter (/upload) "face_", Zeiger (/uploadhandset)
    // "hand_set" - aus "set3_hour.bmp", "hand3_hour.bmp" oder "3_hour.bmp" wird "hand_set3_hour.bmp". Die
    // Endung ".BMP" wird klein geschrieben; andere Dateien (Schriften, Streifen) bleiben unveraendert.

    // Adds a missing prefix on upload: clock faces (/upload) "face_", hands (/uploadhandset) "hand_set" -
    // "set3_hour.bmp", "hand3_hour.bmp" or "3_hour.bmp" becomes "hand_set3_hour.bmp". The extension ".BMP" is
    // lowercased; other files (fonts, strips) stay unchanged.

    String completeUploadName(const String& path, const String& uri) {
        String name = path;
        while (name.startsWith("/")) name = name.substring(1);
        String lower = name;
        lower.toLowerCase();
        if (!lower.endsWith(".bmp")) return "/" + name;
        name = name.substring(0, name.length() - 4) + ".bmp";
        if (uri == "/uploadhandset" && !name.startsWith("hand_set") && !name.startsWith("face_") && !name.startsWith("strip_")) {
            if (name.startsWith("hand_")) name = name.substring(5);
            else if (name.startsWith("hand")) name = name.substring(4);
            if (name.startsWith("set")) name = name.substring(3);
            name = "hand_set" + name;
        }
        else if (uri == "/upload" && !name.startsWith("face_") && !name.startsWith("hand_set") && !name.startsWith("strip_")) {
            name = "face_" + name;
        }
        return "/" + name;
    }

    // Handhabt den Datei-Upload
    // Handles the file upload

    void handleFileUpload() {
        HTTPUpload& upload = webserver.upload();

        if (upload.status == UPLOAD_FILE_START) {
            uploadFilePath = completeUploadName("/" + upload.filename, webserver.uri());
            if (uploadFilePath != "/" + upload.filename) {
                DEBUG_PRINTLN("[UPLOAD] Renamed " + upload.filename + " -> " + uploadFilePath.substring(1));
            }

            // Nur aus einem privaten Netz - HIER pruefen, da dieser Rueckruf schon je Datenblock laeuft, der
            // Request-Handler erst nach dem Upload; sonst stuenden die Daten bereits auf LittleFS.

            // Only from a private network - checked HERE, since this callback already runs per chunk, the
            // request handler only after the upload; otherwise the data would already be on LittleFS.

            if (!isPrivateNetworkIp(webserver.client().remoteIP())) {
                DEBUG_PRINTLN("[UPLOAD] Rejected: not a private network : " + uploadFilePath + " (from " + webserver.client().remoteIP().toString() + ")");
                uploadSuccess = false;
                return;
            }

            // Nur bestimmte Dateinamenmuster zulassen - dazu Schriften fuer den Zifferblatt-Designer (font_*)
            // und die VLW-Schriften des Streifens.

            // Only allow certain filename patterns - including fonts for the clock face designer (font_*) and
            // the strip's VLW fonts.

            bool isFontFile = uploadFilePath.startsWith("/font_") &&
                              (uploadFilePath.endsWith(".ttf") || uploadFilePath.endsWith(".otf") ||
                               uploadFilePath.endsWith(".woff") || uploadFilePath.endsWith(".woff2"));
            bool isStripVlw = uploadFilePath.startsWith("/stripfont_") && uploadFilePath.endsWith(".vlw");
            if (!isFontFile && !isStripVlw && (!uploadFilePath.endsWith(".bmp") ||
                !(uploadFilePath.startsWith("/face_") || uploadFilePath.startsWith("/hand_set") ||
                  (uploadFilePath.startsWith("/strip_") && TFT_HEIGHT > CLOCK_HEIGHT)))) {
                DEBUG_PRINTLN("[UPLOAD] Invalid filename: must start with 'face_', 'hand_set' or 'strip_' (display with strip) and end with '.bmp' : " + uploadFilePath + " (from " + webserver.client().remoteIP().toString() + ")");
                uploadSuccess = false;
                return;
            }

            // Die Zeiger-Route nimmt nur Zeiger an - sonst laege z.B. ein dort hochgeladenes Zifferblatt trotz
            // Fehlermeldung schon auf der Uhr (/uploadhandset prueft den Namen erst nach dem Upload).

            // The hand route only accepts hands - otherwise e.g. a clock face uploaded there would already be on
            // the clock despite the error message (/uploadhandset checks the name only after the upload).

            if (webserver.uri() == "/uploadhandset" && !uploadFilePath.startsWith("/hand_set")) {
                DEBUG_PRINTLN("[UPLOAD] Invalid filename for a hand set: " + uploadFilePath + " (from " + webserver.client().remoteIP().toString() + ")");
                uploadSuccess = false;
                return;
            }

            // Zeichen-Set beschraenken (wie bei /rename): der Name wird spaeter
            // in /handsets in HTML/JS eingebettet - ohne diese Pruefung waere ein
            // praeparierter Dateiname ein Weg zu stored XSS/Pfad-Traversal.

            // Restrict the character set (like /rename): the name is later
            // embedded in HTML/JS in /handsets - without this check a crafted
            // filename would be a path to stored XSS/path traversal.

            {
                bool uploadNameValid = true;
                for (size_t ni = 1; uploadNameValid && ni < uploadFilePath.length(); ni++) {
                    char c = uploadFilePath[ni];
                    if (!(isalnum((unsigned char)c) || c == '_' || c == '-' || c == '.')) {
                        uploadNameValid = false;
                    }
                }
                if (!uploadNameValid || uploadFilePath.indexOf("..") >= 0) {
                    DEBUG_PRINTLN("[UPLOAD] Invalid filename: contains disallowed characters : " + uploadFilePath + " (from " + webserver.client().remoteIP().toString() + ")");
                    uploadSuccess = false;
                    return;
                }
            }

            if (isProtectedFile(uploadFilePath)) {
                DEBUG_PRINTLN("[UPLOAD] Rejected - built-in file: " + uploadFilePath + " (from " + webserver.client().remoteIP().toString() + ")");
                uploadSuccess = false;
                return;
            }

            DEBUG_PRINTLN("[UPLOAD] Start: " + uploadFilePath + " (from " + webserver.client().remoteIP().toString() + ")");
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
                    DEBUG_PRINTLN("[UPLOAD] Finished OK: " + uploadFilePath + " (from " + webserver.client().remoteIP().toString() + ")");
                    if (uploadFilePath.startsWith("/stripfont_")) {
                        stripVlwStale = true;
                        infoStripDirty[0] = infoStripDirty[1] = true;
                    }
                    String lowerPath = uploadFilePath;
                    lowerPath.toLowerCase();
                    if (lowerPath.endsWith(".bmp")) {

                      //  bool isHand = uploadFilePath.indexOf("hour") > 0 || uploadFilePath.indexOf("minute") > 0 || uploadFilePath.indexOf("second") > 0;

                        if (uploadFilePath.startsWith("/face_")) {
                            DEBUG_PRINTLN("[UPLOAD] Detected Clock Face upload (from " + webserver.client().remoteIP().toString() + ")");

                            if (!scaleAndSaveBmp(uploadFilePath.c_str(), uploadFilePath.c_str(), CLOCK_WIDTH, CLOCK_HEIGHT)) {
                                DEBUG_PRINTLN("[UPLOAD] Scaling failed for " + uploadFilePath + " (from " + webserver.client().remoteIP().toString() + ")");
                                LittleFS.remove(uploadFilePath); // unbrauchbare Originaldatei nicht liegen lassen
                                                                 // do not leave the unusable original file behind
                                uploadSuccess = false;
                                return;
                            }

                        }
                        else if (uploadFilePath.startsWith("/strip_")) {

                            // Streifen-Grafik zum Zifferblatt (strip_<Name>.bmp zu face_<Name>.bmp), hochkant
                            // Strip graphic of a clock face (strip_<name>.bmp for face_<name>.bmp), portrait

                            DEBUG_PRINTLN("[UPLOAD] Detected strip upload (from " + webserver.client().remoteIP().toString() + ")");
                            if (!scaleAndSaveBmp(uploadFilePath.c_str(), uploadFilePath.c_str(), TFT_WIDTH, TFT_HEIGHT - CLOCK_HEIGHT)) {
                                DEBUG_PRINTLN("[UPLOAD] Scaling failed for " + uploadFilePath + " (from " + webserver.client().remoteIP().toString() + ")");
                                LittleFS.remove(uploadFilePath); // unbrauchbare Originaldatei nicht liegen lassen
                                                                 // do not leave the unusable original file behind
                                uploadSuccess = false;
                                return;
                            }
                            stripImageFor = "?";
                            infoStripDirty[0] = infoStripDirty[1] = true;
                        }
                        else if (uploadFilePath.startsWith("/hand_set")) {
                            DEBUG_PRINTLN("[UPLOAD] Detected Clock Hand upload (from " + webserver.client().remoteIP().toString() + ")");

                            int targetW, targetH;
                            handTargetSize(uploadFilePath.c_str(), targetW, targetH);
                            if (!scaleAndSaveBmp(uploadFilePath.c_str(), uploadFilePath.c_str(), targetW, targetH)) {
                                DEBUG_PRINTLN("[UPLOAD] Scaling failed for " + uploadFilePath + " (from " + webserver.client().remoteIP().toString() + ")");
                                LittleFS.remove(uploadFilePath); // unbrauchbare Originaldatei nicht liegen lassen
                                                                 // do not leave the unusable original file behind
                                uploadSuccess = false;
                                return;
                            }
                        }                    
                    }
                }
                else {
                    DEBUG_PRINTLN("[UPLOAD] Finished but file missing: " + uploadFilePath + " (from " + webserver.client().remoteIP().toString() + ")");
                    uploadSuccess = false;
                }
            }
            else {
                DEBUG_PRINTLN("[UPLOAD] Failed during writing: " + uploadFilePath + " (from " + webserver.client().remoteIP().toString() + ")");
            }
        }
    }


