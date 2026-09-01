#pragma once
    // ### Board-/Display-Konfiguration, Pin-Belegung, Timing-Makros ######
    // Sortiert nach Modul: System/Debug, Zeit/Timing, Board-Auswahl & Pins, Display-Dimensionen.
    // Hinweis: tft/webserver/preferences/dnsServer/udp/rtc/DCF77-Variablen liegen AUSSCHLIESSLICH in globals.h (nicht duplizieren - fruehere "redefinition"-Ursache).

    // ### Board/display config, pin mapping, timing macros ######
    // Sorted by module: system/debug, time/timing, board selection & pins, display dimensions.
    // Note: tft/webserver/preferences/dnsServer/udp/rtc/DCF77 variables live ONLY in globals.h (do not duplicate - caused past "redefinition" errors).

    // --- System / Debug ---
#define DEBUG_PRINT(x)    { if (loggingEnabled) { Serial.print(x);   logToFile(String(x));}}
#define DEBUG_PRINTLN(x)  { if (loggingEnabled) { Serial.println(x); logToFile(String(x));}}
#define DEBUG_PRINTF(...) { if (loggingEnabled) { char buffer[128]; snprintf(buffer, sizeof(buffer), __VA_ARGS__); Serial.print(buffer); logToFile(String(buffer));}}

    // Schwellwert fuer Heap-Warnungen (siehe checkHeapWarning() in system_utils.h):
    // faellt der freie Heap darunter, wird eine Log-Zeile mit Kontext geschrieben,
    // statt es erst spaeter ueber /status zu bemerken.

    // Heap warning threshold (see checkHeapWarning() in system_utils.h): logs a
    // line with context if free heap drops below it, instead of only noticing
    // later via /status.
#define HEAP_WARNING_THRESHOLD 20480 // 20 KB

    // Fuehrt einen Block aus Zeichenbefehlen einmal fuer Display 1 und einmal
    // fuer Display 2 aus, damit Status-/Boot-Texte (Start, Reboot, SSID, WPS,
    // Factory Reset, ...), die direkt auf 'tft' zeichnen statt ueber das
    // Sprite-System zu laufen (siehe renderClockFrame()/updateClock() in
    // display.h), auf BEIDEN Displays erscheinen - UND dabei jeweils gemaess
    // tftRotation1/tftRotation2 korrekt gedreht sind.
    //
    // Das eigentliche Zeichnen (inkl. Auswahl von CS1/CS2 und - falls noetig -
    // der Rotation) uebernehmen beginStatusDraw()/endStatusDraw() (display.h):
    // je nach Board-Rotationsmodus zeichnet der Block entweder direkt auf den
    // physischen Chip (Hardware-Rotation) oder in ein Sprite mit eigener
    // Rotation (GC9D01-Software-Rotations-Workaround) - siehe dortige
    // Kommentare fuer Details. 'tft' wird dazu innerhalb des jeweiligen Blocks
    // lokal auf das von beginStatusDraw() gelieferte Objekt umgebogen, sodass
    // bestehender Code (der 'tft.xxx(...)' aufruft) unveraendert weiterlaeuft.
    // Hinterlaesst CS1 als aktiv (definierter Zustand fuer nachfolgenden Code).
    //
    // Als Makro statt Funktion, damit beginStatusDraw()/endStatusDraw() (erst
    // in display.h definiert) unabhaengig von der #include-Reihenfolge nutzbar
    // sind - wird bereits in wifi_manager.h verwendet, das vor display.h
    // eingebunden wird. __VA_ARGS__ statt eines einzelnen Parameters, damit
    // Kommas innerhalb des Blocks (z.B. in tft.printf(...)) den Aufruf nicht in
    // mehrere Makro-Argumente aufspalten. Der Block wird in eigene {}-Bloecke
    // gesetzt, damit die zwei lokalen 'tft'-Referenzen (fuer Display 1 und 2)
    // sich nicht gegenseitig als Neudefinition im selben Gueltigkeitsbereich
    // stoeren.

    // Runs a block of drawing commands once for Display 1 and once for
    // Display 2, so status/boot text (start, reboot, SSID, WPS, factory
    // reset, ...) that draws directly to 'tft' instead of going through the
    // sprite system (see renderClockFrame()/updateClock() in display.h)
    // appears on BOTH displays - AND is correctly rotated per
    // tftRotation1/tftRotation2 while doing so.
    //
    // The actual drawing (including selecting CS1/CS2 and - if needed -
    // rotation) is handled by beginStatusDraw()/endStatusDraw() (display.h):
    // depending on the board's rotation mode, the block either draws directly
    // to the physical chip (hardware rotation) or into a sprite with its own
    // rotation (GC9D01 software rotation workaround) - see the comments there
    // for details. 'tft' is locally shadowed within each block to the object
    // beginStatusDraw() returns, so existing code (which calls
    // 'tft.xxx(...)') keeps working unchanged. Leaves CS1 selected (defined
    // state for subsequent code).
    //
    // Implemented as a macro instead of a function so beginStatusDraw()/
    // endStatusDraw() (only defined in display.h) can be used regardless of
    // #include order - it's already used in wifi_manager.h, which is included
    // before display.h. __VA_ARGS__ instead of a single parameter so commas
    // inside the block (e.g. in tft.printf(...)) don't split the call into
    // multiple macro arguments. The block is wrapped in its own {} scopes so
    // the two local 'tft' references (for Display 1 and 2) don't clash as a
    // redefinition within the same scope.
