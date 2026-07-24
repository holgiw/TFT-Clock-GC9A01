#pragma once
    // ### Board-/Display-Konfiguration, Pin-Belegung, Timing-Makros ######
    // Sortiert nach Modul: System/Debug, Zeit/Timing, Board-Auswahl & Pins, Display-Dimensionen.
    // Hinweis: tft/webserver/preferences/dnsServer/udp/rtc/DCF77-Variablen liegen AUSSCHLIESSLICH in globals.h (nicht duplizieren - fruehere "redefinition"-Ursache).

    // --- System / Debug ---
#define DEBUG_PRINT(x)    { if (loggingEnabled) { Serial.print(x);   logToFile(String(x));}}
#define DEBUG_PRINTLN(x)  { if (loggingEnabled) { Serial.println(x); logToFile(String(x));}}
#define DEBUG_PRINTF(...) { if (loggingEnabled) { char buffer[128]; snprintf(buffer, sizeof(buffer), __VA_ARGS__); Serial.print(buffer); logToFile(String(buffer));}}

    // Schwellwert fuer Heap-Warnungen (siehe checkHeapWarning() in system_utils.h):
    // faellt der freie Heap darunter, wird eine Log-Zeile mit Kontext geschrieben,
    // statt es erst spaeter ueber /status zu bemerken.
#define HEAP_WARNING_THRESHOLD 20480 // 20 KB

    // --- GitHub-Repository (Projektseite, Zusatz-Zifferblaetter/Zeigersaetze) ---
    // Zentral an einer Stelle, damit bei einem Fork/Umzug des Repos nur hier
    // geaendert werden muss statt an mehreren Stellen in webserver_routes.h.
#define GITHUB_REPO_OWNER "holgiw"
#define GITHUB_REPO_NAME "TFT-Clock-GC9A01"
#define GITHUB_REPO_URL "https://github.com/" GITHUB_REPO_OWNER "/" GITHUB_REPO_NAME
#define GITHUB_API_CONTENTS_BASE "https://api.github.com/repos/" GITHUB_REPO_OWNER "/" GITHUB_REPO_NAME "/contents/graphic/"
#define GITHUB_ZIP_BASE "https://github.com/" GITHUB_REPO_OWNER "/" GITHUB_REPO_NAME "/blob/master/graphic/"
#define GITHUB_RAW_BASE "https://raw.githubusercontent.com/" GITHUB_REPO_OWNER "/" GITHUB_REPO_NAME "/master/"

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

    // DCF77 uebernimmt die Systemzeit nur, wenn NTP seit mindestens dieser Zeit
    // nicht mehr synchronisiert hat (bzw. kein WLAN verbunden ist) - verhindert,
    // dass ein kurzer NTP-Ausfall sofort durch DCF77 "ueberschrieben" wird.
#define DCF77_NTP_GRACE_PERIOD (2 * WAIT_1h)

    // Geschwindigkeit des Sekundenzeigers im Train-Station-Mode gegenueber der
    // realen Sekunde (in Millisekunden) - der Zeiger "eilt" etwas voraus und
    // verweilt dann kurz auf der 60, wie bei einer klassischen Bahnhofsuhr.
#define FAST_SECOND 972.0f

    // --- Board-Auswahl (Prozessor, TFT-Typ) ---
    // Prozessor
#define ESP32_S2  //only ESP32-S2 supported

    // select TFT
#define GC9A01
    //#define GC9A01_WITH_BACKLIGHT
    //#define GC9D01
    //#define ILI9341 // DEPRECATED - nicht mehr aktiv gepflegt, GC9A01 wird bevorzugt

    // --- Pin-Belegung: ESP32-S2 (Lolin S2 Pico) ---
#ifdef ESP32_S2  // Lolin S2 Pico
    // Pinbelegung ESP32<->TFT: 3.3V->vcc (rot), GND->gnd (blau), weitere siehe Referenzschaltplan.
    // PCB-Referenz: https://github.com/holgiw/TFT-Clock-GC9A01/blob/master/PCB/ESP32-S2%20GC9A01.jpg


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

    // --- Display: Dimensionen je Zifferblatt-Typ ---
