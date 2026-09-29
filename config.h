#pragma once

    // Reihenfolge: zuerst Prozessor-/Display-Auswahl (Pins, Display-Masse),
    // danach die restlichen Werte nach Modul sortiert.
    // tft/webserver/preferences/dnsServer/udp/rtc/DCF77-Variablen: nur in globals.h.
    // Board-Auswahl (Prozessor, TFT-Typ)
    // Board selection (processor, TFT type)

    // Order: processor/display selection (pins, display dimensions) first,
    // then the rest sorted by module.
    // tft/webserver/preferences/dnsServer/udp/rtc/DCF77 variables: only in globals.h.
    // Prozessor
    // Processor

#define ESP32_S2  //nur ESP32-S2 unterstuetzt
                  // only ESP32-S2 supported

    // Displaytyp: KEIN #define mehr - GC9A01 (240x240) oder GC9D01 (160x160)
    // ist eine Einstellung (PK_DISPLAY_TYPE, Zifferblatt-Tab "Display-Typ", wirkt
    // nach Neustart). Eine Firmware fuer alle Varianten; die Masse je Typ
    // stehen in DISPLAY_GEOMETRY weiter unten. DISPLAY_TYPE_DEFAULT legt nur
    // die Werkseinstellung fest. Hintergrundbeleuchtung: ebenfalls eine
    // Einstellung (useBacklight, Helligkeits-Tab).

    // Display type: NO #define anymore - GC9A01 (240x240) or GC9D01 (160x160)
    // is a setting (PK_DISPLAY_TYPE, clock face tab "display type", takes effect
    // after a restart). One firmware for all variants; the dimensions per
    // type are in DISPLAY_GEOMETRY further below. DISPLAY_TYPE_DEFAULT only
    // sets the factory default. Backlight: also a setting (useBacklight,
    // brightness tab).
#define DISPLAY_TYPE_GC9A01 0
#define DISPLAY_TYPE_GC9D01 1
#define DISPLAY_TYPE_COUNT  2
#define DISPLAY_TYPE_DEFAULT DISPLAY_TYPE_GC9A01

#if defined(GC9A01) || defined(GC9D01) || defined(GC9A01_WITH_BACKLIGHT) || defined(ILI9341)
#error "Displaytyp wird nicht mehr per #define gewaehlt - Einstellung im Zifferblatt-Tab / display type is no longer chosen via #define - setting in the clock face tab"
#endif

    // Build-Kennung - steht auf den Info-Seiten und damit in jeder .bin
    // (Suche nach "UHR4_BUILD_DISPLAY="). Seit der Laufzeitauswahl enthaelt
    // jedes Build beide Displaytypen.

    // Build marker - shown on the info pages and therefore in every .bin
    // (search for "UHR4_BUILD_DISPLAY="). Since runtime selection, every build
    // contains both display types.
#define BUILD_DISPLAY_MARKER "UHR4_BUILD_DISPLAY=GC9A01+GC9D01"

    // Interner Schluessel (AES-256, 64 Hex-Zeichen) fuer die WLAN-Daten in
    // Sicherungen (backup.h). Muss in allen uhr4-Firmwares gleich sein, damit
    // eine Sicherung auf einer anderen Uhr wiederhergestellt werden kann.
    // Aendern macht WLAN-Daten aelterer Sicherungen unlesbar. Schuetzt die
    // Datei, nicht gegen jemanden, der diesen Schluessel aus Quelltext oder
    // Firmware ausliest.

    // Internal key (AES-256, 64 hex characters) for the WiFi data in backups
    // (backup.h). Must be the same in all uhr4 firmwares so a backup can be
    // restored on another clock. Changing it makes the WiFi data of older
    // backups unreadable. Protects the file, not against someone who reads
    // this key from the source code or firmware.
#define BACKUP_WIFI_KEY "ade1b6a09c1f5b970301983cddf75b160e99f45a70771438015f88eddbc3d174"

    // Interner Schluessel (AES-256, 64 Hex-Zeichen) fuer die WLAN-Passwoerter
    // in den Einstellungen (NVS, storeWifiPass() in wifi_manager.h): ein
    // Speicherabzug per esptool zeigt sie so nicht im Klartext. Schutz nur
    // gegen blosses Durchsehen - der Schluessel steckt in derselben Firmware.
    // Muss in allen uhr4-Firmwares gleich sein (Sicherungen enthalten die
    // verschluesselten Werte). Aendern macht gespeicherte Passwoerter unlesbar.

    // Internal key (AES-256, 64 hex characters) for the WiFi passwords in the
    // settings (NVS, storeWifiPass() in wifi_manager.h): a flash dump via
    // esptool does not show them in plain text. Protects only against simply
    // looking through - the key is in the same firmware. Must be the same in
    // all uhr4 firmwares (backups contain the encrypted values). Changing it
    // makes stored passwords unreadable.
#define WIFI_STORE_KEY "06c92514eb89bb624df9f3f7b291777edd2b44adf583e3ac403d916992664bdd"

    // Pin-Belegung: ESP32-S2 (Lolin S2 Pico)
    // Pin mapping: ESP32-S2 (Lolin S2 Pico)
