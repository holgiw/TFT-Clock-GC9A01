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

    // Uebersichtsseite aller Projekte des Autors, Schaltplan/Platine und
    // Kontaktadresse - werden auf der Info-Seite (/info in webserver_routes.h)
    // verlinkt. Bewusst hier bei den uebrigen Projektadressen statt im
    // Routencode: bei einem Fork oder Umzug muss so nur diese eine Stelle
    // angepasst werden.

    // Overview page of all the author's projects, the schematic/PCB and the
    // contact address - linked on the info page (/info in
    // webserver_routes.h). Deliberately kept here with the other project
    // addresses instead of in the route code: on a fork or a move only this
    // one place needs adjusting.
#define GITHUB_AUTHOR_URL "https://github.com/" GITHUB_REPO_OWNER "?tab=repositories"
#define GITHUB_GRAPHIC_URL GITHUB_REPO_URL "/tree/master/graphic"
#define GITHUB_PCB_URL GITHUB_REPO_URL "/blob/master/PCB/ESP32-S2%20GC9A01.jpg"
#define PROJECT_CONTACT_MAIL "howl@gmx.de"

    // --- Zeit / NTP-Standardwerte & Timing-Makros ---
    // --- Time / NTP defaults & timing macros ---
    // Zeitserver & Zeitzone Standardwert
    // time server & timezone default
#define NTP_SERVER_1 "pool.ntp.org"
#define NTP_SERVER_2 "ptbtime1.ptb.de"
#define TIMEZONE_DEFAULT "CET-1CEST,M3.5.0,M10.5.0/3" // Mitteleuropaeische Zeit
                                                      // Central European Time

    // Anzahl Versuche PRO konfiguriertem NTP-Server, bevor setupNTP() zum
    // naechsten Server weiterspringt (siehe setupNTP() in time_sync.h). Ein
    // einzelnes verlorenes UDP-Antwortpaket (bei oeffentlichen Pool-Servern
    // wie pool.ntp.org gelegentlich normal) fuehrte bisher sofort zu
    // "[NTP] Failed to synchronize", obwohl der Server an sich erreichbar
    // war. configTzTime() wird pro Versuch NEU aufgerufen (nicht nur
    // getLocalTime() wiederholt), da der zugrundeliegende SNTP-Client sonst
    // keine neue Anfrage verschickt - sein eigener interner Retry-Abstand
    // liegt deutlich ueber WAIT_3s.

    // Number of attempts PER configured NTP server before setupNTP() moves
    // on to the next one (see setupNTP() in time_sync.h). A single lost UDP
    // response packet (occasionally normal with public pool servers like
    // pool.ntp.org) previously caused an immediate "[NTP] Failed to
    // synchronize", even though the server itself was reachable.
    // configTzTime() is called AGAIN for each attempt (not just
    // getLocalTime() repeated), since otherwise the underlying SNTP client
    // never sends a fresh request - its own internal retry interval is far
    // longer than WAIT_3s.
