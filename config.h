#pragma once

    // Reihenfolge: zuerst Prozessor/Display (Pins, Displaymasse), danach die restlichen Werte nach Modul
    // sortiert. Variablen (tft, webserver, preferences, ...) stehen nur in globals.h.

    // Order: processor/display first (pins, display dimensions), then the remaining values sorted by module.
    // Variables (tft, webserver, preferences, ...) are only in globals.h.


    // Displaytyp und Hintergrundbeleuchtung sind Einstellungen (PK_DISPLAY_TYPE, useBacklight) - eine
    // Firmware je Board fuer dessen Displays (GC9A01, GC9D01, ILI9341 bzw. ST7789; rechteckige mit Streifen
    // fuer Uhrzeit und Datum). Masse je Typ in DISPLAY_GEOMETRY, DISPLAY_TYPE_DEFAULT ist die Werkseinstellung.

    // Display type and backlight are settings (PK_DISPLAY_TYPE, useBacklight) - one firmware per board for
    // its displays (GC9A01, GC9D01, ILI9341 or ST7789; rectangular ones with a time and date strip).
    // Dimensions per type in DISPLAY_GEOMETRY, DISPLAY_TYPE_DEFAULT is the factory default.

#define DISPLAY_TYPE_GC9A01  0
#define DISPLAY_TYPE_GC9D01  1
#define DISPLAY_TYPE_ILI9341 2
#define DISPLAY_TYPE_ST7789  3   // Waveshare ESP32-C6-LCD-1.47 (172x320)
#define DISPLAY_TYPE_ST7789_240 4 // Waveshare ESP32-C6-LCD-1.3 (240x240)
#define DISPLAY_TYPE_COUNT   5

    // Board, erkannt am Chip: ESP32-S2 (Lolin S2 Pico) mit wechselbarem Display, Waveshare ESP32-C6-LCD mit
    // fest verbautem ST7789 (1.47 = 172x320 oder 1.3 = 240x240, welches waehlt der Displaytyp) oder Waveshare
    // ESP32-S3-LCD-1.28 mit fest verbautem GC9A01. Pinbelegung weiter unten.

    // Board, recognized by the chip: ESP32-S2 (Lolin S2 Pico) with an exchangeable display, Waveshare
    // ESP32-C6-LCD with a built-in ST7789 (1.47 = 172x320 or 1.3 = 240x240, chosen by the display type) or
    // Waveshare ESP32-S3-LCD-1.28 with a built-in GC9A01. Pin mapping further below.

#if CONFIG_IDF_TARGET_ESP32C6
#define BOARD_WAVESHARE_C6_ST7789 1
#define BOARD_WAVESHARE_S3_GC9A01 0
#define DISPLAY_TYPE_DEFAULT DISPLAY_TYPE_ST7789
#elif CONFIG_IDF_TARGET_ESP32S3
#define BOARD_WAVESHARE_C6_ST7789 0
#define BOARD_WAVESHARE_S3_GC9A01 1
#define DISPLAY_TYPE_DEFAULT DISPLAY_TYPE_GC9A01
#else
#define BOARD_WAVESHARE_C6_ST7789 0
#define BOARD_WAVESHARE_S3_GC9A01 0
#define DISPLAY_TYPE_DEFAULT DISPLAY_TYPE_GC9A01
#endif

#if defined(GC9A01) || defined(GC9D01) || defined(GC9A01_WITH_BACKLIGHT) || defined(ILI9341)
#error "Displaytyp wird nicht mehr per #define gewaehlt - Einstellung im Zifferblatt-Tab / display type is no longer chosen via #define - setting in the clock face tab"
#endif

    // Build-Kennung - steht auf den Info-Seiten und damit in jeder .bin (Suche nach "UHR4_BUILD_DISPLAY=").
    // Der S2-Build enthaelt alle wechselbaren Displaytypen, der C6-Build die beiden ST7789 der
    // Waveshare-Boards, der S3-Build das GC9A01 des Waveshare-Boards.

    // Build marker - shown on the info pages and therefore in every .bin (search for "UHR4_BUILD_DISPLAY=").
    // The S2 build contains all exchangeable display types, the C6 build the two ST7789 of the Waveshare
    // boards, the S3 build the GC9A01 of the Waveshare board.

