    // howl-clock@gmx.de - Stationsuhr. ESP32-S2 Mini (Lolin S2 Pico), LittleFS, TFT GC9A01/GC9D01, LovyanGFX
    // 1.2.30. DCF77-Modul: https://de.elv.com/p/elv-dcf-empfangsmodul-dcf-2-P091610/

    // howl-clock@gmx.de - station clock. ESP32-S2 Mini (Lolin S2 Pico), LittleFS, TFT GC9A01/GC9D01,
    // LovyanGFX 1.2.30. DCF77 module: https://de.elv.com/p/elv-dcf-empfangsmodul-dcf-2-P091610/

    
#include <WiFi.h>
#include <WebServer.h>

#include "prefs_keys.h"
#include "build_defs.h"

#include "config.h"        // Board-/Display-Auswahl, Pins, Timing-Makros
                           // board/display selection, pins, timing macros
#include "lgfx_config.h"   // LovyanGFX: Bus, Panel, Init-Sequenz (Pins aus config.h)
                           // LovyanGFX: bus, panel, init sequence (pins from config.h)
#include <Preferences.h>
#include <LittleFS.h>
#include <set>
#include <base64.h>
#include "nvs_flash.h"
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <map>
#include <esp_wps.h>
#include <esp_wifi.h>

// ESP_ARDUINO_VERSION_STR: arduino-esp32-Core-Version, mit der kompiliert
// wurde (Anzeige im Status, siehe webserver_routes.h).

// ESP_ARDUINO_VERSION_STR: arduino-esp32 core version this was compiled
// with (shown in Status, see webserver_routes.h).

#include <esp_arduino_version.h>

// Fuer esp_reset_reason() - Grund des letzten Neustarts (Power-On, Watchdog,
// Panic, Brownout, ...), Anzeige im Status (siehe webserver_routes.h).

// For esp_reset_reason() - reason for the last restart (power-on, watchdog,
// panic, brownout, ...), shown in Status (see webserver_routes.h).

#include <esp_system.h>
#include <Wire.h>
#include <RTClib.h>
#include <WiFiUdp.h>

// Fuer sntp_set_sync_mode()/sntp_set_sync_interval() - siehe setup() und
// den Kommentar dort zum Sekundenzeiger-Ruecksprung-Fix.

// For sntp_set_sync_mode()/sntp_set_sync_interval() - see setup() and the
// comment there about the second-hand jump-back fix.

#include <esp_sntp.h>


#include "globals.h"       // globale Objekte, Variablen, Structs
                           // global objects, variables, structs
#include "declarations.h"  // Forward-Deklarationen aller Funktionen
                           // forward declarations of all functions

#include "wifi_manager.h"      // WLAN: Verbindung, AP, Scan, Reconnect
                               // WiFi: connection, AP, scan, reconnect
#include "time_sync.h"         // RTC, DCF77, NTP
#include "rocrail_client.h"    // Rocrail-Modellzeit (Discovery, TCP-Client, Clock-Parsing)
                               // Rocrail model time (discovery, TCP client, clock parsing)
#include "display.h"           // Zifferblatt, Zeiger, Helligkeit
                               // dial, hands, brightness
#include "presets_manager.h"   // Presets laden/speichern/wechseln
                               // load/save/switch presets
#include "hand_designer_html.h" // Zeiger-Designer-Seite (HTML/JS im Flash)
                                // hand designer page (HTML/JS in flash)
#include "face_designer_html.h" // Zifferblatt-Designer-Seite (HTML/JS im Flash)
                                // clock face designer page (HTML/JS in flash)
#include "backup.h"            // Komplettsicherung/-wiederherstellung (TAR)
                               // full backup/restore (TAR)
#include "webserver_routes.h"  // Webinterface (alle HTTP-Routen)
                               // web interface (all HTTP routes)
#include "system_utils.h"      // Tasten, Logging, Reset, Neustart
                               // buttons, logging, reset, restart


    // Baut die WLAN-Verbindung beim Booten auf (Zugangsdaten, Scan, Fallback
    // RTC/DCF77/AP). Eigene Funktion statt Block in setup(): mehrere "return"-
    // Ausstiege wuerden sonst das GESAMTE setup() vorzeitig beenden.

    // Establishes the WiFi connection at boot (credentials, scan, RTC/DCF77/AP
    // fallback). Own function instead of a block in setup(): its several
    // "return" exits would otherwise end ALL of setup() early.