#if defined GC9A01 || defined(GC9A01_WITH_BACKLIGHT) 
#include "graphic/240/clock_default.h"

#define ROUND_DISPLAY // Rundes Display - Kreismaskierung der Zifferblatt-Ecken anwenden (siehe scaleAndSaveBmp() in display.h)
#define TFT_WIDTH 240
#define TFT_HEIGHT 240

#define CLOCK_WIDTH 240   
#define CLOCK_HEIGHT 240

#define HAND_WIDTH 21
#define HAND_HEIGHT 131

#define TFT_TEXT_SIZE 2

    // --- TFT_eSPI-Referenzkonfiguration (Arduino IDE!) ---
    // WICHTIG: Diese #defines wirken NICHT automatisch auf die TFT_eSPI-
    // Bibliothek selbst - sie kompiliert ihre eigenen .cpp-Dateien separat
    // vom Sketch, ein #define hier erreicht sie nicht. Diese Werte dienen als
    // dokumentierte Referenz, MUESSEN aber zusaetzlich EINMALIG in die
    // Bibliothek uebernommen werden, z.B. indem in
    // <Arduino-Bibliotheksordner>/TFT_eSPI/User_Setup_Select.h die Zeile
    //   #include <User_Setup.h>
    // durch einen Redirect auf diesen Block ersetzt wird (z.B. per absolutem
    // Pfad auf eine ausgelagerte Kopie dieser Zeilen). Bei einem
    // Bibliotheks-Update/-Neuinstallation muss dieser EINE Redirect erneut
    // gesetzt werden, aber die eigentlichen Werte bleiben hier im Projekt.
#define GC9A01_DRIVER
#define TFT_MOSI  6
#define TFT_SCLK  4
#define TFT_CS    9   // Chip-Select
#define TFT_DC    10  // Data/Command
#define TFT_RST   0   // Reset
#define TFT_BL    5   // Hintergrundbeleuchtung (Bibliotheks-eigene Steuerung -
                      // das Projekt nutzt zusaetzlich Pin 3 fuer eigene PWM-
                      // Helligkeitsregelung, siehe TFT_Backlight weiter unten)
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF
#define SMOOTH_FONT
#define SPI_FREQUENCY       27000000
#define SPI_READ_FREQUENCY  20000000
#endif

#ifdef GC9D01
#include "graphic/160/clock_default.h"

#define ROUND_DISPLAY // Rundes Display - Kreismaskierung der Zifferblatt-Ecken anwenden (siehe scaleAndSaveBmp() in display.h)
#define TFT_WIDTH 160
#define TFT_HEIGHT 160

#define CLOCK_WIDTH 160
#define CLOCK_HEIGHT 160

#define HAND_WIDTH 13
#define HAND_HEIGHT 86

#define TFT_TEXT_SIZE 1

    // ACHTUNG: Fuer GC9D01 fehlen hier noch die TFT_eSPI-Pin-/Treiber-
    // Einstellungen (GC9D01_DRIVER, TFT_MOSI/SCLK/CS/DC/RST/BL etc.) - anders
    // als beim GC9A01-Block oben, da keine bestaetigte Pinbelegung fuer dieses
    // Board vorliegt. Bei Nutzung dieses Boards muessen diese analog zum
    // GC9A01-Block oben ergaenzt werden, sonst schlaegt die Kompilierung fehl.
#endif

#ifdef ILI9341 // DEPRECATED - nicht mehr aktiv gepflegt
#include "graphic/240/clock_default.h"

#define TFT_WIDTH 240
#define TFT_HEIGHT 320

#define CLOCK_WIDTH 240
#define CLOCK_HEIGHT 240

#define HAND_WIDTH 21
#define HAND_HEIGHT 131

#define TFT_TEXT_SIZE 2

    // ACHTUNG: Wie beim GC9D01-Block fehlen hier ebenfalls die TFT_eSPI-Pin-/
    // Treiber-Einstellungen - bei Nutzung dieses (ohnehin deprecateten) Boards
    // analog zum GC9A01-Block oben ergaenzen.
#endif

    // Transparent in R5G6B5 RGB(16)
#define TRANSPARENT_COLOR 0x0120    
