#pragma once

    // Firmware-Update ueber WLAN (nur ESP32-S3, HAS_OTA in config.h): Hochladen von uhr4.ino.bin auf der Seite
    // "Sicherung" (/firmware/update) und ArduinoOTA fuer Visual Micro/Arduino IDE (Port OTA_PORT, ohne
    // Passwort). Die neue Firmware landet in der zweiten App-Partition, erst nach der Pruefung startet sie.

    // Firmware update over WiFi (ESP32-S3 only, HAS_OTA in config.h): uploading uhr4.ino.bin on the "Backup"
    // page (/firmware/update) and ArduinoOTA for Visual Micro/Arduino IDE (port OTA_PORT, without password).
    // The new firmware goes to the second app partition, it only starts after the check.

#if HAS_OTA
#include <ArduinoOTA.h>
#include <Update.h>

    bool arduinoOtaStarted = false;
    int otaShownPercent = -1;
#endif


    // Meldung auf dem Display waehrend des Updates; der naechste Uhr-Frame zeichnet danach wieder voll
    // (beginStatusDraw() markiert das Uhrbild als ueberzeichnet)

    // Message on the display during the update; the next clock frame draws in full again afterwards
    // (beginStatusDraw() marks the clock image as drawn over)

    void showOtaStatus(const String& line2) {
#if HAS_OTA
        showButtonMessage(TFT_CYAN, "Firmware Update", line2, "", TFT_DARKGREY);
#endif
    }

    // Fortschritt in 5-%-Schritten - jede Displayausgabe bremst die Uebertragung
    // Progress in 5 % steps - every display output slows down the transfer

    void showOtaProgress(size_t done, size_t total) {
#if HAS_OTA
        if (total == 0) return;
        int percent = (int)min((size_t)100, done * 100 / total);
        if (otaShownPercent >= 0 && percent < otaShownPercent + 5) return;
        otaShownPercent = percent;
        updateButtonLine(TFT_CYAN, String(percent) + " %"); // nur die Zahl, ohne Flackern / only the number, no flicker
#endif
    }


    // ArduinoOTA nach jedem WLAN-Aufbau neu starten (connectWiFi() in wifi_manager.h) - wie beim NTP-Server
    // ueberlebt der UDP-Socket den Neuaufbau nicht. mDNS verwaltet connectWiFi(), hier kommt nur der Dienst
    // "_arduino._tcp" dazu. Funk-Energiesparen aus: sonst verzoegert Modem-Sleep Pakete und Updates haengen.

    // Restart ArduinoOTA after every WiFi connection (connectWiFi() in wifi_manager.h) - like the NTP server,
    // the UDP socket does not survive the reconnect. connectWiFi() manages mDNS, only the "_arduino._tcp"
    // service is added here. Radio power saving off: modem sleep otherwise delays packets and updates hang.

    void startArduinoOta() {
#if HAS_OTA
        static bool configured = false;
        if (!configured) {
            configured = true;
            ArduinoOTA.setPort(OTA_PORT);
            ArduinoOTA.setMdnsEnabled(false);
            ArduinoOTA.setRebootOnSuccess(false); // Neustart ueber espReboot() / restart via espReboot()
            ArduinoOTA.onStart([]() {
                otaInProgress = true;
                otaStartMillis = millis();
                otaShownPercent = -1;
                DEBUG_PRINTLN("[OTA] Network update started (ArduinoOTA)");
                showOtaStatus("0 %");
            });
            ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
                showOtaProgress(progress, total);
            });
            ArduinoOTA.onEnd([]() {
                DEBUG_PRINTLN("[OTA] Network update finished");
                showOtaStatus("Neustart / restart");
                otaInProgress = false;
                espReboot();
            });
            ArduinoOTA.onError([](ota_error_t error) {
                otaInProgress = false;
                DEBUG_PRINTLN("[OTA] Network update failed, error " + String((int)error));
            });
        }
        WiFi.setSleep(false);
        ArduinoOTA.end();
        ArduinoOTA.setHostname(hostname);
        ArduinoOTA.begin();
        arduinoOtaStarted = true;
        if (pingHostname) MDNS.enableArduino(OTA_PORT, false);
        DEBUG_PRINTLN("[OTA] ArduinoOTA ready on port " + String(OTA_PORT));
