#pragma once

    // Display: Zifferblatt, Zeiger, Sprites, Helligkeit, Touch
    // Benoetigt globals.h, config.h, prefs_keys.h, declarations.h (vor dieser Datei in uhr4.ino eingebunden)
    // PSRAM bevorzugt fuer kurzlebige Puffer (BMP/PNG im Webinterface), um den
    // knappen internen Heap nicht zu fragmentieren. Eigener psramFound()-Check
    // statt gc9d01SwRotation, da dieses Flag nur die GC9D01-Rotation betrifft.
    // Fuer "new (std::nothrow)" unten: garantiert nullptr statt
    // implementierungsabhaengigem Verhalten bei fehlgeschlagener Allokation.

    // Display: clock face, hands, sprites, brightness, touch
    // Requires globals.h, config.h, prefs_keys.h, declarations.h (included before this file in uhr4.ino)
    // Prefers PSRAM for short-lived buffers (BMP/PNG in the web interface) to
    // avoid fragmenting the scarce internal heap. Uses its own psramFound()
    // check instead of gc9d01SwRotation, since that flag only concerns rotation.
    // For "new (std::nothrow)" below: guarantees nullptr instead of
    // implementation-defined behavior on failed allocation.

#include <new>

    void* preferPsramMalloc(size_t size) {
        if (psramFound()) {
            void* p = ps_malloc(size);
            if (p) return p;
        }
        return malloc(size);
    }


    // Ein Display gilt als angeschlossen, solange seine Rotation nicht "n.a." ist
    // A display counts as connected as long as its rotation is not "n.a."

    bool isDisplayConnected(uint8_t displayNum) {
        return ((displayNum == 1) ? tftRotation1 : tftRotation2) != TFT_ROTATION_NA;
    }


    // Rotation des ersten angeschlossenen Displays (Display 1 hat Vorrang), nie "n.a."
    // Rotation of the first connected display (display 1 takes priority), never "n.a."

    uint8_t primaryDisplayRotation() {
        if (isDisplayConnected(1)) return tftRotation1;
        if (isDisplayConnected(2)) return tftRotation2;
        return 0;
    }


    // Rotation fuer Status-/Startmeldungen eines Displays: bei "n.a." 0 Grad, nie der Wert 4
    // Rotation for status/boot messages of a display: 0 degrees for "n.a.", never the value 4

    uint8_t effectiveRotation(uint8_t displayNum) {
        return isDisplayConnected(displayNum) ? ((displayNum == 1) ? tftRotation1 : tftRotation2) : 0;
    }


    // Displayname wie bei den uhr3-Builds und in flashESP ("GC9A01",
    // "GC9A01_WITH_BACKLIGHT", "GC9D01") -> Displaytyp + Backlight-Regelung.
    // Display name as in the uhr3 builds and in flashESP ("GC9A01",
    // "GC9A01_WITH_BACKLIGHT", "GC9D01") -> display type + backlight control.

    bool parseDisplayName(const String& name, uint8_t& type, bool& backlight) {
        if (name == "GC9A01") { type = DISPLAY_TYPE_GC9A01; backlight = false; return true; }
        if (name == "GC9A01_WITH_BACKLIGHT") { type = DISPLAY_TYPE_GC9A01; backlight = true; return true; }
        if (name == "GC9D01") { type = DISPLAY_TYPE_GC9D01; backlight = true; return true; }
        return false;
    }

    // Umkehrung von parseDisplayName(): beim GC9D01 zaehlt die
    // Backlight-Einstellung nicht (immer an Pin 3 verdrahtet).
    // Inverse of parseDisplayName(): for the GC9D01 the backlight setting
    // does not matter (always wired to pin 3).

    const char* displayChoiceName(uint8_t type, bool backlight) {
        if (type == DISPLAY_TYPE_GC9D01) return "GC9D01";
        return backlight ? "GC9A01_WITH_BACKLIGHT" : "GC9A01";
    }

    // Update von uhr3: dessen Firmware vermerkt, fuer welches Display sie
    // kompiliert wurde (PK_UHR3_BUILD_DISPLAY). Nur beim ersten Umstieg auf
    // uhr4 - solange uhr4 noch keinen Displaytyp gespeichert hat - werden
    // Displaytyp und Backlight-Regelung daraus uebernommen. Ist schon einer
    // gespeichert, bleibt er: ein zwischendurch aufgespielter uhr3-Build (z.B.
    // zum Testen, fuer ein anderes Display) stellte die Uhr sonst beim
    // naechsten Start unbemerkt um. Der Vermerk wird in jedem Fall geloescht.
    // Helligkeit, Schwellwerte und Nabengroesse bleiben: uhr3 hat sie bereits
    // passend zu seinem Display gespeichert.

    // Update from uhr3: its firmware records which display it was compiled
    // for (PK_UHR3_BUILD_DISPLAY). Display type and backlight control are
    // taken over from it only on the first switch to uhr4 - as long as uhr4
    // has not stored a display type yet. If one is stored, it stays: a uhr3
    // build flashed in between (e.g. for testing, for another display) would
    // otherwise silently switch the clock at the next boot. The record is
    // deleted in any case. Brightness, thresholds and hub size stay: uhr3
    // already stored them to match its display.

    void adoptUhr3BuildDisplay() {
        if (!preferences.isKey(PK_UHR3_BUILD_DISPLAY)) return;
        String build = preferences.getString(PK_UHR3_BUILD_DISPLAY, "");
        preferences.remove(PK_UHR3_BUILD_DISPLAY);
        if (preferences.isKey(PK_DISPLAY_TYPE)) {
            DEBUG_PRINTLN("[Display] uhr3 build display '" + build + "' ignored - display type already set in uhr4");
            return;
        }

        uint8_t type;
        bool backlight;
        if (!parseDisplayName(build, type, backlight)) {
            DEBUG_PRINTLN("[Display] uhr3 build display '" + build + "' not supported by uhr4 - keeping display type");
            return;
        }
        preferences.putUChar(PK_DISPLAY_TYPE, type);
        preferences.putBool(PK_USE_BACKLIGHT, backlight);
        DEBUG_PRINTLN("[Display] Update from uhr3 (" + build + "): display type " + DISPLAY_GEOMETRY[type].name + ", backlight " + (backlight ? "on" : "off"));
    }


    // Liest den Displaytyp aus den Preferences und stellt displayGeom darauf
    // um - MUSS vor allem laufen, was CLOCK_WIDTH/HAND_* benutzt (Migrationen,
    // Sprites, Puffer), daher direkt nach preferences.begin() in setup().

    // Reads the display type from preferences and points displayGeom at it -
    // MUST run before anything that uses CLOCK_WIDTH/HAND_* (migrations,
    // sprites, buffers), hence right after preferences.begin() in setup().

    void loadDisplayType() {
        adoptUhr3BuildDisplay();
        uint8_t type = preferences.getUChar(PK_DISPLAY_TYPE, DISPLAY_TYPE_DEFAULT);
        if (type >= DISPLAY_TYPE_COUNT) type = DISPLAY_TYPE_DEFAULT; // beschaedigter NVS-Wert
                                                                     // corrupted NVS value
        displayType = type;
        displayGeom = &DISPLAY_GEOMETRY[type];
        tftType = displayGeom->name;

        // Standard-Zeigerbreiten des Typs - parseBackgroundFilename() passt sie
        // spaeter je Zifferblatt an.
        // Default hand widths of the type - parseBackgroundFilename() adjusts
        // them per clock face later.
        hourHandWidth = minuteHandWidth = secondHandWidth = HAND_WIDTH;

        DEBUG_PRINTLN(String("[Display] Type: ") + displayGeom->name + " (" + String(CLOCK_WIDTH) + "x" + String(CLOCK_HEIGHT) + ")");
    }


    // Speichert einen neuen Displaytyp und setzt die typabhaengigen
    // Werksvorgaben (Backlight, Helligkeit/Schwellwerte, Nabengroesse) -
    // wirksam erst nach dem Neustart, den der Aufrufer ausloest.

    // Stores a new display type and sets the type-dependent factory defaults
    // (backlight, brightness/thresholds, hub size) - only effective after the
    // restart the caller triggers.

    // Hardware-Reset beider Displays (gemeinsame RST-Leitung) mit den Zeiten
    // von uhr3: 20 ms Puls, danach 150 ms Wartezeit. LovyanGFX wartet nur
    // 64 ms - das GC9D01 ignorierte dann die Startbefehle (u.a. Sleep Out) und
    // blieb bei leuchtender Hintergrundbeleuchtung schwarz. Daher pin_rst = -1
    // in lgfx_config.h.

    // Hardware reset of both displays (shared RST line) with uhr3's timing:
    // 20 ms pulse, then a 150 ms wait. LovyanGFX only waits 64 ms - the
    // GC9D01 then ignored the init commands (among them Sleep Out) and stayed
    // black with the backlight lit. Hence pin_rst = -1 in lgfx_config.h.

    void resetPanels() {
        pinMode(TFT_RST, OUTPUT);
        digitalWrite(TFT_RST, HIGH);
        delay(5);
        digitalWrite(TFT_RST, LOW);
        delay(20);
        digitalWrite(TFT_RST, HIGH);
        delay(150);
    }


    // Speichert Displaytyp + Backlight-Regelung (Auswahl wie
    // parseDisplayName()) samt den passenden Standardwerten - wirkt erst nach
    // dem Neustart (Aufrufer startet neu).
    // Stores display type + backlight control (choice as in
    // parseDisplayName()) together with the matching defaults - takes effect
    // only after the restart (the caller restarts).

    void setDisplayType(uint8_t type, bool backlight) {
        if (type >= DISPLAY_TYPE_COUNT) return;
        const DisplayGeometry& g = DISPLAY_GEOMETRY[type];
        preferences.putUChar(PK_DISPLAY_TYPE, type);
        preferences.putBool(PK_USE_BACKLIGHT, backlight);
        putBrightnessDefaults(backlight);
        preferences.putUInt(PK_CENTER_SIZE, g.centerSize);
        DEBUG_PRINTLN(String("[Display] Type changed to ") + displayChoiceName(type, backlight) + ", restarting");
    }


    // Hex-Text -> Bytes als String (fuer WLAN-Name/-Passwort per USB, damit
    // Leerzeichen, Umlaute und Sonderzeichen sicher ankommen). false bei
    // ungueltigem Hex oder mehr als maxLen Bytes.
    // Hex text -> bytes as a string (for WiFi name/password via USB, so that
    // spaces, umlauts and special characters arrive safely). false on invalid
    // hex or more than maxLen bytes.

    bool hexToText(const String& hex, String& out, size_t maxLen) {
        out = "";
        if (hex.length() % 2 || hex.length() / 2 > maxLen) return false;
        for (size_t i = 0; i < hex.length(); i += 2) {
            char pair[3] = { hex[i], hex[i + 1], 0 };
            if (!isxdigit((uint8_t)pair[0]) || !isxdigit((uint8_t)pair[1])) return false;
            out += (char)strtol(pair, nullptr, 16);
        }
        return true;
    }


    // Einrichtung per USB - fuer die Erstinbetriebnahme: zeigt das Display
    // wegen des falschen Typs nichts Lesbares, ist die Uhr sonst nur ueber den
    // Access Point erreichbar. flashESP.bat/.sh fragen Displaytyp und WLAN ab
    // und senden nach dem Flashen zeilenweise:
    //   "UHR4 WIFI <Name-Hex> <Passwort-Hex>" -> speichert das WLAN wie nach
    //       WPS (saveWpsCredentials()) und bevorzugt es beim naechsten Start;
    //       Antwort "UHR4 OK WIFI" - das Passwort erscheint nirgends (weder
    //       Antwort noch Log).
    //   "UHR4 DISPLAY <Name>" (Name siehe parseDisplayName()) -> bei Aenderung
    //       wie im Zifferblatt-Tab Werksvorgaben setzen und neu starten;
    //       Antwort "UHR4 OK DISPLAY <Name> RESTART|UNCHANGED".
    //   "UHR4 RESTART" -> Antwort "UHR4 OK RESTART", Neustart.
    //   "UHR4 INFO" -> eingestellter Displaytyp, siehe handleSerialInfo().
    //   "UHR4 TIME <Unix-Sekunden>" -> Uhrzeit vom PC, siehe handleSerialTime().
    // Fehler: "UHR4 ERROR ...". Aufruf aus loop() und den langen
    // Warteschleifen beim Start - kein zusaetzliches Warten beim Start.

    // Setup via USB - for first-time setup: if the display shows nothing
    // readable because of the wrong type, the clock is otherwise only
    // reachable via the access point. flashESP.bat/.sh ask for display type
    // and WiFi and send line by line after flashing:
    //   "UHR4 WIFI <name hex> <password hex>" -> stores the WiFi like after
    //       WPS (saveWpsCredentials()) and prefers it at the next boot; reply
    //       "UHR4 OK WIFI" - the password appears nowhere (neither reply nor
    //       log).
    //   "UHR4 DISPLAY <name>" (name see parseDisplayName()) -> on a change, as
    //       in the clock face tab, set factory defaults and restart; reply
    //       "UHR4 OK DISPLAY <name> RESTART|UNCHANGED".
    //   "UHR4 RESTART" -> reply "UHR4 OK RESTART", restart.
    //   "UHR4 INFO" -> configured display type, see handleSerialInfo().
    //   "UHR4 TIME <Unix seconds>" -> time from the PC, see handleSerialTime().
    // Errors: "UHR4 ERROR ...". Called from loop() and the long wait loops at
    // boot - no additional waiting at boot.

    // Antwort auf einen USB-Befehl: hier darf das Senden kurz warten (der PC
    // liest gerade mit), sonst sind USB-Ausgaben nicht blockierend (setup()).
    // Reply to a USB command: sending may wait briefly here (the PC is reading
    // right now), otherwise USB output is non-blocking (setup()).
    void serialReply(const String& reply) {
#if ARDUINO_USB_CDC_ON_BOOT
        Serial.setTxTimeoutMs(250);
#endif
        Serial.println(reply);
        Serial.flush();
#if ARDUINO_USB_CDC_ON_BOOT
        Serial.setTxTimeoutMs(0);
#endif
    }

    // Neustart erst nach 1 s: das Skript schliesst die Schnittstelle nach der
    // Antwort sofort - verschwindet die Uhr vorher vom USB, kann das Schliessen
    // unter Windows haengen und die Schnittstelle offen halten.
    // Restart only after 1 s: the script closes the port right after the reply
    // - if the clock vanishes from USB before that, closing can hang on
    // Windows and keep the port open.
    void serialRestart(const String& reply) {
        serialReply(reply);
        delay(1000);
        espReboot();
    }

    void handleSerialWifi(const String& args) {
        int sep = args.indexOf(' ');
        String ssid, pass;
        bool ok = sep > 0 &&
                  hexToText(args.substring(0, sep), ssid, 32) &&
                  hexToText(args.substring(sep + 1), pass, 63);
        if (!ok || ssid.length() == 0 || (pass.length() > 0 && pass.length() < 8)) {
            backupWipe(pass);
            serialReply("UHR4 ERROR WIFI invalid (name 1-32 bytes, password empty or 8-63 bytes)");
            return;
        }
        int slot = saveWpsCredentials(ssid, pass);
        preferences.putInt(PK_LAST_WLAN, slot);
        preferences.putBool(PK_WIFI_ACTIVE, true);
        wifiActive = true;
        backupWipe(pass);
        serialReply("UHR4 OK WIFI");
    }

    // "UHR4 INFO" -> "UHR4 OK INFO <Name> <gesetzt>": eingestellter Displaytyp
    // (Name wie parseDisplayName(), Backlight beim GC9A01 beruecksichtigt) und
    // ob er je gespeichert wurde (1) oder nur der Standard gilt (0). flashESP
    // fragt das VOR dem Flashen ab und waehlt den Typ vor. Liest nur.
    // "UHR4 INFO" -> "UHR4 OK INFO <name> <set>": configured display type (name
    // as in parseDisplayName(), backlight considered for the GC9A01) and
    // whether it was ever stored (1) or only the default applies (0). flashESP
    // queries this BEFORE flashing and preselects the type. Read-only.
    void handleSerialInfo() {
        bool backlight = preferences.getBool(PK_USE_BACKLIGHT, DISPLAY_GEOMETRY[displayType].backlightDefault);
        serialReply(String("UHR4 OK INFO ") + displayChoiceName(displayType, backlight) + " " + String(preferences.isKey(PK_DISPLAY_TYPE) ? 1 : 0));
    }

    void handleSerialDisplay(const String& name) {
        uint8_t type;
        bool backlight;
        if (!parseDisplayName(name, type, backlight)) {
            serialReply("UHR4 ERROR DISPLAY unknown '" + name + "' (GC9A01, GC9A01_WITH_BACKLIGHT, GC9D01)");
            return;
        }
        // Gespeicherten Wert vergleichen - der Befehl kann auch waehrend
        // setup() kommen, bevor useBacklight geladen ist.
        // Compare the stored value - the command may also arrive during
        // setup(), before useBacklight is loaded.
        bool storedBacklight = preferences.getBool(PK_USE_BACKLIGHT, DISPLAY_GEOMETRY[displayType].backlightDefault);
        if (type == displayType && backlight == storedBacklight) {
            serialReply("UHR4 OK DISPLAY " + name + " UNCHANGED");
            return;
        }
        setDisplayType(type, backlight);
        serialRestart("UHR4 OK DISPLAY " + name + " RESTART");
    }

#if ARDUINO_USB_CDC_ON_BOOT && !ARDUINO_USB_MODE
#include <esp32-hal-tinyusb.h> // usb_persist_restart()
    // 1200 Baud = Wunsch nach dem Download-Modus (wie Arduino IDE, port.ps1,
    // flashESP.sh). Ersetzt die im Core abgeschaltete Umschaltung
    // (Serial.enableReboot(false) in setup()), ohne deren DTR/RTS-Folge.
    // 1200 baud = request for download mode (like the Arduino IDE, port.ps1,
    // flashESP.sh). Replaces the switch disabled in the core
    // (Serial.enableReboot(false) in setup()), without its DTR/RTS sequence.
    void usbCdcLineCodingEvent(void* arg, esp_event_base_t base, int32_t id, void* data) {
        arduino_usb_cdc_event_data_t* e = (arduino_usb_cdc_event_data_t*)data;
        if (e && e->line_coding.bit_rate == 1200) {
            usb_persist_restart(RESTART_BOOTLOADER);
        }
    }
#endif

    void handleSerialCommands() {
        static String line;
        while (Serial.available()) {
            char c = (char)Serial.read();
            if (c == '\r') continue;
            if (c != '\n') {
                if (line.length() < 256) line += c;
                continue;
            }
            String cmd = line;
            backupWipe(line); // kann WLAN-Daten enthalten / may contain WiFi data
            cmd.trim();
            if (cmd.startsWith("UHR4 WIFI ")) {
                handleSerialWifi(cmd.substring(10));
            }
            else if (cmd.startsWith("UHR4 DISPLAY ")) {
                String name = cmd.substring(13);
                name.trim();
                handleSerialDisplay(name);
            }
            else if (cmd == "UHR4 RESTART") {
                serialRestart("UHR4 OK RESTART");
            }
            else if (cmd == "UHR4 INFO") {
                handleSerialInfo();
            }
            else if (cmd.startsWith("UHR4 TIME ")) {
                String arg = cmd.substring(10);
                arg.trim();
                handleSerialTime(arg);
            }
            backupWipe(cmd);
        }
    }


    // Rotation, die tatsaechlich am Chip (MADCTL) eingestellt wird: bei
    // Software-Rotation (GC9D01) immer 0, sonst effectiveRotation().

    // Rotation actually set on the chip (MADCTL): always 0 with software
    // rotation (GC9D01), otherwise effectiveRotation().

    uint8_t hardwareRotation(uint8_t displayNum) {
        return gc9d01SwRotation ? 0 : effectiveRotation(displayNum);
    }


    // Legt ein 16-Bit-Sprite an, standardmaessig im PSRAM - auch die Zeiger
    // (je ~7,5 KB): der interne RAM ist auf dem ESP32-S2 knapp und wird von
    // WLAN/Webserver gebraucht (Zeiger intern liessen den freien Heap auf
    // 5 KB fallen, die Uhr antwortete dann nicht mehr). Klappt der
    // bevorzugte Speicher nicht, zweiter Versuch im anderen.
    // swapBytes: pushImage() bekommt RGB565 in RAM-Reihenfolge.

    // Creates a 16-bit sprite, by default in PSRAM - the hands too (~7.5 KB
    // each): internal RAM is scarce on the ESP32-S2 and needed by WiFi/the
    // web server (hands in internal RAM dropped the free heap to 5 KB, the
    // clock then stopped responding). If the preferred memory fails, a
    // second attempt in the other one.
    // swapBytes: pushImage() receives RGB565 in RAM byte order.

    bool createSprite16(LGFX_Sprite& sprite, int32_t w, int32_t h, bool preferPsram) {
        sprite.setColorDepth(16);
        sprite.setSwapBytes(true);
        bool first = preferPsram && psramFound();
        sprite.setPsram(first);
        if (sprite.createSprite(w, h) != nullptr) return true;
        if (!psramFound()) return false;
        sprite.setPsram(!first);
        return sprite.createSprite(w, h) != nullptr;
    }


    // Textdarstellung fuer Display und Status-Sprites: GLCD-Schrift (wie
    // bisher Font 1) als CP437-Zeichensatz, ohne UTF-8-Dekodierung - Umlaute
    // und Akzente kommen ueber tftText() als einzelne CP437-Bytes.

    // Text rendering for the display and status sprites: GLCD font (Font 1
    // as before) as the CP437 charset, without UTF-8 decoding - umlauts and
    // accents arrive via tftText() as single CP437 bytes.

    void setupTextStyle(lgfx::LovyanGFX& gfx) {
        gfx.setFont(&fonts::Font0);
        gfx.setAttribute(lgfx::utf8_switch, false);
        gfx.setAttribute(lgfx::cp437_switch, true);
    }


    // Wandelt Text fuers Display um: HTML-Entities aus translation.h
    // (&uuml;, &eacute;, ...) und UTF-8 (z.B. SSIDs) werden zu CP437-Bytes der
    // GLCD-Schrift. Unbekannte Zeichen werden zu '?', statt als Zeichensalat
    // oder roher Entity-Text ("pr&uuml;fen") zu erscheinen.

    // Converts text for the display: HTML entities from translation.h
    // (&uuml;, &eacute;, ...) and UTF-8 (e.g. SSIDs) become CP437 bytes of
    // the GLCD font. Unknown characters become '?' instead of showing up as
    // garbage or raw entity text ("pr&uuml;fen").

    String tftText(const String& text) {
        struct Cp437Map { uint16_t unicode; const char* entity; char cp437; };
        static const Cp437Map cp437Table[] = {
            { 0x00E4, "auml",   (char)0x84 }, { 0x00F6, "ouml",   (char)0x94 }, { 0x00FC, "uuml",   (char)0x81 },
            { 0x00C4, "Auml",   (char)0x8E }, { 0x00D6, "Ouml",   (char)0x99 }, { 0x00DC, "Uuml",   (char)0x9A },
            { 0x00DF, "szlig",  (char)0xE1 }, { 0x00E9, "eacute", (char)0x82 }, { 0x00C9, "Eacute", (char)0x90 },
            { 0x00E8, "egrave", (char)0x8A }, { 0x00EA, "ecirc",  (char)0x88 }, { 0x00EB, "euml",   (char)0x89 },
            { 0x00E0, "agrave", (char)0x85 }, { 0x00E2, "acirc",  (char)0x83 }, { 0x00E7, "ccedil", (char)0x87 },
            { 0x00EE, "icirc",  (char)0x8C }, { 0x00EF, "iuml",   (char)0x8B }, { 0x00F4, "ocirc",  (char)0x93 },
            { 0x00FB, "ucirc",  (char)0x96 }, { 0x00F9, "ugrave", (char)0x97 }, { 0x00C7, "Ccedil", (char)0x80 },
            { 0x00B0, "deg",    (char)0xF8 }, { 0x00A0, "nbsp",   ' ' },        { 0x0026, "amp",    '&' },
            { 0x0027, "#39",    '\'' },       { 0x0022, "quot",   '"' },        { 0x003C, "lt",     '<' },
            { 0x003E, "gt",     '>' },
        };

        String out;
        out.reserve(text.length());
        const char* s = text.c_str();
        size_t i = 0;
        const size_t len = text.length();

        while (i < len) {
            uint8_t c = (uint8_t)s[i];

            if (c == '&') {
                int semi = text.indexOf(';', i + 1);
                if (semi > (int)i + 1 && semi - (int)i <= 8) {
                    String name = text.substring(i + 1, semi);
                    bool found = false;
                    for (const auto& m : cp437Table) {
                        if (name == m.entity) { out += m.cp437; found = true; break; }
                    }
                    if (found) { i = semi + 1; continue; }
                }
                out += '&';
                i++;
                continue;
            }

            if (c < 0x80) { out += (char)c; i++; continue; }

            // UTF-8-Folge dekodieren (2 oder 3 Byte reichen fuer Latin-1)
            // Decode a UTF-8 sequence (2 or 3 bytes cover Latin-1)
            uint32_t cp = 0;
            size_t n = 0;
            if ((c & 0xE0) == 0xC0) { cp = c & 0x1F; n = 1; }
            else if ((c & 0xF0) == 0xE0) { cp = c & 0x0F; n = 2; }
            else if ((c & 0xF8) == 0xF0) { cp = c & 0x07; n = 3; }
            else { out += '?'; i++; continue; }
            if (i + n >= len) { out += '?'; break; } // abgeschnittene Folge
                                                     // truncated sequence
            for (size_t k = 1; k <= n; k++) cp = (cp << 6) | ((uint8_t)s[i + k] & 0x3F);
            i += n + 1;

            char mapped = '?';
            for (const auto& m : cp437Table) {
                if (m.unicode == cp) { mapped = m.cp437; break; }
            }
            out += mapped;
        }
        return out;
    }