#if BOARD_WAVESHARE_C6_ST7789
#define BUILD_DISPLAY_MARKER "UHR4_BUILD_DISPLAY=ST7789+ST7789_240"
#elif BOARD_WAVESHARE_S3_GC9A01
#define BUILD_DISPLAY_MARKER "UHR4_BUILD_DISPLAY=GC9A01_S3"
#else
#define BUILD_DISPLAY_MARKER "UHR4_BUILD_DISPLAY=GC9A01+GC9D01+ILI9341"
#endif

    // Interner Schluessel (AES-256, 64 Hex-Zeichen) fuer die WLAN-Daten in Sicherungen. Muss in allen
    // uhr4-Firmwares gleich sein (Wiederherstellen auf anderer Uhr); Aendern macht aeltere Sicherungen
    // unlesbar. Kein echter Schutz - der Schluessel steckt in Quelltext und Firmware.

    // Internal key (AES-256, 64 hex characters) for the WiFi data in backups. Must be the same in all uhr4
    // firmwares (restore on another clock); changing it makes older backups unreadable. No real protection -
    // the key is in the source code and firmware.

#define BACKUP_WIFI_KEY "ade1b6a09c1f5b970301983cddf75b160e99f45a70771438015f88eddbc3d174"

    // Interner Schluessel (AES-256, 64 Hex-Zeichen) fuer die WLAN-Passwoerter im NVS - ein Speicherabzug
    // zeigt sie nicht im Klartext, schuetzt aber nur gegen blosses Durchsehen. In allen uhr4-Firmwares
    // gleich; Aendern macht gespeicherte Passwoerter unlesbar.

    // Internal key (AES-256, 64 hex characters) for the WiFi passwords in NVS - a flash dump does not show
    // them in plain text, but it only protects against simply looking through. Same in all uhr4 firmwares;
    // changing it makes stored passwords unreadable.

#define WIFI_STORE_KEY "06c92514eb89bb624df9f3f7b291777edd2b44adf583e3ac403d916992664bdd"

#if BOARD_WAVESHARE_C6_ST7789

    // Ohne "USB CDC On Boot: Enabled" laeuft Serial ueber UART0 (GPIO 16/17 = RTC) statt ueber USB - flashESP
    // koennte weder WLAN noch Displaytyp senden. Beim "ESP32C6 Dev Module" steht die Option sonst auf Disabled.

    // Without "USB CDC On Boot: Enabled" Serial runs over UART0 (GPIO 16/17 = RTC) instead of USB - flashESP
    // could send neither WiFi nor display type. On the "ESP32C6 Dev Module" the option is Disabled otherwise.

#if !ARDUINO_USB_CDC_ON_BOOT
#error "ESP32-C6: Board-Option USB CDC On Boot auf Enabled stellen / set the board option USB CDC On Boot to Enabled"
#endif

    // Waveshare ESP32-C6-LCD-1.47 und -1.3 gleich belegt: Display (6, 7, 14, 15, 21, BL 22) und RGB-LED (8,
    // per digitalWrite(LED_BUILTIN)) fest verbaut. Lichtsensor, RTC, DCF77 und Taster an GPIO 1, 2, 3, 16,
    // 17, 20, 23 - die einzigen Pins beider Stiftleisten, dieselbe Verdrahtung passt an beide Boards.

    // Waveshare ESP32-C6-LCD-1.47 and -1.3 mapped alike: display (6, 7, 14, 15, 21, BL 22) and RGB LED (8,
    // via digitalWrite(LED_BUILTIN)) built in. Light sensor, RTC, DCF77 and buttons on GPIO 1, 2, 3, 16, 17,
    // 20, 23 - the only pins on both pin headers, the same wiring fits both boards.

#define LED_BOARD      LED_BUILTIN
#define LED_BOARD_GPIO 8

#define ADC_3V 1
#define ADC_PIN 2
#define ADC_GND 3

#define BUTTON1 23
#define BOOT_BUTTON 9

    // RTC an TX/RX (UART0) - die serielle Ausgabe laeuft ueber USB, nur der Boot-Code sendet kurz auf TX
    // RTC on TX/RX (UART0) - serial output goes via USB, only the boot code briefly sends on TX

#define SDA_PIN 16
#define SCL_PIN 17

#define TFT_SCLK  7
#define TFT_MOSI  6
#define TFT_DC    15
#define TFT_RST   21

