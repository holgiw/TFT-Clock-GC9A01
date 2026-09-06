#pragma once
    // ### Globale Objekte, Variablen und Datenstrukturen #################
    // Benoetigt config.h (davor eingebunden). Sortiert nach Modul (WLAN, Zeit/DCF77/RTC, Zifferblatt/Display, Helligkeit, Touch, Presets, System).
    // Enthaelt bewusst echte Definitionen statt extern-Deklarationen, da nur von uhr3.ino aus eingebunden (eine Uebersetzungseinheit) - hatte sonst zu Problemen gefuehrt.

    // ### Global objects, variables, and data structures #################
    // Requires config.h (included beforehand). Sorted by module (WiFi, time/DCF77/RTC, clock face/display, brightness, touch, presets, system).
    // Contains real definitions instead of extern declarations on purpose, since only included from uhr3.ino (one translation unit) - otherwise caused problems.

    // --- System / Allgemein ---
    // --- System / General ---
    String currentLanguage = "de"; // Standardmäßig Deutsch
                                   // Default: German

    char version[20]; // Build-Version ("YYYY-MM-DD HH:MM:SS" = 19 Zeichen + Nullterminator)
                      // Build version ("YYYY-MM-DD HH:MM:SS" = 19 chars + null terminator)

    bool loggingEnabled = false;

    bool initial = true;

    String ipAddress = "";


    // --- Kern-Hardwareobjekte (TFT, Webserver, Preferences, RTC, ...) ---
    // --- Core hardware objects (TFT, web server, preferences, RTC, ...) ---
    TFT_eSPI tft = TFT_eSPI();
    WebServer webserver(80);
    Preferences preferences;
    DNSServer dnsServer;
    WiFiUDP udp;

#if defined SDA_PIN && defined SCL_PIN
    RTC_DS3231 rtc;
#endif

    // --- WLAN ---
    // --- WiFi ---
#define MAX_WLAN 15
    String wifiSsid[MAX_WLAN];
    String wifiPass[MAX_WLAN];

#define NOT_CONNECTED 0
#define CONNECTED 1
#define CONNECTED_NO_INTERNET 2

    bool wifiActive = true;

    // Zustand fuer eine per Web-Button ausgeloeste WPS-Anfrage (siehe loop() in
    // uhr3.ino und /api/startWPS in webserver_routes.h) - laeuft asynchron, damit
    // der Webserver waehrend der WPS-Aushandlung nicht blockiert.

    // State for a WPS request triggered by a web button (see loop() in uhr3.ino
    // and /api/startWPS in webserver_routes.h) - runs asynchronously so the web
    // server isn't blocked during WPS negotiation.
    // Zustand fuer eine per Web-Button ausgeloeste WPS-Anfrage (siehe loop() in
    // uhr3.ino und /api/startWPS in webserver_routes.h) - event-basiert statt
    // Status-Polling, da WiFi.onEvent() laut offiziellem Espressif-WPS-Beispiel
    // der zuverlaessige Weg ist, den Erfolg/Fehlschlag von WPS zu erkennen.

    // State for a WPS request triggered by a web button (see loop() in uhr3.ino
    // and /api/startWPS in webserver_routes.h) - event-based instead of status
    // polling, since WiFi.onEvent() is, per the official Espressif WPS example,
    // the reliable way to detect WPS success/failure.
    bool wpsPending = false;
    unsigned long wpsStartMillis = 0;
    String wpsPreviousSsid = ""; // Verbindung vor dem WPS-Start, um danach ggf. dorthin zurueckzuwechseln
                                 // Connection before the WPS start, to switch back to it afterward if needed
    volatile bool wpsSuccessEvent = false; // wird im WiFi-Event-Callback gesetzt (anderer Kontext!)
                                           // set in the WiFi event callback (different context!)
    volatile bool wpsFailedEvent = false;

    // MAC Adresse
    // MAC address
    uint8_t mac[6];
    char hostname[32];

    // Zur Laufzeit erzeugtes Passwort des Einrichtungs-Access-Points (aus den
    // letzten vier MAC-Bytes, siehe startAP() in wifi_manager.h). Als Puffer
    // gehalten, damit es sowohl auf dem Display als auch in der Statuszeile der
    // Weboberflaeche angezeigt werden kann. Leer, solange der AP nie lief.

    // Password of the setup access point, generated at runtime (from the last
    // four MAC bytes, see startAP() in wifi_manager.h). Kept as a buffer so it
    // can be shown both on the display and in the web interface's status line.
    // Empty as long as the AP has never run.
    char apPassword[16] = "";
    bool pingHostname = false;

    bool softAPIP = false;  // Flag für SoftAP IP
                            // Flag for SoftAP IP
    long softAPIPstart = 0;  // Startzeit für SoftAP IP
                             // Start time for SoftAP IP

    struct WifiNetwork {
        String ssid;
        int rssi;
        int enc;
    };
    WifiNetwork availableNetworks[MAX_WLAN];
    int foundNetworkCount = 0;
    bool isScanning = false;

    // --- Zeit, NTP, DCF77, RTC ---
    // --- Time, NTP, DCF77, RTC ---
    // NTP-Server-Port
    // NTP server port
    const int NTP_PORT = 123;
    // NTP-Paketgröße
    // NTP packet size
    const int NTP_PACKET_SIZE = 48;
    byte ntpPacket[NTP_PACKET_SIZE];

