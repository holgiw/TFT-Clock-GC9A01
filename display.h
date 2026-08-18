#pragma once
    // ### Display: Zifferblatt, Zeiger, Sprites, Helligkeit, Touch ########
    // Benoetigt globals.h, config.h, prefs_keys.h und declarations.h (werden
    // zentral in uhr3.ino VOR dieser Datei eingebunden).

    // ### Display: clock face, hands, sprites, brightness, touch ########
    // Requires globals.h, config.h, prefs_keys.h and declarations.h (included
    // centrally in uhr3.ino BEFORE this file).

    // Bevorzugt PSRAM fuer Allokationen, die nicht dauerhaft gehalten werden
    // (kurzlebige Puffer waehrend BMP/PNG-Verarbeitung im Webinterface) -
    // haelt den knappen internen Heap frei von Fragmentierung durch die
    // vielen unterschiedlich grossen malloc()/free()-Zyklen. Nutzt bewusst
    // einen eigenen psramFound()-Check statt der globalen Variable
    // psramAvailable, da diese nur fuer die GC9D01-Hardware-Rotation gilt
    // und auf anderen Boards immer false ist, auch wenn dort PSRAM
    // vorhanden ist (siehe deren Verwendung bei clockFaceBuffer weiter
    // unten, die bewusst unveraendert bleibt).

    // Prefers PSRAM for allocations that are not held long-term (short-
    // lived buffers during BMP/PNG processing in the web interface) - keeps
    // the scarce internal heap free of fragmentation caused by the many
    // differently-sized malloc()/free() cycles. Deliberately uses its own
    // psramFound() check instead of the global psramAvailable variable,
    // since that one only applies to the GC9D01 hardware rotation
    // workaround and is always false on other boards, even where PSRAM is
    // actually present (see its use for clockFaceBuffer further below,
    // which is deliberately left unchanged).

    void* preferPsramMalloc(size_t size) {
        if (psramFound()) {
            void* p = ps_malloc(size);
            if (p) return p;
        }
        return malloc(size);
    }


#if defined CS_2


    // Waehlt bei Dual-Display-Aufbauten das erste TFT ueber sein Chip-Select-Pin aus
    // (deaktiviert dabei das zweite Display)

    // Selects the first TFT via its chip-select pin in dual-display setups
    // (disables the second display)

    void setCS1(bool state) {
        if (state == LOW) {
            digitalWrite(CS_2, HIGH);
        }
    
    }


    // Waehlt das zweite TFT bei Dual-Display-Aufbauten ueber seinen Chip-Select-Pin aus
    // Selects the second TFT via its chip-select pin in dual-display setups

    void setCS2(bool state) {
        if (state == LOW) {
            digitalWrite(CS_2, state);
        }
    
    }
