#pragma once
// ####################################################################
// ### Display: Zifferblatt, Zeiger, Sprites, Helligkeit, Touch
// ####################################################################
// Benoetigt globals.h, config.h, prefs_keys.h und declarations.h (werden
// zentral in uhr3.ino VOR dieser Datei eingebunden).

#if defined CS_2
// Waehlt bei Dual-Display-Aufbauten das erste TFT ueber sein Chip-Select-Pin aus
// (deaktiviert dabei das zweite Display)
void setCS1(bool state) {
    if (state == LOW) {
        digitalWrite(CS_2, HIGH);
    }
    
}


// Waehlt das zweite TFT bei Dual-Display-Aufbauten ueber seinen Chip-Select-Pin aus
void setCS2(bool state) {
    if (state == LOW) {
        digitalWrite(CS_2, state);
    }
    
}
#endif




// Passt die Helligkeit eines Pixels basierend auf der aktuellen Helligkeitseinstellung an.
uint16_t setPixelBrightness(uint16_t pixel) {

#ifdef TFT_Backlight
    return pixel;
#else

    // Wenn die Helligkeit maximal ist oder der Pixel transparent/schwarz ist, direkt zurückgeben
    if (pixel == TRANSPARENT_COLOR || pixel == 0x0000 || currentBrightness == 255) {
        return pixel;
    }

    // Multiplikator einmal berechnen (statt 3x Division)
    uint32_t brightnessFactor = (uint32_t)currentBrightness;

    // Farben extrahieren
    uint32_t r = (pixel & 0xF800);
    uint32_t g = (pixel & 0x07E0);
    uint32_t b = (pixel & 0x001F);

    // Multiplikation mit Brightness (optimiert, kein Shift nötig)
    r = ((r * brightnessFactor) >> 8) & 0xF800;
    g = ((g * brightnessFactor) >> 8) & 0x07E0;
    b = ((b * brightnessFactor) >> 8) & 0x001F;

    // Farbwerte zusammenfügen
    return r | g | b;
#endif
}


// Lädt das Zifferblatt, indem entweder ein benutzerdefinierter Hintergrund oder ein Standardhintergrund verwendet wird.
// ####################################################################
// ### RLE-Kompression fuer Zifferblatt-BMPs (face_*.bmp) #############
// ####################################################################
// Eigenes, einfaches PackBits-artiges RLE-Verfahren fuer 16-Bit-RGB565-
// Pixel. Zifferblaetter haben oft grosse einfarbige Flaechen (Hintergrund),
// die sich damit gut komprimieren lassen, waehrend der Overhead im
// unguenstigsten Fall (kein einziger Pixel wiederholt sich) bei nur ca.
// 0.4% zusaetzlicher Groesse liegt (1 Steuerbyte je 128 Pixel).
//
// Eine so gespeicherte Datei beginnt mit dem 4-Byte-Magic "RLEB" statt
// "BM" und ist damit KEIN gueltiges Standard-BMP mehr - Downloads und die
// Dateimanager-Vorschau dekodieren daher bei Bedarf zurueck in ein
// echtes BMP (siehe /file und /download in webserver_routes.h).
//
// Format:
//   Byte  0- 3: Magic "RLEB"
//   Byte  4- 7: Breite (int32)
//   Byte  8-11: Hoehe (int32)
//   Byte 12-15: Groesse der komprimierten Daten in Byte (uint32)
//   Byte 16-19: Groesse der unkomprimierten Daten in Byte (uint32, = Breite*Hoehe*2)
//   ab Byte 20: RLE-Datenstrom
//
// Steuerbyte C je Paket:
//   C = 0..127:   Literal-Lauf von (C+1) Pixeln folgt, roh (2 Byte je Pixel)
//   C = 129..255: Wiederholungs-Lauf von (257-C) identischen Pixeln folgt
//                 (nur 1 Pixel = 2 Byte im Stream)
//   C = 128:      nicht verwendet
bool isRleFace(const uint8_t* header4) {
    return header4[0] == 'R' && header4[1] == 'L' && header4[2] == 'E' && header4[3] == 'B';
}


// Obergrenze fuer die kodierte Groesse (fuer die Allokation des Zielpuffers).
size_t rleMaxEncodedSize(size_t pixelCount) {
    return pixelCount * 2 + (pixelCount / 128 + 2);
}