#if defined DCF77_DATAPIN && defined DCF77_INTERRUPT

    // Die DCF77-Bibliothek und ihre Flanken-Einstellung sind hier ersatzlos
    // entfallen (frueher: "bool dcf77Flank" und eine DCF77-Instanz).
    //
    // Historie: bei anhaltendem "DCF77 last sync: never" trotz nachweislich
    // sauberem Empfang wurde die Flanken-Polaritaet als Ursache vermutet und
    // testweise auf steigende Flanke umgestellt - ohne Wirkung. Die
    // tatsaechliche Ursache lag in der Bibliothek selbst; seit der Umstellung
    // auf den eigenen Dekoder ist die Frage gegenstandslos:
    //
    // Der Interrupt liegt auf CHANGE (siehe attachInterrupt() in uhr3.ino),
    // reagiert also auf BEIDE Flanken, und processDcf77Bits() (time_sync.h)
    // klassifiziert ausschliesslich ueber die DAUER zwischen zwei Flanken.
    // Von zwei aufeinanderfolgenden Intervallen ist das kurze immer der
    // Impuls und das lange die Pause - unabhaengig davon, ob das
    // Empfaengermodul den Impuls als High- oder als Low-Pegel ausgibt. Der
    // Pegel wird nirgends gelesen. Eine einstellbare oder automatisch
    // erkannte Flankenrichtung braucht es damit nicht.

    // The DCF77 library and its edge setting have been removed entirely here
    // (formerly: "bool dcf77Flank" and a DCF77 instance).
    //
    // History: with a persistent "DCF77 last sync: never" despite demonstrably
    // clean reception, the edge polarity was suspected as the cause and
    // switched to rising as a test - with no effect. The actual cause was
    // inside the library itself; since the switch to the own decoder the
    // question is moot:
    //
    // The interrupt sits on CHANGE (see attachInterrupt() in uhr3.ino), so it
    // reacts to BOTH edges, and processDcf77Bits() (time_sync.h) classifies
    // purely by the DURATION between two edges. Of two consecutive intervals
    // the short one is always the pulse and the long one the gap - regardless
    // of whether the receiver module outputs the pulse as a high or a low
    // level. The level is never read. No configurable or auto-detected edge
    // direction is needed.

    volatile uint16_t dcf77Count = 0; // Anzahl der empfangenen DCF77-Signale (wird in der ISR verändert)
                                      // Number of received DCF77 signals (modified in the ISR)

    volatile bool dcfTimeFound = false; // wird in der ISR gelesen/verändert
                                        // read/modified in the ISR

    // Wird in der ISR gesetzt und in loop() abgearbeitet: die ISR darf die LED
    // NICHT selbst schalten, weil setLedOn()/setLedOff() ueber pinMode()/
    // digitalWrite() gehen und damit im Flash liegen. Ist der Flash-Cache
    // gerade deaktiviert (bei JEDEM LittleFS-Schreibvorgang - also auch bei
    // jeder Logzeile - und bei jedem NVS-Commit), fuehrt ein Zugriff aus der
    // ISR heraus zu einem "Cache disabled but cached memory region accessed"-
    // Panic-Reset. Siehe isr() in time_sync.h und die Abarbeitung in loop().

    // Set in the ISR and processed in loop(): the ISR must NOT switch the LED
    // itself, because setLedOn()/setLedOff() go through pinMode()/
    // digitalWrite() and therefore live in flash. While the flash cache is
    // disabled (during EVERY LittleFS write - so also every log line - and
    // every NVS commit), accessing it from inside the ISR causes a "Cache
    // disabled but cached memory region accessed" panic reset. See isr() in
    // time_sync.h and the handling in loop().
    volatile bool dcfLedTogglePending = false;

    // Zeitpunkt (millis()), zu dem der aktuelle Einmal-Blitz der LED wieder
    // ausgeschaltet werden soll; 0 = kein Blitz aktiv (siehe
    // DCF77_LED_BLINK_MS in config.h und die Abarbeitung in loop()).

    // Time (millis()) at which the LED's current one-shot flash is to be
    // switched off again; 0 = no flash active (see DCF77_LED_BLINK_MS in
    // config.h and the handling in loop()).
    unsigned long dcfLedOffAtMillis = 0;

    bool dcfSyncLedEnabled = true; // per Auswahlbox abschaltbar (Default: an) - steuert nur, ob loop() das per dcfLedTogglePending
                                    // angeforderte Blinken tatsaechlich ausfuehrt (siehe PK_DCF_SYNC_LED in prefs_keys.h); die ISR selbst
                                    // setzt dcfLedTogglePending unabhaengig davon immer, das Flag hier wird erst in loop() ausgewertet

                                    // switchable off via a checkbox (default: on) - only controls whether loop() actually carries out the
                                    // blink requested via dcfLedTogglePending (see PK_DCF_SYNC_LED in prefs_keys.h); the ISR itself always
                                    // sets dcfLedTogglePending regardless, this flag is only evaluated in loop()

    time_t lastDcfSyncTime = 0; // Unix-Zeitstempel der letzten erfolgreichen DCF77-Synchronisation (0 = noch nie)
                                // Unix timestamp of the last successful DCF77 sync (0 = never)

    unsigned long lastDcf77PulseChangeMillis = 0; // millis()-Zeitpunkt der letzten beobachteten Aenderung von dcf77Count (siehe checkDcf77Health() in time_sync.h) -
                                                   // erkennt einen kompletten Empfangsausfall waehrend des Betriebs, da dcf77Count selbst nie auf 0 zurueckgesetzt wird
                                                   // millis() timestamp of the last observed change in dcf77Count (see checkDcf77Health() in time_sync.h) -
                                                   // detects a complete reception failure during operation, since dcf77Count itself is never reset to 0

    uint8_t dcf77PlausiblePulseStreak = 0; // Anzahl aufeinanderfolgender dcf77Count-Aenderungen mit plausiblem
                                            // Abstand zueinander (siehe DCF77_PRESENCE_MAX_GAP_MS in config.h,
                                            // gepflegt von checkDcf77Health() in time_sync.h) - filtert einzelne,
                                            // durch Rauschen auf einem floatenden Empfaengerpin ausgeloeste
                                            // Interrupts heraus, siehe dcf77Confirmed unten
                                            // number of consecutive dcf77Count changes with a plausible gap between
                                            // them (see DCF77_PRESENCE_MAX_GAP_MS in config.h, maintained by
                                            // checkDcf77Health() in time_sync.h) - filters out isolated interrupts
                                            // triggered by noise on a floating receiver pin, see dcf77Confirmed below

    bool dcf77Confirmed = false; // wird EINMALIG true, sobald dcf77PlausiblePulseStreak DCF77_PRESENCE_MIN_STREAK
                                 // erreicht (siehe config.h) - bestimmt, ob der DCF77-Eintrag in der Topbar
                                 // ueberhaupt angezeigt wird (siehe getDcf77Status() in webserver_routes.h);
                                 // wird danach NIE zurueckgesetzt, genau wie dcfTimeFound/dcf77Count
                                 // becomes true EXACTLY ONCE, when dcf77PlausiblePulseStreak reaches
                                 // DCF77_PRESENCE_MIN_STREAK (see config.h) - decides whether the DCF77 entry
                                 // in the topbar is shown at all (see getDcf77Status() in webserver_routes.h);
                                 // never reset afterwards, just like dcfTimeFound/dcf77Count

    // --- Eigener, von der DCF77-Bibliothek unabhaengiger Bit-Fortschritt ---
    // Liefert sowohl die Live-Anzeige auf /dcf77 (siehe webserver_routes.h)
    // ALS AUCH die tatsaechliche Zeituebernahme (dcf77LastDecoded weiter
    // unten, ausgewertet von applyDcf77DecodedTime()/updateDcf77Status() in
    // time_sync.h) - dcf.getUTCTime() der Original-Bibliothek wird dafuer
    // NICHT mehr benutzt (siehe dortige Kommentare zur Vorgeschichte).
    // DCF77::int0handler() wird in isr() nicht mehr aufgerufen (lag im Flash
    // und war damit in der ISR ein Absturz-/Flankenverlustrisiko). Die ISR (isr() in time_sync.h)
    // schreibt bei jeder Flanke NUR Zeitstempel + Pegel in das kleine
    // Ringpuffer-Array unten (reines RAM, keine Flash-residenten Aufrufe -
    // sicher fuer die ISR, siehe deren Flash-Cache-Warnung). Die eigentliche
    // Bitklassifizierung und Dekodierung passiert ausschliesslich in
    // processDcf77Bits()/decodeDcf77Telegram() (time_sync.h), aufgerufen aus
    // loop(), NIEMALS in der ISR selbst.

    // --- Own DCF77 bit progress, independent of the DCF77 library ---
    // Drives both the live display on /dcf77 (see webserver_routes.h) AND
    // the actual time takeover (dcf77LastDecoded further below, consumed by
    // applyDcf77DecodedTime()/updateDcf77Status() in time_sync.h) - the
    // original library's dcf.getUTCTime() is NO LONGER used for that (see
    // the comments there for the backstory). DCF77::int0handler() is no
    // longer called at all (see isr() in time_sync.h). The ISR ONLY writes a
    // timestamp into the ring buffer array below on every edge (plain RAM, no
    // flash-resident calls - safe for the ISR, see its flash-cache
    // warning). The actual bit
    // classification and decoding happen exclusively in
    // processDcf77Bits()/decodeDcf77Telegram() (time_sync.h), called from
    // loop(), NEVER in the ISR itself.
    // Von 16 auf 64 vergroessert (kostet 256 Byte RAM): der Puffer muss die
    // laengste Pause zwischen zwei processDcf77Bits()-Aufrufen ueberbruecken.
    // Echter DCF77-Empfang erzeugt 2 Flanken pro Sekunde, 16 Plaetze reichten
    // also nur fuer rund 8 Sekunden - ein blockierender NTP-Versuch
    // (setupNTP(), mehrere WAIT_3s hintereinander), ein groesserer
    // Webserver-Request oder eine Schreibserie auf LittleFS ueberschreitet das
    // muehelos, und JEDE danach ankommende Flanke ging verloren. Mit 64
    // Plaetzen sind es rund 32 Sekunden. Der Zaehler unten zeigt auf /dcf77,
    // ob es trotzdem noch passiert.

    // Enlarged from 16 to 64 (costs 256 bytes of RAM): the buffer has to
    // bridge the longest pause between two processDcf77Bits() calls. Genuine
    // DCF77 reception produces 2 edges per second, so 16 slots only covered
    // about 8 seconds - a blocking NTP attempt (setupNTP(), several WAIT_3s in
    // a row), a larger web server request or a burst of LittleFS writes
    // exceeds that easily, and EVERY edge arriving afterwards was lost. With
    // 64 slots it is about 32 seconds. The counter below shows on /dcf77
    // whether it still happens anyway.
