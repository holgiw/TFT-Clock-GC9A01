// howl@gmx.de
// stationsuhr 05/2025 - 01/2026
// 
// https://github.com/holgiw?tab=repositories
// 
// 
// optimiert für ESP32-S2 Mini  (als Lolin S2 Pico compiliert)
// Filesystem: LittleFS
// TFT: GC9A01 / GC9D01 
// Partition: Default 4MB NO OTA, 2MB, 2MB
// TFT_eSPI: 2.5.34

// Prozessor
#define ESP32_S2  //only ESP32-S2 supported

// select TFT
#define GC9A01
//#define GC9A01_WITH_BACKLIGHT
//#define GC9D01
//#define ILI9341 


 #define DEBUG

// Debug-Makros
#ifdef DEBUG
#define DEBUG_PRINT(x) { Serial.print(x); logToFile(String(x)); }
#define DEBUG_PRINTLN(x) { Serial.println(x); logToFile(String(x)); }
#define DEBUG_PRINTF(...) { \
    char buffer[256]; /* Puffer für die formatierte Nachricht */ \
    snprintf(buffer, sizeof(buffer), __VA_ARGS__); \
    Serial.print(buffer); \
    logToFile(String(buffer)); \
}
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINTF(...)
#endif

// time server & timezone default
#define NTP_SERVER_1 "pool.ntp.org"
#define NTP_SERVER_2 "time.nist.gov"
#define NTP_SERVER_3 "time.google.com"
#define TIMEZONE_DEFAULT "CET-1CEST,M3.5.0,M10.5.0/3" // Central European Time


#include <WiFi.h>
#include <WebServer.h>
// #include <ESPAsyncWebServer.h> // inkompatibel mit ide 3.0.0
#include <TFT_eSPI.h>
#include <Preferences.h>
#include <LittleFS.h>/
#include <set>
#include <base64.h>
#include "nvs_flash.h"
#include <DNSServer.h>
#include <ESPMDNS.h>
#include <ESPping.h>
#include <map>

#include "build_defs.h"


TFT_eSPI tft = TFT_eSPI();
WebServer webserver(80);
Preferences preferences;
DNSServer dnsServer;


#ifdef ESP32_S2  // Lolin S2 Pico
// ##############################################################################
// wires
//               ESP32 PIN    TFT
//               3.3V         vcc     3v3             red
//               GND          gnd     ground          blue
//  see C:\Users\hwage\Documents\Arduino\libraries\TFT_eSPI\user_setups\Setup304__ESP32S3_GC9D01.h
// or   https://github.com/holgiw/TFT-Clock-GC9A01/blob/master/PCB/ESP32-S2%20GC9A01.jpg


#define LED_BOARD 15 // BUILTIN LED

#define ADC_3V 1
#define ADC_PIN 2
#define ADC_GND 4

#define BUTTON1 16

// Touch 
#define TOUCH_PIN 9

// Transparent in R5G6B5 RGB(16)
#define TRANSPARENT_COLOR 0x0120    

#if defined (GC9D01)  || defined(GC9A01_WITH_BACKLIGHT)  
#define TFT_Backlight 3  // Backlight
#define BACKLIGHT_CHANNEL 0  // PWM channel
#define BACKLIGHT_FREQ 5000
#define BACKLIGHT_RESOLUTION 8 
#endif
    
#endif

#if defined GC9A01 || defined(GC9A01_WITH_BACKLIGHT) 
#include "graphic/240/clock_default.h"

#define TFT_WIDTH 240
#define TFT_HEIGHT 240

#define CLOCK_WIDTH 240   
#define CLOCK_HEIGHT 240

#define HAND_WIDTH 21
#define HAND_HEIGHT 131

#define TFT_TEXT_SIZE 2
#endif

#ifdef GC9D01
#include "graphic/160/clock_default.h"

#define TFT_WIDTH 160
#define TFT_HEIGHT 160

#define CLOCK_WIDTH 160
#define CLOCK_HEIGHT 160

#define HAND_WIDTH 13
#define HAND_HEIGHT 86

#define TFT_TEXT_SIZE 1
#endif

#ifdef ILI9341
#include "graphic/240/clock_default.h"

#define TFT_WIDTH 240
#define TFT_HEIGHT 320

#define CLOCK_WIDTH 240
#define CLOCK_HEIGHT 240

#define HAND_WIDTH 21
#define HAND_HEIGHT 131

#define TFT_TEXT_SIZE 2
#endif

String currentLanguage = "de"; // Standardmäßig Deutsch

char version[32]; // Build-Version


// --- Touch / Debounce State ---
unsigned long touchLastMillis = 0;
const unsigned long TOUCH_DEBOUNCE_MS = 300;
bool touchLastState = false;
// --- Touch enable flag: aktivieren erst nach Setup-Initialisierung ---
bool touchEnabled = false;
unsigned long touchEnableAt = 0; // Timestamp wann Touch freigeschaltet wird (ms)
bool useTouch = false; // Touch verwenden

bool wifiActive = true;    

String tft_type = "UNKNOWN";

TFT_eSprite backgroundSprite = TFT_eSprite(&tft);
TFT_eSprite hourHandSprite = TFT_eSprite(&tft);
TFT_eSprite minuteHandSprite = TFT_eSprite(&tft);
TFT_eSprite secondHandSprite = TFT_eSprite(&tft);

#define MAX_WLAN 15
String wifi_ssid[MAX_WLAN];
String wifi_pass[MAX_WLAN];

unsigned long lastNTPRetry = 0;
//const unsigned long retryIntervalNTPms = 12 * 60 * 60 * 1000; // 12h
const unsigned long retryIntervalNTPms = 10 * 60 * 1000; // 10 Minuten

String timezone = TIMEZONE_DEFAULT;

unsigned long lastCheck = 0;

bool loggingEnabled = false;  
String logFileName = "/log.log";

char ntpServers[4][64] = { NTP_SERVER_1, NTP_SERVER_2, NTP_SERVER_3, "fritz.box"};
const int numServers = sizeof(ntpServers) / sizeof(ntpServers[0]);

bool initial = true;

String selectedBackground = "/face_default.bmp";

// Presets
#define MAX_PRESETS 15
struct Preset {
    String name;
    String url;
};
Preset presets[MAX_PRESETS];

bool stationMode;
bool smoothMinute;
bool showSecondHand;

int hourHandWidth = HAND_WIDTH;
int minuteHandWidth = HAND_WIDTH;
int secondHandWidth = HAND_WIDTH;

// nabe
uint16_t hub_color = 0;
uint8_t hub_size = 0;


bool firstRun = true;

uint8_t tftRotation = 0;

float fastSecond = 972.0f;  // Geschwindigkeit des Sekundenzeigers gegenüber der realen Zeit 

uint16_t rowBuffer[CLOCK_WIDTH];

static bool psramAvailable = false;

bool adcInverted = false; // Standardmäßig nicht invertiert
uint16_t adc_min = 0;
uint16_t adc_max = 0;   
bool use_adc = false; 
bool photoresistorFound = false;


uint8_t currentBrightness = 255;
uint8_t lastAppliedBrightness = 255;
uint8_t targetBrightness = 255;
int lowThreshold = 40;
int highThreshold = 60;
uint8_t minBrightness = 100;  // 
uint8_t maxBrightness = 255;  // Obergrenze 

// Zeitabhängige Helligkeit
uint8_t brightStartHour = 8;       // inkl. (z.B. 8)
uint8_t brightEndHour = 22;        // exkl. (z.B. 20)

#if defined (GC9D01)  || defined(GC9A01_WITH_BACKLIGHT) 
float gammaBrightness = 2.2f;  // Gamma-Korrektur für Helligkeit
#endif

#define ADC_SMOOTHING 20
int adcHistory[ADC_SMOOTHING];
int adcIndex = 0;
int currentAdcAvg = 0;  // global definieren
int currentLightPercent = 0;  // global speichern für Anzeige

File uploadFile;
String uploadFilePath = "";
bool uploadSuccess = false;

int lastResetWeek = -1;
int currentWeek = -1;

// MAC Adresse
uint8_t mac[6];
char hostname[32];
bool pingHostname = false;

uint16_t* clockFaceBuffer = nullptr;

bool softAPIP = false;  // Flag für SoftAP IP
long softAPIPstart = 0;  // Startzeit für SoftAP IP

struct WifiNetwork {
    String ssid;
    int rssi;
    int enc;
};
#define MAX_NETWORKS MAX_WLAN
WifiNetwork foundNetworks[MAX_NETWORKS];
int foundNetworkCount = 0;
bool isScanning = false;

//Übersetzungen fÜr verschiedene Sprachen
#include "translation.h"