#define NTP_SYNC_ATTEMPTS 2

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

    // Fuer den DCF77-Status-Punkt in der Topbar (siehe getDcf77Status() in
    // webserver_routes.h): unabhaengig davon, ob DCF77 gerade tatsaechlich die
    // Systemzeit stellt (das entscheidet der stuendliche NTP-/DCF77-Sync-Block in uhr3.ino
    // anhand von lastNtpSuccessMillis, siehe globals.h - kein fester
    // Kulanzzeitraum mehr, sondern ein direkter Erfolgs-Check bei jedem
    // stuendlichen NTP-Versuch). dcfTimeFound und
    // dcf77Count werden im Code selbst NIE zurueckgesetzt (dcfTimeFound bleibt
    // fuer immer true, dcf77Count fuer immer >0, sobald sie einmal gesetzt
    // wurden) - ohne diese beiden Schwellwerte wuerde der Punkt nach einem
    // erfolgreichen Erstsync fuer immer gruen (bzw. nach dem ersten jemals
    // empfangenen Impuls fuer immer mindestens gelb) bleiben, selbst wenn der
    // Empfang spaeter waehrend des Betriebs komplett ausfaellt.
    // DCF77_SYNC_STALE_AFTER: wie lange eine erfolgreiche Dekodierung
    // (lastDcfSyncTime) als "aktuell" gilt, bevor der Punkt von gruen auf
    // gelb/rot zurueckfaellt - grosszuegig ueber der ueblichen ca. 1-Minuten-
    // Telegrammdauer, um bei nur zeitweise schlechtem Empfang nicht zu flackern.
    // DCF77_PULSE_STALE_AFTER: wie lange seit dem letzten tatsaechlich
    // beobachteten Impuls (dcf77Count-Aenderung, siehe checkDcf77Health() in
    // time_sync.h) vergehen darf, bevor der Empfang als komplett tot gilt -
    // waehrend des Empfangs aendert sich dcf77Count etwa einmal pro Sekunde.

    // For the DCF77 status dot in the topbar (see getDcf77Status() in
    // webserver_routes.h): independent of whether DCF77 is actually driving
    // the system time right now (decided by the hourly NTP/DCF77 sync block in
    // uhr3.ino based on lastNtpSuccessMillis, see globals.h - no more fixed
    // grace period, instead a direct success check on every hourly NTP
    // attempt). dcfTimeFound and dcf77Count are NEVER reset anywhere in the code (dcfTimeFound stays
    // true forever, dcf77Count stays >0 forever, once either was ever set) -
    // without these two thresholds the dot would stay green forever after a
    // successful first sync (resp. at least yellow forever after the first
    // pulse ever received), even if reception later fails completely during
    // operation.
    // DCF77_SYNC_STALE_AFTER: how long a successful decode (lastDcfSyncTime)
    // still counts as "current" before the dot falls back from green to
    // yellow/red - generously above the usual ~1 minute telegram duration, so
    // it doesn't flicker during merely temporary poor reception.
    // DCF77_PULSE_STALE_AFTER: how long since the last actually observed pulse
    // (a dcf77Count change, see checkDcf77Health() in time_sync.h) may pass
    // before reception counts as completely dead - during reception dcf77Count
    // changes roughly once per second.
#define DCF77_SYNC_STALE_AFTER (15 * WAIT_1m)
#define DCF77_PULSE_STALE_AFTER WAIT_1m

    // Fuer die Anwesenheitserkennung des DCF77-Eintrags in der Topbar (siehe
    // dcf77Confirmed in globals.h, gepflegt von checkDcf77Health() in
    // time_sync.h): ein einzelner dcf77Count-Wechsel reicht NICHT, um
    // "Empfaenger ist wirklich angeschlossen" zu bedeuten - der Datenpin
    // haengt per CHANGE-Interrupt (siehe attachInterrupt() in uhr3.ino) am
    // GPIO, und ein nicht angeschlossener/floatender Pin kann durch
    // elektrisches Rauschen einzelne oder wenige zufaellige Interrupts
    // ausloesen, die sonst faelschlich als "erster Impuls" gezaehlt wuerden
    // (siehe Bugreport: DCF77-Punkt wurde rot, obwohl nie ein echter Impuls
    // ankam). Echter DCF77-Empfang aendert dcf77Count dagegen sehr regelmaessig
    // (CHANGE = 2 Flanken pro Sekunde) - DCF77_PRESENCE_MIN_STREAK verlangt
    // deshalb mehrere HINTEREINANDER plausible Aenderungen, bevor der Empfaenger
    // als wirklich vorhanden gilt; DCF77_PRESENCE_MAX_GAP_MS legt fest, wie
    // gross die Luecke zwischen zwei Aenderungen hoechstens sein darf, damit sie
    // noch als "aufeinanderfolgend" zaehlt (grosszuegig ueber dem ueblichen
    // ca. 1-Sekunden-Abstand, damit einzelne kurze Jitter nicht die Kette
    // abreissen lassen). Reines Rauschen erzeugt i.d.R. vereinzelte, unregel-
    // maessige Impulse und erreicht diese Kettenlaenge kaum.

    // For the DCF77 topbar entry's presence detection (see dcf77Confirmed in
    // globals.h, maintained by checkDcf77Health() in time_sync.h): a single
    // dcf77Count change is NOT enough to mean "a receiver is really
    // connected" - the data pin sits on a CHANGE interrupt (see
    // attachInterrupt() in uhr3.ino), and an unconnected/floating pin can
    // trigger one or a few random interrupts from electrical noise, which
    // would otherwise be falsely counted as the "first pulse" (see bug
    // report: the DCF77 dot turned red even though no real pulse ever
    // arrived). Genuine DCF77 reception, by contrast, changes dcf77Count very
    // regularly (CHANGE = 2 edges per second) - DCF77_PRESENCE_MIN_STREAK
    // therefore requires several CONSECUTIVE plausible changes before the
    // receiver counts as genuinely present; DCF77_PRESENCE_MAX_GAP_MS sets
    // the maximum gap between two changes for them to still count as
    // "consecutive" (generously above the usual ~1 second spacing, so a
    // single brief jitter doesn't break the chain). Plain noise typically
    // produces sparse, irregular pulses and rarely reaches this chain length.