#define DCF77_EDGE_BUFFER_SIZE 64
    volatile unsigned long dcf77EdgeMillis[DCF77_EDGE_BUFFER_SIZE];
    // dcf77EdgeLevel[] entfernt: der Pegel wurde nie ausgewertet (die
    // Klassifizierung in processDcf77Bits() arbeitet ausschliesslich ueber die
    // Dauer zwischen zwei Flanken), das dafuer noetige digitalRead() lag aber
    // im Flash und war damit in der ISR ein Absturz-/Flankenverlustrisiko -
    // siehe isr() in time_sync.h.
    // dcf77EdgeLevel[] removed: the level was never evaluated (classification
    // in processDcf77Bits() works purely from the duration between two edges),
    // yet the digitalRead() it required lived in flash and was therefore a
    // crash/edge-loss risk inside the ISR - see isr() in time_sync.h.
    volatile uint8_t dcf77EdgeHead = 0; // naechster freier Schreibindex - NUR von der ISR veraendert
                                        // next free write index - ONLY changed by the ISR
    volatile uint32_t dcf77EdgeDropped = 0; // Anzahl verworfener Flanken bei vollem Puffer (Diagnose). Als uint32_t statt
                                            // uint8_t: der Zaehler lief nach 256 verworfenen Flanken still ueber und fing
                                            // wieder bei 0 an - genau bei starkem Verlust war der Diagnosewert also wertlos
                                            // number of edges dropped when the buffer was full (diagnostic). uint32_t instead
                                            // of uint8_t: the counter silently wrapped after 256 dropped edges and started
                                            // over at 0 - so exactly under heavy loss the diagnostic value was worthless
    uint8_t dcf77EdgeTail = 0; // naechster zu lesende Index - NUR im Hauptthread (loop()) veraendert
                               // next index to read - ONLY changed on the main thread (loop())

