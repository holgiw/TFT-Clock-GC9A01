#pragma once
    // Display: Zifferblatt, Zeiger, Sprites, Helligkeit, Touch
    // Benoetigt globals.h, config.h, prefs_keys.h, declarations.h (vor dieser Datei in uhr3.ino eingebunden)

    // Display: clock face, hands, sprites, brightness, touch
    // Requires globals.h, config.h, prefs_keys.h, declarations.h (included before this file in uhr3.ino)

    // PSRAM bevorzugt fuer kurzlebige Puffer (BMP/PNG im Webinterface), um den
    // knappen internen Heap nicht zu fragmentieren. Eigener psramFound()-Check
    // statt gc9d01SwRotation, da dieses Flag nur die GC9D01-Rotation betrifft.

    // Prefers PSRAM for short-lived buffers (BMP/PNG in the web interface) to
    // avoid fragmenting the scarce internal heap. Uses its own psramFound()
    // check instead of gc9d01SwRotation, since that flag only concerns rotation.

    // Fuer "new (std::nothrow)" unten: garantiert nullptr statt
    // implementierungsabhaengigem Verhalten bei fehlgeschlagener Allokation.

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


#if defined CS_2


    // Waehlt Display 1 aus, deaktiviert Display 2. Steuert beide CS-Pins
    // manuell, da TFT_eSPIs automatische CS-Steuerung deaktiviert ist
    // (TFT_CS = -1, siehe config.h) und CS_1 sonst nicht mitgeschaltet wuerde.

    // Selects Display 1, disables Display 2. Drives both CS pins manually,
    // since TFT_eSPI's automatic CS control is disabled (TFT_CS = -1, see
    // config.h) and CS_1 would otherwise not be toggled along.

    void setCS1(bool state) {
        if (state == LOW) {
            digitalWrite(CS_1, LOW);
            digitalWrite(CS_2, HIGH);
        }

    }


    // Waehlt Display 2 bei Dual-Display-Aufbauten ueber seinen Chip-Select-Pin aus
    // (deaktiviert dabei Display 1, siehe Kommentar bei setCS1())

    // Selects Display 2 via its chip-select pin in dual-display setups
    // (disables Display 1, see comment on setCS1())

    void setCS2(bool state) {
        if (state == LOW) {
            digitalWrite(CS_2, LOW);
            digitalWrite(CS_1, HIGH);
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

    TFT_eSPI& beginStatusDraw(uint8_t displayNum) {
        displayNeedsBlank[displayNum - 1] = true; // Meldung auf dem Display - ein "n.a."-Display muss spaeter wieder schwarz werden
                                                  // message on the display - a "n.a." display has to go black again later
        TFT_eSprite& sprite = (displayNum == 1) ? statusSprite1 : statusSprite2;
        bool& created = (displayNum == 1) ? statusSprite1Created : statusSprite2Created;

        if (gc9d01SwRotation && !created) {
            // setColorDepth() vor createSprite() (bestimmt die Puffergroesse).
            // Schlaegt die Allokation fehl, bleibt 'created' false und es wird
            // unten unrotiert direkt auf den Chip gezeichnet statt ins Nichts.

            // setColorDepth() before createSprite() (determines buffer size).
            // If allocation fails, 'created' stays false and drawing falls
            // through to the direct-to-chip path below, unrotated but readable.
            sprite.setColorDepth(16);
            if (sprite.createSprite(CLOCK_WIDTH, CLOCK_HEIGHT) != nullptr) {
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

        TFT_eSprite& sprite = (displayNum == 1) ? statusSprite1 : statusSprite2;
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
            if (newRotation == TFT_ROTATION_NA) displayNeedsBlank[displayNum - 1] = true; // letztes Uhrbild muss weg
                                                                                          // last clock image has to go
        }
        rotation = newRotation;
        preferences.putUChar((displayNum == 1) ? PK_TFT_ROTATION1 : PK_TFT_ROTATION2, rotation);

        if (!gc9d01SwRotation) {
            // tft.setRotation() wirkt nur auf den aktuell selektierten Chip.
            // tft.setRotation() only affects the currently selected chip.
            if (displayNum == 1) setCS1(LOW); else setCS2(LOW);
            tft.setRotation(effectiveRotation(displayNum));
            setCSIdle(); // zurueck auf den Ausgangszustand, damit loop() im gewohnten Zustand weiterlaeuft
                         // back to the initial state, so loop() continues from its usual state
        }
    }


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


    // Laedt das gewaehlte Zifferblatt (BMP/RLEB, Fallback aufs Standardbild) in
    // clockFaceBuffer, wendet die Helligkeit an und zeichnet in backgroundSprite.

    // Loads the selected clock face (BMP/RLEB, falls back to the default) into
    // clockFaceBuffer, applies brightness and draws into backgroundSprite.

    // Haelt clockFaceBuffer (Rohbild) und clockFaceBrightBuffer (angepasst)
    // aktuell. Aus loadClockFace() ausgelagert, damit buildHandComposite() die
    // Vorbereitung mitnutzen kann. Rueckgabe: 2=beide Puffer ok, 1=nur Rohbild, 0=nichts.

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
        if (forceRecompute || currentBrightness != lastAppliedBrightness) {
            for (int i = 0; i < CLOCK_WIDTH * CLOCK_HEIGHT; i++) {
                clockFaceBrightBuffer[i] = setPixelBrightness(clockFaceBuffer[i]);
            }
            lastAppliedBrightness = currentBrightness;
        }

        // GC9D01 mit PSRAM ueberspringt die Hardware-Rotation (tft.setRotation()
        // wirkungslos); Zeiger werden per Software gedreht (rotatedAngle()), das
        // Zifferblatt hier ebenso - sonst blieb nur der Hintergrund ungedreht.

        // GC9D01 with PSRAM skips hardware rotation (tft.setRotation() has no
        // effect); hands are rotated in software (rotatedAngle()), the clock
        // face here too - otherwise only the background would stay unrotated.

        // 'rotation' ist die Zielausrichtung fuer DIESES Display. Bei Hardware-
        // Rotation uebernimmt das MADCTL-Register die Drehung, hier wird dann
        // nur faceOrientation=0 verwendet.

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


    // Kopiert das Zifferblatt in einen Speicherpuffer, bei Bedarf gedreht -
    // gleiche Abbildung wie loadClockFace(), aber ohne Sprite, da
    // buildHandComposite() direkt in diesem Puffer weiterarbeitet.

    // Copies the clock face into a memory buffer, rotated if needed - same
    // mapping as loadClockFace(), but without a sprite, since
    // buildHandComposite() works directly in this buffer afterwards.

    bool blitFaceIntoBuffer(uint16_t* dest, uint8_t rotation) {
        if (!dest) return false;
        if (prepareClockFaceCache() != 2) return false;

        const int N = CLOCK_WIDTH;
        int faceOrientation = faceOrientationFor(rotation);

        if (faceOrientation == 0) {
            memcpy(dest, clockFaceBrightBuffer, (size_t)N * CLOCK_HEIGHT * sizeof(uint16_t));
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
                dest[y * N + x] = clockFaceBrightBuffer[srcY * N + srcX];
            }
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


    // Liest eine Zeiger-Bitmap (RLE oder roh) direkt in einen Pixel-Puffer -
    // fuer die Web-Vorschau des aktiven Zeigersatzes (nicht ueber TFT_eSprite).
    // Spiegelt die Formaterkennung von loadHandBmp(), schreibt aber ins Array.

    // Reads a hand bitmap (RLE or raw) directly into a pixel buffer - for the
    // web preview of the active hand set (not via TFT_eSprite). Mirrors the
    // format detection of loadHandBmp(), but writes into an array.

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


    // Schreibt eine Zeiger-Bitmapzeile ins Sprite, mittig zugeschnitten, falls
    // das Sprite (siehe updateHandWidths()) schmaler als das Bitmap ist - haelt
    // den Zeiger so zentriert auf dem Drehpunkt statt rechtsseitig verschoben.

    // Writes a hand bitmap row into its sprite, cropped in the CENTRE if the
    // sprite (see updateHandWidths()) is narrower than the bitmap - keeps the
    // hand centred on the pivot instead of shifted off it on the right.

    void pushHandRowCentered(TFT_eSprite* sprite, int row, uint16_t* rowPixels, int srcWidth, const uint8_t* transparentColor) {
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
                            pushHandRowCentered(h.sprite, y, rowBuffer, HAND_WIDTH, nullptr);
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
                    // Zeilenweise statt in einem Rutsch, damit der mittige
                    // Zuschnitt fuer schmalere Sprites greift (siehe
                    // pushHandRowCentered()).

                    // Row by row instead of in one go, so the centre cropping for
                    // narrower sprites applies (see pushHandRowCentered()).
                    for (int y = 0; y < HAND_HEIGHT; y++) {
                        // setPixelBrightness() wie im Zweig oben: sonst bliebe ein per
                        // Fallback gezeichneter Zeiger bei Helligkeitswechseln heller.

                        // setPixelBrightness() as in the branch above: otherwise a hand
                        // drawn from the fallback would stay brighter on brightness changes.
                        for (int x = 0; x < HAND_WIDTH; x++) {
                            rowBuffer[x] = setPixelBrightness(h.fallback[y * HAND_WIDTH + x]);
                        }
                        pushHandRowCentered(h.sprite, y, rowBuffer, HAND_WIDTH, nullptr);
                    }
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
                pushHandRowCentered(&hourHandSprite, y, rowBuffer, HAND_WIDTH, nullptr);

                for (int x = 0; x < HAND_WIDTH; x++) {
                    rowBuffer[x] = setPixelBrightness(handMinute[y * HAND_WIDTH + x]);
                }
                pushHandRowCentered(&minuteHandSprite, y, rowBuffer, HAND_WIDTH, nullptr);

                for (int x = 0; x < HAND_WIDTH; x++) {
                    rowBuffer[x] = setPixelBrightness(handSecond[y * HAND_WIDTH + x]);
                }
                pushHandRowCentered(&secondHandSprite, y, rowBuffer, HAND_WIDTH, nullptr);
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
                // Bei Lesefehler false zurueckgeben statt nur die Schleife zu
                // verlassen - sonst merkt loadHandSprites() den Fehler nie und
                // faellt nicht auf den Standard-Zeiger zurueck.

                // Return false on a read error instead of just leaving the loop -
                // otherwise loadHandSprites() never notices and won't fall
                // back to the default hand.
                if (bmp.read((uint8_t*)rowBuffer, rowSize) != rowSize) {
                    bmp.close();
                    return false;
                }
            }

            uint16_t* pixelData = (uint16_t*)rowBuffer;
            for (int x = 0; x < width; x++) {

                if (pixelData[x] == 0xFFFF) {
                    pixelData[x] = TRANSPARENT_COLOR;
                }

                pixelData[x] = setPixelBrightness(pixelData[x]);

            }
            // Verengung von TRANSPARENT_COLOR auf uint8_t sieht falsch aus, MUSS
            // aber so bleiben: mit dem vollen Wert blieben Transparenzpixel beim
            // Laden schwarz statt transparent (schwarzer Rand um die Zeiger).

            // Narrowing TRANSPARENT_COLOR to uint8_t looks wrong but MUST stay:
            // with the full value transparent pixels would stay black instead
            // of transparent when loaded (black fringe around the hands).
            const uint8_t handTransparent = (uint8_t)TRANSPARENT_COLOR;
            pushHandRowCentered(sprite, row, (uint16_t*)rowBuffer, width, &handTransparent);
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

    // Renders ONE frame (face+hands+hub) for the currently chip-select-selected
    // display. lastHourAngleRef/lastMinuteAngleRef are references to
    // per-display variables, so each display keeps its own smoothing.

    // Zeichnet einen Zeiger kantengeglaettet (SUPERSAMPLE^2 Unterpunkte pro
    // Pixel statt Nearest-Neighbour wie pushRotated()). Festkomma 16.16 statt
    // Float, da der ESP32-S2 keine FPU hat; Abbildung/Drehpunkt wie bei pushRotated().

    // Draws a hand anti-aliased (SUPERSAMPLE^2 sub-points per pixel instead of
    // pushRotated()'s nearest-neighbour). Fixed point 16.16 instead of float,
    // since the ESP32-S2 has no FPU; same mapping/pivot as pushRotated().

    void blitHandAntiAliased(uint16_t* canvas, TFT_eSprite* handSprite, float angleDeg) {
        if (!canvas || !handSprite) return;

        const int handW = handSprite->width();
        const int handH = handSprite->height();
        if (handW <= 0 || handH <= 0) return;

        // Zeigerpixel einmal in einen flachen Puffer kopieren - readPixel() pro
        // Subsample (bis zu neun je Zielpixel) waere deutlich zu teuer.

        // Copy the hand pixels into a flat buffer once - readPixel() per
        // subsample (up to nine per destination pixel) would be far too costly.
        if (!handPixelScratch) {
            handPixelScratch = (uint16_t*)preferPsramMalloc((size_t)HAND_WIDTH * HAND_HEIGHT * sizeof(uint16_t));
            if (!handPixelScratch) {
                DEBUG_PRINTLN("[Display] Error: couldnt allocate handPixelScratch");
                return;
            }
        }
        if (handW > HAND_WIDTH || handH > HAND_HEIGHT) return; // passt nicht in den Puffer
                                                               // does not fit the buffer
        for (int y = 0; y < handH; y++) {
            for (int x = 0; x < handW; x++) {
                handPixelScratch[y * handW + x] = handSprite->readPixel(x, y);
            }
        }

        const float rad = angleDeg * (float)PI / 180.0f;
        const float cosA = cosf(rad);
        const float sinA = sinf(rad);

        const int32_t cosF = (int32_t)(cosA * 65536.0f);
        const int32_t sinF = (int32_t)(sinA * 65536.0f);

        // Drehpunkt im Zeigerbild und Ankerpunkt auf der Zielflaeche
        // Pivot inside the hand image and anchor point on the destination
        const int32_t pivotXF = (int32_t)handSprite->getPivotX() << 16;
        const int32_t pivotYF = (int32_t)handSprite->getPivotY() << 16;
        const int cx = backgroundSprite.getPivotX();
        const int cy = backgroundSprite.getPivotY();

        // Enges Huellrechteck aus den vier gedrehten Ecken statt eines
        // Umkreises - spart bei einem 13x86 grossen Zeiger je nach Winkel den
        // groessten Teil der Zielflaeche.

        // Tight bounding box from the four rotated corners instead of a
        // circumscribed circle - depending on the angle this saves most of the
        // destination area for a 13x86 hand.
        float minXf = 1e9f, maxXf = -1e9f, minYf = 1e9f, maxYf = -1e9f;
        const float px0 = (float)handSprite->getPivotX();
        const float py0 = (float)handSprite->getPivotY();
        for (int c = 0; c < 4; c++) {
            float u = ((c & 1) ? (float)handW : 0.0f) - px0;
            float v = ((c & 2) ? (float)handH : 0.0f) - py0;
            float dxF = u * cosA - v * sinA;
            float dyF = u * sinA + v * cosA;
            if (dxF < minXf) minXf = dxF;
            if (dxF > maxXf) maxXf = dxF;
            if (dyF < minYf) minYf = dyF;
            if (dyF > maxYf) maxYf = dyF;
        }

        int minX = (int)floorf(cx + minXf) - 1;
        int maxX = (int)ceilf(cx + maxXf) + 1;
        int minY = (int)floorf(cy + minYf) - 1;
        int maxY = (int)ceilf(cy + maxYf) + 1;
        if (minX < 0) minX = 0;
        if (minY < 0) minY = 0;
        if (maxX > CLOCK_WIDTH - 1) maxX = CLOCK_WIDTH - 1;
        if (maxY > CLOCK_HEIGHT - 1) maxY = CLOCK_HEIGHT - 1;
        if (minX > maxX || minY > maxY) return;

        const int SUPERSAMPLE = 3;
        const int SUBS = SUPERSAMPLE * SUPERSAMPLE;

        // Versatz der Unterpunkte innerhalb eines Zielpixels, bereits mit
        // Sinus/Kosinus verrechnet - so bleibt die innere Schleife reine
        // Ganzzahl-Addition.

        // Offsets of the sub-points inside a destination pixel, pre-multiplied
        // with sine/cosine - this keeps the inner loop pure integer addition.
        int32_t subCos[SUPERSAMPLE], subSin[SUPERSAMPLE];
        for (int i = 0; i < SUPERSAMPLE; i++) {
            float off = ((float)i + 0.5f) / (float)SUPERSAMPLE - 0.5f;
            subCos[i] = (int32_t)(off * cosA * 65536.0f);
            subSin[i] = (int32_t)(off * sinA * 65536.0f);
        }

        for (int py = minY; py <= maxY; py++) {
            const int32_t rowX = (py - cy) * sinF + pivotXF;
            const int32_t rowY = (py - cy) * cosF + pivotYF;

            for (int px = minX; px <= maxX; px++) {
                const int32_t baseX = rowX + (px - cx) * cosF;
                const int32_t baseY = rowY - (px - cx) * sinF;

                uint32_t hits = 0, sumR = 0, sumG = 0, sumB = 0;

                for (int sy = 0; sy < SUPERSAMPLE; sy++) {
                    const int32_t colBaseX = baseX + subSin[sy];
                    const int32_t colBaseY = baseY + subCos[sy];

                    for (int sx = 0; sx < SUPERSAMPLE; sx++) {
                        const int hx = (colBaseX + subCos[sx]) >> 16;
                        if (hx < 0 || hx >= handW) continue;
                        const int hy = (colBaseY - subSin[sx]) >> 16;
                        if (hy < 0 || hy >= handH) continue;

                        const uint16_t pix = handPixelScratch[hy * handW + hx];
                        if (pix == TRANSPARENT_COLOR) continue;

                        hits++;
                        sumR += (pix >> 11) & 0x1F;
                        sumG += (pix >> 5) & 0x3F;
                        sumB += pix & 0x1F;
                    }
                }

                if (hits == 0) continue;

                uint16_t* target = &canvas[py * CLOCK_WIDTH + px];

                if (hits == SUBS) {
                    // Voll gedeckt: Mittelwert der Treffer, kein Blenden noetig.
                    // Fully covered: average of the hits, no blending needed.
                    *target = (uint16_t)(((sumR / SUBS) << 11) | ((sumG / SUBS) << 5) | (sumB / SUBS));
                    continue;
                }

                // Teilweise gedeckt: Zeigerfarbe anteilig gegen den Hintergrund
                // blenden. sumX ist bereits die Summe ueber die Treffer, der
                // Hintergrund steuert die restlichen (SUBS - hits) Anteile bei.

                // Partially covered: blend the hand colour proportionally
                // against the background. sumX is already the sum over the hits,
                // the background contributes the remaining (SUBS - hits) shares.
                const uint16_t bg = *target;
                const uint32_t miss = SUBS - hits;

                const uint32_t r = (((bg >> 11) & 0x1F) * miss + sumR) / SUBS;
                const uint32_t g = (((bg >> 5) & 0x3F) * miss + sumG) / SUBS;
                const uint32_t b = ((bg & 0x1F) * miss + sumB) / SUBS;

                *target = (uint16_t)((r << 11) | (g << 5) | b);
            }
        }
    }


    // Baut das Zwischenbild fuer ein Display neu auf: gedrehtes Zifferblatt,
    // darauf Stunden- und Minutenzeiger kantengeglaettet.

    // Rebuilds the composite image for one display: rotated clock face with the
    // anti-aliased hour and minute hands on top.

    bool buildHandComposite(HandComposite& comp, uint8_t rotation, float hourAngle, float minuteAngle) {
        if (comp.allocationFailed) return false;

        // Erst pruefen, ob das Zifferblatt lieferbar ist, DANN den Puffer
        // belegen - sonst wuerde bei Speichermangel genau der Speicher belegt,
        // den der fehlende Helligkeits-Cache eigentlich braeuchte.

        // First check whether the clock face can be delivered, THEN allocate
        // the buffer - otherwise, under memory pressure, this would grab
        // exactly the memory the missing brightness cache actually needs.
        if (prepareClockFaceCache() != 2) return false;

        if (!comp.buffer) {
            comp.buffer = (uint16_t*)preferPsramMalloc((size_t)CLOCK_WIDTH * CLOCK_HEIGHT * sizeof(uint16_t));
            if (!comp.buffer) {
                // Einmal melden und danach dauerhaft den bisherigen Weg nutzen,
                // statt bei jedem Tick erneut zu versuchen.

                // Report once and then permanently use the previous path instead
                // of retrying on every tick.
                DEBUG_PRINTLN("[Display] couldnt allocate hand composite buffer - falling back to per-tick rendering");
                comp.allocationFailed = true;
                return false;
            }
        }

        if (!blitFaceIntoBuffer(comp.buffer, rotation)) {
            free(comp.buffer);
            comp.buffer = nullptr;
            return false;
        }

        blitHandAntiAliased(comp.buffer, &hourHandSprite, hourAngle);
        blitHandAntiAliased(comp.buffer, &minuteHandSprite, minuteAngle);

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
#ifndef TFT_Backlight
            // Nur ohne Backlight faerbt die Helligkeit Pixel ein (sonst No-Op) -
            // mit Backlight wuerde jeder Rampenschritt sonst einen wirkungslosen
            // Neuaufbau ausloesen.

            // Only without a backlight does brightness tint pixels (otherwise
            // a no-op) - with a backlight every ramp step would otherwise
            // trigger a pointless rebuild.
            || comp.brightness != currentBrightness
#endif
            || fabsf(shortestAngleDiff(comp.hourAngle, hourAngle)) >= COMPOSITE_ANGLE_EPS
            || fabsf(shortestAngleDiff(comp.minuteAngle, minuteAngle)) >= COMPOSITE_ANGLE_EPS;

        if (needsRebuild) {
            if (!buildHandComposite(comp, rotation, hourAngle, minuteAngle)) return false;
        }
        else if (!comp.buffer) {
            return false;
        }

        backgroundSprite.pushImage(0, 0, CLOCK_WIDTH, CLOCK_HEIGHT, comp.buffer);
        return true;
    }


    void renderClockFrame(uint8_t displayNum, uint8_t rotation, float& lastHourAngleRef, float& lastMinuteAngleRef, float& lastSecondAngleRef, bool& firstRunRef) {

        int orientation = rotation;

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

            hourHandSprite.pushRotated(&backgroundSprite, lastHourAngleRef, TRANSPARENT_COLOR);
            minuteHandSprite.pushRotated(&backgroundSprite, lastMinuteAngleRef, TRANSPARENT_COLOR);

            if (showSecondHand && !hideSecondHand) {
                secondHandSprite.pushRotated(&backgroundSprite, rotatedAngle(secAngle, orientation), TRANSPARENT_COLOR);
            }
            backgroundSprite.pushSprite(0, 0);
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

            if (!stationWaiting && currentMillis - stationLastMillis >= stationStepMs) {
                stationTick++;
                stationLastMillis += stationStepMs;

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

            float subTick = (float)(currentMillis - stationLastMillis) / stationStepMs;

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

            // Deliberately NOT uniform movement in the smooth style:
            // easeInOutSine() accelerates and brakes each step, like older
            // station clocks - do not replace with linear interpolation.

            // smoothSecond entkoppelt "wartet auf 12" (waitAtTwelve, s.o.) von
            // der Darstellung: schwingend interpoliert per easeInOutSine()
            // innerhalb des Schritts, tickend haelt exakt auf der Ganzzahl-
            // Position bis zum naechsten Schritt (kein Zwischenwert).

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

            // Ease away a visible jump: backward (time correction) or
            // unusually far forward (a delayed frame, e.g. a blocking web
            // request). Normal ticks still apply immediately.

            // Diagnose-Logging (DEBUG_PRINTLN, wirkungslos wenn loggingEnabled
            // aus): loggt je Episode Start (Betrag, Richtung) und Ende
            // (Dauer), um eine Ursache im Log zuzuordnen statt zu raten.

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

            smoothMinute = preferences.getBool(PK_SMOOTH_MINUTE, false);

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


        // Zifferblatt + Stunden-/Minutenzeiger kommen aus dem zwischen-
        // gespeicherten Bild (drawCompositeInto()), neu gebaut nur bei
        // Bewegung - pro Tick bleibt nur das Kopieren. Fallback bei Speichermangel: alter Zeichenweg.

        // Clock face + hour/minute hands come from the cached composite image
        // (drawCompositeInto()), rebuilt only on movement - per tick only the
        // copy remains. Falls back to the old drawing path on low memory.
        bool compositeOk = drawCompositeInto(displayNum, rotation, hourAngle, minAngle);
        if (!compositeOk) {
            loadClockFace(rotation);
            hourHandSprite.pushRotated(&backgroundSprite, hourAngle, TRANSPARENT_COLOR);
            minuteHandSprite.pushRotated(&backgroundSprite, minAngle, TRANSPARENT_COLOR);
        }

        if (showSecondHand && !hideSecondHand) {

            // Kantengeglaettet NUR im tickenden Stil (smoothSecond == false),
            // unabhaengig davon, ob "wartet auf 12" (waitAtTwelve/stationMode)
            // an oder aus ist: bei tickend bewegt sich der Sekundenzeiger nur
            // einmal pro Sekunde bzw. Schritt, die teurere Supersampling-
            // Blendtechnik (blitHandAntiAliased(), siehe dort) faellt
            // performance-maessig also nicht ins Gewicht. Im schwingenden Stil
            // bewegt er sich dagegen viel oefter pro Sekunde - dort bleibt es
            // beim guenstigeren, nicht kantengeglaetteten TFT_eSPI-eigenen
            // pushRotated(). Setzt ausserdem voraus, dass der Compositing-
            // Cache verfuegbar ist (compositeOk) - im Speichermangel-Fallback
            // oben gibt es kein handComposite[]-Puffer zum Hineinblenden.

            // Anti-aliased ONLY in ticking style (smoothSecond == false),
            // regardless of whether "waits at 12" (waitAtTwelve/stationMode)
            // is on or off: with ticking the second hand only moves once per
            // second/step, so the costlier supersampled blend technique
            // (blitHandAntiAliased(), see there) doesn't matter performance-
            // wise. In the smooth style it moves much more often per second -
            // there it stays on the cheaper, non-anti-aliased TFT_eSPI
            // pushRotated(). Also requires the compositing cache to be
            // available (compositeOk) - the low-memory fallback above has no
            // handComposite[] buffer to blend into.
            bool didAntiAliasedSecondHand = false;

            if (!smoothSecond && compositeOk) {
                if (!secondHandCompositeScratch) {
                    secondHandCompositeScratch = (uint16_t*)preferPsramMalloc((size_t)CLOCK_WIDTH * CLOCK_HEIGHT * sizeof(uint16_t));
                }
                if (secondHandCompositeScratch) {
                    HandComposite& comp = handComposite[(displayNum == 1) ? 0 : 1];
                    memcpy(secondHandCompositeScratch, comp.buffer, (size_t)CLOCK_WIDTH * CLOCK_HEIGHT * sizeof(uint16_t));
                    blitHandAntiAliased(secondHandCompositeScratch, &secondHandSprite, secAngle);
                    backgroundSprite.pushImage(0, 0, CLOCK_WIDTH, CLOCK_HEIGHT, secondHandCompositeScratch);
                    didAntiAliasedSecondHand = true;
                }
            }

            if (!didAntiAliasedSecondHand) {
                secondHandSprite.pushRotated(&backgroundSprite, secAngle, TRANSPARENT_COLOR);
            }
        }


        // Nabe (hub) - ab ROCRAIL_HIDE_DETAILS_DIVIDER ebenfalls ausgeblendet
        // (siehe hideDetailsForRocrail oben)

        // hub - also hidden from ROCRAIL_HIDE_DETAILS_DIVIDER onwards (see
        // hideDetailsForRocrail above)
        if (hubSize > 0 && !hideDetailsForRocrail) {
           backgroundSprite.fillCircle(CLOCK_WIDTH / 2, CLOCK_HEIGHT / 2, hubSize, setPixelBrightness(hubColor));
        }

        backgroundSprite.pushSprite(0, 0);
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
                // NTP-Sync-Periode, siehe WAIT_6h in uhr3.ino) erfolgreich
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
                // (< the NTP sync period, see WAIT_6h in uhr3.ino) (see
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

        if (display1Connected) {
            setCS1(LOW);
            renderClockFrame(1, tftRotation1, lastHourAngle, lastMinuteAngle, lastSecondAngle, firstRun);
        }
        else {
            firstRun = true;
        }

        if (!display2Connected) {
            firstRun2 = true;
        }
        else if (!display1Connected || (gc9d01SwRotation && tftRotation2 != tftRotation1)) {
            setCS2(LOW);
            renderClockFrame(2, tftRotation2, lastHourAngle2, lastMinuteAngle2, lastSecondAngle2, firstRun2);
        }
        else {
            setCS2(LOW);
            backgroundSprite.pushSprite(0, 0);

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

        // Its own comparison variable for the hands: lastAppliedBrightness is
        // maintained by loadClockFace() itself, the clock face needs no call here.

        // Nicht bei JEDEM Rampenschritt neu einfaerben (kostet Preferences-/
        // LittleFS-Zugriffe), sondern nur bei spuerbarer Differenz plus einmal
        // am Rampenende fuer den exakten Endwert.

        // Don't re-tint on EVERY ramp step (costly preferences/LittleFS access),
        // only on a noticeable difference plus once at the end of the ramp for
        // the exact final value.
        const uint8_t HAND_RETINT_STEP = 8;

        int handBrightnessDelta = (int)currentBrightness - (int)lastHandBrightness;
        if (handBrightnessDelta < 0) handBrightnessDelta = -handBrightnessDelta;

        if (handBrightnessDelta >= HAND_RETINT_STEP ||
            (handBrightnessDelta > 0 && currentBrightness == targetBrightness)) {
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

        if (rocrailBrightnessActive) {
            targetBrightness = rocrailBrightness;
#ifdef TFT_Backlight
            // sanfte Anpassung wie beim Zeitfenster/ADC unten, statt eines
            // harten Sprungs bei jeder Server-Aenderung.

            // smooth adjustment like the time window/ADC below, instead of a
            // hard jump on every server-side change.
            if (currentBrightness < targetBrightness) currentBrightness++;
            else if (currentBrightness > targetBrightness) currentBrightness--;
#else
            currentBrightness = targetBrightness;
#endif
        }
        else if (withinDayWindow) {
            // Zeitfenster aktiv und wir sind innerhalb davon: volle Helligkeit erzwingen
            // Time window active and we're inside it: force full brightness
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

                // currentAdcAvg/currentLightPercent wurden oben bereits fuer
                // diesen Durchlauf aktualisiert - hier nur noch die
                // Helligkeits-ENTSCHEIDUNG anhand der frischen Werte.

                // currentAdcAvg/currentLightPercent were already updated
                // above for this pass - only the brightness DECISION based
                // on those fresh values happens here.

                // Schwellwert-Ueberschreitung erst nach BRIGHTNESS_DEBOUNCE_MS
                // Bestand uebernehmen: ein kurzer Ausreisser (z.B. WLAN-
                // Sendeburst waehrend NTP-Sync) soll targetBrightness nicht
                // sofort umschalten, sonst faerbt setPixelBrightness() das
                // komplette Zifferblatt fuer einen Frame sichtbar um.

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
#ifdef TFT_Backlight
                else if (pendingBrightnessState == 0) {
                    float norm = constrain((float)currentAdcAvg / 4095.0f, 0.0f, 1.0f);
                    float gamma = gammaBrightness;
                    float gammaNorm = powf(norm, gamma);
                    targetBrightness = minBrightness + (uint8_t)((maxBrightness - minBrightness) * gammaNorm + 0.5f);
                }
#endif

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
    // Rotations-Workaround (Hardware-Rotation dort wirkungslos, siehe uhr3.ino).
    // Sonst (gc9d01SwRotation=false) wird angle unveraendert zurueckgegeben.

    // Rotates the hand angles - only relevant for GC9D01 with the software
    // rotation workaround active (hardware rotation has no effect there, see
    // uhr3.ino). Otherwise (gc9d01SwRotation=false) angle is returned unchanged.

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

        uint8_t* bmpData = new (std::nothrow) uint8_t[fileSize];
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

        // loadClockFace() calls this on EVERY tick. Without this shortcut all
        // three hand sprites would be reallocated per frame (heap fragmentation,
        // NVS+3x LittleFS) - now only on an actual width change.

        // Vergleich gegen die TATSAECHLICHE Sprite-Breite, nicht gegen die
        // globalen hourHandWidth/...: die werden vom Aufrufer per Referenz VORHER
        // gesetzt und waeren daher immer gleich - ein Breitenwechsel bliebe unerkannt.

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

        allCreated &= (hourHandSprite.createSprite(hourHandWidth, HAND_HEIGHT) != nullptr);
        hourHandSprite.setSwapBytes(true);
        hourHandSprite.setColorDepth(16);
        hourHandSprite.setPivot(hourHandWidth / 2, HAND_HEIGHT * 0.77);

        allCreated &= (minuteHandSprite.createSprite(minuteHandWidth, HAND_HEIGHT) != nullptr);
        minuteHandSprite.setSwapBytes(true);
        minuteHandSprite.setColorDepth(16);
        minuteHandSprite.setPivot(minuteHandWidth / 2, HAND_HEIGHT * 0.77);

        allCreated &= (secondHandSprite.createSprite(secondHandWidth, HAND_HEIGHT) != nullptr);
        secondHandSprite.setSwapBytes(true);
        secondHandSprite.setColorDepth(16);
        secondHandSprite.setPivot(secondHandWidth / 2, HAND_HEIGHT * 0.77);

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



