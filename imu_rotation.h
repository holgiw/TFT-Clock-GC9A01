#pragma once

    // Automatische Rotation ueber den Lagesensor QMI8658 (nur Waveshare ESP32-S3-LCD-1.28, HAS_IMU in config.h).
    // Die Schwerkraft in der Displayebene ergibt eine von vier Richtungen; erst nach IMU_STABLE_MS gleicher Lage
    // dreht die Uhr. Liegt sie flach, bleibt die Rotation. Bezug: beim Einschalten von "automatisch".

    // Automatic rotation via the QMI8658 motion sensor (Waveshare ESP32-S3-LCD-1.28 only, HAS_IMU in config.h).
    // Gravity in the display plane gives one of four directions; the clock only turns after IMU_STABLE_MS in the
    // same position. Lying flat, the rotation stays. Reference: taken when "automatic" is switched on.

#if HAS_IMU
    uint8_t imuAddr = 0;      // 0 = kein Sensor gefunden / no sensor found
    int8_t imuQuadrant = -1;  // zuletzt erkannte Lage 0-3, -1 = unklar (flach) / last detected position 0-3, -1 = unclear (flat)

    bool imuWrite(uint8_t reg, uint8_t value) {
        Wire.beginTransmission(imuAddr);
        Wire.write(reg);
        Wire.write(value);
        return Wire.endTransmission() == 0;
    }

    bool imuRead(uint8_t reg, uint8_t* buf, uint8_t len) {
        Wire.beginTransmission(imuAddr);
        Wire.write(reg);
        if (Wire.endTransmission(false) != 0) return false;
        if (Wire.requestFrom(imuAddr, len) != len) return false;
        for (uint8_t i = 0; i < len; i++) buf[i] = Wire.read();
        return true;
    }