#define CS_1    14
#define CS_2    -1   // kein zweites Display moeglich (Displayleitungen nicht an der Stiftleiste)
                     // no second display possible (display lines not on the pin header)

    // CS schaltet hier LovyanGFX: liegt CS dauerhaft LOW, verschiebt ein Taktimpuls beim Bus-Init nach einem
    // Software-Neustart die Bits und das ST7789 bleibt schwarz. Mit CS HIGH beim Init und je Uebertragung
    // einer neuen Flanke faengt sich das Panel.

    // LovyanGFX drives CS here: with CS held LOW, a clock pulse during bus init after a software restart
    // shifts the bits and the ST7789 stays black. With CS HIGH during init and a fresh edge per transfer
    // the panel resyncs.

#define LGFX_CS_PIN CS_1

#define DCF77_DATAPIN 20
#define TFT_Backlight 22

#elif BOARD_WAVESHARE_S3_GC9A01

    // USB laeuft ueber den CH343P-Wandler an UART0 (GPIO 43/44), nicht ueber das USB des ESP32-S3. Mit "USB
    // CDC On Boot: Enabled" ginge Serial an die nicht angeschlossenen GPIO 19/20 - flashESP koennte weder WLAN
    // noch Displaytyp senden.

    // USB runs via the CH343P converter on UART0 (GPIO 43/44), not via the ESP32-S3's USB. With "USB CDC On
    // Boot: Enabled" Serial would go to the unconnected GPIO 19/20 - flashESP could send neither WiFi nor
    // display type.

#if ARDUINO_USB_CDC_ON_BOOT
#error "ESP32-S3-LCD-1.28: Board-Option USB CDC On Boot auf Disabled stellen / set the board option USB CDC On Boot to Disabled"
#endif

    // LittleFS liegt auf der Partition "ffat" dieses Schemas (LITTLEFS_PARTITION weiter unten)
    // LittleFS lives on this scheme's "ffat" partition (LITTLEFS_PARTITION further below)

#ifndef ARDUINO_PARTITION_app3M_fat9M_16MB
#error "ESP32-S3-LCD-1.28: Flash Size 16MB, Partition Scheme '16M Flash (3MB APP/9.9MB FATFS)' waehlen / choose it"
#endif

    // Waveshare ESP32-S3-LCD-1.28: Display (DC 8, CS 9, SCLK 10, MOSI 11, RST 12, BL 40), Lagesensor QMI8658
    // an I2C 6/7 (INT 47/48) und Akku-Messung an GPIO 1 fest verbaut, keine Board-LED. Lichtsensor, Taster
    // und DCF77 an freien Pins der Stiftleiste (ADC1, da ADC2 bei WLAN gesperrt; GPIO 3 ist Strapping-Pin).

    // Waveshare ESP32-S3-LCD-1.28: display (DC 8, CS 9, SCLK 10, MOSI 11, RST 12, BL 40), QMI8658 motion
    // sensor on I2C 6/7 (INT 47/48) and battery measurement on GPIO 1 built in, no board LED. Light sensor,
    // button and DCF77 on free header pins (ADC1, as ADC2 is blocked with WiFi; GPIO 3 is a strapping pin).

#define LED_BOARD      -1   // keine LED / no LED
#define LED_BOARD_GPIO -1

#define ADC_3V 2
#define ADC_PIN 4
#define ADC_GND 5

#define BUTTON1 16
#define BOOT_BUTTON 0

    // RTC DS3231 (0x68) am I2C des Lagesensors (0x6A/0x6B), Pull-ups sind auf dem Board
    // RTC DS3231 (0x68) on the motion sensor's I2C (0x6A/0x6B), pull-ups are on the board

#define SDA_PIN 6
#define SCL_PIN 7

#define TFT_SCLK  10
#define TFT_MOSI  11
#define TFT_DC    8
#define TFT_RST   12

#define CS_1    9
#define CS_2    -1   // zweites Display (noch) nicht vorgesehen
                     // second display not provided (yet)

    // CS schaltet LovyanGFX wie beim ESP32-C6 - ein Software-Neustart laesst das Panel so nicht schwarz
    // CS is driven by LovyanGFX as on the ESP32-C6 - so a software restart does not leave the panel black

#define LGFX_CS_PIN CS_1

#define DCF77_DATAPIN 17
#define TFT_Backlight 40

