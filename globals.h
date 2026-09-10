#pragma once
    // Globale Objekte/Variablen, nach Modul sortiert. Echte Definitionen statt
    // extern, da nur von uhr3.ino eingebunden (eine Uebersetzungseinheit).

    // Global objects/variables, sorted by module. Real definitions instead of
    // extern, since only included from uhr3.ino (one translation unit).

    // System / Allgemein
    // System / General
    String currentLanguage = "de"; // Standardmäßig Deutsch
                                   // Default: German

    char version[20]; // Build-Version ("YYYY-MM-DD HH:MM:SS" = 19 Zeichen + Nullterminator)
                      // Build version ("YYYY-MM-DD HH:MM:SS" = 19 chars + null terminator)

    bool loggingEnabled = false;

    bool initial = true;

    String ipAddress = "";


    // Kern-Hardwareobjekte (TFT, Webserver, Preferences, RTC, ...)
    // Core hardware objects (TFT, web server, preferences, RTC, ...)
    TFT_eSPI tft = TFT_eSPI();
    WebServer webserver(80);
    Preferences preferences;
    DNSServer dnsServer;
    WiFiUDP udp;

#if defined SDA_PIN && defined SCL_PIN
    RTC_DS3231 rtc;
#endif

    // WLAN
    // WiFi
#define MAX_WLAN 15
    String wifiSsid[MAX_WLAN];
    String wifiPass[MAX_WLAN];

#define NOT_CONNECTED 0
#define CONNECTED 1
#define CONNECTED_NO_INTERNET 2

    bool wifiActive = true;

    // Zustand fuer eine per Web-Button ausgeloeste WPS-Anfrage (siehe loop() in
    // uhr3.ino, /api/startWPS in webserver_routes.h) - laeuft asynchron und
    // event-basiert ueber WiFi.onEvent(), um den Webserver nicht zu blockieren.

    // State for a web-button-triggered WPS request (see loop() in uhr3.ino,
    // /api/startWPS in webserver_routes.h) - runs asynchronously and event-based
    // via WiFi.onEvent(), so the web server isn't blocked.
    bool wpsPending = false;
    unsigned long wpsStartMillis = 0;
    String wpsPreviousSsid = ""; // Verbindung vor dem WPS-Start, um danach ggf. dorthin zurueckzuwechseln
                                 // Connection before the WPS start, to switch back to it afterward if needed
    volatile bool wpsSuccessEvent = false; // wird im WiFi-Event-Callback gesetzt (anderer Kontext!)
                                           // set in the WiFi event callback (different context!)
    volatile bool wpsFailedEvent = false;

    // Verzoegerter WPS-Start (siehe /api/startWPS in webserver_routes.h und
    // loop() in uhr3.ino): der HTTP-Handler darf NICHT per delay() blockieren,
    // um selbst auf startWPS() zu warten - waehrend eines blockierenden
    // delay() kann der Webserver keine weitere Anfrage annehmen, also auch
    // nicht die Folge-GET-Anfrage des Browsers fuer die per Redirect
    // aufgerufene Zielseite (mit dem Trennungs-Banner). startWPS() wuerde die
    // Verbindung dann stoeren, WAEHREND genau diese Seite noch laedt. Deshalb
    // merkt sich der Handler nur den Wunsch + Zeitpunkt, loop() started WPS
    // stattdessen zeitgesteuert (unblockierend, per millis()) etwas spaeter -
    // die Zielseite mit dem Banner ist dann laengst ausgeliefert.

    // Deferred WPS start (see /api/startWPS in webserver_routes.h and loop()
    // in uhr3.ino): the HTTP handler must NOT block with delay() to wait out
    // startWPS() itself - while such a delay() blocks, the web server cannot
    // accept another request either, including the browser's follow-up GET
    // for the redirect's target page (with the disconnect banner). startWPS()
    // would then disturb the connection WHILE that very page is still
    // loading. So the handler only records the request + timestamp, and
    // loop() starts WPS instead on a timer (non-blocking, via millis()) a
    // little later - by then the target page with the banner has long since
    // been delivered.
    bool wpsStartRequested = false;
    unsigned long wpsStartRequestedAtMillis = 0;

    // MAC Adresse
    // MAC address
    uint8_t mac[6];
    char hostname[32];

    // Zur Laufzeit erzeugtes AP-Passwort (aus MAC-Bytes, siehe startAP() in
    // wifi_manager.h) - als Puffer, damit Display und Web-UI-Statuszeile es
    // beide anzeigen koennen. Leer, solange der AP nie lief.

    // AP password generated at runtime (from MAC bytes, see startAP() in
    // wifi_manager.h) - as a buffer so both the display and web UI status line
    // can show it. Empty as long as the AP has never run.
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

    // Zeit, NTP, DCF77, RTC
    // Time, NTP, DCF77, RTC
    // NTP-Server-Port
    // NTP server port
    const int NTP_PORT = 123;
    // NTP-Paketgröße
    // NTP packet size
    const int NTP_PACKET_SIZE = 48;
    byte ntpPacket[NTP_PACKET_SIZE];

