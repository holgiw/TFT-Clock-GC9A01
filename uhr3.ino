// howl@gmx.de
// stationsuhr 05/2025 - 02/2026
// 
// https://github.com/holgiw?tab=repositories
// 
// 
// optimiert für ESP32-S2 Mini  (als Lolin S2 Pico compiliert)
// Filesystem: LittleFS
// TFT: GC9A01 / GC9D01 
// Partition: Default 4MB NO OTA, 2MB, 2MB
// TFT_eSPI: 2.5.34
// 
// DCF77: https://de.elv.com/p/elv-dcf-empfangsmodul-dcf-2-P091610/
// 
// Anpassungen in DCF77.cpp:   
//  zeile: 22: change #include <TimeLib.h> 
//  add IRAM_ATTR   in: void IRAM_ATTR DCF77::int0handler() {


#include <WiFi.h>
#include <WebServer.h> 
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
#include <Wire.h>
#include <RTClib.h>
#include <WiFiUdp.h>
#include <DCF77.h>  // https://forum.arduino.cc/t/dcf77-am-esp-32/1213608/7
                    // https://www.elkoba.com/magazin/eine-funkuhr-selbst-bauen-bauprojekt-dcf77/?srsltid=AfmBOorDPPTdSBRgDzH3HdjPTO9M7wl6MM68TusAIfYBCkYG6z13Rbt0


#include "prefs_keys.h"
#include "build_defs.h"


#include "config.h"        // Board-/Display-Auswahl, Pins, Timing-Makros
#include "globals.h"       // globale Objekte, Variablen, Structs
#include "declarations.h"  // Forward-Deklarationen aller Funktionen

#include "wifi_manager.h"      // WLAN: Verbindung, AP, Scan, Reconnect
#include "time_sync.h"         // RTC, DCF77, NTP
#include "display.h"           // Zifferblatt, Zeiger, Helligkeit, Touch
#include "presets_manager.h"   // Presets laden/speichern/wechseln
#include "webserver_routes.h"  // Webinterface (alle HTTP-Routen)
#include "system_utils.h"      // Tasten, Logging, Reset, Neustart


