#pragma once
// ####################################################################
// ### Board-/Display-Konfiguration, Pin-Belegung, Timing-Makros ######
// ####################################################################
// Sortiert nach Modul/Verwendung: System/Debug, Zeit/Timing, Board-Auswahl
// & Pin-Belegung, Display-Dimensionen.
//
// Hinweis: TFT_eSPI tft, WebServer webserver, Preferences preferences,
// DNSServer dnsServer, WiFiUDP udp, RTC_DS3231 rtc sowie die
// DCF77-Variablen (dcf77Flank, dcf, dcf77Count, dcfTimeFound) werden
// AUSSCHLIESSLICH in globals.h definiert (nicht hier duplizieren - das
// war die Ursache eines fruehen "redefinition"-Fehlers).

// --- System / Debug ---
#define DEBUG_PRINT(x)    { if (loggingEnabled) { Serial.print(x);   logToFile(String(x));}}
#define DEBUG_PRINTLN(x)  { if (loggingEnabled) { Serial.println(x); logToFile(String(x));}}
#define DEBUG_PRINTF(...) { if (loggingEnabled) { char buffer[128]; snprintf(buffer, sizeof(buffer), __VA_ARGS__); Serial.print(buffer); logToFile(String(buffer));}}

// Schwellwert fuer Heap-Warnungen (siehe checkHeapWarning() in system_utils.h):
// faellt der freie Heap an einer der ueberwachten Stellen darunter, wird eine
// Log-Zeile mit Kontext geschrieben, damit sich knapper Speicher einer
// konkreten Codestelle zuordnen laesst statt nur ueber /status im Nachhinein
// zu erfahren, DASS es irgendwann knapp war.
#define HEAP_WARNING_THRESHOLD 20480 // 20 KB

// --- Zeit / NTP-Standardwerte & Timing-Makros ---
// time server & timezone default
#define NTP_SERVER_1 "pool.ntp.org"
#define NTP_SERVER_2 "ptbtime1.ptb.de"
#define TIMEZONE_DEFAULT "CET-1CEST,M3.5.0,M10.5.0/3" // Central European Time

#define DEFAULT_PING_SERVER "1.1.1.1:80"

#define WAIT_1s 1000 // 1 Sekunde in Millisekunden
#define WAIT_3s 3000 // 3 Sekunden in Millisekunden
#define WAIT_5s 5000 // 5 Sekunden in Millisekunden
#define WAIT_10s 10000 // 10 Sekunden in Millisekunden
#define WAIT_15s 15000 // 15 Sekunden in Millisekunden
#define WAIT_30s 30000 // 30 Sekunden in Millisekunden
#define WAIT_1m 60000 // 1 Minute in Millisekunden
#define WAIT_5m 300000 // 5 Minuten in Millisekunden 
#define WAIT_10m 600000 // 10 Minuten in Millisekunden
#define WAIT_30m 1800000 // 30 Minuten in Millisekunden
#define WAIT_1h 3600000 // 1 Stunde in Millisekunden
#define WAIT_6h 21600000 // 6 Stunden in Millisekunden  

// --- Board-Auswahl (Prozessor, TFT-Typ) ---
// Prozessor
#define ESP32_S2  //only ESP32-S2 supported
//#define ESP32_S3 

// select TFT
#define GC9A01
//#define GC9A01_WITH_BACKLIGHT
//#define GC9D01
//#define ILI9341 

// --- Pin-Belegung: ESP32-S2 (Lolin S2 Pico) ---
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
#define BOOT_BUTTON 0

// Touch 
// #define TOUCH_PIN 9

// I2C / RTC
#define SDA_PIN 39
#define SCL_PIN 37

// SPI Chipselect für 2. identisches Display
//#define CS_2    18

// DCF77
#define DCF77_INTERRUPT 0 
#define DCF77_DATAPIN 35 

#if defined (GC9D01)  || defined(GC9A01_WITH_BACKLIGHT)  
#define TFT_Backlight 3  // Backlight
#define BACKLIGHT_CHANNEL 0  // PWM channel
#define BACKLIGHT_FREQ 5000
#define BACKLIGHT_RESOLUTION 8 
#endif
    
#endif

// --- Pin-Belegung: ESP32-S3 (Lolin S3) ---
#ifdef ESP32_S3  // Lolin S3
// ##############################################################################
// wires
//               ESP32 PIN    TFT
//               3.3V         vcc     3v3             red
//               GND          gnd     ground          blue
//  see C:\Users\hwage\Documents\Arduino\libraries\TFT_eSPI\user_setups\Setup304__ESP32S3_GC9D01.h
// or   https://github.com/holgiw/TFT-Clock-GC9A01/blob/master/PCB/ESP32-S2%20GC9A01.jpg


// #define LED_BOARD 15 // BUILTIN LED

//#define ADC_3V 1
//#define ADC_PIN 2
//#define ADC_GND 4

//#define BUTTON1 16

// Touch 
//#define TOUCH_PIN 9


#endif

// --- Display: Dimensionen je Zifferblatt-Typ ---
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

// Transparent in R5G6B5 RGB(16)
#define TRANSPARENT_COLOR 0x0120    