#define DCF77_PRESENCE_MIN_STREAK 6
#define DCF77_PRESENCE_MAX_GAP_MS 1500

    // Fuer den Bit-Fortschritts-Dekoder der /dcf77-Live-Seite (siehe
    // processDcf77Bits() in time_sync.h): derselbe elektrische Jitter, der
    // oben DCF77_PRESENCE_MIN_STREAK/DCF77_PRESENCE_MAX_GAP_MS noetig macht,
    // erzeugt gelegentlich sehr kurze zusaetzliche Flankenpaare (Prellen/
    // Rauschen im Bereich weniger Millisekunden bis niedriger zweistelliger
    // Millisekunden), die deutlich kuerzer sind als jeder echte DCF77-Zustand
    // (kuerzester echter Zustand: ca. 100ms-Impuls). Ohne Filterung wuerde
    // jede solche Mini-Flanke faelschlich als eigenes Bit gezaehlt und den
    // Fortschritt schneller als die tatsaechlich vergangene Zeit voranspringen
    // lassen (sichtbar als "uebersprungene" Sekunden) sowie Bitwerte
    // verfaelschen. DCF77_BIT_NOISE_IGNORE_MS legt die Untergrenze fest, unter
    // der eine Flanke als Rauschen verworfen wird, OHNE den Referenzzeitpunkt
    // fuer die naechste Dauerberechnung zu verschieben (das Rauschen wird so
    // wirksam "uebersprungen", nicht mitgezaehlt).

    // For the /dcf77 live page's bit-progress decoder (see
    // processDcf77Bits() in time_sync.h): the same electrical jitter that
    // makes DCF77_PRESENCE_MIN_STREAK/DCF77_PRESENCE_MAX_GAP_MS necessary
    // above occasionally produces very short extra edge pairs (bounce/noise
    // in the range of a few to low tens of milliseconds), far shorter than
    // any genuine DCF77 state (shortest genuine state: the ~100ms pulse).
    // Without filtering, every such mini-edge would be wrongly counted as
    // its own bit, making the progress advance faster than real elapsed time
    // (visible as "skipped" seconds) and corrupting bit values.
    // DCF77_BIT_NOISE_IGNORE_MS sets the lower bound below which an edge is
    // discarded as noise WITHOUT shifting the reference timestamp used for
    // the next duration calculation (so the noise is effectively skipped
    // over, not counted).
    //
    // Wert von 40 auf 70 angehoben: der kuerzeste ECHTE DCF77-Zustand ist der
    // 100ms-Impuls, auch ein stark verzerrender Empfaenger unterschreitet 70ms
    // praktisch nie. Mit 40ms wurde eine Stoerflanke mitten im Impuls (z.B.
    // bei 20ms und 45ms) zwar als Rauschen erkannt, die zweite davon aber
    // knapp nicht mehr - der Impuls zerfiel dadurch in zwei "Impulse" von
    // 45ms und 55ms, was einen falschen Bitwert UND einen Impulsabstand von
    // 45ms erzeugte. Mit 70ms wird das komplette Stoerpaar uebersprungen und
    // der Impuls anschliessend in voller, korrekter Laenge gemessen.

    //
    // Raised from 40 to 70: the shortest GENUINE DCF77 state is the 100ms
    // pulse, and even a heavily distorting receiver practically never goes
    // below 70ms. With 40ms, a spurious edge inside the pulse (e.g. at 20ms
    // and 45ms) was recognized as noise for the first one but just barely not
    // for the second - so the pulse fell apart into two "pulses" of 45ms and
    // 55ms, producing a wrong bit value AND a pulse distance of 45ms. With
    // 70ms the whole spurious pair is skipped and the pulse is then measured
    // at its full, correct length.
