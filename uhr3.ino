    // howl@gmx.de
    // stationsuhr 05/2025 - 02/2026
    //
    // https://github.com/holgiw?tab=repositories
    //
    //
    // optimiert für ESP32-S2 Mini  (als Lolin S2 Pico compiliert)

    // optimized for ESP32-S2 Mini (compiled as Lolin S2 Pico)
    // Filesystem: LittleFS
    // TFT: GC9A01 / GC9D01
    // Partition: Default 4MB NO OTA, 2MB, 2MB
    // TFT_eSPI: 2.5.34
    //
    // DCF77: https://de.elv.com/p/elv-dcf-empfangsmodul-dcf-2-P091610/
    //
    // Anpassungen in DCF77.cpp:

    // Changes in DCF77.cpp:
    //  zeile: 22: change #include <TimeLib.h>

    //  Zeile 22: #include <TimeLib.h> ändern
    //  add IRAM_ATTR   in: void IRAM_ATTR DCF77::int0handler() {

    //  IRAM_ATTR ergänzen in: void IRAM_ATTR DCF77::int0handler() {


#include <WiFi.h>
#include <WebServer.h>

#include "prefs_keys.h"
#include "build_defs.h"

// WICHTIG: Die TFT_eSPI-Konfiguration (Treiber, Pins, Schriften) muss in der
// Bibliothek selbst gesetzt werden (User_Setup_Select.h -> User_Setup.h) - ein
// #define hier im Sketch wirkt NICHT. Siehe GC9A01-Block in config.h.

// IMPORTANT: The TFT_eSPI config (driver, pins, fonts) must be set inside the
// library itself (User_Setup_Select.h -> User_Setup.h) - a #define here in the
// sketch has NO effect. See the GC9A01 block in config.h.
//
// C:\Users\hwage\Documents\Arduino\libraries\TFT_eSPI\User_Setups\Setup304_ESP32S2_GC9A01_GC9D0.h

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

// Liefert ESP_ARDUINO_VERSION_STR - die arduino-esp32-Core-Version, mit der
// dieser Sketch tatsaechlich kompiliert wurde (Anzeige im Status, siehe
// webserver_routes.h) - hilfreich, um z.B. WPS-Regressionen zwischen
// Core-Versionen (siehe wifi_manager.h) ohne Blick in die IDE zu erkennen.

// Provides ESP_ARDUINO_VERSION_STR - the arduino-esp32 core version this
// sketch was actually compiled with (shown in Status, see
// webserver_routes.h) - useful for spotting e.g. WPS regressions between
// core versions (see wifi_manager.h) without checking the IDE.
#include <esp_arduino_version.h>

// Fuer esp_reset_reason() - Grund des letzten Neustarts (Power-On, Watchdog,
// Panic, Brownout, ...), Anzeige im Status (siehe webserver_routes.h).

// For esp_reset_reason() - reason for the last restart (power-on, watchdog,
// panic, brownout, ...), shown in Status (see webserver_routes.h).
#include <esp_system.h>
#include <Wire.h>
#include <RTClib.h>
#include <WiFiUdp.h>
#include <DCF77.h>  // https://forum.arduino.cc/t/dcf77-am-esp-32/1213608/7
                        // https://www.elkoba.com/magazin/eine-funkuhr-selbst-bauen-bauprojekt-dcf77/?srsltid=AfmBOorDPPTdSBRgDzH3HdjPTO9M7wl6MM68TusAIfYBCkYG6z13Rbt0


#include "globals.h"       // globale Objekte, Variablen, Structs
                           // global objects, variables, structs
#include "declarations.h"  // Forward-Deklarationen aller Funktionen
                           // forward declarations of all functions

#include "wifi_manager.h"      // WLAN: Verbindung, AP, Scan, Reconnect
                               // WiFi: connection, AP, scan, reconnect
#include "time_sync.h"         // RTC, DCF77, NTP
#include "display.h"           // Zifferblatt, Zeiger, Helligkeit, Touch
                               // dial, hands, brightness, touch
#include "presets_manager.h"   // Presets laden/speichern/wechseln
                               // load/save/switch presets
#include "webserver_routes.h"  // Webinterface (alle HTTP-Routen)
                               // web interface (all HTTP routes)