#define DCF77_TELEGRAM_BITS 59

    // dcf77Bits ist nach der RASTERPOSITION indiziert (dcf77Phase, 0..59),
    // NICHT nach der Sekunde der Minute - deshalb 60 Plaetze statt 59.
    //
    // Der Unterschied ist wichtig: die Zuordnung Rasterposition -> Sekunde
    // steht erst fest, wenn die Minutenmarke gefunden ist (dcf77MarkerPos),
    // und das dauert ein paar Minuten. Wurden die Bits vorher nach Sekunde
    // abgelegt, konnte vor dem Markenfund gar nichts gesammelt und angezeigt
    // werden - die /dcf77-Seite blieb in dieser ganzen Zeit leer, obwohl der
    // Empfang laengst lief. Nach Rasterposition abgelegt, laeuft der
    // Bit-Fortschritt ab dem ersten Impuls; die Umrechnung auf die Sekunde
    // passiert erst beim Dekodieren (siehe decodeDcf77Telegram()) bzw. fuer
    // die Anzeige in /api/dcf77status.

    // dcf77Bits is indexed by the GRID POSITION (dcf77Phase, 0..59), NOT by
    // the second of the minute - hence 60 slots instead of 59.
    //
    // The difference matters: the mapping grid position -> second is only
    // fixed once the minute marker has been found (dcf77MarkerPos), and that
    // takes a few minutes. With the bits stored by second, nothing could be
    // collected or displayed before the marker was found - the /dcf77 page
    // stayed empty for that whole time even though reception had long been
    // running. Stored by grid position, the bit progress runs from the very
    // first pulse; conversion to the second happens only when decoding (see
    // decodeDcf77Telegram()) resp. for the display in /api/dcf77status.