#if defined DCF77_DATAPIN && defined DCF77_INTERRUPT

    // Interrupt liegt auf CHANGE (siehe attachInterrupt() in uhr3.ino);
    // processDcf77Bits() (time_sync.h) klassifiziert nur ueber die DAUER
    // zwischen zwei Flanken - der Pegel wird nie gelesen, daher keine
    // Flankenrichtung noetig.

    // Interrupt is on CHANGE (see attachInterrupt() in uhr3.ino);
    // processDcf77Bits() (time_sync.h) classifies purely by the DURATION
    // between two edges - the level is never read, so no edge direction
    // is needed.

    volatile uint16_t dcf77Count = 0; // Anzahl der empfangenen DCF77-Signale (wird in der ISR verändert)
                                      // Number of received DCF77 signals (modified in the ISR)

    volatile bool dcfTimeFound = false; // wird in loop()/updateDcf77Status() gesetzt, nicht mehr in der ISR gelesen
                                        // set in loop()/updateDcf77Status(), no longer read in the ISR

    // Wird in processDcf77Bits() gesetzt, nicht in der ISR: setLedOn/Off()
    // liegen im Flash (Panic-Reset-Risiko bei deaktiviertem Flash-Cache, z.B.
    // waehrend LittleFS-Schreibvorgaengen), und nur processDcf77Bits() weiss
    // ueber dcf77Confirmed, ob wirklich DCF77 erkannt wurde. Siehe isr() in time_sync.h.

    // Set in processDcf77Bits(), not the ISR: setLedOn/Off() live in flash
    // (panic-reset risk while the flash cache is disabled, e.g. during
    // LittleFS writes), and only processDcf77Bits() knows via dcf77Confirmed
    // whether DCF77 was actually recognized. See isr() in time_sync.h.
    volatile bool dcfLedTogglePending = false;

    // Zeitpunkt (millis()), zu dem der aktuelle Einmal-Blitz der LED wieder
    // ausgeschaltet werden soll; 0 = kein Blitz aktiv (siehe
    // DCF77_LED_BLINK_MS in config.h und die Abarbeitung in loop()).

    // Time (millis()) at which the LED's current one-shot flash is to be
    // switched off again; 0 = no flash active (see DCF77_LED_BLINK_MS in
    // config.h and the handling in loop()).
    unsigned long dcfLedOffAtMillis = 0;

    bool dcfSyncLedEnabled = true; // per Checkbox abschaltbar (Default: an) - steuert nur, ob loop() das per
                                    // dcfLedTogglePending angeforderte Blinken ausfuehrt (siehe PK_DCF_SYNC_LED)
                                    // switchable via checkbox (default: on) - only controls whether loop() executes
                                    // the blink requested via dcfLedTogglePending (see PK_DCF_SYNC_LED)

    time_t lastDcfSyncTime = 0; // Unix-Zeitstempel der letzten erfolgreichen DCF77-Synchronisation (0 = noch nie)
                                // Unix timestamp of the last successful DCF77 sync (0 = never)

    unsigned long lastDcf77PulseChangeMillis = 0; // millis()-Zeitpunkt der letzten beobachteten Aenderung von dcf77Count (siehe checkDcf77Health() in time_sync.h) -
                                                   // erkennt einen kompletten Empfangsausfall waehrend des Betriebs, da dcf77Count selbst nie auf 0 zurueckgesetzt wird
                                                   // millis() timestamp of the last observed change in dcf77Count (see checkDcf77Health() in time_sync.h) -
                                                   // detects a complete reception failure during operation, since dcf77Count itself is never reset to 0

    uint8_t dcf77PlausiblePulseStreak = 0; // Zahl aufeinanderfolgender dcf77Count-Aenderungen mit plausiblem Abstand
                                            // (siehe DCF77_PRESENCE_MAX_GAP_MS, checkDcf77Health()) - filtert Rauschen
                                            // number of consecutive dcf77Count changes with a plausible gap (see
                                            // DCF77_PRESENCE_MAX_GAP_MS, checkDcf77Health()) - filters out noise

    bool dcf77Confirmed = false; // wird einmalig true bei DCF77_PRESENCE_MIN_STREAK (config.h) - bestimmt, ob der
                                 // DCF77-Eintrag in der Topbar angezeigt wird; danach nie zurueckgesetzt
                                 // becomes true exactly once at DCF77_PRESENCE_MIN_STREAK (config.h) - decides
                                 // whether the DCF77 topbar entry is shown; never reset afterwards

    // Eigener Bit-Fortschritt statt dcf.getUTCTime() der DCF77-Bibliothek -
    // treibt Live-Anzeige (/dcf77) und Zeituebernahme (dcf77LastDecoded). ISR
    // schreibt nur Zeitstempel in den Ringpuffer (RAM); Dekodierung passiert
    // nur in processDcf77Bits()/decodeDcf77Telegram(), aufgerufen aus loop().

    // Own bit progress instead of the library's dcf.getUTCTime() - drives the
    // live display (/dcf77) and time takeover (dcf77LastDecoded). ISR only
    // writes a timestamp to the ring buffer (RAM); decoding happens only in
    // processDcf77Bits()/decodeDcf77Telegram(), called from loop().

    // Auf 64 vergroessert (256 Byte RAM): muss die laengste Pause zwischen
    // zwei processDcf77Bits()-Aufrufen ueberbruecken (2 Flanken/s) - 16
    // Plaetze (~8s) reichten nicht gegen blockierende NTP-/Web-/LittleFS-Vorgaenge.

    // Enlarged to 64 (256 bytes RAM): must bridge the longest pause between
    // two processDcf77Bits() calls (2 edges/s) - 16 slots (~8s) weren't
    // enough against blocking NTP/web/LittleFS operations.