// Setup-Funktion
void setup() {

    setLedOn();

    Serial.begin(115200);

    // pin 12 Chipselect auf output und Low 

#if defined CS_2
    pinMode(CS_2, OUTPUT);
    
    setCS1(LOW);
    setCS2(HIGH);
#endif

    // async WLAN-Scan starten, damit die Netzwerke bereits erkannt werden, wenn der Nutzer das erste Mal die WLAN-Einstellungen öffnet   
    startWiFiScan();

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
        
    // workaround neuer Parameter
    String pingServer = preferences.getString(PK_PING_SERVER,"#");
    if (pingServer.startsWith("8.8.8.8") or pingServer == "#") {
        preferences.putString(PK_PING_SERVER,"1.1.1.1:80");
    }


    snprintf(version, sizeof(version), "%d-%02d-%02d %02d:%02d:%02d", BUILD_YEAR, BUILD_MONTH, BUILD_DAY, BUILD_HOUR, BUILD_MIN, BUILD_SEC);

    DEBUG_PRINTLN("[SETUP] start");
    DEBUG_PRINTLN(String("[SETUP] Build-Version: ") + version);

    // Bestehende Zifferblaetter im alten Standard-BMP-Format einmalig auf
    // das neue, platzsparende RLE-Format umstellen (siehe display.h).
    migrateFaceBmpsToRLE();

    // Bestehende Zeigersaetze im alten Standard-BMP-Format ebenfalls
    // einmalig auf RLE umstellen (siehe display.h).
    migrateHandBmpsToRLE();

    // Bereits vorhandene, schon RLE-komprimierte Zifferblaetter nachtraeglich
    // mit der Kreismaskierung fuer runde Displays versehen, falls sie vor
    // Einfuehrung dieser Funktion migriert/hochgeladen wurden (siehe display.h).
    remaskExistingFaceCorners();

    if (preferences.getString(PK_VERSION, "") != String(version)) {
        DEBUG_PRINTLN("[Preferences] Version change detected, updating version in preferences..");
        preferences.putString(PK_VERSION, String(version));
        preferences.putBool(PK_LOGGING_ENABLED, true); // Logging bei Version-Änderung aktivieren
        deleteAllLogFiles();
    }
    
    // Logging aktivieren, wenn in den Preferences aktiviert    
    loggingEnabled = preferences.getBool(PK_LOGGING_ENABLED, false);
    if (loggingEnabled) {
        uint16_t logfileNumber = preferences.getInt(PK_LOG_FILE_NUMBER, 0);
        logfileNumber++;
        preferences.putInt(PK_LOG_FILE_NUMBER, logfileNumber);
    }
    
    DEBUG_PRINTLN("[SETUP] Initializing..");
    
    // I2C-Scanner starten, um RTC zu erkennen
#if defined SDA_PIN && defined SCL_PIN
    Wire.begin(SDA_PIN, SCL_PIN);
    if (i2cScan() == 1) {

        if (rtc.begin()) {

            DateTime compileTime(F(__DATE__), F(__TIME__)); // Kompilierzeit

            DEBUG_PRINTLN("[RTC] found RTC");

            // Überprüfen, ob die RTC eine gültige Zeit zurückgibt
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
    tftType = "ILI9341";
#endif



    // Prüfen, ob PSRAM vorhanden ist
    if (psramFound() and ESP.getFreePsram() > 2 * (CLOCK_WIDTH * CLOCK_HEIGHT * sizeof(uint16_t))) {
        psramAvailable = true;
        DEBUG_PRINTLN("[INFO] found PSRAM");
    }
    else {
        psramAvailable = false;
        DEBUG_PRINTLN("[INFO] no PSRAM, use Hardware-Rotation");
#ifdef GC9D01
        preferences.putUChar(PK_TFT_ROTATION, 0);
#endif
    }
#ifndef GC9D01 // wird nur bei Display GC9D01 benötigt
    psramAvailable = false;
#endif


    
#define MAGIC_NUMBER 42  // Erster Start: Alle Preferences mit Standardwerten belegen    

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

        preferences.putUChar(PK_TFT_ROTATION, 0);
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
        preferences.putUChar(PK_TFT_ROTATION, 2);
#else
        preferences.putUChar(PK_TFT_ROTATION, 0);
#endif
        preferences.putBool(PK_ADC_INVERTED, false);
        preferences.putBool(PK_USE_TOUCH, false);

        preferences.putBool(PK_LOGGING_ENABLED, true);

        preferences.end();
        preferences.begin("clock", false);
    }

   
    loadLanguage(); // liest currentLanguage aus den Preferences (siehe translation.h)

    wifiActive = preferences.getBool(PK_WIFI_ACTIVE, true);   


    // NTP-Server initialisieren
    initializeNtpServers();

    timezone = preferences.getString(PK_TIMEZONE, TIMEZONE_DEFAULT);

    DEBUG_PRINTLN("[NTP] Timezone set to: " + timezone);

    stationMode = preferences.getBool(PK_STATION_MODE, true);
    smoothMinute = preferences.getBool(PK_SMOOTH_MINUTE, false);
    showSecondHand = preferences.getBool(PK_SHOW_SECOND_HAND, true);

    // Nabe
    uint32_t hubColorRgb = preferences.getLong(PK_CENTER_COLOR, 0xEC0016); //DB red
    hubColor = tft.color565((hubColorRgb >> 16) & 0xFF, (hubColorRgb >> 8) & 0xFF, hubColorRgb & 0xFF);
    hubSize = preferences.getUInt(PK_CENTER_SIZE, 6);

    lowThreshold = preferences.getInt(PK_LOW_THRESHOLD, 40);
    highThreshold = preferences.getInt(PK_HIGH_THRESHOLD, 60);
    minBrightness = preferences.getUChar(PK_MIN_BRIGHTNESS, 100);
    maxBrightness = preferences.getUChar(PK_MAX_BRIGHTNESS, 255);

    // Zeitabhängige Helligkeit aus Preferences

    brightStartHour = preferences.getUChar(PK_BRIGHT_START_HOUR, 7);
    brightEndHour = preferences.getUChar(PK_BRIGHT_END_HOUR, 21);

    adcInverted = preferences.getBool(PK_ADC_INVERTED, false);

    useTouch = preferences.getBool(PK_USE_TOUCH, false);


#if defined (GC9D01)  || defined (GC9A01_WITH_BACKLIGHT) 
    gammaBrightness = preferences.getFloat(PK_GAMMA_BRIGHTNESS, 2.2f);  // Gamma-Korrektur für Helligkeit
#endif

#ifdef BUTTON1
    pinMode(BUTTON1, INPUT_PULLDOWN);
#endif

    // auf Fotowiderstand prüfen
#ifdef ADC_3V

    uint16_t adcMin = 0;
    uint16_t adcMax = 0;
    uint16_t adcActual = 0;

    analogReadResolution(12);

    // ADC +3,3 / GND über GPIO
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
        // evtl überschreiben
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

#if defined CS_2
    digitalWrite(CS_2, LOW);
#endif

    tft.init();

    delay(75);
    tft.fillScreen(TFT_BLACK);
    

    tftRotation = preferences.getUChar(PK_TFT_ROTATION, 0);
    if (tftRotation > 3) {
        if (tftRotation == 90) tftRotation = 1;
        else if (tftRotation == 180) tftRotation = 2;
        else if (tftRotation == 270) tftRotation = 3;
        else tftRotation = 0;
        preferences.putUChar(PK_TFT_ROTATION, tftRotation);
    }

    selectedBackground = preferences.getString(PK_BACKGROUND, "/face_default.bmp");

    validateSelectedBackground();

#ifndef GC9D01
#if defined CS_2
    setCS2(LOW);
    tft.setRotation(tftRotation);
    setCS1(LOW);
#endif
    tft.setRotation(tftRotation);
#else
    if (!psramAvailable) {
        tftRotation = 0;
        preferences.putUChar(PK_TFT_ROTATION, tftRotation);
#if defined CS_2
        setCS2(LOW);
        tft.setRotation(tftRotation);
        setCS1(LOW);
#endif
        tft.setRotation(tftRotation);
        DEBUG_PRINTF("[TFT] Using stored rotation: %d\n", tftRotation);
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

    setupWebServer();
    webserver.begin();

    // DCF77-Interrupt einrichten
    dcf.Start();
    pinMode(DCF77_DATAPIN, INPUT_PULLUP);
    attachInterrupt(DCF77_DATAPIN, isr, CHANGE);

    // Wenn Button1 gedrückt oder BOOT_BUTTON gedrückt, alle Zugangsdaten löschen
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
    for (int i = 0; i < MAX_WLAN; i++) {
        // Dynamisch berechnete Schlüssel
        String ssidKey = pkSsid(i);
        String passKey = pkPass(i);
        wifiSsid[i] = preferences.getString(ssidKey.c_str(), "");
        wifiPass[i] = preferences.getString(passKey.c_str(), "");
    }

    
    // AP starten, wenn keine SSID gespeichert
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
        while (isScanning) {
            checkWiFiScan();
            delay(50);
            if (loggingEnabled) Serial.print("");
        }
        if (loggingEnabled) Serial.println("");
    }
   
    // versuche Verbindung mit der letzten SSID
    if (!foundLastSSID or connectWiFi(number, true) != CONNECTED) {

        // Wenn Verbindung fehlschlägt, scannen und vergleichen
        DEBUG_PRINTLN("[WiFi] Connection failed. Looking for available networks..");

        // Durchsuche gefundene Netzwerke nach gespeicherten SSIDs  
        for (int i = 0; i < MAX_WLAN; i++) {
            String availableSSID = availableNetworks[i].ssid;
            if (availableSSID == "") continue;
            //  DEBUG_PRINTLN("Found network: " + availableSSID);
            for (int j = 0; j < MAX_WLAN; j++) {

                if (j == number) continue; // überspringe bereits versuchte SSID
                if (trim(wifiSsid[j]) == "") continue; // überspringe leere SSID

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
                if (dcf.getUTCTime() > 0) {    
                    dcfTimeFound = true; // gültige Zeit gefunden    
                    return;
                }
                tft.setCursor(20, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 8));
                tft.print(translate("Waiting"));
                tft.print("...");
                delay(100);               
            }
        }


        // // Alle Verbindungsversuche fehlgeschlagen, starte AP wenn RTC nicht verfügbar oder ungültig
        if (rtcOk != RTC_AVAILABLE) {   
            DEBUG_PRINTLN("[WiFi] Starting Access Point due to failed connections and no valid RTC");
            startAP();
        }
    }


    setupNTP();
  
    if (useTouch) {
        // Touch-Eingang initialisieren
        enableTouch();
    }

    loadPresets();

    if (rtcOk == RTC_AVAILABLE) {
        // UDP starten
        udp.begin(NTP_PORT);
        DEBUG_PRINTLN("[NTPD] NTP Server started");
    }

    DEBUG_PRINTLN("[SETUP] Boot complete, free heap: " + String(ESP.getFreeHeap()) + " bytes");
    checkHeapWarning("Setup Ende");

    setLedOff();
}