#define DCF77_GRID_SLOTS 60
    int8_t dcf77Bits[DCF77_GRID_SLOTS]; // 0/1 je Rasterposition der laufenden Minute, -1 = (noch) unbekannt - NUR Hauptthread
                                        // 0/1 per grid position of the running minute, -1 = unknown (yet) - main thread only
    uint8_t dcf77BitIndex = 0; // Position (Sekunde der Minute) des NAECHSTEN erwarteten Impulses, 0..DCF77_TELEGRAM_BITS -
                               // bei lueckenlosem Empfang gleichbedeutend mit "so viele Bits sind schon da"; nach einer
                               // verlorenen Sekunde bleibt an deren Stelle eine Luecke (dcf77Bits[i] == -1) stehen und der
                               // Index zeigt trotzdem auf die richtige Sekunde weiter (siehe processDcf77Bits() in time_sync.h)

                               // position (second of the minute) of the NEXT expected pulse, 0..DCF77_TELEGRAM_BITS - with
                               // gapless reception this is the same as "this many bits are in"; after a lost second a hole
                               // (dcf77Bits[i] == -1) remains at that spot and the index still points at the correct second
                               // (see processDcf77Bits() in time_sync.h)

    bool dcf77Synced = false;  // true, sobald die Position im Telegramm bekannt ist (eine Minutenmarke wurde erkannt und
                               // seitdem passte jeder Impulsabstand ins Sekundenraster). Solange false, werden empfangene
                               // Impulse bewusst NICHT als Bits abgelegt - ihre Position waere reine Vermutung. Wird wieder
                               // false, wenn ein Impulsabstand in kein Sekundenraster passt (laengerer Aussetzer, Stoerung)
                               // oder ein dekodiertes Telegramm strukturell unmoeglich ist (Bit 0 != 0 bzw. Bit 20 != 1,
                               // siehe decodeDcf77Telegram()) - danach wartet der Dekoder auf die naechste Minutenmarke.

                               // true once the position within the telegram is known (a minute marker was detected and every
                               // pulse distance since then fitted the one-second grid). While false, received pulses are
                               // deliberately NOT stored as bits - their position would be pure guesswork. Goes back to false
                               // when a pulse distance fits no second grid (longer dropout, interference) or a decoded
                               // telegram is structurally impossible (bit 0 != 0 resp. bit 20 != 1, see
                               // decodeDcf77Telegram()) - the decoder then waits for the next minute marker.

    // --- Sekundenraster und Erkennung der Minutenmarke ---------------------
    //
    // dcf77Phase ist eine FREILAUFENDE Rasterposition 0..59 ohne Bezug zur
    // echten Sekunde: sie wird bei jedem erkannten Impuls um die Anzahl der
    // seitdem vergangenen Sekunden weitergezaehlt (siehe processDcf77Bits()).
    // Welche dieser 60 Positionen die 59. Sekunde (die Minutenmarke) ist,
    // ergibt sich aus der Statistik darunter, NICHT aus einem einzelnen
    // Impulsabstand: die Marke ist die einzige Position, an der in JEDER
    // Minute ein Impuls fehlt, waehrend empfangsbedingte Ausfaelle zufaellig
    // ueber alle Positionen streuen. Nach wenigen Minuten hebt sich die Marke
    // dadurch eindeutig ab - auch bei schlechtem Empfang, wo einzelne
    // Impulsabstaende von 2 Sekunden staendig vorkommen und deshalb kein
    // brauchbares Merkmal fuer die Minutenmarke sind.

    // --- Second grid and minute marker detection ---------------------------
    //
    // dcf77Phase is a FREE-RUNNING grid position 0..59 with no relation to the
    // real second: it is advanced by the number of seconds elapsed on every
    // detected pulse (see processDcf77Bits()). Which of those 60 positions is
    // the 59th second (the minute marker) follows from the statistics below,
    // NOT from a single pulse distance: the marker is the only position where
    // a pulse is missing in EVERY minute, while reception-related dropouts
    // scatter randomly across all positions. After a few minutes the marker
    // therefore stands out unambiguously - even with poor reception, where
    // individual pulse distances of 2 seconds occur constantly and are
    // therefore no usable indicator of the minute marker.
    uint8_t dcf77Phase = 0;

    uint8_t dcf77MarkerMiss[60] = { 0 }; // wie oft an dieser Rasterposition ein Impuls fehlte
                                         // how often a pulse was missing at this grid position
    uint8_t dcf77MarkerHit[60] = { 0 };  // wie oft an dieser Rasterposition ein Impuls ankam
                                         // how often a pulse arrived at this grid position

    int8_t dcf77MarkerPos = -1; // erkannte Rasterposition der Minutenmarke, -1 = noch unbekannt
                                // detected grid position of the minute marker, -1 = not known yet

    int8_t dcf77LastSecond = -1; // zuletzt belegte Sekunde der Minute, fuer die Erkennung des Minutenwechsels
                                 // last second of the minute filled in, used to detect the minute change

    uint8_t dcf77StructFails = 0; // aufeinanderfolgende Telegramme mit unmoeglichen Festbits (Bit 0 / Bit 20) -
                                  // ab DCF77_STRUCT_FAIL_LIMIT gilt die erkannte Marke als falsch und wird verworfen
                                  // consecutive telegrams with impossible fixed bits (bit 0 / bit 20) - from
                                  // DCF77_STRUCT_FAIL_LIMIT on, the detected marker counts as wrong and is discarded

    // --- Diagnosewerte fuer die /dcf77-Seite -------------------------------
    // Machen von aussen sichtbar, was der Empfaenger tatsaechlich liefert -
    // ohne Oszilloskop war bisher nicht zu unterscheiden, ob der Dekoder
    // falsch rechnet oder schlicht keine brauchbaren Impulse ankommen.
    // --- Diagnostic values for the /dcf77 page -----------------------------
    // Make visible from the outside what the receiver actually delivers -
    // without an oscilloscope there was previously no way to tell whether the
    // decoder computes wrongly or simply no usable pulses arrive.
    uint32_t dcf77PulsesSeen = 0;    // erkannte Impulse seit dem Start / pulses detected since start
    uint32_t dcf77PulsesMissed = 0;  // uebersprungene Rasterpositionen / grid positions skipped
    uint32_t dcf77PhaseBreaks = 0;   // wie oft das Sekundenraster verlorenging / how often the second grid was lost

#define DCF77_DIAG_SLOTS 12
    uint16_t dcf77DiagWidth[DCF77_DIAG_SLOTS] = { 0 }; // Impulsdauer in ms / pulse width in ms
    uint16_t dcf77DiagGap[DCF77_DIAG_SLOTS] = { 0 };   // Abstand zum vorigen Impulsanfang in ms / distance to the previous pulse start in ms
    uint8_t dcf77DiagIdx = 0;
    uint8_t dcf77DiagCount = 0;

    // Ergebnis der letzten VOLLSTAENDIG dekodierten Minute (siehe
    // decodeDcf77Telegram() in time_sync.h) - bleibt bei einem
    // Paritaetsfehler zu Diagnosezwecken erhalten (valid=false statt das
    // Ergebnis komplett zu verwerfen), damit die Live-Seite auch einen
    // fehlerhaften Empfang sichtbar machen kann.

    // Result of the last FULLY decoded minute (see decodeDcf77Telegram() in
    // time_sync.h) - kept even on a parity error for diagnostic purposes
    // (valid=false instead of discarding the result entirely), so the live
    // page can also make a faulty reception visible.
    struct Dcf77Decoded {
        bool valid = false;       // alle drei Paritaeten (Minute/Stunde/Datum) korrekt
                                  // all three parities (minute/hour/date) correct
        uint8_t minute = 0, hour = 0, day = 0, month = 0, weekday = 0;
        uint16_t year = 0;
        bool dst = false;         // Sommerzeit aktiv (Bit 17)
                                  // daylight saving time active (bit 17)
        bool callBit = false;     // Bit 15 - Anrufbit / unregelmaessige Aussendung
                                  // bit 15 - call bit / irregular transmission
        bool parityMinOk = false, parityHourOk = false, parityDateOk = false;
        uint8_t repairedBits = 0; // wie viele fehlende Bits aus Paritaet/Festwerten rekonstruiert wurden
                                  // how many missing bits were reconstructed from parity/fixed values
        unsigned long decodedAtMillis = 0; // millis() beim Minutenanfang dieses Telegramms, 0 = noch nie
                                           // millis() at this telegram's minute start, 0 = never
    };
    Dcf77Decoded dcf77LastDecoded;

    // Letzte als gueltig bestaetigte Dekodierung, als Bezugspunkt fuer die
    // Kohaerenzpruefung eines rekonstruierten Telegramms (siehe
    // decodeDcf77Telegram() in time_sync.h): wurden fehlende Bits aus der
    // Paritaet ergaenzt, kann die Paritaet diese Gruppe nicht mehr pruefen -
    // stattdessen muss die Zeit exakt zur vorherigen bestaetigten Zeit plus
    // der seitdem verstrichenen Minutenzahl passen. Ein vollstaendig
    // empfangenes Telegramm braucht diese Pruefung nicht.

    // Last decoding confirmed as valid, used as the reference for the
    // coherence check of a reconstructed telegram (see decodeDcf77Telegram()
    // in time_sync.h): when missing bits were filled in from parity, parity
    // can no longer verify that group - instead the time has to match the
    // previously confirmed time plus the number of minutes elapsed since
    // exactly. A fully received telegram does not need this check.
    time_t dcf77PrevEpoch = 0;
    unsigned long dcf77PrevAtMillis = 0;