#ifdef ESP32_S2  // Lolin S2 Pico
    // Pinbelegung ESP32<->TFT: 3.3V->vcc (rot), GND->gnd (blau), Rest siehe
    // PCB-Referenz: https://github.com/holgiw/TFT-Clock-GC9A01/blob/master/PCB/ESP32-S2%20GC9A01.jpg

    // ESP32<->TFT pinout: 3.3V->vcc (red), GND->gnd (blue), rest see
    // PCB reference: https://github.com/holgiw/TFT-Clock-GC9A01/blob/master/PCB/ESP32-S2%20GC9A01.jpg


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

    // TFT-SPI (beide Displays am selben Bus) - wird in lgfx_config.h
    // verwendet, an der LovyanGFX-Bibliothek selbst ist nichts einzustellen.
    // Belegung laut PCB-Referenz: 7 scl, 11 sda, 33 dc, 5 rst.

    // TFT SPI (both displays on the same bus) - used in lgfx_config.h,
    // nothing has to be configured inside the LovyanGFX library itself.
    // Mapping per PCB reference: 7 scl, 11 sda, 33 dc, 5 rst.
#define TFT_SCLK  7
#define TFT_MOSI  11
#define TFT_DC    33  // Data/Command
#define TFT_RST   5   // Reset
#define TFT_SPI_FREQUENCY 40000000 // hoechster ganzzahliger Teiler von 80 MHz unter den frueheren 60 MHz
                                   // highest integer divider of 80 MHz below the former 60 MHz

    // SPI-CS Display 1 - manuell gesteuert (setCS1()/setCS2() in display.h),
    // LovyanGFX bekommt pin_cs = -1 (lgfx_config.h). Beide Displays haengen
    // an einem Geraet, der CS-Wechsel waehlt nur den Chip aus.

    // SPI CS for display 1 - driven manually (setCS1()/setCS2() in
    // display.h), LovyanGFX gets pin_cs = -1 (lgfx_config.h). Both displays
    // hang off one device, switching CS only selects the chip.
#define CS_1    12

    // SPI-CS Display 2 (baugleich) - bei der Uhranzeige nur bedient, solange die Rotation von Display 2 nicht "n.a." ist.
    // SPI CS for display 2 (identical) - for the clock display only driven while display 2's rotation is not "n.a.".
#define CS_2    18

    // Rotationswert "nicht angeschlossen (n.a.)": fuer die Uhranzeige wird das Display
    // dann weder angesteuert noch berechnet (Zifferblatt/Zeiger entfallen). Status- und
    // Startmeldungen (Boot, AP-Modus, Codes) erscheinen weiterhin immer auf beiden Displays.

    // Rotation value "not connected (n.a.)": for the clock display the display is then
    // neither driven nor calculated (face/hands are skipped). Status and boot messages
    // (boot, AP mode, codes) still always appear on both displays.
#define TFT_ROTATION_NA 4

    // Werkseinstellung: Display 1 angeschlossen (0 Grad), Display 2 nicht angeschlossen.
    // Factory default: display 1 connected (0 degrees), display 2 not connected.
#define TFT_ROTATION1_DEFAULT 0
#define TFT_ROTATION2_DEFAULT TFT_ROTATION_NA

    // DCF77
#define DCF77_INTERRUPT 0
#define DCF77_DATAPIN 35

    // Hintergrundbeleuchtung - ob Pin 3 per PWM geregelt wird, entscheidet
    // die Einstellung useBacklight (globals.h), nicht mehr das Build.

    // Backlight - whether pin 3 is PWM-controlled is decided by the
    // useBacklight setting (globals.h), no longer by the build.
#define TFT_Backlight 3  // Hintergrundbeleuchtung
                         // Backlight
#define BACKLIGHT_CHANNEL 0  // PWM-Kanal
                             // PWM channel
#define BACKLIGHT_FREQ 5000
#define BACKLIGHT_RESOLUTION 8

#endif

    // Display: Masse und Standardgrafiken je Displaytyp. Beide Grafiksaetze
    // sind in der Firmware (eigene Namensraeume, da gleiche Array-Namen) -
    // ausgewaehlt wird zur Laufzeit ueber displayGeom (globals.h,
    // loadDisplayType() in display.h). Die Zifferblaetter sind RLE-komprimiert
    // (~33 statt ~166 KB), die kleinen Zeiger nicht. Erzeugt von
    // graphic/make_rle_defaults.py aus clock_default.h - nach einer Aenderung
    // an clock_default.h das Skript erneut laufen lassen. Das Zifferblatt wird
    // bei Bedarf entpackt (decodeDefaultFace()/allocDefaultFace() in display.h).

    // Display: dimensions and default graphics per display type. Both
    // graphics sets are in the firmware (own namespaces, since the array
    // names are identical) - selected at runtime via displayGeom (globals.h,
    // loadDisplayType() in display.h). The clock faces are RLE-compressed
    // (~33 instead of ~166 KB), the small hands aren't. Generated by
    // graphic/make_rle_defaults.py from clock_default.h - rerun the script
    // after changing clock_default.h. The clock face is unpacked on demand
    // (decodeDefaultFace()/allocDefaultFace() in display.h).
namespace gfx240 {
#include "graphic/240/clock_default_rle.h"
}
namespace gfx160 {
#include "graphic/160/clock_default_rle.h"
}

struct RleImage {
    const uint8_t* data;  // RLE-Datenstrom (rleEncode565()-Format)
                          // RLE data stream (rleEncode565() format)
    uint32_t size;        // Bytes
    uint32_t pixels;      // Pixel nach dem Entpacken
                          // pixels after unpacking
};
#define RLE_IMAGE(ns, name) { ns::name##Rle, sizeof(ns::name##Rle), ns::name##Pixels }

