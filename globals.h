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

    bool dcf77Flank = false; // false = fallende Flanke, true = steigende Flanke
                             // false = falling edge, true = rising edge
    DCF77 dcf = DCF77(DCF77_DATAPIN, DCF77_DATAPIN, dcf77Flank);
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
    time_t lastDcfSyncTime = 0; // Unix-Zeitstempel der letzten erfolgreichen DCF77-Synchronisation (0 = noch nie)
                                // Unix timestamp of the last successful DCF77 sync (0 = never)

#endif

    unsigned long lastNTPUpdate = 0; // Zeitpunkt des letzten RTC-Updates
                                     // Timestamp of the last RTC update
    unsigned long lastDCFUpdate = 0; // Wartezeit nach DCF77-Update, bevor RTC aktualisiert wird (ms)
                                     // Wait time after a DCF77 update before the RTC is updated (ms)
    unsigned long lastRTCUpdate = 0; // Zeitpunkt des letzten RTC-Updates
                                     // Timestamp of the last RTC update
    unsigned long lastNtpSuccessMillis = 0; // Zeitpunkt (millis()) der letzten ERFOLGREICHEN NTP-Synchronisation (0 = noch nie); DCF77 uebernimmt die Systemzeit nur, wenn NTP seit laengerer Zeit nicht erfolgreich war

    // Timestamp (millis()) of the last SUCCESSFUL NTP sync (0 = never); DCF77 only
    // takes over the system time if NTP hasn't succeeded for a while

    unsigned long lastNTPRetry = 0;

    unsigned long lastCheck = 0;

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
