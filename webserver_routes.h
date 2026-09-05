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
        html.reserve(5000);  // Header: Dark-Theme + Tab-CSS + Topbar-CSS, einmal pro Seite aufgerufen (angehoben: zusaetzliche CSS-Regeln fuer reduzierte Bewegung/Offline-Hinweis)
                             // Header: dark theme + tab CSS + topbar CSS, called once per page
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
        html += ":root{--bg:#10151a;--panel:#1a2129;--panel-border:#2a333c;--text:#e8edf2;--muted:#8f9ba7;--accent:#f5a623;--accent-dim:#7a530f;--ok:#3ddc84;--bad:#ff5c5c;--mono:ui-monospace,\"SFMono-Regular\",Menlo,Consolas,monospace;}";
        // Bewusst KEIN padding-top auf body: die Topbar ist das erste Kind
        // von body, ein padding-top wuerde (da position:sticky den
        // Textfluss nicht verlaesst) auch sie selbst nach unten schieben
        // statt nur den Inhalt darunter - dann waere sie beim ersten Laden
        // nicht mehr ganz oben. Die frueher hier noetige Zusatz-Reservierung
        // fuer zwei fest positionierte Ecken-Boxen (Live-Vorschau/Status)
        // ist entfallen, seit diese Boxen komplett entfernt wurden (siehe
        // generateTopBar()).

        // Deliberately NO padding-top on body: the topbar is body's first
        // child, and a padding-top would (since position:sticky never
        // leaves the flow) push it down too, not just the content below it
        // - it would then no longer be at the very top on first load. The
        // extra reservation this used to need for two fixed-position
        // corner boxes (live preview/status) is gone now that those boxes
        // were removed entirely (see generateTopBar()).
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
        // --- Statuszeile (Topbar), sitenweit ganz oben auf jeder Seite -
        // siehe generateTopBar() weiter unten fuer die Markup-Erzeugung.
        // Die frueheren zwei fest positionierten Ecken-Boxen (Live-Vorschau
        // links/Status rechts) sind komplett entfernt - die Topbar ist die
        // einzige feste Statusanzeige der Seite, daher kein Extra-Abstand
        // (margin/padding) mehr noetig, nur der normale Textfluss.
        // --- Status bar (topbar), site-wide at the very top of every page -
        // see generateTopBar() further below for the markup generation.
        // The former two fixed-position corner boxes (live preview left/
        // status right) have been removed entirely - the topbar is now the
        // page's only fixed status display, so no extra spacing (margin/
        // padding) is needed beyond normal text flow.
        html += ".topbar{display:flex;flex-wrap:wrap;align-items:center;gap:.6rem 1.2rem;padding:.9rem 1rem;border-bottom:1px solid var(--panel-border);position:sticky;top:0;background:var(--bg);z-index:5;}";
        html += ".brand{display:flex;align-items:baseline;gap:.5rem;margin-right:auto;}";
        html += ".brand-mark{font-family:var(--mono);color:var(--accent);font-size:.8rem;letter-spacing:.05em;}";
        html += ".topbar h1{font-size:1.15rem;margin:0;letter-spacing:.02em;}";
        html += ".topbar h1 a.host-link{color:inherit;text-decoration:none;border-bottom:1px dashed var(--muted);}";
        html += ".topbar h1 a.host-link:hover{color:var(--accent);border-bottom-color:var(--accent);}";
        html += ".ip-hint{font-family:var(--mono);font-size:.7rem;color:var(--muted);}";
        // font-size hier bewusst "inherit" (statt weggelassen) - sonst wuerde
        // die allgemeine Regel "a{color:var(--accent);}" weiter unten zwar
        // nicht die Schriftgroesse aendern (Browser erben die ohnehin von
        // .ip-hint), aber explizit ist hier sicherer als sich auf die
        // Vererbung zu verlassen, falls spaeter eine andere Regel dazwischenfunkt.
        // font-size here deliberately "inherit" (rather than omitted) -
        // without it, the general "a{color:var(--accent);}" rule further
        // below would not actually change the font size (browsers inherit
        // it from .ip-hint anyway), but being explicit here is safer than
        // relying on inheritance in case another rule interferes later.
        html += ".ip-hint a{color:inherit;font-size:inherit;text-decoration:none;border-bottom:1px dashed var(--muted);}";
        html += ".ip-hint a:hover{color:var(--accent);border-bottom-color:var(--accent);}";
        html += ".status-strip{display:flex;gap:.9rem;flex-wrap:wrap;}";
        html += ".status{display:flex;align-items:center;gap:.4rem;font-size:.8rem;color:var(--muted);}";
        // .status setzt selbst "display:flex" - das UEBERSCHREIBT das native
        // 'hidden'-Attribut auf demselben Element (siehe #status-rtc/
        // #status-dcf77 in generateTopBar()): das Browser-Standard-CSS setzt
        // "hidden" nur ueber eine sehr niedrig priorisierte User-Agent-Regel
        // ([hidden]{display:none}) um, die von JEDER eigenen "display"-Angabe
        // auf demselben Element ausgehebelt wird - unabhaengig von
        // Selektor-Spezifitaet, da Autoren-CSS grundsaetzlich vor
        // User-Agent-CSS gewinnt. Ohne diese Regel blieb ein per 'hidden'
        // eigentlich verstecktes RTC/DCF77-Element also trotzdem sichtbar
        // (als roter Punkt, da .dot ohne Zusatzklasse rot ist) - genau der
        // gemeldete Bug ("RTC/DCF77 werden trotz 'nicht vorhanden' weiter
        // angezeigt"), obwohl rtcOk/dcf77Confirmed serverseitig korrekt
        // "nicht vorhanden" waren (siehe getRtcStatus()/getDcf77Status()).
        // Explizite Regel stellt das 'hidden'-Verhalten wieder her.
        // .status itself sets "display:flex" - that OVERRIDES the native
        // 'hidden' attribute on the same element (see #status-rtc/
        // #status-dcf77 in generateTopBar()): the browser's default CSS only
        // implements "hidden" via a very low-priority user-agent rule
        // ([hidden]{display:none}), which gets overridden by ANY "display"
        // declaration on that same element from author CSS - regardless of
        // selector specificity, since author CSS always wins over user-agent
        // CSS. Without this rule, an RTC/DCF77 element that was actually
        // meant to be hidden stayed visible anyway (as a red dot, since
        // .dot with no extra class is red) - exactly the reported bug ("RTC/
        // DCF77 still shown despite being 'not available'"), even though
        // rtcOk/dcf77Confirmed were correctly "not available" server-side
        // (see getRtcStatus()/getDcf77Status()). This rule explicitly
        // restores the 'hidden' behavior.
        html += ".status[hidden]{display:none;}";
        html += ".dot{width:.55rem;height:.55rem;border-radius:50%;background:var(--bad);box-shadow:0 0 0 3px rgba(255,92,92,.15);}";
        html += ".dot.ok{background:var(--ok);box-shadow:0 0 0 3px rgba(61,220,132,.18);}";
        // "na" (nicht verfuegbar): nur noch fuer den "Zeit"-Punkt relevant
        // (Systemzeit seit Boot noch nie gesetzt, siehe getTimeStatus()
        // unten) - ein voruebergehender Startzustand, kein optionales
        // Bauteil. Bewusst GRAU statt rot: rot (die Standardfarbe von .dot
        // ohne Zusatzklasse) ist reserviert fuer einen echten Fehler
        // (Hardware war/ist ansprechbar, liefert aber ungueltige Werte bzw.
        // ist waehrend des Betriebs ausgefallen) - "gar nicht vorhanden" und
        // "kaputt" sind unterschiedliche Zustaende und verdienen
        // unterschiedliche Farben. RTC und DCF77 nutzen "na" NICHT (mehr) fuer
        // einen grauen Punkt: bei beiden ist "Hardware fehlt" der Normalfall
        // auf einem Board ohne dieses optionale Bauteil - dafuer bleibt der
        // ganze Eintrag komplett unsichtbar (hidden), statt dauerhaft grau
        // angezeigt zu werden, siehe getRtcStatus()/getDcf77Status()/
        // generateTopBar() unten.
        // "na" (not available): now only relevant for the "Time" dot (system
        // time never set since boot, see getTimeStatus() below) - a
        // temporary startup state, not optional hardware. Deliberately GRAY
        // instead of red: red (the default color of a plain .dot with no
        // extra class) is reserved for a genuine error (hardware was/is
        // reachable but returns invalid values, or failed during operation)
        // - "not present at all" and "broken" are different states and
        // deserve different colors. RTC and DCF77 no longer use "na" for a
        // gray dot: for both, "hardware missing" is the normal case on a
        // board without that optional add-on - so the whole entry stays
        // fully hidden instead of permanently gray, see
        // getRtcStatus()/getDcf77Status()/generateTopBar() below.
        html += ".dot.na{background:var(--muted);box-shadow:0 0 0 3px rgba(143,155,167,.18);}";
        // Live-Wertanzeige (z.B. Fotowiderstand-Helligkeit) statt farbigem
        // Punkt - heller Monospace-Text, analog zum Kontrast von .datetime
        // gegenueber seinem Muted-Label.
        // Live value reading (e.g. photoresistor brightness) instead of a
        // colored dot - bright monospace text, mirroring the contrast of
        // .datetime against its muted label.
        html += ".statval{font-family:var(--mono);color:var(--text);}";
        // "syncing" (bisher nur DCF77): Empfaenger bekommt Impulse, hat aber
        // noch kein vollstaendiges, gueltiges Zeittelegramm dekodiert (dauert
        // durch das DCF77-Protokoll bis zu einer Minute) - gelb blinkend statt
        // rot, damit man den Unterschied zu "gar kein Empfang" sieht.
        // "syncing" (currently only DCF77): the receiver is getting pulses
        // but has not yet decoded a complete, valid time telegram (the
        // DCF77 protocol can take up to a minute) - blinking yellow instead
        // of red, so it's visibly different from "no reception at all".
        html += ".dot.syncing{background:var(--accent);box-shadow:0 0 0 3px rgba(245,166,35,.18);animation:dotBlink 1s ease-in-out infinite;}";
        html += "@keyframes dotBlink{0%,100%{opacity:1;}50%{opacity:.25;}}";
        // Wer in den Systemeinstellungen reduzierte Bewegung wuenscht (prefers-
        // reduced-motion), bekommt statt des Blinkens eine feste, gedimmte
        // Opazitaet - der Zustand ("syncing") bleibt ueber Punktfarbe/Tooltip
        // weiterhin erkennbar, nur ohne die Animation.
        // Anyone with reduced motion enabled in their system settings
        // (prefers-reduced-motion) gets a fixed, dimmed opacity instead of the
        // blink - the "syncing" state stays recognizable via dot color/tooltip,
        // just without the animation.
        html += "@media (prefers-reduced-motion:reduce){.dot.syncing{animation:none;opacity:.6;}}";
        html += ".datetime{font-family:var(--mono);font-size:.8rem;color:var(--muted);}";
        // Wird per JS eingeblendet, wenn /api/topbarStatus mehrfach in Folge
        // nicht erreichbar war (siehe Live-Status-Skript in generateTopBar())
        // - macht sichtbar, dass die angezeigten Werte (Punkte, Uhrzeit)
        // moeglicherweise nicht mehr aktuell sind, statt das stillschweigend
        // beim letzten bekannten Stand einzufrieren.
        // Shown via JS once /api/topbarStatus has failed repeatedly in a row
        // (see the live-status script in generateTopBar()) - makes it visible
        // that the displayed values (dots, time) may be stale, instead of
        // silently freezing at the last known state.
        html += ".offline-hint{display:none;color:var(--bad);font-size:.75rem;margin-left:.3rem;}";
        html += ".offline-hint.show{display:inline;}";
        // Checkbox/Buttons hier explizit margin/padding auf 0 setzen - sonst
        // greift die allgemeine Regel "input,select,button{margin:10px;padding:10px;...}"
        // weiter oben und sprengt die kompakte Topbar.
        // Explicitly zero out margin/padding here - otherwise the general
        // "input,select,button{margin:10px;padding:10px;...}" rule further
        // above applies and blows up the compact topbar.
        html += ".reset-btn{background:transparent;border:1px solid var(--panel-border);color:var(--bad);border-radius:.4rem;width:2rem;height:2rem;padding:0;margin:0;font-size:1rem;line-height:1;cursor:pointer;}";
        html += ".reset-btn:hover{border-color:var(--bad);background:rgba(255,92,92,.12);}";
        html += ".reset-btn:focus-visible{outline:2px solid var(--bad);outline-offset:1px;}";
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
        for (const char* t : { "status", "log", "wlan", "zifferblatt", "helligkeit", "zeit" }) {
            html += String("#tab-") + t + ":checked ~ .tabnav label[for='tab-" + t + "']{background:var(--accent);color:#1a1200;}";
            html += String("#tab-") + t + ":checked ~ .panel-" + t + "{display:block;}";
        }
        html += "</style>" + extraHead + "</head><body>";

        // Statuszeile ganz oben, noch vor der JavaScript-Warnung - sitenweit,
        // da generateHtmlHeader() von JEDER Seite eingebunden wird (siehe
        // generateTopBar() weiter unten).
        // Status bar at the very top, even before the JavaScript warning -
        // site-wide, since generateHtmlHeader() is included by EVERY page
        // (see generateTopBar() further below).
        html += generateTopBar();

        // Seite benötigt JavaScript
        // Page requires JavaScript
        html += "<noscript><div style='color:red;font-weight:bold;margin:20px;'>" +
                translate("JavaScript is disabled. This page requires JavaScript to work properly!") + "</div></noscript>";

        return html;
    }


    // Vier moegliche Status-Werte fuer jeden Punkt in der Topbar - gemeinsames
    // Vokabular fuer generateTopBar() (Erstladen) und /api/topbarStatus (5s-
    // Live-Poll), damit Text und Punktfarbe nie auseinanderlaufen:
    //
    // "ok"      - gruen  - funktioniert.
    // "syncing" - gelb, blinkend - versucht gerade aktiv zu synchronisieren.
    // "bad"     - rot (Standardfarbe von .dot ohne Zusatzklasse) - echter
    //             Fehler: die Hardware/Verbindung WAR bzw. IST grundsaetzlich
    //             ansprechbar, liefert aber ungueltige Werte oder ist waehrend
    //             des Betriebs ausgefallen.
    // "na"      - grau (nur bei "Zeit"/getTimeStatus()) bzw. der ganze
    //             Eintrag komplett unsichtbar (RTC, DCF77) - nicht
    //             verfuegbar: bei "Zeit" bedeutet "na" "seit Boot noch nie
    //             gesetzt" (ein voruebergehender Startzustand, kein
    //             optionales Bauteil, daher weiterhin ein grauer Punkt). Bei
    //             RTC/DCF77 dagegen ist das Fehlen der Hardware der
    //             Normalfall auf einem Board ohne diese optionale
    //             Zusatzhardware - dafuer erscheint dort gar kein Eintrag,
    //             statt eines dauerhaft grauen Punktes (siehe getRtcStatus()/
    //             getDcf77Status() weiter unten sowie die 'hidden'-Logik in
    //             generateTopBar()). Kein echter Fehler in beiden Faellen,
    //             sondern schlicht "nichts da".
    //
    // Vier possible status values for every dot in the topbar - shared
    // vocabulary for generateTopBar() (initial render) and /api/topbarStatus
    // (5s live poll), so the text and the dot color never drift apart:
    //
    // "ok"      - green  - working.
    // "syncing" - yellow, blinking - actively trying to synchronize right now.
    // "bad"     - red (the default color of a plain .dot with no extra class)
    //             - a genuine error: the hardware/connection WAS or IS
    //             fundamentally reachable, but returns invalid values or
    //             failed during operation.
    // "na"      - gray (only for "Time"/getTimeStatus()) resp. the whole
    //             entry fully hidden (RTC, DCF77) - not available: for
    //             "Time", "na" means "never set since boot" (a temporary
    //             startup state, not optional hardware, so it still gets a
    //             gray dot). For RTC/DCF77, however, missing hardware is the
    //             normal case on a board without that optional add-on - so
    //             no entry appears at all there, instead of a permanently
    //             gray dot (see getRtcStatus()/getDcf77Status() below and
    //             the 'hidden' logic in generateTopBar()). Not a genuine
    //             error in either case, just "nothing there".

    // Ermittelt den Zeit-Status ("ok"/"na") - kein eigener Fehlerzustand
    // definiert, da es fuer die reine Systemzeit kein Konzept von "gefunden,
    // aber ungueltig" gibt (anders als bei RTC/DCF77).
    // Determines the time status ("ok"/"na") - no separate error state
    // defined, since there's no "found but invalid" concept for plain system
    // time (unlike RTC/DCF77).

    String getTimeStatus() {
        return (timeinfo.tm_year > 0) ? "ok" : "na";
    }


    // Ermittelt den RTC-Status ("ok"/"bad"/"na") aus rtcOk (siehe globals.h,
    // wird von checkRtcHealth() in time_sync.h laufend aktualisiert):
    // RTC_AVAILABLE -> "ok", RTC_AVAILABLE_BUT_INVALID (Chip gefunden, aber
    // Zeit ungueltig/Stromausfall) -> "bad" (echter Fehler), RTC_NOT_AVAILABLE
    // (nie gefunden, oder von checkRtcHealth() als nicht mehr ansprechbar
    // erkannt) -> "na". generateTopBar() zeigt bei "na" KEINEN grauen Punkt
    // (mehr) an, sondern blendet den kompletten RTC-Eintrag aus (hidden) -
    // ein Board ohne RTC-Chip soll in der Topbar gar nicht erst auftauchen,
    // nicht als "grau/nicht verfuegbar" markiert sein.

    // Determines the RTC status ("ok"/"bad"/"na") from rtcOk (see globals.h,
    // kept current by checkRtcHealth() in time_sync.h): RTC_AVAILABLE -> "ok",
    // RTC_AVAILABLE_BUT_INVALID (chip found, but time invalid/lost power) ->
    // "bad" (genuine error), RTC_NOT_AVAILABLE (never found, or detected as no
    // longer reachable by checkRtcHealth()) -> "na". generateTopBar() no
    // longer shows a gray dot for "na" - instead it hides the whole RTC entry
    // (hidden): a board with no RTC chip should not appear in the topbar at
    // all, rather than being marked "gray/not available".

    String getRtcStatus() {
#if defined SDA_PIN && defined SCL_PIN
        if (rtcOk == RTC_AVAILABLE) return "ok";
        if (rtcOk == RTC_AVAILABLE_BUT_INVALID) return "bad";
        return "na";
#else
        return "na";
#endif
    }


    // Ermittelt den DCF77-Status ("ok"/"syncing"/"bad"/"na") - gemeinsame
    // Logik fuer generateTopBar() (Erstladen) und /api/topbarStatus (5s-Live-
    // Poll), vorher an beiden Stellen dupliziert. Beruecksichtigt zusaetzlich,
    // ob die letzte erfolgreiche Dekodierung (lastDcfSyncTime) bzw. der letzte
    // tatsaechlich empfangene Impuls (lastDcf77PulseChangeMillis, siehe
    // checkDcf77Health() in time_sync.h) noch aktuell genug sind (siehe
    // DCF77_SYNC_STALE_AFTER/DCF77_PULSE_STALE_AFTER in config.h) - vorher
    // blieb der Punkt nach dem ersten erfolgreichen Sync fuer immer gruen
    // (dcfTimeFound wird nie zurueckgesetzt) bzw. nach dem ersten jemals
    // empfangenen Impuls fuer immer mindestens gelb (dcf77Count wird nie auf 0
    // zurueckgesetzt), selbst wenn der Empfang spaeter waehrend des Betriebs
    // komplett abbricht.
    //
    // "ok": zuletzt erfolgreich dekodiert, nicht laenger als
    //   DCF77_SYNC_STALE_AFTER her.
    // "syncing": entweder noch nie erfolgreich dekodiert, oder der letzte
    //   erfolgreiche Sync ist zu alt - aber es kommen noch Impulse an
    //   (innerhalb von DCF77_PULSE_STALE_AFTER), der Empfaenger versucht also
    //   noch.
    // "na": dcf77Confirmed (siehe globals.h/checkDcf77Health() in
    //   time_sync.h) ist noch false - es wurde noch KEINE Kette aus
    //   mehreren, aufeinanderfolgenden Impulsen mit plausiblem Abstand
    //   beobachtet. Ein Funkempfaenger laesst sich nicht aktiv anpingen, es
    //   gibt also keine Moeglichkeit zu pruefen, ob ueberhaupt eine Antenne
    //   angeschlossen ist, ausser auf echte Aktivitaet zu warten - und ein
    //   EINZELNER dcf77Count-Wechsel reicht dafuer nicht: ein floatender,
    //   nicht angeschlossener Datenpin kann durch Rauschen einzelne
    //   Interrupts ausloesen (siehe DCF77_PRESENCE_MIN_STREAK/
    //   DCF77_PRESENCE_MAX_GAP_MS in config.h fuer die Begruendung). generateTopBar()
    //   zeigt bei "na" DESHALB keinen grauen Punkt mehr, sondern blendet den
    //   kompletten DCF77-Eintrag aus (hidden), bis dcf77Confirmed true wird -
    //   siehe dort.
    // "bad": es GAB bereits Empfang bzw. einen erfolgreichen Sync, aber seit
    //   DCF77_PULSE_STALE_AFTER ist kein einziger Impuls mehr angekommen -
    //   echter Ausfall waehrend des Betriebs (z.B. Antenne abgerissen,
    //   Stoerquelle).

    // Determines the DCF77 status ("ok"/"syncing"/"bad"/"na") - shared logic
    // for generateTopBar() (initial render) and /api/topbarStatus (5s live
    // poll), previously duplicated in both places. Additionally takes into
    // account whether the last successful decode (lastDcfSyncTime) resp. the
    // last actually received pulse (lastDcf77PulseChangeMillis, see
    // checkDcf77Health() in time_sync.h) are still recent enough (see
    // DCF77_SYNC_STALE_AFTER/DCF77_PULSE_STALE_AFTER in config.h) - previously
    // the dot stayed green forever after the first successful sync
    // (dcfTimeFound is never reset) resp. at least yellow forever after the
    // first pulse ever received (dcf77Count is never reset to 0), even if
    // reception later failed completely during operation.
    //
    // "ok": last successfully decoded, no longer than DCF77_SYNC_STALE_AFTER
    //   ago.
    // "syncing": either never successfully decoded yet, or the last successful
    //   sync is too old - but pulses are still arriving (within
    //   DCF77_PULSE_STALE_AFTER), so the receiver is still trying.
    // "na": dcf77Confirmed (see globals.h / checkDcf77Health() in
    //   time_sync.h) is still false - no chain of several consecutive,
    //   plausibly-spaced pulses has been observed yet. A radio receiver
    //   can't be actively pinged, so there's no way to check whether an
    //   antenna is even connected other than waiting for real activity - and
    //   a SINGLE dcf77Count change isn't enough for that: a floating,
    //   unconnected data pin can trigger isolated interrupts from noise (see
    //   DCF77_PRESENCE_MIN_STREAK/DCF77_PRESENCE_MAX_GAP_MS in config.h for
    //   the reasoning). That's why generateTopBar() no longer shows a gray
    //   dot for "na" here - instead it hides the whole DCF77 entry until
    //   dcf77Confirmed becomes true, see there.
    // "bad": reception or a successful sync DID happen before, but not a
    //   single pulse has arrived for DCF77_PULSE_STALE_AFTER since - a
    //   genuine failure during operation (e.g. antenna disconnected,
    //   interference).

    String getDcf77Status() {
#if defined DCF77_DATAPIN && defined DCF77_INTERRUPT
        // dcf77Confirmed statt "lastDcf77PulseChangeMillis == 0": ein
        // EINZELNER Impuls (auch ein durch Rauschen auf einem floatenden Pin
        // ausgeloester) reicht nicht als Nachweis fuer echten Empfang - siehe
        // DCF77_PRESENCE_MIN_STREAK/DCF77_PRESENCE_MAX_GAP_MS in config.h.
        if (!dcf77Confirmed) return "na"; // noch keine plausible Impulskette beobachtet / no plausible pulse chain observed yet
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


    // Baut den Tooltip-/aria-label-Text fuer einen Status-Punkt ("Zeit: OK",
    // "DCF77: Synchronisiere", "RTC: Nicht verfuegbar", ...) - gemeinsam fuer
    // generateTopBar() und /api/topbarStatus, damit Text und Punktfarbe nie
    // auseinanderlaufen. state: "ok" | "syncing" | "bad" | "na" (siehe oben).

    // Builds the tooltip/aria-label text for a status dot ("Time: OK",
    // "DCF77: Syncing", "RTC: Not available", ...) - shared by
    // generateTopBar() and /api/topbarStatus, so the text and the dot color
    // never drift apart. state: "ok" | "syncing" | "bad" | "na" (see above).

    String dotStatusText(const String& label, const String& state) {
        String stateText = (state == "ok") ? translate("OK") :
                            (state == "syncing") ? translate("Syncing") :
                            (state == "na") ? translate("Not available") :
                            translate("Error");
        return label + ": " + stateText;
    }


    // Escaped Text fuer die sichere Einbettung in HTML (z.B. Logdatei-Inhalt
    // im <pre id='logContent'> des Log-Tabs, siehe unten) - Logzeilen koennen
    // beliebige Zeichen enthalten (z.B. "<" aus geloggten Werten), ohne
    // Escaping wuerde das das umgebende Markup brechen.

    // Escapes text for safe embedding in HTML (e.g. log file content inside
    // the Log tab's <pre id='logContent'>, see below) - log lines can contain
    // arbitrary characters (e.g. "<" from logged values), without escaping
    // that would break the surrounding markup.

    // Escapt zusaetzlich Anfuehrungszeichen (' und ") - urspruenglich nur fuer
    // reinen Textinhalt gedacht (z.B. Logzeilen in <pre>), inzwischen aber auch
    // an mehreren Stellen fuer Werte benutzt, die in einfach gequotete HTML-
    // Attribute eingebettet werden (value='...', onclick='...'). Ohne
    // Escaping von "'" haette ein einzelnes Anfuehrungszeichen im Wert (z.B.
    // in einem Dateinamen, Preset-Namen, Hostnamen oder einer WLAN-SSID) das
    // Attribut vorzeitig beendet und zusaetzliche, vom Angreifer kontrollierte
    // Attribute wie onerror='...'/onfocus='...' injizieren koennen - reines
    // "<"/">"-Escaping verhindert zwar das Einschleusen eines neuen <script>-
    // Tags, aber NICHT diese Art von Attribut-Injection.

    // Also escapes quote characters (' and ") - originally meant only for
    // plain text content (e.g. log lines inside <pre>), but by now also used
    // in several places for values embedded into single-quoted HTML
    // attributes (value='...', onclick='...'). Without escaping "'", a single
    // quote character in the value (e.g. in a filename, preset name, hostname,
    // or WiFi SSID) could terminate the attribute early and inject additional,
    // attacker-controlled attributes like onerror='...'/onfocus='...' -
    // escaping only "<"/">" prevents injecting a new <script> tag, but NOT
    // this kind of attribute injection.
    // Fuer Werte, die in ein onclick='...'-HTML-Attribut eingebettet werden,
    // das INTERN wiederum einen JS-String-Literal enthaelt (z.B.
    // onclick=\"fn('TEXT')\" oder onclick='fn(\"TEXT\")') - ein einfaches
    // escapeHtmlText() reicht hier NICHT: der Browser dekodiert HTML-
    // Entities in einem Attributwert, BEVOR der Inhalt als JS geparst wird -
    // ein per &#39; entity-escapetes Anfuehrungszeichen wird also vor der
    // JS-Auswertung wieder zu einem rohen "'" und wuerde den inneren
    // JS-String trotzdem vorzeitig beenden. Deshalb: das AEUSSERE HTML-
    // Anfuehrungszeichen per Entity escapen (verhindert ein vorzeitiges Ende
    // des Attributs schon beim HTML-Parsing, unabhaengig von Entity-
    // Dekodierung), das INNERE JS-Anfuehrungszeichen per Backslash escapen
    // (verhindert ein vorzeitiges Ende des JS-Strings NACH der Dekodierung).
    // jsStringQuote gibt an, welches Zeichen den inneren JS-String begrenzt
    // ('...' oder "...") - das jeweils andere gilt automatisch als aeusseres
    // HTML-Attribut-Anfuehrungszeichen.

    // For values embedded into an onclick='...' HTML attribute that itself
    // contains a JS string literal (e.g. onclick=\"fn('TEXT')\" or
    // onclick='fn(\"TEXT\")') - plain escapeHtmlText() is NOT enough: the
    // browser decodes HTML entities within an attribute value BEFORE the
    // content is parsed as JS - a quote character entity-escaped as &#39;
    // turns back into a raw "'" before JS evaluation and would still
    // terminate the inner JS string early. So: escape the OUTER HTML quote
    // via HTML entity (prevents the attribute from ending early during HTML
    // parsing, regardless of entity decoding), and escape the INNER JS quote
    // via backslash (prevents the JS string from ending early after
    // decoding). jsStringQuote says which character delimits the inner JS
    // string ('...' or "...") - the other one is automatically treated as
    // the outer HTML attribute quote.
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


    // Escaped einen String zur sicheren Einbettung als JS-String-Literal DIREKT
    // innerhalb eines <script>-Blocks (also NICHT in einem HTML-Attribut wie
    // onclick='...' - dafuer escapeForJsStringInAttr() oben verwenden). Wichtig:
    // <script>-Inhalte sind fuer den HTML-Parser ein "raw text element" - HTML-
    // Entities werden dort NICHT dekodiert, escapeHtmlText() waere hier also
    // wirkungslos (ein "&#39;" bliebe woertlich "&#39;" statt zu "'" zu werden).
    // Stattdessen: Backslash und das umschliessende Anfuehrungszeichen per
    // Backslash escapen (verhindert vorzeitiges Ende des JS-Strings), und "<"
    // zusaetzlich als "\x3C" escapen, damit eine Zeichenfolge wie "</script>"
    // im eingebetteten Text nicht das gesamte <script>-Element vorzeitig
    // beendet (das wuerde sonst beliebiges nachfolgendes HTML/JS ermoeglichen).
    //
    // Escapes a string for safe embedding as a JS string literal DIRECTLY
    // inside a <script> block (i.e. NOT inside an HTML attribute like
    // onclick='...' - use escapeForJsStringInAttr() above for that). Important:
    // <script> content is a "raw text element" for the HTML parser - HTML
    // entities are NOT decoded there, so escapeHtmlText() would be ineffective
    // here (an "&#39;" would stay literally "&#39;" instead of becoming "'").
    // Instead: backslash-escape the backslash and the enclosing quote character
    // (prevents the JS string from ending early), and additionally escape "<"
    // as "\x3C" so a sequence like "</script>" in the embedded text cannot
    // prematurely end the whole <script> element (which would otherwise allow
    // arbitrary following HTML/JS to be injected).
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


    // Erzeugt die Statuszeile (Topbar) - sitenweit oben auf jeder Seite (wird
    // aus generateHtmlHeader() direkt nach dem <body>-Tag eingebunden, siehe
    // dort). Zeigt: Projekt-/Geraetename, Hostname/IP (bzw. Access-Point im
    // Setup-Modus), je einen Status-Punkt pro unabhaengiger Zeitquelle (Zeit/
    // NTP, RTC, DCF77 - RTC/DCF77 nur fuer tatsaechlich verbaute Hardware,
    // siehe die #if-Bloecke unten; DCF77 zusaetzlich erst, sobald der erste
    // Impuls seit Boot beobachtet wurde, siehe dcfPresent unten), den
    // Live-Helligkeitswert des Fotowiderstands (falls vorhanden, siehe
    // photoresistorFound unten), die aktuelle Uhrzeit sowie manuelles
    // Neu-Laden und Neustart. KEIN WLAN-Punkt: laedt die Seite ueberhaupt,
    // ist WLAN per Definition aktiv - ein eigener Punkt waere redundant.
    //
    // Ersetzt die vormalige, ausfuehrlichere Statusbox oben rechts
    // (Speicherplatz, AP-Passwort, volle Hostname-/IP-Links) sowie das
    // Live-Zeiger-Vorschau-Widget oben links - beide wurden auf Wunsch
    // vollstaendig entfernt (generateHtmlStatus() geloescht, der
    // entsprechende Codeblock aus generateNavigation() entfernt); diese
    // Topbar ist seitdem die einzige feste Statusanzeige der Seite.

    // Generates the status bar (topbar) - site-wide at the top of every page
    // (included from generateHtmlHeader() right after the <body> tag, see
    // there). Shows: project/device name, hostname/IP (or the access point
    // in setup mode), one status dot per independent time source (time/NTP,
    // RTC, DCF77 - RTC/DCF77 only for hardware actually present, see the #if
    // blocks below; DCF77 additionally only once the first pulse since boot
    // has been observed, see dcfPresent below), the photoresistor's live
    // brightness value (if present, see photoresistorFound below), the
    // current time, plus manual reload and restart. NO WiFi dot: if the page
    // loaded at all, WiFi is by definition up - a dot for it would be
    // redundant.
    //
    // Replaces the former, more detailed status box top-right (storage
    // usage, AP password, full hostname/IP links) as well as the live
    // hand-preview widget top-left - both were removed entirely on request
    // (generateHtmlStatus() deleted, the corresponding block removed from
    // generateNavigation()); this topbar is now the page's only fixed
    // status display.

    String generateTopBar() {
        String html;
        html.reserve(3600); // angehoben: Tooltip-/aria-label-Texte auf den Punkten sowie das erweiterte Live-Status-Skript (Offline-Erkennung, Sichtbarkeits-Pause)

        html += "<header class='topbar'>";
        html += "<div class='brand'>";
        html += "<span class='brand-mark'>UHR&middot;3</span>";

        bool staConnected = (WiFi.getMode() == WIFI_STA && WiFi.status() == WL_CONNECTED);

        if (staConnected && pingHostname) {
            html += "<h1><a class='host-link' href='http://" + String(hostname) + ".local/'>" + String(hostname) + "</a></h1>";
            html += "<span class='ip-hint'><a href='http://" + WiFi.localIP().toString() + "/'>" + WiFi.localIP().toString() + "</a></span>";
        }
        else if (staConnected) {
            // Hostname noch nicht bestaetigt (z.B. sehr kurz nach dem Verbinden) -
            // SSID als Platzhalter zeigen, damit die Zeile nicht leer bleibt.
            // Hostname not confirmed yet (e.g. very shortly after connecting) -
            // show the SSID as a placeholder so the line doesn't stay empty.
            html += "<h1>" + WiFi.SSID() + "</h1>";
            html += "<span class='ip-hint'><a href='http://" + WiFi.localIP().toString() + "/'>" + WiFi.localIP().toString() + "</a></span>";
        }
        else {
            // Access-Point-/Einrichtungsmodus: kein mDNS-Hostname verfuegbar,
            // daher unverlinkt. Das AP-Passwort steht bewusst NICHT hier -
            // frueher wurde es in der (inzwischen entfernten) Statusbox aus
            // generateHtmlStatus() angezeigt; seit deren Entfernung steht es
            // ueberhaupt nicht mehr im Webinterface (nur noch auf dem
            // Display waehrend des AP-Bildschirms, siehe startAP() in
            // wifi_manager.h).
            // Access point/setup mode: no mDNS hostname available, so plain
            // text. The AP password deliberately does NOT appear here -
            // it used to be shown in the (now removed) status box from
            // generateHtmlStatus(); since that box was removed it no longer
            // appears anywhere in the web UI (only on the display during
            // the AP screen, see startAP() in wifi_manager.h).
            html += "<h1>Access Point</h1>";
            html += "<span class='ip-hint'><a href='http://" + WiFi.softAPIP().toString() + "/'>" + WiFi.softAPIP().toString() + "</a></span>";
        }
        html += "</div>";

        html += "<div class='status-strip'>";

        // Systemzeit gilt als plausibel gesetzt, sobald tm_year > 0 ist (0 =
        // seit Boot nie gesetzt, siehe struct tm-Default in globals.h) -
        // unabhaengig davon, ob die Zeit zuletzt von NTP, RTC oder DCF77 kam.
        // System time counts as plausibly set once tm_year > 0 (0 = never set
        // since boot, see the struct tm default in globals.h) - regardless of
        // whether it last came from NTP, RTC or DCF77.
        // IDs auf den drei Status-Punkten ("dot-time"/"dot-rtc"/"dot-dcf77") -
        // werden vom Live-Status-Skript weiter unten gebraucht, um bei einem
        // Ausfall waehrend des Betriebs (WLAN weg, RTC/DCF77 verliert Sync)
        // die Farbe periodisch zu aktualisieren, ohne die ganze Seite neu zu
        // laden (Auto-Refresh gibt es hier bewusst nicht mehr, siehe
        // generateTopBar()-Kommentar oben).
        // IDs on the three status dots ("dot-time"/"dot-rtc"/"dot-dcf77") -
        // needed by the live-status script further below to periodically
        // update the color if something fails during operation (WiFi drops,
        // RTC/DCF77 loses sync), without reloading the whole page (there is
        // deliberately no more auto-refresh here, see the generateTopBar()
        // comment above).
        // title (Hover-Tooltip) + role='img'/aria-label auf jedem Punkt: die
        // Bedeutung der Punktfarbe war vorher NUR ueber die Farbe selbst
        // erkennbar - fuer Farbfehlsichtige oder Screenreader-Nutzer nicht
        // zugaenglich. dotStatusText()/getDcf77Status() siehe oben; das
        // Live-Status-Skript weiter unten aktualisiert title/aria-label bei
        // jedem Poll mit, damit sie nie von der tatsaechlichen Punktfarbe
        // abweichen.
        // title (hover tooltip) + role='img'/aria-label on every dot: the
        // meaning of the dot color was previously only conveyed by the color
        // itself - not accessible to colorblind users or screen readers.
        // dotStatusText()/getDcf77Status() see above; the live-status script
        // further below keeps title/aria-label up to date on every poll so
        // they never drift from the dot's actual color.
        // Status kommt aus getTimeStatus()/getRtcStatus()/getDcf77Status()
        // (siehe oben) - vier moegliche Werte ("ok"/"syncing"/"bad"/"na"), CSS-
        // Klasse und Punktfarbe siehe ".dot"/".dot.ok"/".dot.syncing"/".dot.na"
        // weiter oben in generateHtmlHeader(). "bad" bekommt bewusst KEINE
        // Zusatzklasse - das ist die Standardfarbe (rot) von ".dot" selbst.
        // Status comes from getTimeStatus()/getRtcStatus()/getDcf77Status()
        // (see above) - four possible values ("ok"/"syncing"/"bad"/"na"), CSS
        // class and dot color see ".dot"/".dot.ok"/".dot.syncing"/".dot.na"
        // further above in generateHtmlHeader(). "bad" deliberately gets NO
        // extra class - that's plain ".dot"'s own default color (red).
        // "Zeit" wird bewusst als LETZTER Eintrag im Status-Strip gerendert
        // (direkt vor #topbar-datetime), nicht als erster - so steht der
        // Zeit-Punkt optisch unmittelbar neben der Datumsanzeige, die er
        // betrifft, statt links vor den Hardware-Eintraegen (RTC/DCF77/
        // Licht) zu stehen. timeState/timeOk werden aber weiterhin HIER
        // berechnet (nicht erst spaeter), da timeOk unten bei der
        // #topbar-datetime-Befuellung gebraucht wird - nur die HTML-Ausgabe
        // des Punkts selbst wandert ans Ende des Status-Strips.
        // "Zeit" is deliberately rendered as the LAST entry in the status
        // strip (right before #topbar-datetime), not the first - so the
        // time dot sits visually right next to the date display it relates
        // to, instead of standing to the left of the hardware entries (RTC/
        // DCF77/Light). timeState/timeOk are still computed HERE (not
        // later), since timeOk is needed below when filling in
        // #topbar-datetime - only the dot's own HTML output moves to the
        // end of the status strip.
        String timeState = getTimeStatus();
        bool timeOk = (timeState == "ok"); // fuer die Datumsanzeige weiter unten wiederverwendet / reused for the date display further below
        String timeTitle = dotStatusText(translate("Time"), timeState);

#ifdef ADC_PIN
        // Live-Helligkeitswert des Fotowiderstands statt eines Status-Punkts
        // (siehe .statval-CSS oben) - photoresistorFound wird einmalig beim
        // Boot per Spannungsteiler-Test ermittelt (siehe uhr3.ino) und
        // aendert sich danach nicht mehr; anders als bei DCF77 ist das ein
        // echter, aktiver Hardware-Test (kein reines "noch keine Aktivitaet
        // beobachtet"), ein grauer "na"-Punkt waere hier also unnoetig -
        // ohne Fotowiderstand entfaellt der Eintrag einfach komplett.
        // Zusaetzlich an useAdc gekoppelt: ist Auto-Brightness deaktiviert,
        // legt der Boot-Code ADC_GND/ADC_3V auf INPUT (siehe uhr3.ino) - der
        // Spannungsteiler ist dann unbestromt und currentLightPercent wird
        // von updateBrightness() (display.h) nicht mehr aktualisiert, bliebe
        // also eingefroren stehen. Ohne useAdc daher lieber den Eintrag ganz
        // weglassen statt einen veralteten Wert zu zeigen.
        // "Licht" wird bewusst als ERSTER Eintrag im Status-Strip gerendert
        // (ganz links, direkt nach der Brand-Marke) - siehe Kommentar bei
        // timeState/timeTitle weiter oben zur Reihenfolge der uebrigen
        // Eintraege (RTC/DCF77/Zeit).
        // Live brightness value from the photoresistor instead of a status
        // dot (see the .statval CSS above) - photoresistorFound is
        // determined once at boot via a voltage-divider test (see
        // uhr3.ino) and never changes afterwards; unlike DCF77 this is a
        // genuine, active hardware test (not just "no activity observed
        // yet"), so a gray "na" dot would be pointless here - without a
        // photoresistor the entry is simply omitted entirely.
        // Also gated on useAdc: with auto-brightness disabled, the boot code
        // sets ADC_GND/ADC_3V to INPUT (see uhr3.ino) - the voltage divider
        // is then unpowered and currentLightPercent is no longer updated by
        // updateBrightness() (display.h), so it would stay frozen. Without
        // useAdc, omit the entry entirely rather than show a stale value.
        // "Light" is deliberately rendered as the FIRST entry in the status
        // strip (all the way to the left, right after the brand mark) - see
        // the comment at timeState/timeTitle further above for the ordering
        // of the remaining entries (RTC/DCF77/Time).
        if (photoresistorFound && useAdc) {
            html += "<span class='status'>" + translate("Light") + ": <span id='value-light' class='statval'>" + String(currentLightPercent) + " %</span></span>";
        }
#endif

#if defined SDA_PIN && defined SCL_PIN
        String rtcState = getRtcStatus();
        // rtcPresent: false nur solange rtcOk == RTC_NOT_AVAILABLE ist (siehe
        // getRtcStatus()) - der ganze Eintrag bleibt dann per 'hidden'
        // unsichtbar statt eines grauen "na"-Punkts (Board ohne RTC-Chip soll
        // in der Topbar gar nicht erst auftauchen). checkRtcHealth() prueft
        // NUR bei bereits gefundener RTC periodisch auf Ausfall (siehe dort)
        // und probiert eine beim Boot nicht gefundene RTC nicht erneut - der
        // Eintrag bleibt in dem Fall also dauerhaft ausgeblendet, bis zum
        // naechsten Neustart mit angeschlossener RTC.
        // rtcPresent: false only for as long as rtcOk == RTC_NOT_AVAILABLE
        // (see getRtcStatus()) - the whole entry then stays invisible via
        // 'hidden' instead of a gray "na" dot (a board with no RTC chip
        // should not appear in the topbar at all). checkRtcHealth() only
        // periodically re-checks for failure once an RTC was already found
        // (see there) and does not re-probe an RTC that was never found at
        // boot - so in that case the entry stays hidden permanently until
        // the next restart with an RTC actually connected.
        bool rtcPresent = (rtcState != "na");
        String rtcTitle = dotStatusText("RTC", rtcState);
        html += "<span class='status' id='status-rtc'" + String(rtcPresent ? "" : " hidden") + "><i id='dot-rtc' role='img' aria-label='" + rtcTitle + "' title='" + rtcTitle + "' class='dot";
        if (rtcState == "ok") html += " ok";
        html += "'></i>RTC</span>";
#endif

#if defined DCF77_DATAPIN && defined DCF77_INTERRUPT
        // beruecksichtigt neben den rohen dcfTimeFound/dcf77Count-Werten auch
        // deren zeitliche Aktualitaet (DCF77_SYNC_STALE_AFTER/
        // DCF77_PULSE_STALE_AFTER in config.h), damit der Punkt einen
        // Empfangsausfall waehrend des Betriebs auch tatsaechlich anzeigt.
        // besides the raw dcfTimeFound/dcf77Count values, also takes their
        // recency into account (DCF77_SYNC_STALE_AFTER/DCF77_PULSE_STALE_AFTER
        // in config.h), so the dot actually reflects a reception failure
        // occurring during operation.
        String dcfState = getDcf77Status();
        // dcfPresent: false nur solange noch nie (seit Boot) ein Impuls
        // angekommen ist (dcfState=="na", siehe getDcf77Status()). Ein
        // Funkempfaenger laesst sich nicht aktiv anpingen - es gibt also
        // keinen Weg zu wissen, ob ueberhaupt eine Antenne angeschlossen
        // ist, ausser auf echte Aktivitaet zu warten. Der ganze Eintrag
        // bleibt deshalb per natives 'hidden'-Attribut unsichtbar (statt
        // eines grauen "na"-Punkts, wie es RTC bekommt), bis der erste
        // Impuls beobachtet wurde - das Live-Status-Skript weiter unten
        // blendet ihn dann per setPresent() live ein, ohne Seiten-Reload.
        // Einmal sichtbar geworden bleibt er es dauerhaft (siehe
        // getDcf77Status()): ein spaeterer Empfangsausfall zeigt sich dann
        // als roter "bad"-Punkt, nicht durch erneutes Verstecken.
        // dcfPresent: false only for as long as not a single pulse has
        // arrived since boot (dcfState=="na", see getDcf77Status()). A radio
        // receiver can't be actively pinged - there's no way to know whether
        // an antenna is even connected other than waiting for real activity.
        // The whole entry therefore stays invisible via the native 'hidden'
        // attribute (instead of a gray "na" dot like RTC gets) until the
        // first pulse is observed - the live-status script further below
        // then reveals it live via setPresent(), no page reload needed. Once
        // revealed it stays revealed (see getDcf77Status()): a later
        // reception failure then shows as a red "bad" dot, not by hiding it
        // again.
        bool dcfPresent = (dcfState != "na");
        String dcfTitle = dotStatusText("DCF77", dcfState);
        html += "<span class='status' id='status-dcf77'" + String(dcfPresent ? "" : " hidden") + "><i id='dot-dcf77' role='img' aria-label='" + dcfTitle + "' title='" + dcfTitle + "' class='dot";
        if (dcfState == "ok") html += " ok";
        else if (dcfState == "syncing") html += " syncing";
        html += "'></i>DCF77</span>";
#endif

        // "Zeit"-Punkt ganz nach rechts verschoben - letzter Eintrag im
        // Status-Strip, direkt gefolgt von #topbar-datetime (siehe Kommentar
        // bei timeState/timeTitle weiter oben).
        // "Time" dot moved all the way to the right - last entry in the
        // status strip, immediately followed by #topbar-datetime (see the
        // comment at timeState/timeTitle further above).
        html += "<span class='status'><i id='dot-time' role='img' aria-label='" + timeTitle + "' title='" + timeTitle + "' class='dot";
        if (timeState == "ok") html += " ok";
        else if (timeState == "na") html += " na";
        html += "'></i>" + translate("Time") + "</span>";

        html += "</div>";

        // id='topbar-datetime' - immer rendern (auch leer, wenn die Zeit noch
        // nicht gesetzt ist), damit das Element im DOM existiert und das
        // Live-Status-Skript weiter unten den Text jede Runde aktualisieren
        // kann, ohne die Seite neu zu laden.
        // id='topbar-datetime' - always rendered (empty if the time isn't
        // set yet), so the element exists in the DOM and the live-status
        // script further below can update its text every round without
        // reloading the page.
        html += "<div id='topbar-datetime' class='datetime'>";
        if (timeOk) {
            char nowStr[20];
            strftime(nowStr, sizeof(nowStr), "%d.%m.%Y %H:%M", &timeinfo);
            html += String(nowStr);
        }
        html += "</div>";

        // Versteckter Hinweis, den das Live-Status-Skript weiter unten
        // einblendet, wenn /api/topbarStatus mehrfach in Folge fehlschlaegt -
        // siehe Kommentar bei ".offline-hint" oben in generateHtmlHeader().
        // Hidden hint, shown by the live-status script further below once
        // /api/topbarStatus has failed repeatedly in a row - see the
        // ".offline-hint" comment above in generateHtmlHeader().
        html += "<span id='topbar-offline-hint' class='offline-hint'>&#9888; " + translate("Connection lost") + "</span>";

        // Weder manueller Refresh-Knopf noch Auto-Refresh: ergeben in diesem
        // Projekt keinen Sinn, da alle live angezeigten Werte (Uhrzeit oben,
        // die rotierenden Zeiger auf /preview) ohnehin rein im Browser per
        // JavaScript weiterlaufen, ohne dass die Seite neu geladen werden
        // muesste - ein Refresh wuerde nichts zeigen, was nicht schon aktuell ist.
        // Neustart bleibt (siehe reset-btn) - das ist keine reine Anzeige-
        // Aktualisierung, sondern ein echter Geraete-Neustart.

        // Neither a manual refresh button nor auto-refresh: pointless in this
        // project, since every live value shown (the time above, the
        // rotating hands on /preview) already keeps itself current purely in
        // the browser via JavaScript, with no need to reload the page - a
        // refresh wouldn't show anything that isn't already current.
        // Restart stays (see reset-btn) - that's not a display refresh, it's
        // an actual device restart.
        html += "<button type='button' class='reset-btn' onclick='if(confirm(\"" + translate("Are you sure you want to reboot?") + "\")){location.href=\"/reboot\";}' title='" + translate("Reboot") + "'>&#9211;</button>";

        html += "</header>";

        // Live-Status fuer die drei Punkte (Zeit/RTC/DCF77) UND die
        // Uhrzeit-Anzeige: fragt periodisch /api/topbarStatus ab und
        // aktualisiert die "ok"/"syncing"-Klassen der Punkte sowie den Text
        // von #topbar-datetime - kein Neuladen der Seite noetig (Auto-Refresh
        // gibt es hier bewusst nicht mehr, siehe Kommentar weiter oben). So
        // wechselt die Punktfarbe auch, wenn waehrend des Betriebs etwas
        // ausfaellt oder nicht mehr erreichbar ist (WLAN weg -> die
        // Verbindung selbst bricht dann ohnehin ab, aber RTC/DCF77 koennen
        // unabhaengig davon ihre Synchronisation verlieren), und die
        // angezeigte Uhrzeit bleibt aktuell, ohne staendig die ganze Seite
        // neu zu laden. 5s-Intervall: haeufig genug, um einen Ausfall
        // zeitnah zu zeigen bzw. die Minutenanzeige aktuell zu halten, aber
        // unauffaellig fuer den ESP32.
        // getElementById liefert null fuer Punkte, deren Hardware auf diesem
        // Board nicht verbaut ist (kein #if im DOM) - werden dann einfach
        // uebersprungen.

        // Live status for the three dots (time/RTC/DCF77) AND the displayed
        // time: periodically polls /api/topbarStatus and updates the dots'
        // "ok"/"syncing" classes as well as #topbar-datetime's text - no page
        // reload needed (there is deliberately no more auto-refresh here, see
        // the comment further above). This way the dot color also changes if
        // something fails or becomes unreachable during operation (WiFi
        // dropping breaks the connection itself anyway, but RTC/DCF77 can
        // independently lose their sync), and the displayed time stays
        // current without reloading the whole page over and over. 5s
        // interval: frequent enough to show a failure promptly and keep the
        // minute display current, without being excessive for the ESP32.
        // getElementById returns null for dots whose hardware isn't present
        // on this board (no #if in the DOM) - those are simply skipped.
        // setStatusDot(): gemeinsame Funktion fuer alle drei Punkte (Zeit,
        // RTC, DCF77) - state ist einer von "ok"/"syncing"/"bad"/"na" (siehe
        // dotStatusText()-Kommentar oben), "bad" bekommt bewusst KEINE
        // Zusatzklasse (Standardfarbe rot von ".dot" selbst). Aktualisiert
        // zusaetzlich title/aria-label mit dem vom Server mitgeschickten,
        // uebersetzten Text (siehe dotStatusText() oben), damit Tooltip/
        // Screenreader-Text nie von der Punktfarbe abweichen.
        // setOnline(): blendet den Offline-Hinweis ein/aus (siehe
        // "#topbar-offline-hint" oben) - nach zwei aufeinanderfolgenden
        // fehlgeschlagenen Polls (~10s bei 5s-Intervall), damit ein einzelner
        // kurzer Netzwerk-Hakler nicht sofort einen Alarm ausloest.
        // visibilitychange: pausiert das Polling, waehrend der Tab im
        // Hintergrund ist (unnoetige Anfragen an den ESP32 sparen) und holt
        // beim Zurueckkehren sofort den aktuellen Stand nach, statt bis zu 5s
        // auf den naechsten Intervall-Tick zu warten.
        // setStatusDot(): shared function for all three dots (time, RTC,
        // DCF77) - state is one of "ok"/"syncing"/"bad"/"na" (see the
        // dotStatusText() comment above), "bad" deliberately gets NO extra
        // class (plain ".dot"'s own default color, red). Also updates
        // title/aria-label with the translated text the server sends along
        // (see dotStatusText() above), so the tooltip/screen-reader text never
        // drifts from the dot's color.
        // setOnline(): shows/hides the offline hint (see
        // "#topbar-offline-hint" above) - after two consecutive failed polls
        // (~10s at the 5s interval), so a single brief network hiccup doesn't
        // immediately trigger an alarm.
        // visibilitychange: pauses polling while the tab is in the background
        // (saves pointless requests to the ESP32) and immediately catches up
        // on return instead of waiting up to 5s for the next interval tick.
        html += "<script>(function(){";
        html += "function setStatusDot(id,state,title){var el=document.getElementById(id);if(!el)return;el.classList.toggle('ok',state==='ok');el.classList.toggle('syncing',state==='syncing');el.classList.toggle('na',state==='na');if(title){el.title=title;el.setAttribute('aria-label',title);}}";
        // setPresent(): blendet einen Eintrag ohne aktiven Pruefweg (aktuell
        // nur DCF77 - siehe dcfPresent-Kommentar in generateTopBar()) per
        // natives 'hidden'-Attribut live ein, sobald der Server erstmals
        // echte Aktivitaet bestaetigt - kein Seiten-Reload noetig.
        // setPresent(): reveals an entry with no active probe path
        // (currently only DCF77 - see the dcfPresent comment in
        // generateTopBar()) live via the native 'hidden' attribute, the
        // moment the server first confirms real activity - no page reload.
        html += "function setPresent(id,present){var el=document.getElementById(id);if(!el)return;el.hidden=!present;}";
        // setValue(): aktualisiert eine Live-Wertanzeige (z.B. den
        // Fotowiderstand-Helligkeitswert, siehe .statval oben) statt einer
        // Punktfarbe - der Text kommt bereits fertig formatiert vom Server.
        // setValue(): updates a live value reading (e.g. the photoresistor
        // brightness value, see .statval above) instead of a dot color -
        // the text arrives already formatted from the server.
        html += "function setValue(id,text){var el=document.getElementById(id);if(!el)return;el.textContent=text;}";
        html += "function setOnline(ok){var h=document.getElementById('topbar-offline-hint');if(h)h.classList.toggle('show',!ok);}";
        html += "var failCount=0;";
        html += "function poll(){fetch('/api/topbarStatus').then(function(r){return r.json();}).then(function(s){";
        html += "failCount=0;setOnline(true);";
        html += "setStatusDot('dot-time',s.time,s.timeTitle);";
        html += "setPresent('status-rtc',s.rtcPresent);setStatusDot('dot-rtc',s.rtc,s.rtcTitle);";
        html += "setPresent('status-dcf77',s.dcf77Present);setStatusDot('dot-dcf77',s.dcf77,s.dcf77Title);";
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


    // Kurze, uebersetzte Zeile mit dem LittleFS-Speicherplatzverbrauch -
    // "used"/"total" werden vom Aufrufer uebergeben (viele Seiten haben sie
    // ohnehin schon fuer eine eigene Pruefung berechnet, z.B. den
    // Platzbedarf vor einem Upload) statt sie hier nochmal abzufragen.
    // Liefert reinen Inline-Text OHNE umschliessendes Element - der Aufrufer
    // entscheidet je nach Seitenlayout, ob er ihn in <p>...</p> (Dateiverwaltungs-
    // seiten) oder <li>...</li> (Status-Listen) einbettet. Eingefuehrt, nachdem
    // die alte Statusbox mit dieser Info (siehe generateTopBar()) komplett
    // entfernt wurde und die Info seither auf mehreren Seiten fehlte.

    // Short LittleFS-Speichernutzungszeile - "used"/"total" werden vom
    // Aufrufer uebergeben (viele Seiten berechnen sie ohnehin schon fuer
    // eine eigene Pruefung, z.B. den Platzbedarf vor einem Upload) statt sie
    // hier nochmal abzufragen. Liefert reinen Inline-Text OHNE umschliessendes
    // Element - der Aufrufer entscheidet je nach Seitenlayout, ob er ihn in
    // <p>...</p> (Dateiverwaltungsseiten) oder <li>...</li> (Status-Listen)
    // einbettet. Eingefuehrt, nachdem die alte Statusbox mit dieser Info
    // (siehe generateTopBar()) komplett entfernt wurde und die Info seither
    // auf mehreren Seiten fehlte.
    //
    // forceEnglish: die Status-Seite/der Status-Tab (siehe panel-status) ist
    // - wie das Logging (siehe globaler esp32-arduino-web-ui-statusbar Skill,
    // Abschnitt "Log output language") - grundsaetzlich IMMER auf Englisch,
    // unabhaengig von der UI-Sprache: eine technische Diagnoseansicht, keine
    // lokalisierte Nutzeroberflaeche. Alle anderen Aufrufer dieser Funktion
    // (Dateiverwaltungsseiten wie /listfilesFaces, /handsets) bleiben normal
    // uebersetzt - deshalb ein Parameter statt die Funktion komplett
    // umzustellen, der Default (false = uebersetzt) aendert das Verhalten
    // fuer alle bestehenden Aufrufer nicht.

    // Short, translated line with LittleFS storage usage - "used"/"total" are
    // passed in by the caller (many pages already compute them anyway for
    // their own check, e.g. how much room an upload needs) instead of
    // querying them again here. Returns plain inline text with NO wrapping
    // element - the caller decides, depending on that page's layout, whether
    // to embed it in <p>...</p> (file-management pages) or <li>...</li>
    // (status lists). Introduced after the old status box with this info
    // (see generateTopBar()) was removed entirely and the info was then
    // missing from several pages.
    //
    // forceEnglish: the status page/tab (see panel-status) is - like logging
    // (see the global esp32-arduino-web-ui-statusbar skill, "Log output
    // language" section) - always in English, regardless of the UI
    // language: a technical diagnostic view, not a localized user interface.
    // Every other caller of this function (file-management pages like
    // /listfilesFaces, /handsets) stays normally translated - hence a
    // parameter instead of converting the whole function, with a default
    // (false = translated) that doesn't change behavior for any existing
    // caller.

    String generateStorageInfo(size_t used, size_t total, bool forceEnglish) {
        String usedLabel = forceEnglish ? "Storage used" : translate("Storage used");
        String freeLabel = forceEnglish ? "Free" : translate("Free");
        String html = usedLabel + ": " + String(used / 1024) + " KB / " + String(total / 1024) + " KB";
        html += " (" + freeLabel + ": " + String((total - used) / 1024) + " KB)";
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
            // "/reboot" bewusst NICHT mehr gelistet - der Neustart-Knopf in
            // der Topbar (siehe generateTopBar(), reset-btn) deckt das jetzt
            // ab; die Route "/reboot" selbst bleibt bestehen (wird von
            // diesem Knopf sowie fuer Lesezeichen/direkte Aufrufe weiter
            // benoetigt).
            // "/reboot" deliberately no longer listed - the restart button
            // in the topbar (see generateTopBar(), reset-btn) covers this
            // now; the "/reboot" route itself stays (still needed by that
            // button, and for bookmarks/direct calls).
            // "DCF77" bewusst unuebersetzt (Protokollname, siehe DCF77-Label
            // in generateTopBar()/dotStatusText() weiter unten - dort ebenfalls
            // unuebersetzt). Bewusst vor "Factory Reset".
            // "DCF77" deliberately left untranslated (a protocol name, see the
            // DCF77 label in generateTopBar()/dotStatusText() further below -
            // also untranslated there). Deliberately placed before "Factory Reset".
            {"/dcf77", "DCF77", ""},
            {"/factoryReset", translate("Factory&nbsp;Reset"), ""}
        };

        String currentPath = webserver.uri(); // Aktueller Pfad der Seite
                                              // current path of the page

        for (const auto& item : navItems) {
            // "/dcf77" nur anzeigen, wenn ueberhaupt ein DCF77-Empfaenger
            // konfiguriert ist UND dieser bereits eine plausible Impulskette
            // geliefert hat (dcf77Confirmed, siehe globals.h) - also
            // dieselbe "gefunden"-Bedingung wie beim DCF77-Statuspunkt in
            // der Topbar (siehe getDcf77Status()/dcfPresent in
            // generateTopBar() oben). Anders als dort gibt es hier kein
            // nachtraegliches Live-Einblenden per JS, da generateNavigation()
            // nur einmal pro Seitenaufruf gerendert wird - der Link
            // erscheint also je nach Zeitpunkt des Aufrufs erst ab dem
            // naechsten Seitenwechsel/-neuladen nach dem ersten Impuls.
            // Direktaufrufe von /dcf77 per Lesezeichen/URL funktionieren
            // unabhaengig davon weiterhin (die Route selbst prueft das
            // nicht), zeigen dann aber noch keine Daten an.

            // Only show "/dcf77" when a DCF77 receiver is configured at all
            // AND it has already delivered a plausible pulse chain
            // (dcf77Confirmed, see globals.h) - i.e. the same "found"
            // condition as the DCF77 status dot in the topbar (see
            // getDcf77Status()/dcfPresent in generateTopBar() above). Unlike
            // there, there's no later live reveal via JS here, since
            // generateNavigation() is only rendered once per page load - so
            // depending on timing the link only appears from the next
            // page change/reload after the first pulse. Direct calls to
            // /dcf77 via bookmark/URL keep working regardless (the route
            // itself doesn't check this), just without any data yet.
            if (item.path == "/dcf77") {
#if defined DCF77_DATAPIN && defined DCF77_INTERRUPT
                if (!dcf77Confirmed) continue;
#else
                continue;
#endif
            }

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
        // escapeHtmlText(): der "msg"-Parameter kommt aus der URL (z.B. per
        // Redirect-Link) und wird nach translate() (gibt bei unbekanntem
        // Schluessel den Text unveraendert zurueck) direkt als HTML
        // eingebettet - ohne Escaping ein reflektiertes-XSS-Einfallstor auf
        // praktisch jeder Seite, die generateFlashMessage() einbindet.
        // escapeHtmlText(): the "msg" parameter comes from the URL (e.g. via
        // a redirect link) and, after translate() (returns the text unchanged
        // on an unknown key), is embedded directly as HTML - without escaping
        // a reflected-XSS entry point on practically every page that includes
        // generateFlashMessage().
        String message = escapeHtmlText(translate(webserver.arg("msg")));
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


    // Erzeugt den fuer fast jede Seite gleichen Seitenanfang (Header inkl.
    // Topbar + Navigation) - Reihenfolge entspricht der bisherigen,
    // wiederholten Aufrufkette. generateHtmlStatus() (Statusbox oben rechts)
    // entfaellt seit deren vollstaendiger Entfernung, siehe generateTopBar().

    // Generates the page start common to almost every page (header incl.
    // topbar + navigation) - order matches the previous, repeated call
    // chain. generateHtmlStatus() (status box top-right) is gone since it
    // was removed entirely, see generateTopBar().

    String beginPage() {
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

                    // presets[i].name kommt vom Nutzer (freier Name beim Anlegen
                    // eines Presets, siehe /api/createPreset) und wurde hier an
                    // mehreren Stellen ungeescaped in HTML-Text bzw. in
                    // JS-String-Literale innerhalb von onclick-Attributen
                    // eingebettet - potentiell gespeichertes XSS. Fuer reinen
                    // Textinhalt escapeHtmlText(), fuer die onclick-JS-Strings
                    // escapeForJsStringInAttr() (siehe Kommentar dort).
                    // presets[i].name comes from the user (a free-form name
                    // chosen when creating a preset, see /api/createPreset) and
                    // was embedded here unescaped in several places, both as
                    // HTML text and inside JS string literals within onclick
                    // attributes - potentially stored XSS. escapeHtmlText() for
                    // plain text content, escapeForJsStringInAttr() for the
                    // onclick JS strings (see comment there).
                    String safePresetNameText = escapeHtmlText(presets[i].name);
                    chunk += "<div style='text-align:center;border:1px solid #ccc;border-radius:6px;padding:8px;width:220px;'>";
                    chunk += "<a href='" + displayUrl + "'><img src='/presetpreview?index=" + String(i) + "' style='width:90px;height:90px;'></a>";
                    chunk += "<br><a href='" + displayUrl + "'>" + safePresetNameText + "</a>";
                    String presetName = presets[i].name;
                    presetName.replace(" ", "_"); // Ersetze Leerzeichen durch Unterstriche
                                                  // replace spaces with underscores
                    String ipLink = "http://" + ipAddress + "/api/setPreset?name=" + presetName;
                    // onclick=\"...\" -> aeusseres Attribut ist doppelt gequotet,
                    // der JS-String innen einfach ('...') - jsStringQuote='\''.
                    // onclick=\"...\" -> the outer attribute is double-quoted,
                    // the inner JS string is single-quoted ('...') - jsStringQuote='\''.
                    chunk += "<br><span onclick=\"copyPresetLink('" + escapeForJsStringInAttr(ipLink, '\'') + "', this)\" style='cursor:pointer;font-size:1.3em;' title='" + translate("Copy link") + "'>&#128203;</span>";
                    if (pingHostname) {
                        String hostLink = espHost + "/api/setPreset?name=" + presetName;
                        chunk += " <span onclick=\"copyPresetLink('" + escapeForJsStringInAttr(hostLink, '\'') + "', this)\" style='cursor:pointer;font-size:1.3em;' title='" + translate("Copy link") + " (" + espHost + ")'>&#128203;</span>";
                    }
                    chunk += "<br><a href='/renamepreset_form?index=" + String(i) + "'>" + translate("Rename") + "</a> ";
                    // onclick='...' -> aeusseres Attribut ist einfach gequotet,
                    // der confirm()-String innen doppelt (\"...\") -
                    // jsStringQuote='"'.
                    // onclick='...' -> the outer attribute is single-quoted,
                    // the confirm() string inside is double-quoted (\"...\") -
                    // jsStringQuote='"'.
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
            // escapeForJsStringLiteral(): Preset-Namen (Nutzereingabe) und Datei-
            // namen (aus Uploads, s. handleFileUpload() - dort nicht auf harmlose
            // Zeichen eingeschraenkt) landen hier als JS-String-Literal direkt in
            // einem <script>-Block. Ohne Escaping koennte ein "-Zeichen den String
            // vorzeitig beenden bzw. eine "</script>"-Sequenz das ganze Element
            // abschliessen und beliebiges HTML/JS einschleusen (gespeicherte XSS).
            //
            // escapeForJsStringLiteral(): preset names (user input) and filenames
            // (from uploads, see handleFileUpload() - not restricted to harmless
            // characters there) end up here as a JS string literal directly inside
            // a <script> block. Without escaping, a '"' character could end the
            // string early, or a "</script>" sequence could close the whole
            // element and inject arbitrary HTML/JS (stored XSS).
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

            // escapeHtmlText(): "file" kommt aus der URL (GET-Parameter) und
            // wurde vorher ungeescaped in zwei value='...'-Attribute
            // eingebettet - ein praeparierter Link (z.B. .../rename_form?file=x'%3E%3Cscript%3E...)
            // haette so reflektiertes XSS ausgeloest.
            // escapeHtmlText(): "file" comes from the URL (GET parameter) and
            // was previously embedded unescaped into two value='...'
            // attributes - a crafted link (e.g. .../rename_form?file=x'%3E%3Cscript%3E...)
            // would have triggered reflected XSS this way.
            String oldName = escapeHtmlText(webserver.arg("file"));
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

                // Neuer Dateiname wurde bisher ungeprueft direkt uebernommen.
                // Dateinamen werden spaeter an mehreren Stellen (/files,
                // /listfilesFaces, /handsets) ungeescaped in HTML-Attribute/
                // onclick-Strings eingebettet - ein Name mit "'" oder "<"
                // haette dort gespeichertes XSS ermoeglicht. Auf ein sicheres
                // Zeichen-Set beschraenken (Buchstaben/Ziffern/_/-/./sowie den
                // einen fuehrenden "/").
                // The new filename was previously accepted unchecked. Filenames
                // later get embedded unescaped into HTML attributes/onclick
                // strings in several places (/files, /listfilesFaces,
                // /handsets) - a name containing "'" or "<" would have enabled
                // stored XSS there. Restrict to a safe character set (letters/
                // digits/_/-/. plus the one leading "/").
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
                    // Kollision mit einer bestehenden Datei ablehnen, statt sie
                    // stillschweigend zu ueberschreiben (LittleFS.rename() tut
                    // das sonst kommentarlos).
                    // Reject a collision with an existing file instead of
                    // silently overwriting it (LittleFS.rename() otherwise
                    // does so without any warning).
                    if (newName != oldName && LittleFS.exists(newName)) {
                        webserver.send(409, "text/plain", "A file with the new name already exists");
                        return;
                    }
                    if (LittleFS.rename(oldName, newName)) {
                        // Ist die umbenannte Datei gerade das aktive
                        // Zifferblatt, die Preference mitziehen - sonst zeigt
                        // sie danach auf eine nicht mehr existierende Datei.
                        // If the renamed file is the currently active clock
                        // face, update the preference along with it -
                        // otherwise it would point at a file that no longer
                        // exists.
                        if (preferences.getString(PK_BACKGROUND, "") == oldName) {
                            preferences.putString(PK_BACKGROUND, newName);
                            selectedBackground = newName;
                            freeClockFaceBuffer();
                            loadClockFace();
                        }

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

            // DCF77-Sync-LED-Blinken speichern (siehe dcfSyncLedEnabled in
            // globals.h, ausgewertet in uhr3.ino loop()) - nur auf Builds mit
            // DCF77-Empfaenger, sonst wuerde jedes Speichern dieses Formulars
            // die Einstellung auf Builds ohne DCF77-Hardware unbemerkt auf
            // "aus" ueberschreiben (die Checkbox wird dort ja gar nicht
            // gerendert, haette also nie im POST gestanden).
            // Save DCF77 sync LED blink setting (see dcfSyncLedEnabled in
            // globals.h, evaluated in uhr3.ino loop()) - only on builds with
            // a DCF77 receiver, otherwise every save of this form would
            // silently overwrite the setting to "off" on builds without
            // DCF77 hardware (the checkbox isn't rendered there at all, so
            // it would never have been present in the POST).
#if defined(DCF77_DATAPIN) && defined(DCF77_INTERRUPT)
            dcfSyncLedEnabled = webserver.hasArg("dcfSyncLed");
            preferences.putBool(PK_DCF_SYNC_LED, dcfSyncLedEnabled);
#endif

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
            // "returnTo" wird normalerweise nur ueber ein verstecktes
            // Formularfeld mit festem Wert mitgeschickt, kommt aber technisch
            // trotzdem als gewoehnlicher POST-Parameter an - ohne Pruefung
            // haette ein praeparierter POST (z.B. per CSRF-artigem externen
            // Formular) mit z.B. returnTo=http://evil.example.com einen
            // offenen Redirect auf eine beliebige externe Seite ausgeloest.
            // Nur einen lokalen, mit "/" beginnenden Pfad ohne "://"
            // akzeptieren, sonst auf den Default zurueckfallen.
            // "returnTo" is normally only ever submitted via a hidden form
            // field with a fixed value, but technically still arrives as an
            // ordinary POST parameter - without validation, a crafted POST
            // (e.g. via a CSRF-like external form) with e.g.
            // returnTo=http://evil.example.com would have triggered an open
            // redirect to an arbitrary external site. Only accept a local
            // path starting with "/" and containing no "://", otherwise fall
            // back to the default.
            String returnTo = webserver.hasArg("returnTo") ? webserver.arg("returnTo") : "/?tab=helligkeit";
            if (!returnTo.startsWith("/") || returnTo.indexOf("://") >= 0) {
                returnTo = "/?tab=helligkeit";
            }
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

                    // Beide Pruefungen fehlten hier bisher: (1) open() kann
                    // trotz vorherigem exists()-Check fehlschlagen, wenn die
                    // Datei zwischen beiden Aufrufen von einem parallelen
                    // Request (z.B. /delete, /rename) geloescht/umbenannt
                    // wurde - file.name() auf einem ungueltigen File-Objekt
                    // waere dann undefiniert. (2) LittleFS.exists() liefert
                    // fuer Verzeichnisse ebenfalls true; ohne isDirectory()-
                    // Check haette ein Verzeichnisname als "file" eine
                    // sinnlose "Download"-Antwort erzeugt statt eines
                    // sauberen Fehlercodes (an anderer Stelle in dieser Datei,
                    // z.B. /listfilesFaces, wird isDirectory() bereits korrekt
                    // geprueft).
                    // Both checks were previously missing here: (1) open()
                    // can fail despite the earlier exists() check if the file
                    // was deleted/renamed by a concurrent request (e.g.
                    // /delete, /rename) between the two calls - file.name() on
                    // an invalid File object would then be undefined. (2)
                    // LittleFS.exists() also returns true for directories;
                    // without an isDirectory() check, a directory name passed
                    // as "file" would have produced a meaningless "download"
                    // response instead of a clean error code (isDirectory()
                    // is already correctly checked elsewhere in this file,
                    // e.g. /listfilesFaces).
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
        // Liefert ein winziges, inline erzeugtes SVG-Uhr-Icon (im Marken-Akzent-
        // farbton, siehe --accent in generateHtmlHeader()) als Favicon - vorher
        // gab es hier keine Route, jeder Browser fragte also bei jedem Seiten-
        // aufruf zusaetzlich "/favicon.ico" an, was unbeantwortet in einer
        // eigenen 404-Seite landete (siehe webserver.onNotFound()). Moderne
        // Browser akzeptieren SVG unter diesem Pfad direkt, ein eingebettetes
        // Asset auf LittleFS ist dafuer nicht noetig. Lange Cache-Zeit, da sich
        // das Icon zur Laufzeit nie aendert.
        // Returns a tiny, inline-generated SVG clock icon (in the brand accent
        // color, see --accent in generateHtmlHeader()) as the favicon -
        // previously there was no route here, so every browser additionally
        // requested "/favicon.ico" on every page load, which went unanswered
        // into its own 404 page (see webserver.onNotFound()). Modern browsers
        // accept SVG directly at this path, no embedded asset on LittleFS is
        // needed for it. Long cache lifetime, since the icon never changes at
        // runtime.
        webserver.on("/favicon.ico", HTTP_GET, []() {
            webserver.sendHeader("Cache-Control", "public, max-age=86400");
            String svg = "<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 32 32'>"
                         "<circle cx='16' cy='16' r='14' fill='#10151a' stroke='#f5a623' stroke-width='2.5'/>"
                         "<line x1='16' y1='16' x2='16' y2='7' stroke='#f5a623' stroke-width='2.5' stroke-linecap='round'/>"
                         "<line x1='16' y1='16' x2='21' y2='16' stroke='#f5a623' stroke-width='2.5' stroke-linecap='round'/>"
                         "</svg>";
            webserver.send(200, "image/svg+xml", svg);
            });

        webserver.on("/api/currentTime", HTTP_GET, []() {
            webserver.sendHeader("Cache-Control", "no-store");
            String json = "{\"hour\":" + String(timeinfo.tm_hour) +
                          ",\"minute\":" + String(timeinfo.tm_min) +
                          ",\"second\":" + String(timeinfo.tm_sec) + "}";
            webserver.send(200, "application/json", json);
            });

        // Liefert den aktuellen Zustand der drei Topbar-Status-Punkte (Zeit/
        // RTC/DCF77) als JSON - wird vom Live-Status-Skript in
        // generateTopBar() periodisch abgefragt, damit die Punktfarbe auch
        // waehrend des Betriebs wechselt, wenn etwas ausfaellt oder nicht
        // mehr erreichbar ist (z.B. RTC/DCF77 verliert Sync), OHNE die ganze
        // Seite neu zu laden. Dieselben Bedingungen wie in generateTopBar() -
        // bei einer Aenderung dort auch hier anpassen. rtc/dcf77 werden immer
        // mitgeschickt (auch ohne die jeweilige Hardware) - das Skript
        // ignoriert sie einfach, wenn der zugehoerige Punkt gar nicht im DOM
        // existiert (kein #if noetig, vereinfacht den Handler).

        // Returns the current state of the topbar (the three status dots
        // time/RTC/DCF77, plus the current date/time text) as JSON - polled
        // periodically by the live-status script in generateTopBar() so both
        // the dot colors AND the displayed time keep updating during
        // operation, WITHOUT reloading the whole page. Same conditions/
        // formatting as in generateTopBar() - keep both in sync if one
        // changes. rtc/dcf77 are always included (even without that
        // hardware) - the script simply ignores them if the corresponding
        // dot doesn't exist in the DOM at all (no #if needed, keeps this
        // handler simple).
        // time/rtc/dcf77 sind jetzt ALLE drei ein STRING
        // ("ok"/"syncing"/"bad"/"na", siehe dotStatusText()-Kommentar bei
        // getTimeStatus()/getRtcStatus()/getDcf77Status() oben) statt einem
        // Bool - "na" (grau) zeigt "Feature grundsaetzlich nicht verfuegbar"
        // (z.B. RTC nie gefunden), getrennt von "bad" (rot, echter Fehler).
        // Nutzt dieselben Helper wie generateTopBar()'s Erstladen, damit
        // beide nie auseinanderlaufen - siehe dortigen Kommentar. *Title-
        // Felder: uebersetzter Tooltip-/aria-label-Text (siehe dotStatusText()
        // oben), vom Live-Status-Skript in generateTopBar() bei jedem Poll
        // auf die Punkte uebertragen, damit Text und Punktfarbe nie
        // auseinanderlaufen.
        // time/rtc/dcf77 are now ALL THREE a STRING
        // ("ok"/"syncing"/"bad"/"na", see the dotStatusText() comment at
        // getTimeStatus()/getRtcStatus()/getDcf77Status() above) rather than
        // a bool - "na" (gray) means "feature fundamentally unavailable"
        // (e.g. RTC never found), distinct from "bad" (red, genuine error).
        // Uses the exact same helpers as generateTopBar()'s initial render,
        // so the two can never drift apart - see the comment there. *Title
        // fields: translated tooltip/aria-label text (see dotStatusText()
        // above), applied to the dots by the live-status script in
        // generateTopBar() on every poll, so the text and the dot color never
        // drift apart.
        // rtcPresent: siehe rtcPresent-Kommentar in generateTopBar() - false
        // nur solange rtcOk == RTC_NOT_AVAILABLE ist. Toggelt live das
        // 'hidden'-Attribut auf dem #status-rtc-Wrapper via setPresent() im
        // Live-Status-Skript.
        // dcf77Present: siehe dcf77Confirmed-Kommentar in generateTopBar() -
        // true erst, sobald eine PLAUSIBLE KETTE aus mehreren aufeinander-
        // folgenden Impulsen beobachtet wurde (siehe DCF77_PRESENCE_MIN_STREAK
        // in config.h - ein einzelner, z.B. durch Rauschen ausgeloester
        // Impuls reicht NICHT), danach dauerhaft true. Toggelt live das
        // 'hidden'-Attribut auf dem #status-dcf77-Wrapper via setPresent() im
        // Live-Status-Skript.
        // lightValue: fertig formatierter Helligkeitswert des Fotowiderstands
        // (Prozent, siehe currentLightPercent in display.h) - analog zu
        // datetimeStr bereits serverseitig fertig formatiert, damit das
        // Frontend nichts selbst formatieren muss. Wird immer mitgeschickt
        // (auch ohne Fotowiderstand) - #value-light existiert dann einfach
        // nicht im DOM, siehe setValue() im Live-Status-Skript.
        // rtcPresent: see the rtcPresent comment in generateTopBar() - false
        // only for as long as rtcOk == RTC_NOT_AVAILABLE. Live-toggles the
        // 'hidden' attribute on the #status-rtc wrapper via setPresent() in
        // the live-status script.
        // dcf77Present: see the dcf77Confirmed comment in generateTopBar() -
        // true only once a PLAUSIBLE CHAIN of several consecutive pulses has
        // been observed (see DCF77_PRESENCE_MIN_STREAK in config.h - a
        // single pulse, e.g. triggered by noise, is NOT enough), permanently
        // true afterwards. Live-toggles the 'hidden' attribute on the
        // #status-dcf77 wrapper via setPresent() in the live-status script.
        // lightValue: the photoresistor's brightness reading, already
        // formatted (percent, see currentLightPercent in display.h) - like
        // datetimeStr, formatted server-side so the frontend never has to
        // format anything itself. Always included (even without a
        // photoresistor) - #value-light simply doesn't exist in the DOM
        // then, see setValue() in the live-status script.
        webserver.on("/api/topbarStatus", HTTP_GET, []() {
            webserver.sendHeader("Cache-Control", "no-store");
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
            String json = "{\"time\":\"" + timeState + "\"" +
                          ",\"rtc\":\"" + rtcState + "\"" +
                          ",\"rtcPresent\":" + String(rtcPresent ? "true" : "false") +
                          ",\"dcf77\":\"" + dcfState + "\"" +
                          ",\"dcf77Present\":" + String(dcf77Present ? "true" : "false") +
                          ",\"lightValue\":\"" + lightValue + "\"" +
                          ",\"datetime\":\"" + datetimeStr + "\"" +
                          ",\"timeTitle\":\"" + timeTitle + "\"" +
                          ",\"rtcTitle\":\"" + rtcTitle + "\"" +
                          ",\"dcf77Title\":\"" + dcf77Title + "\"}";
            webserver.send(200, "application/json", json);
            });

        // Liefert den Live-Fortschritt des gerade empfangenen DCF77-Telegramms
        // (Bit fuer Bit, siehe processDcf77Bits() in time_sync.h) sowie das
        // zuletzt vollstaendig dekodierte Telegramm (siehe
        // decodeDcf77Telegram()) als JSON - fuer das sekuendliche Polling der
        // /dcf77-Seite. "bits" ist ein String der Laenge DCF77_TELEGRAM_BITS
        // mit '0'/'1' fuer bereits empfangene Bits und '?' fuer noch nicht
        // empfangene Bits der laufenden Minute.

        // Returns the live progress of the DCF77 telegram currently being
        // received (bit by bit, see processDcf77Bits() in time_sync.h) as
        // well as the last fully decoded telegram (see
        // decodeDcf77Telegram()) as JSON - for the /dcf77 page's
        // once-per-second polling. "bits" is a string of length
        // DCF77_TELEGRAM_BITS with '0'/'1' for bits already received and '?'
        // for bits of the running minute not yet received.
        webserver.on("/api/dcf77status", HTTP_GET, []() {
            webserver.sendHeader("Cache-Control", "no-store");
#if !defined(DCF77_DATAPIN) || !defined(DCF77_INTERRUPT)
            webserver.send(200, "application/json", "{\"present\":false}");
#else
            String bits;
            bits.reserve(DCF77_TELEGRAM_BITS);
            for (uint8_t i = 0; i < DCF77_TELEGRAM_BITS; i++) {
                bits += (dcf77Bits[i] < 0) ? '?' : (char)('0' + dcf77Bits[i]);
            }

            String json = "{\"present\":true";
            json += ",\"state\":\"" + getDcf77Status() + "\"";
            json += ",\"bitIndex\":" + String(dcf77BitIndex);
            json += ",\"bits\":\"" + bits + "\"";
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
                json += ",\"ageSeconds\":" + String(ageMs / 1000);
            }
            json += "}}";
            webserver.send(200, "application/json", json);
#endif
            });

        // Liefert den Inhalt der AKTUELL aktiven Logdatei (siehe
        // getCurrentLogFileName() in system_utils.h) als Klartext - fuer das
        // Auto-Refresh-Polling im Log-Tab (siehe panel-log oben). Loest die
        // Dateinummer bei JEDEM Aufruf frisch auf, damit nach einer Rotation
        // (Datei > 10 KB, siehe logToFile()) automatisch die neue, aktuelle
        // Datei geliefert wird - nicht die beim Seitenaufruf zufaellig aktive.

        // Returns the content of the CURRENTLY active log file (see
        // getCurrentLogFileName() in system_utils.h) as plain text - for the
        // auto-refresh polling in the Log tab (see panel-log above). Resolves
        // the file number freshly on EVERY call, so that after a rotation
        // (file > 10 KB, see logToFile()) the new, current file is served
        // automatically - not whichever one happened to be active when the
        // page was first loaded.
        webserver.on("/api/currentLog", HTTP_GET, []() {
            webserver.sendHeader("Cache-Control", "no-store");
            if (!loggingEnabled) {
                webserver.sendHeader("X-Log-File", "-");
                webserver.send(200, "text/plain; charset=utf-8", translate("Logging is disabled."));
                return;
            }
            String logFileName = getCurrentLogFileName();
            // X-Log-File: Custom-Header, damit das JS im Log-Tab die Anzeige
            // #logFileName bei jedem Poll aktualisieren kann - auch wenn sich
            // die aktive Datei durch Rotation seit dem letzten Aufruf
            // geaendert hat (siehe panel-log im "/" Handler).
            // Custom header so the JS in the Log tab can update the
            // #logFileName display on every poll - even if the active file
            // has changed due to rotation since the last call (see panel-log
            // in the "/" handler).
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
            // Default false, wie ueberall sonst im Projekt (uhr3.ino,
            // display.h, presets_manager.h, sonstige Stellen in dieser Datei)
            // - hier stand abweichend "true", wodurch diese Anzeige direkt
            // nach einem Werkreset einen anderen Zustand behauptete als der
            // tatsaechlich angewendete.
            // Default false, matching everywhere else in the project
            // (uhr3.ino, display.h, presets_manager.h, other spots in this
            // file) - this used to say "true" here, so right after a factory
            // reset this display claimed a different state than what was
            // actually applied.
            bool smoothMinuteActive = preferences.getBool(PK_SMOOTH_MINUTE, false);

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

        // --- DCF77-Live-Seite: Bit-Fortschritt der laufenden Minute
        // (Sekunde 0-58) plus das zuletzt vollstaendig dekodierte Telegramm,
        // sekuendlich per Poll aktualisiert (siehe /api/dcf77status weiter
        // unten). Zeigt live genau die Dekodierung, die inzwischen auch die
        // tatsaechliche Zeituebernahme speist (siehe
        // applyDcf77DecodedTime()/updateDcf77Status() in time_sync.h) - die
        // Original-Bibliothek (dcf.getUTCTime()) wird dafuer nicht mehr
        // benutzt (siehe dortiger Kommentar zur Umstellung).
        //
        // Die Bit-Tooltips sowie die Legende sind bewusst Englisch
        // (Diagnose-/Debuginhalt, wie beim Status-Tab - siehe
        // generateStorageInfo()/forceEnglish weiter unten und die dortige
        // Konvention), waehrend die Feldbezeichnungen der Tabelle (Minute,
        // Stunde, ...) wie der Rest der Seite uebersetzt sind.
        //
        // Ueber das Untermenue verlinkt (navItems in generateNavigation()
        // weiter oben, dort nur sichtbar wenn dcf77Confirmed) - kein Tab auf
        // "/", eine eigenstaendige Seite.

        // --- DCF77 live page: bit progress of the running minute (second
        // 0-58) plus the last fully decoded telegram, polled every second
        // (see /api/dcf77status further below). Shows live exactly the
        // decode that, by now, also feeds the actual time takeover (see
        // applyDcf77DecodedTime()/updateDcf77Status() in time_sync.h) - the
        // original library (dcf.getUTCTime()) is no longer used for that
        // (see the comment there about the switch).
        //
        // The per-bit tooltips and the legend are deliberately English
        // (diagnostic/debug content, like the Status tab - see
        // generateStorageInfo()/forceEnglish further below and the
        // convention there), while the table's field labels (Minute, Hour,
        // ...) are translated like the rest of the page.
        //
        // Linked from the submenu (navItems in generateNavigation() further
        // above, shown there only when dcf77Confirmed) - not a tab on "/", a
        // standalone page.
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
            chunk += "<div id='dcfBitGrid' style='display:flex;flex-wrap:wrap;gap:3px;justify-content:center;font-family:var(--mono);font-size:.7rem;'>";

            for (int i = 0; i < DCF77_TELEGRAM_BITS; i++) {
                String desc;
                if (i == 0) desc = "Start of minute (always 0)";
                else if (i >= 1 && i <= 14) desc = "Weather broadcast / special function (unused)";
                else if (i == 15) desc = "Call bit";
                else if (i == 16) desc = "DST change announcement";
                else if (i == 17) desc = "Summer time (CEST) in effect";
                else if (i == 18) desc = "Winter time (CET) in effect";
                else if (i == 19) desc = "Leap second announcement";
                else if (i == 20) desc = "Start of time (always 1)";
                else if (i >= 21 && i <= 27) desc = "Minute BCD bit " + String(i - 20);
                else if (i == 28) desc = "Minute parity";
                else if (i >= 29 && i <= 34) desc = "Hour BCD bit " + String(i - 28);
                else if (i == 35) desc = "Hour parity";
                else if (i >= 36 && i <= 41) desc = "Day of month BCD bit " + String(i - 35);
                else if (i >= 42 && i <= 44) desc = "Day of week BCD bit " + String(i - 41);
                else if (i >= 45 && i <= 49) desc = "Month BCD bit " + String(i - 44);
                else if (i >= 50 && i <= 57) desc = "Year BCD bit " + String(i - 49);
                else desc = "Date parity (day+weekday+month+year)";
                chunk += "<div id='bit-" + String(i) + "' title='Bit " + String(i) + ": " + desc +
                         "' style='width:1.7rem;height:1.7rem;line-height:1.7rem;display:inline-block;text-align:center;border-radius:.25rem;border:1px solid var(--panel-border);color:var(--muted);'>" +
                         String(i) + "</div>";
            }
            chunk += "</div>";
            chunk += "<p style='color:var(--muted);font-size:.75rem;'>"
                     "<span style='display:inline-block;width:.8rem;height:.8rem;background:var(--accent);border-radius:.2rem;vertical-align:middle;'></span> = 1 &nbsp; "
                     "<span style='display:inline-block;width:.8rem;height:.8rem;border:1px solid var(--panel-border);border-radius:.2rem;vertical-align:middle;'></span> = 0 &nbsp; "
                     "<i class='dot syncing' style='display:inline-block;position:static;'></i> = next</p>";
            // Diagnosewert: Anzahl der wegen vollem Ringpuffer verworfenen
            // Flanken seit dem letzten Neustart (siehe dcf77EdgeDropped in
            // globals.h, isr() in time_sync.h) - hilft zu unterscheiden, ob
            // Aussetzer/uebersprungene Bits an einem ueberlaufenden Puffer
            // liegen (dieser Wert steigt) oder an tatsaechlich schwachem
            // Empfang bzw. Stoerungen (Wert bleibt bei 0). Bewusst englisch,
            // wie die Bit-Tooltips oben (Diagnoseinhalt).
            // Diagnostic value: number of edges dropped due to a full ring
            // buffer since the last restart (see dcf77EdgeDropped in
            // globals.h, isr() in time_sync.h) - helps tell whether
            // dropouts/skipped bits are caused by a buffer overflow (this
            // value rises) or by genuinely weak reception/interference
            // (value stays at 0). Deliberately English, like the bit
            // tooltips above (diagnostic content).
            chunk += "<p style='color:var(--muted);font-size:.7rem;'>Dropped edges (buffer overflow): <span id='dcfEdgeDropped'>-</span></p>";
            chunk += "</div>";

            // Dekodiertes Telegramm: Server rendert die uebersetzten
            // Feldbezeichnungen sowie leere Platzhalter-<span>s mit stabilen
            // IDs; das Poll-Skript unten befuellt NUR die reinen Datenwerte
            // (Zahlen, ein Umschalten von "hidden" zwischen zwei bereits
            // uebersetzten <span>s fuer Sommer-/Winterzeit) - so muss kein
            // uebersetzter Text aus translate() in einen JS-String-Literal
            // eingebettet werden (Gefahr durch Anfuehrungszeichen in einer
            // Uebersetzung, siehe z.B. franzoesische Apostrophe im Rest
            // dieser Datei).

            // Decoded telegram: the server renders the translated field
            // labels plus empty placeholder <span>s with stable IDs; the
            // poll script below only fills in the raw data values (numbers,
            // toggling "hidden" between two already-translated <span>s for
            // summer/winter time) - this way no translate() text ever has
            // to be embedded inside a JS string literal (risk from quote
            // characters inside a translation, see e.g. the French
            // apostrophes elsewhere in this file).
            chunk += "<div class='card' id='dcfDecodedCard'>";
            chunk += "<h3>" + translate("Decoded telegram") + "</h3>";
            chunk += "<div id='dcfWaitingMsg'>" + translate("Waiting for first complete telegram") + "</div>";
            chunk += "<table id='dcfDecodedTable' hidden>";
            chunk += "<tr><td>" + translate("Day") + "/" + translate("Month") + "/" + translate("Year") + "</td><td><span id='dcfDate'>-</span></td></tr>";
            chunk += "<tr><td>" + translate("Hour") + "/" + translate("Minute") + "</td><td><span id='dcfTime'>-</span></td></tr>";
            chunk += "<tr><td>" + translate("Weekday") + "</td><td><span id='dcfWeekday'>-</span> <small>(DCF77: 1=Mon..7=Sun)</small></td></tr>";
            chunk += "<tr><td>" + translate("Summer time") + " / " + translate("Winter time") + "</td><td><span id='dcfDstOn' hidden>" + translate("Summer time") + "</span><span id='dcfDstOff' hidden>" + translate("Winter time") + "</span></td></tr>";
            chunk += "<tr><td>" + translate("Call bit") + "</td><td><span id='dcfCall'>-</span></td></tr>";
            chunk += "<tr><td>" + translate("Parity") + " (" + translate("Minute") + "/" + translate("Hour") + "/" + translate("Day") + ")</td><td><span id='dcfParityMin'>-</span> / <span id='dcfParityHour'>-</span> / <span id='dcfParityDate'>-</span></td></tr>";
            chunk += "<tr><td>" + translate("Last decoded") + "</td><td><span id='dcfAge'>-</span> s</td></tr>";
            chunk += "</table>";
            chunk += "</div>";

            chunk += "<script>";
            chunk += "(function(){";
            chunk += "var TOTAL=" + String(DCF77_TELEGRAM_BITS) + ";";
            chunk += "function pad(n){return (n<10?'0':'')+n;}";
            chunk += "function paintBits(bitIndex,bits){";
            chunk += "for(var i=0;i<TOTAL;i++){";
            chunk += "var el=document.getElementById('bit-'+i); if(!el) continue;";
            chunk += "el.classList.remove('syncing');";
            chunk += "if(i<bitIndex){";
            chunk += "if(bits.charAt(i)==='1'){el.style.background='var(--accent)';el.style.borderColor='var(--accent)';el.style.color='#1a1200';}";
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
            chunk += "}";
            chunk += "var timer=null;";
            chunk += "function poll(){fetch('/api/dcf77status',{cache:'no-store'}).then(function(r){return r.json();}).then(function(s){paintBits(s.bitIndex,s.bits);renderDecoded(s.decoded);var ed=document.getElementById('dcfEdgeDropped');if(ed)ed.textContent=s.edgeDropped;}).catch(function(){});}";
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
            chunk += "<p>" + generateStorageInfo(used, total) + "</p>";
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
                // escapeHtmlText() ergaenzt: "name"/"shortName" stammen aus
                // dem tatsaechlichen Dateinamen auf LittleFS - handleFileUpload()
                // prueft beim Hochladen nur Praefix ("face_") und Suffix
                // (".bmp"), der mittlere Teil ist frei waehlbar. Ausserdem
                // fehlten hier bisher die Anfuehrungszeichen um das href-
                // Attribut komplett (href=http://... statt href='http://...'),
                // wodurch ein Leerzeichen/Sonderzeichen im Dateinamen den Link
                // kaputt gemacht haette.
                // escapeHtmlText() added: "name"/"shortName" come from the
                // actual filename on LittleFS - handleFileUpload() only
                // checks the prefix ("face_") and suffix (".bmp") on upload,
                // the middle part is freely chosen. Also, the href attribute
                // was previously missing its quotes entirely (href=http://...
                // instead of href='http://...'), so a space/special character
                // in the filename would have broken the link.
                String safeName = escapeHtmlText(name);
                String safeShortName = escapeHtmlText(shortName);
                chunk += "<div style='text-align:center;width:100px;'>";
                chunk += "<a href='http://" + ipAddress + "/setbackground?file=" + safeShortName + "'>";
                chunk += "<img src='/facepreview?file=" + safeName + "' style='width:80px;height:80px;border:1px solid #ccc'>";
                chunk += "</a><br>" + escapeHtmlText(displayName) + String(isActive ? " (" + translate("active") + ")" : "");
                chunk += "<br><a href='/rename_form?file=" + safeName + "'>" + translate("Rename") + "</a> ";
                chunk += "<a href='/delete?file=" + safeName + "' onclick='return confirm(\"" + translate("Delete") + " " + escapeHtmlText(displayName) + "?\")'>" + translate("Delete") + "</a>";
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

            // Status-LED kurz aufblitzen lassen, waehrend die Hauptseite
            // aufgebaut/gestreamt wird (setLedOff() ganz am Ende dieses
            // Handlers, siehe dort) - sichtbares Lebenszeichen bei jedem
            // Aufruf von "/", analog zum bestehenden Muster bei Uploads
            // (setLedOn()/setLedOff() rahmen dort den gesamten Vorgang ein,
            // siehe z.B. weiter unten bei "/file"). Kein zusaetzliches
            // delay() noetig - der Seitenaufbau selbst (mehrere
            // sendContent()-Chunks) dauert lang genug, um sichtbar zu sein.
            // War schon einmal vorhanden, ist bei einer frueheren
            // Ueberarbeitung verlorengegangen.

            // Briefly flash the status LED while the main page is being
            // built/streamed (setLedOff() right at the end of this handler,
            // see there) - a visible sign of life on every "/" request,
            // mirroring the existing pattern used for uploads (setLedOn()/
            // setLedOff() bracket the whole operation there, see e.g.
            // "/file" further below). No extra delay() needed - building the
            // page itself (several sendContent() chunks) already takes long
            // enough to be visible. This used to be in place, but got lost
            // in an earlier rework.
            setLedOn();

            webserver.setContentLength(CONTENT_LENGTH_UNKNOWN);
            webserver.send(200, "text/html", "");

            String chunk = beginPage();
            chunk.reserve(2048);
            chunk += generateFlashMessage(); // Erfolgsmeldung, falls vorhanden
                                             // success message, if present

            chunk += generateLanguageSelector();

            bool apMode = (WiFi.getMode() != WIFI_STA);

            // --- CSS-only Tabs: die 6 radio-Inputs (siehe Tab-CSS in
            // generateHtmlHeader()) - muessen direkte Geschwister von .tabnav
            // und allen .panel-* Divs weiter unten sein. Reihenfolge der
            // Labels weiter unten bestimmt die sichtbare Tab-Reihenfolge -
            // Status bewusst ganz nach rechts (ans Ende), direkt gefolgt von
            // Log (Log-Tab soll immer direkt NACH Status stehen), die
            // anderen zuerst. Beim Aufruf wird immer der erste Tab (WLAN)
            // vorausgewaehlt, unabhaengig vom Verbindungsstatus - das deckt
            // automatisch auch das Captive-Portal-Popup ab (das auf "/"
            // verweist, siehe captivePortalRedirect oben).

            // --- CSS-only tabs: the 6 radio inputs (see tab CSS in
            // generateHtmlHeader()) - must be direct siblings of .tabnav and
            // all .panel-* divs further below. Order of the labels below
            // determines the visible tab order - Status is deliberately
            // moved to the far right, immediately followed by Log (the Log
            // tab must always sit right after Status), the others come
            // first. The first tab (WiFi) is always preselected on load,
            // regardless of connection status - this also automatically
            // covers the captive portal popup (which points to "/", see
            // captivePortalRedirect above).
            chunk += "<input type='radio' name='tabs' id='tab-wlan' class='tabctrl' checked>";
            chunk += "<input type='radio' name='tabs' id='tab-zifferblatt' class='tabctrl'>";
            chunk += "<input type='radio' name='tabs' id='tab-helligkeit' class='tabctrl'>";
            chunk += "<input type='radio' name='tabs' id='tab-zeit' class='tabctrl'>";
            chunk += "<input type='radio' name='tabs' id='tab-status' class='tabctrl'>";
            chunk += "<input type='radio' name='tabs' id='tab-log' class='tabctrl'>";

            chunk += "<div class='tabnav'>";
            chunk += "<label for='tab-wlan'>" + translate("WiFi Settings") + "</label>";
            chunk += "<label for='tab-zifferblatt'>" + translate("Clock Setup") + "</label>";
            chunk += "<label for='tab-helligkeit'>" + translate("Brightness") + "</label>";
            chunk += "<label for='tab-zeit'>" + translate("NTP&nbsp;Timezone") + "</label>";
            chunk += "<label for='tab-status'>" + translate("Status") + "</label>";
            chunk += "<label for='tab-log'>" + translate("Log") + "</label>";
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
            // In eine .card gepackt (mittelbreite, zentrierte Box - wie die
            // anderen Tabs), statt die Liste ueber die volle Seitenbreite
            // laufen zu lassen - .card setzt bereits text-align:left, der
            // Inhalt bleibt also linksbuendig, nur der umgebende Rahmen wird
            // schmaler und zentriert.
            // Wrapped in a .card (medium-width, centered box - like the
            // other tabs), instead of letting the list run the full page
            // width - .card already sets text-align:left, so the content
            // stays left-aligned, only the surrounding box becomes narrower
            // and centered.
            chunk += "<div class='tabpanel panel-status'>";
            chunk += "<div class='card'>";
            chunk += "<ul>";
            chunk += "<li>" + generateStorageInfo(LittleFS.usedBytes(), LittleFS.totalBytes(), true) + "</li>";

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
            chunk += "</div>"; // Ende .card
                               // end .card
            chunk += "</div>"; // Ende panel-status
                               // end panel-status

            webserver.sendContent(chunk);
            chunk = "";

            // #####################################################################
            // Panel: Log - zeigt immer den Inhalt der aktuell aktiven Logdatei
            // (siehe getCurrentLogFileName() in system_utils.h - loest die
            // Datei bei JEDEM Aufruf/Refresh frisch auf, damit eine Rotation
            // waehrend des Betrachtens automatisch beruecksichtigt wird).
            // Enthaelt eine per Checkbox abschaltbare 10-Sekunden-Auto-Refresh
            // (Zustand in localStorage gemerkt, Default: aus) sowie einen
            // manuellen "Jetzt aktualisieren"-Button, ueber den /api/currentLog
            // Endpunkt weiter unten. Der eigentliche Loginhalt wird bewusst
            // NICHT serverseitig mitgerendert (nur ein Platzhalter), sondern
            // per JS gleich beim Laden aus /api/currentLog nachgeladen - siehe
            // Kommentar bei "Lazy-Load" weiter unten fuer die Begruendung.
            // Bewusst NACH Status platziert (siehe Tab-Reihenfolge weiter
            // oben).
            // #####################################################################

            // #####################################################################
            // Panel: Log - always shows the content of the currently active
            // log file (see getCurrentLogFileName() in system_utils.h -
            // resolved fresh on EVERY call/refresh, so a rotation while the
            // tab is open is picked up automatically). Includes a togglable
            // 10-second auto-refresh (state remembered in localStorage,
            // default: off) as well as a manual "Refresh now" button, via the
            // /api/currentLog endpoint further below. The actual log content
            // is deliberately NOT rendered server-side (just a placeholder) -
            // instead it's lazy-loaded via JS right on page load from
            // /api/currentLog, see the "lazy load" comment further below for
            // the reasoning. Deliberately placed AFTER Status (see tab order
            // further above).
            // #####################################################################
            chunk += "<div class='tabpanel panel-log'>";

            // Aktueller Log-Dateiname wird fuer die Anzeige aufgeloest - das
            // ist NUR ein Preferences-Zugriff (siehe getCurrentLogFileName()
            // in system_utils.h), kein Datei-Lesevorgang, also unproblematisch
            // bei jedem Seitenaufruf. Der JS-Refresh unten aktualisiert
            // #logFileName danach eigenstaendig ueber den X-Log-File Header
            // von /api/currentLog, damit eine Rotation waehrend des
            // Betrachtens sichtbar wird.

            // The current log filename is resolved for display - this is
            // ONLY a Preferences lookup (see getCurrentLogFileName() in
            // system_utils.h), not a file read, so it's unproblematic on
            // every page load. The JS refresh below then keeps #logFileName
            // up to date independently via the X-Log-File header of
            // /api/currentLog, so a rotation while the tab is open becomes
            // visible.
            String currentLogFileName = loggingEnabled ? getCurrentLogFileName() : "-";

            // Breite bewusst auf 900px angehoben (Standard-.card waere nur
            // 500px) - sonst stuende diese schmalere Karte optisch versetzt
            // ueber dem breiteren <pre>-Logfenster darunter, das ebenfalls
            // 900px breit ist (siehe dort).
            // Width deliberately widened to 900px (the default .card would
            // be only 500px) - otherwise this narrower card would sit
            // visually offset above the wider <pre> log window below, which
            // is also 900px wide (see there).
            // disabledAttr: ist Logging deaktiviert, liefert /api/currentLog
            // ohnehin dauerhaft nur "Logging ist deaktiviert" - Checkbox und
            // Button werden dann deaktiviert (statt einen Klick zu erlauben,
            // der sichtbar nichts bewirkt).
            // disabledAttr: if logging is disabled, /api/currentLog would
            // permanently just return "Logging is disabled" anyway - the
            // checkbox and button are then disabled (instead of allowing a
            // click that visibly does nothing).
            String disabledAttr = loggingEnabled ? "" : " disabled";

            chunk += "<div class='card' style='max-width:900px;'>";
            chunk += "<div style='margin-bottom:8px;'>" + translate("Log file") + ": <code id='logFileName'>" + escapeHtmlText(currentLogFileName) + "</code></div>";
            chunk += "<div style='display:flex;align-items:center;gap:14px;flex-wrap:wrap;'>";
            chunk += "<label style='display:flex;align-items:center;gap:6px;white-space:nowrap;cursor:pointer;'>";
            chunk += "<input type='checkbox' id='logAutoRefresh' style='width:auto;margin:0;'" + disabledAttr + ">";
            chunk += translate("Auto-refresh (10s)");
            chunk += "</label>";
            // Bewusst NICHT die vorhandene .reset-btn Klasse (Neustart-Button
            // oben in der Topbar) wiederverwendet: deren Hover-/Focus-Zustand
            // ist fest auf var(--bad) (rot) eingefaerbt, um vor der
            // Gefahren-Aktion "Neustart" zu warnen - fuer ein harmloses
            // "Jetzt aktualisieren" waere das irrefuehrend. Eigene, neutrale
            // Farbgebung analog zu .tabnav label.
            // Deliberately NOT reusing the existing .reset-btn class (the
            // reboot button up in the topbar): its hover/focus state is
            // hard-coded to var(--bad) (red) to warn about the dangerous
            // "reboot" action - misleading for a harmless "refresh now".
            // Own, neutral styling mirroring .tabnav label instead.
            chunk += "<button type='button' id='logRefreshNow' style='background:var(--panel);border:1px solid var(--panel-border);color:var(--text);border-radius:.4rem;padding:4px 12px;font-size:.8rem;cursor:pointer;'" + disabledAttr + ">" + translate("Refresh now") + "</button>";
            chunk += "<span id='logErrorHint' class='offline-hint'>&#9888; " + translate("Refresh failed - showing last known content") + "</span>";
            chunk += "</div>";
            chunk += "</div>";

            // Kein serverseitiges Vorab-Lesen der Logdatei mehr (frueher hier
            // per LittleFS.open()/readString() bis zu 10 KB pro Seitenaufruf,
            // siehe "Lazy-Load"-Kommentar unten) - nur ein Platzhalter, der
            // Inhalt kommt gleich beim Laden per JS von /api/currentLog.

            // No more server-side pre-reading of the log file (previously up
            // to 10 KB per page load via LittleFS.open()/readString(), see
            // the "lazy load" comment below) - just a placeholder, the actual
            // content arrives right on load via JS from /api/currentLog.
            chunk += "<pre id='logContent' style='background:var(--panel);border:1px solid var(--panel-border);border-radius:10px;max-width:900px;height:400px;overflow-y:auto;margin:15px auto;padding:12px 16px;text-align:left;white-space:pre-wrap;word-break:break-word;font-family:monospace;font-size:.85rem;'>";
            chunk += loggingEnabled ? translate("Loading&hellip;") : translate("Logging is disabled.");
            chunk += "</pre>";

            // Auto-Refresh: Checkbox-Zustand in localStorage gemerkt (Default
            // aus, falls noch nichts gespeichert ist). Bei aktivem Refresh
            // wird alle 10s /api/currentLog gepollt und der <pre>-Inhalt
            // ersetzt; bei deaktiviertem Refresh passiert kein Polling mehr,
            // nur der "Jetzt aktualisieren"-Button oder das Einschalten der
            // Checkbox loesen dann noch einen Abruf aus. Pausiert zusaetzlich
            // per visibilitychange, waehrend der Browser-Tab im Hintergrund
            // ist - analog zum Live-Status-Skript der Topbar weiter oben
            // (siehe Kommentar dort: "unnoetige Anfragen an den ESP32
            // sparen") - und holt beim Zurueckkehren sofort den aktuellen
            // Stand nach, statt bis zu 10s auf den naechsten Intervall-Tick
            // zu warten.
            //
            // Lazy-Load: der initiale Loginhalt wird NICHT mehr serverseitig
            // mitgerendert (siehe Platzhalter oben), sondern hier per fetch()
            // direkt beim Laden nachgeladen - die CSS-only-Tabs rendern ALLE
            // Panels bei JEDEM Aufruf von "/" mit, auch wenn der Log-Tab nie
            // geoeffnet wird; ein serverseitiges Lesen von bis zu 10 KB aus
            // LittleFS bei jedem Seitenaufruf (WLAN, Helligkeit, ...) waere
            // unnoetiger Heap-/CPU-Verbrauch auf dem speicherknappen ESP32.
            // Ist Logging deaktiviert wird der Platzhalter-Text stehen
            // gelassen und gar nicht erst gepollt - da /api/currentLog dann
            // ohnehin nur "Logging ist deaktiviert" liefern wuerde.
            //
            // Fehleranzeige: schlaegt ein Abruf fehl (z.B. WLAN kurz weg),
            // bleibt der zuletzt bekannte Inhalt sichtbar, statt ihn
            // stillschweigend zu verwerfen - #logErrorHint (analog zu
            // "#topbar-offline-hint" oben) macht das sichtbar; verschwindet
            // automatisch beim naechsten erfolgreichen Abruf.
            //
            // Scroll-Verhalten: JEDER Log-Refresh (Timer-Tick, Checkbox
            // einschalten, "Jetzt aktualisieren"-Button, Rueckkehr aus dem
            // Hintergrund) scrollt IMMER ans Ende - klassisches "tail -f"-
            // Verhalten. Eine fruehere Fassung scrollte bei automatischen
            // Hintergrund-Refreshs nur, wenn man schon (nahe) am Ende war,
            // um die Ansicht beim Lesen aelterer Zeilen nicht wegzureissen -
            // das wurde auf ausdruecklichen Wunsch durch dieses einfachere
            // "immer ans Ende" ersetzt: wer Auto-Refresh aktiviert hat, will
            // dem Live-Log folgen.

            // Also pauses via visibilitychange while the browser tab is in
            // the background - mirroring the topbar's live-status script
            // further above (see the comment there: "saves pointless
            // requests to the ESP32") - and immediately catches up on
            // return instead of waiting up to 10s for the next interval tick.
            //
            // Lazy load: the initial log content is no longer rendered
            // server-side (see the placeholder above) - instead it's fetched
            // here right on load. The CSS-only tabs render ALL panels on
            // EVERY "/" request, even if the Log tab is never opened; a
            // server-side read of up to 10 KB from LittleFS on every page
            // load (WiFi, brightness, ...) would be pointless heap/CPU usage
            // on the memory-constrained ESP32. If logging is disabled, the
            // placeholder text is left as-is and never polled at all - since
            // /api/currentLog would just return "Logging is disabled." anyway.
            //
            // Error display: if a fetch fails (e.g. WiFi briefly drops), the
            // last known content stays visible instead of being silently
            // discarded - #logErrorHint (mirroring "#topbar-offline-hint"
            // above) makes that visible; disappears automatically on the
            // next successful fetch.
            //
            // Scroll behavior: EVERY log refresh (timer tick, enabling the
            // checkbox, the "Refresh now" button, returning from the
            // background) ALWAYS scrolls to the bottom - classic "tail -f"
            // behavior. An earlier version only auto-scrolled a background
            // refresh if the view was already (near) the bottom, to avoid
            // yanking the view away while reading older lines - replaced, on
            // explicit request, by this simpler "always scroll to the
            // bottom": enabling Auto-Refresh means you want to follow the
            // live log.
            chunk += "<script>";
            chunk += "(function() {";
            chunk += "  var cb = document.getElementById('logAutoRefresh');";
            chunk += "  var pre = document.getElementById('logContent');";
            chunk += "  var fnEl = document.getElementById('logFileName');";
            chunk += "  var errEl = document.getElementById('logErrorHint');";
            chunk += "  var refreshBtn = document.getElementById('logRefreshNow');";
            chunk += "  var timer = null;";
            chunk += "  var loggingEnabled = " + String(loggingEnabled ? "true" : "false") + ";";
            chunk += "  var stored = localStorage.getItem('uhr3LogAutoRefresh');";
            chunk += "  cb.checked = (stored === null) ? false : (stored === '1');";
            chunk += "  function scrollToBottom() { pre.scrollTop = pre.scrollHeight; }";
            chunk += "  function refreshLog() {";
            chunk += "    if (!loggingEnabled) return;";
            chunk += "    fetch('/api/currentLog', {cache:'no-store'}).then(function(r){";
            chunk += "      var fname = r.headers.get('X-Log-File');";
            chunk += "      if (fname !== null) fnEl.textContent = fname;";
            chunk += "      return r.text();";
            chunk += "    }).then(function(text){";
            chunk += "      errEl.classList.remove('show');";
            chunk += "      pre.textContent = text;";
            chunk += "      scrollToBottom();";
            chunk += "    }).catch(function(){ errEl.classList.add('show'); });";
            chunk += "  }";
            chunk += "  function stopTimer() { if (timer) { clearInterval(timer); timer = null; } }";
            chunk += "  function applyState() {";
            chunk += "    stopTimer();";
            chunk += "    if (cb.checked && !document.hidden) {";
            chunk += "      timer = setInterval(refreshLog, 10000);";
            chunk += "    }";
            chunk += "  }";
            chunk += "  refreshBtn.addEventListener('click', refreshLog);";
            chunk += "  cb.addEventListener('change', function() {";
            chunk += "    localStorage.setItem('uhr3LogAutoRefresh', cb.checked ? '1' : '0');";
            chunk += "    if (cb.checked) refreshLog();";
            chunk += "    applyState();";
            chunk += "  });";
            chunk += "  document.addEventListener('visibilitychange', function() {";
            chunk += "    if (document.hidden) { stopTimer(); }";
            chunk += "    else { if (cb.checked) refreshLog(); applyState(); }";
            chunk += "  });";
            chunk += "  refreshLog();";
            chunk += "  applyState();";
            chunk += "})();";
            chunk += "</script>";

            chunk += "</div>"; // Ende panel-log
                               // end panel-log

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
            // HINWEIS: hostname wird hier bewusst NICHT escapeHtmlText()-behandelt -
            // title ist mit "'" begrenzt, ein "'" im Hostnamen wuerde also
            // trotzdem ausbrechen koennen (Hostname stammt aus /sethostname,
            // s. Kommentar oben beim value-Attribut). Deshalb hier zusaetzlich escapen.
            // NOTE: hostname is deliberately NOT left unescaped here either -
            // title is delimited with "'", so a "'" in the hostname could still
            // break out (hostname comes from /sethostname, see the comment
            // above at the value attribute). So escape it here too.
            chunk += "<span title='" + translate("The clock can also be reached at http://&quot;hostname&quot;.local instead of its IP address, e.g.") + " http://" + escapeHtmlText(String(hostname)) + ".local. " + translate("A restart is required for a changed hostname to take effect. Not all routers support hostname resolution") + ".' style='cursor:help;'>&#9432;</span>";
            // escapeHtmlText(): hostname wird vom Nutzer selbst gesetzt
            // (/sethostname) und ohne Zeichenfilterung gespeichert - ohne
            // Escaping haette ein "'" darin dieses value-Attribut aufbrechen
            // und beliebiges HTML/Attribute einschleusen koennen.
            // escapeHtmlText(): hostname is set by the user themselves
            // (/sethostname) and stored without character filtering -
            // without escaping, a "'" in it could have broken this value
            // attribute and injected arbitrary HTML/attributes.
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
                // escapeHtmlText(): wifiSsid[i] stammt aus gespeicherten Preferences,
                // die urspruenglich aus einem WLAN-Scan oder einer Nutzereingabe (/savewifi)
                // stammen - ein SSID-Name mit "'" darin wuerde sonst dieses value-Attribut
                // aufbrechen koennen (gespeicherte/stored XSS ueber SSID-Namen).
                // escapeHtmlText(): wifiSsid[i] comes from stored preferences, originally
                // populated from a WiFi scan or user input (/savewifi) - an SSID name
                // containing "'" could otherwise break out of this value attribute
                // (stored XSS via SSID name).
                chunk += "<input name='" + ssidKey + "' id='" + ssidKey + "' placeholder='" + ssidKey + "' value='" + escapeHtmlText(wifiSsid[i]) + "' style='width:110px;'>";
                chunk += "<input name='" + passKey + "' id='" + passKey + "' placeholder='Password' type='password' value='' style='width:110px;'>";
                if (wifiSsid[i] != "") {
                    // "Verbinden"-Button nur anzeigen, wenn mehr als ein Netzwerk
                    // gespeichert ist - bei nur einem Eintrag ist er bereits
                    // (oder wird beim Speichern) die aktive Verbindung.

                    // Only show the "Connect" button when more than one network
                    // is saved - with just one entry it's already (or will
                    // become, once saved) the active connection anyway.
                    // escapeForJsStringInAttr(..., '"'): outer HTML-Attribut nutzt "'" (per
                    // HTML-Entity zu escapen), das innere JS-String-Literal in confirm(...)
                    // nutzt '"' (per Backslash zu escapen) - siehe Erklaerung bei der Definition
                    // von escapeForJsStringInAttr() weiter oben in dieser Datei.
                    // escapeForJsStringInAttr(..., '"'): the outer HTML attribute uses "'"
                    // (escape via HTML entity), the inner JS string literal in confirm(...)
                    // uses '"' (escape via backslash) - see the explanation at
                    // escapeForJsStringInAttr()'s definition earlier in this file.
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
            // Default false, wie ueberall sonst (siehe Kommentar bei
            // smoothMinuteActive weiter oben in dieser Datei) - hier stand
            // abweichend "true": direkt nach einem Werkreset zeigte diese
            // Checkbox faelschlich "aktiviert", obwohl der Minutenzeiger
            // tatsaechlich in 1-Minuten-Schritten sprang. Speicherte man das
            // Formular unveraendert (z.B. weil nur eine andere Einstellung
            // geaendert werden sollte), wurde "Smooth Minute Hand" dadurch
            // ungewollt aktiviert.
            // Default false, matching everywhere else (see the comment at
            // smoothMinuteActive further above in this file) - this used to
            // say "true": right after a factory reset this checkbox falsely
            // showed as "enabled", even though the minute hand actually
            // jumped in 1-minute steps. Saving the form unchanged (e.g.
            // because only some other setting was meant to change) would
            // then unintentionally enable "Smooth Minute Hand".
            chunk += preferences.getBool(PK_SMOOTH_MINUTE, false) ? "checked" : "";
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

            // Nur auf Builds mit angeschlossenem DCF77-Empfaenger sichtbar
            // (dieselbe Bedingung wie auf der /dcf77-Seite selbst) - NICHT an
            // dcf77Confirmed gekoppelt (das Flag, das den Nav-Link/den
            // Topbar-Punkt erst nach der ersten plausiblen Impulskette
            // einblendet): genau in der Phase VOR dcf77Confirmed blinkt die
            // LED am haeufigsten, waere die Checkbox erst danach sichtbar,
            // liesse sie sich nicht abschalten, wenn man sie am noetigsten
            // braucht (schwacher/noch fehlender Empfang).
            //
            // Only visible on builds with a DCF77 receiver wired up (same
            // condition as on the /dcf77 page itself) - deliberately NOT
            // tied to dcf77Confirmed (the flag that only reveals the nav
            // link/topbar dot after the first plausible pulse chain): the
            // LED blinks most often in exactly the phase BEFORE
            // dcf77Confirmed, so gating the checkbox on it would make it
            // impossible to turn off right when it's needed most (weak or
            // not-yet-established reception).
#if defined(DCF77_DATAPIN) && defined(DCF77_INTERRUPT)
            chunk += "<div style='display:flex;align-items:center;gap:6px;white-space:nowrap;'><input type='checkbox' name='dcfSyncLed' value='1' ";
            chunk += dcfSyncLedEnabled ? "checked" : "";
            chunk += " style='width:auto;margin:0;'>" + translate("DCF77 Sync LED Blink");
            chunk += " <span title='" + translate("Flashes the LED for every received DCF77 pulse while the clock is still acquiring the time signal") + ".' style='cursor:help;'>&#9432;</span></div><br>";
#endif

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

            setLedOff(); // Gegenstueck zu setLedOn() ganz oben in diesem Handler
                         // counterpart to setLedOn() at the very top of this handler
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
                // setId wird aus hochgeladenen Dateinamen extrahiert (siehe handleFileUpload()) -
                // dort wird nur ein "/hand_set"-Praefix und ".bmp"-Suffix erzwungen, der Teil
                // dazwischen (also setId) ist NICHT auf harmlose Zeichen eingeschraenkt. Ein
                // praeparierter Upload-Dateiname koennte daher HTML/JS-Metazeichen enthalten
                // (gespeicherte/stored XSS) - deshalb hier ueberall escapen, bevor setId in HTML
                // bzw. in den confirm()-JS-String eingebettet wird. Fuer die Dateisystem-Pfade
                // (LittleFS.exists()/-Zugriff) wird weiterhin die unescapte Rohvariante benutzt,
                // da diese exakt dem echten Dateinamen entsprechen muss.
                //
                // setId is extracted from uploaded filenames (see handleFileUpload()) - there,
                // only a "/hand_set" prefix and ".bmp" suffix are enforced, the part in between
                // (i.e. setId) is NOT restricted to harmless characters. A crafted upload
                // filename could therefore contain HTML/JS metacharacters (stored XSS) - so
                // escape it everywhere before embedding it into HTML or into the confirm() JS
                // string. The filesystem paths (LittleFS.exists()/access) still use the raw,
                // unescaped variant, since that must exactly match the real filename.
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

            // Zusaetzlich auf ein sicheres Zeichen-Set beschraenken (analog zur
            // gleichartigen Pruefung bei /rename oben): bisher wurde der Teil
            // zwischen Praefix und ".bmp" ungeprueft uebernommen. Aus diesem
            // Dateinamen wird spaeter u.a. der "setId"-Teil extrahiert und in
            // /handsets sowie im GitHub-Merge-Skript ungeescaped in HTML-
            // Attribute, onclick-Strings bzw. JS-String-Literale in <script>-
            // Bloecken eingebettet (siehe escapeHtmlText()/escapeForJsStringInAttr()/
            // escapeForJsStringLiteral()-Aufrufe dort) - ohne diese Einschraenkung
            // waere ein praeparierter Upload-Dateiname (z.B. mit "</script>" oder
            // "..") ein Weg zu gespeichertem XSS bzw. potenziell zu Pfad-Traversal.
            //
            // Additionally restrict to a safe character set (mirroring the same
            // kind of check at /rename above): until now, the part between the
            // prefix and ".bmp" was accepted unchecked. This filename is later
            // used to extract the "setId" part among other things and gets
            // embedded, unescaped without this restriction, into HTML attributes,
            // onclick strings and JS string literals inside <script> blocks in
            // /handsets and the GitHub merge script (see the escapeHtmlText()/
            // escapeForJsStringInAttr()/escapeForJsStringLiteral() calls there) -
            // without this restriction, a crafted upload filename (e.g.
            // containing "</script>" or "..") would be a path to stored XSS or
            // potentially path traversal.
            {
                bool uploadNameValid = true;
                for (size_t ni = 1; uploadNameValid && ni < uploadFilePath.length(); ni++) {
                    char c = uploadFilePath[ni];
                    if (!(isalnum((unsigned char)c) || c == '_' || c == '-' || c == '.')) {
                        uploadNameValid = false;
                    }
                }
                if (!uploadNameValid || uploadFilePath.indexOf("..") >= 0) {
                    DEBUG_PRINTLN("[UPLOAD] Invalid filename: contains disallowed characters : " + uploadFilePath);
                    uploadSuccess = false;
                    return;
                }
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