#endif


    // Sensor suchen (WHO_AM_I 0x05 auf 0x6B oder 0x6A) und nur den Beschleunigungsmesser einschalten: CTRL1 0x40
    // (Adressen hochzaehlen), CTRL2 0x06 (+-2 g, ca. 125 Hz), CTRL7 0x01. Nach Wire.begin() aufrufen.

    // Find the sensor (WHO_AM_I 0x05 on 0x6B or 0x6A) and switch on only the accelerometer: CTRL1 0x40 (address
    // auto increment), CTRL2 0x06 (+-2 g, about 125 Hz), CTRL7 0x01. Call after Wire.begin().

    void imuBegin() {
#if HAS_IMU
        Wire.begin(SDA_PIN, SCL_PIN); // ohne RTC beendet setup() den Bus ggf. (Wire.end()) / without an RTC setup() may end the bus
        const uint8_t addresses[] = { 0x6B, 0x6A };
        for (uint8_t a : addresses) {
            imuAddr = a;
            uint8_t who = 0;
            if (imuRead(0x00, &who, 1) && who == 0x05) {
                bool ok = imuWrite(0x02, 0x40) && imuWrite(0x03, 0x06) && imuWrite(0x08, 0x01);
                DEBUG_PRINTLN("[IMU] QMI8658 found at 0x" + String(a, HEX) + (ok ? "" : " - configuration failed"));
                if (ok) return;
            }
        }
        imuAddr = 0;
        DEBUG_PRINTLN("[IMU] QMI8658 not found - no automatic rotation");
#endif
    }

    bool imuAvailable() {
#if HAS_IMU
        return imuAddr != 0;
#else
        return false;
#endif
    }

    // Lage aus der Schwerkraft: 0 = +X, 1 = +Y, 2 = -X, 3 = -Y nach unten; -1, wenn die Uhr flach liegt oder
    // zwischen zwei Lagen steht. Die Byte-Reihenfolge waehlt die Auswertung, deren Betrag naeher an 1 g liegt.

    // Position from gravity: 0 = +X, 1 = +Y, 2 = -X, 3 = -Y pointing down; -1 if the clock lies flat or is
    // between two positions. The byte order picks the interpretation whose magnitude is closer to 1 g.

    int8_t imuReadQuadrant() {
#if HAS_IMU
        uint8_t b[6];
        if (!imuAvailable() || !imuRead(0x35, b, 6)) return -1;
        float le[3], be[3];
        for (int i = 0; i < 3; i++) {
            le[i] = (int16_t)(b[2 * i] | (b[2 * i + 1] << 8)) / 16384.0f;
            be[i] = (int16_t)((b[2 * i] << 8) | b[2 * i + 1]) / 16384.0f;
        }
        float nle = sqrtf(le[0] * le[0] + le[1] * le[1] + le[2] * le[2]);
        float nbe = sqrtf(be[0] * be[0] + be[1] * be[1] + be[2] * be[2]);
        const float* g = fabsf(nle - 1.0f) <= fabsf(nbe - 1.0f) ? le : be;
        float ax = g[0], ay = g[1];
        float ix = fabsf(ax), iy = fabsf(ay);

        // Deutlich gekippt (mind. 0,5 g in der Ebene) und eine Achse klar vorn (Faktor 1,5 = gut 33 Grad Abstand
        // zur Diagonale), sonst unklar
        // Clearly tilted (at least 0.5 g in the plane) and one axis clearly ahead (factor 1.5 = a good 33 degrees
        // away from the diagonal), otherwise unclear

        if (max(ix, iy) < 0.5f) return -1;
        if (ix > 1.5f * iy) return ax > 0 ? 0 : 2;
        if (iy > 1.5f * ix) return ay > 0 ? 1 : 3;
#endif
        return -1;
    }

    // Lage + gespeicherter Bezug -> Rotation 0-3 (IMU_ROTATION_DIR in config.h: Drehsinn des Sensors)
    // Position + stored reference -> rotation 0-3 (IMU_ROTATION_DIR in config.h: turning direction of the sensor)

    uint8_t imuRotationFor(int8_t quadrant) {
        int offset = preferences.getUChar(PK_IMU_ROT_OFFSET, 0);
        return (uint8_t)(((quadrant * IMU_ROTATION_DIR + offset) % 4 + 4) % 4);
    }

    // Beim Einschalten von "automatisch": die aktuelle Lage gehoert zur gerade eingestellten Rotation. false,
    // wenn die Lage unklar ist - dann bleibt der bisherige Bezug.

    // When "automatic" is switched on: the current position belongs to the rotation set right now. false if the
    // position is unclear - then the previous reference stays.

    bool imuCalibrate(uint8_t currentRotation) {
        int8_t q = imuReadQuadrant();
        if (q < 0 || currentRotation > 3) {
            DEBUG_PRINTLN("[IMU] Reference not taken - position unclear (lying flat?)");
            return false;
        }
        uint8_t offset = (uint8_t)(((currentRotation - q * IMU_ROTATION_DIR) % 4 + 4) % 4);
        preferences.putUChar(PK_IMU_ROT_OFFSET, offset);
        DEBUG_PRINTLN("[IMU] Reference taken: position " + String(q) + " = rotation " + String(currentRotation * 90) + " deg");
        return true;
    }

    // Aus loop(): alle 250 ms lesen, nach IMU_STABLE_MS gleicher Lage Display 1 drehen (wie beim Speichern der
    // Rotation in der Weboberflaeche: Zifferblatt und Zeiger neu laden)

    // From loop(): read every 250 ms, turn display 1 after IMU_STABLE_MS in the same position (as when saving the
    // rotation in the web interface: reload clock face and hands)

    void handleAutoRotation() {
#if HAS_IMU
        if (!autoRotation || !imuAvailable()) return;
        static unsigned long lastRead = 0, stableSince = 0;
        static int8_t candidate = -1;
        if (millis() - lastRead < 250) return;
        lastRead = millis();

        int8_t q = imuReadQuadrant();
        if (q < 0) return; // flach oder unklar: nichts aendern / flat or unclear: change nothing
        if (q != candidate) {
            candidate = q;
            stableSince = millis();
            return;
        }
        if (millis() - stableSince < IMU_STABLE_MS) return;
        imuQuadrant = q;
        uint8_t target = imuRotationFor(q);
        if (target == tftRotation1) return;
        DEBUG_PRINTLN("[IMU] Position " + String(q) + " - rotating display to " + String(target * 90) + " deg");
        applyDisplayRotation(1, target);
        freeClockFaceBuffer();
        loadClockFace();
        loadHandSprites();
#endif
    }