#endif

    // Ist der EIGENE NTP-Server (die Uhr als Zeitquelle fuer andere Geraete,
    // siehe startNtpServer() in time_sync.h und die Beantwortung in loop())
    // gerade an Port 123 gebunden? Der Rueckgabewert von udp.begin() wurde
    // frueher verworfen und der Start unbesehen als "[NTPD] NTP Server
    // started" geloggt - ob wirklich jemand zuhoert, war weder im Log noch auf
    // der Statusseite zu erkennen.

    // Is the OWN NTP server (the clock as a time source for other devices, see
    // startNtpServer() in time_sync.h and the answering code in loop())
    // currently bound to port 123? The return value of udp.begin() used to be
    // discarded and the start logged unconditionally as "[NTPD] NTP Server
    // started" - whether anyone was actually listening was visible neither in
    // the log nor on the status page.
    bool ntpServerRunning = false;

    // Diagnosezaehler fuer den eigenen NTP-Server (Anzeige auf /status): ohne
    // sie liess sich ein ausbleibender Client-Erfolg nicht einordnen - kommt
    // die Anfrage gar nicht an (Socket/Netz/Firewall) oder wird sie empfangen
    // und nur nicht beantwortet (keine gueltige Systemzeit)?

    // Diagnostic counters for the own NTP server (shown on /status): without
    // them there was no way to place a client failure - does the request not
    // arrive at all (socket/network/firewall), or is it received and merely not
    // answered (no valid system time)?
    uint32_t ntpRequestsReceived = 0;
    uint32_t ntpRepliesSent = 0;

    unsigned long lastNTPUpdate = 0; // Zeitpunkt des letzten RTC-Updates
                                     // Timestamp of the last RTC update
    unsigned long lastDCFUpdate = 0; // Wartezeit nach DCF77-Update, bevor RTC aktualisiert wird (ms)
                                     // Wait time after a DCF77 update before the RTC is updated (ms)
    unsigned long lastRTCUpdate = 0; // Zeitpunkt des letzten RTC-Updates
                                     // Timestamp of the last RTC update
    unsigned long lastNtpSuccessMillis = 0; // Zeitpunkt (millis()) der letzten ERFOLGREICHEN NTP-Synchronisation (0 = noch nie) -
                                            // dient checkHourlyTimeSync-Aufrufstellen in uhr3.ino dazu, einen echten NTP-Erfolg
                                            // vom (irrefuehrenden) blossen Rueckgabewert true von setupNTP() bei fehlendem WLAN
                                            // zu unterscheiden; nur wenn dieser Wert NICHT waehrend des jeweiligen Versuchs
                                            // aktualisiert wurde, springt DCF77 als Zeitquelle ein (siehe time_sync.h)

    // Timestamp (millis()) of the last SUCCESSFUL NTP sync (0 = never) - used
    // at the checkHourlyTimeSync call sites in uhr3.ino to distinguish a real
    // NTP success from setupNTP()'s (misleading) plain true return value when
    // WiFi isn't connected; only when this value was NOT updated during that
    // attempt does DCF77 step in as the time source (see time_sync.h)

    struct tm timeinfo;

    String timezone = TIMEZONE_DEFAULT;

    char ntpServers[MAX_WLAN][64];