// Setup-Funktion
void setup() {

    Serial.begin(115200);

    unsigned long serialStart = millis();
    while (!Serial && (millis() - serialStart < 1000)) {
        delay(10);
    }
    delay(1500); // Warten, bis die serielle Verbindung hergestellt ist

    if (!LittleFS.begin(true)) {
        Serial.println("[LittleFS] Mount Failed");
    }


    


    preferences.begin("clock", false);

    sprintf(version, "%d-%02d-%02d %02d:%02d%:%02d", BUILD_YEAR, BUILD_MONTH, BUILD_DAY, BUILD_HOUR, BUILD_MIN, BUILD_SEC);

    DEBUG_PRINTLN("[SETUP] start");
    DEBUG_PRINTLN(String("[SETUP] Build-Version: ") + version);

    if (preferences.getString("version", "") != String(version)) {
        DEBUG_PRINTLN("[Preferences] Version change detected, updating version in preferences...");
        preferences.putString("version", String(version));
        deleteAllLogFiles();
    }
    setLogFileName();



#if defined GC9A01 || defined (GC9A01_WITH_BACKLIGHT)
    tft_type = "GC9A01";
#elif defined GC9D01    
    tft_type = "GC9D01";
#else   
    tft_type = "ILI9341";
#endif



    // Pr%uuml;fen, ob PSRAM vorhanden ist
    if (psramFound() and ESP.getFreePsram() > 2 * (CLOCK_WIDTH * CLOCK_HEIGHT * sizeof(uint16_t))) {
        psramAvailable = true;
        DEBUG_PRINTLN("[INFO] PSRAM gefunden.");
    }
    else {
        psramAvailable = false;
        DEBUG_PRINTLN("[INFO] Kein PSRAM gefunden, Hardware-Rotation wird verwendet.");
#ifdef GC9D01
        preferences.putUChar("tftRotation", 0);
#endif
    }
#ifndef GC9D01 // wird nur bei Display GC9D01 benötigt
    psramAvailable = false;
#endif



#define MAGIC_NUMBER 42

    if (preferences.getInt("firstStart", 0) != MAGIC_NUMBER) {
        DEBUG_PRINTLN("[Preferences] First start detected, initializing...");

        preferences.putInt("firstStart", MAGIC_NUMBER);

        preferences.putString("language", "en");

        preferences.putBool("wifiActive", true);

        for (int i = 0; i < MAX_WLAN; i++) {
            String ssid_key = "ssid" + String(i - 1);
            String pass_key = "pass" + String(i - 1);                       

            preferences.putString(ssid_key.c_str(), "");
            preferences.putString(pass_key.c_str(), "");
        }
        



        preferences.putInt("lastWLan", 0);

        preferences.putString("ntpServer1", NTP_SERVER_1);
        preferences.putString("ntpServer2", NTP_SERVER_2);

        preferences.putString("timezone", TIMEZONE_DEFAULT);

        preferences.putUChar("tftRotation", 0);
        preferences.putString("handset", "default");
        preferences.putString("background", "/face_default.bmp");

        preferences.putBool("stationMode", true);
        preferences.putBool("showSecondHand", true);
        preferences.putBool("smoothMinute", false);


#if defined (GC9D01)  || defined (GC9A01_WITH_BACKLIGHT)
        preferences.putUChar("minBrightness", 5);
#else
        preferences.putUChar("minBrightness", 100);
#endif
        preferences.putUChar("maxBrightness", 255);

        preferences.putFloat("gammaBrightness", 2.2f);  // Gamma-Korrektur f%uuml;r Helligkeit

#if defined (GC9D01)  || defined (GC9A01_WITH_BACKLIGHT)
        preferences.putInt("lowThreshold", 1);
        preferences.putInt("highThreshold", 255);
#else
        preferences.putInt("lowThreshold", 40);
        preferences.putInt("highThreshold", 60);
#endif

        preferences.putUInt("centerColor", 0xEC0016);

        if (tft_type == "GC9A01" || tft_type == "ILI9341") {
            preferences.putUInt("centerSize", 6);
        }
        if (tft_type == "GC9D01") {
            preferences.putUInt("centerSize", 3);
        }

#if defined GC9A01_WITH_BACKLIGHT
        preferences.putUChar("tftRotation", 2);
#else
        preferences.putUChar("tftRotation", 0);
#endif
        preferences.putBool("adcInverted", false);
        preferences.putBool("useTouch", false);

        preferences.putBool("loggingEnabled", false);


        preferences.end();
        preferences.begin("clock", false);
    }


    loggingEnabled = preferences.getBool("loggingEnabled", false);

    currentLanguage = preferences.getString("language", "en");

    wifiActive = preferences.getBool("wifiActive", true);   

    strncpy(ntpServers[0], preferences.getString("ntpServer1", NTP_SERVER_1).c_str(), sizeof(ntpServers[0]) - 1);
    ntpServers[0][sizeof(ntpServers[0]) - 1] = '\0'; // Null-terminieren

    strncpy(ntpServers[1], preferences.getString("ntpServer2", NTP_SERVER_2).c_str(), sizeof(ntpServers[1]) - 1);
    ntpServers[1][sizeof(ntpServers[1]) - 1] = '\0'; // Null-terminieren

    timezone = preferences.getString("timezone", TIMEZONE_DEFAULT);

    DEBUG_PRINTLN("[NTP] NTP-Server: " + String(ntpServers[0]) + " / " + String(ntpServers[1]));
    DEBUG_PRINTLN("timezone is: " + timezone);

    stationMode = preferences.getBool("stationMode", true);
    smoothMinute = preferences.getBool("smoothMinute", false);
    showSecondHand = preferences.getBool("showSecondHand", true);

    // Nabe
    uint32_t hub_color_RGB = preferences.getLong("centerColor", 0xEC0016); //DB red
    hub_color = tft.color565((hub_color_RGB >> 16) & 0xFF, (hub_color_RGB >> 8) & 0xFF, hub_color_RGB & 0xFF);
    // DEBUG_PRINTF("[HUB] Color RGB: #%06X 565: 0x%04X", hub_color_RGB, hub_color);
    hub_size = preferences.getUInt("centerSize", 6);

    lowThreshold = preferences.getInt("lowThreshold", 40);
    highThreshold = preferences.getInt("highThreshold", 60);
    minBrightness = preferences.getUChar("minBrightness", 100);
    maxBrightness = preferences.getUChar("maxBrightness", 255);

    // Zeitabh&auml;ngige Helligkeit aus Preferences

    brightStartHour = preferences.getUChar("brightStart", 8);
    brightEndHour = preferences.getUChar("brightEnd", 22);

    adcInverted = preferences.getBool("adcInverted", false);

    useTouch = preferences.getBool("useTouch", false);


#if defined (GC9D01)  || defined (GC9A01_WITH_BACKLIGHT) 
    gammaBrightness = preferences.getFloat("gammaBrightness", 2.2f);  // Gamma-Korrektur f%uuml;r Helligkeit
#endif

    pinMode(BUTTON1, INPUT_PULLDOWN);

    // auf Fotowiderstand prüfen
#ifdef ADC_3V
    analogReadResolution(12);

    // ADC +3,3 / GND über GPIO
    pinMode(ADC_GND, OUTPUT);
    pinMode(ADC_3V, OUTPUT);

    digitalWrite(ADC_GND, false);
    digitalWrite(ADC_3V, false);
    delay(100);
    adc_min = analogRead(ADC_PIN);

    digitalWrite(ADC_GND, true);
    digitalWrite(ADC_3V, true);
    delay(100);
    adc_max = analogRead(ADC_PIN);

    DEBUG_PRINTF("ADC min: %d max: %d", adc_min, adc_max);
    Serial.println();

    if (adc_min < 1000 && adc_max > 2000) {
        DEBUG_PRINTF("ADC act: %d", analogRead(ADC_PIN));
        Serial.println();
        DEBUG_PRINTLN("found photoresistor");
        digitalWrite(ADC_GND, 0);
        digitalWrite(ADC_3V, 1);
        use_adc = true;
        photoresistorFound = true;
        // evtl überschreiben
        use_adc = preferences.getBool("use_adc", true);
    }
    else {
        pinMode(ADC_GND, INPUT);
        pinMode(ADC_3V, INPUT);
        use_adc = false;
    }

#else
    use_adc = false;
#endif

    minBrightness = preferences.getUChar("minBrightness", 100);
    maxBrightness = preferences.getUChar("maxBrightness", 255);

    //  smoothMinute = preferences.getBool("smoothMinute", false);
    //  DEBUG_PRINTLN("[SETUP] smoothMinute: " + String(smoothMinute));    

    pinMode(LED_BOARD, OUTPUT); digitalWrite(LED_BOARD, HIGH);

    Serial.println("debug is:" + String(loggingEnabled ? "enabled" : "disabled"));

    tft.init();
    delay(50);

    tft.fillScreen(TFT_BLACK);

    tftRotation = preferences.getUChar("tftRotation", 0);
    if (tftRotation > 3) {
        if (tftRotation == 90) tftRotation = 1;
        else if (tftRotation == 180) tftRotation = 2;
        else if (tftRotation == 270) tftRotation = 3;
        else tftRotation = 0;
        preferences.putUChar("tftRotation", tftRotation);
    }

    selectedBackground = preferences.getString("background", "/face_default.bmp");


    validateSelectedBackground();

#ifndef GC9D01
    tft.setRotation(tftRotation);
#else
    if (!psramAvailable) {
        tftRotation = 0;
        preferences.putUChar("tftRotation", tftRotation);
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


    // Schleife mit sizeof, um die Anzahl der Elemente im Array zu berechnen
    for (int i = 0; i < MAX_WLAN; i++) {

        // Dynamisch berechnete Schlüssel
        String ssidKey = "ssid" + String(i + 1);
        String passKey = "pass" + String(i + 1);

        // Werte speichern
        wifi_ssid[i] = preferences.getString(ssidKey.c_str(), "");
        wifi_pass[i] = preferences.getString(passKey.c_str(), "");
    }

    scanAndCacheNetworks();

   
    if (digitalRead(BUTTON1) == HIGH) {
        for (int i = 0; i < MAX_WLAN; i++) {
            // Dynamisch berechnete Schlüssel
            String ssidKey = "ssid" + String(i + 1);
            String passKey = "pass" + String(i + 1);
            wifi_ssid[i] = "";
            wifi_pass[i] = "";
        }
        startAP();
    }
    // wenn noch keine SSID1 gespeichert → sofort AP starten (erleichtert Erstkonfiguration)
    else if (wifi_ssid[0].length() == 0) {
        DEBUG_PRINTLN("[WiFi] No stored credentials — starting AP for configuration");
        startAP();
    }
    else {

        DEBUG_PRINTLN("[TFT] Selected background: " + selectedBackground);

        uint32_t number = preferences.getInt("lastWLan", 0);
        DEBUG_PRINTLN("[WiFi] Last successful WLAN number: " + String(number + 1) + " " + wifi_ssid[number]);

        
        bool foundLastSSID = false;
        for (int i = 0; i < MAX_NETWORKS; i++) {
            if (wifi_ssid[number] == foundNetworks[i].ssid) {
                DEBUG_PRINTLN("[WiFi] Last connected SSID found in scan: " + foundNetworks[i].ssid);
                foundLastSSID = true;
                break;
            }
        }


        if (!foundLastSSID or !connectWiFi(number, true)) {
           
            // Wenn Verbindung fehlschlägt, scannen und vergleichen
            DEBUG_PRINTLN("[WiFi] Connection failed. Scanning for available networks...");


            for (int i = 0; i < MAX_NETWORKS; i++) {
                String availableSSID = foundNetworks[i].ssid;
                if (availableSSID == "") continue;
                DEBUG_PRINTLN("Gefundenes Netzwerk: " + availableSSID);
                for (int j = 0; j < MAX_WLAN; j++) {

                    if (j == number) continue; // überspringe bereits versuchte SSID
                    if (trim(wifi_ssid[j]) == "") continue; // überspringe leere SSID

                    DEBUG_PRINTLN("vergleiche " + wifi_ssid[j] + " mit " + availableSSID);
                                        
                    if (wifi_ssid[j] == availableSSID) {
                        DEBUG_PRINTLN("[WiFi] Found matching network: " + availableSSID);
                        if (connectWiFi(j, true)) {
                            DEBUG_PRINTLN("[WiFi] Connected to " + availableSSID + " using stored credentials.");
                            preferences.putInt("lastWLan", j);
                            return;
                        }
                    }

                }                
            }
            DEBUG_PRINTLN("[WiFi] No matching networks found.");
            startAP();
        }          

    }


    //if (WiFi.getMode() != WIFI_STA) startAP();
    setupNTP();
    setupWebServer();
    webserver.begin();

    digitalWrite(LED_BOARD, LOW);

    if (useTouch) {
        // Touch-Eingang initialisieren
        enableTouch();
    }

    loadPresets();

    //startWiFiScan(); // Starte den Scan, falls nicht bereits aktiv

}


void enableTouch() {
    touchEnabled = true;
    pinMode(TOUCH_PIN, INPUT_PULLDOWN);

    // Touch erst nach kurzer Verzögerung aktivieren (verhindert frühe Reads während Init)
    touchEnableAt = millis() + 1000; // 1000 ms Verzögerung
    DEBUG_PRINTLN("[TOUCH] Touch aktiviert.");
}

void disableTouch() {
    touchEnabled = false;
    pinMode(TOUCH_PIN, INPUT);
    DEBUG_PRINTLN("[TOUCH] Touch deaktiviert.");
}




void loadPresets() {
    String espIP = "http://" + WiFi.localIP().toString(); // Aktuelle IP-Adresse des ESP32

    for (int i = 0; i < MAX_PRESETS; i++) {
        String nameKey = "preset" + String(i) + "_name";
        String urlKey = "preset" + String(i) + "_url";

        presets[i].name = preferences.getString(nameKey.c_str(), "");
        presets[i].url = preferences.getString(urlKey.c_str(), "");

        // Ersetze die gespeicherte IP durch die aktuelle IP des ESP
        if (presets[i].url.startsWith("http://")) {
            int ipEnd = presets[i].url.indexOf('/', 7); // Suche nach dem Ende der IP-Adresse
            if (ipEnd != -1) {
                presets[i].url = espIP + presets[i].url.substring(ipEnd); // Ersetze die IP
            }
            else {
                presets[i].url = espIP; // Nur die IP ohne Pfad
            }
        }
    }
}

void savePresets() {
    String espIP = "http://" + WiFi.localIP().toString(); // Aktuelle IP-Adresse des ESP32

    for (int i = 0; i < MAX_PRESETS; i++) {
        String nameKey = "preset" + String(i) + "_name";
        String urlKey = "preset" + String(i) + "_url";

        // Ersetze eine vorhandene IP-Adresse durch die aktuelle IP des ESP
        if (presets[i].url.startsWith("http://")) {
            int ipEnd = presets[i].url.indexOf('/', 7); // Suche nach dem Ende der IP-Adresse
            if (ipEnd != -1) {
                presets[i].url = espIP + presets[i].url.substring(ipEnd); // Ersetze die IP
            }
            else {
                presets[i].url = espIP; // Nur die IP ohne Pfad
            }
        }

        preferences.putString(nameKey.c_str(), presets[i].name);
        preferences.putString(urlKey.c_str(), presets[i].url);
    }
}

void createPresetFromPreferences() {
    // Suche das erste leere Preset
    int presetIndex = -1;
    for (int i = 0; i < MAX_PRESETS; i++) {
        if (presets[i].name.isEmpty() && presets[i].url.isEmpty()) {
            presetIndex = i;
            break;
        }
    }

    // Wenn kein leeres Preset gefunden wurde, abbrechen
    if (presetIndex == -1) {
        DEBUG_PRINTLN("[Preset] No empty preset slot available.");
        return;
    }

    // Lese die aktuellen Einstellungen aus den Preferences
    String background = preferences.getString("background", "/face_default.bmp");
    String handset = preferences.getString("handset", "default");
   
    uint8_t rotation = preferences.getUChar("tftRotation", 0);
    bool stationMode = preferences.getBool("stationMode", true);
    bool showSecondHand = preferences.getBool("showSecondHand", true);
    bool smoothMinute = preferences.getBool("smoothMinute", false);
    uint8_t hubSize = preferences.getUInt("centerSize", 6);
    uint32_t hubColor = preferences.getLong("centerColor", 0xEC0016);

    // Hole die aktuelle IP-Adresse des ESP
    String espIP = "http://" + WiFi.localIP().toString();

    // Erstelle die URL mit den aktuellen Einstellungen
    String url = espIP + "/api/setMode?";
    url += "face=" + background;
    url += "&handSet=" + handset;
    
    url += "&rotation=" + String(rotation);
    url += "&stationMode=" + String(stationMode ? "true" : "false");
    url += "&showSecondHand=" + String(showSecondHand ? "true" : "false");
    url += "&smoothMinute=" + String(smoothMinute ? "true" : "false");
    url += "&hubSize=" + String(hubSize);
    url += "&hubColor=" + String(hubColor, HEX);

    // Speichere das Preset
    String presetName = "Preset_" + String(presetIndex + 1);
    presets[presetIndex].name = presetName;
    presets[presetIndex].url = url;

    // Schreibe das Preset in die Preferences
    String nameKey = "preset" + String(presetIndex) + "_name";
    String urlKey = "preset" + String(presetIndex) + "_url";
    preferences.putString(nameKey.c_str(), presetName);
    preferences.putString(urlKey.c_str(), url);

    DEBUG_PRINTLN("[Preset] Created preset: " + presetName);
    DEBUG_PRINTLN("[Preset] URL: " + url);
}

// Haupt-Loop
void loop() {

    // Wenn im AP-Modus: DNS-Requests abarbeiten (captive portal)
    if (softAPIP) {
        dnsServer.processNextRequest();
    }
       

    webserver.handleClient();

    if (WiFi.getMode() == WIFI_STA) {

        updateClock();
        
      //  checkWiFiScan(); // Überprüfe den Status des Scans
        if (wifiActive || WiFi.isConnected()) {
            checkNTPRetry();
            checkWiFiReconnect();
            checkNightlyTimeSync();
            checkWeeklyRestart();
        }
        initial = false;
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
        if (millis() - softAPIPstart > (30 * 60000)) {
            ESP.restart();
        }
    }

}

/// <summary>
/// Passt die Helligkeit eines Pixels basierend auf der aktuellen Helligkeitseinstellung an.
/// </summary>
/// <param name="pixel">Der 16-Bit-Farbwert des Pixels, der angepasst werden soll.</param>
/// <returns>Der angepasste 16-Bit-Farbwert des Pixels, basierend auf der aktuellen Helligkeit. Wenn die Helligkeit maximal ist oder der Pixel transparent/schwarz ist, wird der urspr%uuml;ngliche Wert zur%uuml;ckgegeben.</returns>
 
uint16_t setPixelBrightness(uint16_t pixel) {

#ifdef TFT_Backlight
    return pixel;
#else

    // Wenn die Helligkeit maximal ist oder der Pixel transparent/schwarz ist, direkt zurückgeben
    if (pixel == TRANSPARENT_COLOR || pixel == 0x0000 || currentBrightness == 255) {
        return pixel;
    }

    // Multiplikator einmal berechnen (statt 3x Division)
    uint32_t brightnessFactor = (uint32_t)currentBrightness;

    // Farben extrahieren
    uint32_t r = (pixel & 0xF800);
    uint32_t g = (pixel & 0x07E0);
    uint32_t b = (pixel & 0x001F);

    // Multiplikation mit Brightness (optimiert, kein Shift nötig)
    r = ((r * brightnessFactor) >> 8) & 0xF800;
    g = ((g * brightnessFactor) >> 8) & 0x07E0;
    b = ((b * brightnessFactor) >> 8) & 0x001F;

    // Farbwerte zusammenfügen
    return r | g | b;
#endif
}



/// <summary>
/// Lädt das Zifferblatt, indem entweder ein benutzerdefinierter Hintergrund oder ein Standardhintergrund verwendet wird.           
/// </summary>  
void loadClockFace() {
    // Prüfen, ob Buffer schon existiert
    if (!clockFaceBuffer) {
        size_t bufSize = CLOCK_WIDTH * CLOCK_HEIGHT * sizeof(uint16_t);
        if (psramFound() and ESP.getFreePsram() > bufSize) {
            DEBUG_PRINTLN("[clockFaceBuffer] allocate psram");
            clockFaceBuffer = (uint16_t*)ps_malloc(bufSize);
        }
        else {
            DEBUG_PRINTLN("allocte ram: " + bufSize);
            DEBUG_PRINTLN("[clockFaceBuffer] allocate ram");
            clockFaceBuffer = (uint16_t*)malloc(bufSize);
        }
        if (!clockFaceBuffer) {
            DEBUG_PRINTLN("Fehler: clockFaceBuffer konnte nicht allokiert werden!");
            return;
        }


        if (!selectedBackground.startsWith("/")) selectedBackground = "/" + selectedBackground;
        // Bild aus Datei laden und dekodieren
        if (LittleFS.exists(selectedBackground)) {
            File bmp = LittleFS.open(selectedBackground, "r");
            if (bmp) {
                uint8_t header[54];
                if (bmp.read(header, 54) == 54 && header[0] == 'B' && header[1] == 'M') {
                    int32_t width = *(int32_t*)&header[18];
                    int32_t height = *(int32_t*)&header[22];
                    uint16_t bpp = *(uint16_t*)&header[28];
                    uint32_t offset = *(uint32_t*)&header[10];
                    if (width == CLOCK_WIDTH && abs(height) == CLOCK_HEIGHT && bpp == 16) {
                        bool flip = height > 0;
                        height = abs(height);
                        bmp.seek(offset);
                        for (int y = 0; y < height; y++) {
                            int row = flip ? height - 1 - y : y;
                            bmp.read((uint8_t*)&clockFaceBuffer[row * CLOCK_WIDTH], CLOCK_WIDTH * 2);
                        }
                    }
                }
                bmp.close();
            }
        }
        else {
            // Fallback: Standard-Zifferblatt aus Array kopieren
            memcpy(clockFaceBuffer, clockFace, CLOCK_WIDTH * CLOCK_HEIGHT * sizeof(uint16_t));
        }    
    }

    // Breiten aus Dateinamen extrahieren
    parseBackgroundFilename(selectedBackground, hourHandWidth, minuteHandWidth, secondHandWidth);
    updateHandWidths(hourHandWidth, minuteHandWidth, secondHandWidth);

    // Buffer ins Sprite kopieren (mit Helligkeit)
    for (int y = 0; y < CLOCK_HEIGHT; y++) {
        for (int x = 0; x < CLOCK_WIDTH; x++) {
            rowBuffer[x] = setPixelBrightness(clockFaceBuffer[y * CLOCK_WIDTH + x]);
        }
        backgroundSprite.pushImage(0, y, CLOCK_WIDTH, 1, rowBuffer);
    }

}

// Buffer freigeben, wenn ein neues Zifferblatt gewählt wird
void freeClockFaceBuffer() {
    if (clockFaceBuffer) {
        free(clockFaceBuffer);
        clockFaceBuffer = nullptr;
        DEBUG_PRINTLN("[clockFaceBuffer] free");
    }
}


// Lädt die Grafiken für die Zeiger eines Uhren-Widgets, entweder aus einer benutzerdefinierten Konfiguration oder aus Standardwerten.
  
void loadHandSprites() {
    String set = preferences.getString("handset", "");

    // DEBUG_PRINTLN("[HANDS] Active hand set: " + set);

    bool usedDefault = false;
    if (set != "" && set != "default") {
        struct HandConfig {
            String label;
            TFT_eSprite* sprite;
            const uint16_t* fallback;
        } hands[3] = {
            {"hour", &hourHandSprite, handHour},
            {"minute", &minuteHandSprite, handMinute},
            {"second", &secondHandSprite, handSecond}
        };

        for (auto& h : hands) {
            String path = "/hand_set" + set + "_" + h.label + ".bmp";
        //    DEBUG_PRINTLN("[HANDS] Looking for: " + path);

            if (LittleFS.exists(path)) {
                if (!loadHandBmp(h.sprite, path.c_str(), HAND_WIDTH, HAND_HEIGHT)) {
                    for (int y = 0; y < HAND_HEIGHT; y++) {

                        for (int x = 0; x < HAND_WIDTH; x++) {
                            uint16_t px = h.fallback[y * HAND_WIDTH + x];

                            rowBuffer[x] = setPixelBrightness(px);

                        }
                        h.sprite->pushImage(0, y, HAND_WIDTH, 1, rowBuffer);
                    }
                    usedDefault = true;
                 //   DEBUG_PRINTLN("[HANDS] Failed to load " + h.label + ", fallback used.");
                }
                else {
                  //  DEBUG_PRINTLN("[HANDS] Loaded " + h.label);
                }
                // DEBUG_PRINTLN("found");
            }
            else {
                h.sprite->pushImage(0, 0, HAND_WIDTH, HAND_HEIGHT, h.fallback);
                usedDefault = true;
                // DEBUG_PRINTLN("[HANDS] Missing " + h.label + ", using default.");
            }
        }

        if (!usedDefault) {
           // DEBUG_PRINTLN("[HANDS] Loaded handset: " + set);
        }
        else {
            // DEBUG_PRINTLN("[HANDS] Incomplete set, used default for missing hands.");
        }

    }
    else {
        for (int y = 0; y < HAND_HEIGHT; y++) {
            for (int x = 0; x < HAND_WIDTH; x++) {
                rowBuffer[x] = setPixelBrightness(handHour[y * HAND_WIDTH + x]);
            }
            hourHandSprite.pushImage(0, y, HAND_WIDTH, 1, rowBuffer);

            for (int x = 0; x < HAND_WIDTH; x++) {
                rowBuffer[x] = setPixelBrightness(handMinute[y * HAND_WIDTH + x]);
            }
            minuteHandSprite.pushImage(0, y, HAND_WIDTH, 1, rowBuffer);

            for (int x = 0; x < HAND_WIDTH; x++) {
                rowBuffer[x] = setPixelBrightness(handSecond[y * HAND_WIDTH + x]);
            }
            secondHandSprite.pushImage(0, y, HAND_WIDTH, 1, rowBuffer);
        }

     //   DEBUG_PRINTLN("[HANDS] No set selected, using defaults.");

    }
}
 
// Hilfsfunktion zum Laden von Zeiger-BMPs 
bool loadHandBmp(TFT_eSprite* sprite, const char* filename, int width, int height) {
    File bmp = LittleFS.open(filename, "r");
    if (!bmp) return false;

    uint8_t header[54];
    if (bmp.read(header, 54) != 54 || header[0] != 'B' || header[1] != 'M') {
        bmp.close();
        return false;
    }

    int32_t bmpWidth = *(int32_t*)&header[18];
    int32_t bmpHeight = *(int32_t*)&header[22];
    uint16_t bpp = *(uint16_t*)&header[28];
    uint32_t offset = *(uint32_t*)&header[10];

    if (bmpWidth != width || abs(bmpHeight) != height || bpp != 16) {
        bmp.close();
        return false;
    }

    bool flip = bmpHeight > 0;
    bmpHeight = abs(bmpHeight);

    bmp.seek(offset);
    int rowSize = ((width * 2 + 3) / 4) * 4;
    for (int y = 0; y < bmpHeight; y++) {
        int row = flip ? bmpHeight - 1 - y : y;
        //uint8_t rowBuffer[rowSize];
        if (bmp.read((uint8_t*)rowBuffer, rowSize) != rowSize) break;

        uint16_t* pixelData = (uint16_t*)rowBuffer;
        for (int x = 0; x < width; x++) {

            if (pixelData[x] == 0xFFFF) {
                pixelData[x] = TRANSPARENT_COLOR;
            }

            pixelData[x] = setPixelBrightness(pixelData[x]);

        }
        sprite->pushImage(0, row, width, 1, (uint16_t*)rowBuffer, TRANSPARENT_COLOR);
    }

    bmp.close();
    return true;
}



// Button prüfen und ggf. Anzeige oder Factory Reset auslösen
void checkButton() {
    bool resetStarted = false;
    if (digitalRead(BUTTON1) == HIGH) {

        uint8_t secs = 5;
        unsigned long pressStart = millis();

        clearTFT();

        // Einmalig Anzeige zeichnen
        showWlanCredentials(WiFi.SSID());

        // Blockierender Loop während Button gedrückt
        while (digitalRead(BUTTON1) == HIGH) {
            if (millis() - pressStart > 10000 && millis() - pressStart < 15000) {
                resetStarted = true;
                tft.fillScreen(TFT_RED);
                tft.setTextColor(TFT_WHITE, TFT_RED);
                tft.setTextSize(TFT_TEXT_SIZE);
                tft.setCursor(20, CLOCK_HEIGHT / 2);
                tft.printf("Factory Reset", secs);

                tft.setCursor(20, (CLOCK_HEIGHT / 2) + 20);
                tft.printf("in %d secs", secs);
                delay(1000);
                if (secs > 0) secs--;
            }

            if (millis() - pressStart > 15000) {
                // 15 Sekunden überschritten → Factory Reset
                tft.fillScreen(TFT_RED);
                tft.setTextColor(TFT_WHITE, TFT_RED);
                tft.setTextSize(TFT_TEXT_SIZE);
                tft.setCursor(20, CLOCK_HEIGHT / 2);
                tft.println("Factory Reset...");
                delay(1000);
                factoryReset();  
                return;    
            }
            delay(10);  
        }
        // Button wurde vor 10secs losgelassen → WLAN-Credentials für 10 Sekunden anzeigen
        if (!resetStarted) {            
            delay(3000);
        }
        
    }
}


// Hilfsfunktion: Winkel an die aktuelle Display-Rotation anpassen
float shortestAngleDiff(float from, float to) {
    float diff = fmodf(to - from + 360.0f, 360.0f); // Modulo 360, um Werte im Bereich [0, 360) zu halten
    if (diff > 180.0f) diff -= 360.0f;             // Kürzeste Richtung wählen
    return diff;
}

static float lastHourAngle = 0.0f;
static float lastMinuteAngle = 0.0f;

// updateClock Funktion
void updateClock() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        // Keine gültige Uhrzeit verfügbar, auf 0:00:00 setzen
        timeinfo.tm_hour = 0;
        timeinfo.tm_min = 0;
        timeinfo.tm_sec = 0;
    }

    int orientation = preferences.getUChar("tftRotation", 0);

    float secAngle = timeinfo.tm_sec * 6.0f;
    float minAngle = timeinfo.tm_min * 6.0f;
    float hourAngle = (timeinfo.tm_hour % 12) * 30.0f + (timeinfo.tm_min / 2.0f) + (timeinfo.tm_sec / 120.0f);

    static uint8_t stationTick = 0;
    static uint32_t stationLastMillis = 0;
    static bool stationWaiting = false;

    unsigned long currentMillis = millis();

    if (firstRun) {
        stationTick = timeinfo.tm_sec;
        stationLastMillis = millis();
        stationWaiting = false;
        firstRun = false;

        lastHourAngle = rotatedAngle(hourAngle, orientation);
        lastMinuteAngle = rotatedAngle(minAngle, orientation);

        hourHandSprite.pushRotated(&backgroundSprite, lastHourAngle, TRANSPARENT_COLOR);
        minuteHandSprite.pushRotated(&backgroundSprite, lastMinuteAngle, TRANSPARENT_COLOR);

        if (showSecondHand) {
            secondHandSprite.pushRotated(&backgroundSprite, rotatedAngle(secAngle, orientation), TRANSPARENT_COLOR);
        }
        backgroundSprite.pushSprite(0, 0);
    }

    if (stationMode) {
        if (!stationWaiting && currentMillis - stationLastMillis >= fastSecond) {
            stationTick++;
            stationLastMillis += fastSecond;

            if (stationTick >= 60) {
                stationTick = 60;
                stationWaiting = true;
            }
        }
        else if (stationWaiting) {
            if (timeinfo.tm_sec == 0) {
                stationTick = 0;
                stationWaiting = false;
                stationLastMillis = currentMillis;

                // Sekundenzeiger korrekt synchronisieren
                secAngle = rotatedAngle(0, orientation);
            }
        }

        float subTick = (currentMillis - stationLastMillis) / fastSecond;
        if (subTick > 1.0f || stationWaiting) subTick = 0.0f;

        float smoothSec = (stationTick >= 60) ? 60.0f : stationTick + easeInOutSine(subTick);
        secAngle = rotatedAngle(smoothSec * 6.0f, orientation);

        minAngle = rotatedAngle(timeinfo.tm_min * 6.0f, orientation);   
    }

    if (!stationMode) {
        secAngle = rotatedAngle(secAngle, orientation);
          
        smoothMinute = preferences.getBool("smoothMinute", false);

        if (smoothMinute) {
            // Millisekunden einbeziehen
            unsigned long currentMillis = millis();
            int milliseconds = currentMillis % 1000;
            float smoothMinuteValue = timeinfo.tm_min + (timeinfo.tm_sec / 60.0f) + (milliseconds / 60000.0f);

            float rawMinAngle = smoothMinuteValue * 6.0f;
            float targetMinAngle = rotatedAngle(rawMinAngle, orientation);
            float angleDiff = shortestAngleDiff(lastMinuteAngle, targetMinAngle);

            // Sicherstellen, dass der Zeiger immer vorw&auml;rts l&auml;uft
            if (angleDiff < -180.0f) angleDiff += 360.0f;
            if (angleDiff > 180.0f) angleDiff -= 360.0f;

            lastMinuteAngle += angleDiff * 0.01f;  // noch feinere Bewegung

        }
        else {
            // Normale Minutenanzeige mit sanfter Korrektur bei Wechsel
            float rawMinAngle = timeinfo.tm_min * 6.0f;
            float targetMinAngle = rotatedAngle(rawMinAngle, orientation);
            float angleDiff = shortestAngleDiff(lastMinuteAngle, targetMinAngle);

            if (fabs(angleDiff) > 0.1f) {
                lastMinuteAngle += angleDiff * 0.1f;
                if (lastMinuteAngle < 0.0f) lastMinuteAngle += 360.0f;
                if (lastMinuteAngle >= 360.0f) lastMinuteAngle -= 360.0f;
            }
            else {
                lastMinuteAngle = targetMinAngle;
            }
        }

        minAngle = lastMinuteAngle;
    }
         
    

    float targetHourAngle = rotatedAngle(hourAngle, orientation);
    float hourAngleDiff = shortestAngleDiff(lastHourAngle, targetHourAngle);

    if (fabs(hourAngleDiff) > 0.05f) {
        lastHourAngle += hourAngleDiff * 0.1f;  // Gl&auml;ttungsfaktor
    }
    else {
        lastHourAngle = targetHourAngle;
    }
    hourAngle = lastHourAngle;


    loadClockFace();

    hourHandSprite.pushRotated(&backgroundSprite, hourAngle, TRANSPARENT_COLOR);
    minuteHandSprite.pushRotated(&backgroundSprite, minAngle, TRANSPARENT_COLOR);
    if (showSecondHand) {
        secondHandSprite.pushRotated(&backgroundSprite, secAngle, TRANSPARENT_COLOR);
    }
    
    // Nabe (hub)
    if (hub_size > 0 && hub_color > 0) {
        backgroundSprite.fillCircle(CLOCK_WIDTH / 2, CLOCK_HEIGHT / 2, hub_size, setPixelBrightness(hub_color));
        // DEBUG_PRINTLN("[HUB] Drawn hub with size " + String(hub_size));
        // DEBUG_PRINTLN("[HUB] Color: " + String(hub_color, HEX));
    }

    backgroundSprite.pushSprite(0, 0);
}
 