#define DCF77_EDGE_BUFFER_SIZE 64
    volatile unsigned long dcf77EdgeMillis[DCF77_EDGE_BUFFER_SIZE];
    // dcf77EdgeLevel[] entfernt: Pegel wurde nie ausgewertet (Klassifizierung
    // laeuft ueber die Flankendauer), digitalRead() lag aber im Flash - Absturzrisiko in der ISR.
    // dcf77EdgeLevel[] removed: level was never evaluated (classification
    // works via edge duration), but digitalRead() lived in flash - crash risk in the ISR.
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

    // dcf77Bits ist nach Rasterposition (dcf77Phase, 0..59) indiziert, nicht
    // nach Sekunde - die Zuordnung Position->Sekunde steht erst nach Fund der
    // Minutenmarke fest, sonst bliebe die /dcf77-Seite bis dahin leer.

    // dcf77Bits is indexed by grid position (dcf77Phase, 0..59), not by
    // second - the mapping position->second is only fixed once the minute
    // marker is found, otherwise the /dcf77 page would stay empty until then.
#define DCF77_GRID_SLOTS 60
    int8_t dcf77Bits[DCF77_GRID_SLOTS]; // 0/1 je Rasterposition der laufenden Minute, -1 = (noch) unbekannt - NUR Hauptthread
                                        // 0/1 per grid position of the running minute, -1 = unknown (yet) - main thread only
    uint8_t dcf77BitIndex = 0; // Position (Sekunde) des naechsten erwarteten Impulses - bei einer verlorenen Sekunde
                               // bleibt dort eine Luecke (-1), der Index zeigt trotzdem korrekt weiter (siehe processDcf77Bits())
                               // position (second) of the next expected pulse - a lost second leaves a hole (-1)
                               // there, the index still points correctly onward (see processDcf77Bits())

    bool dcf77Synced = false;  // true, sobald Minutenmarke erkannt und seither jeder Abstand ins Sekundenraster passte;
                               // Impulse werden nur dann als Bits abgelegt. Faellt bei Rasterbruch oder unmoeglichem
                               // Telegramm (Bit 0/20, siehe decodeDcf77Telegram()) wieder auf false zurueck.

                               // true once the minute marker is detected and every distance since fit the second
                               // grid; pulses are only then stored as bits. Falls back to false on a grid break or
                               // an impossible telegram (bit 0/20, see decodeDcf77Telegram()).

    // dcf77Phase ist eine freilaufende Rasterposition 0..59 ohne Sekundenbezug.
    // Die Minutenmarke wird ueber Statistik erkannt (einzige Position, an der
    // IMMER ein Impuls fehlt), nicht ueber einen einzelnen Abstand - robust
    // auch bei Stoerungen mit haeufigen 2s-Abstaenden.

    // dcf77Phase is a free-running grid position 0..59 with no relation to the
    // real second. The minute marker is detected via statistics (the only
    // position where a pulse is ALWAYS missing), not a single pulse distance -
    // robust even with interference causing frequent 2s gaps.
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

    // Diagnosewerte fuer die /dcf77-Seite: machen sichtbar, was der Empfaenger
    // tatsaechlich liefert - ohne Oszilloskop sonst nicht zu unterscheiden, ob
    // der Dekoder falsch rechnet oder keine brauchbaren Impulse ankommen.
    // Diagnostic values for the /dcf77 page: make visible what the receiver
    // actually delivers - without an oscilloscope there's no other way to
    // tell whether the decoder computes wrongly or no usable pulses arrive.
    uint32_t dcf77PulsesSeen = 0;    // erkannte Impulse seit dem Start / pulses detected since start
    uint32_t dcf77PulsesMissed = 0;  // uebersprungene Rasterpositionen / grid positions skipped
    uint32_t dcf77PhaseBreaks = 0;   // wie oft das Sekundenraster verlorenging / how often the second grid was lost