#define ROUND_DISPLAY // rundes Display - Kreismaskierung der Ecken (siehe scaleAndSaveBmp() in display.h)
                      // round display - circular corner masking (see scaleAndSaveBmp() in display.h)

    // Zeigerformat: Drehpunkte und Breiten je Display fest (Dateiformat, darf
    // sich nicht aendern); Drehpunkt-Spalte ist die halbe Breite. Alte Zeiger
    // werden oben (HAND_TOP_PAD) und seitlich (HAND_SIDE_PAD) transparent
    // aufgefuellt. handPivotY = Displayradius, der Zeiger reicht bis zum Rand.

    // Hand format: pivots and widths are fixed per display (file format, must
    // not change); the pivot column is half the width. Old hands are padded
    // transparent at the top (HAND_TOP_PAD) and at the sides (HAND_SIDE_PAD).
    // handPivotY = display radius, the hand reaches the edge.
struct DisplayGeometry {
    const char* name;
    int clock;             // Zifferblatt = Displayausschnitt, quadratisch
                           // clock face = display area, square
    int handLegacyWidth;   // alte Zeigerbreite (Dateiformat)
                           // old hand width (file format)
    int handWidth;         // neue Zeigerbreite
                           // new hand width
    int handLegacyHeight;
    int handLegacyPivotY;  // Drehpunkt im alten Format
                           // pivot in the old format
    int handPivotY;        // Drehpunkt im neuen Format
                           // pivot in the new format
    int textSize;          // Status-/Boottext
                           // status/boot text
    int centerSize;        // Werkseinstellung Nabe
                           // factory default hub size
    bool backlightDefault; // Werkseinstellung useBacklight: GC9D01 immer an Pin 3 verdrahtet
                           // factory default useBacklight: GC9D01 always wired to pin 3
    bool swRotation;       // Hardware-Rotation wirkungslos -> Software-Rotation (mit PSRAM)
                           // hardware rotation ineffective -> software rotation (with PSRAM)
    RleImage face;         // Standard-Zifferblatt, clock x clock, RLE
                           // default clock face, clock x clock, RLE
    const uint16_t* hour;  // Standardzeiger im alten Format (handLegacyWidth x handLegacyHeight), unkomprimiert
                           // default hands in the old format (handLegacyWidth x handLegacyHeight), uncompressed
    const uint16_t* minute;
    const uint16_t* second;
    uint32_t handPixels;   // Pixel je Standardzeiger (fuer die Pruefung unten)
                           // pixels per default hand (for the check below)
};

constexpr DisplayGeometry DISPLAY_GEOMETRY[DISPLAY_TYPE_COUNT] = {
    // name      clock legW  W  legH legPiv piv text hub   BL     swRot
    { "GC9A01",  240,  21,  25, 131, 100,  120,  2,  6,  false, false,
      RLE_IMAGE(gfx240, clockFace), gfx240::handHour, gfx240::handMinute, gfx240::handSecond,
      sizeof(gfx240::handHour) / sizeof(uint16_t) },
    { "GC9D01",  160,  13,  15,  86,  66,   80,  1,  3,  true,  true,
      RLE_IMAGE(gfx160, clockFace), gfx160::handHour, gfx160::handMinute, gfx160::handSecond,
      sizeof(gfx160::handHour) / sizeof(uint16_t) },
};

    // Alle drei Standardzeiger je Typ muessen gleich gross sein - handPixels
    // wird nur am Stundenzeiger gemessen.
    // All three default hands per type must be the same size - handPixels is
    // only measured on the hour hand.
static_assert(sizeof(gfx240::handHour) == sizeof(gfx240::handMinute) && sizeof(gfx240::handHour) == sizeof(gfx240::handSecond) &&
              sizeof(gfx160::handHour) == sizeof(gfx160::handMinute) && sizeof(gfx160::handHour) == sizeof(gfx160::handSecond),
              "default hands of one display type differ in size");

    // Obergrenzen ueber alle Typen - fuer fest dimensionierte Puffer
    // (rowBuffer in globals.h).
    // Upper bounds across all types - for fixed-size buffers (rowBuffer in globals.h).
#define CLOCK_MAX 240

constexpr bool displayGeometryValid(const DisplayGeometry& g) {
    return g.handPivotY >= g.handLegacyPivotY                                       // HAND_TOP_PAD >= 0
        && g.handWidth >= g.handLegacyWidth && (g.handWidth - g.handLegacyWidth) % 2 == 0 // Drehpunkt-Spalte bleibt mittig / pivot column stays centred
        && g.handPivotY <= g.clock / 2                                              // Drehpunkt innerhalb des Radius / pivot inside the radius
        && g.clock <= CLOCK_MAX
        && g.face.pixels == (uint32_t)(g.clock * g.clock)                           // Standardgrafiken passen zur Groesse / default graphics match the size
        && g.handPixels == (uint32_t)(g.handLegacyWidth * g.handLegacyHeight);
}
static_assert(displayGeometryValid(DISPLAY_GEOMETRY[DISPLAY_TYPE_GC9A01]) &&
              displayGeometryValid(DISPLAY_GEOMETRY[DISPLAY_TYPE_GC9D01]),
              "DISPLAY_GEOMETRY: hand pivot/width/clock size inconsistent");

    // Bisherige Konstanten-Namen bleiben erhalten, zeigen aber auf den zur
    // Laufzeit gewaehlten Typ (displayGeom, globals.h) - daher NICHT in
    // Array-Groessen, static_assert oder #if verwenden.

    // The former constant names stay, but point to the type selected at
    // runtime (displayGeom, globals.h) - so do NOT use them in array sizes,
    // static_assert or #if.