#else

    // Pinbelegung ESP32-S2 (Lolin S2 Pico) <-> TFT: 3.3V->VCC (rot), GND->GND (blau), Rest siehe
    // PCB-Referenz: https://github.com/holgiw/ESP32-Station-Clock/blob/master/PCB/ESP32-S2%20GC9A01.jpg

    // Pin mapping ESP32-S2 (Lolin S2 Pico) <-> TFT: 3.3V->VCC (red), GND->GND (blue), rest see PCB reference:
    // https://github.com/holgiw/ESP32-Station-Clock/blob/master/PCB/ESP32-S2%20GC9A01.jpg


#define LED_BOARD 15 // BUILTIN LED
#define LED_BOARD_GPIO LED_BOARD

#define ADC_3V 1
#define ADC_PIN 2
#define ADC_GND 4

#define BUTTON1 16
#define BOOT_BUTTON 0


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

    // SPI-CS Display 1 - manuell gesteuert (setCS1()/setCS2() in display.h),
    // LovyanGFX bekommt pin_cs = -1 (lgfx_config.h). Beide Displays haengen
    // an einem Geraet, der CS-Wechsel waehlt nur den Chip aus.

    // SPI CS for display 1 - driven manually (setCS1()/setCS2() in
    // display.h), LovyanGFX gets pin_cs = -1 (lgfx_config.h). Both displays
    // hang off one device, switching CS only selects the chip.

#define CS_1    12
#define LGFX_CS_PIN -1   // CS manuell, siehe oben / CS manual, see above

    // SPI-CS Display 2 (baugleich) - bei der Uhranzeige nur bedient, solange die Rotation von Display 2 nicht "n.a." ist.
    // SPI CS for display 2 (identical) - for the clock display only driven while display 2's rotation is not "n.a.".

#define CS_2    18

    // DCF77

#define DCF77_DATAPIN 35

    // Hintergrundbeleuchtung - ob der Pin per PWM geregelt wird, entscheidet die Einstellung useBacklight
    // (globals.h).

    // Backlight - whether the pin is PWM-controlled is decided by the useBacklight setting (globals.h).

#define TFT_Backlight 3  // Hintergrundbeleuchtung
                         // Backlight
#endif

    // Partition fuer LittleFS: ESP32-S3 mit Schema "16M Flash (3MB APP/9.9MB FATFS)" (zwei App-Partitionen fuer
    // OTA) - die Datenpartition heisst dort "ffat", LittleFS nutzt sie trotzdem. S2/C6 mit "No OTA": "spiffs".

    // Partition for LittleFS: ESP32-S3 with the scheme "16M Flash (3MB APP/9.9MB FATFS)" (two app partitions
    // for OTA) - the data partition is called "ffat" there, LittleFS uses it anyway. S2/C6 with "No OTA": "spiffs".

#if BOARD_WAVESHARE_S3_GC9A01
#define LITTLEFS_PARTITION "ffat"
#else
#define LITTLEFS_PARTITION "spiffs"
#endif

    // Firmware-Update ueber WLAN (ota_update.h) nur mit zwei App-Partitionen, also nur auf dem ESP32-S3.
    // Haengt ein Update, startet die Uhr nach OTA_TIMEOUT_MS neu, statt stehen zu bleiben.

    // Firmware update over WiFi (ota_update.h) only with two app partitions, so only on the ESP32-S3. If an
    // update hangs, the clock restarts after OTA_TIMEOUT_MS instead of standing still.

#define HAS_OTA BOARD_WAVESHARE_S3_GC9A01
#define OTA_TIMEOUT_MS (3 * WAIT_1m)
#define OTA_PORT 3232 // ArduinoOTA (Visual Micro, Arduino IDE)

#define HAS_DISPLAY2 (CS_2 >= 0)
#define TFT_SPI_FREQUENCY 40000000 // hoechster ganzzahliger Teiler von 80 MHz unter den frueheren 60 MHz
                                   // highest integer divider of 80 MHz below the former 60 MHz

    // Rotationswert "nicht angeschlossen (n.a.)": fuer die Uhranzeige wird das Display
    // dann weder angesteuert noch berechnet (Zifferblatt/Zeiger entfallen). Status- und
    // Startmeldungen (Boot, AP-Modus, Codes) erscheinen weiterhin immer auf beiden Displays.

    // Rotation value "not connected (n.a.)": for the clock display the display is then
    // neither driven nor calculated (face/hands are skipped). Status and boot messages
    // (boot, AP mode, codes) still always appear on both displays.