#define DCF77_DIAG_SLOTS 12
    uint16_t dcf77DiagWidth[DCF77_DIAG_SLOTS] = { 0 }; // Impulsdauer in ms / pulse width in ms
    uint16_t dcf77DiagGap[DCF77_DIAG_SLOTS] = { 0 };   // Abstand zum vorigen Impulsanfang in ms / distance to the previous pulse start in ms
    uint8_t dcf77DiagIdx = 0;
    uint8_t dcf77DiagCount = 0;

    // Ergebnis der letzten vollstaendig dekodierten Minute - bleibt bei
    // Paritaetsfehler erhalten (valid=false statt verwerfen), damit die
    // Live-Seite auch fehlerhaften Empfang zeigen kann.

    // Result of the last fully decoded minute - kept on a parity error
    // (valid=false instead of discarding it), so the live page can also
    // show faulty reception.
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

    // Letzte bestaetigte Dekodierung als Referenz fuer die Kohaerenzpruefung
    // rekonstruierter Telegramme (siehe decodeDcf77Telegram()): bei aus
    // Paritaet ergaenzten Bits muss die Zeit exakt zur vorherigen plus
    // verstrichenen Minuten passen - ein vollstaendiges Telegramm braucht das nicht.

    // Last confirmed decoding, used as the reference for the coherence check
    // of reconstructed telegrams (see decodeDcf77Telegram()): with bits filled
    // from parity, the time must exactly match the previous one plus elapsed
    // minutes - a fully received telegram doesn't need this.
    time_t dcf77PrevEpoch = 0;
    unsigned long dcf77PrevAtMillis = 0;