#define CLOCK_WIDTH         (displayGeom->clock)
#define CLOCK_HEIGHT        (displayGeom->clock)
#define TFT_WIDTH           CLOCK_WIDTH
#define TFT_HEIGHT          CLOCK_HEIGHT
#define HAND_LEGACY_WIDTH   (displayGeom->handLegacyWidth)
#define HAND_WIDTH          (displayGeom->handWidth)
#define HAND_LEGACY_HEIGHT  (displayGeom->handLegacyHeight)
#define HAND_LEGACY_PIVOT_Y (displayGeom->handLegacyPivotY)
#define HAND_PIVOT_Y        (displayGeom->handPivotY)
#define TFT_TEXT_SIZE       (displayGeom->textSize)
#define BACKLIGHT_DEFAULT   (displayGeom->backlightDefault)
    // Standardzeiger direkt aus dem Flash. Das Standard-Zifferblatt ist
    // RLE-komprimiert und hat bewusst KEIN solches Makro - decodeDefaultFace()/
    // allocDefaultFace() (display.h) entpacken es nur bei Bedarf ins Ziel.

    // Default hands straight from flash. The default clock face is
    // RLE-compressed and deliberately has NO such macro - decodeDefaultFace()/
    // allocDefaultFace() (display.h) unpack it into the target only when needed.
#define handHour            (displayGeom->hour)
#define handMinute          (displayGeom->minute)
#define handSecond          (displayGeom->second)

#define HAND_TOP_PAD (HAND_PIVOT_Y - HAND_LEGACY_PIVOT_Y)          // 20 bzw. 14
                                                                   // 20 or 14
#define HAND_HEIGHT (HAND_LEGACY_HEIGHT + HAND_TOP_PAD)            // 151 bzw. 100
                                                                   // 151 or 100
#define HAND_SIDE_PAD ((HAND_WIDTH - HAND_LEGACY_WIDTH) / 2)       // 2 bzw. 1
                                                                   // 2 or 1


    // System / Debug
#define DEBUG_PRINT(x)    { if (loggingEnabled) { Serial.print(x);   logToFile(String(x));}}
#define DEBUG_PRINTLN(x)  { if (loggingEnabled) { Serial.println(x); logToFile(String(x));}}
#define DEBUG_PRINTF(...) { if (loggingEnabled) { char buffer[128]; snprintf(buffer, sizeof(buffer), __VA_ARGS__); Serial.print(buffer); logToFile(String(buffer));}}

    // Schwellwert fuer Heap-Warnungen (siehe checkHeapWarning() in
    // system_utils.h) - loggt fruehzeitig statt erst spaeter ueber /status.

    // Heap warning threshold (see checkHeapWarning() in system_utils.h) -
    // logs early instead of only being noticed later via /status.
#define HEAP_WARNING_THRESHOLD 20480 // 20 KB

    // Erlaubter Wertebereich fuer die Groesse der Web-Vorschau (/preview) -
    // vom Nutzer per Schieberegler einstellbar, in PK_PREVIEW_SIZE
    // gespeichert. Zentral hier statt in /preview und /api/setPreviewSize dupliziert.

    // Allowed value range for the web preview's size (/preview) - user-
    // adjustable via a slider, stored in PK_PREVIEW_SIZE. Centralized here
    // instead of duplicated in /preview and /api/setPreviewSize.
#define PREVIEW_SIZE_MIN 150
#define PREVIEW_SIZE_MAX 800
#define PREVIEW_SIZE_DEFAULT 400

    // Fuehrt einen Zeichenblock je einmal fuer Display 1 und 2 aus, korrekt
    // rotiert (beginStatusDraw()/endStatusDraw() in display.h). Bedient immer
    // BEIDE Displays, auch bei Rotation "n.a." (Boot, AP-Modus, Codes).
    // Makro statt Funktion, da schon vor display.h benutzt (wifi_manager.h).

    // Runs a drawing block once for display 1 and once for 2, correctly
    // rotated (beginStatusDraw()/endStatusDraw() in display.h). Always serves
    // BOTH displays, even with rotation "n.a." (boot, AP mode, codes).
    // Macro instead of function since it's used before display.h is included (wifi_manager.h).
#define DRAW_ON_BOTH_DISPLAYS(...) \
    do { \
        { lgfx::LovyanGFX& tft = beginStatusDraw(1); __VA_ARGS__ } \
        endStatusDraw(1); \
        { lgfx::LovyanGFX& tft = beginStatusDraw(2); __VA_ARGS__ } \
        endStatusDraw(2); \
        setCSIdle(); \
    } while (0)

    // GitHub-Repository - zentral hier, damit ein Fork/Umzug nur diese
    // Stelle statt mehrerer in webserver_routes.h aendern muss.

    // GitHub repository - kept centrally here, so a fork/move only needs
    // changing this spot instead of several in webserver_routes.h.