#define TFT_ROTATION_NA 4

    // Auswahlwert "automatisch" fuer Display 1 (nur mit Lagesensor, HAS_IMU): gespeichert wird er nicht als
    // Rotation, sondern als autoRotation (imu_rotation.h). IMU_ROTATION_DIR: Drehsinn des Sensors gegenueber
    // dem Display (1 oder -1, am Geraet pruefen), IMU_STABLE_MS: so lange muss eine neue Lage anliegen.

    // Selection value "automatic" for display 1 (only with a motion sensor, HAS_IMU): it is not stored as a
    // rotation but as autoRotation (imu_rotation.h). IMU_ROTATION_DIR: turning direction of the sensor relative
    // to the display (1 or -1, check on the device), IMU_STABLE_MS: a new position must last this long.

#define TFT_ROTATION_AUTO 5
#define HAS_IMU BOARD_WAVESHARE_S3_GC9A01
#define IMU_ROTATION_DIR 1
#define IMU_STABLE_MS 1500

    // Werkseinstellung: Display 1 angeschlossen (0 Grad), Display 2 nicht angeschlossen.
    // Factory default: display 1 connected (0 degrees), display 2 not connected.

#define TFT_ROTATION1_DEFAULT 0
#define TFT_ROTATION2_DEFAULT TFT_ROTATION_NA

#define BACKLIGHT_FREQ 5000
#define BACKLIGHT_RESOLUTION 8

    // PWM-Helligkeit, solange die Uhr beim Start auf Daten wartet (noch keine Zeit, WPS, Access Point): 50 %.
    // Auf dem ESP32-C6 warnt die Helligkeitsseite ab einem Maximum ueber BACKLIGHT_C6_MAX_SAFE - Waveshare
    // weist auf moegliche Ueberhitzung der Beleuchtung hin.

    // PWM brightness while the clock waits for data at boot (no time yet, WPS, access point): 50 %. On the
    // ESP32-C6 the brightness page warns from a maximum above BACKLIGHT_C6_MAX_SAFE - Waveshare points out
    // possible overheating of the backlight.

#define BACKLIGHT_SETUP_LEVEL 128
#define BACKLIGHT_C6_MAX_SAFE 128


    // Zeigerformat: Drehpunkte und Breiten je Display fest (Dateiformat), Drehpunkt-Spalte = halbe Breite.
    // Alte Zeiger werden oben und seitlich transparent aufgefuellt. handPivotY = Displayradius, der Zeiger
    // reicht bis zum Rand.

    // Hand format: pivots and widths fixed per display (file format), pivot column = half the width. Old
    // hands are padded transparent at the top and sides. handPivotY = display radius, the hand reaches the
    // edge.

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
    int panelWidth;        // Panel bei Rotation 0 - groesser als clock: Streifen fuer Uhrzeit/Datum
                           // panel at rotation 0 - larger than clock: strip for time/date
    int panelHeight;
    bool round;            // rundes Display: Ecken der Zifferblaetter weiss maskieren (scaleAndSaveBmp())
                           // round display: mask the clock face corners white (scaleAndSaveBmp())
};

constexpr DisplayGeometry DISPLAY_GEOMETRY[DISPLAY_TYPE_COUNT] = {

    // name       clock legW  W  legH legPiv piv text hub   BL     swRot  panelW panelH round

    // GC9A01: BL nur auf dem Waveshare ESP32-S3-LCD-1.28 ab Werk an (Beleuchtung dort fest an GPIO 40)
    // GC9A01: BL on by default only on the Waveshare ESP32-S3-LCD-1.28 (backlight hard-wired to GPIO 40 there)

    { "GC9A01",   240,  21,  25, 131, 100,  120,  2,  6,  BOARD_WAVESHARE_S3_GC9A01 != 0, false, 240, 240, true },
    { "GC9D01",   160,  13,  15,  86,  66,   80,  1,  3,  true,  true,  160,   160,   true },
    { "ILI9341",  240,  21,  25, 131, 100,  120,  2,  6,  false, false, 240,   320,   false },
    { "ST7789",   172,  15,  17,  94,  72,   86,  2,  4,  true,  false, 172,   320,   false }, // Meldungen quer (statusLandscape())
                                                                                        // messages in landscape (statusLandscape())
    { "ST7789_240", 240, 21, 25, 131, 100,  120,  2,  6,  true,  false, 240,   240,   false },
};

    // Obergrenzen ueber alle Typen - fuer fest dimensionierte Puffer
    // (rowBuffer in globals.h).

    // Upper bounds across all types - for fixed-size buffers (rowBuffer in globals.h).

