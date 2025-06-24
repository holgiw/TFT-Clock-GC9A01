// howl@gmx.de
// stationsuhr 05/2025
// 
// https://github.com/holgiw/-TFT-Clock-GC9A01
// 
// 
// optimiert für ESP32-S2 Mini  (als Lolin S2 Pico compiliert)
// Filesystem: LittleFS
// TFT: GC9A01 / GC9D01 / ILI9341
// Partition: Default 4MB NO OTA, 2MB, 2MB
// TFT_eSPI: 2.5.34

// Prozessor
//#define ESP32_D1 // -- funktioniert nicht !!!! SPI & LittleFs ???
#define ESP32_S2

// TFT
//#define GC9A01
#define GC9A01_WITH_BACKLIGHT
//#define GC9D01
//#define ILI9341 


//#define DEBUG

#include <WiFi.h>
#include <WebServer.h>
#include <TFT_eSPI.h>
#include <Preferences.h>
#include <LittleFS.h>
#include <time.h>
#include <set>
#include <base64.h>
#include "nvs_flash.h"

#include "build_defs.h"

TFT_eSPI tft = TFT_eSPI();
WebServer server(80);
Preferences preferences;



#ifdef ESP32_D1  // Mini D1 ESP32 oder LOLIN32 (der mit dem Akku Anschluß)
// ##############################################################################
// wires
//               ESP32 PIN    TFT
//               3.3V         vcc     3v3             red
//               GND          gnd     ground          black
//see C:\Users\hwage\Documents\Arduino\libraries\TFT_eSPI\user_setups\Setup304__ESP32S3_GC9D01.h


#define LED_BOARD 2    // pin2  LED ESP32 D1 Mini
#define LED_BOARD 22   // pin22 LED LOLIN32

#define ADC_3V 33
#define ADC_PIN 34
#define ADC_GND 14

#define BUTTON1 16

#ifdef GC9D01
#define TFT_Backlight 21  // Backlight
#endif
#endif


#ifdef ESP32_S2  // Lolin S2 Pico
// ##############################################################################
// wires
//               ESP32 PIN    TFT
//               3.3V         vcc     3v3             red
//               GND          gnd     ground          blue
//  see C:\Users\hwage\Documents\Arduino\libraries\TFT_eSPI\user_setups\Setup304__ESP32S3_GC9D01.h




#define LED_BOARD 15 // BUILTIN LED

#define ADC_3V 1
#define ADC_PIN 2
#define ADC_GND 4

#define BUTTON1 16

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

#define TRANSPARENT_COLOR 0x0120


String tft_type = "UNKNOWN";

TFT_eSprite backgroundSprite = TFT_eSprite(&tft);
TFT_eSprite hourHandSprite = TFT_eSprite(&tft);
TFT_eSprite minuteHandSprite = TFT_eSprite(&tft);
TFT_eSprite secondHandSprite = TFT_eSprite(&tft);

String wifi_ssid;
String wifi_pass;
String wifi_ssid2;
String wifi_pass2;

String timezone = "CET-1CEST,M3.5.0,M10.5.0/3";
String ntpServer1 = "pool.ntp.org";
String ntpServer2 = "time.nist.gov";

bool initial = true;

String selectedBackground = "/face_default.bmp";

bool stationMode;
bool smoothMinute;
bool showSecondHand;

bool firstRun = true;

uint8_t tft_rotation = 0;

float fastSecond = 972.0f;// 966.0f;

uint16_t rowBuffer[CLOCK_WIDTH];

static bool psramAvailable = false;

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

uint16_t* clockFaceBuffer = nullptr;

/// <summary>
/// Passt die Helligkeit eines Pixels basierend auf der aktuellen Helligkeitseinstellung an.
/// </summary>
/// <param name="pixel">Der 16-Bit-Farbwert des Pixels, der angepasst werden soll.</param>
/// <returns>Der angepasste 16-Bit-Farbwert des Pixels, basierend auf der aktuellen Helligkeit. Wenn die Helligkeit maximal ist oder der Pixel transparent/schwarz ist, wird der ursprüngliche Wert zurückgegeben.</returns>
 
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
            Serial.println("[clockFaceBuffer] allocate psram");
            clockFaceBuffer = (uint16_t*)ps_malloc(bufSize);
        }
        else {
            Serial.println("allocte ram: " + bufSize);
            Serial.println("[clockFaceBuffer] allocate ram");
            clockFaceBuffer = (uint16_t*)malloc(bufSize);
        }
        if (!clockFaceBuffer) {
            Serial.println("Fehler: clockFaceBuffer konnte nicht allokiert werden!");
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
        Serial.println("[clockFaceBuffer] free");
    }
}

/// <summary>
/// Lädt die Grafiken für die Zeiger eines Uhren-Widgets, entweder aus einer benutzerdefinierten Konfiguration oder aus Standardwerten.
/// </summary>  
void loadHandSprites() {
    String set = preferences.getString("handset", "");

#ifdef DEBUG
    Serial.println("[HANDS] Active hand set: " + set);
#endif

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
#ifdef DEBUG
            Serial.println("[HANDS] Looking for: " + path);
#endif

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
#ifdef DEBUG
                    Serial.println("[HANDS] Failed to load " + h.label + ", fallback used.");
#endif
                }
                else {
#ifdef DEBUG
                    Serial.println("[HANDS] Loaded " + h.label);
#endif
                }
                // Serial.println("found");
            }
            else {
                h.sprite->pushImage(0, 0, HAND_WIDTH, HAND_HEIGHT, h.fallback);
                usedDefault = true;
#ifdef DEBUG
                Serial.println("[HANDS] Missing " + h.label + ", using default.");
#endif
            }
        }

#ifdef DEBUG
        if (!usedDefault) {
            Serial.println("[HANDS] Loaded handset: " + set);
        }
        else {
            Serial.println("[HANDS] Incomplete set, used default for missing hands.");
        }