/// Helligkeit aktualisieren
void updateBrightness() {

    // Wenn Helligkeit geändert → neu zeichnen
    if (currentBrightness != lastAppliedBrightness) {
        loadClockFace();
        loadHandSprites();
        lastAppliedBrightness = currentBrightness;
    }

    // Prüfen, ob wir aktuell im konfigurierten Voll-Helligkeits-Zeitfenster sind
    bool withinDayWindow = false;
    
        struct tm timeinfo;
        if (getLocalTime(&timeinfo)) {
            int h = timeinfo.tm_hour;
            if (brightStartHour <= brightEndHour) {
                // normaler Bereich z.B. 8..20
                withinDayWindow = (h >= brightStartHour && h < brightEndHour);
            }
            else {
                // über Mitternacht z.B. 20..6
                withinDayWindow = (h >= brightStartHour || h < brightEndHour);
            }
        }
    

    // Wenn Zeitfenster aktiv und wir innerhalb davon sind: volle Helligkeit erzwingen
    if (withinDayWindow) {
        targetBrightness = maxBrightness;
#ifdef TFT_Backlight
        // sanfte Erhöhung, falls gewünscht (ähnlich wie ADC-Rampen)
        if (currentBrightness < targetBrightness) currentBrightness++;
        else if (currentBrightness > targetBrightness) currentBrightness--;
#else
        currentBrightness = targetBrightness;
#endif
    }
    else {
        // Normale Auto-Brightness oder statische Helligkeit
        if (use_adc) {

            int adcRaw = getAdjustedAdcValue(analogRead(ADC_PIN));
            // DEBUG_PRINTF("[ADC] Raw value: %d\n", adcRaw);

            if (initial) {
                for (int i = 0; i < ADC_SMOOTHING; i++) adcHistory[i] = adcRaw;
            }

            adcHistory[adcIndex] = adcRaw;
            adcIndex = (adcIndex + 1) % ADC_SMOOTHING;

            uint32_t avg = 0;
            for (int i = 0; i < ADC_SMOOTHING; i++) avg += adcHistory[i];
            avg /= ADC_SMOOTHING;

            currentAdcAvg = avg;  // speichern

            int lightPercent = map(avg, 0, 4095, 5, 100);

            if (lightPercent < lowThreshold) targetBrightness = minBrightness;
            else if (lightPercent > highThreshold) targetBrightness = maxBrightness;
#ifdef TFT_Backlight
            else {
                float norm = constrain((float)avg / 4095.0f, 0.0f, 1.0f);
                float gamma = gammaBrightness;
                float gammaNorm = powf(norm, gamma);
                targetBrightness = minBrightness + (uint8_t)((maxBrightness - minBrightness) * gammaNorm + 0.5f);
            }
#endif

            currentLightPercent = lightPercent;

            if (initial) currentBrightness = targetBrightness;

#ifdef TFT_Backlight
            if (currentBrightness != targetBrightness) {
                if (currentBrightness < targetBrightness) {
                    currentBrightness++;
                }
                else {
                    currentBrightness--;
                }
            }
#else
            currentBrightness = targetBrightness;
#endif

        }
        else {
            // kein ADC: Standardeinstellung
            currentBrightness = maxBrightness;
            targetBrightness = currentBrightness;
        }
    }

#ifdef TFT_Backlight
    ledcWrite(TFT_Backlight, currentBrightness);  // 0–255
#endif

}

// Passt den ADC-Wert an, wenn die Invertierung aktiviert ist
int getAdjustedAdcValue(int rawValue) {
    if (adcInverted) {
        return 4096 - rawValue; // Invertiere den Wert
    }
    return rawValue; // Standardwert
}

/// Easing-Funktion f%uuml;r sanfte Animationen
float easeInOutSine(float t) {
    // Intensität steuert die Kurve: 1.0 = Standard, >1.0 = steiler, <1.0 = flacher
    float intensity = 0.5f;
    return -(cos(PI * pow(t, intensity)) - 1.0f) / 2.0f;
}

/// Kodiert ein 16-Bit RGB565 Bild in das BMP-Format und gibt es als Base64-kodierten String zurück.
String encodeBmpToBase64(const uint16_t* data, int width, int height) {
    const int headerSize = 54;
    const int rowSize = ((width * 2 + 3) / 4) * 4;
    const int dataSize = rowSize * height;
    const int fileSize = headerSize + dataSize;

    uint8_t* bmpData = new uint8_t[fileSize];
    if (!bmpData) return "";

    memset(bmpData, 0, fileSize);

    // BMP Header
    bmpData[0] = 'B'; bmpData[1] = 'M';
    *(uint32_t*)&bmpData[2] = fileSize;
    *(uint32_t*)&bmpData[10] = headerSize;
    *(uint32_t*)&bmpData[14] = 40;
    *(int32_t*)&bmpData[18] = width;
    *(int32_t*)&bmpData[22] = -height; // Top-down BMP
    *(uint16_t*)&bmpData[26] = 1;
    *(uint16_t*)&bmpData[28] = 16;
    *(uint32_t*)&bmpData[34] = dataSize;

    // Pixel-Daten (RGB565 → BMP raw)
    for (int y = 0; y < height; y++) {
        uint8_t* rowPtr = bmpData + headerSize + y * rowSize;
        for (int x = 0; x < width; x++) {
            uint16_t px = data[y * width + x];
            if (px == TRANSPARENT_COLOR) px = 0xFFFF;

            rowPtr[x * 2] = px & 0xFF;
            rowPtr[x * 2 + 1] = px >> 8;
        }
    }

    String result = base64::encode(bmpData, fileSize);
    result.replace("\n", "");

    delete[] bmpData;

    return result;
}


//
// NTP-Zeitsynchronisation um 02:00:05 und 03:00:05
//
void checkNightlyTimeSync() {
    static bool triggered2 = false;
    static bool triggered3 = false;

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        DEBUG_PRINTLN("[TIME SYNC] Failed to get local time.");
        return;
    }

    //  DEBUG_PRINTF("[TIME SYNC] Current time: %02d:%02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

    if (timeinfo.tm_hour == 2 && timeinfo.tm_min == 0 && timeinfo.tm_sec == 5 && !triggered2) {
        DEBUG_PRINTLN("[TIME SYNC] Triggered at 02:00:05");
        checkNtpOffset();
        lastCheck = millis();
        triggered2 = true;        
    }

    if (timeinfo.tm_hour == 3 && timeinfo.tm_min == 0 && timeinfo.tm_sec == 5 && !triggered3) {
        DEBUG_PRINTLN("[TIME SYNC] Triggered at 03:00:05"); 
        checkNtpOffset();
        lastCheck = millis();
        triggered3 = true;
    }

    if (timeinfo.tm_hour > 3 && (triggered2 || triggered3)) {       
        triggered2 = false;
        triggered3 = false;
    }
}


// überpüft die WiFi-Verbindung und versucht, sie alle 5 Minuten wiederherzustellen, wenn sie getrennt ist.
void checkWiFiReconnect() {
    static unsigned long lastAttempt = 0;
    if (WiFi.status() == WL_CONNECTED) return;

    unsigned long now = millis();
    if (now - lastAttempt < 300000) return;
    lastAttempt = now;

    DEBUG_PRINTLN("[WiFi] Disconnected. Attempting reconnect...");
    WiFi.disconnect();
    connectWiFi(preferences.getInt("lastWlan",0), false);
    
}



// clear TFT display
void clearTFT() {
    tft.fillRect(0, 0, CLOCK_WIDTH, CLOCK_HEIGHT, TFT_BLACK);
}



// Startet einen WLAN-Access-Point mit dem Namen und Passwort 'clock123' und zeigt Verbindungsinformationen auf dem Display an.

void startAP() {
#ifdef TFT_Backlight
    ledcWrite(TFT_Backlight, 255);
#endif

    // 1. WLAN-Scan durchführen
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setTextSize(TFT_TEXT_SIZE);
    tft.setCursor(10, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 8));
    tft.println("WLAN-Scan...");
    int n = WiFi.scanNetworks();
    delay(100); // Kurze Pause für Anzeige

    // foundNetworks füllen
    foundNetworkCount = 0;
    for (int i = 0; i < n && i < MAX_NETWORKS; i++) {
        foundNetworks[i].ssid = WiFi.SSID(i);
        foundNetworks[i].rssi = WiFi.RSSI(i);
        foundNetworks[i].enc = WiFi.encryptionType(i);
        foundNetworkCount++;
    }


    WiFi.softAP("clock123", "clock123");
    DEBUG_PRINTLN("[WiFi] Started Access Point: clock123");

    // Captive portal: leite alle DNS-Anfragen auf die AP-IP um
    dnsServer.start(53, "*", WiFi.softAPIP());

    clearTFT();

    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setTextSize(TFT_TEXT_SIZE);
    tft.setCursor(10, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 8)) ;
    tft.println("AccessPoint active");
    tft.setCursor(10, (CLOCK_HEIGHT / 2));
    tft.println("clock123 clock123");
    tft.setCursor(10, (CLOCK_HEIGHT / 2 ) + (CLOCK_HEIGHT / 8));

    tft.print("http://");
    tft.println(WiFi.softAPIP());

    softAPIP = true;
    softAPIPstart = millis();
}


// connect wifi
bool connectWiFi(int number, bool verbose_mode) {
#ifdef TFT_Backlight
    if (verbose_mode) {
        ledcWrite(TFT_Backlight, 255);
    }
#endif
    if (wifi_ssid[number] == "") return false;

             
    DEBUG_PRINTLN("[WiFi] Trying SSID" + String(number+1) + ": " + wifi_ssid[number]);

    if (verbose_mode) {
        clearTFT();
        tft.setTextColor(TFT_GREEN, TFT_BLACK);

        tft.setTextSize(TFT_TEXT_SIZE / 2);
        tft.setCursor(60, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 3));
        tft.println(String(version));

        tft.setTextSize(TFT_TEXT_SIZE);
        tft.setCursor(20, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 8));
        tft.println("Connect to SSID" + String(number+1));
        tft.setCursor(20, (CLOCK_HEIGHT / 2));

        if (wifi_ssid[number].length() > 15) {
            tft.print(wifi_ssid[number].substring(0,15));
            tft.println("...");
        } else tft.println(wifi_ssid[number]);
    }

    WiFi.disconnect();
    WiFi.mode(WIFI_MODE_NULL);
    
    WiFi.mode(WIFI_STA);
    // MAC-Adresse holen    
    WiFi.macAddress(mac);

    snprintf(hostname, sizeof(hostname), "clock_%02X%02X%02X",
        mac[3], mac[4], mac[5]);
    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
    WiFi.setHostname(hostname);
    DEBUG_PRINTLN("[WiFi] Hostname set to: " + String(hostname));

    
    WiFi.begin(wifi_ssid[number].c_str(), wifi_pass[number].c_str());
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 30000) {
         
        Serial.print(".");
        if (verbose_mode) {
            animateCursor(tft, 20, (CLOCK_HEIGHT / 2) + (CLOCK_HEIGHT / 8), 100);
        }
        else {
            delay(100);
        }

    }
    Serial.println();
    if (WiFi.status() == WL_CONNECTED) {

        // mDNS initialisieren
        if (!MDNS.begin(hostname)) {
            DEBUG_PRINTLN("Error starting mDNS");
        }
        else {
            MDNS.addService("http", "tcp", 80); // Beispiel: HTTP-Dienst auf Port 80
        }

        DEBUG_PRINTLN("[WiFi] Connected to: " + wifi_ssid[number]);
        DEBUG_PRINTLN("[WiFi] IP address: " + WiFi.localIP().toString());
        String fullHostname = String(hostname) + ".local";
        pingHostname = true;
     //   pingHostname = Ping.ping(fullHostname.c_str(),3);
     //   DEBUG_PRINTF("[mDNS] Ping to %s: %s\n", fullHostname.c_str(), pingHostname ? "success" : "failed");
        
        if (verbose_mode) {
            showWlanCredentials(wifi_ssid[number]);           
        }

        if (preferences.getInt("lastWLan", -1) != number) {
            preferences.putInt("lastWLan", number);        
            DEBUG_PRINTLN("[WiFi] set lastWLan: " + (String)number+1);
        }

        delay(100);
        if (!WiFi.softAPgetStationNum()) updateClock();
        return true;
    }

    return false;

    
}

// Animation w&auml;hrend WLAN-Verbindung
void animateCursor(TFT_eSPI& tft, int x, int y, int delayMs) {
    tft.setCursor(x, y);    tft.print("/");    delay(delayMs);
    tft.setCursor(x, y);    tft.print("-");    delay(delayMs);
    tft.setCursor(x, y);    tft.print("\\");   delay(delayMs);
    tft.setCursor(x, y);    tft.print("-");    delay(delayMs);
}


// Anzeige WLAN Parameter
void showWlanCredentials(String wlan) {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);

    tft.setTextSize(TFT_TEXT_SIZE/2);
    tft.setCursor(60, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 3));
    tft.println(String(version));

    tft.setTextSize(TFT_TEXT_SIZE);
    tft.setCursor(20, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 4));
    tft.println("Connected to SSID");
    tft.setCursor(20, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 8));
    if (wlan.length() > 15) {
        tft.print(wlan.substring(0, 15));
        tft.println("...");
    }
    else tft.println(wlan);
    tft.setCursor(20, (CLOCK_HEIGHT / 2));
    tft.println(WiFi.localIP());

    if (pingHostname) {
        tft.setCursor(20, (CLOCK_HEIGHT / 2) + (CLOCK_HEIGHT / 8));
        tft.println(String(hostname) + ".local");
    }
}

/// <summary>
/// Initialisiert die Zeitsynchronisierung %uuml;ber NTP und stellt die Zeitzone ein. Bei Fehlern werden bis zu 10 Versuche unternommen, um die Zeit zu erhalten. Im Fehlerfall wird die zuletzt bekannte Zeit verwendet.
/// </summary>
boolean setupNTP() {
    if (WiFi.getMode() != WIFI_STA || !WiFi.isConnected()) {
        DEBUG_PRINTLN("[NTP] Skipping NTP setup: Not in STA mode or WiFi not connected.");
        return true;
    }
    timezone = preferences.getString("timezone", TIMEZONE_DEFAULT);
    for (int i = 0; i < numServers; i++) {
       
        DEBUG_PRINTLN("[NTP] Trying server: " + String(ntpServers[i]));
        configTzTime(timezone.c_str(), ntpServers[i]);
        struct tm timeinfo;
        if (getLocalTime(&timeinfo, 5000)) {
            DEBUG_PRINTLN("[NTP] Time synchronized successfully.");
            return true;
        }
        DEBUG_PRINTLN("[NTP] Failed to synchronize with server: " + String(ntpServers[i]));
    }
    handleNTPFailure();
}

void checkNtpOffset() {
    struct tm localTime;
    if (!getLocalTime(&localTime)) {
        DEBUG_PRINTLN("[NTP] Failed to get local time.");
        return;
    }

    // Hole die aktuelle Zeit vom NTP-Server
    time_t beforeSync = mktime(&localTime); // Lokale Zeit vor der Synchronisation
    setupNTP(); // Synchronisiere die Zeit mit dem NTP-Server
    time_t afterSync = time(nullptr); // Zeit nach der Synchronisation

    // Berechne die Abweichung
    long offset = difftime(afterSync, beforeSync);

    // Ausgabe der Abweichung
    if (offset == 0) {
        DEBUG_PRINTLN("[NTP] No deviation from local time.");
    }
    else {
        DEBUG_PRINTF("[NTP] Difference from local time: %ld sec\n", offset);
    }
}

void handleNTPFailure() {
    DEBUG_PRINTLN("[NTP] Handling NTP synchronization failure...");

    // Versuche, die letzte bekannte Zeit zu verwenden
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        char timeStr[32];
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
        DEBUG_PRINTLN("[NTP] Using last known time: " + String(timeStr));
    }
    else {
        // Wenn keine gültige Zeit verfügbar ist, auf 12:00 Uhr setzen
        DEBUG_PRINTLN("[NTP] No valid time available. Setting time to 12:00.");
        timeinfo.tm_hour = 12;
        timeinfo.tm_min = 0;
        timeinfo.tm_sec = 0;
        timeinfo.tm_year = 126; // Jahr 2026 (1900 + 126)
        timeinfo.tm_mon = 0;    // Januar
        timeinfo.tm_mday = 1;   // 1. Tag des Monats
        setTimeStruct(timeinfo); // Funktion, um die Zeit zu setzen
    }
    // Optionally, schedule a retry later
    scheduleNTPRetry();
}

void setTimeStruct(const struct tm& timeinfo) {
    timeval tv = { mktime(const_cast<struct tm*>(&timeinfo)), 0 }; // Konvertiere `tm` in `time_t`
    settimeofday(&tv, nullptr); // Setze die Systemzeit
    DEBUG_PRINTLN("[NTP] Time manually set to: " + String(asctime(&timeinfo)));

}
void scheduleNTPRetry() {
    lastNTPRetry = millis();
    DEBUG_PRINTLN("[NTP] Scheduled retry in 30 minutes.");
}

void checkNTPRetry() {
    if (WiFi.status() != WL_CONNECTED) {
        //DEBUG_PRINTLN("[NTP] Skipping retry: Not connected to WiFi.");
        lastNTPRetry = millis();
        return;
    }
    if (lastNTPRetry > 0 && millis() - lastNTPRetry >= retryIntervalNTPms) {
        DEBUG_PRINTLN("[NTP] Retrying NTP synchronization...");
        setupNTP();
        lastNTPRetry = millis();
    }
}


// Generiert den HTML-Header für die Weboberfläche
String generateHtmlHeader() {
    String html = "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
    html += "<style>body{font-family:Arial;text-align:center;}input,select,button{margin:10px;padding:10px;width:80%;}";
    html += "h1 { color: #333333; } ";
    html += "hr { border: 0; height: 1px; background-color: #cccccc; margin: 20px 0; } ";
    html += "table { margin: auto; border-collapse: collapse; } "; // Tabellen zentrieren
    html += "th, td { padding: 10px; text-align: center; border: 1px solid #cccccc; } "; // Tabellenzellen 
    html += "li { text-align: left; } "; // <li> linksb%uuml;ndig formatieren
    html += "</style></head><body>";
    // Seite benötigt JavaScript
    html += "<noscript><div style='color:red;font-weight:bold;margin:20px;'>JavaScript is disabled. This page requires JavaScript to work properly!</div></noscript>";
    return html;
}


/// Generiert den HTML-Statusabschnitt für die Weboberfläche
String generateHtmlStatus() {
    size_t total = LittleFS.totalBytes();
    size_t used = LittleFS.usedBytes();
    String html;
    if (WiFi.getMode() == WIFI_STA) {
        html = translate("Connected to") + ": <strong>" + WiFi.SSID() + "</strong>";
        html += "<br>" + translate("IP Address") + ": <strong>" + "<a href='http://" + WiFi.localIP().toString() + "'>http://" + WiFi.localIP().toString() +"</a></strong> ";
        if (pingHostname)  html += "<br>" + translate("Hostname") + ": <strong>" + "<a href='http://" + hostname + ".local'>http://" + hostname + ".local</a>" + "</strong>";
    }
    else html = "<br>Access Point: <strong>" + String(WiFi.softAPSSID()) + "</strong> (" + WiFi.softAPIP().toString() + ")";

    html += "<br>" + translate("Storage used") + ": " + String(used / 1024) + " KB / " + String(total / 1024) + " KB";
    html += " (" + translate("Free") + ": " + String((total - used) / 1024) + " KB)<hr>";
    return html;
}


// Navigationsleiste generieren

String generateLanguageSelector() {
    String html = "<form method='POST' action='/setLanguage'>";
    html += "<label for='lang'>Language/Sprache:</label>";
    html += "<select name='lang'>";
    html += "<option value='en'" + String(currentLanguage == "en" ? " selected" : "") + ">English</option>";
    html += "<option value='de'" + String(currentLanguage == "de" ? " selected" : "") + ">Deutsch</option>";
    html += "</select>";
    html += "<button type='submit'>" + translate("save") + "</button>";
    html += "</form><hr>";
    return html;
}

String generateNavigation() {
    if (WiFi.getMode() != WIFI_STA) {
        DEBUG_PRINTLN("[HTML] Skipping HTML navigation.");
        return "";
    }

    String nav = "<style>";
    nav += "a { text-decoration: underline; color: blue; font-weight: bold; }";
    nav += "a:hover { text-decoration: underline; }";
    nav += "</style>";
    nav += "<div style='text-align:center; margin-bottom:20px;'>";

    const struct NavItem {
        String path;
        String label;
        String confirmMessage; // Optional: Bestätigungsnachricht
    } navItems[] = {
        {"/", translate("Main"), ""},
        {"/presets", translate("Presets"), ""},
        {"/listfilesFaces", translate("Clock&nbsp;Face"), ""},
        {"/handsets", translate("Hand&nbsp;Set"), ""},        
        {"/timezone_form", translate("NTP&nbsp;Timezone"), ""},
        {"/brightness", translate("Brightness"), ""},
        {"/status", "Status", ""},
        {"/files", translate("<br>File&nbsp;Manager"), ""},
        {"/reboot", translate("Reboot"), translate("Are you sure you want to reboot?")},
        {"/factoryReset", translate("Factory&nbsp;Reset"), translate("Are you sure you want to reset to factory settings?")}
    };

    String currentPath = webserver.uri(); // Aktueller Pfad der Seite

    for (const auto& item : navItems) {
        if (item.path == currentPath) {
            // Wenn der aktuelle Pfad mit dem Navigationseintrag übereinstimmt, nur Text anzeigen
            nav += "<span style=\"margin-right:15px; font-weight:bold;\">" + item.label + "</span> ";
        }
        else {
            // Andernfalls als Link anzeigen
            nav += "<a href=\"" + item.path + "\" style=\"margin-right:15px;\"";
            if (!item.confirmMessage.isEmpty()) {
                nav += " onclick=\"return confirm('" + item.confirmMessage + "')\"";
            }
            nav += ">" + item.label + "</a> ";
        }
    }

    nav += "</div>";
    return nav;
}