#define DCF77_BIT_NOISE_IGNORE_MS 70

    // --- Sekundenraster-Dekoder (siehe processDcf77Bits() in time_sync.h) ---
    //
    // Der Dekoder klassifiziert nicht mehr jede einzelne Flankendauer isoliert,
    // sondern legt die Impulse auf ein Sekundenraster: der Abstand ZWEIER
    // aufeinanderfolgender IMPULSANFAENGE ist bei DCF77 immer ein ganzzahliges
    // Vielfaches einer Sekunde. Daraus ergibt sich, wie viele Sekunden seit dem
    // letzten Impuls vergangen sind - auch dann, wenn dazwischen einzelne
    // Sekunden zu schwach empfangen wurden. Genau das war die Ursache dafuer,
    // dass frueher praktisch nie ein vollstaendiges Telegramm zusammenkam:
    // JEDE Luecke >=1300ms galt als Minutenmarke, eine einzige verlorene
    // Sekunde hat das laufende Telegramm also mitten in der Minute verworfen.
    //
    // DCF77_PULSE_MAX_MS:      laengstes Intervall, das noch als Impuls gilt
    //                          (nominal 100/200ms, plus Verzerrung im Empfaenger)
    // DCF77_PULSE_ONE_MIN_MS:  ab dieser Impulsdauer gilt das Bit als 1
    // DCF77_SECOND_MS:         Rasterweite (eine DCF77-Sekunde)
    // DCF77_STEP_TOLERANCE_MS: zulaessige Abweichung eines Impulsabstands vom
    //                          naechsten ganzzahligen Sekundenvielfachen; groesser
    //                          -> Position unklar, Synchronisation wird verworfen
    // DCF77_MAX_LOST_SECONDS:  so viele Sekunden am Stueck duerfen fehlen, ohne
    //                          dass die Position im Telegramm verloren geht

    // --- Second-grid decoder (see processDcf77Bits() in time_sync.h) ---
    //
    // The decoder no longer classifies each edge duration in isolation; it puts
    // the pulses onto a one-second grid: with DCF77, the distance between two
    // consecutive PULSE STARTS is always a whole number of seconds. That yields
    // how many seconds have passed since the last pulse - even when individual
    // seconds in between were received too weakly. This was exactly why a
    // complete telegram practically never came together before: ANY gap
    // >=1300ms counted as the minute marker, so a single lost second discarded
    // the running telegram in the middle of the minute.
    //
    // DCF77_PULSE_MAX_MS:      longest interval still counted as a pulse
    //                          (100/200ms nominal, plus receiver distortion)
    // DCF77_PULSE_ONE_MIN_MS:  from this pulse width on, the bit counts as 1
    // DCF77_SECOND_MS:         grid width (one DCF77 second)
    // DCF77_STEP_TOLERANCE_MS: permitted deviation of a pulse distance from the
    //                          nearest whole multiple of a second; larger ->
    //                          position unclear, synchronization is dropped
    // DCF77_MAX_LOST_SECONDS:  this many consecutive seconds may be missing
    //                          without losing the position within the telegram
#define DCF77_PULSE_MAX_MS 450
#define DCF77_PULSE_ONE_MIN_MS 150
#define DCF77_SECOND_MS 1000
#define DCF77_STEP_TOLERANCE_MS 300
    // Wie viele Sekunden eine Empfangsluecke hoechstens ueberbruecken darf,
    // ohne dass das Sekundenraster (dcf77Phase in globals.h) verlorengeht.
    // Loest das frueher hier stehende DCF77_MAX_LOST_SECONDS (5) ab, das viel
    // zu eng war: bei laengeren Aussetzern - genau dann also, wenn die Uhr die
    // Synchronisation am dringendsten halten muesste - wurde das Raster
    // verworfen und die Suche nach der Minutenmarke begann von vorn. Ueber
    // eine Luecke laesst sich das Raster problemlos fortschreiben, solange der
    // Abstand nahe genug an einem ganzzahligen Vielfachen einer Sekunde liegt;
    // bei 60 Sekunden liegt der Quarzfehler des ESP32 (Groessenordnung 0,01 %)
    // noch weit unter DCF77_STEP_TOLERANCE_MS.

    // How many seconds a reception gap may bridge at most without losing the
    // second grid (dcf77Phase in globals.h). Replaces the former
    // DCF77_MAX_LOST_SECONDS (5), which was far too tight: on longer dropouts
    // - i.e. exactly when the clock would need to hold synchronization most -
    // the grid was discarded and the search for the minute marker started over.
    // The grid can be carried across a gap without trouble as long as the
    // distance is close enough to a whole multiple of a second; at 60 seconds
    // the ESP32's crystal error (order of 0.01 %) is still far below
    // DCF77_STEP_TOLERANCE_MS.