#define GITHUB_REPO_OWNER "holgiw"
#define GITHUB_REPO_NAME "TFT-Clock-GC9A01"
#define GITHUB_REPO_URL "https://github.com/" GITHUB_REPO_OWNER "/" GITHUB_REPO_NAME
#define GITHUB_API_CONTENTS_BASE "https://api.github.com/repos/" GITHUB_REPO_OWNER "/" GITHUB_REPO_NAME "/contents/graphic/"
#define GITHUB_ZIP_BASE "https://github.com/" GITHUB_REPO_OWNER "/" GITHUB_REPO_NAME "/blob/master/graphic/"
#define GITHUB_RAW_BASE "https://raw.githubusercontent.com/" GITHUB_REPO_OWNER "/" GITHUB_REPO_NAME "/master/"

    // Zeit / NTP-Standardwerte & Timing-Makros
    // Time / NTP defaults & timing macros

    // Zeitserver & Zeitzone Standardwert
    // time server & timezone default
#define NTP_SERVER_1 "pool.ntp.org"
#define NTP_SERVER_2 "ptbtime1.ptb.de"
#define TIMEZONE_DEFAULT "CET-1CEST,M3.5.0,M10.5.0/3" // Mitteleuropaeische Zeit
                                                      // Central European Time

    // Versuche PRO NTP-Server, bevor setupNTP() zum naechsten wechselt (siehe
    // time_sync.h) - ein einzelnes verlorenes UDP-Paket soll nicht sofort als

    // Fehlschlag zaehlen. configTzTime() wird pro Versuch neu aufgerufen,
    // da der SNTP-Client sonst keine neue Anfrage verschickt.

    // Attempts PER NTP server before setupNTP() moves to the next one (see
    // time_sync.h) - a single lost UDP packet shouldn't count as a failure

    // right away. configTzTime() is called fresh each attempt, since
    // otherwise the SNTP client won't send a new request.
#define NTP_SYNC_ATTEMPTS 2

    // Ab dieser Abweichung (Sekunden) wird die RTC bei einem NTP-/DCF77-Sync
    // ueberhaupt geschrieben (siehe rtcDriftSec() in time_sync.h) - kleinere
    // Differenzen sind normale Rundung, kein unnoetiger I2C-Schreibzugriff.

    // Above this deviation (seconds) the RTC is actually written on an NTP/
    // DCF77 sync (see rtcDriftSec() in time_sync.h) - smaller differences
    // are normal rounding, not worth an unnecessary I2C write.
#define RTC_UPDATE_MIN_DRIFT_SEC 2

    // Startzeit der Anzeige, solange noch keine Uhrzeit aus NTP/DCF77/RTC/USB
    // vorliegt (klassische Uhrmacher-Stellung 10:10:30 statt 12:00:00). Die
    // Uhr laeuft ab dem Start von dort aus weiter (updateClock() in
    // display.h), bis eine echte Zeit kommt - die Systemzeit bleibt dabei
    // ungueltig (NTP-Server schweigt, Einrichtungs-Helligkeit bleibt).

    // Start time of the display as long as no time from NTP/DCF77/RTC/USB is
    // available yet (classic watchmaker position 10:10:30 instead of
    // 12:00:00). The clock keeps running from there after boot (updateClock()
    // in display.h) until a real time arrives - the system time stays invalid
    // meanwhile (NTP server stays silent, setup brightness stays).
#define START_TIME_HOUR 10
#define START_TIME_MIN  10
#define START_TIME_SEC  30

    // So lange zeigt das Display nach dem Start des Access Points (kein WLAN)
    // dessen Zugangsdaten, danach laeuft die Uhr (ohne Zeitquelle ab der
    // Startzeit). Der Access Point bleibt aktiv, ein kurzer Tasterdruck zeigt
    // die Daten erneut.
    // For this long after starting the access point (no WiFi) the display
    // shows its credentials, then the clock runs (from the start time without
    // a time source). The access point stays active, a short button press
    // shows the credentials again.
#define AP_INFO_SHOW_MS (2 * WAIT_1m)

    // Versuche PRO WLAN-Netzwerk beim Boot (siehe connectWiFiAtBoot() in
    // uhr4.ino), bevor mit dem naechsten Netzwerk weitergemacht bzw. ganz
    // aufgegeben wird (-> WPS/Access-Point) - ein einzelner fehlgeschlagener
    // Verbindungsversuch (z.B. Router kurz beschaeftigt) soll das gefundene
    // Netzwerk nicht gleich verwerfen.

    // Attempts PER WiFi network at boot (see connectWiFiAtBoot() in
    // uhr4.ino), before moving on to the next network or giving up entirely
    // (-> WPS/access point) - a single failed connection attempt (e.g. the
    // router being briefly busy) shouldn't discard a network that was found.
#define WIFI_CONNECT_ATTEMPTS 2

    // Access-Point (Einrichtungsmodus): SSID und Passwort fest in der Firmware
    // - stehen in der Anleitung und auf dem Display, die Einrichtung klappt so
    // auch, wenn das Display (noch) nichts Lesbares zeigt. Das Passwort ist
    // damit auf jeder Uhr gleich; wer das nicht will, setzt hier ein eigenes.
    // WPA2 verlangt 8 bis 63 Zeichen.

    // Access point (setup mode): SSID and password fixed in the firmware -
    // they are in the manual and on the display, so setup also works if the
    // display shows nothing readable (yet). The password is therefore the same
    // on every clock; set your own here if you don't want that. WPA2 requires
    // 8 to 63 characters.