#endif
    }
    else {
        for (int y = 0; y < HAND_HEIGHT; y++) {

            for (int x = 0; x < HAND_WIDTH; x++) {
                uint16_t px = handHour[y * HAND_WIDTH + x];
                rowBuffer[x] = setPixelBrightness(px);
            }
            hourHandSprite.pushImage(0, y, HAND_WIDTH, 1, rowBuffer);
        }
        for (int y = 0; y < HAND_HEIGHT; y++) {

            for (int x = 0; x < HAND_WIDTH; x++) {
                uint16_t px = handMinute[y * HAND_WIDTH + x];
                rowBuffer[x] = setPixelBrightness(px);
            }
            minuteHandSprite.pushImage(0, y, HAND_WIDTH, 1, rowBuffer);
        }
        for (int y = 0; y < HAND_HEIGHT; y++) {

            for (int x = 0; x < HAND_WIDTH; x++) {
                uint16_t px = handSecond[y * HAND_WIDTH + x];
                rowBuffer[x] = setPixelBrightness(px);
            }
            secondHandSprite.pushImage(0, y, HAND_WIDTH, 1, rowBuffer);
        }
#ifdef DEBUG
        Serial.println("[HANDS] No set selected, using defaults.");
#endif
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

void loop() {
    
    server.handleClient();
    
    if (WiFi.getMode() == WIFI_STA) {
        updateBrightness();
        checkWiFiReconnect();
        updateClock();
        // updateBrightness();
        checkNightlyTimeSync();  
        checkWeeklyRestart();
        initial = false;
    }

    checkButton();    
}

void checkButton() {
    if (digitalRead(BUTTON1) == HIGH) {

        uint8_t secs = 5;
        unsigned long pressStart = millis();

        // Einmalig Anzeige zeichnen
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.setTextSize(TFT_TEXT_SIZE);
        tft.setCursor(20, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 8));
        tft.println("Connected to:");
        tft.setCursor(20, (CLOCK_HEIGHT / 2));
        tft.println(wifi_ssid);
        tft.setCursor(20, (CLOCK_HEIGHT / 2) + (CLOCK_HEIGHT / 8));
        tft.println(WiFi.localIP());


        // Blockierender Loop während Button gedrückt
        while (digitalRead(BUTTON1) == HIGH) {
            if (millis() - pressStart > 10000 && millis() - pressStart < 15000) {
                tft.fillScreen(TFT_RED);
                tft.setTextColor(TFT_WHITE, TFT_RED);
                tft.setTextSize(TFT_TEXT_SIZE);
                tft.setCursor(20, CLOCK_HEIGHT / 2);
                tft.printf("Reset in %d secs", secs);
                delay(1000);
                if (secs>0) secs--;
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

        // Button wurde vor 10 Sek. losgelassen: nur Anzeige bleibt sichtbar
        delay(500); 
    }
}



float shortestAngleDiff(float from, float to) {
    float diff = fmodf(to - from + 540.0f, 360.0f) - 180.0f;
    return diff;
}

static float lastHourAngle = 0.0f;
static float lastMinuteAngle = 0.0f;

void updateClock() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return;

    int orientation = preferences.getUChar("tft_rotation", 0);

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

            lastMinuteAngle += angleDiff * 0.01f;  // noch feinere Bewegung

        }
        else {
            // Normale Minutenanzeige mit sanfter Korrektur bei Wechsel
            float rawMinAngle = timeinfo.tm_min * 6.0f;
            float targetMinAngle = rotatedAngle(rawMinAngle, orientation);
            float angleDiff = shortestAngleDiff(lastMinuteAngle, targetMinAngle);

            if (fabs(angleDiff) > 0.1f) {
                lastMinuteAngle += angleDiff * 0.1f;
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
        lastHourAngle += hourAngleDiff * 0.1f;  // Glättungsfaktor
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

    uint16_t color = preferences.getUInt("centerColor", TFT_RED);
    uint8_t size = preferences.getUInt("centerSize", 6);

    if (size > 0) {
        color = setPixelBrightness(color);
        backgroundSprite.fillCircle(CLOCK_WIDTH / 2, CLOCK_HEIGHT / 2, size, color);
    }

    backgroundSprite.pushSprite(0, 0);
}
 
void updateBrightness() {

    if (currentBrightness != lastAppliedBrightness) {
        loadClockFace();
        loadHandSprites();
        lastAppliedBrightness = currentBrightness;
    }

    if (use_adc) {

        int adcRaw = analogRead(ADC_PIN);

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
          //  Serial.printf("[ADC] lightPercent: %d, targetBrightness: %d\n", lightPercent, targetBrightness);
            
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
        currentBrightness = maxBrightness;
        targetBrightness = currentBrightness;
    }

#ifdef TFT_Backlight
    ledcWrite(TFT_Backlight, currentBrightness);  // 0–255

    // invertiert
   // ledcWrite(TFT_Backlight, 255 - currentBrightness);

   // Serial.println(255 - currentBrightness);
#endif

}

float easeInOutSine(float t) {
    // Intensität steuert die Kurve: 1.0 = Standard, >1.0 = steiler, <1.0 = flacher
    float intensity = 0.5f;
    return -(cos(PI * pow(t, intensity)) - 1.0f) / 2.0f;
}

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
    if (!getLocalTime(&timeinfo)) return;

    /*if (timeinfo.tm_sec == 30) {
        delay(1000);
        Serial.println("[TIME SYNC] Time sync triggered");
        WiFi.disconnect();
        wifi_ssid = "asdfghj";
        wifi_ssid2 = "asdfghj";
    }*/


    if (timeinfo.tm_hour == 2 && timeinfo.tm_min == 0 && timeinfo.tm_sec == 5 && !triggered2) {
        Serial.println("[TIME SYNC] Triggered at 02:00:05");
        setupNTP();
        triggered2 = true;
    }

    if (timeinfo.tm_hour == 3 && timeinfo.tm_min == 0 && timeinfo.tm_sec == 5 && !triggered3) {
        Serial.println("[TIME SYNC] Triggered at 03:00:05");
        setupNTP();
        triggered3 = true;
    }

    if (timeinfo.tm_hour == 4 && triggered2 && triggered3) {
        triggered2 = false;
        triggered3 = false;
    }
}

void checkWiFiReconnect() {
    static unsigned long lastAttempt = 0;
    if (WiFi.status() == WL_CONNECTED) return;

    unsigned long now = millis();
    if (now - lastAttempt < 300000) return;
    lastAttempt = now;

    Serial.println("[WiFi] Disconnected. Attempting reconnect...");
    WiFi.disconnect();
    connectWiFi(false);
}

void setup() {

    Serial.begin(115200);   


    unsigned long serialStart = millis();
    while (!Serial && (millis() - serialStart < 1000)) {
        delay(10);
    }

    Serial.println("[Setup] Start");


#if defined GC9A01 || defined (GC9A01_WITH_BACKLIGHT)
    tft_type = "GC9A01";
#elif defined GC9D01    
    tft_type = "GC9D01";    
#else   
    tft_type = "ILI9341";   
#endif



    preferences.begin("clock", false);


    // Prüfen, ob PSRAM vorhanden ist
    if (psramFound() and ESP.getFreePsram() > 2 * (CLOCK_WIDTH * CLOCK_HEIGHT * sizeof(uint16_t))) {
        psramAvailable = true;
        Serial.println("[INFO] PSRAM gefunden.");
    }
    else {
        psramAvailable = false;
        Serial.println("[INFO] Kein PSRAM gefunden, Hardware-Rotation wird verwendet.");
#ifdef GC9D01
        preferences.putUChar("tft_rotation", 0); 
#endif
    }
#ifndef GC9D01 // wird nur bei Display GC9D01 benötigt
    psramAvailable = false; 
#endif
 

    if (preferences.getInt("firstStart", 0) != 42) {
        Serial.println("[Preferences] First start detected, initializing...");

        preferences.putInt("firstStart", 42);

        preferences.putString("ssid", "");
        preferences.putString("pass", "");
        preferences.putString("ssid2", "");
        preferences.putString("pass2", "");

        preferences.putString("ntpServer1", "pool.ntp.org");
        preferences.putString("ntpServer2", "time.nist.gov");

        preferences.putString("timezone", "CET-1CEST,M3.5.0,M10.5.0/3");

        preferences.putUChar("tft_rotation", 0);
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

        preferences.putFloat("gammaBrightness", 2.2f);  // Gamma-Korrektur für Helligkeit
        
#if defined (GC9D01)  || defined (GC9A01_WITH_BACKLIGHT)
        preferences.putInt("lowThreshold", 1);
        preferences.putInt("highThreshold", 255);
#else
        preferences.putInt("lowThreshold", 40);
        preferences.putInt("highThreshold", 60);
#endif

        preferences.putUInt("centerColor", TFT_RED);   

        if (tft_type == "GC9A01" || tft_type == "ILI9341") {
            preferences.putUInt("centerSize", 6);
        }
        if (tft_type == "GC9D01") {
            preferences.putUInt("centerSize", 3);
        }        
       preferences.end();
       preferences.begin("clock", false);         
    }
     
    ntpServer1 = preferences.getString("ntpServer1", "pool.ntp.org");
    ntpServer2 = preferences.getString("ntpServer2", "time.nist.gov");  

    timezone = preferences.getString("timezone", "CET-1CEST,M3.5.0,M10.5.0/3");
    setTimezone(timezone);

    Serial.println("[NTP] Aktuelle NTP-Server: " + ntpServer1 + " / " + ntpServer2);
    Serial.println("Zeitzone eingestellt auf: " + timezone);

    stationMode = preferences.getBool("stationMode", true);
    smoothMinute = preferences.getBool("smoothMinute", false);
    showSecondHand = preferences.getBool("showSecondHand", true);

    lowThreshold = preferences.getInt("lowThreshold", 40);
    highThreshold = preferences.getInt("highThreshold", 60);    
    minBrightness = preferences.getUChar("minBrightness", 100);
    maxBrightness = preferences.getUChar("maxBrightness", 255); 

#if defined (GC9D01)  || defined (GC9A01_WITH_BACKLIGHT) 
    gammaBrightness = preferences.getFloat("gammaBrightness", 2.2f);  // Gamma-Korrektur für Helligkeit
#endif

    pinMode(BUTTON1, INPUT_PULLDOWN);

    // auf Fotowiderstand prüfen
#ifdef ADC_3V
    analogReadResolution(12);

    // ADC +3,3 / GND über GPIO
    pinMode(ADC_GND, OUTPUT);
    pinMode(ADC_3V, OUTPUT);
    delay(20);

    digitalWrite(ADC_GND, false);
    digitalWrite(ADC_3V, false);
    delay(20);
    adc_min = analogRead(ADC_PIN);

    digitalWrite(ADC_GND, true);
    digitalWrite(ADC_3V, true);
    delay(20);
    adc_max = analogRead(ADC_PIN);

    Serial.printf("ADC min: %d max: %d\n", adc_min, adc_max);

    if (adc_min < 1000 && adc_max > 3000) {
        Serial.println("found photoresistor");
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
  //  Serial.println("[SETUP] smoothMinute: " + String(smoothMinute));    

    pinMode(LED_BOARD, OUTPUT); digitalWrite(LED_BOARD, HIGH);

    if (!LittleFS.begin(true)) {
        Serial.println("[LittleFS] Mount Failed");
    }
    /* else {
        Serial.println("[LittleFS] Listing all files in root:");
        File root = LittleFS.open("/");
        File entry = root.openNextFile();
        while (entry) {
            Serial.printf(" - %s (%d bytes)  ", entry.name(), entry.size());
            entry = root.openNextFile();
        }
    } */

    tft.init();
    delay(50);
   
    tft.fillScreen(TFT_BLACK);

    tft_rotation = preferences.getUChar("tft_rotation", 0);

    selectedBackground = preferences.getString("background", "/face_default.bmp");
    Serial.println("selected background: " + selectedBackground);

#ifndef GC9D01
    tft.setRotation(tft_rotation);
#else
    if (!psramAvailable) {
        tft_rotation = 0;
        preferences.putUChar("tft_rotation", tft_rotation);
        tft.setRotation(tft_rotation);
        Serial.printf("[TFT] Using stored rotation: %d\n", tft_rotation);
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

    wifi_ssid = preferences.getString("ssid", "");
    wifi_pass = preferences.getString("pass", "");
    wifi_ssid2 = preferences.getString("ssid2", "");
    wifi_pass2 = preferences.getString("pass2", "");
         
   
    if (digitalRead(BUTTON1) == HIGH) startAP();
    

    connectWiFi(true);
    if (WiFi.getMode() != WIFI_STA) startAP();
    setupNTP();
    setupWebServer();
    server.begin();

    // MAC-Adresse holen    
    WiFi.macAddress(mac);
    
    snprintf(hostname, sizeof(hostname), "Clock-%02X%02X%02X",
        mac[3], mac[4], mac[5]);

    WiFi.setHostname(hostname);
    Serial.println("[WiFi] Hostname set to: " + String(hostname));

    digitalWrite(LED_BOARD, LOW);
}

/// <summary>
/// Startet einen WLAN-Access-Point mit dem Namen und Passwort 'clock123' und zeigt Verbindungsinformationen auf dem Display an.
/// </summary>
void startAP() {
#ifdef TFT_Backlight
    ledcWrite(TFT_Backlight, 255);
#endif
    WiFi.softAP("clock123", "clock123");
    Serial.println("[WiFi] Started Access Point: clock123");

    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setTextSize(TFT_TEXT_SIZE);
    tft.setCursor(20, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 8)) ;
    tft.println("AccessPoint active");
    tft.setCursor(20, (CLOCK_HEIGHT / 2));
    tft.println("clock123 clock123");
    tft.setCursor(20, (CLOCK_HEIGHT / 2 ) + (CLOCK_HEIGHT / 8));
    tft.println(WiFi.softAPIP());
}

bool connectWiFi(bool verbose_mode) {
#ifdef TFT_Backlight
    if (verbose_mode) {
        ledcWrite(TFT_Backlight, 255);
    }
#endif
    if (wifi_ssid == "" && wifi_ssid2 == "") return false;

    Serial.println(wifi_ssid);
    // Serial.println(wifi_pass);
   
           
    Serial.print("[WiFi] Trying primary SSID ");

    if (verbose_mode) {
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.setTextSize(TFT_TEXT_SIZE);
        tft.setCursor(20, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 8));
        tft.println("Connect to:");
        tft.setCursor(20, (CLOCK_HEIGHT / 2));
        tft.println(wifi_ssid);
    }

    //WiFi.enableIPv6();

    WiFi.begin(wifi_ssid.c_str(), wifi_pass.c_str());
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
        delay(200);
        Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n[WiFi] Connected to primary: " + wifi_ssid);
        if (verbose_mode) {
            tft.fillScreen(TFT_BLACK);
            tft.setTextColor(TFT_GREEN, TFT_BLACK);
            tft.setTextSize(TFT_TEXT_SIZE);
            tft.setCursor(20, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 8));
            tft.println("Connected to:");
            tft.setCursor(20, (CLOCK_HEIGHT / 2) );
            tft.println(wifi_ssid);
            tft.setCursor(20, (CLOCK_HEIGHT / 2) + (CLOCK_HEIGHT / 8));
            tft.println(WiFi.localIP());
        }
        delay(3000);
        if (!WiFi.softAPgetStationNum()) updateClock();
        return true;
    }

    if (wifi_ssid2 == "") return false;

    Serial.println(wifi_ssid2);
    // Serial.println(wifi_pass2);

    Serial.print("\n[WiFi] Primary failed, trying secondary SSID ");

    if (verbose_mode) {
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.setTextSize(TFT_TEXT_SIZE);
        tft.setCursor(20, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 8));
        tft.println("Connect to:.");
        tft.setCursor(20, (CLOCK_HEIGHT / 2));
        tft.println(wifi_ssid2);
    }

    WiFi.begin(wifi_ssid2.c_str(), wifi_pass2.c_str());
    start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 5000) {
        delay(200);
        Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n[WiFi] Connected to : " + wifi_ssid2);
        if (verbose_mode) {
            tft.fillScreen(TFT_BLACK);
            tft.setTextColor(TFT_GREEN, TFT_BLACK);
            tft.setTextSize(TFT_TEXT_SIZE);
            tft.setCursor(20, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 8));
            tft.println("Connected to");
            tft.setCursor(20, (CLOCK_HEIGHT / 2));
            tft.println(wifi_ssid2);
            tft.setCursor(20, ((CLOCK_HEIGHT / 2) + (CLOCK_HEIGHT / 8)));
            tft.println(WiFi.localIP());
        }
        delay(5000);
        if (!WiFi.softAPgetStationNum()) updateClock();
        return true;
    }

    Serial.println("[WiFi] Both connections failed.");
    if (verbose_mode) {
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.setTextSize(TFT_TEXT_SIZE);
        tft.setCursor(20, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 8));
        tft.println("WiFi failed");
        tft.setCursor(20, CLOCK_HEIGHT / 2);
        tft.println("AP Mode active");
    }
    return false;    
}