#if defined CS_2


    // Waehlt Display 1 aus, deaktiviert Display 2. Steuert beide CS-Pins
    // manuell, da LovyanGFX keinen CS-Pin fuehrt (pin_cs = -1, siehe
    // lgfx_config.h) und CS_1 sonst nicht mitgeschaltet wuerde.
    // Vor dem Umschalten muss die laufende Uebertragung fertig sein. Danach
    // setRotation(): stellt MADCTL fuer DIESEN Chip ein und verwirft
    // LovyanGFXs gemerktes Adressfenster - das gilt sonst noch fuer den
    // anderen Chip, und der neue bekaeme falsche Bilddaten.

    // Selects Display 1, disables Display 2. Drives both CS pins manually,
    // since LovyanGFX drives no CS pin (pin_cs = -1, see lgfx_config.h) and
    // CS_1 would otherwise not be toggled along.
    // The running transfer must be finished before switching. Afterwards
    // setRotation(): sets MADCTL for THIS chip and discards LovyanGFX's
    // cached address window - it still belongs to the other chip otherwise,
    // and the new one would receive wrong frame data.

    void setCS1(bool state) {
        if (state == LOW) {
            if (tftInitialized) tft.waitDMA();
            digitalWrite(CS_1, LOW);
            digitalWrite(CS_2, HIGH);
            if (tftInitialized) tft.setRotation(hardwareRotation(1));
        }

    }


    // Waehlt Display 2 bei Dual-Display-Aufbauten ueber seinen Chip-Select-Pin aus
    // (deaktiviert dabei Display 1, siehe Kommentar bei setCS1())

    // Selects Display 2 via its chip-select pin in dual-display setups
    // (disables Display 1, see comment on setCS1())

    void setCS2(bool state) {
        if (state == LOW) {
            if (tftInitialized) tft.waitDMA();
            digitalWrite(CS_2, LOW);
            digitalWrite(CS_1, HIGH);
            if (tftInitialized) tft.setRotation(hardwareRotation(2));
        }

    }


    // Ruhezustand nach dem Zeichnen: selektiert das erste angeschlossene Display,
    // damit ein "n.a."-Display ausserhalb von Status-/Startmeldungen nicht selektiert bleibt.

    // Idle state after drawing: selects the first connected display, so a "n.a."
    // display doesn't stay selected outside of status/boot messages.

    void setCSIdle() {
        if (isDisplayConnected(1) || !isDisplayConnected(2)) setCS1(LOW); else setCS2(LOW);
    }


    // Bereitet Status-/Boot-Text vor: Hardware-Rotation zeichnet direkt auf 'tft',
    // Software-Rotation kann Text nicht drehen und zeichnet daher immer unrotiert
    // in ein persistentes Sprite - Drehung folgt erst in endStatusDraw().

    // Prepares status/boot text: hardware rotation draws straight to 'tft',
    // software rotation can't rotate text and always draws unrotated into a
    // persistent sprite instead - rotation happens only in endStatusDraw().

    lgfx::LovyanGFX& beginStatusDraw(uint8_t displayNum) {
        displayNeedsBlank[displayNum - 1] = true; // Meldung auf dem Display - ein "n.a."-Display muss spaeter wieder schwarz werden
                                                  // message on the display - a "n.a." display has to go black again later
        clockFrameDirty[displayNum - 1] = true;   // Uhrbild ist ueberzeichnet - naechster Frame voll senden
                                                  // clock image got drawn over - send the next frame in full
        LGFX_Sprite& sprite = (displayNum == 1) ? statusSprite1 : statusSprite2;
        bool& created = (displayNum == 1) ? statusSprite1Created : statusSprite2Created;

        if (gc9d01SwRotation && !created) {
            // Schlaegt die Allokation fehl, bleibt 'created' false und es wird
            // unten unrotiert direkt auf den Chip gezeichnet statt ins Nichts.

            // If allocation fails, 'created' stays false and drawing falls
            // through to the direct-to-chip path below, unrotated but readable.
            if (createSprite16(sprite, CLOCK_WIDTH, CLOCK_HEIGHT)) {
                setupTextStyle(sprite);
                sprite.fillSprite(TFT_BLACK);
                created = true;
            }
            else {
                DEBUG_PRINTLN("[Display] Error: couldnt allocate statusSprite - status text stays unrotated");
            }
        }

        if (!gc9d01SwRotation || !created) {
            if (displayNum == 1) setCS1(LOW); else setCS2(LOW);
            return tft;
        }
        // Bewusst KEIN sprite.setRotation() (siehe Kommentar oben) - das
        // Sprite bleibt immer in seiner unrotierten 0-Grad-Ausgangslage,
        // die Drehung erfolgt erst in endStatusDraw().

        // Deliberately NO sprite.setRotation() (see comment above) - the
        // sprite always stays in its unrotated 0-degree starting state,
        // rotation happens only in endStatusDraw().
        return sprite;
    }


    // Sendet das vorbereitete Sprite ans Display (No-Op bei Hardware-Rotation,
    // da dort schon direkt gezeichnet wurde). Ohne Drehung en bloc uebertragen,
    // sonst zeilenweise wie in loadClockFace(); teilt sich rowBuffer mit ihr.

    // Sends the prepared sprite to the display (no-op with hardware rotation,
    // which already drew directly). Transferred in one go without rotation,
    // otherwise row by row like loadClockFace(); shares its rowBuffer.

    void endStatusDraw(uint8_t displayNum) {
        // Auch das fehlgeschlagene Sprite-Anlegen abfangen (siehe
        // beginStatusDraw()): dann wurde bereits direkt auf den Chip gezeichnet
        // und es gibt nichts zu uebertragen.

        // Also catch a failed sprite allocation (see beginStatusDraw()): in that
        // case drawing already went straight to the chip and there is nothing to
        // transfer.
        bool created = (displayNum == 1) ? statusSprite1Created : statusSprite2Created;
        if (!gc9d01SwRotation || !created) return;

        LGFX_Sprite& sprite = (displayNum == 1) ? statusSprite1 : statusSprite2;
        uint8_t rotation = effectiveRotation(displayNum);
        if (displayNum == 1) setCS1(LOW); else setCS2(LOW);

        if (rotation == 0) {
            sprite.pushSprite(0, 0);
            return;
        }

        // pushImage() sendet Bytes wie im RAM (little-endian), das Display
        // erwartet die andere Reihenfolge - ohne setSwapBytes(true) kommen
        // Farben vertauscht an (z.B. Gruen 0x07E0 wird zu 0xE007).

        // pushImage() sends bytes as they sit in RAM (little-endian), but the
        // display expects the other byte order - without setSwapBytes(true)
        // colors arrive swapped (e.g. green 0x07E0 becomes 0xE007).
        tft.setSwapBytes(true);

        const int N = CLOCK_WIDTH;
        for (int y = 0; y < N; y++) {
            for (int x = 0; x < N; x++) {
                int srcX, srcY;
                switch (rotation) {
                    case 1:  srcX = y;         srcY = N - 1 - x; break; // 90 Grad im Uhrzeigersinn
                                                                        // 90 degrees clockwise
                    case 2:  srcX = N - 1 - x; srcY = N - 1 - y; break; // 180 Grad
                                                                        // 180 degrees
                    default: srcX = N - 1 - y; srcY = x;         break; // 270 Grad im Uhrzeigersinn
                                                                        // 270 degrees clockwise
                }
                rowBuffer[x] = sprite.readPixel(srcX, srcY);
            }
            tft.pushImage(0, y, N, 1, rowBuffer);
        }

        // Zustand zuruecksetzen - 'tft' wird an anderer Stelle (Text ueber
        // Hardware-Rotation, updateClock() usw.) mit swapBytes=false erwartet.

        // Reset the state - 'tft' is expected to have swapBytes=false
        // elsewhere (text via hardware rotation, updateClock(), etc.).
        tft.setSwapBytes(false);
    }