#include "system_utils.h"      // Tasten, Logging, Reset, Neustart
                               // buttons, logging, reset, restart


    // Setup-Funktion
    // Setup function

    void setup() {

        setLedOn();

        Serial.begin(115200);

        // CS_1 (Chip-Select von Display 1, vormals TFT_CS) manuell auf Output/LOW
        // setzen - die TFT_eSPI-Bibliothek steuert ihren eigenen CS-Pin nicht mehr
        // automatisch (TFT_CS = -1 in der Referenzkonfiguration/User_Setup.h,
        // siehe config.h). Muss hier, VOR tft.init() weiter unten, passieren und
        // ist UNABHAENGIG von cs2Enabled noetig - ohne das wuerde auch der
        // Einzeldisplay-Betrieb (kein CS2) nichts mehr anzeigen, weil kein Chip
        // mehr automatisch selektiert wird.

        // Manually set CS_1 (display 1's chip select, formerly TFT_CS) to
        // output/LOW - the TFT_eSPI library no longer drives its own CS pin
        // automatically (TFT_CS = -1 in the reference config/User_Setup.h, see
        // config.h). Has to happen here, BEFORE tft.init() further below, and is
        // needed REGARDLESS of cs2Enabled - without this, even single-display
        // operation (no CS2) would show nothing, since no chip is automatically
        // selected anymore.
        pinMode(CS_1, OUTPUT);
        digitalWrite(CS_1, LOW);

        // HINWEIS: Die eigentliche CS2-Pin-Initialisierung (pinMode + setCS1/setCS2)
        // steht jetzt weiter unten, NACH preferences.begin()/dem Laden von
        // cs2Enabled - vorher waere cs2Enabled noch auf seinem Default (false)
        // und die gespeicherte Einstellung wuerde bei diesem fruehen Aufruf
        // ignoriert (siehe Ladepunkt bei "cs2Enabled = preferences.getBool(...)").

        // NOTE: The actual CS2 pin initialization (pinMode + setCS1/setCS2) now
        // lives further below, AFTER preferences.begin()/loading cs2Enabled -
        // any earlier, cs2Enabled would still be at its default (false) and the
        // saved setting would be ignored at this early call (see the load point
        // at "cs2Enabled = preferences.getBool(...)").

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



        // Pruefen, ob PSRAM vorhanden ist. GC9D01 braucht fuer korrekte
        // Hardware-Rotation eigentlich einen eigenen TFT_eSPI-Treiber
        // (GC9D01_Defines.h/Init.h/Rotation.h), der hier nicht eingebunden
        // ist - stattdessen wird der GC9A01-Treiber wiederverwendet (siehe
        // config.h), dessen Rotations-Register-Mapping beim GC9D01 aber
        // nicht wie erwartet funktioniert (tft.setRotation() bleibt ohne
        // sichtbare Wirkung). Workaround, gesteuert ueber gc9d01SwRotation:
        // bei vorhandenem PSRAM wird die Hardware-Rotation uebersprungen
        // (siehe Rotations-Block weiter unten in dieser Datei) und Zeiger
        // sowie Zifferblatt werden stattdessen per Software gedreht (siehe
        // rotatedAngle() und der Zifferblatt-Rotationsblock in loadClockFace(),
        // beide in display.h).

        // Check whether PSRAM is available. GC9D01 would actually need its
        // own TFT_eSPI driver (GC9D01_Defines.h/Init.h/Rotation.h) for
        // correct hardware rotation, which isn't wired up here - the GC9A01
        // driver is reused instead (see config.h), whose rotation register
        // mapping doesn't work as expected on the GC9D01 (tft.setRotation()
        // has no visible effect). Workaround, controlled via gc9d01SwRotation:
        // when PSRAM is available, hardware rotation is skipped (see the
        // rotation block further below in this file) and both the hands and
        // the clock face are rotated in software instead (see rotatedAngle()
        // and the clock face rotation block in loadClockFace(), both in
        // display.h).
        if (psramFound() and ESP.getFreePsram() > 2 * (CLOCK_WIDTH * CLOCK_HEIGHT * sizeof(uint16_t))) {
            gc9d01SwRotation = true;
            DEBUG_PRINTLN("[INFO] found PSRAM");
        }
        else {
            gc9d01SwRotation = false;
            DEBUG_PRINTLN("[INFO] no PSRAM, use Hardware-Rotation");
#ifdef GC9D01
            preferences.putUChar(PK_TFT_ROTATION1, 0);
#endif
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

            preferences.putUInt(PK_CENTER_COLOR, 0xEC0016);

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
            preferences.putBool(PK_USE_CS2, false);

            preferences.putBool(PK_LOGGING_ENABLED, true);

            preferences.end();
            preferences.begin("clock", false);
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
        cs2Enabled = preferences.getBool(PK_USE_CS2, false);

        // CS2-Pin ist immer eingebunden (siehe config.h), wird aber nur bei
        // aktivierter Einstellung (Zifferblatt-Tab, Default aus) tatsaechlich
        // als Output konfiguriert und angesteuert (CS_1 = Pin 12 = Display 1,
        // siehe Kommentar weiter oben in setup()).

        // The CS2 pin is always compiled in (see config.h), but is only
        // actually configured as an output and driven when the setting is
        // enabled (Clock Face tab, default off) (CS_1 = pin 12 = display 1, see
        // comment further up in setup()).
        if (cs2Enabled) {
            pinMode(CS_2, OUTPUT);

            // Definierter Ausgangszustand: Display 1 (CS_1) ausgewaehlt, Display 2
            // (CS_2) abgewaehlt. setCS2(HIGH) waere hier ein No-Op (setCS2()
            // reagiert nur auf LOW) - setCS1(LOW) erledigt beides bereits (siehe
            // display.h), deshalb reicht dieser eine Aufruf.

            // Defined starting state: display 1 (CS_1) selected, display 2 (CS_2)
            // deselected. setCS2(HIGH) here would be a no-op (setCS2() only reacts
            // to LOW) - setCS1(LOW) already handles both (see display.h), so this
            // one call is enough.
            setCS1(LOW);
        }

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

        if (cs2Enabled) {
            digitalWrite(CS_2, LOW);
        }

        tft.init();

        delay(75);
        tft.fillScreen(TFT_BLACK);


        // Migration: alter Preferences-Key "tftRotation" (aus Versionen vor der
        // Display-2-Unterstuetzung) auf den neuen Key "tftRotation1" uebertragen,
        // falls unter dem neuen Key noch kein Wert existiert - sonst wuerde eine
        // bereits gespeicherte Rotationseinstellung nach diesem Update verloren
        // gehen und auf 0 Grad zurueckfallen. Einmalig: sobald PK_TFT_ROTATION1
        // existiert, greift dieser Zweig nie wieder (auch auf einem komplett
        // neuen Geraet nicht, da der Erststart-Block weiter oben PK_TFT_ROTATION1
        // bereits mit einem Default belegt).

        // Migration: transfer the old Preferences key "tftRotation" (from
        // versions before Display 2 support) to the new key "tftRotation1" if
        // no value exists yet under the new key - otherwise an already-saved
        // rotation setting would be lost after this update and fall back to 0
        // degrees. One-time: once PK_TFT_ROTATION1 exists, this branch never
        // runs again (not even on a brand-new device, since the first-start
        // block further above already gives PK_TFT_ROTATION1 a default).
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

        selectedBackground = preferences.getString(PK_BACKGROUND, "/face_default.bmp");

        validateSelectedBackground();

        // GC9D01 nutzt hier den GC9A01-Treiber (siehe config.h/PSRAM-Block
        // oben), dessen Hardware-Rotation beim GC9D01 aber wirkungslos
        // bleibt. Deshalb: bei aktivem Software-Rotations-Workaround
        // (gc9d01SwRotation, nur beim GC9D01 relevant) tft.setRotation()
        // ueberspringen - die Rotation wird dann stattdessen per Software auf
        // die Zeigerwinkel (rotatedAngle()) und das Zifferblatt (Rotationsblock
        // in loadClockFace()) angewendet, beide in display.h. Fuer alle
        // anderen Boards (GC9A01, ILI9341) laeuft die Hardware-Rotation
        // unveraendert unbedingt.

        // GC9D01 uses the GC9A01 driver here (see config.h/PSRAM block
        // above), but its hardware rotation has no effect on the GC9D01.
        // Therefore: when the software rotation workaround is active
        // (gc9d01SwRotation, only relevant for the GC9D01) skip
        // tft.setRotation() - the rotation is then applied in software to
        // both the hand angles (rotatedAngle()) and the clock face (rotation
        // block in loadClockFace()), both in display.h. For all other boards
        // (GC9A01, ILI9341) hardware rotation still runs unconditionally as
        // before.
        // CS2 bekommt seine EIGENE Rotation (tftRotation2) - das MADCTL-Kommando
        // von tft.setRotation() wird nur vom gerade selektierten Chip uebernommen,
        // jedes der beiden Displays behaelt seine Ausrichtung danach dauerhaft im
        // eigenen Register, ein wiederholtes Setzen pro Tick ist nicht noetig
        // (siehe loop() - dort wird nur noch das Chip-Select umgeschaltet).

        // CS2 gets its OWN rotation (tftRotation2) - the MADCTL command from
        // tft.setRotation() is only picked up by the currently selected chip;
        // each display then keeps that orientation permanently in its own
        // register, so it doesn't need to be re-applied every tick (see loop() -
        // it only toggles the chip select from here on).
        if (cs2Enabled) {
            setCS2(LOW);
#ifndef GC9D01
            tft.setRotation(tftRotation2);
#else
            if (!gc9d01SwRotation) {
                tft.setRotation(tftRotation2);
            }
#endif
            setCS1(LOW);
        }
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
        ledcWrite(TFT_Backlight, 255);
#endif


        backgroundSprite.createSprite(CLOCK_WIDTH, CLOCK_HEIGHT);
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

        // Bei gueltiger RTC die Uhrzeit sofort anzeigen - noch bevor die WLAN-
        // Verbindungsversuche unten beginnen (koennen bis zu 15-30s je Anlauf dauern),
        // statt erst einen "Connect to SSID.."-Bildschirm trotz laengst bekannter Zeit.

        // If the RTC is valid, show the time immediately - even before the WiFi
        // connection attempts below start (each attempt can take 15-30s), instead
        // of showing a "Connect to SSID.." screen despite already knowing the time.
        if (rtcOk == RTC_AVAILABLE) {
            updateClock();
        }

        setupWebServer();
        webserver.begin();

        // DCF77-Interrupt einrichten
        // Set up the DCF77 interrupt
        dcf.Start();
        pinMode(DCF77_DATAPIN, INPUT_PULLUP);
        attachInterrupt(DCF77_DATAPIN, isr, CHANGE);

        // Wenn Button1 oder BOOT_BUTTON gedrückt ist, alle Zugangsdaten löschen
        // If Button1 or BOOT_BUTTON is pressed, clear all credentials
        if (digitalRead(BUTTON1) == HIGH || digitalRead(BOOT_BUTTON) == LOW) {
            DEBUG_PRINTLN("[SETUP] Reset button pressed, clearing WiFi credentials and starting AP..");
            tft.fillScreen(TFT_RED);
            tft.setTextColor(TFT_WHITE);
            tft.setTextSize(TFT_TEXT_SIZE);
            tft.setCursor(20, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 4));
            tft.println(translate("Reset WLan..."));
            delay(1000);
            for (int i = 0; i < MAX_WLAN; i++) {
                wifiSsid[i] = "";
                wifiPass[i] = "";
                String ssidKey = pkSsid(i);
                String passKey = pkPass(i);
                preferences.putString(ssidKey.c_str(), "");
                preferences.putString(passKey.c_str(), "");
                preferences.end();
                preferences.begin("clock", false);
            }
        }

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



        uint32_t number = preferences.getInt(PK_LAST_WLAN, 0);
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
        }

        // versuche Verbindung mit der letzten SSID
        // Try connecting with the last SSID
        if (!foundLastSSID or connectWiFi(number, true) != CONNECTED) {

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

                    if (j == number) continue; // überspringe bereits versuchte SSID
                                               // skip already-tried SSID
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
                tft.fillScreen(TFT_BLACK);
                tft.setTextColor(TFT_WHITE);
                tft.setTextSize(TFT_TEXT_SIZE);
                tft.setCursor(20, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 4));
                tft.println(translate("Check RTC"));
                delay(1000);
                loadTimeFromRTC();
                return;
            }

            // delay(3000);
            if (dcf77Count >= 1) {
                DEBUG_PRINTLN("[DCF77] DCF77 signal received during setup, waiting for valid time..");
                tft.fillScreen(TFT_BLACK);
                tft.setTextColor(TFT_WHITE);
                tft.setTextSize(TFT_TEXT_SIZE);
                tft.setCursor(20, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 4));
                tft.println(translate("DCF77 detected"));

                unsigned long startWait = millis();
                while (millis() - startWait < WAIT_1h) { // Warte bis zu 1 Stunde auf gültige DCF77-Zeit
                                                         // wait up to 1 hour for a valid DCF77 time
                    if (dcf.getUTCTime() > 0) {
                        dcfTimeFound = true; // gültige Zeit gefunden
                                             // valid time found
                        return;
                    }
                    tft.setCursor(20, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 8));
                    tft.print(translate("Waiting"));
                    tft.print("...");
                    delay(100);
                }
            }


            // // Alle Verbindungsversuche fehlgeschlagen, starte AP wenn RTC nicht verfügbar oder ungültig
            // // All connection attempts failed, start AP if the RTC is unavailable or invalid
            if (rtcOk != RTC_AVAILABLE) {
                DEBUG_PRINTLN("[WiFi] Starting Access Point due to failed connections and no valid RTC");
                startAP();
            }
        }


        setupNTP();

        if (useTouch) {
            // Touch-Eingang initialisieren
            // Initialize touch input
            enableTouch();
        }

        loadPresets();

        if (rtcOk == RTC_AVAILABLE) {
            // UDP starten
            // Start UDP
            udp.begin(NTP_PORT);
            DEBUG_PRINTLN("[NTPD] NTP Server started");
        }

        DEBUG_PRINTLN("[SETUP] Boot complete, free heap: " + String(ESP.getFreeHeap()) + " bytes");
        checkHeapWarning("Setup Ende");

        setLedOff();
    }


    // Main-Loop
    // Main loop

    void loop() {

        // HINWEIS: Das frueher hier stehende Umschalten zwischen CS1/CS2 (abwechselnd
        // pro Tick) entfaellt - updateClock() steuert beide Displays jetzt selbst und
        // vollstaendig pro Tick an (siehe renderClockFrame()/updateClock() in
        // display.h), inklusive je eigener Rotation. So bekommen bei GC9D01 (Software-
        // Rotation) beide Displays wirklich unabhaengige Ausrichtungen statt sich die
        // Anzeige abwechselnd zu teilen.

        // NOTE: The CS1/CS2 toggling that used to sit here (alternating per tick) is
        // gone - updateClock() now drives both displays itself, fully, every tick
        // (see renderClockFrame()/updateClock() in display.h), each with its own
        // rotation. This way, with GC9D01 (software rotation) both displays get
        // genuinely independent orientations instead of taking turns sharing the
        // same rendered frame.

        // Asynchrone Pruefung einer per Web-Button gestarteten WPS-Anfrage (siehe
        // /api/startWPS) - blockiert loop() nicht, reagiert auf die im WiFi-Event-
        // Callback gesetzten Flags statt WiFi.status() zu pollen (zuverlaessiger).

        // Async check of a WPS request started via the web button (see
        // /api/startWPS) - doesn't block loop(), reacts to the flags set in the
        // WiFi event callback instead of polling WiFi.status() (more reliable).
        if (wpsPending) {
            if (wpsSuccessEvent) {
                wpsSuccessEvent = false;
                wpsPending = false;

                // Nach mehreren erfolglosen Versuchen, die Verbindung in dieser Session
                // wiederherzustellen (esp_wifi_connect() schlug wiederholt fehl): stattdessen
                // Zugangsdaten sichern und neu starten - connectWiFi() uebernimmt die Verbindung.

                // After several failed attempts to restore the connection in this session
                // (esp_wifi_connect() kept failing): save the credentials instead and
                // reboot - connectWiFi() handles reconnecting after restart.

                // esp_wifi_get_config() liefert direkt nach dem Erfolgs-Event manchmal
                // leere Daten (bekannter ESP-IDF/arduino-esp32-Bug, #10339/#11705) -
                // daher mehrfach mit kurzer Pause versuchen statt sofort aufzugeben.

                // esp_wifi_get_config() sometimes returns empty data right after the
                // success event (known ESP-IDF/arduino-esp32 bug, #10339/#11705) -
                // so retry a few times with a short delay instead of giving up at once.
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
                DEBUG_PRINTLN("[WPS] Restarting to reconnect via the normal boot sequence..");
                delay(WAIT_1s);
                espReboot();
            }
            else if (wpsFailedEvent) {
                DEBUG_PRINTLN("[WPS] WPS failed or timed out (event)");
                wpsFailedEvent = false;
                esp_wifi_wps_disable();
                wpsPending = false;
                // Ggf. urspruengliche Verbindung wiederherstellen, falls durch
                // den WPS-Versuch getrennt.

                // Restore the original connection if it was dropped by the
                // WPS attempt, if applicable.
                if (wpsPreviousSsid != "" && !WiFi.isConnected()) {
                    for (int i = 0; i < MAX_WLAN; i++) {
                        if (wifiSsid[i] == wpsPreviousSsid) {
                            connectWiFi(i, false);
                            break;
                        }
                    }
                }
                wpsPreviousSsid = "";
            }
            else if (millis() - wpsStartMillis > (2 * WAIT_1m)) {
                DEBUG_PRINTLN("[WPS] Timeout waiting for WPS button press - disabling WPS");
                esp_wifi_wps_disable();
                wpsPending = false;
                if (wpsPreviousSsid != "" && !WiFi.isConnected()) {
                    for (int i = 0; i < MAX_WLAN; i++) {
                        if (wifiSsid[i] == wpsPreviousSsid) {
                            connectWiFi(i, false);
                            break;
                        }
                    }
                }
                wpsPreviousSsid = "";
            }
        }

        if (WiFi.getMode() == WIFI_STA && WiFi.isConnected()) {
            ipAddress = WiFi.localIP().toString();
        }
        else {
            ipAddress = WiFi.softAPIP().toString();
        }

        // DCF77-Zeit abrufen
        // Fetch DCF77 time
        getDCF77Time();

        // Überprüfen, ob seit dem letzten Aufruf Zeit vergangen ist
        // Check whether time has passed since the last call
        if (millis() - lastNTPUpdate > WAIT_1h) {
            if (timeinfo.tm_sec < 10 || timeinfo.tm_sec > 55) {
                lastNTPUpdate = millis() + 15 * 1000;
            }
            else {
                setupNTP();
                lastNTPUpdate = millis();
            }

        }

        if (timeinfo.tm_sec == 0) setLedOff(); // wg. DCF
                                               // because of DCF

        // Wenn im AP-Modus: DNS-Requests abarbeiten (captive portal)
        // In AP mode: process DNS requests (captive portal)
        if (softAPIP) {
            dnsServer.processNextRequest();
        }


        webserver.handleClient();


        if (WiFi.getMode() == WIFI_STA || rtcOk == RTC_AVAILABLE || dcfTimeFound) {

        updateClock();

        checkWeeklyRestart();

        if (wifiActive && !WiFi.isConnected()) {
            checkWiFiReconnect();
        }

        //  checkWiFiScan(); // Überprüfe den Status des Scans
        if (wifiActive && WiFi.isConnected()) {
            checkNTPRetry();
            checkNightlyTimeSync();
        }

        initial = false;

        }

          // NTP-Server anfragen bearbeiten

          // Handle NTP server requests
          // win:  w32tm /stripchart /computer:192.168.0.214
        if ((WiFi.getMode() == WIFI_STA && rtcOk == RTC_AVAILABLE) || dcfTimeFound) {
            int packetSize = udp.parsePacket();
            if (packetSize) {
                // IP-Adresse des Clients abrufen
                // Get the client's IP address
                IPAddress clientIP = udp.remoteIP();
                DEBUG_PRINTLN("[NTPD] Request from: " + clientIP.toString());

                // NTP-Paket lesen
                // Read NTP packet
                udp.read(ntpPacket, NTP_PACKET_SIZE);

                // Aktuelle Zeit holen
                // Get current time
                time_t currentTime = mktime(&timeinfo);

                // Antwort erstellen
                // Build response
                createNtpResponse(ntpPacket, currentTime);

                // Antwort senden
                // Send response
                udp.beginPacket(udp.remoteIP(), udp.remotePort());
                udp.write(ntpPacket, NTP_PACKET_SIZE);
                udp.endPacket();
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

        setLedOff();
    }