#define RTC_NOT_AVAILABLE 0
#define RTC_AVAILABLE 1
#define RTC_AVAILABLE_BUT_INVALID 2

    int rtcOk = RTC_NOT_AVAILABLE;

    String i2cAddr = "";

    // --- Zifferblatt / Display ---
    // --- Clock face / Display ---
    String tftType = "UNKNOWN";

    TFT_eSprite backgroundSprite = TFT_eSprite(&tft);
    TFT_eSprite hourHandSprite = TFT_eSprite(&tft);
    TFT_eSprite minuteHandSprite = TFT_eSprite(&tft);
    TFT_eSprite secondHandSprite = TFT_eSprite(&tft);

    // Sprites fuer Status-/Boot-Text (DRAW_ON_BOTH_DISPLAYS(), siehe config.h),
    // NUR benoetigt beim GC9D01-Software-Rotations-Workaround (gc9d01SwRotation) -
    // dort wird die Hardware-Rotation der Chips bewusst uebersprungen, daher muss
    // Text dort stattdessen in ein Sprite mit eigener sprite.setRotation() gemalt
    // werden, um pro Display korrekt gedreht zu erscheinen (siehe beginStatusDraw()/
    // endStatusDraw() in display.h). Auf allen anderen Boards ungenutzt (dort malt
    // DRAW_ON_BOTH_DISPLAYS direkt auf 'tft' - das MADCTL-Register des jeweils
    // selektierten Chips dreht automatisch mit). Werden erst bei Bedarf angelegt
    // (siehe statusSprite1Created/statusSprite2Created), um auf Boards ohne
    // Software-Rotation keinen (P)SRAM zu verschwenden.

    // Sprites for status/boot text (DRAW_ON_BOTH_DISPLAYS(), see config.h), ONLY
    // needed with the GC9D01 software rotation workaround (gc9d01SwRotation) -
    // there, the chips' hardware rotation is deliberately skipped, so text must
    // instead be drawn into a sprite with its own sprite.setRotation() to appear
    // correctly rotated per display (see beginStatusDraw()/endStatusDraw() in
    // display.h). Unused on every other board (there DRAW_ON_BOTH_DISPLAYS draws
    // straight to 'tft' - the MADCTL register of whichever chip is currently
    // selected rotates it automatically). Created lazily on first use (see
    // statusSprite1Created/statusSprite2Created) to avoid wasting (P)SRAM on
    // boards without the software rotation workaround.
    TFT_eSprite statusSprite1 = TFT_eSprite(&tft);
    TFT_eSprite statusSprite2 = TFT_eSprite(&tft);
    bool statusSprite1Created = false;
    bool statusSprite2Created = false;

    String selectedBackground = "/face_default.bmp";

    bool stationMode;
    bool smoothMinute;
    bool showSecondHand;

    int hourHandWidth = HAND_WIDTH;
    int minuteHandWidth = HAND_WIDTH;
    int secondHandWidth = HAND_WIDTH;

    // nabe
    // hub
    uint16_t hubColor = 0;
    uint8_t hubSize = 0;

    bool firstRun = true;
    bool firstRun2 = true; // wie firstRun, aber fuer Display 2 (CS2) - siehe renderClockFrame() in display.h
                           // like firstRun, but for Display 2 (CS2) - see renderClockFrame() in display.h

    uint8_t tftRotation1 = 0;
    uint8_t tftRotation2 = 0; // Rotation von Display 2 (CS2) - eigener Wert, damit beide Displays
                              // unterschiedlich ausgerichtet montiert sein koennen (siehe uhr3.ino/webserver_routes.h)
                              // rotation of Display 2 (CS2) - its own value, so both displays can be
                              // mounted with a different orientation (see uhr3.ino/webserver_routes.h)


    uint16_t rowBuffer[CLOCK_WIDTH];

    // Nicht "ist PSRAM vorhanden" allgemein (dafuer wird ueberall direkt
    // psramFound() aufgerufen) - steuert ausschliesslich den GC9D01-
    // Software-Rotations-Workaround (siehe uhr3.ino und rotatedAngle() /
    // loadClockFace() in display.h). Fuer alle anderen Boards fest false.

    // Not "is PSRAM available" in general (psramFound() is called directly
    // everywhere for that) - controls exclusively the GC9D01 software
    // rotation workaround (see uhr3.ino and rotatedAngle() / loadClockFace()
    // in display.h). Hard-set to false for all other boards.
    static bool gc9d01SwRotation = false;

    uint16_t* clockFaceBuffer = nullptr;

    // --- Zwischenbild fuer Stunden- und Minutenzeiger -------------------------
    //
    // Haelt je Display ein fertiges "gedrehtes Zifferblatt + Stundenzeiger +
    // Minutenzeiger". Grund: Stunden- und Minutenzeiger bewegen sich extrem
    // langsam (Stundenzeiger 0,008 Grad/s, Minutenzeiger 0,1 Grad/s bzw. ein
    // Sprung pro Minute), wurden aber bisher in JEDEM Tick neu rotiert - genau
    // wie das Zifferblatt selbst, das bei Software-Rotation pro Tick pixelweise
    // neu gedreht wurde. Jetzt wird dieses Bild nur noch aufgebaut, wenn sich
    // wirklich etwas geaendert hat, und pro Tick nur noch kopiert. Dadurch
    // bleibt Rechenzeit fuer den schleichenden Sekundenzeiger uebrig UND es
    // wird gleichzeitig moeglich, die beiden langsamen Zeiger beim Aufbau in
    // deutlich hoeherer Qualitaet zu zeichnen (kantengeglaettet, siehe
    // blitHandAntiAliased() in display.h) statt mit dem harten
    // Nearest-Neighbour von pushRotated().

    // --- Composite image for the hour and minute hands ------------------------
    //
    // Holds, per display, a finished "rotated clock face + hour hand + minute
    // hand". Reason: the hour and minute hands move extremely slowly (hour hand
    // 0.008 deg/s, minute hand 0.1 deg/s or one jump per minute), yet they were
    // rotated anew on EVERY tick - just like the clock face itself, which with
    // software rotation was re-rotated pixel by pixel every tick. This image is
    // now only rebuilt when something actually changed, and merely copied per
    // tick. That leaves compute time for the sweeping second hand AND at the
    // same time makes it possible to draw the two slow hands at much higher
    // quality during the rebuild (anti-aliased, see blitHandAntiAliased() in
    // display.h) instead of pushRotated()'s hard nearest-neighbour sampling.
    struct HandComposite {
        uint16_t* buffer = nullptr;
        bool valid = false;
        float hourAngle = 0.0f;
        float minuteAngle = 0.0f;
        uint8_t rotation = 0xFF;
        uint8_t brightness = 0xFF;
        uint32_t assetGeneration = 0xFFFFFFFF;
        bool allocationFailed = false; // nach einem Fehlschlag nicht bei jedem Tick erneut versuchen
                                       // don't retry on every tick after a failure
    };
    HandComposite handComposite[2]; // [0] = Display 1, [1] = Display 2

    // Wird hochgezaehlt, sobald sich Zifferblatt, Zeigersatz oder Zeigerbreiten
    // aendern - macht jedes Zwischenbild ungueltig, ohne dass jede einzelne
    // Aenderungsstelle das Zwischenbild selbst kennen muss.

    // Incremented whenever the clock face, hand set or hand widths change -
    // invalidates every composite image without each individual change site
    // having to know about the composite itself.
    uint32_t clockAssetGeneration = 1;

    // Arbeitskopie der Zeigerpixel fuer den kantengeglaetteten Aufbau (die
    // Sprite-eigenen readPixel()-Aufrufe waeren pro Subsample zu teuer).
    // Working copy of the hand pixels for the anti-aliased rebuild (the sprite's
    // own readPixel() calls would be too expensive per subsample).
    uint16_t* handPixelScratch = nullptr;

    // Cache: clockFaceBuffer bereits mit currentBrightness vorberechnet (siehe
    // loadClockFace()) - vermeidet die teure Pixel-Helligkeitsanpassung bei
    // jedem Tick, obwohl sich die Helligkeit dazwischen fast nie aendert.

    // Cache: clockFaceBuffer is pre-adjusted for currentBrightness (see
    // loadClockFace()) - avoids the expensive per-pixel brightness adjustment
    // on every tick, even though brightness rarely changes in between.
    uint16_t* clockFaceBrightBuffer = nullptr;

    // Display 2, baugleich mit Display 1, am CS2-Pin (siehe config.h) - fest
    // aktiviert, kein Preferences-/UI-Schalter mehr (frueher useCS2/PK_USE_CS2).

    // Display 2, identical to Display 1, on the CS2 pin (see config.h) -
    // permanently enabled, no more preferences/UI toggle (formerly
    // useCS2/PK_USE_CS2).

    // --- Helligkeit / Fotowiderstand (ADC) ---
    // --- Brightness / photoresistor (ADC) ---
    bool adcInverted = false; // Standardmäßig nicht invertiert
                              // Not inverted by default

    bool useAdc = false;
    bool photoresistorFound = false;

    uint8_t currentBrightness = 255;
    uint8_t lastAppliedBrightness = 255; // gehoert zum Zifferblatt-Cache und wird von loadClockFace() gepflegt
                                         // belongs to the clock face cache and is maintained by loadClockFace()

    // Eigener Vergleichswert fuer die Zeiger-Sprites (siehe updateBrightness() in
    // display.h): darf NICHT lastAppliedBrightness mitbenutzen, weil
    // loadClockFace() diesen Wert bereits selbst aktualisiert und die
    // Zeiger-Neueinfaerbung dadurch nie ausgeloest wurde.

    // Its own comparison value for the hand sprites (see updateBrightness() in
    // display.h): must NOT share lastAppliedBrightness, because loadClockFace()
    // already updates that value itself, which meant the hands were never
    // re-tinted.
    uint8_t lastHandBrightness = 255;
    uint8_t targetBrightness = 255;
    int lowThreshold = 40;
    int highThreshold = 60;
    uint8_t minBrightness = 100;  //
    uint8_t maxBrightness = 255;  // Obergrenze
                                  // Upper limit

    // Zeitabhängige Helligkeit
    // Time-dependent brightness
    uint8_t brightStartHour = 8;       // inkl. (z.B. 8)
                                       // inclusive (e.g. 8)
    uint8_t brightEndHour = 22;        // exkl. (z.B. 20)
                                       // exclusive (e.g. 20)

