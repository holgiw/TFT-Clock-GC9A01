#pragma once
    // Reihenfolge: zuerst Prozessor-/Display-Auswahl (Pins, Display-Masse),
    // danach die restlichen Werte nach Modul sortiert.
    // tft/webserver/preferences/dnsServer/udp/rtc/DCF77-Variablen: nur in globals.h.

    // Order: processor/display selection (pins, display dimensions) first,
    // then the rest sorted by module.
    // tft/webserver/preferences/dnsServer/udp/rtc/DCF77 variables: only in globals.h.

    // Board-Auswahl (Prozessor, TFT-Typ)
    // Board selection (processor, TFT type)

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

    // SPI-CS Display 1 - manuell gesteuert (setCS1()/setCS2() in display.h),
    // nicht mehr automatisch von TFT_eSPI. Muss deshalb hier UND in

    // User_Setup.h der Bibliothek auf -1 stehen, sonst "stehlen" sich beide
    // Displays bei unterschiedlicher Rotation gegenseitig Bilddaten.

    // SPI CS for display 1 - driven manually (setCS1()/setCS2() in
    // display.h), no longer automatically by TFT_eSPI. Must therefore be -1

    // here AND in the library's User_Setup.h, otherwise the two displays
    // "steal" each other's frame data when rotated differently.
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

#if defined (GC9D01)  || defined(GC9A01_WITH_BACKLIGHT)
#define TFT_Backlight 3  // Hintergrundbeleuchtung
                         // Backlight
#define BACKLIGHT_CHANNEL 0  // PWM-Kanal
                             // PWM channel
#define BACKLIGHT_FREQ 5000
#define BACKLIGHT_RESOLUTION 8
#endif

#endif

    // Display: Dimensionen je Zifferblatt-Typ
    // Display: dimensions per dial type
#if defined GC9A01 || defined(GC9A01_WITH_BACKLIGHT)
#include "graphic/240/clock_default.h"

#define ROUND_DISPLAY // rundes Display - Kreismaskierung der Ecken (siehe scaleAndSaveBmp() in display.h)
                      // round display - circular corner masking (see scaleAndSaveBmp() in display.h)
#define TFT_WIDTH 240
#define TFT_HEIGHT 240

#define CLOCK_WIDTH 240
#define CLOCK_HEIGHT 240

#define HAND_WIDTH 21
#define HAND_HEIGHT 131

#define TFT_TEXT_SIZE 2

    // TFT_eSPI-Referenzkonfiguration (Arduino IDE!) - wirkt nicht automatisch
    // auf die Bibliothek; muss einmalig in deren User_Setup_Select.h
    // eingebunden werden (Redirect auf diesen Block).

    // TFT_eSPI reference config (Arduino IDE!) - does not automatically
    // affect the library; must be included once into its
    // User_Setup_Select.h (redirect to this block).
#define GC9A01_DRIVER
#define TFT_MOSI  6
#define TFT_SCLK  4
#define TFT_CS    -1  // manuelle CS-Steuerung aktiv (siehe CS_1 oben) - muss auch in User_Setup.h -1 sein
                      // manual CS control active (see CS_1 above) - must also be -1 in User_Setup.h
#define TFT_DC    10  // Data/Command
#define TFT_RST   0   // Reset
#define TFT_BL    5   // Backlight (Bibliothekseigene Steuerung, zusaetzlich zu Pin 3/TFT_Backlight fuer eigene PWM-Helligkeit)
                      // Backlight (library's own control, in addition to pin 3/TFT_Backlight for our own PWM brightness)
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

#define ROUND_DISPLAY // rundes Display - Kreismaskierung der Ecken (siehe scaleAndSaveBmp() in display.h)
                      // round display - circular corner masking (see scaleAndSaveBmp() in display.h)
#define TFT_WIDTH 160
#define TFT_HEIGHT 160

#define CLOCK_WIDTH 160
#define CLOCK_HEIGHT 160

