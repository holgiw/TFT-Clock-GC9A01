    // howl@gmx.de - Stationsuhr
    // ESP32-S2 Mini (Lolin S2 Pico), LittleFS, TFT GC9A01/GC9D01, TFT_eSPI 2.5.34
    // DCF77-Modul: https://de.elv.com/p/elv-dcf-empfangsmodul-dcf-2-P091610/

    // howl@gmx.de - station clock
    // ESP32-S2 Mini (built as Lolin S2 Pico), LittleFS, TFT GC9A01/GC9D01, TFT_eSPI 2.5.34
    // DCF77 module: https://de.elv.com/p/elv-dcf-empfangsmodul-dcf-2-P091610/

    
#include <WiFi.h>
#include <WebServer.h>

#include "prefs_keys.h"
#include "build_defs.h"

// WICHTIG: TFT_eSPI-Konfiguration (Treiber/Pins/Schriften) nur in der
// Bibliothek selbst setzen (User_Setup.h) - ein #define hier wirkt NICHT.
// Siehe GC9A01-Block in config.h.

// IMPORTANT: set the TFT_eSPI config (driver/pins/fonts) only inside the
// library itself (User_Setup.h) - a #define here has NO effect.
// See the GC9A01 block in config.h.

#include "config.h"        // Board-/Display-Auswahl, Pins, Timing-Makros
                           // board/display selection, pins, timing macros
#include <TFT_eSPI.h>
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


#include "globals.h"       // globale Objekte, Variablen, Structs
                           // global objects, variables, structs
#include "declarations.h"  // Forward-Deklarationen aller Funktionen
                           // forward declarations of all functions
#include "readme_text.h"   // Projektbeschreibung (README) in drei Sprachen fuer die /info-Seite
                           // project description (README) in three languages for the /info page

#include "wifi_manager.h"      // WLAN: Verbindung, AP, Scan, Reconnect
                               // WiFi: connection, AP, scan, reconnect
#include "time_sync.h"         // RTC, DCF77, NTP
#include "rocrail_client.h"    // Rocrail-Modellzeit (Discovery, TCP-Client, Clock-Parsing)
                               // Rocrail model time (discovery, TCP client, clock parsing)
#include "display.h"           // Zifferblatt, Zeiger, Helligkeit, Touch
                               // dial, hands, brightness, touch