/// <summary> 
/// Sets up the web server routes for clock face and hand set changes.
/// </summary>
void setupWebServer() {

    // API to set clock face, hand set, timezone, hub size/color, station mode, rotation, second hand visibility, and smooth minute hand

    // DB
    // http://192.168.0.214/api/setMode?face=face_db_uhr.bmp&handSet=0&hubSize=6&hubColor=ff0000showSecondHand=1&stationMode=true&smoothMinute=false&rotation=2

    // Irish Pub
    // http://192.168.0.214/api/setMode?face=face_irish_pub.bmp&handSet=0&hubSize=2&hubColor=aaaaaa&showSecondHand=false&stationMode=false&smoothMinute=true&rotation=2



    // API zum Zur%uuml;cksetzen der WiFi-Einstellungen
    // http://192.168.0.214/api/resetWiFi  

    webserver.on("/setLanguage", HTTP_POST, []() {
        if (webserver.hasArg("lang")) {
            String lang = webserver.arg("lang");
            if (lang == "en") {
                saveLanguage(lang);
                // webserver.send(200, "text/plain", "Language updated to " + lang);
                webserver.sendHeader("Location", "/", true);
                webserver.send(302, "text/plain", "");
                return;
            }

            if (translations.count(lang)) {
                saveLanguage(lang);
                // webserver.send(200, "text/plain", "Language updated to " + lang);
                webserver.sendHeader("Location", "/", true);
                webserver.send(302, "text/plain", "");
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

        for (int i = 0; i < MAX_WLAN; i++) {
            // Dynamisch berechnete Schlüssel
            String ssidKey = "ssid" + String(i + 1);
            String passKey = "pass" + String(i + 1);

            preferences.putString(ssidKey.c_str(), "");
            preferences.putString(passKey.c_str(), "");             
        }


        eraseWiFiConfig();

        // Sende eine Best&auml;tigung zur%uuml;ck
        webserver.send(200, "application/json", "{\"status\":\"WiFi settings reset successfully\"}");
        DEBUG_PRINTLN("[API] WiFi settings reset via /api/resetWiFi");

        delay(1000);
        // Neustart des ESP
        esp_reboot();

        });

    webserver.on("/api/createPreset", HTTP_POST, []() {
        createPresetFromPreferences(); // Ruft die Funktion auf, um ein neues Preset zu erstellen

        // Weiterleitung zur Presets-Seite
        webserver.sendHeader("Location", "/presets", true);
        webserver.send(302, "text/plain", "Redirecting to /presets...");
        });

    // API zum Setzen von Uhrmodus und anderen Einstellungen
    webserver.on("/api/setMode", HTTP_GET, []() {

        if (webserver.hasArg("face")) {
            String face = webserver.arg("face");
            face.replace("..", "");
            if (!face.startsWith("/")) face = "/" + face;
            if (face == "/face_default.bmp" || LittleFS.exists(face)) {
                preferences.putString("background", face);
                selectedBackground = face;
            }
        }

        if (webserver.hasArg("handSet")) {
            String handSet = webserver.arg("handSet");
            preferences.putString("handset", handSet);
        }

        if (webserver.hasArg("timeZone")) {
            String tz = webserver.arg("timeZone");
            preferences.putString("timezone", tz);
            timezone = tz;
            setupNTP();
        }
        if (webserver.hasArg("ntpServer1")) {
            strncpy(ntpServers[0], webserver.arg("ntpServer1").c_str(), sizeof(ntpServers[0]) - 1);
            ntpServers[0][sizeof(ntpServers[0]) - 1] = '\0'; // Null-terminieren
            preferences.putString("ntpServer1", ntpServers[0]);
        }
        if (webserver.hasArg("ntpServer2")) {
            strncpy(ntpServers[1], webserver.arg("ntpServer2").c_str(), sizeof(ntpServers[1]) - 1);
            ntpServers[1][sizeof(ntpServers[1]) - 1] = '\0'; // Null-terminieren
            preferences.putString("ntpServer1", ntpServers[1]);
        }


        if (webserver.hasArg("hubSize")) {
            hub_size = webserver.arg("hubSize").toInt();
            preferences.putUInt("centerSize", hub_size);
        }
        if (webserver.hasArg("hubColor")) {
            uint32_t rgb = strtoul(webserver.arg("hubColor").c_str(), NULL, 16); // 24-Bit RGB
            DEBUG_PRINTLN("[API] Received hubColor: " + webserver.arg("hubColor") + " -> " + String(rgb, HEX));
            uint8_t r = (rgb >> 16) & 0xFF; // Rot extrahieren
            uint8_t g = (rgb >> 8) & 0xFF;  // Grün extrahieren
            uint8_t b = rgb & 0xFF;         // Blau extrahieren

            // Konvertiere RGB888 zu RGB565
            hub_color = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
            preferences.putLong("centerColor", rgb);
        }


        if (webserver.hasArg("stationMode")) {
            String arg = webserver.arg("stationMode");
            stationMode = (arg == "1" || arg.equalsIgnoreCase("true")); // Konvertiere zu bool            
            preferences.putBool("stationMode", stationMode);
        }

        if (webserver.hasArg("rotation")) {
            String rotationArg = webserver.arg("rotation");
            uint8_t tftRotation = 0;

            // Prüfe, ob der Wert in Grad angegeben ist
            if (rotationArg == "0" || rotationArg == "90" || rotationArg == "180" || rotationArg == "270") {
                if (rotationArg == "0") tftRotation = 0;
                else if (rotationArg == "90") tftRotation = 1;
                else if (rotationArg == "180") tftRotation = 2;
                else if (rotationArg == "270") tftRotation = 3;
            }
            // Prüfe, ob der Wert als Index (0-3) angegeben ist
            else {
                tftRotation = rotationArg.toInt();
            }

            // Validierung des Wertes
            if (tftRotation >= 0 && tftRotation <= 3) {
                preferences.putUChar("tftRotation", tftRotation);
                firstRun = true;
                if (!psramAvailable) {
                    tft.setRotation(tftRotation); // sofort anwenden
                }
            }
        }

        if (webserver.hasArg("showSecondHand")) {
            String arg = webserver.arg("showSecondHand");
            showSecondHand = (arg == "1" || arg.equalsIgnoreCase("true")); // Konvertiere zu bool
            preferences.putBool("showSecondHand", showSecondHand);
        }

        if (webserver.hasArg("smoothMinute")) {
            String arg = webserver.arg("smoothMinute");
            smoothMinute = (arg == "1" || arg.equalsIgnoreCase("true")); // Konvertiere zu bool 
            preferences.putBool("smoothMinute", smoothMinute);
        }

        freeClockFaceBuffer();
        loadClockFace();
        loadHandSprites();
        updateClock();

        if (webserver.hasArg("source")) {
            String arg = webserver.arg("source");
            if (arg == "preset") {
                DEBUG_PRINTLN("[API] Request source: preset");
                webserver.sendHeader("Location", "/presets", true);
                webserver.send(302, "text/plain", "Redirecting to /presets...");
                return;
            }
        }

        webserver.send(200, "text/plain", "ok");
        });

    // Preset-Verwaltung
    webserver.on("/presets", HTTP_GET, []() {
        String html = generateHtmlHeader();
        html += generateHtmlStatus();
        html += generateNavigation();
        html += "<h2>" + translate("Manage Presets") + "</h2>";

        // Links oben anzeigen
        html += "<div style='text-align:center;'>";

        if (pingHostname) {
            if (currentLanguage == "de") html += "<p>Benutze den Hostnamen <strong>" + String(hostname) + ".local</strong> anstelle der IP Adresse f&uuml;r bessere Erreichbarkeit..</p>";
            else html += "<p>Use the host name <strong>" + String(hostname) + ".local</strong> instead of the IP address for better reliability.</p>";
        }

        html += "<ul style='list-style-type:none; padding:0; display:inline-block; text-align:left;'>";

        String espIP = "http://" + WiFi.localIP().toString(); // Aktuelle IP-Adresse des ESP
        String espHost = "http://" + String(hostname) + ".local"; // Aktueller Hostname des ESP

        html += "<table style='width:100%; border-collapse:collapse;'>";
        html += "<tr><th style='border:1px solid #ccc; padding:8px;'>" + translate("Preset Name") +
            "</th><th style = 'border:1px solid #ccc; padding:8px;'>Link</th></tr>";

        for (int i = 0; i < MAX_PRESETS; i++) {
            if (!presets[i].name.isEmpty() && !presets[i].url.isEmpty()) {
                String displayUrl = presets[i].url;

                // Ersetze die gespeicherte IP durch die aktuelle IP des ESP
                if (displayUrl.startsWith("http://")) {
                    int ipEnd = displayUrl.indexOf('/', 7); // Suche nach dem Ende der IP-Adresse
                    if (ipEnd != -1) {
                        displayUrl = espIP + displayUrl.substring(ipEnd); // Ersetze die IP
                    }
                    else {
                        displayUrl = espIP; // Nur die IP ohne Pfad
                    }
                }
                displayUrl += "&source=preset";
                presets[i].name.replace(" ", "_"); // Ersetze Leerzeichen durch Unterstriche

                html += "<tr>";
                html += "<td style='border:1px solid #ccc; padding:8px; text-align: left;'><a href='" + displayUrl + "'>" + presets[i].name + "</a></td>";
                String presetName = presets[i].name;
                presetName.replace(" ", "_"); // Ersetze Leerzeichen durch Unterstriche
                html += "<td style='border:1px solid #ccc; padding:8px; text-align: left;'>" + espIP + "/api/setPreset?name=" + presetName;
                if (pingHostname) {
                    html += "<br>" + espHost + "/api/setPreset?name=" + presetName;
                }
                html += "</td></tr>";
            }
        }
        html += "</table>";
        html += "</ul>";
        html += "</div><hr>";

        html += "<hr>";
        html += "<h3>" + translate("Create New Preset") + "</h3>";
        html += "<form method='POST' action='/api/createPreset'>";
        html += "<button type='submit'>" + translate("Create Preset from Current Settings") + "</button>";
        html += "</form>";
        html += "<hr>";

        // Gefüllte Presets anzeigen
        html += "<h3>" + translate("Edit Presets") + "</h3>";
        html += "<form method='POST' action='/save_presets'>";
        for (int i = 0; i < MAX_PRESETS; i++) {

            String displayUrl = presets[i].url;

            // Ersetze die gespeicherte IP durch die aktuelle IP des ESP
            if (displayUrl.startsWith("http://")) {
                int ipEnd = displayUrl.indexOf('/', 7); // Suche nach dem Ende der IP-Adresse
                if (ipEnd != -1) {
                    displayUrl = espIP + displayUrl.substring(ipEnd); // Ersetze die IP
                    DEBUG_PRINTLN("[HTML] 1 Replaced preset URL for display: " + displayUrl);
                }
                else {
                    displayUrl = espIP; // Nur die IP ohne Pfad
                    DEBUG_PRINTLN("[HTML] 2 Replaced preset URL for display: " + displayUrl);
                }
            }
            presets[i].name.replace(" ", "_"); // Ersetze Leerzeichen durch Unterstriche
            html += "<h4>" + translate("Preset") + " " + String(i + 1) + "</h4>";
            html += "Name: <input type='text' name='name" + String(i) + "' value='" + presets[i].name + "'><br>";
            html += "URL: <input type='text' name='url" + String(i) + "' value='" + displayUrl + "'><br><br>";

            if (presets[i].name.isEmpty() && presets[i].url.isEmpty()) {
                break;
            }
        }


        html += "<button type='submit'>" + translate("Save Presets") + "</button>";
        html += "</form><hr>";

        html += "</body></html>";
        webserver.send(200, "text/html", html);
        });

    // Presets speichern    
    webserver.on("/save_presets", HTTP_POST, []() {
        for (int i = 0; i < MAX_PRESETS; i++) {
            String nameArg = "name" + String(i);
            String urlArg = "url" + String(i);
            presets[i].name.replace(" ", "_"); // Ersetze Leerzeichen durch Unterstriche
            presets[i].name = webserver.hasArg(nameArg) ? webserver.arg(nameArg) : "";
            presets[i].url = webserver.hasArg(urlArg) ? webserver.arg(urlArg) : "";
        }

        savePresets();

        webserver.send(200, "text/html", "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='0; url=/presets'><title>Saved</title></head><body><h2>Presets saved</h2><p>Redirecting...</p></body></html>");
        });

    // API zum Setzen eines Presets
    webserver.on("/api/setPreset", HTTP_GET, []() {
        if (!webserver.hasArg("name")) {
            webserver.send(400, "text/plain", "Missing 'name' parameter");
            return;
        }

        String presetName = webserver.arg("name");
        presetName.replace(" ", "_"); // Ersetze Leerzeichen durch Unterstriche

        // Suche das Preset mit dem angegebenen Namen
        for (int i = 0; i < MAX_PRESETS; i++) {
            if (presets[i].name.equalsIgnoreCase(presetName)) {
                if (!presets[i].url.isEmpty()) {
                    // Redirect zur URL des Presets
                    webserver.sendHeader("Location", presets[i].url, true);
                    webserver.send(302, "text/plain", "Redirecting to preset URL...");
                    DEBUG_PRINTLN("[setPreset] Redirecting to preset: " + presetName + " -> " + presets[i].url);
                    return;
                }
                else {
                    webserver.send(404, "text/plain", "Preset URL is empty");
                    return;
                }
            }
        }

        // Preset nicht gefunden
        webserver.send(404, "text/plain", "Preset not found");
        DEBUG_PRINTLN("[setPreset] Preset not found: " + presetName);
        });

    // NTP Server und Zeitzone setzen
    webserver.on("/set_timezone", HTTP_POST, []() {
        if (webserver.hasArg("ntpServer1")) {
            strncpy(ntpServers[0], webserver.arg("ntpServer1").c_str(), sizeof(ntpServers[0]) - 1);
            ntpServers[0][sizeof(ntpServers[0]) - 1] = '\0'; // Ensure null-termination
            preferences.putString("ntpServer1", ntpServers[0]);
            DEBUG_PRINTLN("[NTP] NTP Server1 set to: " + String(ntpServers[0]));
        }
        if (webserver.hasArg("ntpServer2")) {
            strncpy(ntpServers[1], webserver.arg("ntpServer2").c_str(), sizeof(ntpServers[1]) - 1);
            ntpServers[0][sizeof(ntpServers[1]) - 1] = '\0'; // Ensure null-termination
            preferences.putString("ntpServer2", ntpServers[1]);
            DEBUG_PRINTLN("[NTP] NTP Server2 set to: " + String(ntpServers[1]));
        }



        if (webserver.hasArg("timezone")) {
            String tz = webserver.arg("timezone");
            preferences.putString("timezone", tz);
            setupNTP();
        }

        webserver.send(200, "text/html",
            "<!DOCTYPE html><html><head>"
            "<meta http-equiv='refresh' content='0; url=/' />"
            "<title>NTP / Timezone Updated</title></head>"
            "<body><h2>NTP / Timezone updated</h2>"
            "</body></html>");

        setupNTP();
        });


    // NTP Server und Zeitzone Formular
    webserver.on("/timezone_form", HTTP_GET, []() {
        String timezone = preferences.getString("timezone", TIMEZONE_DEFAULT);

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

        String html = generateHtmlHeader();
        html += generateHtmlStatus(); // Statusleiste einfügen
        html += generateNavigation(); // Navigation einfügen
        html += "<h2>" + translate("NTP Server / Timezone (DST String)") + "</h2>";
        html += "<form method='POST' action='/set_timezone'>";

        html += "NTP Server1: <input type='text' name='ntpServer1' value='" + String(ntpServers[0]) + "'><br>";
        html += "NTP Server2: <input type='text' name='ntpServer2' value='" + String(ntpServers[1]) + "'><br><br>";

        // Kombiniertes Select + Input
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
        //  html += generateNavigation(); // Navigation einf%uuml;gen
        html += "<br><br>";
        html += "</form></body></html>";
        webserver.send(200, "text/html", html);
        });

    // Datei umbenennen Formular
    webserver.on("/rename_form", HTTP_GET, []() {
        if (!webserver.hasArg("file")) {
            webserver.send(400, "text/plain", "Missing file parameter.");
            return;
        }

        String oldName = webserver.arg("file");
        String html = generateHtmlHeader();

        html += generateHtmlStatus(); // Statusleiste einfügen
        html += generateNavigation(); // Navigation einfügen
        html += "<h2>" + translate("Rename File") + "</h2>";
        html += "<form action='/rename' method='POST'>";
        html += "<input type='hidden' name='old' value='" + oldName + "'>";
        html += "<label>" + translate("New Name") + ":</label><br>";
        html += "<input name='new' value='" + oldName + "' required><br><br>";
        html += "<button type='submit'>" + translate("Rename") + "</button></form>";
        html += "<br><a href='/files'>" + translate("Cancel") + "</a></body></html>";
        webserver.send(200, "text/html", html);
        });

    // Datei umbenennen Aktion
    webserver.on("/rename", HTTP_POST, []() {
        if (webserver.hasArg("old") && webserver.hasArg("new")) {
            String oldName = webserver.arg("old");
            String newName = webserver.arg("new");

            oldName.replace("..", ""); newName.replace("..", "");
            if (!oldName.startsWith("/")) oldName = "/" + oldName;
            if (!newName.startsWith("/")) newName = "/" + newName;

            if (LittleFS.exists(oldName)) {
                if (LittleFS.rename(oldName, newName)) {
                    webserver.sendHeader("Location", "/files", true);
                    webserver.send(302, "text/plain", "");
                }
                else {
                    webserver.send(500, "text/plain", "Rename failed.");
                }
            }
            else {
                webserver.send(404, "text/plain", "Original file not found.");
            }
        }
        else {
            webserver.send(400, "text/plain", "Missing parameters.");
        }
        });


    // BMP skalieren Formular
    webserver.on("/scalebmp_form", HTTP_GET, []() {
        if (!webserver.hasArg("file")) {
            webserver.send(400, "text/plain", "Missing file name.");
            return;
        }
        String src = webserver.arg("file");
        String html = generateHtmlHeader();
        html += generateHtmlStatus(); // Statusleiste einf%uuml;gen
        html += generateNavigation(); // Navigation einf%uuml;gen
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
    webserver.on("/scalebmp_run", HTTP_GET, []() {
        if (!webserver.hasArg("src") || !webserver.hasArg("dst") || !webserver.hasArg("w") || !webserver.hasArg("h")) {
            webserver.send(400, "text/plain", "Missing parameters.");
            return;
        }

        String src = webserver.arg("src");
        String dst = webserver.arg("dst");
        int w = webserver.arg("w").toInt();
        int h = webserver.arg("h").toInt();

        bool ok = scaleAndSaveBmp(src.c_str(), dst.c_str(), w, h);
        if (ok) {
            webserver.send(200, "text/html", "<html><body style='font-family:Arial;'><h3>" + translate("Scaling successful") + "!</h3><p>" + translate("Saved as") + ": " + dst + "</p><a href = '/files'>" + translate("Back") + " </a></body></html>");
        }
        else {
            webserver.send(500, "text/html", "<html><body style='font-family:Arial;'><h3>" + translate("Failed to scale BMP") + "</h3><p>Check source file and format.</p><a href='/files'>Back</a></body></html>");
        }
        });

    // Anzeigeeinstellungen speichern
    webserver.on("/applydisplaysettings", HTTP_POST, []() {
        // Save to Preferences

        stationMode = preferences.getBool("stationMode", false);
        showSecondHand = preferences.getBool("showSecondHand", true);
        smoothMinute = preferences.getBool("smoothMinute", false);

        stationMode = webserver.hasArg("stationMode");
        showSecondHand = webserver.hasArg("showSecondHand");
        smoothMinute = webserver.hasArg("smoothMinute");

        useTouch = webserver.hasArg("useTouch");
        preferences.putBool("useTouch", useTouch);

        if (useTouch) enableTouch();
        else disableTouch();

        // Logging-Einstellung speichern
        loggingEnabled = webserver.hasArg("loggingEnabled");
        preferences.putBool("loggingEnabled", loggingEnabled);

        wifiActive = webserver.hasArg("wifiActive");
        preferences.putBool("wifiActive", wifiActive);  

        preferences.putBool("stationMode", stationMode);
        preferences.putBool("showSecondHand", showSecondHand);
        preferences.putBool("smoothMinute", smoothMinute);

        if (webserver.hasArg("rotation")) {
            tftRotation = webserver.arg("rotation").toInt();
            if (tftRotation >= 0 && tftRotation <= 3) {
                preferences.putUChar("tftRotation", tftRotation);
                firstRun = true;
                if (!psramAvailable) {
                    tft.setRotation(tftRotation); // sofort anwenden
                }
            }

            freeClockFaceBuffer();
            loadClockFace();      // neu zeichnen mit neuer Ausrichtung
            loadHandSprites();
        }

        webserver.send(200, "text/html",
            "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='0; url=/'><title>Saved</title></head>"
            "<body style='font-family:Arial;text-align:center;'><h2>Saved</h2><p>Back...</p></body></html>");
        });


    // Helligkeitseinstellungen Formular
    webserver.on("/brightness", HTTP_POST, []() {
        String html = generateHtmlHeader();
        html += generateHtmlStatus(); // Statusleiste einfügen
        html += generateNavigation(); // Navigation einfügen
        html += "<h2>" + translate("Brightness Settings") + "</h2><form method = 'POST' action = '/save_brightness'>";

        if (photoresistorFound) {
            html += "<table style='margin:auto;text-align:left;'><tr>";
            html += "<td><label><input type='checkbox' name='use_adc' value='1' " + String(use_adc ? "checked" : "") + "> " + translate("Enable Auto Brightness") + "</label></td>";

            html += "<td><label><input type='checkbox' name='adcInverted' value='1' " + String(adcInverted ? "checked" : "") + "> " + translate("Invert ADC Reading") + "</label></td>";
            html += "</tr></table><hr><br>";

            html += "<label>" + translate("Low Threshold") + " (0 - 100 %) :</label><br><input name = 'lowThreshold' type = 'number' min = '0' max = '100' value = '" + String(lowThreshold) + "'><br>";
            html += "<label>" + translate("High Threshold") + " (0 - 100 %) :</label><br><input name = 'highThreshold' type = 'number' min = '0' max = '100' value = '" + String(highThreshold) + "'><br>";
        }

        html += "<label>" + translate("Min Brightness") + " (0 - 255) : </label><br><input name = 'minBrightness' type = 'number' min = '0' max = '255' value = '" + String(minBrightness) + "'><br>";

        html += "<label>" + translate("Max Brightness") + " (0 - 255) : </label><br><input name = 'maxBrightness' type = 'number' min = '0' max = '255' value = '" + String(maxBrightness) + "'><br>";


        html += "<label>" + translate("Full brightness from (hour, 0-23)") + ":</label><br><input name = 'brightStart' type = 'number' min = '0' max = '23' value = '" + String(brightStartHour) + "'><br>";
        html += "<label>" + translate("Full brightness until (hour, 0-23)") + ":</label><br><input name = 'brightEnd' type = 'number' min = '0' max = '23' value = '" + String(brightEndHour) + "'><br>";

#if defined (GC9D01)  || defined(GC9A01_WITH_BACKLIGHT) 
        html += "<label>" + translate("Gamma Correction") + " (0.1 - 3.0) : </label><br>";
        html += "<input type='number' name='gamma' step='0.1' min='0.1' max='3.0' value='" + String(gammaBrightness) + "' required><br>";

#endif


        html += "<button type='submit'>" + translate("Save") + "</button></form>";

        if (photoresistorFound) {
            html += "<br>";
            html += "<hr><strong>" + translate("Current ADC Value") + ":</strong> " + String(currentAdcAvg) + "<br>";
            html += "<strong>" + translate("Current Brightness") + ":</strong> " + String(currentBrightness) + " / 255<br>";
            html += "<strong>" + translate("Light (for Threshold)") + ":</strong> " + String(currentLightPercent) + " % <br>";

            html += "<br>";
            html += "<form method='GET' action='/brightness'><button type='submit'>" + translate("Refresh") + "</button></form>";
            html += "<br>"; html += "<br>";


#if defined (GC9D01)  || defined(GC9A01_WITH_BACKLIGHT) 
            html += "<script src='https://cdn.plot.ly/plotly-latest.min.js'></script>\n";

            html += "<h2>Gamma-Korrektur: adc -> targetBrightness</h2>\n";
            html += "<label for='gammaSlider'>Gamma: <span id='gammaValue'>" + String(gammaBrightness) + "</span></label>\n";
            html += "<input type='range' id='gammaSlider' min='0.1' max='3.0' step='0.1' value='" + String(gammaBrightness) + "' style='width:300px;'><br><br>\n";
            html += "<div id='plot' style='width:100%; height:600px;'></div>\n";

            html += "<script>\n";
            html += "const minBrightness = " + String(minBrightness) + ";\n";
            html += "const maxBrightness = " + String(maxBrightness) + ";\n";
            html += "const avg = Array.from({length: 500}, (_, i) => i * (4095 / 499));\n\n";

            html += "function computeBrightness(gamma) {\n";
            html += "  return avg.map(val => {\n";
            html += "    let norm = Math.min(Math.max(val / 4095.0, 0.0), 1.0);\n";
            html += "    let gammaNorm = Math.pow(norm, gamma);\n";
            html += "    return minBrightness + Math.round((maxBrightness - minBrightness) * gammaNorm);\n";
            html += "  });\n";
            html += "}\n\n";

            html += "function plotGamma(gamma) {\n";
            html += "  const y = computeBrightness(gamma);\n";
            html += "  Plotly.newPlot('plot', [{\n";
            html += "    x: avg,\n";
            html += "    y: y,\n";
            html += "    mode: 'lines',\n";
            html += "    name: `Gamma = ${gamma.toFixed(1)}`\n";
            html += "  }], {\n";
            html += "    title: 'Gamma-Korrektur-Kurve',\n";
            html += "    xaxis: { title: 'adc (0 - 4095)' },\n";
            html += "    yaxis: { title: 'targetBrightness (0 - 255)' }\n";
            html += "  });\n";
            html += "}\n\n";

            html += "const slider = document.getElementById('gammaSlider');\n";
            html += "const gammaValue = document.getElementById('gammaValue');\n";
            html += "slider.addEventListener('input', () => {\n";
            html += "  const gamma = parseFloat(slider.value);\n";
            html += "  gammaValue.textContent = gamma.toFixed(1);\n";
            html += "  plotGamma(gamma);\n";
            html += "});\n\n";

            html += "plotGamma(" + String(gammaBrightness) + ");\n";
            html += "</script>\n";
#endif
        }

        html += "<br><br>";
        html += generateNavigation(); // Navigation einfügen
        html += "</body></html>";
        webserver.send(200, "text/html", html);
        });

    // Helligkeitseinstellungen Formular
    webserver.on("/brightness", HTTP_GET, []() {
        String html = generateHtmlHeader();
        html += generateHtmlStatus(); // Statusleiste einfügen
        html += generateNavigation(); // Navigation einfügen
        html += "<h2>" + translate("Brightness Settings") + "</h2><form method = 'POST' action = '/save_brightness'>";

        if (photoresistorFound) {
            html += "<table style='margin:auto;text-align:left;'><tr>";
            html += "<td><label><input type='checkbox' name='use_adc' value='1' " + String(use_adc ? "checked" : "") + "> " + translate("Enable Auto Brightness") + "</label></td>";

            html += "<td><label><input type='checkbox' name='adcInverted' value='1' " + String(adcInverted ? "checked" : "") + "> " + translate("Invert ADC Reading") + "</label></td>";
            html += "</tr></table><hr><br>";

            html += "<label>" + translate("Low Threshold") + " (0 - 100) : </label><br><input name = 'lowThreshold' type = 'number' min = '0' max = '100' value = '" + String(lowThreshold) + "'><br>";
            html += "<label>" + translate("High Threshold") + " (0 - 100) : </label><br><input name = 'highThreshold' type = 'number' min = '0' max = '100' value = '" + String(highThreshold) + "'><br>";
        }

        html += "<label>" + translate("Min Brightness") + " (0 - 255) : </label><br><input name = 'minBrightness' type = 'number' min = '0' max = '255' value = '" + String(minBrightness) + "'><br>";

        html += "<label>" + translate("Max Brightness") + " (0 - 255) : </label><br><input name = 'maxBrightness' type = 'number' min = '0' max = '255' value = '" + String(maxBrightness) + "'><br>";


        html += "<label>" + translate("Full brightness from (hour, 0-23)") + ":</label><br><input name = 'brightStart' type = 'number' min = '0' max = '23' value = '" + String(brightStartHour) + "'><br>";
        html += "<label>" + translate("Full brightness until (hour, 0-23)") + ":</label><br><input name = 'brightEnd' type = 'number' min = '0' max = '23' value = '" + String(brightEndHour) + "'><br>";
#if defined (GC9D01)  || defined(GC9A01_WITH_BACKLIGHT) 
        html += "<label>" + translate("Gamma Correction") + " (0.1 - 3.0) : </label><br>";
        html += "<input type='number' name='gamma' step='0.1' min='0.1' max='3.0' value='" + String(gammaBrightness) + "' required><br>";
#endif


        html += "<button type='submit'>" + translate("Save") + "</button></form>";

        if (photoresistorFound) {
            html += "<br>";
            html += "<hr><strong>" + translate("Current ADC Value") + ":</strong> " + String(currentAdcAvg) + "<br>";
            html += "<strong>" + translate("Current Brightness") + ":</strong> " + String(currentBrightness) + " / 255<br>";
            html += "<strong>" + translate("Light (for Threshold)") + ":</strong> " + String(currentLightPercent) + " % <br>";
            html += "<br>";
            html += "<form method='GET' action='/brightness'><button type='submit'>" + translate("Refresh") + "</button></form>";
            html += "<br>";

#if defined (GC9D01)  || defined(GC9A01_WITH_BACKLIGHT) 
            html += "<script src='https://cdn.plot.ly/plotly-latest.min.js'></script>\n";

            html += "<h2>Gamma-Korrektur: adc -> targetBrightness</h2>\n";
            html += "<label for='gammaSlider'>Gamma: <span id='gammaValue'>" + String(gammaBrightness) + "</span></label>\n";
            html += "<input type='range' id='gammaSlider' min='0.1' max='3.0' step='0.1' value='" + String(gammaBrightness) + "' style='width:300px;'><br><br>\n";
            html += "<div id='plot' style='width:100%; height:600px;'></div>\n";

            html += "<script>\n";
            html += "const minBrightness = " + String(minBrightness) + ";\n";
            html += "const maxBrightness = " + String(maxBrightness) + ";\n";
            html += "const avg = Array.from({length: 500}, (_, i) => i * (4095 / 499));\n\n";

            html += "function computeBrightness(gamma) {\n";
            html += "  return avg.map(val => {\n";
            html += "    let norm = Math.min(Math.max(val / 4095.0, 0.0), 1.0);\n";
            html += "    let gammaNorm = Math.pow(norm, gamma);\n";
            html += "    return minBrightness + Math.round((maxBrightness - minBrightness) * gammaNorm);\n";
            html += "  });\n";
            html += "}\n\n";

            html += "function plotGamma(gamma) {\n";
            html += "  const y = computeBrightness(gamma);\n";
            html += "  Plotly.newPlot('plot', [{\n";
            html += "    x: avg,\n";
            html += "    y: y,\n";
            html += "    mode: 'lines',\n";
            html += "    name: `Gamma = ${gamma.toFixed(1)}`\n";
            html += "  }], {\n";
            html += "    title: 'Gamma-Korrektur-Kurve',\n";
            html += "    xaxis: { title: 'adc (0 - 4095)' },\n";
            html += "    yaxis: { title: 'targetBrightness (0 - 255)' }\n";
            html += "  });\n";
            html += "}\n\n";

            html += "const slider = document.getElementById('gammaSlider');\n";
            html += "const gammaValue = document.getElementById('gammaValue');\n";
            html += "slider.addEventListener('input', () => {\n";
            html += "  const gamma = parseFloat(slider.value);\n";
            html += "  gammaValue.textContent = gamma.toFixed(1);\n";
            html += "  plotGamma(gamma);\n";
            html += "});\n\n";

            html += "plotGamma(" + String(gammaBrightness) + ");\n";
            html += "</script>\n";
#endif
        }

        html += generateNavigation(); // Navigation einf%uuml;gen    
        html += "<br><br></body></html>";
        webserver.send(200, "text/html", html);
        });

    // Helligkeitseinstellungen speichern
    webserver.on("/save_brightness", HTTP_POST, []() {
        use_adc = webserver.hasArg("use_adc");
        adcInverted = webserver.hasArg("adcInverted");
        lowThreshold = webserver.arg("lowThreshold").toInt();
        highThreshold = webserver.arg("highThreshold").toInt();

        maxBrightness = (uint8_t)webserver.arg("maxBrightness").toInt();
        minBrightness = (uint8_t)webserver.arg("minBrightness").toInt();

        // neue: Zeitabhängige Helligkeit speichern


        brightStartHour = (uint8_t)constrain(webserver.arg("brightStart").toInt(), 0, 23);
        brightEndHour = (uint8_t)constrain(webserver.arg("brightEnd").toInt(), 0, 23);

#if defined (GC9D01)  || defined(GC9A01_WITH_BACKLIGHT) 
        gammaBrightness = webserver.arg("gamma").toFloat();
        preferences.putFloat("gammaBrightness", gammaBrightness);
#endif

        preferences.putBool("use_adc", use_adc);
        preferences.putBool("adcInverted", adcInverted);
        preferences.putInt("lowThreshold", lowThreshold);
        preferences.putInt("highThreshold", highThreshold);

        preferences.putUChar("maxBrightness", maxBrightness);
        preferences.putUChar("minBrightness", minBrightness);

        // persist time-based settings

        preferences.putUChar("brightStart", brightStartHour);
        preferences.putUChar("brightEnd", brightEndHour);


        webserver.send(200, "text/html",
            "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='0; url=/brightness'><title>Saved</title></head>"
            "<body style='font-family:Arial;text-align:center;'><h2>Settings saved</h2><p>Returning...</p></body></html>");
        });


    // Alle Dateien auflisten
    webserver.on("/files", HTTP_GET, []() {
        String html = generateHtmlHeader();

        html += generateHtmlStatus(); // Statusleiste einfügen
        html += generateNavigation(); // Navigation einfügen

        html += "<h2>" + translate("All Files on LittleFS") + "</h2><table border = '1'><tr><th align = left>" + translate("Filename") + "</th><th>" + translate("Size(bytes)") + "</th><th>Info</th><th>" + translate("Action") + "</th></tr>";

        File root = LittleFS.open("/");
        File file = root.openNextFile();
        while (file) {
            String info = getBmpInfo(file.name());
            String name = file.name();
            html += "<tr><td align=left>" + name + "</td><td align=right>" + String(file.size()) + "</td>";
            html += "<td align=right>" + String(info) + "</td>";
            html += " <td><a href = '/delete?file=" + name + "' onclick = 'return confirm(\"Delete " + name + "?\")'>" + translate("Delete") + "</a> ";
            // Scale-Option nur für .bmp-Dateien anzeigen
            if (name.endsWith(".bmp")) {
                html += "<a href = '/scalebmp_form?file=" + name + "'>" + translate("Scale") + "</a> ";
                html += "<a href='/rename_form?file=" + name + "'>" + translate("Rename") + "</a> ";
            }
            else {
                html += translate("Scale") + " ";
                html += translate("Rename") + " ";
            }

            html += "<a href='/file?name=" + name + "'>" + translate("View") + "</a> "; // "View"-Link für Logdateien
            html += "</td></tr>";

            file = root.openNextFile();
        }
        html += "</table><br><br>";
        html += generateNavigation(); // Navigation einfügen
        html += "</body></html>";
        webserver.send(200, "text/html", html);
        });



    // Systemstatus Seite
    webserver.on("/status", HTTP_GET, []() {

        String html = generateHtmlHeader();

        html += generateHtmlStatus(); // Statusleiste einfügen
        html += generateNavigation(); // Navigation einfügen

        html += "<h2>ESP System Status</h2><ul>";

        String tzLabel = preferences.getString("timezone", "DE");
        String tzDesc;

        tzDesc = tzLabel;

        struct tm timeinfo;
        if (getLocalTime(&timeinfo)) {
            char nowStr[32];
            strftime(nowStr, sizeof(nowStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
            html += "<li>Current Time: " + String(nowStr) + "</li>";
            html += "<li>Timezone: " + tzDesc + "</li>";
            html += "<li>Current week: " + String(currentWeek) + "</li>";
            html += "<li>Last week reset: " + String(lastResetWeek) + "</li>";

            unsigned long seconds = millis() / 1000;
            unsigned long days = seconds / 86400;
            unsigned long hours = (seconds % 86400) / 3600;
            unsigned long minutes = (seconds % 3600) / 60;
            unsigned long secs = seconds % 60;
            html += "<li>Uptime: " + String(days) + "d " + String(hours) + "h " + String(minutes) + "m " + String(secs) + "s</li>";

            html += "<br>";
        }

        html += "<li>Compiled on: <strong>" + (String)version + "</strong></li><br>";


        html += "<li>TFT Driver: " + tft_type + "</li>";

        html += "<li>TFT Size: " + String(TFT_WIDTH) + " x " + String(TFT_HEIGHT) + "</li>";

        html += "<br>";
        html += "<li>ChipModel: " + String(ESP.getChipModel()) + "</li>";
        html += "<li>ChipRevision: " + String(ESP.getChipRevision()) + "</li>";
        html += "<li>ChipCores: " + String(ESP.getChipCores()) + "</li>";
        html += "<li>Chip ID: " + String((uint32_t)ESP.getEfuseMac(), HEX) + "</li>";
        html += "<li>CPU Frequency: " + String(getCpuFrequencyMhz()) + " MHz</li><br>";

        html += "<li>Hostname: " + String(hostname) + ".local" + "</li>";
        html += "<li>IP Address: " + WiFi.localIP().toString() + "</li>";
        html += "<li>MAC Address: " + WiFi.macAddress() + "</li>";
        html += "<li>WiFi SSID: " + String(WiFi.SSID()) + "</li>";
        html += "<li>WiFi Mode: " + String(WiFi.getMode() == WIFI_AP ? "WIFI_AP" : (WiFi.getMode() == WIFI_STA ? "WIFI_STA" : "AP_STA")) + "</li>";
        html += "<li>WiFi Channel: " + String(WiFi.channel()) + "</li>";
        html += "<li>Signal Strength (RSSI): " + String(WiFi.RSSI()) + " dBm</li><br>";

        html += "<li>SDK Version: " + String(ESP.getSdkVersion()) + "</li><br>";

        html += "<li>Flash Size: " + String(ESP.getFlashChipSize() / 1024) + " KB</li>";
        html += "<li>Free Heap: " + String(ESP.getFreeHeap() / 1024) + " KB</li>";
        html += "<li>Sketch Size: " + String(ESP.getSketchSize() / 1024) + " KB</li><br>";
        html += "<li>Free Sketch Space: " + String(ESP.getFreeSketchSpace() / 1024) + " KB</li><br>";

        html += "<li>PSRam size: " + String(ESP.getPsramSize() / 1024) + " kB</li>";
        html += "<li>PSRam free: " + String(ESP.getFreePsram() / 1024) + " kB</li>";
        // html += "<li>PSRam used: " + String(psramAvailable == true ? "true" : "false") + "</li><br>";


        html += "<li>LittleFS Size: " + String(LittleFS.totalBytes() / 1024) + " KB</li>";
        html += "<li>LittleFS Used: " + String(LittleFS.usedBytes() / 1024) + " KB</li>";
        html += "<li>LittleFS Free: " + String((LittleFS.totalBytes() - LittleFS.usedBytes()) / 1024) + " KB</li><br>";


        if (photoresistorFound) {
            html += "<li>Photoresistor found on GPIO " + String(ADC_PIN) + "</li>";
            html += "<li>Actual brightness (0-255): " + String(currentBrightness) + "</li><br>";
        }
        else {
            html += "<li>Photoresistor not found on GPIO " + String(ADC_PIN) + "</li><br>";
        }



        html += "<li>TFT_SCLK: " + String(TFT_SCLK) + "</li>";
        //html += "<li>TFT_MISO: " + String(TFT_MISO) + "</li>";  
        html += "<li>TFT_MOSI: " + String(TFT_MOSI) + "</li>";
        html += "<li>TFT_CS: " + String(TFT_CS) + "</li>";
        html += "<li>TFT_DC: " + String(TFT_DC) + "</li>";
        html += "<li>TFT_RST: " + String(TFT_RST) + "</li><br>";

        html += "<li>BUTTON: " + String(BUTTON1) + "</li>";
        html += "<li>LED_BOARD: " + String(LED_BOARD) + "</li>";
        html += "<li>TOUCH_PIN: " + String(TOUCH_PIN) + "</li>";
        html += "<li>use Touch: " + String(useTouch ? "true" : "false") + "</li><br>";


        html += "<li>ADC_VCC: " + String(ADC_3V) + "</li>";
        html += "<li>ADC (photoresistor): " + String(ADC_PIN) + "</li>";
        html += "<li>ADC_GND: " + String(ADC_GND) + "</li>";
        if (photoresistorFound) {
            html += "<li>ADC val: " + String(getAdjustedAdcValue(analogRead(ADC_PIN))) + "</li><br>";
        }


#ifndef TFT_Backlight 
        html += "<li>TFT_Backlight: none</li>";
#else
        html += "<li>TFT_Backlight: " + String(TFT_Backlight) + "</li>";
#endif
        html += "<br>";


        html += "<li><h3>Actual Preferences</h3></li><ul>";

        for (int i = 0; i < MAX_WLAN; i++) {
            // Dynamisch berechnete Schlüssel
            String ssidKey = "ssid" + String(i + 1);

            if (preferences.getString(ssidKey.c_str(), "") != "") {
                if (preferences.getInt("lastWLan") != i) {
                    html += "<li><b>" + ssidKey + ":</b> " + preferences.getString(ssidKey.c_str(), "") + "</li>";
                }
                else {
                    html += "<li><b>" + ssidKey + ": " + preferences.getString(ssidKey.c_str(), "") + "</b></li>";
                }
            }
    
        }

        html += "<li><b>ntpServer1</b>: " + preferences.getString("ntpServer1", NTP_SERVER_1) + "</li>";
        html += "<li><b>ntpServer2</b>: " + preferences.getString("ntpServer2", NTP_SERVER_2) + "</li>";
        html += "<li><b>timezone</b>: " + preferences.getString("timezone", TIMEZONE_DEFAULT) + "</li>";
        html += "<li><b>background</b>: " + preferences.getString("background", "/faces/default") + "</li>";
        html += "<li><b>handset</b>: " + preferences.getString("handset", "") + "</li>";
        html += "<li><b>centerColor (RGB565)</b>: " + String(preferences.getUInt("centerColor", TFT_RED), HEX) + "</li>";
        html += "<li><b>centerSize</b>: " + String(preferences.getUInt("centerSize", 6)) + "</li>";

        uint8_t rotation = preferences.getUChar("tftRotation", 0);
        const char* rotationLabels[] = { "0&deg;", "90&deg;", "180&deg;", "270&deg;" };
        html += "<li><b>tftRotation</b>: " + String(rotationLabels[rotation]) + "</li>";

        // Booleans als Text
        html += "<li><b>use_adc</b>: " + String(preferences.getBool("use_adc", true) ? "true" : "false") + "</li>";
        html += "<li><b>stationMode</b>: " + String(preferences.getBool("stationMode", true) ? "true" : "false") + "</li>";
        html += "<li><b>showSecondhand</b>: " + String(preferences.getBool("showSecondHand", true) ? "true" : "false") + "</li>";
        html += "<li><b>smoothMinute</b>: " + String(preferences.getBool("smoothMinute", false) ? "true" : "false") + "</li>";

        html += "<li><b>minBrightness</b>: " + String(preferences.getUChar("minBrightness", 100)) + "</li>";
        html += "<li><b>maxBrightness</b>: " + String(preferences.getUChar("maxBrightness", 255)) + "</li>";

        html += "<li><b>lowThreshold</b>: " + String(preferences.getInt("lowThreshold", 40)) + "</li>";
        html += "<li><b>highThreshold</b>: " + String(preferences.getInt("highThreshold", 60)) + "</li>";
        html += "<li><b>adc Inverted</b>: " + String(preferences.getBool("adcInverted", false) ? "true" : "false") + "</li>";
        html += "<li><b>use Touch</b>: " + String(preferences.getBool("useTouch", false) ? "true" : "false") + "</li>";
        html += "</ul>";
        html += "</br>";
        html += "<li>Contact: holger.wagenlehner@gmx.de</li>";

        html += "<li><a href='https://github.com/holgiw/TFT-Clock-GC9A01' target='_blank'>GitHub</a></li>";

        html += "</ul>";
        html += generateNavigation(); // Navigation einfügen
        html += "</body></html>";
        webserver.send(200, "text/html", html);
        });

    webserver.on("/preview_defaultface", HTTP_GET, []() {
        const int headerSize = 54;
        const int rowSize = ((CLOCK_WIDTH * 3 + 3) / 4) * 4; // 3 Bytes pro Pixel für RGB888
        const int dataSize = rowSize * CLOCK_HEIGHT;
        const int fileSize = headerSize + dataSize;

        uint8_t* bmpData = new uint8_t[fileSize];
        memset(bmpData, 0, fileSize);

        // BMP-Header
        bmpData[0] = 'B'; bmpData[1] = 'M';
        *(uint32_t*)&bmpData[2] = fileSize;
        *(uint32_t*)&bmpData[10] = headerSize;
        *(uint32_t*)&bmpData[14] = 40;
        *(int32_t*)&bmpData[18] = CLOCK_WIDTH;
        *(int32_t*)&bmpData[22] = -CLOCK_HEIGHT; // Top-down BMP
        *(uint16_t*)&bmpData[26] = 1;
        *(uint16_t*)&bmpData[28] = 24; // 24-Bit Farbtiefe
        *(uint32_t*)&bmpData[34] = dataSize;

        // Pixel-Daten (RGB565 ? RGB888)
        for (int y = 0; y < CLOCK_HEIGHT; y++) {
            uint8_t* rowPtr = bmpData + headerSize + y * rowSize;
            for (int x = 0; x < CLOCK_WIDTH; x++) {
                uint16_t px = clockFace[y * CLOCK_WIDTH + x];

                // Transparente Farbe ersetzen
                if (px == TRANSPARENT_COLOR) {
                    rowPtr[x * 3 + 0] = 255; // Blau
                    rowPtr[x * 3 + 1] = 255; // Gr%uuml;n
                    rowPtr[x * 3 + 2] = 255; // Rot
                    continue;
                }

                // RGB565 ? RGB888
                uint8_t r = (px >> 8) & 0xF8; // obere 5 Bits
                uint8_t g = (px >> 3) & 0xFC; // mittlere 6 Bits
                uint8_t b = (px << 3) & 0xF8; // untere 5 Bits

                rowPtr[x * 3 + 0] = b; // Blau
                rowPtr[x * 3 + 1] = g; // Grün
                rowPtr[x * 3 + 2] = r; // Rot
            }
        }

        webserver.send_P(200, "image/bmp", (const char*)bmpData, fileSize);
        delete[] bmpData;
        });

    // Uhr-Gesichter verwalten
    webserver.on("/listfilesFaces", HTTP_GET, []() {

        size_t total = LittleFS.totalBytes();
        size_t used = LittleFS.usedBytes();


        String html = generateHtmlHeader();


        html += generateHtmlStatus(); // Statusleiste einfügen
        html += generateNavigation(); // Navigation einfügen
        html += "<h2>" + translate("Manage Clock Face Files") + " " + String(CLOCK_WIDTH) + " x " + String(CLOCK_HEIGHT) + "</h2><table border='1'><tr><th>" + translate("Preview/Set") + "</th></tr>";

        // Add built-in default face
        html += "<tr><td>";
        html += "<a href='/setbackground?file=face_default.bmp'>";
        html += "<img src='/preview_defaultface' style='width:80px;height:80px;border:1px solid #ccc'>";
        html += "</a><br>face_default.bmp<br><small>" + (String)TFT_WIDTH + " x " + (String)TFT_HEIGHT + " / " + "16 bpp"; +" </small> </td>";
        html += "</tr>";

        File root = LittleFS.open("/");
        File file = root.openNextFile();


        bool anyFile = false;
        while (file) {
            String name = file.name();

            //  DEBUG_PRINTLN(name);

            if (!file.isDirectory() && name.startsWith("face_") && name.endsWith(".bmp")) {
                anyFile = true;
                String shortName = name;
                String info = getBmpInfo(name);
                html += "<tr><td>";
                html += "<a href='/setbackground?file=" + shortName + "'>";
                html += "<img src='/file?name=" + name + "' style='width:80px;height:80px;border:1px solid #ccc'>";
                html += "</a><br>" + shortName + "<br><small>" + String(info) + "</small></td></tr>";
            }
            file = root.openNextFile();
        }

        if (!anyFile) html += "<tr><td colspan='3'>No BMP files found in /</td></tr>";
        html += "</table><hr>";

        // Hinweis und Download-Link f%uuml;r die ZIP-Datei
        html += "<h3>" + translate("Download Additional Clock Faces") + "</h3>";
        html += "<p>" + translate("You can download a ZIP file containing additional clock faces and hand sets from the following link: (use 'view raw')") + "</p>";
        html += "<a href='https://github.com/holgiw/TFT-Clock-GC9A01/blob/master/graphic/faces_handsets_240.zip' target='_blank'>Download faces_handsets_240.zip</a>";
        html += "<br><small>" + translate("After downloading, upload the extracted BMP files using the form below") + ".</small><hr>";

        if (used + (TFT_WIDTH * TFT_HEIGHT * 2) + 54 > total) {
            html += "<div style='color:red;font-weight:bold;'>" + translate("Warning: Not enough free space to upload new clock faces! Free up some space first") + ".</div><br><br>";
        }
        else {

            html += "<h3>" + translate("Upload New Clock Face") + "</h3>";
            html += "<small>" + translate("Requirements") + ": " + String(CLOCK_WIDTH) + " x " + String(CLOCK_HEIGHT) + " pixels, 16 - bit BMP(RGB565), " + translate("name must start with") + "  <code>face_</code></small><br><br>";

            html += "<form method = 'POST' action = '/upload' enctype = 'multipart/form-data' onsubmit = 'showProgress()'>";
            html += "<input type='file' name='upload' accept='.bmp' multiple required><br>";

            html += "<button type='submit'>" + translate("Upload") + " BMP</button>";
            html += "<div id='progress' style='display:none;'>Uploading... please wait</div>";
            html += "<script>function showProgress(){document.getElementById('progress').style.display='block';}</script></form><br><br>";
        }


        html += generateNavigation(); // Navigation einfügen
        html += "</body> </html>";
        webserver.send(200, "text/html", html);
        });

    // WLAN Netzwerke scannen
    webserver.on("/api/scanwifi", HTTP_GET, []() {
        String json = "";

        // beim Aufruf alle Netzwerke scannen
        if (WiFi.getMode() == WIFI_STA) {
            //int n = WiFi.scanNetworks();
             // JSON immer aus foundNetworks erzeugen
            json = "[";
            for (int i = 0; i < foundNetworkCount; ++i) {
                if (i > 0) json += ",";
                json += "{\"ssid\":\"" + foundNetworks[i].ssid + "\"";
                json += ",\"rssi\":" + String(foundNetworks[i].rssi);
                json += ",\"enc\":" + String(foundNetworks[i].enc);
                json += "}";
            }
            json += "]";
        }
        else {
            // im AP-Modus die letzten Scan-Ergebnisse zurückgeben
            json = "[";
            for (int i = 0; i < foundNetworkCount; ++i) {
                if (i > 0) json += ",";
                json += "{\"ssid\":\"" + foundNetworks[i].ssid + "\"";
                json += ",\"rssi\":" + String(foundNetworks[i].rssi);
                json += ",\"enc\":" + String(foundNetworks[i].enc);
                json += "}";
            }
            json += "]";
        }
        webserver.send(200, "application/json", json);
        });

    webserver.on("/api/rescanwifi", HTTP_POST, []() {
        scanAndCacheNetworks();
        webserver.send(200, "application/json", "{\"status\":\"ok\"}");
        });



    // Hauptseite - WLAN Einstellungen
    webserver.on("/", HTTP_GET, []() {

        String html = generateHtmlHeader();

        html += generateHtmlStatus(); // Statusleiste einfügen
        html += generateNavigation(); // Navigation einfügen

        html += "<h2>" + translate("Clock Setup") + "</h2>";

        html += generateLanguageSelector();

        html += "<form action = '/save' method = 'POST'>";

        html += "<button id='rescanBtn' type='button'>" + translate("Rescan Networks") + "</button><br>";



        for (int i = 0; i < MAX_WLAN; i++) {
            // Dynamisch berechnete Schlüssel
            String ssidKey = "ssid" + String(i + 1);
            String passKey = "pass" + String(i + 1);
            String ssid_select = "ssid_select" + String(i + 1);
            wifi_ssid[i] = preferences.getString(ssidKey.c_str(), "");

            String upperSsidKey = ssidKey; // Kopie erstellen
            upperSsidKey.toUpperCase();    // Kopie in Großbuchstaben umwandeln
            html += "<h3>" + upperSsidKey + "</h3>";

            // html += "<label for='" + ssidKey + "'>" + ssidKey + ":</label><br>";
            html += "<select id='" + ssid_select + "' onchange=\"document.getElementById('" + ssidKey + "').value=this.value\">";
            // html += "<option value=''>WLAN-Scan in progress</option>";
            html += "</select><br>";
            html += "<input name='" + ssidKey + "' id='" + ssidKey + "' placeholder='" + ssidKey + "' value='" + wifi_ssid[i] + "'><br>";
            html += "<small>" + translate("You can also enter an SSID manually") + ".</small><br>";

            html += "<input name='" + passKey + "' id='" + passKey + "' placeholder='Password' type='password' value=''><br>";
            if (WiFi.getMode() == WIFI_STA) {
                html += "<small>" + translate("Password is hidden.Leave empty to keep current") + ".</small>";
            }
            html += "<br><hr><br>";

            if (wifi_ssid[i] == "") {
                break; // Keine weiteren SSIDs, Schleife beenden
            }
        }

        
        html += "<br><br>";


        html += "<button type='submit'>" + translate("Save WiFi settings") + "</button></form><hr>";

        if (WiFi.getMode() == WIFI_STA) {


            html += "<form action='/applydisplaysettings' method='POST'>";

            html += "<table style='margin:auto;text-align:left;'><tr>";

            html += "<td><input type='checkbox' name='stationMode' value='1' ";
            html += preferences.getBool("stationMode", true) ? "checked" : "";
            html += "> " + translate("Train Station Mode") + "</td>";

            html += "<td><input type='checkbox' name='showSecondHand' value='1' ";
            html += preferences.getBool("showSecondHand", true) ? "checked" : "";
            html += "> " + translate("Show Seconds") + "</td>";

            html += "<td><input type='checkbox' name='smoothMinute' value='1' ";
            html += preferences.getBool("smoothMinute", true) ? "checked" : "";
            html += "> " + translate("Smooth Minute Hand") + "</td>";

            
            html += "<td>&nbsp;</td>"; // leeres Feld

            html += "</tr><tr>";

            // Neue Checkbox f%uuml;r Touch-Freigabe
            //html += "<td><input type='checkbox' name='useTouch' value='1' ";
            //html += preferences.getBool("useTouch", false) ? "checked" : "";
            //html += "> " + translate("Enable Touch") + "</td>";

            html += "<td><input type='checkbox' name='wifiActive' value='1' ";
            html += wifiActive ? "checked" : "";
            html += "> " + translate("Reconnect WiFi") + "</td>";
            

            html += "<td><input type='checkbox' name='loggingEnabled' value='1' ";
            html += loggingEnabled ? "checked" : "";
            html += "> " + translate("enable logging") + "</td>";

            
            html += "<td>Rotation: <select name='rotation'>";
            const char* rotationLabels[] = { "0&deg;", "90&deg;", "180&deg;", "270&deg;" };
            for (int i = 0; i <= 3; i++) {
                html += "<option value='" + String(i) + "'";
                if (i == tftRotation) html += " selected";
                html += ">" + String(rotationLabels[i]) + "</option>";
            }
            html += "</select></td>";

            // html += "<td><input type='checkbox' name='loggingEnabled' value='1' ";
            // html += loggingEnabled ? "checked" : "";
            // html += "> Logging aktivieren</td>";

            html += "<td valign=bottom><button type='submit'>" + translate("Apply") + "</button></td>";
            html += "</tr></table></form>";

            html += "<hr>";

            /*
            html += "<a href='/timezone_form'><button>Set Timezone</button></a><br><br>";

            html += "<a href='/listfilesFaces'><button>Manage Clock Face Files</button></a><br><br>";
            html += "<a href='/handsets'><button>Manage Hand Sets</button></a><br><br>";

            html += "<form action='/syncnow' method='POST'><button type='submit'>Sync Time Now</button></form><br>";
            html += "<form action='/brightness' method='POST'><button type='submit'>Brightness Settings</button></form><br>";
               */

        }




        html += "<script>";
        html += "document.getElementById('rescanBtn').onclick = function() {";
        html += "  fetch('/api/rescanwifi', {method: 'POST'})";
        html += "    .then(() => {";
        html += "      select1.innerHTML = \"<option>WLAN scan in progress...</option>\";";
        html += "      select2.innerHTML = \"<option>WLAN scan in progress...</option>\";";
        html += "      setTimeout(function() {";
        html += "        fetch('/api/scanwifi').then(response => response.json()).then(data => {";
        html += "          select1.innerHTML = \"<option value=''>" + translate("select network") + "</option>\";";
        html += "          data.forEach(function(net) {";
        html += "            var opt = document.createElement('option');";
        html += "            opt.value = net.ssid;";
        html += "            opt.text = net.ssid + ' (' + net.rssi + ' dBm)';";

        for (int i = 0; i < MAX_WLAN; i++) {
            String ssidKey = "ssid" + String(i + 1);
            String select = "select" + String(i + 1);
            html += "            " + select + ".appendChild(opt);";
            if (preferences.getString(ssidKey.c_str(), "") == "") {
                break; // Keine weiteren SSIDs, Schleife beenden
            }
        }

        html += "          });";        

        html += "        });";
        html += "      }, 2000);"; // 2 Sekunden warten für Scan
        html += "    });";
        html += "};";

        html += "window.addEventListener('DOMContentLoaded', function() {";
        for (int i = 0; i < MAX_WLAN; i++) {

           

            // Dynamisch berechnete Schlüssel
            String ssidKey = "ssid" + String(i + 1);
            String passKey = "pass" + String(i + 1);
            String select = "select" + String(i + 1); 
            String input = "input" + String(i + 1);
            String current = "current" + String(i + 1); 
            String ssid_select = "ssid_select" + String(i + 1);
            
                        

            html += "  var " + select + " = document.getElementById('" + ssid_select + "');";
            html += "  var " + input + " = document.getElementById('ssid1');";
            html += "  var " + current + " = " + input + ".value;";
            html += "  " + select + ".innerHTML = \"<option>WLAN scan in progress...</option>\";";
            html += "  fetch('/api/scanwifi')";
            html += "    .then(response => response.json())";
            html += "    .then(data => {";
            html += "      " + select + ".innerHTML = \"<option value=''>" + translate("select network") + "</option>\";";
            html += "      data.forEach(function(net) {";
            html += "        var opt = document.createElement('option');";
            html += "        opt.value = net.ssid;";
            html += "        opt.text = net.ssid + ' (' + net.rssi + ' dBm)';";
            html += "        if(net.ssid === " + current + ") opt.selected = true;";
            html += "        " + select + ".appendChild(opt);";
            html += "      });";
            html += "    })";
            html += "    .catch(() => { " + select + ".innerHTML = \"<option>Scan failed</option>\"; });";

            if (preferences.getString(ssidKey.c_str(), "") == "") {
                break; // Keine weiteren SSIDs, Schleife beenden
            }

        }
        html += "});";
        html += "</script>";

        html += generateNavigation(); // Navigation einfügen


        html += "</body></html>";
        webserver.send(200, "text/html", html);
        });

    // Speichern der WiFi-Einstellungen
    webserver.on("/save", HTTP_POST, []() {
        //if (webserver.hasArg("ssid1")) {

            for (int i = 0; i < MAX_WLAN; i++) {
                // Dynamisch berechnete Schlüssel
                String ssidKey = "ssid" + String(i + 1);
                String passKey = "pass" + String(i + 1);

                preferences.putString(ssidKey.c_str(), webserver.arg(ssidKey));                
                if (webserver.arg(passKey) != "") preferences.putString(passKey.c_str(), webserver.arg(passKey));                
            }


            // leere Einträge aussortieren
            String tempSsid[MAX_WLAN];
            String tempPass[MAX_WLAN];


            int j = 0;
            for (int i = 0; i < MAX_WLAN; i++) {

                // Dynamisch berechnete Schlüssel
                String ssidKey = "ssid" + String(i + 1);
                String passKey = "pass" + String(i + 1);

                String ssid = trim(preferences.getString(ssidKey.c_str(), ""));                
                if (ssid.length() > 0) {
                    tempSsid[j] = preferences.getString(ssidKey.c_str(), "");
                    tempPass[j] = preferences.getString(passKey.c_str(), "");                    
                    j++;
                }
            }

            for (int i = 0; i < MAX_WLAN; i++) {
                // Dynamisch berechnete Schlüssel
                String ssidKey = "ssid" + String(i + 1);
                String passKey = "pass" + String(i + 1);

                preferences.putString(ssidKey.c_str(), tempSsid[i]);
                preferences.putString(passKey.c_str(), tempPass[i]);   

                wifi_ssid[i] = tempSsid[i];
                wifi_pass[i] = tempPass[i];
            }


            

            if (WiFi.getMode() == WIFI_STA) {
                webserver.send(200, "text/html", "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='0; url=/'>"
                    "<title>Settings saved</title></head><body style='font-family:Arial;text-align:center;'>"
                    "<h2>Settings saved</h2><p>.</p></body></html>");
            }
            else {
                webserver.send(200, "text/html", "<!DOCTYPE html><html><head>"
                    "<title>Settings saved</title></head><body style='font-family:Arial;text-align:center;'>"
                    "<h2>Settings saved</h2><p>Please connect to your home network and go to the ESP website at http:// IPADDRESS</p></body></html>");

                esp_reboot();
            }
       // }
        });

    // Upload-Formular anzeigen
    webserver.on("/upload", HTTP_GET, []() {
        webserver.send(200, "text/html", "<form method='POST' action='/upload' enctype='multipart/form-data' onsubmit='showProgress()'><input type='file' name='upload' accept='.bmp' multiple required><br><br><button type='submit'>Upload BMP</button><div id='progress' style='display:none;'>Uploading... please wait</div><script>function showProgress(){document.getElementById('progress').style.display='block';}</script></form><br><a href='/listfilesFaces'>Back to file list</a>");
        });

    // Datei-Upload verarbeiten
    webserver.on("/upload", HTTP_POST, []() {
        if (uploadSuccess) {
            webserver.sendHeader("Location", "/listfilesFaces", true);
            webserver.send(302, "text/plain", "");
        }
        else {
            String errorHtml = "<!DOCTYPE html><html><head><title>Upload Failed</title><meta name='viewport' content='width=device-width, initial-scale=1'></head><body style='font-family:Arial;text-align:center;'>";
            errorHtml += "<h2>Upload failed</h2>";
            errorHtml += "<p>Only .bmp files starting with <code>face_</code> or <code>hand_</code> are accepted.</p>";
            errorHtml += "<p>Please also check the available space</p>";
            errorHtml += "<a href='/upload'>Try again</a></body></html>";
            webserver.send(400, "text/html", errorHtml);
        }
        }, handleFileUpload);

    // Hintergrundbild setzen
    webserver.on("/setbackground", HTTP_GET, []() {
        if (webserver.hasArg("file")) {
            String file = webserver.arg("file");
            file.replace("..", "");
            if (!file.startsWith("/")) file = "/" + file;

            if (file == "/face_default.bmp") {
                selectedBackground = file;
                preferences.putString("background", file);
                freeClockFaceBuffer();
                loadClockFace();
                loadHandSprites();
                webserver.sendHeader("Location", "/listfilesFaces", true);
                webserver.send(302, "text/plain", "");
                return;
            }

            if (LittleFS.exists(file)) {
                selectedBackground = file;
                preferences.putString("background", file);
                DEBUG_PRINTLN("set bg to: " + file);
                freeClockFaceBuffer();
                loadClockFace();
                loadHandSprites();
                webserver.sendHeader("Location", "/listfilesFaces", true);
                webserver.send(302, "text/plain", "");
                return;
            }
        }
        webserver.send(404, "text/plain", "File not found");
        });

    // Datei löschen
    webserver.on("/delete", HTTP_GET, []() {
        if (webserver.hasArg("file")) {
            String path = webserver.arg("file");
            path.replace("..", "");
            if (!path.startsWith("/")) path = "/" + path;
            if (LittleFS.exists(path)) {
                LittleFS.remove(path);
                webserver.sendHeader("Location", "/files", true);
                webserver.send(302, "text/plain", "");
            }
            else {
                webserver.send(404, "text/plain", "File not found");
            }
        }
        });

    // Datei anzeigen (BMP)
    webserver.on("/file", HTTP_GET, []() {
        if (webserver.hasArg("name")) {
            String path = webserver.arg("name");
            if (!path.startsWith("/")) path = "/" + path;

            if (LittleFS.exists(path)) {
                File file = LittleFS.open(path, "r");

                // Prüfe den Dateityp basierend auf der Dateiendung
                if (path.endsWith(".log")) {
                    webserver.streamFile(file, "text/plain"); // Logdateien als Text senden
                }
                else if (path.endsWith(".bmp")) {
                    webserver.streamFile(file, "image/bmp"); // BMP-Dateien als Bild senden
                }
                else {
                    webserver.streamFile(file, "application/octet-stream"); // Andere Dateien als Binärdaten senden
                }

                file.close();
                return;
            }
        }
        webserver.send(404, "text/plain", "File not found");
        });

    // Hand-Sets verwalten
    webserver.on("/handsets", HTTP_GET, []() {

        size_t total = LittleFS.totalBytes();
        size_t used = LittleFS.usedBytes();

        String html = generateHtmlHeader();
        html += generateHtmlStatus(); // Statusleiste einfügen
        html += generateNavigation(); // Navigation einfügen
        html += "<h2>" + translate("Manage Clock Hand Sets") + " " + String(HAND_WIDTH) + " x " + String(HAND_HEIGHT) + "</h2><table border = '1'><tr><th colspan = 2>Preview / Set</th></tr>";

        String activeSet = preferences.getString("handset", "");
        std::set<String> foundSets;

        File root = LittleFS.open("/");
        File file = root.openNextFile();
        while (file) {
            String name = file.name();

            if (!file.isDirectory() && name.startsWith("hand_set") && name.endsWith(".bmp")) {
                int start = 8;
                int end = name.indexOf('_', start);
                if (end > start) {
                    String set = name.substring(start, end);
                    foundSets.insert(set);
                }
            }
            file = root.openNextFile();
        }


        String handHourBase64 = encodeBmpToBase64(handHour, HAND_WIDTH, HAND_HEIGHT);
        String handMinuteBase64 = encodeBmpToBase64(handMinute, HAND_WIDTH, HAND_HEIGHT);
        String handSecondBase64 = encodeBmpToBase64(handSecond, HAND_WIDTH, HAND_HEIGHT);



        // Always show default as built-in
        html += "<tr><td>0</td><td>";
        html += "<a href='/sethandset?set=default'>";
        html += "<img src='data:image/bmp;charset=utf-8;base64, " + handHourBase64 + "'> ";
        html += "<img src='data:image/bmp;charset=utf-8;base64, " + handMinuteBase64 + "'> ";
        html += "<img src='data:image/bmp;charset=utf-8;base64, " + handSecondBase64 + "'>";
        html += "</a>";
        html += "</td></tr>";


        for (const String& set : foundSets) {
            html += "<tr><td>" + set + (set == activeSet ? " (active)" : "") + "</td><td>";
            String hourPath = "/hand_set" + set + "_hour.bmp";
            String minutePath = "/hand_set" + set + "_minute.bmp";
            String secondPath = "/hand_set" + set + "_second.bmp";
            html += "<a href='/sethandset?set=" + set + "'>";
            html += LittleFS.exists(hourPath) ? "<img src='/file?name=" + hourPath + "'> " : "<img src='data:image/bmp;charset=utf-8;base64, " + handHourBase64 + "'> ";
            html += LittleFS.exists(minutePath) ? "<img src='/file?name=" + minutePath + "'> " : "<img src='data:image/bmp;charset=utf-8;base64, " + handMinuteBase64 + "'> ";
            html += LittleFS.exists(secondPath) ? "<img src='/file?name=" + secondPath + "'> " : "<img src='data:image/bmp;charset=utf-8;base64," + handSecondBase64 + "'>";
            html += "</a>";


            html += "</td>";

            html += "</tr>";
        }

        html += "</table><hr>";

        uint8_t hub_size = preferences.getUInt("centerSize", 6);
        uint32_t hub_color_rgb = preferences.getLong("centerColor", 0xEC0016);

        html += "<h2>" + translate("Centre point") + "</h2><form action = '/setcenter' method = 'POST'>";
        html += "<label>Size (Pixel):</label><br><input name='size' type='number' min='0' max='50' value='" + String(hub_size) + "'><br>";
        html += "<label>" + translate("Color (RGB hex, e.g. FF0000 = Red, 000000 = Black, EC0016 = DB red)") + ":</label><br><input name = 'color' value = '" + String(hub_color_rgb, HEX) + "'><br>";
        html += "<button type='submit'>" + translate("Apply") + "</button></form><hr>";

        if (used + 5818 > total) {
            html += "<div style='color:red;font-weight:bold;'>" + translate("Warning: Not enough free space to upload new hand sets!Free up some space first") + ".</div><br><br>";
        }
        else {

            html += "<h3>" + translate("Upload New Hand Set") + "</h3>";
            html += "<small>" + translate("Requirements") + ": " + String(HAND_WIDTH) + " x " + String(HAND_HEIGHT) + " pixels, 16 - bit BMP(RGB565), <br>name must start with <code>hand_set + no + _hour, _minute or _second.bmp e.g.hand_set1_second.bmp</code><br>Pivot point : " + String(int(HAND_WIDTH / 2)) + " / " + String(int(HAND_HEIGHT * 0.77)) + "<br><br>";
            html += "<form method='POST' action='/uploadhandset' enctype='multipart/form-data'>";

            html += "File: <input type='file' name='upload' accept='.bmp' multiple required><br><br>";
            html += "<button type='submit'>" + translate("Upload to Set") + "</button></form>";
        }
        html += "<br><br>";
        html += generateNavigation(); // Navigation einfügen

        html += "<br><br>";
        html += "</body></html>";
        //DEBUG_PRINTLN(html);
        webserver.send(200, "text/html", html);
        });

    webserver.on("/setcenter", HTTP_POST, []() {
        if (webserver.hasArg("size") && webserver.hasArg("color")) {
            hub_size = webserver.arg("size").toInt();
            uint32_t rgb = (uint32_t)strtoul(webserver.arg("color").c_str(), nullptr, 16);

            // Convert 24-bit RGB888 to RGB565
            uint8_t r = (rgb >> 16) & 0xFF;
            uint8_t g = (rgb >> 8) & 0xFF;
            uint8_t b = rgb & 0xFF;
            uint16_t rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);

            preferences.putUInt("centerSize", hub_size);
            preferences.putLong("centerColor", rgb);

            hub_color = rgb565;

        }
        webserver.send(200, "text/html",
            "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='0; url=/handsets'>"
            "<title>Updated</title></head><body style='font-family:Arial;text-align:center;'>"
            "<h2>Centre point updated</h2><p>back to handsets in 3 seconds</p></body></html>");
        });

    //  Handsets Datei-Upload verarbeiten
    webserver.on("/uploadhandset", HTTP_POST, []() {
        if (uploadSuccess) {
            // Sicherheitsprüfung auf Dateinamenmuster
            if (!uploadFilePath.endsWith(".bmp") || !uploadFilePath.startsWith("/hand_set")) {
                String errorHtml = "<!DOCTYPE html><html><head><title>Upload Failed</title><meta name='viewport' content='width=device-width, initial-scale=1'></head><body style='font-family:Arial;text-align:center;'>";
                errorHtml += "<h2>Upload failed</h2>";
                errorHtml += "<p>Only .bmp files starting with <code>hand_</code> are accepted for handset upload.</p>";
                errorHtml += "<a href='/handsets'>Try again</a></body></html>";
                webserver.send(400, "text/html", errorHtml);
                return;
            }
            String set = webserver.arg("set");
            //  String target = server.arg("target");
            String dir = "/";
            if (!LittleFS.exists(dir)) LittleFS.mkdir(dir);
            //  String finalPath = "/hand_set" + set + "_" + target + ".bmp";
            //  LittleFS.rename(uploadFilePath, finalPath);
            //  DEBUG_PRINTLN("[UPLOAD] Hand uploaded to: " + finalPath);
            DEBUG_PRINTLN("[UPLOAD] Hand uploaded to: " + uploadFilePath);
            webserver.sendHeader("Location", "/handsets", true);
            webserver.send(302, "text/plain", "");
        }
        else {
            webserver.send(500, "text/html", " Upload failed!<br><a href='/handsets'>Try again</a>");
        }
        }, handleFileUpload);


    // Handset setzen
    webserver.on("/sethandset", HTTP_GET, []() {
        if (webserver.hasArg("set")) {
            String chosen = webserver.arg("set");
            preferences.putString("handset", chosen);
            DEBUG_PRINTLN("[HANDSET] Set to: " + chosen);
            freeClockFaceBuffer();
            loadClockFace();
            loadHandSprites();
            updateClock();
            webserver.sendHeader("Location", "/handsets", true);
            webserver.send(302, "text/plain", "");
        }
        else {
            webserver.send(400, "text/plain", "Missing set name");
        }
        });

    // Handset l&ouml;schen
    webserver.on("/deletehandset", HTTP_GET, []() {
        if (webserver.hasArg("set")) {
            String set = webserver.arg("set");
            String targets[] = { "hour", "minute", "second" };
            for (const String& target : targets) {
                String path = "/hand_set" + set + "_" + target + ".bmp";
                DEBUG_PRINTLN("[DELETE] Looking for: " + path);
                if (LittleFS.exists(path)) {
                    LittleFS.remove(path);
                    DEBUG_PRINTLN("[DELETE] Removed: " + path);
                }
            }
            webserver.sendHeader("Location", "/handsets", true);
            webserver.send(302, "text/plain", "");
        }
        else {
            webserver.send(400, "text/plain", "Missing set name");
        }
        });

    // ESP neu starten
    webserver.on("/reboot", HTTP_GET, []() {
        webserver.send(200, "text/html", "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='10; url=/'><title>Rebooting</title></head><body style='font-family:Arial;text-align:center;'><h2>" + translate("Rebooting...") + "</h2><p>" + translate("Return to the main page in 10 seconds or refresh the website when the ESP is online again") + ".</p></body></html>");
        esp_reboot();
        });

    // Factory Reset    
    webserver.on("/factoryReset", HTTP_GET, []() {
        factoryReset();
        });

    // Sofortige Zeitsynchronisation
    webserver.on("/syncnow", HTTP_POST, []() {
        setupNTP();
        struct tm timeinfo;
        getLocalTime(&timeinfo);

        char timeStr[32];
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);

        String html = "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='0; url=/'>"
            "<title>Time Synced</title></head><body style='font-family:Arial;text-align:center;'>"
            "<h2>Time synced</h2><p>" + String(timeStr) + "</p><p>Returning to main page in 3 seconds.</p></body></html>";

        webserver.send(200, "text/html", html);
        });

}





// Handhabt den Datei-Upload
void handleFileUpload() {
    HTTPUpload& upload = webserver.upload();

    if (upload.status == UPLOAD_FILE_START) {
        uploadFilePath = upload.filename;
        uploadFilePath.replace("..", "");
        uploadFilePath.replace("#", "_");

        if (!uploadFilePath.startsWith("/")) uploadFilePath = "/" + uploadFilePath;

        // Nur bestimmte Dateinamenmuster zulassen
        if (!uploadFilePath.endsWith(".bmp") ||
            !(uploadFilePath.startsWith("/face_") || uploadFilePath.startsWith("/hand_set"))) {
            DEBUG_PRINTLN("[UPLOAD] Invalid filename: must start with 'face_' or 'hand_set' and end with '.bmp'");
            uploadSuccess = false;
            return;
        }

        uploadFilePath.replace("..", "");
        if (!uploadFilePath.startsWith("/")) uploadFilePath = "/" + uploadFilePath;

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
                String ext = uploadFilePath;
                ext.toLowerCase();
                if (ext.endsWith(".bmp")) {
                    bool isHand = uploadFilePath.indexOf("hour") > 0 || uploadFilePath.indexOf("minute") > 0 || uploadFilePath.indexOf("second") > 0;
                    if (uploadFilePath.startsWith("/face_")) {
                        DEBUG_PRINTLN("[UPLOAD] Detected Clock Face upload");

                        if (!scaleAndSaveBmp(uploadFilePath.c_str(), uploadFilePath.c_str(), TFT_WIDTH, TFT_HEIGHT)) {
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

// Lädt eine BMP-Datei in einen Sprite ohne PSRAM
bool loadBmpToSprite(TFT_eSprite* sprite, const char* filename) {

    if (psramAvailable) {
        return loadBmpToSprite_PS_RAM(sprite, filename);
    }

   
    File bmp = LittleFS.open(filename, "r");
    if (!bmp) return false;

    uint8_t header[54];
    if (bmp.read(header, 54) != 54 || header[0] != 'B' || header[1] != 'M') {
        bmp.close();
        return false;
    }

    int32_t width = *(int32_t*)&header[18];
    int32_t height = *(int32_t*)&header[22];
    uint16_t bpp = *(uint16_t*)&header[28];
    uint32_t offset = *(uint32_t*)&header[10];

    if (width != CLOCK_WIDTH || abs(height) != CLOCK_HEIGHT || bpp != 16) {
        bmp.close();
        return false;
    }

    bool flip = height > 0;
    height = abs(height);

    bmp.seek(offset);
    for (int y = 0; y < height; y++) {
        int row = flip ? height - 1 - y : y;
        bmp.read((uint8_t*)rowBuffer, CLOCK_WIDTH * 2);

        for (int x = 0; x < CLOCK_WIDTH; x++) {
            rowBuffer[x] = setPixelBrightness(rowBuffer[x]);
        }

        sprite->pushImage(0, row, CLOCK_WIDTH, 1, rowBuffer);
    }

    bmp.close();
    return true;
}



// Lädt eine BMP-Datei in einen Sprite unter Verwendung von PSRAM und rotiert das Bild basierend auf der Display-Rotation
bool loadBmpToSprite_PS_RAM(TFT_eSprite* sprite, const char* filename) {

    
    File bmp = LittleFS.open(filename, "r");
    if (!bmp) return false;

    uint8_t header[54];
    if (bmp.read(header, 54) != 54 || header[0] != 'B' || header[1] != 'M') {
        bmp.close();
        return false;
    }

    int32_t width = *(int32_t*)&header[18];
    int32_t height = *(int32_t*)&header[22];
    uint16_t bpp = *(uint16_t*)&header[28];
    uint32_t offset = *(uint32_t*)&header[10];

    if (width != CLOCK_WIDTH || abs(height) != CLOCK_HEIGHT || bpp != 16) {
        bmp.close();
        return false;
    }

    bool flip = height > 0;
    height = abs(height);

    bmp.seek(offset);

    
    // Temporärer Buffer für die Bitmap-Daten
    uint16_t* tempBuffer = (uint16_t*)ps_malloc(CLOCK_WIDTH * CLOCK_HEIGHT * sizeof(uint16_t));
    if (!tempBuffer) {
        DEBUG_PRINTLN("PSRAM konnte nicht allokiert werden f%uuml;r Bitmap-Daten!");
        bmp.close();
        return false;
    }

    for (int y = 0; y < height; y++) {
        bmp.read((uint8_t*)&tempBuffer[y * width], width * 2);
    }

    // Neuen Buffer erstellen, der rotiert ist
    uint16_t* rotatedBuffer = (uint16_t*)ps_malloc(CLOCK_WIDTH * CLOCK_HEIGHT * sizeof(uint16_t));
    if (!rotatedBuffer) {
        DEBUG_PRINTLN("PSRAM konnte nicht allokiert werden f%uuml;r Rotation!");
        free(tempBuffer);
        bmp.close();
        return false;
    }
    
    tftRotation = preferences.getUChar("tftRotation", 0);
       
    // 4. Pixelrotation (Korrektur der Spiegelung und Drehung)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int newX = x, newY = y;

            switch (tftRotation) {
            case 0: // 0&deg;
                newX = x;
                newY = y;
                break;
            case 1: // 90&deg;
                newX = y;
                newY = width - x - 1;
                break;
            case 2: // 180&deg;
                newX = width - x - 1;
                newY = height - y - 1;
                break;
            case 3: // 270&deg;
                newX = height - y - 1;
                newY = x;
                break;
            }

            // Korrektur für invertierte Darstellung
            newY = height - newY - 1;

            if (newX >= 0 && newX < width && newY >= 0 && newY < height) {
                rotatedBuffer[newY * width + newX] = setPixelBrightness(tempBuffer[y * width + x]);
            }
        }
    }

    // 5. In den Sprite pushen
    sprite->setSwapBytes(true); // Byte-Reihenfolge für TFT_eSPI korrigieren
    sprite->fillSprite(TFT_BLACK); // Hintergrund löschen
    for (int y = 0; y < height; y++) {
        sprite->pushImage(0, y, width, 1, &rotatedBuffer[y * width]);
    }

    // 6. Speicher freigeben
    free(tempBuffer);
    free(rotatedBuffer);

    bmp.close();
    return true;
}

// Rotiert die Zeiger basierend auf der Display-Rotation
float rotatedAngle(float angle, int orientation) {
    if (psramAvailable) {
        return angle + (orientation * 90);
    }   
    return angle;
}

//überprüft, ob die BMP-Datei das erwartete Format hat
bool checkBmpFormat(const String& filename, int expectedWidth = CLOCK_WIDTH, int expectedHeight = CLOCK_HEIGHT) {
    File bmpFile = LittleFS.open(filename, "r");
    if (!bmpFile) {
        DEBUG_PRINTLN("[BMP Check] Failed to open file");
        return false;
    }

    uint8_t header[54];
    if (bmpFile.read(header, 54) != 54) {
        DEBUG_PRINTLN("[BMP Check] Failed to read header");
        bmpFile.close();
        return false;
    }

    if (header[0] != 'B' || header[1] != 'M') {
        DEBUG_PRINTLN("[BMP Check] Not a BMP file");
        bmpFile.close();
        return false;
    }

    int32_t width = *(int32_t*)&header[18];
    int32_t height = *(int32_t*)&header[22];
    uint16_t bpp = *(uint16_t*)&header[28];

    bmpFile.close();

    if (width != expectedWidth || abs(height) != expectedHeight || bpp != 16) {
        DEBUG_PRINTF("[BMP Check] Invalid BMP dimensions or format: %d x %d, %d bpp", width, height, bpp);
        return false;
    }

    DEBUG_PRINTLN("[BMP Check] BMP format valid");
    return true;
}

// Liest die BMP-Header-Informationen und gibt sie als String zurück
String getBmpInfo(const String& filename) {
    // Normalisiere Pfad (einfach und eindeutig)
    String file = filename;
    if (!file.startsWith("/")) file = "/" + file;

    File bmp = LittleFS.open(file, "r");
    if (!bmp) {
        return "n/a";
    }
    uint8_t header[54];
    if (bmp.read(header, 54) != 54 || header[0] != 'B' || header[1] != 'M') {
        bmp.close();
        return "n/a";
    }

    int32_t width = *(int32_t*)&header[18];
    int32_t height = *(int32_t*)&header[22];
    uint16_t bpp = *(uint16_t*)&header[28];
    bmp.close();

    return String(abs(width)) + "&nbsp;x&nbsp;" + String(abs(height)) + " / " + String(bpp) + " bpp";
}

// Skaliert eine BMP-Datei auf die gewünschte Größe und speichert sie
bool scaleAndSaveBmp(const char* sourcePath, const char* targetPath, int outW, int outH) {
    DEBUG_PRINTLN("[BMP Scale] Scaling BMP: " + String(sourcePath) + " to " + String(targetPath));
    File bmp = LittleFS.open(sourcePath, "r");
    if (!bmp) {
        DEBUG_PRINTLN("[BMP Scale] Failed to open source file");
        return false;
    }

    uint8_t header[54];
    if (bmp.read(header, 54) != 54 || header[0] != 'B' || header[1] != 'M') {
        bmp.close();
        DEBUG_PRINTLN("[BMP Scale] Invalid BMP header");
        return false;
    }

    int32_t inW = *(int32_t*)&header[18];
    int32_t inH = *(int32_t*)&header[22];
    uint16_t bpp = *(uint16_t*)&header[28];
    uint32_t offset = *(uint32_t*)&header[10];

    if (inW <= 0 || abs(inH) <= 0) {
        bmp.close();
        DEBUG_PRINTLN("[BMP Scale] Invalid BMP dimensions");
        return false;
    }

    bool flip = inH > 0;
    inH = abs(inH);
    float scaleX = (float)inW / outW;
    float scaleY = (float)inH / outH;

    int inRowSize = ((inW * (bpp / 8) + 3) / 4) * 4;
    uint8_t* rowBuf = (uint8_t*)malloc(inRowSize);
    if (!rowBuf) {
        bmp.close();
        DEBUG_PRINTLN("[BMP Scale] Memory allocation failed");
        return false;
    }

    uint16_t* outImage = new uint16_t[outW * outH];

    for (int y = 0; y < outH; y++) {
        int srcY = flip ? (inH - 1 - int(y * scaleY)) : int(y * scaleY);
        bmp.seek(offset + inRowSize * srcY);
        bmp.read(rowBuf, inRowSize);

        for (int x = 0; x < outW; x++) {
            int srcX = int(x * scaleX);
            uint16_t pixel = 0;

            if (bpp == 16) {
                // 16 bpp (RGB565) → direkt übernehmen
                uint16_t* row16 = (uint16_t*)rowBuf;
                pixel = row16[srcX];
            }
            else if (bpp == 24) {
                // 24 bpp (RGB888) → 16 bpp (RGB565)
                uint8_t* row24 = rowBuf + (srcX * 3);
                uint8_t r = row24[2];
                uint8_t g = row24[1];
                uint8_t b = row24[0];
                pixel = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
            }
            else if (bpp == 32) {
                // 32 bpp (ARGB8888) → 16 bpp (RGB565)
                uint8_t* row32 = rowBuf + (srcX * 4);
                uint8_t r = row32[2];
                uint8_t g = row32[1];
                uint8_t b = row32[0];
                pixel = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
            }

            outImage[y * outW + x] = pixel;
        }
    }

    bmp.close();
    free(rowBuf);

    // Header schreiben
    File out = LittleFS.open(targetPath, "w");
    if (!out) {
        delete[] outImage;
        DEBUG_PRINTLN("[BMP Scale] Failed to open target file");
        return false;
    }

    const int rowSize = ((outW * 2 + 3) / 4) * 4;
    const int dataSize = rowSize * outH;
    const int fileSize = 54 + dataSize;
    uint8_t bmpHeader[54] = { 0 };

    bmpHeader[0] = 'B'; bmpHeader[1] = 'M';
    *(uint32_t*)&bmpHeader[2] = fileSize;
    *(uint32_t*)&bmpHeader[10] = 54;
    *(uint32_t*)&bmpHeader[14] = 40;
    *(int32_t*)&bmpHeader[18] = outW;
    *(int32_t*)&bmpHeader[22] = -outH; // Top-down BMP
    *(uint16_t*)&bmpHeader[26] = 1;
    *(uint16_t*)&bmpHeader[28] = 16;
    *(uint32_t*)&bmpHeader[34] = dataSize;

    out.write(bmpHeader, 54);

    for (int y = 0; y < outH; y++) {
        uint8_t rowOut[rowSize];
        memset(rowOut, 0, rowSize);
        memcpy(rowOut, &outImage[y * outW], outW * 2);
        out.write(rowOut, rowSize);
    }

    out.close();
    delete[] outImage;
    return true;
}

// --- Funktion: Löscht die gespeicherten WiFi-Konfigurationen ---
void eraseWiFiConfig() {
    // WLAN trennen und komplett deaktivieren
    WiFi.disconnect(true, true);  // true,true => auch gespeicherte Daten löschen    
    delay(100);
    WiFi.mode(WIFI_OFF);
    delay(1000);

    // Zusätzlich: Manuell NVS-Einträge für WiFi löschen
    nvs_handle_t nvsHandle;
    esp_err_t err = nvs_open("wifi", NVS_READWRITE, &nvsHandle);
    if (err == ESP_OK) {
        nvs_erase_all(nvsHandle);
        nvs_commit(nvsHandle);
        nvs_close(nvsHandle);
        DEBUG_PRINTLN("WiFi-NVS-Eintr&auml;ge gel&ouml;scht.");
    }
    else {
        DEBUG_PRINTF("Fehler beim &ouml;ffnen des NVS-WiFi-Handles: %s\n", esp_err_to_name(err));
    }
}

// --- Funktion: Löscht den gesamten NVS-Speicher ---
void eraseAllNVS() {
    // Löscht gesamten NVS-Speicher
    esp_err_t result = nvs_flash_erase();
    if (result == ESP_OK) {
        DEBUG_PRINTLN("Kompletter NVS-Speicher gelöscht (inkl. WiFi, Preferences).");
        nvs_flash_init();  // Wichtig: Danach wieder initialisieren!
    }
    else {
        DEBUG_PRINTF("NVS-Erase fehlgeschlagen: %s\n", esp_err_to_name(result));
    }
}

// --- Funktion: F%ührt einen Factory Reset durch ---
void factoryReset() {
    tft.fillScreen(TFT_BLACK);
    preferences.begin("clock", false);
    preferences.putInt("firstStart", 0);
    preferences.end();
    delay(100);
    DEBUG_PRINTLN(">>> Factory Reset gestartet...");
    LittleFS.begin();
    LittleFS.format();
    LittleFS.end();
    eraseWiFiConfig();
    eraseAllNVS();
    delay(2000);
    DEBUG_PRINTLN(">>> Neustart...");
    esp_reboot();
         
}

// --- Funktion: führt einen Reboot durch mit Anzeige ---
void esp_reboot() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextSize(TFT_TEXT_SIZE);
    tft.setCursor(20, (CLOCK_HEIGHT / 2));
    tft.println("Rebooting...");
    delay(900);
    tft.fillScreen(TFT_BLACK);
    delay(100);
    ESP.restart();
}


// --- Funktion: Prüft, ob ein wöchentlicher Neustart fällig ist ---
void checkWeeklyRestart() {

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return;

    if (timeinfo.tm_wday == 0 && timeinfo.tm_hour == 3 && timeinfo.tm_min == 5) {

        lastResetWeek = preferences.getInt("last_reset_week", -1);

        char weekStr[3];
        strftime(weekStr, sizeof(weekStr), "%V", &timeinfo); // ISO-Woche (01–53)
        currentWeek = atoi(weekStr);

        if (lastResetWeek == -1) {
            lastResetWeek = currentWeek;
            preferences.putInt("last_reset_week", lastResetWeek);
        }


        if (currentWeek != lastResetWeek) {
            DEBUG_PRINTF("→ W&ouml;chentlicher Reboot in Woche %d\n", currentWeek);
            preferences.putInt("last_reset_week", currentWeek);
            preferences.end();
            delay(1000);
            ESP.restart();
        }
    }
}


void startWiFiScan__() {
    if (!isScanning) {
        DEBUG_PRINTLN("[WiFi] Starting asynchronous scan...");
        WiFi.scanNetworks(true); // Asynchroner Scan
        isScanning = true;
    }
}

void checkWiFiScan__() {
    if (isScanning) {
        int scanStatus = WiFi.scanComplete();
        if (scanStatus == WIFI_SCAN_RUNNING) {
            // Scan läuft noch
            // DEBUG_PRINTLN("[WiFi] Scan in progress...");
        }
        else if (scanStatus >= 0) {
            // Scan abgeschlossen
            DEBUG_PRINTLN("[WiFi] Scan complete. Found networks: " + String(scanStatus));
            for (int i = 0; i < scanStatus && i <= MAX_NETWORKS; i++) {
                foundNetworks[i].ssid = WiFi.SSID(i);
                foundNetworks[i].rssi = WiFi.RSSI(i);
                foundNetworks[i].enc = WiFi.encryptionType(i);
                foundNetworkCount++;
                DEBUG_PRINTLN("  [WiFi] " + foundNetworks[i].ssid + " (" + String(foundNetworks[i].rssi) + " dBm) " + (foundNetworks[i].enc == WIFI_AUTH_OPEN ? "Open" : "Secured"));

            }
            DEBUG_PRINTLN("[WiFi] done");
            WiFi.scanDelete(); // Ergebnisse löschen
            isScanning = false;
        }
        else {
            // Fehler beim Scan
            DEBUG_PRINTLN("[WiFi] Scan failed with error: " + String(scanStatus));
            isScanning = false;
        }
    }
}
// --- Funktion: Scannt verfügbare WLANs und speichert sie im Cache ---
void scanAndCacheNetworks() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextSize(TFT_TEXT_SIZE);
    tft.setCursor(10, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 8));
    tft.println("WLAN-Scan...");

    DEBUG_PRINTLN("[WiFi] Scanning for WiFi networks...");
    digitalWrite(LED_BOARD, HIGH);
    int n = WiFi.scanNetworks();
    foundNetworkCount = 0;
    for (int i = 0; i < n && i <= MAX_NETWORKS; i++) {
        foundNetworks[i].ssid = WiFi.SSID(i);
        foundNetworks[i].rssi = WiFi.RSSI(i);
        foundNetworks[i].enc = WiFi.encryptionType(i);
        foundNetworkCount++;

        DEBUG_PRINTLN("  [WiFi] " + foundNetworks[i].ssid + " (" + String(foundNetworks[i].rssi) + " dBm) " + (foundNetworks[i].enc == WIFI_AUTH_OPEN ? "Open" : "Secured"));

    }    
    WiFi.scanDelete(); // Ergebnisse löschen
    DEBUG_PRINTLN("[WiFi] done.");
    digitalWrite(LED_BOARD, LOW);
}

// --- Funktion: Wechselt zum nächsten Preset ---
void switchToNextPreset() {
    // Sammle alle gültigen Presets
    std::vector<int> validPresets;
    for (int i = 0; i < MAX_PRESETS; i++) {
        if (!presets[i].name.isEmpty() && !presets[i].url.isEmpty()) {
            validPresets.push_back(i);
        }
    }

    if (validPresets.empty()) {
        DEBUG_PRINTLN("[PRESET] No valid presets found");
        return;
    }

    // Bestimme den aktuellen Preset-Index
    String currentPresetName = preferences.getString("currentPreset", "");
    int currentIndex = -1;
    for (size_t i = 0; i < validPresets.size(); i++) {
        if (presets[validPresets[i]].name == currentPresetName) {
            currentIndex = (int)i;
            break;
        }
    }

    // Wähle das nächste Preset
    int nextIndex = (currentIndex + 1) % validPresets.size();
    int nextPresetIndex = validPresets[nextIndex];

    // Lade das nächste Preset
    String nextPresetUrl = presets[nextPresetIndex].url;

    // Sicherstellen, dass die URL ab "/api" beginnt
    if (!nextPresetUrl.startsWith("/api")) {
        DEBUG_PRINTLN("[PRESET] Invalid URL format, adjusting...");
        int apiIndex = nextPresetUrl.indexOf("/api");
        if (apiIndex != -1) {
            nextPresetUrl = nextPresetUrl.substring(apiIndex);
        }
        else {
            DEBUG_PRINTLN("[PRESET] URL does not contain '/api', aborting...");
            return;
        }
    }

    DEBUG_PRINTLN("[PRESET] Switching to preset: " + presets[nextPresetIndex].name + " -> " + nextPresetUrl);

    // Speichere den aktuellen Preset-Namen
    preferences.putString("currentPreset", presets[nextPresetIndex].name);

    // Entferne die Basis-URL, falls vorhanden
    int queryStart = nextPresetUrl.indexOf('?');
    if (queryStart == -1) {
        DEBUG_PRINTLN("[PRESET] No query parameters found in URL");
        return;
    }
    String query = nextPresetUrl.substring(queryStart + 1);

    // Parse die Parameter
    while (query.length() > 0) {
        int ampersandIndex = query.indexOf('&');
        String param = query.substring(0, ampersandIndex);
        if (ampersandIndex == -1) {
            query = "";
        }
        else {
            query = query.substring(ampersandIndex + 1);
        }

        int equalsIndex = param.indexOf('=');
        if (equalsIndex == -1) continue;

        String key = param.substring(0, equalsIndex);
        String value = param.substring(equalsIndex + 1);

        // Wende die Einstellungen an
        if (key == "face") {
            value.replace("..", "");
            if (!value.startsWith("/")) value = "/" + value;
            if (value == "/face_default.bmp" || LittleFS.exists(value)) {
                preferences.putString("background", value);
                selectedBackground = value;               
            }
        }
        else if (key == "handSet") {
            preferences.putString("handset", value);            
        }
        else if (key == "timeZone") {
            preferences.putString("timezone", value);
            setupNTP();
        }
        else if (key == "hubSize") {
            hub_size = value.toInt();
            preferences.putUInt("centerSize", hub_size);
        }
        else if (key == "hubColor") {
            uint32_t rgb = strtoul(value.c_str(), NULL, 16);
            uint8_t r = (rgb >> 16) & 0xFF;
            uint8_t g = (rgb >> 8) & 0xFF;
            uint8_t b = rgb & 0xFF;
            hub_color = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
            preferences.putLong("centerColor", rgb);
        }
        else if (key == "stationMode") {
            stationMode = (value == "1" || value.equalsIgnoreCase("true"));
            preferences.putBool("stationMode", stationMode);
        }
        else if (key == "rotation") {
            tftRotation = value.toInt();
            if (tftRotation >= 0 && tftRotation <= 3) {
                preferences.putUChar("tftRotation", tftRotation);
                firstRun = true;
                if (!psramAvailable) {
                    tft.setRotation(tftRotation);
                }                
            }
        }
        else if (key == "showSecondHand") {
            showSecondHand = (value == "1" || value.equalsIgnoreCase("true"));
            preferences.putBool("showSecondHand", showSecondHand);
        }
        else if (key == "smoothMinute") {
            smoothMinute = (value == "1" || value.equalsIgnoreCase("true"));
            preferences.putBool("smoothMinute", smoothMinute);
        }
    }

    freeClockFaceBuffer();
    loadClockFace();
    loadHandSprites();
    updateClock();

    // Speichere den aktuellen Preset-Namen
    preferences.putString("currentPreset", presets[nextPresetIndex].name);

    DEBUG_PRINTLN("[PRESET] Switched to preset: " + presets[nextPresetIndex].name);
}


// --- Funktion: Wechselt zum n&auml;chsten Hintergrundbild ---
void switchToNextBackground() {
    std::vector<String> faces;

    // 1) Sammle alle face_*.bmp Dateien aus LittleFS (verwende Pfad wie File::name() liefert)
    File root = LittleFS.open("/");
    if (root) {
        File f = root.openNextFile();
        while (f) {
            String name = f.name(); // meist mit führendem '/'
            // Akzeptiere sowohl "/face_..." als auch "face_..."
            if (name.startsWith("/")) {
                if (name.startsWith("/face_") && name.endsWith(".bmp")) {
                    // Duplikate vermeiden
                    bool exists = false;
                    for (const auto& s : faces) if (s == name) { exists = true; break; }
                    if (!exists) faces.push_back(name);
                }
            }
            else {
                if (name.startsWith("face_") && name.endsWith(".bmp")) {
                    String n = "/" + name;
                    bool exists = false;
                    for (const auto& s : faces) if (s == n) { exists = true; break; }
                    if (!exists) faces.push_back(n);
                }
            }
            f = root.openNextFile();
        }
        root.close();
    }

    // 2) Stelle sicher, dass builtin default vorhanden ist (am Anfang)
    bool hasDefault = false;
    for (const auto& s : faces) if (s == "/face_default.bmp") { hasDefault = true; break; }
    if (!hasDefault) faces.insert(faces.begin(), "/face_default.bmp");

    if (faces.empty()) {
        DEBUG_PRINTLN("[TOUCH] No faces found");
        return;
    }

    // 3) Normalisiere aktuellen Auswahlwert
    String sel = selectedBackground;
    sel.trim();
    if (!sel.startsWith("/")) sel = "/" + sel;

    // 4) Bestimme aktuellen Index
    int idx = -1;
    for (size_t i = 0; i < faces.size(); ++i) {
        if (faces[i] == sel) { idx = (int)i; break; }
    }

    // 5) Wenn nicht gefunden: versuche eine tolerantere Suche (ohne führenden '/')
    if (idx < 0) {
        String selNoSlash = sel;
        if (selNoSlash.startsWith("/")) selNoSlash = selNoSlash.substring(1);
        for (size_t i = 0; i < faces.size(); ++i) {
            String cmp = faces[i];
            if (cmp.startsWith("/")) cmp = cmp.substring(1);
            if (cmp == selNoSlash) { idx = (int)i; break; }
        }
    }

    // 6) Wähle nächstes Element:
    int next;
    if (idx < 0) {
        // Falls aktuelle Auswahl unbekannt ist, w&auml;hle erstes user-face (wenn default an pos 0) sonst 0
        if (faces.size() > 1 && faces[0] == "/face_default.bmp") next = 1;
        else next = 0;
    }
    else {
        next = (idx + 1) % (int)faces.size();
    }

    // 7) übernehme und speichere
    selectedBackground = faces[next];
    preferences.putString("background", selectedBackground);

    // Debug
    DEBUG_PRINT("[TOUCH] Faces: ");
    for (const auto& s : faces) DEBUG_PRINT(s + " ");
    DEBUG_PRINTLN();
    DEBUG_PRINTLN("[TOUCH] Current: " + sel + " idx=" + String(idx) + " -> Next: " + selectedBackground);
        
    freeClockFaceBuffer();
    loadClockFace();
    loadHandSprites();
    updateClock();
}

// --- Funktion: Touch prüfen (nicht-blockierend, mit Entprellung) ---
void checkTouchInput() {

    uint16_t var = touchRead(TOUCH_PIN);

    bool state = false;

    if (var > 15000 && var < 65535) state = true;

    // DEBUG_PRINTLN("Touch read: " + String(var));
    //DEBUG_PRINTLN("Touch state: " + String(state));

    // Flanke LOW->HIGH (kurzer Tip) mit Debounce
    if (state && !touchLastState && (millis() - touchLastMillis) > TOUCH_DEBOUNCE_MS) {
        touchLastMillis = millis();
        DEBUG_PRINTLN("switch");
        //switchToNextBackground();
        switchToNextPreset();
    }
    touchLastState = state;
}


// Validiert den geladenen Preferences-Eintrag für background und repariert falls nötig
static void validateSelectedBackground() {
    // Normalisieren
    selectedBackground.trim();
    if (selectedBackground.length() == 0) selectedBackground = "/face_default.bmp";
    if (!selectedBackground.startsWith("/")) selectedBackground = "/" + selectedBackground;

    DEBUG_PRINTLN("[BG] Pref load: '" + selectedBackground + "'");

    // LittleFS muss gemountet sein
    if (!LittleFS.exists(selectedBackground)) {
        DEBUG_PRINTLN("[BG] File not found: " + selectedBackground);
        // Versuche tolerant auch ohne f%uuml;hrenden Slash (falls gespeichert ohne '/')
        String withoutSlash = selectedBackground;
        if (withoutSlash.startsWith("/")) withoutSlash = withoutSlash.substring(1);
        if (LittleFS.exists("/" + withoutSlash)) {
            selectedBackground = "/" + withoutSlash;
            DEBUG_PRINTLN("[BG] Found (alt) file: " + selectedBackground);
        }
        else {
            // Fallback auf Default
            selectedBackground = "/face_default.bmp";
            preferences.putString("background", selectedBackground);
            DEBUG_PRINTLN("[BG] Falling back to default and saved: " + selectedBackground);
            return;
        }
    }

    // Prüfe BMP-Format (Größe / bpp)
    if (!checkBmpFormat(selectedBackground)) {
        DEBUG_PRINTLN("[BG] BMP format invalid: " + selectedBackground);
        selectedBackground = "/face_default.bmp";
        preferences.putString("background", selectedBackground);
        DEBUG_PRINTLN("[BG] Falling back to default and saved: " + selectedBackground);
        return;
    }

    DEBUG_PRINTLN("[BG] Background OK: " + selectedBackground);
}

// Aktualisiert die Zeigerbreiten und lädt die Zeiger-Sprites neu
void updateHandWidths(int newHourWidth, int newMinuteWidth, int newSecondWidth) {
    // Aktualisiere die globalen Breiten
    hourHandWidth = newHourWidth;
    minuteHandWidth = newMinuteWidth;
    secondHandWidth = newSecondWidth;

    // Alte Sprites löschen
    hourHandSprite.deleteSprite();
    minuteHandSprite.deleteSprite();
    secondHandSprite.deleteSprite();

    // Neue Sprites erstellen
    hourHandSprite.createSprite(hourHandWidth, HAND_HEIGHT);
    hourHandSprite.setSwapBytes(true);
    hourHandSprite.setColorDepth(16);
    hourHandSprite.setPivot(hourHandWidth / 2, HAND_HEIGHT * 0.77);

    minuteHandSprite.createSprite(minuteHandWidth, HAND_HEIGHT);
    minuteHandSprite.setSwapBytes(true);
    minuteHandSprite.setColorDepth(16);
    minuteHandSprite.setPivot(minuteHandWidth / 2, HAND_HEIGHT * 0.77);

    secondHandSprite.createSprite(secondHandWidth, HAND_HEIGHT);
    secondHandSprite.setSwapBytes(true);
    secondHandSprite.setColorDepth(16);
    secondHandSprite.setPivot(secondHandWidth / 2, HAND_HEIGHT * 0.77);

    // Zeiger neu laden
    loadHandSprites();
}

// Parst die Zeigerbreiten aus dem Dateinamen des Hintergrundbildes (test)
void parseBackgroundFilename(const String& filename, int& hourWidth, int& minuteWidth, int& secondWidth) {
    // Standardwerte setzen
    hourWidth = HAND_WIDTH;
    minuteWidth = HAND_WIDTH;
    secondWidth = HAND_WIDTH;

    // Suche nach dem ersten `!`
    int firstHash = filename.indexOf('!');
    if (firstHash == -1) {
        // Kein `!` gefunden, Standardwerte verwenden
        return;
    }

    // Schneide den relevanten Teil nach dem ersten `#` ab
    String params = filename.substring(firstHash + 1);

    // Teile die Parameter anhand von `!`
    int secondHash = params.indexOf('!');
    int thirdHash = params.indexOf('!', secondHash + 1);

    if (secondHash != -1 && thirdHash != -1) {
        // Extrahiere die Werte
        hourWidth = params.substring(0, secondHash).toInt();
        minuteWidth = params.substring(secondHash + 1, thirdHash).toInt();
        secondWidth = params.substring(thirdHash + 1).toInt();
    }

    if (hourWidth <= 0) hourWidth = HAND_WIDTH;
    if (minuteWidth <= 0) minuteWidth = HAND_WIDTH;
    if (secondWidth < 0) secondWidth = HAND_WIDTH;

    if (hourWidth > HAND_WIDTH) hourWidth = HAND_WIDTH;
    if (minuteWidth > HAND_WIDTH) minuteWidth = HAND_WIDTH;
    if (secondWidth > HAND_WIDTH) secondWidth = HAND_WIDTH;

}

// ####################################################################
// ### LOG-FUNKTIONEN #################################################
void deleteAllLogFiles() {
    
    File root = LittleFS.open("/");
    File file = root.openNextFile();

    while (file) {
        String fileName = file.name();
        file.close(); // Datei schließen, bevor sie gelöscht wird

        if (fileName.endsWith(".log")) {
            if (LittleFS.remove("/" + fileName)) {
                DEBUG_PRINTLN("[LOG] Successfully deleted: " + fileName);
            }
        }
        file = root.openNextFile(); // Nächste Datei öffnen
    }
    preferences.putInt("logFileNumber", 0); // Log-Dateinummer zurücksetzen
}

void setLogFileName() {

    
    int logFileNumber = preferences.getInt("logFileNumber", 0);
    logFileNumber++;
    preferences.putInt("logFileNumber", logFileNumber);

    if (logFileNumber > 99) {
        logFileNumber = 1;
        preferences.putInt("logFileNumber", logFileNumber);
        preferences.putBool("loggingEnabled", false); // Logging deaktivieren
        deleteAllLogFiles();
    }

    char formattedLogFileName[20];
    sprintf(formattedLogFileName, "/log_%04d.log", logFileNumber);
    logFileName = String(formattedLogFileName);


    // Überprüfen, ob Anzahl Logfiles > 9 ist
    if (logFileNumber > 9) {
        char oldLogFileName[20];
        sprintf(oldLogFileName, "/log_%04d.log", logFileNumber - 9);
        if (LittleFS.exists(oldLogFileName)) {
            LittleFS.remove(oldLogFileName);
            Serial.println("[LOG] Alte Logdatei gelöscht: " + String(oldLogFileName));
        }
    }
}

void logToFile(const String& message) {
    if (!loggingEnabled) {
        return; // Logging ist deaktiviert
    }

    // Überprüfe, ob LittleFS gemountet ist
    if (!LittleFS.begin()) {
        Serial.println("[LOG] LittleFS ist nicht gemountet. Log wird nicht geschrieben.");
        return;
    }

    // Überprüfe, ob die Nachricht leer ist oder nur aus Leerzeichen/Zeilenumbrüchen besteht
    String trimmedMessage = message;
    trimmedMessage.trim(); // Entfernt führende und nachfolgende Leerzeichen sowie \n, \r
    if (trimmedMessage.isEmpty()) {
        return; // Nachricht nicht schreiben
    }

    // Überprüfe, ob genügend Speicherplatz verfügbar ist
    size_t freeSpace = LittleFS.totalBytes() - LittleFS.usedBytes();
    if (freeSpace < 10 * 1024) { // Weniger als 10 KB frei
        Serial.println("[LOG] Nicht genügend Speicherplatz im LittleFS. Log wird nicht geschrieben.");
        return;
    }

    // Überprüfe die Größe des aktuellen Logfiles
    File currentLogFile = LittleFS.open(logFileName, FILE_READ);
    if (currentLogFile) {
        size_t fileSize = currentLogFile.size();
        currentLogFile.close();

        if (fileSize > 10 * 1024) { // Wenn die Datei größer als 10 KB ist
            setLogFileName(); // Neues Logfile erstellen
        }
    }

    // Öffne die Datei im Anhängemodus (append)
    File logFile = LittleFS.open(logFileName, FILE_APPEND);
    if (!logFile) {
        Serial.println("[LOG] Fehler beim Öffnen der Logdatei.");
        return;
    }

    // Zeitstempel generieren
    char timestamp[32];
    struct tm timeinfo;

    // Zeitzone aus den Preferences abrufen
    String timezone = preferences.getString("timezone", TIMEZONE_DEFAULT);
    configTzTime(timezone.c_str(), ntpServers[0]); // Zeitzone anwenden

    if (getLocalTime(&timeinfo, 1)) {
        strftime(timestamp, sizeof(timestamp), "[%Y-%m-%d %H:%M:%S] ", &timeinfo);
    }
    else {
        snprintf(timestamp, sizeof(timestamp), "[%lu ms] ", millis());
    }

    // Schreibe Zeitstempel und Nachricht in die Datei
    logFile.print(timestamp);
    logFile.println(message);
    logFile.close();
}

// eigenes trim function, da Arduino String keine trim()-Methode hat
String trim(const String& str) {
    int start = 0;
    int end = str.length() - 1;

    // Führende Leerzeichen entfernen
    while (start <= end && isspace(str[start])) {
        start++;
    }

    // Nachfolgende Leerzeichen entfernen
    while (end >= start && isspace(str[end])) {
        end--;
    }

    // Substring zurückgeben, der keine führenden oder nachfolgenden Leerzeichen enthält
    return str.substring(start, end + 1);
}