#pragma once
    // Globale Objekte/Variablen, nach Modul sortiert. Echte Definitionen statt
    // extern, da nur von uhr3.ino eingebunden (eine Uebersetzungseinheit).

    // Global objects/variables, sorted by module. Real definitions instead of
    // extern, since only included from uhr3.ino (one translation unit).

    // Fuer den Mutex des Log-Puffers (siehe logBufferMutex weiter unten) -
    // xTaskCreate() u.ae. kommen bereits transitiv rein, Semaphoren nicht
    // unbedingt, deshalb hier explizit.

    // For the log buffer's mutex (see logBufferMutex further below) -
    // xTaskCreate() etc. already come in transitively, semaphores not
    // necessarily, hence explicit here.
#include <freertos/semphr.h>

    // System / Allgemein
    // System / General
    String currentLanguage = "de"; // Standardmäßig Deutsch
                                   // Default: German

    char version[20]; // Build-Version ("YYYY-MM-DD HH:MM:SS" = 19 Zeichen + Nullterminator)
                      // Build version ("YYYY-MM-DD HH:MM:SS" = 19 chars + null terminator)

    bool loggingEnabled = false;

    // Log-Zeilen werden gesammelt statt bei jeder DEBUG_PRINTLN() sofort auf
    // Flash geschrieben (siehe logToFile()/flushLogBuffer() in
    // system_utils.h) - jeder Flash-Zugriff blockiert kurz die komplette
    // Ausfuehrung (auch eine laufende SPI-Uebertragung zum Display), waehrend
    // NTP-Sync fallen viele Log-Zeilen kurz hintereinander an. logBufferMutex
    // schuetzt logLineBuffer, da sowohl der Haupt-Loop als auch die NTP-/
    // Rocrail-Sync-Tasks (eigene FreeRTOS-Tasks, siehe time_sync.h/
    // rocrail_client.h) hineinschreiben.

    // Log lines are collected instead of being written to flash immediately
    // on every DEBUG_PRINTLN() call (see logToFile()/flushLogBuffer() in
    // system_utils.h) - each flash access briefly stalls ALL execution
    // (including any SPI transfer to the display in progress); many log
    // lines occur in quick succession during NTP sync. logBufferMutex
    // protects logLineBuffer, since both the main loop and the NTP/Rocrail
    // sync tasks (their own FreeRTOS tasks, see time_sync.h/
    // rocrail_client.h) write into it.
    String logLineBuffer = "";
    SemaphoreHandle_t logBufferMutex = nullptr; // erzeugt in setup() via xSemaphoreCreateMutex()
                                                // created in setup() via xSemaphoreCreateMutex()
    unsigned long lastLogFlushMillis = 0;
#define LOG_FLUSH_INTERVAL_MS 3000 // spaetestens nach dieser Zeit spuelen, auch bei nur einer Zeile
                                  // flush at the latest after this long, even for just one line