/// <summary>
/// Initialisiert die Zeitsynchronisierung über NTP und stellt die Zeitzone ein. Bei Fehlern werden bis zu 10 Versuche unternommen, um die Zeit zu erhalten. Im Fehlerfall wird die zuletzt bekannte Zeit verwendet.
/// </summary>
void setupNTP() {
    
    bool verbose_mode = false;
     
    if (verbose_mode) {
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.setTextSize(TFT_TEXT_SIZE);
        tft.setCursor(10, (CLOCK_HEIGHT / 2));
    }

    timezone = preferences.getString("timezone", "CET-1CEST,M3.5.0,M10.5.0/3");

    //Serial.println("[NTP] " + timezone);
    setTimezone(timezone);
    struct tm timeinfo;
    int attempts = 0;
    while (!getLocalTime(&timeinfo, 5000) && attempts < 10) {
        attempts++;
        if (verbose_mode) {
            tft.fillScreen(TFT_BLACK);
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.setCursor(10, (CLOCK_HEIGHT / 2));
            tft.printf("NTP failed (%d/10)", attempts);
        }
        delay(2000);
    }
    if (attempts >= 10) {
        if (verbose_mode) {
            tft.fillScreen(TFT_BLACK);
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.setCursor(10, (CLOCK_HEIGHT / 2));
            tft.println("NTP timeout! Using last known time.");
        }
        delay(3000);
    }
}

/// <summary>
/// Setzt die Zeitzone und konfiguriert die Zeitsynchronisation entsprechend.   
/// </summary>
/// <param name="tz">Die zu setzende Zeitzone als String.</param>
void setTimezone(String tz) {
    preferences.putString("timezone", tz);
    configTzTime(tz.c_str(), ntpServer1.c_str(), ntpServer2.c_str());
    Serial.println("Set Timezone: " + tz);
}

