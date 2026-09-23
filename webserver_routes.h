#pragma once
    // Alle HTTP-Routen und HTML-Generierung. Benoetigt globals.h, config.h,
    // prefs_keys.h und declarations.h (vorher in uhr3.ino eingebunden).

    // All HTTP routes and HTML generation. Requires globals.h, config.h,
    // prefs_keys.h and declarations.h (included earlier in uhr3.ino).

    // Fuer "new (std::nothrow)": garantiert nullptr statt
    // implementierungsabhaengigem Verhalten bei fehlgeschlagener Allokation.

    // For "new (std::nothrow)": guarantees nullptr instead of
    // implementation-defined behaviour on a failed allocation.
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
        // "na" (grau) gilt nur noch fuer den Zeit-Punkt (Systemzeit noch nie
        // gesetzt). RTC/DCF77 nutzen es nicht mehr - fehlende Hardware macht
        // den Eintrag komplett unsichtbar statt dauerhaft grau.

        // "na" (gray) now only applies to the Time dot (system time never
        // set). RTC/DCF77 no longer use it - missing hardware hides the
        // whole entry instead of showing it permanently gray.
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
        // CSS-only Tabs: Radios ausblenden, Panels per :checked ~ .panel-X
        // einblenden. Radios/.tabnav/.panel-* muessen direkte Geschwister sein.

        // CSS-only tabs: hide radios, show panels via :checked ~ .panel-X.
        // Radios/.tabnav/.panel-* must be direct siblings.
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


    // Status-Werte fuer die Topbar-Punkte, gemeinsam genutzt von
    // generateTopBar() und /api/topbarStatus: "ok" gruen, "syncing" gelb
    // blinkend, "bad" rot (echter Fehler), "na" grau bzw. Eintrag ausgeblendet.

    // Status values for the topbar dots, shared by generateTopBar() and
    // /api/topbarStatus: "ok" green, "syncing" blinking yellow, "bad" red
    // (genuine error), "na" gray or entry hidden (hardware missing).

    // Ermittelt den Zeit-Status ("ok"/"na") - kein Fehlerzustand, da es fuer
    // Systemzeit kein "gefunden, aber ungueltig" gibt.

    // Determines the time status ("ok"/"na") - no error state, since there's
    // no "found but invalid" concept for system time.

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
#if defined SDA_PIN && defined SCL_PIN
        if (rtcOk == RTC_AVAILABLE) return "ok";
        if (rtcOk == RTC_AVAILABLE_BUT_INVALID) return "bad";
        return "na";
#else
        return "na";