#define CLOCK_MAX 240

constexpr bool displayGeometryValid(const DisplayGeometry& g) {
    return g.handPivotY >= g.handLegacyPivotY                                       // HAND_TOP_PAD >= 0
        && g.handWidth >= g.handLegacyWidth && (g.handWidth - g.handLegacyWidth) % 2 == 0 // Drehpunkt-Spalte bleibt mittig / pivot column stays centred
        && g.handPivotY <= g.clock / 2                                              // Drehpunkt innerhalb des Radius / pivot inside the radius
        && g.clock <= CLOCK_MAX
        && g.panelWidth >= g.clock && g.panelHeight >= g.clock;                     // Uhr passt aufs Panel / clock fits the panel
}
static_assert(displayGeometryValid(DISPLAY_GEOMETRY[DISPLAY_TYPE_GC9A01]) &&
              displayGeometryValid(DISPLAY_GEOMETRY[DISPLAY_TYPE_GC9D01]) &&
              displayGeometryValid(DISPLAY_GEOMETRY[DISPLAY_TYPE_ILI9341]) &&
              displayGeometryValid(DISPLAY_GEOMETRY[DISPLAY_TYPE_ST7789]) &&
              displayGeometryValid(DISPLAY_GEOMETRY[DISPLAY_TYPE_ST7789_240]),
              "DISPLAY_GEOMETRY: hand pivot/width/clock size inconsistent");

    // Diese Namen zeigen auf den zur Laufzeit gewaehlten Typ (displayGeom, globals.h) - daher NICHT in
    // Array-Groessen, static_assert oder #if verwenden.

    // These names point to the type selected at runtime (displayGeom, globals.h) - so do NOT use them in
    // array sizes, static_assert or #if.

#define CLOCK_WIDTH         (displayGeom->clock)
#define CLOCK_HEIGHT        (displayGeom->clock)
#define TFT_WIDTH           (displayGeom->panelWidth)
#define TFT_HEIGHT          (displayGeom->panelHeight)
#define HAND_LEGACY_WIDTH   (displayGeom->handLegacyWidth)
#define HAND_WIDTH          (displayGeom->handWidth)
#define HAND_LEGACY_HEIGHT  (displayGeom->handLegacyHeight)
#define HAND_LEGACY_PIVOT_Y (displayGeom->handLegacyPivotY)
#define HAND_PIVOT_Y        (displayGeom->handPivotY)
#define TFT_TEXT_SIZE       (displayGeom->textSize)
#define BACKLIGHT_DEFAULT   (displayGeom->backlightDefault)

#define HAND_TOP_PAD (HAND_PIVOT_Y - HAND_LEGACY_PIVOT_Y)          // 20 bzw. 14
                                                                   // 20 or 14
#define HAND_HEIGHT (HAND_LEGACY_HEIGHT + HAND_TOP_PAD)            // 151 bzw. 100
                                                                   // 151 or 100


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

    // Fuehrt einen Zeichenblock fuer Display 1 und 2 aus, korrekt rotiert (beginStatusDraw()/endStatusDraw())
    // - fuer BEIDE Displays, auch bei "n.a.", ausser das Board hat kein zweites. Makro, da schon vor display.h
    // benutzt (wifi_manager.h).

    // Runs a drawing block for display 1 and 2, correctly rotated (beginStatusDraw()/endStatusDraw()) - for
    // BOTH displays, even with "n.a.", unless the board has no second one. Macro since it is used before
    // display.h (wifi_manager.h).