void connectWiFiAtBoot() {

        // WLAN-Zugangsdaten laden

        // Load WiFi credentials

        for (int i = 0; i < MAX_WLAN; i++) {

            // Dynamisch berechnete Schlüssel

            // Dynamically computed keys

            String ssidKey = pkSsid(i);
            wifiSsid[i] = preferences.getString(ssidKey.c_str(), "");
            wifiPass[i] = loadWifiPass(i);
        }


        // AP starten, wenn keine SSID gespeichert ist

        // Start AP if no SSID is stored

        for (int i = 0; i < MAX_WLAN; i++) {
            if (wifiSsid[i].length() > 0) {
                DEBUG_PRINTLN("[WiFi] Found stored SSID: " + wifiSsid[i]);
                break;
            }
            if (i == MAX_WLAN - 1) {
                DEBUG_PRINTLN("[WiFi] No stored SSID found");
                startAP();
                return;
            }
        }


        // WLAN-Netzwerke scannen und cachen

        // Scan and cache WiFi networks
        //scanAndCacheNetworks();

        while (isScanning) {
            checkWiFiScan();
            delay(10);
            if (loggingEnabled)  Serial.print("");
        }
        if (loggingEnabled) Serial.println("");
        // DEBUG_PRINTLN("[WiFi] Scan complete");



        // Bereichspruefung: der Wert kommt aus dem NVS und dient direkt als
        // Array-Index. Ein beschaedigter Wert ausserhalb 0..MAX_WLAN-1 wuerde
        // sonst zu undefiniertem Verhalten statt einem sauberen Fehler fuehren.

        // Range check: the value comes from NVS and is used directly as an
        // array index. A corrupted value outside 0..MAX_WLAN-1 would otherwise
        // cause undefined behavior instead of a clean error.

        int storedWlanNumber = preferences.getInt(PK_LAST_WLAN, 0);
        if (storedWlanNumber < 0 || storedWlanNumber >= MAX_WLAN) {
            DEBUG_PRINTLN("[WiFi] Stored WLAN number out of range (" + String(storedWlanNumber) + "), falling back to 0");
            storedWlanNumber = 0;
            preferences.putInt(PK_LAST_WLAN, storedWlanNumber);
        }
        uint32_t number = (uint32_t)storedWlanNumber;
        DEBUG_PRINTLN("[WiFi] Last successful WLAN number: (" + String(number + 1) + ") " + wifiSsid[number]);


        // ist die letzte SSID im Scan vorhanden?

        // Is the last SSID present in the scan?

        bool foundLastSSID = false;
        for (int i = 0; i < MAX_WLAN; i++) {
            if (wifiSsid[number] == availableNetworks[i].ssid) {
                DEBUG_PRINTLN("[WiFi] Last connected SSID found in scan: " + availableNetworks[i].ssid);
                foundLastSSID = true;
                break;
            }
        }
        if (foundLastSSID == false) {
            DEBUG_PRINTLN("[WiFi] Last connected SSID '" + wifiSsid[number] + "' not found in scan, starting new scan..");
            startWiFiScan();
            delay(100); // Kurze Verzögerung, damit der Scan starten kann
                        // brief delay to let the scan start
            while (isScanning) {
                checkWiFiScan();
                delay(50);
                if (loggingEnabled) Serial.print("");
            }
            if (loggingEnabled) Serial.println("");

            // Nach dem zweiten Scan neu auswerten - sonst blieb foundLastSSID
            // immer false und der zweite Scan war wirkungslos.

            // Re-evaluate after the second scan - otherwise foundLastSSID stayed
            // false regardless, making the second scan pointless.

            for (int i = 0; i < MAX_WLAN; i++) {
                if (wifiSsid[number] == availableNetworks[i].ssid) {
                    DEBUG_PRINTLN("[WiFi] Last connected SSID found in second scan: " + availableNetworks[i].ssid);
                    foundLastSSID = true;
                    break;
                }
            }
        }

        // Zuletzt genutztes Netz IMMER direkt versuchen, auch ohne Scan-Treffer -
        // WiFi.begin() braucht keinen Scan, das deckt auch versteckte SSIDs und
        // Router ab, die beim Booten noch nicht bereit sind.

        // Always try the last used network directly, even without a scan hit -
        // WiFi.begin() needs no scan match, covering hidden SSIDs and routers
        // not yet ready at boot.

        bool lastSsidTried = false;
        int lastSsidResult = NOT_CONNECTED;

        if (trim(wifiSsid[number]) != "") {
            if (!foundLastSSID) {
                DEBUG_PRINTLN("[WiFi] Last connected SSID '" + wifiSsid[number] + "' not found in scan - trying it anyway (may be hidden or router not up yet)");
            }
            lastSsidTried = true;

            // Mehrere Versuche (siehe WIFI_CONNECT_ATTEMPTS in config.h) - ein
            // einzelner Fehlschlag (z.B. Router kurz beschaeftigt) soll dieses
            // Netzwerk nicht sofort verwerfen.

            // Multiple attempts (see WIFI_CONNECT_ATTEMPTS in config.h) - a
            // single failure (e.g. the router being briefly busy) shouldn't
            // discard this network right away.

            lastSsidResult = connectWiFiWithRetries(number, wifiSsid[number]);
        }

        if (lastSsidResult == NOT_CONNECTED) {

            // Wenn Verbindung fehlschlägt, scannen und vergleichen

            // If connection fails, scan and compare

            DEBUG_PRINTLN("[WiFi] Connection failed. Looking for available networks..");

            // Durchsuche gefundene Netzwerke nach gespeicherten SSIDs

            // Search found networks for stored SSIDs

            for (int i = 0; i < MAX_WLAN; i++) {
                String availableSSID = availableNetworks[i].ssid;
                if (availableSSID == "") continue;
                //  DEBUG_PRINTLN("Found network: " + availableSSID);
                for (int j = 0; j < MAX_WLAN; j++) {

                    // Nur ueberspringen, wenn oben tatsaechlich versucht wurde.

                    // Only skip if it was actually attempted above.

                    if (lastSsidTried && j == (int)number) continue;
                    if (trim(wifiSsid[j]) == "") continue; // überspringe leere SSID
                                                           // skip empty SSID

                    // DEBUG_PRINTLN("vergleiche " + wifiSsid[j] + " mit " + availableSSID);

                    if (wifiSsid[j] == availableSSID) {
                        DEBUG_PRINTLN("[WiFi] Found matching network: " + availableSSID);

                        // Mehrere Versuche (siehe WIFI_CONNECT_ATTEMPTS in
                        // config.h) - gleicher Grund wie beim zuletzt
                        // verbundenen Netz oben.

                        // Multiple attempts (see WIFI_CONNECT_ATTEMPTS in
                        // config.h) - same reason as for the last connected
                        // network above.

                        bool connectedNow = (connectWiFiWithRetries(j, availableSSID) != NOT_CONNECTED);

                        if (connectedNow) {
                            DEBUG_PRINTLN("[WiFi] Connected to " + availableSSID + " using stored credentials");
                            preferences.putInt(PK_LAST_WLAN, j);
                            return;
                        }
                    }

                }
            }

            if (rtcOk == RTC_AVAILABLE) {
                DRAW_ON_BOTH_DISPLAYS(
                    tft.fillScreen(TFT_BLACK);
                    tft.setTextColor(TFT_WHITE);
                    tft.setTextSize(TFT_TEXT_SIZE);
                    tft.setCursor(20, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 4));
                    tft.println(tftText(translate("Check RTC")));
                );
                delay(1000);
                loadTimeFromRTC();
                return;
            }

            // delay(3000);
            if (dcf77Count >= 1) {
                DEBUG_PRINTLN("[DCF77] DCF77 signal received during setup, waiting for valid time..");
                DRAW_ON_BOTH_DISPLAYS(
                    tft.fillScreen(TFT_BLACK);
                    tft.setTextColor(TFT_WHITE);
                    tft.setTextSize(TFT_TEXT_SIZE);
                    tft.setCursor(20, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 4));
                    tft.println(tftText(translate("DCF77 detected")));
                );

                unsigned long startWait = millis();
                while (millis() - startWait < WAIT_1h) { // Warte bis zu 1 Stunde auf gültige DCF77-Zeit
                                                         // wait up to 1 hour for a valid DCF77 time

                    // Vor loop() muessen processDcf77Bits()/updateDcf77Status()
                    // manuell aufgerufen werden, sonst laeuft der ISR-
                    // Ringpuffer (globals.h) schnell voll.

                    // Before loop() starts, processDcf77Bits()/updateDcf77Status()
                    // must be called manually, or the ISR's ring buffer
                    // (globals.h) fills up quickly.

                    processDcf77Bits();
                    updateDcf77Status();
                    handleSerialCommands(); // Displaytyp per USB auch waehrend dieser Wartezeit (siehe handleSerialCommands())
                                            // display type via USB also during this wait (see handleSerialCommands())
                    if (applyDcf77DecodedTime("[DCF77] boot (no WiFi/RTC)")) {
                        return;
                    }
                    if (serialTimeSet) { // Uhrzeit per USB vom PC / time via USB from the PC
                        DEBUG_PRINTLN("[DCF77] Wait ended - time set via USB");
                        return;
                    }
                    DRAW_ON_BOTH_DISPLAYS(
                        tft.setCursor(20, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 8));
                        tft.print(tftText(translate("Waiting")));
                        tft.print("...");
                    );
                    delay(100);
                }
            }


            // Alle Verbindungsversuche fehlgeschlagen: AP nur ohne gueltige RTC starten

            // All connection attempts failed: only start AP without a valid RTC

            if (rtcOk != RTC_AVAILABLE) {
                DEBUG_PRINTLN("[WiFi] Starting Access Point due to failed connections and no valid RTC");
                startAP();
            }
        }
}


    // Setup-Funktion

    // Setup function