#endif


    // Uebernimmt eine neue Rotation (0-3 oder TFT_ROTATION_NA) fuer Display 1 oder 2:
    // speichert sie und wendet sie bei Hardware-Rotation sofort am Chip an.
    // Bei "n.a." bekommt der Chip 0 Grad (effectiveRotation()), nie den Wert 4 (waere beim GC9A01 gespiegelt).

    // Applies a new rotation (0-3 or TFT_ROTATION_NA) for display 1 or 2:
    // stores it and, with hardware rotation, applies it on the chip right away.
    // For "n.a." the chip gets 0 degrees (effectiveRotation()), never the value 4 (mirrored on the GC9A01).

    void applyDisplayRotation(uint8_t displayNum, uint8_t newRotation) {
        uint8_t& rotation = (displayNum == 1) ? tftRotation1 : tftRotation2;
        bool& firstRunFlag = (displayNum == 1) ? firstRun : firstRun2;

        // firstRun nur bei tatsaechlicher Aenderung zuruecksetzen, sonst
        // startet jedes Speichern die Bahnhofsmodus-Wartephase neu.

        // Only reset firstRun on an actual change, otherwise every save
        // restarts the station-mode wait phase.
        if (rotation != newRotation) {
            firstRunFlag = true;
            clockFrameDirty[displayNum - 1] = true; // auch im Spiegelbetrieb (Display 2 ohne eigenes Rendern) neu senden
                                                    // resend in mirror mode too (display 2 without its own rendering)
            if (newRotation == TFT_ROTATION_NA) displayNeedsBlank[displayNum - 1] = true; // letztes Uhrbild muss weg
                                                                                          // last clock image has to go
        }
        rotation = newRotation;
        preferences.putUChar((displayNum == 1) ? PK_TFT_ROTATION1 : PK_TFT_ROTATION2, rotation);

        if (!gc9d01SwRotation) {
            // setCS1()/setCS2() setzen die neue Rotation am gewaehlten Chip
            // (hardwareRotation()) - einmal auswaehlen genuegt.

            // setCS1()/setCS2() set the new rotation on the selected chip
            // (hardwareRotation()) - selecting it once is enough.
            if (displayNum == 1) setCS1(LOW); else setCS2(LOW);
            setCSIdle(); // zurueck auf den Ausgangszustand, damit loop() im gewohnten Zustand weiterlaeuft
                         // back to the initial state, so loop() continues from its usual state
        }
    }


    // Werksvorgaben fuer min. Helligkeit und ADC-Schwellwerte je nach
    // Helligkeitsverfahren: mit Backlight kann die PWM fast bis auf 0 dimmen
    // und zwischen den Schwellen stufenlos regeln; ohne Backlight werden die
    // Pixel abgedunkelt, unter ~100 wird das Zifferblatt unleserlich.

    // Factory defaults for min. brightness and ADC thresholds depending on
    // the brightness method: with a backlight the PWM can dim almost to 0 and
    // regulate steplessly between the thresholds; without one the pixels get
    // darkened, below ~100 the clock face becomes unreadable.

    void putBrightnessDefaults(bool backlight) {
        preferences.putUChar(PK_MIN_BRIGHTNESS, backlight ? 5 : 100);
        preferences.putInt(PK_LOW_THRESHOLD, backlight ? 1 : 40);
        preferences.putInt(PK_HIGH_THRESHOLD, backlight ? 100 : 60); // 100 statt frueher 255: das Formularfeld erlaubt nur 0-100, der
                                                                     // Lichtwert (5-100 %) ueberschreitet 100 ohnehin nie - gleiche Wirkung
                                                                     // 100 instead of the former 255: the form field only allows 0-100, the
                                                                     // light value (5-100 %) never exceeds 100 anyway - same effect
    }


    // Automatische Helligkeit (Fotowiderstand) ein-/ausschalten - inklusive
    // der Versorgungspins des Spannungsteilers: setup() legt sie bei
    // ausgeschalteter Automatik auf INPUT, ein spaeteres Einschalten (Formular,
    // Preset) liess den Teiler sonst bis zum Neustart ohne Spannung.

    // Switch automatic brightness (photoresistor) on/off - including the
    // voltage divider's supply pins: setup() sets them to INPUT when the
    // automatic is off, so enabling it later (form, preset) would otherwise
    // leave the divider unpowered until the next restart.

    void setAutoBrightness(bool enabled) {
#ifdef ADC_3V
        enabled = enabled && photoresistorFound;
        if (enabled) {
            pinMode(ADC_GND, OUTPUT);
            pinMode(ADC_3V, OUTPUT);
            digitalWrite(ADC_GND, LOW);
            digitalWrite(ADC_3V, HIGH);
        }
        else {
            pinMode(ADC_GND, INPUT);
            pinMode(ADC_3V, INPUT);
        }
#else
        enabled = false;
#endif
        useAdc = enabled;
        preferences.putBool(PK_USE_ADC, useAdc);
    }


    // Helligkeitswert aus einem Preset (URL-Parameter) uebernehmen und
    // speichern - gemeinsam fuer /api/setMode und switchToNextPreset(), damit
    // beide Wege gleich klemmen. false = kein Helligkeits-Schluessel.
    // Wirkt beim naechsten updateBrightness() in loop().

    // Take over and store a brightness value from a preset (URL parameter) -
    // shared by /api/setMode and switchToNextPreset(), so both paths clamp
    // the same way. false = not a brightness key. Takes effect on the next
    // updateBrightness() in loop().

    bool applyBrightnessPresetValue(const String& key, const String& value) {
        if (key == "minBrightness") {
            minBrightness = (uint8_t)constrain(value.toInt(), 0, 255);
            preferences.putUChar(PK_MIN_BRIGHTNESS, minBrightness);
        }
        else if (key == "maxBrightness") {
            maxBrightness = (uint8_t)constrain(value.toInt(), 0, 255);
            preferences.putUChar(PK_MAX_BRIGHTNESS, maxBrightness);
        }
        else if (key == "brightStart") {
            brightStartHour = (uint8_t)constrain(value.toInt(), 0, 23);
            preferences.putUChar(PK_BRIGHT_START_HOUR, brightStartHour);
        }
        else if (key == "brightEnd") {
            brightEndHour = (uint8_t)constrain(value.toInt(), 0, 23);
            preferences.putUChar(PK_BRIGHT_END_HOUR, brightEndHour);
        }
        else if (key == "lowThreshold") {
            lowThreshold = constrain(value.toInt(), 0, 100);
            preferences.putInt(PK_LOW_THRESHOLD, lowThreshold);
        }
        else if (key == "highThreshold") {
            highThreshold = constrain(value.toInt(), 0, 100);
            preferences.putInt(PK_HIGH_THRESHOLD, highThreshold);
        }
        else if (key == "gamma") {
            gammaBrightness = constrain(value.toFloat(), 0.1f, 3.0f);
            preferences.putFloat(PK_GAMMA_BRIGHTNESS, gammaBrightness);
        }
        else if (key == "autoBrightness") {
            setAutoBrightness(value == "1" || value.equalsIgnoreCase("true"));
        }
        else {
            return false;
        }
        return true;
    }


    // Setzt Pin 3 passend zu useBacklight: an = PWM anhaengen und aktuelle
    // Helligkeit ausgeben; aus = PWM abhaengen und Pin fest HIGH (volle
    // Beleuchtung). HIGH auch beim Start: ein verdrahteter, aber offener
    // BL-Eingang (z.B. GC9D01 mit abgeschalteter Regelung) bliebe sonst je
    // nach Modul dunkel - auf der GC9A01-Platine ist Pin 3 unbelegt.

    // Sets pin 3 according to useBacklight: on = attach PWM and output the
    // current brightness; off = detach PWM and drive the pin HIGH (full
    // backlight). HIGH at boot too: a wired but floating BL input (e.g. a
    // GC9D01 with the control switched off) would otherwise stay dark on some
    // modules - on the GC9A01 board pin 3 is unconnected.

    void applyBacklightPin() {
        if (useBacklight) {
            if (!backlightAttached) {
                backlightAttached = ledcAttach(TFT_Backlight, BACKLIGHT_FREQ, BACKLIGHT_RESOLUTION);
                if (!backlightAttached) DEBUG_PRINTLN("[Display] Error: couldnt attach backlight PWM");
            }
            if (backlightAttached) ledcWrite(TFT_Backlight, currentBrightness);
            return;
        }
        if (backlightAttached) {
            ledcDetach(TFT_Backlight);
            backlightAttached = false;
        }
        pinMode(TFT_Backlight, OUTPUT);
        digitalWrite(TFT_Backlight, HIGH);
    }


    // Umschalten zur Laufzeit (Weboberflaeche): speichert die Einstellung,
    // setzt min. Helligkeit/Schwellwerte auf die passenden Werksvorgaben und
    // faerbt Zifferblatt und Zeiger neu ein (mit Backlight unverdunkelt).

    // Switching at runtime (web UI): stores the setting, resets min.
    // brightness/thresholds to the matching factory defaults and re-tints
    // the clock face and hands (undimmed with a backlight).

    void setBacklightMode(bool enabled) {
        if (enabled == useBacklight) return;
        useBacklight = enabled;
        preferences.putBool(PK_USE_BACKLIGHT, enabled);

        putBrightnessDefaults(enabled);
        minBrightness = preferences.getUChar(PK_MIN_BRIGHTNESS, 100);
        lowThreshold = preferences.getInt(PK_LOW_THRESHOLD, 40);
        highThreshold = preferences.getInt(PK_HIGH_THRESHOLD, 60);

        DEBUG_PRINTLN(String("[Display] Backlight ") + (enabled ? "on" : "off"));

        // Erst Pin umstellen und die Helligkeit fuer das neue Verfahren
        // berechnen, DANN neu einfaerben - sonst wuerde z.B. beim Abschalten
        // der niedrige PWM-Wert (5) kurz als Pixel-Abdunklung sichtbar.

        // Switch the pin and compute brightness for the new method first,
        // THEN re-tint - otherwise e.g. when switching off, the low PWM value
        // (5) would briefly show up as pixel dimming.
        // Mit max. Helligkeit neu anfangen: liegt das Umgebungslicht zwischen
        // den Schwellen, aendert updateBrightness() targetBrightness nicht -
        // ohne Reset bliebe sonst der Zielwert des alten Verfahrens stehen.

        // Restart from max. brightness: if ambient light lies between the
        // thresholds, updateBrightness() leaves targetBrightness unchanged -
        // without a reset the old method's target value would stick.
        targetBrightness = maxBrightness;
        currentBrightness = maxBrightness;

        applyBacklightPin();
        updateBrightness();
        freeClockFaceBuffer();
        loadClockFace();
        loadHandSprites();
        lastHandBrightness = currentBrightness; // Zeiger sind eben mit diesem Wert eingefaerbt
                                                // hands were just tinted with this value
        updateClock();
    }


    // Passt die Helligkeit eines Pixels basierend auf der aktuellen Helligkeitseinstellung an.
    // Adjusts a pixel's brightness based on the current brightness setting.

    uint16_t setPixelBrightness(uint16_t pixel) {

        // Mit Hintergrundbeleuchtung regelt die PWM auf TFT_Backlight die
        // Helligkeit (updateBrightness()) - die Pixel bleiben unveraendert.

        // With a backlight the PWM on TFT_Backlight controls brightness
        // (updateBrightness()) - pixels stay unchanged.
        if (useBacklight) return pixel;

        // Wenn die Helligkeit maximal ist oder der Pixel transparent/schwarz ist, direkt zurückgeben
        // If brightness is at maximum or the pixel is transparent/black, return immediately
        if (pixel == TRANSPARENT_COLOR || pixel == 0x0000 || currentBrightness == 255) {
            return pixel;
        }

        // Multiplikator einmal berechnen (statt 3x Division)
        // Compute the multiplier once (instead of 3x division)
        uint32_t brightnessFactor = (uint32_t)currentBrightness;

        // Farben extrahieren
        // Extract colors
        uint32_t r = (pixel & 0xF800);
        uint32_t g = (pixel & 0x07E0);
        uint32_t b = (pixel & 0x001F);

        // Multiplikation mit Brightness (optimiert, kein Shift nötig)
        // Multiply by brightness (optimized, no shift needed)
        r = ((r * brightnessFactor) >> 8) & 0xF800;
        g = ((g * brightnessFactor) >> 8) & 0x07E0;
        b = ((b * brightnessFactor) >> 8) & 0x001F;

        // Farbwerte zusammenfügen; gedimmtes Reingrün kann genau TRANSPARENT_COLOR ergeben und würde sonst verschwinden
        // Combine color values; dimmed pure green can hit exactly TRANSPARENT_COLOR and would otherwise vanish
        uint16_t result = r | g | b;
        return (result == TRANSPARENT_COLOR) ? 0x0100 : result;
    }


    // RLE-Kompression fuer Zifferblatt-BMPs (face_*.bmp): eigenes PackBits-artiges Verfahren fuer 16-Bit RGB565, Worst-Case-Overhead nur ~0.4%.
    // Datei beginnt mit Magic "RLEB" statt "BM" (kein gueltiges BMP mehr) + Breite/Hoehe/Groessen (je int32/uint32) + RLE-Datenstrom.
    // Steuerbyte je Paket: 0-127=Literal-Lauf (C+1 Pixel roh), 129-255=Wiederholung ((257-C) identische Pixel, nur 1x gespeichert), 128=unbenutzt.

    // RLE compression for clock-face BMPs (face_*.bmp): custom PackBits-style scheme for 16-bit RGB565, worst-case overhead only ~0.4%.
    // File starts with magic "RLEB" instead of "BM" (no longer a valid BMP) + width/height/sizes (int32/uint32 each) + RLE data stream.
    // Control byte per packet: 0-127=literal run (C+1 raw pixels), 129-255=repeat ((257-C) identical pixels, stored once), 128=unused.

    bool isRleFace(const uint8_t* header4) {
        return header4[0] == 'R' && header4[1] == 'L' && header4[2] == 'E' && header4[3] == 'B';
    }


    // Obergrenze fuer die kodierte Groesse (fuer die Allokation des Zielpuffers).
    // Upper bound for the encoded size (for allocating the destination buffer).

    size_t rleMaxEncodedSize(size_t pixelCount) {
        return pixelCount * 2 + (pixelCount / 128 + 2);
    }


    // Kodiert ein Array von RGB565-Pixeln PackBits-artig (Lauflaengenkodierung);
    // gibt die Anzahl tatsaechlich geschriebener Bytes in 'out' zurueck

    // Encodes an array of RGB565 pixels PackBits-style (run-length encoding);
    // returns the number of bytes actually written to 'out'

    size_t rleEncode565(const uint16_t* pixels, size_t count, uint8_t* out) {
        size_t i = 0, o = 0;
        while (i < count) {
            if (i % 5000 == 0) yield(); // Watchdog-Reset vermeiden bei grossen Bildern
                                        // avoid watchdog reset on large images
            size_t runLen = 1;
            while (i + runLen < count && runLen < 128 && pixels[i + runLen] == pixels[i]) runLen++;

            if (runLen >= 2) {
                out[o++] = (uint8_t)(257 - runLen);
                out[o++] = pixels[i] & 0xFF;
                out[o++] = pixels[i] >> 8;
                i += runLen;
            }
            else {
                size_t litStart = i;
                size_t litLen = 0;
                while (i < count && litLen < 128) {
                    size_t rl = 1;
                    while (i + rl < count && rl < 128 && pixels[i + rl] == pixels[i]) rl++;
                    if (rl >= 2) break;
                    litLen++;
                    i++;
                }
                out[o++] = (uint8_t)(litLen - 1);
                for (size_t k = 0; k < litLen; k++) {
                    uint16_t px = pixels[litStart + k];
                    out[o++] = px & 0xFF;
                    out[o++] = px >> 8;
                }
            }
        }
        return o;
    }


    // Dekodiert einen mit rleEncode565() erzeugten Datenstrom vollstaendig in ein
    // bereits vorhandenes uint16_t-Pixel-Array (RGB565)

    // Fully decodes a stream produced by rleEncode565() into an
    // already-allocated uint16_t pixel array (RGB565)

    void rleDecode565(const uint8_t* in, size_t inSize, uint16_t* out, size_t outCount) {
        size_t i = 0, o = 0;
        while (i < inSize && o < outCount) {
            if (o % 5000 == 0) yield(); // Watchdog-Reset vermeiden bei grossen Bildern
                                        // avoid watchdog reset on large images
            uint8_t ctrl = in[i++];
            if (ctrl <= 127) {
                size_t len = ctrl + 1;
                for (size_t k = 0; k < len && o < outCount && i + 1 < inSize; k++) {
                    uint16_t px = in[i] | (in[i + 1] << 8);
                    i += 2;
                    out[o++] = px;
                }
            }
            else {
                size_t len = 257 - ctrl;
                if (i + 1 >= inSize) break;
                uint16_t px = in[i] | (in[i + 1] << 8);
                i += 2;
                for (size_t k = 0; k < len && o < outCount; k++) out[o++] = px;
            }
        }
    }


    // Wie rleDecode565(), schreibt aber direkt in einen zeilenweise auf 4-Byte-
    // Grenzen gepolsterten BMP-Pixelbereich - vermeidet einen zusaetzlichen
    // Zwischenpuffer, spart bei 240x240 bis zu ~115 KB Spitzen-Heap-Bedarf.

    // Like rleDecode565(), but writes directly into a BMP pixel area
    // padded to 4-byte row boundaries - avoids an extra
    // intermediate buffer, saving up to ~115 KB peak heap at 240x240.

    void rleDecode565ToBmpRows(const uint8_t* in, size_t inSize, uint8_t* pixelArea, int width, int height, int rowStride) {
        size_t i = 0;
        int col = 0, row = 0;
        size_t written = 0;
        const size_t total = (size_t)width * height;

        while (i < inSize && written < total && row < height) {
            if (written % 5000 == 0) yield(); // Watchdog-Reset vermeiden bei grossen Bildern
                                              // avoid watchdog reset on large images
            uint8_t ctrl = in[i++];
            bool literal = ctrl <= 127;
            size_t len;
            uint16_t litPx = 0;

            if (literal) {
                len = ctrl + 1;
            }
            else {
                len = 257 - ctrl;
                if (i + 1 >= inSize) break;
                litPx = in[i] | (in[i + 1] << 8);
                i += 2;
            }

            for (size_t k = 0; k < len && written < total; k++) {
                uint16_t px;
                if (literal) {
                    if (i + 1 >= inSize) { written = total; break; }
                    px = in[i] | (in[i + 1] << 8);
                    i += 2;
                }
                else {
                    px = litPx;
                }

                uint8_t* dst = pixelArea + (size_t)row * rowStride + (size_t)col * 2;
                dst[0] = px & 0xFF;
                dst[1] = px >> 8;

                col++;
                if (col >= width) { col = 0; row++; }
                written++;
            }
        }
    }


    // Eingebaute Standardgrafiken (RLE, siehe DISPLAY_GEOMETRY in config.h)

    // Built-in default graphics (RLE, see DISPLAY_GEOMETRY in config.h)

    // Entpackt das Standard-Zifferblatt des aktiven Displaytyps nach 'dest'
    // (CLOCK_WIDTH x CLOCK_HEIGHT Pixel).
    // Unpacks the active display type's default clock face into 'dest'
    // (CLOCK_WIDTH x CLOCK_HEIGHT pixels).
    void decodeDefaultFace(uint16_t* dest) {
        if (!dest) return;
        rleDecode565(displayGeom->face.data, displayGeom->face.size, dest, displayGeom->face.pixels);
    }

    // Wie decodeDefaultFace(), legt den Puffer aber selbst an (PSRAM bevorzugt) -
    // der Aufrufer gibt ihn mit free() frei. nullptr bei Speichermangel.
    // Like decodeDefaultFace(), but allocates the buffer itself (PSRAM
    // preferred) - the caller frees it with free(). nullptr when out of memory.
    uint16_t* allocDefaultFace() {
        uint16_t* buf = (uint16_t*)preferPsramMalloc((size_t)displayGeom->face.pixels * sizeof(uint16_t));
        if (!buf) {
            DEBUG_PRINTLN("[Display] Error: couldnt allocate buffer for the default clock face");
            return nullptr;
        }
        decodeDefaultFace(buf);
        return buf;
    }

    // Liest eine face_*.bmp-Datei (Standard-BMP oder RLEB-komprimiert) direkt in
    // 'dest' (expectedW x expectedH, RGB565, Top-Down). False bei Lesefehler
    // oder falschen Dimensionen (dest bleibt dann unveraendert).

    // Reads a face_*.bmp file (standard BMP or RLEB-compressed) directly into
    // 'dest' (expectedW x expectedH, RGB565, top-down). False on read error
    // or wrong dimensions (dest stays unchanged in that case).

    bool loadFaceBmpInto(const String& path, uint16_t* dest, int32_t expectedW, int32_t expectedH) {
        File f = LittleFS.open(path, "r");
        if (!f) return false;

        uint8_t magic[4];
        if (f.read(magic, 4) != 4) { f.close(); return false; }

        if (isRleFace(magic)) {
            uint8_t rest[16];
            if (f.read(rest, 16) != 16) { f.close(); return false; }
            int32_t width = *(int32_t*)&rest[0];
            int32_t height = *(int32_t*)&rest[4];
            uint32_t compressedSize = *(uint32_t*)&rest[8];
            uint32_t uncompressedSize = *(uint32_t*)&rest[12];

            if (width != expectedW || height != expectedH ||
                uncompressedSize != (uint32_t)width * height * 2) {
                f.close();
                return false;
            }

            uint8_t* compBuf = (uint8_t*)preferPsramMalloc(compressedSize);
            if (!compBuf) { f.close(); return false; }
            if (f.read(compBuf, compressedSize) != compressedSize) {
                free(compBuf); f.close(); return false;
            }
            f.close();

            rleDecode565(compBuf, compressedSize, dest, (size_t)width * height);
            free(compBuf);
            return true;
        }
        else {
            f.seek(0);
            uint8_t header[54];
            if (f.read(header, 54) != 54 || header[0] != 'B' || header[1] != 'M') {
                f.close();
                return false;
            }
            int32_t width = *(int32_t*)&header[18];
            int32_t height = *(int32_t*)&header[22];
            uint16_t bpp = *(uint16_t*)&header[28];
            uint32_t offset = *(uint32_t*)&header[10];

            if (width != expectedW || abs(height) != expectedH || bpp != 16) {
                f.close();
                return false;
            }
            bool flip = height > 0;
            height = abs(height);

            int rowSize = ((width * 2 + 3) / 4) * 4;
            for (int y = 0; y < height; y++) {
                int row = flip ? height - 1 - y : y;
                f.seek(offset + (uint32_t)rowSize * y);
                // Rueckgabewert pruefen: bei unvollstaendiger Zeile false liefern,
                // sonst enthaelt dest teils uninitialisierten Speicher, obwohl der
                // Aufrufer es als vollstaendig geladen behandelt.

                // Check the return value: return false on an incomplete row,
                // otherwise dest partly holds uninitialized memory while the
                // caller treats it as fully loaded.
                if (f.read((uint8_t*)&dest[row * width], width * 2) != width * 2) {
                    f.close();
                    return false;
                }
            }
            f.close();
            return true;
        }
    }

    // Zeigerbild (w x h, je altes oder neues Mass) waagerecht mittig und unten
    // buendig in 'dest' (HAND_WIDTH x HAND_HEIGHT) setzen, der Rest transparent -
    // Drehpunkt und Stueck unter dem Drehpunkt liegen dann wie im neuen Format.

    // Put a hand image (w x h, old or new size each) horizontally centred and
    // flush at the bottom into 'dest' (HAND_WIDTH x HAND_HEIGHT), the rest
    // transparent - pivot and the part below it then sit like in the new format.
    void placeHand(const uint16_t* src, int w, int h, uint16_t* dest) {
        int ox = (HAND_WIDTH - w) / 2, oy = HAND_HEIGHT - h;
        for (int i = 0; i < HAND_WIDTH * HAND_HEIGHT; i++) dest[i] = TRANSPARENT_COLOR;
        for (int y = 0; y < h; y++) memcpy(dest + (y + oy) * HAND_WIDTH + ox, src + y * w, (size_t)w * sizeof(uint16_t));
    }

    // Eingebaute Standardzeiger liegen im alten Format vor
    // Built-in default hands are in the old format
    void copyLegacyHand(const uint16_t* legacy, uint16_t* dest) {
        placeHand(legacy, HAND_LEGACY_WIDTH, HAND_LEGACY_HEIGHT, dest);
    }

    // Laedt eine Zeigerdatei (BMP/RLEB) in einem der vier gueltigen Formate (Breite
    // und Hoehe je alt oder neu) nach 'dest'. False, wenn keines passt.

    // Loads a hand file (BMP/RLEB) in one of the four valid formats (width and
    // height old or new each) into 'dest'. False if none matches.
    bool loadHandPixels(const String& path, uint16_t* dest) {
        if (loadFaceBmpInto(path, dest, HAND_WIDTH, HAND_HEIGHT)) return true;
        const int formats[3][2] = {
            { HAND_WIDTH, HAND_LEGACY_HEIGHT }, { HAND_LEGACY_WIDTH, HAND_HEIGHT }, { HAND_LEGACY_WIDTH, HAND_LEGACY_HEIGHT }
        };
        uint16_t* tmp = (uint16_t*)preferPsramMalloc((size_t)HAND_WIDTH * HAND_HEIGHT * sizeof(uint16_t));
        if (!tmp) return false;
        bool ok = false;
        for (const auto& fm : formats) {
            if (loadFaceBmpInto(path, tmp, fm[0], fm[1])) {
                placeHand(tmp, fm[0], fm[1], dest);
                ok = true;
                break;
            }
        }
        free(tmp);
        return ok;
    }

    // Zielgroesse beim Speichern einer Zeigerdatei: eines der vier gueltigen
    // Formate bleibt unskaliert, alles andere wird wie frueher aufs alte skaliert.

    // Target size when storing a hand file: one of the four valid formats stays
    // unscaled, everything else is scaled to the old format as before.
    void handTargetSize(const char* path, int& outW, int& outH) {
        outW = HAND_LEGACY_WIDTH;
        outH = HAND_LEGACY_HEIGHT;
        int32_t w, h;
        if (readImageSize(path, w, h) && isValidHandSize(w, h)) {
            outW = w;
            outH = h;
        }
    }

    // Breite und Hoehe aus dem Kopf einer BMP- oder RLEB-Datei, ohne Pixel zu lesen
    // Width and height from the header of a BMP or RLEB file, without reading pixels
    bool readImageSize(const char* path, int32_t& w, int32_t& h) {
        w = 0;
        h = 0;
        File f = LittleFS.open(path, "r");
        if (!f) return false;
        uint8_t head[26];
        int n = f.read(head, sizeof(head));
        f.close();

        if (n >= 12 && isRleFace(head)) {
            w = *(int32_t*)&head[4];
            h = *(int32_t*)&head[8];
        }
        else if (n == (int)sizeof(head) && head[0] == 'B' && head[1] == 'M') {
            w = *(int32_t*)&head[18];
            h = abs(*(int32_t*)&head[22]);
        }
        return w > 0 && h > 0;
    }

    bool isValidHandSize(int32_t w, int32_t h) {
        return (w == HAND_WIDTH || w == HAND_LEGACY_WIDTH) && (h == HAND_HEIGHT || h == HAND_LEGACY_HEIGHT);
    }

    // Kurzbezeichnung des Zeigerformats fuer den Dateimanager
    // Short label of the hand format for the file manager
    String handFormatLabel(const String& path) {
        int32_t w, h;
        if (!readImageSize(path.c_str(), w, h)) return "";
        if (!isValidHandSize(w, h)) return translate("unsupported size - default hand is used");
        if (w == HAND_LEGACY_WIDTH && h == HAND_LEGACY_HEIGHT) return translate("old format");
        if (w == HAND_WIDTH && h == HAND_HEIGHT) return translate("new format");
        return (w == HAND_WIDTH) ? translate("new width") : translate("new length");
    }

    // Laedt das gewaehlte Zifferblatt (BMP/RLEB, Fallback aufs Standardbild) in
    // clockFaceBuffer, wendet die Helligkeit an und zeichnet in backgroundSprite.
    // Haelt clockFaceBuffer (Rohbild) und clockFaceBrightBuffer (angepasst)
    // aktuell. Aus loadClockFace() ausgelagert, damit buildHandComposite() die
    // Vorbereitung mitnutzen kann. Rueckgabe: 2=beide Puffer ok, 1=nur Rohbild, 0=nichts.

    // Loads the selected clock face (BMP/RLEB, falls back to the default) into
    // clockFaceBuffer, applies brightness and draws into backgroundSprite.
    // Keeps clockFaceBuffer (raw) and clockFaceBrightBuffer (adjusted)
    // up to date. Split out of loadClockFace() so buildHandComposite() can
    // reuse this prep. Returns: 2=both buffers ok, 1=raw only, 0=neither.

    int prepareClockFaceCache() {
        bool forceRecompute = false; // Neues Zifferblatt geladen -> Cache muss neu berechnet werden
                                     // New clock face loaded -> cache must be recalculated
        // Prüfen, ob Buffer schon existiert
        // Check whether the buffer already exists
        if (!clockFaceBuffer) {
            size_t bufSize = CLOCK_WIDTH * CLOCK_HEIGHT * sizeof(uint16_t);
            if (psramFound() and ESP.getFreePsram() > bufSize) {
                DEBUG_PRINTLN("[PSRAM] Allocate psram");
                clockFaceBuffer = (uint16_t*)ps_malloc(bufSize);
            }
            else {
                // String(bufSize) explizit: "..." + bufSize waere Zeigerarithmetik
                // auf dem String-Literal statt Verkettung und liest wild aus dem Speicher.

                // String(bufSize) explicitly: "..." + bufSize would be pointer
                // arithmetic on the literal instead of concatenation, reading wild memory.
                DEBUG_PRINTLN("allocate ram: " + String(bufSize));
                DEBUG_PRINTLN("[PSRAM] Allocate ram");
                clockFaceBuffer = (uint16_t*)malloc(bufSize);
            }
            if (!clockFaceBuffer) {
                DEBUG_PRINTLN("[PSRAM] Error: couldnt allocate clockFaceBuffer RAM!");
                return 0;
            }


            if (!selectedBackground.startsWith("/")) selectedBackground = "/" + selectedBackground;
            // Bild aus Datei laden und dekodieren (Standard-BMP oder RLEB-komprimiert)
            // Load and decode the image from file (standard BMP or RLEB-compressed)
            bool loaded = false;
            if (LittleFS.exists(selectedBackground)) {
                loaded = loadFaceBmpInto(selectedBackground, clockFaceBuffer, CLOCK_WIDTH, CLOCK_HEIGHT);
            }
            if (!loaded) {
                // Fallback: Standard-Zifferblatt aus Array kopieren (auch bei
                // Lesefehler oder falschen Dimensionen - vorher blieb der
                // Puffer in diesem Fall unveraendert/undefiniert)

                // Fallback: copy the built-in default clock face from the array (also on
                // read errors or wrong dimensions - previously the buffer
                // was left unchanged/undefined in this case)
                decodeDefaultFace(clockFaceBuffer);
            }
            forceRecompute = true;
        }

        // Breiten aus Dateinamen extrahieren
        // Extract widths from the filename
        parseBackgroundFilename(selectedBackground, hourHandWidth, minuteHandWidth, secondHandWidth);
        updateHandWidths(hourHandWidth, minuteHandWidth, secondHandWidth);


        // Cache-Puffer fuer die bereits helligkeitsangepasste Fassung des
        // Zifferblatts anlegen, falls noch nicht vorhanden.

        // Allocate the cache buffer for the already brightness-adjusted
        // version of the clock face, if not already present.
        if (!clockFaceBrightBuffer) {
            size_t bufSize = CLOCK_WIDTH * CLOCK_HEIGHT * sizeof(uint16_t);
            if (psramFound() and ESP.getFreePsram() > bufSize) {
                clockFaceBrightBuffer = (uint16_t*)ps_malloc(bufSize);
            }
            else {
                clockFaceBrightBuffer = (uint16_t*)malloc(bufSize);
            }
            if (!clockFaceBrightBuffer) {
                DEBUG_PRINTLN("[PSRAM] Error: couldnt allocate clockFaceBrightBuffer RAM! Falling back to per-pixel path");
                // Ohne Cache muss der Aufrufer Pixel fuer Pixel selbst rechnen
                // (funktionsfaehig, nur ohne die Optimierung) - siehe
                // loadClockFace() weiter unten.

                // Without the cache the caller has to do the per-pixel work
                // itself (works, just without the optimization) - see
                // loadClockFace() further below.
                return 1;
            }
            forceRecompute = true;
        }

        // Die teure Pixel-fuer-Pixel Helligkeitsanpassung nur durchlaufen, wenn
        // sich seit dem letzten Mal etwas geaendert hat (neues Zifferblatt oder
        // Helligkeit) - im Regelfall (jeder Tick) entfaellt dieser Durchlauf.

        // Only run the costly per-pixel brightness adjustment when
        // something changed since last time (new clock face or
        // brightness) - normally (every tick) this is skipped.
        // Mit Backlight aendert die Helligkeit keine Pixel - dann nur bei neuem
        // Zifferblatt bzw. Umschalten (freeClockFaceBuffer() -> forceRecompute).

        // With a backlight, brightness changes no pixels - then only on a new
        // clock face or when switching (freeClockFaceBuffer() -> forceRecompute).
        if (forceRecompute || (!useBacklight && currentBrightness != lastAppliedBrightness)) {
            for (int i = 0; i < CLOCK_WIDTH * CLOCK_HEIGHT; i++) {
                clockFaceBrightBuffer[i] = setPixelBrightness(clockFaceBuffer[i]);
            }
            lastAppliedBrightness = currentBrightness;
        }

        // GC9D01 mit PSRAM ueberspringt die Hardware-Rotation (tft.setRotation()
        // wirkungslos); Zeiger werden per Software gedreht (rotatedAngle()), das
        // Zifferblatt hier ebenso - sonst blieb nur der Hintergrund ungedreht.
        // 'rotation' ist die Zielausrichtung fuer DIESES Display. Bei Hardware-
        // Rotation uebernimmt das MADCTL-Register die Drehung, hier wird dann
        // nur faceOrientation=0 verwendet.

        // GC9D01 with PSRAM skips hardware rotation (tft.setRotation() has no
        // effect); hands are rotated in software (rotatedAngle()), the clock
        // face here too - otherwise only the background would stay unrotated.
        // 'rotation' is the target orientation for THIS display. With hardware
        // rotation the chip's MADCTL register does the rotating, so only
        // faceOrientation=0 is used here.

        return 2;
    }


    // Software-Ausrichtung fuer DIESES Display. Bei Hardware-Rotation (alle
    // Boards ausser GC9D01) uebernimmt MADCTL die Drehung, hier bleibt es bei 0.

    // Software orientation for THIS display. With hardware rotation (every
    // board except GC9D01) MADCTL does the rotation, so this stays 0.

    int faceOrientationFor(uint8_t rotation) {
        return gc9d01SwRotation ? rotation : 0;
    }


    // Kopiert das Zifferblatt in ein Sprite, bei Bedarf gedreht - gleiche
    // Abbildung wie loadClockFace(), aber in ein beliebiges Ziel, da
    // buildHandComposite() in sein eigenes Zwischenbild-Sprite zeichnet.

    // Copies the clock face into a sprite, rotated if needed - same mapping
    // as loadClockFace(), but into any target, since buildHandComposite()
    // draws into its own composite sprite.

    bool blitFaceIntoSprite(LGFX_Sprite& dest, uint8_t rotation) {
        if (dest.width() <= 0) return false;
        if (prepareClockFaceCache() != 2) return false;

        const int N = CLOCK_WIDTH;
        int faceOrientation = faceOrientationFor(rotation);

        if (faceOrientation == 0) {
            dest.pushImage(0, 0, N, CLOCK_HEIGHT, clockFaceBrightBuffer);
            return true;
        }

        for (int y = 0; y < N; y++) {
            for (int x = 0; x < N; x++) {
                int srcX, srcY;
                switch (faceOrientation) {
                    case 1:  srcX = y;         srcY = N - 1 - x; break; // 90 Grad im Uhrzeigersinn
                                                                        // 90 degrees clockwise
                    case 2:  srcX = N - 1 - x; srcY = N - 1 - y; break; // 180 Grad
                                                                        // 180 degrees
                    default: srcX = N - 1 - y; srcY = x;         break; // 270 Grad im Uhrzeigersinn
                                                                        // 270 degrees clockwise
                }
                rowBuffer[x] = clockFaceBrightBuffer[srcY * N + srcX];
            }
            dest.pushImage(0, y, N, 1, rowBuffer);
        }
        return true;
    }


    // Zeichnet das Zifferblatt ins backgroundSprite - unveraendertes Verhalten
    // fuer alle bisherigen Aufrufer.

    // Draws the clock face into backgroundSprite - unchanged behaviour for all
    // existing callers.

    void loadClockFace(uint8_t rotation) {
        int cacheState = prepareClockFaceCache();
        if (cacheState == 0) return;

        if (cacheState == 1) {
            // Kein Helligkeits-Cache verfuegbar: Pixel fuer Pixel rechnen,
            // Rotation wird dabei beruecksichtigt (sonst bliebe das Zifferblatt
            // bei Software-Rotation ungedreht, waehrend die Zeiger sich drehen).

            // No brightness cache available: compute pixel by pixel, taking
            // rotation into account (otherwise the face would stay unrotated
            // under software rotation while the hands rotate).
            const int N = CLOCK_WIDTH;
            int fallbackOrientation = faceOrientationFor(rotation);

            for (int y = 0; y < N; y++) {
                for (int x = 0; x < N; x++) {
                    int srcX = x, srcY = y;
                    switch (fallbackOrientation) {
                        case 1:  srcX = y;         srcY = N - 1 - x; break;
                        case 2:  srcX = N - 1 - x; srcY = N - 1 - y; break;
                        case 3:  srcX = N - 1 - y; srcY = x;         break;
                        default: break; // 0 Grad: unveraendert
                                        // 0 degrees: unchanged
                    }
                    rowBuffer[x] = setPixelBrightness(clockFaceBuffer[srcY * N + srcX]);
                }
                backgroundSprite.pushImage(0, y, N, 1, rowBuffer);
            }
            return;
        }

        int faceOrientation = faceOrientationFor(rotation);

        if (faceOrientation == 0) {
            // Guenstiger Regelfall: den vorberechneten Puffer in einem Rutsch ins
            // Sprite kopieren - loescht dabei auch die alte Zeigerposition vom
            // letzten Tick, ohne die Helligkeit erneut pro Pixel berechnen zu muessen.

            // Cheap common case: copy the precomputed buffer into the
            // sprite in one go - this also clears the old hand position
            // from the last tick, without recalculating brightness per pixel.
            backgroundSprite.pushImage(0, 0, CLOCK_WIDTH, CLOCK_HEIGHT, clockFaceBrightBuffer);
        }
        else {
            // Zeilenweise mit gedrehten Quellkoordinaten (quadratisch, kein
            // Breiten-/Hoehentausch). Richtung passend zur Zeigerformel
            // (angle + orientation*90, im Uhrzeigersinn) gewaehlt.

            // Row by row with rotated source coordinates (square, no width/
            // height swap needed). Direction matches the hand formula
            // (angle + orientation*90, clockwise).
            const int N = CLOCK_WIDTH;
            for (int y = 0; y < N; y++) {
                for (int x = 0; x < N; x++) {
                    int srcX, srcY;
                    switch (faceOrientation) {
                        case 1:  srcX = y;         srcY = N - 1 - x; break; // 90 Grad im Uhrzeigersinn
                                                                            // 90 degrees clockwise
                        case 2:  srcX = N - 1 - x; srcY = N - 1 - y; break; // 180 Grad
                                                                            // 180 degrees
                        default: srcX = N - 1 - y; srcY = x;         break; // 270 Grad im Uhrzeigersinn
                                                                            // 270 degrees clockwise
                    }
                    rowBuffer[x] = clockFaceBrightBuffer[srcY * N + srcX];
                }
                backgroundSprite.pushImage(0, y, N, 1, rowBuffer);
            }
        }
    }


    // Buffer freigeben, wenn ein neues Zifferblatt gewählt wird
    // Free the buffer when a new clock face is selected

    void freeClockFaceBuffer() {
        // Zwischenbilder ungueltig machen: sie enthalten das alte Zifferblatt.
        // Invalidate the composite images: they contain the old clock face.
        clockAssetGeneration++;

        if (clockFaceBuffer) {
            free(clockFaceBuffer);
            clockFaceBuffer = nullptr;
            // DEBUG_PRINTLN("[clockFaceBuffer] free");
        }
        if (clockFaceBrightBuffer) {
            free(clockFaceBrightBuffer);
            clockFaceBrightBuffer = nullptr;
        }
    }


    // Loescht alle hochgeladenen Zifferblaetter (face_*.bmp) - der eingebaute
    // Standard bleibt erhalten, da er nicht als Datei existiert. Raeumt
    // verwaiste Presets auf und schaltet bei Bedarf auf den Standard zurueck.

    // Deletes all uploaded clock faces (face_*.bmp) - the built-in
    // default remains, since it doesn't exist as a file. Cleans up
    // orphaned presets and falls back to the default if needed.

    void resetFacesToDefault() {
        std::vector<String> toDelete;
        File root = LittleFS.open("/");
        File file = root.openNextFile();
        while (file) {
            String name = file.name();
            if (!file.isDirectory() && name.startsWith("face_") && name.endsWith(".bmp")) {
                toDelete.push_back(name);
            }
            file = root.openNextFile();
        }
        for (const String& name : toDelete) {
            String path = "/" + name;
            LittleFS.remove(path);
            removeOrphanedPresets(path, "");
        }

        preferences.putString(PK_BACKGROUND, "/face_default.bmp");
        selectedBackground = "/face_default.bmp";
        freeClockFaceBuffer();
        loadClockFace();
        loadHandSprites();
        updateClock();
    }


    // Loescht alle hochgeladenen Zeigersaetze (hand_set*.bmp) - der eingebaute
    // Standard bleibt erhalten. Raeumt verwaiste Presets auf und schaltet auf
    // den Standard-Zeigersatz zurueck.

    // Deletes all uploaded hand sets (hand_set*.bmp) - the built-in
    // default remains. Cleans up orphaned presets and falls back
    // to the default hand set.

    void resetHandsToDefault() {
        std::vector<String> toDelete;
        std::set<String> setIds;
        File root = LittleFS.open("/");
        File file = root.openNextFile();
        while (file) {
            String name = file.name();
            if (!file.isDirectory() && name.startsWith("hand_set") && name.endsWith(".bmp")) {
                toDelete.push_back(name);
                int start = 8; // Laenge von "hand_set"
                               // length of "hand_set"
                int end = name.indexOf('_', start);
                if (end > start) setIds.insert(name.substring(start, end));
            }
            file = root.openNextFile();
        }
        for (const String& name : toDelete) {
            LittleFS.remove("/" + name);
        }
        for (const String& setId : setIds) {
            removeOrphanedPresets("", setId);
        }

        preferences.putString(PK_HANDSET, "default");
        freeClockFaceBuffer();
        loadClockFace();
        loadHandSprites();
        updateClock();
    }


    // Schreibt eine Zeiger-Bitmapzeile ins Sprite, mittig zugeschnitten, falls
    // das Sprite (siehe updateHandWidths()) schmaler als das Bitmap ist - haelt
    // den Zeiger so zentriert auf dem Drehpunkt statt rechtsseitig verschoben.

    // Writes a hand bitmap row into its sprite, cropped in the CENTRE if the
    // sprite (see updateHandWidths()) is narrower than the bitmap - keeps the
    // hand centred on the pivot instead of shifted off it on the right.

    void pushHandRowCentered(LGFX_Sprite* sprite, int row, uint16_t* rowPixels, int srcWidth, const uint8_t* transparentColor) {
        int dstWidth = sprite->width();

        if (dstWidth > 0 && dstWidth < srcWidth) {
            int srcOffset = (srcWidth - dstWidth) / 2;
            memmove(rowPixels, rowPixels + srcOffset, (size_t)dstWidth * sizeof(uint16_t));
            srcWidth = dstWidth;
        }

        if (transparentColor) sprite->pushImage(0, row, srcWidth, 1, rowPixels, *transparentColor);
        else                  sprite->pushImage(0, row, srcWidth, 1, rowPixels);
    }


    void loadHandSprites() {
        // Zwischenbilder ungueltig machen: sie enthalten die alten Zeigerbilder
        // (anderer Zeigersatz oder andere Helligkeit).

        // Invalidate the composite images: they contain the old hand images
        // (different hand set or different brightness).
        clockAssetGeneration++;

        String setId = preferences.getString(PK_HANDSET, "");
        bool customSet = (setId != "" && setId != "default");

        // Ein Puffer fuer alle drei Zeiger: Datei (neues oder altes Format) oder
        // eingebauter Standard, die alte Hoehe jeweils oben transparent aufgefuellt.

        // One buffer for all three hands: file (new or old format) or built-in
        // default, the old height padded transparent at the top in either case.
        uint16_t* pix = (uint16_t*)preferPsramMalloc((size_t)HAND_WIDTH * HAND_HEIGHT * sizeof(uint16_t));
        if (!pix) {
            DEBUG_PRINTLN("[HANDS] Error: couldnt allocate hand buffer");
            return;
        }

        struct HandConfig {
            const char* label;
            LGFX_Sprite* sprite;
            const uint16_t* fallback;
        } hands[3] = {
            {"hour", &hourHandSprite, handHour},
            {"minute", &minuteHandSprite, handMinute},
            {"second", &secondHandSprite, handSecond}
        };

        for (auto& h : hands) {
            bool fromFile = customSet && loadHandPixels("/hand_set" + setId + "_" + h.label + ".bmp", pix);
            if (!fromFile) copyLegacyHand(h.fallback, pix);

            // Zeilenweise, damit der mittige Zuschnitt fuer schmalere Sprites greift
            // (siehe pushHandRowCentered()); jede Zeile wird komplett geschrieben.

            // Row by row, so the centre cropping for narrower sprites applies
            // (see pushHandRowCentered()); every row is written completely.
            for (int y = 0; y < HAND_HEIGHT; y++) {
                for (int x = 0; x < HAND_WIDTH; x++) {
                    uint16_t px = pix[y * HAND_WIDTH + x];
                    if (px == 0xFFFF) px = TRANSPARENT_COLOR;
                    rowBuffer[x] = setPixelBrightness(px);
                }
                pushHandRowCentered(h.sprite, y, rowBuffer, HAND_WIDTH, nullptr);
            }
        }
        free(pix);
    }


    // Hilfsfunktion: Winkel an die aktuelle Display-Rotation anpassen
    // Helper function: adjust angle to the current display rotation

    float shortestAngleDiff(float from, float to) {
        float diff = fmodf(to - from + 360.0f, 360.0f); // Modulo 360, um Werte im Bereich [0, 360) zu halten
                                                        // modulo 360 to keep values within [0, 360)
        if (diff > 180.0f) diff -= 360.0f;             // Kürzeste Richtung wählen
                                                       // choose the shortest direction
        return diff;
    }

    // Zeigerbewegung bei grossen Spruengen: bekommt die Uhr zum ersten Mal
    // eine Zeit (alle Zeiger stehen bis dahin auf 12), wird die Zeit
    // korrigiert oder springt die Rocrail-Modellzeit, laufen die Zeiger in
    // HAND_MOVE_MS auf dem kuerzesten Weg (auch rueckwaerts) ans Ziel, mit
    // sanftem Anfahren und Abbremsen. Normale Bewegungen (Tick, Minutenschritt,
    // schwingende Zeiger) liegen unter HAND_MOVE_THRESHOLD_DEG und bleiben
    // unveraendert.
    // Gerechnet wird mit dem Abstand zum Ziel (offset), der ueber die Zeit
    // auf 0 schrumpft (Smoothstep). Bewegt sich das Ziel waehrend der
    // Animation (weiterlaufender Sekundenzeiger oder die NTP-Korrektur kurz
    // nach dem Start, z.B. +7 s), bleibt der Zeiger zunaechst stehen und der
    // Unterschied wird in den Restweg eingerechnet - frueher sprang der
    // Zeiger dabei um den entsprechenden Anteil (sichtbarer Ruckler ~1 s
    // nach dem Start). Kommt ein grosser Sprung erst spaet in der Animation,
    // beginnt sie neu, statt den Rest in wenigen Bildern nachzuholen. Die
    // Richtung ergibt sich aus dem Vorzeichen des Abstands und kehrt sich
    // daher auch bei ~180 Grad nicht um.

    // Hand movement on large jumps: when the clock gets a time for the first
    // time (all hands stand at 12 until then), the time gets corrected or the
    // Rocrail model time jumps, the hands move to their target in HAND_MOVE_MS
    // along the shortest path (also backwards), easing in and out. Normal
    // movements (tick, minute step, sweeping hands) stay below
    // HAND_MOVE_THRESHOLD_DEG and are unchanged.
    // The computation works on the distance to the target (offset), which
    // shrinks to 0 over time (smoothstep). If the target moves during the
    // animation (a still-running second hand, or the NTP correction shortly
    // after boot, e.g. +7 s), the hand stays where it is at first and the
    // difference is folded into the remaining way - before, the hand jumped
    // by the corresponding share (a visible jerk ~1 s after boot). If a large
    // jump only comes late in the animation, it restarts instead of catching
    // up the rest in a few frames. The direction follows from the sign of the
    // offset and therefore does not reverse near 180 degrees either.

    const float HAND_MOVE_THRESHOLD_DEG = 10.0f;
    const unsigned long HAND_MOVE_MS = 3000;

    struct HandMove {
        bool active = false;
        float offset = 0.0f;      // angezeigt - Ziel, ungewrappt / shown minus target, unwrapped
        float lastTarget = 0.0f;
        float lastRemain = 1.0f;  // Restanteil (1 - Smoothstep) im letzten Bild / remaining share in the last frame
        unsigned long startMillis = 0;
    };

    float wrapAngle(float a) {
        a = fmodf(a, 360.0f);
        return (a < 0.0f) ? a + 360.0f : a;
    }

    float animateHand(HandMove& m, float shown, float target, unsigned long now) {
        if (!m.active) {
            float diff = shortestAngleDiff(shown, target);
            if (fabsf(diff) <= HAND_MOVE_THRESHOLD_DEG) return target;
            m.active = true;
            m.offset = -diff;
            m.lastTarget = target;
            m.lastRemain = 1.0f;
            m.startMillis = now;
            return shown;
        }

        // Zielbewegung seit dem letzten Bild auffangen: der Zeiger bleibt, wo er ist
        // Absorb the target's movement since the last frame: the hand stays where it is
        float jump = shortestAngleDiff(m.lastTarget, target);
        m.lastTarget = target;
        m.offset -= jump;
        if (fabsf(jump) > HAND_MOVE_THRESHOLD_DEG && m.lastRemain < 0.5f) {
            m.startMillis = now; // grosser Sprung spaet in der Animation: neu beginnen / large jump late in the animation: restart
            m.lastRemain = 1.0f;
        }

        float p = (float)(now - m.startMillis) / (float)HAND_MOVE_MS;
        if (p >= 1.0f) {
            m.active = false;
            return target;
        }
        float remain = 1.0f - p * p * (3.0f - 2.0f * p); // 1 - Smoothstep
        if (remain <= 0.0f) { // Rundung kurz vor p = 1 / rounding just before p = 1
            m.active = false;
            return target;
        }
        m.offset *= remain / m.lastRemain;
        m.lastRemain = remain;
        return wrapAngle(target + m.offset);
    }

    static float lastHourAngle = 0.0f;
    static float lastMinuteAngle = 0.0f;

    // Eigener Glaettungs-Zustand fuer Display 2 - relevant nur bei
    // unterschiedlicher Software-Rotation beider Displays. Bei Hardware-
    // Rotation haelt es dieselben Werte wie lastHourAngle/lastMinuteAngle.

    // Own smoothing state for Display 2 - only relevant when both displays
    // are rotated differently in software. With hardware rotation this ends
    // up holding the same values as lastHourAngle/lastMinuteAngle.
    static float lastHourAngle2 = 0.0f;
    static float lastMinuteAngle2 = 0.0f;

    // Analoger Glaettungs-Zustand fuer den Sekundenzeiger im Normalmodus
    // (kein "wartet auf 12") - siehe renderClockFrame() weiter unten: faengt
    // einen sichtbaren Ruecksprung ab, falls die zugrundeliegende Zeit selbst
    // rueckwaerts korrigiert wird (z.B. durch einen NTP-/RTC-/DCF77-Abgleich).
    // Die Bahnhofsuhr-Schrittlogik (stationTick) braucht das nicht, da sie
    // bereits eigenstaendig gegen Ruecksprünge abgesichert ist.

    // Analogous smoothing state for the second hand in normal mode (not
    // "waits at 12") - see renderClockFrame() further below: catches a
    // visible jump-back if the underlying time itself is corrected backward
    // (e.g. by an NTP/RTC/DCF77 resync). The station-clock step logic
    // (stationTick) doesn't need this, since it is already independently
    // guarded against jumping back.
    static float lastSecondAngle = 0.0f;
    static float lastSecondAngle2 = 0.0f;

    // Rendert EIN Frame (Zifferblatt+Zeiger+Nabe) fuers per Chip-Select
    // gewaehlte Display. lastHourAngleRef/lastMinuteAngleRef sind Referenzen
    // auf pro-Display-Variablen, damit jedes Display seine eigene Glaettung behaelt.
    // Baut das Zwischenbild fuer ein Display neu auf: gedrehtes Zifferblatt,
    // darauf Stunden- und Minutenzeiger kantengeglaettet (LovyanGFX
    // pushRotatedWithAA(), wie der Sekundenzeiger - ein Verfahren fuer alle).

    // Renders ONE frame (face+hands+hub) for the currently chip-select-selected
    // display. lastHourAngleRef/lastMinuteAngleRef are references to
    // per-display variables, so each display keeps its own smoothing.
    // Rebuilds the composite image for one display: rotated clock face with the
    // anti-aliased hour and minute hands on top (LovyanGFX pushRotatedWithAA(),
    // like the second hand - one method for all).

    bool buildHandComposite(HandComposite& comp, uint8_t rotation, float hourAngle, float minuteAngle) {
        if (comp.allocationFailed) return false;

        // Erst pruefen, ob das Zifferblatt lieferbar ist, DANN den Puffer
        // belegen - sonst wuerde bei Speichermangel genau der Speicher belegt,
        // den der fehlende Helligkeits-Cache eigentlich braeuchte.

        // First check whether the clock face can be delivered, THEN allocate
        // the buffer - otherwise, under memory pressure, this would grab
        // exactly the memory the missing brightness cache actually needs.
        if (prepareClockFaceCache() != 2) return false;

        if (!comp.sprite) {
            comp.sprite = new (std::nothrow) LGFX_Sprite(&tft);
            if (!comp.sprite || !createSprite16(*comp.sprite, CLOCK_WIDTH, CLOCK_HEIGHT)) {
                // Einmal melden und danach dauerhaft den bisherigen Weg nutzen,
                // statt bei jedem Tick erneut zu versuchen.

                // Report once and then permanently use the previous path instead
                // of retrying on every tick.
                DEBUG_PRINTLN("[Display] couldnt allocate hand composite sprite - falling back to per-tick rendering");
                delete comp.sprite;
                comp.sprite = nullptr;
                comp.allocationFailed = true;
                return false;
            }
        }

        if (!blitFaceIntoSprite(*comp.sprite, rotation)) return false;
        compositeBuildCount++; // renderClockFrame(): Teil-Aktualisierung dann nicht zulaessig
                               // renderClockFrame(): partial update not allowed then

        // Drehpunkt = Pivot des Zwischenbilds (Mitte, wie backgroundSprite)
        // pivot = the composite's pivot (centre, like backgroundSprite)
        hourHandSprite.pushRotatedWithAA(comp.sprite, hourAngle, TRANSPARENT_COLOR);
        minuteHandSprite.pushRotatedWithAA(comp.sprite, minuteAngle, TRANSPARENT_COLOR);

        // Nur als gueltig markieren, wenn die Zeiger wirklich drin sind - sonst
        // (Speichermangel bei den Zeiger-Sprites) lieber naechsten Tick erneut
        // versuchen, statt ein Bild ohne Zeiger dauerhaft festzuhalten.

        // Only mark it valid if the hands are really in it - otherwise (hand
        // sprite allocation failed) better retry next tick than keep an image
        // without hands permanently.
        if (hourHandSprite.width() <= 0 || minuteHandSprite.width() <= 0) {
            return true;
        }

        comp.valid = true;
        comp.hourAngle = hourAngle;
        comp.minuteAngle = minuteAngle;
        comp.rotation = rotation;
        comp.brightness = currentBrightness;
        comp.assetGeneration = clockAssetGeneration;
        return true;
    }


    // Haelt das Zwischenbild aktuell und kopiert es ins backgroundSprite. Neu
    // aufgebaut wird nur bei spuerbarer Aenderung (Bild/Rotation/Helligkeit/
    // Winkel > COMPOSITE_ANGLE_EPS, deutlich unter 1 Pixel am Zeigerende).

    // Keeps the composite image current and copies it into backgroundSprite.
    // Rebuilt only on a noticeable change (image/rotation/brightness/angle >
    // COMPOSITE_ANGLE_EPS, clearly under 1 pixel at the hand tip).

    bool drawCompositeInto(uint8_t displayNum, uint8_t rotation, float hourAngle, float minuteAngle) {
        const float COMPOSITE_ANGLE_EPS = 0.12f;

        HandComposite& comp = handComposite[(displayNum == 1) ? 0 : 1];

        bool needsRebuild = !comp.valid
            || comp.rotation != rotation
            || comp.assetGeneration != clockAssetGeneration
            // Nur ohne Backlight faerbt die Helligkeit Pixel ein (sonst No-Op) -
            // mit Backlight wuerde jeder Rampenschritt sonst einen wirkungslosen
            // Neuaufbau ausloesen.

            // Only without a backlight does brightness tint pixels (otherwise
            // a no-op) - with a backlight every ramp step would otherwise
            // trigger a pointless rebuild.
            || (!useBacklight && comp.brightness != currentBrightness)
            || fabsf(shortestAngleDiff(comp.hourAngle, hourAngle)) >= COMPOSITE_ANGLE_EPS
            || fabsf(shortestAngleDiff(comp.minuteAngle, minuteAngle)) >= COMPOSITE_ANGLE_EPS;

        if (needsRebuild) {
            if (!buildHandComposite(comp, rotation, hourAngle, minuteAngle)) return false;
        }
        else if (!comp.sprite) {
            return false;
        }

        comp.sprite->pushSprite(&backgroundSprite, 0, 0);
        return true;
    }


    // Erweitert das Rechteck (x0,y0)-(x1,y1) um die Flaeche, die ein per
    // pushRotated*() auf backgroundSprite gezeichnetes Zeiger-Sprite belegt -
    // gleiche Abbildung wie LovyanGFX (make_rotation_matrix(): Pixelmitte
    // +0,5, (u,v) -> (u*cos - v*sin, u*sin + v*cos)), plus 2 px Rand fuer
    // die geglaetteten Kanten.

    // Extends the rectangle (x0,y0)-(x1,y1) by the area a hand sprite drawn
    // onto backgroundSprite via pushRotated*() covers - same mapping as
    // LovyanGFX (make_rotation_matrix(): pixel centre +0.5,
    // (u,v) -> (u*cos - v*sin, u*sin + v*cos)), plus 2 px margin for the
    // anti-aliased edges.

    void addRotatedSpriteBounds(LGFX_Sprite& sprite, float angleDeg, int32_t& x0, int32_t& y0, int32_t& x1, int32_t& y1) {
        const float rad = fmodf(angleDeg, 360.0f) * (float)DEG_TO_RAD;
        const float c = cosf(rad), s = sinf(rad);
        const float dstX = backgroundSprite.getPivotX() + 0.5f;
        const float dstY = backgroundSprite.getPivotY() + 0.5f;
        const float srcX = sprite.getPivotX() + 0.5f;
        const float srcY = sprite.getPivotY() + 0.5f;
        const float w = (float)sprite.width(), h = (float)sprite.height();
        const float cornersX[4] = { 0.0f, w, 0.0f, w };
        const float cornersY[4] = { 0.0f, 0.0f, h, h };
        for (int i = 0; i < 4; i++) {
            float u = cornersX[i] - srcX, v = cornersY[i] - srcY;
            float x = dstX + u * c - v * s;
            float y = dstY + u * s + v * c;
            x0 = min(x0, (int32_t)floorf(x) - 2);
            y0 = min(y0, (int32_t)floorf(y) - 2);
            x1 = max(x1, (int32_t)ceilf(x) + 2);
            y1 = max(y1, (int32_t)ceilf(y) + 2);
        }
    }


    // Sendet nur das Rechteck (x,y,w,h) aus backgroundSprite ans Display -
    // zeilenweise. Setzt den SPI-Bus ohne DMA-Kanal voraus (lgfx_config.h):
    // mit DMA-Kanal leitet LovyanGFX Zeilen von 65..1023 Byte intern auf DMA
    // um, und DMA verwirft LovyanGFX 1.2.x auf dem ESP32-S2 still.

    // Sends only the rectangle (x,y,w,h) of backgroundSprite to the display -
    // row by row. Requires the SPI bus without a DMA channel (lgfx_config.h):
    // with a DMA channel LovyanGFX internally reroutes rows of 65..1023 bytes
    // to DMA, and LovyanGFX 1.2.x silently drops DMA on the ESP32-S2.

    void pushBackgroundRect(int32_t x, int32_t y, int32_t w, int32_t h) {
        const lgfx::swap565_t* buf = (const lgfx::swap565_t*)backgroundSprite.getBuffer();
        const int32_t stride = backgroundSprite.width();
        if (!buf || w <= 0 || h <= 0) return;
        tft.startWrite();
        tft.setAddrWindow(x, y, w, h);
        for (int32_t row = 0; row < h; row++) {
            tft.writePixels(buf + (y + row) * stride + x, w);
        }
        tft.endWrite();
    }


    // Zaehlt einen gesendeten Frame fuer die Statusseite; wertet alle 5 s aus.
    // Counts a sent frame for the status page; evaluates every 5 s.

    void recordRenderFrame(uint32_t durationMicros, bool partial) {
        RenderStats& s = renderStats;
        uint32_t now = millis();
        if (s.windowStartMillis == 0) s.windowStartMillis = now;
        s.frames++;
        if (partial) s.partialFrames++;
        s.sumMicros += durationMicros;
        if (durationMicros > s.maxMicros) s.maxMicros = durationMicros;

        uint32_t elapsed = now - s.windowStartMillis;
        if (elapsed >= 5000) {
            s.fps = s.frames * 1000.0f / elapsed;
            s.avgMs = s.sumMicros / 1000.0f / s.frames;
            s.maxMs = s.maxMicros / 1000.0f;
            s.partialPercent = 100.0f * s.partialFrames / s.frames;
            s.windowStartMillis = now;
            s.frames = s.partialFrames = 0;
            s.sumMicros = 0;
            s.maxMicros = 0;
        }
    }


    bool renderClockFrame(uint8_t displayNum, uint8_t rotation, float& lastHourAngleRef, float& lastMinuteAngleRef, float& lastSecondAngleRef, bool& firstRunRef) {

        int orientation = rotation;
        bool forceRender = firstRunRef;

        // Rocrail-Modus: Zeiger folgen der Modellzeit statt der echten Zeit -
        // nur solange ein <clock>-Signal angekommen ist, die Verbindung
        // steht und das letzte Signal nicht laenger als ROCRAIL_STALE_TIMEOUT_MS her ist, sonst faellt die Uhr auf die echte Zeit zurueck.

        // Rocrail mode: hands follow model time instead of real time - only
        // while a <clock> signal has arrived, the connection is up, and the
        // last signal isn't older than ROCRAIL_STALE_TIMEOUT_MS, otherwise the clock falls back to real time.
        bool rocrailTimeReady = rocrailEnabled && rocrailConnected && rocrailLastClockMillis != 0 &&
                                 (millis() - rocrailLastClockMillis) < ROCRAIL_STALE_TIMEOUT_MS;
        struct tm& t = rocrailTimeReady ? rocrailTimeinfo : timeinfo;

        // Bahnhofsuhr-"Wartet auf 12"-Verhalten (stationMode): der Sekunden-
        // zeiger eilt in einer komprimierten Zeit (~58,5s) einmal rum und
        // wartet dann bis zum Minutenwechsel oben auf der 12 - unabhaengig
        // davon, OB diese Bewegung schwingend oder tickend dargestellt wird
        // (siehe smoothSecond weiter unten, an renderClockFrame() entkoppelt).
        // Die Schrittanimation ist auf FAST_SECOND kalibriert - im Rocrail-
        // Modus wird die Schrittdauer weiter unten (stationStepMs) durch
        // rocrailDivider geteilt, damit der Umlauf genauso viel schneller
        // laeuft wie die Modellzeit selbst.

        // Station-clock "wait at 12" behaviour (stationMode): the second hand
        // races around once in a compressed time (~58.5s) and then waits at
        // the top until the minute changes - independent of WHETHER that
        // motion is rendered smoothly or in ticks (see smoothSecond further
        // below, decoupled in renderClockFrame()). The stepping animation is
        // calibrated to FAST_SECOND - in Rocrail mode the step duration
        // further below (stationStepMs) is divided by rocrailDivider, so the
        // lap runs exactly as much faster as the model time itself.
        bool waitAtTwelve = stationMode;

        // Schrittdauer fuer die Sweep-Animation: im Rocrail-Modus durch den
        // Divider geteilt (siehe Kommentar oben), sonst die reale FAST_SECOND.
        // float statt der #define-Konstante direkt, da sie sich pro Frame
        // aendern kann (Divider-Aenderungen kommen per <clock>-Update).

        // Step duration for the sweep animation: divided by the divider in
        // Rocrail mode (see comment above), otherwise the real FAST_SECOND.
        // A float instead of using the #define constant directly, since it
        // can change per frame (divider changes arrive via <clock> updates).

        float stationStepMs = (rocrailTimeReady && rocrailDivider > 1)
                               ? (FAST_SECOND / (float)rocrailDivider)
                               : FAST_SECOND;

        // Ab ROCRAIL_HIDE_DETAILS_DIVIDER (siehe config.h) Sekundenzeiger UND
        // Nabe ganz ausblenden - bei so hoher Beschleunigung waere ihre
        // Bewegung/Sichtbarkeit ohnehin kaum noch sinnvoll.

        // From ROCRAIL_HIDE_DETAILS_DIVIDER (see config.h) onwards, hide the
        // second hand AND the hub entirely - at such high acceleration
        // their movement/visibility wouldn't be meaningfully useful anyway.
        bool hideDetailsForRocrail = rocrailTimeReady && rocrailDivider >= ROCRAIL_HIDE_DETAILS_DIVIDER;

        // Zusaetzlich NUR den Sekundenzeiger (nicht die Nabe) ausblenden, wenn
        // Rocrail ueberhaupt beschleunigt (divider > 1) UND tickend statt
        // schwingend dargestellt wird: ein springender Zeiger wirkt bei jeder
        // Beschleunigung unruhig/ruckelig (Sprungrate skaliert mit dem Divider,
        // Anzeige-Framerate aber nicht), waehrend die schwingende Darstellung
        // bei jeder Geschwindigkeit saubersieht - dafuer bleibt sie ja da.

        // Additionally hide ONLY the second hand (not the hub) when Rocrail is
        // accelerated at all (divider > 1) AND rendered in ticking instead of
        // smooth style: a jumping hand looks jittery/unsteady at any
        // acceleration (the jump rate scales with the divider, but the
        // display's frame rate doesn't), while the smooth style looks clean
        // at any speed - that's exactly what it stays available for.
        bool hideSecondHandTicking = rocrailTimeReady && rocrailDivider > 1 && !smoothSecond;
        bool hideSecondHand = hideDetailsForRocrail || hideSecondHandTicking;

        float secAngle = t.tm_sec * 6.0f;
        float minAngle = t.tm_min * 6.0f;
        float hourAngle = (t.tm_hour % 12) * 30.0f + (t.tm_min / 2.0f) + (t.tm_sec / 120.0f);

        static uint8_t stationTick = 0;
        static uint32_t stationLastMillis = 0;
        static bool stationWaiting = false;

        // Minute, in der die Wartephase (Zeiger auf 12) begonnen hat - wird
        // gebraucht, um das Aufwachen robust an einem Minutenwechsel statt an
        // exakt "Sekunde 0" festzumachen (siehe Kommentar weiter unten).

        // Minute in which the wait phase (hand parked at 12) started - needed
        // to make waking up robust against a minute change instead of pinning
        // it to exactly "second 0" (see comment further below).
        static int stationWaitStartMinute = -1;

        unsigned long currentMillis = millis();

        if (firstRunRef) {

            // Sekundenzeiger dorthin setzen, wo er in der laufenden Minute
            // stehen muesste: da ein Umlauf FAST_SECOND * 60 ms dauert (kuerzer
            // als eine echte Minute), folgt die Position aus tm_sec / FAST_SECOND.

            // Put the second hand where it should be within the current minute:
            // since one sweep takes FAST_SECOND * 60 ms (less than a real
            // minute), the position follows from tm_sec / FAST_SECOND.
            float sweepPosition = ((float)t.tm_sec * 1000.0f) / FAST_SECOND;

            if (sweepPosition >= 60.0f) {
                // Der Zeiger waere schon oben angekommen und wuerde warten.
                // The hand would already have arrived at the top and be waiting.
                stationTick = 60;
                stationWaiting = true;
                stationWaitStartMinute = t.tm_min;
                stationLastMillis = millis();
            }
            else {
                stationTick = (uint8_t)sweepPosition;
                stationWaiting = false;
                // Anfang des angebrochenen Schritts so zurueckdatieren, dass
                // auch der Bruchteil stimmt.

                // Back-date the start of the current step so the fractional
                // part is correct too.
                stationLastMillis = millis() - (unsigned long)((sweepPosition - (float)stationTick) * stationStepMs);
            }

            firstRunRef = false;

            lastHourAngleRef = rotatedAngle(hourAngle, orientation);
            lastMinuteAngleRef = rotatedAngle(minAngle, orientation);
            lastSecondAngleRef = rotatedAngle(secAngle, orientation); // Basiswert fuer die Ruecksprung-Abfederung unten (Normalmodus)
                                                                     // baseline for the jump-back easing below (normal mode)

            // Hier bewusst NICHT sofort zeichnen: das Bild entsteht weiter unten
            // im selben Durchlauf (forceRender). Frueher wurden die Zeiger hier
            // direkt an die Zielposition gemalt - ueber das alte Bild mit den
            // Zeigern auf 12 - und dieses Bild gesendet; beim Zeitempfang
            // blitzten sie so kurz doppelt auf, bevor animateHand() sie von 12
            // aus loslaufen liess.
            // Deliberately do NOT draw right here: the frame is produced further
            // below in the same pass (forceRender). The hands used to be painted
            // straight at their target here - over the old frame with the hands
            // at 12 - and that frame sent; on receiving the time they briefly
            // flashed doubled before animateHand() let them start from 12.
        }

        // Bahnhofsuhr-"Wartet auf 12"-Modus: Sekundenzeiger schreitet in 60
        // Schritten je stationStepMs (schwingend per easeInOutSine() oder
        // tickend, je nach smoothSecond - s.u.), erreicht nach ~58,5s
        // (bzw. ~58,5s/divider im Rocrail-Modus) die 12 und wartet dort,
        // bis die (Modell-)Minute wechselt (Pause ~1,5s bzw. ~1,5s/divider).

        // Station-clock "waits at 12" mode: second hand steps in 60 steps of
        // stationStepMs each (smooth via easeInOutSine(), or ticking,
        // depending on smoothSecond - see below), reaches the top after
        // ~58.5s (or ~58.5s/divider in Rocrail mode) and waits there until
        // the (model) minute changes (pause ~1.5s, or ~1.5s/divider).

        if (waitAtTwelve) {
            // Bei divider 1 (keine Beschleunigung) verwendet die Uhr die
            // gewohnte schrittweise Bahnhofsuhr-Logik unten (Schritt +
            // easeInOutSine() bzw. reines Ticken, je nach smoothSecond) - die
            // reale Kalibrierung passt dort exakt, da die Modellzeit 1:1 mit
            // der realen Zeit fortschreitet. Erst ab divider > 1 wird die
            // Position direkt aus rocrailSecFrac abgeleitet (siehe Kommentar
            // im if-Zweig) - smoothSecond entscheidet auch dort zwischen
            // stufenloser und auf ganze Schritte gerundeter Darstellung.

            // At divider 1 (no acceleration) the clock uses the usual
            // stepwise station-clock logic below (step + easeInOutSine(), or
            // plain ticking, depending on smoothSecond) - the real calibration
            // matches exactly there, since the model time advances 1:1 with
            // real time. Only from divider > 1 onwards is the position
            // derived directly from rocrailSecFrac (see comment in the if
            // branch) - smoothSecond decides there too between a continuous
            // and a whole-step-rounded rendering.
            // At divider 1 (no acceleration), the clock runs with the usual
            // swinging station-clock second hand (else branch below) - the
            // real calibration matches exactly there, since the model time
            // advances 1:1 with real time. Only from divider > 1 onwards is
            // it rendered smoothly instead of ticking (see comment in the
            // if branch).

            if (rocrailTimeReady && rocrailDivider > 1) {
                // Rocrail: glatt statt tickend - kein easeInOutSine() pro
                // Schritt noetig. rocrailSecFrac (advanceRocrailTime()) traegt
                // die Nachkommastellen, dieselbe Positions-Formel wie oben deckelt bei 60.

                // Rocrail: smooth instead of ticking - no per-step
                // easeInOutSine() needed. rocrailSecFrac (advanceRocrailTime())
                // carries the fractional part, the same position formula as above caps at 60.
                float smoothPos = (rocrailSecFrac * 1000.0f) / FAST_SECOND;
                if (smoothPos > 60.0f) smoothPos = 60.0f;

                // smoothSecond entkoppelt "wartet auf 12" von der Darstellung:
                // bei tickend wird auf den ganzzahligen Schritt abgerundet statt
                // die (durch rocrailSecFrac ohnehin schon gleitende) Position
                // stufenlos zu uebernehmen.

                // smoothSecond decouples "waits at 12" from the rendering
                // style: with ticking, round down to the whole step instead of
                // taking over the (already continuously gliding via
                // rocrailSecFrac) position as-is.
                if (!smoothSecond) smoothPos = floorf(smoothPos);

                secAngle = rotatedAngle(smoothPos * 6.0f, orientation);
                minAngle = rotatedAngle(t.tm_min * 6.0f, orientation);
            }
            else {

            // Ganzzahlig und vorzeichenbehaftet rechnen: uint32 += float rundete ab ~4,7 h
            // Laufzeit (24-Bit-Mantisse), der Zeitstempel lag dann 1 ms in der Zukunft und
            // der Sekundenzeiger sprang fuer ein Bild einen Schritt vor und wieder zurueck.

            // Compute as integer and signed: uint32 += float rounded from ~4.7 h uptime
            // (24-bit mantissa), the timestamp then lay 1 ms in the future and the second
            // hand jumped one step ahead for one frame and back again.
            const uint32_t stationStepWholeMs = (uint32_t)(stationStepMs + 0.5f);
            if (!stationWaiting && (int32_t)(currentMillis - stationLastMillis) >= (int32_t)stationStepWholeMs) {
                stationTick++;
                stationLastMillis += stationStepWholeMs;

                if (stationTick >= 60) {
                    stationTick = 60;
                    stationWaiting = true;
                    stationWaitStartMinute = t.tm_min;
                }
            }
            else if (stationWaiting) {

                // Aufwachen ueber Minutenwechsel statt exakt "Sekunde 0": ein
                // blockierender loadClockFace()-Aufruf kann Sekunde 0 verpassen,
                // ein Minutenwechsel bleibt aber auch danach erkennbar.

                // Wake up on a minute change instead of exactly "second 0": a
                // blocking loadClockFace() call can cause updateClock() to miss
                // exactly second 0, but a minute change stays detectable after.

                // Sollposition, die die Uhrzeit gerade verlangt.
                // Position the current time is asking for.
                float expectedPosition = ((float)t.tm_sec * 1000.0f) / FAST_SECOND;

                // Sicherheitsnetz gegen verpassten Minutenwechsel: wacht auch
                // auf, wenn die Uhrzeit laengst wieder mitten im Umlauf steht.
                // Schwelle 55 liegt sicher unter dem Pausenfenster (~59).

                // Safety net for a missed minute change: also wakes up if the
                // time says the sweep should long be running again. Threshold
                // 55 sits safely below the pause window (~59).
                const float RESYNC_BELOW = 55.0f;

                if (t.tm_min != stationWaitStartMinute || expectedPosition < RESYNC_BELOW) {

                    // Auf die verlangte Position SPRINGEN statt stur bei 0
                    // anzufangen - bei verpasstem Wechsel steht der Zeiger so
                    // sofort richtig statt eine Minute nachzulaufen.

                    // JUMP to the requested position instead of always starting
                    // at 0 - on a missed change the hand is immediately correct
                    // instead of trailing a minute behind.
                    if (expectedPosition >= 60.0f) {
                        // Nur moeglich, wenn der Minutenwechsel mit noch alter
                        // Sekundenanzeige gemeldet wurde - dann von vorn beginnen.

                        // Only possible if the minute change was reported while the
                        // seconds still read the old value - then start from the top.
                        expectedPosition = 0.0f;
                    }

                    stationTick = (uint8_t)expectedPosition;
                    stationWaiting = false;
                    stationLastMillis = currentMillis - (unsigned long)((expectedPosition - (float)stationTick) * stationStepMs);

                    // Sekundenzeiger korrekt synchronisieren
                    // Synchronize the second hand correctly
                    secAngle = rotatedAngle(expectedPosition * 6.0f, orientation);
                }
            }

            float subTick = (float)(int32_t)(currentMillis - stationLastMillis) / (float)stationStepWholeMs;
            if (subTick < 0.0f) subTick = 0.0f;

            // Auf 1.0 begrenzen statt auf 0.0 zurueckzusetzen: bei einem zu
            // spaeten Frame ist der Zeiger mindestens am Ende des Schritts,
            // ein Reset auf 0 wuerde ihn sichtbar zurueckspringen lassen.

            // Clamp to 1.0 instead of resetting to 0.0: on a late frame the
            // hand is at least at the end of the step, resetting to 0 would
            // make it visibly jump back.
            if (subTick > 1.0f) subTick = 1.0f;
            if (stationWaiting) subTick = 0.0f;

            // Bewusst KEINE gleichfoermige Bewegung im schwingenden Stil:
            // easeInOutSine() beschleunigt und bremst pro Schritt, wie bei
            // aelteren Bahnhofsuhren - nicht durch lineare Interpolation ersetzen.
            // smoothSecond entkoppelt "wartet auf 12" (waitAtTwelve, s.o.) von
            // der Darstellung: schwingend interpoliert per easeInOutSine()
            // innerhalb des Schritts, tickend haelt exakt auf der Ganzzahl-
            // Position bis zum naechsten Schritt (kein Zwischenwert).

            // Deliberately NOT uniform movement in the smooth style:
            // easeInOutSine() accelerates and brakes each step, like older
            // station clocks - do not replace with linear interpolation.
            // smoothSecond decouples "waits at 12" (waitAtTwelve, see above)
            // from the rendering style: smooth interpolates via
            // easeInOutSine() within the step, ticking holds exactly at the
            // whole-number position until the next step (no intermediate value).

            float smoothSec;
            if (smoothSecond) {
                smoothSec = (stationTick >= 60) ? 60.0f : (float)stationTick + easeInOutSine(subTick);
            }
            else {
                smoothSec = (stationTick >= 60) ? 60.0f : (float)stationTick;
            }
            secAngle = rotatedAngle(smoothSec * 6.0f, orientation);

            minAngle = rotatedAngle(t.tm_min * 6.0f, orientation);
            } // Ende "else" (nicht Rocrail ODER Rocrail mit divider 1) - siehe Bedingung oben
              // end "else" (not Rocrail OR Rocrail with divider 1) - see condition above
        }

        // Normaler Modus (kein "wartet auf 12"): Sekundenzeiger laeuft mit der
        // echten (bzw. bei Rocrail: modellzeit-)Sekunde mit, ohne Pause oben.
        // Auch hier entkoppelt smoothSecond Stil von Timing: schwingend
        // interpoliert stufenlos innerhalb der laufenden Sekunde (per Millis.
        // bzw. bei Rocrail per rocrailSecFrac, das den Divider bereits
        // beruecksichtigt), tickend springt einmal pro Sekunde wie bisher.
        // Minutenzeiger kann optional weiterhin sanft laufen (unveraendert).

        // Normal mode (no "waits at 12"): the second hand keeps pace with the
        // real (or, in Rocrail mode, model-time) second, without pausing at
        // the top. smoothSecond decouples style from timing here too: smooth
        // interpolates continuously within the running second (via millis(),
        // or via rocrailSecFrac in Rocrail mode, which already accounts for
        // the divider), ticking jumps once per second as before. The minute
        // hand can still optionally run smoothly (unchanged).
        if (!waitAtTwelve) {

            // Praeziser Sekundenbruchteil fuer sanfte Zeigerbewegung (Minute
            // UND Sekunde): statt millis()%1000 (frueher - phasenversetzt zum
            // echten Sekundenwechsel, da millis() seit dem Boot zaehlt und in
            // keiner festen Beziehung zu t.tm_sec steht) wird der Zeitpunkt
            // des zuletzt beobachteten Sekundenwechsels selbst gemerkt und die
            // seitdem vergangene Zeit gebildet. Dadurch endet die Bewegung
            // IMMER genau dann, wenn t.tm_sec tatsaechlich weiterspringt,
            // statt mit einem zufaelligen, boot-abhaengigen Versatz zu enden
            // oder anzufangen (kleiner Ruckler moeglich, je nach Bootzeitpunkt).
            // Bei aktiver Rocrail-Modellzeit dagegen bereits exakt aus
            // rocrailSecFrac abgeleitet (siehe dort) - kein Tracking noetig.

            // Precise sub-second fraction for smooth hand motion (minute AND
            // second): instead of millis()%1000 (previously - out of phase
            // with the real second change, since millis() counts since boot
            // and has no fixed relationship to t.tm_sec), the moment of the
            // last observed second change is remembered and the time elapsed
            // since then is used instead. This way the motion ALWAYS finishes
            // exactly when t.tm_sec actually advances, instead of ending or
            // starting with a random, boot-dependent offset (a small hitch
            // was possible, depending on boot time). With active Rocrail
            // model time, on the other hand, already derived exactly from
            // rocrailSecFrac (see there) - no tracking needed.
            static long lastWholeSecondValue = -1;
            static unsigned long secondBoundaryMillis = 0;
            float realSubSecond = 0.0f;

            // Nur pflegen, wenn tatsaechlich gebraucht (nicht waehrend
            // aktiver Rocrail-Modellzeit, die ihren Bruchteil bereits exakt
            // aus rocrailSecFrac bezieht) - sonst wuerde hier fuer nichts
            // mitgezaehlt, waehrend Rocrail aktiv ist.

            // Only maintained when actually needed (not while Rocrail model
            // time is active, which already gets its fraction exactly from
            // rocrailSecFrac) - otherwise this would keep counting for
            // nothing while Rocrail is active.
            if (!rocrailTimeReady) {
                long currentWholeSecond = (long)t.tm_hour * 3600 + (long)t.tm_min * 60 + t.tm_sec;
                if (currentWholeSecond != lastWholeSecondValue) {
                    lastWholeSecondValue = currentWholeSecond;
                    secondBoundaryMillis = currentMillis;
                }
                realSubSecond = (currentMillis - secondBoundaryMillis) / 1000.0f;

                // Deckeln statt ueberlaufen zu lassen: bei einem verzoegerten
                // Frame (z.B. durch einen blockierenden Aufruf anderswo) waere
                // sonst kurzzeitig eine Bewegung ueber die naechste Sekunde
                // hinaus sichtbar, bevor t.tm_sec nachzieht.

                // Clamp instead of letting it overshoot: on a delayed frame
                // (e.g. due to a blocking call elsewhere) motion past the
                // next second would otherwise be briefly visible before
                // t.tm_sec catches up.
                if (realSubSecond < 0.0f) realSubSecond = 0.0f;
                else if (realSubSecond > 0.999f) realSubSecond = 0.999f;
            }

            // rocrailSecFrac traegt Ganzzahl- und Bruchteil bereits zusammen
            // (siehe advanceRocrailTime()) - t.tm_sec (derselbe Ganzzahlteil)
            // abziehen liefert exakt den divider-skalierten Bruchteil, analog
            // zu realSubSecond oben.

            // rocrailSecFrac already carries the whole and fractional part
            // together (see advanceRocrailTime()) - subtracting t.tm_sec (the
            // same whole part) yields exactly the divider-scaled fraction,
            // analogous to realSubSecond above.
            float secondFraction = rocrailTimeReady ? (rocrailSecFrac - (float)t.tm_sec) : realSubSecond;

            float targetSecAngle;
            if (smoothSecond) {
                float smoothSecondValue = (float)t.tm_sec + secondFraction;
                targetSecAngle = rotatedAngle(smoothSecondValue * 6.0f, orientation);
            }
            else {
                targetSecAngle = rotatedAngle(secAngle, orientation);
            }

            // Abfedern bei sichtbarem Sprung: rueckwaerts (Zeitkorrektur)
            // oder ungewoehnlich weit vorwaerts (verzoegerter Frame, z.B.
            // durch eine blockierende Web-Anfrage). Normale Ticks bleiben sofort.
            // Diagnose-Logging (DEBUG_PRINTLN, wirkungslos wenn loggingEnabled
            // aus): loggt je Episode Start (Betrag, Richtung) und Ende
            // (Dauer), um eine Ursache im Log zuzuordnen statt zu raten.

            // Ease away a visible jump: backward (time correction) or
            // unusually far forward (a delayed frame, e.g. a blocking web
            // request). Normal ticks still apply immediately.
            // Diagnostic logging (DEBUG_PRINTLN, a no-op while loggingEnabled
            // is off): logs each episode's start (magnitude, direction) and
            // end (duration), to match a cause in the log instead of guessing.

            static bool secondEasingActive[2] = { false, false };
            static unsigned long secondEasingStartMillis[2] = { 0, 0 };
            uint8_t easingIdx = displayNum - 1;

            float secAngleDiff = shortestAngleDiff(lastSecondAngleRef, targetSecAngle);
            bool jumpedBack = (secAngleDiff < 0.0f);
            bool skippedForward = (secAngleDiff > SECOND_HAND_MAX_NORMAL_FORWARD_STEP_DEG);
            if (jumpedBack || skippedForward) {
                if (!secondEasingActive[easingIdx]) {
                    secondEasingActive[easingIdx] = true;
                    secondEasingStartMillis[easingIdx] = currentMillis;
                    DEBUG_PRINTLN("[Clock] Display " + String(displayNum) + ": second hand " +
                                  (jumpedBack ? "jumped back by " : "skipped forward by ") +
                                  String(fabsf(secAngleDiff) / 6.0f, 2) + "s, easing in (time now " +
                                  String(t.tm_hour) + ":" + String(t.tm_min) + ":" + String(t.tm_sec) + ")");
                }
                lastSecondAngleRef += secAngleDiff * 0.2f;
                if (lastSecondAngleRef < 0.0f) lastSecondAngleRef += 360.0f;
                else if (lastSecondAngleRef >= 360.0f) lastSecondAngleRef -= 360.0f;
            }
            else {
                if (secondEasingActive[easingIdx]) {
                    secondEasingActive[easingIdx] = false;
                    DEBUG_PRINTLN("[Clock] Display " + String(displayNum) + ": second hand jump settled after " +
                                  String(currentMillis - secondEasingStartMillis[easingIdx]) + "ms");
                }
                lastSecondAngleRef = targetSecAngle;
            }
            secAngle = lastSecondAngleRef;

            if (smoothMinute) {
                float smoothMinuteValue = t.tm_min + (t.tm_sec / 60.0f) + (secondFraction / 60.0f);

                float rawMinAngle = smoothMinuteValue * 6.0f;
                minAngle = rotatedAngle(rawMinAngle, orientation);
                lastMinuteAngleRef = minAngle; // Direkt setzen, da wir den exakten Winkel berechnen
                                               // set directly since we compute the exact angle

            }
            else {
                // Normale Minutenanzeige mit sanfter Korrektur bei Wechsel
                // Normal minute display with smooth correction on change
                float rawMinAngle = t.tm_min * 6.0f;
                float targetMinAngle = rotatedAngle(rawMinAngle, orientation);
                float angleDiff = shortestAngleDiff(lastMinuteAngleRef, targetMinAngle);

                if (fabs(angleDiff) > 0.1f) {
                    lastMinuteAngleRef += angleDiff * 0.1f;
                    if (lastMinuteAngleRef < 0.0f) lastMinuteAngleRef += 360.0f;
                    if (lastMinuteAngleRef >= 360.0f) lastMinuteAngleRef -= 360.0f;
                }
                else {
                    lastMinuteAngleRef = targetMinAngle;
                }
            }

            minAngle = lastMinuteAngleRef;
        }


        float targetHourAngle = rotatedAngle(hourAngle, orientation);
        float hourAngleDiff = shortestAngleDiff(lastHourAngleRef, targetHourAngle);

        if (fabs(hourAngleDiff) > 0.05f) {
            lastHourAngleRef += hourAngleDiff * 0.1f;  // Glättungsfaktor
                                                       // smoothing factor

            // In [0,360) zurueckholen wie bei lastMinuteAngleRef - sonst waechst
            // der Wert bei sehr langer Laufzeit unbegrenzt und kostet Float-Praezision.

            // Fold back into [0,360) like lastMinuteAngleRef - otherwise the
            // value grows unbounded over very long runtime, costing float precision.
            if (lastHourAngleRef < 0.0f) lastHourAngleRef += 360.0f;
            if (lastHourAngleRef >= 360.0f) lastHourAngleRef -= 360.0f;
        }
        else {
            lastHourAngleRef = targetHourAngle;
        }
        hourAngle = lastHourAngleRef;

        // Grosse Spruenge langsam und auf dem kuerzesten Weg ausfuehren (siehe
        // animateHand()). Bei geaenderter Rotation sofort uebernehmen - dann
        // dreht sich ohnehin das ganze Zifferblatt.
        // Perform large jumps slowly and along the shortest path (see
        // animateHand()). On a changed rotation adopt immediately - the whole
        // clock face turns then anyway.
        {
            static HandMove moves[2][3];
            static float shown[2][3];
            static bool shownValid[2] = { false, false };
            static uint8_t shownRotation[2] = { 0, 0 };
            uint8_t d = displayNum - 1;
            float* angles[3] = { &hourAngle, &minAngle, &secAngle };
            unsigned long now = millis();
            if (!shownValid[d] || shownRotation[d] != rotation) {
                for (int h = 0; h < 3; h++) {
                    shown[d][h] = *angles[h];
                    moves[d][h].active = false;
                }
                shownValid[d] = true;
                shownRotation[d] = rotation;
            }
            for (int h = 0; h < 3; h++) {
                shown[d][h] = animateHand(moves[d][h], shown[d][h], *angles[h], now);
                *angles[h] = shown[d][h];
            }
        }

        // Nichts geaendert (Winkel, Sichtbarkeit, Nabe, Helligkeit, Rotation,
        // Stil, Grafiken) und nichts drueber gezeichnet: Zeichnen + SPI-Push
        // sparen - die Winkel-Zustaende oben laufen trotzdem jeden Tick weiter.

        // Nothing changed (angles, visibility, hub, brightness, rotation,
        // style, graphics) and nothing drawn over it: skip drawing + SPI push
        // - the angle state above still advances every tick.
        bool drawSecond = showSecondHand && !hideSecondHand;
        bool drawHub = hubSize > 0 && !hideDetailsForRocrail;
        ClockFrameKey& lastFrame = lastClockFrame[displayNum - 1];
        if (!forceRender && !clockFrameDirty[displayNum - 1] && lastFrame.valid
            && lastFrame.hourAngle == hourAngle
            && lastFrame.minuteAngle == minAngle
            && (!drawSecond || lastFrame.secondAngle == secAngle)
            && lastFrame.drawSecond == drawSecond
            && lastFrame.drawHub == drawHub
            && lastFrame.hubSize == hubSize
            && lastFrame.hubColor == hubColor
            && lastFrame.brightness == currentBrightness
            && lastFrame.rotation == rotation
            && lastFrame.smoothSecond == smoothSecond
            && lastFrame.assetGeneration == clockAssetGeneration) {
            return false;
        }


        // Zifferblatt + Stunden-/Minutenzeiger kommen aus dem zwischen-
        // gespeicherten Bild (drawCompositeInto()), neu gebaut nur bei
        // Bewegung - pro Tick bleibt nur das Kopieren. Fallback bei Speichermangel: alter Zeichenweg.

        // Clock face + hour/minute hands come from the cached composite image
        // (drawCompositeInto()), rebuilt only on movement - per tick only the
        // copy remains. Falls back to the old drawing path on low memory.

        // Teil-Aktualisierung: gegenueber dem letzten Frame hat sich NUR der
        // Sekundenzeiger bewegt (alle anderen Werte gleich, siehe Abbruch-
        // pruefung oben) - dann Kopieren, Zeichnen und SPI-Senden auf das
        // Rechteck um alten + neuen Sekundenzeiger und Nabe beschraenken.
        // Beim schwingenden Zeiger spart das den Grossteil der 115 KB je Frame.

        // Partial update: compared to the last frame ONLY the second hand
        // moved (all other values equal, see the early-out check above) - then
        // limit copying, drawing and the SPI send to the rectangle around old
        // + new second hand and hub. With the smooth hand this saves most of
        // the 115 KB per frame.
        const uint32_t renderStartMicros = micros(); // fuer recordRenderFrame() / for recordRenderFrame()

        // Stunden-/Minutenwinkel bewusst NICHT verglichen: beim schwingenden
        // Minutenzeiger aendert sich der Winkel jedes Frame minimal, das Bild
        // aber nur, wenn das Zwischenbild neu aufgebaut wird - das faengt die
        // compositeBuildCount-Pruefung unten ab.
        // Hour/minute angles deliberately NOT compared: with the smooth minute
        // hand the angle changes minimally every frame, but the image only
        // changes when the composite is rebuilt - the compositeBuildCount
        // check below catches that.
        bool partial = partialUpdateEnabled
            && !forceRender && !clockFrameDirty[displayNum - 1] && lastFrame.valid
            && drawSecond && lastFrame.drawSecond
            && lastFrame.drawHub == drawHub
            && lastFrame.hubSize == hubSize
            && lastFrame.hubColor == hubColor
            && lastFrame.brightness == currentBrightness
            && lastFrame.rotation == rotation
            && lastFrame.smoothSecond == smoothSecond
            && lastFrame.assetGeneration == clockAssetGeneration;

        int32_t px0 = INT32_MAX, py0 = INT32_MAX, px1 = INT32_MIN, py1 = INT32_MIN;
        if (partial) {
            addRotatedSpriteBounds(secondHandSprite, lastFrame.secondAngle, px0, py0, px1, py1);
            addRotatedSpriteBounds(secondHandSprite, secAngle, px0, py0, px1, py1);
            if (drawHub) {
                px0 = min(px0, (int32_t)(CLOCK_WIDTH / 2 - hubSize - 2));
                py0 = min(py0, (int32_t)(CLOCK_HEIGHT / 2 - hubSize - 2));
                px1 = max(px1, (int32_t)(CLOCK_WIDTH / 2 + hubSize + 2));
                py1 = max(py1, (int32_t)(CLOCK_HEIGHT / 2 + hubSize + 2));
            }
            px0 = max(px0, (int32_t)0);
            py0 = max(py0, (int32_t)0);
            px1 = min(px1, (int32_t)(CLOCK_WIDTH - 1));
            py1 = min(py1, (int32_t)(CLOCK_HEIGHT - 1));
            partial = (px1 >= px0 && py1 >= py0);
        }
        if (partial) backgroundSprite.setClipRect(px0, py0, px1 - px0 + 1, py1 - py0 + 1);

        uint32_t buildsBefore = compositeBuildCount;
        bool compositeOk = drawCompositeInto(displayNum, rotation, hourAngle, minAngle);

        // Wurde das Zwischenbild neu aufgebaut (Stunden-/Minutenzeiger ueber
        // die Schwelle) oder fehlt es (Speichermangel - dann werden die Zeiger
        // unten direkt gezeichnet), hat sich das Bild auch ausserhalb des
        // Rechtecks geaendert - dann dieses Frame voll zeichnen und senden.
        // If the composite was rebuilt (hour/minute hand crossed the
        // threshold) or is missing (low memory - then the hands are drawn
        // directly below), the image also changed outside the rectangle - then
        // draw and send this frame in full.
        if (partial && (compositeBuildCount != buildsBefore || !compositeOk)) {
            partial = false;
            backgroundSprite.clearClipRect();
            if (compositeOk) compositeOk = drawCompositeInto(displayNum, rotation, hourAngle, minAngle);
        }
        if (!compositeOk) {
            loadClockFace(rotation);
            hourHandSprite.pushRotatedWithAA(&backgroundSprite, hourAngle, TRANSPARENT_COLOR);
            minuteHandSprite.pushRotatedWithAA(&backgroundSprite, minAngle, TRANSPARENT_COLOR);
        }

        if (drawSecond) {
            // Tickend wie schwingend: LovyanGFX pushRotatedWithAA() - dasselbe
            // Verfahren wie bei Stunden-/Minutenzeiger (buildHandComposite()),
            // rechnet in Festkomma und ist damit schnell genug fuer jeden Frame.

            // Ticking and smooth alike: LovyanGFX pushRotatedWithAA() - the same
            // method as for the hour/minute hands (buildHandComposite()),
            // computes in fixed point and is therefore fast enough for every frame.
            secondHandSprite.pushRotatedWithAA(&backgroundSprite, secAngle, TRANSPARENT_COLOR);
        }


        // Nabe (hub) - ab ROCRAIL_HIDE_DETAILS_DIVIDER ebenfalls ausgeblendet
        // (siehe hideDetailsForRocrail oben)

        // hub - also hidden from ROCRAIL_HIDE_DETAILS_DIVIDER onwards (see
        // hideDetailsForRocrail above)
        if (drawHub) {
            // Kantengeglaettet wie die Zeiger (gleiche Flaeche wie fillCircle())
            // anti-aliased like the hands (same area as fillCircle())
            backgroundSprite.fillSmoothCircle(CLOCK_WIDTH / 2, CLOCK_HEIGHT / 2, hubSize, setPixelBrightness(hubColor));
        }

        if (partial) {
            // Nur das Rechteck senden (siehe pushBackgroundRect())
            // Send only the rectangle (see pushBackgroundRect())
            backgroundSprite.clearClipRect();
            pushBackgroundRect(px0, py0, px1 - px0 + 1, py1 - py0 + 1);
            lastRenderRect[0] = px0; lastRenderRect[1] = py0;
            lastRenderRect[2] = px1 - px0 + 1; lastRenderRect[3] = py1 - py0 + 1;
        }
        else {
            backgroundSprite.pushSprite(0, 0);
        }
        lastRenderPartial = partial;
        lastRenderSecondAngle = secAngle;
        recordRenderFrame(micros() - renderStartMicros, partial);

        lastFrame.valid = true;
        lastFrame.hourAngle = hourAngle;
        lastFrame.minuteAngle = minAngle;
        lastFrame.secondAngle = secAngle;
        lastFrame.drawSecond = drawSecond;
        lastFrame.drawHub = drawHub;
        lastFrame.hubSize = hubSize;
        lastFrame.hubColor = hubColor;
        lastFrame.brightness = currentBrightness;
        lastFrame.rotation = rotation;
        lastFrame.smoothSecond = smoothSecond;
        lastFrame.assetGeneration = clockAssetGeneration;
        clockFrameDirty[displayNum - 1] = false;
        return true;
    }


    // Liest Zeit/RTC einmal, dann ein renderClockFrame() pro angeschlossenem
    // Display ("n.a." wird uebersprungen). Bei Hardware-Rotation genuegt fuer
    // Display 2 erneutes Senden; bei GC9D01 nur, wenn tftRotation2 von tftRotation1 abweicht.

    // Reads time/RTC once, then one renderClockFrame() per connected display
    // ("n.a." is skipped). With hardware rotation, re-sending suffices for
    // Display 2; with GC9D01, only if tftRotation2 differs from tftRotation1.

    void updateClock() {
        // In eine lokale Kopie lesen statt direkt in die globale timeinfo:
        // waehrend die NTP-Sync-Task laeuft, wird die Systemzeit dort bewusst
        // kurzzeitig auf 1970 (ungueltig) gesetzt, um einen echten Sync-
        // Erfolg zu erkennen (siehe settimeofday(&invalidTime, ...) in
        // setupNTP(), time_sync.h) - das betrifft die Systemzeit insgesamt,
        // nicht nur die Sync-Task selbst. Wuerde getLocalTime() direkt in die
        // globale timeinfo schreiben, laende dieser Zwischenzustand bei einem
        // Fehlschlag (Jahr <= 2016) fuer einen Frame sichtbar hier: falsch
        // stehende Zeiger, und ueber updateBrightness() sogar ein kurzes
        // Abdunkeln, da die Stunde dann faelschlich ausserhalb des
        // Tagesfensters liegt.

        // Read into a local copy instead of directly into the global
        // timeinfo: while the NTP sync task is running, it deliberately sets
        // the system time to 1970 (invalid) for a moment to detect a genuine
        // sync success (see settimeofday(&invalidTime, ...) in setupNTP(),
        // time_sync.h) - that affects the system time as a whole, not just
        // the sync task itself. If getLocalTime() wrote directly into the
        // global timeinfo, this intermediate state would become visible here
        // for a frame on a failure (year <= 2016): hands pointing to the
        // wrong time, and via updateBrightness() even a brief dimming, since
        // the hour would then wrongly fall outside the daytime window.
        // Letzte bekannte gueltige Zeit + Zeitpunkt (millis()), zu dem sie
        // gelesen wurde - Grundlage fuer das Weiterrechnen unten, wenn
        // getLocalTime() kurzzeitig fehlschlaegt.
        // Last known valid time + the millis() moment it was read at - basis
        // for extrapolating below when getLocalTime() briefly fails.

        static time_t lastGoodEpoch = 0;
        static unsigned long lastGoodEpochMillis = 0;

        // Timeout 0 statt eines Wartewerts: schlaegt der Lesevorgang fehl,
        // rechnen wir unten ohnehin aus der letzten guten Zeit weiter - ein
        // Warten von bis zu 1s PRO loop()-Tick waere nur verschenkte Zeit
        // und wuerde ausgerechnet waehrend der NTP-Invalidierung (siehe
        // unten) den kompletten Haupt-Loop (Webserver, Touch, ...) fuer die
        // Dauer des Sync-Versuchs lahmlegen. Gleiches Prinzip wie beim
        // Timeout 0 in logToFile() (system_utils.h).

        // Timeout 0 instead of a wait value: if the read fails, we
        // extrapolate from the last good time below anyway - waiting up to
        // 1s PER loop() tick would just waste time, and during the NTP
        // invalidation (see below) would stall the entire main loop
        // (web server, touch, ...) for the whole sync attempt. Same
        // principle as the 0 timeout in logToFile() (system_utils.h).
        struct tm freshTimeinfo;
        if (getLocalTime(&freshTimeinfo, 0)) {
            timeinfo = freshTimeinfo;
            time_t now;
            time(&now);
            lastGoodEpoch = now;
            lastGoodEpochMillis = millis();
        }
        else if (lastGoodEpoch != 0) {

            // Waehrend die Systemzeit kurz ungueltig ist (z.B. die NTP-Sync-
            // Invalidierung, siehe settimeofday(&invalidTime, ...) in
            // setupNTP()), aus der letzten bekannten guten Zeit plus
            // verstrichener Zeit weiterrechnen, statt den Sekundenzeiger
            // anzuhalten. Sobald die echte Systemzeit wieder gueltig ist,
            // steigt der obige Zweig nahtlos wieder ein.

            // While the system time is briefly invalid (e.g. the NTP sync
            // invalidation, see settimeofday(&invalidTime, ...) in
            // setupNTP()), keep advancing from the last known good time plus
            // elapsed time, instead of pausing the second hand. Once the
            // real system time is valid again, the branch above picks back
            // up seamlessly.
            time_t estimatedNow = lastGoodEpoch + (time_t)((millis() - lastGoodEpochMillis) / 1000);
            localtime_r(&estimatedNow, &timeinfo);
        }
        else {
            // Noch nie eine gueltige Zeit gesehen (z.B. ganz am Anfang nach
            // dem Boot) - wie bisher auf die RTC zurueckfallen.

            // Never seen a valid time yet (e.g. right after boot) - fall
            // back to the RTC as before.
            loadTimeFromRTC();
        }


        static unsigned long lastRtcReloadMillis = 0; // Zeitpunkt des letzten RTC-Lesevorgangs (eigenstaendig, NICHT dieselbe Variable wie das globale lastRTCUpdate in time_sync.h/applyDcf77DecodedTime)
                                                      // timestamp of the last RTC read (independent, NOT the same variable as the global lastRTCUpdate in time_sync.h/applyDcf77DecodedTime)
        if (rtcOk == RTC_AVAILABLE) {
            // Überprüfen, ob seit dem letzten Aufruf Zeit vergangen ist
            // Check whether time has passed since the last call
            if (millis() - lastRtcReloadMillis >= WAIT_1h) {

                // NUR neu laden, wenn NTP nicht ohnehin erst vor kurzem (< der
                // NTP-Sync-Periode, siehe WAIT_6h in uhr4.ino) erfolgreich
                // synchronisiert hat (siehe lastNtpSuccessMillis in
                // globals.h/setupNTP()). Dieser Reload ist ein Sicherheitsnetz
                // fuer den Fall, dass WLAN/NTP laenger ausfaellt - laeuft NTP
                // aber normal (der Regelfall - dieser Check hier laeuft
                // bewusst weiterhin stuendlich, unabhaengig von der laengeren
                // NTP-Periode, um zeitnah zu reagieren, falls NTP tatsaechlich
                // ausfaellt), wuerde er eine bereits aktuelle, NTP-genaue
                // Systemzeit unnoetig durch die RTC ueberschreiben. Die RTC
                // zaehlt seit dem letzten Abgleich eigenstaendig weiter
                // (eigene, weniger praezise Uhr als der NTP-korrigierte
                // Systemtakt) und kann dabei um ein paar Sekunden abweichen -
                // loadTimeFromRTC() setzt ohne jede Richtungspruefung direkt
                // per settimeofday(), wodurch genau das den Sekundenzeiger
                // sichtbar (und faelschlich) zurueckspringen liess. Die
                // Schwelle MUSS mit der tatsaechlichen NTP-Sync-Periode
                // uebereinstimmen (WAIT_6h, nicht mehr WAIT_1h) - sonst wuerde
                // dieser Reload bei einer laengeren NTP-Periode wieder bei
                // JEDEM Durchlauf greifen, weil NTP zwischen zwei Syncs
                // immer "aelter als 1h" waere, obwohl es normal laeuft.

                // ONLY reload when NTP hasn't already succeeded recently
                // (< the NTP sync period, see WAIT_6h in uhr4.ino) (see
                // lastNtpSuccessMillis in globals.h/setupNTP()). This reload
                // is a safety net for when WiFi/NTP is down for a longer
                // stretch - but if NTP is running normally (the usual case -
                // this check here deliberately still runs hourly, independent
                // of the longer NTP period, to react promptly if NTP
                // actually does fail), it would needlessly overwrite an
                // already-current, NTP-accurate system time with the RTC's.
                // The RTC keeps counting on its own since the last
                // adjustment (a separate, less precise clock than the
                // NTP-corrected system clock) and can have drifted by a few
                // seconds by then - loadTimeFromRTC() sets it directly via
                // settimeofday() with no direction check at all, which is
                // exactly what made the second hand visibly (and wrongly)
                // jump backward. The threshold MUST match the actual NTP
                // sync period (WAIT_6h, no longer WAIT_1h) - otherwise this
                // reload would fire on EVERY pass again with a longer NTP
                // period, since NTP would always be "older than 1h" between
                // two syncs even while running normally.
                if (lastNtpSuccessMillis == 0 || millis() - lastNtpSuccessMillis >= WAIT_6h) {
                    loadTimeFromRTC();
                }
                lastRtcReloadMillis = millis();
            }
        }

        // Modellzeit unabhaengig von obigem real-zeit-basiertem Block
        // fortschreiben - laeuft nur an, wenn rocrailEnabled (siehe dort).

        // Advance the model time independent of the real-time-based block
        // above - only does anything when rocrailEnabled (see there).
        advanceRocrailTime();

        // Bug: die Bahnhofsuhr-Schrittlogik (stationTick, siehe
        // renderClockFrame()) initialisiert sich nur beim ALLERERSTEN Aufruf
        // (firstRun/firstRun2) anhand der dann verfuegbaren Zeit. War timeinfo
        // zu dem Zeitpunkt noch nicht gueltig (kein RTC, NTP/DCF77 noch nicht
        // synchronisiert - timeinfo steht dann auf seinem Nullwert, Jahr 1900),
        // blieb stationTick auf dieser falschen Basis stehen und wurde erst
        // beim naechsten Bahnhofsuhr-Minutenwechsel per Resync korrigiert -
        // sichtbar als "Sekundenzeiger stimmt anfangs nicht, springt erst nach
        // einer Weile auf den richtigen Wert". Sobald timeinfo zum ERSTEN Mal
        // plausibel wird (Jahr >= 2000 - auch der 12:00-Notfallwert aus
        // handleNTPFailure() zaehlt dazu, siehe dort), daher einmalig
        // firstRun/firstRun2 erneut auf true setzen, damit sich die Animation
        // sauber auf die jetzt gueltige Zeit neu einstellt. Betrifft nur den
        // Bahnhofsuhr-Modus - tickende/sanfte Darstellung im Normalmodus leiten
        // ihre Position ohnehin jeden Frame frisch aus timeinfo ab und
        // korrigieren sich dadurch schon von selbst (bei einem Ruecksprung
        // sanft nachgefuehrt statt gesprungen - siehe die Sekundenzeiger-
        // Behandlung in renderClockFrame()).

        // Bug: the station-clock stepping logic (stationTick, see
        // renderClockFrame()) initializes itself only on the VERY FIRST call
        // (firstRun/firstRun2), based on whatever time is available then. If
        // timeinfo wasn't valid yet at that point (no RTC, NTP/DCF77 not yet
        // synced - timeinfo then sits at its zero value, year 1900),
        // stationTick stayed on that wrong baseline and only got corrected via
        // resync at the next station-clock minute change - visible as "the
        // second hand doesn't match at first, only jumps to the right value
        // after a while". So, once timeinfo becomes plausible for the FIRST
        // time (year >= 2000 - the 12:00 emergency value from
        // handleNTPFailure() counts too, see there), force firstRun/firstRun2
        // back to true once, so the animation cleanly re-baselines on the now
        // valid time. Only affects station-clock mode - ticking/smooth
        // rendering in normal mode derives its position fresh from timeinfo
        // every single frame anyway and therefore already self-corrects on
        // its own (eased rather than snapped for a backward step - see the
        // second-hand handling in renderClockFrame()).
        static bool hadPlausibleTime = false;
        if (!hadPlausibleTime && timeinfo.tm_year >= 100) {
            hadPlausibleTime = true;
            firstRun = true;
            firstRun2 = true;
        }

        // Displays mit Rotation "n.a." werden bei der Uhranzeige weder selektiert
        // noch berechnet. Ist nur Display 2 angeschlossen, rendert es sein Bild selbst.
        // firstRun bleibt dabei auf true, damit das Display beim spaeteren
        // Anschliessen (Einstellungsaenderung zur Laufzeit) sofort korrekt einrastet.

        // Displays with rotation "n.a." are neither selected nor calculated for
        // the clock display. If only display 2 is connected, it renders its own frame.
        // firstRun stays true meanwhile, so the display snaps correctly as
        // soon as it gets connected later (settings change at runtime).
        const bool display1Connected = isDisplayConnected(1);
        const bool display2Connected = isDisplayConnected(2);

        // Ein "n.a."-Display zeigt nur schwarz: letzten Inhalt (Uhr oder Status-/
        // Startmeldung) einmalig loeschen, danach wird es nicht mehr angefasst.

        // A "n.a." display shows only black: clear its last content (clock or
        // status/boot message) once, after that it is not touched anymore.
        for (uint8_t d = 1; d <= 2; d++) {
            if (!isDisplayConnected(d) && displayNeedsBlank[d - 1]) {
                if (d == 1) setCS1(LOW); else setCS2(LOW);
                tft.fillScreen(TFT_BLACK);
                displayNeedsBlank[d - 1] = false;
            }
        }

        bool display1Pushed = false;
        if (display1Connected) {
            setCS1(LOW);
            display1Pushed = renderClockFrame(1, tftRotation1, lastHourAngle, lastMinuteAngle, lastSecondAngle, firstRun);
        }
        else {
            firstRun = true;
        }

        if (!display2Connected) {
            firstRun2 = true;
            clockFrameDirty[1] = true; // beim spaeteren Anschliessen sofort senden (auch im Spiegelbetrieb)
                                       // send right away once connected later (mirror mode too)
        }
        else if (!display1Connected || (gc9d01SwRotation && tftRotation2 != tftRotation1)) {
            setCS2(LOW);
            renderClockFrame(2, tftRotation2, lastHourAngle2, lastMinuteAngle2, lastSecondAngle2, firstRun2);
        }
        else {
            // Spiegelbetrieb: nur senden, wenn Display 1 ein neues Bild hat
            // oder Display 2 selbst ueberzeichnet wurde - backgroundSprite
            // enthaelt dann weiterhin das zuletzt gerenderte Uhrbild.

            // Mirror mode: only send when display 1 has a new image or display
            // 2 itself got drawn over - backgroundSprite then still holds the
            // last rendered clock image.
            if (display1Pushed || clockFrameDirty[1]) {
                setCS2(LOW);
                // Hat Display 1 nur ein Rechteck gesendet (Teil-Aktualisierung),
                // reicht fuer Display 2 dasselbe - ausser es wurde selbst
                // ueberzeichnet (clockFrameDirty[1]), dann voll senden.
                // If display 1 only sent a rectangle (partial update), the same
                // is enough for display 2 - unless it got drawn over itself
                // (clockFrameDirty[1]), then send in full.
                if (lastRenderPartial && !clockFrameDirty[1]) {
                    pushBackgroundRect(lastRenderRect[0], lastRenderRect[1], lastRenderRect[2], lastRenderRect[3]);
                }
                else {
                    backgroundSprite.pushSprite(0, 0);
                }
                clockFrameDirty[1] = false;
            }

            // firstRun2 bewusst auf true halten: rastet tftRotation2 spaeter
            // (per Einstellungsaenderung zur Laufzeit) wieder von tftRotation1
            // ab, soll der naechste eigenstaendige Frame fuer Display 2 sofort
            // an der korrekten Winkelposition einrasten, statt sich aus einem
            // waehrend dieser Zeit nie aktualisierten (also veralteten)
            // lastHourAngle2/lastMinuteAngle2 heranzutasten.

            // Deliberately keep firstRun2 at true: if tftRotation2 later
            // diverges again from tftRotation1 (via a runtime settings
            // change), the next standalone frame for Display 2 should snap
            // straight to the correct angle instead of easing in from a
            // lastHourAngle2/lastMinuteAngle2 that was never updated (and so
            // went stale) during this time.

            firstRun2 = true;
        }

        setCSIdle(); // definierter Zustand fuer alles, was danach noch direkt auf 'tft' zeichnet
                     // defined state for anything that draws directly to 'tft' afterwards
    }


    // Aktualisiert die Display-Helligkeit anhand Einstellung, ADC-Wert (falls
    // aktiviert) und Tageszeitfenster - oder, mit Vorrang vor beidem, anhand
    // der vom Rocrail-Server gemeldeten Helligkeit (siehe rocrailBrightnessActive weiter unten).

    // Updates the display brightness from the setting, ADC value (if
    // enabled), and the daytime window - or, taking priority over both, from
    // the brightness reported by the Rocrail server (see rocrailBrightnessActive below).

    void updateBrightness() {
        // Eigene Vergleichsvariable fuer die Zeiger: lastAppliedBrightness wird
        // von loadClockFace() selbst gepflegt, das Zifferblatt braucht hier keinen Aufruf.
        // Nicht bei JEDEM Rampenschritt neu einfaerben (kostet Preferences-/
        // LittleFS-Zugriffe), sondern nur bei spuerbarer Differenz plus einmal
        // am Rampenende fuer den exakten Endwert.

        // Its own comparison variable for the hands: lastAppliedBrightness is
        // maintained by loadClockFace() itself, the clock face needs no call here.
        // Don't re-tint on EVERY ramp step (costly preferences/LittleFS access),
        // only on a noticeable difference plus once at the end of the ramp for
        // the exact final value.

        const uint8_t HAND_RETINT_STEP = 8;

        int handBrightnessDelta = (int)currentBrightness - (int)lastHandBrightness;
        if (handBrightnessDelta < 0) handBrightnessDelta = -handBrightnessDelta;

        // Mit Backlight bleiben die Pixel unveraendert (setPixelBrightness()) -
        // Neueinfaerben waere nur Datei-Zugriff und ein verworfenes Zwischenbild.

        // With a backlight the pixels stay unchanged (setPixelBrightness()) -
        // re-tinting would only cost file access and a discarded composite.
        if (!useBacklight && (handBrightnessDelta >= HAND_RETINT_STEP ||
            (handBrightnessDelta > 0 && currentBrightness == targetBrightness))) {
            loadHandSprites();
            lastHandBrightness = currentBrightness;
        }

        // Prüfen, ob wir aktuell im konfigurierten Voll-Helligkeits-Zeitfenster sind
        // Check whether we're currently within the configured full-brightness time window
        // Letzten bekannten Stand beibehalten statt bei einem getLocalTime()-
        // Fehlschlag faelschlich auf "false" (Nacht) zu wechseln - genau das
        // wuerde waehrend der kurzen NTP-Sync-bedingten Systemzeit-
        // Invalidierung (siehe updateClock()/setupNTP()) sonst die
        // Helligkeit grundlos auf minBrightness fallen lassen.

        // Keep the last known state instead of wrongly falling back to
        // "false" (night) on a getLocalTime() failure - that's exactly what
        // would otherwise drop the brightness to minBrightness for no
        // reason during the brief NTP-sync-induced system time invalidation
        // (see updateClock()/setupNTP()).

        static bool withinDayWindow = false;
        static bool timeEverValid = false; // seit dem Start je eine gueltige Uhrzeit / valid time ever since boot

        // struct tm timeinfo;
        struct tm freshTimeinfo;
        // Timeout 0 - siehe Begruendung bei updateClock() (display.h):
        // schlaegt der Lesevorgang fehl, bleibt withinDayWindow ohnehin auf
        // dem letzten bekannten Stand (siehe oben), ein Warten wuerde nur
        // den Haupt-Loop waehrend der NTP-Invalidierung unnoetig blockieren.

        // Timeout 0 - see the reasoning at updateClock() (display.h): if the
        // read fails, withinDayWindow stays at its last known state anyway
        // (see above), waiting would just needlessly block the main loop
        // during the NTP invalidation.
        if (getLocalTime(&freshTimeinfo, 0)) {
            timeinfo = freshTimeinfo; // nur bei Erfolg uebernehmen, siehe updateClock()
                                      // only adopt on success, see updateClock()
            timeEverValid = true;
            int h = freshTimeinfo.tm_hour;
            if (brightStartHour <= brightEndHour) {
                // normaler Bereich z.B. 8..20
                // normal range e.g. 8..20
                withinDayWindow = (h >= brightStartHour && h < brightEndHour);
            }
            else {
                // über Mitternacht z.B. 20..6
                // spanning midnight e.g. 20..6
                withinDayWindow = (h >= brightStartHour || h < brightEndHour);
            }
        }

        // Fotowiderstand-Rohwert IMMER aktualisieren, unabhaengig vom
        // Tagesfenster - dient auch als Live-Anzeige (Topbar); die
        // Helligkeits-ENTSCHEIDUNG haengt weiterhin vom Zeitfenster ab.

        // Always update the raw photoresistor reading, independent of the
        // daytime window - also serves as a live display (topbar); the
        // brightness DECISION still depends on the time window.
#ifdef ADC_PIN
        if (useAdc) {

            // Nur alle ADC_SAMPLE_INTERVAL_MS neu abtasten statt bei jedem
            // loop()-Tick: sonst deckt das ADC_SMOOTHING-Mittel nur wenige
            // Millisekunden ab, und ein kurzer Stromspitzen-Einbruch (z.B.
            // WLAN-Sendeburst waehrend NTP-Sync) faerbt fast jedes Sample im
            // Fenster gleich ein, statt herausgemittelt zu werden.

            // Only re-sample every ADC_SAMPLE_INTERVAL_MS instead of on every
            // loop() tick: otherwise the ADC_SMOOTHING average spans only a
            // few milliseconds, and a brief current-draw dip (e.g. a WiFi TX
            // burst during NTP sync) taints nearly every sample in the
            // window instead of being averaged out.
            if (initial || millis() - lastAdcSampleMillis >= ADC_SAMPLE_INTERVAL_MS) {
                lastAdcSampleMillis = millis();

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
                                      // save

                currentLightPercent = map(avg, 0, 4095, 5, 100);
            }
        }
#endif

        // Rocrail-Helligkeit hat Vorrang vor Zeitfenster UND Fotowiderstand,
        // sobald verbunden und ein bri-Wert bekannt ist - dieselbe Stale-
        // Pruefung wie rocrailTimeReady faellt sonst nach ROCRAIL_STALE_TIMEOUT_MS auf lokale Steuerung zurueck.

        // Rocrail brightness takes priority over both the time window and
        // the photoresistor, once connected and a bri value is known - the
        // same staleness check as rocrailTimeReady falls back to local control after ROCRAIL_STALE_TIMEOUT_MS otherwise.
        bool rocrailBrightnessActive = rocrailEnabled && rocrailConnected && rocrailBrightnessKnown &&
                                        (millis() - rocrailLastClockMillis) < ROCRAIL_STALE_TIMEOUT_MS;

        // Einrichtung: noch nie eine gueltige Uhrzeit (Tag/Nacht unbekannt,
        // die Uhr zeigt ohnehin noch keine Zeit), WPS-Suche oder Access Point
        // -> volle Helligkeit, damit Meldungen und AP-Passwort lesbar sind.
        // Sonst blieb eine neue Uhr ohne WLAN und ohne hellen Lichtsensor auf
        // minBrightness - mit Backlight (5 von 255) praktisch dunkel (GC9D01).

        // Setup: never a valid time yet (day/night unknown, the clock shows no
        // time anyway), WPS search or access point -> full brightness so that
        // messages and the AP password are readable. Otherwise a new clock
        // without WiFi and without a bright light sensor stayed at
        // minBrightness - with a backlight (5 of 255) practically dark (GC9D01).
        bool setupBrightness = !timeEverValid || softAPIP || wpsPending;

        if (setupBrightness) {
            targetBrightness = maxBrightness;
            currentBrightness = maxBrightness;
        }
        else if (rocrailBrightnessActive) {
            targetBrightness = rocrailBrightness;
            if (useBacklight) {
                // sanfte Anpassung wie beim Zeitfenster/ADC unten, statt eines
                // harten Sprungs bei jeder Server-Aenderung.

                // smooth adjustment like the time window/ADC below, instead of a
                // hard jump on every server-side change.
                if (currentBrightness < targetBrightness) currentBrightness++;
                else if (currentBrightness > targetBrightness) currentBrightness--;
            }
            else {
                currentBrightness = targetBrightness;
            }
        }
        else if (withinDayWindow) {
            // Zeitfenster aktiv und wir sind innerhalb davon: volle Helligkeit erzwingen
            // Time window active and we're inside it: force full brightness
            targetBrightness = maxBrightness;
            if (useBacklight) {
                // sanfte Erhöhung, falls gewünscht (ähnlich wie ADC-Rampen)
                // smooth increase if desired (similar to ADC ramps)
                if (currentBrightness < targetBrightness) currentBrightness++;
                else if (currentBrightness > targetBrightness) currentBrightness--;
            }
            else {
                currentBrightness = targetBrightness;
            }
        }
        else {
#ifdef ADC_PIN
            // Normale Auto-Brightness oder statische Helligkeit
            // Normal auto-brightness or static brightness
            if (useAdc) {
                // currentAdcAvg/currentLightPercent wurden oben bereits fuer
                // diesen Durchlauf aktualisiert - hier nur noch die
                // Helligkeits-ENTSCHEIDUNG anhand der frischen Werte.
                // Schwellwert-Ueberschreitung erst nach BRIGHTNESS_DEBOUNCE_MS
                // Bestand uebernehmen: ein kurzer Ausreisser (z.B. WLAN-
                // Sendeburst waehrend NTP-Sync) soll targetBrightness nicht
                // sofort umschalten, sonst faerbt setPixelBrightness() das
                // komplette Zifferblatt fuer einen Frame sichtbar um.

                // currentAdcAvg/currentLightPercent were already updated
                // above for this pass - only the brightness DECISION based
                // on those fresh values happens here.
                // Only act on a threshold crossing once it has persisted for
                // BRIGHTNESS_DEBOUNCE_MS: a brief outlier (e.g. a WiFi TX
                // burst during NTP sync) must not flip targetBrightness
                // immediately, or setPixelBrightness() visibly re-tints the
                // whole clock face for a frame.

                int desiredBrightnessState = 0; // -1 = Kandidat fuer minBrightness, 1 = fuer maxBrightness
                                                // -1 = candidate for minBrightness, 1 = for maxBrightness
                if (currentLightPercent < lowThreshold) desiredBrightnessState = -1;
                else if (currentLightPercent > highThreshold) desiredBrightnessState = 1;

                if (desiredBrightnessState != pendingBrightnessState) {
                    pendingBrightnessState = desiredBrightnessState;
                    // initial: Debounce ueberspringen, sonst startete die Uhr
                    // erst nach BRIGHTNESS_DEBOUNCE_MS mit korrekter Helligkeit.
                    // initial: skip the debounce, otherwise the clock would
                    // only reach the correct brightness after BRIGHTNESS_DEBOUNCE_MS.
                    brightnessThresholdSinceMillis = initial ? (millis() - BRIGHTNESS_DEBOUNCE_MS) : millis();
                }

                bool brightnessStateDebounced = (millis() - brightnessThresholdSinceMillis) >= BRIGHTNESS_DEBOUNCE_MS;
                if (pendingBrightnessState != 0 && brightnessStateDebounced) {
                    targetBrightness = (pendingBrightnessState < 0) ? minBrightness : maxBrightness;
                }
                else if (useBacklight && pendingBrightnessState == 0) {
                    // Zwischen den Schwellen stufenlos (nur mit Backlight -
                    // ohne muesste jeder Zwischenwert das Zifferblatt neu einfaerben).

                    // Stepless between the thresholds (backlight only - without
                    // one, every intermediate value would re-tint the clock face).
                    float norm = constrain((float)currentAdcAvg / 4095.0f, 0.0f, 1.0f);
                    float gamma = gammaBrightness;
                    float gammaNorm = powf(norm, gamma);
                    targetBrightness = minBrightness + (uint8_t)((maxBrightness - minBrightness) * gammaNorm + 0.5f);
                }

                if (initial) currentBrightness = targetBrightness;

                if (useBacklight) {
                    if (currentBrightness < targetBrightness) currentBrightness++;
                    else if (currentBrightness > targetBrightness) currentBrightness--;
                }
                else {
                    currentBrightness = targetBrightness;
                }

            }
            else {
                // kein ADC: Standardeinstellung
                // No ADC: default setting
                currentBrightness = minBrightness;
                targetBrightness = currentBrightness;
            }
#endif
        }

        if (useBacklight && backlightAttached) {
            ledcWrite(TFT_Backlight, currentBrightness);  // 0–255
                                                          // 0-255
        }

    }


    // Passt den ADC-Wert an, wenn die Invertierung aktiviert ist
    // Adjusts the ADC value when inversion is enabled

    uint16_t getAdjustedAdcValue(int rawValue) {
        if (adcInverted) {
            return 4096 - rawValue; // Invertiere den Wert
                                    // invert the value
        }
        return rawValue; // Standardwert
                         // default value
    }


    /// Easing-Funktion für sanfte Animationen
    // Easing function for smooth animations

    float easeInOutSine(float t) {
        // Bildet die Sekundenzeiger-Bewegung im Bahnhofsuhr-Modus (siehe
        // renderClockFrame()); als JavaScript in der Web-Vorschau gespiegelt
        // (webserver_routes.h) - Aenderungen hier dort nachziehen.

        // Shapes the second hand's movement in station clock mode (see
        // renderClockFrame()); mirrored as JavaScript in the web preview
        // (webserver_routes.h) - keep changes here in sync there.

        // Intensität steuert die Kurve: 1.0 = Standard, >1.0 = steiler, <1.0 = flacher
        // Intensity controls the curve: 1.0 = default, >1.0 = steeper, <1.0 = flatter
        float intensity = 0.5f;
        return -(cos(PI * pow(t, intensity)) - 1.0f) / 2.0f;
    }


    // CRC32 (Standard-Polynom 0xEDB88320) - fuer PNG-Chunk-Pruefsummen
    // CRC32 (standard polynomial 0xEDB88320) - for PNG chunk checksums

    uint32_t crc32Update(uint32_t crc, const uint8_t* buf, size_t len) {
        crc = ~crc;
        while (len--) {
            crc ^= *buf++;
            for (int i = 0; i < 8; i++) {
                crc = (crc >> 1) ^ (0xEDB88320 & (-(int32_t)(crc & 1)));
            }
        }
        return ~crc;
    }


    // Adler32 - fuer den zlib-Trailer im PNG-IDAT-Chunk
    // Adler32 - for the zlib trailer in the PNG IDAT chunk

    uint32_t adler32(const uint8_t* data, size_t len) {
        uint32_t a = 1, b = 0;
        const uint32_t MOD_ADLER = 65521;
        for (size_t i = 0; i < len; i++) {
            a = (a + data[i]) % MOD_ADLER;
            b = (b + a) % MOD_ADLER;
        }
        return (b << 16) | a;
    }


    // Haengt einen PNG-Chunk (Typ + Daten + CRC32) an einen dynamischen Puffer an.
    // Appends a PNG chunk (type + data + CRC32) to a dynamic buffer.

    void appendPngChunk(std::vector<uint8_t>& out, const char* type, const uint8_t* data, uint32_t len) {
        uint8_t lenBytes[4] = { (uint8_t)(len >> 24), (uint8_t)(len >> 16), (uint8_t)(len >> 8), (uint8_t)len };
        out.insert(out.end(), lenBytes, lenBytes + 4);
        size_t typeStart = out.size();
        out.insert(out.end(), type, type + 4);
        if (len > 0) out.insert(out.end(), data, data + len);
        uint32_t crc = crc32Update(0, &out[typeStart], 4 + len);
        uint8_t crcBytes[4] = { (uint8_t)(crc >> 24), (uint8_t)(crc >> 16), (uint8_t)(crc >> 8), (uint8_t)crc };
        out.insert(out.end(), crcBytes, crcBytes + 4);
    }


    // Kodiert ein RGB565-Bild als PNG (echte Alpha-Transparenz) und liefert
    // Base64. TRANSPARENT_COLOR und Weiss (0xFFFF) werden zu Alpha=0 - anders
    // als encodeBmpToBase64() (BMP ohne Alpha, Transparenz nur als Weiss).

    // Encodes an RGB565 image as PNG (true alpha transparency) and returns
    // base64. TRANSPARENT_COLOR and white (0xFFFF) become alpha=0 - unlike
    // encodeBmpToBase64() (BMP without alpha, transparency shown as white only).

    String encodePngToBase64(const uint16_t* data, int width, int height) {
        // Rohe Bilddaten: pro Zeile 1 Filter-Byte (0 = "None") + width*4 Byte RGBA
        // Raw image data: 1 filter byte per row (0 = "None") + width*4 bytes RGBA
        size_t rawRowSize = 1 + (size_t)width * 4;
        size_t rawSize = rawRowSize * height;
        uint8_t* raw = (uint8_t*)preferPsramMalloc(rawSize);
        if (!raw) return "";

        for (int y = 0; y < height; y++) {
            uint8_t* rowPtr = raw + y * rawRowSize;
            rowPtr[0] = 0; // Filter-Byte: keine Filterung
                           // filter byte: no filtering
            for (int x = 0; x < width; x++) {
                uint16_t px = data[y * width + x];
                uint8_t r = ((px >> 11) & 0x1F) * 255 / 31;
                uint8_t g = ((px >> 5) & 0x3F) * 255 / 63;
                uint8_t b = (px & 0x1F) * 255 / 31;
                uint8_t a = (px == TRANSPARENT_COLOR || px == 0xFFFF) ? 0 : 255;
                uint8_t* px_out = rowPtr + 1 + x * 4;
                px_out[0] = r; px_out[1] = g; px_out[2] = b; px_out[3] = a;
            }
        }

        // zlib-Stream mit unkomprimierten ("stored") Deflate-Bloecken - vermeidet
        // eine vollstaendige Deflate-Implementierung, bleibt aber gueltiges PNG.

        // zlib stream with uncompressed ("stored") deflate blocks - avoids
        // a full deflate implementation while staying valid PNG.
        std::vector<uint8_t> zlibStream;
        zlibStream.push_back(0x78); zlibStream.push_back(0x01); // zlib-Header (keine Kompression)
                                                                // zlib header (no compression)

        size_t offset = 0;
        const size_t maxBlock = 65535;
        while (offset < rawSize) {
            size_t blockLen = min(maxBlock, rawSize - offset);
            bool isFinal = (offset + blockLen >= rawSize);
            zlibStream.push_back(isFinal ? 0x01 : 0x00);
            uint16_t len16 = (uint16_t)blockLen;
            uint16_t nlen16 = ~len16;
            zlibStream.push_back(len16 & 0xFF); zlibStream.push_back(len16 >> 8);
            zlibStream.push_back(nlen16 & 0xFF); zlibStream.push_back(nlen16 >> 8);
            zlibStream.insert(zlibStream.end(), raw + offset, raw + offset + blockLen);
            offset += blockLen;
        }
        uint32_t adler = adler32(raw, rawSize);
        zlibStream.push_back((adler >> 24) & 0xFF);
        zlibStream.push_back((adler >> 16) & 0xFF);
        zlibStream.push_back((adler >> 8) & 0xFF);
        zlibStream.push_back(adler & 0xFF);
        free(raw);

        // PNG zusammenbauen: Signatur + IHDR + IDAT + IEND
        // Assemble the PNG: signature + IHDR + IDAT + IEND
        std::vector<uint8_t> png;
        const uint8_t pngSig[8] = { 0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A };
        png.insert(png.end(), pngSig, pngSig + 8);

        uint8_t ihdr[13];
        ihdr[0] = (width >> 24) & 0xFF; ihdr[1] = (width >> 16) & 0xFF; ihdr[2] = (width >> 8) & 0xFF; ihdr[3] = width & 0xFF;
        ihdr[4] = (height >> 24) & 0xFF; ihdr[5] = (height >> 16) & 0xFF; ihdr[6] = (height >> 8) & 0xFF; ihdr[7] = height & 0xFF;
        ihdr[8] = 8;  // Bittiefe
                      // bit depth
        ihdr[9] = 6;  // Farbtyp: RGBA
                      // color type: RGBA
        ihdr[10] = 0; ihdr[11] = 0; ihdr[12] = 0;
        appendPngChunk(png, "IHDR", ihdr, 13);
        appendPngChunk(png, "IDAT", zlibStream.data(), zlibStream.size());
        appendPngChunk(png, "IEND", nullptr, 0);

        String result = base64::encode(png.data(), png.size());
        result.replace("\n", "");
        return result;
    }


    // Erzeugt rohe BMP-Bytes aus RGB565-Daten (Aufrufer muss delete[]).
    // WICHTIG: BI_BITFIELDS mit expliziten RGB565-Masken statt BI_RGB - sonst
    // interpretieren Viewer/Browser ein 16-Bit-BMP als 5-5-5 und Farben verrauschen.

    // Generates raw BMP bytes from RGB565 data (caller must delete[]).
    // IMPORTANT: BI_BITFIELDS with explicit RGB565 masks instead of BI_RGB -
    // otherwise viewers/browsers interpret a 16-bit BMP as 5-5-5 and colors turn to noise.

    uint8_t* encodeBmpToBytes(const uint16_t* data, int width, int height, size_t* outSize) {
        const int fileHeaderSize = 14;
        const int infoHeaderSize = 40;
        const int bitmasksSize = 12; // 3x uint32_t: R-, G-, B-Maske
                                     // 3x uint32_t: R, G, B mask
        const int headerSize = fileHeaderSize + infoHeaderSize + bitmasksSize; // 66
                                                                               // 66
        const int rowSize = ((width * 2 + 3) / 4) * 4;
        const int dataSize = rowSize * height;
        const int fileSize = headerSize + dataSize;

        uint8_t* bmpData = new (std::nothrow) uint8_t[fileSize];
        if (!bmpData) { *outSize = 0; return nullptr; }

        memset(bmpData, 0, fileSize);

        // BITMAPFILEHEADER (14 Byte)
        // BITMAPFILEHEADER (14 bytes)
        bmpData[0] = 'B'; bmpData[1] = 'M';
        *(uint32_t*)&bmpData[2] = fileSize;
        *(uint32_t*)&bmpData[10] = headerSize; // Offset zu den Pixeldaten
                                               // offset to the pixel data
        // BITMAPINFOHEADER (40 Byte)
        // BITMAPINFOHEADER (40 bytes)
        *(uint32_t*)&bmpData[14] = infoHeaderSize;
        *(int32_t*)&bmpData[18] = width;
        *(int32_t*)&bmpData[22] = -height; // Top-down-BMP
                                           // Top-down BMP
        *(uint16_t*)&bmpData[26] = 1;
        *(uint16_t*)&bmpData[28] = 16;
        *(uint32_t*)&bmpData[30] = 3; // biCompression = BI_BITFIELDS
                                      // biCompression = BI_BITFIELDS
        *(uint32_t*)&bmpData[34] = dataSize;

        // Explizite RGB565-Bitmasken (direkt nach der BITMAPINFOHEADER)
        // Explicit RGB565 bit masks (right after the BITMAPINFOHEADER)
        *(uint32_t*)&bmpData[54] = 0xF800; // Rot:   5 Bit
                                           // Red:   5 bits
        *(uint32_t*)&bmpData[58] = 0x07E0; // Gruen: 6 Bit
                                           // Green: 6 bits
        *(uint32_t*)&bmpData[62] = 0x001F; // Blau:  5 Bit
                                           // Blue:  5 bits

        for (int y = 0; y < height; y++) {
            uint8_t* rowPtr = bmpData + headerSize + y * rowSize;
            for (int x = 0; x < width; x++) {
                uint16_t px = data[y * width + x];
                if (px == TRANSPARENT_COLOR) px = 0xFFFF;

                rowPtr[x * 2] = px & 0xFF;
                rowPtr[x * 2 + 1] = px >> 8;
            }
        }

        *outSize = fileSize;
        return bmpData;
    }


    String encodeBmpToBase64(const uint16_t* data, int width, int height) {
        size_t fileSize = 0;
        uint8_t* bmpData = encodeBmpToBytes(data, width, height, &fileSize);
        if (!bmpData) return "";

        String result = base64::encode(bmpData, fileSize);
        result.replace("\n", "");

        delete[] bmpData;

        return result;
    }


    // TFT-Display loeschen
    // clear TFT display

    void clearTFT() {
        DRAW_ON_BOTH_DISPLAYS(
            tft.fillRect(0, 0, CLOCK_WIDTH, CLOCK_HEIGHT, TFT_BLACK);
        );
    }


    // Rotiert die Zeigerwinkel - nur relevant fuer GC9D01 mit aktivem Software-
    // Rotations-Workaround (Hardware-Rotation dort wirkungslos, siehe uhr4.ino).
    // Sonst (gc9d01SwRotation=false) wird angle unveraendert zurueckgegeben.

    // Rotates the hand angles - only relevant for GC9D01 with the software
    // rotation workaround active (hardware rotation has no effect there, see
    // uhr4.ino). Otherwise (gc9d01SwRotation=false) angle is returned unchanged.

    float rotatedAngle(float angle, int orientation) {
        if (gc9d01SwRotation) {
            return angle + (orientation * 90);
        }
        return angle;
    }


    // überprüft, ob die BMP-Datei das erwartete Format hat
    // Checks whether the BMP file has the expected format

    bool checkBmpFormat(const String& filename, int expectedWidth, int expectedHeight) {
        File bmpFile = LittleFS.open(filename, "r");
        if (!bmpFile) {
            DEBUG_PRINTLN("[BMP Check] Failed to open file");
            return false;
        }

        uint8_t magic[4];
        if (bmpFile.read(magic, 4) != 4) {
            DEBUG_PRINTLN("[BMP Check] Failed to read header");
            bmpFile.close();
            return false;
        }

        if (isRleFace(magic)) {
            uint8_t rest[16];
            bool ok = bmpFile.read(rest, 16) == 16;
            bmpFile.close();
            if (!ok) {
                DEBUG_PRINTLN("[BMP Check] Failed to read RLEB header");
                return false;
            }
            int32_t width = *(int32_t*)&rest[0];
            int32_t height = *(int32_t*)&rest[4];
            if (width != expectedWidth || height != expectedHeight) {
                DEBUG_PRINTF("[BMP Check] Invalid RLEB dimensions: %d x %d", width, height);
                return false;
            }
            DEBUG_PRINTLN("[BMP Check] RLEB format valid");
            return true;
        }

        bmpFile.seek(0);
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


    // Liest die BMP-/RLEB-Header-Informationen und gibt sie als String zurück
    // Reads the BMP/RLEB header info and returns it as a string

    String getBmpInfo(const String& filename) {
        // Normalisiere Pfad (einfach und eindeutig)
        // Normalize path (simple and unambiguous)
        String file = filename;
        if (!file.startsWith("/")) file = "/" + file;

        File bmp = LittleFS.open(file, "r");
        if (!bmp) {
            return "n/a";
        }
        uint8_t magic[4];
        if (bmp.read(magic, 4) != 4) {
            bmp.close();
            return "n/a";
        }

        if (isRleFace(magic)) {
            uint8_t rest[16];
            bool ok = bmp.read(rest, 16) == 16;
            bmp.close();
            if (!ok) return "n/a";
            int32_t width = *(int32_t*)&rest[0];
            int32_t height = *(int32_t*)&rest[4];
            uint32_t compressedSize = *(uint32_t*)&rest[8];
            uint32_t uncompressedSize = *(uint32_t*)&rest[12];
            String ratio = uncompressedSize > 0 ? String(100 - (compressedSize * 100 / uncompressedSize)) + "%" : "?";
            return String(width) + " x " + String(height) + " / 16 bpp (RLE, -" + ratio + ")";
        }

        bmp.seek(0);
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


    // Skaliert eine BMP-Datei auf die gewünschte Größe und speichert sie
    // Scales a BMP file to the desired size and saves it

    bool scaleAndSaveBmp(const char* sourcePath, const char* targetPath, int outW, int outH) {
        DEBUG_PRINTLN("[BMP Scale] Scaling BMP: " + String(sourcePath) + " to " + String(targetPath));
        File bmp = LittleFS.open(sourcePath, "r");
        if (!bmp) {
            DEBUG_PRINTLN("[BMP Scale] Failed to open source file");
            return false;
        }

        uint8_t magic[4];
        if (bmp.read(magic, 4) != 4) {
            bmp.close();
            DEBUG_PRINTLN("[BMP Scale] Invalid header");
            return false;
        }

        // Quelle einlesen: RLEB komplett dekodiert, Standard-BMP zeilenweise (speicherschonend)
        // Read source: RLEB fully decoded, standard BMP row by row (memory-friendly)
        int32_t inW = 0, inH = 0;
        uint16_t bpp = 16;
        bool flip = false;
        uint32_t offset = 0;
        int inRowSize = 0;
        uint8_t* rowBuf = nullptr;   // fuer Standard-BMP: ein Zeilenpuffer
                                     // for standard BMP: a row buffer
        uint16_t* rleSrcBuf = nullptr; // fuer RLEB: komplett dekodiertes Bild
                                       // for RLEB: fully decoded image

        if (isRleFace(magic)) {
            uint8_t rest[16];
            if (bmp.read(rest, 16) != 16) {
                bmp.close();
                DEBUG_PRINTLN("[BMP Scale] Invalid RLEB header");
                return false;
            }
            inW = *(int32_t*)&rest[0];
            inH = *(int32_t*)&rest[4];
            uint32_t compressedSize = *(uint32_t*)&rest[8];
            uint32_t uncompressedSize = *(uint32_t*)&rest[12];

            if (inW <= 0 || inH <= 0 || uncompressedSize != (uint32_t)inW * inH * 2) {
                bmp.close();
                DEBUG_PRINTLN("[BMP Scale] Invalid RLEB dimensions");
                return false;
            }

            uint8_t* compBuf = (uint8_t*)preferPsramMalloc(compressedSize);
            if (!compBuf) {
                bmp.close();
                DEBUG_PRINTLN("[BMP Scale] Memory allocation failed (compBuf)");
                return false;
            }
            if (bmp.read(compBuf, compressedSize) != compressedSize) {
                free(compBuf); bmp.close();
                DEBUG_PRINTLN("[BMP Scale] Failed to read RLEB data");
                return false;
            }
            bmp.close();

            rleSrcBuf = (uint16_t*)preferPsramMalloc(uncompressedSize);
            if (!rleSrcBuf) {
                free(compBuf);
                DEBUG_PRINTLN("[BMP Scale] Memory allocation failed (rleSrcBuf)");
                return false;
            }
            rleDecode565(compBuf, compressedSize, rleSrcBuf, (size_t)inW * inH);
            free(compBuf);

            flip = false; // RLEB ist immer bereits Top-Down gespeichert
                          // RLEB is always already stored top-down
            bpp = 16;
        }
        else {
            bmp.seek(0);
            uint8_t header[54];
            if (bmp.read(header, 54) != 54 || header[0] != 'B' || header[1] != 'M') {
                bmp.close();
                DEBUG_PRINTLN("[BMP Scale] Invalid BMP header");
                return false;
            }

            inW = *(int32_t*)&header[18];
            inH = *(int32_t*)&header[22];
            bpp = *(uint16_t*)&header[28];
            offset = *(uint32_t*)&header[10];

            if (inW <= 0 || abs(inH) <= 0) {
                bmp.close();
                DEBUG_PRINTLN("[BMP Scale] Invalid BMP dimensions");
                return false;
            }

            flip = inH > 0;
            inH = abs(inH);

            inRowSize = ((inW * (bpp / 8) + 3) / 4) * 4;
            rowBuf = (uint8_t*)preferPsramMalloc(inRowSize);
            if (!rowBuf) {
                bmp.close();
                DEBUG_PRINTLN("[BMP Scale] Memory allocation failed");
                return false;
            }
        }

        float scaleX = (float)inW / outW;
        float scaleY = (float)inH / outH;

        uint16_t* outImage = new (std::nothrow) uint16_t[outW * outH];
        if (!outImage) {
            if (rowBuf) { bmp.close(); free(rowBuf); }
            if (rleSrcBuf) free(rleSrcBuf);
            DEBUG_PRINTLN("[BMP Scale] Memory allocation failed (outImage)");
            return false;
        }

        for (int y = 0; y < outH; y++) {
            if (y % 20 == 0) yield(); // Watchdog-Reset vermeiden (Flash-I/O je Zeile kann laenger dauern)
                                      // avoid watchdog reset (flash I/O per row can take longer)
            int srcY = flip ? (inH - 1 - int(y * scaleY)) : int(y * scaleY);

            uint16_t* row16 = nullptr;
            uint8_t* rowSource = nullptr;

            if (rleSrcBuf) {
                row16 = &rleSrcBuf[srcY * inW];
            }
            else {
                bmp.seek(offset + inRowSize * srcY);
                // Rueckgabewert pruefen: sonst enthielte rowBuf bei einer
                // beschaedigten Datei unbemerkt die vorherige Zeile und wuerde
                // trotzdem als vermeintlich gueltig gespeichert.

                // Check the return value: otherwise rowBuf would silently keep
                // the previous row on a corrupted file and still get saved
                // as an apparently valid result.
                if (bmp.read(rowBuf, inRowSize) != inRowSize) {
                    bmp.close();
                    free(rowBuf);
                    delete[] outImage;
                    DEBUG_PRINTLN("[BMP Scale] Read error while scaling");
                    return false;
                }
                rowSource = rowBuf;
            }

            for (int x = 0; x < outW; x++) {
                int srcX = int(x * scaleX);
                uint16_t pixel = 0;

                if (row16 != nullptr) {
                    // Aus bereits dekodiertem RLEB-Quellbild (immer 16 bpp RGB565)
                    // From an already decoded RLEB source image (always 16 bpp RGB565)
                    pixel = row16[srcX];
                }
                else if (bpp == 16) {
                    // 16 bpp (RGB565) → direkt übernehmen
                    // 16 bpp (RGB565) -> use directly
                    uint16_t* r16 = (uint16_t*)rowSource;
                    pixel = r16[srcX];
                }
                else if (bpp == 24) {
                    // 24 bpp (RGB888) → 16 bpp (RGB565)
                    // 24 bpp (RGB888) -> 16 bpp (RGB565)
                    uint8_t* row24 = rowSource + (srcX * 3);
                    uint8_t r = row24[2];
                    uint8_t g = row24[1];
                    uint8_t b = row24[0];
                    pixel = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
                }
                else if (bpp == 32) {
                    // 32 bpp (ARGB8888) → 16 bpp (RGB565)
                    // 32 bpp (ARGB8888) -> 16 bpp (RGB565)
                    uint8_t* row32 = rowSource + (srcX * 4);
                    uint8_t r = row32[2];
                    uint8_t g = row32[1];
                    uint8_t b = row32[0];
                    pixel = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
                }

                outImage[y * outW + x] = pixel;
            }
        }

        if (rowBuf) { bmp.close(); free(rowBuf); }
        if (rleSrcBuf) free(rleSrcBuf);

        // Zielformat entscheiden: face_*.bmp UND hand_set*.bmp werden RLE-
        // komprimiert (spart Flash-Platz, bei Zeigern wegen grosser einfarbiger
        // Flaechen noch mehr) - alles andere bleibt Standard-BMP wie bisher.

        // Decide the target format: face_*.bmp AND hand_set*.bmp are RLE-
        // compressed (saves flash space, even more so for hands due to large
        // solid-color areas) - everything else stays standard BMP as before.
        String targetPathStr = String(targetPath);
        if (!targetPathStr.startsWith("/")) targetPathStr = "/" + targetPathStr;
        bool isFaceTarget = targetPathStr.startsWith("/face_");
        bool isHandTarget = targetPathStr.startsWith("/hand_set");
        bool storeAsRle = isFaceTarget || isHandTarget;

        // Bildpuffer ist quadratisch, runde Displays (GC9A01/GC9D01) zeigen
        // aber nur einen Kreis - alles ausserhalb wird weiss. ILI9341
        // (rechteckig) behaelt die Ecken, Maskierung gilt nur fuer Zifferblaetter.

        // Image buffer is square, but round displays (GC9A01/GC9D01) only show
        // a circle - everything outside is set white. ILI9341 (rectangular)
        // keeps the corners, masking only applies to clock faces.
#ifdef ROUND_DISPLAY
        if (isFaceTarget) {
            float cx = outW / 2.0f;
            float cy = outH / 2.0f;
            float radius = (outW < outH ? outW : outH) / 2.0f;
            float radiusSq = radius * radius;
            for (int y = 0; y < outH; y++) {
                if (y % 20 == 0) yield(); // Watchdog-Reset vermeiden
                                          // avoid watchdog reset
                for (int x = 0; x < outW; x++) {
                    float dx = (x + 0.5f) - cx;
                    float dy = (y + 0.5f) - cy;
                    if (dx * dx + dy * dy > radiusSq) {
                        outImage[y * outW + x] = 0xFFFF; // Weiss (RGB565)
                                                         // white (RGB565)
                    }
                }
            }
        }
#endif

        File out = LittleFS.open(targetPath, "w");
        if (!out) {
            delete[] outImage;
            DEBUG_PRINTLN("[BMP Scale] Failed to open target file");
            return false;
        }

        if (storeAsRle) {
            size_t pixelCount = (size_t)outW * outH;
            size_t maxSize = rleMaxEncodedSize(pixelCount);
            uint8_t* rleBuf = (uint8_t*)preferPsramMalloc(maxSize);
            if (!rleBuf) {
                out.close();
                delete[] outImage;
                DEBUG_PRINTLN("[BMP Scale] Memory allocation failed (rleBuf)");
                return false;
            }
            size_t compressedSize = rleEncode565(outImage, pixelCount, rleBuf);

            uint8_t rleHeader[20];
            rleHeader[0] = 'R'; rleHeader[1] = 'L'; rleHeader[2] = 'E'; rleHeader[3] = 'B';
            *(int32_t*)&rleHeader[4] = outW;
            *(int32_t*)&rleHeader[8] = outH;
            *(uint32_t*)&rleHeader[12] = compressedSize;
            *(uint32_t*)&rleHeader[16] = (uint32_t)(pixelCount * 2);

            out.write(rleHeader, 20);
            out.write(rleBuf, compressedSize);
            free(rleBuf);

            DEBUG_PRINTLN("[BMP Scale] Saved as RLEB (" + String(compressedSize) + " von " + String(pixelCount * 2) + " Byte, -" +
                String(100 - (compressedSize * 100 / (pixelCount * 2))) + "%)");
        }
        else {
            const int rowSize = ((outW * 2 + 3) / 4) * 4;
            const int dataSize = rowSize * outH;
            const int fileSize = 66 + dataSize;
            uint8_t bmpHeader[66] = { 0 };

            bmpHeader[0] = 'B'; bmpHeader[1] = 'M';
            *(uint32_t*)&bmpHeader[2] = fileSize;
            *(uint32_t*)&bmpHeader[10] = 66;
            *(uint32_t*)&bmpHeader[14] = 40;
            *(int32_t*)&bmpHeader[18] = outW;
            *(int32_t*)&bmpHeader[22] = -outH; // Top-down-BMP
                                               // Top-down BMP
            *(uint16_t*)&bmpHeader[26] = 1;

            *(uint16_t*)&bmpHeader[28] = 16; // Auf 16 bpp fuer RGB565 setzen
                                             // Set to 16 bpp for RGB565
            *(uint32_t*)&bmpHeader[30] = 3; // Kompressionsmethode: BI_BITFIELDS
                                            // Compression method: BI_BITFIELDS
            *(uint32_t*)&bmpHeader[34] = dataSize;

            // RGB565-Farbmasken hinzufuegen
            // Add RGB565 color masks
            *(uint32_t*)&bmpHeader[54] = 0xF800; // Rot-Maske
                                                 // Red mask
            *(uint32_t*)&bmpHeader[58] = 0x07E0; // Gruen-Maske
                                                 // Green mask
            *(uint32_t*)&bmpHeader[62] = 0x001F; // Blau-Maske
                                                 // Blue mask

            out.write(bmpHeader, 66);

            for (int y = 0; y < outH; y++) {
                uint8_t rowOut[rowSize];
                memset(rowOut, 0, rowSize);
                memcpy(rowOut, &outImage[y * outW], outW * 2);
                out.write(rowOut, rowSize);
            }
        }

        out.close();
        delete[] outImage;
        return true;
    }


    // Durchsucht das Dateisystem nach face_*.bmp-Dateien im ALTEN Standard-BMP-
    // Format und konvertiert sie einmalig zum RLE-Format (scaleAndSaveBmp()
    // speichert "face_"-Dateien automatisch als RLE - Quelle=Ziel=gleicher Pfad).

    // Scans the filesystem for face_*.bmp files in the OLD standard-BMP
    // format and converts them to the RLE format once (scaleAndSaveBmp()
    // automatically saves "face_" files as RLE - source=target=same path).

    void migrateFaceBmpsToRLE() {
        File root = LittleFS.open("/");
        if (!root) return;

        std::vector<String> toConvert;
        File file = root.openNextFile();
        while (file) {
            if (!file.isDirectory()) {
                String name = file.name();
                String nameOnly = name.startsWith("/") ? name.substring(1) : name;
                if (nameOnly.startsWith("face_") && nameOnly.endsWith(".bmp")) {
                    uint8_t magic[4] = { 0 };
                    file.read(magic, 4);
                    if (!isRleFace(magic)) {
                        toConvert.push_back(name.startsWith("/") ? name : "/" + name);
                    }
                }
            }
            file = root.openNextFile();
        }

        if (toConvert.empty()) {
            DEBUG_PRINTLN("[MIGRATE] No clock faces in the old format found.");
            return;
        }

        DEBUG_PRINTLN("[MIGRATE] " + String(toConvert.size()) + " clock face(s) found in the old format, converting to RLE...");

        for (const String& path : toConvert) {
            File before = LittleFS.open(path, "r");
            size_t sizeBefore = before ? before.size() : 0;
            if (before) before.close();

            if (scaleAndSaveBmp(path.c_str(), path.c_str(), CLOCK_WIDTH, CLOCK_HEIGHT)) {
                File after = LittleFS.open(path, "r");
                size_t sizeAfter = after ? after.size() : 0;
                if (after) after.close();
                DEBUG_PRINTLN("[MIGRATE] OK: " + path + " (" + String(sizeBefore) + " -> " + String(sizeAfter) + " bytes)");
                checkHeapWarning("Migration " + path);
            }
            else {
                DEBUG_PRINTLN("[MIGRATE] ERROR for " + path + " - file remains in the old format");
            }
        }
    }


    // Durchsucht das Dateisystem nach hand_set*.bmp-Dateien im ALTEN Standard-
    // BMP-Format und konvertiert sie einmalig zum RLE-Format (scaleAndSaveBmp()
    // speichert "hand_set"-Dateien seither ebenfalls automatisch als RLE).

    // Scans the filesystem for hand_set*.bmp files in the OLD
    // BMP format and converts them to the RLE format once (scaleAndSaveBmp()
    // has since automatically saved "hand_set" files as RLE too).

    void migrateHandBmpsToRLE() {
        File root = LittleFS.open("/");
        if (!root) return;

        std::vector<String> toConvert;
        File file = root.openNextFile();
        while (file) {
            if (!file.isDirectory()) {
                String name = file.name();
                String nameOnly = name.startsWith("/") ? name.substring(1) : name;
                if (nameOnly.startsWith("hand_set") && nameOnly.endsWith(".bmp")) {
                    uint8_t magic[4] = { 0 };
                    file.read(magic, 4);
                    if (!isRleFace(magic)) {
                        toConvert.push_back(name.startsWith("/") ? name : "/" + name);
                    }
                }
            }
            file = root.openNextFile();
        }

        if (toConvert.empty()) {
            DEBUG_PRINTLN("[MIGRATE] No hand sets in the old format found.");
            return;
        }

        DEBUG_PRINTLN("[MIGRATE] " + String(toConvert.size()) + " hand set file(s) found in the old format, converting to RLE...");

        for (const String& path : toConvert) {
            File before = LittleFS.open(path, "r");
            size_t sizeBefore = before ? before.size() : 0;
            if (before) before.close();

            int targetW, targetH;
            handTargetSize(path.c_str(), targetW, targetH);
            if (scaleAndSaveBmp(path.c_str(), path.c_str(), targetW, targetH)) {
                File after = LittleFS.open(path, "r");
                size_t sizeAfter = after ? after.size() : 0;
                if (after) after.close();
                DEBUG_PRINTLN("[MIGRATE] OK: " + path + " (" + String(sizeBefore) + " -> " + String(sizeAfter) + " bytes)");
                checkHeapWarning("Migration " + path);
            }
            else {
                DEBUG_PRINTLN("[MIGRATE] ERROR for " + path + " - file remains in the old format");
            }
        }
    }


    // Liest nur Pixel (0,0) einer RLEB-Datei, ohne das ganze Bild zu dekodieren -
    // preiswerte Pruefung, ob die Kreismaskierung fuer runde Displays bereits
    // angewendet wurde (Pixel (0,0) liegt garantiert ausserhalb des Kreises).

    // Reads only pixel (0,0) of an RLEB file without decoding the whole image -
    // a cheap check for whether the circular masking for round displays has
    // already been applied (pixel (0,0) is guaranteed to lie outside the circle).

    bool peekFirstPixelIsWhite(const String& path) {
        File f = LittleFS.open(path, "r");
        if (!f) return false;
        uint8_t magic[4];
        if (f.read(magic, 4) != 4 || !isRleFace(magic)) { f.close(); return false; }
        uint8_t rest[16];
        if (f.read(rest, 16) != 16) { f.close(); return false; }
        uint8_t ctrl;
        if (f.read(&ctrl, 1) != 1) { f.close(); return false; }
        uint8_t b0, b1;
        bool ok = (f.read(&b0, 1) == 1) && (f.read(&b1, 1) == 1);
        f.close();
        if (!ok) return false;
        uint16_t px = b0 | (b1 << 8);
        return px == 0xFFFF;
    }


    // Wendet die Kreismaskierung einmalig auf RLE-Zifferblaetter an, die VOR
    // ihrer Einfuehrung migriert/hochgeladen wurden (nur runde Displays,
    // peekFirstPixelIsWhite() ueberspringt bereits maskierte).

    // Applies the circular mask once to RLE clock faces migrated/uploaded
    // BEFORE the mask was introduced (round displays only,
    // peekFirstPixelIsWhite() skips ones already masked).

    void remaskExistingFaceCorners() {
#ifndef ROUND_DISPLAY
        return; // Rechteckiges Display (z.B. ILI9341) - keine Kreismaskierung noetig
                // rectangular display (e.g. ILI9341) - no circular masking needed
#endif
        File root = LittleFS.open("/");
        if (!root) return;

        std::vector<String> toRemask;
        File file = root.openNextFile();
        while (file) {
            if (!file.isDirectory()) {
                String name = file.name();
                String nameOnly = name.startsWith("/") ? name.substring(1) : name;
                if (nameOnly.startsWith("face_") && nameOnly.endsWith(".bmp")) {
                    uint8_t magic[4] = { 0 };
                    file.read(magic, 4);
                    if (isRleFace(magic)) {
                        String path = name.startsWith("/") ? name : "/" + name;
                        if (!peekFirstPixelIsWhite(path)) {
                            toRemask.push_back(path);
                        }
                    }
                }
            }
            file = root.openNextFile();
        }

        if (toRemask.empty()) {
            DEBUG_PRINTLN("[REMASK] No clock faces need corner masking.");
            return;
        }

        DEBUG_PRINTLN("[REMASK] " + String(toRemask.size()) + " clock face(s) need corner masking, processing...");

        for (const String& path : toRemask) {
            if (scaleAndSaveBmp(path.c_str(), path.c_str(), CLOCK_WIDTH, CLOCK_HEIGHT)) {
                DEBUG_PRINTLN("[REMASK] OK: " + path);
                checkHeapWarning("Remask " + path);
            }
            else {
                DEBUG_PRINTLN("[REMASK] ERROR for " + path);
            }
        }
    }


    // Liest eine BMP-Datei (16 bpp RGB565), skaliert sie in-memory auf outW x outH
    // herunter und sendet sie DIREKT als HTTP-Antwort (keine Flash-Kopie) - schnelle
    // <img>-Vorschau statt der vollen Aufloesung (z.B. 240x240=~115 KB) je Seitenaufruf.

    // Reads a BMP file (16 bpp RGB565), downscales it in memory to outW x outH
    // and sends it DIRECTLY as an HTTP response (no flash copy) - fast
    // <img> preview instead of the full resolution (e.g. 240x240=~115 KB) per page load.

    void sendScaledBmpPreview(const String& sourcePath, int outW, int outH) {
        checkHeapWarning("sendScaledBmpPreview Start (" + sourcePath + ")");

        File f = LittleFS.open(sourcePath, "r");
        if (!f) {
            webserver.send(404, "text/plain", "File not found");
            return;
        }

        uint8_t magic[4];
        if (f.read(magic, 4) != 4) {
            f.close();
            webserver.send(404, "text/plain", "File not found or invalid format");
            return;
        }

        bool isRle = isRleFace(magic);
        int32_t inW = 0, inH = 0;
        uint32_t compressedSize = 0;
        uint32_t offset = 0;      // nur fuer Standard-BMP
                                  // only for standard BMP
        int inRowSizeStd = 0;     // nur fuer Standard-BMP
                                  // only for standard BMP
        bool flipStd = false;     // nur fuer Standard-BMP
                                  // only for standard BMP

        if (isRle) {
            uint8_t rest[16];
            if (f.read(rest, 16) != 16) {
                f.close();
                webserver.send(500, "text/plain", "Invalid RLEB header");
                return;
            }
            inW = *(int32_t*)&rest[0];
            inH = *(int32_t*)&rest[4];
            compressedSize = *(uint32_t*)&rest[8];
            uint32_t uncompressedSize = *(uint32_t*)&rest[12];
            if (inW <= 0 || inH <= 0 || uncompressedSize != (uint32_t)inW * inH * 2) {
                f.close();
                webserver.send(500, "text/plain", "Invalid RLEB dimensions");
                return;
            }
        }
        else {
            f.seek(0);
            uint8_t header[54];
            if (f.read(header, 54) != 54 || header[0] != 'B' || header[1] != 'M') {
                f.close();
                webserver.send(404, "text/plain", "File not found or invalid format");
                return;
            }
            inW = *(int32_t*)&header[18];
            inH = *(int32_t*)&header[22];
            uint16_t bpp = *(uint16_t*)&header[28];
            offset = *(uint32_t*)&header[10];
            if (inW <= 0 || abs(inH) <= 0 || bpp != 16) {
                f.close();
                webserver.send(500, "text/plain", "Unsupported BMP (nur 16 bpp)");
                return;
            }
            flipStd = inH > 0;
            inH = abs(inH);
            inRowSizeStd = ((inW * 2 + 3) / 4) * 4;
        }

        float scaleX = (float)inW / outW;
        float scaleY = (float)inH / outH;

        const int outRowSize = ((outW * 2 + 3) / 4) * 4;
        const int outDataSize = outRowSize * outH;
        const int outFileSize = 66 + outDataSize; // 66 = 14 (Datei-Header) + 40 (DIB-Header) + 12 (RGB565-Farbmasken)
                                                  // 66 = 14 (file header) + 40 (DIB header) + 12 (RGB565 color masks)

        uint8_t* outBmp = new (std::nothrow) uint8_t[outFileSize];
        if (!outBmp) {
            f.close();
            webserver.send(500, "text/plain", "Memory allocation failed");
            return;
        }
        memset(outBmp, 0, outFileSize);

        outBmp[0] = 'B'; outBmp[1] = 'M';
        *(uint32_t*)&outBmp[2] = outFileSize;
        *(uint32_t*)&outBmp[10] = 66;
        *(uint32_t*)&outBmp[14] = 40;
        *(int32_t*)&outBmp[18] = outW;
        *(int32_t*)&outBmp[22] = -outH; // Top-down-BMP
                                        // Top-down BMP
        *(uint16_t*)&outBmp[26] = 1;
        *(uint16_t*)&outBmp[28] = 16; // 16 bpp fuer RGB565
                                      // 16 bpp fuer RGB565
        *(uint32_t*)&outBmp[30] = 3; // Kompressionsmethode: BI_BITFIELDS
                                     // Kompressionsmethode: BI_BITFIELDS
        *(uint32_t*)&outBmp[34] = outDataSize;

        // RGB565-Farbmasken ergaenzen (ohne diese interpretieren Browser 16-bpp-BMPs
        // standardmaessig als RGB555 statt RGB565 -> sichtbare Falschfarben)

        // Add RGB565 color masks (without these, browsers interpret 16-bpp BMPs
        // by default as RGB555 instead of RGB565 -> visible false colors)
        *(uint32_t*)&outBmp[54] = 0xF800; // Rot-Maske
                                          // red mask
        *(uint32_t*)&outBmp[58] = 0x07E0; // Gruen-Maske
                                          // green mask
        *(uint32_t*)&outBmp[62] = 0x001F; // Blau-Maske
                                          // blue mask

        if (isRle) {
            // RLEB: sequentiell dekodieren, nur die fuer das Downsampling
            // benoetigten Zeilen behalten - kein voller ~115-KB-Puffer noetig
            // (RLE erlaubt kein direktes Anspringen einzelner Zeilen).

            // RLEB: decode sequentially, keep only the rows needed for
            // downsampling - no full ~115 KB buffer needed
            // (RLE doesn't allow jumping directly to individual rows).
            const size_t IN_CHUNK = 512;
            uint8_t inBuf[IN_CHUNK];
            size_t inPos = 0, inLen = 0, consumedTotal = 0;

            auto readByte = [&](uint8_t& out) -> bool {
                if (consumedTotal >= compressedSize) return false;
                if (inPos >= inLen) {
                    size_t remaining = compressedSize - consumedTotal;
                    size_t toRead = remaining < IN_CHUNK ? remaining : IN_CHUNK;
                    inLen = f.read(inBuf, toRead);
                    inPos = 0;
                    if (inLen == 0) return false;
                }
                out = inBuf[inPos++];
                consumedTotal++;
                return true;
                };

            uint16_t* srcRow = new (std::nothrow) uint16_t[inW];
            if (!srcRow) {
                // Null-Check ergaenzt: die Nachbarallokation (outBmp weiter oben)
                // wird geprueft, diese nicht - bei knappem Heap wurde direkt
                // danach hineingeschrieben.

                // Null check added: the neighbouring allocation (outBmp further
                // above) is checked, this one was not - with a tight heap it was
                // written to right afterwards.
                DEBUG_PRINTLN("[Preview] Error: couldnt allocate srcRow buffer");
                delete[] outBmp;
                f.close();
                webserver.send(500, "text/plain", "Memory allocation failed");
                return;
            }
            int srcCol = 0, srcRowIdx = 0;
            int nextOutRow = 0;
            int nextNeededSrcRow = int(nextOutRow * scaleY);
            size_t written = 0;
            const size_t total = (size_t)inW * inH;
            bool ok = true;

            while (written < total && nextOutRow < outH && ok) {
                uint8_t ctrl;
                if (!readByte(ctrl)) { ok = false; break; }

                bool literal = ctrl <= 127;
                size_t len;
                uint16_t litPx = 0;

                if (literal) {
                    len = ctrl + 1;
                }
                else {
                    len = 257 - ctrl;
                    uint8_t b0, b1;
                    if (!readByte(b0) || !readByte(b1)) { ok = false; break; }
                    litPx = b0 | (b1 << 8);
                }

                for (size_t k = 0; k < len && written < total; k++) {
                    uint16_t px;
                    if (literal) {
                        uint8_t b0, b1;
                        if (!readByte(b0) || !readByte(b1)) { ok = false; break; }
                        px = b0 | (b1 << 8);
                    }
                    else {
                        px = litPx;
                    }

                    if (srcRowIdx == nextNeededSrcRow) {
                        srcRow[srcCol] = px;
                    }
                    srcCol++;
                    written++;

                    if (srcCol >= inW) {
                        if (srcRowIdx == nextNeededSrcRow) {
                            uint8_t* outRow = outBmp + 66 + nextOutRow * outRowSize;
                            for (int x = 0; x < outW; x++) {
                                int sx = int(x * scaleX);
                                uint16_t p = srcRow[sx];
                                outRow[x * 2] = p & 0xFF;
                                outRow[x * 2 + 1] = p >> 8;
                            }
                            nextOutRow++;
                            nextNeededSrcRow = int(nextOutRow * scaleY);
                        }
                        srcCol = 0;
                        srcRowIdx++;
                    }
                }
            }

            delete[] srcRow;
        }
        else {
            // Standard-BMP: direktes Anspringen der benoetigten Zeilen per
            // Datei-Seek, wie zuvor - hier war die Speichereffizienz schon
            // immer gegeben (kein Vollpuffer noetig).

            // Standard BMP: jump directly to the needed rows via
            // file seek, as before - memory efficiency was already
            // a given here (no full buffer needed).
            uint8_t* rowBuf = (uint8_t*)preferPsramMalloc(inRowSizeStd);
            if (rowBuf) {
                for (int y = 0; y < outH; y++) {
                    int srcY = flipStd ? (inH - 1 - int(y * scaleY)) : int(y * scaleY);
                    f.seek(offset + (uint32_t)inRowSizeStd * srcY);
                    f.read(rowBuf, inRowSizeStd);
                    uint16_t* row16 = (uint16_t*)rowBuf;
                    uint8_t* outRow = outBmp + 66 + y * outRowSize;
                    for (int x = 0; x < outW; x++) {
                        int srcX = int(x * scaleX);
                        uint16_t px = row16[srcX];
                        outRow[x * 2] = px & 0xFF;
                        outRow[x * 2 + 1] = px >> 8;
                    }
                }
                free(rowBuf);
            }
        }

        f.close();

        webserver.send_P(200, "image/bmp", (const char*)outBmp, outFileSize);
        delete[] outBmp;
    }


    // Liest eine RLEB-komprimierte face_*.bmp-Datei zeilenweise und sendet das
    // Ergebnis SOFORT per Chunked-Response, statt es komplett im RAM zu
    // materialisieren - haelt nie mehr als eine Bildzeile im RAM (statt ~115 KB).

    // Reads an RLEB-compressed face_*.bmp file row by row and sends the
    // result IMMEDIATELY via chunked response, instead of materializing
    // it fully in RAM - never holds more than one image row in RAM (instead of ~115 KB).

    bool streamRleFaceAsStandardBmp(const String& path, const char* contentType) {
        File f = LittleFS.open(path, "r");
        if (!f) return false;

        uint8_t magic[4];
        if (f.read(magic, 4) != 4 || !isRleFace(magic)) {
            f.close();
            return false;
        }

        uint8_t rest[16];
        if (f.read(rest, 16) != 16) { f.close(); return false; }
        int32_t w = *(int32_t*)&rest[0];
        int32_t h = *(int32_t*)&rest[4];
        uint32_t compressedSize = *(uint32_t*)&rest[8];
        uint32_t uncompressedSize = *(uint32_t*)&rest[12];

        if (w <= 0 || h <= 0 || uncompressedSize != (uint32_t)w * h * 2) {
            f.close();
            return false;
        }

        const int rowSize = ((w * 2 + 3) / 4) * 4;
        const int dataSize = rowSize * h;
        const int fileSize = 66 + dataSize;

        uint8_t bmpHeader[66] = { 0 };
        bmpHeader[0] = 'B'; bmpHeader[1] = 'M';
        *(uint32_t*)&bmpHeader[2] = fileSize;
        *(uint32_t*)&bmpHeader[10] = 66;
        *(uint32_t*)&bmpHeader[14] = 40;
        *(int32_t*)&bmpHeader[18] = w;
        *(int32_t*)&bmpHeader[22] = -h; // Top-down-BMP
                                        // Top-down BMP
        *(uint16_t*)&bmpHeader[26] = 1;
        *(uint16_t*)&bmpHeader[28] = 16;
        *(uint32_t*)&bmpHeader[30] = 3; // BI_BITFIELDS
                                        // BI_BITFIELDS
        *(uint32_t*)&bmpHeader[34] = dataSize;
        *(uint32_t*)&bmpHeader[54] = 0xF800;
        *(uint32_t*)&bmpHeader[58] = 0x07E0;
        *(uint32_t*)&bmpHeader[62] = 0x001F;

        // Kleine Bilder (z.B. Zeiger) komplett dekodieren und in EINEM Rutsch senden -
        // bei kleinen Dateien ueberwiegt sonst der Netzwerk-Overhead vieler einzelner
        // sendContent()-Aufrufe. Grosse Zifferblaetter bleiben zeilenweise gestreamt.

        // Fully decode small images (e.g. hands) and send them in ONE go -
        // for small files the network overhead of many individual
        // sendContent() calls would otherwise dominate. Large clock faces stay streamed row by row.
        const uint32_t SMALL_IMAGE_THRESHOLD = 20000;
        if (uncompressedSize <= SMALL_IMAGE_THRESHOLD) {
            uint8_t* compBuf = (uint8_t*)preferPsramMalloc(compressedSize);
            if (!compBuf) { f.close(); return false; }
            if (f.read(compBuf, compressedSize) != compressedSize) {
                free(compBuf); f.close(); return false;
            }
            f.close();

            uint8_t* fullBmp = new (std::nothrow) uint8_t[fileSize];
            if (!fullBmp) { free(compBuf); return false; }
            memcpy(fullBmp, bmpHeader, 66);
            rleDecode565ToBmpRows(compBuf, compressedSize, fullBmp + 66, w, h, rowSize);
            free(compBuf);

            webserver.send_P(200, contentType, (const char*)fullBmp, fileSize);
            delete[] fullBmp;
            return true;
        }

        webserver.setContentLength(CONTENT_LENGTH_UNKNOWN);
        webserver.send(200, contentType, "");
        webserver.sendContent_P((const char*)bmpHeader, 66);

        // Kleiner Lese-Puffer fuer die komprimierten Eingabedaten (aus der
        // Datei nachgefuellt, statt sie komplett vorab einzulesen).

        // Small read buffer for the compressed input data (refilled
        // from the file instead of reading it all in advance).
        const size_t IN_CHUNK = 512;
        uint8_t inBuf[IN_CHUNK];
        size_t inPos = 0, inLen = 0, consumedTotal = 0;

        auto readByte = [&](uint8_t& out) -> bool {
            if (consumedTotal >= compressedSize) return false;
            if (inPos >= inLen) {
                size_t remaining = compressedSize - consumedTotal;
                size_t toRead = remaining < IN_CHUNK ? remaining : IN_CHUNK;
                inLen = f.read(inBuf, toRead);
                inPos = 0;
                if (inLen == 0) return false;
            }
            out = inBuf[inPos++];
            consumedTotal++;
            return true;
            };

        // Antwort laeuft schon chunked, ein 500er ist also nicht mehr moeglich -
        // stattdessen Uebertragung sauber beenden und false zurueckgeben.

        // The response is already streaming chunked, so a 500 is no longer
        // possible - instead terminate the transfer cleanly and return false.
        uint8_t* rowBuf = new (std::nothrow) uint8_t[rowSize];
        if (!rowBuf) {
            DEBUG_PRINTLN("[BMP] Error: couldnt allocate row buffer for RLE streaming");
            f.close();
            webserver.sendContent("");
            return false;
        }
        memset(rowBuf, 0, rowSize);
        int col = 0, row = 0;
        size_t written = 0;
        const size_t total = (size_t)w * h;
        bool ok = true;

        while (written < total && row < h && ok) {
            uint8_t ctrl;
            if (!readByte(ctrl)) { ok = false; break; }

            bool literal = ctrl <= 127;
            size_t len;
            uint16_t litPx = 0;

            if (literal) {
                len = ctrl + 1;
            }
            else {
                len = 257 - ctrl;
                uint8_t b0, b1;
                if (!readByte(b0) || !readByte(b1)) { ok = false; break; }
                litPx = b0 | (b1 << 8);
            }

            for (size_t k = 0; k < len && written < total; k++) {
                uint16_t px;
                if (literal) {
                    uint8_t b0, b1;
                    if (!readByte(b0) || !readByte(b1)) { ok = false; break; }
                    px = b0 | (b1 << 8);
                }
                else {
                    px = litPx;
                }

                rowBuf[col * 2] = px & 0xFF;
                rowBuf[col * 2 + 1] = px >> 8;
                col++;
                written++;

                if (col >= w) {
                    webserver.sendContent_P((const char*)rowBuf, rowSize);
                    memset(rowBuf, 0, rowSize);
                    col = 0;
                    row++;
                }
            }
        }

        // Falls die letzte Zeile nicht vollstaendig gefuellt wurde (bei
        // gueltigen Dateien sollte das nicht vorkommen), trotzdem senden,
        // damit die Gesamtlaenge zur angekuendigten Content-Length passt.

        // If the last row wasn't fully filled (shouldn't happen
        // with valid files), send it anyway,
        // so the total length matches the announced content length.
        if (col > 0 && row < h) {
            webserver.sendContent_P((const char*)rowBuf, rowSize);
            row++;
        }

        delete[] rowBuf;
        f.close();
        webserver.sendContent(""); // Ende der Chunked-Uebertragung signalisieren
                                   // signal the end of the chunked transfer

        return ok && written >= total;
    }


    // Erzeugt ein Vorschaubild fuer die Preset-Verwaltung: Komposition aus Zifferblatt,
    // Zeigern (Demo-Zeit 10:10:30) und Mittelpunkt in angegebener Farbe/Groesse.
    // Liefert ein Standard-BMP im RAM zurueck (Aufrufer muss outBytes freigeben).

    // Generates a preview image for preset management: composed of clock face,
    // hands (demo time 10:10:30), and center hub in the given color/size.
    // Returns a standard BMP in RAM (caller must free outBytes).
    bool generatePresetPreviewBmp(const String& faceFile, const String& handSetName,
        uint16_t hubColorRgb565, uint8_t hubSize, bool showSecond,
        uint8_t** outBytes, size_t& outSize) {

        checkHeapWarning("generatePresetPreviewBmp Start (" + faceFile + ")");

        // Vorschau als LovyanGFX-Sprite: Zeiger werden wie auf der Uhr mit
        // pushRotateZoomWithAA() gedreht und kantengeglaettet - gleicher Drehpunkt
        // (Sprite-Mitte bzw. HAND_WIDTH/2, HAND_PIVOT_Y), nur verkleinert.

        // Preview as a LovyanGFX sprite: hands are rotated and anti-aliased
        // with pushRotateZoomWithAA() like on the clock - same pivot (sprite
        // centre resp. HAND_WIDTH/2, HAND_PIVOT_Y), just scaled down.
        const int PREVIEW_SIZE = 100;
        LGFX_Sprite canvas(&tft);
        if (!createSprite16(canvas, PREVIEW_SIZE, PREVIEW_SIZE)) return false;

        // 1) Zifferblatt laden und auf die Vorschaugroesse herunterskalieren
        // (Datei, sonst bzw. bei Lesefehler das entpackte eingebaute Standard-
        // Zifferblatt - gleiches Prinzip wie bei /preview_defaultface bzw.
        // sendScaledBmpPreview()).

        // 1) Load the clock face and downscale it to preview size (file,
        // otherwise resp. on a read error the unpacked built-in default face -
        // same approach as /preview_defaultface resp. sendScaledBmpPreview()).
        float faceScaleX = (float)CLOCK_WIDTH / PREVIEW_SIZE;
        float faceScaleY = (float)CLOCK_HEIGHT / PREVIEW_SIZE;
        bool isDefaultFace = (faceFile == "/face_default.bmp") || !LittleFS.exists(faceFile);

        uint16_t* faceBuf = (uint16_t*)preferPsramMalloc((size_t)CLOCK_WIDTH * CLOCK_HEIGHT * 2);
        if (!faceBuf) return false;
        if (isDefaultFace || !loadFaceBmpInto(faceFile, faceBuf, CLOCK_WIDTH, CLOCK_HEIGHT)) {
            decodeDefaultFace(faceBuf);
        }
        for (int y = 0; y < PREVIEW_SIZE; y++) {
            int sy = (int)(y * faceScaleY);
            for (int x = 0; x < PREVIEW_SIZE; x++) {
                int sx = (int)(x * faceScaleX);
                rowBuffer[x] = faceBuf[sy * CLOCK_WIDTH + sx];
            }
            canvas.pushImage(0, y, PREVIEW_SIZE, 1, rowBuffer);
        }
        free(faceBuf);

        // 2) Zeiger laden (aus Datei, falls Set vorhanden, sonst eingebauter Standard)
        // 2) Load hands (from file if a set exists, otherwise built-in default)
        bool useCustomSet = (handSetName != "default" && handSetName != "");
        auto loadPreviewHand = [&](const char* label, const uint16_t* fallback) -> uint16_t* {
            uint16_t* buf = (uint16_t*)preferPsramMalloc((size_t)HAND_WIDTH * HAND_HEIGHT * 2);
            if (!buf) return nullptr;
            if (!useCustomSet || !loadHandPixels("/hand_set" + handSetName + "_" + label + ".bmp", buf)) {
                copyLegacyHand(fallback, buf);
            }
            return buf;
        };
        uint16_t* hourPix = loadPreviewHand("hour", handHour);
        uint16_t* minutePix = loadPreviewHand("minute", handMinute);
        uint16_t* secondPix = showSecond ? loadPreviewHand("second", handSecond) : nullptr;

        // 3) Demo-Zeit 10:10:30 - klassischer Uhrenwerbung-Winkel
        // 3) Demo time 10:10:30 - the classic clock-advertisement angle
        const float hourAngle = (10 % 12) * 30.0f + (10 / 2.0f) + (30 / 120.0f);
        const float minuteAngle = 10 * 6.0f + (30 / 10.0f);
        const float secondAngle = 30 * 6.0f;

        float handScale = (float)PREVIEW_SIZE / CLOCK_WIDTH; // Zeiger im gleichen Massstab wie das Zifferblatt
                                                             // hands at the same scale as the clock face

        // Ein Zeiger-Sprite fuer alle drei nacheinander. Weiss gilt wie in
        // loadHandSprites() als transparent (alte Zeigerdateien).
        // One hand sprite for all three in turn. White counts as transparent,
        // as in loadHandSprites() (old hand files).
        LGFX_Sprite handSprite(&tft);
        bool handSpriteOk = createSprite16(handSprite, HAND_WIDTH, HAND_HEIGHT);
        if (handSpriteOk) handSprite.setPivot(HAND_WIDTH / 2, HAND_PIVOT_Y);

        const struct { uint16_t* pix; float angle; } previewHands[] = {
            { hourPix, hourAngle }, { minutePix, minuteAngle }, { secondPix, secondAngle }
        };
        for (const auto& h : previewHands) {
            if (!h.pix) continue;
            if (handSpriteOk) {
                for (int i = 0; i < HAND_WIDTH * HAND_HEIGHT; i++) {
                    if (h.pix[i] == 0xFFFF) h.pix[i] = TRANSPARENT_COLOR;
                }
                handSprite.pushImage(0, 0, HAND_WIDTH, HAND_HEIGHT, h.pix);
                handSprite.pushRotateZoomWithAA(&canvas, h.angle, handScale, handScale, TRANSPARENT_COLOR);
            }
            free(h.pix);
        }

        // 4) Mittelpunkt (Hub) in der angegebenen Farbe/Groesse - wie auf der
        // Uhr kantengeglaettet um die Sprite-Mitte (fillSmoothCircle())
        // 4) Center hub in the given color/size - anti-aliased around the
        // sprite centre as on the clock (fillSmoothCircle())
        int hubRadius = (int)roundf(hubSize * handScale);
        if (hubRadius < 1) hubRadius = 1;
        canvas.fillSmoothCircle(PREVIEW_SIZE / 2, PREVIEW_SIZE / 2, hubRadius, hubColorRgb565);

        // 5) Als Standard-BMP (mit BI_BITFIELDS-Header) verpacken
        // 5) Package as standard BMP (with BI_BITFIELDS header)
        const int rowSize = ((PREVIEW_SIZE * 2 + 3) / 4) * 4;
        const int dataSize = rowSize * PREVIEW_SIZE;
        const int fileSize = 66 + dataSize;

        uint8_t* bmpData = new (std::nothrow) uint8_t[fileSize];
        if (!bmpData) return false;
        memset(bmpData, 0, fileSize);

        bmpData[0] = 'B'; bmpData[1] = 'M';
        *(uint32_t*)&bmpData[2] = fileSize;
        *(uint32_t*)&bmpData[10] = 66;
        *(uint32_t*)&bmpData[14] = 40;
        *(int32_t*)&bmpData[18] = PREVIEW_SIZE;
        *(int32_t*)&bmpData[22] = -PREVIEW_SIZE; // Top-down-BMP
                                                 // Top-down BMP
        *(uint16_t*)&bmpData[26] = 1;
        *(uint16_t*)&bmpData[28] = 16;
        *(uint32_t*)&bmpData[30] = 3; // BI_BITFIELDS
                                      // BI_BITFIELDS
        *(uint32_t*)&bmpData[34] = dataSize;
        *(uint32_t*)&bmpData[54] = 0xF800;
        *(uint32_t*)&bmpData[58] = 0x07E0;
        *(uint32_t*)&bmpData[62] = 0x001F;

        // readPixel() liefert RGB565 in RAM-Reihenfolge - das Sprite selbst
        // speichert in Display-Reihenfolge, daher nicht direkt kopieren.
        // readPixel() returns RGB565 in RAM byte order - the sprite itself
        // stores display byte order, so don't copy it directly.
        for (int y = 0; y < PREVIEW_SIZE; y++) {
            uint16_t* row = (uint16_t*)(bmpData + 66 + y * rowSize);
            for (int x = 0; x < PREVIEW_SIZE; x++) row[x] = canvas.readPixel(x, y);
        }

        *outBytes = bmpData;
        outSize = (size_t)fileSize;
        return true;
    }


    // Schaltet die LED ein (wenn definiert)
    // Turns the LED on (if defined)

    void setLedOff() {
#ifdef LED_BOARD
        pinMode(LED_BOARD, OUTPUT);
        digitalWrite(LED_BOARD, LOW);
#endif
    }


    // Schaltet die LED aus (wenn definiert)
    // Turns the LED off (if defined)

    void setLedOn() {
#ifdef LED_BOARD
        pinMode(LED_BOARD, OUTPUT);
        digitalWrite(LED_BOARD, HIGH);
#endif
    }


    // LED toggeln
    // Toggles the LED

    void toggleLED() {
#ifdef LED_BOARD
        static bool toggle = true;
        if (toggle) {
            setLedOn();        
        }
        else {
            setLedOff();
        }       
        toggle = !toggle;
#endif
    }


    // Touch pruefen (nicht-blockierend, mit Entprellung)
    // Check touch (non-blocking, with debouncing)

    void checkTouchInput() {
#ifdef TOUCH_PIN

        uint16_t var = touchRead(TOUCH_PIN);

        bool state = false;

        if (var > 15000 && var < 65535) state = true;

        // DEBUG_PRINTLN("Touch read: " + String(var));
        //DEBUG_PRINTLN("Touch state: " + String(state));

        // Flanke LOW->HIGH (kurzer Tip) mit Debounce
        // LOW->HIGH edge (short tap) with debounce
        if (state && !touchLastState && (millis() - touchLastMillis) > TOUCH_DEBOUNCE_MS) {
            touchLastMillis = millis();
            DEBUG_PRINTLN("switch");
            switchToNextPreset();
        }
        touchLastState = state;
#endif
    }


    // Validiert den geladenen Preferences-Eintrag für background und repariert falls nötig
    // Validates the loaded preferences entry for background and repairs it if needed

    static void validateSelectedBackground() {
        // Normalisieren
        // Normalize
        selectedBackground.trim();
        if (selectedBackground.length() == 0) selectedBackground = "/face_default.bmp";
        if (!selectedBackground.startsWith("/")) selectedBackground = "/" + selectedBackground;

        DEBUG_PRINTLN("[BG] Pref load: '" + selectedBackground + "'");

        // LittleFS muss gemountet sein
        // LittleFS must be mounted
        if (!LittleFS.exists(selectedBackground)) {
            DEBUG_PRINTLN("[BG] File not found: " + selectedBackground);
            // Versuche tolerant auch ohne führenden Slash (falls gespeichert ohne '/')
            // Also try tolerantly without a leading slash (if saved without '/')
            String withoutSlash = selectedBackground;
            if (withoutSlash.startsWith("/")) withoutSlash = withoutSlash.substring(1);
            if (LittleFS.exists("/" + withoutSlash)) {
                selectedBackground = "/" + withoutSlash;
                DEBUG_PRINTLN("[BG] Found (alt) file: " + selectedBackground);
            }
            else {
                // Fallback auf Default
                // Fallback to default
                selectedBackground = "/face_default.bmp";
                preferences.putString(PK_BACKGROUND, selectedBackground);
                DEBUG_PRINTLN("[BG] Falling back to default and saved: " + selectedBackground);
                return;
            }
        }

        // Prüfe BMP-Format (Größe / bpp)
        // Check BMP format (size / bpp)
        if (!checkBmpFormat(selectedBackground)) {
            DEBUG_PRINTLN("[BG] BMP format invalid: " + selectedBackground);
            selectedBackground = "/face_default.bmp";
            preferences.putString(PK_BACKGROUND, selectedBackground);
            DEBUG_PRINTLN("[BG] Falling back to default and saved: " + selectedBackground);
            return;
        }

        DEBUG_PRINTLN("[BG] Background OK: " + selectedBackground);
    }


    // Aktualisiert die Zeigerbreiten und lädt die Zeiger-Sprites neu
    // Updates the hand widths and reloads the hand sprites

    void updateHandWidths(int newHourWidth, int newMinuteWidth, int newSecondWidth) {
        // loadClockFace() ruft dies bei JEDEM Tick auf. Ohne diese Abkuerzung
        // wuerden alle drei Zeiger-Sprites pro Frame neu allokiert (Heap-
        // Fragmentierung, NVS+3x LittleFS) - jetzt nur bei echter Breitenaenderung.
        // Vergleich gegen die TATSAECHLICHE Sprite-Breite, nicht gegen die
        // globalen hourHandWidth/...: die werden vom Aufrufer per Referenz VORHER
        // gesetzt und waeren daher immer gleich - ein Breitenwechsel bliebe unerkannt.

        // loadClockFace() calls this on EVERY tick. Without this shortcut all
        // three hand sprites would be reallocated per frame (heap fragmentation,
        // NVS+3x LittleFS) - now only on an actual width change.
        // Comparison against the sprites' ACTUAL width, not the global
        // hourHandWidth/...: the caller sets those by reference just before, so
        // they'd always match - a width change would go undetected.

        static bool handSpritesCreated = false;

        if (handSpritesCreated &&
            hourHandSprite.width() == newHourWidth &&
            minuteHandSprite.width() == newMinuteWidth &&
            secondHandSprite.width() == newSecondWidth) {
            return;
        }

        // Aktualisiere die globalen Breiten
        // Update the global widths
        hourHandWidth = newHourWidth;
        minuteHandWidth = newMinuteWidth;
        secondHandWidth = newSecondWidth;

        // Alte Sprites löschen
        // Delete old sprites
        hourHandSprite.deleteSprite();
        minuteHandSprite.deleteSprite();
        secondHandSprite.deleteSprite();

        // Rueckgabewerte pruefen: bei fehlgeschlagener Allokation blieben Sprites
        // sonst still ungueltig (Zeiger verschwindet ohne Absturz); bei Fehlschlag
        // bleibt handSpritesCreated false fuer einen Retry beim naechsten Aufruf.

        // Check return values: on a failed allocation, sprites would otherwise
        // silently stay invalid (the hand disappears without a crash); on
        // failure handSpritesCreated stays false to retry on the next call.
        bool allCreated = true;

        allCreated &= createSprite16(hourHandSprite, hourHandWidth, HAND_HEIGHT);
        hourHandSprite.setPivot(hourHandWidth / 2, HAND_PIVOT_Y);

        allCreated &= createSprite16(minuteHandSprite, minuteHandWidth, HAND_HEIGHT);
        minuteHandSprite.setPivot(minuteHandWidth / 2, HAND_PIVOT_Y);

        allCreated &= createSprite16(secondHandSprite, secondHandWidth, HAND_HEIGHT);
        secondHandSprite.setPivot(secondHandWidth / 2, HAND_PIVOT_Y);

        handSpritesCreated = allCreated;
        if (!allCreated) {
            DEBUG_PRINTLN("[Display] Error: couldnt allocate hand sprites - will retry on next clock face load");
        }

        // Zeiger neu laden
        // Reload hands
        loadHandSprites();
    }


    // Parst die Zeigerbreiten aus dem Dateinamen des Hintergrundbildes (test)
    // Parses the hand widths from the background image filename (test)

    void parseBackgroundFilename(const String& filename, int& hourWidth, int& minuteWidth, int& secondWidth) {
        // Standardwerte setzen
        // Set default values
        hourWidth = HAND_WIDTH;
        minuteWidth = HAND_WIDTH;
        secondWidth = HAND_WIDTH;

        // Suche nach dem ersten `!`
        // Search for the first `!`
        int firstHash = filename.indexOf('!');
        if (firstHash == -1) {
            // Kein `!` gefunden, Standardwerte verwenden
            // No `!` found, use default values
            return;
        }

        // Schneide den relevanten Teil nach dem ersten `#` ab
        // Cut off the relevant part after the first `#`
        String params = filename.substring(firstHash + 1);

        // Teile die Parameter anhand von `!`
        // Split the parameters by `!`
        int secondHash = params.indexOf('!');
        int thirdHash = params.indexOf('!', secondHash + 1);

        if (secondHash != -1 && thirdHash != -1) {
            // Extrahiere die Werte
            // Extract the values
            hourWidth = params.substring(0, secondHash).toInt();
            minuteWidth = params.substring(secondHash + 1, thirdHash).toInt();
            secondWidth = params.substring(thirdHash + 1).toInt();
        }

        if (hourWidth <= 0) hourWidth = HAND_WIDTH;
        if (minuteWidth <= 0) minuteWidth = HAND_WIDTH;
        // "<= 0" statt "< 0": bei einem Dateinamen mit "!0" als Sekundenbreite
        // blieb der Wert 0 stehen, createSprite(0, HAND_HEIGHT) schlaegt fehl und
        // der Sekundenzeiger verschwand bis zum naechsten Zifferblattwechsel.

        // "<= 0" instead of "< 0": with a filename specifying "!0" as the second
        // hand width the value stayed 0, createSprite(0, HAND_HEIGHT) fails and the
        // second hand disappeared until the next clock face change.
        if (secondWidth <= 0) secondWidth = HAND_WIDTH;

        if (hourWidth > HAND_WIDTH) hourWidth = HAND_WIDTH;
        if (minuteWidth > HAND_WIDTH) minuteWidth = HAND_WIDTH;
        if (secondWidth > HAND_WIDTH) secondWidth = HAND_WIDTH;

    }


    // Touch-Funktionalität aktivieren/deaktivieren
    // Enable/disable touch functionality

    void enableTouch() {
#ifdef TOUCH_PIN
        touchEnabled = true;
        pinMode(TOUCH_PIN, INPUT_PULLDOWN);

        // Touch erst nach kurzer Verzögerung aktivieren (verhindert frühe Reads während Init)
        // Enable touch only after a short delay (prevents early reads during init)
        touchEnableAt = millis() + 1000; // 1000 ms Verzögerung
                                         // 1000 ms delay
        DEBUG_PRINTLN("[TOUCH] Touch aktiviert");
#endif
    }


    // Touch-Funktionalität deaktivieren
    // Disable touch functionality

    void disableTouch() {
#ifdef TOUCH_PIN
        touchEnabled = false;
        pinMode(TOUCH_PIN, INPUT);
        DEBUG_PRINTLN("[TOUCH] Touch deaktiviert");
#endif
    }