#define DRAW_ON_BOTH_DISPLAYS(...) \
    do { \
        { TFT_eSPI& tft = beginStatusDraw(1); __VA_ARGS__ } \
        endStatusDraw(1); \
        { TFT_eSPI& tft = beginStatusDraw(2); __VA_ARGS__ } \
        endStatusDraw(2); \
        setCS1(LOW); \
    } while (0)

    // --- GitHub-Repository (Projektseite, Zusatz-Zifferblaetter/Zeigersaetze) ---
    // Zentral an einer Stelle, damit bei einem Fork/Umzug des Repos nur hier
    // geaendert werden muss statt an mehreren Stellen in webserver_routes.h.

    // --- GitHub repository (project page, extra dials/hand sets) ---
    // Kept in one place so a fork/repo move only needs changing here instead
    // of in multiple spots in webserver_routes.h.
#define GITHUB_REPO_OWNER "holgiw"
#define GITHUB_REPO_NAME "TFT-Clock-GC9A01"
#define GITHUB_REPO_URL "https://github.com/" GITHUB_REPO_OWNER "/" GITHUB_REPO_NAME
#define GITHUB_API_CONTENTS_BASE "https://api.github.com/repos/" GITHUB_REPO_OWNER "/" GITHUB_REPO_NAME "/contents/graphic/"
#define GITHUB_ZIP_BASE "https://github.com/" GITHUB_REPO_OWNER "/" GITHUB_REPO_NAME "/blob/master/graphic/"
#define GITHUB_RAW_BASE "https://raw.githubusercontent.com/" GITHUB_REPO_OWNER "/" GITHUB_REPO_NAME "/master/"

    // --- Zeit / NTP-Standardwerte & Timing-Makros ---
    // --- Time / NTP defaults & timing macros ---
    // Zeitserver & Zeitzone Standardwert
    // time server & timezone default
#define NTP_SERVER_1 "pool.ntp.org"
#define NTP_SERVER_2 "ptbtime1.ptb.de"
#define TIMEZONE_DEFAULT "CET-1CEST,M3.5.0,M10.5.0/3" // Mitteleuropaeische Zeit
                                                      // Central European Time

#define DEFAULT_PING_SERVER "1.1.1.1:80"

    // --- Access-Point (Einrichtungsmodus) ---
    // Die SSID ist bewusst auf allen Uhren gleich und fest: sie steht in der
    // Anleitung und der Nutzer sucht danach. Das PASSWORT wird dagegen zur
    // Laufzeit aus den letzten vier Bytes der MAC-Adresse gebildet (siehe
    // startAP() in wifi_manager.h) und ist damit pro Geraet verschieden.
    // Frueher war das Passwort mit der SSID identisch ("clock123") - also auf
    // jedem Geraet dasselbe und allgemein bekannt, womit jeder in Funkreichweite
    // die Weboberflaeche samt gespeicherter WLAN-Zugangsdaten oeffnen konnte.

    // --- Access point (setup mode) ---
    // The SSID is deliberately the same and fixed on all clocks: it is in the
    // manual and that is what the user looks for. The PASSWORD, in contrast, is
    // derived at runtime from the last four bytes of the MAC address (see
    // startAP() in wifi_manager.h) and therefore differs per device. It used to
    // be identical to the SSID ("clock123") - the same on every device and
    // publicly known, which let anyone in radio range open the web interface
    // including the stored WiFi credentials.