#define DCF77_MAX_PHASE_GAP_SECONDS 60

    // Erkennung der Minutenmarke (siehe dcf77MarkerMiss/-Hit in globals.h und
    // processDcf77Bits() in time_sync.h): eine Rasterposition gilt als
    // Minutenmarke, wenn dort noch NIE ein Impuls ankam, sie mindestens
    // DCF77_MARKER_MIN_MISSES mal gefehlt hat und sie sich um mindestens
    // DCF77_MARKER_MIN_LEAD Fehlstellen vom naechstbesten Kandidaten absetzt.
    // Der Vorsprung verhindert, dass eine zufaellig mehrfach ausgefallene
    // Sekunde faelschlich zur Marke erklaert wird. DCF77_MARKER_COUNT_MAX ist
    // die Saettigungsgrenze der Zaehler; wird sie erreicht, werden alle
    // halbiert, damit alte Ereignisse ausduennen und ein aufgeloester
    // Dauerstoerer nicht ewig nachwirkt.

    // Minute marker detection (see dcf77MarkerMiss/-Hit in globals.h and
    // processDcf77Bits() in time_sync.h): a grid position counts as the minute
    // marker when a pulse has NEVER arrived there, it has been missing at
    // least DCF77_MARKER_MIN_MISSES times, and it leads the next best
    // candidate by at least DCF77_MARKER_MIN_LEAD missing pulses. That lead
    // prevents a second that happened to drop out repeatedly from being
    // declared the marker. DCF77_MARKER_COUNT_MAX is the counters' saturation
    // limit; when it is reached all of them are halved, so old events thin out
    // and a resolved persistent interferer does not keep echoing forever.
    // Bis zu wie vielen uebersprungenen Sekunden eine Luecke noch als
    // "einzelne ausgefallene Sekunden" gewertet und in der Fehlstellen-
    // Statistik gezaehlt wird. Laengere Luecken sind Empfangspausen: dort ist
    // kein einzelner Impuls ausgefallen, es kam gar nichts an. Wuerden sie
    // mitgezaehlt, bekaemen alle 60 Rasterpositionen gleichmaessig Fehlstellen
    // und die Minutenmarke - die sich gerade dadurch abhebt, dass NUR sie
    // immer fehlt - waere nicht mehr herauszufiltern.

    // Up to how many skipped seconds a gap still counts as "individual
    // dropped seconds" and is recorded in the missing-pulse statistics. Longer
    // gaps are reception pauses: no individual pulse dropped there, nothing
    // arrived at all. Counting them would add missing pulses evenly to all 60
    // grid positions, and the minute marker - which stands out precisely
    // because ONLY it is always missing - could no longer be filtered out.
#define DCF77_MISS_COUNT_MAX_GAP 5

#define DCF77_MARKER_MIN_MISSES 3
#define DCF77_MARKER_MIN_LEAD 2
#define DCF77_MARKER_COUNT_MAX 200

    // So viele aufeinanderfolgende Telegramme mit unmoeglichen Festbits
    // (Bit 0 != 0 bzw. Bit 20 != 1) gelten als Beweis, dass die erkannte
    // Minutenmarke falsch liegt - danach wird sie verworfen und neu gesucht.
    // Mehr als eines, weil auch ein einzelner Stoerimpuls genau auf diesen
    // beiden Bits landen kann, ohne dass die Marke deswegen falsch waere.

    // This many consecutive telegrams with impossible fixed bits (bit 0 != 0
    // resp. bit 20 != 1) count as proof that the detected minute marker is
    // wrong - it is then discarded and searched for anew. More than one,
    // because a single spurious pulse can land on exactly those two bits
    // without the marker being wrong because of it.