/// <summary> 
/// Sets up the web server routes for clock face and hand set changes.
/// </summary>
void setupWebServer() {

    // http://192.168.0.128/api/setface?file=face_bigben.bmp

    server.on("/api/setface", HTTP_GET, []() {
        if (server.hasArg("file")) {
            String file = server.arg("file");
            file.replace("..", "");
            if (!file.startsWith("/")) file = "/" + file;
            if (file == "/face_default.bmp" || LittleFS.exists(file)) {
                preferences.putString("background", file);
                selectedBackground = file;
                freeClockFaceBuffer();
                loadClockFace();
                loadHandSprites();
                updateClock();
                server.send(200, "application/json", "{\"status\":\"ok\",\"face\":\"" + file + "\"}");
                return;
            }
            server.send(404, "application/json", "{\"status\":\"error\",\"msg\":\"File not found\"}");
        }
        else {
            server.send(400, "application/json", "{\"status\":\"error\",\"msg\":\"Missing file parameter\"}");
        }
        });

    // http://192.168.0.128/api/sethandset?set=3

    server.on("/api/sethandset", HTTP_GET, []() {
        if (server.hasArg("set")) {
            String set = server.arg("set");
            Serial.println("[HANDSET] Set to: " + set);
            preferences.putString("handset", set);
            freeClockFaceBuffer();
            loadClockFace();
            loadHandSprites();
            updateClock();
            server.send(200, "application/json", "{\"status\":\"ok\",\"handset\":\"" + set + "\"}");
        }
        else {
            server.send(400, "application/json", "{\"status\":\"error\",\"msg\":\"Missing set parameter\"}");
        }
        });

    
    server.on("/set_timezone", HTTP_POST, []() {
        if (server.hasArg("ntpServer1")) {
            ntpServer1 = server.arg("ntpServer1");
            ntpServer2 = server.arg("ntpServer2");
            preferences.putString("ntpServer1", ntpServer1);
            preferences.putString("ntpServer2", ntpServer2);
            Serial.println("[NTP] NTP Server1 set to: " + ntpServer1);    
            Serial.println("[NTP] NTP Server2 set to: " + ntpServer2);
            setupNTP();
        }

        if (server.hasArg("timezone")) {
            String tz = server.arg("timezone");
            preferences.putString("timezone", tz);
            setTimezone(tz);            

            server.send(200, "text/html",
                "<!DOCTYPE html><html><head>"
                "<meta http-equiv='refresh' content='3; url=/' />"
                "<title>NTP / Timezone Updated</title></head>"
                "<body><h2>NTP / Timezone updated to: " + ntpServer1 + " / " + ntpServer2 + " / " + tz + "</h2>"
                "<p>Returning to the main page in 3 seconds...</p></body></html>"
            );
        }
        else {
            server.send(400, "text/plain", "Timezone parameter missing");
        }
        setupNTP();
        });

     

    server.on("/timezone_form", HTTP_GET, []() {
        String timezone = preferences.getString("timezone", "CET-1CEST,M3.5.0,M10.5.0/3");

        String html = "<!DOCTYPE html><html><head><title>Set Timezone</title></head><body>";
        html += "<h2>NTP Server / Timezone (DST String)</h2>";
        html += "<form method='POST' action='/set_timezone'>";

        html += "NTP Server1: <input type='text' name='ntpServer1' value='" + ntpServer1 + "'><br>";
        html += "NTP Server2: <input type='text' name='ntpServer2' value='" + ntpServer2 + "'><br><br>";

        html += "Timezone: <input type='text' name='timezone' value='" + String(timezone) + "' style = 'width: 300px;' required><br>";
        
        html += "<ul>";
        html += "<li><strong>Germany (DE)</strong> - With Daylight Saving Time (DST) automatically: <code>CET-1CEST,M3.5.0,M10.5.0/3</code></li>";
        html += "<li>Germany (DE) - Permanent Summer Time: <code>CEST-2</code></li>";
        html += "<li>Germany (DE) - Permanent Winter Time: <code>CET-1</code></li>";
        html += "<li><strong>United Kingdom (UK)</strong> - With Daylight Saving Time (DST) automatically: <code>GMT0BST,M3.5.0/1,M10.5.0</code></li>";
        html += "<li>United Kingdom (UK) - Permanent Summer Time: <code>BST-1</code></li>";
        html += "<li>United Kingdom (UK) - Permanent Winter Time: <code>GMT0</code></li>";
        html += "<li><strong>USA (Pacific Time, Los Angeles)</strong> - DST Auto: <code>PST8PDT,M3.2.0,M11.1.0</code></li>";
        html += "<li>USA (Central Time, Chicago) - DST Auto: <code>CST6CDT,M3.2.0,M11.1.0</code></li>";
        html += "<li>USA (Mountain Time, Denver) - DST Auto: <code>MST7MDT,M3.2.0,M11.1.0</code></li>";
        html += "<li>USA (Eastern Time, New York) - DST Auto: <code>EST5EDT,M3.2.0,M11.1.0</code></li>";
        html += "<li>USA (Eastern Time, New York) - Permanent Summer Time: <code>EDT-4</code></li>";
        html += "<li>USA (Eastern Time, New York) - Permanent Winter Time: <code>EST-5</code></li>";
        html += "<li><strong>Japan (Tokyo, JST)</strong> - No Daylight Saving Time: <code>JST-9</code></li>";
        html += "<li><strong>Australia (Sydney, AEDT)</strong> - DST Auto: <code>AEST-10AEDT,M10.1.0,M4.1.0/3</code></li>";
        html += "<li>Australia (Sydney, AEDT) - Permanent Summer Time: <code>AEDT-11</code></li>";
        html += "<li>Australia (Sydney, AEDT) - Permanent Winter Time: <code>AEST-10</code></li>";
        html += "<li><strong>India (New Delhi, IST)</strong> - No Daylight Saving Time: <code>IST-5:30</code></li>";
        html += "<li><strong>Brazil (Brasília, BRT)</strong> - No Daylight Saving Time: <code>BRT-3</code></li>";
        html += "<li><strong>China (Shanghai, CST)</strong> - No Daylight Saving Time: <code>CST-8</code></li>";
        html += "<li><strong>Singapore (SGT)</strong> - No Daylight Saving Time: <code>SGT-8</code></li>";
        html += "<li><strong>Indonesia (Jakarta, WIB)</strong> - No Daylight Saving Time: <code>WIB-7</code></li>";
        html += "<li><strong>South Korea (Seoul, KST)</strong> - No Daylight Saving Time: <code>KST-9</code></li>";
        html += "<li><strong>Argentina (Buenos Aires, ART)</strong> - No Daylight Saving Time: <code>ART-3</code></li>";
        html += "<li><strong>Chile (Santiago, CLT)</strong> - DST Auto: <code>CLT4CLST,M9.1.6/24,M4.1.6/24</code></li>";
        html += "<li><strong>New Zealand (Wellington, NZST)</strong> - DST Auto: <code>NZST-12NZDT,M9.5.0,M4.1.0/3</code></li>";
        html += "<li><strong>Fiji (Suva, FJT)</strong> - No Daylight Saving Time: <code>FJT-12</code></li>";
        html += "<li><strong>Nigeria (Lagos, WAT)</strong> - No Daylight Saving Time: <code>WAT-1</code></li>";
        html += "<li><strong>South Africa (Johannesburg, SAST)</strong> - No Daylight Saving Time: <code>SAST-2</code></li>";
        html += "<li><strong>Egypt (Cairo, EET)</strong> - No Daylight Saving Time: <code>EET-2</code></li>";

        html += "</ul>";


        html += "<button type='submit'>Save Timezone</button>";
        html += "<br><a href='/'>Back</a></body></html>";
        html += "</form></body></html>";
        server.send(200, "text/html", html);
        });

    server.on("/rename_form", HTTP_GET, []() {
        if (!server.hasArg("file")) {
            server.send(400, "text/plain", "Missing file parameter.");
            return;
        }

        String oldName = server.arg("file");
        String html = "<!DOCTYPE html><html><head><title>Rename File</title><meta name='viewport' content='width=device-width, initial-scale=1'>";
        html += "<style>body{font-family:Arial;text-align:center;}input{margin:10px;padding:10px;}</style></head><body>";
        html += "<h2>Rename File</h2>";
        html += "<form action='/rename' method='POST'>";
        html += "<input type='hidden' name='old' value='" + oldName + "'>";
        html += "<label>New Name:</label><br>";
        html += "<input name='new' value='" + oldName + "' required><br><br>";
        html += "<button type='submit'>Rename</button></form>";
        html += "<br><a href='/files'>Cancel</a></body></html>";
        server.send(200, "text/html", html);
        });

    server.on("/rename", HTTP_POST, []() {
        if (server.hasArg("old") && server.hasArg("new")) {
            String oldName = server.arg("old");
            String newName = server.arg("new");

            oldName.replace("..", ""); newName.replace("..", "");
            if (!oldName.startsWith("/")) oldName = "/" + oldName;
            if (!newName.startsWith("/")) newName = "/" + newName;

            if (LittleFS.exists(oldName)) {
                if (LittleFS.rename(oldName, newName)) {
                    server.sendHeader("Location", "/files", true);
                    server.send(302, "text/plain", "");
                }
                else {
                    server.send(500, "text/plain", "Rename failed.");
                }
            }
            else {
                server.send(404, "text/plain", "Original file not found.");
            }
        }
        else {
            server.send(400, "text/plain", "Missing parameters.");
        }
        });



    server.on("/scalebmp_form", HTTP_GET, []() {
        if (!server.hasArg("file")) {
            server.send(400, "text/plain", "Missing file name.");
            return;
        }
        String src = server.arg("file");
        String html = "<!DOCTYPE html><html><head><title>Scale BMP</title><meta name='viewport' content='width=device-width, initial-scale=1'><style>body{font-family:Arial;}input{margin:5px;}</style></head><body>";
        html += "<h2>Scale and Save BMP</h2>";
        html += "<form action='/scalebmp_run' method='GET'>";
        html += "Source: <input name='src' value='/" + src + "' readonly><br>";
        html += "Target: <input name='dst' value='/scaled_" + src + "'><br>";
        html += "Width: <input name='w' type='number' value='" + String(CLOCK_WIDTH) +"' required><br>";
        html += "Height: <input name='h' type='number' value='" + String(CLOCK_HEIGHT) + "' required><br>";
        html += "<button type='submit'>Scale and Save</button></form>";
        html += "<a href='/status'>Systemstatus</a><br>";
        html += "<a href='/files'>File Manager</a><br>";
        html += "<br><a href='/files'>Back</a></body></html>";
        server.send(200, "text/html", html);
        });

    server.on("/scalebmp_run", HTTP_GET, []() {
        if (!server.hasArg("src") || !server.hasArg("dst") || !server.hasArg("w") || !server.hasArg("h")) {
            server.send(400, "text/plain", "Missing parameters.");
            return;
        }

        String src = server.arg("src");
        String dst = server.arg("dst");
        int w = server.arg("w").toInt();
        int h = server.arg("h").toInt();

        bool ok = scaleAndSaveBmp(src.c_str(), dst.c_str(), w, h);
        if (ok) {
            server.send(200, "text/html", "<html><body style='font-family:Arial;'><h3>Scaling successful!</h3><p>Saved as: " + dst + "</p><a href='/files'>Back</a></body></html>");
        }
        else {
            server.send(500, "text/html", "<html><body style='font-family:Arial;'><h3>Failed to scale BMP.</h3><p>Check source file and format.</p><a href='/files'>Back</a></body></html>");
        }
        });


    server.on("/applydisplaysettings", HTTP_POST, []() {
        // Save to Preferences

        stationMode = server.hasArg("stationMode");
        showSecondHand = server.hasArg("showSecondHand");        
        smoothMinute = server.hasArg("smoothMinute");

        preferences.putBool("stationMode", stationMode);
        preferences.putBool("showSecondHand", showSecondHand);
        preferences.putBool("smoothMinute", smoothMinute);

        if (server.hasArg("rotation")) {
            tft_rotation = server.arg("rotation").toInt();  
            if (tft_rotation >= 0 && tft_rotation <= 3) {
                preferences.putUChar("tft_rotation", tft_rotation);
                firstRun = true;
                if (!psramAvailable) {
                    tft.setRotation(tft_rotation); // sofort anwenden
                }
            }

            freeClockFaceBuffer();
            loadClockFace();      // neu zeichnen mit neuer Ausrichtung
            loadHandSprites();            
        }

        server.send(200, "text/html",
            "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='3; url=/'><title>Gespeichert</title></head>"
            "<body style='font-family:Arial;text-align:center;'><h2>Saved</h2><p>Back...</p></body></html>");
        });

   



    server.on("/brightness", HTTP_POST, []() {
        String html = "<!DOCTYPE html><html><head><title>Brightness Settings</title><meta name='viewport' content='width=device-width, initial-scale=1'>";
        html += "<style>body{font-family:Arial;text-align:center;}input{margin:8px;padding:8px;width:80%;}</style></head><body>";
        html += "<h2>Brightness Settings</h2><form method='POST' action='/save_brightness'>";

        if (photoresistorFound) {
            html += "<label><input type='checkbox' name='use_adc' value='1' " + String(use_adc ? "checked" : "") + "> Enable Auto Brightness</label><br>";

            html += "<label>Low Threshold (0 - 100%):</label><br><input name='lowThreshold' type='number' min='0' max='100' value='" + String(lowThreshold) + "'><br>";
            html += "<label>High Threshold (0 - 100%):</label><br><input name='highThreshold' type='number' min='0' max='100' value='" + String(highThreshold) + "'><br>";
        }

        html += "<label>Min Brightness (0 - 255):</label><br><input name='minBrightness' type='number' min='0' max='255' value='" + String(minBrightness) + "'><br>";

        html += "<label>Max Brightness (0 - 255):</label><br><input name='maxBrightness' type='number' min='0' max='255' value='" + String(maxBrightness) + "'><br>";

#if defined (GC9D01)  || defined(GC9A01_WITH_BACKLIGHT) 
        html += "<label>Gamma Correction (0.1 - 3.0):</label><br>";
        html += "<input type='number' name='gamma' step='0.1' min='0.1' max='3.0' value='" + String(gammaBrightness) + "' required><br>";
       
#endif
        

        html += "<button type='submit'>Save</button></form>";

        if (photoresistorFound) {
            html += "<br>";
            html += "<hr><strong>Current ADC Value:</strong> " + String(currentAdcAvg) + "<br>";
            html += "<strong>Current Brightness:</strong> " + String(currentBrightness) + " / 255<br>";
            html += "<strong>Light (for Threshold):</strong> " + String(currentLightPercent) + " %<br>";

            html += "<br>";
            html += "<form method='GET' action='/brightness'><button type='submit'>Refresh</button></form>";
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


        
        html += "<br><a href='/'>Back</a></body></html>";
        server.send(200, "text/html", html);
        });

    server.on("/brightness", HTTP_GET, []() {
        String html = "<!DOCTYPE html><html><head><title>Brightness Settings</title><meta name='viewport' content='width=device-width, initial-scale=1'>";
        html += "<style>body{font-family:Arial;text-align:center;}input{margin:8px;padding:8px;width:80%;}</style></head><body>";
        html += "<h2>Brightness Settings</h2><form method='POST' action='/save_brightness'>";

        if (photoresistorFound) {
            html += "<label><input type='checkbox' name='use_adc' value='1' " + String(use_adc ? "checked" : "") + "> Enable Auto Brightness</label><br>";

            html += "<label>Low Threshold (0 - 100):</label><br><input name='lowThreshold' type='number' min='0' max='100' value='" + String(lowThreshold) + "'><br>";
            html += "<label>High Threshold (0 - 100):</label><br><input name='highThreshold' type='number' min='0' max='100' value='" + String(highThreshold) + "'><br>";
        }

        html += "<label>Min Brightness (0 - 255):</label><br><input name='minBrightness' type='number' min='0' max='255' value='" + String(minBrightness) + "'><br>";

        html += "<label>Max Brightness (0 - 255):</label><br><input name='maxBrightness' type='number' min='0' max='255' value='" + String(maxBrightness) + "'><br>";

#if defined (GC9D01)  || defined(GC9A01_WITH_BACKLIGHT) 
        html += "<label>Gamma Correction (0.1 - 3.0):</label><br>";
        html += "<input type='number' name='gamma' step='0.1' min='0.1' max='3.0' value='" + String(gammaBrightness) + "' required><br>";
#endif


        html += "<button type='submit'>Save</button></form>";

        if (photoresistorFound) {
            html += "<br>";
            html += "<hr><strong>Current ADC Value:</strong> " + String(currentAdcAvg) + "<br>";
            html += "<strong>Current Brightness:</strong> " + String(currentBrightness) + " / 255<br>";
            html += "<strong>Light (for Threshold):</strong> " + String(currentLightPercent) + " %<br>";
            html += "<br>";
            html += "<form method='GET' action='/brightness'><button type='submit'>Refresh</button></form>";
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
        
        html += "<br><a href='/'>Back</a></body></html>";
        server.send(200, "text/html", html);
        });


    server.on("/save_brightness", HTTP_POST, []() {
        use_adc = server.hasArg("use_adc");
        lowThreshold = server.arg("lowThreshold").toInt();
        highThreshold = server.arg("highThreshold").toInt();
        
        maxBrightness = (uint8_t)server.arg("maxBrightness").toInt();
        minBrightness = (uint8_t)server.arg("minBrightness").toInt();

#if defined (GC9D01)  || defined(GC9A01_WITH_BACKLIGHT) 
        gammaBrightness = server.arg("gamma").toFloat();
        preferences.putFloat("gammaBrightness", gammaBrightness);
#endif

        preferences.putBool("use_adc", use_adc);
        preferences.putInt("lowThreshold", lowThreshold);
        preferences.putInt("highThreshold", highThreshold);
       
        preferences.putUChar("maxBrightness", maxBrightness);        
        preferences.putUChar("minBrightness", minBrightness);
         


        server.send(200, "text/html",
            "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='2; url=/brightness'><title>Saved</title></head>"
            "<body style='font-family:Arial;text-align:center;'><h2>Settings saved</h2><p>Returning...</p></body></html>");
        });



    server.on("/files", HTTP_GET, []() {
        String html = "<!DOCTYPE html><html><head><title>All Files</title><meta name='viewport' content='width=device-width, initial-scale=1'><style>body{font-family:Arial;text-align:center;}table{margin:auto;}th,td{padding:8px;}</style></head><body>";
        size_t total = LittleFS.totalBytes();
        size_t used = LittleFS.usedBytes();
        html += "Connected to: <strong>" + WiFi.SSID() + "</strong> (" + WiFi.localIP().toString() + ")" + "&nbsp;&nbsp;";
        html += "<br>Storage: " + String(used / 1024) + " KB / " + String(total / 1024) + " KB <hr>";

        html += "<h2>All Files on LittleFS</h2><table border='1'><tr><th align=left>Filename</th><th>Size (bytes)</th><th>Info</th><th>Action</th></tr>";

        File root = LittleFS.open("/");
        File file = root.openNextFile();
        while (file) {
            String info = getBmpInfo(file.name());
            String name = file.name();
            html += "<tr><td align=left>" + name + "</td><td align=right>" + String(file.size()) + "</td>";
            html += "<td align=right>" + String(info) + "</td>";
            html += " <td><a href = '/delete?file=" + name + "' onclick = 'return confirm(\"Delete " + name + "?\")'>Delete</a> ";
            html += "<a href = '/scalebmp_form?file=" + name + "'>Scale</a> ";
            html += "<a href='/rename_form?file=" + name + "'>Rename</a> ";
            html += "</td></tr>";

            file = root.openNextFile();
        }
        html += "</table><br><a href='/'>Back</a></body></html>";
        server.send(200, "text/html", html);
        });




    server.on("/status", HTTP_GET, []() {

        char version[32];
        sprintf(version, "%d-%02d-%02d %02d:%02d%:%02d", BUILD_YEAR, BUILD_MONTH, BUILD_DAY, BUILD_HOUR, BUILD_MIN, BUILD_SEC);

        String html = "<!DOCTYPE html><html><head><title>Status</title><meta name='viewport' content='width=device-width, initial-scale=1'><style>body{font-family:Arial;}table{margin:auto;}th,td{padding:8px;}</style></head><body>";

        size_t total = LittleFS.totalBytes();
        size_t used = LittleFS.usedBytes();
        html += "Connected to: <strong>" + WiFi.SSID() + "</strong> (" + WiFi.localIP().toString() + ")" + "&nbsp;&nbsp;<br>Storage: " + String(used / 1024) + " KB / " + String(total / 1024) + " KB<hr>";

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

        html += "<li>Hostname: " + String(hostname) + "</li>";
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

        html += "<li>PSRam size: " + String(ESP.getPsramSize() /1024) + " kB</li>";
        html += "<li>PSRam free: " + String(ESP.getFreePsram() / 1024) + " kB</li>";
        html += "<li>PSRam used: " + String(psramAvailable == true ? "true" : "false") + "</li><br>";
         
       
        html += "<li>LittleFS Size: " + String(LittleFS.totalBytes() / 1024) + " KB</li>";
        html += "<li>LittleFS Used: " + String(LittleFS.usedBytes() / 1024) + " KB</li>";   
        html += "<li>LittleFS Free: " + String((LittleFS.totalBytes() - LittleFS.usedBytes()) / 1024) + " KB</li><br>";   
              
        
        if (photoresistorFound) {
            html += "<li>Photoresistor found on GPIO " + String(ADC_PIN) + "</li>";
        } else { 
            html += "<li>Photoresistor not found on GPIO " + String(ADC_PIN) + "</li><br>";
        }
        html += "<li>Actual brightness (0-255): " + String(currentBrightness) + "</li><br>"; 


        html += "<li>TFT_SCLK: " + String(TFT_SCLK) + "</li>";
        //html += "<li>TFT_MISO: " + String(TFT_MISO) + "</li>";  
        html += "<li>TFT_MOSI: " + String(TFT_MOSI) + "</li>";  
        html += "<li>TFT_CS: " + String(TFT_CS) + "</li>";  
        html += "<li>TFT_DC: " + String(TFT_DC) + "</li>";  
        html += "<li>TFT_RST: " + String(TFT_RST) + "</li><br>";

        html += "<li>BUTTON: " + String(BUTTON1) + "</li>";           
        html += "<li>LED_BOARD: " + String(LED_BOARD) + "</li><br>";

        html += "<li>ADC_VCC: " + String(ADC_3V) + "</li>";
        html += "<li>ADC(photoresistor): " + String(ADC_PIN) + "</li>";        
        html += "<li>ADC_GND: " + String(ADC_GND) + "</li>";           


#ifndef TFT_Backlight 
        html += "<li>TFT_Backlight: none</li>";
#else
        html += "<li>TFT_Backlight: " + String(TFT_Backlight) + "</li>";  
#endif
        html += "<br>";

        
        html += "<h3>Actual Preferences</h3><ul>";
        html += "<li><b>ssid</b>: " + preferences.getString("ssid", "") + "</li>";
        html += "<li><b>ssid2</b>: " + preferences.getString("ssid2", "") + "</li>";
        html += "<li><b>ntpServer1</b>: " + preferences.getString("ntpServer1", "pool.ntp.org") + "</li>";
        html += "<li><b>ntpServer2</b>: " + preferences.getString("ntpServer2", "time.nist.gov") + "</li>";   
        html += "<li><b>timezone</b>: " + preferences.getString("timezone", "CET-1CEST,M3.5.0,M10.5.0/3") + "</li>";
        html += "<li><b>background</b>: " + preferences.getString("background", "/faces/default") + "</li>";
        html += "<li><b>handset</b>: " + preferences.getString("handset", "") + "</li>";
        html += "<li><b>centerColor (RGB565)</b>: " + String(preferences.getUInt("centerColor", TFT_RED), HEX) + "</li>";
        html += "<li><b>centerSize</b>: " + String(preferences.getUInt("centerSize", 6)) + "</li>";
        html += "<li><b>tft_rotation</b>: " + String(preferences.getUChar("tft_rotation", 0)) + "</li>";

        // Booleans als Text
        html += "<li><b>use_adc</b>: " + String(preferences.getBool("use_adc", true) ? "true" : "false") + "</li>";
        html += "<li><b>stationMode</b>: " + String(preferences.getBool("stationMode", true) ? "true" : "false") + "</li>";
        html += "<li><b>secondhand</b>: " + String(preferences.getBool("showSecondHand", true) ? "true" : "false") + "</li>";
        html += "<li><b>smoothMinute</b>: " + String(preferences.getBool("smoothMinute", false) ? "true" : "false") + "</li>";
        
        html += "<li><b>minBrightness</b>: " + String(preferences.getUChar("minBrightness", 100)) + "</li>";
        html += "<li><b>maxBrightness</b>: " + String(preferences.getUChar("maxBrightness", 255)) + "</li>";
        
        html += "<li><b>lowThreshold</b>: " + String(preferences.getInt("lowThreshold", 40)) + "</li>";
        html += "<li><b>highThreshold</b>: " + String(preferences.getInt("highThreshold", 60)) + "</li>";
        html += "</ul>";


        html += "</br>";




        html += "<li>Contact: holger.wagenlehner@gmx.de</li>";



        html += "</ul>";
            
        html += "<br><a href = '/'>Back</a></body></html>";
        server.send(200, "text/html", html);
        });

    server.on("/preview_defaultface", HTTP_GET, []() {
        const int headerSize = 54;
        const int rowSize = ((CLOCK_WIDTH * 2 + 3) / 4) * 4;
        const int dataSize = rowSize * CLOCK_HEIGHT;
        const int fileSize = headerSize + dataSize;

        uint8_t* bmpData = new uint8_t[fileSize];
        memset(bmpData, 0, fileSize);

        bmpData[0] = 'B'; bmpData[1] = 'M';
        *(uint32_t*)&bmpData[2] = fileSize;
        *(uint32_t*)&bmpData[10] = headerSize;
        *(uint32_t*)&bmpData[14] = 40;
        *(int32_t*)&bmpData[18] = CLOCK_WIDTH;
        *(int32_t*)&bmpData[22] = -CLOCK_HEIGHT;
        *(uint16_t*)&bmpData[26] = 1;
        *(uint16_t*)&bmpData[28] = 16;
        *(uint32_t*)&bmpData[34] = dataSize;

        for (int y = 0; y < CLOCK_WIDTH; y++) {
            uint8_t* rowPtr = bmpData + headerSize + y * rowSize;
            for (int x = 0; x < CLOCK_HEIGHT; x++) {
                uint16_t px = clockFace[y * CLOCK_WIDTH + x];
                if (px == TRANSPARENT_COLOR) px = 0xFFFF;
                ((uint16_t*)rowPtr)[x] = px;
            }
        }

        server.send_P(200, "image/bmp", (const char*)bmpData, fileSize);
        delete[] bmpData;
        });

    server.on("/listfilesFaces", HTTP_GET, []() {

        size_t total = LittleFS.totalBytes();
        size_t used = LittleFS.usedBytes();


        String html = "<!DOCTYPE html><html><head><title>Clock Face Files</title><meta name='viewport' content='width=device-width, initial-scale=1'><style>body{font-family:Arial;text-align:center;}table{margin:auto;}th,td{padding:8px;}</style></head>";


        html += "Connected to : <strong>" + WiFi.SSID() + " </strong > (" + WiFi.localIP().toString() + ")" + (WiFi.SSID() == wifi_ssid2 ? " (secondary)" : "") + " &nbsp;&nbsp; ";
        html += "<br>Storage used : " + String(used / 1024) + " KB / " + String(total / 1024) + " KB<hr>";

        html += "<h2>Manage Clock Face Files " + String(CLOCK_WIDTH) + " x " + String(CLOCK_HEIGHT) + "</h2><table border='1'><tr><th>Preview</th><th>Action</th></tr>";

        // Add built-in default face
        html += "<tr><td>default (built-in)<br>";
        html += "<img src='/preview_defaultface' style='width:80px;height:80px;border:1px solid #ccc'></td>";
        html += "<td><a href='/setbackground?file=face_default.bmp'>Set</a></td></tr>";

        File root = LittleFS.open("/");
        File file = root.openNextFile();


        bool anyFile = false;
        while (file) {
            String name = file.name();
#ifdef DEBUG
            Serial.println(name);
#endif
            if (!file.isDirectory() && name.startsWith("face_") && name.endsWith(".bmp")) {
                anyFile = true;
                String shortName = name;
                String info = getBmpInfo(name);
                html += "<tr><td>" + shortName + "<br><img src='/file?name=" + name + "' style='width:80px;height:80px;border:1px solid #ccc'><br><small>" + String(info) + "</small></td>";
                html += "<td><a href='/setbackground?file=" + shortName + "'>Set</a> | ";
                html += "<a href='/delete?file=" + shortName + "'>Delete</a></td></tr>";
            }
            file = root.openNextFile();
        }

        if (!anyFile) html += "<tr><td colspan='3'>No BMP files found in /</td></tr>";
        html += "</table><hr>";

        html += "<h3>Upload New Clock Face</h3>";
        html += "<small>Requirements: " + String(CLOCK_WIDTH) + " x " + String(CLOCK_HEIGHT) + " pixels, 16-bit BMP (RGB565), name must start with <code>face_</code></small><br><br>";
        html += "<form method = 'POST' action = '/upload' enctype = 'multipart/form-data' onsubmit = 'showProgress()'>";
        html += "<input type='file' name='upload' accept='.bmp' required><br>";
        
        html += "<button type='submit'>Upload BMP</button>";
        html += "<div id='progress' style='display:none;'>Uploading... please wait</div>";
        html += "<script>function showProgress(){document.getElementById('progress').style.display='block';}</script></form>";
        html += "<br><a href='/'>Back</a></body> </html>";
        server.send(200, "text/html", html);
        });

    server.on("/", HTTP_GET, []() {


        String html = "<!DOCTYPE html><html><head><title>Clock Setup</title><meta name='viewport' content='width=device-width, initial-scale=1'>";
        html += "<style>body{font-family:Arial;text-align:center;}input,select,button{margin:10px;padding:10px;width:80%;}</style></head><body>";
        size_t total = LittleFS.totalBytes();
        size_t used = LittleFS.usedBytes();
        html += "Connected to: <strong>" + WiFi.SSID() + "</strong> (" + WiFi.localIP().toString() + ")" + "&nbsp;&nbsp;";
        html += "<br>Storage: " + String(used / 1024) + " KB / " + String(total / 1024) + " KB <hr>";
        html += "<h2>Clock Setup</h2>";

        html += "<form action = '/save' method = 'POST'>";

        html += "<h3>Primary WiFi</h3>";
        html += "<input name='ssid' placeholder='SSID' value='" + wifi_ssid + "'><br>";


        html += "<input name='pass' id='pass' placeholder='Password' type='password' value=''><br>";
        if (WiFi.getMode() == WIFI_STA) {
            html += "<small>Password is hidden. Leave empty to keep current.</small>";
        }   
        html += "<br>";


        html += "<h3>Alternative WiFi</h3>";
        html += "<input name='ssid2' placeholder='SSID 2' value='" + wifi_ssid2 + "'><br>";
        html += "<input name='pass2' placeholder='Password 2' type='password' value=''><br>";
        if (WiFi.getMode() == WIFI_STA) {
            html += "<small>Password is hidden. Leave empty to keep current.</small>";
        }

        html += "<br>";


        html += "<button type='submit'>Save</button></form><hr>";

        if (WiFi.getMode() == WIFI_STA) {


            html += "<form action='/applydisplaysettings' method='POST'>";


            //html += "<a href='/timezone_form'><button>Set Timezone</button></a><br><br>";

            html += "<li><a href='/timezone_form'>Set Timezone</a></li>";



            html += "<table style='margin:auto;text-align:left;'><tr>";

            html += "<td><input type='checkbox' name='stationMode' value='1' ";
            html += preferences.getBool("stationMode", true) ? "checked" : "";
            html += "> Train Station Mode</td>";

            html += "<td><input type='checkbox' name='showSecondHand' value='1' ";
            html += preferences.getBool("showSecondHand", true) ? "checked" : "";
            html += "> Show Seconds</td>";

            html += "<td><input type='checkbox' name='smoothMinute' value='1' ";
            html += preferences.getBool("smoothMinute", true) ? "checked" : "";
            html += "> Smooth Minute Hand</td>";



            tft_rotation = preferences.getUChar("tft_rotation", 0);
            html += "<td>Rotation: <select name='rotation'>";
            for (int i = 0; i <= 3; i++) {
                html += "<option value='" + String(i) + "'";
                if (i == tft_rotation) html += " selected";
                html += ">" + String(i) + "</option>";
            }
            html += "</select></td>";



            html += "<td valign=bottom><button type='submit'>Apply</button></td>";
            html += "</tr></table></form>";


            html += "<hr>";


            html += "<a href='/listfilesFaces'><button>Manage Clock Face Files</button></a><br><br>";
            html += "<a href='/handsets'><button>Manage Hand Sets</button></a><br><br>";

            html += "<form action='/syncnow' method='POST'><button type='submit'>Sync Time Now</button></form><br>";
            html += "<form action='/brightness' method='POST'><button type='submit'>Brightness Settings</button></form><br>";
            
            html += "<a href='/files'>File Manager</a><br>"; 
        }

        html += "<a href='/status'>Systemstatus</a><br>";
        html += "<br><a href='/reboot'>Reboot</a><br>";
        html += "<br><a href='/factoryReset' onclick = \"return confirm('Really ?')\">Factory Reset</a><br>";
        html += "</body></html>";
        server.send(200, "text/html", html);
        });

    server.on("/save", HTTP_POST, []() {
        if (server.hasArg("ssid")) {
            if (server.arg("ssid") != "") preferences.putString("ssid", server.arg("ssid"));       
            if (server.arg("ssid2") != "") preferences.putString("ssid2", server.arg("ssid2"));       

            if (server.arg("pass") != "") preferences.putString("pass", server.arg("pass"));
            if (server.arg("pass2") != "") preferences.putString("pass2", server.arg("pass2"));

             
            //preferences.putBool("showSecondHand", server.hasArg("show"));
            //preferences.putBool("stationMode", server.hasArg("stationMode"));    

            if (WiFi.getMode() == WIFI_STA) {
                server.send(200, "text/html", "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='10; url=/'>"
                    "<title>Settings saved</title></head><body style='font-family:Arial;text-align:center;'>"
                    "<h2>Settings saved</h2><p>Return to the main page in 10 seconds or refresh the website when the ESP is online again.</p></body></html>");
            } else {
                server.send(200, "text/html", "<!DOCTYPE html><html><head>"
                    "<title>Settings saved</title></head><body style='font-family:Arial;text-align:center;'>"
                    "<h2>Settings saved</h2><p>Please connect to your home network and go to the ESP website at http:// IPADDRESS</p></body></html>");
            }
            esp_reboot();
        }
        });

    server.on("/upload", HTTP_GET, []() {
        server.send(200, "text/html", "<form method='POST' action='/upload' enctype='multipart/form-data' onsubmit='showProgress()'><input type='file' name='upload' accept='.bmp'><br><br><button type='submit'>Upload BMP</button><div id='progress' style='display:none;'>Uploading... please wait</div><script>function showProgress(){document.getElementById('progress').style.display='block';}</script></form><br><a href='/listfilesFaces'>Back to file list</a>");
        });

    server.on("/upload", HTTP_POST, []() {
        if (uploadSuccess) {
            server.sendHeader("Location", "/listfilesFaces", true);
            server.send(302, "text/plain", "");
        }
        else {
            String errorHtml = "<!DOCTYPE html><html><head><title>Upload Failed</title><meta name='viewport' content='width=device-width, initial-scale=1'></head><body style='font-family:Arial;text-align:center;'>";
            errorHtml += "<h2>Upload failed</h2>";
            errorHtml += "<p>Only .bmp files starting with <code>face_</code> or <code>hand_</code> are accepted.</p>";
            errorHtml += "<a href='/upload'>Try again</a></body></html>";
            server.send(400, "text/html", errorHtml);
        }
        }, handleFileUpload);

    server.on("/setbackground", HTTP_GET, []() {
        if (server.hasArg("file")) {
            String file = server.arg("file");
            file.replace("..", "");
            if (!file.startsWith("/")) file = "/" + file;

            if (file == "/face_default.bmp") {
                selectedBackground = file;
                preferences.putString("background", file);
                freeClockFaceBuffer();
                loadClockFace();
                loadHandSprites();
                server.sendHeader("Location", "/listfilesFaces", true);
                server.send(302, "text/plain", "");
                return;
            }

            if (LittleFS.exists(file)) {
                selectedBackground = file;
                preferences.putString("background", file);
                Serial.println("set bg to: " + file);
                freeClockFaceBuffer();
                loadClockFace();
                loadHandSprites();
                server.sendHeader("Location", "/listfilesFaces", true);
                server.send(302, "text/plain", "");
                return;
            }
        }
        server.send(404, "text/plain", "File not found");
        });


    server.on("/delete", HTTP_GET, []() {
        if (server.hasArg("file")) {
            String path = server.arg("file");
            path.replace("..", "");
            if (!path.startsWith("/")) path = "/" + path;
            if (LittleFS.exists(path)) {
                LittleFS.remove(path);
                server.sendHeader("Location", "/listfilesFaces", true);
                server.send(302, "text/plain", "");
            }
            else {
                server.send(404, "text/plain", "File not found");
            }
        }
        });

    server.on("/file", HTTP_GET, []() {
        if (server.hasArg("name")) {
            String path = server.arg("name");
            if (!path.startsWith("/")) path = "/" + path;
            if (LittleFS.exists(path)) {
                File f = LittleFS.open(path, "r");
                server.streamFile(f, "image/bmp");
                f.close();
                return;
            }
        }
        server.send(404, "text/plain", "File not found");
        });

    server.on("/handsets", HTTP_GET, []() {
        String html = "<!DOCTYPE html><html><head><title>Clock Hand Set Files</title><meta name='viewport' content='width=device-width, initial-scale=1'><style>body{font-family:Arial;text-align:center;}table{margin:auto;}th,td{padding:10px;}img{height:50px;}</style></head><body>";
        size_t total = LittleFS.totalBytes();
        size_t used = LittleFS.usedBytes();
        html += "Connected to: <strong>" + WiFi.SSID() + "</strong> (" + WiFi.localIP().toString() + ")" + "&nbsp;&nbsp;<br>Storage: " + String(used / 1024) + " KB / " + String(total / 1024) + " KB<hr>";
        html += "<h2>Manage Clock Hand Sets " + String(HAND_WIDTH) + " x " + String(HAND_HEIGHT) + "</h2><table border = '1'><tr><th>Set</th><th>Preview</th><th>Action</th></tr>";

        String activeSet = preferences.getString("handset", "");
        std::set<String> foundSets;

        File root = LittleFS.open("/");
        File file = root.openNextFile();
        while (file) {
            String name = file.name();
#ifdef DEBUG
            Serial.println(name);
#endif
            if (!file.isDirectory() && name.startsWith("hand_set") && name.endsWith(".bmp")) {
                int start = 8;
                int end = name.indexOf('_', start);
                if (end > start) {
                    String set = name.substring(start, end);
#ifdef DEBUG
                    Serial.println("[HANDS] Found set: " + set);
#endif
                    foundSets.insert(set);
                }
            }
            file = root.openNextFile();
        }

        if (foundSets.empty()) {
            html += "<tr><td colspan='3'>No hand sets found.</td></tr>";
        }


        String handHourBase64 = encodeBmpToBase64(handHour, HAND_WIDTH, HAND_HEIGHT);
        String handMinuteBase64 = encodeBmpToBase64(handMinute, HAND_WIDTH, HAND_HEIGHT);
        String handSecondBase64 = encodeBmpToBase64(handSecond, HAND_WIDTH, HAND_HEIGHT);
        


        // Always show default as built-in
        html += "<tr><td>default (built-in)</td><td>";
        html += "<img src='data:image/bmp;charset=utf-8;base64, " + handHourBase64 + "'> ";
        html += "<img src='data:image/bmp;charset=utf-8;base64, " + handMinuteBase64 + "'> ";
        html += "<img src='data:image/bmp;charset=utf-8;base64, " + handSecondBase64 + "'> ";
        html += "</td>";
        //html += "<td> </td>";
        html += "<td><a href = '/sethandset?set=default'>Set</a></td> ";
        html += "</tr>";

        
        for (const String& set : foundSets) {
            html += "<tr><td>" + set + (set == activeSet ? " (active)" : "") + "</td><td>";
            String hourPath = "/hand_set" + set + "_hour.bmp";
            String minutePath = "/hand_set" + set + "_minute.bmp";
            String secondPath = "/hand_set" + set + "_second.bmp";
            html += LittleFS.exists(hourPath) ? "<img src='/file?name=" + hourPath + "'> " : "<img src='data:image/bmp;charset=utf-8;base64, " + handHourBase64 + "'> ";
            html += LittleFS.exists(minutePath) ? "<img src='/file?name=" + minutePath + "'> " : "<img src='data:image/bmp;charset=utf-8;base64, " + handMinuteBase64 + "'> ";
           
             
            html += LittleFS.exists(secondPath) ? "<img src='/file?name=" + secondPath + "'> " : "<img src='data:image/bmp;charset=utf-8;base64," + handSecondBase64 + "'> ";
                

            html += "</td>";
         //   html += "<td> </td>";
            html += "<td><a href = '/sethandset?set=" + set + "'>Set</a> | <a href = '/deletehandset?set=" + set + "' onclick = \"return confirm('Delete this hand set?')\">Delete</a></td>";
            html +="</tr>";
        }
        
        html += "</table><hr>";
               
        uint8_t centerSize = preferences.getUInt("centerSize", 6);
        uint32_t centerColor = preferences.getUInt("centerColor", 0x000000);

        html += "<h2>Centre point</h2><form action='/setcenter' method='POST'>";
        html += "<label>Size (Pixel):</label><br><input name='size' type='number' min='0' max='50' value='" + String(centerSize) + "'><br>";
        html += "<label>Color (RGB hex, e.g. FF0000 = Red, 000000 = Black):</label><br><input name='color' value='" + String((centerColor >> 11 & 0x1F) * 255 / 31 << 16 | (centerColor >> 5 & 0x3F) * 255 / 63 << 8 | (centerColor & 0x1F) * 255 / 31, HEX) + "'><br>";
        html += "<button type='submit'>Apply</button></form><hr>";

        html += "<h3>Upload New Hand Set</h3>";
        html += "<small>Requirements: " + String(HAND_WIDTH) + " x " + String(HAND_HEIGHT) + " pixels, 16-bit BMP (RGB565), <br>name must start with <code>hand_set + no + _hour, _minute or _second .bmp e.g. hand_set1_second.bmp</code><br>Pivot point:" + String(int(HAND_WIDTH / 2)) + " / " + String(int(HAND_HEIGHT * 0.77)) + "<br><br>";
        html += "<form method='POST' action='/uploadhandset' enctype='multipart/form-data'>";
        
        html += "File: <input type='file' name='upload' accept='.bmp' required><br><br>";
        html += "<button type='submit'>Upload to Set</button></form><br>";
        html += "<a href='/'>Back</a><br>";
        html += "<a href='/status'>Systemstatus</a><br>";
        html += "<a href='/files'>File Manager</a><br>";
        html += "</body></html>";
        //Serial.println(html);
        server.send(200, "text/html", html);
        });

    server.on("/setcenter", HTTP_POST, []() {
        if (server.hasArg("size") && server.hasArg("color")) {
            uint8_t size = server.arg("size").toInt();
            uint32_t rgb = (uint32_t)strtoul(server.arg("color").c_str(), nullptr, 16);

            // Convert 24-bit RGB888 to RGB565
            uint8_t r = (rgb >> 16) & 0xFF;
            uint8_t g = (rgb >> 8) & 0xFF;
            uint8_t b = rgb & 0xFF;
            uint16_t rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);

            preferences.putUInt("centerSize", size);
            preferences.putUInt("centerColor", rgb565);
        }
        server.send(200, "text/html",
            "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='3; url=/handsets'>"
            "<title>Updated</title></head><body style='font-family:Arial;text-align:center;'>"
            "<h2>Centre point updated</h2><p>back to handsets...</p></body></html>");
        });


    server.on("/uploadhandset", HTTP_POST, []() {
        if (uploadSuccess) {
            // Sicherheitsprüfung auf Dateinamenmuster
            if (!uploadFilePath.endsWith(".bmp") || !uploadFilePath.startsWith("/hand_set")) {
                String errorHtml = "<!DOCTYPE html><html><head><title>Upload Failed</title><meta name='viewport' content='width=device-width, initial-scale=1'></head><body style='font-family:Arial;text-align:center;'>";
                errorHtml += "<h2>Upload failed</h2>";
                errorHtml += "<p>Only .bmp files starting with <code>hand_</code> are accepted for handset upload.</p>";
                errorHtml += "<a href='/handsets'>Try again</a></body></html>";
                server.send(400, "text/html", errorHtml);
                return;
            }
            String set = server.arg("set");
          //  String target = server.arg("target");
            String dir = "/";
            if (!LittleFS.exists(dir)) LittleFS.mkdir(dir);
          //  String finalPath = "/hand_set" + set + "_" + target + ".bmp";
          //  LittleFS.rename(uploadFilePath, finalPath);
          //  Serial.println("[UPLOAD] Hand uploaded to: " + finalPath);
            Serial.println("[UPLOAD] Hand uploaded to: " + uploadFilePath);
            server.sendHeader("Location", "/handsets", true);
            server.send(302, "text/plain", "");
        }
        else {
            server.send(500, "text/html", " Upload failed!<br><a href='/handsets'>Try again</a>");
        }
        }, handleFileUpload);

    server.on("/sethandset", HTTP_GET, []() {
        if (server.hasArg("set")) {
            String chosen = server.arg("set");
            preferences.putString("handset", chosen);
            Serial.println("[HANDSET] Set to: " + chosen);
            freeClockFaceBuffer();
            loadClockFace();
            loadHandSprites();
            updateClock();
            server.sendHeader("Location", "/handsets", true);
            server.send(302, "text/plain", "");
        }
        else {
            server.send(400, "text/plain", "Missing set name");
        }
        });

    server.on("/deletehandset", HTTP_GET, []() {
        if (server.hasArg("set")) {
            String set = server.arg("set");
            String targets[] = { "hour", "minute", "second" };
            for (const String& target : targets) {
                String path = "/hand_set" + set + "_" + target + ".bmp";
                Serial.println("[DELETE] Looking for: " + path);
                if (LittleFS.exists(path)) {
                    LittleFS.remove(path);
                    Serial.println("[DELETE] Removed: " + path);
                }
            }
            server.sendHeader("Location", "/handsets", true);
            server.send(302, "text/plain", "");
        }
        else {
            server.send(400, "text/plain", "Missing set name");
        }
        });

    server.on("/reboot", HTTP_GET, []() {
        server.send(200, "text/html", "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='10; url=/'><title>Rebooting</title></head><body style='font-family:Arial;text-align:center;'><h2>Rebooting...</h2><p>Return to the main page in 10 seconds or refresh the website when the ESP is online again.</p></body></html>");        
        esp_reboot();
        }); 

    server.on("/factoryReset", HTTP_GET, []() {
          factoryReset();
        });



    server.on("/syncnow", HTTP_POST, []() {
        setupNTP();
        struct tm timeinfo;
        getLocalTime(&timeinfo);

        char timeStr[32];
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);

        String html = "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='5; url=/'>"
            "<title>Time Synced</title></head><body style='font-family:Arial;text-align:center;'>"
            "<h2>Time synced</h2><p>" + String(timeStr) + "</p><p>Returning to main page in 5 seconds.</p></body></html>";

        server.send(200, "text/html", html);
        });

}