#define AP_SSID "clock123"
#define AP_PASSWORD "clocksetup"
static_assert(sizeof(AP_PASSWORD) - 1 >= 8 && sizeof(AP_PASSWORD) - 1 <= 63, "AP_PASSWORD: WPA2 verlangt 8-63 Zeichen / WPA2 requires 8-63 characters");

#define WAIT_1s 1000 // 1 Sekunde in Millisekunden
                     // 1 second in milliseconds
#define WAIT_3s 3000 // 3 Sekunden in Millisekunden
                     // 3 seconds in milliseconds
#define WAIT_5s 5000 // 5 Sekunden in Millisekunden
                     // 5 seconds in milliseconds
#define WAIT_10s 10000 // 10 Sekunden in Millisekunden
                       // 10 seconds in milliseconds
#define WAIT_15s 15000 // 15 Sekunden in Millisekunden
                       // 15 seconds in milliseconds
#define WAIT_30s 30000 // 30 Sekunden in Millisekunden
                       // 30 seconds in milliseconds
#define WAIT_1m 60000 // 1 Minute in Millisekunden
                      // 1 minute in milliseconds
#define WAIT_15m 900000 // 15 Minuten in Millisekunden
                        // 15 minutes in milliseconds

    // Gueltigkeitsdauer des Web-Factory-Reset-Codes (siehe factoryResetCode
    // in globals.h) - danach verschwindet der Code vom Display wieder,
    // ungenutzt, und muss bei Bedarf erneut angefordert werden.

    // Validity period of the web factory-reset code (see factoryResetCode in
    // globals.h) - after this it disappears from the display again, unused,
    // and has to be requested again if still needed.
#define FACTORY_RESET_CODE_TIMEOUT_MS WAIT_1m

    // Maximale Fehlversuche fuer einen einzelnen Bestaetigungscode (siehe
    // factoryResetCodeAttempts in globals.h), bevor er ungueltig wird und ein
    // neuer angefordert werden muss - Bremse gegen Brute-Force-Raten.

    // Maximum wrong attempts for a single confirmation code (see
    // factoryResetCodeAttempts in globals.h) before it becomes invalid and a
    // new one has to be requested - a brake against brute-force guessing.
#define FACTORY_RESET_MAX_ATTEMPTS 5
#define WAIT_30m 1800000 // 30 Minuten in Millisekunden
                         // 30 minutes in milliseconds
#define WAIT_1h 3600000 // 1 Stunde in Millisekunden
                        // 1 hour in milliseconds
#define WAIT_6h 21600000 // 6 Stunden in Millisekunden
                         // 6 hours in milliseconds

    // Rocrail-Modellzeit (siehe rocrail_client.h): TCP-Client-Port des
    // Servers ist IANA-registriert und praktisch immer 8051. Verbindungs-
    // versuch laeuft minuetlich - ein haengender Modellbahn-PC soll nicht

    // im Sekundentakt angeklopft werden. Der Verbindungsaufbau laeuft in einer
    // eigenen Task (siehe rocrailConnectTaskFunc()), ein nicht erreichbarer
    // Server blockiert loop()/den Webserver daher NICHT mehr - der Timeout
    // kann grosszuegiger als noetig gewaehlt werden, ohne das zu riskieren.

    // Rocrail model time (see rocrail_client.h): the server's TCP client
    // port is IANA-registered and practically always 8051. The connection
    // attempt runs once a minute - an unreachable layout PC shouldn't be

    // knocked on every second. The connection attempt runs in its own task
    // (see rocrailConnectTaskFunc()), so an unreachable server no longer
    // blocks loop()/the web server - the timeout can be set generously
    // without risking that.
#define ROCRAIL_DEFAULT_PORT 8051
#define ROCRAIL_CONNECT_TIMEOUT_MS 5000 // 5 Sekunden in Millisekunden - laeuft in einer eigenen Task, blockiert also nichts (siehe oben)
                                        // 5 seconds in milliseconds - runs in its own task, so this blocks nothing (see above)
#define ROCRAIL_RECONNECT_INTERVAL_MS WAIT_1m
#define ROCRAIL_LOG_THROTTLE_LIMIT 2 // gemeinsame Grenze fuer gedrosselte Rocrail-Logs (siehe
                                     // shouldLogThrottled() in rocrail_client.h) - eine Stelle
                                     // statt mehrfach wiederholtem Literal.

                                     // shared limit for throttled Rocrail logs (see
                                     // shouldLogThrottled() in rocrail_client.h) - one spot
                                     // instead of a repeated literal.

    // Ab dieser Winkeldifferenz gilt ein Vorwaerts-Frame als abnormal
    // (verzoegerter Frame, z.B. durch eine blockierende Web-Anfrage) statt
    // als normaler Tick - siehe Abfederung in renderClockFrame() (display.h).

    // Above this angle difference, a forward frame counts as abnormal (a
    // delayed frame, e.g. a blocking web request) instead of a normal tick
    // - see the easing in renderClockFrame() (display.h).
#define SECOND_HAND_MAX_NORMAL_FORWARD_STEP_DEG 6.5f

    // Ab diesem Divider werden Sekundenzeiger UND Nabe ausgeblendet (siehe
    // renderClockFrame()) - bei so hoher Beschleunigung ist ihre Bewegung/
    // Sichtbarkeit ohnehin kaum noch sinnvoll.

    // From this divider onwards, the second hand AND the hub are hidden
    // (see renderClockFrame()) - at such high acceleration their movement/
    // visibility isn't meaningfully useful anyway.
