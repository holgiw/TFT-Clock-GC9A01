#pragma once

    // Display: Zifferblatt, Zeiger, Sprites, Helligkeit (benoetigt globals.h, config.h, prefs_keys.h,
    // declarations.h). Kurzlebige Puffer (BMP/PNG im Webinterface) bevorzugt im PSRAM, um den internen Heap
    // nicht zu zerstueckeln; new (std::nothrow) liefert bei Fehlschlag sicher nullptr.

    // Display: clock face, hands, sprites, brightness (requires globals.h, config.h, prefs_keys.h,
    // declarations.h). Short-lived buffers (BMP/PNG in the web interface) preferably in PSRAM, so the
    // internal heap does not fragment; new (std::nothrow) reliably returns nullptr on failure.

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


    // Streifen neben der Uhr (nur rechteckige Displays wie das ILI9341): hochkant unter der Uhr, quer
    // (Rotation 90/270 Grad) rechts daneben - mit stripBefore ueber bzw. links davon. false ohne Streifen.

    // Strip next to the clock (rectangular displays like the ILI9341 only): portrait below the clock,
    // landscape (rotation 90/270 degrees) to its right - with stripBefore above or left of it. false without a strip.

    bool infoStripRect(uint8_t displayNum, int& x, int& y, int& w, int& h, bool& landscape) {
        if (TFT_WIDTH == CLOCK_WIDTH && TFT_HEIGHT == CLOCK_HEIGHT) return false;
        landscape = (effectiveRotation(displayNum) % 2) == 1;
        int pw = landscape ? TFT_HEIGHT : TFT_WIDTH;
        int ph = landscape ? TFT_WIDTH : TFT_HEIGHT;
        w = (pw > CLOCK_WIDTH) ? pw - CLOCK_WIDTH : pw;
        h = (pw > CLOCK_WIDTH) ? ph : ph - CLOCK_HEIGHT;
        x = (!stripBefore && pw > CLOCK_WIDTH) ? CLOCK_WIDTH : 0;
        y = (!stripBefore && pw <= CLOCK_WIDTH) ? CLOCK_HEIGHT : 0;
        return true;
    }


    // Linke obere Ecke der Uhr auf dem Display - nur verschoben, wenn der Streifen davor liegt (stripBefore)
    // Top left corner of the clock on the display - only shifted when the strip lies before it (stripBefore)

    void clockOrigin(uint8_t displayNum, int& ox, int& oy) {
        ox = oy = 0;
        int x, y, w, h;
        bool landscape;
        if (!stripBefore || !infoStripRect(displayNum, x, y, w, h, landscape)) return;
        if (landscape) ox = w; else oy = h;
    }


    // Displaytypen dieses Boards: der S2 alle wechselbaren, das C6 die fest verbauten ST7789 (1.47 / 1.3),
    // das S3 sein fest verbautes GC9A01

    // Display types of this board: the S2 all exchangeable ones, the C6 the built-in ST7789 (1.47 / 1.3),
    // the S3 its built-in GC9A01

    bool displayTypeSupported(uint8_t type) {
        bool c6Type = type == DISPLAY_TYPE_ST7789 || type == DISPLAY_TYPE_ST7789_240;
        if (BOARD_WAVESHARE_S3_GC9A01) return type == DISPLAY_TYPE_GC9A01;
        return BOARD_WAVESHARE_C6_ST7789 ? c6Type : type < DISPLAY_TYPE_COUNT && !c6Type;
    }

    // Namen der Displaytypen dieses Boards fuer Fehlermeldungen an flashESP
    // Names of this board's display types for error messages to flashESP

    const char* supportedDisplayNames() {
        if (BOARD_WAVESHARE_S3_GC9A01) return "GC9A01, GC9A01_WITH_BACKLIGHT";
        return BOARD_WAVESHARE_C6_ST7789 ? "ST7789, ST7789_240" : "GC9A01, GC9A01_WITH_BACKLIGHT, GC9D01, ILI9341";
    }

    // Displayname wie bei den uhr3-Builds und in flashESP ("GC9A01", "GC9A01_WITH_BACKLIGHT", "GC9D01",
    // "ILI9341", "ST7789", "ST7789_240") -> Displaytyp + Backlight-Regelung; false auch fuer Typen, die
    // dieses Board nicht kennt. Das ILI9341 hat wie in uhr3 feste Beleuchtung.

    // Display name as in the uhr3 builds and in flashESP ("GC9A01", "GC9A01_WITH_BACKLIGHT", "GC9D01",
    // "ILI9341", "ST7789", "ST7789_240") -> display type + backlight control; false also for types this
    // board does not know. The ILI9341 has a fixed backlight as in uhr3.

    bool parseDisplayName(const String& name, uint8_t& type, bool& backlight) {
        if (name == "GC9A01") { type = DISPLAY_TYPE_GC9A01; backlight = false; }
        else if (name == "GC9A01_WITH_BACKLIGHT") { type = DISPLAY_TYPE_GC9A01; backlight = true; }
        else if (name == "GC9D01") { type = DISPLAY_TYPE_GC9D01; backlight = true; }
        else if (name == "ILI9341") { type = DISPLAY_TYPE_ILI9341; backlight = false; }
        else if (name == "ST7789") { type = DISPLAY_TYPE_ST7789; backlight = true; }
        else if (name == "ST7789_240") { type = DISPLAY_TYPE_ST7789_240; backlight = true; }
        else return false;
        return displayTypeSupported(type);
    }

    // Umkehrung von parseDisplayName(): beim GC9D01 und ST7789 (Beleuchtung immer am Pin) und ILI9341 (feste
    // Beleuchtung) zaehlt die Backlight-Einstellung nicht.

    // Inverse of parseDisplayName(): for the GC9D01 and ST7789 (backlight always on the pin) and the ILI9341
    // (fixed backlight) the backlight setting does not matter.

    const char* displayChoiceName(uint8_t type, bool backlight) {
        if (type == DISPLAY_TYPE_GC9D01) return "GC9D01";
        if (type == DISPLAY_TYPE_ILI9341) return "ILI9341";
        if (type == DISPLAY_TYPE_ST7789) return "ST7789";
        if (type == DISPLAY_TYPE_ST7789_240) return "ST7789_240";
        return backlight ? "GC9A01_WITH_BACKLIGHT" : "GC9A01";
    }

    // Update von uhr3: dessen Firmware vermerkt ihren Displaytyp (PK_UHR3_BUILD_DISPLAY). Uebernommen nur,
    // solange uhr4 noch keinen hat - sonst stellte ein zwischendurch geflashter uhr3-Build die Uhr um. Der
    // Vermerk wird immer geloescht.

    // Update from uhr3: its firmware records its display type (PK_UHR3_BUILD_DISPLAY). Taken over only while
    // uhr4 has none yet - otherwise a uhr3 build flashed in between would switch the clock. The record is
    // always deleted.

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
        if (!displayTypeSupported(type)) type = DISPLAY_TYPE_DEFAULT; // beschaedigter NVS-Wert oder anderes Board
                                                                      // corrupted NVS value or another board
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


    // Neuen Displaytyp samt typabhaengigen Werksvorgaben speichern (wirkt nach dem Neustart). Hardware-Reset
    // beider Displays mit den Zeiten von uhr3 (20 ms Puls, 150 ms Pause) - bei den 64 ms von LovyanGFX blieb
    // das GC9D01 schwarz, daher pin_rst = -1.

    // Store a new display type with type-dependent factory defaults (effective after the restart). Hardware
    // reset of both displays with uhr3's timing (20 ms pulse, 150 ms wait) - with LovyanGFX's 64 ms the
    // GC9D01 stayed black, hence pin_rst = -1.

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
        if (!displayTypeSupported(type)) return;
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


    // Einrichtung per USB (flashESP, setTime), zeilenweise: "UHR4 WIFI <Name-Hex> <Passwort-Hex>", "UHR4
    // DISPLAY <Name>", "UHR4 RESTART", "UHR4 INFO", "UHR4 TIME <Unix-Sek.>" - Antwort "UHR4 OK/ERROR ...",
    // Passwort nie im Log. serialReply() darf kurz warten, sonst sind USB-Ausgaben nicht blockierend.

    // Setup via USB (flashESP, setTime), line by line: "UHR4 WIFI <name hex> <password hex>", "UHR4 DISPLAY
    // <name>", "UHR4 RESTART", "UHR4 INFO", "UHR4 TIME <Unix sec>" - reply "UHR4 OK/ERROR ...", password
    // never logged. serialReply() may wait briefly, otherwise USB output is non-blocking.

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

    // "UHR4 OPENWIFI" (Diagnose): holt die Uhrzeit wie beim Start aus einem offenen WLAN (Schritte im Log), danach Neustart.
    // Gespeicherte WLANs bleiben unberuehrt. Die Antwort kommt sofort, der Test dauert bis zu mehreren Minuten.

    // "UHR4 OPENWIFI" (diagnostic): gets the time from an open WiFi like at boot (steps in the log), then restarts.
    // Stored WiFis stay untouched. The reply comes right away, the test takes up to several minutes.

    void handleSerialOpenWifi() {
        serialReply("UHR4 OK OPENWIFI started");

        // Ohne Auto-Verbindung, sonst verbindet die Uhr sich sofort wieder und der Scan scheitert
        // Without auto-connect, otherwise the clock reconnects right away and the scan fails

        WiFi.setAutoReconnect(false);
        WiFi.disconnect();
        delay(500);
        WiFi.mode(WIFI_STA);
        applyWifiTxPower();
        int found = WiFi.scanNetworks();
        if (found < 0) {
            delay(1000);
            found = WiFi.scanNetworks();
        }
        DEBUG_PRINTLN("[OPEN-WIFI] Scan: " + String(found) + " networks");
        collectStrongestNetworks(found < 0 ? 0 : found);
        WiFi.scanDelete();
        bool ok = fetchTimeFromOpenWifi(true);
        serialRestart(ok ? "UHR4 OK OPENWIFI time set" : "UHR4 OK OPENWIFI no time");
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

    // "UHR4 INFO" -> "UHR4 OK INFO <Name> <gesetzt>": eingestellter Displaytyp und ob er je gespeichert wurde
    // (1) oder nur der Standard gilt (0). flashESP fragt das vor dem Flashen ab und waehlt den Typ vor. Liest
    // nur.

    // "UHR4 INFO" -> "UHR4 OK INFO <name> <set>": configured display type and whether it was ever stored (1)
    // or only the default applies (0). flashESP queries this before flashing and preselects the type.
    // Read-only.

    void handleSerialInfo() {
        bool backlight = preferences.getBool(PK_USE_BACKLIGHT, DISPLAY_GEOMETRY[displayType].backlightDefault);
        serialReply(String("UHR4 OK INFO ") + displayChoiceName(displayType, backlight) + " " + String(preferences.isKey(PK_DISPLAY_TYPE) ? 1 : 0));
    }

    void handleSerialDisplay(const String& name) {
        uint8_t type;
        bool backlight;
        if (!parseDisplayName(name, type, backlight)) {
            serialReply("UHR4 ERROR DISPLAY unknown '" + name + "' (" + supportedDisplayNames() + ")");
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
            else if (cmd == "UHR4 OPENWIFI") {
                handleSerialOpenWifi();
            }
            else if (cmd == "UHR4 PINS") {

                // Diagnose: Pegel von Taster (gedrueckt = HIGH) und Boot-Taste (gedrueckt = LOW), liest nur
                // Diagnostics: level of the button (pressed = HIGH) and the boot button (pressed = LOW), read-only

                serialReply("UHR4 OK PINS BUTTON" + String(BUTTON1) + "=" + String(digitalRead(BUTTON1)) +
                            " BOOT" + String(BOOT_BUTTON) + "=" + String(digitalRead(BOOT_BUTTON)));
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


    // Legt ein 16-Bit-Sprite an, standardmaessig im PSRAM - auch die Zeiger, da interner RAM knapp ist
    // (intern fiel der freie Heap auf 5 KB). Klappt es dort nicht, zweiter Versuch im anderen Speicher.
    // swapBytes fuer RGB565 aus dem RAM.

    // Creates a 16-bit sprite, by default in PSRAM - the hands too, since internal RAM is scarce (internally
    // the free heap dropped to 5 KB). If that fails, a second attempt in the other memory. swapBytes for
    // RGB565 from RAM.

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


    // Textdarstellung fuer Display und Status-Sprites: GLCD-Schrift als CP437-Zeichensatz, ohne
    // UTF-8-Dekodierung - Umlaute und Akzente kommen ueber tftText() als einzelne CP437-Bytes.

    // Text rendering for the display and status sprites: GLCD font as the CP437 charset, without UTF-8
    // decoding - umlauts and accents arrive via tftText() as single CP437 bytes.

    void setupTextStyle(lgfx::LovyanGFX& gfx) {
        gfx.setFont(&fonts::Font0);
        gfx.setAttribute(lgfx::utf8_switch, false);
        gfx.setAttribute(lgfx::cp437_switch, true);
    }


    // Text fuers Display umwandeln: HTML-Entities (&uuml; ...) und UTF-8 (z.B. SSIDs) werden zu CP437-Bytes
    // der GLCD-Schrift, Unbekanntes zu '?'.

    // Convert text for the display: HTML entities (&uuml; ...) and UTF-8 (e.g. SSIDs) become CP437 bytes of
    // the GLCD font, anything unknown becomes '?'.

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




    // Waehlt Display 1 und schaltet beide CS-Pins selbst (LovyanGFX fuehrt keinen, pin_cs = -1). Vorher die
    // laufende Uebertragung abwarten; setRotation() stellt MADCTL fuer diesen Chip und verwirft das gemerkte
    // Adressfenster.

    // Selects display 1 and drives both CS pins itself (LovyanGFX drives none, pin_cs = -1). Wait for the
    // running transfer first; setRotation() sets MADCTL for this chip and discards the cached address window.

    void setCS1(bool state) {
        if (state == LOW) {
            if (tftInitialized) tft.waitDMA();
            if (LGFX_CS_PIN < 0) digitalWrite(CS_1, LOW); // sonst schaltet LovyanGFX CS / otherwise LovyanGFX drives CS
            if (HAS_DISPLAY2) digitalWrite(CS_2, HIGH);
            if (tftInitialized) tft.setRotation(hardwareRotation(1));
        }

    }


    // Waehlt Display 2 bei Dual-Display-Aufbauten ueber seinen Chip-Select-Pin aus
    // (deaktiviert dabei Display 1, siehe Kommentar bei setCS1())

    // Selects Display 2 via its chip-select pin in dual-display setups
    // (disables Display 1, see comment on setCS1())

    void setCS2(bool state) {
        if (state == LOW && HAS_DISPLAY2) {
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

    // Schmales Display mit Streifen (ST7789 172x320): Meldungen quer, 90 Grad weiter gedreht - so passt die doppelt
    // grosse Schrift in die Zeilen (320 statt 172 px breit), die Hoehe bleibt 172 wie der Uhrbereich.

    // Narrow display with a strip (ST7789 172x320): messages in landscape, rotated 90 degrees further - so the
    // double-size font fits the lines (320 instead of 172 px wide), the height stays 172 like the clock area.

    bool statusLandscape() {
        return displayType == DISPLAY_TYPE_ST7789;
    }

    // Breite des Bereichs fuer Meldungen: quer die Panelhoehe, sonst der Uhrbereich
    // Width of the area for messages: in landscape the panel height, otherwise the clock area

    int statusWidth() {
        return statusLandscape() ? TFT_HEIGHT : CLOCK_WIDTH;
    }

    lgfx::LovyanGFX& beginStatusDraw(uint8_t displayNum) {
        bool clockShown = !clockFrameDirty[displayNum - 1]; // bisher Uhrbild auf dem Display / clock image shown so far
        displayNeedsBlank[displayNum - 1] = true; // Meldung auf dem Display - ein "n.a."-Display muss spaeter wieder schwarz werden
                                                  // message on the display - a "n.a." display has to go black again later
        clockFrameDirty[displayNum - 1] = true;   // Uhrbild ist ueberzeichnet - naechster Frame voll senden
                                                  // clock image got drawn over - send the next frame in full
        bool stripShown = !infoStripDirty[displayNum - 1];
        infoStripDirty[displayNum - 1] = true;    // Streifen fuer Uhrzeit/Datum ebenso (nur ILI9341)
                                                  // the time/date strip as well (ILI9341 only)
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

            // Displays mit Streifen vor der ersten Meldung einmal ganz loeschen - Meldungen stehen oben links im
            // Uhrbereich, sonst blieben Streifen und (liegt der Streifen oben) Teile der Uhr eingefroren stehen.

            // Clear displays with a strip completely once before the first message - messages sit top left in the
            // clock area, otherwise the strip and (with the strip on top) parts of the clock would stay frozen.

            int x, y, w, h;
            bool landscape;
            if (stripShown && infoStripRect(displayNum, x, y, w, h, landscape)) tft.fillScreen(TFT_BLACK);

            // Quer: setCS1()/setCS2() setzen beim naechsten Uhrbild wieder die normale Lage. Beim Wechsel vom
            // Uhrbild ganz loeschen, sonst blieben Teile der hochkant gezeichneten Uhr stehen.

            // Landscape: setCS1()/setCS2() restore the normal orientation with the next clock frame. Clear
            // completely when switching from the clock image, otherwise parts of the portrait clock would remain.

            if (statusLandscape()) {
                tft.setRotation((hardwareRotation(displayNum) + 1) % 4);
                if (clockShown || stripShown) tft.fillScreen(TFT_BLACK);
            }
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

        bool changed = rotation != newRotation;
        if (changed) {
            firstRunFlag = true;
            clockFrameDirty[displayNum - 1] = true; // auch im Spiegelbetrieb (Display 2 ohne eigenes Rendern) neu senden
                                                    // resend in mirror mode too (display 2 without its own rendering)
            if (newRotation == TFT_ROTATION_NA) displayNeedsBlank[displayNum - 1] = true; // letztes Uhrbild muss weg
                                                                                          // last clock image has to go
        }
        rotation = newRotation;
        preferences.putUChar((displayNum == 1) ? PK_TFT_ROTATION1 : PK_TFT_ROTATION2, rotation);

        if (changed || !gc9d01SwRotation) {

            // setCS1()/setCS2() setzen die neue Rotation am gewaehlten Chip (hardwareRotation()). Bei einer
            // Aenderung das Display einmal schwarz loeschen - sonst blieben Reste der alten Lage stehen (beim
            // ILI9341 z.B. der Streifen mit Uhrzeit/Datum).

            // setCS1()/setCS2() set the new rotation on the selected chip (hardwareRotation()). On a change,
            // clear the display to black once - otherwise remains of the old orientation would stay (on the
            // ILI9341 e.g. the time/date strip).

            if (displayNum == 1) setCS1(LOW); else setCS2(LOW);
            if (changed) {
                tft.fillScreen(TFT_BLACK);
                infoStripDirty[displayNum - 1] = true;
            }
            setCSIdle(); // zurueck auf den Ausgangszustand, damit loop() im gewohnten Zustand weiterlaeuft
                         // back to the initial state, so loop() continues from its usual state
        }
    }


    // Werksvorgaben fuer min. Helligkeit und Schwellwerte: mit Backlight dimmt die PWM fast bis 0 stufenlos;
    // ohne werden die Pixel abgedunkelt, unter ~100 wird das Zifferblatt unleserlich.

    // Factory defaults for min. brightness and thresholds: with a backlight the PWM dims steplessly almost to
    // 0; without one the pixels are darkened, below ~100 the clock face becomes unreadable.

    void putBrightnessDefaults(bool backlight) {
        preferences.putUChar(PK_MIN_BRIGHTNESS, backlight ? 5 : 100);
        preferences.putInt(PK_LOW_THRESHOLD, backlight ? 1 : 40);
        preferences.putInt(PK_HIGH_THRESHOLD, backlight ? 100 : 60); // 100 statt frueher 255: das Formularfeld erlaubt nur 0-100, der
                                                                     // Lichtwert (5-100 %) ueberschreitet 100 ohnehin nie - gleiche Wirkung
                                                                     // 100 instead of the former 255: the form field only allows 0-100, the
                                                                     // light value (5-100 %) never exceeds 100 anyway - same effect
    }


    // Automatische Helligkeit ein-/ausschalten, samt Versorgungspins des Spannungsteilers - sonst bliebe der
    // Teiler nach spaeterem Einschalten (Formular, Preset) bis zum Neustart ohne Spannung.

    // Switch automatic brightness on/off, including the voltage divider's supply pins - otherwise the divider
    // would stay unpowered until the next restart after enabling it later (form, preset).

    void setAutoBrightness(bool enabled) {
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
        useAdc = enabled;
        preferences.putBool(PK_USE_ADC, useAdc);
    }


    // Helligkeitswert aus einem Preset (/api/setMode) uebernehmen, begrenzen und speichern. false = kein
    // Helligkeits-Schluessel; wirkt beim naechsten updateBrightness().

    // Take over, clamp and store a brightness value from a preset (/api/setMode). false = not a brightness key;
    // takes effect on the next updateBrightness().

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


    // TFT_Backlight passend zu useBacklight: an = PWM mit aktueller Helligkeit, aus = fest HIGH (volle
    // Beleuchtung). HIGH auch beim Start - ein offener BL-Eingang bliebe sonst je nach Modul dunkel.

    // TFT_Backlight according to useBacklight: on = PWM with the current brightness, off = HIGH (full backlight).
    // HIGH at boot too - a floating BL input would otherwise stay dark on some modules.

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

        // Erst Pin umstellen und die Helligkeit neu berechnen, DANN neu einfaerben (sonst blitzte der
        // niedrige PWM-Wert als Abdunklung auf). Mit max. Helligkeit neu beginnen, sonst bliebe der Zielwert
        // des alten Verfahrens stehen.

        // Switch the pin and recompute brightness first, THEN re-tint (otherwise the low PWM value would
        // flash as dimming). Restart from max. brightness, otherwise the old method's target value would
        // stick.

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


    // Ziffern der erzeugten Zifferblaetter: nur 12/3/6/9 (face_default), 1-12, I-XII und 12/3/9 mit Sekundenfeld
    // statt der 6 (face_subdial). Sekundenfeld: Mitte bei SUBDIAL_Y der Breite, Radius SUBDIAL_R der Breite.

    // Numerals of the generated clock faces: only 12/3/6/9 (face_default), 1-12, I-XII and 12/3/9 with a
    // seconds subdial instead of the 6 (face_subdial). Subdial: centre at SUBDIAL_Y, radius SUBDIAL_R of the width.

    enum : uint8_t { FACE_NUMERALS_QUARTER, FACE_NUMERALS_ARABIC, FACE_NUMERALS_ROMAN, FACE_NUMERALS_SUBDIAL };
    constexpr float SUBDIAL_Y = 0.72f;
    constexpr float SUBDIAL_R = 0.14f;

    // Erzeugtes Zifferblatt wie der Generator im Zifferblatt-Designer mit dessen Vorgaben: weiss, schwarzer
    // Rand, Stunden- und Minutenstriche, Ziffern. Je Pixel 4 x 4 Abtastpunkte (Kantenglaettung); zeilenweise,
    // damit nie ein ganzes Bild im RAM liegen muss.

    // Generated clock face like the generator in the clock face designer with its defaults: white, black
    // ring, hour and minute marks, numerals. 4 x 4 samples per pixel (anti-aliasing); row by row, so a whole
    // image never has to be held in RAM.

    struct DefaultFaceGen {
        struct Mark {
            int16_t x0, x1, y0, y1; // Pixelbereich des Strichs
                                    // pixel range of the mark
            int16_t s, c;           // Richtung (sin, -cos) x 1024
                                    // direction (sin, -cos) x 1024
            int16_t t0, t1, hw;     // radial von t0 bis t1, halbe Breite hw (Achtelpixel)
                                    // radially from t0 to t1, half width hw (eighths of a pixel)
            int16_t cx8, cy8;       // Mitte (Achtelpixel): Zifferblatt oder Sekundenfeld
                                    // centre (eighths of a pixel): clock face or subdial
        };

        // Strich bei Winkel a um die Mitte cx/cy (Pixel), aussen bei rOuter, Laenge len, Breite width
        // Mark at angle a around the centre cx/cy (pixels), outer end at rOuter, length len, width width

        void setMark(Mark& m, float cx, float cy, float a, int rOuter, int len, int width) {
            float s = sinf(a), c = -cosf(a);
            m.s = lroundf(s * 1024);
            m.c = lroundf(c * 1024);
            m.t0 = 8 * (rOuter - len);
            m.t1 = 8 * rOuter;
            m.hw = 4 * width;
            m.cx8 = lroundf(8 * cx);
            m.cy8 = lroundf(8 * cy);
            float minX = w, maxX = 0, minY = w, maxY = 0;
            for (int k = 0; k < 4; k++) {
                float rr = (k & 1) ? rOuter : rOuter - len, side = (k & 2) ? width / 2.0f : -width / 2.0f;
                float x = cx + s * rr - c * side, y = cy + c * rr + s * side;
                minX = min(minX, x); maxX = max(maxX, x);
                minY = min(minY, y); maxY = max(maxY, y);
            }
            m.x0 = max(0, (int)floorf(minX)); m.x1 = min(w - 1, (int)floorf(maxX));
            m.y0 = max(0, (int)floorf(minY)); m.y1 = min(w - 1, (int)floorf(maxY));
        }

        // Ziffer als fertige Deckung je Pixel (0..16 getroffene Abtastpunkte) ab x0/y0
        // Numeral as finished coverage per pixel (0..16 samples hit) from x0/y0

        struct Numeral {
            int16_t x0 = 0, y0 = 0, w = 0, h = 0;
            std::vector<uint8_t> cov;
        };
        std::vector<Mark> marks;    // 60 am Rand, mit Sekundenfeld 60 weitere darin (Heap statt Stack)
                                    // 60 at the rim, with a subdial 60 more in it (heap instead of stack)
        Numeral numerals[12];
        int w, c8, ringIn, ringOut; // Mitte und Rand (Achtelpixel)
                                    // centre and ring (eighths of a pixel)
        int32_t subX8 = 0, subY8 = 0, subIn = 0, subOut = 0; // Sekundenfeld-Ring (Achtelpixel), 0 = keiner
                                                             // subdial ring (eighths of a pixel), 0 = none
        bool round;                 // rundes Display: ausserhalb des Kreises weiss wie scaleAndSaveBmp()
                                    // round display: white outside the circle like scaleAndSaveBmp()

        // Setzt eine Ziffer in 4-facher Aufloesung (1-Bit-Sprite, je Sprite-Pixel ein Abtastpunkt) mit der
        // Mitte ihres Umrisses auf den Winkel a - so weit aussen, dass sie limitQ (Viertelpixel) nicht
        // ueberschreitet. False, wenn der Sprite keinen Speicher bekommt.

        // Places a numeral at 4x resolution (1-bit sprite, one sample per sprite pixel) with the centre of its
        // outline at angle a - as far out as possible without exceeding limitQ (quarter pixels). False if the
        // sprite gets no memory.

        bool addNumeral(Numeral& n, const char* text, const lgfx::GFXfont* font, float scale, float a, float limitQ) {
            LGFX_Sprite s;
            s.setColorDepth(1);
            s.setFont(font);
            s.setTextSize(scale);
            int margin = s.fontHeight() / 4 + 2;
            int sw = s.textWidth(text) + 2 * margin, sh = s.fontHeight() + 2 * margin;
            if (!s.createSprite(sw, sh)) return false;
            s.fillSprite(TFT_BLACK);
            s.setTextColor(TFT_WHITE);
            s.setTextDatum(lgfx::top_left);
            s.drawString(text, margin, margin);

            int bx0 = sw, bx1 = -1, by0 = sh, by1 = -1;
            for (int y = 0; y < sh; y++) {
                for (int x = 0; x < sw; x++) {
                    if (!s.readPixelValue(x, y)) continue;
                    bx0 = min(bx0, x); bx1 = max(bx1, x);
                    by0 = min(by0, y); by1 = max(by1, y);
                }
            }
            if (bx1 < 0) return true;

            // Mitte so, dass der Umriss in Strahlrichtung genau bis limitQ reicht
            // Centre placed so that the outline reaches exactly limitQ in the ray direction

            float sn = sinf(a), cs = -cosf(a);
            float extent = (bx1 - bx0 + 1) / 2.0f * fabsf(sn) + (by1 - by0 + 1) / 2.0f * fabsf(cs);
            float rq = limitQ - extent;
            int ox = lroundf(4 * w / 2.0f + sn * rq - (bx0 + bx1 + 1) / 2.0f);
            int oy = lroundf(4 * w / 2.0f + cs * rq - (by0 + by1 + 1) / 2.0f);
            n.x0 = max(0, (bx0 + ox) / 4);
            n.y0 = max(0, (by0 + oy) / 4);
            n.w = min(w - 1, (bx1 + ox) / 4) - n.x0 + 1;
            n.h = min(w - 1, (by1 + oy) / 4) - n.y0 + 1;
            if (n.w <= 0 || n.h <= 0) return true;
            n.cov.assign((size_t)n.w * n.h, 0);
            for (int y = by0; y <= by1; y++) {
                int py = (y + oy) / 4 - n.y0;
                if (py < 0 || py >= n.h) continue;
                for (int x = bx0; x <= bx1; x++) {
                    int px = (x + ox) / 4 - n.x0;
                    if (px >= 0 && px < n.w && s.readPixelValue(x, y)) n.cov[(size_t)py * n.w + px]++;
                }
            }
            return true;
        }

        // numerals: FACE_NUMERALS_*. False, wenn eine Ziffer fehlt (zu wenig Speicher) - dann nicht speichern.
        // numerals: FACE_NUMERALS_*. False if a numeral is missing (too little memory) - then do not save.

        bool init(uint8_t numeralMode) {
            w = CLOCK_WIDTH;
            round = displayGeom->round;
            int r = w / 2;
            int ring = lroundf(w * 0.02f);
            int outer = r - ring - lroundf(r * 0.03f);
            c8 = 8 * r;
            ringIn = 8 * (r - ring);
            ringOut = 8 * r;

            // 60 Striche reihum, jeder fuenfte ist ein Stundenstrich
            // 60 marks around, every fifth is an hour mark

            marks.resize(numeralMode == FACE_NUMERALS_SUBDIAL ? 120 : 60);
            for (int i = 0; i < 60; i++) {
                bool hour = (i % 5 == 0);
                setMark(marks[i], r, r, i * 2 * PI / 60, outer, lroundf(w * (hour ? 0.08f : 0.03f)),
                        max(1L, lroundf(w * (hour ? 0.025f : 0.008f))));
            }

            // Sekundenfeld: duenner Ring und 60 Striche um den Drehpunkt (Pixelmitte wie der Zeiger-Drehpunkt)
            // Seconds subdial: thin ring and 60 marks around the pivot (pixel centre like the hand pivot)

            if (numeralMode == FACE_NUMERALS_SUBDIAL) {
                float sx = w / 2 + 0.5f, sy = lroundf(w * SUBDIAL_Y) + 0.5f;
                int sr = lroundf(w * SUBDIAL_R), sw = max(1L, lroundf(w * 0.007f));
                subX8 = lroundf(8 * sx); subY8 = lroundf(8 * sy);
                subIn = 8 * (sr - sw); subOut = 8 * sr;
                for (int i = 0; i < 60; i++) {
                    bool five = (i % 5 == 0);
                    setMark(marks[60 + i], sx, sy, i * 2 * PI / 60, sr - sw - 1, lroundf(w * (five ? 0.025f : 0.012f)),
                            max(1L, lroundf(w * (five ? 0.01f : 0.004f))));
                }
            }

            // Ziffern: 1-12 in FreeSans Bold, I-XII in FreeSerif Bold, Hoehe wie im Designer (Schriftgroesse 9 %
            // der Breite); jede endet 5 % der Breite vor den Stundenstrichen.

            // Numerals: 1-12 in FreeSans Bold, I-XII in FreeSerif Bold, height as in the designer (font size 9 %
            // of the width); each ends 5 % of the width before the hour marks.

            static const char* const romanNumerals[12] = { "XII", "I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX", "X", "XI" };
            bool roman = numeralMode == FACE_NUMERALS_ROMAN;
            const lgfx::GFXfont* font = roman ? &fonts::FreeSerifBold24pt7b : &fonts::FreeSansBold24pt7b;
            int refHeight = font->glyph[(roman ? 'I' : '1') - font->first].height;
            float scale = 4 * w * (roman ? 0.06f : 0.065f) / refHeight;
            float limitQ = 4 * (outer - lroundf(w * 0.08f) - w * 0.05f);
            bool ok = true;
            for (int h = 0; h < 12; h++) {
                if (numeralMode != FACE_NUMERALS_ARABIC && numeralMode != FACE_NUMERALS_ROMAN && h % 3) continue;
                if (numeralMode == FACE_NUMERALS_SUBDIAL && h == 6) continue; // dort liegt das Sekundenfeld
                                                                              // the subdial is there
                char arabic[3];
                snprintf(arabic, sizeof(arabic), "%d", h ? h : 12);
                if (!addNumeral(numerals[h], roman ? romanNumerals[h] : arabic, font, scale, h * PI / 6, limitQ)) ok = false;
            }
            return ok;
        }

        // Eine Zeile (w Pixel, RGB565) - Schwarz deckt Weiss je nach Zahl getroffener Abtastpunkte
        // One row (w pixels, RGB565) - black covers white according to the number of samples hit

        void row(int y, uint16_t* out) const {
            uint8_t cov[CLOCK_MAX] = { 0 };
            const int32_t in2 = (int32_t)ringIn * ringIn, out2 = (int32_t)ringOut * ringOut;
            const int32_t nearIn = (int32_t)(ringIn - 6) * (ringIn - 6), nearOut = (int32_t)(ringOut + 6) * (ringOut + 6);
            const int32_t fullIn = (int32_t)(ringIn + 6) * (ringIn + 6), fullOut = (int32_t)(ringOut - 6) * (ringOut - 6);

            // Rand: Pixelmitte weit genug innen oder aussen -> ohne Abtastung entschieden
            // Ring: pixel centre far enough inside or outside -> decided without sampling

            int32_t cy = 8 * y + 4 - c8;
            for (int x = 0; x < w; x++) {
                int32_t cx = 8 * x + 4 - c8, d2 = cx * cx + cy * cy;
                if (d2 < nearIn || d2 > nearOut || (round && d2 > out2)) continue;
                if (d2 > fullIn && d2 < fullOut) { cov[x] = 16; continue; }
                for (int sy = 0; sy < 4; sy++) {
                    int32_t dy = 8 * y + 2 * sy + 1 - c8;
                    for (int sx = 0; sx < 4; sx++) {
                        int32_t dx = 8 * x + 2 * sx + 1 - c8, sd2 = dx * dx + dy * dy;
                        if (sd2 >= in2 && sd2 <= out2) cov[x]++;
                    }
                }
            }

            // Striche als gedrehte Rechtecke: radial t, quer u (beides x 1024)
            // Marks as rotated rectangles: radial t, across u (both x 1024)

            for (const Mark& m : marks) {
                if (y < m.y0 || y > m.y1) continue;
                for (int x = m.x0; x <= m.x1; x++) {
                    int n = 0;
                    for (int sy = 0; sy < 4; sy++) {
                        int32_t dy = 8 * y + 2 * sy + 1 - m.cy8;
                        for (int sx = 0; sx < 4; sx++) {
                            int32_t dx = 8 * x + 2 * sx + 1 - m.cx8;
                            int32_t t = dx * m.s + dy * m.c, u = dy * m.s - dx * m.c;
                            if (t >= m.t0 * 1024 && t <= m.t1 * 1024 && abs(u) <= m.hw * 1024) n++;
                        }
                    }
                    cov[x] = min(16, cov[x] + n);
                }
            }

            // Ring des Sekundenfelds, je Pixel 4 x 4 Abtastpunkte
            // Ring of the subdial, 4 x 4 samples per pixel

            if (subOut > 0 && abs(8 * y + 4 - subY8) <= subOut + 8) {
                for (int x = max(0L, (long)((subX8 - subOut) / 8 - 1)); x < w && 8 * x <= subX8 + subOut + 8; x++) {
                    for (int sy = 0; sy < 4; sy++) {
                        int32_t dy = 8 * y + 2 * sy + 1 - subY8;
                        for (int sx = 0; sx < 4; sx++) {
                            int32_t dx = 8 * x + 2 * sx + 1 - subX8, d2 = dx * dx + dy * dy;
                            if (d2 >= subIn * subIn && d2 <= subOut * subOut && cov[x] < 16) cov[x]++;
                        }
                    }
                }
            }
            for (const Numeral& n : numerals) {
                if (y < n.y0 || y >= n.y0 + n.h) continue;
                const uint8_t* src = n.cov.data() + (size_t)(y - n.y0) * n.w;
                for (int x = 0; x < n.w; x++) cov[n.x0 + x] = min(16, cov[n.x0 + x] + src[x]);
            }
            for (int x = 0; x < w; x++) {
                int v = 255 - (cov[x] * 255 + 8) / 16;
                uint16_t r5 = (v * 31 + 127) / 255, g6 = (v * 63 + 127) / 255;
                out[x] = (r5 << 11) | (g6 << 5) | r5;
            }
        }
    };

    // Zeichnet das Standard-Zifferblatt nach 'dest' (CLOCK_WIDTH x CLOCK_HEIGHT) - Notloesung, falls
    // face_default.bmp nicht gespeichert werden kann (Dateisystem voll).

    // Draws the default clock face into 'dest' (CLOCK_WIDTH x CLOCK_HEIGHT) - fallback in case
    // face_default.bmp cannot be stored (file system full).

    void drawDefaultFace(uint16_t* dest) {
        if (!dest) return;
        DefaultFaceGen gen;
        gen.init(FACE_NUMERALS_QUARTER);
        for (int y = 0; y < gen.w; y++) gen.row(y, dest + (size_t)y * gen.w);
    }

    // Schreibt ein Bild zeilenweise als RLEB-Datei wie hochgeladene Zifferblaetter und Zeiger: erst die Groesse
    // zaehlen, dann schreiben. rowFn liefert Zeile y (w <= CLOCK_MAX Pixel). Bei einem Fehler wird die Datei entfernt.

    // Writes an image row by row as an RLEB file like uploaded clock faces and hands: count the size first, then
    // write. rowFn delivers row y (w <= CLOCK_MAX pixels). On an error the file is removed.

    bool writeRleImage(const String& path, int w, int h, const std::function<void(int, uint16_t*)>& rowFn) {
        uint16_t row[CLOCK_MAX];
        uint8_t enc[CLOCK_MAX * 2 + 8]; // >= rleMaxEncodedSize(CLOCK_MAX)
                                        // >= rleMaxEncodedSize(CLOCK_MAX)
        uint32_t compressed = 0;
        for (int y = 0; y < h; y++) {
            rowFn(y, row);
            compressed += rleEncode565(row, w, enc);
        }
        File f = LittleFS.open(path, "w");
        if (!f) {
            DEBUG_PRINTLN("[FS] Error: couldnt create " + path);
            return false;
        }
        uint8_t header[20];
        header[0] = 'R'; header[1] = 'L'; header[2] = 'E'; header[3] = 'B';
        *(int32_t*)&header[4] = w;
        *(int32_t*)&header[8] = h;
        *(uint32_t*)&header[12] = compressed;
        *(uint32_t*)&header[16] = (uint32_t)w * h * 2;
        bool ok = f.write(header, sizeof(header)) == sizeof(header);
        for (int y = 0; ok && y < h; y++) {
            if (y % 20 == 0) yield();
            rowFn(y, row);
            size_t n = rleEncode565(row, w, enc);
            ok = f.write(enc, n) == n;
        }
        f.close();
        if (!ok) {
            LittleFS.remove(path);
            DEBUG_PRINTLN("[FS] Error: couldnt write " + path + " (file system full?)");
            return false;
        }
        DEBUG_PRINTLN("[FS] Created " + path + " (" + String(compressed + 20) + " bytes)");
        return true;
    }

    // Erzeugt ein Zifferblatt (FACE_NUMERALS_*) und speichert es unter 'path'
    // Generates a clock face (FACE_NUMERALS_*) and stores it under 'path'

    bool writeGeneratedFace(const char* path, uint8_t numeralMode) {
        DefaultFaceGen gen;
        if (!gen.init(numeralMode)) {
            DEBUG_PRINTLN(String("[FS] Error: not enough memory for the numerals of ") + path);
            return false;
        }

        // Mit Sekundenfeld gleich den passenden Drehpunkt des Sekundenzeigers (facecfg_*.txt)
        // With a subdial the matching second hand pivot right away (facecfg_*.txt)

        if (numeralMode == FACE_NUMERALS_SUBDIAL) saveFaceSettings(path, CLOCK_WIDTH / 2, lroundf(CLOCK_WIDTH * SUBDIAL_Y));
        return writeRleImage(path, gen.w, gen.w, [&gen](int y, uint16_t* row) { gen.row(y, row); });
    }

    // Zifferblaetter des Startpakets (ensureStarterSet(), refreshGeneratedAssets())
    // Clock faces of the starter set (ensureStarterSet(), refreshGeneratedAssets())

    struct StarterFace { const char* path; uint8_t numerals; };
    const StarterFace STARTER_FACES[] = {
        { "/face_default.bmp", FACE_NUMERALS_QUARTER },
        { "/face_numbers.bmp", FACE_NUMERALS_ARABIC },
        { "/face_roman.bmp", FACE_NUMERALS_ROMAN },
        { "/face_subdial.bmp", FACE_NUMERALS_SUBDIAL },
    };

    // Legt face_default.bmp (12, 3, 6, 9) an, falls es fehlt - Ersatz fuer jedes fehlende Zifferblatt
    // Creates face_default.bmp (12, 3, 6, 9) if it is missing - substitute for every missing clock face

    bool ensureDefaultFace() {
        if (LittleFS.exists("/face_default.bmp")) return true;
        return writeGeneratedFace("/face_default.bmp", FACE_NUMERALS_QUARTER);
    }

    // Laedt ein Zifferblatt nach 'dest'; fehlt es oder ist es unlesbar, das Standard-Zifferblatt (bei Bedarf
    // erzeugt, notfalls nur gezeichnet).

    // Loads a clock face into 'dest'; if it is missing or unreadable, the default clock face (created if needed,
    // drawn only as a last resort).

    void loadFaceOrDefault(const String& path, uint16_t* dest) {
        if (LittleFS.exists(path) && loadFaceBmpInto(path, dest, CLOCK_WIDTH, CLOCK_HEIGHT)) return;
        if (ensureDefaultFace() && loadFaceBmpInto("/face_default.bmp", dest, CLOCK_WIDTH, CLOCK_HEIGHT)) return;
        drawDefaultFace(dest);
    }

    // Halbe Breite eines geschwungenen Zeigers (Satz 2) im Abstand d vom Drehpunkt (Pixel, d > 0 Richtung
    // Spitze, L = Laenge): Stunden- und Minutenzeiger als Blatt mit eingezogener Spitze auf duennem Stiel,
    // runde Nabe; Sekundenzeiger spitz zulaufend mit rundem Gegengewicht.

    // Half width of a curved hand (set 2) at distance d from the pivot (pixels, d > 0 towards the tip, L =
    // length): hour and minute hand as a leaf with a drawn-in tip on a thin stem, round boss; second hand
    // tapering with a round counterweight.

    float curvedHandHalfWidth(const char* part, float L, float d) {
        float c = CLOCK_WIDTH, w = 0;
        if (strcmp(part, "second") == 0) {
            float ws = max(0.6f, 0.0045f * c);
            if (d >= 0 && d <= L) w = ws * (1 - 0.45f * d / L);
            float tail = 0.1f * c, rw = 0.022f * c, dw = d + tail - rw;
            if (d < 0 && d >= -tail) w = ws;
            if (fabsf(dw) <= rw) w = max(w, sqrtf(rw * rw - dw * dw));
            return w;
        }
        bool hour = strcmp(part, "hour") == 0;
        float u0 = hour ? 0.3f : 0.45f, leaf = (hour ? 0.036f : 0.026f) * c, k = hour ? 0.6f : 0.55f;
        if (d >= 0 && d <= L) {
            float u = d / L;
            if (u <= u0 + 0.05f) w = max(0.6f, 0.007f * c);
            if (u >= u0) w = max(w, leaf * powf(sinf(PI * powf((u - u0) / (1 - u0), k)), 1.4f));
        }
        float rb = 0.028f * c;
        if (fabsf(d) <= rb) w = max(w, sqrtf(rb * rb - d * d));
        return w;
    }

    // Erzeugte Zeigersaetze, mittig auf dem Drehpunkt, Hintergrund weiss wie bei hochgeladenen Zeigern (gilt
    // ueberall als transparent). Satz 0, 1 und 3: Stunden- und Minutenzeiger als schwarze Balken bis zum unteren
    // Bildrand, Sekundenzeiger als duenne rote (0) bzw. schwarze (1) Linie, bei 3 kurz fuers Sekundenfeld; Satz 2
    // geschwungen.

    // Generated hand sets, centred on the pivot, background white like uploaded hands (counts as transparent
    // everywhere). Sets 0, 1 and 3: hour and minute hand as black bars down to the bottom edge, second hand as a
    // thin red (0) or black (1) line, short for the subdial with 3; set 2 curved.

    // Halbe Breite des kurzen Sekundenzeigers von Satz 3 (Sekundenfeld): Nadel bis kurz vor den Feldrand, kurzes
    // Gegengewicht, eigene runde Scheibe am Drehpunkt
    // Half width of the short second hand of set 3 (subdial): needle up to just before the subdial rim, short
    // counterweight, its own round disc at the pivot

    float subdialSecondHalfWidth(float d) {
        float c = CLOCK_WIDTH, r = SUBDIAL_R * c, w = 0, rd = 0.03f * c;
        if (d >= -0.3f * r && d <= 0.92f * r) w = max(0.6f, 0.0083f * c);
        if (fabsf(d) <= rd) w = max(w, sqrtf(rd * rd - d * d));
        return w;
    }

    void generatedHandRow(int set, const char* part, int y, uint16_t* row) {
        int top = strcmp(part, "hour") == 0 ? HAND_PIVOT_Y - lroundf(HAND_LEGACY_PIVOT_Y * 0.56f) : HAND_TOP_PAD;
        bool second = strcmp(part, "second") == 0;
        if (set == 2 || (set == 3 && second)) {

            // Je Pixel 4 x 4 Abtastpunkte, gesetzt ab der Haelfte - scharfe Kante, die Uhr glaettet beim Drehen
            // 4 x 4 samples per pixel, set from half of them - sharp edge, the clock smooths when rotating

            float half[4];
            for (int sy = 0; sy < 4; sy++) {
                float d = HAND_PIVOT_Y + 0.5f - (y + (sy + 0.5f) / 4);
                half[sy] = set == 3 ? subdialSecondHalfWidth(d) : curvedHandHalfWidth(part, HAND_PIVOT_Y - top, d);
            }
            for (int x = 0; x < HAND_WIDTH; x++) {
                int n = 0;
                for (int sx = 0; sx < 4; sx++) {
                    float dx = fabsf(x + (sx + 0.5f) / 4 - (HAND_WIDTH / 2 + 0.5f));
                    for (int sy = 0; sy < 4; sy++) if (dx <= half[sy]) n++;
                }
                row[x] = n < 8 ? 0xFFFF : set == 3 ? 0xF800 : 0x0000;
            }
            return;
        }
        int half = lroundf(CLOCK_WIDTH * (second ? 0.0083f : 0.027f));
        uint16_t color = (second && set == 0) ? 0xF800 : 0x0000;
        for (int x = 0; x < HAND_WIDTH; x++) {
            row[x] = (y >= top && abs(x - HAND_WIDTH / 2) <= half) ? color : 0xFFFF;
        }
    }

    // Zeichnet einen Zeiger des Standardsatzes 0 nach 'dest' (HAND_WIDTH x HAND_HEIGHT) - Notloesung ohne Datei
    // Draws a hand of the default set 0 into 'dest' (HAND_WIDTH x HAND_HEIGHT) - fallback without a file

    void drawDefaultHand(const char* part, uint16_t* dest) {
        for (int y = 0; y < HAND_HEIGHT; y++) generatedHandRow(0, part, y, dest + y * HAND_WIDTH);
    }

    // Legt fehlende Dateien eines erzeugten Zeigersatzes an (hand_set<id>_hour/minute/second.bmp) - style waehlt
    // die Form (0-3, siehe generatedHandRow())
    // Creates missing files of a generated hand set (hand_set<id>_hour/minute/second.bmp) - style picks the
    // shape (0-3, see generatedHandRow())

    bool writeGeneratedHandSet(int style, const String& id) {
        bool ok = true;
        for (const char* part : { "hour", "minute", "second" }) {
            String path = "/hand_set" + id + "_" + part + ".bmp";
            if (LittleFS.exists(path)) continue;
            ok = writeRleImage(path, HAND_WIDTH, HAND_HEIGHT, [style, part](int y, uint16_t* row) { generatedHandRow(style, part, y, row); }) && ok;
        }
        return ok;
    }

    bool handSetExists(const String& id) {
        for (const char* part : { "hour", "minute", "second" }) {
            if (LittleFS.exists("/hand_set" + id + "_" + part + ".bmp")) return true;
        }
        return false;
    }

    // Nummer des Zeigersatzes fuers Sekundenfeld (Form 3): beim ersten Mal die erste freie ab 3 - auf aelteren
    // Uhren kann 3 schon ein eigener Satz sein -, danach gemerkt (PK_SUBDIAL_SET)
    // Number of the hand set for the subdial (style 3): the first free one from 3 the first time - on older
    // clocks 3 can already be an own set -, remembered afterwards (PK_SUBDIAL_SET)

    String subdialHandSet() {
        String id = preferences.getString(PK_SUBDIAL_SET, "");
        if (id.length()) return id;
        int n = 3;
        while (n < 99 && handSetExists(String(n))) n++;
        id = String(n);
        preferences.putString(PK_SUBDIAL_SET, id);
        return id;
    }

    // Von der Firmware erzeugte Dateien (Startpaket): die Zifferblaetter STARTER_FACES und die Zeigersaetze 0, 1, 2
    // und der Sekundenfeld-Satz. Sie lassen sich weder loeschen noch umbenennen noch ueberschreiben (Weboberflaeche,
    // Upload, Designer, Sicherung) - die Uhr erzeugt sie bei Bedarf neu.

    // Files created by the firmware (starter set): the clock faces STARTER_FACES and the hand sets 0, 1, 2 and the
    // subdial set. They can be neither deleted nor renamed nor overwritten (web interface, upload, designer,
    // backup) - the clock creates them again when needed.

    bool isProtectedHandSetId(const String& id) {
        return id == "0" || id == "1" || id == "2" || id == subdialHandSet();
    }

    bool isProtectedFile(const String& path) {
        String name = path.startsWith("/") ? path.substring(1) : path;
        if (name.startsWith("face_")) {
            for (const auto& f : STARTER_FACES) {
                if (name == String(f.path + 1)) return true;
            }
            return false;
        }
        if (name.startsWith("hand_set") && name.endsWith(".bmp")) {
            int us = name.indexOf('_', 8);
            return us > 8 && isProtectedHandSetId(name.substring(8, us));
        }
        return false;
    }

    // Namen fuer die Designer-Seiten und die Reihenfolge der Uebersichten (Startpaket zuerst)
    // Names for the designer pages and the order of the overviews (starter set first)

    String protectedFacesJs() {
        String out;
        for (const auto& f : STARTER_FACES) out += String(out.length() ? "," : "") + "'" + (f.path + 1) + "'";
        return out;
    }

    String protectedSetsJs() {
        return String("'0','1','2','") + subdialHandSet() + "'";
    }

    // Datei kopieren (LittleFS)
    // Copy a file (LittleFS)

    bool copyLittleFile(const String& from, const String& to) {
        File in = LittleFS.open(from, "r");
        if (!in) return false;
        File out = LittleFS.open(to, "w");
        if (!out) {
            in.close();
            return false;
        }
        uint8_t buf[512];
        bool ok = true;
        for (size_t n = in.read(buf, sizeof(buf)); n > 0 && ok; n = in.read(buf, sizeof(buf))) ok = out.write(buf, n) == n;
        in.close();
        out.close();
        if (!ok) LittleFS.remove(to);
        return ok;
    }

    // Jeder Zeigersatz bekommt alle drei Zeiger: einen fehlenden kopiert die Uhr aus dem Satz 0 (bisher zeigte sie
    // ihn nur ersatzweise an). So bleibt ein Satz auch bestehen, wenn sich Satz 0 aendert, und jeder Zeiger
    // laesst sich einzeln anpassen.

    // Every hand set gets all three hands: the clock copies a missing one from set 0 (it used to show it only as a
    // substitute). This way a set stays as it is when set 0 changes, and every hand can be adjusted on its own.

    void completeHandSet(const String& id) {
        if (id.isEmpty() || id == "0" || id.startsWith("~")) return;
        for (const char* part : { "hour", "minute", "second" }) {
            String path = "/hand_set" + id + "_" + part + ".bmp", source = String("/hand_set0_") + part + ".bmp";
            if (!LittleFS.exists(path) && LittleFS.exists(source)) copyLittleFile(source, path);
        }
    }

    void completeHandSets() {
        std::set<String> ids;
        File root = LittleFS.open("/");
        for (File f = root.openNextFile(); f; f = root.openNextFile()) {
            String n = f.name();
            if (f.isDirectory() || !n.startsWith("hand_set") || !n.endsWith(".bmp")) continue;
            int us = n.indexOf('_', 8);
            if (us > 8) ids.insert(n.substring(8, us));
        }
        root.close();
        for (const String& id : ids) completeHandSet(id);
    }

    // Alle Zeigerdateien im neuen Format (HAND_WIDTH x HAND_HEIGHT) speichern. Zeiger im alten Format (schmaler
    // und/oder kuerzer) werden transparent aufgefuellt wie beim Laden (placeHand) - verlustfrei, die Uhr zeigt sie
    // unveraendert. Dateien anderer Groesse bleiben unberuehrt. Gibt die Zahl der umgeschriebenen Dateien zurueck.

    // Store all hand files in the new format (HAND_WIDTH x HAND_HEIGHT). Hands in the old format (narrower
    // and/or shorter) are padded transparent as when loading (placeHand) - lossless, the clock shows them
    // unchanged. Files of any other size stay untouched. Returns the number of rewritten files.

    int upgradeHandFiles() {
        std::vector<String> names;
        File root = LittleFS.open("/");
        for (File f = root.openNextFile(); f; f = root.openNextFile()) {
            String n = f.name();
            if (f.isDirectory() || !n.startsWith("hand_set") || !n.endsWith(".bmp")) continue;
            int32_t w, h;
            if (readImageSize(("/" + n).c_str(), w, h) && isValidHandSize(w, h) && (w != HAND_WIDTH || h != HAND_HEIGHT)) names.push_back(n);
        }
        root.close();
        if (names.empty()) return 0;

        uint16_t* pixels = (uint16_t*)preferPsramMalloc((size_t)HAND_WIDTH * HAND_HEIGHT * sizeof(uint16_t));
        if (!pixels) return 0;
        const String tmp = "/hand_upgrade.tmp";
        int done = 0;
        for (const String& n : names) {
            String path = "/" + n;
            if (!loadHandPixels(path, pixels)) continue;
            bool ok = writeRleImage(tmp, HAND_WIDTH, HAND_HEIGHT, [pixels](int y, uint16_t* row) {
                memcpy(row, pixels + (size_t)y * HAND_WIDTH, (size_t)HAND_WIDTH * sizeof(uint16_t));
            });
            if (ok && LittleFS.rename(tmp, path)) done++; // rename ersetzt die alte Datei in einem Schritt
                                                          // rename replaces the old file in one step
            else LittleFS.remove(tmp);
        }
        free(pixels);
        DEBUG_PRINTLN("[Starter] Hand files brought to the new size: " + String(done));
        return done;
    }

    // Legt fehlende Dateien des Standardsatzes 0 an - Ersatz fuer jeden fehlenden Zeiger
    // Creates missing files of the default set 0 - substitute for every missing hand

    bool ensureDefaultHands() {
        if (LittleFS.exists("/hand_set0_hour.bmp") && LittleFS.exists("/hand_set0_minute.bmp") &&
            LittleFS.exists("/hand_set0_second.bmp")) return true;
        return writeGeneratedHandSet(0, "0");
    }

    // Satz-ID der Dateien: leer und "default" (alte Einstellungen, Presets) stehen fuer den Standardsatz 0
    // Set ID of the files: empty and "default" (old settings, presets) stand for the default set 0

    String handSetFileId(const String& setId) {
        return (setId.isEmpty() || setId == "default") ? String("0") : setId;
    }

    // Laedt einen Zeiger des Satzes nach 'dest'; fehlt er, den des Standardsatzes 0 (bei Bedarf erzeugt, notfalls
    // nur gezeichnet).

    // Loads a hand of the set into 'dest'; if it is missing, the one of the default set 0 (created if needed, drawn
    // only as a last resort).

    void loadHandOrDefault(const String& setId, const char* part, uint16_t* dest) {
        String path = "/hand_set" + handSetFileId(setId) + "_" + part + ".bmp";
        if (LittleFS.exists(path) && loadHandPixels(path, dest)) return;
        String fallback = String("/hand_set0_") + part + ".bmp";
        if (ensureDefaultHands() && loadHandPixels(fallback, dest)) return;
        drawDefaultHand(part, dest);
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

    // Zielgroesse beim Speichern einer Zeigerdatei: eines der vier gueltigen Formate bleibt unskaliert, alles
    // andere wird auf das Legacy-Format (HAND_LEGACY_WIDTH/-HEIGHT) skaliert.

    // Target size when storing a hand file: one of the four valid formats stays unscaled, everything else is
    // scaled to the legacy format (HAND_LEGACY_WIDTH/-HEIGHT).

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

    // Versionskennung einer Datei (Groesse, Aenderungszeit, Displaygroesse) fuer Bild-Adressen "&v=...": ein Bild
    // laesst sich dauerhaft zwischenspeichern, weil sich die Adresse mit der Datei aendert.

    // Version tag of a file (size, modification time, display size) for image addresses "&v=...": an image can be
    // cached permanently because the address changes with the file.

    String fileVersion(const String& path) {
        File f = LittleFS.open(path, "r");
        if (!f) return "0";
        String v = String((uint32_t)f.size(), HEX) + "-" + String((uint32_t)f.getLastWrite(), HEX) + "-" + String(CLOCK_WIDTH, HEX);
        f.close();
        return v;
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

    // Liest ein RLEB-Zifferblatt Pixel fuer Pixel - aus dem RAM (clockFaceRle) oder in kleinen Bloecken aus der
    // Datei. Laeufe duerfen ueber Zeilengrenzen gehen (scaleAndSaveBmp() kodiert das ganze Bild am Stueck).

    // Reads an RLEB clock face pixel by pixel - from RAM (clockFaceRle) or in small blocks from the file. Runs
    // may cross row boundaries (scaleAndSaveBmp() encodes the whole image in one go).

    struct RleFaceReader {
        const uint8_t* mem = nullptr;
        File file;
        uint8_t buf[256];
        size_t bufLen = 0, bufPos = 0;
        size_t remaining = 0;   // noch nicht gelesene komprimierte Bytes
                                // compressed bytes not read yet
        int runLeft = 0;
        bool literal = false;
        uint16_t runPixel = 0;

        bool nextByte(uint8_t& b) {
            if (remaining == 0) return false;
            if (mem) {
                b = *mem++;
            }
            else {
                if (bufPos >= bufLen) {
                    bufLen = file.read(buf, min(sizeof(buf), remaining));
                    bufPos = 0;
                    if (bufLen == 0) return false;
                }
                b = buf[bufPos++];
            }
            remaining--;
            return true;
        }

        bool nextPixel(uint16_t& px) {
            uint8_t lo, hi;
            if (runLeft == 0) {
                uint8_t ctrl;
                if (!nextByte(ctrl)) return false;
                literal = ctrl <= 127;
                runLeft = literal ? ctrl + 1 : 257 - ctrl;
                if (!literal) {
                    if (!nextByte(lo) || !nextByte(hi)) return false;
                    runPixel = lo | (hi << 8);
                }
            }
            if (literal) {
                if (!nextByte(lo) || !nextByte(hi)) return false;
                px = lo | (hi << 8);
            }
            else {
                px = runPixel;
            }
            runLeft--;
            return true;
        }
    };

    // Ohne PSRAM: nur mit Rohbild, Helligkeits-Cache und Zwischenbild (je ein Vollbild) bliebe beim 240er-Display
    // fuer WLAN und Webserver kein Speicher. Dann zeichnet loadClockFace() jedes Bild aus den RLE-Daten neu.

    // Without PSRAM: with raw image, brightness cache and composite (one full frame each) nothing would be left for
    // WiFi and web server on the 240 display. Then loadClockFace() redraws every frame from the RLE data.

    bool lowMemoryFace() {
        return !psramFound();
    }

    // Waehlt die Quelle des Zifferblatts im Sparmodus: die gewaehlte RLEB-Datei in passender Groesse, sonst
    // face_default.bmp, sonst das erzeugte Zifferblatt. Kleine Dateien kommen ganz in den RAM, solange danach
    // noch genug fuer WLAN und Webserver frei bleibt.

    // Chooses the clock face source in low-memory mode: the selected RLEB file of the right size, otherwise
    // face_default.bmp, otherwise the generated clock face. Small files go completely into RAM, as long as
    // enough stays free for WiFi and web server afterwards.

    void prepareClockFaceRle() {
        parseBackgroundFilename(selectedBackground, hourHandWidth, minuteHandWidth, secondHandWidth);
        updateHandWidths(hourHandWidth, minuteHandWidth, secondHandWidth);
        if (clockFaceRleReady) return;
        clockFaceRleReady = true;
        clockFaceRlePath = "";

        if (!selectedBackground.startsWith("/")) selectedBackground = "/" + selectedBackground;
        int32_t w, h;
        if (readRleSize(selectedBackground, w, h) && w == CLOCK_WIDTH && h == CLOCK_HEIGHT) {
            clockFaceRlePath = selectedBackground;
        }
        else if (ensureDefaultFace() && readRleSize("/face_default.bmp", w, h) && w == CLOCK_WIDTH && h == CLOCK_HEIGHT) {
            clockFaceRlePath = "/face_default.bmp";
        }
        if (clockFaceRlePath.isEmpty()) {
            DEBUG_PRINTLN("[Display] No usable clock face file - drawing the generated one");
            return;
        }

        const size_t RLE_RAM_MAX = 48 * 1024;      // groessere Dateien (Fotos) bleiben in der Datei
                                                   // larger files (photos) stay in the file
        const size_t HEAP_RESERVE = 96 * 1024;     // bleibt fuer WLAN, Webserver und Dateizugriffe frei
                                                   // stays free for WiFi, web server and file access
        File f = LittleFS.open(clockFaceRlePath, "r");
        uint8_t head[20];
        if (!f || f.read(head, sizeof(head)) != sizeof(head)) {
            if (f) f.close();
            return;
        }
        size_t size = *(uint32_t*)&head[12];
        if (size <= RLE_RAM_MAX && ESP.getMaxAllocHeap() >= size + HEAP_RESERVE) {
            clockFaceRle = (uint8_t*)malloc(size);
            if (clockFaceRle && f.read(clockFaceRle, size) == size) {
                clockFaceRleSize = size;
            }
            else if (clockFaceRle) {
                free(clockFaceRle);
                clockFaceRle = nullptr;
            }
        }
        f.close();
        DEBUG_PRINTLN("[Display] Low-memory clock face: " + clockFaceRlePath + " (" + String(size) + " bytes, " +
                      (clockFaceRle ? "in RAM" : "read from file") + ")");
    }

    // Zeichnet das Zifferblatt im Sparmodus zeilenweise in 'dest', mit Helligkeit. Software-Rotation gibt es
    // ohne PSRAM nicht (gc9d01SwRotation), daher immer ungedreht.

    // Draws the clock face row by row into 'dest' in low-memory mode, with brightness. Without PSRAM there is
    // no software rotation (gc9d01SwRotation), so always unrotated.

    void drawFaceRows(LGFX_Sprite& dest) {
        const int N = CLOCK_WIDTH;
        if (clockFaceRlePath.isEmpty()) {
            DefaultFaceGen gen;
            gen.init(FACE_NUMERALS_QUARTER);
            for (int y = 0; y < N; y++) {
                gen.row(y, rowBuffer);
                for (int x = 0; x < N; x++) rowBuffer[x] = setPixelBrightness(rowBuffer[x]);
                dest.pushImage(0, y, N, 1, rowBuffer);
            }
            return;
        }
        RleFaceReader r;
        if (clockFaceRle) {
            r.mem = clockFaceRle;
            r.remaining = clockFaceRleSize;
        }
        else {
            r.file = LittleFS.open(clockFaceRlePath, "r");
            uint8_t head[20];
            if (r.file && r.file.read(head, sizeof(head)) == sizeof(head)) r.remaining = *(uint32_t*)&head[12];
        }
        for (int y = 0; y < CLOCK_HEIGHT; y++) {
            for (int x = 0; x < N; x++) {
                uint16_t px = 0xFFFF;   // weiss, falls die Daten zu kurz sind / white if the data is too short
                r.nextPixel(px);
                rowBuffer[x] = setPixelBrightness(px);
            }
            dest.pushImage(0, y, N, 1, rowBuffer);
        }
        if (r.file) r.file.close();
    }

    // Laedt das gewaehlte Zifferblatt (BMP/RLEB, sonst Standard) in clockFaceBuffer, wendet die Helligkeit an
    // (clockFaceBrightBuffer) und zeichnet in backgroundSprite. Rueckgabe: 2 = beide Puffer ok, 1 = nur
    // Rohbild, 0 = nichts, 3 = Sparmodus ohne PSRAM (drawFaceRows()).

    // Loads the selected clock face (BMP/RLEB, else default) into clockFaceBuffer, applies brightness
    // (clockFaceBrightBuffer) and draws into backgroundSprite. Returns: 2 = both buffers ok, 1 = raw only, 0
    // = neither, 3 = low-memory mode without PSRAM (drawFaceRows()).

    int prepareClockFaceCache() {
        if (lowMemoryFace()) {
            prepareClockFaceRle();
            return 3;
        }

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

            // Bild aus Datei laden (Standard-BMP oder RLEB), sonst das Standard-Zifferblatt
            // Load the image from file (standard BMP or RLEB), otherwise the default clock face

            loadFaceOrDefault(selectedBackground, clockFaceBuffer);
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

        // Die teure Helligkeitsanpassung Pixel fuer Pixel nur bei Aenderung (neues Zifferblatt, Helligkeit).
        // Mit Backlight aendert die Helligkeit keine Pixel - dann nur bei neuem Zifferblatt bzw. Umschalten
        // (forceRecompute).

        // Run the costly per-pixel brightness adjustment only on a change (new clock face, brightness). With
        // a backlight, brightness changes no pixels - then only on a new clock face or when switching
        // (forceRecompute).

        if (forceRecompute || (!useBacklight && currentBrightness != lastAppliedBrightness)) {
            for (int i = 0; i < CLOCK_WIDTH * CLOCK_HEIGHT; i++) {
                clockFaceBrightBuffer[i] = setPixelBrightness(clockFaceBuffer[i]);
            }
            lastAppliedBrightness = currentBrightness;
        }

        // GC9D01 mit PSRAM hat keine Hardware-Rotation: Zeiger (rotatedAngle()) und hier auch das Zifferblatt
        // werden per Software gedreht. 'rotation' ist die Zielausrichtung dieses Displays; bei
        // Hardware-Rotation gilt faceOrientation=0.

        // GC9D01 with PSRAM has no hardware rotation: hands (rotatedAngle()) and here the clock face too are
        // rotated in software. 'rotation' is this display's target orientation; with hardware rotation
        // faceOrientation=0 applies.

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


    // Zeichnet das Zifferblatt ins backgroundSprite.
    // Draws the clock face into backgroundSprite.

    void loadClockFace(uint8_t rotation) {
        int cacheState = prepareClockFaceCache();
        if (cacheState == 0) return;
        if (cacheState == 3) {
            drawFaceRows(backgroundSprite);
            return;
        }

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
        if (clockFaceRle) {
            free(clockFaceRle);
            clockFaceRle = nullptr;
        }
        clockFaceRleSize = 0;
        clockFaceRleReady = false;
    }


    // Loescht alle Zifferblaetter (face_*.bmp) - die drei erzeugten Zifferblaetter entstehen danach neu
    // (ensureDefaultFace()). Raeumt verwaiste Presets auf und schaltet auf den Standard zurueck.

    // Deletes all clock faces (face_*.bmp) - the three generated clock faces are created again afterwards
    // (ensureDefaultFace()). Cleans up orphaned presets and falls back to the default.

    void resetFacesToDefault() {
        std::vector<String> toDelete;
        File root = LittleFS.open("/");
        File file = root.openNextFile();
        while (file) {
            String name = file.name();
            if (!file.isDirectory() && name.startsWith("face_") && name.endsWith(".bmp") && !isProtectedFile(name)) {
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


    // Loescht alle Zeigersaetze (hand_set*.bmp) - der Standardsatz 0 wird danach neu erzeugt. Raeumt
    // verwaiste Presets auf und schaltet auf den Standardsatz zurueck.

    // Deletes all hand sets (hand_set*.bmp) - the default set 0 is created again afterwards. Cleans up
    // orphaned presets and falls back to the default set.

    void resetHandsToDefault() {
        std::vector<String> toDelete;
        std::set<String> setIds;
        File root = LittleFS.open("/");
        File file = root.openNextFile();
        while (file) {
            String name = file.name();
            if (!file.isDirectory() && name.startsWith("hand_set") && name.endsWith(".bmp") && !isProtectedFile(name)) {
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

        // Ein Puffer fuer alle drei Zeiger: Datei (neues oder altes Format, sonst Standardsatz 0), die alte Hoehe
        // jeweils oben transparent aufgefuellt.

        // One buffer for all three hands: file (new or old format, otherwise default set 0), the old height padded
        // transparent at the top in either case.

        uint16_t* pix = (uint16_t*)preferPsramMalloc((size_t)HAND_WIDTH * HAND_HEIGHT * sizeof(uint16_t));
        if (!pix) {
            DEBUG_PRINTLN("[HANDS] Error: couldnt allocate hand buffer");
            return;
        }

        struct HandConfig {
            const char* label;
            LGFX_Sprite* sprite;
        } hands[3] = {
            {"hour", &hourHandSprite},
            {"minute", &minuteHandSprite},
            {"second", &secondHandSprite}
        };

        for (auto& h : hands) {
            loadHandOrDefault(setId, h.label, pix);

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

    // Grosse Spruenge (erste Zeit, Korrektur, Rocrail): Zeiger laufen in HAND_MOVE_MS auf dem kuerzesten Weg,
    // auch rueckwaerts, mit sanftem Anfahren/Bremsen. Der Abstand zum Ziel schrumpft per Smoothstep; bewegt
    // sich das Ziel (z.B. NTP-Korrektur), fliesst das in den Restweg ein, statt dass der Zeiger springt.

    // Large jumps (first time, correction, Rocrail): the hands move in HAND_MOVE_MS along the shortest path,
    // also backwards, easing in and out. The distance to the target shrinks via smoothstep; if the target
    // moves (e.g. an NTP correction), that is folded into the remaining way instead of making the hand jump.

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

    // Glaettung des Sekundenzeigers im Normalmodus: faengt einen sichtbaren Ruecksprung ab, wenn die Zeit
    // selbst rueckwaerts korrigiert wird (NTP/RTC/DCF77). Die Bahnhofsuhr-Logik (stationTick) ist schon
    // selbst abgesichert.

    // Second-hand smoothing in normal mode: catches a visible jump-back when the time itself is corrected
    // backward (NTP/RTC/DCF77). The station-clock logic (stationTick) is already guarded on its own.

    static float lastSecondAngle = 0.0f;
    static float lastSecondAngle2 = 0.0f;

    // Rendert EIN Frame (Zifferblatt, Zeiger, Nabe) fuer das gewaehlte Display, mit je eigener Glaettung. Das
    // Zwischenbild: gedrehtes Zifferblatt mit Stunden- und Minutenzeiger per pushRotatedWithAA() (wie der
    // Sekundenzeiger).

    // Renders ONE frame (clock face, hands, hub) for the selected display, each with its own smoothing. The
    // composite: rotated clock face with hour and minute hand via pushRotatedWithAA() (like the second hand).

    bool buildHandComposite(HandComposite& comp, uint8_t rotation, float hourAngle, float minuteAngle, bool hands) {
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

                // Einmal melden und danach dauerhaft je Tick ohne Composite-Sprite zeichnen, statt es bei
                // jedem Tick erneut zu versuchen.

                // Report once and then permanently render per tick without the composite sprite instead of
                // retrying on every tick.

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

        // Drehpunkt = Pivot des Zwischenbilds (Mitte, wie backgroundSprite). Ohne hands (Sekundenfeld) nur das
        // Zifferblatt - Stunden- und Minutenzeiger zeichnet renderClockFrame() dann ueber den Sekundenzeiger.

        // pivot = the composite's pivot (centre, like backgroundSprite). Without hands (seconds subdial) only the
        // clock face - renderClockFrame() then draws hour and minute hand over the second hand.

        comp.hands = hands;
        if (hands) {
            hourHandSprite.pushRotatedWithAA(comp.sprite, hourAngle, TRANSPARENT_COLOR);
            minuteHandSprite.pushRotatedWithAA(comp.sprite, minuteAngle, TRANSPARENT_COLOR);
        }

        // Nur als gueltig markieren, wenn die Zeiger wirklich drin sind - sonst
        // (Speichermangel bei den Zeiger-Sprites) lieber naechsten Tick erneut
        // versuchen, statt ein Bild ohne Zeiger dauerhaft festzuhalten.

        // Only mark it valid if the hands are really in it - otherwise (hand
        // sprite allocation failed) better retry next tick than keep an image
        // without hands permanently.

        if (hands && (hourHandSprite.width() <= 0 || minuteHandSprite.width() <= 0)) {
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

    bool drawCompositeInto(uint8_t displayNum, uint8_t rotation, float hourAngle, float minuteAngle, bool hands) {
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
            || comp.hands != hands
            || (hands && fabsf(shortestAngleDiff(comp.hourAngle, hourAngle)) >= COMPOSITE_ANGLE_EPS)
            || (hands && fabsf(shortestAngleDiff(comp.minuteAngle, minuteAngle)) >= COMPOSITE_ANGLE_EPS);

        if (needsRebuild) {
            if (!buildHandComposite(comp, rotation, hourAngle, minuteAngle, hands)) return false;
        }
        else if (!comp.sprite) {
            return false;
        }

        comp.sprite->pushSprite(&backgroundSprite, 0, 0);
        return true;
    }


    // Erweitert das Rechteck um die Flaeche eines per pushRotated*() gezeichneten Zeigers - gleiche Abbildung
    // wie LovyanGFX (Pixelmitte +0,5, Drehmatrix), plus 2 px Rand fuer die geglaetteten Kanten.

    // Extends the rectangle by the area of a hand drawn via pushRotated*() - same mapping as LovyanGFX (pixel
    // centre +0.5, rotation matrix), plus 2 px margin for the anti-aliased edges.

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


    // Sendet nur das Rechteck (x,y,w,h) aus backgroundSprite, zeilenweise. Setzt SPI ohne DMA-Kanal voraus -
    // sonst leitet LovyanGFX Zeilen von 65..1023 Byte auf DMA um, das es auf dem ESP32-S2 still verwirft.

    // Sends only the rectangle (x,y,w,h) of backgroundSprite, row by row. Requires SPI without a DMA channel
    // - otherwise LovyanGFX reroutes rows of 65..1023 bytes to DMA, which it silently drops on the ESP32-S2.

    void pushBackgroundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t ox, int32_t oy) {
        const lgfx::swap565_t* buf = (const lgfx::swap565_t*)backgroundSprite.getBuffer();
        const int32_t stride = backgroundSprite.width();
        if (!buf || w <= 0 || h <= 0) return;
        tft.startWrite();
        tft.setAddrWindow(x + ox, y + oy, w, h); // ox/oy = Lage der Uhr (clockOrigin())
                                                 // ox/oy = position of the clock (clockOrigin())
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

        ensureFaceSettings();
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

        // Bahnhofsuhr "wartet auf 12" (stationMode): der Sekundenzeiger laeuft in ~58,5 s um und wartet oben
        // auf den Minutenwechsel - egal ob schwingend oder tickend (smoothSecond). Im Rocrail-Modus wird die
        // Schrittdauer durch den Divider geteilt.

        // Station clock "waits at 12" (stationMode): the second hand laps in ~58.5 s and waits at the top for
        // the minute change - whether smooth or ticking (smoothSecond). In Rocrail mode the step duration is
        // divided by the divider.

        bool waitAtTwelve = stationMode;

        // Schrittdauer der Sweep-Animation: im Rocrail-Modus durch den Divider geteilt, sonst FAST_SECOND -
        // als float, da sich der Divider pro <clock>-Update aendern kann.

        // Step duration of the sweep animation: divided by the divider in Rocrail mode, otherwise FAST_SECOND
        // - as a float, since the divider can change with each <clock> update.

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

        // Den Sekundenzeiger (nicht die Nabe) ausblenden, wenn Rocrail beschleunigt (Divider > 1) UND tickend
        // dargestellt wird - springend wirkt er dann unruhig, schwingend bleibt er bei jeder Geschwindigkeit
        // sauber.

        // Hide the second hand (not the hub) when Rocrail accelerates (divider > 1) AND it is rendered
        // ticking - jumping it looks jittery then, while the smooth style stays clean at any speed.

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

        // Noch keine Zeit (nach dem Start ohne NTP, RTC, DCF77 oder Rocrail, Systemzeit 1970): alle Zeiger auf
        // 12. Mit der ersten Zeit wie beim ersten Bild neu ausrichten - die Zeiger laufen dann zur Uhrzeit.

        // No time yet (after the start without NTP, RTC, DCF77 or Rocrail, system time 1970): all hands at 12.
        // With the first time realign as on the first frame - the hands then run to the time.

        static bool waitingForTime[2] = { false, false };
        const bool noTimeYet = !rocrailTimeReady && t.tm_year < 100;
        if (!noTimeYet && waitingForTime[displayNum - 1]) {
            waitingForTime[displayNum - 1] = false;
            firstRunRef = true;
            forceRender = true;
        }

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

            // Hier bewusst NICHT zeichnen - das Bild entsteht weiter unten im selben Durchlauf (forceRender),
            // sonst blitzen die Zeiger beim Zeitempfang kurz doppelt auf.

            // Deliberately do NOT draw here - the frame is produced further below in the same pass
            // (forceRender), otherwise the hands flash doubled for a moment on receiving the time.

        }

        // "Wartet auf 12": der Sekundenzeiger macht 60 Schritte zu je stationStepMs (schwingend per
        // easeInOutSine() oder tickend), erreicht nach ~58,5 s (Rocrail: /Divider) die 12 und wartet dort bis
        // zum (Modell-)Minutenwechsel.

        // "Waits at 12": the second hand makes 60 steps of stationStepMs each (smooth via easeInOutSine() or
        // ticking), reaches the top after ~58.5 s (Rocrail: /divider) and waits there until the (model)
        // minute changes.

        if (waitAtTwelve) {

            // Bei Divider 1 gilt die normale schrittweise Bahnhofsuhr-Logik unten (Modellzeit laeuft 1:1).
            // Erst ab Divider > 1 wird die Position direkt aus rocrailSecFrac abgeleitet; smoothSecond waehlt
            // stufenlos oder auf Schritte gerundet.

            // At divider 1 the normal stepwise station-clock logic below applies (model time runs 1:1). Only
            // from divider > 1 is the position derived directly from rocrailSecFrac; smoothSecond chooses
            // continuous or rounded to steps.

            if (rocrailTimeReady && rocrailDivider > 1) {

                // Rocrail: glatt statt tickend - kein easeInOutSine() pro
                // Schritt noetig. rocrailSecFrac (advanceRocrailTime()) traegt
                // die Nachkommastellen, dieselbe Positions-Formel wie oben deckelt bei 60.

                // Rocrail: smooth instead of ticking - no per-step
                // easeInOutSine() needed. rocrailSecFrac (advanceRocrailTime())
                // carries the fractional part, the same position formula as above caps at 60.

                float smoothPos = (rocrailSecFrac * 1000.0f) / FAST_SECOND;
                if (smoothPos > 60.0f) smoothPos = 60.0f;

                // smoothSecond trennt "wartet auf 12" von der Darstellung: tickend auf den ganzen Schritt
                // abrunden statt die gleitende Position zu uebernehmen.

                // smoothSecond separates "waits at 12" from the rendering: when ticking, round down to the
                // whole step instead of taking over the gliding position.

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

                // Aufwachen beim Minutenwechsel statt exakt bei "Sekunde 0" - ein blockierender
                // loadClockFace() kann Sekunde 0 verpassen. Danach die Sollposition, die die Uhrzeit gerade
                // verlangt.

                // Wake up on a minute change instead of exactly "second 0" - a blocking loadClockFace() can
                // miss second 0. Then the target position the current time is asking for.

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

            // Schwingend bewusst NICHT gleichfoermig: easeInOutSine() beschleunigt und bremst je Schritt wie
            // alte Bahnhofsuhren. Tickend haelt der Zeiger exakt auf der ganzen Position bis zum naechsten
            // Schritt.

            // Smooth style deliberately NOT uniform: easeInOutSine() accelerates and brakes each step like
            // old station clocks. When ticking, the hand holds exactly at the whole position until the next
            // step.

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

        // Normalmodus: der Sekundenzeiger laeuft mit der echten (Rocrail: Modell-)Sekunde, ohne Pause oben.
        // Schwingend stufenlos innerhalb der Sekunde, tickend ein Sprung pro Sekunde; der Minutenzeiger kann
        // optional sanft laufen.

        // Normal mode: the second hand keeps pace with the real (Rocrail: model) second, without a pause at
        // the top. Smooth glides within the second, ticking jumps once per second; the minute hand can
        // optionally run smoothly.

        if (!waitAtTwelve) {

            // Sekundenbruchteil fuer sanfte Bewegung aus dem zuletzt beobachteten Sekundenwechsel statt
            // millis()%1000 - so endet die Bewegung genau beim Weiterspringen von t.tm_sec. Mit Rocrail kommt
            // der Bruchteil schon aus rocrailSecFrac.

            // Sub-second fraction for smooth motion from the last observed second change instead of
            // millis()%1000 - so the motion ends exactly when t.tm_sec advances. With Rocrail the fraction
            // already comes from rocrailSecFrac.

            static long lastWholeSecondValue = -1;
            static unsigned long secondBoundaryMillis = 0;
            float realSubSecond = 0.0f;

            // Nur pflegen, wenn gebraucht - bei aktiver Rocrail-Modellzeit kommt der Bruchteil aus
            // rocrailSecFrac.

            // Only maintained when needed - with active Rocrail model time the fraction comes from
            // rocrailSecFrac.

            if (!rocrailTimeReady) {
                long currentWholeSecond = (long)t.tm_hour * 3600 + (long)t.tm_min * 60 + t.tm_sec;
                if (currentWholeSecond != lastWholeSecondValue) {
                    lastWholeSecondValue = currentWholeSecond;
                    secondBoundaryMillis = currentMillis;
                }
                realSubSecond = (currentMillis - secondBoundaryMillis) / 1000.0f;

                // Deckeln statt ueberlaufen: bei einem verzoegerten Frame waere sonst kurz eine Bewegung
                // ueber die naechste Sekunde hinaus sichtbar, bevor t.tm_sec nachzieht.

                // Clamp instead of overshooting: on a delayed frame motion past the next second would
                // otherwise be briefly visible before t.tm_sec catches up.

                if (realSubSecond < 0.0f) realSubSecond = 0.0f;
                else if (realSubSecond > 0.999f) realSubSecond = 0.999f;
            }

            // rocrailSecFrac enthaelt Ganzzahl und Bruchteil - minus t.tm_sec ergibt genau den
            // Divider-skalierten Bruchteil, analog zu realSubSecond.

            // rocrailSecFrac holds the whole and fractional part - minus t.tm_sec yields exactly the
            // divider-scaled fraction, analogous to realSubSecond.

            float secondFraction = rocrailTimeReady ? (rocrailSecFrac - (float)t.tm_sec) : realSubSecond;

            float targetSecAngle;
            if (smoothSecond) {
                float smoothSecondValue = (float)t.tm_sec + secondFraction;
                targetSecAngle = rotatedAngle(smoothSecondValue * 6.0f, orientation);
            }
            else {
                targetSecAngle = rotatedAngle(secAngle, orientation);
            }

            // Sichtbaren Sprung abfedern: rueckwaerts (Zeitkorrektur) oder ungewoehnlich weit vorwaerts
            // (verzoegerter Frame); normale Ticks gelten sofort. Das Log vermerkt Beginn und Ende jeder
            // Episode zur Ursachensuche.

            // Ease away a visible jump: backward (time correction) or unusually far forward (delayed frame);
            // normal ticks apply immediately. The log records start and end of each episode to find the
            // cause.

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

        if (noTimeYet) {
            waitingForTime[displayNum - 1] = true;
            hourAngle = minAngle = secAngle = rotatedAngle(0.0f, orientation);
            lastHourAngleRef = lastMinuteAngleRef = lastSecondAngleRef = hourAngle;
        }

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


        // Zifferblatt und Stunden-/Minutenzeiger kommen aus dem Zwischenbild (drawCompositeInto()), sonst
        // alter Zeichenweg. Bewegte sich nur der Sekundenzeiger, nur das Rechteck um alten und neuen Zeiger
        // samt Nabe senden - spart beim schwingenden Zeiger den Grossteil der 115 KB je Frame.

        // Clock face and hour/minute hands come from the composite (drawCompositeInto()), otherwise the old
        // drawing path. If only the second hand moved, send just the rectangle around old and new hand plus
        // hub - saves most of the 115 KB per frame with the smooth hand.

        const uint32_t renderStartMicros = micros(); // fuer recordRenderFrame() / for recordRenderFrame()

        // Stunden-/Minutenwinkel bewusst NICHT vergleichen - beim schwingenden Minutenzeiger aendert er sich
        // jedes Frame minimal; ein neues Zwischenbild faengt die compositeBuildCount-Pruefung ab.

        // Hour/minute angles deliberately NOT compared - with the smooth minute hand they change minimally
        // every frame; a rebuilt composite is caught by the compositeBuildCount check.

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

        // Sekundenfeld (Drehpunkt nicht in der Mitte): der Sekundenzeiger liegt unter Stunden- und Minutenzeiger.
        // Das Zwischenbild ist dann nur das Zifferblatt, die grossen Zeiger kommen je Bild darueber - im
        // Rechteck um den Sekundenzeiger mit den Winkeln des letzten vollen Bildes, damit es zum Rest passt.

        // Seconds subdial (pivot not in the centre): the second hand lies below the hour and minute hand. The
        // composite is then the clock face only, the large hands go over it each frame - in the rectangle around
        // the second hand with the angles of the last full frame, so that it matches the rest.

        float subX = 0, subY = 0;
        const bool subdial = drawSecond && secondPivotAt(rotation, subX, subY);
        const float centreX = backgroundSprite.getPivotX(), centreY = backgroundSprite.getPivotY();
        if (subdial && partial) {
            partial = fabsf(shortestAngleDiff(lastFrame.hourAngle, hourAngle)) < 0.12f &&
                      fabsf(shortestAngleDiff(lastFrame.minuteAngle, minAngle)) < 0.12f;
        }
        const float drawHourAngle = (subdial && partial) ? lastFrame.hourAngle : hourAngle;
        const float drawMinAngle = (subdial && partial) ? lastFrame.minuteAngle : minAngle;

        int32_t px0 = INT32_MAX, py0 = INT32_MAX, px1 = INT32_MIN, py1 = INT32_MIN;
        if (partial) {
            if (subdial) backgroundSprite.setPivot(subX, subY);
            addRotatedSpriteBounds(secondHandSprite, lastFrame.secondAngle, px0, py0, px1, py1);
            addRotatedSpriteBounds(secondHandSprite, secAngle, px0, py0, px1, py1);
            backgroundSprite.setPivot(centreX, centreY);
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
        bool compositeOk = drawCompositeInto(displayNum, rotation, hourAngle, minAngle, !subdial);

        // Wurde das Zwischenbild neu aufgebaut oder fehlt es (Speichermangel), hat sich das Bild auch
        // ausserhalb des Rechtecks geaendert - dann das Frame voll zeichnen und senden.

        // If the composite was rebuilt or is missing (low memory), the image also changed outside the
        // rectangle - then draw and send the frame in full.

        if (partial && (compositeBuildCount != buildsBefore || !compositeOk)) {
            partial = false;
            backgroundSprite.clearClipRect();
            if (compositeOk) compositeOk = drawCompositeInto(displayNum, rotation, hourAngle, minAngle, !subdial);
        }
        if (!compositeOk) loadClockFace(rotation);
        if (subdial) {
            backgroundSprite.setPivot(subX, subY);
            secondHandSprite.pushRotatedWithAA(&backgroundSprite, secAngle, TRANSPARENT_COLOR);
            backgroundSprite.setPivot(centreX, centreY);
        }
        if (!compositeOk || subdial) {
            hourHandSprite.pushRotatedWithAA(&backgroundSprite, drawHourAngle, TRANSPARENT_COLOR);
            minuteHandSprite.pushRotatedWithAA(&backgroundSprite, drawMinAngle, TRANSPARENT_COLOR);
        }

        if (drawSecond && !subdial) {

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
            int ox, oy;
            clockOrigin(displayNum, ox, oy);
            pushBackgroundRect(px0, py0, px1 - px0 + 1, py1 - py0 + 1, ox, oy);
            lastRenderRect[0] = px0; lastRenderRect[1] = py0;
            lastRenderRect[2] = px1 - px0 + 1; lastRenderRect[3] = py1 - py0 + 1;
        }
        else {
            int ox, oy;
            clockOrigin(displayNum, ox, oy);
            backgroundSprite.pushSprite(ox, oy);
        }
        lastRenderPartial = partial;
        recordRenderFrame(micros() - renderStartMicros, partial);

        lastFrame.valid = true;
        lastFrame.hourAngle = drawHourAngle;
        lastFrame.minuteAngle = drawMinAngle;
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


    // Schriftarten fuer den Streifen mit Uhrzeit/Datum, Auswahl im Zifferblatt-Designer (stripFont). size =
    // Vergroesserung, nur die GLCD-Schrift wird skaliert.

    // Fonts for the time/date strip, chosen in the clock face designer (stripFont). size = magnification, only
    // the GLCD font is scaled.

    struct StripFont {
        const char* name;
        const lgfx::IFont* time;
        const lgfx::IFont* date;
        uint8_t size;
    };
    const StripFont STRIP_FONTS[] = {
        { "GLCD", &fonts::Font0, &fonts::Font0, 3 },
        { "7-Segment / DejaVu", &fonts::Font7, &fonts::DejaVu18, 1 },
        { "FreeSans Bold", &fonts::FreeSansBold24pt7b, &fonts::FreeSansBold12pt7b, 1 },
        { "DejaVu", &fonts::DejaVu40, &fonts::DejaVu18, 1 },
        { "Orbitron", &fonts::Orbitron_Light_32, &fonts::Orbitron_Light_24, 1 },
    };
    constexpr uint8_t STRIP_FONT_COUNT = sizeof(STRIP_FONTS) / sizeof(STRIP_FONTS[0]);
    constexpr uint8_t STRIP_FONT_VLW = 255; // VLW-Schriften aus dem Designer (kantengeglaettet)
                                            // VLW fonts from the designer (anti-aliased)
    constexpr uint8_t STRIP_DATE_FMT_COUNT = 7;
    constexpr long STRIP_SCALE_MIN = 50;  // Groesse der eingebauten Schrift in % / size of the built-in font in %
    constexpr long STRIP_SCALE_MAX = 200;


    // Streifen-Einstellungen gehoeren zum Zifferblatt: /stripcfg_<Name>.txt zu /face_<Name>.bmp, Inhalt wie die
    // Streifen-Schluessel frueherer Presets ("stripBg=ffffff&stripFont=2&..."). Ohne Datei gilt der Standard.

    // Strip settings belong to the clock face: /stripcfg_<name>.txt for /face_<name>.bmp, content like the strip
    // keys of earlier presets ("stripBg=ffffff&stripFont=2&..."). Without a file the default applies.

    String faceSidePath(const String& facePath, const char* prefix) {
        String name = facePath.startsWith("/") ? facePath.substring(1) : facePath;
        if (!name.startsWith("face_") || !name.endsWith(".bmp") || name.indexOf('/') >= 0 || name.indexOf("..") >= 0) return "";
        return "/" + String(prefix) + name.substring(5, name.length() - 4) + ".txt";
    }

    String stripConfigPath(const String& facePath) { return faceSidePath(facePath, "stripcfg_"); }

    // Weitere Einstellungen des Zifferblatts auf allen Displays: /facecfg_<Name>.txt, "secX=..&secY=.." =
    // Drehpunkt des Sekundenzeigers in Zifferblatt-Pixeln (-1 = Mitte). Ohne Datei: Mitte.

    // Further settings of the clock face on all displays: /facecfg_<name>.txt, "secX=..&secY=.." = pivot of the
    // second hand in clock face pixels (-1 = centre). Without a file: centre.

    String faceConfigPath(const String& facePath) { return faceSidePath(facePath, "facecfg_"); }

    void readFaceSecPivot(const String& facePath, int16_t& x, int16_t& y) {
        x = y = -1;
        String path = faceConfigPath(facePath);
        if (!path.length() || !LittleFS.exists(path)) return;
        File f = LittleFS.open(path, "r");
        if (!f) return;
        String text = f.size() < 512 ? f.readString() : String("");
        f.close();
        int px = text.indexOf("secX="), py = text.indexOf("secY=");
        if (px < 0 || py < 0) return;
        x = (int16_t)constrain(text.substring(px + 5).toInt(), -1L, (long)CLOCK_WIDTH - 1);
        y = (int16_t)constrain(text.substring(py + 5).toInt(), -1L, (long)CLOCK_HEIGHT - 1);
        if (x < 0 || y < 0) x = y = -1;
    }

    bool saveFaceSettings(const String& facePath, int16_t x, int16_t y) {
        String path = faceConfigPath(facePath);
        if (!path.length()) return false;
        if (x < 0 || y < 0) {
            LittleFS.remove(path); // Mitte = Werkseinstellung, keine Datei / centre = default, no file
            return true;
        }
        File f = LittleFS.open(path, "w");
        if (!f) return false;
        f.print("secX=" + String(x) + "&secY=" + String(y));
        f.close();
        return true;
    }

    // Einstellungen des aktiven Zifferblatts laden, sobald es wechselt (wie ensureStripSettings())
    // Load the active clock face's settings as soon as it changes (like ensureStripSettings())

    void ensureFaceSettings() {
        if (faceSettingsFor == selectedBackground) return;
        faceSettingsFor = selectedBackground;
        readFaceSecPivot(selectedBackground, secPivotX, secPivotY);
        clockFrameDirty[0] = clockFrameDirty[1] = true;
    }

    // Drehpunkt des Sekundenzeigers auf dem Display: Zifferblatt-Pixel, mit der Software-Rotation gedreht wie
    // blitFaceIntoSprite(). false = Mitte.

    // Pivot of the second hand on the display: clock face pixels, turned with the software rotation like
    // blitFaceIntoSprite(). false = centre.

    bool secondPivotAt(uint8_t rotation, float& x, float& y) {
        if (secPivotX < 0 || secPivotY < 0) return false;
        const int n = CLOCK_WIDTH - 1;
        switch (faceOrientationFor(rotation)) {
            case 1:  x = n - secPivotY; y = secPivotX;     break;
            case 2:  x = n - secPivotX; y = n - secPivotY; break;
            case 3:  x = secPivotY;     y = n - secPivotX; break;
            default: x = secPivotX;     y = secPivotY;     break;
        }
        return true;
    }

    // Standard-Streifen: weiss mit schwarzer Schrift FreeSans Bold, Uhrzeit und Datum automatisch, kein Wochentag
    // Default strip: white with black text FreeSans Bold, time and date automatic, no weekday

    void resetStripSettings() {
        stripBgRgb = 0xFFFFFF;
        stripFgRgb = 0x000000;
        stripFont = 0;
        for (uint8_t f = 0; f < STRIP_FONT_COUNT; f++) {
            if (strcmp(STRIP_FONTS[f].name, "FreeSans Bold") == 0) stripFont = f;
        }
        stripTimeFmt = stripSeconds = stripDateFmt = 0;
        stripVlwName = "";
        stripVlwTimeSize = 44;
        stripVlwDateSize = stripVlwWeekdaySize = 22;
        stripTimeScale = stripDateScale = stripWeekdayScale = 100;
        stripShowTime = stripShowDate = true;
        stripShowWeekday = false;
        stripBlink = true;
        stripBefore = false;
        stripTimeX = stripTimeY = stripDateX = stripDateY = stripWeekdayX = stripWeekdayY = -1;
    }

    // Aktuelle Streifen-Einstellungen als Text (Schluessel wie applyStripValue())
    // Current strip settings as text (keys as in applyStripValue())

    String stripSettingsQuery() {
        return "stripBefore=" + String(stripBefore ? 1 : 0) + "&stripBg=" + String(stripBgRgb, HEX) +
               "&stripFg=" + String(stripFgRgb, HEX) + "&stripFont=" + String(stripFont) +
               "&stripVlw=" + presetUrlEncode(stripVlwName) + "&stripVt=" + String(stripVlwTimeSize) +
               "&stripVd=" + String(stripVlwDateSize) + "&stripVw=" + String(stripVlwWeekdaySize) +
               "&stripTs=" + String(stripTimeScale) + "&stripDs=" + String(stripDateScale) + "&stripWs=" + String(stripWeekdayScale) +
               "&stripShowT=" + String(stripShowTime ? 1 : 0) + "&stripShowD=" + String(stripShowDate ? 1 : 0) +
               "&stripShowW=" + String(stripShowWeekday ? 1 : 0) + "&stripTfmt=" + String(stripTimeFmt) +
               "&stripSec=" + String(stripSeconds) + "&stripDfmt=" + String(stripDateFmt) + "&stripBlink=" + String(stripBlink ? 1 : 0) +
               "&stripTx=" + String(stripTimeX) + "&stripTy=" + String(stripTimeY) + "&stripDx=" + String(stripDateX) +
               "&stripDy=" + String(stripDateY) + "&stripWx=" + String(stripWeekdayX) + "&stripWy=" + String(stripWeekdayY);
    }

    // Schluessel=Wert-Paare (getrennt mit '&', ab einem '?') uebernehmen - fremde Schluessel bleiben unbeachtet
    // Take over key=value pairs (separated by '&', after a '?') - other keys are ignored

    void applyStripQuery(const String& text) {
        int q = text.indexOf('?');
        String query = (q >= 0) ? text.substring(q + 1) : text;
        query.trim();
        while (query.length()) {
            int amp = query.indexOf('&');
            String param = (amp < 0) ? query : query.substring(0, amp);
            query = (amp < 0) ? "" : query.substring(amp + 1);
            int eq = param.indexOf('=');
            if (eq > 0) applyStripValue(param.substring(0, eq), presetUrlDecode(param.substring(eq + 1)));
        }
    }

    // Einstellungen eines Zifferblatts in die globalen Werte laden (ohne Displays zu loeschen); true = Datei da
    // Load a clock face's settings into the global values (without clearing the displays); true = file present

    bool loadStripSettingsForFace(const String& facePath) {
        resetStripSettings();
        String path = stripConfigPath(facePath);
        if (!path.length() || !LittleFS.exists(path)) return false;
        File f = LittleFS.open(path, "r");
        if (!f) return false;
        String text = f.size() < 2048 ? f.readString() : String("");
        f.close();
        applyStripQuery(text);
        return true;
    }

    bool saveStripSettingsForFace(const String& facePath) {
        String path = stripConfigPath(facePath);
        if (!path.length()) return false;
        File f = LittleFS.open(path, "w");
        if (!f) return false;
        f.print(stripSettingsQuery());
        f.close();
        return true;
    }

    // Einstellungen des aktiven Zifferblatts laden, sobald es wechselt (stripSettingsFor, "?" = neu laden). Eine
    // andere Lage loescht die Displays einmal (setStripBefore()).

    // Load the active clock face's settings as soon as it changes (stripSettingsFor, "?" = reload). Another
    // placement clears the displays once (setStripBefore()).

    void ensureStripSettings() {
        if (TFT_HEIGHT <= CLOCK_HEIGHT || stripSettingsFor == selectedBackground) return;
        stripSettingsFor = selectedBackground;
        bool before = stripBefore;
        loadStripSettingsForFace(selectedBackground);
        bool wanted = stripBefore;
        stripBefore = before;
        setStripBefore(wanted);
        infoStripDirty[0] = infoStripDirty[1] = true;
    }

    // Naechstliegende vorhandene Groesse der VLW-Schrift stripVlwName (Datei auf der Uhr), sonst target
    // Nearest available size of the VLW font stripVlwName (file on the clock), otherwise target

    uint8_t nearestStripVlwSize(uint8_t target, bool weekday) {
        int best = -1;
        for (int n = 8; n <= 120; n++) {
            if (!LittleFS.exists(stripVlwPath(n, weekday))) continue;
            if (best < 0 || abs(n - target) < abs(best - target)) best = n;
        }
        return best < 0 ? target : (uint8_t)best;
    }

    // Streifen-Einstellungen aus der Sicherung eines anderen Displaytyps auf diesen Streifen umrechnen: Positionen
    // im Verhaeltnis von Breite bzw. Hoehe, Groessen mit dem kleineren Verhaeltnis (Text laeuft nicht ueber).
    // VLW-Groessen auf die naechste vorhandene Datei - genau passend erzeugt sie der Designer beim Speichern.

    // Convert strip settings from another display type's backup to this strip: positions by the ratio of width
    // or height, sizes by the smaller ratio (text does not overflow). VLW sizes to the nearest available file -
    // the designer creates exactly fitting ones when saving.

    void scaleStripConfigFile(const String& path, int fromW, int fromH) {
        int toW = TFT_WIDTH, toH = TFT_HEIGHT - CLOCK_HEIGHT;
        if (fromW <= 0 || fromH <= 0 || toH <= 0) return;
        File f = LittleFS.open(path, "r");
        if (!f) return;
        String text = f.size() < 2048 ? f.readString() : String("");
        f.close();
        String saved = stripSettingsQuery();
        resetStripSettings();
        applyStripQuery(text);
        float sx = (float)toW / fromW, sy = (float)toH / fromH, sf = min(sx, sy);
        auto pos = [](int16_t v, float k) -> int16_t { return v < 0 ? v : (int16_t)lroundf(v * k); };
        auto pct = [&](uint8_t v) -> uint8_t { return (uint8_t)constrain(lroundf(v * sf), STRIP_SCALE_MIN, STRIP_SCALE_MAX); };
        auto px = [&](uint8_t v, bool weekday) -> uint8_t {
            return nearestStripVlwSize((uint8_t)constrain(lroundf(v * sf), 8L, 120L), weekday);
        };
        stripTimeX = pos(stripTimeX, sx); stripDateX = pos(stripDateX, sx); stripWeekdayX = pos(stripWeekdayX, sx);
        stripTimeY = pos(stripTimeY, sy); stripDateY = pos(stripDateY, sy); stripWeekdayY = pos(stripWeekdayY, sy);
        stripTimeScale = pct(stripTimeScale); stripDateScale = pct(stripDateScale); stripWeekdayScale = pct(stripWeekdayScale);
        stripVlwTimeSize = px(stripVlwTimeSize, false);
        stripVlwDateSize = px(stripVlwDateSize, false);
        stripVlwWeekdaySize = px(stripVlwWeekdaySize, true);
        f = LittleFS.open(path, "w");
        if (f) {
            f.print(stripSettingsQuery());
            f.close();
        }
        resetStripSettings();
        applyStripQuery(saved);
        stripSettingsFor = "?";
    }

    // Einstellungen von vor 2026-10-08 aus den Preferences (galten fuer alle Zifferblaetter); false = keine da
    // Settings from before 2026-10-08 from the preferences (applied to all clock faces); false = none there

    bool loadLegacyStripPrefs() {
        resetStripSettings();
        if (!preferences.isKey(PK_STRIP_BG) && !preferences.isKey(PK_STRIP_FONT)) return false;
        stripBgRgb = preferences.getULong(PK_STRIP_BG, 0x000000) & 0xFFFFFF;
        stripFgRgb = preferences.getULong(PK_STRIP_FG, 0xFFFFFF) & 0xFFFFFF;
        stripFont = preferences.getUChar(PK_STRIP_FONT, 0);
        if (stripFont >= STRIP_FONT_COUNT && stripFont != STRIP_FONT_VLW) stripFont = 0;
        stripTimeFmt = min((int)preferences.getUChar(PK_STRIP_TIME_FMT, 0), 2);
        stripSeconds = min((int)preferences.getUChar(PK_STRIP_SECONDS, 0), 2);
        stripDateFmt = min((int)preferences.getUChar(PK_STRIP_DATE_FMT, 0), STRIP_DATE_FMT_COUNT - 1);
        stripVlwName = preferences.getString(PK_STRIP_VLW_NAME, "");
        stripVlwTimeSize = preferences.getUChar(PK_STRIP_VLW_TSIZE, 44);
        stripVlwDateSize = preferences.getUChar(PK_STRIP_VLW_DSIZE, 22);
        stripBefore = preferences.getBool(PK_STRIP_BEFORE, false);
        stripBlink = preferences.getBool(PK_STRIP_BLINK, true);
        stripTimeX = preferences.getShort(PK_STRIP_TIME_X, -1);
        stripTimeY = preferences.getShort(PK_STRIP_TIME_Y, -1);
        stripDateX = preferences.getShort(PK_STRIP_DATE_X, -1);
        stripDateY = preferences.getShort(PK_STRIP_DATE_Y, -1);
        return true;
    }


    // Schrift fuer eine Zeile des Streifens: eingebaute LovyanGFX-Schrift (font) oder VLW-Daten (vlw)
    // Font for one line of the strip: built-in LovyanGFX font (font) or VLW data (vlw)

    struct StripFace {
        const lgfx::IFont* font;
        const uint8_t* vlw;
        float size;
    };

    void useStripFace(lgfx::LovyanGFX& g, const StripFace& f) {
        if (f.vlw) g.loadFont(f.vlw);
        else g.setFont(f.font);
        g.setTextSize(f.size);
    }


    // Datei vollstaendig in den PSRAM lesen (VLW-Schriften) - nullptr bei Fehler; free() durch den Aufrufer
    // Read a file completely into PSRAM (VLW fonts) - nullptr on error; free() by the caller

    uint8_t* loadFileToPsram(const char* path) {
        File f = LittleFS.open(path, "r");
        if (!f) return nullptr;
        size_t n = f.size();
        uint8_t* buf = (n >= 24 && n < 512 * 1024) ? (uint8_t*)preferPsramMalloc(n) : nullptr;
        if (buf && f.read(buf, n) != n) {
            free(buf);
            buf = nullptr;
        }
        f.close();
        return buf;
    }


    // VLW-Datei einer Schrift in einer Groesse: stripfont_<Name>_<Groesse>.vlw (Leerzeichen -> '-') - so liegt jede
    // Schrift nur einmal auf der Uhr, und Presets mit verschiedenen Schriften koennen nebeneinander bestehen.
    // weekday: stripfont_<Name>_<Groesse>_wd.vlw mit den Buchstaben der Wochentage.

    // VLW file of a font in one size: stripfont_<name>_<size>.vlw (spaces -> '-') - this way each font exists only
    // once on the clock, and presets with different fonts can coexist. weekday: stripfont_<name>_<size>_wd.vlw
    // with the letters of the weekdays.

    String stripVlwPath(uint8_t size, bool weekday) {
        String id;
        for (size_t i = 0; i < stripVlwName.length(); i++) {
            char c = stripVlwName[i];
            if (isalnum((unsigned char)c) || c == '-' || c == '_') id += c;
            else if (c == ' ') id += '-';
        }
        return "/stripfont_" + id + "_" + String(size) + (weekday ? "_wd" : "") + ".vlw";
    }


    // VLW-Schriften des Streifens laden - erneut, wenn sich Schrift oder Groesse aendern oder eine Datei
    // hochgeladen/geloescht wurde (stripVlwStale). Gleiche Groesse fuer Uhrzeit und Datum = ein Puffer. false,
    // wenn Uhrzeit oder Datum fehlt; ohne Wochentag-Datei nimmt der Wochentag eine eingebaute Schrift.

    // Load the strip's VLW fonts - again if font or size change or a file was uploaded/deleted (stripVlwStale).
    // Same size for time and date = one buffer. false if time or date is missing; without a weekday file the
    // weekday uses a built-in font.

    bool ensureStripVlw() {
        static String loadedTime, loadedDate, loadedWeekday;
        String pathTime = stripVlwPath(stripVlwTimeSize, false), pathDate = stripVlwPath(stripVlwDateSize, false);
        String pathWeekday = stripShowWeekday ? stripVlwPath(stripVlwWeekdaySize, true) : String("");
        if (stripVlwStale || pathTime != loadedTime || pathDate != loadedDate || pathWeekday != loadedWeekday) {
            stripVlwStale = false;
            stripVlwGeneration++;
            loadedTime = pathTime;
            loadedDate = pathDate;
            loadedWeekday = pathWeekday;
            if (stripVlwDate && stripVlwDate != stripVlwTime) free(stripVlwDate);
            if (stripVlwTime) free(stripVlwTime);
            if (stripVlwWeekday) free(stripVlwWeekday);
            stripVlwTime = loadFileToPsram(pathTime.c_str());
            stripVlwDate = (pathDate == pathTime) ? stripVlwTime : loadFileToPsram(pathDate.c_str());
            stripVlwWeekday = pathWeekday.length() ? loadFileToPsram(pathWeekday.c_str()) : nullptr;
        }
        return stripVlwTime && stripVlwDate;
    }


    // Oberste und unterste gesetzte Pixelzeile der Ziffern einer Schrift (Oberkante = 0) - die Schriftarten liegen
    // unterschiedlich in ihrer Zeilenhoehe, so lassen sich alle gleich mittig setzen. Einmal je Schrift gemessen.

    // Top and bottom set pixel row of a font's digits (top edge = 0) - the fonts sit differently within their line
    // height, this way all of them can be centred the same. Measured once per font.

    void stripDigitRows(LGFX_Sprite& s, const StripFace& f, int& top, int& bottom) {
        static const void* cachedKey[4] = { nullptr, nullptr, nullptr, nullptr };
        static uint32_t cachedGen[4] = { 0, 0, 0, 0 };
        static float cachedSize[4] = { 0, 0, 0, 0 };
        static int16_t cachedTop[4], cachedBottom[4];
        static uint8_t next = 0;
        const void* key = f.vlw ? (const void*)f.vlw : (const void*)f.font;
        uint32_t gen = f.vlw ? stripVlwGeneration : 0;
        for (uint8_t i = 0; i < 4; i++) {
            if (cachedKey[i] == key && cachedGen[i] == gen && cachedSize[i] == f.size) {
                top = cachedTop[i];
                bottom = cachedBottom[i];
                return;
            }
        }
        s.fillSprite(TFT_BLACK);
        useStripFace(s, f);
        s.setTextDatum(lgfx::top_left);
        s.setTextColor(TFT_WHITE, TFT_BLACK);
        s.drawString("0123456789:", 0, 0);
        top = -1;
        bottom = 0;
        for (int yy = 0; yy < s.height(); yy++) {
            for (int xx = 0; xx < s.width(); xx++) {
                if (s.readPixel(xx, yy) != 0) {
                    if (top < 0) top = yy;
                    bottom = yy + 1;
                    break;
                }
            }
        }
        if (top < 0) {
            top = 0;
            bottom = s.fontHeight();
        }
        cachedKey[next] = key;
        cachedGen[next] = gen;
        cachedSize[next] = f.size;
        cachedTop[next] = top;
        cachedBottom[next] = bottom;
        next = (next + 1) % 4;
    }


    // Uhrzeit mit Doppelpunkt zeichnen (Mitte cx, Oberkante y) - ein ausgeblendeter Doppelpunkt laesst seinen Platz
    // frei, die Ziffern springen beim Blinken nicht.

    // Draws the time with its colon (centre cx, top edge y) - a hidden colon keeps its space, the digits don't
    // jump while blinking.

    void drawStripTime(lgfx::LovyanGFX& g, const String& text, bool colon, int cx, int y) {
        int sep = text.indexOf(':');
        g.setTextDatum(lgfx::top_left);
        if (sep < 0) {
            g.drawString(text, cx - g.textWidth(text) / 2, y);
            return;
        }
        String hours = text.substring(0, sep);
        String rest = text.substring(sep + 1);
        int wH = g.textWidth(hours), wC = g.textWidth(":"), wR = g.textWidth(rest);
        int x0 = cx - (wH + wC + wR) / 2;
        g.drawString(hours, x0, y);
        if (colon) g.drawString(":", x0 + wH, y);
        g.drawString(rest, x0 + wH + wC, y);
    }


    // Streifen-Grafik zum Zifferblatt: face_<Name>.bmp -> strip_<Name>.bmp, leer fuer andere Namen
    // Strip graphic belonging to the clock face: face_<name>.bmp -> strip_<name>.bmp, empty for other names

    String stripPathForFace(const String& facePath) {
        String name = facePath.startsWith("/") ? facePath.substring(1) : facePath;
        if (!name.startsWith("face_")) return "";
        return "/strip_" + name.substring(5);
    }


    // Laedt die Streifen-Grafik des aktiven Zifferblatts nach stripImage - nur wenn sich das Zifferblatt
    // geaendert hat oder stripImageFor auf "?" steht. false ohne Grafik.

    // Loads the strip graphic of the active clock face into stripImage - only if the clock face changed or
    // stripImageFor is "?". false without a graphic.

    bool ensureStripImage() {
        String path = stripPathForFace(selectedBackground);
        if (path == stripImageFor) return stripImage != nullptr;
        stripImageFor = path;
        if (stripImage) {
            free(stripImage);
            stripImage = nullptr;
        }
        int w = TFT_WIDTH, h = TFT_HEIGHT - CLOCK_HEIGHT;
        if (path.length() == 0 || h <= 0 || !LittleFS.exists(path)) return false;
        stripImage = (uint16_t*)preferPsramMalloc((size_t)w * h * sizeof(uint16_t));
        if (!stripImage) return false;
        if (!loadFaceBmpInto(path, stripImage, w, h)) {
            free(stripImage);
            stripImage = nullptr;
            return false;
        }
        return true;
    }


    // Ohne PSRAM: Streifen-Grafik zeilenweise aus der RLEB-Datei direkt ins Streifen-Sprite - fuer eine Kopie im
    // RAM (stripImage, 51 KB bei 172 x 148) reichte der Speicher nicht, der Streifen blieb einfarbig. Quer wird
    // jede Quellzeile zu einer Spalte (90 Grad im Uhrzeigersinn wie in renderInfoStrip()). false ohne Grafik.

    // Without PSRAM: strip graphic row by row from the RLEB file straight into the strip sprite - there was not
    // enough memory for a copy in RAM (stripImage, 51 KB at 172 x 148), the strip stayed plain. In landscape each
    // source row becomes a column (90 degrees clockwise as in renderInfoStrip()). false without a graphic.

    bool drawStripImageStreamed(LGFX_Sprite& s, bool landscape, bool push) {
        String path = stripPathForFace(selectedBackground);
        const int sw = TFT_WIDTH, sh = TFT_HEIGHT - CLOCK_HEIGHT;
        int32_t w, h;
        if (path.length() == 0 || sh <= 0 || sw > 320 || !readRleSize(path, w, h) || w != sw || h != sh) return false;
        RleFaceReader r;
        r.file = LittleFS.open(path, "r");
        uint8_t head[20];
        if (!r.file || r.file.read(head, sizeof(head)) != sizeof(head)) return false;
        r.remaining = *(uint32_t*)&head[12];
        static uint16_t line[320];
        for (int y = 0; y < sh; y++) {
            for (int x = 0; x < sw; x++) {
                uint16_t px = 0xFFFF;   // weiss, falls die Daten zu kurz sind / white if the data is too short
                r.nextPixel(px);
                line[x] = push ? setPixelBrightness(px) : px;
            }
            if (landscape) s.pushImage(sh - 1 - y, 0, 1, sw, line);
            else s.pushImage(0, y, sw, 1, line);
        }
        r.file.close();
        return true;
    }


    // Lage des Streifens umschalten: beide Displays einmal loeschen, danach Uhr und Streifen komplett neu senden
    // Switch the strip placement: clear both displays once, then resend the clock and the strip completely

    void setStripBefore(bool before) {
        if (before == stripBefore) return;
        stripBefore = before;
        if (TFT_HEIGHT == CLOCK_HEIGHT) return; // ohne Streifen (runde Displays) nur merken
                                                // without a strip (round displays) only remember it
        for (uint8_t d = 1; d <= 2; d++) {
            if (!isDisplayConnected(d)) continue;
            if (d == 1) setCS1(LOW); else setCS2(LOW);
            tft.fillScreen(TFT_BLACK);
            clockFrameDirty[d - 1] = true;
            infoStripDirty[d - 1] = true;
        }
        setCSIdle();
    }


    // Eine Streifen-Einstellung setzen (Schluessel wie stripSettingsQuery()), ohne zu speichern und ohne die Lage
    // auf den Displays umzuschalten. true, wenn der Schluessel zum Streifen gehoert.

    // Set one strip setting (keys as in stripSettingsQuery()), without storing it and without switching the
    // placement on the displays. true if the key belongs to the strip.

    bool applyStripValue(const String& key, const String& value) {
        if (!key.startsWith("strip")) return false;
        long v = value.toInt();
        bool on = (value == "1" || value.equalsIgnoreCase("true"));
        int16_t pos = (v < 0) ? -1 : (int16_t)v;
        if (key == "stripBefore") {
            stripBefore = on;
        }
        else if (key == "stripBg") {
            stripBgRgb = strtoul(value.c_str(), nullptr, 16) & 0xFFFFFF;
        }
        else if (key == "stripFg") {
            stripFgRgb = strtoul(value.c_str(), nullptr, 16) & 0xFFFFFF;
        }
        else if (key == "stripFont") {
            if ((v >= 0 && v < STRIP_FONT_COUNT) || v == STRIP_FONT_VLW) stripFont = (uint8_t)v;
        }
        else if (key == "stripVlw") {
            stripVlwName = value.substring(0, 40);
        }
        else if (key == "stripVt") {
            stripVlwTimeSize = (uint8_t)constrain(v, 8L, 120L);
        }
        else if (key == "stripVd") {
            stripVlwDateSize = (uint8_t)constrain(v, 8L, 120L);
        }
        else if (key == "stripTs") {
            stripTimeScale = (uint8_t)constrain(v, STRIP_SCALE_MIN, STRIP_SCALE_MAX);
        }
        else if (key == "stripDs") {
            stripDateScale = (uint8_t)constrain(v, STRIP_SCALE_MIN, STRIP_SCALE_MAX);
        }
        else if (key == "stripShowT") {
            stripShowTime = on;
        }
        else if (key == "stripShowD") {
            stripShowDate = on;
        }
        else if (key == "stripShowW") {
            stripShowWeekday = on;
        }
        else if (key == "stripWs") {
            stripWeekdayScale = (uint8_t)constrain(v, STRIP_SCALE_MIN, STRIP_SCALE_MAX);
        }
        else if (key == "stripVw") {
            stripVlwWeekdaySize = (uint8_t)constrain(v, 8L, 120L);
        }
        else if (key == "stripTfmt") {
            stripTimeFmt = (uint8_t)constrain(v, 0L, 2L);
        }
        else if (key == "stripSec") {
            stripSeconds = (uint8_t)constrain(v, 0L, 2L);
        }
        else if (key == "stripDfmt") {
            stripDateFmt = (uint8_t)constrain(v, 0L, (long)STRIP_DATE_FMT_COUNT - 1);
        }
        else if (key == "stripBlink") {
            stripBlink = on;
        }
        else if (key == "stripTx") stripTimeX = pos;
        else if (key == "stripTy") stripTimeY = pos;
        else if (key == "stripDx") stripDateX = pos;
        else if (key == "stripDy") stripDateY = pos;
        else if (key == "stripWx") stripWeekdayX = pos;
        else if (key == "stripWy") stripWeekdayY = pos;
        else {
            return false;
        }
        infoStripDirty[0] = infoStripDirty[1] = true;
        return true;
    }


    // Zeichnet den Streifen (x/y/w/h wie infoStripRect()) ins Sprite und sendet es. Hochkant: Uhrzeit (AM/PM
    // klein rechts oben in der Datumsschrift) und Datum an ihren Positionen; quer Zeilen untereinander in der
    // Datumsschrift. Ohne Sprite direkt, dann ohne VLW und ohne Grafik.

    // Draws the strip (x/y/w/h as infoStripRect()) into the sprite and sends it. Portrait: time (AM/PM small
    // at the top right in the date font) and date at their positions; landscape lines below each other in the
    // date font. Without a sprite directly, then without VLW and without the graphic.

    void renderInfoStrip(int x, int y, int w, int h, bool landscape, const String* lines, uint8_t count, bool colon,
                         const String& suffix, uint16_t bg, uint16_t fg, bool push, bool useImage) {
        LGFX_Sprite& s = infoStripSprite;
        if (infoStripSpriteCreated && (s.width() != w || s.height() != h)) {
            s.deleteSprite();
            infoStripSpriteCreated = false;
        }
        if (!infoStripSpriteCreated) infoStripSpriteCreated = createSprite16(s, w, h);
        const bool useSprite = infoStripSpriteCreated;
        if (!push && !useSprite) return; // Vorschau (push = false) geht nur mit Sprite
                                         // preview (push = false) only works with a sprite
        lgfx::LovyanGFX& g = useSprite ? (lgfx::LovyanGFX&)s : (lgfx::LovyanGFX&)tft;
        const int ox = useSprite ? 0 : x, oy = useSprite ? 0 : y;

        // Schriften: VLW aus dem Designer (nur mit Sprite - kantengeglaettet braucht lesbaren Hintergrund) oder

        // eingebaut
        // Fonts: VLW from the designer (sprite only - anti-aliasing needs a readable background) or built-in

        const bool vlw = stripFont == STRIP_FONT_VLW && useSprite && ensureStripVlw();
        StripFace timeFace, dateFace, weekdayFace;
        if (vlw) {
            timeFace = { nullptr, stripVlwTime, 1 };
            dateFace = { nullptr, stripVlwDate, 1 };
            weekdayFace = stripVlwWeekday ? StripFace{ nullptr, stripVlwWeekday, 1 }
                                          : StripFace{ &fonts::DejaVu18, nullptr, stripWeekdayScale / 100.0f };
        }
        else {
            const StripFont& f = STRIP_FONTS[stripFont < STRIP_FONT_COUNT ? stripFont : 0];
            timeFace = { f.time, nullptr, f.size * stripTimeScale / 100.0f };
            dateFace = { f.date, nullptr, f.size * stripDateScale / 100.0f };
            weekdayFace = { f.date, nullptr, f.size * stripWeekdayScale / 100.0f };
        }

        // Ziffernhoehe messen (ohne Sprite nur die Schrifthoehe)
        // Measure the digit height (without a sprite only the font height)

        auto rows = [&](const StripFace& face, int& top, int& bottom) {
            if (useSprite) {
                stripDigitRows(s, face, top, bottom);
                return;
            }
            useStripFace(g, face);
            top = 0;
            bottom = g.fontHeight();
        };

        // Automatische Positionen immer fuer hochkant berechnen - der Designer zeigt sie an
        // Always compute the automatic positions for portrait - the designer shows them

        // Eingeschaltete Zeilen von oben nach unten (Uhrzeit, Wochentag, Datum) mit gleichen Abstaenden
        // Switched-on lines from top to bottom (time, weekday, date) with equal gaps

        int tTop, tBottom, dTop, dBottom, wTop = 0, wBottom = 0;
        rows(timeFace, tTop, tBottom);
        rows(dateFace, dTop, dBottom);
        if (stripShowWeekday) rows(weekdayFace, wTop, wBottom);
        int portraitH = TFT_HEIGHT - CLOCK_HEIGHT;
        const bool shown[3] = { stripShowTime, stripShowWeekday, stripShowDate };
        const int heights[3] = { tBottom - tTop, wBottom - wTop, dBottom - dTop };
        int16_t* autoY[3] = { &stripAutoTimeY, &stripAutoWeekdayY, &stripAutoDateY };
        int shownCount = 0, shownHeight = 0;
        for (int i = 0; i < 3; i++) {
            if (shown[i]) { shownCount++; shownHeight += heights[i]; }
        }
        int gap = max(0, (portraitH - shownHeight) / (shownCount + 1));
        int cursor = gap;
        for (int i = 0; i < 3; i++) {
            *autoY[i] = portraitH / 2;
            if (!shown[i]) continue;
            *autoY[i] = cursor + heights[i] / 2;
            cursor += heights[i] + gap;
        }

        // Hintergrund: Streifen-Grafik des Zifferblatts (quer um 90 Grad im Uhrzeigersinn gedreht), sonst die
        // Farbe. Gedimmt wie das Zifferblatt, nur fuers Display (push), nicht fuer die Vorschau.

        // Background: the clock face's strip graphic (rotated 90 degrees clockwise in landscape), otherwise the
        // color. Dimmed like the clock face, only for the display (push), not for the preview.

        const int sw = TFT_WIDTH, sh = TFT_HEIGHT - CLOCK_HEIGHT;
        bool imageFits = landscape ? (w == sh && h == sw) : (w == sw && h == sh);
        bool imageDrawn = false;
        if (useImage && useSprite && imageFits) {
            if (lowMemoryFace()) {
                imageDrawn = drawStripImageStreamed(s, landscape, push);
            }
            else if (ensureStripImage()) {
                static uint16_t row[320];
                for (int yy = 0; yy < h; yy++) {
                    for (int xx = 0; xx < w && xx < 320; xx++) {
                        uint16_t px = landscape ? stripImage[(sh - 1 - xx) * sw + yy] : stripImage[yy * sw + xx];
                        row[xx] = push ? setPixelBrightness(px) : px;
                    }
                    s.pushImage(0, yy, w, 1, row);
                }
                imageDrawn = true;
            }
        }
        if (!imageDrawn) g.fillRect(ox, oy, w, h, bg);

        // Nur Schriftfarbe setzen: ohne Hintergrundfarbe mischt LovyanGFX die Kanten mit dem Sprite-Inhalt
        // Only set the text color: without a background color LovyanGFX blends the edges with the sprite content

        g.setTextColor(fg);
        if (!landscape) {
            int tx = (stripTimeX >= 0) ? stripTimeX : w / 2;
            int ty = (stripTimeX >= 0 && stripTimeY >= 0) ? stripTimeY : stripAutoTimeY;
            int dx = (stripDateX >= 0) ? stripDateX : w / 2;
            int dy = (stripDateX >= 0 && stripDateY >= 0) ? stripDateY : stripAutoDateY;
            int timeTop = oy + ty - (tTop + tBottom) / 2;

            // AM/PM rechts neben der Uhrzeit, oben buendig mit den Ziffern - beide zusammen mittig
            // AM/PM to the right of the time, top-aligned with the digits - both centred together

            int suffixW = 0, suffixGap = 0;
            if (suffix.length()) {
                useStripFace(g, dateFace);
                suffixW = g.textWidth(suffix);
                suffixGap = 3;
            }
            useStripFace(g, timeFace);
            int timeW = g.textWidth(lines[0]);
            int timeCx = ox + tx - (suffixW + suffixGap) / 2;
            drawStripTime(g, lines[0], colon, timeCx, timeTop);
            if (suffix.length()) {
                useStripFace(g, dateFace);
                g.setTextDatum(lgfx::top_left);
                g.drawString(suffix, timeCx + timeW / 2 + suffixGap, timeTop + tTop - dTop);
            }
            if (count > 1 && lines[1].length()) {
                useStripFace(g, dateFace);
                g.setTextDatum(lgfx::top_center);
                g.drawString(lines[1], ox + dx, oy + dy - (dTop + dBottom) / 2);
            }
            if (count > 2 && lines[2].length()) {
                int wx = (stripWeekdayX >= 0) ? stripWeekdayX : w / 2;
                int wy = (stripWeekdayX >= 0 && stripWeekdayY >= 0) ? stripWeekdayY : stripAutoWeekdayY;
                useStripFace(g, weekdayFace);
                g.setTextDatum(lgfx::top_center);
                g.drawString(lines[2], ox + wx, oy + wy - (wTop + wBottom) / 2);
            }
        }
        else {
            for (uint8_t n = 0; n < count; n++) {
                StripFace face = (n == stripWeekdayLine) ? weekdayFace : dateFace;
                if (!face.vlw && face.font == &fonts::Font0) face.size = 2;
                useStripFace(g, face);
                if (g.textWidth(lines[n]) > w - 4) face = { &fonts::Font0, nullptr, 2 };
                useStripFace(g, face);
                if (g.textWidth(lines[n]) > w - 4) face.size = 1; // langer Wochentag im schmalen Streifen
                                                                  // long weekday in a narrow strip
                int top, bottom;
                rows(face, top, bottom);
                useStripFace(g, face);
                g.setTextColor(fg);
                int cy = h * (2 * n + 1) / (2 * count);
                if (n == 0) {
                    drawStripTime(g, lines[n], colon, ox + w / 2, oy + cy - (top + bottom) / 2);
                }
                else {
                    g.setTextDatum(lgfx::top_center);
                    g.drawString(lines[n], ox + w / 2, oy + cy - (top + bottom) / 2);
                }
            }
        }

        if (useSprite) {
            if (vlw) s.unloadFont();
            if (push) s.pushSprite(x, y);
        }
        else {
            setupTextStyle(tft);
            tft.setTextDatum(lgfx::top_left);
            tft.setTextSize(TFT_TEXT_SIZE);
        }
    }


    // Datum im eingestellten Format (stripDateFmt): full fuer hochkant, a/b fuer die zwei Zeilen quer.
    // 0 = T.MM.JJJJ, 1 = TT.MM.JJJJ, 2 = TT.MM.JJ, 3 = MM/TT/JJJJ, 4 = JJJJ-MM-TT, 5 = TT.MM., 6 = T.MM.JJ

    // Date in the set format (stripDateFmt): full for portrait, a/b for the two lines in landscape.
    // 0 = D.MM.YYYY, 1 = DD.MM.YYYY, 2 = DD.MM.YY, 3 = MM/DD/YYYY, 4 = YYYY-MM-DD, 5 = DD.MM., 6 = D.MM.YY

    void stripDateText(const struct tm& t, String& full, String& a, String& b) {
        char d[4], dd[4], mm[4], yyyy[6], yy[4];
        snprintf(d, sizeof(d), "%d", t.tm_mday);
        snprintf(dd, sizeof(dd), "%02d", t.tm_mday);
        snprintf(mm, sizeof(mm), "%02d", t.tm_mon + 1);
        snprintf(yyyy, sizeof(yyyy), "%04d", t.tm_year + 1900);
        snprintf(yy, sizeof(yy), "%02d", (t.tm_year + 1900) % 100);
        switch (stripDateFmt) {
            case 1:  a = String(dd) + "." + mm + ".";  b = yyyy;               full = a + b; break;
            case 2:  a = String(dd) + "." + mm + ".";  b = yy;                 full = a + b; break;
            case 3:  a = String(mm) + "/" + dd + "/";  b = yyyy;               full = a + b; break;
            case 4:  a = String(yyyy);                 b = String(mm) + "-" + dd; full = a + "-" + b; break;
            case 5:  a = String(dd) + "." + mm + ".";  b = "";                 full = a; break;
            case 6:  a = String(d) + "." + mm + ".";   b = yy;                 full = a + b; break;
            default: a = String(d) + "." + mm + ".";   b = yyyy;               full = a + b; break;
        }
    }


    // Inhalt des Streifens: hochkant Uhrzeit und Datum (AM/PM separat in suffix), quer je Zeile Uhrzeit, Sekunden,
    // AM/PM und die Datumsteile. Zeitquelle wie bei den Zeigern (renderClockFrame()), das Datum aus der echten Zeit.

    // Content of the strip: portrait time and date (AM/PM separately in suffix), landscape one line each for time,
    // seconds, AM/PM and the date parts. Time source as for the hands (renderClockFrame()), the date from the real
    // time.

    // Wochentag ausgeschrieben in der Sprache der Weboberflaeche (Deutsch, sonst Englisch)
    // Weekday written out in the language of the web interface (German, otherwise English)

    String stripWeekdayText(const struct tm& t) {
        static const char* const DE[7] = { "Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag" };
        static const char* const EN[7] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
        if (t.tm_wday < 0 || t.tm_wday > 6) return "";
        return currentLanguage == "de" ? DE[t.tm_wday] : EN[t.tm_wday];
    }

    void stripContent(bool landscape, String* lines, uint8_t& count, bool& colon, String& suffix, const struct tm* fixedTime) {
        bool rocrailTimeReady = rocrailEnabled && rocrailConnected && rocrailLastClockMillis != 0 &&
                                (millis() - rocrailLastClockMillis) < ROCRAIL_STALE_TIMEOUT_MS;
        const struct tm& t = fixedTime ? *fixedTime : (rocrailTimeReady ? rocrailTimeinfo : timeinfo); // fixedTime: Vorschau
                                                                                                           // fixedTime: preview
        int hour = t.tm_hour;
        suffix = "";
        if (stripTimeFmt != 0) {
            if (stripTimeFmt == 1) suffix = (hour < 12) ? "AM" : "PM";
            hour = (hour % 12 == 0) ? 12 : hour % 12;
        }
        char hourMin[8];
        char seconds[4];
        snprintf(hourMin, sizeof(hourMin), "%d:%02d", hour, t.tm_min);
        snprintf(seconds, sizeof(seconds), "%02d", t.tm_sec);

        // Der Doppelpunkt blinkt nur ohne Sekunden
        // The colon only blinks without seconds

        // Erst mit aktueller Zeit anzeigen: Uhrzeit aus der Vorschau, von Rocrail oder ab dem Jahr 2000 (nach dem
        // Start steht die Systemzeit auf 1970), Datum und Wochentag nur aus der echten Zeit

        // Only show with a current time: time from the preview, from Rocrail or from the year 2000 on (after the
        // start the system time is 1970), date and weekday only from the real time

        const bool realTime = timeinfo.tm_year >= 100;
        const bool showTime = stripShowTime && (fixedTime || rocrailTimeReady || realTime);
        const bool showDate = stripShowDate && realTime;
        bool withSeconds = stripShowsSeconds();
        colon = !(stripBlink && showTime && !withSeconds && t.tm_sec % 2);
        String dateFull, dateA, dateB;
        String weekday;
        if (realTime) {
            stripDateText(timeinfo, dateFull, dateA, dateB);
            weekday = stripWeekdayText(timeinfo);
        }
        stripWeekdayLine = -1;

        // Ausgeschaltete Zeilen: quer ganz weg, hochkant als leere Zeile
        // Switched-off lines: gone in landscape, an empty line in portrait

        count = 0;
        if (landscape) {
            if (showTime) {
                lines[count++] = hourMin;
                if (withSeconds) lines[count++] = seconds;
                if (suffix.length()) lines[count++] = suffix;
            }
            if (stripShowWeekday && weekday.length()) {
                stripWeekdayLine = count;
                lines[count++] = weekday;
            }
            if (showDate) {
                lines[count++] = dateA;
                if (dateB.length()) lines[count++] = dateB;
            }
            suffix = "";
        }
        else {
            lines[count++] = !showTime ? String("") : withSeconds ? String(hourMin) + ":" + seconds : String(hourMin);
            lines[count++] = showDate ? dateFull : String("");
            lines[count++] = stripShowWeekday ? weekday : String("");
            if (!showTime) suffix = "";
        }
    }

    // Zeigt der Streifen Sekunden? Automatisch nur ohne Sekundenzeiger. Die Vorschauen laden ihn danach jede
    // Sekunde oder nur beim Minutenwechsel neu.

    // Does the strip show seconds? Automatically only without the second hand. The previews reload it every
    // second or only on a minute change accordingly.

    bool stripShowsSeconds() {
        return stripSeconds == 2 || (stripSeconds == 0 && !showSecondHand);
    }

    uint16_t stripColor565(uint32_t rgb) {
        return tft.color565((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
    }


    // Streifen neben der Uhr bei rechteckigen Displays (ILI9341, wie in uhr3): Uhrzeit (24 h oder 12 h mit/ohne
    // AM/PM) als "H:MM" (Doppelpunkt blinkt wahlweise) oder "H:MM:SS" (stripSeconds) und Datum im gewaehlten
    // Format. Gezeichnet nur bei Aenderung oder nach einer Meldung (infoStripDirty).

    // Strip next to the clock on rectangular displays (ILI9341, as in uhr3): time (24 h or 12 h with/without
    // AM/PM) as "H:MM" (colon optionally blinks) or "H:MM:SS" (stripSeconds), and the date in the chosen
    // format. Drawn only on change or after a message (infoStripDirty).

    void drawInfoStrips() {
        int x, y, w, h;
        bool landscape;
        if (!infoStripRect(1, x, y, w, h, landscape)) return;
        ensureStripSettings();
        uint16_t bg = setPixelBrightness(stripColor565(stripBgRgb));
        uint16_t fg = setPixelBrightness(stripColor565(stripFgRgb));

        static String lastText[2];
        for (uint8_t d = 1; d <= 2; d++) {
            if (!isDisplayConnected(d)) continue;
            uint8_t i = d - 1;
            infoStripRect(d, x, y, w, h, landscape);
            String lines[6];
            uint8_t count;
            bool colon;
            String suffix;
            stripContent(landscape, lines, count, colon, suffix);
            String text = String(colon ? "1" : "0") + "|" + String(bg) + "|" + String(fg) + "|" + selectedBackground + "|" + suffix;
            for (uint8_t n = 0; n < count; n++) text += "|" + lines[n];
            if (!infoStripDirty[i] && lastText[i] == text) continue;

            if (d == 1) setCS1(LOW); else setCS2(LOW);
            renderInfoStrip(x, y, w, h, landscape, lines, count, colon, suffix, bg, fg, true, true);
            lastText[i] = text;
            infoStripDirty[i] = false;
        }
    }


    // Streifen hochkant und ungedimmt nur ins Sprite zeichnen - fuer die Vorschauen (/api/stripimg). textOnly:
    // Hintergrund textBg ohne Grafik, damit der Designer die Schrift ueber seine eigene Zeichnung legen kann.
    // false ohne Streifen oder Sprite. Das Display zeichnet ihn danach neu (anderes Sprite-Format).

    // Draw the strip in portrait and undimmed into the sprite only - for the previews (/api/stripimg). textOnly:
    // background textBg without the graphic, so the designer can lay the text over its own drawing.
    // false without a strip or sprite. The display redraws it afterwards (other sprite format).

    bool renderStripPreview(int w, int h, bool textOnly, const struct tm* fixedTime, uint16_t textBg) {
        if (h <= 0) return false;
        String lines[6];
        uint8_t count;
        bool colon;
        String suffix;
        stripContent(false, lines, count, colon, suffix, fixedTime);
        uint16_t bg = textOnly ? textBg : stripColor565(stripBgRgb);
        renderInfoStrip(0, 0, w, h, false, lines, count, true, suffix, bg, stripColor565(stripFgRgb), false, !textOnly);
        infoStripDirty[0] = infoStripDirty[1] = true;
        return infoStripSpriteCreated && infoStripSprite.width() == w && infoStripSprite.height() == h;
    }


    // Liest Zeit/RTC einmal, dann ein renderClockFrame() pro angeschlossenem
    // Display ("n.a." wird uebersprungen). Bei Hardware-Rotation genuegt fuer
    // Display 2 erneutes Senden; bei GC9D01 nur, wenn tftRotation2 von tftRotation1 abweicht.

    // Reads time/RTC once, then one renderClockFrame() per connected display
    // ("n.a." is skipped). With hardware rotation, re-sending suffices for
    // Display 2; with GC9D01, only if tftRotation2 differs from tftRotation1.

    void updateClock() {

        // In eine lokale Kopie lesen: die NTP-Sync-Task setzt die Systemzeit kurz auf 1970, um echten Erfolg
        // zu erkennen - direkt in timeinfo stuenden sonst fuer ein Frame falsche Zeiger (und eine falsche
        // Helligkeit). Die letzte gute Zeit samt millis() dient zum Weiterrechnen.

        // Read into a local copy: the NTP sync task briefly sets the system time to 1970 to detect real
        // success - directly in timeinfo the hands (and brightness) would be wrong for a frame. The last good
        // time plus millis() is used to extrapolate.

        static time_t lastGoodEpoch = 0;
        static unsigned long lastGoodEpochMillis = 0;

        // Timeout 0: schlaegt das Lesen fehl, wird aus der letzten guten Zeit weitergerechnet - Warten wuerde
        // den Haupt-Loop waehrend der NTP-Invalidierung nur lahmlegen.

        // Timeout 0: if the read fails, extrapolation from the last good time follows - waiting would only
        // stall the main loop during the NTP invalidation.

        struct tm freshTimeinfo;
        if (getLocalTime(&freshTimeinfo, 0)) {
            timeinfo = freshTimeinfo;
            time_t now;
            time(&now);
            lastGoodEpoch = now;
            lastGoodEpochMillis = millis();
        }
        else if (lastGoodEpoch != 0) {

            // Waehrend die Systemzeit kurz ungueltig ist (NTP-Invalidierung), aus der letzten guten Zeit plus
            // verstrichener Zeit weiterrechnen, statt den Sekundenzeiger anzuhalten.

            // While the system time is briefly invalid (NTP invalidation), extrapolate from the last good
            // time plus elapsed time instead of pausing the second hand.

            time_t estimatedNow = lastGoodEpoch + (time_t)((millis() - lastGoodEpochMillis) / 1000);
            localtime_r(&estimatedNow, &timeinfo);
        }
        else {

            // Noch nie eine gueltige Zeit: auf die RTC zurueckfallen, ohne RTC ab der Startzeit
            // (START_TIME_*) weiterlaufen. Jahr 0 = "noch keine Zeit": Zeiger auf 12, Streifen leer
            // (renderClockFrame(), stripContent()) - die erste echte Zeit uebernimmt sofort.

            // Never a valid time yet: fall back to the RTC, without an RTC keep running from the start time
            // (START_TIME_*). Year 0 = "no time yet": hands at 12, strip empty (renderClockFrame(),
            // stripContent()) - the first real time takes over right away.

            loadTimeFromRTC();
            if (rtcOk != RTC_AVAILABLE) {
                static unsigned long fallbackStartMillis = millis();
                unsigned long t = (unsigned long)START_TIME_HOUR * 3600UL + START_TIME_MIN * 60UL + START_TIME_SEC
                                + (millis() - fallbackStartMillis) / 1000UL;
                timeinfo.tm_hour = (t / 3600UL) % 24UL;
                timeinfo.tm_min = (t / 60UL) % 60UL;
                timeinfo.tm_sec = t % 60UL;
                timeinfo.tm_year = 0;
            }
        }


        static unsigned long lastRtcReloadMillis = 0; // Zeitpunkt des letzten RTC-Lesevorgangs (eigenstaendig, NICHT dieselbe Variable wie das globale lastRTCUpdate in time_sync.h/applyDcf77DecodedTime)
                                                      // timestamp of the last RTC read (independent, NOT the same variable as the global lastRTCUpdate in time_sync.h/applyDcf77DecodedTime)
        if (rtcOk == RTC_AVAILABLE) {

            // Überprüfen, ob seit dem letzten Aufruf Zeit vergangen ist
            // Check whether time has passed since the last call

            if (millis() - lastRtcReloadMillis >= WAIT_1h) {

                // Nur neu aus der RTC laden, wenn NTP nicht innerhalb der Sync-Periode (WAIT_6h) erfolgreich
                // war - sonst ueberschriebe die ungenauere RTC die NTP-Zeit und der Sekundenzeiger spraenge
                // zurueck. Die Schwelle muss zur NTP-Periode passen.

                // Only reload from the RTC if NTP has not succeeded within the sync period (WAIT_6h) -
                // otherwise the less precise RTC would overwrite the NTP time and the second hand would jump
                // back. The threshold must match the NTP period.

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

        // Die Bahnhofsuhr-Logik (stationTick) richtet sich nur beim ersten Aufruf aus - daher firstRun
        // einmalig neu setzen, sobald timeinfo erstmals plausibel wird (Jahr >= 2000; die laufende Startzeit
        // ohne Zeitquelle zaehlt nicht). Der Normalmodus korrigiert sich jedes Frame selbst.

        // The station-clock logic (stationTick) only aligns on its first call - so set firstRun again once
        // timeinfo first becomes plausible (year >= 2000; the running start time without a time source does
        // not count). Normal mode corrects itself every frame.

        static bool hadPlausibleTime = false;
        if (!hadPlausibleTime && timeinfo.tm_year >= 100) {
            hadPlausibleTime = true;
            firstRun = true;
            firstRun2 = true;
        }

        // Displays mit "n.a." werden weder angesteuert noch berechnet; ist nur Display 2 angeschlossen,
        // rendert es selbst. firstRun bleibt true, damit ein spaeter angeschlossenes Display sofort richtig
        // steht.

        // Displays set to "n.a." are neither driven nor calculated; if only display 2 is connected, it
        // renders itself. firstRun stays true, so a display connected later is right immediately.

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
                infoStripDirty[d - 1] = true;
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

                int ox, oy;
                clockOrigin(2, ox, oy);
                if (lastRenderPartial && !clockFrameDirty[1]) {
                    pushBackgroundRect(lastRenderRect[0], lastRenderRect[1], lastRenderRect[2], lastRenderRect[3], ox, oy);
                }
                else {
                    backgroundSprite.pushSprite(ox, oy);
                }
                clockFrameDirty[1] = false;
            }

            // firstRun2 auf true halten: weicht tftRotation2 spaeter wieder ab, soll Display 2 sofort richtig
            // stehen statt sich aus veralteten Winkeln heranzutasten.

            // Keep firstRun2 at true: if tftRotation2 diverges again later, display 2 should be right
            // immediately instead of easing in from stale angles.

            firstRun2 = true;
        }

        drawInfoStrips();

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

        // Eigene Vergleichsvariable fuer die Zeiger (das Zifferblatt pflegt lastAppliedBrightness selbst).
        // Nicht bei jedem Rampenschritt neu einfaerben, nur bei spuerbarer Differenz und einmal am
        // Rampenende.

        // Own comparison variable for the hands (the clock face maintains lastAppliedBrightness itself). Do
        // not re-tint on every ramp step, only on a noticeable difference and once at the end of the ramp.

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

        // Liegt die Uhrzeit im Zeitfenster fuer volle Helligkeit? Bei einem getLocalTime()-Fehlschlag den
        // letzten Stand behalten, sonst fiele die Helligkeit waehrend der NTP-Invalidierung grundlos auf
        // minBrightness.

        // Is the time within the full-brightness window? On a getLocalTime() failure keep the last state,
        // otherwise the brightness would drop to minBrightness during the NTP invalidation for no reason.

        static bool withinDayWindow = false;
        static bool timeEverValid = false; // seit dem Start je eine gueltige Uhrzeit / valid time ever since boot

        // struct tm timeinfo;

        struct tm freshTimeinfo;

        // Timeout 0 wie in updateClock(): bei Fehlschlag bleibt withinDayWindow auf dem letzten Stand, Warten
        // wuerde nur den Haupt-Loop blockieren.

        // Timeout 0 as in updateClock(): on failure withinDayWindow keeps its last state, waiting would only
        // block the main loop.

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

        if (useAdc) {

            // Nur alle ADC_SAMPLE_INTERVAL_MS abtasten statt jeden Tick - sonst deckt das Mittel nur
            // Millisekunden ab und ein kurzer Einbruch (WLAN-Sendeburst) wird nicht herausgemittelt.

            // Only sample every ADC_SAMPLE_INTERVAL_MS instead of every tick - otherwise the average spans
            // only milliseconds and a brief dip (WiFi TX burst) is not averaged out.

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

        // Rocrail-Helligkeit hat Vorrang vor Zeitfenster UND Fotowiderstand,
        // sobald verbunden und ein bri-Wert bekannt ist - dieselbe Stale-
        // Pruefung wie rocrailTimeReady faellt sonst nach ROCRAIL_STALE_TIMEOUT_MS auf lokale Steuerung zurueck.

        // Rocrail brightness takes priority over both the time window and
        // the photoresistor, once connected and a bri value is known - the
        // same staleness check as rocrailTimeReady falls back to local control after ROCRAIL_STALE_TIMEOUT_MS otherwise.

        bool rocrailBrightnessActive = rocrailEnabled && rocrailConnected && rocrailBrightnessKnown &&
                                        (millis() - rocrailLastClockMillis) < ROCRAIL_STALE_TIMEOUT_MS;

        // Einrichtung (noch nie gueltige Zeit, WPS oder Access Point): mit PWM 50 % (BACKLIGHT_SETUP_LEVEL),
        // sonst volle Helligkeit - Meldungen und AP-Passwort bleiben lesbar, auch ohne hellen Lichtsensor.

        // Setup (never a valid time, WPS or access point): with PWM 50 % (BACKLIGHT_SETUP_LEVEL), otherwise
        // full brightness - messages and the AP password stay readable, also without a bright light sensor.

        bool setupBrightness = !timeEverValid || softAPIP || wpsPending;

        if (setupBrightness) {
            targetBrightness = useBacklight ? BACKLIGHT_SETUP_LEVEL : maxBrightness;
            currentBrightness = targetBrightness;
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

            // Normale Auto-Brightness oder statische Helligkeit
            // Normal auto-brightness or static brightness

            if (useAdc) {

                // Helligkeits-Entscheidung anhand der oben aktualisierten Werte; eine
                // Schwellwert-Ueberschreitung erst nach BRIGHTNESS_DEBOUNCE_MS uebernehmen - ein kurzer
                // Ausreisser faerbte sonst das Zifferblatt fuer ein Frame um.

                // Brightness decision based on the values updated above; act on a threshold crossing only
                // after BRIGHTNESS_DEBOUNCE_MS - a brief outlier would otherwise re-tint the clock face for a
                // frame.

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

        // Bewegung des Sekundenzeigers im Bahnhofsuhr-Modus, in der Web-Vorschau als JavaScript gespiegelt -
        // Aenderungen dort nachziehen. Intensitaet: 1.0 = Standard, >1.0 steiler, <1.0 flacher.

        // Second-hand motion in station-clock mode, mirrored as JavaScript in the web preview - keep changes
        // in sync there. Intensity: 1.0 = default, >1.0 steeper, <1.0 flatter.

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


    // Sendet einen Zeiger als PNG (RGBA, unkomprimierte "stored"-Deflate-Bloecke, Adler32 laufend mitgerechnet)
    // Zeile fuer Zeile - ohne das Bild im RAM. Breite w schneidet mittig zu wie auf dem Display; Weiss und
    // TRANSPARENT_COLOR werden durchsichtig.

    // Sends a hand as PNG (RGBA, uncompressed "stored" deflate blocks, Adler32 computed on the fly) row by row -
    // without the image in RAM. Width w crops in the centre as on the display; white and TRANSPARENT_COLOR
    // become transparent.

    void streamHandPng(const uint16_t* src, int srcW, int w, int h) {
        const size_t rawRow = 1 + (size_t)w * 4, rawSize = rawRow * h;
        const size_t blocks = (rawSize + 65534) / 65535;
        const size_t zlibLen = 2 + 5 * blocks + rawSize + 4;
        webserver.setContentLength(8 + 25 + 12 + zlibLen + 12);
        webserver.sendHeader("Cache-Control", "no-store");
        webserver.send(200, "image/png", "");

        uint8_t buf[512];
        size_t fill = 0;
        uint32_t crc = 0;
        auto flush = [&]() {
            if (fill) webserver.sendContent((const char*)buf, fill);
            fill = 0;
        };
        auto put = [&](const uint8_t* data, size_t n, bool inCrc) {
            if (inCrc) crc = crc32Update(crc, data, n);
            while (n) {
                size_t k = min(n, sizeof(buf) - fill);
                memcpy(buf + fill, data, k);
                fill += k; data += k; n -= k;
                if (fill == sizeof(buf)) flush();
            }
        };
        auto putBE = [&](uint32_t v, bool inCrc) {
            uint8_t b[4] = { (uint8_t)(v >> 24), (uint8_t)(v >> 16), (uint8_t)(v >> 8), (uint8_t)v };
            put(b, 4, inCrc);
        };

        static const uint8_t sig[8] = { 0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A };
        put(sig, 8, false);
        uint8_t ihdr[13] = { 0, 0, (uint8_t)(w >> 8), (uint8_t)w, 0, 0, (uint8_t)(h >> 8), (uint8_t)h, 8, 6, 0, 0, 0 };
        putBE(13, false);
        crc = 0;
        put((const uint8_t*)"IHDR", 4, true);
        put(ihdr, 13, true);
        putBE(crc, false);

        putBE(zlibLen, false);
        crc = 0;
        put((const uint8_t*)"IDAT", 4, true);
        static const uint8_t zlibHead[2] = { 0x78, 0x01 };
        put(zlibHead, 2, true);
        uint32_t adlerA = 1, adlerB = 0;
        size_t done = 0, blockLeft = 0;
        int off = (w < srcW) ? (srcW - w) / 2 : 0;
        uint8_t row[1 + CLOCK_MAX * 4];
        for (int y = 0; y < h; y++) {
            row[0] = 0; // Filter-Byte: keine Filterung / filter byte: no filtering
            for (int x = 0; x < w; x++) {
                int sx = x + off;
                uint16_t px = (sx < srcW) ? src[y * srcW + sx] : TRANSPARENT_COLOR;
                uint8_t* p = row + 1 + x * 4;
                p[0] = ((px >> 11) & 0x1F) * 255 / 31;
                p[1] = ((px >> 5) & 0x3F) * 255 / 63;
                p[2] = (px & 0x1F) * 255 / 31;
                p[3] = (px == TRANSPARENT_COLOR || px == 0xFFFF) ? 0 : 255;
            }

            // In Deflate-Bloecke zu hoechstens 65535 Byte aufteilen
            // Split into deflate blocks of at most 65535 bytes

            size_t pos = 0;
            while (pos < rawRow) {
                if (blockLeft == 0) {
                    size_t len = min((size_t)65535, rawSize - done);
                    uint8_t blockHead[5] = { (uint8_t)(done + len >= rawSize ? 1 : 0), (uint8_t)len, (uint8_t)(len >> 8),
                                             (uint8_t)~len, (uint8_t)(~len >> 8) };
                    put(blockHead, 5, true);
                    blockLeft = len;
                }
                size_t k = min(blockLeft, rawRow - pos);
                put(row + pos, k, true);
                for (size_t i = 0; i < k; i++) {
                    adlerA = (adlerA + row[pos + i]) % 65521;
                    adlerB = (adlerB + adlerA) % 65521;
                }
                pos += k; done += k; blockLeft -= k;
            }
        }
        putBE((adlerB << 16) | adlerA, true);
        putBE(crc, false);

        putBE(0, false);
        crc = 0;
        put((const uint8_t*)"IEND", 4, true);
        putBE(crc, false);
        flush();
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


    // Liest die BMP-/RLEB-Header-Informationen und gibt sie als String zurück - leer, wenn die Datei kein Bild ist
    // Reads the BMP/RLEB header info and returns it as a string - empty if the file is not an image

    String getBmpInfo(const String& filename) {

        // Normalisiere Pfad (einfach und eindeutig)
        // Normalize path (simple and unambiguous)

        String file = filename;
        if (!file.startsWith("/")) file = "/" + file;

        File bmp = LittleFS.open(file, "r");
        if (!bmp) {
            return "";
        }
        uint8_t magic[4];
        if (bmp.read(magic, 4) != 4) {
            bmp.close();
            return "";
        }

        if (isRleFace(magic)) {
            uint8_t rest[16];
            bool ok = bmp.read(rest, 16) == 16;
            bmp.close();
            if (!ok) return "";
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
            return "";
        }

        int32_t width = *(int32_t*)&header[18];
        int32_t height = *(int32_t*)&header[22];
        uint16_t bpp = *(uint16_t*)&header[28];
        bmp.close();

        return String(abs(width)) + " x " + String(abs(height)) + " / " + String(bpp) + " bpp";
    }


    // Wie scaleAndSaveBmp(), aber ohne Vollbild im RAM (ohne PSRAM): jede Zielzeile entsteht aus der passenden
    // Quellzeile und geht ueber writeRleImage() bzw. direkt als BMP in eine Zwischendatei, die danach das Ziel
    // ersetzt - Quelle und Ziel duerfen gleich sein. Braucht dafuer kurz Platz fuer beide Dateien.

    // Like scaleAndSaveBmp(), but without a full frame in RAM (without PSRAM): every target row comes from the
    // matching source row and goes via writeRleImage() or directly as BMP into a temporary file, which then
    // replaces the target - source and target may be the same. Briefly needs room for both files.

    bool scaleAndSaveBmpLowMem(const char* sourcePath, const char* targetPath, int outW, int outH) {
        const char* TMP_PATH = "/scale_tmp.bmp";
        if (outW <= 0 || outH <= 0 || outW > CLOCK_MAX) return false;
        File src = LittleFS.open(sourcePath, "r");
        if (!src) return false;

        uint8_t head[54];
        int inW = 0, inH = 0;     // int statt int32_t: auf dem ESP32-C6 ist int32_t ein long (min())
                                  // int instead of int32_t: on the ESP32-C6 int32_t is a long (min())
        uint16_t bpp = 16;
        bool rle = false, flip = false, rgb555 = false;
        uint32_t offset = 0, rleSize = 0;
        size_t inRowSize = 0;
        if (src.read(head, 20) == 20 && isRleFace(head)) {
            rle = true;
            inW = *(int32_t*)&head[4];
            inH = *(int32_t*)&head[8];
            rleSize = *(uint32_t*)&head[12];
            if (inW <= 0 || inH <= 0 || *(uint32_t*)&head[16] != (uint32_t)inW * inH * 2) {
                src.close();
                DEBUG_PRINTLN("[BMP Scale] Invalid RLEB header");
                return false;
            }
        }
        else {
            src.seek(0);
            if (src.read(head, 54) != 54 || head[0] != 'B' || head[1] != 'M') {
                src.close();
                DEBUG_PRINTLN("[BMP Scale] Invalid BMP header");
                return false;
            }
            inW = *(int32_t*)&head[18];
            inH = *(int32_t*)&head[22];
            bpp = *(uint16_t*)&head[28];
            offset = *(uint32_t*)&head[10];
            uint32_t compression = *(uint32_t*)&head[30];
            if (inW <= 0 || inH == 0 || (bpp != 16 && bpp != 24 && bpp != 32) || (compression != 0 && compression != 3)) {
                src.close();
                DEBUG_PRINTLN("[BMP Scale] Unsupported BMP format: " + String(bpp) + " bpp, compression " + String(compression));
                return false;
            }
            if (bpp == 16) {
                uint32_t redMask = 0;
                if (compression == 3) {
                    src.seek(54);
                    src.read((uint8_t*)&redMask, 4);
                }
                rgb555 = compression == 0 || redMask == 0x7C00;
            }
            flip = inH > 0;
            inH = abs(inH);
            inRowSize = ((inW * (bpp / 8) + 3) / 4) * 4;
        }

        // Eine Quellzeile: roh (BMP) bzw. dekodiert (RLEB)
        // One source row: raw (BMP) or decoded (RLEB)

        uint8_t* srcRow = (uint8_t*)malloc(rle ? (size_t)inW * 2 : inRowSize);
        if (!srcRow) {
            src.close();
            DEBUG_PRINTLN("[BMP Scale] Memory allocation failed (source row)");
            return false;
        }

        String target = String(targetPath);
        if (!target.startsWith("/")) target = "/" + target;
        bool isFace = target.startsWith("/face_");
        bool storeAsRle = isFace || target.startsWith("/hand_set") || target.startsWith("/strip_");
        float scaleX = (float)inW / outW, scaleY = (float)inH / outH;
        float cx = outW / 2.0f, cy = outH / 2.0f, radius = min(outW, outH) / 2.0f;
        bool readError = false;
        RleFaceReader reader;
        int decodedRow = -1;

        auto rowFn = [&](int y, uint16_t* out) {
            int srcY = min(inH - 1, (int)(y * scaleY));
            uint16_t* row16 = (uint16_t*)srcRow;
            if (rle) {

                // Zeilen laufen nur vorwaerts - je Durchgang von writeRleImage() von vorn
                // Rows only advance - from the start for every pass of writeRleImage()

                if (y == 0) {
                    src.seek(20);
                    reader = RleFaceReader();
                    reader.file = src;
                    reader.remaining = rleSize;
                    decodedRow = -1;
                }
                while (decodedRow < srcY) {
                    for (int x = 0; x < inW; x++) {
                        uint16_t px = 0xFFFF;
                        if (!reader.nextPixel(px)) readError = true;
                        row16[x] = px;
                    }
                    decodedRow++;
                }
            }
            else {
                src.seek(offset + inRowSize * (flip ? inH - 1 - srcY : srcY));
                if (src.read(srcRow, inRowSize) != inRowSize) readError = true;
            }
            for (int x = 0; x < outW; x++) {
                int srcX = min(inW - 1, (int)(x * scaleX));
                uint16_t pixel;
                if (rle) pixel = row16[srcX];
                else if (bpp == 16) {
                    pixel = row16[srcX];
                    if (rgb555) pixel = ((pixel & 0x7FE0) << 1) | ((pixel >> 4) & 0x20) | (pixel & 0x1F);
                }
                else {
                    const uint8_t* p = srcRow + srcX * (bpp / 8);
                    pixel = ((p[2] & 0xF8) << 8) | ((p[1] & 0xFC) << 3) | (p[0] >> 3);
                }

                // Runde Displays: ausserhalb des Kreises weiss (wie scaleAndSaveBmp())
                // Round displays: white outside the circle (like scaleAndSaveBmp())

                if (isFace && displayGeom->round) {
                    float dx = (x + 0.5f) - cx, dy = (y + 0.5f) - cy;
                    if (dx * dx + dy * dy > radius * radius) pixel = 0xFFFF;
                }
                out[x] = pixel;
            }
        };

        LittleFS.remove(TMP_PATH);
        bool ok;
        if (storeAsRle) {
            ok = writeRleImage(TMP_PATH, outW, outH, rowFn);
        }
        else {
            uint8_t bmpHeader[66];
            buildBmpHeader565(bmpHeader, outW, outH);
            File out = LittleFS.open(TMP_PATH, "w");
            ok = out && out.write(bmpHeader, 66) == 66;
            const int rowSize = ((outW * 2 + 3) / 4) * 4;
            uint16_t row[CLOCK_MAX + 2] = { 0 };
            for (int y = 0; ok && y < outH; y++) {
                rowFn(y, row);
                ok = out.write((uint8_t*)row, rowSize) == (size_t)rowSize;
            }
            if (out) out.close();
        }
        src.close();
        free(srcRow);
        if (!ok || readError) {
            LittleFS.remove(TMP_PATH);
            DEBUG_PRINTLN("[BMP Scale] Low-memory scaling failed for " + target);
            return false;
        }
        LittleFS.remove(target);
        if (!LittleFS.rename(TMP_PATH, target)) {
            LittleFS.remove(TMP_PATH);
            return false;
        }
        DEBUG_PRINTLN("[BMP Scale] Saved " + target + " (low-memory, " + String(outW) + "x" + String(outH) + ")");
        return true;
    }

    // Skaliert eine BMP-Datei auf die gewünschte Größe und speichert sie
    // Scales a BMP file to the desired size and saves it

    bool scaleAndSaveBmp(const char* sourcePath, const char* targetPath, int outW, int outH) {
        DEBUG_PRINTLN("[BMP Scale] Scaling BMP: " + String(sourcePath) + " to " + String(targetPath));
        if (lowMemoryFace()) return scaleAndSaveBmpLowMem(sourcePath, targetPath, outW, outH);
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
        bool rgb555 = false;         // 16-Bit-BMP mit 5 statt 6 Bit Gruen
                                     // 16-bit BMP with 5 instead of 6 green bits
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
            uint32_t compression = *(uint32_t*)&header[30];

            if (inW <= 0 || abs(inH) <= 0) {
                bmp.close();
                DEBUG_PRINTLN("[BMP Scale] Invalid BMP dimensions");
                return false;
            }

            // Nur unkomprimiert (BI_RGB) oder mit Bitmasken (BI_BITFIELDS) in 16, 24 oder 32 Bit - andere Formate
            // (Palette, RLE) wuerden sonst still als schwarzes Bild gespeichert.

            // Only uncompressed (BI_RGB) or with bit masks (BI_BITFIELDS) in 16, 24 or 32 bit - other formats
            // (palette, RLE) would otherwise be stored silently as a black image.

            if ((bpp != 16 && bpp != 24 && bpp != 32) || (compression != 0 && compression != 3)) {
                bmp.close();
                DEBUG_PRINTLN("[BMP Scale] Unsupported BMP format: " + String(bpp) + " bpp, compression " + String(compression));
                return false;
            }

            // 16 Bit ohne Bitmasken ist laut BMP-Standard RGB555, mit Bitmasken zeigt die Rotmaske das Format
            // 16 bit without bit masks is RGB555 per the BMP standard, with bit masks the red mask shows the format

            if (bpp == 16) {
                uint32_t redMask = 0;
                if (compression == 3) {
                    bmp.seek(54);
                    bmp.read((uint8_t*)&redMask, 4);
                }
                rgb555 = compression == 0 || redMask == 0x7C00;
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
                    if (rgb555) pixel = ((pixel & 0x7FE0) << 1) | ((pixel >> 4) & 0x20) | (pixel & 0x1F);
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

        // Zielformat: face_*.bmp UND hand_set*.bmp werden RLE-komprimiert (spart Flash-Platz, bei Zeigern
        // wegen grosser einfarbiger Flaechen noch mehr) - alles andere bleibt Standard-BMP.

        // Target format: face_*.bmp AND hand_set*.bmp are RLE-compressed (saves flash space, even more so for
        // hands due to large solid-color areas) - everything else stays standard BMP.

        String targetPathStr = String(targetPath);
        if (!targetPathStr.startsWith("/")) targetPathStr = "/" + targetPathStr;
        bool isFaceTarget = targetPathStr.startsWith("/face_");
        bool isHandTarget = targetPathStr.startsWith("/hand_set");
        bool isStripTarget = targetPathStr.startsWith("/strip_"); // Streifen-Grafik zum Zifferblatt (ILI9341)
                                                                   // strip graphic of a clock face (ILI9341)
        bool storeAsRle = isFaceTarget || isHandTarget || isStripTarget;

        // Bildpuffer ist quadratisch, runde Displays (GC9A01/GC9D01) zeigen
        // aber nur einen Kreis - alles ausserhalb wird weiss. ILI9341
        // (rechteckig) behaelt die Ecken, Maskierung gilt nur fuer Zifferblaetter.

        // Image buffer is square, but round displays (GC9A01/GC9D01) only show
        // a circle - everything outside is set white. ILI9341 (rectangular)
        // keeps the corners, masking only applies to clock faces.

        if (isFaceTarget && displayGeom->round) {
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
        if (!displayGeom->round) return; // Rechteckiges Display (ILI9341) - keine Kreismaskierung noetig
                                         // rectangular display (ILI9341) - no circular masking needed
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


    // Liest ein RLEB-Zifferblatt Pixel fuer Pixel aus der Datei (ab der Position nach dem Kopf), mit kleinem
    // Lesepuffer statt Vollpuffer. next() liefert false am Ende oder bei einem Lesefehler.

    // Reads an RLEB clock face pixel by pixel from the file (from the position after the header), with a small
    // read buffer instead of a full buffer. next() returns false at the end or on a read error.

    struct RleFileReader {
        File& file;
        size_t remaining;
        uint8_t buf[512];
        size_t pos = 0, len = 0;
        size_t runLeft = 0;
        bool runLiteral = false;
        uint16_t runPx = 0;

        RleFileReader(File& f, size_t compressedSize) : file(f), remaining(compressedSize) {}

        bool byte(uint8_t& out) {
            if (pos >= len) {
                if (remaining == 0) return false;
                len = file.read(buf, remaining < sizeof(buf) ? remaining : sizeof(buf));
                pos = 0;
                if (len == 0) return false;
                remaining -= len;
            }
            out = buf[pos++];
            return true;
        }

        bool next(uint16_t& px) {
            uint8_t b0, b1;
            if (runLeft == 0) {
                uint8_t ctrl;
                if (!byte(ctrl)) return false;
                runLiteral = ctrl <= 127;
                runLeft = runLiteral ? ctrl + 1 : 257 - ctrl;
                if (!runLiteral) {
                    if (!byte(b0) || !byte(b1)) return false;
                    runPx = b0 | (b1 << 8);
                }
            }
            if (runLiteral) {
                if (!byte(b0) || !byte(b1)) return false;
                px = b0 | (b1 << 8);
            }
            else {
                px = runPx;
            }
            runLeft--;
            return true;
        }
    };


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

            RleFileReader rle(f, compressedSize);
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
            uint16_t px;

            while (written < total && nextOutRow < outH && rle.next(px)) {
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


    // Breite und Hoehe einer RLEB-Datei - false, wenn sie kein gueltiger RLEB-Kopf ist
    // Width and height of an RLEB file - false if it has no valid RLEB header

    bool readRleSize(const String& path, int32_t& w, int32_t& h) {
        w = 0;
        h = 0;
        File f = LittleFS.open(path, "r");
        if (!f) return false;
        uint8_t head[20];
        bool rle = f.read(head, sizeof(head)) == sizeof(head) && isRleFace(head);
        f.close();
        if (!rle) return false;
        w = *(int32_t*)&head[4];
        h = *(int32_t*)&head[8];
        return w > 0 && h > 0 && *(uint32_t*)&head[16] == (uint32_t)w * h * 2;
    }

    // Groesse als Standard-BMP: 66-Byte-Kopf, Zeilen auf 4 Byte aufgefuellt
    // Size as a standard BMP: 66-byte header, rows padded to 4 bytes

    size_t bmpFileSize565(int32_t w, int32_t h) {
        return 66 + (size_t)((w * 2 + 3) / 4 * 4) * h;
    }

    // 66-Byte-Kopf eines Top-down-BMP mit 16 Bit RGB565 (BI_BITFIELDS)
    // 66-byte header of a top-down BMP with 16-bit RGB565 (BI_BITFIELDS)

    void buildBmpHeader565(uint8_t* hdr, int32_t w, int32_t h) {
        memset(hdr, 0, 66);
        hdr[0] = 'B'; hdr[1] = 'M';
        *(uint32_t*)&hdr[2] = bmpFileSize565(w, h);
        *(uint32_t*)&hdr[10] = 66;
        *(uint32_t*)&hdr[14] = 40;
        *(int32_t*)&hdr[18] = w;
        *(int32_t*)&hdr[22] = -h;
        *(uint16_t*)&hdr[26] = 1;
        *(uint16_t*)&hdr[28] = 16;
        *(uint32_t*)&hdr[30] = 3;
        *(uint32_t*)&hdr[34] = bmpFileSize565(w, h) - 66;
        *(uint32_t*)&hdr[54] = 0xF800;
        *(uint32_t*)&hdr[58] = 0x07E0;
        *(uint32_t*)&hdr[62] = 0x001F;
    }

    // Gibt eine RLEB-Datei als Standard-BMP ueber 'out' aus, Zeile fuer Zeile (nie mehr als eine Bildzeile im
    // RAM) und immer in voller Laenge (bmpFileSize565()), fehlende Pixel als Nullen. False, wenn sie kein
    // gueltiges RLEB ist (dann wird nichts ausgegeben) oder Daten fehlten.

    // Outputs an RLEB file as a standard BMP via 'out', row by row (never more than one image row in RAM) and
    // always at full length (bmpFileSize565()), missing pixels as zeros. False if it is not a valid RLEB (then
    // nothing is output) or data was missing.

    bool writeRleAsBmp(const String& path, const std::function<void(const uint8_t*, size_t)>& out) {
        int32_t w, h;
        if (!readRleSize(path, w, h)) return false;
        File f = LittleFS.open(path, "r");
        if (!f) return false;
        uint8_t head[20];
        f.read(head, sizeof(head));
        const int rowSize = (w * 2 + 3) / 4 * 4;
        uint8_t* rowBuf = new (std::nothrow) uint8_t[rowSize];
        if (!rowBuf) {
            f.close();
            return false;
        }
        uint8_t hdr[66];
        buildBmpHeader565(hdr, w, h);
        out(hdr, sizeof(hdr));

        RleFileReader rle(f, *(uint32_t*)&head[12]);
        bool complete = true;
        uint16_t px;
        for (int y = 0; y < h; y++) {
            memset(rowBuf, 0, rowSize);
            for (int x = 0; x < w && complete; x++) {
                complete = rle.next(px);
                if (complete) {
                    rowBuf[x * 2] = px & 0xFF;
                    rowBuf[x * 2 + 1] = px >> 8;
                }
            }
            out(rowBuf, rowSize);
            if (y % 20 == 0) yield();
        }
        delete[] rowBuf;
        f.close();
        return complete;
    }

    // Sendet eine RLEB-Datei als Standard-BMP (fuer externe Tools lesbar). Kleine Bilder wie Zeiger in EINEM
    // Rutsch - sonst ueberwiegt der Overhead vieler sendContent()-Aufrufe -, grosse Zifferblaetter gestreamt.

    // Sends an RLEB file as a standard BMP (readable by external tools). Small images like hands in ONE go -
    // otherwise the overhead of many sendContent() calls dominates -, large clock faces streamed.

    bool streamRleFaceAsStandardBmp(const String& path, const char* contentType) {
        int32_t w, h;
        if (!readRleSize(path, w, h)) return false;
        const size_t fileSize = bmpFileSize565(w, h);

        if (fileSize <= 20000) {
            uint8_t* bmp = new (std::nothrow) uint8_t[fileSize];
            if (!bmp) return false;
            size_t fill = 0;
            bool ok = writeRleAsBmp(path, [&](const uint8_t* data, size_t n) {
                memcpy(bmp + fill, data, n);
                fill += n;
            });
            if (ok) webserver.send_P(200, contentType, (const char*)bmp, fileSize);
            delete[] bmp;
            return ok;
        }

        // Ab hier laeuft die Antwort schon, ein 500er ist nicht mehr moeglich
        // From here on the response is already running, a 500 is no longer possible

        webserver.setContentLength(CONTENT_LENGTH_UNKNOWN);
        webserver.send(200, contentType, "");
        bool ok = writeRleAsBmp(path, [](const uint8_t* data, size_t n) { webserver.sendContent_P((const char*)data, n); });
        webserver.sendContent("");
        return ok;
    }


    // Streifen eines Zifferblatts hochkant ins Sprite zeichnen (Demo-Zeit 10:10:30, heutiges Datum): seine
    // Einstellungen kurz laden, danach die aktuellen wiederherstellen. before = seine Lage.

    // Draw a clock face's strip in portrait into the sprite (demo time 10:10:30, today's date): load its settings
    // briefly, then restore the current ones. before = its placement.

    bool renderFaceStripPreview(const String& faceFile, bool& before) {
        int w = TFT_WIDTH, h = TFT_HEIGHT - CLOCK_HEIGHT;
        if (h <= 0) return false;
        ensureStripSettings();
        String saved = stripSettingsQuery(), savedFace = selectedBackground;
        loadStripSettingsForFace(faceFile);
        selectedBackground = faceFile; // Streifen-Grafik des Zifferblatts
                                       // strip graphic of the clock face
        struct tm demo = timeinfo;
        demo.tm_hour = 10;
        demo.tm_min = 10;
        demo.tm_sec = 30;
        bool ok = renderStripPreview(w, h, false, &demo);
        before = stripBefore;
        resetStripSettings();
        applyStripQuery(saved);
        selectedBackground = savedFace;
        return ok;
    }


    // Den zuletzt gezeichneten Streifen (infoStripSprite) verkleinert ab Zeile y0 in ein Sprite uebernehmen -
    // Mittelwert je Zielpixel, damit die Schrift lesbar bleibt.

    // Take over the last drawn strip (infoStripSprite) scaled down into a sprite from row y0 - average per target
    // pixel, so the text stays readable.

    void drawStripThumb(LGFX_Sprite& canvas, int y0, int w, int h) {
        const int sw = infoStripSprite.width(), sh = infoStripSprite.height();
        for (int y = 0; y < h; y++) {
            int sy0 = y * sh / h, sy1 = max(sy0 + 1, (y + 1) * sh / h);
            for (int x = 0; x < w; x++) {
                int sx0 = x * sw / w, sx1 = max(sx0 + 1, (x + 1) * sw / w);
                uint32_t r = 0, g = 0, b = 0, n = 0;
                for (int yy = sy0; yy < sy1; yy++) {
                    for (int xx = sx0; xx < sx1; xx++) {
                        uint16_t px = infoStripSprite.readPixel(xx, yy);
                        r += (px >> 11) & 0x1F; g += (px >> 5) & 0x3F; b += px & 0x1F; n++;
                    }
                }
                rowBuffer[x] = (uint16_t)(((r / n) << 11) | ((g / n) << 5) | (b / n));
            }
            canvas.pushImage(0, y0 + y, w, 1, rowBuffer);
        }
    }


    // Streifen eines Zifferblatts als kleines BMP fuer die Zifferblatt-Uebersicht: aktuelle Einstellungen,
    // Streifen-Grafik dieses Zifferblatts, Demo-Zeit 10:10:30 wie in den Preset-Vorschauen. Aufrufer gibt outBytes
    // mit delete[] frei.

    // A clock face's strip as a small BMP for the clock face overview: current settings, this face's strip
    // graphic, demo time 10:10:30 as in the preset previews. The caller frees outBytes with delete[].

    bool generateFaceStripBmp(const String& faceFile, int outW, uint8_t** outBytes, size_t& outSize) {
        const int outH = max(1, outW * (TFT_HEIGHT - CLOCK_HEIGHT) / CLOCK_WIDTH);
        bool before;
        if (TFT_HEIGHT <= CLOCK_HEIGHT || !renderFaceStripPreview(faceFile, before)) return false;
        LGFX_Sprite canvas(&tft);
        if (!createSprite16(canvas, outW, outH)) return false;
        drawStripThumb(canvas, 0, outW, outH);

        const int rowSize = ((outW * 2 + 3) / 4) * 4;
        const int fileSize = 66 + rowSize * outH;
        uint8_t* bmp = new (std::nothrow) uint8_t[fileSize];
        if (!bmp) return false;
        memset(bmp, 0, fileSize);
        bmp[0] = 'B'; bmp[1] = 'M';
        *(uint32_t*)&bmp[2] = fileSize;
        *(uint32_t*)&bmp[10] = 66;
        *(uint32_t*)&bmp[14] = 40;
        *(int32_t*)&bmp[18] = outW;
        *(int32_t*)&bmp[22] = -outH; // Top-down-BMP
                                     // top-down BMP
        *(uint16_t*)&bmp[26] = 1;
        *(uint16_t*)&bmp[28] = 16;
        *(uint32_t*)&bmp[30] = 3; // BI_BITFIELDS
        *(uint32_t*)&bmp[34] = rowSize * outH;
        *(uint32_t*)&bmp[54] = 0xF800;
        *(uint32_t*)&bmp[58] = 0x07E0;
        *(uint32_t*)&bmp[62] = 0x001F;
        for (int y = 0; y < outH; y++) {
            uint16_t* row = (uint16_t*)(bmp + 66 + y * rowSize);
            for (int x = 0; x < outW; x++) row[x] = canvas.readPixel(x, y);
        }
        *outBytes = bmp;
        outSize = (size_t)fileSize;
        return true;
    }


    // Zifferblatt verkleinert in 'canvas' ab Zeile oy, ohne Rohbild im RAM (ohne PSRAM): die RLEB-Datei wird
    // zeilenweise gelesen, je Zielzeile die passende Quellzeile. Fehlt die Datei, face_default.bmp.

    // Clock face scaled down into 'canvas' from row oy, without a raw image in RAM (without PSRAM): the RLEB
    // file is read row by row, the matching source row for every target row. If the file is missing, face_default.bmp.

    bool drawFaceThumbStreamed(const String& faceFile, LGFX_Sprite& canvas, int oy, int size) {
        String path = faceFile.startsWith("/") ? faceFile : "/" + faceFile;
        int32_t w, h;
        if (!(readRleSize(path, w, h) && w == CLOCK_WIDTH && h == CLOCK_HEIGHT)) {
            path = "/face_default.bmp";
            if (!(ensureDefaultFace() && readRleSize(path, w, h) && w == CLOCK_WIDTH && h == CLOCK_HEIGHT)) return false;
        }
        RleFaceReader r;
        r.file = LittleFS.open(path, "r");
        uint8_t head[20];
        if (!r.file || r.file.read(head, sizeof(head)) != sizeof(head)) return false;
        r.remaining = *(uint32_t*)&head[12];
        uint16_t line[CLOCK_MAX];
        int decoded = -1;
        for (int y = 0; y < size; y++) {
            int sy = y * CLOCK_HEIGHT / size;
            while (decoded < sy) {
                for (int x = 0; x < CLOCK_WIDTH; x++) {
                    uint16_t px = 0xFFFF;
                    r.nextPixel(px);
                    line[x] = px;
                }
                decoded++;
            }
            for (int x = 0; x < size; x++) rowBuffer[x] = line[x * CLOCK_WIDTH / size];
            canvas.pushImage(0, oy + y, size, 1, rowBuffer);
        }
        r.file.close();
        return true;
    }


    // Erzeugt ein Vorschaubild fuer die Preset-Verwaltung: Komposition aus Zifferblatt,
    // Zeigern (Demo-Zeit 10:10:30) und Mittelpunkt in angegebener Farbe/Groesse.
    // Sendet es als Standard-BMP an die laufende Web-Anfrage; false = nichts gesendet (kein Speicher).

    // Generates a preview image for preset management: composed of clock face,
    // hands (demo time 10:10:30), and center hub in the given color/size.
    // Sends it as a standard BMP to the current web request; false = nothing sent (no memory).

    bool sendPresetPreviewBmp(const String& faceFile, const String& handSetName,
        uint16_t hubColorRgb565, uint8_t hubSize, bool showSecond, const String& presetUrl) {

        checkHeapWarning("sendPresetPreviewBmp Start (" + faceFile + ")");

        // Vorschau als LovyanGFX-Sprite: Zeiger werden wie auf der Uhr mit
        // pushRotateZoomWithAA() gedreht und kantengeglaettet - gleicher Drehpunkt
        // (Sprite-Mitte bzw. HAND_WIDTH/2, HAND_PIVOT_Y), nur verkleinert.

        // Preview as a LovyanGFX sprite: hands are rotated and anti-aliased
        // with pushRotateZoomWithAA() like on the clock - same pivot (sprite
        // centre resp. HAND_WIDTH/2, HAND_PIVOT_Y), just scaled down.

        const int PREVIEW_SIZE = 100;

        // Mit Streifen (ILI9341): Bild so hoch wie das Display, Uhr und Streifen an der Lage des Presets
        // With a strip (ILI9341): image as tall as the display, clock and strip at the preset's placement

        const int stripPrevH = (TFT_HEIGHT > CLOCK_HEIGHT) ? PREVIEW_SIZE * (TFT_HEIGHT - CLOCK_HEIGHT) / CLOCK_WIDTH : 0;
        bool stripBeforeClock = false;
        bool stripOk = stripPrevH > 0 && renderFaceStripPreview(faceFile, stripBeforeClock);
        const int PREVIEW_H = PREVIEW_SIZE + stripPrevH;
        const int oy = stripBeforeClock ? stripPrevH : 0;
        LGFX_Sprite canvas(&tft);
        if (!createSprite16(canvas, PREVIEW_SIZE, PREVIEW_H)) return false;
        canvas.fillSprite(TFT_BLACK);
        canvas.setPivot(PREVIEW_SIZE / 2, oy + PREVIEW_SIZE / 2);

        // Streifen verkleinert uebernehmen (Mittelwert je Zielpixel, damit die Schrift lesbar bleibt)
        // Take over the strip scaled down (average per target pixel, so the text stays readable)

        if (stripOk) drawStripThumb(canvas, stripBeforeClock ? 0 : PREVIEW_SIZE, PREVIEW_SIZE, stripPrevH);

        // 1) Zifferblatt laden und auf Vorschaugroesse verkleinern (Datei, sonst das Standard-Zifferblatt)
        // 1) Load the clock face and scale it down to preview size (file, otherwise the default clock face)

        float faceScaleX = (float)CLOCK_WIDTH / PREVIEW_SIZE;
        float faceScaleY = (float)CLOCK_HEIGHT / PREVIEW_SIZE;

        // Ohne PSRAM passt kein Rohbild in den Speicher - dann zeilenweise aus der Datei
        // Without PSRAM no raw image fits into memory - then row by row from the file

        uint16_t* faceBuf = lowMemoryFace() ? nullptr : (uint16_t*)preferPsramMalloc((size_t)CLOCK_WIDTH * CLOCK_HEIGHT * 2);
        if (faceBuf) {
            loadFaceOrDefault(faceFile, faceBuf);
            for (int y = 0; y < PREVIEW_SIZE; y++) {
                int sy = (int)(y * faceScaleY);
                for (int x = 0; x < PREVIEW_SIZE; x++) {
                    int sx = (int)(x * faceScaleX);
                    rowBuffer[x] = faceBuf[sy * CLOCK_WIDTH + sx];
                }
                canvas.pushImage(0, oy + y, PREVIEW_SIZE, 1, rowBuffer);
            }
            free(faceBuf);
        }
        else if (!drawFaceThumbStreamed(faceFile, canvas, oy, PREVIEW_SIZE)) {
            return false;
        }

        // 2) Zeiger laden (aus Datei, sonst Standardsatz 0)
        // 2) Load hands (from file, otherwise default set 0)

        auto loadPreviewHand = [&](const char* label) -> uint16_t* {
            uint16_t* buf = (uint16_t*)preferPsramMalloc((size_t)HAND_WIDTH * HAND_HEIGHT * 2);
            if (buf) loadHandOrDefault(handSetName, label, buf);
            return buf;
        };
        uint16_t* hourPix = loadPreviewHand("hour");
        uint16_t* minutePix = loadPreviewHand("minute");
        uint16_t* secondPix = showSecond ? loadPreviewHand("second") : nullptr;

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

        // Sekundenfeld des Zifferblatts: Sekundenzeiger zuerst und an seinem Drehpunkt (wie renderClockFrame())
        // Seconds subdial of the clock face: second hand first and at its pivot (as in renderClockFrame())

        int16_t subX, subY;
        readFaceSecPivot(faceFile, subX, subY);
        const bool subdial = subX >= 0;
        const struct { uint16_t* pix; float angle; bool sub; } previewHands[] = {
            { subdial ? secondPix : nullptr, secondAngle, true },
            { hourPix, hourAngle, false }, { minutePix, minuteAngle, false },
            { subdial ? nullptr : secondPix, secondAngle, false }
        };
        for (const auto& h : previewHands) {
            if (!h.pix) continue;
            if (handSpriteOk) {
                for (int i = 0; i < HAND_WIDTH * HAND_HEIGHT; i++) {
                    if (h.pix[i] == 0xFFFF) h.pix[i] = TRANSPARENT_COLOR;
                }
                handSprite.pushImage(0, 0, HAND_WIDTH, HAND_HEIGHT, h.pix);
                if (h.sub) canvas.setPivot((subX + 0.5f) * handScale - 0.5f, oy + (subY + 0.5f) * handScale - 0.5f);
                handSprite.pushRotateZoomWithAA(&canvas, h.angle, handScale, handScale, TRANSPARENT_COLOR);
                if (h.sub) canvas.setPivot(PREVIEW_SIZE / 2, oy + PREVIEW_SIZE / 2);
            }
            free(h.pix);
        }

        // 4) Mittelpunkt (Hub) in der angegebenen Farbe/Groesse - wie auf der
        // Uhr kantengeglaettet um die Sprite-Mitte (fillSmoothCircle())

        // 4) Center hub in the given color/size - anti-aliased around the
        // sprite centre as on the clock (fillSmoothCircle())

        int hubRadius = (int)roundf(hubSize * handScale);
        if (hubRadius < 1) hubRadius = 1;
        canvas.fillSmoothCircle(PREVIEW_SIZE / 2, oy + PREVIEW_SIZE / 2, hubRadius, hubColorRgb565);

        // 5) Als Standard-BMP (mit BI_BITFIELDS-Header) senden - in Bloecken zu 8 Zeilen statt als ganze Kopie
        // im RAM (37 KB bei 100 x 186), die bei mehreren Anfragen kurz nacheinander oft nicht mehr frei waren.

        // 5) Send as a standard BMP (with BI_BITFIELDS header) - in blocks of 8 rows instead of a full copy in
        // RAM (37 KB at 100 x 186), which often was no longer free with several requests in quick succession.

        const int rowSize = ((PREVIEW_SIZE * 2 + 3) / 4) * 4;
        const int dataSize = rowSize * PREVIEW_H;
        const int fileSize = 66 + dataSize;
        const int BLOCK_ROWS = 8;

        uint8_t* block = new (std::nothrow) uint8_t[rowSize * BLOCK_ROWS];
        if (!block) return false;

        uint8_t header[66] = {};
        header[0] = 'B'; header[1] = 'M';
        *(uint32_t*)&header[2] = fileSize;
        *(uint32_t*)&header[10] = 66;
        *(uint32_t*)&header[14] = 40;
        *(int32_t*)&header[18] = PREVIEW_SIZE;
        *(int32_t*)&header[22] = -PREVIEW_H; // Top-down-BMP
                                             // Top-down BMP
        *(uint16_t*)&header[26] = 1;
        *(uint16_t*)&header[28] = 16;
        *(uint32_t*)&header[30] = 3; // BI_BITFIELDS
                                     // BI_BITFIELDS
        *(uint32_t*)&header[34] = dataSize;
        *(uint32_t*)&header[54] = 0xF800;
        *(uint32_t*)&header[58] = 0x07E0;
        *(uint32_t*)&header[62] = 0x001F;

        webserver.setContentLength(fileSize);
        webserver.send(200, "image/bmp", "");
        webserver.sendContent((const char*)header, sizeof(header));

        // readPixel() liefert RGB565 in RAM-Reihenfolge - das Sprite selbst
        // speichert in Display-Reihenfolge, daher nicht direkt kopieren.

        // readPixel() returns RGB565 in RAM byte order - the sprite itself
        // stores display byte order, so don't copy it directly.

        memset(block, 0, rowSize * BLOCK_ROWS);
        for (int y0 = 0; y0 < PREVIEW_H; y0 += BLOCK_ROWS) {
            int rows = min(BLOCK_ROWS, PREVIEW_H - y0);
            for (int r = 0; r < rows; r++) {
                uint16_t* row = (uint16_t*)(block + r * rowSize);
                for (int x = 0; x < PREVIEW_SIZE; x++) row[x] = canvas.readPixel(x, y0 + r);
            }
            webserver.sendContent((const char*)block, rows * rowSize);
        }
        delete[] block;
        return true;
    }


    // Schaltet die LED aus - ohne Board-LED (LED_BOARD -1, ESP32-S3-LCD-1.28) nichts
    // Turns the LED off - nothing without a board LED (LED_BOARD -1, ESP32-S3-LCD-1.28)

    void setLedOff() {
        if (LED_BOARD < 0) return;
        pinMode(LED_BOARD, OUTPUT);
        digitalWrite(LED_BOARD, LOW);
    }


    // Schaltet die LED ein - ohne Board-LED nichts
    // Turns the LED on - nothing without a board LED

    void setLedOn() {
        if (LED_BOARD < 0) return;
        pinMode(LED_BOARD, OUTPUT);
        digitalWrite(LED_BOARD, HIGH);
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

        // loadClockFace() ruft dies jeden Tick - Zeiger-Sprites nur bei echter Breitenaenderung neu anlegen.
        // Verglichen wird die TATSAECHLICHE Sprite-Breite, die globalen Breiten setzt der Aufrufer vorher
        // selbst.

        // loadClockFace() calls this every tick - recreate the hand sprites only on an actual width change.
        // The ACTUAL sprite width is compared, the caller sets the global widths itself beforehand.

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