#define AP_SSID "clock123"

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
#define WAIT_30m 1800000 // 30 Minuten in Millisekunden
                         // 30 minutes in milliseconds
#define WAIT_1h 3600000 // 1 Stunde in Millisekunden
                        // 1 hour in milliseconds
#define WAIT_6h 21600000 // 6 Stunden in Millisekunden
                         // 6 hours in milliseconds

    // DCF77 uebernimmt die Systemzeit nur, wenn NTP seit mindestens dieser Zeit
    // nicht mehr synchronisiert hat (bzw. kein WLAN verbunden ist) - verhindert,
    // dass ein kurzer NTP-Ausfall sofort durch DCF77 "ueberschrieben" wird.

    // DCF77 only takes over the system time once NTP has not synced for at
    // least this long (or no WiFi is connected) - prevents a brief NTP
    // outage from immediately being "overwritten" by DCF77.
#define DCF77_NTP_GRACE_PERIOD (2 * WAIT_1h)

    // Geschwindigkeit des Sekundenzeigers im Train-Station-Mode gegenueber der
    // realen Sekunde (in Millisekunden) - der Zeiger "eilt" etwas voraus und
    // verweilt dann kurz auf der 60, wie bei einer klassischen Bahnhofsuhr.

    // Speed of the second hand in train-station mode vs. a real second (in
    // milliseconds) - the hand runs slightly ahead and pauses briefly at 60,
    // like a classic railway station clock.
    // Dauer einer Sekundenteilung im Bahnhofsuhr-Modus. 60 * 975 ms = 58,5 s
    // fuer einen Umlauf - das ist der Wert der originalen Hilfiker-Uhr, die
    // restlichen rund 1,5 s der echten Minute steht der Zeiger oben auf der 12
    // und wartet auf den Minutenimpuls (siehe renderClockFrame() in display.h).
    // Vorher standen hier 972 ms, also 58,32 s.
    //
    // Der Wert wird auch in die Live-Vorschau der Weboberflaeche uebernommen
    // (webserver_routes.h), damit Vorschau und Uhr identisch laufen.

    // Duration of one second division in station clock mode. 60 * 975 ms =
    // 58.5 s per sweep - the value of the original Hilfiker clock; for the
    // remaining ~1.5 s of the real minute the hand rests at the top on the 12
    // waiting for the minute pulse (see renderClockFrame() in display.h).
    // This used to be 972 ms, i.e. 58.32 s.
    //
    // The value is also passed into the web interface's live preview
    // (webserver_routes.h) so that preview and clock run identically.
#define FAST_SECOND 975.0f

    // --- Board-Auswahl (Prozessor, TFT-Typ) ---
    // --- Board selection (processor, TFT type) ---
    // Prozessor
    // Processor
#define ESP32_S2  //nur ESP32-S2 unterstuetzt
                  // only ESP32-S2 supported

    // TFT auswaehlen
    // select TFT
#define GC9A01
    //#define GC9A01_WITH_BACKLIGHT
    //#define GC9D01
    //#define ILI9341 // DEPRECATED - nicht mehr aktiv gepflegt, GC9A01 wird bevorzugt / DEPRECATED - no longer maintained, GC9A01 is preferred

    // --- Pin-Belegung: ESP32-S2 (Lolin S2 Pico) ---
    // --- Pin mapping: ESP32-S2 (Lolin S2 Pico) ---