void setup() {

        setLedOn();

        Serial.begin(115200);

#if ARDUINO_USB_CDC_ON_BOOT
        // USB-Ausgaben nie blockieren lassen: hat der PC die Schnittstelle offen
        // (DTR), liest aber nicht mit, wartet jedes print() sonst bis zu 250 ms -
        // bei vielen Log-Zeilen stand die Uhr scheinbar still (z.B. solange ein
        // flashESP-Fenster offen war). Volle Puffer werden jetzt verworfen;
        // Antworten auf USB-Befehle siehe serialReply() in display.h.

        // Never let USB output block: if the PC has the port open (DTR) but does
        // not read, every print() otherwise waits up to 250 ms - with many log
        // lines the clock seemingly stood still (e.g. while a flashESP window
        // was open). Full buffers are now dropped; replies to USB commands see
        // serialReply() in display.h.
        Serial.setTxTimeoutMs(0);
#endif
#if ARDUINO_USB_CDC_ON_BOOT && !ARDUINO_USB_MODE
        // Kein Neustart in den Download-Modus ueber die DTR/RTS-Folge: sie
        // entstand auch beim blossen Oeffnen/Schliessen der Schnittstelle
        // (flashESP-Einrichtung, serielle Monitore), die Uhr blieb dann im
        // Bootloader haengen. Die 1200-Baud-Umschaltung, die flashESP (port.ps1,
        // flashESP.sh) und die Arduino IDE zum Flashen nutzen, uebernimmt
        // usbCdcLineCodingEvent() (display.h) selbst.

        // No restart into download mode via the DTR/RTS sequence: it also
        // occurred when merely opening/closing the port (flashESP setup, serial
        // monitors), the clock then got stuck in the bootloader. The 1200 baud
        // switch that flashESP (port.ps1, flashESP.sh) and the Arduino IDE use
        // for flashing is handled by usbCdcLineCodingEvent() (display.h) itself.
        Serial.enableReboot(false);
        Serial.onEvent(ARDUINO_USB_CDC_LINE_CODING_EVENT, usbCdcLineCodingEvent);
#endif

        // Muss vor jedem moeglichen ersten DEBUG_PRINTLN()-Aufruf existieren
        // (siehe logToFile()/logLineBuffer in globals.h) - loggingEnabled ist
        // zu diesem Zeitpunkt zwar noch false, aber sicher ist sicher.

        // Must exist before any possible first DEBUG_PRINTLN() call (see
        // logToFile()/logLineBuffer in globals.h) - loggingEnabled is still
        // false at this point, but better safe than sorry.

        logBufferMutex = xSemaphoreCreateMutex();

        // CS_1 (Display-1-Chip-Select) manuell auf Output/LOW setzen - LovyanGFX
        // steuert keinen CS-Pin (pin_cs = -1, siehe lgfx_config.h).
        // Muss VOR tft.init() weiter unten passieren.

        // Manually set CS_1 (display 1's chip select) to output/LOW - LovyanGFX
        // drives no CS pin (pin_cs = -1, see lgfx_config.h).
        // Must happen BEFORE tft.init() further below.

        pinMode(CS_1, OUTPUT);
        digitalWrite(CS_1, LOW);

        // CS2-Pin folgt erst nach preferences.begin() (Display 2 haengt an seiner Rotation, "n.a." = aus).
        // Asynchronen WLAN-Scan starten, damit die Netze beim ersten Oeffnen der WLAN-Einstellungen schon
        // bekannt sind.

        // The CS2 pin follows only after preferences.begin() (display 2 depends on its rotation, "n.a." =
        // off). Start an async WiFi scan so the networks are already known when the WiFi settings are first
        // opened.

        startWiFiScan();

        // Event-Handler fuer per Web-Button gestartete WPS-Anfragen registrieren
        // (siehe /api/startWPS in webserver_routes.h) - event-basiert statt
        // Status-Polling, wie im Espressif-WPS-Beispiel empfohlen.

        // Register event handler for WPS requests started via the web button
        // (see /api/startWPS in webserver_routes.h) - event-based instead of
        // status polling, as recommended in Espressif's WPS example.

        WiFi.onEvent(onWpsEvent);

        // Jahr 0 (1900) = "noch keine Uhrzeit", die Zeiger stehen bis dahin
        // auf der Startzeit (siehe START_TIME_* in config.h)

        // Year 0 (1900) = "no time yet", until then the hands show the start
        // time (see START_TIME_* in config.h)

        memset(&timeinfo, 0, sizeof(timeinfo));
        timeinfo.tm_hour = START_TIME_HOUR;
        timeinfo.tm_min = START_TIME_MIN;
        timeinfo.tm_sec = START_TIME_SEC;

        if (loggingEnabled) {
            unsigned long serialStart = millis();
            while (!Serial && (millis() - serialStart < WAIT_5s)) {
                delay(1);
            }
        }


        if (!LittleFS.begin(true)) {
            if (loggingEnabled) Serial.println("[LittleFS] Mount Failed");
        }

        preferences.begin("clock", false);

        // Altlasten frueherer Firmware entfernen (u.a. ein WLAN-Passwort im
        // Klartext unter "pass") - siehe OBSOLETE_PREF_KEYS in prefs_keys.h

        // Remove leftovers of earlier firmware (among others a plain-text WiFi
        // password under "pass") - see OBSOLETE_PREF_KEYS in prefs_keys.h

        for (const char* key : OBSOLETE_PREF_KEYS) {
            if (preferences.isKey(key)) preferences.remove(key);
        }

        // Keine zweite Klartext-Kopie der WLAN-Zugangsdaten im WiFi-Treiber
        // (siehe wipeWifiDriverStorage() in wifi_manager.h)

        // No second plain-text copy of the WiFi credentials in the WiFi driver
        // (see wipeWifiDriverStorage() in wifi_manager.h)

        WiFi.persistent(false);
        wipeWifiDriverStorage();

        // WLAN-Passwoerter im Klartext (aeltere Firmware/Sicherung) verschluesseln

        // Encrypt plain-text WiFi passwords (older firmware/backup)

        migrateWifiPasswords();

        // Displaytyp sofort nach preferences.begin() - alles Folgende
        // (Migrationen, Sprites, Puffer, Werksvorgaben) haengt an den Massen.

        // Display type right after preferences.begin() - everything that
        // follows (migrations, sprites, buffers, factory defaults) depends on
        // the dimensions.

        loadDisplayType();

        snprintf(version, sizeof(version), "%d-%02d-%02d %02d:%02d:%02d", BUILD_YEAR, BUILD_MONTH, BUILD_DAY, BUILD_HOUR, BUILD_MIN, BUILD_SEC);

        DEBUG_PRINTLN("[SETUP] start");
        DEBUG_PRINTLN(String("[SETUP] Build-Version: ") + version);

        // Diese drei Migrationsschritte muessen nur EINMAL laufen (neu hochgeladene
        // Dateien werden bereits beim Upload RLE-komprimiert/maskiert) - ohne Flag
        // wuerde bei jedem Boot das komplette Dateisystem durchsucht.

        // These three migration steps only need to run ONCE (newly uploaded files
        // are already RLE-compressed/masked on upload) - without this flag the
        // whole filesystem would be scanned on every boot.

        if (!preferences.getBool(PK_MIGRATIONS_DONE, false)) {

            // Bestehende Zifferblaetter im alten Standard-BMP-Format einmalig auf
            // das neue, platzsparende RLE-Format umstellen (siehe display.h).

            // One-time conversion of existing dials from the old standard BMP format
            // to the new, space-saving RLE format (see display.h).

            migrateFaceBmpsToRLE();

            // Bestehende Zeigersaetze im alten Standard-BMP-Format ebenfalls
            // einmalig auf RLE umstellen (siehe display.h).

            // Also do a one-time conversion of existing hand sets from standard BMP
            // to RLE (see display.h).

            migrateHandBmpsToRLE();

            // Bereits vorhandene, schon RLE-komprimierte Zifferblaetter nachtraeglich
            // mit der Kreismaskierung fuer runde Displays versehen, falls sie vor
            // Einfuehrung dieser Funktion migriert/hochgeladen wurden (siehe display.h).

            // Retroactively add the circular mask for round displays to dials that
            // were already RLE-compressed but migrated/uploaded before this feature
            // existed (see display.h).

            remaskExistingFaceCorners();

            preferences.putBool(PK_MIGRATIONS_DONE, true);
        }

        // Merken, ob deleteAllLogFiles() die Lognummer gerade (Versionswechsel) auf 1 gesetzt hat - dann
        // nicht zusaetzlich hochzaehlen, sonst begaenne die erste Datei nach dem Flashen bei 2.

        // Remember whether deleteAllLogFiles() just reset the log number to 1 (version change) - then do not
        // increment it again, otherwise the first file after flashing would start at 2.

        bool logNumberJustReset = false;

        if (preferences.getString(PK_VERSION, "") != String(version)) {
            DEBUG_PRINTLN("[Preferences] Version change detected, updating version in preferences..");
            preferences.putString(PK_VERSION, String(version));
            preferences.putBool(PK_LOGGING_ENABLED, true); // Logging bei Version-Änderung aktivieren
                                                           // enable logging on version change
            deleteAllLogFiles();
            logNumberJustReset = true;
        }

        // Logging aktivieren, wenn in den Preferences aktiviert

        // Enable logging if enabled in preferences

        loggingEnabled = preferences.getBool(PK_LOGGING_ENABLED, false);
        if (loggingEnabled && !logNumberJustReset) {
            uint16_t logfileNumber = preferences.getInt(PK_LOG_FILE_NUMBER, 0);
            logfileNumber++;
            preferences.putInt(PK_LOG_FILE_NUMBER, logfileNumber);
        }

        // LED-Blitz waehrend der DCF77-Synchronisation, abschaltbar per Haken (dcfSyncLedEnabled) - ab Werk
        // an.

        // LED flash during DCF77 sync, switchable via checkbox (dcfSyncLedEnabled) - on by default.

        dcfSyncLedEnabled = preferences.getBool(PK_DCF_SYNC_LED, true);

        DEBUG_PRINTLN("[SETUP] Initializing..");

        // Zeitzone VOR dem ersten RTC-/DCF77-Lesen anwenden - sonst wird die
        // dort gespeicherte Ortszeit als UTC interpretiert (siehe
        // applyTimezoneToSystem() in time_sync.h).

        // Apply the timezone BEFORE the first RTC/DCF77 read - otherwise the
        // local time stored there gets interpreted as UTC (see
        // applyTimezoneToSystem() in time_sync.h).

        timezone = preferences.getString(PK_TIMEZONE, TIMEZONE_DEFAULT);
        applyTimezoneToSystem();
        DEBUG_PRINTLN("[NTP] Timezone set to: " + timezone);

        // I2C-Scanner starten, um RTC zu erkennen

        // Start I2C scanner to detect the RTC

        Wire.begin(SDA_PIN, SCL_PIN);
        if (i2cScan() == 1) {

            if (rtc.begin()) {

                DateTime compileTime(F(__DATE__), F(__TIME__)); // Kompilierzeit
                                                                // compile time

                DEBUG_PRINTLN("[RTC] found RTC");

                // Überprüfen, ob die RTC eine gültige Zeit zurückgibt

                // Check whether the RTC returns a valid time
                // DateTime now = rtc.now();

                if (rtc.lostPower()) {
                    DEBUG_PRINTLN("[RTC] RTC lost power");
                    rtcOk = RTC_AVAILABLE_BUT_INVALID;
                } else if (rtc.now() > compileTime) {
                    DEBUG_PRINTLN("[RTC] RTC is running and returning valid time");
                    rtcOk = RTC_AVAILABLE;
                    loadTimeFromRTC();
                } else {
                    DEBUG_PRINTLN("[RTC] RTC time is less than compilation time");
                    rtcOk = RTC_AVAILABLE_BUT_INVALID;
                }

                rtc.disable32K(); // 32kHz-Output deaktivieren, wird nicht benötigt
                                  // disable 32kHz output, not needed
                DEBUG_PRINTLN("[RTC] Temperature: " + String(rtc.getTemperature()) + " C");
            } else {
                DEBUG_PRINTLN("[RTC] RTC not found");
                rtcOk = RTC_NOT_AVAILABLE;
            }
        }
        else {
            Wire.end();
            rtcOk = RTC_NOT_AVAILABLE;
        }


        // PSRAM-Check: der GC9D01-Treiber hat keine Hardware-Rotation (MADCTL immer 0), mit PSRAM wird per
        // Software gedreht (gc9d01SwRotation). Den Displaytyp hat loadDisplayType() schon gesetzt.

        // PSRAM check: the GC9D01 driver has no hardware rotation (MADCTL always 0), with PSRAM rotation is
        // done in software (gc9d01SwRotation). loadDisplayType() has already set the display type.

        if (psramFound() and ESP.getFreePsram() > 2 * (CLOCK_WIDTH * CLOCK_HEIGHT * sizeof(uint16_t))) {
            gc9d01SwRotation = true;
            DEBUG_PRINTLN("[INFO] found PSRAM");
        }
        else {
            gc9d01SwRotation = false;
            DEBUG_PRINTLN("[INFO] no PSRAM, use Hardware-Rotation");

            // Gespeicherte Rotation darf beim Start nicht ueberschrieben werden -
            // fehlende Wirkung der Hardware-Rotation ohne PSRAM ist Treiber-
            // bedingt und wird im Status als "rotation mode" angezeigt.

            // A saved rotation must not be overwritten at startup - hardware
            // rotation being ineffective without PSRAM is a driver quirk,
            // shown in Status as "rotation mode".

        }
        if (!displayGeom->swRotation) { // wird nur beim GC9D01 benoetigt (DISPLAY_GEOMETRY)
                                        // only needed for the GC9D01 (DISPLAY_GEOMETRY)
            gc9d01SwRotation = false;
        }



#define MAGIC_NUMBER 42  // Erster Start: Alle Preferences mit Standardwerten belegen
                         // first start: set all preferences to defaults

        if (preferences.getInt(PK_FIRST_START, 0) != MAGIC_NUMBER) {
            DEBUG_PRINTLN("[Preferences] First start detected, initializing..");

            preferences.putInt(PK_FIRST_START, MAGIC_NUMBER);

            preferences.putString(PK_LANGUAGE, "en");

            preferences.putBool(PK_WIFI_ACTIVE, true);


            for (int i = 0; i < MAX_WLAN; i++) {
                String ssidKey = pkSsid(i);
                String passKey = pkPass(i);

                preferences.putString(ssidKey.c_str(), "");
                preferences.putString(passKey.c_str(), "");
            }

            preferences.putInt(PK_LAST_WLAN, 0);

            preferences.putString(pkNtpServer(0).c_str(), NTP_SERVER_1);
            preferences.putString(pkNtpServer(1).c_str(), NTP_SERVER_2);

            preferences.putString(PK_TIMEZONE, TIMEZONE_DEFAULT);

            preferences.putUChar(PK_TFT_ROTATION1, TFT_ROTATION1_DEFAULT);
            preferences.putUChar(PK_TFT_ROTATION2, TFT_ROTATION2_DEFAULT); // Display 2 werkseitig "nicht angeschlossen"
                                                                           // display 2 is "not connected" by factory default
            preferences.putString(PK_HANDSET, "default");
            preferences.putString(PK_BACKGROUND, "/face_default.bmp");

            preferences.putBool(PK_STATION_MODE, true);
            preferences.putBool(PK_SHOW_SECOND_HAND, true);
            preferences.putBool(PK_SMOOTH_MINUTE, false);
            preferences.putBool(PK_SMOOTH_SECOND, true); // Faktoreinstellung passend zu stationMode=true oben (klassischer Bahnhofsuhr-Look: schwingend + wartet auf 12)
                                                         // factory default matching stationMode=true above (classic station-clock look: smooth + waits at 12)


            // Hintergrundbeleuchtung und die davon abhaengigen Helligkeits-
            // vorgaben (min. Helligkeit, Schwellwerte) - siehe
            // putBrightnessDefaults() in display.h.

            // Backlight and the brightness defaults depending on it (min.
            // brightness, thresholds) - see putBrightnessDefaults() in display.h.

            preferences.putBool(PK_USE_BACKLIGHT, BACKLIGHT_DEFAULT);
            putBrightnessDefaults(BACKLIGHT_DEFAULT);
            preferences.putUChar(PK_MAX_BRIGHTNESS, 255);

            preferences.putFloat(PK_GAMMA_BRIGHTNESS, 2.2f);  // Gamma-Korrektur für Helligkeit
                                                              // gamma correction for brightness

            // putLong statt putUInt: dieser Key wird ueberall sonst mit
            // putLong()/getLong() angefasst - ein abweichender NVS-Typ fuehrt
            // sonst zu ESP_ERR_NVS_TYPE_MISMATCH und stillem Default-Rueckfall.

            // putLong instead of putUInt: this key is handled with
            // putLong()/getLong() everywhere else - a mismatched NVS type
            // would cause ESP_ERR_NVS_TYPE_MISMATCH and a silent default.

            preferences.putLong(PK_CENTER_COLOR, 0xEC0016);

            preferences.putUInt(PK_CENTER_SIZE, displayGeom->centerSize); // Nabengroesse je Displaytyp (DISPLAY_GEOMETRY)
                                                                          // hub size per display type (DISPLAY_GEOMETRY)

            preferences.putUChar(PK_TFT_ROTATION1, TFT_ROTATION1_DEFAULT);
            preferences.putBool(PK_ADC_INVERTED, false);

            preferences.putBool(PK_LOGGING_ENABLED, true);

            preferences.end();
            preferences.begin("clock", false);
        }

        // tftRotation1/2 schon HIER laden - DRAW_ON_BOTH_DISPLAYS() braucht sie fuer den ersten fillScreen().
        // Migration: alten Key "tftRotation" nach "tftRotation1" uebernehmen, falls dort noch nichts steht.

        // Load tftRotation1/2 HERE already - DRAW_ON_BOTH_DISPLAYS() needs them for the first fillScreen().
        // Migration: copy the old key "tftRotation" to "tftRotation1" if nothing is stored there yet.

        if (!preferences.isKey(PK_TFT_ROTATION1) && preferences.isKey(PK_TFT_ROTATION_LEGACY)) {
            preferences.putUChar(PK_TFT_ROTATION1, preferences.getUChar(PK_TFT_ROTATION_LEGACY, 0));
        }

        // Werte 0-3 und TFT_ROTATION_NA sind gueltig; alles darueber sind
        // Altwerte in Grad (90/180/270) und werden auf 0-3 umgerechnet.

        // Values 0-3 and TFT_ROTATION_NA are valid; anything above is a legacy
        // value in degrees (90/180/270) and gets converted to 0-3.

        tftRotation1 = preferences.getUChar(PK_TFT_ROTATION1, TFT_ROTATION1_DEFAULT);
        if (tftRotation1 > TFT_ROTATION_NA) {
            if (tftRotation1 == 90) tftRotation1 = 1;
            else if (tftRotation1 == 180) tftRotation1 = 2;
            else if (tftRotation1 == 270) tftRotation1 = 3;
            else tftRotation1 = TFT_ROTATION1_DEFAULT;
            preferences.putUChar(PK_TFT_ROTATION1, tftRotation1);
        }

        // Rotation von Display 2 (CS2) - unabhaengig von tftRotation1, damit
        // beide Displays unterschiedlich ausgerichtet montiert sein koennen.

        // rotation of Display 2 (CS2) - independent of tftRotation1, so
        // both displays can be mounted with a different orientation.

        tftRotation2 = preferences.getUChar(PK_TFT_ROTATION2, TFT_ROTATION2_DEFAULT);
        if (tftRotation2 > TFT_ROTATION_NA) {
            if (tftRotation2 == 90) tftRotation2 = 1;
            else if (tftRotation2 == 180) tftRotation2 = 2;
            else if (tftRotation2 == 270) tftRotation2 = 3;
            else tftRotation2 = TFT_ROTATION2_DEFAULT;
            preferences.putUChar(PK_TFT_ROTATION2, tftRotation2);
        }


        loadLanguage(); // liest currentLanguage aus den Preferences (siehe translation.h)
                        // reads currentLanguage from preferences (see translation.h)

        wifiActive = preferences.getBool(PK_WIFI_ACTIVE, true);


        // NTP-Server initialisieren

        // Initialize NTP servers

        initializeNtpServers();

        stationMode = preferences.getBool(PK_STATION_MODE, true);
        smoothMinute = preferences.getBool(PK_SMOOTH_MINUTE, false);

        // Fallback bewusst stationMode: vor der Trennung von stationMode und smoothSecond gab es schwingend
        // nur mit stationMode - Uhren ohne gespeichertes smoothSecond behalten so ihr bisheriges Aussehen.

        // Fallback deliberately stationMode: before stationMode and smoothSecond were split, smooth motion
        // only existed with stationMode - clocks without a stored smoothSecond keep their previous look this
        // way.

        smoothSecond = getSmoothSecondPref(stationMode);
        showSecondHand = preferences.getBool(PK_SHOW_SECOND_HAND, true);

        // CS2-Pin wird immer als Output konfiguriert: Status-/Startmeldungen
        // bedienen beide Displays, und Display 2 laesst sich zur Laufzeit
        // (Rotation von "n.a." auf einen Winkel) ohne Neustart aktivieren.

        // The CS2 pin is always configured as output: status/boot messages
        // serve both displays, and display 2 can be enabled at runtime
        // (rotation from "n.a." to an angle) without a reboot.

        pinMode(CS_2, OUTPUT);

        // Startzustand: erstes angeschlossenes Display ausgewaehlt, das andere
        // abgewaehlt (siehe setCSIdle() in display.h).

        // Starting state: first connected display selected, the other one
        // deselected (see setCSIdle() in display.h).

        setCSIdle();

        // Nabe

        // Hub

        uint32_t hubColorRgb = preferences.getLong(PK_CENTER_COLOR, 0xEC0016); //DB-Rot
                                                                               // DB red
        hubColor = tft.color565((hubColorRgb >> 16) & 0xFF, (hubColorRgb >> 8) & 0xFF, hubColorRgb & 0xFF);
        hubSize = preferences.getUInt(PK_CENTER_SIZE, 6);

        // Auf 0-100 begrenzen: fruehere Backlight-Builds speicherten 255, das
        // Formularfeld (max 100) liesse sich damit gar nicht mehr absenden.

        // Clamp to 0-100: former backlight builds stored 255, which would make
        // the form field (max 100) impossible to submit.

        lowThreshold = constrain(preferences.getInt(PK_LOW_THRESHOLD, 40), 0, 100);
        highThreshold = constrain(preferences.getInt(PK_HIGH_THRESHOLD, 60), 0, 100);
        minBrightness = preferences.getUChar(PK_MIN_BRIGHTNESS, 100);
        maxBrightness = preferences.getUChar(PK_MAX_BRIGHTNESS, 255);

        // Zeitabhängige Helligkeit aus Preferences

        // Time-dependent brightness from preferences

        brightStartHour = preferences.getUChar(PK_BRIGHT_START_HOUR, 7);
        brightEndHour = preferences.getUChar(PK_BRIGHT_END_HOUR, 21);

        adcInverted = preferences.getBool(PK_ADC_INVERTED, false);

        rocrailEnabled = preferences.getBool(PK_ROCRAIL_ENABLED, false);
        rocrailServerHost = preferences.getString(PK_ROCRAIL_SERVER, "");
        rocrailServerPort = preferences.getUShort(PK_ROCRAIL_SRV_PORT, ROCRAIL_DEFAULT_PORT);
        loadRocrailServerList();


        gammaBrightness = preferences.getFloat(PK_GAMMA_BRIGHTNESS, 2.2f);  // Gamma-Korrektur für Helligkeit
                                                                            // gamma correction for brightness

        // Hintergrundbeleuchtung ja/nein - vor updateBrightness() weiter
        // unten, da setPixelBrightness()/updateBrightness() davon abhaengen.

        // Backlight yes/no - before updateBrightness() further below, since
        // setPixelBrightness()/updateBrightness() depend on it.

        useBacklight = preferences.getBool(PK_USE_BACKLIGHT, BACKLIGHT_DEFAULT);

        pinMode(BUTTON1, INPUT_PULLDOWN);

        // auf Fotowiderstand prüfen

        // Check for photoresistor

        uint16_t adcMin = 0;
        uint16_t adcMax = 0;
        uint16_t adcActual = 0;

        analogReadResolution(12);

        // ADC +3,3V / GND über GPIO

        // ADC +3.3V / GND via GPIO

        pinMode(ADC_GND, OUTPUT);
        pinMode(ADC_3V, OUTPUT);

        digitalWrite(ADC_GND, LOW);
        digitalWrite(ADC_3V, LOW);
        delay(10);
        adcMin = analogRead(ADC_PIN);

        digitalWrite(ADC_GND, HIGH);
        digitalWrite(ADC_3V, HIGH);
        delay(10);
        adcMax = analogRead(ADC_PIN);

        digitalWrite(ADC_GND, LOW);
        digitalWrite(ADC_3V, HIGH);
        delay(10);
        adcActual = analogRead(ADC_PIN);

        DEBUG_PRINTF("[ADC] min: %d max: %d act: %d", adcMin, adcMax, adcActual);
        if (loggingEnabled) Serial.println("");

        if (abs(adcMin - adcMax) > 1000) {
            DEBUG_PRINTLN("[ADC] Found photoresistor");
            useAdc = true;
            photoresistorFound = true;

            // evtl. überschreiben

            // possibly override

            useAdc = preferences.getBool(PK_USE_ADC, true);
        }
        if (!useAdc) {
            pinMode(ADC_GND, INPUT);
            pinMode(ADC_3V, INPUT);
            useAdc = false;
        }


        minBrightness = preferences.getUChar(PK_MIN_BRIGHTNESS, 100);
        maxBrightness = preferences.getUChar(PK_MAX_BRIGHTNESS, 255);

        updateBrightness();

        if (loggingEnabled)  Serial.println("debug is " + String(loggingEnabled ? "enabled" : "disabled"));

        // Beide Chips fuer tft.init() gleichzeitig selektieren - auch ein als
        // "n.a." markiertes Display wird so initialisiert (Startmeldungen laufen
        // auf beiden) und ist spaeter ohne Neustart aktivierbar.

        // Select both chips at once for tft.init() - a display marked "n.a."
        // is initialized as well (boot messages run on both) and can be
        // enabled later without a reboot.

        digitalWrite(CS_1, LOW);
        digitalWrite(CS_2, LOW);

        tft.selectPanel(displayType == DISPLAY_TYPE_GC9D01); // GC9D01 mit eigenem Treiber (lgfx_config.h)
                                                            // GC9D01 with its own driver (lgfx_config.h)
        resetPanels();
        tft.init();
        tftInitialized = true; // ab jetzt duerfen setCS1()/setCS2() die Rotation am Chip setzen
                               // from now on setCS1()/setCS2() may set the rotation on the chip

        // GLCD-Schrift wie bisher, aber als echter CP437-Zeichensatz ohne
        // UTF-8-Dekodierung - tftText() (display.h) liefert Umlaute/Akzente
        // passend dazu als CP437-Bytes.

        // GLCD font as before, but as a real CP437 charset without UTF-8
        // decoding - tftText() (display.h) delivers umlauts/accents as
        // matching CP437 bytes.

        setupTextStyle(tft);

        delay(75);
        DRAW_ON_BOTH_DISPLAYS(
            tft.fillScreen(TFT_BLACK);
        );


        // tftRotation1/2 sind hier bereits geladen (Begruendung siehe oben,
        // vor tft.init()).

        // tftRotation1/2 are already loaded here (reasoning above,
        // before tft.init()).

        selectedBackground = preferences.getString(PK_BACKGROUND, "/face_default.bmp");

        validateSelectedBackground();

        // Rotation setzen: mit gc9d01SwRotation per Software, sonst in Hardware - jedes Display behaelt sein
        // eigenes MADCTL (tftRotation1/2), setCS1()/setCS2() waehlen es aus. Bei "n.a." 0 Grad statt 4 (waere
        // beim GC9A01 gespiegelt), damit Startmeldungen auf beiden Displays richtig stehen.

        // Set the rotation: in software with gc9d01SwRotation, otherwise in hardware - each display keeps its
        // own MADCTL (tftRotation1/2), setCS1()/setCS2() select it. For "n.a." 0 degrees instead of 4
        // (mirrored on the GC9A01), so boot messages look right on both displays.

        setCS2(LOW);
        setCS1(LOW);
        setCSIdle();


        // Backlight-PWM anhaengen, falls eingeschaltet - mit currentBrightness, die updateBrightness() schon
        // ermittelt hat. Ohne Backlight-Regelung liegt Pin 3 fest auf HIGH.

        // Attach the backlight PWM if enabled - with currentBrightness, already determined by
        // updateBrightness(). Without backlight control pin 3 is driven HIGH.

        applyBacklightPin();


        // Rueckgabewert pruefen: schlaegt die Allokation fehl, werden spaetere
        // pushImage()/pushSprite()-Aufrufe stillschweigend No-Ops - beide
        // Displays blieben schwarz, ohne Absturz oder Log-Hinweis.

        // Check the return value: if allocation fails, later pushImage()/
        // pushSprite() calls silently become no-ops - both displays stay
        // black, with no crash or log hint.

        if (!createSprite16(backgroundSprite, CLOCK_WIDTH, CLOCK_HEIGHT)) {
            DEBUG_PRINTLN("[Display] FATAL: couldnt allocate backgroundSprite - clock face cannot be drawn");
        }

        createSprite16(hourHandSprite, HAND_WIDTH, HAND_HEIGHT);
        hourHandSprite.setPivot(HAND_WIDTH / 2, HAND_PIVOT_Y);

        createSprite16(minuteHandSprite, HAND_WIDTH, HAND_HEIGHT);
        minuteHandSprite.setPivot(HAND_WIDTH / 2, HAND_PIVOT_Y);

        createSprite16(secondHandSprite, HAND_WIDTH, HAND_HEIGHT);
        secondHandSprite.setPivot(HAND_WIDTH / 2, HAND_PIVOT_Y);

        loadClockFace();
        loadHandSprites();

        // Bei gueltiger RTC die Uhrzeit sofort zeigen - noch vor den WLAN-
        // Verbindungsversuchen unten (je bis zu 15-30s je Anlauf).

        // If the RTC is valid, show the time immediately - even before the WiFi
        // connection attempts below (each up to 15-30s).

        if (rtcOk == RTC_AVAILABLE) {
            updateClock();
        }

        setupWebServer();
        webserver.begin();

        // DCF77-Interrupt einrichten - dcf.Start() entfaellt, die Bibliothek
        // wird nicht mehr benutzt (siehe isr() in time_sync.h).

        // Set up the DCF77 interrupt - dcf.Start() is gone, the library
        // is no longer used (see isr() in time_sync.h).

        pinMode(DCF77_DATAPIN, INPUT_PULLUP);
        attachInterrupt(DCF77_DATAPIN, isr, CHANGE);

        // Bit-Puffer fuer /dcf77 auf "unbekannt" (-1) initialisieren -
        // int8_t-Arrays lassen sich nicht per Deklaration vorbelegen.

        // Initialize the /dcf77 bit buffer to "unknown" (-1) -
        // int8_t arrays can't be pre-filled via declaration.

        for (uint8_t i = 0; i < DCF77_GRID_SLOTS; i++) dcf77Bits[i] = -1;

        // Wenn Button1 oder BOOT_BUTTON gedrückt ist, alle Zugangsdaten löschen

        // If Button1 or BOOT_BUTTON is pressed, clear all credentials

        if (digitalRead(BUTTON1) == HIGH || digitalRead(BOOT_BUTTON) == LOW) {
            DEBUG_PRINTLN("[SETUP] Reset button pressed, clearing WiFi credentials and starting AP..");
            DRAW_ON_BOTH_DISPLAYS(
                tft.fillScreen(TFT_RED);
                tft.setTextColor(TFT_WHITE);
                tft.setTextSize(TFT_TEXT_SIZE);
                tft.setCursor(20, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 4));
                tft.println(tftText(translate("Reset WLan...")));
            );
            delay(1000);

            // preferences.end()/begin() nicht pro Eintrag noetig: putString()
            // committet ohnehin einzeln - vgl. eraseWiFiConfig() in
            // wifi_manager.h.

            // No need for preferences.end()/begin() per entry: putString()
            // already commits individually - compare eraseWiFiConfig() in
            // wifi_manager.h.

            for (int i = 0; i < MAX_WLAN; i++) {
                wifiSsid[i] = "";
                wifiPass[i] = "";
                String ssidKey = pkSsid(i);
                String passKey = pkPass(i);
                preferences.putString(ssidKey.c_str(), "");
                preferences.putString(passKey.c_str(), "");
            }
        }

        connectWiFiAtBoot();

        // Bei konfiguriertem Rocrail-Server nach dem Neustart sofort einen
        // Verbindungsversuch anstossen, statt bis zum ersten Zeitfenster zu
        // warten - no-op, wenn Rocrail aus ist, kein Server hinterlegt ist, oder im AP-/Einrichtungsmodus.

        // With a configured Rocrail server, kick off a connection attempt
        // right after a restart, instead of waiting for the first regular
        // window - a no-op if Rocrail is off, no server is configured, or in AP/setup mode.

        if (WiFi.getMode() == WIFI_STA) {
            triggerRocrailConnectNow();
        }

        // NTP hat Vorrang, DCF77 ist der Fallback; Erfolg zaehlt ueber lastNtpSuccessMillis. Die Sync laeuft
        // asynchron - der Start wartet nicht auf DNS/UDP, pollNtpSyncTask() wertet aus. SNTP synct danach
        // selbst weiter, SMOOTH gleicht Korrekturen per adjtime() an statt zu springen.

        // NTP has priority, DCF77 is the fallback; success counts via lastNtpSuccessMillis. The sync runs
        // asynchronously - boot does not wait on DNS/UDP, pollNtpSyncTask() evaluates. SNTP keeps syncing on
        // its own, SMOOTH eases corrections in via adjtime() instead of stepping.

        sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
        sntp_set_sync_interval(WAIT_6h);

        startNtpSyncTask("Initial sync");

        loadPresets();

        // NTP-Server unabhaengig von rtcOk starten - loop() prueft vor jeder Antwort ohnehin, ob eine
        // gueltige Zeit da ist. startNtpServer() statt udp.begin(): prueft das Ergebnis und wird nach jedem
        // Reconnect neu aufgerufen.

        // Start the NTP server independent of rtcOk - loop() checks for a valid time before every reply
        // anyway. startNtpServer() instead of udp.begin(): checks the result and is called again after every
        // reconnect.

        startNtpServer();

        // R2RNet-Multicast-Diagnose (nur Analyse, keine Discovery), nach jedem Reconnect neu starten wie
        // startNtpServer(). Nur mit aktiviertem Rocrail - der Rocrail-Schalter startet/stoppt sie auch zur
        // Laufzeit.

        // R2RNet multicast diagnostics (analysis only, no discovery), restart after every reconnect like
        // startNtpServer(). Only with Rocrail enabled - the Rocrail switch also starts/stops it at runtime.

        if (rocrailEnabled) {
            startR2rnetDebugListener();
        }

        DEBUG_PRINTLN("[SETUP] Boot complete, free heap: " + String(ESP.getFreeHeap()) + " bytes");
        checkHeapWarning("Setup Ende");

        setLedOff();
    }


    // Main-Loop

    // Main loop

    void loop() {

        // updateClock() bedient beide Displays je Tick samt eigener Rotation. WPS per Web-Button startet
        // verzoegert hier, nicht im Handler von /api/startWPS - so wird die Zielseite des Redirects sofort
        // ausgeliefert.

        // updateClock() serves both displays each tick with their own rotation. WPS via the web button starts
        // here with a delay, not in the /api/startWPS handler - so the redirect's target page is served right
        // away.

        if (wpsStartRequested && millis() - wpsStartRequestedAtMillis >= (2 * WAIT_1s)) {
            wpsStartRequested = false;
            startWPS();
            wpsPending = true;
            wpsStartMillis = millis();
        }

        // WPS per Web-Button asynchron pruefen - blockiert loop() nicht und reagiert auf die Flags aus dem
        // WiFi-Event statt WiFi.status() abzufragen.

        // Check a WPS request from the web button asynchronously - does not block loop() and reacts to the
        // flags from the WiFi event instead of polling WiFi.status().

        if (wpsPending) {
            if (wpsSuccessEvent) {
                wpsSuccessEvent = false;
                wpsPending = false;

                // WPS erfolgreich: Zugangsdaten sichern und neu starten. esp_wifi_get_config() liefert direkt
                // nach dem Event manchmal leere Daten (Bug #10339/#11705) - daher mehrfach mit Pause
                // versuchen.

                // WPS succeeded: save the credentials and reboot. esp_wifi_get_config() sometimes returns
                // empty data right after the event (bug #10339/#11705) - so retry with a short delay.

                String newSsid = "";
                String newPass = "";
                for (int wpsReadAttempt = 0; wpsReadAttempt < 20 && newSsid == ""; wpsReadAttempt++) {
                    wifi_config_t wpsResultConfig;
                    if (esp_wifi_get_config(WIFI_IF_STA, &wpsResultConfig) == ESP_OK) {
                        char ssidBuf[33] = { 0 };
                        char passBuf[65] = { 0 };
                        memcpy(ssidBuf, wpsResultConfig.sta.ssid, sizeof(wpsResultConfig.sta.ssid));
                        memcpy(passBuf, wpsResultConfig.sta.password, sizeof(wpsResultConfig.sta.password));
                        newSsid = String(ssidBuf);
                        newPass = String(passBuf);
                    }
                    if (newSsid == "") delay(100); // kurz warten, dann erneut versuchen
                                                   // wait briefly, then retry
                }
                DEBUG_PRINTLN("[WPS] Captured SSID '" + newSsid + "', password length: " + String(newPass.length()));

                if (newSsid != "") {
                    saveWpsCredentials(newSsid, newPass);
                }
                else {
                    DEBUG_PRINTLN("[WPS] Could not read back SSID after retries - nothing saved");
                }

                esp_wifi_wps_disable();
                wpsPreviousSsid = "";
                setLedOff(); // Blink-Signalisierung unten beenden, sauberer Zustand vor dem Neustart
                             // end the blink signaling below, clean state before the restart
                DEBUG_PRINTLN("[WPS] Restarting to reconnect via the normal boot sequence..");
                delay(WAIT_1s);
                espReboot();
            }
            else if (wpsFailedEvent) {
                DEBUG_PRINTLN("[WPS] WPS failed or timed out (event)");
                wpsFailedEvent = false;
                esp_wifi_wps_disable();
                wpsPending = false;
                setLedOff(); // Blink-Signalisierung unten beenden
                             // end the blink signaling below

                // Ggf. urspruengliche Verbindung wiederherstellen, falls durch
                // den WPS-Versuch getrennt (siehe restorePreviousWpsConnection()
                // in wifi_manager.h - gemeinsame Logik fuer diesen und den Timeout-Zweig unten).

                // Restore the original connection if it was dropped by the
                // WPS attempt, if applicable (see restorePreviousWpsConnection()
                // in wifi_manager.h - shared logic for this and the timeout branch below).

                restorePreviousWpsConnection();
            }
            else if (millis() - wpsStartMillis > (2 * WAIT_1m)) {
                DEBUG_PRINTLN("[WPS] Timeout waiting for WPS button press - disabling WPS");
                esp_wifi_wps_disable();
                wpsPending = false;
                setLedOff(); // Blink-Signalisierung unten beenden
                             // end the blink signaling below
                restorePreviousWpsConnection();
            }
            else {

                // Warten auf den Router: die Status-LED blinkt als Lebenszeichen - das DCF77-Blinken ist
                // dabei zurueckgestellt, damit sich beide nicht ueberlagern.

                // Waiting for the router: the status LED blinks as a sign of life - the DCF77 blink is held
                // back meanwhile, so the two do not overlap.

                static unsigned long lastWpsBlinkMillis = 0;
                static bool wpsLedOn = false;
                if (millis() - lastWpsBlinkMillis >= 300) {
                    lastWpsBlinkMillis = millis();
                    wpsLedOn = !wpsLedOn;
                    if (wpsLedOn) setLedOn(); else setLedOff();
                }
            }
        }

        if (WiFi.getMode() == WIFI_STA && WiFi.isConnected()) {
            ipAddress = WiFi.localIP().toString();
        }
        else {
            ipAddress = WiFi.softAPIP().toString();
        }

        // DCF77-Empfangsstatus aktuell halten (lastDcfSyncTime/dcfTimeFound) -
        // die eigentliche Zeituebernahme passiert getrennt davon, periodisch
        // mit NTP-Vorrang (siehe Sync-Logik weiter unten).

        // Keep the DCF77 reception status up to date (lastDcfSyncTime/
        // dcfTimeFound) - the actual time takeover happens separately,
        // periodically with NTP priority (see the sync logic further below).

        updateDcf77Status();

        // Empfangsausfall bzw. RTC-Ausfall waehrend des Betriebs erkennen -
        // unabhaengig vom WLAN-Status, damit der Topbar-Live-Status
        // (/api/topbarStatus in webserver_routes.h) auch dann aktuell bleibt.

        // Check for a reception failure resp. RTC failure during operation -
        // independent of WiFi state, so the topbar live status
        // (/api/topbarStatus in webserver_routes.h) stays current either way.

        checkDcf77Health();
        checkRtcHealth();

        // Eigener DCF77-Bit-Fortschritt fuer /dcf77 - unabhaengig vom WLAN-
        // Status abgearbeitet, damit der ISR-Ringpuffer (isr() in time_sync.h)
        // auch ohne WLAN zeitnah geleert wird.

        // Own DCF77 bit progress for /dcf77 - handled independent of WiFi
        // status, so the ISR's ring buffer (isr() in time_sync.h) is
        // drained promptly even without WiFi.

        processDcf77Bits();

        // Periodische Zeitsynchronisation wie beim Start: NTP hat Vorrang, DCF77 ist der Fallback. Intervall
        // WAIT_6h - die Sommer-/Winterzeit-Umstellung haengt nicht davon ab.

        // Periodic time sync like at boot: NTP has priority, DCF77 is the fallback. Interval WAIT_6h - the
        // DST transition does not depend on it.

        if (millis() - lastNTPUpdate > WAIT_6h) {
            if (timeinfo.tm_sec < 10 || timeinfo.tm_sec > 55) {

                // 15s verschieben ohne den Zeitstempel in die Zukunft zu
                // setzen: "millis() + 15000" fuehrte zu Unterlauf/sofortigem
                // erneuten Trigger. Diese Form ist modulo-/ueberlaufsicher.

                // Postpone by 15s without setting the timestamp into the
                // future: "millis() + 15000" caused underflow and an
                // immediate re-trigger. This form is overflow-safe.

                lastNTPUpdate = millis() - WAIT_6h + 15 * 1000;
            }
            else {

                // Startet nur die Task (siehe time_sync.h) - das Ergebnis
                // wertet pollNtpSyncTask() weiter unten in loop() aus, sobald
                // sie fertig ist, statt hier auf DNS/UDP zu warten.

                // Only starts the task (see time_sync.h) - the result is
                // evaluated by pollNtpSyncTask() further below in loop() once
                // it finishes, instead of waiting on DNS/UDP here.

                startNtpSyncTask("Periodic sync");
                lastNTPUpdate = millis();
            }

        }

        // Wertet eine ggf. laufende NTP-Sync-Task aus (Boot-, periodischer
        // oder manueller Sync ueber /syncnow) - no-op, solange keine Task
        // fertig ist. Siehe startNtpSyncTask()/pollNtpSyncTask() in time_sync.h.

        // Evaluates any running NTP sync task (boot, periodic, or manual
        // sync via /syncnow) - no-op as long as no task has finished. See
        // startNtpSyncTask()/pollNtpSyncTask() in time_sync.h.

        pollNtpSyncTask();

        // Sicherheits-Abschaltung der LED einmal pro Minute, falls ein Blitz
        // haengen bleibt. Ueber millis() statt timeinfo.tm_sec, damit es auch
        // ohne gueltige Zeit greift (waehrend der DCF77-Suche).

        // Safety switch-off of the LED once per minute, in case a flash gets
        // stuck. Uses millis() instead of timeinfo.tm_sec so it works even
        // without a valid time (during DCF77 acquisition).

        static unsigned long lastLedSafetyOffMillis = 0;
        if (millis() - lastLedSafetyOffMillis >= WAIT_1m) {
            lastLedSafetyOffMillis = millis();
            dcfLedOffAtMillis = 0;
            setLedOff();
        }

        // Wenn im AP-Modus: DNS-Requests abarbeiten (captive portal)

        // In AP mode: process DNS requests (captive portal)

        if (softAPIP) {
            dnsServer.processNextRequest();
        }


        webserver.handleClient();

        // Displaytyp per USB von flashESP.bat/.sh (siehe display.h)

        // Display type via USB from flashESP.bat/.sh (see display.h)

        handleSerialCommands();


        // DCF77-LED HIER statt in der ISR schalten (pinMode()/digitalWrite() liegen im Flash). Fester Blitz
        // je Impuls (DCF77_LED_BLINK_MS) statt Umschalten - sonst blieb die LED bei ungerader Impulszahl an.

        // Switch the DCF77 LED HERE instead of in the ISR (pinMode()/digitalWrite() live in flash). A fixed
        // flash per pulse (DCF77_LED_BLINK_MS) instead of toggling - otherwise the LED stayed on with an odd
        // pulse count.

        if (dcfLedTogglePending) {
            dcfLedTogglePending = false;
            if (!dcfTimeFound && dcfSyncLedEnabled) {
                setLedOn();
                dcfLedOffAtMillis = millis() + DCF77_LED_BLINK_MS;
                if (dcfLedOffAtMillis == 0) dcfLedOffAtMillis = 1; // 0 bedeutet "kein Blitz aktiv"
                                                                   // 0 means "no flash active"
            }
        }

        // Vorzeichenbehafteter Vergleich: so bleibt die Abschaltung auch beim
        // millis()-Ueberlauf (nach ca. 49 Tagen) korrekt.

        // Signed comparison: keeps the switch-off correct across the millis()
        // overflow (after about 49 days) as well.

        if (dcfLedOffAtMillis != 0 && (long)(millis() - dcfLedOffAtMillis) >= 0) {
            dcfLedOffAtMillis = 0;
            setLedOff();
        }

        // Im Access-Point-Modus nach AP_INFO_SHOW_MS ebenfalls die Uhr zeigen und beim Wechsel einmal
        // komplett neu zeichnen. Eigener Zeitstempel statt softAPIPstart - den setzt die Weboberflaeche bei
        // jedem Zugriff zurueck.

        // In access point mode also show the clock after AP_INFO_SHOW_MS and redraw completely once on the
        // switch. Own timestamp instead of softAPIPstart - the web interface resets that on every access.

        static unsigned long apSinceMillis = 0;
        static bool apSeen = false;
        if (softAPIP && !apSeen) {
            apSeen = true;
            apSinceMillis = millis();
        }
        bool apClockDue = softAPIP && (millis() - apSinceMillis >= AP_INFO_SHOW_MS);
        static bool apClockShown = false;
        if (apClockDue && !apClockShown) {
            apClockShown = true;
            firstRun = true;
            firstRun2 = true;
            clockFrameDirty[0] = true;
            clockFrameDirty[1] = true;
        }

        if (WiFi.getMode() == WIFI_STA || rtcOk == RTC_AVAILABLE || dcfTimeFound || apClockDue) {

        // checkFactoryResetCodePending() zeichnet bei Bedarf den Bestaetigungscode und haelt das Zifferblatt
        // so lange an, damit der Code lesbar bleibt.

        // checkFactoryResetCodePending() draws the confirmation code when needed and pauses the clock face
        // meanwhile, so the code stays readable.

        if (!checkFactoryResetCodePending()) {
            updateClock();
        }

        checkWeeklyRestart();

        // Nicht waehrend WPS (wpsPending): das WLAN trennt sich dabei kurz, ein eigener Reconnect-Versuch
        // kollidierte sonst mit der laufenden WPS-Verhandlung um denselben Funkchip.

        // Not during WPS (wpsPending): WiFi drops briefly then, and a reconnect attempt would collide with
        // the running WPS negotiation over the same radio.

        if (wifiActive && !WiFi.isConnected() && !wpsPending) {
            checkWiFiReconnect();
        }

        // Nur im normalen STA-Betrieb sinnvoll - im AP-/Einrichtungsmodus
        // ist kein Rocrail-Server im Heimnetz erreichbar. pollRocrailClient()
        // ist selbst ein no-op, solange rocrailEnabled aus ist.

        // Only meaningful in normal STA operation - no Rocrail server on the
        // home network is reachable in AP/setup mode. pollRocrailClient()
        // itself is a no-op as long as rocrailEnabled is off.

        if (WiFi.getMode() == WIFI_STA) {
            pollRocrailClient();

            // R2RNet-Multicast-Diagnose (siehe rocrail_client.h) - non-blocking,
            // no-op solange kein Paket wartet oder die Gruppe nicht beigetreten
            // werden konnte (r2rnetDebugListening).

            // R2RNet multicast diagnostics (see rocrail_client.h) - non-
            // blocking, no-op as long as no packet is waiting or the group
            // couldn't be joined (r2rnetDebugListening).

            pollR2rnetDebugListener();
        }

        //  checkWiFiScan(); // Überprüfe den Status des Scans
        // NTP-/DCF77-Zeitsynchronisation laeuft jetzt periodisch weiter oben
        // (unabhaengig vom WLAN-Status abgearbeitet) - hier daher nichts

        // mehr zu tun.
        // NTP/DCF77 time sync now runs periodically further above (handled
        // independent of WiFi status) - nothing left to do here.

        initial = false;

        }

          // NTP-Server-Anfragen beantworten (Test: w32tm /stripchart /computer:<ip>) -
          // die Bedingung prueft die SYSTEMZEIT, nicht die Zeitquelle, damit
          // jede gueltige Quelle (NTP/DCF77/RTC) antwortet. Schwelle wie setupNTP(): Jahr > 2016.

          // Answer NTP server requests (test: w32tm /stripchart /computer:<ip>) -
          // the condition checks the SYSTEM TIME, not the time source, so
          // any valid source (NTP/DCF77/RTC) triggers a reply. Threshold as in setupNTP(): year > 2016.

        if (ntpServerRunning) {
            int packetSize = udp.parsePacket();
            if (packetSize) {

                // Empfangszeitpunkt SOFORT festhalten, vor allem Weiteren -
                // er geht als Receive-Timestamp in die Antwort ein und ist die
                // Grundlage, aus der der Client Laufzeit und Offset berechnet.

                // Capture the receive instant IMMEDIATELY, before anything else
                // - it goes into the reply as the receive timestamp and is what
                // the client uses to compute delay and offset.

                struct timeval receivedAt;
                gettimeofday(&receivedAt, nullptr);

                ntpRequestsReceived++;

                IPAddress clientIP = udp.remoteIP();

                // Puffer vorher leeren: udp.read() fuellt nur ankommende Bytes -
                // sonst blieben bei kurzen Anfragen Reste der VORIGEN Anfrage in
                // Byte 40-47 stehen (createNtpResponse() spiegelt sie zurueck).

                // Clear the buffer first: udp.read() only fills the bytes that
                // actually arrived - otherwise leftovers of the PREVIOUS
                // request would remain in bytes 40-47 (mirrored by createNtpResponse()).

                memset(ntpPacket, 0, NTP_PACKET_SIZE);
                udp.read(ntpPacket, NTP_PACKET_SIZE);

                // Nur Anfragen aus einem privaten Netz beantworten (isPrivateNetworkIp(), derzeit
                // abgeschaltet) - ein offener NTP-Server ist ein klassischer Verstaerker fuer DDoS-Angriffe
                // mit gefaelschter Absender-IP.

                // Only answer requests from a private network (isPrivateNetworkIp(), currently disabled) - an
                // open NTP server is a classic amplifier for DDoS attacks with a spoofed source IP.

                if (!isPrivateNetworkIp(clientIP)) {
                    DEBUG_PRINTLN("[NTPD] Request from " + clientIP.toString() + " ignored - not a private network");
                }

                // Ohne gueltige Systemzeit waere jede Antwort schlimmer als
                // keine (Client stellt sich auf 1970) - Anfrage dann verwerfen,
                // Client faellt auf seinen Timeout/naechste Quelle zurueck.

                // Without a valid system time, any answer is worse than none
                // (client would set itself to 1970) - discard the request,
                // the client falls back to its timeout/next source.

                else if (receivedAt.tv_sec <= 1483228800L) { // 2017-01-01
                    DEBUG_PRINTLN("[NTPD] Request from " + clientIP.toString() +
                                  " ignored - no valid system time yet");
                }
                else {
                    createNtpResponse(ntpPacket, receivedAt);

                    udp.beginPacket(udp.remoteIP(), udp.remotePort());
                    udp.write(ntpPacket, NTP_PACKET_SIZE);
                    udp.endPacket();

                    ntpRepliesSent++;
                    DEBUG_PRINTLN("[NTPD] Answered request from " + clientIP.toString());
                }
            }
        }

        checkButton();
        updateBrightness();
        checkLogFlush();

        // Nach 15 Minuten im AP-Modus neu starten, damit die Uhr wieder einen WLAN-Versuch macht (z.B. Router
        // wieder da). Nur mit gespeicherten Netzen - ohne gibt es nichts zu versuchen, der Neustart
        // unterbraeche nur die laufende Uhr.

        // Restart after 15 minutes in AP mode, so the clock tries WiFi again (e.g. router back). Only with
        // stored networks - without, there is nothing to retry and the restart would only interrupt the
        // running clock.

        bool anyWifiStored = false;
        for (int i = 0; i < MAX_WLAN; i++) {
            if (wifiSsid[i].length() > 0) { anyWifiStored = true; break; }
        }
        if (softAPIP == true && anyWifiStored) {
            if (millis() - softAPIPstart > WAIT_15m) {
                DEBUG_PRINTLN("[WiFi] 15 minutes in AP mode without configuration - restarting to retry WiFi");
                espReboot();
            }
        }

        // Kein unbedingtes setLedOff() mehr am Zeilenende von loop() (loeschte
        // den DCF77-Blitz sofort) - Abschaltung erfolgt jetzt ueber
        // DCF77_LED_BLINK_MS und die Sicherheits-Abschaltung oben.

        // No more unconditional setLedOff() at the end of loop() (it cleared
        // the DCF77 flash immediately) - switch-off now happens via
        // DCF77_LED_BLINK_MS and the safety switch-off above.

    }