void handleFileUpload() {
    HTTPUpload& upload = server.upload();

    if (upload.status == UPLOAD_FILE_START) {
        uploadFilePath = upload.filename;
        uploadFilePath.replace("..", "");
        if (!uploadFilePath.startsWith("/")) uploadFilePath = "/" + uploadFilePath;

        // Nur bestimmte Dateinamenmuster zulassen
        if (!uploadFilePath.endsWith(".bmp") ||
            !(uploadFilePath.startsWith("/face_") || uploadFilePath.startsWith("/hand_set"))) {
            Serial.println("[UPLOAD] Invalid filename: must start with 'face_' or 'hand_set' and end with '.bmp'");
            uploadSuccess = false;
            return;
        }

        uploadFilePath.replace("..", "");
        if (!uploadFilePath.startsWith("/")) uploadFilePath = "/" + uploadFilePath;

        Serial.println("[UPLOAD] Start: " + uploadFilePath);
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
                Serial.println("[UPLOAD] Finished OK: " + uploadFilePath);
                String ext = uploadFilePath;
                ext.toLowerCase();
                if (ext.endsWith(".bmp")) {
                    bool isHand = uploadFilePath.indexOf("hour") > 0 || uploadFilePath.indexOf("minute") > 0 || uploadFilePath.indexOf("second") > 0;
                    if (uploadFilePath.startsWith("/face_")) {
                        Serial.println("[UPLOAD] Detected Clock Face upload");
                    }
                    else if (uploadFilePath.startsWith("/hand_set")) {
                        Serial.println("[UPLOAD] Detected Clock Hand upload");
                    }                    
                }
            }
            else {
                Serial.println("[UPLOAD] Finished but file missing!");
                uploadSuccess = false;
            }
        }
        else {
            Serial.println("[UPLOAD] Failed during writing");
        }
    }
}


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

        for (int x = 0; x < CLOCK_HEIGHT; x++) {
            rowBuffer[x] = setPixelBrightness(rowBuffer[x]);
        }

        sprite->pushImage(0, row, CLOCK_WIDTH, 1, rowBuffer);
    }

    bmp.close();
    return true;
}




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
        Serial.println("PSRAM konnte nicht allokiert werden für Bitmap-Daten!");
        bmp.close();
        return false;
    }

    for (int y = 0; y < height; y++) {
        bmp.read((uint8_t*)&tempBuffer[y * width], width * 2);
    }

    // Neuen Buffer erstellen, der rotiert ist
    uint16_t* rotatedBuffer = (uint16_t*)ps_malloc(CLOCK_WIDTH * CLOCK_HEIGHT * sizeof(uint16_t));
    if (!rotatedBuffer) {
        Serial.println("PSRAM konnte nicht allokiert werden für Rotation!");
        free(tempBuffer);
        bmp.close();
        return false;
    }

    tft_rotation = preferences.getUChar("tft_rotation", 0);

    // 4. Pixelrotation (Korrektur der Spiegelung und Drehung)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int newX = x, newY = y;

            switch (tft_rotation) {
            case 0: // 0°
                newX = x;
                newY = y;
                break;
            case 1: // 90°
                newX = y;
                newY = width - x - 1;
                break;
            case 2: // 180°
                newX = width - x - 1;
                newY = height - y - 1;
                break;
            case 3: // 270°
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

bool checkBmpFormat(const String& filename, int expectedWidth = CLOCK_WIDTH, int expectedHeight = CLOCK_HEIGHT) {
    File bmpFile = LittleFS.open(filename, "r");
    if (!bmpFile) {
        Serial.println("[BMP Check] Failed to open file");
        return false;
    }

    uint8_t header[54];
    if (bmpFile.read(header, 54) != 54) {
        Serial.println("[BMP Check] Failed to read header");
        bmpFile.close();
        return false;
    }

    if (header[0] != 'B' || header[1] != 'M') {
        Serial.println("[BMP Check] Not a BMP file");
        bmpFile.close();
        return false;
    }

    int32_t width = *(int32_t*)&header[18];
    int32_t height = *(int32_t*)&header[22];
    uint16_t bpp = *(uint16_t*)&header[28];

    bmpFile.close();

    if (width != expectedWidth || abs(height) != expectedHeight || bpp != 16) {
        Serial.printf("[BMP Check] Invalid BMP dimensions or format: %d x %d, %d bpp", width, height, bpp);
        return false;
    }

    Serial.println("[BMP Check] BMP format valid");
    return true;
}

String getBmpInfo(const String& filename) {

    String file;
    if (filename.startsWith("/")) file = filename;
    else file = "/" + filename; if (!filename.startsWith("/")) file = "/" + filename;

    //Serial.println("[BMP Info] Checking file: " + file);
    File bmp = LittleFS.open(file, "r");
    if (!bmp) return "n/a";

    uint8_t header[54];
    if (bmp.read(header, 54) != 54 || header[0] != 'B' || header[1] != 'M') {
        bmp.close();
        return "n/a";
    }

    int32_t width = *(int32_t*)&header[18];
    int32_t height = *(int32_t*)&header[22];
    uint16_t bpp = *(uint16_t*)&header[28];
    bmp.close();

    return String(abs(width)) + " x " + String(abs(height)) + " / " + String(bpp) + " bpp";
}

bool scaleAndSaveBmp(const char* sourcePath, const char* targetPath, int outW, int outH) {

    Serial.println("[BMP Scale] Scaling BMP: " + String(sourcePath) + " to " + String(targetPath));
    File bmp = LittleFS.open(sourcePath, "r");
    if (!bmp) {        
        Serial.println("[BMP Scale] Failed to open source file");
    }
    uint8_t header[54];
    if (bmp.read(header, 54) != 54 || header[0] != 'B' || header[1] != 'M') {
        bmp.close();
        Serial.println("[BMP Scale] Invalid BMP header");
        return false;
    }

    int32_t inW = *(int32_t*)&header[18];
    int32_t inH = *(int32_t*)&header[22];
    uint16_t bpp = *(uint16_t*)&header[28];
    uint32_t offset = *(uint32_t*)&header[10];

    if (bpp != 16 || inW <= 0 || abs(inH) <= 0) {
        bmp.close();
        Serial.println("[BMP Scale] Invalid BMP format or dimensions");
        return false;
    }

    bool flip = inH > 0;
    inH = abs(inH);
    float scaleX = (float)inW / outW;
    float scaleY = (float)inH / outH;

    int inRowSize = ((inW * 2 + 3) / 4) * 4;
    uint8_t* rowBuf = (uint8_t*)malloc(inRowSize);
    if (!rowBuf) {
        bmp.close();
        Serial.println("[BMP Scale] Memory allocation failed");
        return false;
    }

    uint16_t* outImage = new uint16_t[outW * outH];

    for (int y = 0; y < outH; y++) {
        int srcY = flip ? (inH - 1 - int(y * scaleY)) : int(y * scaleY);
        bmp.seek(offset + inRowSize * srcY);
        bmp.read(rowBuf, inRowSize);
        uint16_t* row16 = (uint16_t*)rowBuf;

        for (int x = 0; x < outW; x++) {
            int srcX = int(x * scaleX);
            outImage[y * outW + x] = row16[srcX];
        }
    }

    bmp.close();
    free(rowBuf);

    // Header schreiben
    File out = LittleFS.open(targetPath, "w");
    if (!out) {
        delete[] outImage;
        Serial.println("[BMP Scale] Failed to open target file");
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

void eraseWiFiConfig() {
    // WLAN trennen und komplett deaktivieren
    WiFi.disconnect(true, true);  // true,true => auch gespeicherte Daten löschen
    WiFi.mode(WIFI_OFF);
    delay(1000);

    // Zusätzlich: Manuell NVS-Einträge für WiFi löschen
    nvs_handle_t nvsHandle;
    esp_err_t err = nvs_open("wifi", NVS_READWRITE, &nvsHandle);
    if (err == ESP_OK) {
        nvs_erase_all(nvsHandle);
        nvs_commit(nvsHandle);
        nvs_close(nvsHandle);
        Serial.println("WiFi-NVS-Einträge gelöscht.");
    }
    else {
        Serial.printf("Fehler beim Öffnen des NVS-WiFi-Handles: %s\n", esp_err_to_name(err));
    }
}

void eraseAllNVS() {
    // Löscht gesamten NVS-Speicher
    esp_err_t result = nvs_flash_erase();
    if (result == ESP_OK) {
        Serial.println("Kompletter NVS-Speicher gelöscht (inkl. WiFi, Preferences).");
        nvs_flash_init();  // Wichtig: Danach wieder initialisieren!
    }
    else {
        Serial.printf("NVS-Erase fehlgeschlagen: %s\n", esp_err_to_name(result));
    }
}

void factoryReset() {
    tft.fillScreen(TFT_BLACK);
    preferences.begin("clock", false);
    preferences.putInt("firstStart", 0);
    preferences.end();
    delay(100);
    Serial.println(">>> Factory Reset gestartet...");
    eraseWiFiConfig();
    eraseAllNVS();
    delay(2000);
    Serial.println(">>> Neustart...");
         
}

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
            Serial.printf("→ Wöchentlicher Reboot in Woche %d\n", currentWeek);
            preferences.putInt("last_reset_week", currentWeek);
            preferences.end();
            delay(1000);
            ESP.restart();
        }
    }
}

void scaleAllFacesTo80x80() {
    File root = LittleFS.open("/");
    File file = root.openNextFile();
    while (file) {
        String name = file.name();
        if (!file.isDirectory() && name.startsWith("face_") && name.endsWith(".bmp")) {
            String target = "/face80_" + name.substring(1); // z.B. /face80_bigben.bmp
            Serial.printf("[SCALE] %s -> %s\n", name.c_str(), target.c_str());
            bool ok = scaleAndSaveBmp(name.c_str(), target.c_str(), 80, 80);
            if (ok) {
                Serial.println("  -> OK");
            }
            else {
                Serial.println("  -> Fehler beim Skalieren");
            }
        }
        file = root.openNextFile();
    }
}