#include "presets_manager.h"   // Presets laden/speichern/wechseln
                               // load/save/switch presets
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
            String passKey = pkPass(i);
            wifiSsid[i] = preferences.getString(ssidKey.c_str(), "");
            wifiPass[i] = preferences.getString(passKey.c_str(), "");
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
            DEBUG_PRINTLN("[WiFi] Last connected SSID not found in scan, starting new scan..");
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
                DEBUG_PRINTLN("[WiFi] Last connected SSID not found in scan - trying it anyway (may be hidden or router not up yet)");
            }
            lastSsidTried = true;
            lastSsidResult = connectWiFi(number, true);
        }

        if (lastSsidResult != CONNECTED) {

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
                        if (connectWiFi(j, true) == CONNECTED) {
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
                    tft.println(translate("Check RTC"));
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
                    tft.println(translate("DCF77 detected"));
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
                    if (applyDcf77DecodedTime("[DCF77] boot (no WiFi/RTC)")) {
                        return;
                    }
                    DRAW_ON_BOTH_DISPLAYS(
                        tft.setCursor(20, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 8));
                        tft.print(translate("Waiting"));
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

        // CS_1 (Display-1-Chip-Select) manuell auf Output/LOW setzen - TFT_eSPI
        // steuert seinen CS-Pin nicht mehr selbst (TFT_CS = -1, siehe config.h).
        // Muss VOR tft.init() weiter unten passieren.

        // Manually set CS_1 (display 1's chip select) to output/LOW - TFT_eSPI
        // no longer drives its own CS pin (TFT_CS = -1, see config.h).
        // Must happen BEFORE tft.init() further below.
        pinMode(CS_1, OUTPUT);
        digitalWrite(CS_1, LOW);

        // CS2-Pin-Initialisierung folgt weiter unten, NACH preferences.begin()
        // (Display 2 ist fest aktiviert).

        // CS2 pin initialization follows further below, AFTER preferences.begin()
        // (display 2 is permanently enabled).

        // Asynchronen WLAN-Scan starten, damit Netzwerke schon erkannt sind, wenn
        // der Nutzer die WLAN-Einstellungen zum ersten Mal öffnet

        // Start async WiFi scan so networks are already found when the user
        // first opens the WiFi settings
        startWiFiScan();

        // Event-Handler fuer per Web-Button gestartete WPS-Anfragen registrieren
        // (siehe /api/startWPS in webserver_routes.h) - event-basiert statt
        // Status-Polling, wie im Espressif-WPS-Beispiel empfohlen.

        // Register event handler for WPS requests started via the web button
        // (see /api/startWPS in webserver_routes.h) - event-based instead of
        // status polling, as recommended in Espressif's WPS example.
        WiFi.onEvent(onWpsEvent);

        memset(&timeinfo, 0, sizeof(timeinfo));

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

        // Workaround für neuen Parameter
        // Workaround for new parameter
        String pingServer = preferences.getString(PK_PING_SERVER,"#");
        if (pingServer.startsWith("8.8.8.8") or pingServer == "#") {
            preferences.putString(PK_PING_SERVER,"1.1.1.1:80");
        }


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

        if (preferences.getString(PK_VERSION, "") != String(version)) {
            DEBUG_PRINTLN("[Preferences] Version change detected, updating version in preferences..");
            preferences.putString(PK_VERSION, String(version));
            preferences.putBool(PK_LOGGING_ENABLED, true); // Logging bei Version-Änderung aktivieren
                                                           // enable logging on version change
            deleteAllLogFiles();
        }

        // Logging aktivieren, wenn in den Preferences aktiviert
        // Enable logging if enabled in preferences
        loggingEnabled = preferences.getBool(PK_LOGGING_ENABLED, false);
        if (loggingEnabled) {
            uint16_t logfileNumber = preferences.getInt(PK_LOG_FILE_NUMBER, 0);
            logfileNumber++;
            preferences.putInt(PK_LOG_FILE_NUMBER, logfileNumber);
        }

        // LED-Blitz waehrend DCF77-Sync, abschaltbar per Checkbox
        // (dcfSyncLedEnabled in globals.h) - Default an.

        // LED flash during DCF77 sync, toggleable via checkbox
        // (dcfSyncLedEnabled in globals.h) - default on.
        dcfSyncLedEnabled = preferences.getBool(PK_DCF_SYNC_LED, true);

        DEBUG_PRINTLN("[SETUP] Initializing..");

        // I2C-Scanner starten, um RTC zu erkennen
        // Start I2C scanner to detect the RTC
#if defined SDA_PIN && defined SCL_PIN
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
#else
        rtcOk = RTC_NOT_AVAILABLE;
#endif


#if defined GC9A01 || defined (GC9A01_WITH_BACKLIGHT)
        tftType = "GC9A01";
#elif defined GC9D01
        tftType = "GC9D01";
#else
        tftType = "ILI9341"; // DEPRECATED - nicht mehr aktiv gepflegt
                             // deprecated, no longer actively maintained
#endif



        // PSRAM-Check: GC9D01 nutzt den GC9A01-Treiber, dessen Hardware-
        // Rotation beim GC9D01 wirkungslos bleibt. Mit PSRAM wird die
        // Rotation stattdessen per Software angewendet (gc9d01SwRotation).

        // PSRAM check: the GC9D01 reuses the GC9A01 driver, whose hardware
        // rotation has no effect on the GC9D01. With PSRAM, rotation is
        // applied in software instead (gc9d01SwRotation).
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
#ifndef GC9D01 // wird nur bei Display GC9D01 benoetigt
                // only needed for the GC9D01 display
        gc9d01SwRotation = false;
#endif



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

            preferences.putString(PK_PING_SERVER, DEFAULT_PING_SERVER);

            preferences.putInt(PK_LAST_WLAN, 0);

            preferences.putString(pkNtpServer(0).c_str(), NTP_SERVER_1);
            preferences.putString(pkNtpServer(1).c_str(), NTP_SERVER_2);

            preferences.putString(PK_TIMEZONE, TIMEZONE_DEFAULT);

            preferences.putUChar(PK_TFT_ROTATION1, 0);
            preferences.putUChar(PK_TFT_ROTATION2, 0);
            preferences.putString(PK_HANDSET, "default");
            preferences.putString(PK_BACKGROUND, "/face_default.bmp");

            preferences.putBool(PK_STATION_MODE, true);
            preferences.putBool(PK_SHOW_SECOND_HAND, true);
            preferences.putBool(PK_SMOOTH_MINUTE, false);


#if defined (GC9D01)  || defined (GC9A01_WITH_BACKLIGHT)
            preferences.putUChar(PK_MIN_BRIGHTNESS, 5);
#else
            preferences.putUChar(PK_MIN_BRIGHTNESS, 100);
#endif
            preferences.putUChar(PK_MAX_BRIGHTNESS, 255);

            preferences.putFloat(PK_GAMMA_BRIGHTNESS, 2.2f);  // Gamma-Korrektur für Helligkeit
                                                              // gamma correction for brightness

#if defined (GC9D01)  || defined (GC9A01_WITH_BACKLIGHT)
            preferences.putInt(PK_LOW_THRESHOLD, 1);
            preferences.putInt(PK_HIGH_THRESHOLD, 255);
#else
            preferences.putInt(PK_LOW_THRESHOLD, 40);
            preferences.putInt(PK_HIGH_THRESHOLD, 60);
#endif

            // putLong statt putUInt: dieser Key wird ueberall sonst mit
            // putLong()/getLong() angefasst - ein abweichender NVS-Typ fuehrt
            // sonst zu ESP_ERR_NVS_TYPE_MISMATCH und stillem Default-Rueckfall.

            // putLong instead of putUInt: this key is handled with
            // putLong()/getLong() everywhere else - a mismatched NVS type
            // would cause ESP_ERR_NVS_TYPE_MISMATCH and a silent default.
            preferences.putLong(PK_CENTER_COLOR, 0xEC0016);

            if (tftType == "GC9A01" || tftType == "ILI9341") {
                preferences.putUInt(PK_CENTER_SIZE, 6);
            }
            if (tftType == "GC9D01") {
                preferences.putUInt(PK_CENTER_SIZE, 3);
            }

#if defined GC9A01_WITH_BACKLIGHT
            preferences.putUChar(PK_TFT_ROTATION1, 2);
#else
            preferences.putUChar(PK_TFT_ROTATION1, 0);
#endif
            preferences.putBool(PK_ADC_INVERTED, false);
            preferences.putBool(PK_USE_TOUCH, false);

            preferences.putBool(PK_LOGGING_ENABLED, true);

            preferences.end();
            preferences.begin("clock", false);
        }

        // tftRotation1/2 werden bewusst schon HIER geladen: DRAW_ON_BOTH_DISPLAYS()
        // braucht die echten Werte schon fuer den ersten fillScreen() nach
        // tft.init(), nicht die Default-Initialisierung (0) aus globals.h.

        // tftRotation1/2 are deliberately loaded HERE already: DRAW_ON_BOTH_DISPLAYS()
        // needs the real values already for the first fillScreen() after
        // tft.init(), not the default initialization (0) from globals.h.

        // Migration: alten Key "tftRotation" (vor Display-2-Support) auf
        // "tftRotation1" uebertragen, falls dort noch kein Wert existiert -
        // sonst ginge eine gespeicherte Rotation verloren.

        // Migration: transfer the old key "tftRotation" (pre display-2 support)
        // to "tftRotation1" if it has no value yet - otherwise a saved
        // rotation would be lost.
        if (!preferences.isKey(PK_TFT_ROTATION1) && preferences.isKey(PK_TFT_ROTATION_LEGACY)) {
            preferences.putUChar(PK_TFT_ROTATION1, preferences.getUChar(PK_TFT_ROTATION_LEGACY, 0));
        }

        tftRotation1 = preferences.getUChar(PK_TFT_ROTATION1, 0);
        if (tftRotation1 > 3) {
            if (tftRotation1 == 90) tftRotation1 = 1;
            else if (tftRotation1 == 180) tftRotation1 = 2;
            else if (tftRotation1 == 270) tftRotation1 = 3;
            else tftRotation1 = 0;
            preferences.putUChar(PK_TFT_ROTATION1, tftRotation1);
        }

        // Rotation von Display 2 (CS2) - unabhaengig von tftRotation1, damit
        // beide Displays unterschiedlich ausgerichtet montiert sein koennen.
        // rotation of Display 2 (CS2) - independent of tftRotation1, so
        // both displays can be mounted with a different orientation.
        tftRotation2 = preferences.getUChar(PK_TFT_ROTATION2, 0);
        if (tftRotation2 > 3) {
            if (tftRotation2 == 90) tftRotation2 = 1;
            else if (tftRotation2 == 180) tftRotation2 = 2;
            else if (tftRotation2 == 270) tftRotation2 = 3;
            else tftRotation2 = 0;
            preferences.putUChar(PK_TFT_ROTATION2, tftRotation2);
        }


        loadLanguage(); // liest currentLanguage aus den Preferences (siehe translation.h)
                        // reads currentLanguage from preferences (see translation.h)

        wifiActive = preferences.getBool(PK_WIFI_ACTIVE, true);


        // NTP-Server initialisieren
        // Initialize NTP servers
        initializeNtpServers();

        timezone = preferences.getString(PK_TIMEZONE, TIMEZONE_DEFAULT);

        DEBUG_PRINTLN("[NTP] Timezone set to: " + timezone);

        stationMode = preferences.getBool(PK_STATION_MODE, true);
        smoothMinute = preferences.getBool(PK_SMOOTH_MINUTE, false);
        showSecondHand = preferences.getBool(PK_SHOW_SECOND_HAND, true);

        // Display 2 (CS2) ist fest aktiviert, kein Preferences-/UI-Schalter
        // mehr - CS2-Pin wird immer als Output konfiguriert.

        // Display 2 (CS2) is permanently enabled, no more preferences/UI
        // toggle - the CS2 pin is always configured as output.
        pinMode(CS_2, OUTPUT);

        // Startzustand: Display 1 ausgewaehlt, Display 2 abgewaehlt -
        // setCS1(LOW) erledigt beides bereits (siehe display.h).

        // Starting state: display 1 selected, display 2 deselected -
        // setCS1(LOW) already handles both (see display.h).
        setCS1(LOW);

        // Nabe
        // Hub
        uint32_t hubColorRgb = preferences.getLong(PK_CENTER_COLOR, 0xEC0016); //DB-Rot
                                                                               // DB red
        hubColor = tft.color565((hubColorRgb >> 16) & 0xFF, (hubColorRgb >> 8) & 0xFF, hubColorRgb & 0xFF);
        hubSize = preferences.getUInt(PK_CENTER_SIZE, 6);

        lowThreshold = preferences.getInt(PK_LOW_THRESHOLD, 40);
        highThreshold = preferences.getInt(PK_HIGH_THRESHOLD, 60);
        minBrightness = preferences.getUChar(PK_MIN_BRIGHTNESS, 100);
        maxBrightness = preferences.getUChar(PK_MAX_BRIGHTNESS, 255);

        // Zeitabhängige Helligkeit aus Preferences
        // Time-dependent brightness from preferences

        brightStartHour = preferences.getUChar(PK_BRIGHT_START_HOUR, 7);
        brightEndHour = preferences.getUChar(PK_BRIGHT_END_HOUR, 21);

        adcInverted = preferences.getBool(PK_ADC_INVERTED, false);

        useTouch = preferences.getBool(PK_USE_TOUCH, false);

        rocrailEnabled = preferences.getBool(PK_ROCRAIL_ENABLED, false);
        rocrailServerHost = preferences.getString(PK_ROCRAIL_SERVER, "");
        rocrailServerPort = preferences.getUShort(PK_ROCRAIL_SRV_PORT, ROCRAIL_DEFAULT_PORT);
        loadRocrailServerList();


#if defined (GC9D01)  || defined (GC9A01_WITH_BACKLIGHT)
        gammaBrightness = preferences.getFloat(PK_GAMMA_BRIGHTNESS, 2.2f);  // Gamma-Korrektur für Helligkeit
                                                                            // gamma correction for brightness
#endif

#ifdef BUTTON1
        pinMode(BUTTON1, INPUT_PULLDOWN);
#endif

        // auf Fotowiderstand prüfen
        // Check for photoresistor
#ifdef ADC_3V

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

#else
        useAdc = false;
#endif

        minBrightness = preferences.getUChar(PK_MIN_BRIGHTNESS, 100);
        maxBrightness = preferences.getUChar(PK_MAX_BRIGHTNESS, 255);

        updateBrightness();

        if (loggingEnabled)  Serial.println("debug is " + String(loggingEnabled ? "enabled" : "disabled"));

        digitalWrite(CS_2, LOW);

        tft.init();

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

        // gc9d01SwRotation aktiv: tft.setRotation() ueberspringen, Rotation
        // laeuft dann per Software (siehe PSRAM-Check oben). Sonst normale
        // Hardware-Rotation.

        // If gc9d01SwRotation is active: skip tft.setRotation(), rotation
        // runs in software instead (see PSRAM check above). Otherwise normal
        // hardware rotation.

        // CS2 bekommt seine EIGENE Rotation (tftRotation2) - jedes Display
        // behaelt sein MADCTL-Register dauerhaft, kein erneutes Setzen pro
        // Tick noetig (loop() schaltet nur noch das Chip-Select um).

        // CS2 gets its OWN rotation (tftRotation2) - each display keeps its
        // MADCTL register permanently, no need to re-set it every tick
        // (loop() only toggles the chip select).
        setCS2(LOW);
#ifndef GC9D01
        tft.setRotation(tftRotation2);
#else
        if (!gc9d01SwRotation) {
            tft.setRotation(tftRotation2);
        }
#endif
        setCS1(LOW);
#ifndef GC9D01
        tft.setRotation(tftRotation1);
#else
        if (!gc9d01SwRotation) {
            tft.setRotation(tftRotation1);
        }
#endif


#ifdef TFT_Backlight
        pinMode(TFT_Backlight, OUTPUT);
        ledcAttach(TFT_Backlight, BACKLIGHT_FREQ, BACKLIGHT_RESOLUTION);

        // currentBrightness statt fest 255: updateBrightness() hat die
        // Helligkeit schon ermittelt, ihr ledcWrite() lief aber ins Leere,
        // da ledcAttach() erst hier passiert.

        // currentBrightness instead of a fixed 255: updateBrightness() already
        // determined the brightness, but its ledcWrite() had no effect since
        // ledcAttach() only happens here.
        ledcWrite(TFT_Backlight, currentBrightness);
#endif


        // Rueckgabewert pruefen: schlaegt die Allokation fehl, werden spaetere
        // pushImage()/pushSprite()-Aufrufe stillschweigend No-Ops - beide
        // Displays blieben schwarz, ohne Absturz oder Log-Hinweis.

        // Check the return value: if allocation fails, later pushImage()/
        // pushSprite() calls silently become no-ops - both displays stay
        // black, with no crash or log hint.
        if (backgroundSprite.createSprite(CLOCK_WIDTH, CLOCK_HEIGHT) == nullptr) {
            DEBUG_PRINTLN("[Display] FATAL: couldnt allocate backgroundSprite - clock face cannot be drawn");
        }
        backgroundSprite.setSwapBytes(true);
        backgroundSprite.setColorDepth(16);

        hourHandSprite.createSprite(HAND_WIDTH, HAND_HEIGHT);
        hourHandSprite.setSwapBytes(true);
        hourHandSprite.setColorDepth(16);
        hourHandSprite.setPivot(HAND_WIDTH / 2, HAND_HEIGHT * 0.77);

        minuteHandSprite.createSprite(HAND_WIDTH, HAND_HEIGHT);
        minuteHandSprite.setSwapBytes(true);
        minuteHandSprite.setColorDepth(16);
        minuteHandSprite.setPivot(HAND_WIDTH / 2, HAND_HEIGHT * 0.77);

        secondHandSprite.createSprite(HAND_WIDTH, HAND_HEIGHT);
        secondHandSprite.setSwapBytes(true);
        secondHandSprite.setColorDepth(16);
        secondHandSprite.setPivot(HAND_WIDTH / 2, HAND_HEIGHT * 0.77);

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

#if defined DCF77_DATAPIN && defined DCF77_INTERRUPT
        // Bit-Puffer fuer /dcf77 auf "unbekannt" (-1) initialisieren -
        // int8_t-Arrays lassen sich nicht per Deklaration vorbelegen.

        // Initialize the /dcf77 bit buffer to "unknown" (-1) -
        // int8_t arrays can't be pre-filled via declaration.
        for (uint8_t i = 0; i < DCF77_GRID_SLOTS; i++) dcf77Bits[i] = -1;
#endif

        // Wenn Button1 oder BOOT_BUTTON gedrückt ist, alle Zugangsdaten löschen
        // If Button1 or BOOT_BUTTON is pressed, clear all credentials
        if (digitalRead(BUTTON1) == HIGH || digitalRead(BOOT_BUTTON) == LOW) {
            DEBUG_PRINTLN("[SETUP] Reset button pressed, clearing WiFi credentials and starting AP..");
            DRAW_ON_BOTH_DISPLAYS(
                tft.fillScreen(TFT_RED);
                tft.setTextColor(TFT_WHITE);
                tft.setTextSize(TFT_TEXT_SIZE);
                tft.setCursor(20, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 4));
                tft.println(translate("Reset WLan..."));
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
        // Verbindungsversuch anstossen, statt bis zum ersten regulaeren
        // Zeitfenster (Sekunde 59, siehe connectRocrailClient()) zu warten -
        // dieselbe Funktion, die auch beim Speichern im Webinterface greift
        // (siehe /applydisplaysettings, /save_rocrail in webserver_routes.h).
        // Nur im normalen STA-Betrieb sinnvoll (wie bei pollRocrailClient()
        // weiter unten in loop()) - im AP-/Einrichtungsmodus ist ohnehin kein
        // Rocrail-Server im Heimnetz erreichbar. triggerRocrailConnectNow()
        // selbst ist ein no-op, wenn rocrailEnabled aus ist oder kein Server
        // hinterlegt ist.

        // With a configured Rocrail server, kick off a connection attempt
        // right after a restart, instead of waiting for the first regular
        // window (second 59, see connectRocrailClient()) - the same function
        // that also fires when saving in the web interface (see
        // /applydisplaysettings, /save_rocrail in webserver_routes.h). Only
        // meaningful in normal STA operation (as with pollRocrailClient()
        // further below in loop()) - no Rocrail server on the home network
        // is reachable in AP/setup mode anyway. triggerRocrailConnectNow()
        // itself is a no-op if rocrailEnabled is off or no server is configured.
        if (WiFi.getMode() == WIFI_STA) {
            triggerRocrailConnectNow();
        }


        // NTP hat Vorrang, DCF77 (applyDcf77DecodedTime()) ist der Fallback.
        // Erfolg wird ueber lastNtpSuccessMillis geprueft, nicht ueber den
        // Rueckgabewert von setupNTP() (der bei fehlendem WLAN faelschlich true liefert).

        // NTP has priority, DCF77 (applyDcf77DecodedTime()) is the fallback.
        // Success is checked via lastNtpSuccessMillis, not setupNTP()'s return
        // value (which is misleadingly true when WiFi is down).
        {
            unsigned long beforeNtpMillis = millis();
            setupNTP();
            bool ntpJustSucceeded = (lastNtpSuccessMillis != 0 && lastNtpSuccessMillis >= beforeNtpMillis);
            if (ntpJustSucceeded) {
                DEBUG_PRINTLN("[TIME SYNC] Initial sync: NTP succeeded");
            }
            else {
                DEBUG_PRINTLN("[TIME SYNC] Initial sync: NTP unavailable, trying DCF77 fallback..");
                applyDcf77DecodedTime("[DCF77] initial sync (NTP unavailable)");
            }
        }

        if (useTouch) {
            // Touch-Eingang initialisieren
            // Initialize touch input
            enableTouch();
        }

        loadPresets();

        // Bewusst UNABHAENGIG von rtcOk gestartet: loop() prueft den Zeit-
        // Status ohnehin live vor jeder NTP-Antwort - waere der Start an rtcOk
        // gebunden, bliebe der Server auf Geraeten ohne RTC dauerhaft unbenutzt.

        // Deliberately started INDEPENDENT of rtcOk: loop() live-checks the
        // time status before every NTP reply anyway - tying the start to
        // rtcOk would leave the server permanently unused on RTC-less devices.

        // startNtpServer() statt udp.begin(): prueft den Rueckgabewert, haelt
        // ntpServerRunning aktuell und wird nach jedem Reconnect (connectWiFi()
        // in wifi_manager.h) erneut aufgerufen - der Socket ueberlebt sonst nicht.

        // startNtpServer() instead of udp.begin(): checks the return value,
        // keeps ntpServerRunning current, and is called again after every
        // reconnect (connectWiFi() in wifi_manager.h) - the socket wouldn't survive otherwise.
        startNtpServer();

        DEBUG_PRINTLN("[SETUP] Boot complete, free heap: " + String(ESP.getFreeHeap()) + " bytes");
        checkHeapWarning("Setup Ende");

        setLedOff();
    }


    // Main-Loop
    // Main loop

    void loop() {

        // updateClock() steuert beide Displays vollstaendig pro Tick, inkl.
        // eigener Rotation (renderClockFrame() in display.h) - kein manuelles
        // CS1/CS2-Umschalten mehr noetig.

        // updateClock() drives both displays fully each tick, each with its
        // own rotation (renderClockFrame() in display.h) - no manual
        // CS1/CS2 toggling needed anymore.

        // Asynchrone Pruefung einer per Web-Button gestarteten WPS-Anfrage (siehe
        // /api/startWPS) - blockiert loop() nicht, reagiert auf die im WiFi-Event-
        // Callback gesetzten Flags statt WiFi.status() zu pollen (zuverlaessiger).

        // Verzoegerter, NICHT-blockierender Start von WPS: der HTTP-Handler von
        // /api/startWPS setzt nur wpsStartRequested + wpsStartRequestedAtMillis
        // und kehrt sofort zurueck, damit der Webserver die per Redirect
        // aufgerufene Zielseite (mit dem Trennungs-Banner) sofort ausliefern
        // kann. startWPS() selbst wird erst hier aufgerufen, nachdem dem
        // Browser genug Zeit blieb, die Seite zu laden und den Banner
        // anzuzeigen - ein delay() im Handler wuerde stattdessen genau diese
        // Auslieferung blockieren.

        // Deferred, NON-blocking start of WPS: the /api/startWPS HTTP handler
        // only sets wpsStartRequested + wpsStartRequestedAtMillis and returns
        // immediately, so the web server can serve the redirect's target page
        // (with the disconnect banner) right away. startWPS() itself is only
        // called here, once the browser has had enough time to load the page
        // and show the banner - a delay() in the handler would instead block
        // exactly that delivery.
        if (wpsStartRequested && millis() - wpsStartRequestedAtMillis >= (2 * WAIT_1s)) {
            wpsStartRequested = false;
            startWPS();
            wpsPending = true;
            wpsStartMillis = millis();
        }

        // Async check of a WPS request started via the web button (see
        // /api/startWPS) - doesn't block loop(), reacts to the flags set in the
        // WiFi event callback instead of polling WiFi.status() (more reliable).
        if (wpsPending) {
            if (wpsSuccessEvent) {
                wpsSuccessEvent = false;
                wpsPending = false;

                // WPS erfolgreich: Zugangsdaten sichern und neu starten statt
                // die Verbindung in dieser Session wiederherzustellen.

                // WPS succeeded: save credentials and reboot instead of
                // trying to restore the connection in this session.

                // esp_wifi_get_config() liefert direkt nach dem Event manchmal
                // leere Daten (bekannter Bug #10339/#11705) - daher mehrfach
                // mit Pause versuchen.

                // esp_wifi_get_config() sometimes returns empty data right
                // after the event (known bug #10339/#11705) - so retry with
                // a short delay.
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
                // in wifi_manager.h - gemeinsame Logik fuer diesen und den
                // Timeout-Zweig unten, vorher hier dupliziert).

                // Restore the original connection if it was dropped by the
                // WPS attempt, if applicable (see restorePreviousWpsConnection()
                // in wifi_manager.h - shared logic for this and the timeout
                // branch below, previously duplicated here).
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
                // Wartephase (noch kein Event, noch kein Timeout): Status-LED
                // blinkt periodisch als sichtbares Lebenszeichen, solange auf
                // den Tastendruck am Router gewartet wird - unabhaengig vom
                // DCF77-Blinken (siehe processDcf77Bits() in time_sync.h, dort
                // per "&& !wpsPending" bewusst zurueckgestellt, damit sich
                // beide Signalisierungen nicht optisch ueberlagern).

                // Waiting phase (no event yet, no timeout yet): the status LED
                // blinks periodically as a visible sign of life while waiting
                // for the button press on the router - independent of the
                // DCF77 blink (see processDcf77Bits() in time_sync.h, held
                // back there via "&& !wpsPending" so the two signals don't
                // visually overlap).
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
        // die eigentliche Zeituebernahme passiert getrennt davon, stuendlich
        // mit NTP-Vorrang (siehe checkHourlyTimeSync-Logik weiter unten).
        // Keep the DCF77 reception status up to date (lastDcfSyncTime/
        // dcfTimeFound) - the actual time takeover happens separately, hourly
        // with NTP priority (see the checkHourlyTimeSync logic further below).
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

        // Stuendliche Zeitsynchronisation: NTP hat Vorrang, DCF77 (siehe
        // applyDcf77DecodedTime()) ist der Fallback - gleicher Ablauf wie
        // der initiale Sync in setup() (Begruendung dort).

        // Hourly time sync: NTP has priority, DCF77 (see
        // applyDcf77DecodedTime()) is the fallback - same flow as the
        // initial sync in setup() (reasoning there).
        if (millis() - lastNTPUpdate > WAIT_1h) {
            if (timeinfo.tm_sec < 10 || timeinfo.tm_sec > 55) {

                // 15s verschieben ohne den Zeitstempel in die Zukunft zu
                // setzen: "millis() + 15000" fuehrte zu Unterlauf/sofortigem
                // erneuten Trigger. Diese Form ist modulo-/ueberlaufsicher.

                // Postpone by 15s without setting the timestamp into the
                // future: "millis() + 15000" caused underflow and an
                // immediate re-trigger. This form is overflow-safe.
                lastNTPUpdate = millis() - WAIT_1h + 15 * 1000;
            }
            else {
                unsigned long beforeNtpMillis = millis();
                setupNTP();
                bool ntpJustSucceeded = (lastNtpSuccessMillis != 0 && lastNtpSuccessMillis >= beforeNtpMillis);
                if (ntpJustSucceeded) {
                    DEBUG_PRINTLN("[TIME SYNC] Hourly sync: NTP succeeded");
                }
                else {
                    DEBUG_PRINTLN("[TIME SYNC] Hourly sync: NTP unavailable, trying DCF77 fallback..");
                    applyDcf77DecodedTime("[DCF77] hourly sync (NTP unavailable)");
                }
                lastNTPUpdate = millis();
            }

        }

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

#if defined DCF77_DATAPIN && defined DCF77_INTERRUPT
        // LED-Blinken fuer DCF77-Impulse HIER statt in der ISR: pinMode()/
        // digitalWrite() liegen im Flash und wuerden aus der ISR bei
        // deaktiviertem Flash-Cache einen Panic-Reset ausloesen.

        // DCF77 pulse LED blink HERE, not in the ISR: pinMode()/digitalWrite()
        // live in flash and would trigger a panic reset if called from the
        // ISR while the flash cache is disabled.

        // Einmal-Blitz statt Umschalten: ein Toggle konnte bei ungerader
        // Impulszahl die LED dauerhaft an lassen. Jetzt schaltet jeder Impuls
        // sie fuer DCF77_LED_BLINK_MS ein, der Block darunter immer wieder aus.

        // One-shot flash instead of toggling: a toggle could leave the LED on
        // permanently on an odd pulse count. Now every pulse turns it on for
        // DCF77_LED_BLINK_MS, and the block below always turns it off again.
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
#endif

        if (WiFi.getMode() == WIFI_STA || rtcOk == RTC_AVAILABLE || dcfTimeFound) {

        updateClock();

        checkWeeklyRestart();

        // wpsPending ausschliessen: waehrend einer laufenden WPS-Verhandlung
        // (bis zu 2 Minuten, siehe oben) trennt sich das WLAN typischerweise
        // kurzzeitig - ohne diese Bedingung griff hier vor allem beim ALLERERSTEN
        // Verbindungsverlust seit Boot (firstAttempt in checkWiFiReconnect())
        // sofort ein eigener Reconnect-Versuch ein und kollidierte mit der noch
        // laufenden WPS-Verhandlung um denselben Funkchip.

        // Exclude wpsPending: while a WPS negotiation is in progress (up to 2
        // minutes, see above), WiFi typically drops briefly - without this
        // condition, especially on the VERY FIRST disconnect since boot
        // (firstAttempt in checkWiFiReconnect()), a reconnect attempt fired
        // immediately and collided with the still-running WPS negotiation over
        // the same radio.
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
        }

        //  checkWiFiScan(); // Überprüfe den Status des Scans
        // NTP-/DCF77-Zeitsynchronisation laeuft jetzt stuendlich weiter oben
        // (unabhaengig vom WLAN-Status abgearbeitet) - hier daher nichts
        // mehr zu tun.
        // NTP/DCF77 time sync now runs hourly further above (handled
        // independent of WiFi status) - nothing left to do here.

        initial = false;

        }

          // NTP-Server-Anfragen beantworten (Test: w32tm /stripchart /computer:<ip>)
          //
          // Die Bedingung prueft die SYSTEMZEIT, nicht die Zeitquelle - nur so
          // wird geantwortet, sobald irgendeine Quelle (NTP/DCF77/RTC) eine
          // gueltige Zeit gesetzt hat. Schwelle wie setupNTP(): Jahr > 2016.

          // Answer NTP server requests (test: w32tm /stripchart /computer:<ip>)
          //
          // The condition checks the SYSTEM TIME, not the time source - this
          // way it answers once any source (NTP/DCF77/RTC) has set a valid
          // time. Threshold as in setupNTP(): year > 2016.
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

                // Ohne gueltige Systemzeit waere jede Antwort schlimmer als
                // keine (Client stellt sich auf 1970) - Anfrage dann verwerfen,
                // Client faellt auf seinen Timeout/naechste Quelle zurueck.

                // Without a valid system time, any answer is worse than none
                // (client would set itself to 1970) - discard the request,
                // the client falls back to its timeout/next source.
                if (receivedAt.tv_sec <= 1483228800L) { // 2017-01-01
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

        if (useTouch) {
            // Touch erst aktivieren, wenn die Startverzögerung vorbei ist
            // Enable touch only once the startup delay has passed
            if (!touchEnabled && touchEnableAt != 0 && millis() >= touchEnableAt) {
                touchEnabled = true;
                DEBUG_PRINTLN("[TOUCH] Enabled");
            }

            if (touchEnabled) {
                // Touch-Input prüfen und ggf. Hintergrund wechseln
                // Check touch input and switch background if needed
                checkTouchInput();
            }
        }

        // restart im AP Mode nach 30 Minuten
        // Restart in AP mode after 30 minutes
        if (softAPIP == true) {
            if (millis() - softAPIPstart > WAIT_30m) {
                //        ESP.restart();
            }
        }

#ifdef ILI9341 // DEPRECATED - nicht mehr aktiv gepflegt
               // deprecated, no longer actively maintained
        // Datum und Uhrzeit auf dem TFT ausgeben
        // Print date and time on the TFT
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextSize(3);

        String hourStr = String(timeinfo.tm_hour);
        int xPos = 50;
        // vornull entfernen wenn vorhanden
        // Remove leading zero if present
        if (hourStr.startsWith("0")) {
            hourStr = hourStr.substring(1);
            xPos = 30; // etwas weiter links positionieren, wenn nur 1-stellige Stunde
                       // position a bit further left for a single-digit hour
        }

        // Uhrzeit auf dem TFT ausgeben
        // Print time on the TFT
        if (!preferences.getBool(PK_SHOW_SECOND_HAND, true)) {
            tft.setCursor(xPos, 260);
            tft.printf("%2d:%02d:%02d", hourStr.toInt(), timeinfo.tm_min, timeinfo.tm_sec);
        }
        else {
            tft.setCursor(xPos + 20, 260);
            if (timeinfo.tm_sec % 2 == 0) {
                tft.printf("%2d:%02d", hourStr.toInt(), timeinfo.tm_min);
            }
            else {
                tft.printf("%2d %02d", hourStr.toInt(), timeinfo.tm_min);
            }

        }

        // Datum auf dem TFT ausgeben
        // Print date on the TFT
        if (timeinfo.tm_mday < 10) {
            tft.setCursor(20, 290);
        }
        else {
            tft.setCursor(40, 290);
        }

        tft.printf("%2d.%02d.%04d", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
#endif

        // Kein unbedingtes setLedOff() mehr am Zeilenende von loop() (loeschte
        // den DCF77-Blitz sofort) - Abschaltung erfolgt jetzt ueber
        // DCF77_LED_BLINK_MS und die Sicherheits-Abschaltung oben.

        // No more unconditional setLedOff() at the end of loop() (it cleared
        // the DCF77 flash immediately) - switch-off now happens via
        // DCF77_LED_BLINK_MS and the safety switch-off above.
    }