#define ROCRAIL_HIDE_DETAILS_DIVIDER 11

    // Bleibt ein <clock>-Update laenger als das aus, gilt die Modellzeit als
    // veraltet - die Uhr faellt dann auf NTP/RTC/DCF77 zurueck (siehe
    // rocrailTimeReady in renderClockFrame()).

    // If a <clock> update stays absent longer than this, the model time
    // counts as stale - the clock then falls back to NTP/RTC/DCF77 (see
    // rocrailTimeReady in renderClockFrame()).
#define ROCRAIL_STALE_TIMEOUT_MS (2 * WAIT_1m)

    // Weicht eine neu gemeldete <clock>-Zeit ab, wird sanft statt schlagartig
    // angeglichen (siehe advanceRocrailTime()) - Anteil der Abweichung, der
    // pro Sekunde ausgeglichen wird. Oberhalb von SNAP_THRESHOLD wird stattdessen sofort gesprungen.

    // If a newly reported <clock> time differs, it's eased in smoothly
    // instead of abruptly (see advanceRocrailTime()) - fraction of the
    // deviation corrected per second. Above SNAP_THRESHOLD it snaps directly instead.
#define ROCRAIL_DRIFT_CORRECTION_RATE 0.5f
#define ROCRAIL_DRIFT_SNAP_THRESHOLD_SECONDS 30.0f

    // Diagnose: R2RNet-Multicast-Gruppe mithoeren und jedes empfangene Paket
    // unveraendert loggen (siehe startR2rnetDebugListener() in rocrail_client.h).
    // Dient NUR der Analyse des Antwortformats - echte R2RNet-Discovery wurde
    // entfernt (siehe Kommentar am Kopf von rocrail_client.h).
    // 224.0.1.20:8051 ist laut wiki.rocrail.net die tatsaechliche R2RNet-
    // Adresse - 224.0.0.1 (zuvor hier) ist die reservierte "All Hosts"-
    // Gruppe (RFC 1112), fuer die ein expliziter IGMP-Join meist scheitert.

    // Diagnostic: listen on the R2RNet multicast group and log every
    // received packet unchanged (see startR2rnetDebugListener() in
    // rocrail_client.h). ONLY for analyzing the reply format - actual
    // R2RNet discovery was removed (see the comment at the top of
    // rocrail_client.h).
    // 224.0.1.20:8051 is the actual R2RNet address per wiki.rocrail.net -
    // 224.0.0.1 (previously here) is the reserved "All Hosts" group
    // (RFC 1112), for which an explicit IGMP join usually fails.

#define R2RNET_DEBUG_MULTICAST_IP "224.0.1.20"
#define R2RNET_DEBUG_MULTICAST_PORT 8051
#define R2RNET_DEBUG_PACKET_BUFFER_SIZE 512
#define R2RNET_DEBUG_LOG_LIMIT 20 // nur so viele Pakete je Beitritt loggen - reicht zur Formatanalyse, schont Log/Flash
                                  // log only this many packets per join - enough for format analysis, spares log/flash

    // DCF77-Status-Punkt in der Topbar: dcfTimeFound/dcf77Count werden nie
    // zurueckgesetzt, daher diese Schwellwerte, damit der Punkt bei
    // Empfangsausfall wieder auf gelb/rot faellt statt fuer immer gruen zu bleiben.

    // DCF77 status dot in the topbar: dcfTimeFound/dcf77Count are never
    // reset, hence these thresholds so the dot falls back to yellow/red on
    // a reception outage instead of staying green forever.
#define DCF77_SYNC_STALE_AFTER (15 * WAIT_1m)
#define DCF77_PULSE_STALE_AFTER WAIT_1m

    // Anwesenheitserkennung DCF77: ein floatender Pin kann durch Rauschen
    // einzelne Interrupts ausloesen, echter Empfang aendert dcf77Count
    // dagegen regelmaessig - MIN_STREAK/MAX_GAP_MS verlangen mehrere passende Aenderungen in Folge.

    // DCF77 presence detection: a floating pin can trigger stray interrupts
    // from noise, genuine reception changes dcf77Count regularly instead -
    // MIN_STREAK/MAX_GAP_MS require several matching changes in a row.
#define DCF77_PRESENCE_MIN_STREAK 6
#define DCF77_PRESENCE_MAX_GAP_MS 1500

    // Rausch-Filter fuer den Bit-Fortschritt (processDcf77Bits() in
    // time_sync.h): sehr kurze Stoerflanken (Prellen) sind deutlich kuerzer
    // als jeder echte Zustand (kuerzester: ~100ms). Flanken unter diesem Wert
    // werden verworfen, ohne den Referenzzeitpunkt zu verschieben.

    // Noise filter for the bit progress (processDcf77Bits() in time_sync.h):
    // very short spurious edges (bounce) are much shorter than any genuine
    // state (shortest: ~100ms). Edges below this value are discarded without
    // shifting the reference timestamp.

