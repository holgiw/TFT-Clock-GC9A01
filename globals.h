#pragma once
// ####################################################################
// ### Globale Objekte, Variablen und Datenstrukturen #################
// ####################################################################
// Benoetigt config.h (davor eingebunden). Sortiert nach Modul/Verwendung
// (WLAN, Zeit/DCF77/RTC, Zifferblatt/Display, Helligkeit, Touch, Presets,
// System), damit die Zuordnung zu wifi_manager.h / time_sync.h / display.h /
// presets_manager.h / system_utils.h leichter nachvollziehbar ist.
// translation.h / uhr3.h werden hier wie im Original geladen (Projekt-eigene
// Dateien, unveraendert). Da diese Datei nur von uhr3.ino aus eingebunden
// wird (eine einzige Uebersetzungseinheit), stehen hier bewusst die echten
// Definitionen - keine extern-Deklarationen (das hatte zuvor zu Problemen
// gefuehrt).

// --- System / Allgemein ---
String currentLanguage = "de"; // Standardmäßig Deutsch

char version[20]; // Build-Version ("YYYY-MM-DD HH:MM:SS" = 19 Zeichen + Nullterminator)

bool loggingEnabled = false;  

bool initial = true;

String ipAddress = "";


// --- Kern-Hardwareobjekte (TFT, Webserver, Preferences, RTC, ...) ---
TFT_eSPI tft = TFT_eSPI();
WebServer webserver(80);
Preferences preferences;
DNSServer dnsServer;
WiFiUDP udp;

#if defined SDA_PIN && defined SCL_PIN
RTC_DS3231 rtc;
#endif

// --- WLAN ---
#define MAX_WLAN 15
String wifiSsid[MAX_WLAN];
String wifiPass[MAX_WLAN];

#define NOT_CONNECTED 0
#define CONNECTED 1
#define CONNECTED_NO_INTERNET 2

bool wifiActive = true;    

// MAC Adresse
uint8_t mac[6];
char hostname[32];
bool pingHostname = false;

bool softAPIP = false;  // Flag für SoftAP IP
long softAPIPstart = 0;  // Startzeit für SoftAP IP

struct WifiNetwork {
    String ssid;
    int rssi;
    int enc;
};
WifiNetwork availableNetworks[MAX_WLAN];
int foundNetworkCount = 0;
bool isScanning = false;

// --- Zeit, NTP, DCF77, RTC ---
// NTP-Server-Port
const int NTP_PORT = 123;
// NTP-Paketgröße
const int NTP_PACKET_SIZE = 48;
byte ntpPacket[NTP_PACKET_SIZE];

#if defined DCF77_DATAPIN && defined DCF77_INTERRUPT

bool dcf77Flank = false; // false = fallende Flanke, true = steigende Flanke
DCF77 dcf = DCF77(DCF77_DATAPIN, DCF77_DATAPIN, dcf77Flank);
volatile uint16_t dcf77Count = 0; // Anzahl der empfangenen DCF77-Signale (wird in der ISR verändert)

volatile bool dcfTimeFound = false; // wird in der ISR gelesen/verändert
time_t lastDcfSyncTime = 0; // Unix-Zeitstempel der letzten erfolgreichen DCF77-Synchronisation (0 = noch nie)

#endif

unsigned long lastNTPUpdate = 0; // Zeitpunkt des letzten RTC-Updates
unsigned long lastDCFUpdate = 0; // Wartezeit nach DCF77-Update, bevor RTC aktualisiert wird (ms)   
unsigned long lastRTCUpdate = 0; // Zeitpunkt des letzten RTC-Updates
unsigned long lastNtpSuccessMillis = 0; // Zeitpunkt (millis()) der letzten ERFOLGREICHEN NTP-Synchronisation (0 = noch nie); DCF77 uebernimmt die Systemzeit nur, wenn NTP seit laengerer Zeit nicht erfolgreich war

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
String tftType = "UNKNOWN";

TFT_eSprite backgroundSprite = TFT_eSprite(&tft);
TFT_eSprite hourHandSprite = TFT_eSprite(&tft);
TFT_eSprite minuteHandSprite = TFT_eSprite(&tft);
TFT_eSprite secondHandSprite = TFT_eSprite(&tft);

String selectedBackground = "/face_default.bmp";

bool stationMode;
bool smoothMinute;
bool showSecondHand;

int hourHandWidth = HAND_WIDTH;
int minuteHandWidth = HAND_WIDTH;
int secondHandWidth = HAND_WIDTH;

// nabe
uint16_t hubColor = 0;
uint8_t hubSize = 0;

bool firstRun = true;

uint8_t tftRotation = 0;


uint16_t rowBuffer[CLOCK_WIDTH];

static bool psramAvailable = false;

uint16_t* clockFaceBuffer = nullptr;

bool cs = true;

// --- Helligkeit / Fotowiderstand (ADC) ---
bool adcInverted = false; // Standardmäßig nicht invertiert

bool useAdc = false; 
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

// --- Touch ---
// --- Touch / Debounce State ---
unsigned long touchLastMillis = 0;
const unsigned long TOUCH_DEBOUNCE_MS = 300;
bool touchLastState = false;
// --- Touch enable flag: aktivieren erst nach Setup-Initialisierung ---
bool touchEnabled = false;
unsigned long touchEnableAt = 0; // Timestamp wann Touch freigeschaltet wird (ms)
bool useTouch = false; // Touch verwenden

// --- Presets ---
// Presets
#define MAX_PRESETS 15
struct Preset {
    String name;
    String url;
};
Preset presets[MAX_PRESETS];

// --- Datei-Upload / Wartung ---
File uploadFile;
String uploadFilePath = "";
bool uploadSuccess = false;

int lastResetWeek = -1;
int currentWeek = -1;

//Übersetzungen fÜr verschiedene Sprachen
#include "translation.h"