#ifdef ESP32_S2  // Lolin S2 Pico
    // Pinbelegung ESP32<->TFT: 3.3V->vcc (rot), GND->gnd (blau), weitere siehe Referenzschaltplan.
    // PCB-Referenz: https://github.com/holgiw/TFT-Clock-GC9A01/blob/master/PCB/ESP32-S2%20GC9A01.jpg

    // ESP32<->TFT pinout: 3.3V->vcc (red), GND->gnd (blue), see reference schematic for the rest.
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

    // SPI Chipselect für Display 1 - wird MANUELL vom Sketch gesteuert (siehe
    // setCS1()/setCS2() in display.h), NICHT mehr automatisch von der
    // TFT_eSPI-Bibliothek. Derselbe physische Pin, der vorher als TFT_CS lief -
    // nur jetzt manuell statt automatisch angesteuert. Dafuer MUSS TFT_CS in der
    // Referenzkonfiguration weiter unten (und in der tatsaechlichen User_Setup.h
    // der Bibliothek!) auf -1 gesetzt sein. Grund: die Bibliothek wuerde ihren
    // eigenen CS-Pin sonst bei JEDER SPI-Uebertragung automatisch mit ansteuern -
    // unabhaengig vom Zustand von CS_2 - wodurch sich Display 1 und Display 2 bei
    // unterschiedlichen Rotationen gegenseitig die zuletzt gesendeten Bilddaten
    // "stehlen" wuerden.

    // SPI chip-select for display 1 - now driven MANUALLY by the sketch (see
    // setCS1()/setCS2() in display.h), no longer automatically by the TFT_eSPI
    // library. Same physical pin that used to be TFT_CS - just now driven
    // manually instead of automatically. For this, TFT_CS in the reference
    // config further below (and in the library's actual User_Setup.h!) MUST be
    // set to -1. Reason: otherwise the library would still drive its own CS pin
    // automatically on EVERY SPI transfer - regardless of CS_2's state - which
    // would cause display 1 and display 2 to "steal" each other's last-sent
    // frame whenever they have different rotations.
#define CS_1    12

    // SPI Chipselect für Display 2 (baugleich mit Display 1) - der Pin wird
    // immer eingebunden UND immer angesteuert (Display 2 ist fest aktiviert,
    // kein Laufzeit-Umschalter mehr), siehe uhr3.ino und display.h.

    // SPI chip-select for Display 2 (identical to Display 1) - the pin is
    // always compiled in AND always driven (Display 2 is permanently
    // enabled, no more runtime toggle), see uhr3.ino and display.h.
#define CS_2    18

    // DCF77
#define DCF77_INTERRUPT 0
#define DCF77_DATAPIN 35

#if defined (GC9D01)  || defined(GC9A01_WITH_BACKLIGHT)
#define TFT_Backlight 3  // Hintergrundbeleuchtung
                         // Backlight
#define BACKLIGHT_CHANNEL 0  // PWM-Kanal
                             // PWM channel
#define BACKLIGHT_FREQ 5000
#define BACKLIGHT_RESOLUTION 8
#endif

#endif

    // --- Display: Dimensionen je Zifferblatt-Typ ---
    // --- Display: dimensions per dial type ---
#if defined GC9A01 || defined(GC9A01_WITH_BACKLIGHT)
#include "graphic/240/clock_default.h"

#define ROUND_DISPLAY // Rundes Display - Kreismaskierung der Zifferblatt-Ecken anwenden (siehe scaleAndSaveBmp() in display.h)
                      // Round display - apply circular masking to the dial corners (see scaleAndSaveBmp() in display.h)
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

    // --- TFT_eSPI reference config (Arduino IDE!) ---
    // IMPORTANT: These #defines do NOT automatically affect the TFT_eSPI
    // library itself - it compiles its own .cpp files separately from the
    // sketch, so a #define here won't reach it. These values are a
    // documented reference but MUST ALSO be copied into the library ONCE,
    // e.g. by redirecting the line
    //   #include <User_Setup.h>
    // in <ArduinoLibraryFolder>/TFT_eSPI/User_Setup_Select.h to this block
    // (e.g. via an absolute path to an extracted copy of these lines). A
    // library update/reinstall requires re-adding that ONE redirect, but
    // the actual values stay here in the project.