#define DRAW_ON_BOTH_DISPLAYS(...) \
    do { \
        { lgfx::LovyanGFX& tft = beginStatusDraw(1); __VA_ARGS__ } \
        endStatusDraw(1); \
        if (HAS_DISPLAY2) { \
            { lgfx::LovyanGFX& tft = beginStatusDraw(2); __VA_ARGS__ } \
            endStatusDraw(2); \
        } \
        setCSIdle(); \
    } while (0)

    // GitHub-Repository - zentral hier, damit ein Fork/Umzug nur diese
    // Stelle statt mehrerer in webserver_routes.h aendern muss.

    // GitHub repository - kept centrally here, so a fork/move only needs
    // changing this spot instead of several in webserver_routes.h.

#define GITHUB_REPO_OWNER "holgiw"
#define GITHUB_REPO_NAME "ESP32-Station-Clock"
#define GITHUB_REPO_URL "https://github.com/" GITHUB_REPO_OWNER "/" GITHUB_REPO_NAME
#define GITHUB_API_CONTENTS_BASE "https://api.github.com/repos/" GITHUB_REPO_OWNER "/" GITHUB_REPO_NAME "/contents/graphic/"

    // Ordner der Zifferblaetter auf GitHub (graphic/240, graphic/160) - das ST7789 (172) nimmt die 240er,
    // /upload verkleinert sie auf seine Groesse.

    // Folder of the clock faces on GitHub (graphic/240, graphic/160) - the ST7789 (172) takes the 240 ones,
    // /upload scales them down to its size.

#define GITHUB_GRAPHIC_SIZE (CLOCK_WIDTH == 160 ? 160 : 240)

    // Zeit / NTP-Standardwerte & Timing-Makros
    // Time / NTP defaults & timing macros

    // Zeitserver & Zeitzone Standardwert
    // time server & timezone default

#define NTP_SERVER_1 "pool.ntp.org"
#define NTP_SERVER_2 "ptbtime1.ptb.de"
#define TIMEZONE_DEFAULT "CET-1CEST,M3.5.0,M10.5.0/3" // Mitteleuropaeische Zeit
                                                      // Central European Time

    // Versuche PRO NTP-Server, bevor setupNTP() zum naechsten wechselt - ein verlorenes UDP-Paket soll nicht
    // gleich als Fehlschlag zaehlen. configTzTime() wird je Versuch neu aufgerufen, sonst sendet der
    // SNTP-Client keine neue Anfrage.

    // Attempts PER NTP server before setupNTP() moves on - a single lost UDP packet should not count as a
    // failure. configTzTime() is called again per attempt, otherwise the SNTP client sends no new request.

#define NTP_SYNC_ATTEMPTS 2

    // Ab dieser Abweichung (Sekunden) wird die RTC bei einem NTP-/DCF77-Sync
    // ueberhaupt geschrieben (siehe rtcDriftSec() in time_sync.h) - kleinere
    // Differenzen sind normale Rundung, kein unnoetiger I2C-Schreibzugriff.

    // Above this deviation (seconds) the RTC is actually written on an NTP/
    // DCF77 sync (see rtcDriftSec() in time_sync.h) - smaller differences
    // are normal rounding, not worth an unnecessary I2C write.

#define RTC_UPDATE_MIN_DRIFT_SEC 2

    // Startzeit der Anzeige ohne Uhrzeit aus NTP/DCF77/RTC/USB (Uhrmacher-Stellung 10:10:30). Die Uhr laeuft
    // von dort weiter, bis eine echte Zeit kommt; die Systemzeit bleibt ungueltig (NTP-Server schweigt).

    // Display start time without a time from NTP/DCF77/RTC/USB (watchmaker position 10:10:30). The clock
    // keeps running from there until a real time arrives; the system time stays invalid (NTP server stays
    // silent).

#define START_TIME_HOUR 10
#define START_TIME_MIN  10
#define START_TIME_SEC  30

    // So lange zeigt das Display nach dem Start des Access Points dessen Zugangsdaten, danach laeuft die Uhr.
    // Der Access Point bleibt aktiv, ein kurzer Tasterdruck zeigt die Daten erneut.

    // For this long after starting the access point the display shows its credentials, then the clock runs.
    // The access point stays active, a short button press shows the credentials again.

#define AP_INFO_SHOW_MS (2 * WAIT_1m)

    // Versuche PRO WLAN-Netz beim Start (connectWiFiAtBoot()), bevor das naechste versucht bzw. aufgegeben
    // wird (WPS/AP) - ein einzelner Fehlversuch soll ein gefundenes Netz nicht gleich verwerfen.

    // Attempts PER WiFi network at boot (connectWiFiAtBoot()), before trying the next one or giving up
    // (WPS/AP) - a single failed attempt should not discard a network that was found.