#if defined (GC9D01)  || defined(GC9A01_WITH_BACKLIGHT)
    float gammaBrightness = 2.2f;  // Gamma-Korrektur für Helligkeit
                                   // Gamma correction for brightness
#endif

#define ADC_SMOOTHING 20
    int adcHistory[ADC_SMOOTHING];
    int adcIndex = 0;
    int currentAdcAvg = 0;  // global definieren
                            // define globally
    int currentLightPercent = 0;  // global speichern für Anzeige
                                  // store globally for display

    // --- Touch ---
    // --- Touch ---
    // --- Touch / Debounce State ---
    // --- Touch / debounce state ---
    unsigned long touchLastMillis = 0;
    const unsigned long TOUCH_DEBOUNCE_MS = 300;
    bool touchLastState = false;
    // --- Touch enable flag: aktivieren erst nach Setup-Initialisierung ---
    // --- Touch enable flag: only enabled after setup initialization ---
    bool touchEnabled = false;
    unsigned long touchEnableAt = 0; // Timestamp wann Touch freigeschaltet wird (ms)
                                     // Timestamp when touch is enabled (ms)
    bool useTouch = false; // Touch verwenden
                           // Use touch

    // --- Presets ---
    // --- Presets ---
    // Presets
    // Presets
#define MAX_PRESETS 50
    struct Preset {
        String name;
        String url;
    };
    Preset presets[MAX_PRESETS];

    // --- Datei-Upload / Wartung ---
    // --- File upload / maintenance ---
    File uploadFile;
    String uploadFilePath = "";
    bool uploadSuccess = false;

    // --- Presets-Import (separat vom BMP-Upload oben, um Statuskonflikte zu vermeiden) ---
    // --- Presets import (separate from the BMP upload above, to avoid status conflicts) ---
    File presetImportFile;
    bool presetImportSuccess = false;
    const char* PRESET_IMPORT_TMP_PATH = "/tmp_presets_import.txt";

    int lastResetWeek = -1;
    int currentWeek = -1;

    //Übersetzungen fÜr verschiedene Sprachen
    // Translations for various languages
#include "translation.h"