#endif
    }


    // Ermittelt den DCF77-Status: "ok" wenn zuletzt erfolgreich dekodiert
    // (DCF77_SYNC_STALE_AFTER), sonst "syncing" solange noch Impulse kommen,
    // "na" ohne bestaetigten Empfang (dcf77Confirmed), sonst "bad".

    // Determines DCF77 status: "ok" if last decoded within
    // DCF77_SYNC_STALE_AFTER, else "syncing" while pulses still arrive,
    // "na" without confirmed reception (dcf77Confirmed), else "bad".

    String getDcf77Status() {
#if defined DCF77_DATAPIN && defined DCF77_INTERRUPT
        // Ein einzelner Impuls (auch durch Rauschen) reicht nicht als Nachweis -
        // siehe DCF77_PRESENCE_MIN_STREAK/MAX_GAP_MS in config.h.
        if (!dcf77Confirmed) return "na"; // noch keine plausible Impulskette / no plausible pulse chain yet
        bool syncFresh = dcfTimeFound && lastDcfSyncTime != 0 &&
                          (time(nullptr) - lastDcfSyncTime) < (time_t)(DCF77_SYNC_STALE_AFTER / 1000);
        if (syncFresh) return "ok";
        bool pulsesFresh = (lastDcf77PulseChangeMillis != 0) &&
                            (millis() - lastDcf77PulseChangeMillis) < DCF77_PULSE_STALE_AFTER;
        if (pulsesFresh) return "syncing";
        return "bad";
#else
        return "na";
#endif
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


    // Escaped Text fuer sichere HTML-Einbettung (z.B. Logdatei-Inhalt) sowie
    // Anfuehrungszeichen, da Werte auch in einfach gequotete Attribute
    // (value='...', onclick='...') eingebettet werden - sonst Attribut-Injection.

    // Escapes text for safe HTML embedding (e.g. log content) and also
    // quote characters, since values are also embedded into single-quoted
    // attributes (value='...', onclick='...') - otherwise attribute injection.

    // Fuer Werte in onclick='...', das intern selbst einen JS-String enthaelt:
    // aeusseres HTML-Zeichen als Entity, inneres JS-Zeichen per Backslash
    // escapen (Browser dekodiert HTML-Entities VOR dem JS-Parsing).

    // For values inside onclick='...' that itself contains a JS string:
    // escape the outer HTML quote as an entity, the inner JS quote with a
    // backslash (the browser decodes HTML entities BEFORE JS parsing).
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


    // Prueft, ob eine anfragende IP-Adresse tatsaechlich im GLEICHEN Netz
    // steht wie die Uhr selbst - egal ob diese gerade per STA mit dem
    // Heimnetz verbunden ist oder ihren eigenen Access Point betreibt. Nicht
    // nur, ob die anfragende IP irgendwo in einem der privaten RFC1918-
    // Bereiche liegt (10.0.0.0/8, 172.16.0.0/12, 192.168.0.0/16). Genutzt, um
    // Status-Tab und diverse Aktionen (siehe die Aufrufer) vor Zugriff von
    // ausserhalb des lokalen Netzes zu verbergen/sperren, z.B. bei
    // Erreichbarkeit ueber eine DMZ/Port-Weiterleitung.
    //
    // Der reine Bereichscheck allein reicht nicht: ueber bestimmte Aufbauten
    // (z.B. ein VPN-/Tunnel-Gateway oder ein Reverse-Proxy mit eigenem
    // privaten Adressraum vor der Uhr) koennte eine Anfrage von ausserhalb
    // trotzdem mit einer privat aussehenden Quell-IP ankommen und faelschlich
    // als vertrauenswuerdig durchgehen. Das Netz, in dem die Uhr selbst
    // steht, soll dagegen IMMER als privat gelten - deshalb wird explizit
    // sowohl das STA- als auch das AP-eigene Subnetz geprueft, statt sich im
    // AP-Fall nur zufaellig auf den RFC1918-Rueckfall zu verlassen:
    //
    // - STA (auch im gleichzeitigen AP+STA-Betrieb): eigenes Subnetz ueber
    //   WiFi.localIP()/WiFi.subnetMask().
    // - AP (auch AP+STA): eigenes Subnetz ueber WiFi.softAPIP() mit der
    //   Standardmaske 255.255.255.0 - dieses Projekt ruft WiFi.softAP() ohne
    //   softAPConfig() auf (siehe startAP() in wifi_manager.h), die ESP32-
    //   Arduino-Bibliothek vergibt dann immer 192.168.4.1/24, daher hier
    //   fest angenommen statt von einer moeglicherweise nicht ueberall
    //   verfuegbaren WiFi.softAPSubnetMask() abhaengig zu sein.
    //
    // Byteweise verglichen (IPAddress::operator[]), um keine Annahmen ueber
    // eine eventuelle uint32_t-Konvertierung/Byte-Reihenfolge treffen zu muessen.
    //
    // Nur als letzter Rueckfall (z.B. Subnetzmaske kurzzeitig unbekannt,
    // gleich nach dem Verbindungsaufbau) der grobe RFC1918-Bereichscheck wie
    // bisher. Absichtlich weiterhin NUR die drei RFC1918-Bereiche als
    // Rueckfall, nicht auch Link-Local (169.254.x.x) oder Loopback - beide
    // sind fuer den normalen Zugriff auf dieses Geraet ohnehin nicht relevant.

    // Checks whether a requesting IP address is actually in the SAME
    // network as the clock itself - regardless of whether it's currently
    // connected to the home network via STA or running its own access
    // point. Not just whether the requesting IP falls somewhere into one of
    // the private RFC1918 ranges (10.0.0.0/8, 172.16.0.0/12, 192.168.0.0/16).
    // Used to hide/block the Status tab and various actions (see the
    // callers) from access outside the local network, e.g. when reachable
    // via a DMZ/port forward.
    //
    // The plain range check alone isn't enough: via certain setups (e.g. a
    // VPN/tunnel gateway or a reverse proxy with its own private address
    // space in front of the clock), a request from outside could still
    // arrive with a private-looking source IP and wrongly pass as
    // trustworthy. The network the clock itself is on, on the other hand,
    // should ALWAYS count as private - so both the STA and the AP's own
    // subnet are checked explicitly, instead of relying on the AP case
    // coincidentally falling under the RFC1918 fallback:
    //
    // - STA (also while running AP+STA simultaneously): own subnet via
    //   WiFi.localIP()/WiFi.subnetMask().
    // - AP (also AP+STA): own subnet via WiFi.softAPIP() with the default
    //   mask 255.255.255.0 - this project calls WiFi.softAP() without
    //   softAPConfig() (see startAP() in wifi_manager.h), so the ESP32
    //   Arduino library always assigns 192.168.4.1/24, hence assumed fixed
    //   here instead of depending on a WiFi.softAPSubnetMask() that may not
    //   be available everywhere.
    //
    // Compared byte by byte (IPAddress::operator[]), to avoid any
    // assumptions about a possible uint32_t conversion/byte order.
    //
    // Only as a last-resort fallback (e.g. the subnet mask briefly unknown
    // right after connecting) the rough RFC1918 range check as before.
    // Deliberately still ONLY these three RFC1918 ranges as the fallback,
    // not link-local (169.254.x.x) or loopback either - neither is relevant
    // for normal access to this device anyway.

    bool isSameSubnet(IPAddress ip, IPAddress ownIp, IPAddress mask) {
        for (int i = 0; i < 4; i++) {
            if ((ip[i] & mask[i]) != (ownIp[i] & mask[i])) return false;
        }
        return true;
    }

    bool isPrivateNetworkIp(IPAddress ip) {
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


    // Liefert die Seite, zu der /delete und /rename nach ihrer Aktion
    // zurueckspringen sollen. /files UND /listfilesFaces zeigen beide
    // Zifferblatt-/Zeigersatz-Dateien mit denselben Delete-/Rename-Links an -
    // frueher wurde das Ziel aus dem Dateinamen geraten ("face_*.bmp" ->
    // /listfilesFaces, "hand_set*.bmp" -> /handsets), was beim Loeschen/
    // Umbenennen einer solchen Datei ÜBER /files faelschlich auf die
    // spezialisierte Seite statt zurueck zu /files sprang. Jetzt haengen
    // /files und /listfilesFaces stattdessen einen expliziten "from"-
    // Parameter an ihre Links an. Ein unbekannter/fehlender Wert faellt auf
    // den allgemeinen Dateimanager zurueck, der fuer jede Datei passt.

    // Returns the page /delete and /rename should redirect back to after
    // their action. Both /files AND /listfilesFaces show clock-face/hand-set
    // files with the same delete/rename links - the target used to be
    // guessed from the filename ("face_*.bmp" -> /listfilesFaces,
    // "hand_set*.bmp" -> /handsets), which wrongly jumped to the specialized
    // page instead of back to /files when deleting/renaming such a file FROM
    // /files. Now /files and /listfilesFaces instead attach an explicit
    // "from" parameter to their links. An unknown/missing value falls back
    // to the general file manager, which fits for any file.

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
        html += "<span class='brand-mark'>UHR&middot;3</span>";

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
        html += "</div>";

        html += "<div class='status-strip'>";

        // IDs ("dot-time"/"dot-rtc"/"dot-dcf77") + title/aria-label auf jedem
        // Punkt: Live-Status-Skript aktualisiert Farbe und Text bei jedem
        // Poll, damit Farbfehlsichtige/Screenreader die Bedeutung erkennen.

        // IDs ("dot-time"/"dot-rtc"/"dot-dcf77") + title/aria-label on every
        // dot: the live-status script updates color and text on every poll,
        // so colorblind users/screen readers can tell the meaning apart.

        // "Zeit" wird als letzter Eintrag gerendert, direkt neben der
        // Datumsanzeige - timeState wird aber schon hier berechnet, da es
        // unten fuer #topbar-datetime gebraucht wird.

        // "Time" is rendered last, right next to the date display -
        // timeState is computed here already since it's needed below for
        // #topbar-datetime.
        String timeState = getTimeStatus();
        bool timeOk = (timeState == "ok"); // fuer die Datumsanzeige weiter unten wiederverwendet / reused for the date display further below
        String timeTitle = dotStatusText(translate("Time"), timeState);

#ifdef ADC_PIN
        // Live-Helligkeitswert statt Status-Punkt. Ohne useAdc (Spannungsteiler
        // unbestromt, siehe uhr3.ino) den Eintrag lieber weglassen statt einen
        // eingefrorenen Wert zu zeigen.

        // Live brightness value instead of a status dot. Without useAdc
        // (voltage divider unpowered, see uhr3.ino) omit the entry rather
        // than show a stale value.
        if (photoresistorFound && useAdc) {
            html += "<span class='status' id='status-light'>" + translate("Light") + ": <span id='value-light' class='statval'>" + String(currentLightPercent) + " %</span></span>";
        }
#endif

#if defined SDA_PIN && defined SCL_PIN
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
#endif

#if defined DCF77_DATAPIN && defined DCF77_INTERRUPT
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
#endif

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

        // Versteckter Hinweis, siehe ".offline-hint" in generateHtmlHeader().
        // Hidden hint, see ".offline-hint" in generateHtmlHeader().
        html += "<span id='topbar-offline-hint' class='offline-hint'>&#9888; " + translate("Connection lost") + "</span>";

        // Kein Refresh-Knopf/Auto-Refresh - alle Werte aktualisieren sich
        // ohnehin per JS. Neustart bleibt (echter Geraete-Neustart).

        // No refresh button/auto-refresh - all values already update via
        // JS. Restart stays (an actual device restart).
        html += "<button type='button' class='reset-btn' onclick='if(confirm(\"" + translate("Are you sure you want to reboot?") + "\")){location.href=\"/reboot\";}' title='" + translate("Reboot") + "'>&#9211;</button>";

        html += "</header>";

        // Live-Status: pollt /api/topbarStatus alle 5s und aktualisiert Punkte
        // + Uhrzeit ohne Seiten-Reload. Offline-Hinweis erst nach zwei
        // fehlgeschlagenen Polls; pausiert, waehrend der Tab im Hintergrund ist.

        // Live status: polls /api/topbarStatus every 5s and updates dots +
        // time without a page reload. Offline hint only after two failed
        // polls; pauses while the tab is in the background.

        // pageVersion: die Build-Version, mit der DIESE Seite gerendert
        // wurde (siehe "version" in globals.h). Weicht ein spaeterer Poll
        // davon ab, laedt die Seite komplett neu (neuere Firmware per OTA/WPS-Neustart), statt mit veraltetem UI weiterzulaufen.

        // pageVersion: the build version THIS page was rendered with (see
        // "version" in globals.h). If a later poll reports a different
        // version, the page does a full reload (newer firmware via OTA/WPS reboot) instead of running on with a stale UI.
        html += "<script>(function(){";
        // "version" ist ein reiner Build-Zeitstempel ohne Anfuehrungszeichen
        // o.ae. (siehe globals.h) - daher hier ohne Escaping direkt als
        // JS-String-Literal eingebettet, wie auch beim JSON oben.

        // "version" is a plain build timestamp with no quotes etc. (see
        // globals.h) - so it's embedded here directly as a JS string
        // literal without escaping, same as in the JSON above.
        html += "var pageVersion=\"" + String(version) + "\";";
        html += "function setStatusDot(id,state,title){var el=document.getElementById(id);if(!el)return;el.classList.toggle('ok',state==='ok');el.classList.toggle('syncing',state==='syncing');el.classList.toggle('na',state==='na');if(title){el.title=title;el.setAttribute('aria-label',title);}}";
        // setPresent(): blendet einen Eintrag (aktuell nur DCF77) live ein,
        // sobald der Server echte Aktivitaet bestaetigt.

        // setPresent(): reveals an entry (currently only DCF77) live once
        // the server confirms real activity.
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
        // Gleiches Feld wie oben (s.dcf77Present) treibt auch den
        // Navigations-Eintrag - siehe Kommentar in generateNavigation().

        // Same field as above (s.dcf77Present) also drives the navigation
        // entry - see the comment in generateNavigation().
        html += "setPresent('nav-dcf77',s.dcf77Present);";
        html += "setValue('value-light',s.lightValue);";
        html += "var dt=document.getElementById('topbar-datetime');if(dt)dt.textContent=s.datetime;";
        html += "}).catch(function(){failCount++;if(failCount>=2)setOnline(false);});}";
        html += "var pollTimer=null;";
        html += "function startPolling(){if(pollTimer)return;poll();pollTimer=setInterval(poll,5000);}";
        html += "function stopPolling(){if(!pollTimer)return;clearInterval(pollTimer);pollTimer=null;}";
        html += "document.addEventListener('visibilitychange',function(){if(document.hidden){stopPolling();}else{startPolling();}});";
        html += "if(!document.hidden)startPolling();";
        html += "})();</script>";

        return html;
    }


    // Kurze Zeile mit LittleFS-Speichernutzung, reiner Inline-Text ohne
    // umschliessendes Element - der Aufrufer bettet ihn je nach Layout ein.

    // Short line with LittleFS storage usage, plain inline text with no
    // wrapping element - the caller embeds it depending on layout.

    // forceEnglish: die Status-Seite ist immer Englisch (technische
    // Diagnoseansicht), andere Aufrufer bleiben normal uebersetzt.

    // forceEnglish: the status page is always English (technical diagnostic
    // view), other callers stay normally translated.

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
        // Ueberschreibt "display:block!important" der mobilen Regel unten
        // fuer ausgeblendete Eintraege (z.B. DCF77 vor dcf77Confirmed) - hoehere
        // Selektor-Spezifitaet reicht theoretisch, explizit ist aber robuster.

        // Overrides the mobile rule's "display:block!important" below for
        // hidden entries (e.g. DCF77 before dcf77Confirmed) - higher selector
        // specificity would technically suffice, but being explicit is more robust.
        nav += ".navLinks a[hidden],.navLinks span[hidden]{display:none !important;}";
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
            // WiFi/Zeit/Helligkeit/Status sind jetzt Tabs auf "/", daher hier
            // nicht mehr gelistet - Routen bleiben fuer Lesezeichen bestehen.

            // WiFi/time/brightness/status are now tabs on "/", so no longer
            // listed here - routes stay for bookmarks.
            {"/", translate("Main"), ""},
            {"/preview", translate("Preview"), ""},
            {"/presets", translate("Presets"), ""},
            {"/listfilesFaces", translate("Clock&nbsp;Face"), ""},
            {"/handsets", translate("Hand&nbsp;Set"), ""},
            {"/files", translate("File&nbsp;Manager"), ""},
            // "/reboot" nicht mehr gelistet - der reset-btn in der Topbar
            // deckt das ab, die Route bleibt fuer Lesezeichen bestehen.
            // "/reboot" no longer listed - the topbar's reset-btn covers

            // this, the route stays for bookmarks.
            // "DCF77" bewusst unuebersetzt (Protokollname, wie in generateTopBar()).
            // "DCF77" deliberately untranslated (a protocol name, as in generateTopBar()).
            {"/dcf77", "DCF77", ""},
            {"/factoryReset", translate("Factory&nbsp;Reset"), ""}
        };

        String currentPath = webserver.uri(); // Aktueller Pfad der Seite
                                              // current path of the page

        for (const auto& item : navItems) {
            // "/dcf77" nur weglassen, wenn keine Hardware verbaut ist - sonst
            // wird der Eintrag per 'hidden' unsichtbar gerendert und per
            // Live-Poll eingeblendet, sobald DCF77 erkannt wird.

            // Only omit "/dcf77" when no hardware is wired up - otherwise
            // the entry is rendered invisible via 'hidden' and gets
            // revealed live once DCF77 is recognized at runtime.
            bool dcf77Hidden = false;
            if (item.path == "/dcf77") {
#if defined DCF77_DATAPIN && defined DCF77_INTERRUPT
                dcf77Hidden = !dcf77Confirmed;
#else
                continue;
#endif
            }

            // id nur fuer den DCF77-Eintrag, damit setPresent() ihn per
            // Live-Poll gezielt ein-/ausblenden kann (siehe oben).

            // id only for the DCF77 entry, so setPresent() can toggle its
            // visibility via the live poll (see above).
            String idAttr = (item.path == "/dcf77") ? " id='nav-dcf77'" : "";
            String hiddenAttr = dcf77Hidden ? " hidden" : "";

            if (item.path == currentPath) {
                // Wenn der aktuelle Pfad mit dem Navigationseintrag übereinstimmt, nur Text anzeigen
                // If the current path matches the nav entry, show plain text only
                nav += "<span" + idAttr + hiddenAttr + " style=\"margin-right:15px; font-weight:bold;\">" + item.label + "</span> ";
            }
            else {
                // Andernfalls als Link anzeigen
                // Otherwise show as a link
                nav += "<a" + idAttr + hiddenAttr + " href=\"" + item.path + "\" style=\"margin-right:15px;\"";
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


    // Zeigt eine einheitliche Erfolgsmeldung, wenn die Route per Weiterleitung
    // einen "msg"-Parameter mitgibt (translate()-Schluessel, automatisch uebersetzt) -
    // blendet sich nach ein paar Sekunden per JS aus, ein Muster fuer alle Aktionen.

    // Shows a uniform success message when the route redirect passes a
    // "msg" parameter (translate() key, auto-translated) - fades out after
    // a few seconds via JS, a pattern used for all actions.

    String generateFlashMessage() {
        if (!webserver.hasArg("msg")) return "";
        String rawMsg = webserver.arg("msg");
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

        String html = "<div id='flashMsg' style='background:rgba(61,220,132,0.12);color:var(--ok);border:1px solid var(--ok);border-radius:6px;padding:10px 15px;margin:10px auto;max-width:500px;'>";
        html += "&#9989; " + message;
        html += "</div>";
        if (isWpsBanner) {
            // Waehrend WPS laeuft, ist die Uhr kurz nicht erreichbar - jeder
            // fetch() schlaegt erwartungsgemaess fehl und wird per catch()
            // wiederholt. AbortController begrenzt jeden Versuch auf 3s, damit ein haengender fetch() die Wiederholung nicht blockiert.

            // While WPS runs, the clock is briefly unreachable - every
            // fetch() is expected to fail and gets retried via catch().
            // AbortController caps each attempt at 3s, so a hanging fetch() doesn't block the retry.
            html += "<script>(function(){var e=document.getElementById('flashMsg');var startedAt=Date.now();function poll(){if(!e)return;if(Date.now()-startedAt>180000){e.style.display='none';return;}var ctrl=(typeof AbortController!=='undefined')?new AbortController():null;var timer=ctrl?setTimeout(function(){ctrl.abort();},3000):null;fetch('/api/wpsStatus',ctrl?{signal:ctrl.signal}:{}).then(function(r){if(timer)clearTimeout(timer);return r.json();}).then(function(d){if(!d.pending){e.style.display='none';}else{setTimeout(poll,1000);}}).catch(function(){if(timer)clearTimeout(timer);setTimeout(poll,1000);});}poll();})();</script>";
        }
        else {
            html += "<script>setTimeout(function(){var e=document.getElementById('flashMsg'); if(e) e.style.display='none';}, 4000);</script>";
        }
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

        // Verweilzeit im AP-Modus verlaengern (siehe softAPIPstart/WAIT_15m in
        // uhr3.ino): jede Formular-Aktion (Speichern etc.) zaehlt als aktiver
        // Zugriff, der 15-Minuten-Neustart soll also nicht mitten in einer
        // Konfiguration dazwischenfunken. Kein Aufwand, wenn gar nicht im
        // AP-Modus (softAPIP dann false).

        // Extend the dwell time in AP mode (see softAPIPstart/WAIT_15m in
        // uhr3.ino): every form action (save, etc.) counts as active use, so
        // the 15-minute restart shouldn't interrupt an ongoing configuration.
        // No cost when not even in AP mode (softAPIP is false then).
        if (softAPIP) softAPIPstart = millis();

        // Zugriffs-IP mitloggen (siehe precise-log-messages-Konvention) -
        // deckt Formular-Aktionen (Speichern etc.) ab, beginPage() unten
        // deckt normale Seitenaufrufe (GET) ab. Bewusst NICHT bei jeder
        // API-Antwort (z.B. /api/currentTime, /api/topbarStatus) - die
        // pollen alle paar Sekunden automatisch im Hintergrund und wuerden
        // das Log mit Eintraegen ohne echten Erkenntniswert zuspammen.

        // Log the accessing IP too (see the precise-log-messages convention)
        // - covers form actions (save, etc.), beginPage() below covers
        // normal page views (GET). Deliberately NOT on every API response
        // (e.g. /api/currentTime, /api/topbarStatus) - those poll
        // automatically in the background every few seconds and would spam
        // the log with entries that carry no real information.
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


    // Uebernimmt eine komplette neue WLAN-Liste (SSID+Passwort je Slot, wie
    // vom Formular oder aus einem bestaetigten Aenderungsantrag kommend) in
    // Preferences und die RAM-Arrays wifiSsid[]/wifiPass[]. Ein leeres
    // Passwortfeld laesst das gespeicherte Passwort dieses Slots unveraendert
    // (siehe Hinweistext "Leave empty to keep current" im WLAN-Tab); ein
    // leeres SSID-Feld wird beim anschliessenden Kompaktieren als geloescht
    // behandelt. Gemeinsame Logik von /save (sofortiger Fall: nicht das
    // aktive Netzwerk betroffen) und /factoryReset/confirm (Aktion
    // "wlanOverwriteActive", nachdem das aktive Netzwerk betroffen war und
    // der angezeigte Code bestaetigt wurde - siehe pendingWifiSsid[]/
    // pendingWifiPass[] in globals.h).

    // Applies a complete new WiFi list (SSID+password per slot, as coming
    // from the form or a confirmed change request) to preferences and the
    // RAM arrays wifiSsid[]/wifiPass[]. An empty password field leaves that
    // slot's stored password unchanged (see the "Leave empty to keep
    // current" hint in the WiFi tab); an empty SSID field is treated as
    // deleted during the subsequent compaction. Shared logic between /save
    // (immediate case: the active network isn't affected) and
    // /factoryReset/confirm (action "wlanOverwriteActive", once the active
    // network was affected and the displayed code was confirmed - see
    // pendingWifiSsid[]/pendingWifiPass[] in globals.h).

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
                effectivePass[i] = preferences.getString(pkPass(i).c_str(), "");
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
    }


    // Fuehrt eine der acht bestaetigten bzw. (aus privatem Netz) direkt
    // erlaubten Aktionen tatsaechlich aus - ausgelagert aus
    // /factoryReset/confirm, damit /factoryReset/requestCode, /deletewifi,
    // /save und /api/connectWifi sie bei Zugriff aus einem privaten Netz
    // auch OHNE den Code-Umweg aufrufen koennen (Code-Bestaetigung ist
    // dann nur noch fuer Anfragen aus einem NICHT-privaten Netz noetig -
    // das eigene Heimnetz gilt bereits als hinreichend vertrauenswuerdig).
    // Erwartet, dass ein evtl. benoetigtes Payload (pendingWifiChangeIndex
    // bzw. pendingWifiSsid[]/pendingWifiPass[]) vom Aufrufer schon gesetzt wurde.

    // Actually executes one of the eight confirmed, or (from a private
    // network) directly allowed, actions - factored out of
    // /factoryReset/confirm so /factoryReset/requestCode, /deletewifi,
    // /save and /api/connectWifi can also call it without the code detour
    // when accessed from a private network (code confirmation is then
    // only needed for requests from a NON-private network - one's own
    // home network already counts as sufficiently trustworthy). Expects
    // that any needed payload (pendingWifiChangeIndex or
    // pendingWifiSsid[]/pendingWifiPass[]) has already been set by the caller.

    void executePendingAction(String action) {

        // Einen evtl. noch anhaengigen (fremden) Bestaetigungscode
        // verwerfen: wurde diese Aktion direkt aus einem privaten Netz
        // ausgefuehrt, waehrend zufaellig noch ein Code fuer eine ANDERE
        // Anfrage aussteht (z.B. von einem frueheren, nicht-privaten
        // Versuch), wuerde checkFactoryResetCodePending() sonst
        // weiterhin die Code-Anzeige statt der Uhrzeit zeigen, obwohl
        // hier bereits eine andere, definitive Aktion ausgefuehrt wurde.
        // Bei Aufruf ueber /factoryReset/confirm ohnehin schon leer
        // (dort vorher geloescht) - dieser Reset ist dann ein
        // wirkungsloses No-op.

        // Discard any still-pending (unrelated) confirmation code: if
        // this action was executed directly from a private network while
        // a code for a DIFFERENT request happened to still be pending
        // (e.g. from an earlier, non-private attempt),
        // checkFactoryResetCodePending() would otherwise keep showing
        // the code screen instead of the time, even though a different,
        // definitive action has now already been taken. Already empty
        // when called via /factoryReset/confirm (cleared there
        // beforehand) - this reset is then a no-op.
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
            redirectTo("/factoryReset?msg=Clock%20faces%20deleted");
        }
        else if (action == "hands") {
            resetHandsToDefault();
            redirectTo("/factoryReset?msg=Hand%20sets%20deleted");
        }
        else if (action == "presets") {
            resetAllPresets();
            redirectTo("/factoryReset?msg=Presets%20deleted");
        }
        else if (action == "wlanDeleteActive") {

            // Slot loeschen und Liste kompaktieren - dieselbe Logik wie
            // beim sofortigen Loeschpfad in /deletewifi (nicht-aktive
            // Netzwerke), hier ueber applyWlanList() geteilt. Danach neu
            // starten: die soeben geloeschten Zugangsdaten waren die der
            // aktuell laufenden Verbindung, ein sauberer
            // Neuverbindungsversuch mit den verbleibenden gespeicherten
            // Netzwerken (oder WPS/AP-Modus) ist daher angebracht statt
            // die alte Verbindung einfach weiterlaufen zu lassen.

            // Delete the slot and compact the list - same logic as the
            // immediate deletion path in /deletewifi (non-active
            // networks), shared here via applyWlanList(). Reboot
            // afterwards: the credentials just deleted were those of the
            // currently running connection, so a clean reconnect attempt
            // with the remaining saved networks (or WPS/AP mode) is
            // appropriate instead of just letting the old connection
            // keep running.
            int idx = pendingWifiChangeIndex;
            pendingWifiChangeIndex = -1;

            if (idx >= 0 && idx < MAX_WLAN) {
                String newSsid[MAX_WLAN];
                String newPass[MAX_WLAN];
                for (int i = 0; i < MAX_WLAN; i++) {
                    newSsid[i] = preferences.getString(pkSsid(i).c_str(), "");
                    newPass[i] = preferences.getString(pkPass(i).c_str(), "");
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
                redirectTo("/?tab=wlan&msg=Nothing%20to%20delete");
            }
        }
        else if (action == "wlanOverwriteActive") {

            // Die komplette, im Formular abgeschickte WLAN-Liste
            // uebernehmen (applyWlanList() - dieselbe Logik wie beim
            // sofortigen Speicherpfad in /save, wenn der aktive Slot
            // NICHT betroffen ist). pendingWifiSsid[]/pendingWifiPass[]
            // danach loeschen, damit kein Klartext-Passwort laenger als
            // noetig im RAM steht. Neu starten wie bei "wlanDeleteActive"
            // oben - die soeben ueberschriebenen Zugangsdaten waren die
            // der aktuell laufenden Verbindung.

            // Apply the complete WiFi list submitted by the form
            // (applyWlanList() - same logic as the immediate save path
            // in /save when the active slot is NOT affected).
            // Clear pendingWifiSsid[]/pendingWifiPass[] afterwards, so no
            // plaintext password lingers in RAM longer than necessary.
            // Reboot as with "wlanDeleteActive" above - the credentials
            // just overwritten were those of the currently running
            // connection.
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

            // Wechselt auf den gemerkten Slot, sofern er noch existiert
            // (koennte zwischenzeitlich geloescht worden sein) - gleiches
            // Prinzip wie "wlanDeleteActive" oben: pendingWifiChangeIndex
            // zeigt auf den betroffenen Slot.

            // Switches to the remembered slot, provided it still exists
            // (could have been deleted in the meantime) - same principle
            // as "wlanDeleteActive" above: pendingWifiChangeIndex points
            // at the affected slot.
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
                redirectTo("/?tab=wlan&msg=Network%20no%20longer%20available");
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


    // Baut den wiederverwendeten inneren Teil des Helligkeits-Formulars -
    // vermeidet die fruehere Duplizierung an mehreren Stellen. Umschliessendes
    // <form>/<div> und Save-Button bleiben bei den Aufrufern.

    // Builds the reused inner part of the brightness form - avoids the
    // previous duplication in several places. Surrounding <form>/<div> and
    // the save button stay with the callers.

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

        // API zum Setzen von Zifferblatt, Zeigersatz, Zeitzone, Mittelpunkt-Groesse/-Farbe, Bahnhofsmodus ("wartet auf 12"), Rotation, Sekundenzeiger-Sichtbarkeit/-Stil und sanftem Minutenzeiger
        // API to set clock face, hand set, timezone, hub size/color, station mode ("waits at 12"), rotation, second hand visibility/style, and smooth minute hand

        // DB
        // http://192.168.0.214/api/setMode?face=face_db_uhr.bmp&handSet=0&hubSize=6&hubColor=ff0000showSecondHand=1&stationMode=true&smoothMinute=false&smoothSecond=true&rotation=2

        // Irish Pub
        // http://192.168.0.214/api/setMode?face=face_irish_pub.bmp&handSet=0&hubSize=2&hubColor=aaaaaa&showSecondHand=false&stationMode=false&smoothMinute=true&smoothSecond=false&rotation=2



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

            if (webserver.hasArg("rotation")) {
                String rotationArg = webserver.arg("rotation");
                // Bugfix: hiess vorher ebenfalls "tftRotation" und ueberschattete
                // damit die globale Variable, die renderClockFrame() liest.

                // Bugfix: this used to also be named "tftRotation" and shadowed
                // the global variable that renderClockFrame() reads.

                // Als long fuehren, nicht als uint8_t: sonst wuerde z.B.
                // "rotation=256" vor der Pruefung stillschweigend zu 0 abgeschnitten.

                // Keep as long, not uint8_t: otherwise e.g. "rotation=256" would
                // silently truncate to 0 before validation.
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

                    // Relativer Pfad statt absoluter IP: die gespeicherte
                    // Preset-URL enthaelt aus externen Gruenden (siehe
                    // "Copy link"/ipLink weiter unten - fuer Home-Automation
                    // o.ae. gedacht) immer eine feste IP. Als anklickbarer
                    // In-Page-Link fuehrte das faelschlich zurueck auf die
                    // LOKALE IP, selbst wenn die Seite z.B. ueber eine DMZ-/
                    // Port-Weiterleitung von aussen aufgerufen wurde. Ein
                    // relativer Pfad loest sich dagegen immer gegen die
                    // Adresse auf, mit der die Seite tatsaechlich gerade
                    // erreicht wurde.

                    // Relative path instead of an absolute IP: the stored
                    // preset URL always contains a fixed IP for external
                    // reasons (see "Copy link"/ipLink further below - meant
                    // for home automation etc.). As an in-page clickable
                    // link, that incorrectly led back to the LOCAL IP, even
                    // when the page itself was reached from outside via a
                    // DMZ/port forward, say. A relative path, by contrast,
                    // always resolves against whatever address the page was
                    // actually reached with.
                    if (displayUrl.startsWith("http://")) {
                        int pathStart = displayUrl.indexOf('/', 7); // Suche nach dem Beginn des Pfads (nach der IP)
                                                                    // find the start of the path (after the IP)
                        displayUrl = (pathStart != -1) ? displayUrl.substring(pathStart) : "/";
                    }
                    displayUrl += "&source=preset";
                    // Bugfix: presets[i].name.replace() direkt hier mutierte den
                    // globalen Preset-Zustand bei einem reinen GET-Request. Nicht
                    // mehr noetig - presetName unten ist eine lokale Kopie.

                    // Bugfix: presets[i].name.replace() directly here mutated the
                    // global preset state on a plain GET request. No longer
                    // needed - presetName below is a local copy.

                    // presets[i].name kommt vom Nutzer - potentiell gespeichertes
                    // XSS ohne Escaping (escapeHtmlText()/escapeForJsStringInAttr()).

                    // presets[i].name comes from the user - potential stored XSS
                    // without escaping (escapeHtmlText()/escapeForJsStringInAttr()).
                    String safePresetNameText = escapeHtmlText(presets[i].name);
                    chunk += "<div style='text-align:center;border:1px solid #ccc;border-radius:6px;padding:8px;width:220px;'>";
                    chunk += "<a href='" + displayUrl + "'><img src='/presetpreview?index=" + String(i) + "' style='width:90px;height:90px;'></a>";
                    chunk += "<br><a href='" + displayUrl + "'>" + safePresetNameText + "</a>";
                    String presetName = presets[i].name;
                    presetName.replace(" ", "_"); // Ersetze Leerzeichen durch Unterstriche
                                                  // replace spaces with underscores

                    // "Copy link" verwendet den Host-Header DIESER Anfrage statt
                    // der festen lokalen IP: der Host-Header enthaelt genau die
                    // Adresse, mit der die Seite gerade tatsaechlich aufgerufen
                    // wurde (LAN-IP, mDNS-Name, oder - bei einer Port-
                    // Weiterreichung auf Router-Ebene - die externe DMZ-Adresse
                    // samt Port) und wird von einer solchen Weiterleitung nicht
                    // veraendert. So kopiert man immer einen Link, der von dort
                    // aus erreichbar ist, wo man sich gerade befindet, statt
                    // immer die lokale IP zu bekommen, selbst wenn man von
                    // ausserhalb zugreift. Fallback auf ipAddress, falls der
                    // Header ausnahmsweise leer sein sollte.

                    // "Copy link" uses THIS request's host header instead of the
                    // fixed local IP: the host header carries exactly the
                    // address the page was actually reached with (LAN IP, mDNS
                    // name, or - with a router-level port forward - the
                    // external DMZ address with its port) and a port forward
                    // doesn't alter it. This way the copied link is always
                    // reachable from wherever the user currently is, instead of
                    // always getting the local IP even when accessing from
                    // outside. Falls back to ipAddress if the header should
                    // ever come back empty.
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
            chunk += "<p>" + translate("Presets used") + ": " + String(rowCount) + " / " + String(MAX_PRESETS) + "</p>";
            chunk += "</div><hr>";

            chunk += "<button type='button' id='ghPresetBtn' onclick='loadPresetsFromGithub()'>" + translate("Load Presets from GitHub") + "</button>";
            chunk += "<div id='ghPresetStatus'></div>";

            // Fuer den JS-Merge-Abgleich: vorhandene Preset-Namen und Dateien
            // bereitstellen, damit der GitHub-Download nur Neues hinzufuegt.

            // For the JS merge comparison: provide existing preset names and
            // files, so the GitHub download only adds what's new.
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
            // escapeForJsStringLiteral(): Preset-/Dateinamen (Nutzereingabe) landen
            // hier als JS-String-Literal - ohne Escaping gespeichertes XSS moeglich.

            // escapeForJsStringLiteral(): preset/file names (user input) end up
            // here as a JS string literal - without escaping, stored XSS is possible.
            chunk += "var existingPresetNames = [";
            for (size_t gi = 0; gi < existingPresetNamesForJs.size(); gi++) {
                if (gi > 0) chunk += ",";
                chunk += "\"" + escapeForJsStringLiteral(existingPresetNamesForJs[gi]) + "\"";
            }
            chunk += "];";
            chunk += "var existingFacesForPresets = [";
            for (size_t gi = 0; gi < existingFacesForJs.size(); gi++) {
                if (gi > 0) chunk += ",";
                chunk += "\"" + escapeForJsStringLiteral(existingFacesForJs[gi]) + "\"";
            }
            chunk += "];";
            chunk += "var existingHandsForPresets = [";
            for (size_t gi = 0; gi < existingHandsForJs.size(); gi++) {
                if (gi > 0) chunk += ",";
                chunk += "\"" + escapeForJsStringLiteral(existingHandsForJs[gi]) + "\"";
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

            // Ohne vorhandene Presets automatisch fragen, ob welche von GitHub
            // geladen werden sollen - prueft Erreichbarkeit vorher mehrfach.

            // Without any presets, automatically ask whether to load some from
            // GitHub - checks reachability multiple times first.
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
                    // Host-Teil durch Platzhalter ersetzen - wird beim Import ohnehin
                    // durch die dann aktuelle IP ersetzt (siehe loadPresets()).

                    // Replace the host part with a placeholder - gets replaced by
                    // the then-current IP on import anyway (see loadPresets()).
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

            // Nur aus einem privaten Netz erlaubt (siehe isPrivateNetworkIp()
            // oben) - ein von aussen jederzeit ausloesbarer Neustart ist ein
            // DoS-Vektor (Dauerreboot) und riskiert, einen laufenden
            // Schreibvorgang (Presets speichern, GitHub-Download) zu unterbrechen.

            // Only allowed from a private network (see isPrivateNetworkIp()
            // above) - a restart triggerable from outside at any time is a
            // DoS vector (endless reboot loop) and risks interrupting an
            // in-progress write (saving presets, a GitHub download).
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

            // Kompaktierte Liste auch in die Preferences zurueckschreiben -
            // sonst laufen RAM und Preferences auseinander und geloeschte/
            // verschobene Eintraege tauchen nach einem Neustart wieder an
            // ihrer alten Position auf. Nur bei tatsaechlicher Aenderung
            // schreiben, wie beim analogen Rocrail-Serverlisten-Handler oben.

            // Write the compacted list back to preferences too - otherwise
            // RAM and preferences drift apart and deleted/reordered entries
            // reappear at their old position after a restart. Only write on
            // an actual change, as in the analogous Rocrail server list
            // handler above.
            for (int i = 0; i < MAX_WLAN; i++) {
                String ntpKey = pkNtpServer(i);
                if (preferences.getString(ntpKey.c_str(), "") != String(ntpServers[i])) {
                    preferences.putString(ntpKey.c_str(), ntpServers[i]);
                }
            }

            // Faellt die Liste dadurch komplett leer, sofort auf die
            // eingebauten Standardserver zurueckfallen (siehe time_sync.h) -
            // sonst blieb die Uhr bis zum naechsten Sync-Versuch ohne NTP,
            // obwohl startNtpSyncTask() gleich im Anschluss aufgerufen wird.

            // If this leaves the list completely empty, fall back to the
            // built-in default servers immediately (see time_sync.h) -
            // otherwise the clock would be without NTP until the next sync
            // attempt, even though startNtpSyncTask() is called right after.
            applyNtpServerDefaultsIfNoneConfigured();


            if (webserver.hasArg("timezone")) {
                String tz = webserver.arg("timezone");
                preferences.putString(PK_TIMEZONE, tz);

                // Globale timezone-Variable ebenfalls aktualisieren (wie im
                // anderen Speicherpfad oben) - vorher aktualisierte das ein
                // impliziter Nebeneffekt von setupNTP() (das die Preference
                // beim Start neu einliest); seit der Async-Umstellung
                // (startNtpSyncTask()) liest die Task nur noch einen Snapshot
                // (siehe timezoneSnapshot in globals.h), daher hier explizit -
                // sonst wuerde die Einstellungsseite die alte Zeitzone zeigen.

                // Also update the global timezone variable (as the other save
                // path above already does) - this used to happen as an
                // implicit side effect of setupNTP() (which re-read the
                // preference on start); since the async conversion
                // (startNtpSyncTask()), the task only reads a snapshot (see
                // timezoneSnapshot in globals.h), so this must be explicit
                // here now - otherwise the settings page would keep showing
                // the old timezone.
                timezone = tz;

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

            // "from" ans Formular (versteckes Feld) und den Abbrechen-Link
            // durchreichen, damit /rename und ein Klick auf "Abbrechen" zur
            // selben Seite zurueckkehren, von der aus umbenannt wurde (siehe
            // fileManagerReturnTarget()).

            // Pass "from" through to the form (hidden field) and the Cancel
            // link, so /rename and a click on "Cancel" return to the same
            // page renaming was started from (see fileManagerReturnTarget()).
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

            // Nur aus einem privaten Netz erlaubt (siehe isPrivateNetworkIp()
            // oben und die analoge Begruendung bei /delete) - /rename_form
            // (die reine Formularanzeige) bleibt dagegen frei erreichbar, da
            // sie fuer sich genommen nichts veraendert.

            // Only allowed from a private network (see isPrivateNetworkIp()
            // above and the analogous reasoning at /delete) - /rename_form
            // (the plain form display) stays freely reachable, since it
            // doesn't change anything by itself.
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

                // Neuer Dateiname wurde bisher ungeprueft uebernommen -
                // Dateinamen landen ungeescaped in HTML-Attributen (/files,
                // /listfilesFaces, /handsets), ein "'" oder "<" haette dort gespeichertes XSS ermoeglicht.

                // The new filename was previously accepted unchecked -
                // filenames get embedded unescaped into HTML attributes
                // (/files, /listfilesFaces, /handsets), a "'" or "<" would have enabled stored XSS there.
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

            stationMode = preferences.getBool(PK_STATION_MODE, false);
            showSecondHand = preferences.getBool(PK_SHOW_SECOND_HAND, true);
            smoothMinute = preferences.getBool(PK_SMOOTH_MINUTE, false);
            smoothSecond = preferences.getBool(PK_SMOOTH_SECOND, false);


            stationMode = webserver.hasArg("stationMode");
            showSecondHand = webserver.hasArg("showSecondHand");
            smoothMinute = webserver.hasArg("smoothMinute");
            smoothSecond = webserver.hasArg("smoothSecond");


            useTouch = webserver.hasArg("useTouch");
            preferences.putBool(PK_USE_TOUCH, useTouch);

            if (useTouch) enableTouch();
            else disableTouch();

            // Logging-Einstellung speichern - wird sie dabei gerade
            // ausgeschaltet (vorher an, jetzt aus), alle Logdateien loeschen
            // und den Rotations-Zaehler zuruecksetzen (deleteAllLogFiles()
            // erledigt beides, siehe system_utils.h), statt sie ungenutzt
            // liegen zu lassen, bis Logging irgendwann wieder aktiviert wird.

            // Save the logging setting - if it's being switched off right now
            // (was on, now off), delete all log files and reset the rotation
            // counter (deleteAllLogFiles() does both, see system_utils.h),
            // instead of leaving them sitting around unused until logging
            // gets re-enabled at some point.
            bool wasLoggingEnabled = preferences.getBool(PK_LOGGING_ENABLED, false);
            loggingEnabled = webserver.hasArg("loggingEnabled");
            preferences.putBool(PK_LOGGING_ENABLED, loggingEnabled);

            if (wasLoggingEnabled && !loggingEnabled) {
                deleteAllLogFiles();

                // Gepufferte, noch nicht geschriebene Log-Zeilen verwerfen -
                // Logging wurde gerade abgeschaltet, ein spaeterer Flush
                // wuerde sie sonst in eine Datei schreiben, die der Nutzer
                // eben bewusst geloescht hat.

                // Discard any buffered, not-yet-written log lines - logging
                // was just switched off, a later flush would otherwise write
                // them into a file the user just deliberately deleted.
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
#if defined(DCF77_DATAPIN) && defined(DCF77_INTERRUPT)
            if (dcf77Confirmed) {
                dcfSyncLedEnabled = webserver.hasArg("dcfSyncLed");
                preferences.putBool(PK_DCF_SYNC_LED, dcfSyncLedEnabled);
            }
#endif

            wifiActive = webserver.hasArg("wifiActive");
            preferences.putBool(PK_WIFI_ACTIVE, wifiActive);

            // Rocrail-Master-Schalter: schaltet Nutzung und Sichtbarkeit des
            // Rocrail-Tabs frei (siehe generateSettingsTabNav()). Beim
            // Deaktivieren eine evtl. offene Verbindung sofort trennen.

            // Rocrail master switch: unlocks use and visibility of the
            // Rocrail tab (see generateSettingsTabNav()). When disabling,
            // immediately close any open connection.
            rocrailEnabled = webserver.hasArg("rocrailEnabled");
            preferences.putBool(PK_ROCRAIL_ENABLED, rocrailEnabled);
            if (!rocrailEnabled) {
                if (rocrailClient.connected()) rocrailClient.stop();
                rocrailConnected = false;

                // R2RNet-Multicast-Diagnose sofort verlassen statt bis zum
                // naechsten Reconnect/Neustart weiterlaufen zu lassen (siehe
                // startR2rnetDebugListener()/pollR2rnetDebugListener() in
                // rocrail_client.h) - sonst bliebe der Socket nach dem
                // Deaktivieren unnoetig offen.

                // Leave the R2RNet multicast diagnostic listener immediately
                // instead of letting it keep running until the next
                // reconnect/restart (see startR2rnetDebugListener()/
                // pollR2rnetDebugListener() in rocrail_client.h) - otherwise
                // the socket would stay needlessly open after disabling.
                r2rnetDebugUdp.stop();
                r2rnetDebugListening = false;
            }
            else {
                // Sofort versuchen statt bis zum naechsten regulaeren
                // Zeitfenster zu warten (siehe triggerRocrailConnectNow()) -
                // no-op, falls z.B. noch keine Serveradresse hinterlegt ist.

                // Try immediately instead of waiting for the next regular
                // window (see triggerRocrailConnectNow()) - a no-op if e.g.
                // no server address is configured yet.
                triggerRocrailConnectNow();

                // R2RNet-Multicast-Diagnose sofort mit starten statt erst
                // beim naechsten Reconnect/Neustart (siehe
                // startR2rnetDebugListener() in rocrail_client.h).

                // Also start the R2RNet multicast diagnostic listener right
                // away instead of only at the next reconnect/restart (see
                // startR2rnetDebugListener() in rocrail_client.h).
                startR2rnetDebugListener();
            }

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
                    if (requestedRotation >= 0 && requestedRotation <= TFT_ROTATION_NA) {
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
                chunk += "    x: avg,\n";
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
                chunk += "    x: avg,\n";
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
                // Bestehende Verbindung trennen, damit connectRocrailClient()
                // sofort den neuen Server versucht. Anlagenname muss nicht
                // geleert werden - haengt jetzt am Listenplatz, nicht mehr am globalen aktiven Server.

                // Disconnect the existing connection, so connectRocrailClient()
                // tries the new server immediately. Layout name doesn't need
                // clearing - it now lives per list slot, not on the global active server.
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
                chunk += " <td><a href = '/delete?file=" + name + "&from=files' title='" + translate("Delete") + "' onclick = 'return confirm(\"" + translate("Delete") + " " + name + "?\")'>&#128465;&#65039;</a> ";
                // Scale-Option nur für .bmp-Dateien anzeigen
                // Show the scale option only for .bmp files
                if (name.endsWith(".bmp")) {
                    chunk += "<a href = '/scalebmp_form?file=" + name + "' title='" + translate("Scale") + "'>&#128208;</a> ";
                    chunk += "<a href='/rename_form?file=" + name + "&from=files' title='" + translate("Rename") + "'>&#9999;&#65039;</a> ";
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

                // Auf den Stand nach einem evtl. eingebetteten Nullbyte
                // kappen: String::endsWith() prueft die LOGISCHE Laenge des
                // Arduino-String (kann ein Nullbyte enthalten), waehrend
                // LittleFS.exists()/open() intern ueber .c_str() (also
                // nullterminiert) zugreifen. Ohne diese Normalisierung
                // koennte ".../log_1.log\0.bmp" die Endungspruefung als
                // ".bmp" bestehen, aber tatsaechlich die Logdatei oeffnen.

                // Truncate at any embedded null byte: String::endsWith()
                // checks the Arduino String's LOGICAL length (which can
                // contain a null byte), while LittleFS.exists()/open()
                // internally go through .c_str() (i.e. null-terminated).
                // Without this normalization, ".../log_1.log\0.bmp" could
                // pass the extension check as ".bmp" while actually opening
                // the log file.
                path = String(path.c_str());

                // Logdateien nur aus einem privaten Netz herunterladbar (siehe
                // isPrivateNetworkIp() oben und /api/currentLog) - koennen
                // IP-Adressen, SSIDs u.ae. enthalten. Andere Dateitypen (BMPs
                // usw.) bleiben unveraendert von ueberall herunterladbar.

                // Log files only downloadable from a private network (see
                // isPrivateNetworkIp() above and /api/currentLog) - can
                // contain IP addresses, SSIDs, etc. Other file types (BMPs
                // etc.) remain downloadable from anywhere, unchanged.
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

            // Nur bei Zugriff aus einem privaten Netz anzeigen (siehe
            // isPrivateNetworkIp() weiter oben) - zeigt u.a. WLAN-Modus,
            // Reset-Grund, Speicherbelegung und Rocrail-Serveradresse, was
            // von aussen nicht einsehbar sein soll. Frueher Ausstieg hier
            // moeglich (anders als beim Status-TAB auf "/"), da diese Seite
            // fuer sich alleine steht statt in eine groessere Seite eingebettet zu sein.

            // Only shown on access from a private network (see
            // isPrivateNetworkIp() further above) - shows things like WiFi
            // mode, reset reason, storage usage and the Rocrail server
            // address, which shouldn't be visible from the outside. An
            // early exit is possible here (unlike the Status TAB on "/"),
            // since this page stands on its own instead of being embedded in a larger page.
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
            chunk += "<li>" + generateStorageInfo(LittleFS.usedBytes(), LittleFS.totalBytes(), true) + "</li>";

            String tzLabel = preferences.getString(PK_TIMEZONE, "DE");
            String tzDesc;

            tzDesc = tzLabel;

            // Lokale Kopie statt der globalen timeinfo: die wird auch vom
            // Haupt-Loop (updateClock()/updateBrightness(), siehe display.h)
            // gelesen/geschrieben - ein Zugriff hier (Webserver-Kontext)
            // sollte sie bei einem Fehlschlag nicht mit einer ungueltigen
            // Zwischenzeit ueberschreiben (siehe Kommentar bei updateClock()).

            // Local copy instead of the global timeinfo: that one is also
            // read/written by the main loop (updateClock()/
            // updateBrightness(), see display.h) - a read here (web server
            // context) shouldn't overwrite it with an invalid intermediate
            // time on failure (see the comment at updateClock()).
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

            webserver.sendContent(chunk);
            chunk = "";

            chunk += "<li>Compiled on: <strong>" + (String)version + "</strong></li><br>";

            chunk += "<li>TFT Driver: " + tftType + "</li>";

            chunk += "<li>TFT Size: " + String(TFT_WIDTH) + " x " + String(TFT_HEIGHT) + "</li>";

            chunk += "<br>";

            webserver.sendContent(chunk);
            chunk = "";

            chunk += "<li>Chip Model: " + String(ESP.getChipModel()) + "</li>";
            chunk += "<li>Chip Revision: " + String(ESP.getChipRevision()) + "</li>";
            chunk += "<li>Chip Cores: " + String(ESP.getChipCores()) + "</li>";
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
            chunk += "<li>Signal Strength (RSSI): " + String(WiFi.RSSI()) + " dBm</li>";

            // ntpServerRunning kommt vom echten Rueckgabewert von udp.begin()
            // (siehe startNtpServer()), nicht nur aus einem blinden Log.

            // ntpServerRunning comes from udp.begin()'s real return value
            // (see startNtpServer()), not just a blind log entry.
            chunk += "<li>NTP Server (own): " + String(ntpServerRunning ? "running on port " + String(NTP_PORT) : "not running") +
                     " - requests: " + String(ntpRequestsReceived) + ", answered: " + String(ntpRepliesSent) + "</li><br>";

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
                chunk += "<li>I2C SCL GPIO: " + String(SCL_PIN) + "</li><br>";
            }
            else {
                chunk += "<li>I2C: no device found</li><br>";
            }
#endif

            webserver.sendContent(chunk);
            chunk = "";

#if defined DCF77_DATAPIN && defined DCF77_INTERRUPT
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
            chunk += "<li>Use Touch: " + String(useTouch ? "true" : "false") + "</li><br>";
#endif
#ifdef ADC_PIN
            chunk += "<li>ADC_VCC GPIO: " + String(ADC_3V) + "</li>";
            chunk += "<li>ADC (photoresistor) GPIO: " + String(ADC_PIN) + "</li>";
            chunk += "<li>ADC_GND GPIO: " + String(ADC_GND) + "</li>";
            if (photoresistorFound) {
                chunk += "<li>ADC Value: " + String(getAdjustedAdcValue(analogRead(ADC_PIN))) + "</li><br>";
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
       

            chunk += "<li><b>timezone</b>: " + preferences.getString(PK_TIMEZONE, TIMEZONE_DEFAULT) + "</li>";
            chunk += "<li><b>background</b>: " + preferences.getString(PK_BACKGROUND, "/faces/default") + "</li>";
            chunk += "<li><b>handset</b>: " + preferences.getString(PK_HANDSET, "") + "</li>";
            // Bugfix: getUInt() auf einem mit putLong() geschriebenen Key liefert
            // wegen NVS-Typkonflikt stillschweigend den Default. Ausserdem war das
            // Label falsch: gespeichert wird RGB888, nicht RGB565.

            // Bugfix: getUInt() on a key written with putLong() silently returns
            // the default due to an NVS type mismatch. The label was also wrong:
            // what's stored is RGB888, not RGB565.
            chunk += "<li><b>centerColor (RGB888)</b>: " + String(preferences.getLong(PK_CENTER_COLOR, 0xEC0016), HEX) + "</li>";
            chunk += "<li><b>centerSize</b>: " + String(preferences.getUInt(PK_CENTER_SIZE, 6)) + "</li>";

            uint8_t rotation = preferences.getUChar(PK_TFT_ROTATION1, TFT_ROTATION1_DEFAULT);
            chunk += "<li><b>tftRotation1</b>: " + rotationLabelHtml(rotation) + "</li>";
            {
                uint8_t rotation2 = preferences.getUChar(PK_TFT_ROTATION2, TFT_ROTATION2_DEFAULT);
                chunk += "<li><b>tftRotation2</b>: " + rotationLabelHtml(rotation2) + "</li>";
            }

            // Rotationsmodus zeigt, WIE die Werte angewendet werden: beim GC9D01
            // ist Hardware-Rotation wirkungslos, nur mit PSRAM wird auf
            // Software-Rotation umgeschaltet (siehe gc9d01SwRotation in uhr3.ino).

            // Rotation mode shows HOW the values are applied: on the GC9D01
            // hardware rotation has no effect, only with PSRAM does it switch to
            // software rotation (see gc9d01SwRotation in uhr3.ino).
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
        
            bool stationModeStatus = preferences.getBool(PK_STATION_MODE, true);
            chunk += "<li><b>stationMode</b>: " + String(stationModeStatus ? "true" : "false") + "</li>";
            chunk += "<li><b>smoothSecond</b>: " + String(getSmoothSecondPref(stationModeStatus) ? "true" : "false") + "</li>";
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
            if (preferences.getBool(PK_ROCRAIL_ENABLED, false)) {
                chunk += "<li><b>rocrailEnabled</b>: " + String(preferences.getBool(PK_ROCRAIL_ENABLED, false) ? "true" : "false") + "</li>";
                chunk += "<li><b>rocrailServer</b>: " + preferences.getString(PK_ROCRAIL_SERVER, "") + "</li>";
                chunk += "<li><b>rocrailServerPort</b>: " + String(preferences.getUShort(PK_ROCRAIL_SRV_PORT, ROCRAIL_DEFAULT_PORT)) + "</li>";
            }
            chunk += "</ul>";
            chunk += "</br>";
            chunk += "<li>Contact: <a href='mailto:howl@gmx.de'>howl@gmx.de</a></li>";

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

        // Vorschaubild aus den aktuell aktiven Einstellungen, ohne Zeiger, als BMP -
        // Hintergrund fuer das Live-Zeiger-Widget, spart ~150 KB Base64-Inline-Daten.

        // Preview image from the currently active settings, without hands, as a
        // BMP - background for the live hand widget, avoids ~150 KB inline base64.

        // Liefert die tatsaechliche ESP32-Zeit, danach laeuft die Anzeige lokal
        // im Browser weiter (performance.now()), ohne staendiges Nachfragen.

        // Returns the ESP32's actual time, afterwards the display keeps running
        // locally in the browser (performance.now()), without constant polling.

        // Inline erzeugtes SVG-Icon als Favicon, damit kein extra 404 fuer
        // "/favicon.ico" anfaellt. Lange Cache-Zeit, da es sich nie aendert.

        // Inline-generated SVG icon as favicon, so "/favicon.ico" no longer
        // causes an extra 404. Long cache lifetime, since it never changes.
        webserver.on("/favicon.ico", HTTP_GET, []() {
            webserver.sendHeader("Cache-Control", "public, max-age=86400");
            String svg = "<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 32 32'>"
                         "<circle cx='16' cy='16' r='14' fill='#10151a' stroke='#f5a623' stroke-width='2.5'/>"
                         "<line x1='16' y1='16' x2='16' y2='7' stroke='#f5a623' stroke-width='2.5' stroke-linecap='round'/>"
                         "<line x1='16' y1='16' x2='21' y2='16' stroke='#f5a623' stroke-width='2.5' stroke-linecap='round'/>"
                         "</svg>";
            webserver.send(200, "image/svg+xml", svg);
            });

        // Weist brave Crawler (Google, Bing usw.) an, die Weboberflaeche
        // nicht zu indexieren - kein Zugriffsschutz (Portscanner ignorieren
        // das ohnehin), aber verhindert, dass die Uhr bei versehentlicher
        // oeffentlicher Erreichbarkeit (siehe isPrivateNetworkIp()) in
        // Suchergebnissen auftaucht. Lange Cache-Zeit, da es sich nie aendert.

        // Tells well-behaved crawlers (Google, Bing, etc.) not to index the
        // web interface - not an access control (port scanners ignore this
        // anyway), but prevents the clock from showing up in search results
        // if it's ever reachable publicly by accident (see
        // isPrivateNetworkIp()). Long cache lifetime, since it never changes.
        webserver.on("/robots.txt", HTTP_GET, []() {
            webserver.sendHeader("Cache-Control", "public, max-age=86400");
            webserver.send(200, "text/plain", "User-agent: *\nDisallow: /\n");
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
            const struct tm& t = rocrailTimeReady ? rocrailTimeinfo : timeinfo;

            String json = "{\"hour\":" + String(t.tm_hour) +
                          ",\"minute\":" + String(t.tm_min) +
                          ",\"second\":" + String(t.tm_sec) +
                          ",\"rocrail\":" + String(rocrailTimeReady ? "true" : "false") +
                          ",\"divider\":" + String(rocrailTimeReady ? rocrailDivider : 1) +
                          ",\"frozen\":" + String((rocrailTimeReady && rocrailFrozen) ? "true" : "false") +
                          ",\"previewSig\":\"" + escapeJsonText(currentPreviewSignature()) + "\"}";
            webserver.send(200, "application/json", json);
            });

        // Liefert den Zustand der Topbar-Status-Punkte als JSON, gepollt vom
        // Live-Status-Skript in generateTopBar() - dieselben Bedingungen wie
        // dort, bei Aenderung dort auch hier anpassen.

        // Returns the topbar status dots' state as JSON, polled by the
        // live-status script in generateTopBar() - same conditions as there,
        // keep both in sync if one changes.

        // rtcPresent/dcf77Present/lightValue: siehe jeweilige Kommentare in
        // generateTopBar() - togglen live per setPresent()/setValue().

        // rtcPresent/dcf77Present/lightValue: see the respective comments in
        // generateTopBar() - toggled live via setPresent()/setValue().
        webserver.on("/api/topbarStatus", HTTP_GET, []() {
            webserver.sendHeader("Cache-Control", "no-store");

            // Nur aus einem privaten Netz mit echten Werten beantworten (siehe
            // isPrivateNetworkIp() oben) - dieses Polling laeuft auf JEDER
            // Seite (siehe generateHtmlHeader(), alle 5s), nicht nur auf dem
            // Status-Tab, und wuerde sonst dessen Sperre umgehen: Zeit-/RTC-/
            // DCF77-Sync-Status und Rocrail-Verbindungsstatus blieben live von
            // aussen einsehbar, obwohl der Status-Tab selbst blockiert ist.
            // Vollstaendiges, aber neutrales JSON zurueckgeben (nicht nur ein
            // leeres Objekt) - das Poll-Skript erwartet alle Felder und wuerde
            // sonst "undefined" anzeigen oder Eintraege in falschem Zustand lassen.

            // Only answer with real values from a private network (see
            // isPrivateNetworkIp() above) - this polling runs on EVERY page
            // (see generateHtmlHeader(), every 5s), not just the Status tab,
            // and would otherwise bypass its block: time/RTC/DCF77 sync
            // status and the Rocrail connection status would stay visible
            // live from the outside even though the Status tab itself is
            // blocked. Return complete but neutral JSON (not just an empty
            // object) - the polling script expects every field and would
            // otherwise show "undefined" or leave entries in the wrong state.
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
#if defined SDA_PIN && defined SCL_PIN
            rtcState = getRtcStatus();
#endif
#if defined DCF77_DATAPIN && defined DCF77_INTERRUPT
            dcfState = getDcf77Status();
#endif
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
#if !defined(DCF77_DATAPIN) || !defined(DCF77_INTERRUPT)
            webserver.send(200, "application/json", "{\"present\":false}");
#else
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
#endif
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

            String activeHandSet = preferences.getString(PK_HANDSET, "");
            String previewSig = currentPreviewSignature();
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
            // Default false, wie ueberall sonst im Projekt - hier stand
            // abweichend "true", wodurch diese Anzeige nach einem Werkreset
            // einen anderen Zustand behauptete als tatsaechlich angewendet.

            // Default false, matching everywhere else in the project - this
            // used to say "true" here, so after a factory reset this
            // display claimed a different state than what was actually applied.
            bool smoothMinuteActive = preferences.getBool(PK_SMOOTH_MINUTE, false);

            // Fallback bewusst stationModeActive statt eines festen Literals -
            // siehe Kommentar bei der smoothSecond-Ladezeile in uhr3.ino.
            // Fallback deliberately stationModeActive instead of a fixed
            // literal - see the comment at the smoothSecond load line in uhr3.ino.
            bool smoothSecondActive = getSmoothSecondPref(stationModeActive);

            float scaleFactor = (float)previewSize / CLOCK_WIDTH;
            int scaledHandWidth = (int)(HAND_WIDTH * scaleFactor + 0.5);
            int scaledHandHeight = (int)(HAND_HEIGHT * scaleFactor + 0.5);
            int scaledPivotX = (int)((HAND_WIDTH / 2.0) * scaleFactor + 0.5);
            int scaledPivotY = (int)((HAND_HEIGHT * 0.77) * scaleFactor + 0.5);
            // hubSize ist ein Radius, der CSS-Kreis braucht aber den Durchmesser -
            // Bugfix: fehlende Verdopplung liess den Punkt halb so gross wirken.

            // hubSize is a radius, but the CSS circle needs the diameter -
            // bugfix: the missing doubling made the hub look half as large.
            int scaledHubSize = (int)(hubSize * 2 * scaleFactor + 0.5);
            if (scaledHubSize < 4) scaledHubSize = 4;

            // Versteckt per Default - JS blendet ihn ein, sobald Rocrail-
            // Modellzeit aktiv ist. Gleicher Banner-Stil wie im Helligkeit-
            // Tab. max-width bewusst fest, damit der Text beim Skalieren der Uhr lesbar bleibt.

            // Hidden by default - JS reveals it once Rocrail model time is
            // active. Same banner style as in the Brightness tab. max-width
            // deliberately fixed, so the text stays readable while the clock is resized.
            chunk += "<div id='rocrailPreviewHint' hidden style='background:#fff3cd;color:#856404;border:1px solid #ffeeba;border-radius:6px;padding:8px 12px;margin:0 auto 10px;max-width:400px;text-align:center;'></div>";

            // Groessenregler: skaliert die fertige Uhr client-seitig per CSS
            // transform:scale() - kein Server-Request, kein Neuaufbau der Zeigerbilder noetig.

            // Size slider: scales the finished clock client-side via CSS
            // transform:scale() - no server request, no need to rebuild the hand images.
            chunk += "<div style='display:flex;align-items:center;justify-content:center;gap:8px;margin-bottom:10px;flex-wrap:wrap;'>";
            chunk += "<label for='previewSizeSlider'>" + translate("Preview Size") + ":</label>";
            chunk += "<input type='range' id='previewSizeSlider' min='" + String(PREVIEW_SIZE_MIN) + "' max='" + String(PREVIEW_SIZE_MAX) + "' step='10' value='" + String(previewSize) + "' style='width:200px;'>";
            chunk += "<span id='previewSizeValue'>" + String(previewSize) + "</span>&nbsp;px";
            chunk += "</div>";

            // previewSizer traegt die tatsaechliche Boxgroesse, previewInner
            // bleibt fest bei previewSize und wird nur per transform:scale()
            // skaliert - die Zeiger-Pixel-Offsets muessen so nicht neu ermittelt werden.

            // previewSizer carries the actual box size, previewInner stays
            // fixed at previewSize and is only scaled via transform:scale() -
            // the hands' pixel offsets don't need to be recalculated this way.
            chunk += "<div id='previewSizer' style='width:" + String(previewSize) + "px;height:" + String(previewSize) + "px;margin:20px auto;'>";
            chunk += "<div id='previewInner' style='width:" + String(previewSize) + "px;height:" + String(previewSize) + "px;transform-origin:top left;'>";
            chunk += "<div style='width:" + String(previewSize) + "px;height:" + String(previewSize) + "px;box-sizing:border-box;border:3px solid #333;border-radius:50%;background:#fff url(/currentfacebg) center/cover no-repeat;overflow:hidden;position:relative;'>";
            chunk += "<div id='liveHandsPivotFull' style='position:absolute;left:50%;top:50%;width:0;height:0;'>";
            chunk += "<img id='liveHourHandFull' src='data:image/png;base64," + hourB64 + "' style='position:absolute;left:-" + String(scaledPivotX) + "px;top:-" + String(scaledPivotY) + "px;width:" + String(scaledHandWidth) + "px;height:" + String(scaledHandHeight) + "px;transform-origin:" + String(scaledPivotX) + "px " + String(scaledPivotY) + "px;'>";
            chunk += "<img id='liveMinuteHandFull' src='data:image/png;base64," + minuteB64 + "' style='position:absolute;left:-" + String(scaledPivotX) + "px;top:-" + String(scaledPivotY) + "px;width:" + String(scaledHandWidth) + "px;height:" + String(scaledHandHeight) + "px;transform-origin:" + String(scaledPivotX) + "px " + String(scaledPivotY) + "px;'>";
            if (showSecond) {
                chunk += "<img id='liveSecondHandFull' src='data:image/png;base64," + secondB64 + "' style='position:absolute;left:-" + String(scaledPivotX) + "px;top:-" + String(scaledPivotY) + "px;width:" + String(scaledHandWidth) + "px;height:" + String(scaledHandHeight) + "px;transform-origin:" + String(scaledPivotX) + "px " + String(scaledPivotY) + "px;'>";
            }
            chunk += "<div id='liveHubFull' style='position:absolute;left:-" + String(scaledHubSize / 2) + "px;top:-" + String(scaledHubSize / 2) + "px;width:" + String(scaledHubSize) + "px;height:" + String(scaledHubSize) + "px;border-radius:50%;background:" + String(hubHex) + ";'></div>";
            chunk += "</div>"; // Ende Zifferblatt-Kreis
                               // end clock-face circle
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
            chunk += "    sizer.style.height = px + 'px';";
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
            chunk += "  var hintEl = document.getElementById('rocrailPreviewHint');";
            chunk += "  var stationMode = " + String(stationModeActive ? "true" : "false") + ";";
            chunk += "  var smoothMinute = " + String(smoothMinuteActive ? "true" : "false") + ";";
            chunk += "  var smoothSecond = " + String(smoothSecondActive ? "true" : "false") + ";";
            chunk += "  var fastSecondMs = " + String((int)FAST_SECOND) + ";";
            chunk += "  var rocrailHideDetailsDivider = " + String(ROCRAIL_HIDE_DETAILS_DIVIDER) + ";";
            chunk += "  var rocrailHintTpl = '" + translate("Showing Rocrail model time ({divider}&times; speed)") + "';";
            chunk += "  var baseH = 0, baseM = 0, baseS = 0, baseAt = 0, haveBase = false;";
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

            // Bugfix: baseAt = performance.now() HIER (nachdem die Antwort
            // schon angekommen und verarbeitet ist) ignorierte die Round-
            // Trip-Zeit des Requests komplett - t.second galt dann faelschlich
            // erst ab diesem spaeteren Zeitpunkt, wodurch die Vorschau um
            // genau diese Zeit (Netzwerk + Serververarbeitung) hinter dem
            // Display nachhinkte. Fix wie bei NTP: der Zeitpunkt, auf den sich
            // t.second bezieht, liegt am ehesten in der MITTE des Requests -
            // bei symmetrischer Latenz gleicht das Hin- und Rueckweg aus.

            // Bugfix: baseAt = performance.now() HERE (after the response had
            // already arrived and been processed) completely ignored the
            // request's round-trip time - t.second was then wrongly treated
            // as only valid from this later point on, making the preview lag
            // behind the display by exactly that time (network + server
            // processing). Fixed like NTP does: the moment t.second actually
            // refers to sits roughly at the MIDPOINT of the request - with
            // symmetric latency that balances out the outbound and return leg.
            chunk += "      var roundTrip = performance.now() - fetchStart;";
            chunk += "      baseH = t.hour; baseM = t.minute; baseS = t.second; baseAt = fetchStart + roundTrip / 2; haveBase = true;";
            // rocrailDivider/-Frozen: siehe /api/currentTime - dieselbe
            // rocrailTimeReady-Bedingung wie in renderClockFrame() (display.h).
            // Ohne aktive Rocrail-Zeit bleibt divider=1, frozen=false.

            // rocrailDivider/-Frozen: see /api/currentTime - the same
            // rocrailTimeReady condition as in renderClockFrame() (display.h).
            // Without an active Rocrail time, divider stays 1, frozen stays false.
            chunk += "      rocrailDivider = t.rocrail ? t.divider : 1;";
            chunk += "      rocrailFrozen = t.rocrail && t.frozen;";
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
            chunk += "    var hourDeg = (h + minuteDeg / 360) * 30;";
            chunk += "    var secDeg;";

            // Wie renderClockFrame() (display.h) seit der Entkopplung:
            // stationMode ("wartet auf 12") und smoothSecond (schwingend/
            // tickend) sind zwei unabhaengige Achsen - siehe dort fuer die
            // ausfuehrliche Begruendung jedes Zweigs.

            // As in renderClockFrame() (display.h) since the decoupling:
            // stationMode ("waits at 12") and smoothSecond (smooth/ticking)
            // are two independent axes - see there for the detailed
            // reasoning behind each branch.
            chunk += "    if (stationMode) {";
            chunk += "      var elapsedMs = (s + ms / 1000) * 1000;";
            // Bugfix: fastSecondMs wurde hier zusaetzlich durch rocrailDivider
            // geteilt, obwohl elapsedMs bereits durch "elapsed" oben mit
            // demselben Divider beschleunigt ist - lief dadurch doppelt so schnell (bei divider 2 4x statt 2x).

            // Bugfix: fastSecondMs used to be additionally divided by
            // rocrailDivider here, even though elapsedMs is already
            // accelerated by that divider via "elapsed" above - ran twice as fast (4x instead of 2x at divider 2).
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
            chunk += "    hourEl.style.transform = 'rotate(' + hourDeg + 'deg)';";
            chunk += "    minuteEl.style.transform = 'rotate(' + minuteDeg + 'deg)';";

            // Wie renderClockFrame() (display.h): zwei unabhaengige Ausblend-
            // Regeln. hideDetails (Divider ueber Schwelle) betrifft Sekunden-
            // zeiger UND Nabe; hideSecondTicking (Divider>1 + tickend) betrifft
            // NUR den Sekundenzeiger, die Nabe bleibt davon unberuehrt.

            // As in renderClockFrame() (display.h): two independent hiding
            // rules. hideDetails (divider above threshold) affects the second
            // hand AND the hub; hideSecondTicking (divider>1 + ticking style)
            // affects ONLY the second hand, the hub is unaffected by it.
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

#if !defined(DCF77_DATAPIN) || !defined(DCF77_INTERRUPT)
            chunk += "<div class='card'>" + translate("Not available") + "</div>";
#else
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
            chunk += "var timer=null;";
            chunk += "function poll(){fetch('/api/dcf77status',{cache:'no-store'}).then(function(r){return r.json();}).then(function(s){paintBits(s.bitIndex,s.bits);renderDecoded(s.decoded);var ed=document.getElementById('dcfEdgeDropped');if(ed)ed.textContent=s.edgeDropped;var rn=document.getElementById('dcfRawNote');if(rn)rn.hidden=!!s.synced;var sy=document.getElementById('dcfSynced');if(sy)sy.textContent=s.synced?SYNC_YES.replace('{pos}',s.markerPos):SYNC_NO;var se=document.getElementById('dcfSeen');if(se)se.textContent=s.pulsesSeen;var mi=document.getElementById('dcfMissed');if(mi)mi.textContent=s.pulsesMissed;var br=document.getElementById('dcfBreaks');if(br)br.textContent=s.phaseBreaks;var pu=document.getElementById('dcfPulses');if(pu){if(!s.pulses||!s.pulses.length){pu.textContent='-';}else{pu.textContent=s.pulses.map(function(p){return p[0]+'/'+p[1];}).join('  ');}}}).catch(function(){});}";
            chunk += "function start(){if(timer)return;poll();timer=setInterval(poll,1000);}";
            chunk += "function stop(){if(!timer)return;clearInterval(timer);timer=null;}";
            chunk += "document.addEventListener('visibilitychange',function(){if(document.hidden)stop();else start();});";
            chunk += "if(!document.hidden)start();";
            chunk += "})();";
            chunk += "</script>";
#endif

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

            // Null-Check: ohne ihn endete eine fehlgeschlagene ~19 KB Allokation
            // in einem Panic-Reset statt einer sauberen Fehlerantwort.

            // Null check: without it a failed ~19 KB allocation ended in a
            // panic reset instead of a clean error response.
            uint8_t* bmpData = new (std::nothrow) uint8_t[fileSize];
            if (!bmpData) {
                DEBUG_PRINTLN("[Preview] Error: couldnt allocate preview buffer for /preview_defaultface (from " + webserver.client().remoteIP().toString() + ")");
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
            chunk += "<p>" + generateStorageInfo(used, total) + "</p>";
            chunk += "<div style='display:flex;flex-wrap:wrap;gap:24px 18px;justify-content:center;align-items:flex-start;'>";

            String activeBackground = preferences.getString(PK_BACKGROUND, "/face_default.bmp");

            // Eingebautes Standard-Zifferblatt hinzufuegen
            // Add built-in default face
            chunk += "<div style='text-align:center;width:100px;'>";
            // Relativer Pfad statt "http://" + ipAddress: In-Page-Links
            // sollen sich immer gegen die aktuell aufgerufene Adresse
            // aufloesen (siehe ausfuehrliche Begruendung bei displayUrl auf
            // der Presets-Seite), nicht fest auf die lokale IP.

            // Relative path instead of "http://" + ipAddress: in-page links
            // should always resolve against the currently used address (see
            // the detailed reasoning at displayUrl on the presets page), not
            // hardcoded to the local IP.
            chunk += "<a href='/setbackground?file=face_default.bmp'>";
            chunk += "<img src='/preview_defaultface' style='width:80px;height:80px;border:1px solid #ccc'>";
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
                // escapeHtmlText(): Dateiname ist frei waehlbar (handleFileUpload()
                // prueft nur Praefix/Suffix), daher unbedingt escapen.

                // escapeHtmlText(): the filename is freely chosen (handleFileUpload()
                // only checks prefix/suffix), so it must be escaped.
                String safeName = escapeHtmlText(name);
                String safeShortName = escapeHtmlText(shortName);
                chunk += "<div style='text-align:center;width:100px;'>";
                chunk += "<a href='/setbackground?file=" + safeShortName + "'>";
                chunk += "<img src='/facepreview?file=" + safeName + "' style='width:80px;height:80px;border:1px solid #ccc'>";
                chunk += "</a><br>" + escapeHtmlText(displayName) + String(isActive ? " (" + translate("active") + ")" : "");
                chunk += "<br><a href='/rename_form?file=" + safeName + "&from=listfilesFaces'>" + translate("Rename") + "</a> ";
                chunk += "<a href='/delete?file=" + safeName + "&from=listfilesFaces' onclick='return confirm(\"" + translate("Delete") + " " + escapeHtmlText(displayName) + "?\")'>" + translate("Delete") + "</a>";
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

            // Browser laedt neue Faces per HTTPS von GitHub und laedt sie per
            // lokalem HTTP zu /upload hoch - die Uhr braucht nie HTTPS.

            // Browser downloads new faces via HTTPS from GitHub and uploads
            // them via local HTTP to /upload - the clock never needs HTTPS.
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

            // CSS-only Tabs: radio-Inputs muessen direkte Geschwister von
            // .tabnav/.panel-* sein. WLAN ist immer vorausgewaehlt - deckt
            // auch das Captive-Portal-Popup ab.

            // CSS-only tabs: radio inputs must be direct siblings of
            // .tabnav/.panel-*. WiFi is always preselected - also covers
            // the captive portal popup.

            // Der ERSTE Tab bekommt "checked", damit immer einer vorausgewaehlt
            // ist (siehe Begruendung oben). Reihenfolge aus SETTINGS_TAB_KEYS.

            // The FIRST tab gets "checked", so one is always preselected (see
            // the reasoning above). Order from SETTINGS_TAB_KEYS.
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

            // Panel Status: identisch zur bisherigen /status-Seite, in eine
            // .card gepackt (900px breit, zentriert, scrollbar) statt volle
            // Seitenbreite - siehe Kommentar an der Kartenoeffnung unten.

            // Status panel: identical to the previous /status page, wrapped
            // in a .card (900px wide, centered, scrollable) instead of full
            // page width - see the comment at the card's opening below.
            chunk += "<div class='tabpanel panel-status'>";
            // Scrollbares Fenster wie beim Log- und Info-Tab (gleiche Hoehe
            // ueber INFO_LOG_WINDOW_HEIGHT_CSS, gleiche 900px-Breite) - vorher
            // wuchs diese Karte ueber die ganze Seite statt nur der Inhalt.

            // Scrollable window like the Log and Info tabs (same height via
            // INFO_LOG_WINDOW_HEIGHT_CSS, same 900px width) - before, this
            // card grew over the full page instead of just the content.
            chunk += "<div class='card' id='statusContent' style='max-width:900px;" INFO_LOG_WINDOW_HEIGHT_CSS "overflow-y:auto;'>";

            // Bei Zugriff von ausserhalb eines privaten Netzes (siehe
            // isPrivateNetworkIp() oben) bleibt der Inhalt verborgen - zeigt
            // sonst u.a. WLAN-Modus, Reset-Grund, Speicherbelegung und
            // Rocrail-Serveradresse, was von aussen nicht einsehbar sein soll.

            // On access from outside a private network (see
            // isPrivateNetworkIp() above) the content stays hidden -
            // otherwise shows things like WiFi mode, reset reason, storage
            // usage and the Rocrail server address, which shouldn't be
            // visible from the outside.
            if (!statusAccessAllowed) {
                chunk += "<p>" + translate("Status information is only shown when accessing the clock from a private network") + ".</p>";
                chunk += "</div>"; // Ende .card
                                   // end .card
                chunk += "</div>"; // Ende panel-status
                                   // end panel-status
            }
            else {
            chunk += "<ul>";
            chunk += "<li>" + generateStorageInfo(LittleFS.usedBytes(), LittleFS.totalBytes(), true) + "</li>";

            String tzLabel = preferences.getString(PK_TIMEZONE, "DE");
            String tzDesc = tzLabel;

            // Lokale Kopie statt der globalen timeinfo - siehe Begruendung
            // an der ersten Statusseiten-Stelle weiter oben.

            // Local copy instead of the global timeinfo - see the reasoning
            // at the first status-page spot further above.
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

            webserver.sendContent(chunk);
            chunk = "";

            chunk += "<li>Compiled on: <strong>" + (String)version + "</strong></li><br>";
            chunk += "<li>TFT Driver: " + tftType + "</li>";
            chunk += "<li>TFT Size: " + String(TFT_WIDTH) + " x " + String(TFT_HEIGHT) + "</li>";
            chunk += "<br>";

            webserver.sendContent(chunk);
            chunk = "";

            chunk += "<li>Chip Model: " + String(ESP.getChipModel()) + "</li>";
            chunk += "<li>Chip Revision: " + String(ESP.getChipRevision()) + "</li>";
            chunk += "<li>Chip Cores: " + String(ESP.getChipCores()) + "</li>";
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

            // PSRAM-Erkennung, siehe Kommentar oben in /status.
            // PSRAM detection, see the comment above in /status.
            chunk += "<li>PSRAM Detected: " + String(psramFound() ? "yes" : "no") + "</li>";
            chunk += "<li>PSRAM Size: " + String(ESP.getPsramSize() / 1024) + " kB</li>";
            chunk += "<li>PSRAM Free: " + String(ESP.getFreePsram() / 1024) + " kB</li><br>";
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
                chunk += "<li>I2C SCL GPIO: " + String(SCL_PIN) + "</li><br>";
            }
            else {
                chunk += "<li>I2C: no device found</li><br>";
            }
#endif

            webserver.sendContent(chunk);
            chunk = "";

#if defined DCF77_DATAPIN && defined DCF77_INTERRUPT
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
            chunk += "<li>Use Touch: " + String(useTouch ? "true" : "false") + "</li><br>";
#endif
#ifdef ADC_PIN
            chunk += "<li>ADC_VCC GPIO: " + String(ADC_3V) + "</li>";
            chunk += "<li>ADC (photoresistor) GPIO: " + String(ADC_PIN) + "</li>";
            chunk += "<li>ADC_GND GPIO: " + String(ADC_GND) + "</li>";
            if (photoresistorFound) {
                chunk += "<li>ADC Value: " + String(getAdjustedAdcValue(analogRead(ADC_PIN))) + "</li><br>";
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


            chunk += "<li><b>timezone</b>: " + preferences.getString(PK_TIMEZONE, TIMEZONE_DEFAULT) + "</li>";
            chunk += "<li><b>background</b>: " + preferences.getString(PK_BACKGROUND, "/faces/default") + "</li>";
            chunk += "<li><b>handset</b>: " + preferences.getString(PK_HANDSET, "") + "</li>";
            // centerColor/rotation mode: siehe Kommentare oben in /status.
            // centerColor/rotation mode: see the comments above in /status.
            chunk += "<li><b>centerColor (RGB888)</b>: " + String(preferences.getLong(PK_CENTER_COLOR, 0xEC0016), HEX) + "</li>";
            chunk += "<li><b>centerSize</b>: " + String(preferences.getUInt(PK_CENTER_SIZE, 6)) + "</li>";

            uint8_t rotation = preferences.getUChar(PK_TFT_ROTATION1, TFT_ROTATION1_DEFAULT);
            chunk += "<li><b>tftRotation1</b>: " + rotationLabelHtml(rotation) + "</li>";
            {
                uint8_t rotation2Panel = preferences.getUChar(PK_TFT_ROTATION2, TFT_ROTATION2_DEFAULT);
                chunk += "<li><b>tftRotation2</b>: " + rotationLabelHtml(rotation2Panel) + "</li>";
            }
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

            bool stationModeStatus = preferences.getBool(PK_STATION_MODE, true);
            chunk += "<li><b>stationMode</b>: " + String(stationModeStatus ? "true" : "false") + "</li>";
            chunk += "<li><b>smoothSecond</b>: " + String(getSmoothSecondPref(stationModeStatus) ? "true" : "false") + "</li>";
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
            if (preferences.getBool(PK_ROCRAIL_ENABLED, false)) {
                chunk += "<li><b>rocrailEnabled</b>: " + String(preferences.getBool(PK_ROCRAIL_ENABLED, false) ? "true" : "false") + "</li>";
                chunk += "<li><b>rocrailServer</b>: " + preferences.getString(PK_ROCRAIL_SERVER, "") + "</li>";
                chunk += "<li><b>rocrailServerPort</b>: " + String(preferences.getUShort(PK_ROCRAIL_SRV_PORT, ROCRAIL_DEFAULT_PORT)) + "</li>";
            }
            chunk += "</ul>";
            chunk += "</br>";
            chunk += "<li>Contact: <a href='mailto:howl@gmx.de'>howl@gmx.de</a></li>";
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

            // Nur aus einem privaten Netz sichtbar (siehe isPrivateNetworkIp()
            // oben, gleiches Muster wie beim Status-Tab) - Logeintraege
            // koennen IP-Adressen, SSIDs u.ae. enthalten, die von aussen
            // nicht einsehbar sein sollen.

            // Only visible from a private network (see isPrivateNetworkIp()
            // above, same pattern as the Status tab) - log entries can
            // contain IP addresses, SSIDs, etc. that shouldn't be visible
            // from the outside.
            if (!isPrivateNetworkIp(webserver.client().remoteIP())) {
                chunk += "<p>" + translate("This action is only available when accessing the clock from a private network") + ".</p>";
            }
            else {

            // Nur ein Preferences-Zugriff, kein Datei-Lesevorgang - unproblematisch
            // bei jedem Seitenaufruf. JS-Refresh haelt #logFileName aktuell.

            // Just a Preferences lookup, not a file read - unproblematic on
            // every page load. The JS refresh keeps #logFileName up to date.
            String currentLogFileName = loggingEnabled ? getCurrentLogFileName() : "-";

            // 900px statt Standard-.card (500px), damit sie mit dem gleich
            // breiten <pre>-Logfenster darunter fluchtet.

            // 900px instead of the default .card (500px), so it aligns with
            // the equally wide <pre> log window below.

            // disabledAttr: bei deaktiviertem Logging haette ein Klick ohnehin
            // keinen sichtbaren Effekt.

            // disabledAttr: with logging disabled, a click would have no
            // visible effect anyway.
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

            // Kein serverseitiges Vorlesen mehr - nur ein Platzhalter, Inhalt
            // kommt per JS von /api/currentLog (siehe "Lazy-Load" unten).

            // No more server-side pre-reading - just a placeholder, content
            // arrives via JS from /api/currentLog (see "lazy load" below).
            chunk += "<pre id='logContent' style='background:var(--panel);border:1px solid var(--panel-border);border-radius:10px;max-width:900px;" INFO_LOG_WINDOW_HEIGHT_CSS "overflow-y:auto;margin:15px auto;padding:12px 16px;text-align:left;white-space:pre-wrap;word-break:break-word;font-family:monospace;font-size:.85rem;'>";
            chunk += loggingEnabled ? translate("Loading&hellip;") : translate("Logging is disabled.");
            chunk += "</pre>";

            // Auto-Refresh: Checkbox-Status in localStorage, pollt alle 10s.
            // Dateiliste kommt per JS von /api/logFileList, neueste zuerst.

            // Auto-refresh: checkbox state in localStorage, polls every 10s.
            // File list arrives via JS from /api/logFileList, newest first.
            chunk += "<script>";
            chunk += "(function() {";
            chunk += "  var cb = document.getElementById('logAutoRefresh');";
            chunk += "  var pre = document.getElementById('logContent');";
            chunk += "  var select = document.getElementById('logFileSelect');";
            chunk += "  var errEl = document.getElementById('logErrorHint');";
            chunk += "  var refreshBtn = document.getElementById('logRefreshNow');";
            chunk += "  var timer = null;";
            chunk += "  var loggingEnabled = " + String(loggingEnabled ? "true" : "false") + ";";
            chunk += "  var stored = localStorage.getItem('uhr3LogAutoRefresh');";
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

            // Laedt die Dateiliste - die neueste (erstes Element, siehe
            // /api/logFileList) wird automatisch ausgewaehlt, ausser der
            // Nutzer stand bereits auf einer AELTEREN Datei (bewusst zum
            // Durchsehen ausgewaehlt) - diese bleibt dann erhalten, sofern
            // sie noch existiert. So verpasst weder das anfaengliche Laden
            // noch ein spaeteres Auto-Refresh eine inzwischen per Rotation
            // neu angelegte Logdatei (siehe flushLogBuffer() in
            // system_utils.h), waehrend ein bewusst geoeffnetes altes Logfile
            // nicht durch die Rotation weggerissen wird.

            // Loads the file list - the newest (first element, see
            // /api/logFileList) is auto-selected, unless the user was
            // already on an OLDER file (deliberately picked to review) -
            // that stays selected as long as it still exists. This way
            // neither the initial load nor a later auto-refresh misses a
            // log file newly created by rotation in the meantime (see
            // flushLogBuffer() in system_utils.h), while a deliberately
            // opened older log file doesn't get yanked away by rotation.
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
            // Wird eine AELTERE als die neueste Datei ausgewaehlt, das
            // Auto-Refresh abschalten - beim periodischen Neuladen der
            // Dateiliste (loadFileList()) waere sonst wieder auf die
            // neueste gesprungen worden (siehe deren Kommentar oben), was
            // eine bewusst zum Durchsehen gewaehlte aeltere Datei nicht
            // mehr wegreissen wuerde, aber unnoetige Anfragen fuer eine
            // ohnehin statische Datei blieben.

            // If a file OLDER than the newest is selected, turn off
            // auto-refresh - the periodic file-list reload (loadFileList())
            // would otherwise jump back to the newest one (see its comment
            // above), and while that no longer yanks away a deliberately
            // reviewed older file, needless requests for an otherwise
            // static file would remain.
            chunk += "    var isNewest = select.options.length > 0 && select.value === select.options[0].value;";
            chunk += "    if (!isNewest && cb.checked) {";
            chunk += "      cb.checked = false;";
            chunk += "      localStorage.setItem('uhr3LogAutoRefresh', '0');";
            chunk += "      applyState();";
            chunk += "    }";
            chunk += "    refreshLog();";
            chunk += "  });";
            chunk += "  cb.addEventListener('change', function() {";
            chunk += "    localStorage.setItem('uhr3LogAutoRefresh', cb.checked ? '1' : '0');";
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

            // Panel: WLAN, uebernommen aus der frueheren eigenstaendigen
            // /wifi-Seite (Hostname-Formular, WPS/Rescan, WLAN-Slots).

            // Panel: WiFi, taken over from the former standalone /wifi
            // page (hostname form, WPS/rescan, WiFi slots).
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
            // hostname kommt unfiltriert aus /sethostname - ohne escapeHtmlText()
            // koennte ein "'" darin dieses title-Attribut aufbrechen.

            // hostname comes unfiltered from /sethostname - without
            // escapeHtmlText(), a "'" in it could break out of this title attribute.
            chunk += "<span title='" + translate("The clock can also be reached at http://&quot;hostname&quot;.local instead of its IP address, e.g.") + " http://" + escapeHtmlText(String(hostname)) + ".local. " + translate("A restart is required for a changed hostname to take effect. Not all routers support hostname resolution") + ".' style='cursor:help;'>&#9432;</span>";
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
            chunk += "<span title='" + translate("Adds a new network via WPS - press the WPS button on your router when prompted. The clock's connection may be lost for about 2 minutes while this happens") + ".' style='cursor:help;'>&#9432;</span>";
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

            chunk += "<div style='display:flex;align-items:center;gap:6px;white-space:nowrap;'><input type='checkbox' name='stationMode' value='1' ";
            chunk += preferences.getBool(PK_STATION_MODE, true) ? "checked" : "";
            chunk += " style='width:auto;margin:0;'>" + translate("Train Station Mode");
            chunk += " <span title='" + translate("The second hand completes its lap in about 58.5 seconds and then waits at 60 until the minute changes, like a classic train station clock") + ".' style='cursor:help;'>&#9432;</span></div><br>";

            chunk += "<div style='display:flex;align-items:center;gap:6px;white-space:nowrap;'><input type='checkbox' name='smoothSecond' value='1' ";
            // Fallback bewusst PK_STATION_MODE statt eines festen Literals -
            // siehe Kommentar bei der smoothSecond-Ladezeile in uhr3.ino
            // (Geraete ohne je gespeicherten smoothSecond-Wert behalten so ihr
            // bisheriges Aussehen bei).

            // Fallback deliberately PK_STATION_MODE instead of a fixed literal
            // - see the comment at the smoothSecond load line in uhr3.ino
            // (devices that never saved a smoothSecond value keep their
            // previous look this way).
            chunk += getSmoothSecondPref(preferences.getBool(PK_STATION_MODE, true)) ? "checked" : "";
            chunk += " style='width:auto;margin:0;'>" + translate("Smooth Second Hand");
            chunk += " <span title='" + translate("The second hand moves smoothly instead of jumping in 1-second steps") + ".' style='cursor:help;'>&#9432;</span></div><br>";

            chunk += "<div style='display:flex;align-items:center;gap:6px;white-space:nowrap;'><input type='checkbox' name='showSecondHand' value='1' ";
            chunk += preferences.getBool(PK_SHOW_SECOND_HAND, true) ? "checked" : "";
            chunk += " style='width:auto;margin:0;'>" + translate("Show Seconds");
            chunk += " <span title='" + translate("Shows or hides the second hand on the clock face") + ".' style='cursor:help;'>&#9432;</span></div><br>";

            chunk += "<div style='display:flex;align-items:center;gap:6px;white-space:nowrap;'><input type='checkbox' name='smoothMinute' value='1' ";
            // Bugfix: Default muss false sein, sonst zeigte die Checkbox nach
            // einem Werksreset faelschlich "aktiviert".

            // Bugfix: default must be false, otherwise the checkbox falsely
            // showed "enabled" after a factory reset.
            chunk += preferences.getBool(PK_SMOOTH_MINUTE, false) ? "checked" : "";
            chunk += " style='width:auto;margin:0;'>" + translate("Smooth Minute Hand");
            chunk += " <span title='" + translate("The minute hand moves smoothly instead of jumping in 1-minute steps") + ".' style='cursor:help;'>&#9432;</span></div><br>";

            chunk += "<div style='display:flex;align-items:center;gap:6px;white-space:nowrap;'><input type='checkbox' name='wifiActive' value='1' ";
            chunk += wifiActive ? "checked" : "";
            chunk += " style='width:auto;margin:0;'>" + translate("Reconnect WiFi");
            chunk += " <span title='" + translate("Automatically tries to reconnect if the WiFi connection is lost") + ".' style='cursor:help;'>&#9432;</span></div><br>";

            // Default aus - schaltet Nutzung und Sichtbarkeit des separaten
            // Rocrail-Tabs frei (siehe generateSettingsTabNav()).

            // Default off - unlocks use and visibility of the separate
            // Rocrail tab (see generateSettingsTabNav()).
            chunk += "<div style='display:flex;align-items:center;gap:6px;white-space:nowrap;'><input type='checkbox' name='rocrailEnabled' value='1' ";
            chunk += rocrailEnabled ? "checked" : "";
            chunk += " style='width:auto;margin:0;'>" + translate("Rocrail");
            chunk += " <span title='" + translate("Take over the model time from a Rocrail server (model railroad control software) for the hands - unlocks the Rocrail tab, where the server address can then be entered") + ".' style='cursor:help;'>&#9432;</span></div><br>";

            chunk += "<div style='display:flex;align-items:center;gap:6px;white-space:nowrap;'><input type='checkbox' name='loggingEnabled' value='1' ";
            chunk += loggingEnabled ? "checked" : "";
            chunk += " style='width:auto;margin:0;'>" + translate("Enable Logging");
            chunk += " <span title='" + translate("Writes up to 9 log files to LittleFS for troubleshooting") + ".' style='cursor:help;'>&#9432;</span></div><br>";

            // Nur sichtbar mit DCF77-Hardware UND dcf77Confirmed - ohne je
            // erkanntes Signal soll die Option gar nicht erst auftauchen.

            // Only visible with DCF77 hardware AND dcf77Confirmed - without a
            // signal ever recognized, the option should not appear at all.
#if defined(DCF77_DATAPIN) && defined(DCF77_INTERRUPT)
            if (dcf77Confirmed) {
                chunk += "<div style='display:flex;align-items:center;gap:6px;white-space:nowrap;'><input type='checkbox' name='dcfSyncLed' value='1' ";
                chunk += dcfSyncLedEnabled ? "checked" : "";
                chunk += " style='width:auto;margin:0;'>" + translate("DCF77 Sync LED Blink");
                chunk += " <span title='" + translate("Flashes the LED for every received DCF77 pulse while the clock is still acquiring the time signal") + ".' style='cursor:help;'>&#9432;</span></div><br>";
            }
#endif

            chunk += "<div style='display:flex;align-items:center;gap:6px;white-space:nowrap;'>" + translate("Rotation Display 1") + ": <span title='" + translate("Rotates the clock face by the selected number of degrees, useful if the display is mounted rotated in its housing") + ".' style='cursor:help;'>&#9432;</span> <select name='rotation' style='width:190px;'>";
            const char* rotationLabels[] = { "0&deg;", "90&deg;", "180&deg;", "270&deg;" };
            String rotationNaLabel = translate("not connected (n.a.)");
            for (int i = 0; i <= TFT_ROTATION_NA; i++) {
                chunk += "<option value='" + String(i) + "'";
                if (i == tftRotation1) chunk += " selected";
                chunk += ">" + (i == TFT_ROTATION_NA ? rotationNaLabel : String(rotationLabels[i])) + "</option>";
            }
            chunk += "</select></div>";

            chunk += "<div style='display:flex;align-items:center;gap:6px;white-space:nowrap;'>" + translate("Rotation Display 2") + ": <span title='" + translate("Rotates Display 2's (CS2) clock face independently of Display 1") + ".' style='cursor:help;'>&#9432;</span> <select name='rotation2' style='width:190px;'>";
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

            chunk += "</div>";
            chunk += "<div style='text-align:center;margin-top:15px;'><button type='submit'>" + translate("Save") + "</button></div>";
            chunk += "</form>";
            chunk += "</div>"; // Ende panel-zifferblatt
                               // end panel-zifferblatt

            webserver.sendContent(chunk);
            chunk = "";

            // Panel: Helligkeit, uebernommen aus der frueheren /brightness-Seite
            // (inkl. optionalem Plotly-Gamma-Chart).

            // Panel: brightness, taken over from the former /brightness
            // page (incl. optional Plotly gamma chart).
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
                chunk += "<div style='background:#fff3cd;color:#856404;border:1px solid #ffeeba;border-radius:6px;padding:10px 15px;margin:10px auto;max-width:500px;'>" +
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

#if defined (GC9D01)  || defined(GC9A01_WITH_BACKLIGHT)
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
#endif
            }

            chunk += "</div>"; // Ende panel-helligkeit
                               // end panel-helligkeit

            webserver.sendContent(chunk);
            chunk = "";

            // Panel: Zeit/NTP/Timezone, uebernommen aus der frueheren
            // /timezone_form-Seite.

            // Panel: time/NTP/timezone, taken over from the former
            // /timezone_form page.
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

            // Nur aus einem privaten Netz erlaubt (siehe isPrivateNetworkIp()
            // oben) - WLAN-Loeschungen (aktiv wie nicht-aktiv) sollen
            // grundsaetzlich nicht von aussen moeglich sein, auch nicht per
            // Bestaetigungscode. Anders als /save (Ueberschreiben) und
            // /api/connectWifi (Wechseln), wo ein Bestaetigungscode als
            // Alternative fuer Zugriff von aussen bleibt.

            // Only allowed from a private network (see isPrivateNetworkIp()
            // above) - WLAN deletions (active or not) should never be
            // possible from outside at all, not even via a confirmation
            // code. Unlike /save (overwrite) and /api/connectWifi
            // (switching), where a confirmation code remains available as
            // an alternative for access from outside.
            if (!isPrivateNetworkIp(webserver.client().remoteIP())) {
                webserver.send(200, "text/html", simpleMessagePage(translate("Delete"), "<p>" + translate("This action is only available when accessing the clock from a private network") + ".</p>"));
                return;
            }

            if (webserver.hasArg("index")) {
                int idx = webserver.arg("index").toInt();
                if (idx >= 0 && idx < MAX_WLAN) {

                    // Ist es das aktiv verbundene Netzwerk, ueber
                    // executePendingAction() loeschen (kompaktiert die Liste
                    // und startet danach neu, siehe dort) - Zugriff ist an
                    // dieser Stelle bereits als privat bestaetigt (siehe
                    // Guard oben), daher ohne Bestaetigungscode-Umweg.

                    // If this is the actively connected network, delete it
                    // via executePendingAction() (compacts the list and
                    // reboots afterwards, see there) - access has already
                    // been confirmed as private at this point (see the
                    // guard above), so no confirmation-code detour needed.
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
                        newPass[i] = preferences.getString(pkPass(i).c_str(), "");
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

                    // Wechsel auf ein ANDERES als das gerade aktive Netzwerk
                    // kann die Verbindung dauerhaft kappen, wenn das Ziel von
                    // hier aus nicht erreichbar ist (z.B. falsches Passwort
                    // oder ausserhalb der Reichweite): aus einem privaten Netz
                    // direkt ausfuehren (das eigene Heimnetz gilt als
                    // hinreichend vertrauenswuerdig), aus einem NICHT-privaten
                    // Netz erst einen Bestaetigungscode verlangen, wie beim
                    // Loeschen/Ueberschreiben des aktiven Netzwerks. Ist idx
                    // ohnehin schon das aktive Netzwerk, ist ein Neustart
                    // darauf risikolos, bleibt aber trotzdem privat-only
                    // (unten) - ein von aussen jederzeit ausloesbarer Neustart
                    // ist unabhaengig vom Risiko ein DoS-Vektor.

                    // Switching to a network OTHER than the currently active
                    // one can permanently sever the connection if the target
                    // isn't reachable from here (e.g. wrong password or out
                    // of range): execute directly from a private network
                    // (one's own home network counts as sufficiently
                    // trustworthy), but from a NON-private network first
                    // require a confirmation code, as with deleting/
                    // overwriting the active network. If idx is already the
                    // active network, restarting into it is risk-free, but
                    // still stays private-only (below) - a restart
                    // triggerable from outside at any time is a DoS vector
                    // regardless of the risk involved.
                    bool isAlreadyActive = (WiFi.status() == WL_CONNECTED && WiFi.SSID() == wifiSsid[idx]);

                    if (!isAlreadyActive) {
                        if (isPrivateNetworkIp(webserver.client().remoteIP())) {
                            pendingWifiChangeIndex = idx;
                            DEBUG_PRINTLN("[SECURITY] WiFi network switch executed directly (private network) from " + webserver.client().remoteIP().toString());
                            executePendingAction("wlanSwitchActive");
                            return;
                        }

                        // rejectIfConfirmationPending() ZUERST, bevor
                        // pendingWifiChangeIndex gesetzt wird - siehe
                        // Begruendung dort (Payload einer anderen, noch
                        // ausstehenden Anfrage nicht ueberschreiben).

                        // rejectIfConfirmationPending() FIRST, before
                        // pendingWifiChangeIndex is set - see the reasoning
                        // there (don't overwrite the payload of a different,
                        // still-pending request).
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

                // Betrifft dieses Formular den AKTUELL VERBUNDENEN Slot? Dann
                // nicht sofort anwenden: aus einem privaten Netz direkt
                // ausfuehren (das eigene Heimnetz gilt als hinreichend
                // vertrauenswuerdig), aus einem NICHT-privaten Netz erst
                // einen Bestaetigungscode auf dem Display anfordern (siehe
                // requestConfirmationCode()/checkFactoryResetCodePending()/
                // executePendingAction() in system_utils.h bzw. weiter oben
                // sowie /deletewifi) - verhindert, dass die aktive Verbindung
                // aus der Ferne (z.B. ueber eine DMZ/Port-Weiterreitung) ohne
                // physischen Zugriff auf die Uhr veraendert wird. Andere
                // (nicht aktive) Slots im selben Formular bleiben unten
                // privat-only, ganz ohne Code-Option.

                // Does this form touch the CURRENTLY CONNECTED slot? If so,
                // don't apply it right away: execute directly from a private
                // network (one's own home network counts as sufficiently
                // trustworthy), but from a NON-private network first request
                // a confirmation code on the display (see
                // requestConfirmationCode()/checkFactoryResetCodePending()/
                // executePendingAction() in system_utils.h and further above,
                // and /deletewifi) - prevents the active connection from
                // being changed remotely (e.g. via a DMZ/port forward)
                // without physical access to the clock. Other (non-active)
                // slots in the same form stay private-only below, with no
                // code option at all.
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
                    if (newPass[activeIdx] != "" && newPass[activeIdx] != preferences.getString(pkPass(activeIdx).c_str(), "")) {
                        activeSlotChanged = true;
                    }
                }

                if (activeSlotChanged) {

                    // Leeres SSID-Feld fuer den aktiven Slot ist der Sache
                    // nach ein LOESCHEN (genau das, was /deletewifi fuer den
                    // aktiven Slot macht), keine Aenderung - muss daher
                    // genauso strikt behandelt werden: ausschliesslich aus
                    // einem privaten Netz, OHNE Code-Alternative fuer
                    // Zugriff von aussen. Sonst waere das Loeschen des
                    // aktiven Netzwerks ueber /save (statt ueber
                    // /deletewifi) ein Umweg, der trotz der dortigen
                    // Sperre per Code von aussen moeglich bliebe.

                    // An empty SSID field for the active slot effectively
                    // amounts to DELETING it (exactly what /deletewifi does
                    // for the active slot), not a change - must therefore be
                    // treated just as strictly: only from a private network,
                    // with NO code alternative for access from outside.
                    // Otherwise deleting the active network via /save
                    // (instead of via /deletewifi) would be a detour that
                    // stayed possible from outside via a code, despite the
                    // block there.
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

                    // rejectIfConfirmationPending() ZUERST, bevor
                    // pendingWifiSsid[]/pendingWifiPass[] gesetzt werden -
                    // siehe Begruendung dort (Payload einer anderen, noch
                    // ausstehenden Anfrage nicht ueberschreiben).

                    // rejectIfConfirmationPending() FIRST, before
                    // pendingWifiSsid[]/pendingWifiPass[] are set - see the
                    // reasoning there (don't overwrite the payload of a
                    // different, still-pending request).
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

                // Nur nicht-aktive Slots betroffen: nur aus einem privaten
                // Netz erlaubt (siehe isPrivateNetworkIp() oben) - bisher
                // ohne jede Ruecksprache sofort uebernommen.

                // Only non-active slots affected: only allowed from a
                // private network (see isPrivateNetworkIp() above) -
                // previously applied immediately without any confirmation.
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
                    DEBUG_PRINTLN("set bg to: " + file + " (from " + webserver.client().remoteIP().toString() + ")");
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

                // Logdateien nur aus einem privaten Netz einsehbar (siehe
                // isPrivateNetworkIp() oben und /api/currentLog) - koennen
                // IP-Adressen, SSIDs u.ae. enthalten. Andere Dateitypen
                // bleiben unveraendert von ueberall einsehbar.

                // Log files only viewable from a private network (see
                // isPrivateNetworkIp() above and /api/currentLog) - can
                // contain IP addresses, SSIDs, etc. Other file types remain
                // viewable from anywhere, unchanged.
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
            chunk += "<h2>" + translate("Manage Clock Hand Sets") + " " + String(HAND_WIDTH) + " x " + String(HAND_HEIGHT) + "</h2>";
            chunk += "<p>" + generateStorageInfo(used, total) + "</p>";
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
                // setId kommt aus Upload-Dateinamen und ist nicht auf harmlose Zeichen
                // eingeschraenkt (stored XSS) - daher ueberall escapen, ausser fuer
                // Dateisystem-Pfade, die den echten Namen brauchen.

                // setId comes from upload filenames and isn't restricted to harmless
                // characters (stored XSS) - so escape it everywhere except for
                // filesystem paths, which need the real name.
                String safeSetId = escapeHtmlText(setId);
                chunk = "<div style='text-align:center;border:1px solid #ccc;border-radius:6px;padding:8px;'>";
                String hourPath = "/hand_set" + setId + "_hour.bmp";
                String minutePath = "/hand_set" + setId + "_minute.bmp";
                String secondPath = "/hand_set" + setId + "_second.bmp";
                chunk += "<a href='/sethandset?set=" + safeSetId + "'>";
                chunk += LittleFS.exists(hourPath) ? "<img src='/file?name=" + escapeHtmlText(hourPath) + "'> " : "<img src='data:image/bmp;charset=utf-8;base64, " + handHourBase64 + "'> ";
                chunk += LittleFS.exists(minutePath) ? "<img src='/file?name=" + escapeHtmlText(minutePath) + "'> " : "<img src='data:image/bmp;charset=utf-8;base64, " + handMinuteBase64 + "'> ";
                chunk += LittleFS.exists(secondPath) ? "<img src='/file?name=" + escapeHtmlText(secondPath) + "'> " : "<img src='data:image/bmp;charset=utf-8;base64," + handSecondBase64 + "'>";
                chunk += "</a><br>" + safeSetId + (setId == activeSet ? " (" + translate("active") + ")" : "");
                chunk += "<br><a href='/deletehandset?set=" + safeSetId + "' onclick='return confirm(\"" + translate("Delete") + " " + escapeForJsStringInAttr(setId, '"') + "?\")'>" + translate("Delete") + "</a>";
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
                chunk += "<div style='color:red;font-weight:bold;'>" + translate("Warning: Not enough free space to upload new hand sets! Free up some space first") + ".</div><br><br>";
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
                DEBUG_PRINTLN("[UPLOAD] Hand uploaded to: " + uploadFilePath + " (from " + webserver.client().remoteIP().toString() + ")");
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

            // Nur aus einem privaten Netz erlaubt (siehe isPrivateNetworkIp() oben).
            // Only allowed from a private network (see isPrivateNetworkIp() above).
            if (!isPrivateNetworkIp(webserver.client().remoteIP())) {
                webserver.send(200, "text/html", simpleMessagePage(translate("Delete"), "<p>" + translate("This action is only available when accessing the clock from a private network") + ".</p>"));
                return;
            }

            if (webserver.hasArg("set")) {
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

        // Werkseinstellungen: Uebersichtsseite mit mehreren, einzeln
        // bestaetigten Reset-Optionen statt einer einzigen Alles-oder-nichts-Aktion.

        // Factory settings: overview page with several individually confirmed
        // reset options instead of one single all-or-nothing action.
        webserver.on("/factoryReset", HTTP_GET, []() {
            String html = beginPage();
            html += generateFlashMessage();
            html += "<h2>" + translate("Factory&nbsp;Reset") + "</h2>";

            // Keine Aktion mehr direkt per Klick: alle fuenf fuehren erst zu
            // einem Bestaetigungscode auf dem Display (siehe /factoryReset/
            // requestCode und checkFactoryResetCodePending() in
            // system_utils.h) - schuetzt vor einer Aktion ohne physischen
            // Zugriff auf die Uhr, z.B. bei Erreichbarkeit ueber eine DMZ/
            // Port-Weiterleitung. Das versteckte "action"-Feld sagt
            // /factoryReset/requestCode, welche der fuenf Aktionen nach der
            // Code-Bestaetigung ausgefuehrt werden soll.

            // No action happens directly on click anymore: all five first
            // lead to a confirmation code shown on the display (see
            // /factoryReset/requestCode and checkFactoryResetCodePending()
            // in system_utils.h) - protects against an action without
            // physical access to the clock, e.g. when reachable via a DMZ/
            // port forward. The hidden "action" field tells
            // /factoryReset/requestCode which of the five actions to run
            // once the code is confirmed.
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
            html += "<p>" + translate("Deletes all uploaded clock faces - the built-in default remains") + ".</p>";
            html += "<form method='POST' action='/factoryReset/requestCode' onsubmit=\"return confirm('" + translate("Are you sure you want to delete all clock faces except the default one?") + "');\">";
            html += "<input type='hidden' name='action' value='faces'>";
            html += "<button type='submit'>" + translate("Delete Clock Faces (except default)") + "</button></form><hr>";

            html += "<h3>" + translate("Delete Hand Sets (except default)") + "</h3>";
            html += "<p>" + translate("Deletes all uploaded hand sets - the built-in default remains") + ".</p>";
            html += "<form method='POST' action='/factoryReset/requestCode' onsubmit=\"return confirm('" + translate("Are you sure you want to delete all hand sets except the default one?") + "');\">";
            html += "<input type='hidden' name='action' value='hands'>";
            html += "<button type='submit'>" + translate("Delete Hand Sets (except default)") + "</button></form><hr>";

            html += "<h3>" + translate("Delete Presets") + "</h3>";
            html += "<p>" + translate("Deletes all saved presets") + ".</p>";
            html += "<form method='POST' action='/factoryReset/requestCode' onsubmit=\"return confirm('" + translate("Are you sure you want to delete all presets?") + "');\">";
            html += "<input type='hidden' name='action' value='presets'>";
            html += "<button type='submit'>" + translate("Delete Presets") + "</button></form><hr>";

            html += "</body></html>";
            webserver.send(200, "text/html", html);
            });

        // Schritt 1: Bestaetigungscode erzeugen und auf dem Display anzeigen
        // (siehe checkFactoryResetCodePending() in system_utils.h), die
        // angeforderte Aktion merken, dann zur Eingabeseite weiterleiten.
        // Ersetzt den frueher direkten Klick auf einen der fuenf Buttons.

        // Step 1: generate a confirmation code and show it on the display
        // (see checkFactoryResetCodePending() in system_utils.h), remember
        // which action was requested, then redirect to the entry page.
        // Replaces the formerly direct click on one of the five buttons.
        webserver.on("/factoryReset/requestCode", HTTP_POST, []() {
            String action = webserver.arg("action");
            if (action != "all" && action != "wifi" && action != "faces" && action != "hands" && action != "presets") {
                redirectTo("/factoryReset");
                return;
            }

            // Aus einem privaten Netz keine Code-Bestaetigung noetig - Zugriff
            // aus dem eigenen Heimnetz gilt bereits als hinreichend
            // vertrauenswuerdig. Der Code ist nur noch die zusaetzliche
            // Huerde fuer Anfragen von ausserhalb (z.B. ueber eine DMZ/Port-
            // Weiterleitung), wo niemand physisch am Geraet sein muss, um
            // sie auszuloesen.

            // No code confirmation needed from a private network - access
            // from one's own home network already counts as sufficiently
            // trustworthy. The code is now only the extra hurdle for
            // requests from outside (e.g. via a DMZ/port forward), where no
            // one needs to be physically at the device to trigger them.
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

            // Kurze, englische Aktionsbeschriftung (siehe
            // factoryResetActionLabel() in system_utils.h) - seit das Display
            // selbst nur noch den nackten Code zeigt, ist dies die einzige
            // Stelle, an der zu sehen ist, WELCHE Aktion gerade bestaetigt wird.

            // Short, English action label (see factoryResetActionLabel() in
            // system_utils.h) - now that the display itself shows only the
            // bare code, this is the only place showing WHICH action is
            // currently being confirmed.
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

        // Schritt 3: Code pruefen, erst dann die gemerkte Aktion ausfuehren -
        // gemeinsamer Bestaetigungs-Endpunkt fuer alle fuenf Aktionen.
        // executePendingAction() (siehe weiter oben) fuehrt die eigentliche
        // Aktion aus.

        // Step 3: verify the code, only then run the remembered action -
        // shared confirmation endpoint for all five actions.
        // executePendingAction() (see further above) runs the actual action.
        webserver.on("/factoryReset/confirm", HTTP_POST, []() {
            String action = factoryResetPendingAction;

            if (factoryResetCode.isEmpty() || action.isEmpty() || millis() - factoryResetCodeStartMillis > FACTORY_RESET_CODE_TIMEOUT_MS) {
                factoryResetCode = "";
                factoryResetPendingAction = "";
                factoryResetCodeAttempts = 0;
                DEBUG_PRINTLN("[SECURITY] Confirmation attempted from " + webserver.client().remoteIP().toString() + " with no valid code pending");
                redirectTo("/factoryReset?msg=Code%20expired%2C%20please%20try%20again");
                return;
            }
            if (!webserver.hasArg("code") || webserver.arg("code") != factoryResetCode) {
                factoryResetCodeAttempts++;
                DEBUG_PRINTLN("[SECURITY] Confirmation attempted from " + webserver.client().remoteIP().toString() + " with wrong code (action: " + action + ", attempt " + String(factoryResetCodeAttempts) + "/" + String(FACTORY_RESET_MAX_ATTEMPTS) + ")");

                // Zu viele Fehlversuche: Code sofort ungueltig machen statt
                // das Zeitfenster weiter zum Raten offen zu lassen - ein
                // neuer Code (und damit ein neuer, sichtbarer Anzeigevorgang
                // auf dem Display) ist dann faellig.

                // Too many wrong attempts: invalidate the code right away
                // instead of leaving the window open for further guessing -
                // a new code (and with it a new, visible display prompt) is
                // then required.
                if (factoryResetCodeAttempts >= FACTORY_RESET_MAX_ATTEMPTS) {
                    factoryResetCode = "";
                    factoryResetPendingAction = "";
                    DEBUG_PRINTLN("[SECURITY] Too many wrong attempts from " + webserver.client().remoteIP().toString() + " - code invalidated");
                    redirectTo("/factoryReset?msg=Too%20many%20wrong%20attempts%2C%20please%20try%20again");
                    return;
                }

                redirectTo("/factoryReset/enterCode?msg=Wrong%20code%2C%20please%20try%20again");
                return;
            }

            // Code verbraucht - vor der Aktion loeschen, damit
            // checkFactoryResetCodePending() das Display nicht mehr
            // beansprucht, waehrend die Aktion selbst schon zeichnet
            // (factoryReset()) bzw. neu startet (WiFi-Reset).

            // Code consumed - clear before the action, so
            // checkFactoryResetCodePending() no longer claims the display
            // while the action itself is already drawing on it
            // (factoryReset()) or restarting (WiFi reset).
            factoryResetCode = "";
            factoryResetPendingAction = "";
            factoryResetCodeAttempts = 0;
            DEBUG_PRINTLN("[SECURITY] Confirmed from " + webserver.client().remoteIP().toString() + " (action: " + action + ")");

            executePendingAction(action);
            });

        // Sofortige Zeitsynchronisation - startet nur die asynchrone Task
        // (siehe time_sync.h) und antwortet sofort, statt den Webserver auf
        // DNS/UDP warten zu lassen; das Ergebnis zeigt die naechste Aktualisierung
        // der Statuszeile (poll alle 5s) bzw. der Zeit-Tab beim naechsten Laden.

        // Immediate time synchronization - only starts the asynchronous task
        // (see time_sync.h) and responds right away instead of letting the
        // web server wait on DNS/UDP; the result shows up in the next status
        // bar refresh (polled every 5s) or the Time tab on its next load.
        webserver.on("/syncnow", HTTP_POST, []() {
            startNtpSyncTask("Manual sync");

            String html = simpleMessagePage(translate("Time sync started"), "<p>" + translate("Returning to main page in 3 seconds") + ".</p>", "<meta http-equiv='refresh' content='3; url=/'>");

            webserver.send(200, "text/html", html);
            });

    }


    // Handhabt den Datei-Upload
    // Handles the file upload

    void handleFileUpload() {
        HTTPUpload& upload = webserver.upload();

        if (upload.status == UPLOAD_FILE_START) {
            uploadFilePath = upload.filename;
            if (!uploadFilePath.startsWith("/")) uploadFilePath = "/" + uploadFilePath;

            // Nur aus einem privaten Netz erlaubt (siehe isPrivateNetworkIp()
            // weiter oben) - HIER pruefen, nicht erst im "Request Handler"
            // von /upload bzw. /uploadhandset: der laeuft laut ESP32-
            // WebServer-API erst NACH dem kompletten Upload, dieser Callback
            // hier bereits waehrend jedes einzelnen Chunks. Ohne die Sperre
            // an dieser Stelle waeren die Daten laengst auf LittleFS
            // geschrieben (Flutungs-/Speicherplatz-Erschoepfungsrisiko aus
            // der Ferne), bevor die Ablehnung ueberhaupt greifen wuerde.

            // Only allowed from a private network (see isPrivateNetworkIp()
            // further above) - checked HERE, not only in /upload's or
            // /uploadhandset's "request handler": per the ESP32 WebServer
            // API, that only runs AFTER the complete upload, while this
            // callback already runs during each individual chunk. Without
            // the gate at this point, the data would already have been
            // written to LittleFS (a remote flooding/storage-exhaustion
            // risk) before the rejection could take effect at all.
            if (!isPrivateNetworkIp(webserver.client().remoteIP())) {
                DEBUG_PRINTLN("[UPLOAD] Rejected: not a private network : " + uploadFilePath + " (from " + webserver.client().remoteIP().toString() + ")");
                uploadSuccess = false;
                return;
            }

            // Nur bestimmte Dateinamenmuster zulassen
            // Only allow certain filename patterns
            if (!uploadFilePath.endsWith(".bmp") ||
                !(uploadFilePath.startsWith("/face_") || uploadFilePath.startsWith("/hand_set"))) {
                DEBUG_PRINTLN("[UPLOAD] Invalid filename: must start with 'face_' or 'hand_set' and end with '.bmp' : " + uploadFilePath + " (from " + webserver.client().remoteIP().toString() + ")");
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
                    String lowerPath = uploadFilePath;
                    lowerPath.toLowerCase();
                    if (lowerPath.endsWith(".bmp")) {
                      //  bool isHand = uploadFilePath.indexOf("hour") > 0 || uploadFilePath.indexOf("minute") > 0 || uploadFilePath.indexOf("second") > 0;
                        if (uploadFilePath.startsWith("/face_")) {
                            DEBUG_PRINTLN("[UPLOAD] Detected Clock Face upload (from " + webserver.client().remoteIP().toString() + ")");

                            if (!scaleAndSaveBmp(uploadFilePath.c_str(), uploadFilePath.c_str(), CLOCK_WIDTH, CLOCK_HEIGHT)) {
                                DEBUG_PRINTLN("[UPLOAD] Scaling failed for " + uploadFilePath + " (from " + webserver.client().remoteIP().toString() + ")");
                                uploadSuccess = false;
                                return;
                            }

                        }
                        else if (uploadFilePath.startsWith("/hand_set")) {
                            DEBUG_PRINTLN("[UPLOAD] Detected Clock Hand upload (from " + webserver.client().remoteIP().toString() + ")");

                            if (!scaleAndSaveBmp(uploadFilePath.c_str(), uploadFilePath.c_str(), HAND_WIDTH, HAND_HEIGHT)) {
                                DEBUG_PRINTLN("[UPLOAD] Scaling failed for " + uploadFilePath + " (from " + webserver.client().remoteIP().toString() + ")");
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


    // Upload fuer /importpresets: liest "Name<TAB>URL"-Zeilen und fuegt nur
    // NEUE Presets in freie Slots ein - bestehende bleiben unangetastet.

    // Upload for /importpresets: reads "Name<TAB>URL" lines and inserts only
    // NEW presets into free slots - existing ones stay untouched.

    void handlePresetImportUpload() {
        HTTPUpload& upload = webserver.upload();

        if (upload.status == UPLOAD_FILE_START) {
            DEBUG_PRINTLN("[PRESET-IMPORT] Start (from " + webserver.client().remoteIP().toString() + ")");
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
                    DEBUG_PRINTLN("[PRESET-IMPORT] Could not read file (from " + webserver.client().remoteIP().toString() + ")");
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
                        DEBUG_PRINTLN("[PRESET-IMPORT] Ungueltige Zeile (kein Tab): " + line + " (from " + webserver.client().remoteIP().toString() + ")");
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
                        DEBUG_PRINTLN("[PRESET-IMPORT] Skipped (already exists): " + name + " (from " + webserver.client().remoteIP().toString() + ")");
                        skippedCount++;
                        continue;
                    }

                    if (!validateAndFixPresetFace(url, existingFaces)) {
                        DEBUG_PRINTLN("[PRESET-IMPORT] Skipped (clock face not found): " + name + " (from " + webserver.client().remoteIP().toString() + ")");
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
                        DEBUG_PRINTLN("[PRESET-IMPORT] No free slot left - aborted (from " + webserver.client().remoteIP().toString() + ")");
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

                DEBUG_PRINTLN("[PRESET-IMPORT] " + String(importedCount) + " presets imported, " + String(skippedCount) + " skipped (from " + webserver.client().remoteIP().toString() + ")");
                presetImportSuccess = true; // auch 0 neue Presets ist kein Fehler (z.B. alles schon vorhanden)
                                            // 0 new presets is also not an error (e.g. everything already existed)
            }
            else {
                DEBUG_PRINTLN("[PRESET-IMPORT] Failed while writing (from " + webserver.client().remoteIP().toString() + ")");
            }
        }
    }


    // Wie handlePresetImportUpload(), aber ohne Namens-Dopplungspruefung -
    // der aufrufende JS-Code filtert vorhandene Namen bereits vorher heraus.

    // Like handlePresetImportUpload(), but without a name-duplicate check -
    // the calling JS code already filters out existing names beforehand.

    void handlePresetMergeUpload() {
        HTTPUpload& upload = webserver.upload();

        if (upload.status == UPLOAD_FILE_START) {
            DEBUG_PRINTLN("[PRESET-MERGE] Start (from " + webserver.client().remoteIP().toString() + ")");
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
                    DEBUG_PRINTLN("[PRESET-MERGE] Could not read file (from " + webserver.client().remoteIP().toString() + ")");
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
                        DEBUG_PRINTLN("[PRESET-MERGE] Skipped (clock face not found): " + name + " (from " + webserver.client().remoteIP().toString() + ")");
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
                        DEBUG_PRINTLN("[PRESET-MERGE] No free slot left - aborted (preset: " + name + ") (from " + webserver.client().remoteIP().toString() + ")");
                        break;
                    }

                    presets[freeIndex].name = name;
                    presets[freeIndex].url = url;
                    addedCount++;
                    DEBUG_PRINTLN("[PRESET-MERGE] Added: " + name + " (from " + webserver.client().remoteIP().toString() + ")");
                }
                readFile.close();
                LittleFS.remove(PRESET_IMPORT_TMP_PATH);

                if (addedCount > 0) {
                    savePresets();
                }
                presetImportSuccess = true; // Auch bei 0 neuen Presets kein Fehler (z.B. alles schon vorhanden)
                                            // 0 new presets is also not an error (e.g. everything already existed)
                DEBUG_PRINTLN("[PRESET-MERGE] " + String(addedCount) + " new presets added, " + String(skippedCount) + " skipped (from " + webserver.client().remoteIP().toString() + ")");
            }
            else {
                DEBUG_PRINTLN("[PRESET-MERGE] Failed while writing (from " + webserver.client().remoteIP().toString() + ")");
            }
        }
    }