#define GC9A01_DRIVER
#define TFT_MOSI  6
#define TFT_SCLK  4
#define TFT_CS    -1  // WICHTIG: manuelle CS-Steuerung aktiv (siehe CS_1 weiter oben) -
                      // die Bibliothek darf ihren CS-Pin hier NICHT mehr selbst
                      // automatisch schalten. In der tatsaechlichen User_Setup.h der
                      // Bibliothek MUSS dieser Wert ebenfalls -1 sein.

                      // IMPORTANT: manual CS control is active (see CS_1 above) - the
                      // library must NOT drive its CS pin automatically here anymore.
                      // The library's actual User_Setup.h MUST also have this set to -1.
#define TFT_DC    10  // Data/Command
#define TFT_RST   0   // Reset
#define TFT_BL    5   // Hintergrundbeleuchtung (Bibliotheks-eigene Steuerung -
                      // das Projekt nutzt zusaetzlich Pin 3 fuer eigene PWM-
                      // Helligkeitsregelung, siehe TFT_Backlight weiter unten)

                      // Backlight (library's own control - the project also
                      // uses pin 3 for its own PWM brightness control, see
                      // TFT_Backlight further below)
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
                      // Round display - apply circular masking to the dial corners (see scaleAndSaveBmp() in display.h)
#define TFT_WIDTH 160
#define TFT_HEIGHT 160

#define CLOCK_WIDTH 160
#define CLOCK_HEIGHT 160

#define HAND_WIDTH 13
#define HAND_HEIGHT 86

#define TFT_TEXT_SIZE 1

    // GC9D01 wird elektrisch/treiberseitig wie GC9A01 angesteuert (gleicher
    // Treiber, gleiche Pinbelegung) - daher hier dieselben TFT_eSPI-Pin-/
    // Treibereinstellungen wie im GC9A01-Block oben, nur mit den
    // GC9D01-spezifischen Aufloesungs-/Zeiger-Massen oben in diesem Block.

    // GC9D01 is driven electrically/at the driver level the same as GC9A01
    // (same driver, same pinout) - so the same TFT_eSPI pin/driver settings
    // as in the GC9A01 block above apply here, only with the GC9D01-specific
    // resolution/hand dimensions set earlier in this block.
#define GC9A01_DRIVER
#define TFT_MOSI  6
#define TFT_SCLK  4
#define TFT_CS    -1  // WICHTIG: manuelle CS-Steuerung aktiv (siehe CS_1 weiter oben) -
                      // die Bibliothek darf ihren CS-Pin hier NICHT mehr selbst
                      // automatisch schalten. In der tatsaechlichen User_Setup.h der
                      // Bibliothek MUSS dieser Wert ebenfalls -1 sein.

                      // IMPORTANT: manual CS control is active (see CS_1 above) - the
                      // library must NOT drive its CS pin automatically here anymore.
                      // The library's actual User_Setup.h MUST also have this set to -1.
#define TFT_DC    10  // Data/Command
#define TFT_RST   0   // Reset
#define TFT_BL    5   // Hintergrundbeleuchtung (Bibliotheks-eigene Steuerung -
                      // das Projekt nutzt zusaetzlich Pin 3 fuer eigene PWM-
                      // Helligkeitsregelung, siehe TFT_Backlight weiter unten)

                      // Backlight (library's own control - the project also
                      // uses pin 3 for its own PWM brightness control, see
                      // TFT_Backlight further below)
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

#ifdef ILI9341 // DEPRECATED - nicht mehr aktiv gepflegt
               // DEPRECATED - no longer maintained
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

    // NOTE: As with the GC9D01 block, TFT_eSPI pin/driver settings are also
    // missing here - if using this (already deprecated) board, add them
    // analogous to the GC9A01 block above.
#endif

    // --- Web: Live-Vorschau (/preview-Route) ---
    // Groesse (Breite/Hoehe in Pixeln) der grossen Live-Zeiger-Vorschau auf der
    // /preview-Webseite (siehe webserver_routes.h) - zentral hier hinterlegt
    // statt als magische Zahl im Routencode, damit sich die Groesse zum Testen
    // an einer Stelle aendern laesst.

    // --- Web: live preview (/preview route) ---
    // Size (width/height in pixels) of the large live hand preview on the
    // /preview page (see webserver_routes.h) - kept here centrally instead
    // of a magic number in route code, so it can be changed for testing in
    // one place.
#define LIVE_PREVIEW_SIZE 400

    // Transparent in R5G6B5 RGB(16)
#define TRANSPARENT_COLOR 0x0120