#define DCF77_BIT_NOISE_IGNORE_MS 70

    // Sekundenraster-Dekoder: Impulsabstand ist bei DCF77 immer ein
    // Vielfaches einer Sekunde, daher bleibt die Position auch bei
    // schwachem Empfang erhalten. PULSE_MAX/ONE_MIN: Impuls-/Bit-1-Schwelle, SECOND_MS: Rasterweite, STEP_TOLERANCE: erlaubte Abweichung.

    // Second-grid decoder: with DCF77 the pulse spacing is always a whole
    // number of seconds, so the position survives even weak reception.
    // PULSE_MAX/ONE_MIN: pulse/bit-1 threshold, SECOND_MS: grid width, STEP_TOLERANCE: allowed deviation.
#define DCF77_PULSE_MAX_MS 450
#define DCF77_PULSE_ONE_MIN_MS 150
#define DCF77_SECOND_MS 1000
#define DCF77_STEP_TOLERANCE_MS 300
    // Wie viele Sekunden eine Empfangsluecke ueberbruecken darf, ohne dass
    // das Sekundenraster (dcf77Phase in globals.h) verlorengeht - bei 60s
    // liegt der ESP32-Quarzfehler noch weit unter STEP_TOLERANCE_MS.

    // How many seconds a reception gap may bridge without losing the second
    // grid (dcf77Phase in globals.h) - at 60s the ESP32's crystal error is
    // still far below STEP_TOLERANCE_MS.
#define DCF77_MAX_PHASE_GAP_SECONDS 60

    // Minutenmarken-Erkennung: eine Rasterposition gilt als Marke, wenn sie
    // mind. MIN_MISSES mal fehlte und mind. MIN_LEAD Vorsprung vor dem
    // naechstbesten Kandidaten hat. COUNT_MAX halbiert die Zaehler, MISS_COUNT_MAX_GAP begrenzt eine einzelne ausgefallene Sekunde.

    // Minute marker detection: a grid position counts as the marker once it
    // missed at least MIN_MISSES times and leads the next best candidate by
    // at least MIN_LEAD. COUNT_MAX halves the counters, MISS_COUNT_MAX_GAP bounds a single dropped second.
#define DCF77_MISS_COUNT_MAX_GAP 5

#define DCF77_MARKER_MIN_MISSES 3
#define DCF77_MARKER_MIN_LEAD 2
#define DCF77_MARKER_COUNT_MAX 200

    // So viele Telegramme in Folge mit unmoeglichen Festbits (Bit0!=0 bzw.
    // Bit20!=1) gelten als Beweis fuer eine falsche Minutenmarke - mehr als
    // eines, da ein einzelner Stoerimpuls zufaellig auch nur diese Bits treffen kann.

    // This many consecutive telegrams with impossible fixed bits (bit0!=0
    // resp. bit20!=1) count as proof the minute marker is wrong - more than
    // one, since a single spurious pulse can happen to hit just these bits.
#define DCF77_STRUCT_FAIL_LIMIT 3

    // Maximalalter des letzten dekodierten Telegramms, um noch als
    // Zeitquelle zu gelten (applyDcf77DecodedTime() in time_sync.h) - ein
    // aelteres Telegramm ist unproblematisch, da die verstrichene Zeit ueber
    // millis() exakt nachgerechnet wird.

    // Max age of the last decoded telegram to still count as a time source
    // (applyDcf77DecodedTime() in time_sync.h) - an older telegram is fine,
    // since the elapsed time is added back precisely via millis().

#define DCF77_DECODED_MAX_AGE (10 * WAIT_1m)

    // Dauer des LED-Aufblitzens pro DCF77-Impuls (loop() in uhr4.ino) -
    // bewusst ein Blitz mit fester Abschaltzeit statt toggleLED(), da sonst
    // der Endzustand von der (geraden/ungeraden) Impulsanzahl abhinge.

    // Duration of the LED flash per DCF77 pulse (loop() in uhr4.ino) -
    // deliberately a flash with a fixed switch-off time instead of
    // toggleLED(), since the final state would otherwise depend on whether
    // the pulse count was even or odd.

#define DCF77_LED_BLINK_MS 80

    // Geschwindigkeit des Sekundenzeigers im Bahnhofsuhr-Modus: 60*975ms =
    // 58,5s Umlauf (Original-Hilfiker-Wert), Rest der Minute ruht der Zeiger
    // oben auf der 12. Wert fliesst auch in die Web-Live-Vorschau ein.

    // Second hand speed in station-clock mode: 60*975ms = 58.5s per sweep
    // (original Hilfiker value), the hand rests at the top for the rest of
    // the minute. Also used by the web live preview.
#define FAST_SECOND 975.0f

    // Web: Live-Vorschau (/preview-Route)
    // Groesse der Zeiger-Vorschau in Pixeln - zentral statt als magische Zahl im Routencode.

    // Web: live preview (/preview route)
    // Size of the hand preview in pixels - kept here instead of a magic number in route code.
#define LIVE_PREVIEW_SIZE 400

    // Hoehe der scrollbaren Textfenster im Log-Tab und auf der Info-Seite -
    // an EINER Stelle, damit beide Fenster gleich hoch bleiben. vh statt
    // fester Pixel: reicht auf grossen Monitoren weiter runter, min-height haelt es auf Handys trotzdem gross genug.

    // Height of the scrollable text windows in the Log tab and on the info
    // page - in ONE place, so both windows stay the same height. vh instead
    // of fixed pixels: reaches further down on large monitors, min-height still keeps it usable on phones.
#define INFO_LOG_WINDOW_HEIGHT_CSS "height:72vh;min-height:400px;"

    // Transparent in R5G6B5 RGB(16)
#define TRANSPARENT_COLOR 0x0120