#define HAND_WIDTH 13
#define HAND_HEIGHT 86

#define TFT_TEXT_SIZE 1

    // GC9D01 wird elektrisch/treiberseitig wie GC9A01 angesteuert - Pin-/
    // Treibereinstellungen daher identisch zum GC9A01-Block oben.

    // GC9D01 is driven electrically/at the driver level like GC9A01 - pin/
    // driver settings therefore identical to the GC9A01 block above.
#define GC9A01_DRIVER
#define TFT_MOSI  6
#define TFT_SCLK  4
#define TFT_CS    -1
#define TFT_DC    10
#define TFT_RST   0
#define TFT_BL    5
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

    // TFT_eSPI-Pin-/Treibereinstellungen fehlen hier (deprecated) - bei
    // Bedarf analog zum GC9A01-Block oben ergaenzen.

    // TFT_eSPI pin/driver settings are missing here (deprecated) - add
    // analogous to the GC9A01 block above if needed.
#endif


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
        { TFT_eSPI& tft = beginStatusDraw(1); __VA_ARGS__ } \
        endStatusDraw(1); \
        { TFT_eSPI& tft = beginStatusDraw(2); __VA_ARGS__ } \
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

    // Versuche PRO WLAN-Netzwerk beim Boot (siehe connectWiFiAtBoot() in
    // uhr3.ino), bevor mit dem naechsten Netzwerk weitergemacht bzw. ganz
    // aufgegeben wird (-> WPS/Access-Point) - ein einzelner fehlgeschlagener
    // Verbindungsversuch (z.B. Router kurz beschaeftigt) soll das gefundene
    // Netzwerk nicht gleich verwerfen.

    // Attempts PER WiFi network at boot (see connectWiFiAtBoot() in
    // uhr3.ino), before moving on to the next network or giving up entirely
    // (-> WPS/access point) - a single failed connection attempt (e.g. the
    // router being briefly busy) shouldn't discard a network that was found.
#define WIFI_CONNECT_ATTEMPTS 2

    // Access-Point (Einrichtungsmodus): SSID ist bewusst fest/gleich (steht
    // in der Anleitung); Passwort wird pro Geraet aus den letzten 4 MAC-Bytes
    // gebildet (startAP() in wifi_manager.h) statt wie frueher fest "clock123".

    // Access point (setup mode): SSID is deliberately fixed/identical (it's
    // in the manual); password is derived per device from the last 4 MAC
    // bytes (startAP() in wifi_manager.h) instead of the old fixed "clock123".
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

    // Ab diesem Divider werden Sekundenzeiger UND Nabe ausgeblendet (siehe
    // renderClockFrame()) - bei so hoher Beschleunigung ist ihre Bewegung/
    // Sichtbarkeit ohnehin kaum noch sinnvoll.

    // From this divider onwards, the second hand AND the hub are hidden
    // (see renderClockFrame()) - at such high acceleration their movement/
    // visibility isn't meaningfully useful anyway.
#define ROCRAIL_HIDE_DETAILS_DIVIDER 10

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

    // Diagnostic: listen on the R2RNet multicast group and log every
    // received packet unchanged (see startR2rnetDebugListener() in
    // rocrail_client.h). ONLY for analyzing the reply format - actual
    // R2RNet discovery was removed (see the comment at the top of
    // rocrail_client.h).
#define R2RNET_DEBUG_MULTICAST_IP "224.0.0.1"
#define R2RNET_DEBUG_MULTICAST_PORT 4321
#define R2RNET_DEBUG_PACKET_BUFFER_SIZE 512

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

    // Dauer des LED-Aufblitzens pro DCF77-Impuls (loop() in uhr3.ino) -
    // bewusst ein Blitz mit fester Abschaltzeit statt toggleLED(), da sonst
    // der Endzustand von der (geraden/ungeraden) Impulsanzahl abhinge.

    // Duration of the LED flash per DCF77 pulse (loop() in uhr3.ino) -
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