#endif

    // Ist der eigene NTP-Server (siehe startNtpServer() in time_sync.h) an
    // Port 123 gebunden - fuer korrektes Logging/Statusanzeige, statt den
    // Erfolg von udp.begin() blind anzunehmen.

    // Is the own NTP server (see startNtpServer() in time_sync.h) bound to
    // port 123 - for correct logging/status display, instead of blindly
    // assuming udp.begin() succeeded.
    bool ntpServerRunning = false;

    // Diagnosezaehler fuer den eigenen NTP-Server (siehe /status): unterscheiden,
    // ob Anfragen ankommen (Netz/Firewall) oder nur unbeantwortet bleiben
    // (keine gueltige Systemzeit).

    // Diagnostic counters for the own NTP server (see /status): distinguish
    // whether requests arrive at all (network/firewall) or just go unanswered
    // (no valid system time).
    uint32_t ntpRequestsReceived = 0;
    uint32_t ntpRepliesSent = 0;

    unsigned long lastNTPUpdate = 0; // Zeitpunkt des letzten RTC-Updates
                                     // Timestamp of the last RTC update
    unsigned long lastDCFUpdate = 0; // Wartezeit nach DCF77-Update, bevor RTC aktualisiert wird (ms)
                                     // Wait time after a DCF77 update before the RTC is updated (ms)
    unsigned long lastRTCUpdate = 0; // Zeitpunkt des letzten RTC-Updates
                                     // Timestamp of the last RTC update
    unsigned long lastNtpSuccessMillis = 0; // Zeitpunkt (millis()) der letzten erfolgreichen NTP-Sync (0 = nie) -
                                            // unterscheidet echten Erfolg vom irrefuehrenden true-Rueckgabewert von
                                            // setupNTP() ohne WLAN; bleibt er unveraendert, springt DCF77 ein (time_sync.h)

    // Timestamp (millis()) of the last successful NTP sync (0 = never) -
    // distinguishes a real success from setupNTP()'s misleading true return
    // value without WiFi; if unchanged, DCF77 steps in (time_sync.h)

    struct tm timeinfo;

    // Rocrail-Modellzeit (siehe rocrail_client.h): eigenstaendige, ggf.
    // gegenueber der echten Zeit beschleunigte Zeitstruktur - komplett
    // getrennt von "timeinfo" oben, das weiterhin die echte Systemzeit fuer
    // Log/NTP-Server/woechentlichen Neustart traegt. renderClockFrame() liest
    // je nach rocrailEnabled aus der einen oder der anderen Struktur.

    // Rocrail model time (see rocrail_client.h): an independent time struct,
    // possibly running faster than real time - fully separate from
    // "timeinfo" above, which keeps carrying the real system time for
    // logging/NTP server/weekly restart. renderClockFrame() reads from
    // whichever struct applies, depending on rocrailEnabled.
    struct tm rocrailTimeinfo;
    bool rocrailEnabled = false;    // per Tab-Schalter/Preferences aktiviert
                                    // enabled via the tab switch/preferences
    bool rocrailConnected = false;  // TCP-Verbindung zum Server aktuell offen
                                    // TCP connection to the server currently open
    String rocrailServerHost = "";  // aktuell aktiver Server - siehe rocrailServerList[] unten
                                    // fuer die vollstaendige Liste moeglicher Server (bis zu
                                    // MAX_WLAN); wird beim Speichern in /save_rocrail aus dem
                                    // per Haekchen ausgewaehlten Listeneintrag uebernommen.
                                    // currently active server - see rocrailServerList[] below
                                    // for the full list of possible servers (up to MAX_WLAN);
                                    // taken over from the entry selected via the checkmark
                                    // when saving in /save_rocrail.
    uint16_t rocrailServerPort = ROCRAIL_DEFAULT_PORT;
    char rocrailServerList[MAX_WLAN][64];        // Hostname/IP je Listenplatz (siehe pkRocrailServerHost())
                                                 // hostname/IP per list slot (see pkRocrailServerHost())
    uint16_t rocrailServerPortList[MAX_WLAN];    // Port je Listenplatz (siehe pkRocrailServerPort())
                                                 // port per list slot (see pkRocrailServerPort())

    // Anlagenname je Listenplatz - vom Nutzer frei editierbar (siehe
    // panel-rocrail in webserver_routes.h). Ist das Feld fuer den gerade
    // aktiven Server leer, uebernimmt processRocrailPlanTag()
    // (rocrail_client.h) einmalig den vom Server per <plan title="..."/>
    // gemeldeten Namen; ist es NICHT leer (vom Nutzer gesetzt oder schon
    // einmal per RCP befuellt), wird es von dort an nicht mehr angetastet -
    // erst ein manuelles Leeren des Felds durch den Nutzer schaltet die
    // automatische Uebernahme fuer diesen Listenplatz wieder frei.

    // Layout name per list slot - freely editable by the user (see
    // panel-rocrail in webserver_routes.h). If the field for the currently
    // active server is empty, processRocrailPlanTag() (rocrail_client.h)
    // takes over the name reported by the server via <plan title="..."/>
    // once; if it is NOT empty (set by the user, or already filled in once
    // via RCP), it is left untouched from then on - only the user manually
    // clearing the field re-enables automatic takeover for that list slot.
    char rocrailServerNameList[MAX_WLAN][40];

    int rocrailActiveServerIndex = -1;           // 0-basierter Index des per Haekchen ausgewaehlten
                                                 // Listenplatzes, -1 = keiner ausgewaehlt
                                                 // 0-based index of the list slot selected via the
                                                 // checkmark, -1 = none selected
    uint8_t rocrailDivider = 1;     // Modellzeit-Beschleunigungsfaktor vom Server
                                    // model-time acceleration factor from the server
    bool rocrailFrozen = false;     // per <clock state="freeze"/> angehalten (siehe processRocrailClockPayload())
                                    // paused via <clock state="freeze"/> (see processRocrailClockPayload())
    uint8_t rocrailBrightness = 255;    // zuletzt vom Server per <clock bri="..."/> gemeldeter
                                        // Helligkeitswert (0-255, siehe processRocrailClockPayload())
    bool rocrailBrightnessKnown = false; // true, sobald mindestens einmal ein bri-Wert empfangen
                                         // wurde - erst dann uebernimmt updateBrightness() ihn
                                         // (siehe dort); manche Rocrail-Installationen senden gar
                                         // kein bri (keine Lichtsteuerung konfiguriert), dann bleibt
                                         // die lokale Helligkeitssteuerung dauerhaft aktiv

                                        // last brightness value reported by the server via
                                        // <clock bri="..."/> (0-255, see processRocrailClockPayload())
                                        // true once at least one bri value has been received - only
                                        // then does updateBrightness() take it over (see there); some
                                        // Rocrail setups never send bri at all (no lighting control
                                        // configured), in which case the local brightness control
                                        // stays active permanently
    WiFiClient rocrailClient;
    String rocrailRxBuffer;         // Empfangspuffer fuer XML-Tag-Bruchstuecke
                                    // receive buffer for XML tag fragments
    uint8_t rocrailLogChunkCount = 0; // Zaehler fuer die Diagnose-Logeintraege der ersten
                                      // 5 empfangenen RCP-Rohdaten-Haeppchen (siehe pollRocrailClient())
                                      // counter for the diagnostic log entries of the first 5
                                      // received RCP raw data chunks (see pollRocrailClient())
    unsigned long rocrailLastClockMillis = 0;  // millis() beim letzten <clock>-Update (0 = noch keins)
                                               // millis() at the last <clock> update (0 = none yet)
    float rocrailDisplaySeconds = 0.0f;        // aktuell angezeigte Modellzeit in Sekunden seit
                                               // Mitternacht (fliessend, siehe advanceRocrailTime())
                                               // currently displayed model time in seconds since
                                               // midnight (continuous, see advanceRocrailTime())
    unsigned long rocrailLastAdvanceMillis = 0; // millis() beim letzten Fortschreiben (advanceRocrailTime())
                                                // millis() at the last advance step (advanceRocrailTime())
    float rocrailDriftSeconds = 0.0f;          // verbleibende, sanft auszugleichende Abweichung zur
                                               // zuletzt gemeldeten Server-Zeit (siehe advanceRocrailTime()/
                                               // processRocrailClockPayload()) - 0 = keine Korrektur noetig
                                               // remaining deviation to the last reported server time,
                                               // eased in smoothly (see advanceRocrailTime()/
                                               // processRocrailClockPayload()) - 0 = no correction needed
    float rocrailSecFrac = 0.0f;               // Sekunde mit Nachkommastellen (0.0-59.999) fuer die
                                               // glatte, nicht tickende Zeigerbewegung im Rocrail-
                                               // Modus (siehe advanceRocrailTime()/renderClockFrame())
                                               // second with a fractional part (0.0-59.999) for the
                                               // smooth, non-ticking hand motion in Rocrail mode (see
                                               // advanceRocrailTime()/renderClockFrame())
    unsigned long rocrailLastConnectAttemptMillis = 0;
    TaskHandle_t rocrailConnectTaskHandle = NULL; // eigene, kurzlebige Task fuer den
                                                  // (blockierenden) Connect-Versuch, damit
                                                  // loop() dabei nicht blockiert (siehe rocrail_client.h)
                                                  // own short-lived task for the (blocking) connect
                                                  // attempt, so loop() doesn't block during it (see rocrail_client.h)
    volatile bool rocrailConnectTaskRunning = false; // Task laeuft gerade
                                                     // task is currently running
    volatile bool rocrailConnectTaskDone = false;    // Task fertig, Ergebnis in rocrailConnectTaskResult
                                                     // task finished, result in rocrailConnectTaskResult
    volatile bool rocrailConnectTaskResult = false;  // Ergebnis des letzten Connect-Versuchs
                                                     // result of the last connect attempt

    String timezone = TIMEZONE_DEFAULT;

    char ntpServers[MAX_WLAN][64];