#define DCF77_STRUCT_FAIL_LIMIT 3

    // Wie alt das zuletzt dekodierte Telegramm hoechstens sein darf, damit es
    // noch als Zeitquelle uebernommen wird (siehe applyDcf77DecodedTime() in
    // time_sync.h). Vorher genau eine Minute - das hiess: nur wenn ausgerechnet
    // die letzte Minute vor dem stuendlichen Sync-Versuch lueckenlos empfangen
    // wurde, sprang DCF77 ueberhaupt ein; sonst passierte bis zum naechsten
    // Versuch eine Stunde lang nichts. Bei durchwachsenem Empfang war das
    // haeufig genug der Fall, um den DCF77-Rueckfall praktisch nie greifen zu
    // lassen. Ein aelteres Telegramm ist voellig unproblematisch:
    // applyDcf77DecodedTime() rechnet die seit der Dekodierung verstrichene
    // Zeit ueber millis() sauber auf (inklusive Minuten-/Stunden-/Tageswechsel),
    // und der Quarzfehler des ESP32 liegt in dieser Groessenordnung weit unter
    // einer Sekunde.

    // Maximum age of the last decoded telegram for it to still be taken as a
    // time source (see applyDcf77DecodedTime() in time_sync.h). Previously
    // exactly one minute - which meant DCF77 only stepped in when precisely
    // the last minute before the hourly sync attempt had been received without
    // gaps; otherwise nothing happened for another hour. With mixed reception
    // that was frequent enough to make the DCF77 fallback practically never
    // engage. An older telegram is entirely unproblematic:
    // applyDcf77DecodedTime() cleanly adds the time elapsed since decoding via
    // millis() (including minute/hour/day rollovers), and the ESP32's crystal
    // error over this span is far below one second.
#define DCF77_DECODED_MAX_AGE (10 * WAIT_1m)

    // Dauer des Einmal-Aufblitzens der LED pro empfangenem DCF77-Impuls
    // (siehe loop() in uhr3.ino). Bewusst ein kurzer Blitz mit eigener
    // Abschaltzeit statt eines Umschaltens (toggleLED()): beim Umschalten
    // haengt der Endzustand davon ab, ob die Anzahl der empfangenen Impulse
    // gerade oder ungerade war - bleibt der Empfang stehen (oder wird die
    // Zeit gefunden), blieb die LED bei ungerader Anzahl dauerhaft an.

    // Duration of the LED's one-shot flash per received DCF77 pulse (see
    // loop() in uhr3.ino). Deliberately a short flash with its own switch-off
    // time instead of a toggle (toggleLED()): with a toggle the final state
    // depends on whether the number of received pulses was even or odd - if
    // reception stops (or the time is found), an odd count left the LED
    // permanently on.
#define DCF77_LED_BLINK_MS 80

    // Historie: hier stand testweise DCF77_MINUTE_MARKER_MIN_BIT_INDEX - eine
    // Pause >=1300ms sollte nur noch nahe dem Telegrammende (dcf77BitIndex
    // >=56) als echte Minutenmarke gelten, sonst nur als einzelne verlorene
    // Sekunde. Grund war, dass eine einzelne schwache/verlorene Sekunde
    // dieselbe Pausenlaenge erzeugt wie die echte Marke und diese sonst
    // faelschlich mitten im Telegramm ausgeloest haette. Wieder entfernt:
    // bei ZWEI oder mehr aufeinanderfolgenden verlorenen Sekunden (laengere
    // Pause) wurde dcf77BitIndex trotzdem nur um 1 weitergezaehlt und geriet
    // dadurch hinter die Wanduhr - die anschliessend tatsaechlich folgende
    // echte Minutenmarke wurde dadurch entweder verworfen oder an einer
    // falschen Position akzeptiert. Siehe processDcf77Bits() in time_sync.h
    // fuer die jetzt wieder einfache, dafuer aber immer selbstkorrigierende
    // Variante (jede Pause >=1300ms zaehlt als Minutenmarke, unabhaengig von
    // der Position).

    // History: this used to hold DCF77_MINUTE_MARKER_MIN_BIT_INDEX - a
    // >=1300ms gap was only accepted as the genuine minute marker near the
    // end of a telegram (dcf77BitIndex >=56), otherwise treated as a single
    // lost second. The reasoning was that a single weak/lost second produces
    // the same gap length as the real marker and would otherwise wrongly
    // trigger it mid-telegram. Removed again: with TWO or more consecutive
    // lost seconds (a longer gap), dcf77BitIndex was still only advanced by
    // 1 and fell behind the wall clock - the genuine minute marker that then
    // actually followed was as a result either discarded or accepted at what
    // was actually the wrong position. See processDcf77Bits() in time_sync.h
    // for the simple, always self-correcting variant now in place again
    // (any gap >=1300ms counts as the minute marker, regardless of position).

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