#define LOG_FLUSH_MAX_BUFFER_BYTES 1024 // frueher spuelen, wenn der Puffer waechst, statt RAM zu verschwenden
                                        // flush earlier if the buffer grows, instead of wasting RAM

    bool initial = true;

    String ipAddress = "";


    // Kern-Hardwareobjekte (TFT, Webserver, Preferences, RTC, ...)
    // Core hardware objects (TFT, web server, preferences, RTC, ...)
    TFT_eSPI tft = TFT_eSPI();
    WebServer webserver(80);
    Preferences preferences;
    DNSServer dnsServer;
    WiFiUDP udp;

    // Eigene, separate WiFiUDP-Instanz fuer die R2RNet-Multicast-Diagnose
    // (siehe startR2rnetDebugListener() in rocrail_client.h) - unabhaengig
    // von 'udp' oben (eigener NTP-Server, anderer Port/Zweck).

    // Own, separate WiFiUDP instance for the R2RNet multicast diagnostic
    // listener (see startR2rnetDebugListener() in rocrail_client.h) -
    // independent of 'udp' above (the clock's own NTP server, different
    // port/purpose).
    WiFiUDP r2rnetDebugUdp;
    bool r2rnetDebugListening = false; // true, sobald der Multicast-Gruppe erfolgreich beigetreten wurde
                                       // true once the multicast group has been joined successfully

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

    // Verzoegerter WPS-Start: der HTTP-Handler darf nicht per delay()
    // blockieren, sonst kann der Webserver die Folge-Anfrage fuer die
    // Redirect-Zielseite nicht mehr annehmen - loop() startet WPS stattdessen zeitgesteuert etwas spaeter.

    // Deferred WPS start: the HTTP handler must not block with delay(),
    // otherwise the web server can't accept the redirect target page's
    // follow-up request - loop() starts WPS on a timer a bit later instead.
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

    // Bestaetigungscode fuer destruktive Aktionen, die physischen Zugriff auf
    // die Uhr voraussetzen sollen: die fuenf Factory-Reset-Aktionen UND das
    // Loeschen/Ueberschreiben des AKTUELL VERBUNDENEN WLAN-Netzwerks (siehe
    // checkFactoryResetCodePending() in system_utils.h sowie die
    // /factoryReset/*-Routen, /deletewifi und /save in webserver_routes.h):
    // "" = keine Aktion angefordert, sonst der aktuell auf dem Display
    // gezeigte 3-stellige Code. Verhindert eine Aktion aus der Ferne (z.B.
    // ueber eine DMZ/Port-Weiterleitung erreichbar) ohne physischen Zugriff
    // auf die Uhr. Andere (nicht aktive) WLAN-Netzwerke bleiben ohne Code
    // aenderbar/loeschbar.

    // Confirmation code for destructive actions that should require physical
    // access to the clock: the five factory-reset actions AND deleting/
    // overwriting the CURRENTLY CONNECTED WiFi network (see
    // checkFactoryResetCodePending() in system_utils.h and the
    // /factoryReset/* routes, /deletewifi and /save in webserver_routes.h):
    // "" = no action requested, otherwise the 3-digit code currently shown on
    // the display. Prevents an action triggered remotely (e.g. reachable via
    // a DMZ/port forward) without physical access to the clock. Other
    // (non-active) WiFi networks remain changeable/deletable without a code.
    String factoryResetCode = "";
    unsigned long factoryResetCodeStartMillis = 0;
    bool factoryResetCodeShown = false; // wurde der Code schon gezeichnet? (nur einmal, nicht jeden Frame)
                                       // has the code already been drawn? (only once, not every frame)
    String factoryResetPendingAction = ""; // welche Aktion nach Code-Bestaetigung ausgefuehrt wird:
                                           // "all"/"wifi"/"faces"/"hands"/"presets"/"wlanDeleteActive"/
                                           // "wlanOverwriteActive" (siehe /factoryReset/confirm)
    uint8_t factoryResetCodeAttempts = 0; // Fehlversuche fuer den AKTUELLEN Code (siehe
                                         // FACTORY_RESET_MAX_ATTEMPTS in config.h) - ohne diese Bremse waere
                                         // der Code per Brute-Force erratbar, da 3 Stellen nur 900
                                         // Kombinationen sind und sonst beliebig oft geraten werden koennte.
                                         // failed attempts for the CURRENT code (see FACTORY_RESET_MAX_ATTEMPTS
                                         // in config.h) - without this brake the code would be brute-forceable,
                                         // since 3 digits are only 900 combinations and could otherwise be
                                         // guessed an unlimited number of times.
                                           // which action runs once the code is confirmed:
                                           // "all"/"wifi"/"faces"/"hands"/"presets"/"wlanDeleteActive"/
                                           // "wlanOverwriteActive" (see /factoryReset/confirm)

    // Nutzlast fuer die beiden WLAN-bezogenen Bestaetigungsaktionen oben -
    // nur gueltig, waehrend factoryResetPendingAction entsprechend gesetzt
    // ist. pendingWifiChangeIndex: der zu loeschende Slot (Aktion
    // "wlanDeleteActive", siehe /deletewifi). pendingWifiSsid[]/
    // pendingWifiPass[]: die komplette neue, noch nicht angewendete
    // WLAN-Liste aus dem Formular (Aktion "wlanOverwriteActive", siehe /save
    // und applyWlanList() in webserver_routes.h).

    // Payload for the two WiFi-related confirmation actions above - only
    // valid while factoryResetPendingAction is set accordingly.
    // pendingWifiChangeIndex: the slot to delete (action
    // "wlanDeleteActive", see /deletewifi). pendingWifiSsid[]/
    // pendingWifiPass[]: the complete new, not-yet-applied WiFi list from the
    // form (action "wlanOverwriteActive", see /save and applyWlanList() in
    // webserver_routes.h).
    int pendingWifiChangeIndex = -1;
    String pendingWifiSsid[MAX_WLAN];
    String pendingWifiPass[MAX_WLAN];

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
    volatile uint32_t dcf77EdgeDropped = 0; // Anzahl verworfener Flanken bei vollem Puffer - uint32_t, da uint8_t nach 256 ueberlaufen wuerde (Diagnose).
                                            // number of edges dropped when the buffer was full - uint32_t, since uint8_t would wrap after 256 (diagnostic).
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
    volatile unsigned long lastNtpSuccessMillis = 0; // Zeitpunkt (millis()) der letzten erfolgreichen NTP-Sync (0 = nie) -
                                            // unterscheidet echten Erfolg vom irrefuehrenden true-Rueckgabewert von
                                            // setupNTP() ohne WLAN; bleibt er unveraendert, springt DCF77 ein (time_sync.h).
                                            // volatile seit der Async-Umstellung (startNtpSyncTask()): wird jetzt aus der
                                            // Sync-Task heraus geschrieben und von pollNtpSyncTask() auf dem Hauptthread gelesen.

    // Timestamp (millis()) of the last successful NTP sync (0 = never) -
    // distinguishes a real success from setupNTP()'s misleading true return
    // value without WiFi; if unchanged, DCF77 steps in (time_sync.h)

    // Asynchrone NTP-Sync ueber eine eigene, kurzlebige FreeRTOS-Task - analog
    // zur Rocrail-Connect-Task oben (rocrailConnectTask*), damit setupNTP()'s
    // DNS-/UDP-Wartezeiten (bis zu NTP_SYNC_ATTEMPTS x WAIT_3s PRO konfiguriertem
    // Server, siehe time_sync.h) weder loop() noch den Webserver blockieren.

    // Asynchronous NTP sync via its own short-lived FreeRTOS task - analogous
    // to the Rocrail connect task above (rocrailConnectTask*), so setupNTP()'s
    // DNS/UDP wait times (up to NTP_SYNC_ATTEMPTS x WAIT_3s PER configured
    // server, see time_sync.h) block neither loop() nor the web server.
    TaskHandle_t ntpSyncTaskHandle = NULL;
    volatile bool ntpSyncTaskRunning = false; // Task laeuft gerade
                                              // task currently running
    volatile bool ntpSyncTaskDone = false;    // Task fertig, Ergebnis in ntpSyncTaskResult
                                              // task finished, result in ntpSyncTaskResult
    volatile bool ntpSyncTaskResult = false;  // Rueckgabewert von setupNTP() des letzten Laufs (siehe Hinweis oben: bei
                                              // fehlendem WLAN irrefuehrend true - pollNtpSyncTask() prueft daher zusaetzlich lastNtpSuccessMillis)
                                              // return value of the last setupNTP() run (see note above: misleadingly
                                              // true without WiFi - pollNtpSyncTask() therefore also checks lastNtpSuccessMillis)
    unsigned long ntpSyncCheckBeforeMillis = 0; // millis() unmittelbar VOR dem Start der laufenden Sync-Task -
                                                // Referenzwert fuer den lastNtpSuccessMillis-Vergleich in pollNtpSyncTask()
                                                // millis() immediately BEFORE starting the current sync task -
                                                // reference value for the lastNtpSuccessMillis comparison in pollNtpSyncTask()
    String ntpSyncCheckLabel = ""; // Label fuer die Logausgabe ("Initial sync"/"Periodic sync"/"Manual sync"), von pollNtpSyncTask() ausgewertet
                                   // label for the log output ("Initial sync"/"Periodic sync"/"Manual sync"), evaluated by pollNtpSyncTask()

    struct tm timeinfo;

    // Rocrail-Modellzeit (siehe rocrail_client.h): eigenstaendige,
    // ggf. beschleunigte Zeitstruktur, komplett getrennt von "timeinfo"
    // oben. renderClockFrame() liest je nach rocrailEnabled aus der einen oder anderen Struktur.

    // Rocrail model time (see rocrail_client.h): an independent, possibly
    // accelerated time struct, fully separate from "timeinfo" above.
    // renderClockFrame() reads from whichever struct applies, depending on rocrailEnabled.
    struct tm rocrailTimeinfo;
    bool rocrailEnabled = false;    // per Tab-Schalter/Preferences aktiviert
                                    // enabled via the tab switch/preferences
    bool rocrailConnected = false;  // TCP-Verbindung zum Server aktuell offen
                                    // TCP connection to the server currently open
    String rocrailServerHost = "";  // aktuell aktiver Server - siehe rocrailServerList[] unten;
                                    // wird beim Speichern aus dem angehakten Listeneintrag uebernommen.
                                    // currently active server - see rocrailServerList[] below;
                                    // taken over from the checked list entry when saving.
    uint16_t rocrailServerPort = ROCRAIL_DEFAULT_PORT;

    // Snapshot von rocrailServerHost/-Port fuer die laufende Connect-Task
    // (siehe startRocrailConnectTask() in rocrail_client.h): rocrailServerHost
    // ist ein Arduino String, NICHT thread-sicher (interne Heap-Reallokation
    // bei Zuweisung) - die Rocrail-Einstellungsseite kann ihn jederzeit vom
    // Hauptthread aus neu zuweisen (webserver_routes.h), waehrend die Task
    // gerade rocrailClient.connect(rocrailServerHost.c_str(), ...) ausfuehrt.

    // Snapshot of rocrailServerHost/-port for the currently running connect
    // task (see startRocrailConnectTask() in rocrail_client.h):
    // rocrailServerHost is an Arduino String, NOT thread-safe (internal heap
    // reallocation on assignment) - the Rocrail settings page can reassign it
    // from the main thread at any time (webserver_routes.h) while the task is
    // in the middle of rocrailClient.connect(rocrailServerHost.c_str(), ...).
    char rocrailServerHostSnapshot[64];
    uint16_t rocrailServerPortSnapshot = ROCRAIL_DEFAULT_PORT;

    char rocrailServerList[MAX_WLAN][64];        // Hostname/IP je Listenplatz (siehe pkRocrailServerHost())
                                                 // hostname/IP per list slot (see pkRocrailServerHost())
    uint16_t rocrailServerPortList[MAX_WLAN];    // Port je Listenplatz (siehe pkRocrailServerPort())
                                                 // port per list slot (see pkRocrailServerPort())

    // Anlagenname je Listenplatz - rein manuell, vom Nutzer frei editierbar
    // zur eigenen Orientierung (z.B. bei mehreren Servern). RCP meldet
    // keinen Anlagennamen, daher keine automatische Uebernahme.

    // Layout name per list slot - purely manual, freely editable by the
    // user for their own reference (e.g. with several servers). RCP does
    // not report a layout name, so there is no automatic takeover.
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
    bool rocrailBrightnessKnown = false; // true, sobald einmal ein bri-Wert empfangen wurde - erst
                                         // dann uebernimmt updateBrightness() ihn; sendet der Server
                                         // nie bri, bleibt die lokale Helligkeitssteuerung dauerhaft aktiv.

                                        // true once a bri value has been received - only then does
                                        // updateBrightness() take it over; if the server never sends
                                        // bri, local brightness control stays active permanently.
    WiFiClient rocrailClient;
    String rocrailRxBuffer;         // Empfangspuffer fuer XML-Tag-Bruchstuecke
                                    // receive buffer for XML tag fragments
    uint8_t rocrailLogChunkCount = 0; // Zaehler fuer die Diagnose-Logeintraege der ersten
                                      // 5 empfangenen RCP-Rohdaten-Haeppchen (siehe pollRocrailClient())
                                      // counter for the diagnostic log entries of the first 5
                                      // received RCP raw data chunks (see pollRocrailClient())
    uint8_t rocrailClockLogCount = 0; // Zaehler fuer die routinemaessige "[ROCRAIL] clock ..."-Zeile
                                      // (siehe processRocrailClockPayload()) - nur die ersten 2 Updates,
                                      // danach nur noch bei Divider-/Pause-Wechsel
                                      // counter for the routine "[ROCRAIL] clock ..." line (see
                                      // processRocrailClockPayload()) - only the first 2 updates,
                                      // afterwards only on divider/pause changes
    uint8_t rocrailConnectFailLogCount = 0; // Zaehler fuer "Connecting to.."/"did not respond"
                                            // (siehe connectRocrailClient()/pollRocrailConnectTask()) -
                                            // nur die ersten 2 Fehlschlaege einer Ausfall-Serie loggen,
                                            // die Versuche selbst laufen unabhaengig davon stumm weiter;
                                            // wird bei erfolgreicher Verbindung zurueckgesetzt
                                            // counter for "Connecting to.."/"did not respond" (see
                                            // connectRocrailClient()/pollRocrailConnectTask()) - only
                                            // log the first 2 failures of a failure streak, the actual
                                            // attempts keep running silently regardless; reset on a
                                            // successful connection
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

    // Snapshot von timezone fuer die laufende NTP-Sync-Task (siehe
    // startNtpSyncTask() in time_sync.h): das globale String-Objekt `timezone`
    // wird auch von Web-Handlern (Zeitzone speichern) und der Einstellungsseite
    // gelesen/geschrieben - ein Arduino String ist dabei NICHT thread-sicher
    // (interne Heap-Reallokation). Die Task liest daher nur diese, auf dem
    // Hauptthread angelegte, waehrend ihrer Laufzeit unveraenderliche Kopie.

    // Snapshot of timezone for the currently running NTP sync task (see
    // startNtpSyncTask() in time_sync.h): the global String object `timezone`
    // is also read/written by web handlers (saving the timezone) and the
    // settings page - an Arduino String is NOT thread-safe for that (internal
    // heap reallocation). The task therefore only ever reads this copy,
    // created on the main thread and immutable for the task's lifetime.
    char timezoneSnapshot[64];

    char ntpServers[MAX_WLAN][64];

    // Snapshot von ntpServers[] fuer die laufende NTP-Sync-Task (siehe
    // startNtpSyncTask() in time_sync.h): wird beim Start der Task auf dem
    // Hauptthread angelegt, damit setupNTP() waehrend der Task NICHT das
    // live-Array liest, in das die Weboberflaeche (updateNtpServersFromRequest())
    // parallel aus einem Web-Request-Handler schreiben koennte.

    // Snapshot of ntpServers[] for the currently running NTP sync task (see
    // startNtpSyncTask() in time_sync.h): created on the main thread when the
    // task starts, so setupNTP() does NOT read the live array while the task
    // runs - the web UI (updateNtpServersFromRequest()) could be writing to
    // it concurrently from a web request handler.
    char ntpServersSnapshot[MAX_WLAN][64];

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

    bool stationMode; // "wartet auf 12": Sekundenzeiger eilt in komprimierter Zeit voraus und pausiert oben (siehe display.h)
                      // "waits at 12": second hand races ahead in compressed time and pauses at the top (see display.h)
    bool smoothMinute;
    bool smoothSecond; // Darstellungsstil des Sekundenzeigers: schwingend (true) oder tickend (false) -
                       // unabhaengig von stationMode (siehe renderClockFrame() in display.h)
                       // second hand rendering style: smooth/swinging (true) or ticking (false) -
                       // independent of stationMode (see renderClockFrame() in display.h)
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

    // Rotation 0-3 (0/90/180/270 Grad) oder TFT_ROTATION_NA (Display nicht angeschlossen,
    // siehe config.h) - fuer n.a. wird kein Zifferblatt/Zeiger berechnet, Status-Meldungen
    // laufen aber weiter. Abfrage ueber isDisplayConnected()/effectiveRotation(), nie direkt gegen den Wert.

    // Rotation 0-3 (0/90/180/270 degrees) or TFT_ROTATION_NA (display not connected,
    // see config.h) - for n.a. no face/hands are calculated, but status messages still
    // run. Query via isDisplayConnected()/effectiveRotation(), never compare the value directly.
    uint8_t tftRotation1 = TFT_ROTATION1_DEFAULT;
    uint8_t tftRotation2 = TFT_ROTATION2_DEFAULT; // Rotation von Display 2 (CS2) - eigener Wert, damit beide Displays
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

    // Kopie von handComposite[].buffer + darauf kantengeglaetteter Sekunden-
    // zeiger (siehe renderClockFrame()) - NUR im tickenden (Nicht-Bahnhofsuhr-)
    // Modus genutzt, da der Sekundenzeiger dort nur einmal pro Sekunde bewegt
    // wird und die teurere Blendtechnik so nicht ins Gewicht faellt. Eine
    // Kopie statt direktem Arbeiten auf handComposite[].buffer, damit der
    // eigentliche Cache (nur Zifferblatt+Stunden-/Minutenzeiger) unveraendert
    // bleibt und nicht bei jedem Sekundenwechsel neu aufgebaut werden muss.

    // Copy of handComposite[].buffer with the anti-aliased second hand
    // blended on top (see renderClockFrame()) - used ONLY in ticking (non-
    // station-clock) mode, since the second hand only moves once per second
    // there, so the costlier blend technique doesn't matter performance-wise.
    // A copy instead of working directly on handComposite[].buffer, so the
    // actual cache (face + hour/minute hand only) stays unchanged and doesn't
    // need rebuilding on every second change.
    uint16_t* secondHandCompositeScratch = nullptr;

    // Cache: clockFaceBuffer bereits mit currentBrightness vorberechnet (siehe
    // loadClockFace()) - vermeidet die teure Pixel-Helligkeitsanpassung bei
    // jedem Tick, obwohl sich die Helligkeit dazwischen fast nie aendert.

    // Cache: clockFaceBuffer is pre-adjusted for currentBrightness (see
    // loadClockFace()) - avoids the expensive per-pixel brightness adjustment
    // on every tick, even though brightness rarely changes in between.
    uint16_t* clockFaceBrightBuffer = nullptr;

    // Index 0 = Display 1, 1 = Display 2: true, solange auf dem Display noch Inhalt steht
    // (Uhr oder Status-/Startmeldung), der nicht mehr zur Anzeige passt. Ein "n.a."-Display
    // wird daraufhin von updateClock() einmalig schwarz geloescht. Startwert true, da der
    // Bildinhalt nach tft.init() undefiniert ist.

    // Index 0 = display 1, 1 = display 2: true while the display still shows content
    // (clock or status/boot message) that no longer fits. A "n.a." display is then
    // cleared to black once by updateClock(). Initially true, since the screen content
    // is undefined after tft.init().
    bool displayNeedsBlank[2] = { true, true };

    // Display 2 (baugleich, am CS2-Pin, siehe config.h) ist ueber seine
    // Rotation "n.a." (tftRotation2) abschaltbar, kein eigener Preferences-/UI-Schalter.

    // Display 2 (identical, on the CS2 pin, see config.h) can be switched off
    // via its rotation "n.a." (tftRotation2), no separate preferences/UI toggle.

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

    // Mindestabstand zwischen zwei ADC-Abtastungen (siehe updateBrightness()) -
    // ohne das wuerde das ADC_SMOOTHING-Fenster bei schnellen loop()-Durchlaeufen
    // nur wenige Millisekunden abdecken statt eine Sekunde.

    // Minimum spacing between two ADC samples (see updateBrightness()) -
    // without it, the ADC_SMOOTHING window would span only a few
    // milliseconds on fast loop() iterations instead of one second.
#define ADC_SAMPLE_INTERVAL_MS 50
    unsigned long lastAdcSampleMillis = 0;

    // Wie lange eine Schwellwert-Ueberschreitung anhalten muss, bevor
    // targetBrightness tatsaechlich springt (siehe updateBrightness()) - filtert
    // kurze Stromspitzen-Einbrueche (z.B. WLAN-Sendeburst waehrend NTP-Sync),
    // die sonst das Zifferblatt fuer einen Frame sichtbar umfaerben wuerden.

    // How long a threshold crossing must persist before targetBrightness
    // actually jumps (see updateBrightness()) - filters brief current-draw
    // dips (e.g. a WiFi TX burst during NTP sync) that would otherwise
    // visibly re-tint the clock face for a frame.
#define BRIGHTNESS_DEBOUNCE_MS 1500
    int pendingBrightnessState = 0; // -1 = Kandidat fuer minBrightness, 1 = fuer maxBrightness, 0 = im Hysterese-Band
                                    // -1 = candidate for minBrightness, 1 = for maxBrightness, 0 = inside the hysteresis band
    unsigned long brightnessThresholdSinceMillis = 0;

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