#endif


    // Passt die Helligkeit eines Pixels basierend auf der aktuellen Helligkeitseinstellung an.
    // Adjusts a pixel's brightness based on the current brightness setting.

    uint16_t setPixelBrightness(uint16_t pixel) {

#ifdef TFT_Backlight
        return pixel;
#else

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

        // Farbwerte zusammenfügen
        // Combine color values
        return r | g | b;
#endif
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
                f.read((uint8_t*)&dest[row * width], width * 2);
            }
            f.close();
            return true;
        }
    }


    // Laedt das aktuell gewaehlte Zifferblatt (selectedBackground) in clockFaceBuffer
    // (Standard-BMP oder RLEB, mit Fallback auf das eingebaute Standard-Zifferblatt),
    // wendet die aktuelle Helligkeit an und zeichnet es in backgroundSprite

    // Loads the currently selected clock face (selectedBackground) into clockFaceBuffer
    // (standard BMP or RLEB, falling back to the built-in default face),
    // applies the current brightness and draws it into backgroundSprite

    void loadClockFace() {
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
                DEBUG_PRINTLN("allocte ram: " + bufSize);
                DEBUG_PRINTLN("[PSRAM] Allocate ram");
                clockFaceBuffer = (uint16_t*)malloc(bufSize);
            }
            if (!clockFaceBuffer) {
                DEBUG_PRINTLN("[PSRAM] Error: couldnt allocate clockFaceBuffer RAM!");
                return;
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
                memcpy(clockFaceBuffer, clockFace, CLOCK_WIDTH * CLOCK_HEIGHT * sizeof(uint16_t));
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
                // Fallback ohne Cache: wie zuvor Pixel fuer Pixel direkt ins
                // Sprite kopieren (funktionsfaehig, nur ohne die Optimierung).

                // Fallback without cache: copy pixel by pixel into the
                // sprite as before (works, just without the optimization).
                for (int y = 0; y < CLOCK_HEIGHT; y++) {
                    for (int x = 0; x < CLOCK_WIDTH; x++) {
                        rowBuffer[x] = setPixelBrightness(clockFaceBuffer[y * CLOCK_WIDTH + x]);
                    }
                    backgroundSprite.pushImage(0, y, CLOCK_WIDTH, 1, rowBuffer);
                }
                return;
            }
            forceRecompute = true;
        }

        // Die teure Pixel-fuer-Pixel Helligkeitsanpassung nur durchlaufen, wenn
        // sich seit dem letzten Mal etwas geaendert hat (neues Zifferblatt oder
        // Helligkeit) - im Regelfall (jeder Tick) entfaellt dieser Durchlauf.

        // Only run the costly per-pixel brightness adjustment when
        // something changed since last time (new clock face or
        // brightness) - normally (every tick) this is skipped.
        if (forceRecompute || currentBrightness != lastAppliedBrightness) {
            for (int i = 0; i < CLOCK_WIDTH * CLOCK_HEIGHT; i++) {
                clockFaceBrightBuffer[i] = setPixelBrightness(clockFaceBuffer[i]);
            }
            lastAppliedBrightness = currentBrightness;
        }

        // Guenstiger Regelfall: den vorberechneten Puffer in einem Rutsch ins
        // Sprite kopieren - loescht dabei auch die alte Zeigerposition vom
        // letzten Tick, ohne die Helligkeit erneut pro Pixel berechnen zu muessen.

        // Cheap common case: copy the precomputed buffer into the
        // sprite in one go - this also clears the old hand position
        // from the last tick, without recalculating brightness per pixel.
        backgroundSprite.pushImage(0, 0, CLOCK_WIDTH, CLOCK_HEIGHT, clockFaceBrightBuffer);
    }


    // Buffer freigeben, wenn ein neues Zifferblatt gewählt wird
    // Free the buffer when a new clock face is selected

    void freeClockFaceBuffer() {
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


    // Lädt die Grafiken für die Zeiger eines Uhren-Widgets, entweder aus einer benutzerdefinierten Konfiguration oder aus Standardwerten.
    // Liest eine Zeiger-Bitmap-Datei (RLE-komprimiert oder roh) direkt in einen
    // einfachen Pixel-Puffer ein - fuer die Web-Vorschau des TATSAECHLICH aktiven
    // Zeigersatzes (nicht ueber TFT_eSprite, das nur fuer das Display genutzt wird).
    // Spiegelt die Format-Erkennung von loadHandBmp(), schreibt aber in ein Array.

    // Loads the hand graphics for a clock widget, either from a custom configuration or from defaults.
    // Reads a hand bitmap file (RLE-compressed or raw) directly into a
    // simple pixel buffer - for the web preview of the ACTUALLY active
    // hand set (not via TFT_eSprite, which is only used for the display).
    // Mirrors the format detection of loadHandBmp(), but writes into an array.

    bool loadHandPixelsForPreview(const char* filename, uint16_t* outBuffer, int width, int height) {
        File bmp = LittleFS.open(filename, "r");
        if (!bmp) return false;

        uint8_t magic[4];
        if (bmp.read(magic, 4) != 4) { bmp.close(); return false; }

        if (isRleFace(magic)) {
            uint8_t rest[16];
            if (bmp.read(rest, 16) != 16) { bmp.close(); return false; }
            int32_t bmpWidth = *(int32_t*)&rest[0];
            int32_t bmpHeight = *(int32_t*)&rest[4];
            uint32_t compressedSize = *(uint32_t*)&rest[8];
            uint32_t uncompressedSize = *(uint32_t*)&rest[12];

            if (bmpWidth != width || bmpHeight != height || uncompressedSize != (uint32_t)width * height * 2) {
                bmp.close();
                return false;
            }

            uint8_t* compBuf = (uint8_t*)preferPsramMalloc(compressedSize);
            if (!compBuf) { bmp.close(); return false; }
            if (bmp.read(compBuf, compressedSize) != compressedSize) {
                free(compBuf); bmp.close(); return false;
            }
            bmp.close();

            rleDecode565(compBuf, compressedSize, outBuffer, (size_t)width * height);
            free(compBuf);
            return true;
        }
        else {
            bmp.seek(0);
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
            int rowSize = ((width * 2 + 3) / 4) * 4;
            bmp.seek(offset);

            for (int y = 0; y < height; y++) {
                int row = flip ? height - 1 - y : y;
                if (bmp.read((uint8_t*)rowBuffer, rowSize) != rowSize) { bmp.close(); return false; }
                memcpy(&outBuffer[row * width], rowBuffer, width * 2);
            }
            bmp.close();
            return true;
        }
    }


    void loadHandSprites() {
        String setId = preferences.getString(PK_HANDSET, "");

        // DEBUG_PRINTLN("[HANDS] Active hand set: " + setId);

        bool usedDefault = false;
        if (setId != "" && setId != "default") {
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
                String path = "/hand_set" + setId + "_" + h.label + ".bmp";
            //    DEBUG_PRINTLN("[HANDS] Looking for: " + path);

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
                     //   DEBUG_PRINTLN("[HANDS] Failed to load " + h.label + ", fallback used");
                    }
                    else {
                      //  DEBUG_PRINTLN("[HANDS] Loaded " + h.label);
                    }
                    // DEBUG_PRINTLN("found");
                }
                else {
                    h.sprite->pushImage(0, 0, HAND_WIDTH, HAND_HEIGHT, h.fallback);
                    usedDefault = true;
                    // DEBUG_PRINTLN("[HANDS] Missing " + h.label + ", using default");
                }
            }

            if (!usedDefault) {
               // DEBUG_PRINTLN("[HANDS] Loaded handset: " + setId);
            }
            else {
                // DEBUG_PRINTLN("[HANDS] Incomplete set, used default for missing hands");
            }

        }
        else {
            for (int y = 0; y < HAND_HEIGHT; y++) {
                for (int x = 0; x < HAND_WIDTH; x++) {
                    rowBuffer[x] = setPixelBrightness(handHour[y * HAND_WIDTH + x]);
                }
                hourHandSprite.pushImage(0, y, HAND_WIDTH, 1, rowBuffer);

                for (int x = 0; x < HAND_WIDTH; x++) {
                    rowBuffer[x] = setPixelBrightness(handMinute[y * HAND_WIDTH + x]);
                }
                minuteHandSprite.pushImage(0, y, HAND_WIDTH, 1, rowBuffer);

                for (int x = 0; x < HAND_WIDTH; x++) {
                    rowBuffer[x] = setPixelBrightness(handSecond[y * HAND_WIDTH + x]);
                }
                secondHandSprite.pushImage(0, y, HAND_WIDTH, 1, rowBuffer);
            }

         //   DEBUG_PRINTLN("[HANDS] No set selected, using defaults");

        }
    }


    // Hilfsfunktion zum Laden von Zeiger-BMPs 
    // Helper function for loading hand BMPs

    bool loadHandBmp(TFT_eSprite* sprite, const char* filename, int width, int height) {
        File bmp = LittleFS.open(filename, "r");
        if (!bmp) return false;

        uint8_t magic[4];
        if (bmp.read(magic, 4) != 4) { bmp.close(); return false; }

        uint16_t* fullImage = nullptr; // nur im RLE-Zweig belegt (Zeiger sind klein genug fuer einen Komplett-Puffer)
                                       // only used in the RLE branch (hands are small enough for a full buffer)
        bool flip = false;
        int32_t bmpWidth = 0, bmpHeight = 0;
        uint32_t offset = 0;
        int rowSize = 0;

        if (isRleFace(magic)) {
            uint8_t rest[16];
            if (bmp.read(rest, 16) != 16) { bmp.close(); return false; }
            bmpWidth = *(int32_t*)&rest[0];
            bmpHeight = *(int32_t*)&rest[4];
            uint32_t compressedSize = *(uint32_t*)&rest[8];
            uint32_t uncompressedSize = *(uint32_t*)&rest[12];

            if (bmpWidth != width || bmpHeight != height || uncompressedSize != (uint32_t)width * height * 2) {
                bmp.close();
                return false;
            }

            uint8_t* compBuf = (uint8_t*)preferPsramMalloc(compressedSize);
            if (!compBuf) { bmp.close(); return false; }
            if (bmp.read(compBuf, compressedSize) != compressedSize) {
                free(compBuf); bmp.close(); return false;
            }
            bmp.close();

            fullImage = (uint16_t*)preferPsramMalloc(uncompressedSize);
            if (!fullImage) { free(compBuf); return false; }
            rleDecode565(compBuf, compressedSize, fullImage, (size_t)width * height);
            free(compBuf);

            flip = false; // RLEB ist immer bereits Top-Down gespeichert
                          // RLEB is always already stored top-down
        }
        else {
            bmp.seek(0);
            uint8_t header[54];
            if (bmp.read(header, 54) != 54 || header[0] != 'B' || header[1] != 'M') {
                bmp.close();
                return false;
            }

            bmpWidth = *(int32_t*)&header[18];
            bmpHeight = *(int32_t*)&header[22];
            uint16_t bpp = *(uint16_t*)&header[28];
            offset = *(uint32_t*)&header[10];

            if (bmpWidth != width || abs(bmpHeight) != height || bpp != 16) {
                bmp.close();
                return false;
            }

            flip = bmpHeight > 0;
            bmpHeight = abs(bmpHeight);
            rowSize = ((width * 2 + 3) / 4) * 4;
            bmp.seek(offset);
        }

        for (int y = 0; y < height; y++) {
            int row = flip ? height - 1 - y : y;

            if (fullImage) {
                memcpy(rowBuffer, &fullImage[row * width], width * 2);
            }
            else {
                if (bmp.read((uint8_t*)rowBuffer, rowSize) != rowSize) break;
            }

            uint16_t* pixelData = (uint16_t*)rowBuffer;
            for (int x = 0; x < width; x++) {

                if (pixelData[x] == 0xFFFF) {
                    pixelData[x] = TRANSPARENT_COLOR;
                }

                pixelData[x] = setPixelBrightness(pixelData[x]);

            }
            sprite->pushImage(0, row, width, 1, (uint16_t*)rowBuffer, (uint8_t)TRANSPARENT_COLOR);
        }

        if (fullImage) {
            free(fullImage);
        }
        else {
            bmp.close();
        }
        return true;
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

    static float lastHourAngle = 0.0f;
    static float lastMinuteAngle = 0.0f;


    // updateClock Funktion
    // updateClock function

    void updateClock() {
       // struct tm timeinfo;
        if (!getLocalTime(&timeinfo, 1000)) {
            // Keine gültige Uhrzeit verfügbar
            // No valid time available
            loadTimeFromRTC();
        }


        static unsigned long lastRtcReloadMillis = 0; // Zeitpunkt des letzten RTC-Lesevorgangs (eigenstaendig, NICHT dieselbe Variable wie das globale lastRTCUpdate in time_sync.h/getDCF77Time)
                                                      // timestamp of the last RTC read (independent, NOT the same variable as the global lastRTCUpdate in time_sync.h/getDCF77Time)
        if (rtcOk == RTC_AVAILABLE) {            
            // Überprüfen, ob seit dem letzten Aufruf Zeit vergangen ist
            // Check whether time has passed since the last call
            if (millis() - lastRtcReloadMillis >= WAIT_1h) {            
                loadTimeFromRTC();
                lastRtcReloadMillis = millis();
            }
        }
    

        int orientation = preferences.getUChar(PK_TFT_ROTATION, 0);

        float secAngle = timeinfo.tm_sec * 6.0f;
        float minAngle = timeinfo.tm_min * 6.0f;
        float hourAngle = (timeinfo.tm_hour % 12) * 30.0f + (timeinfo.tm_min / 2.0f) + (timeinfo.tm_sec / 120.0f);

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

        if (firstRun) {
            stationTick = timeinfo.tm_sec + 2;
            if (stationTick >= 60) {
                stationTick = 60;
            }   
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

    
        // Station Mode: Sekundenzeiger springt nicht, sondern läuft in 672ms Schritten mit sanfter Bewegung dazwischen
        // Station mode: second hand doesn't jump, but moves in 672ms steps with smooth motion in between
        if (stationMode) {
        
            if (!stationWaiting && currentMillis - stationLastMillis >= FAST_SECOND) {
                stationTick++;
                stationLastMillis += FAST_SECOND;

                if (stationTick >= 60) {
                    stationTick = 60;
                    stationWaiting = true;
                    stationWaitStartMinute = timeinfo.tm_min;
                }
            }
            else if (stationWaiting) {

                // Aufwachen ueber einen Minutenwechsel statt exakt "Sekunde 0"
                // pruefen: blockierende Operationen wie loadClockFace() beim
                // Wechsel von Zifferblatt/Preset (Bild laden + Pixel-fuer-Pixel
                // Helligkeitsberechnung) koennen updateClock() fuer einen Moment
                // aussetzen lassen. Traf das genau die eine Sekunde mit
                // tm_sec==0, wurde sie nie beobachtet und der Zeiger blieb bis
                // zur naechsten vollen Minute auf 12 stehen. Ein Minutenwechsel
                // bleibt dagegen auch bei einem verpassten Aufruf zuverlaessig
                // erkennbar, solange updateClock() irgendwann innerhalb der
                // neuen Minute wieder laeuft.

                // Wake up based on a minute change instead of checking for
                // exactly "second 0": blocking operations like loadClockFace()
                // when switching the clock face/preset (loading the image +
                // per-pixel brightness computation) can cause updateClock() to
                // be skipped for a moment. If that happened to hit the one
                // second where tm_sec==0, it was never observed and the hand
                // stayed parked at 12 until the next full minute. A minute
                // change, in contrast, stays reliably detectable even after a
                // missed call, as long as updateClock() runs again at some
                // point within the new minute.
                if (timeinfo.tm_min != stationWaitStartMinute) {
                    stationTick = 0;
                    stationWaiting = false;
                    stationLastMillis = currentMillis;

                    // Sekundenzeiger korrekt synchronisieren
                    // Synchronize the second hand correctly
                    secAngle = rotatedAngle(0, orientation);
                }
            }

            float subTick = (currentMillis - stationLastMillis) / FAST_SECOND;
            if (subTick > 1.0f || stationWaiting) subTick = 0.0f;

            float smoothSec = (stationTick >= 60) ? 60.0f : stationTick + easeInOutSine(subTick);
            secAngle = rotatedAngle(smoothSec * 6.0f, orientation);

            minAngle = rotatedAngle(timeinfo.tm_min * 6.0f, orientation);   
        }

        // Normaler Modus: Sekundenzeiger läuft normal, Minutenzeiger kann optional sanft laufen
        // Normal mode: second hand runs normally, minute hand can optionally move smoothly
        if (!stationMode) {
            secAngle = rotatedAngle(secAngle, orientation);
          
            smoothMinute = preferences.getBool(PK_SMOOTH_MINUTE, false);

            if (smoothMinute) {
                // Millisekunden einbeziehen
                // Include milliseconds
                /*unsigned long currentMillis = millis();
                int milliseconds = currentMillis % 1000;
                float smoothMinuteValue = timeinfo.tm_min + (timeinfo.tm_sec / 60.0f) + (milliseconds / 60000.0f);

                float rawMinAngle = smoothMinuteValue * 6.0f;
                float targetMinAngle = rotatedAngle(rawMinAngle, orientation);
                float angleDiff = shortestAngleDiff(lastMinuteAngle, targetMinAngle);

                // Sicherstellen, dass der Zeiger immer vorwärts läuft
                if (angleDiff < -180.0f) angleDiff += 360.0f;
                if (angleDiff > 180.0f) angleDiff -= 360.0f;

                lastMinuteAngle += angleDiff * 0.06f;  // noch feinere Bewegung
                */

                // Millisekunden einbeziehen
                // Include milliseconds
                unsigned long currentMillis = millis();
                int milliseconds = currentMillis % 1000;
                float smoothMinuteValue = timeinfo.tm_min + (timeinfo.tm_sec / 60.0f) + (milliseconds / 60000.0f);

                float rawMinAngle = smoothMinuteValue * 6.0f;
                minAngle = rotatedAngle(rawMinAngle, orientation);
                lastMinuteAngle = minAngle; // Direkt setzen, da wir den exakten Winkel berechnen
                                            // set directly since we compute the exact angle

            }
            else {
                // Normale Minutenanzeige mit sanfter Korrektur bei Wechsel
                // Normal minute display with smooth correction on change
                float rawMinAngle = timeinfo.tm_min * 6.0f;
                float targetMinAngle = rotatedAngle(rawMinAngle, orientation);
                float angleDiff = shortestAngleDiff(lastMinuteAngle, targetMinAngle);

                if (fabs(angleDiff) > 0.1f) {
                    lastMinuteAngle += angleDiff * 0.1f;
                    if (lastMinuteAngle < 0.0f) lastMinuteAngle += 360.0f;
                    if (lastMinuteAngle >= 360.0f) lastMinuteAngle -= 360.0f;
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
                                                    // smoothing factor
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

   
        // Nabe (hub)
        // hub
        if (hubSize > 0) {
           backgroundSprite.fillCircle(CLOCK_WIDTH / 2, CLOCK_HEIGHT / 2, hubSize, setPixelBrightness(hubColor));
        }

        backgroundSprite.pushSprite(0, 0);
    }


    // Aktualisiert die Helligkeit des Displays basierend auf der aktuellen Einstellung, 
    // dem ADC-Wert (falls aktiviert) und dem Tageszeitfenster für volle Helligkeit.

    // Updates the display brightness based on the current setting,
    // the ADC value (if enabled), and the full-brightness time window.

    void updateBrightness() {

        // Wenn Helligkeit geändert → neu zeichnen
        // If brightness changed -> redraw
        if (currentBrightness != lastAppliedBrightness) {
            loadClockFace();
            loadHandSprites();
            lastAppliedBrightness = currentBrightness;
        }

        // Prüfen, ob wir aktuell im konfigurierten Voll-Helligkeits-Zeitfenster sind
        // Check whether we're currently within the configured full-brightness time window
        bool withinDayWindow = false;
    
        // struct tm timeinfo;
        if (getLocalTime(&timeinfo, 500)) {
            int h = timeinfo.tm_hour;
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
    

        // Wenn Zeitfenster aktiv und wir innerhalb davon sind: volle Helligkeit erzwingen
        // If the time window is active and we're inside it: force full brightness
        if (withinDayWindow) {
            targetBrightness = maxBrightness;
#ifdef TFT_Backlight
            // sanfte Erhöhung, falls gewünscht (ähnlich wie ADC-Rampen)
            // smooth increase if desired (similar to ADC ramps)
            if (currentBrightness < targetBrightness) currentBrightness++;
            else if (currentBrightness > targetBrightness) currentBrightness--;
#else
            currentBrightness = targetBrightness;
#endif
        }
        else {
#ifdef ADC_PIN
            // Normale Auto-Brightness oder statische Helligkeit
            // Normal auto-brightness or static brightness
            if (useAdc) {


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

                int lightPercent = map(avg, 0, 4095, 5, 100);

                if (lightPercent < lowThreshold) targetBrightness = minBrightness;
                else if (lightPercent > highThreshold) targetBrightness = maxBrightness;
#ifdef TFT_Backlight
                else {
                    float norm = constrain((float)avg / 4095.0f, 0.0f, 1.0f);
                    float gamma = gammaBrightness;
                    float gammaNorm = powf(norm, gamma);
                    targetBrightness = minBrightness + (uint8_t)((maxBrightness - minBrightness) * gammaNorm + 0.5f);
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
                // kein ADC: Standardeinstellung
                // No ADC: default setting
                currentBrightness = minBrightness;
                targetBrightness = currentBrightness;
            }
#endif
        }

#ifdef TFT_Backlight
        ledcWrite(TFT_Backlight, currentBrightness);  // 0–255
                                                      // 0-255
#endif

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


    // Kodiert ein 16-Bit RGB565 Bild als PNG (RGBA, echte Alpha-Transparenz) und
    // gibt es als Base64-String zurueck. Sowohl die interne Transparenzfarbe
    // (TRANSPARENT_COLOR) als auch reines Weiss (0xFFFF) werden zu Alpha=0 -
    // im Gegensatz zu encodeBmpToBase64(), das BMP (ohne Alpha-Kanal) erzeugt
    // und Transparenz nur als sichtbares Weiss darstellen kann.

    // Encodes a 16-bit RGB565 image as PNG (RGBA, true alpha transparency) and
    // returns it as a base64 string. Both the internal transparency color
    // (TRANSPARENT_COLOR) and pure white (0xFFFF) become alpha=0 -
    // unlike encodeBmpToBase64(), which produces BMP (no alpha channel)
    // and can only show transparency as visible white.

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


    // Erzeugt rohe BMP-Bytes aus RGB565-Pixeldaten (ohne Base64) - wird sowohl
    // von encodeBmpToBase64() (fuer Inline-Einbettung) als auch von Routen, die
    // das Bild direkt als HTTP-Antwort senden, genutzt. Aufrufer muss delete[]
    // auf das Ergebnis aufrufen.
    //
    // WICHTIG: Nutzt BI_BITFIELDS (biCompression=3) mit expliziten RGB565-
    // Bitmasken statt des einfachen BI_RGB (0). Ohne diese Masken interpretieren
    // die meisten Bildbetrachter/Browser ein 16-Bit-BMP standardmaessig als
    // 5-5-5 (X1R5G5B5) statt der hier tatsaechlich genutzten 5-6-5-Kodierung -
    // bei Schwarz/Weiss faellt das nicht auf (alle Kanaele gleich), bei echten
    // Farben verschieben sich dadurch die Bit-Grenzen zwischen den Kanaelen und
    // das Bild wirkt wie Rauschen.

    // Generates raw BMP bytes from RGB565 pixel data (without base64) - used both
    // by encodeBmpToBase64() (for inline embedding) and by routes that
    // send the image directly as an HTTP response. Caller must delete[]
    // the result.
    //
    // IMPORTANT: Uses BI_BITFIELDS (biCompression=3) with explicit RGB565
    // bit masks instead of plain BI_RGB (0). Without these masks, most
    // image viewers/browsers interpret a 16-bit BMP by default as
    // 5-5-5 (X1R5G5B5) instead of the 5-6-5 encoding actually used here -
    // this isn't noticeable for black/white (all channels equal), but for real
    // colors the bit boundaries between channels shift and
    // the image looks like noise.

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

        uint8_t* bmpData = new uint8_t[fileSize];
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
#if defined CS_2
        setCS2(LOW);
        tft.fillRect(0, 0, CLOCK_WIDTH, CLOCK_HEIGHT, TFT_BLACK);
        setCS1(LOW);
#endif
        tft.fillRect(0, 0, CLOCK_WIDTH, CLOCK_HEIGHT, TFT_BLACK);
    }


    // Rotiert die Zeiger basierend auf der Display-Rotation.
    //
    // Fruehere Logik: addierte die Rotation zusaetzlich in Software, wenn
    // psramAvailable true war - das war urspruenglich fuer GC9D01 gedacht,
    // wo (bei vorhandenem PSRAM) die Hardware-Rotation bewusst uebersprungen
    // wurde (siehe die inzwischen entfernte GC9D01-Sonderbehandlung in
    // uhr3.ino) und die Zeiger deshalb per Software nachgedreht werden
    // mussten. Seit tft.setRotation() jetzt fuer alle Boards unbedingt
    // laeuft, wuerde diese Software-Rotation die Hardware-Rotation ein
    // zweites Mal draufaddieren (doppelte/inkonsistente Drehung der Zeiger,
    // sobald PSRAM vorhanden ist) - daher entfernt, angle bleibt unveraendert.

    // Rotates the hands based on the display rotation.
    //
    // Previous logic: additionally added the rotation in software when
    // psramAvailable was true - this was originally meant for GC9D01, where
    // (with PSRAM present) hardware rotation was deliberately skipped (see
    // the now-removed GC9D01 special handling in uhr3.ino), so the hands had
    // to be rotated in software instead. Since tft.setRotation() now always
    // runs unconditionally for every board, this software rotation would add
    // on top of the hardware rotation a second time (double/inconsistent
    // hand rotation whenever PSRAM is present) - so it was removed, angle is
    // returned unchanged.

    float rotatedAngle(float angle, int orientation) {
        (void)orientation; // Parameter bleibt fuer Aufrufkompatibilitaet erhalten, aktuell ungenutzt
                           // parameter kept for call-site compatibility, currently unused
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

        // --- Quelle einlesen: entweder RLEB (komplett dekodieren) oder ---
        // --- Standard-BMP (zeilenweise, wie bisher, speicherschonend)   ---

        // --- Read source: either RLEB (fully decode) or ---
        // --- standard BMP (row by row, as before, memory-friendly) ---
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

        uint16_t* outImage = new uint16_t[outW * outH];
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
                bmp.read(rowBuf, inRowSize);
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

        // Der Bildpuffer ist quadratisch, das sichtbare Display bei runden Modellen
        // (GC9A01/GC9D01) aber rund: dort wird alles ausserhalb des sichtbaren
        // Kreises auf Weiss gesetzt. Beim rechteckigen ILI9341 bleiben die Ecken
        // sichtbar, da dort kein physisch verdeckter Bereich existiert. Zeiger
        // werden einzeln gedreht, Maskierung gilt ohnehin nur fuer Zifferblaetter.

        // The image buffer is square, but the visible display on round models
        // (GC9A01/GC9D01) is round: everything outside the visible circle is
        // set to white there. On the rectangular ILI9341 the corners stay
        // visible, since no physically hidden area exists there. Hands are
        // rotated individually; masking only applies to clock faces anyway.
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

            if (scaleAndSaveBmp(path.c_str(), path.c_str(), HAND_WIDTH, HAND_HEIGHT)) {
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


    // Wendet die Kreismaskierung einmalig auf bereits vorhandene, schon RLE-
    // komprimierte Zifferblaetter an, die VOR Einfuehrung dieser Maskierung
    // migriert/hochgeladen wurden - nutzt peekFirstPixelIsWhite() zum Ueberspringen.
    // Nur fuer runde Displays relevant (siehe ROUND_DISPLAY in config.h).

    // Applies the circular mask once to existing, already RLE-
    // compressed clock faces that were migrated/uploaded BEFORE this
    // masking was introduced - uses peekFirstPixelIsWhite() to skip ones already done.
    // Only relevant for round displays (see ROUND_DISPLAY in config.h).

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

        uint8_t* outBmp = new uint8_t[outFileSize];
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

            uint16_t* srcRow = new uint16_t[inW];
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

            uint8_t* fullBmp = new uint8_t[fileSize];
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

        uint8_t* rowBuf = new uint8_t[rowSize];
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


    // Rotiert ein Zeigerbild um seinen Drehpunkt (pivotX/Y) und komponiert es auf
    // die Canvas (zentriert bei cx/cy, skaliert). angleDeg: 0=12-Uhr-Position,
    // im Uhrzeigersinn - nutzt inverse Rueckwaerts-Abbildung, Weiss/TRANSPARENT_COLOR = durchsichtig.

    // Rotates a hand image around its pivot point (pivotX/Y) and composites it onto
    // the canvas (centered at cx/cy, scaled). angleDeg: 0=12 o'clock position,
    // clockwise - uses inverse backward mapping, white/TRANSPARENT_COLOR = transparent.
    void blitRotatedHand(uint16_t* canvas, int canvasW, int canvasH,
        const uint16_t* hand, int handW, int handH,
        float pivotX, float pivotY,
        float cx, float cy, float angleDeg, float scale) {
        float rad = angleDeg * (float)PI / 180.0f;
        float cosA = cosf(rad), sinA = sinf(rad);

        float maxDim = sqrtf((float)(handW * handW + handH * handH)) * scale;
        int minX = (int)fmaxf(0, cx - maxDim);
        int maxX = (int)fminf(canvasW - 1, cx + maxDim);
        int minY = (int)fmaxf(0, cy - maxDim);
        int maxY = (int)fminf(canvasH - 1, cy + maxDim);

        for (int py = minY; py <= maxY; py++) {
            for (int px = minX; px <= maxX; px++) {
                float dx = (px - cx) / scale;
                float dy = (py - cy) / scale;

                // Inverse Rotation (um -angleDeg), um die Quellkoordinate im
                // unrotierten Zeigerbild zu finden.

                // Inverse rotation (by -angleDeg) to find the source coordinate
                // in the unrotated hand image.
                float sx = dx * cosA + dy * sinA + pivotX;
                float sy = -dx * sinA + dy * cosA + pivotY;

                int hx = (int)roundf(sx);
                int hy = (int)roundf(sy);
                if (hx < 0 || hx >= handW || hy < 0 || hy >= handH) continue;

                uint16_t p = hand[hy * handW + hx];
                if (p == TRANSPARENT_COLOR || p == 0xFFFF) continue; // transparent
                                                                     // transparent

                canvas[py * canvasW + px] = p;
            }
        }
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

        const int PREVIEW_SIZE = 100;
        uint16_t* canvas = (uint16_t*)preferPsramMalloc((size_t)PREVIEW_SIZE * PREVIEW_SIZE * 2);
        if (!canvas) return false;

        // 1) Zifferblatt laden und auf die Vorschaugroesse herunterskalieren
        // (eingebauter Standard direkt aus dem PROGMEM-Array, sonst per Datei -
        // gleiches Prinzip wie bei /preview_defaultface bzw. sendScaledBmpPreview()).

        // 1) Load the clock face and downscale it to preview size
        // (built-in default straight from the PROGMEM array, otherwise from file -
        // same approach as /preview_defaultface resp. sendScaledBmpPreview()).
        float faceScaleX = (float)CLOCK_WIDTH / PREVIEW_SIZE;
        float faceScaleY = (float)CLOCK_HEIGHT / PREVIEW_SIZE;
        bool isDefaultFace = (faceFile == "/face_default.bmp") || !LittleFS.exists(faceFile);

        if (isDefaultFace) {
            for (int y = 0; y < PREVIEW_SIZE; y++) {
                int sy = (int)(y * faceScaleY);
                for (int x = 0; x < PREVIEW_SIZE; x++) {
                    int sx = (int)(x * faceScaleX);
                    canvas[y * PREVIEW_SIZE + x] = clockFace[sy * CLOCK_WIDTH + sx];
                }
            }
        }
        else {
            uint16_t* faceBuf = (uint16_t*)preferPsramMalloc((size_t)CLOCK_WIDTH * CLOCK_HEIGHT * 2);
            if (!faceBuf) { free(canvas); return false; }
            if (!loadFaceBmpInto(faceFile, faceBuf, CLOCK_WIDTH, CLOCK_HEIGHT)) {
                for (int i = 0; i < CLOCK_WIDTH * CLOCK_HEIGHT; i++) faceBuf[i] = clockFace[i];
            }
            for (int y = 0; y < PREVIEW_SIZE; y++) {
                int sy = (int)(y * faceScaleY);
                for (int x = 0; x < PREVIEW_SIZE; x++) {
                    int sx = (int)(x * faceScaleX);
                    canvas[y * PREVIEW_SIZE + x] = faceBuf[sy * CLOCK_WIDTH + sx];
                }
            }
            free(faceBuf);
        }

        // 2) Zeiger laden (aus Datei, falls Set vorhanden, sonst eingebauter Standard)
        // 2) Load hands (from file if a set exists, otherwise built-in default)
        uint16_t* hourPix = nullptr;
        uint16_t* minutePix = nullptr;
        uint16_t* secondPix = nullptr;
        bool useCustomSet = (handSetName != "default" && handSetName != "");

        if (useCustomSet) {
            hourPix = (uint16_t*)preferPsramMalloc((size_t)HAND_WIDTH * HAND_HEIGHT * 2);
            if (hourPix && !loadFaceBmpInto("/hand_set" + handSetName + "_hour.bmp", hourPix, HAND_WIDTH, HAND_HEIGHT)) {
                free(hourPix); hourPix = nullptr;
            }
            minutePix = (uint16_t*)preferPsramMalloc((size_t)HAND_WIDTH * HAND_HEIGHT * 2);
            if (minutePix && !loadFaceBmpInto("/hand_set" + handSetName + "_minute.bmp", minutePix, HAND_WIDTH, HAND_HEIGHT)) {
                free(minutePix); minutePix = nullptr;
            }
            if (showSecond) {
                secondPix = (uint16_t*)preferPsramMalloc((size_t)HAND_WIDTH * HAND_HEIGHT * 2);
                if (secondPix && !loadFaceBmpInto("/hand_set" + handSetName + "_second.bmp", secondPix, HAND_WIDTH, HAND_HEIGHT)) {
                    free(secondPix); secondPix = nullptr;
                }
            }
        }

        // 3) Demo-Zeit 10:10:30 - klassischer Uhrenwerbung-Winkel
        // 3) Demo time 10:10:30 - the classic clock-advertisement angle
        const float hourAngle = (10 % 12) * 30.0f + (10 / 2.0f) + (30 / 120.0f);
        const float minuteAngle = 10 * 6.0f + (30 / 10.0f);
        const float secondAngle = 30 * 6.0f;

        float cx = PREVIEW_SIZE / 2.0f;
        float cy = PREVIEW_SIZE / 2.0f;
        float handScale = (float)PREVIEW_SIZE / CLOCK_WIDTH; // Zeiger im gleichen Massstab wie das Zifferblatt
                                                             // hands at the same scale as the clock face
        float pivotX = HAND_WIDTH / 2.0f;
        float pivotY = HAND_HEIGHT * 0.77f;

        blitRotatedHand(canvas, PREVIEW_SIZE, PREVIEW_SIZE,
            hourPix ? hourPix : handHour, HAND_WIDTH, HAND_HEIGHT,
            pivotX, pivotY, cx, cy, hourAngle, handScale);

        blitRotatedHand(canvas, PREVIEW_SIZE, PREVIEW_SIZE,
            minutePix ? minutePix : handMinute, HAND_WIDTH, HAND_HEIGHT,
            pivotX, pivotY, cx, cy, minuteAngle, handScale);

        if (showSecond) {
            blitRotatedHand(canvas, PREVIEW_SIZE, PREVIEW_SIZE,
                secondPix ? secondPix : handSecond, HAND_WIDTH, HAND_HEIGHT,
                pivotX, pivotY, cx, cy, secondAngle, handScale);
        }

        if (hourPix) free(hourPix);
        if (minutePix) free(minutePix);
        if (secondPix) free(secondPix);

        // 4) Mittelpunkt (Hub) in der angegebenen Farbe/Groesse zeichnen
        // 4) Draw the center hub in the given color/size
        float hubRadius = hubSize * handScale;
        if (hubRadius < 1.0f) hubRadius = 1.0f;
        for (int y = 0; y < PREVIEW_SIZE; y++) {
            for (int x = 0; x < PREVIEW_SIZE; x++) {
                float dx = (x + 0.5f) - cx;
                float dy = (y + 0.5f) - cy;
                if (dx * dx + dy * dy <= hubRadius * hubRadius) {
                    canvas[y * PREVIEW_SIZE + x] = hubColorRgb565;
                }
            }
        }

        // 5) Als Standard-BMP (mit BI_BITFIELDS-Header) verpacken
        // 5) Package as standard BMP (with BI_BITFIELDS header)
        const int rowSize = ((PREVIEW_SIZE * 2 + 3) / 4) * 4;
        const int dataSize = rowSize * PREVIEW_SIZE;
        const int fileSize = 66 + dataSize;

        uint8_t* bmpData = new uint8_t[fileSize];
        if (!bmpData) { free(canvas); return false; }
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

        for (int y = 0; y < PREVIEW_SIZE; y++) {
            memcpy(bmpData + 66 + y * rowSize, &canvas[y * PREVIEW_SIZE], PREVIEW_SIZE * 2);
        }

        free(canvas);
        *outBytes = bmpData;
        outSize = (size_t)fileSize;
        return true;
    }


    // --- Funktion: Schaltet die LED ein (wenn definiert) ---  
    // --- Function: turns the LED on (if defined) ---

    void setLedOff() {
#ifdef LED_BOARD
        pinMode(LED_BOARD, OUTPUT);
        digitalWrite(LED_BOARD, LOW);
#endif
    }


    // --- Funktion: Schaltet die LED aus (wenn definiert) ---
    // --- Function: turns the LED off (if defined) ---

    void setLedOn() {
#ifdef LED_BOARD
        pinMode(LED_BOARD, OUTPUT);
        digitalWrite(LED_BOARD, HIGH);
#endif
    }


    // --- Funktion: LED toggeln
    // --- Function: toggles the LED

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


    // --- Funktion: Touch prüfen (nicht-blockierend, mit Entprellung) ---
    // --- Function: check touch (non-blocking, with debouncing) ---

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

        // Neue Sprites erstellen
        // Create new sprites
        hourHandSprite.createSprite(hourHandWidth, HAND_HEIGHT);
        hourHandSprite.setSwapBytes(true);
        hourHandSprite.setColorDepth(16);
        hourHandSprite.setPivot(hourHandWidth / 2, HAND_HEIGHT * 0.77);

        minuteHandSprite.createSprite(minuteHandWidth, HAND_HEIGHT);
        minuteHandSprite.setSwapBytes(true);
        minuteHandSprite.setColorDepth(16);
        minuteHandSprite.setPivot(minuteHandWidth / 2, HAND_HEIGHT * 0.77);

        secondHandSprite.createSprite(secondHandWidth, HAND_HEIGHT);
        secondHandSprite.setSwapBytes(true);
        secondHandSprite.setColorDepth(16);
        secondHandSprite.setPivot(secondHandWidth / 2, HAND_HEIGHT * 0.77);

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
        if (secondWidth < 0) secondWidth = HAND_WIDTH;

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