// Main-Loop
void loop() {


#if defined CS_2
    cs = !cs;
    if (cs == true) {
        setCS1(LOW);
    }
    else {
        setCS2(LOW);
    }
#endif

    //scanWPS(); // WPS-Scan durchführen

    if (WiFi.getMode() == WIFI_STA && WiFi.isConnected()) {
        ipAddress = WiFi.localIP().toString();
    }
    else {
        ipAddress = WiFi.softAPIP().toString();
    }

    // DCF77-Zeit abrufen
    getDCF77Time();

    // Überprüfen, ob seit dem letzten Aufruf Zeit vergangen ist
    if (millis() - lastNTPUpdate > WAIT_1h) {
        if (timeinfo.tm_sec < 10 || timeinfo.tm_sec > 55) {
            lastNTPUpdate = millis() + 15 * 1000;
        }
        else {
            setupNTP();
            lastNTPUpdate = millis();
        }
    
    }

    if (timeinfo.tm_sec == 0) setLedOff(); // wg.DCF

    // Wenn im AP-Modus: DNS-Requests abarbeiten (captive portal)
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
      // win:  w32tm /stripchart /computer:192.168.0.214
    if ((WiFi.getMode() == WIFI_STA && rtcOk == RTC_AVAILABLE) || dcfTimeFound) {
        int packetSize = udp.parsePacket();
        if (packetSize) {
            // IP-Adresse des Clients abrufen
            IPAddress clientIP = udp.remoteIP();
            DEBUG_PRINTLN("[NTPD] Request from: " + clientIP.toString());

            // NTP-Paket lesen
            udp.read(ntpPacket, NTP_PACKET_SIZE);

            // Aktuelle Zeit holen
            time_t currentTime = mktime(&timeinfo);

            // Antwort erstellen
            createNtpResponse(ntpPacket, currentTime);

            // Antwort senden
            udp.beginPacket(udp.remoteIP(), udp.remotePort());
            udp.write(ntpPacket, NTP_PACKET_SIZE);
            udp.endPacket();
        }
    }

    checkButton();
    updateBrightness();

    if (useTouch) {
        // Touch erst aktivieren, wenn die Startverzögerung vorbei ist
        if (!touchEnabled && touchEnableAt != 0 && millis() >= touchEnableAt) {
            touchEnabled = true;
            DEBUG_PRINTLN("[TOUCH] Enabled");
        }

        if (touchEnabled) {
            // Touch-Input prüfen und ggf. Hintergrund wechseln
            checkTouchInput();
        }
    }

    // restart im AP Mode nach 30 Minuten
    if (softAPIP == true) {
        if (millis() - softAPIPstart > WAIT_30m) {
            //        ESP.restart();
        }
    }

#ifdef ILI9341
    // Datum und Uhrzeit auf dem TFT ausgeben
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(3);

    String hourStr = String(timeinfo.tm_hour);
    int xPos = 50;
    // vornull entfernen wenn vorhanden
    if (hourStr.startsWith("0")) {
        hourStr = hourStr.substring(1);
        xPos = 30; // etwas weiter links positionieren, wenn nur 1-stellige Stunde
    }

    // Uhrzeit auf dem TFT ausgeben
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