#define RTC_NOT_AVAILABLE 0
#define RTC_AVAILABLE 1
#define RTC_AVAILABLE_BUT_INVALID 2

    int rtcOk = RTC_NOT_AVAILABLE;

    String i2cAddr = "";

    // Zifferblatt / Display
    // Clock face / Display
    String tftType = "UNKNOWN";

    TFT_eSprite backgroundSprite = TFT_eSprite(&tft);
    TFT_eSprite hourHandSprite = TFT_eSprite(&tft);
    TFT_eSprite minuteHandSprite = TFT_eSprite(&tft);
    TFT_eSprite secondHandSprite = TFT_eSprite(&tft);

    // Sprites fuer Status-/Boot-Text, nur fuer den GC9D01-Software-Rotations-
    // Workaround noetig (dort wird die HW-Rotation uebersprungen) - auf
    // anderen Boards ungenutzt. Werden erst bei Bedarf angelegt (kein (P)SRAM-Verschwenden).

    // Sprites for status/boot text, only needed for the GC9D01 software
    // rotation workaround (there, HW rotation is skipped) - unused on other
    // boards. Created lazily on first use (avoids wasting (P)SRAM).
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

    // Steuert nur den GC9D01-Software-Rotations-Workaround (nicht "ist PSRAM
    // vorhanden" allgemein - dafuer wird ueberall psramFound() aufgerufen).
    // Auf allen anderen Boards fest false.

    // Controls only the GC9D01 software rotation workaround (not "is PSRAM
    // available" in general - psramFound() is called directly for that).
    // Hard-set to false on all other boards.
    static bool gc9d01SwRotation = false;

    uint16_t* clockFaceBuffer = nullptr;

    // Fertiges "Zifferblatt + Stunden-/Minutenzeiger" pro Display - beide
    // bewegen sich kaum, wurden aber bisher jeden Tick neu rotiert. Jetzt nur
    // bei Aenderung (kantengeglaettet) neu aufgebaut, sonst kopiert - spart Zeit fuer den Sekundenzeiger.

    // Finished "clock face + hour/minute hand" per display - both barely
    // move but were re-rotated every tick before. Now rebuilt (anti-aliased)
    // only on change, otherwise just copied - frees time for the second hand.
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

    // Display 2 (baugleich, am CS2-Pin, siehe config.h) ist fest aktiviert,
    // kein Preferences-/UI-Schalter.
    // Display 2 (identical, on the CS2 pin, see config.h) is permanently
    // enabled, no preferences/UI toggle.

    // Helligkeit / Fotowiderstand (ADC)
    // Brightness / photoresistor (ADC)
    bool adcInverted = false; // Standardmäßig nicht invertiert
                              // Not inverted by default

    bool useAdc = false;
    bool photoresistorFound = false;

    uint8_t currentBrightness = 255;
    uint8_t lastAppliedBrightness = 255; // gehoert zum Zifferblatt-Cache und wird von loadClockFace() gepflegt
                                         // belongs to the clock face cache and is maintained by loadClockFace()

    // Eigener Vergleichswert fuer die Zeiger-Sprites (siehe updateBrightness()) -
    // darf nicht lastAppliedBrightness teilen, sonst wuerde loadClockFace()
    // die Neueinfaerbung nie ausloesen.

    // Own comparison value for the hand sprites (see updateBrightness()) -
    // must not share lastAppliedBrightness, or loadClockFace() would never
    // trigger the re-tinting.
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

    // Touch / Debounce
    // Touch / debounce
    unsigned long touchLastMillis = 0;
    const unsigned long TOUCH_DEBOUNCE_MS = 300;
    bool touchLastState = false;
    // Touch-Freigabe erst nach Setup-Initialisierung
    // Touch enabled only after setup initialization
    bool touchEnabled = false;
    unsigned long touchEnableAt = 0; // Timestamp wann Touch freigeschaltet wird (ms)
                                     // Timestamp when touch is enabled (ms)
    bool useTouch = false; // Touch verwenden
                           // Use touch

    // Presets
    // Presets
#define MAX_PRESETS 50
    struct Preset {
        String name;
        String url;
    };
    Preset presets[MAX_PRESETS];

    // Datei-Upload / Wartung
    // File upload / maintenance
    File uploadFile;
    String uploadFilePath = "";
    bool uploadSuccess = false;

    // Presets-Import (separat vom BMP-Upload, um Statuskonflikte zu vermeiden)
    // Presets import (separate from the BMP upload, to avoid status conflicts)
    File presetImportFile;
    bool presetImportSuccess = false;
    const char* PRESET_IMPORT_TMP_PATH = "/tmp_presets_import.txt";

    int lastResetWeek = -1;
    int currentWeek = -1;

    // Uebersetzungen fuer verschiedene Sprachen
    // Translations for various languages
#include "translation.h"