// Kodiert ein Array von RGB565-Pixeln PackBits-artig (Lauflaengenkodierung);
// gibt die Anzahl tatsaechlich geschriebener Bytes in 'out' zurueck
size_t rleEncode565(const uint16_t* pixels, size_t count, uint8_t* out) {
    size_t i = 0, o = 0;
    while (i < count) {
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
void rleDecode565(const uint8_t* in, size_t inSize, uint16_t* out, size_t outCount) {
    size_t i = 0, o = 0;
    while (i < inSize && o < outCount) {
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


// Wie rleDecode565(), schreibt aber direkt in einen zeilenweise auf
// 4-Byte-Grenzen gepolsterten BMP-Pixelbereich (rowStride Byte je Zeile,
// width Pixel je Zeile nutzbar). Vermeidet einen zusaetzlichen, komplett
// entpackten Zwischenpuffer - spart bei einem 240x240-Zifferblatt bis zu
// ~115 KB Spitzen-Heap-Bedarf gegenueber "erst flach dekodieren, dann in
// den gepolsterten Puffer kopieren".
void rleDecode565ToBmpRows(const uint8_t* in, size_t inSize, uint8_t* pixelArea, int width, int height, int rowStride) {
    size_t i = 0;
    int col = 0, row = 0;
    size_t written = 0;
    const size_t total = (size_t)width * height;

    while (i < inSize && written < total && row < height) {
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


// Liest eine face_*.bmp-Datei (egal ob Standard-BMP oder RLEB-komprimiert)
// direkt in einen bereits allozierten Ziel-Puffer 'dest' (expectedW x
// expectedH Pixel, RGB565, Top-Down). Gibt false zurueck bei Lesefehler
// oder wenn die Dimensionen nicht passen (dest bleibt dann unveraendert).
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

        uint8_t* compBuf = (uint8_t*)malloc(compressedSize);
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
void loadClockFace() {
    // Prüfen, ob Buffer schon existiert
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
        bool loaded = false;
        if (LittleFS.exists(selectedBackground)) {
            loaded = loadFaceBmpInto(selectedBackground, clockFaceBuffer, CLOCK_WIDTH, CLOCK_HEIGHT);
        }
        if (!loaded) {
            // Fallback: Standard-Zifferblatt aus Array kopieren (auch bei
            // Lesefehler oder falschen Dimensionen - vorher blieb der
            // Puffer in diesem Fall unveraendert/undefiniert)
            memcpy(clockFaceBuffer, clockFace, CLOCK_WIDTH * CLOCK_HEIGHT * sizeof(uint16_t));
        }
    }

    // Breiten aus Dateinamen extrahieren
    parseBackgroundFilename(selectedBackground, hourHandWidth, minuteHandWidth, secondHandWidth);
    updateHandWidths(hourHandWidth, minuteHandWidth, secondHandWidth);

    
    // Buffer ins Sprite kopieren (mit Helligkeit)
    for (int y = 0; y < CLOCK_HEIGHT; y++) {
        for (int x = 0; x < CLOCK_WIDTH; x++) {
            rowBuffer[x] = setPixelBrightness(clockFaceBuffer[y * CLOCK_WIDTH + x]);
        }
        backgroundSprite.pushImage(0, y, CLOCK_WIDTH, 1, rowBuffer);
    }
}


// Buffer freigeben, wenn ein neues Zifferblatt gewählt wird
void freeClockFaceBuffer() {
    if (clockFaceBuffer) {
        free(clockFaceBuffer);
        clockFaceBuffer = nullptr;
        // DEBUG_PRINTLN("[clockFaceBuffer] free");
    }
}


// Lädt die Grafiken für die Zeiger eines Uhren-Widgets, entweder aus einer benutzerdefinierten Konfiguration oder aus Standardwerten.
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
bool loadHandBmp(TFT_eSprite* sprite, const char* filename, int width, int height) {
    File bmp = LittleFS.open(filename, "r");
    if (!bmp) return false;

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

    bmp.seek(offset);
    int rowSize = ((width * 2 + 3) / 4) * 4;
    for (int y = 0; y < bmpHeight; y++) {
        int row = flip ? bmpHeight - 1 - y : y;
        //uint8_t rowBuffer[rowSize];
        if (bmp.read((uint8_t*)rowBuffer, rowSize) != rowSize) break;

        uint16_t* pixelData = (uint16_t*)rowBuffer;
        for (int x = 0; x < width; x++) {

            if (pixelData[x] == 0xFFFF) {
                pixelData[x] = TRANSPARENT_COLOR;
            }

            pixelData[x] = setPixelBrightness(pixelData[x]);

        }
        sprite->pushImage(0, row, width, 1, (uint16_t*)rowBuffer, (uint8_t)TRANSPARENT_COLOR);
    }

    bmp.close();
    return true;
}


// Hilfsfunktion: Winkel an die aktuelle Display-Rotation anpassen
float shortestAngleDiff(float from, float to) {
    float diff = fmodf(to - from + 360.0f, 360.0f); // Modulo 360, um Werte im Bereich [0, 360) zu halten
    if (diff > 180.0f) diff -= 360.0f;             // Kürzeste Richtung wählen
    return diff;
}

static float lastHourAngle = 0.0f;
static float lastMinuteAngle = 0.0f;

// updateClock Funktion

void updateClock() {
   // struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 1000)) {
        // Keine gültige Uhrzeit verfügbar
        loadTimeFromRTC();
    }


    static unsigned long lastRTCUpdate = 0; // Zeitpunkt des letzten RTC-Updates
    if (rtcOk == RTC_AVAILABLE) {            
        // Überprüfen, ob seit dem letzten Aufruf Zeit vergangen ist
        if (millis() - lastRTCUpdate >= WAIT_1h) {            
            loadTimeFromRTC();
            lastRTCUpdate = millis();
        }
    }
    

    int orientation = preferences.getUChar(PK_TFT_ROTATION, 0);

    float secAngle = timeinfo.tm_sec * 6.0f;
    float minAngle = timeinfo.tm_min * 6.0f;
    float hourAngle = (timeinfo.tm_hour % 12) * 30.0f + (timeinfo.tm_min / 2.0f) + (timeinfo.tm_sec / 120.0f);

    static uint8_t stationTick = 0;
    static uint32_t stationLastMillis = 0;
    static bool stationWaiting = false;

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
    if (stationMode) {
        
        if (!stationWaiting && currentMillis - stationLastMillis >= fastSecond) {
            stationTick++;
            stationLastMillis += fastSecond;

            if (stationTick >= 60) {
                stationTick = 60;
                stationWaiting = true;
            }
        }
        else if (stationWaiting) {
            if (timeinfo.tm_sec == 0) {
                stationTick = 0;
                stationWaiting = false;
                stationLastMillis = currentMillis;

                // Sekundenzeiger korrekt synchronisieren
                secAngle = rotatedAngle(0, orientation);
            }
        }

        float subTick = (currentMillis - stationLastMillis) / fastSecond;
        if (subTick > 1.0f || stationWaiting) subTick = 0.0f;

        float smoothSec = (stationTick >= 60) ? 60.0f : stationTick + easeInOutSine(subTick);
        secAngle = rotatedAngle(smoothSec * 6.0f, orientation);

        minAngle = rotatedAngle(timeinfo.tm_min * 6.0f, orientation);   
    }

    // Normaler Modus: Sekundenzeiger läuft normal, Minutenzeiger kann optional sanft laufen
    if (!stationMode) {
        secAngle = rotatedAngle(secAngle, orientation);
          
        smoothMinute = preferences.getBool(PK_SMOOTH_MINUTE, false);

        if (smoothMinute) {
            // Millisekunden einbeziehen
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
            unsigned long currentMillis = millis();
            int milliseconds = currentMillis % 1000;
            float smoothMinuteValue = timeinfo.tm_min + (timeinfo.tm_sec / 60.0f) + (milliseconds / 60000.0f);

            float rawMinAngle = smoothMinuteValue * 6.0f;
            minAngle = rotatedAngle(rawMinAngle, orientation);
            lastMinuteAngle = minAngle; // Direkt setzen, da wir den exakten Winkel berechnen   

        }
        else {
            // Normale Minutenanzeige mit sanfter Korrektur bei Wechsel
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
    if (hubSize > 0) {
       backgroundSprite.fillCircle(CLOCK_WIDTH / 2, CLOCK_HEIGHT / 2, hubSize, setPixelBrightness(hubColor));
    }

    backgroundSprite.pushSprite(0, 0);
}


// Aktualisiert die Helligkeit des Displays basierend auf der aktuellen Einstellung, 
// dem ADC-Wert (falls aktiviert) und dem Tageszeitfenster für volle Helligkeit.
void updateBrightness() {

    // Wenn Helligkeit geändert → neu zeichnen
    if (currentBrightness != lastAppliedBrightness) {
        loadClockFace();
        loadHandSprites();
        lastAppliedBrightness = currentBrightness;
    }

    // Prüfen, ob wir aktuell im konfigurierten Voll-Helligkeits-Zeitfenster sind
    bool withinDayWindow = false;
    
    // struct tm timeinfo;
    if (getLocalTime(&timeinfo, 500)) {
        int h = timeinfo.tm_hour;
        if (brightStartHour <= brightEndHour) {
            // normaler Bereich z.B. 8..20
            withinDayWindow = (h >= brightStartHour && h < brightEndHour);
        }
        else {
            // über Mitternacht z.B. 20..6
            withinDayWindow = (h >= brightStartHour || h < brightEndHour);
        }
    }
    

    // Wenn Zeitfenster aktiv und wir innerhalb davon sind: volle Helligkeit erzwingen
    if (withinDayWindow) {
        targetBrightness = maxBrightness;
#ifdef TFT_Backlight
        // sanfte Erhöhung, falls gewünscht (ähnlich wie ADC-Rampen)
        if (currentBrightness < targetBrightness) currentBrightness++;
        else if (currentBrightness > targetBrightness) currentBrightness--;
#else
        currentBrightness = targetBrightness;
#endif
    }
    else {
#ifdef ADC_PIN
        // Normale Auto-Brightness oder statische Helligkeit
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
            currentBrightness = minBrightness;
            targetBrightness = currentBrightness;
        }
#endif
    }

#ifdef TFT_Backlight
    ledcWrite(TFT_Backlight, currentBrightness);  // 0–255
#endif

}


// Passt den ADC-Wert an, wenn die Invertierung aktiviert ist
uint16_t getAdjustedAdcValue(int rawValue) {
    if (adcInverted) {
        return 4096 - rawValue; // Invertiere den Wert
    }
    return rawValue; // Standardwert
}


/// Easing-Funktion für sanfte Animationen
float easeInOutSine(float t) {
    // Intensität steuert die Kurve: 1.0 = Standard, >1.0 = steiler, <1.0 = flacher
    float intensity = 0.5f;
    return -(cos(PI * pow(t, intensity)) - 1.0f) / 2.0f;
}


// Kodiert ein 16-Bit RGB565 Bild in das BMP-Format und gibt es als Base64-kodierten String zurück.
String encodeBmpToBase64(const uint16_t* data, int width, int height) {
    const int headerSize = 54;
    const int rowSize = ((width * 2 + 3) / 4) * 4;
    const int dataSize = rowSize * height;
    const int fileSize = headerSize + dataSize;

    uint8_t* bmpData = new uint8_t[fileSize];
    if (!bmpData) return "";

    memset(bmpData, 0, fileSize);

    // BMP Header
    bmpData[0] = 'B'; bmpData[1] = 'M';
    *(uint32_t*)&bmpData[2] = fileSize;
    *(uint32_t*)&bmpData[10] = headerSize;
    *(uint32_t*)&bmpData[14] = 40;
    *(int32_t*)&bmpData[18] = width;
    *(int32_t*)&bmpData[22] = -height; // Top-down BMP
    *(uint16_t*)&bmpData[26] = 1;
    *(uint16_t*)&bmpData[28] = 16;
    *(uint32_t*)&bmpData[34] = dataSize;

    // Pixel-Daten (RGB565 → BMP raw)
    for (int y = 0; y < height; y++) {
        uint8_t* rowPtr = bmpData + headerSize + y * rowSize;
        for (int x = 0; x < width; x++) {
            uint16_t px = data[y * width + x];
            if (px == TRANSPARENT_COLOR) px = 0xFFFF;

            rowPtr[x * 2] = px & 0xFF;
            rowPtr[x * 2 + 1] = px >> 8;
        }
    }

    String result = base64::encode(bmpData, fileSize);
    result.replace("\n", "");

    delete[] bmpData;

    return result;
}


// clear TFT display
void clearTFT() {
#if defined CS_2
    setCS2(LOW);
    tft.fillRect(0, 0, CLOCK_WIDTH, CLOCK_HEIGHT, TFT_BLACK);
    setCS1(LOW);
#endif
    tft.fillRect(0, 0, CLOCK_WIDTH, CLOCK_HEIGHT, TFT_BLACK);
}


// Rotiert die Zeiger basierend auf der Display-Rotation
float rotatedAngle(float angle, int orientation) {
    if (psramAvailable) {
        return angle + (orientation * 90);
    }   
    return angle;
}


// überprüft, ob die BMP-Datei das erwartete Format hat
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
String getBmpInfo(const String& filename) {
    // Normalisiere Pfad (einfach und eindeutig)
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
        return String(width) + "&nbsp;x&nbsp;" + String(height) + " / 16 bpp (RLE, -" + ratio + ")";
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

    return String(abs(width)) + "&nbsp;x&nbsp;" + String(abs(height)) + " / " + String(bpp) + " bpp";
}


// Skaliert eine BMP-Datei auf die gewünschte Größe und speichert sie
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
    int32_t inW = 0, inH = 0;
    uint16_t bpp = 16;
    bool flip = false;
    uint32_t offset = 0;
    int inRowSize = 0;
    uint8_t* rowBuf = nullptr;   // fuer Standard-BMP: ein Zeilenpuffer
    uint16_t* rleSrcBuf = nullptr; // fuer RLEB: komplett dekodiertes Bild

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

        uint8_t* compBuf = (uint8_t*)malloc(compressedSize);
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

        rleSrcBuf = (uint16_t*)malloc(uncompressedSize);
        if (!rleSrcBuf) {
            free(compBuf);
            DEBUG_PRINTLN("[BMP Scale] Memory allocation failed (rleSrcBuf)");
            return false;
        }
        rleDecode565(compBuf, compressedSize, rleSrcBuf, (size_t)inW * inH);
        free(compBuf);

        flip = false; // RLEB ist immer bereits Top-Down gespeichert
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
        rowBuf = (uint8_t*)malloc(inRowSize);
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
                pixel = row16[srcX];
            }
            else if (bpp == 16) {
                // 16 bpp (RGB565) → direkt übernehmen
                uint16_t* r16 = (uint16_t*)rowSource;
                pixel = r16[srcX];
            }
            else if (bpp == 24) {
                // 24 bpp (RGB888) → 16 bpp (RGB565)
                uint8_t* row24 = rowSource + (srcX * 3);
                uint8_t r = row24[2];
                uint8_t g = row24[1];
                uint8_t b = row24[0];
                pixel = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
            }
            else if (bpp == 32) {
                // 32 bpp (ARGB8888) → 16 bpp (RGB565)
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

    // Zielformat entscheiden: face_*.bmp wird RLE-komprimiert gespeichert
    // (spart deutlich Flash-Platz), alles andere (z.B. hand_set*.bmp oder
    // manuell skalierte Dateien) bleibt Standard-BMP wie bisher.
    String targetPathStr = String(targetPath);
    if (!targetPathStr.startsWith("/")) targetPathStr = "/" + targetPathStr;
    bool storeAsRle = targetPathStr.startsWith("/face_");

    // Der Bildpuffer ist quadratisch, das sichtbare Display aber rund: bei
    // Zifferblaettern wird daher alles ausserhalb des sichtbaren Kreises
    // (Durchmesser = kuerzere Kantenlaenge) auf Weiss gesetzt, damit dort
    // keine zufaelligen Bildreste aus den Ecken des Original-Uploads
    // sichtbar werden.
    if (storeAsRle) {
        float cx = outW / 2.0f;
        float cy = outH / 2.0f;
        float radius = (outW < outH ? outW : outH) / 2.0f;
        float radiusSq = radius * radius;
        for (int y = 0; y < outH; y++) {
            for (int x = 0; x < outW; x++) {
                float dx = (x + 0.5f) - cx;
                float dy = (y + 0.5f) - cy;
                if (dx * dx + dy * dy > radiusSq) {
                    outImage[y * outW + x] = 0xFFFF; // Weiss (RGB565)
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
        uint8_t* rleBuf = (uint8_t*)malloc(maxSize);
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
        *(int32_t*)&bmpHeader[22] = -outH; // Top-down BMP
        *(uint16_t*)&bmpHeader[26] = 1;

        *(uint16_t*)&bmpHeader[28] = 16; // Set to 16 bpp for RGB565
        *(uint32_t*)&bmpHeader[30] = 3; // Compression method: BI_BITFIELDS
        *(uint32_t*)&bmpHeader[34] = dataSize;

        // Add RGB565 color masks
        *(uint32_t*)&bmpHeader[54] = 0xF800; // Red mask
        *(uint32_t*)&bmpHeader[58] = 0x07E0; // Green mask
        *(uint32_t*)&bmpHeader[62] = 0x001F; // Blue mask

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
// Format und konvertiert sie einmalig zum neuen, platzsparenden RLE-Format.
// scaleAndSaveBmp() speichert Zieldateien mit "face_"-Praefix automatisch
// als RLE (siehe dort) - hier wird die Datei einfach auf ihre eigene
// Groesse "umskaliert" (Quelle = Ziel = derselbe Pfad), wodurch derselbe
// Inhalt entsteht, nur eben komprimiert. Bereits RLE-komprimierte Dateien
// werden uebersprungen. Wird einmalig in setup() aufgerufen.
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


// Liest nur das allererste Pixel (0,0) einer RLEB-Datei, ohne das ganze Bild
// zu dekodieren - preiswerte Pruefung, ob die Kreismaskierung fuer runde
// Displays (siehe scaleAndSaveBmp()) fuer diese Datei bereits angewendet
// wurde (Pixel (0,0) liegt garantiert ausserhalb des Kreises).
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


// Wendet die Kreismaskierung (siehe scaleAndSaveBmp()) einmalig auf bereits
// vorhandene, schon RLE-komprimierte Zifferblaetter an, die VOR Einfuehrung
// dieser Maskierung migriert bzw. hochgeladen wurden. Nutzt die billige
// peekFirstPixelIsWhite()-Pruefung, um bereits maskierte Dateien zu
// ueberspringen, ohne einen zusaetzlichen Persistenz-Zustand zu benoetigen.
// Wird einmalig in setup() aufgerufen.
void remaskExistingFaceCorners() {
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
// herunter und sendet sie DIREKT als HTTP-Antwort - ohne eine skalierte Kopie
// auf dem Flash abzulegen. Dient als schnelle Vorschau fuer <img>-Tags im
// Webinterface: statt bei jedem Seitenaufruf die volle Zifferblatt-Aufloesung
// (z.B. 240x240 = ~115 KB) zu uebertragen, nur fuer eine 80x80-Miniaturansicht,
// wird hier nur die tatsaechlich benoetigte, kleine Groesse gesendet.
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
    int inRowSizeStd = 0;     // nur fuer Standard-BMP
    bool flipStd = false;     // nur fuer Standard-BMP

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
    *(int32_t*)&outBmp[22] = -outH; // Top-down BMP
    *(uint16_t*)&outBmp[26] = 1;
    *(uint16_t*)&outBmp[28] = 16; // 16 bpp fuer RGB565
    *(uint32_t*)&outBmp[30] = 3; // Kompressionsmethode: BI_BITFIELDS
    *(uint32_t*)&outBmp[34] = outDataSize;

    // RGB565-Farbmasken ergaenzen (ohne diese interpretieren Browser 16-bpp-BMPs
    // standardmaessig als RGB555 statt RGB565 -> sichtbare Falschfarben)
    *(uint32_t*)&outBmp[54] = 0xF800; // Rot-Maske
    *(uint32_t*)&outBmp[58] = 0x07E0; // Gruen-Maske
    *(uint32_t*)&outBmp[62] = 0x001F; // Blau-Maske

    if (isRle) {
        // RLEB: sequentiell durch die Quelle dekodieren und NUR die
        // tatsaechlich fuer das Downsampling benoetigten Zeilen behalten -
        // kein voller ~115-KB-Quellpuffer noetig (RLE erlaubt kein
        // direktes Anspringen einzelner Zeilen wie bei Standard-BMP).
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
        uint8_t* rowBuf = (uint8_t*)malloc(inRowSizeStd);
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
// Ergebnis SOFORT per Chunked-Response an den Webserver-Client, statt es
// komplett im RAM zu materialisieren. Haelt zu keinem Zeitpunkt mehr als
// eine Bildzeile + einen kleinen Lese-Puffer im RAM (statt bis zu ~115 KB
// bei einem 240x240-Zifferblatt) - wichtig auf Geraeten mit knappem freiem
// SRAM (siehe Boot-Log: TFT-Sprites/clockFaceBuffer liegen zwar im PSRAM,
// aber der freie SRAM-Sockel reicht oft nicht fuer eine komplette
// Dekodierung auf einmal). Wird von /file und /download in
// webserver_routes.h fuer RLEB-komprimierte face_*.bmp-Dateien genutzt.
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
    *(int32_t*)&bmpHeader[22] = -h; // Top-down BMP
    *(uint16_t*)&bmpHeader[26] = 1;
    *(uint16_t*)&bmpHeader[28] = 16;
    *(uint32_t*)&bmpHeader[30] = 3; // BI_BITFIELDS
    *(uint32_t*)&bmpHeader[34] = dataSize;
    *(uint32_t*)&bmpHeader[54] = 0xF800;
    *(uint32_t*)&bmpHeader[58] = 0x07E0;
    *(uint32_t*)&bmpHeader[62] = 0x001F;

    webserver.setContentLength(CONTENT_LENGTH_UNKNOWN);
    webserver.send(200, contentType, "");
    webserver.sendContent_P((const char*)bmpHeader, 66);

    // Kleiner Lese-Puffer fuer die komprimierten Eingabedaten (aus der
    // Datei nachgefuellt, statt sie komplett vorab einzulesen).
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
    if (col > 0 && row < h) {
        webserver.sendContent_P((const char*)rowBuf, rowSize);
        row++;
    }

    delete[] rowBuf;
    f.close();
    webserver.sendContent(""); // Ende der Chunked-Uebertragung signalisieren

    return ok && written >= total;
}


// --- Funktion: Schaltet die LED ein (wenn definiert) ---  
void setLedOff() {
#ifdef LED_BOARD
    pinMode(LED_BOARD, OUTPUT);
    digitalWrite(LED_BOARD, LOW);
#endif
}


// --- Funktion: Schaltet die LED aus (wenn definiert) ---
void setLedOn() {
#ifdef LED_BOARD
    pinMode(LED_BOARD, OUTPUT);
    digitalWrite(LED_BOARD, HIGH);
#endif
}


// --- Funktion: LED toggeln
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


// Laesst die Status-LED blinken, sobald ein gueltiges DCF77-Zeitsignal empfangen wurde
void toggleLedDcf77() {
    if (dcfTimeFound) toggleLED();
}


// --- Funktion: Wechselt zum nächsten Hintergrundbild ---
void switchToNextBackground() {
    std::vector<String> faces;

    // 1) Sammle alle face_*.bmp Dateien aus LittleFS (verwende Pfad wie File::name() liefert)
    File root = LittleFS.open("/");
    if (root) {
        File f = root.openNextFile();
        while (f) {
            String name = f.name(); // meist mit führendem '/'
            // Akzeptiere sowohl "/face_.." als auch "face_.."
            if (name.startsWith("/")) {
                if (name.startsWith("/face_") && name.endsWith(".bmp")) {
                    // Duplikate vermeiden
                    bool exists = false;
                    for (const auto& s : faces) if (s == name) { exists = true; break; }
                    if (!exists) faces.push_back(name);
                }
            }
            else {
                if (name.startsWith("face_") && name.endsWith(".bmp")) {
                    String normalizedName = "/" + name;
                    bool exists = false;
                    for (const auto& s : faces) if (s == normalizedName) { exists = true; break; }
                    if (!exists) faces.push_back(normalizedName);
                }
            }
            f = root.openNextFile();
        }
        root.close();
    }

    // 2) Stelle sicher, dass builtin default vorhanden ist (am Anfang)
    bool hasDefault = false;
    for (const auto& s : faces) if (s == "/face_default.bmp") { hasDefault = true; break; }
    if (!hasDefault) faces.insert(faces.begin(), "/face_default.bmp");

    if (faces.empty()) {
        DEBUG_PRINTLN("[TOUCH] No faces found");
        return;
    }

    // 3) Normalisiere aktuellen Auswahlwert
    String sel = selectedBackground;
    sel.trim();
    if (!sel.startsWith("/")) sel = "/" + sel;

    // 4) Bestimme aktuellen Index
    int idx = -1;
    for (size_t i = 0; i < faces.size(); ++i) {
        if (faces[i] == sel) { idx = (int)i; break; }
    }

    // 5) Wenn nicht gefunden: versuche eine tolerantere Suche (ohne führenden '/')
    if (idx < 0) {
        String selNoSlash = sel;
        if (selNoSlash.startsWith("/")) selNoSlash = selNoSlash.substring(1);
        for (size_t i = 0; i < faces.size(); ++i) {
            String cmp = faces[i];
            if (cmp.startsWith("/")) cmp = cmp.substring(1);
            if (cmp == selNoSlash) { idx = (int)i; break; }
        }
    }

    // 6) Wähle nächstes Element:
    int next;
    if (idx < 0) {
        // Falls aktuelle Auswahl unbekannt ist, wähle erstes user-face (wenn default an pos 0) sonst 0
        if (faces.size() > 1 && faces[0] == "/face_default.bmp") next = 1;
        else next = 0;
    }
    else {
        next = (idx + 1) % (int)faces.size();
    }

    // 7) übernehme und speichere
    selectedBackground = faces[next];
    preferences.putString(PK_BACKGROUND, selectedBackground);

    // Debug
    DEBUG_PRINT("[TOUCH] Faces: ");
    for (const auto& s : faces) DEBUG_PRINT(s + " ");
    DEBUG_PRINTLN();
    DEBUG_PRINTLN("[TOUCH] Current: " + sel + " idx=" + String(idx) + " -> Next: " + selectedBackground);
        
    freeClockFaceBuffer();
    loadClockFace();
    loadHandSprites();
    updateClock();
}


// --- Funktion: Touch prüfen (nicht-blockierend, mit Entprellung) ---
void checkTouchInput() {
#ifdef TOUCH_PIN

    uint16_t var = touchRead(TOUCH_PIN);

    bool state = false;

    if (var > 15000 && var < 65535) state = true;

    // DEBUG_PRINTLN("Touch read: " + String(var));
    //DEBUG_PRINTLN("Touch state: " + String(state));

    // Flanke LOW->HIGH (kurzer Tip) mit Debounce
    if (state && !touchLastState && (millis() - touchLastMillis) > TOUCH_DEBOUNCE_MS) {
        touchLastMillis = millis();
        DEBUG_PRINTLN("switch");
        //switchToNextBackground();
        switchToNextPreset();
    }
    touchLastState = state;
#endif
}


// Validiert den geladenen Preferences-Eintrag für background und repariert falls nötig
static void validateSelectedBackground() {
    // Normalisieren
    selectedBackground.trim();
    if (selectedBackground.length() == 0) selectedBackground = "/face_default.bmp";
    if (!selectedBackground.startsWith("/")) selectedBackground = "/" + selectedBackground;

    DEBUG_PRINTLN("[BG] Pref load: '" + selectedBackground + "'");

    // LittleFS muss gemountet sein
    if (!LittleFS.exists(selectedBackground)) {
        DEBUG_PRINTLN("[BG] File not found: " + selectedBackground);
        // Versuche tolerant auch ohne führenden Slash (falls gespeichert ohne '/')
        String withoutSlash = selectedBackground;
        if (withoutSlash.startsWith("/")) withoutSlash = withoutSlash.substring(1);
        if (LittleFS.exists("/" + withoutSlash)) {
            selectedBackground = "/" + withoutSlash;
            DEBUG_PRINTLN("[BG] Found (alt) file: " + selectedBackground);
        }
        else {
            // Fallback auf Default
            selectedBackground = "/face_default.bmp";
            preferences.putString(PK_BACKGROUND, selectedBackground);
            DEBUG_PRINTLN("[BG] Falling back to default and saved: " + selectedBackground);
            return;
        }
    }

    // Prüfe BMP-Format (Größe / bpp)
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
void updateHandWidths(int newHourWidth, int newMinuteWidth, int newSecondWidth) {

    // Aktualisiere die globalen Breiten
    hourHandWidth = newHourWidth;
    minuteHandWidth = newMinuteWidth;
    secondHandWidth = newSecondWidth;

    // Alte Sprites löschen
    hourHandSprite.deleteSprite();
    minuteHandSprite.deleteSprite();
    secondHandSprite.deleteSprite();

    // Neue Sprites erstellen
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
    loadHandSprites();
}


// Parst die Zeigerbreiten aus dem Dateinamen des Hintergrundbildes (test)
void parseBackgroundFilename(const String& filename, int& hourWidth, int& minuteWidth, int& secondWidth) {
    // Standardwerte setzen
    hourWidth = HAND_WIDTH;
    minuteWidth = HAND_WIDTH;
    secondWidth = HAND_WIDTH;

    // Suche nach dem ersten `!`
    int firstHash = filename.indexOf('!');
    if (firstHash == -1) {
        // Kein `!` gefunden, Standardwerte verwenden
        return;
    }

    // Schneide den relevanten Teil nach dem ersten `#` ab
    String params = filename.substring(firstHash + 1);

    // Teile die Parameter anhand von `!`
    int secondHash = params.indexOf('!');
    int thirdHash = params.indexOf('!', secondHash + 1);

    if (secondHash != -1 && thirdHash != -1) {
        // Extrahiere die Werte
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
void enableTouch() {
#ifdef TOUCH_PIN
    touchEnabled = true;
    pinMode(TOUCH_PIN, INPUT_PULLDOWN);

    // Touch erst nach kurzer Verzögerung aktivieren (verhindert frühe Reads während Init)
    touchEnableAt = millis() + 1000; // 1000 ms Verzögerung
    DEBUG_PRINTLN("[TOUCH] Touch aktiviert");
#endif
}


// Touch-Funktionalität deaktivieren
void disableTouch() {
#ifdef TOUCH_PIN
    touchEnabled = false;
    pinMode(TOUCH_PIN, INPUT);
    DEBUG_PRINTLN("[TOUCH] Touch deaktiviert");
#endif
}