#define WIFI_CONNECT_ATTEMPTS 2

    // Access Point (Einrichtung): SSID und Passwort fest in der Firmware - stehen in Anleitung und auf dem
    // Display, das klappt auch ohne lesbares Display. Auf jeder Uhr gleich; eigenes Passwort hier setzen
    // (WPA2: 8-63 Zeichen).

    // Access point (setup): SSID and password fixed in the firmware - they are in the manual and on the
    // display, this works even without a readable display. Same on every clock; set your own password here
    // (WPA2: 8-63 characters).

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
#define WAIT_1h 3600000 // 1 Stunde in Millisekunden
                        // 1 hour in milliseconds
#define WAIT_6h 21600000 // 6 Stunden in Millisekunden
                         // 6 hours in milliseconds

    // Rocrail-Modellzeit: der TCP-Port des Servers ist praktisch immer 8051. Verbindungsversuch minuetlich,
    // in eigener Task (rocrailConnectTaskFunc()) - ein nicht erreichbarer Server blockiert loop()/Webserver
    // nicht.

    // Rocrail model time: the server's TCP port is practically always 8051. Connection attempt once a minute,
    // in its own task (rocrailConnectTaskFunc()) - an unreachable server does not block loop()/the web
    // server.

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

    // Diagnose: R2RNet-Multicast mithoeren und jedes Paket unveraendert loggen (startR2rnetDebugListener()) -
    // nur zur Analyse. 224.0.1.20:8051 ist laut wiki.rocrail.net die R2RNet-Adresse.

    // Diagnostic: listen on the R2RNet multicast and log every packet unchanged (startR2rnetDebugListener())
    // - analysis only. 224.0.1.20:8051 is the R2RNet address per wiki.rocrail.net.

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

    // Rauschfilter fuer den Bit-Fortschritt (processDcf77Bits()): Flanken unter diesem Wert (Prellen, echte
    // Zustaende dauern >= ~100 ms) werden verworfen, ohne den Referenzzeitpunkt zu verschieben.

    // Noise filter for the bit progress (processDcf77Bits()): edges below this value (bounce, genuine states
    // last >= ~100 ms) are discarded without shifting the reference timestamp.

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

    // Maximalalter des letzten dekodierten Telegramms als Zeitquelle (applyDcf77DecodedTime()) - die
    // verstrichene Zeit wird ueber millis() exakt nachgerechnet.

    // Max age of the last decoded telegram as a time source (applyDcf77DecodedTime()) - the elapsed time is
    // added back precisely via millis().

#define DCF77_DECODED_MAX_AGE (10 * WAIT_1m)

    // Dauer des LED-Blitzes je DCF77-Impuls - ein fester Blitz, damit der Endzustand nicht von der Impulsanzahl
    // abhaengt.

    // Duration of the LED flash per DCF77 pulse - a fixed flash, so the final state does not depend on the pulse
    // count.

#define DCF77_LED_BLINK_MS 80

    // Geschwindigkeit des Sekundenzeigers im Bahnhofsuhr-Modus: 60*975ms =
    // 58,5s Umlauf (Original-Hilfiker-Wert), Rest der Minute ruht der Zeiger
    // oben auf der 12. Wert fliesst auch in die Web-Live-Vorschau ein.

    // Second hand speed in station-clock mode: 60*975ms = 58.5s per sweep
    // (original Hilfiker value), the hand rests at the top for the rest of
    // the minute. Also used by the web live preview.

#define FAST_SECOND 975.0f

    // Hoehe der scrollbaren Textfenster im Log-Tab und auf der Info-Seite -
    // an EINER Stelle, damit beide Fenster gleich hoch bleiben. vh statt
    // fester Pixel: reicht auf grossen Monitoren weiter runter, min-height haelt es auf Handys trotzdem gross genug.

    // Height of the scrollable text windows in the Log tab and on the info
    // page - in ONE place, so both windows stay the same height. vh instead
    // of fixed pixels: reaches further down on large monitors, min-height still keeps it usable on phones.

#define INFO_LOG_WINDOW_HEIGHT_CSS "height:72vh;min-height:400px;"

    // Transparent in R5G6B5 RGB(16)

#define TRANSPARENT_COLOR 0x0120