#endif
    }

    // Aus loop(): ArduinoOTA bedienen; true = Update laeuft, loop() soll sonst nichts tun. Feuert onEnd()/
    // onError() nie (Verbindung bricht still ab), startet die Uhr nach OTA_TIMEOUT_MS neu, statt stehen zu
    // bleiben.

    // From loop(): serve ArduinoOTA; true = update running, loop() should do nothing else. If onEnd()/
    // onError() never fire (connection drops silently), the clock restarts after OTA_TIMEOUT_MS instead of
    // standing still.

    bool handleOta() {
#if HAS_OTA
        if (arduinoOtaStarted) ArduinoOTA.handle();
        if (otaInProgress) {
            if (millis() - otaStartMillis > OTA_TIMEOUT_MS) {
                DEBUG_PRINTLN("[OTA] Update timed out - restarting");
                espReboot();
            }
            return true;
        }
#endif
        return false;
    }


    // Upload-Teil von /firmware/update: schreibt die Datei beim Empfang in die zweite App-Partition. Nur
    // uhr4.ino.bin dieses Chips - der Kopf (Magic 0xE9, Chip-ID) wird im ersten Block geprueft, merged-,
    // Bootloader- und Partitions-Dateien abgelehnt. Fehler landen in otaUploadError.

    // Upload part of /firmware/update: writes the file to the second app partition while receiving. Only
    // uhr4.ino.bin of this chip - the header (magic 0xE9, chip ID) is checked in the first block, merged,
    // bootloader and partition files are rejected. Errors end up in otaUploadError.

    void handleFirmwareUpload() {
#if HAS_OTA
        HTTPUpload& upload = webserver.upload();
        if (upload.status == UPLOAD_FILE_START) {
            otaUploadError = "";
            if (!isPrivateNetworkIp(webserver.client().remoteIP())) {
                otaUploadError = "Only available from a private network";
                return;
            }
            String name = upload.filename;
            name.toLowerCase();
            if (!name.endsWith(".bin") || name.indexOf("merged") >= 0 || name.indexOf("bootloader") >= 0 ||
                name.indexOf("partitions") >= 0 || name.indexOf("boot_app0") >= 0) {
                otaUploadError = "Wrong file - uhr4.ino.bin from the esp32s3 folder is needed";
                return;
            }
            if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH)) {
                otaUploadError = Update.errorString();
                return;
            }
            otaInProgress = true;
            otaStartMillis = millis();
            otaShownPercent = -1;
            DEBUG_PRINTLN("[OTA] Web update '" + upload.filename + "' from " + webserver.client().remoteIP().toString());
            showOtaStatus("0 %");
        }
        else if (upload.status == UPLOAD_FILE_WRITE) {
            if (otaUploadError.length() || !Update.isRunning()) return;
            if (upload.totalSize == 0) {
                uint16_t chip = upload.currentSize >= 16 ? (uint16_t)(upload.buf[12] | (upload.buf[13] << 8)) : 0xFFFF;
                if (upload.buf[0] != 0xE9 || chip != CONFIG_IDF_FIRMWARE_CHIP_ID) {
                    otaUploadError = "Not a firmware for this chip (ESP32-S3)";
                    Update.abort();
                    otaInProgress = false;
                    return;
                }
            }
            if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                otaUploadError = Update.errorString();
                Update.abort();
                otaInProgress = false;
                return;
            }
            showOtaProgress(upload.totalSize + upload.currentSize, (size_t)max(0, webserver.clientContentLength()));
        }
        else if (upload.status == UPLOAD_FILE_END) {
            if (otaUploadError.length() || !Update.isRunning()) return;
            if (!Update.end(true)) {
                otaUploadError = Update.errorString();
                otaInProgress = false;
                return;
            }
            DEBUG_PRINTLN("[OTA] Web update received: " + String(upload.totalSize) + " bytes");
        }
        else if (upload.status == UPLOAD_FILE_ABORTED) {
            if (Update.isRunning()) Update.abort();
            otaUploadError = "Upload aborted";
            otaInProgress = false;
        }
#endif
    }

    void setupOtaRoutes() {
#if HAS_OTA
        webserver.on("/firmware/update", HTTP_POST, []() {
            if (otaUploadError.length() || !otaInProgress) {
                String error = otaUploadError.length() ? otaUploadError : String("no upload received");
                otaInProgress = false;
                DEBUG_PRINTLN("[OTA] Web update failed: " + error);
                webserver.send(400, "text/plain", error);
                return;
            }
            webserver.send(200, "text/plain", "OK");
            showOtaStatus("Neustart / restart");
            otaInProgress = false;
            delay(500);
            espReboot();
            }, handleFirmwareUpload);
#endif
    }
