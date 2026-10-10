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
#include <HTTPClient.h>
#include <mbedtls/sha256.h>

    // Kennung der Build-Zeit in der .bin (wie version, nur als Text "Oct  1 2026 12:00:00"): build_uhr4/publish_s3_update.ps1
    // liest sie aus der Datei und schreibt daraus uhr4-s3.txt

    // Marker of the build time in the .bin (like version, only as text "Oct  1 2026 12:00:00"): build_uhr4/publish_s3_update.ps1
    // reads it from the file and writes uhr4-s3.txt from it

    const char fwBuildMarker[] __attribute__((used)) = "UHR4_FW_BUILD=" __DATE__ " " __TIME__;

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


#if HAS_OTA

    // Datei des neuesten GitHub-Releases per HTTPS holen (Zertifikatspruefung mit dem eingebauten Zertifikatspaket,
    // dafuer muss die Uhrzeit gueltig sein). Weiterleitungen folgt die Funktion selbst, mit einer frischen
    // TLS-Verbindung je Server (bei wiederverwendeter Verbindung setzte der Zielserver sie oft zurueck) und
    // einem zweiten Versuch je Schritt. Bei Erfolg steht die Antwort bereit, der Aufrufer ruft danach http.end().

    // Fetch a file of the latest GitHub release via HTTPS (certificate check with the built-in certificate bundle,
    // so the time must be valid). The function follows redirects itself, with a fresh TLS connection per server
    // (with a reused connection the target server often reset it) and a second try per step. On success the
    // response is ready, the caller then calls http.end().

    bool githubOpen(HTTPClient& http, std::unique_ptr<NetworkClientSecure>& client, const char* extension, String& error) {
        if (WiFi.status() != WL_CONNECTED) {
            error = "no WiFi connection";
            return false;
        }
        if (time(nullptr) < 1700000000) {
            error = "time not set (needed for the certificate check)";
            return false;
        }
        String url = String(FW_UPDATE_URL) + extension;
        for (int hop = 0; hop < 5; hop++) {
            int code = 0;
            for (int attempt = 0; attempt < 2; attempt++) {
                http.end();
                client.reset(new NetworkClientSecure());
                client->useBuiltinCACertBundle();
                client->setHandshakeTimeout(10);
                http.setConnectTimeout(10000);
                http.setTimeout(10000);
                http.setFollowRedirects(HTTPC_DISABLE_FOLLOW_REDIRECTS);
                http.setUserAgent("uhr4");
                if (!http.begin(*client, url)) {
                    error = "begin failed";
                    return false;
                }
                code = http.GET();
                if (code > 0) break;
                error = HTTPClient::errorToString(code);
                char tlsError[80] = "";
                int tlsCode = client->lastError(tlsError, sizeof(tlsError));
                if (tlsCode) error += " (TLS " + String(tlsCode) + (tlsError[0] ? String(": ") + tlsError : String("")) + ")";
                int hostEnd = url.indexOf('/', 8);
                error += " via " + url.substring(8, hostEnd > 0 ? hostEnd : url.length()) + " [" + extension + "]";
                DEBUG_PRINTLN("[UPDATE] " + error + (attempt == 0 ? " - retrying" : ""));
                delay(1500);
            }
            if (code <= 0) {
                http.end();
                return false;
            }
            if (code == HTTP_CODE_OK) return true;
            if (code == 301 || code == 302 || code == 303 || code == 307 || code == 308) {
                url = http.getLocation();
                if (!url.startsWith("https://")) {
                    error = "unexpected redirect";
                    http.end();
                    return false;
                }
                continue;
            }
            error = "HTTP " + String(code) + " [" + extension + "]";
            http.end();
            return false;
        }
        error = "too many redirects";
        http.end();
        return false;
    }

    // uhr4-s3.txt lesen: Zeile 1 = Build-Zeit, Zeile 2 = SHA-256 der .bin (64 Hex-Zeichen, darf fehlen)
    // Read uhr4-s3.txt: line 1 = build time, line 2 = SHA-256 of the .bin (64 hex characters, may be missing)

    bool githubReadInfo(String& build, String& sha, String& error) {
        std::unique_ptr<NetworkClientSecure> client;
        HTTPClient http;
        if (!githubOpen(http, client, ".txt", error)) return false;
        String body = http.getString();
        http.end();
        body.replace("\r", "");
        build = body.substring(0, body.indexOf('\n') < 0 ? body.length() : body.indexOf('\n'));
        build.trim();
        int second = body.indexOf('\n');
        sha = second < 0 ? String("") : body.substring(second + 1);
        sha.trim();
        sha.toLowerCase();
        if (build.length() != 19 || build[4] != '-' || build[10] != ' ') {
            error = "unexpected answer";
            return false;
        }
        return true;
    }

#endif

    // Pruefung auf ein neueres Release: true = Abfrage geklappt (fwRemoteBuild gesetzt), sonst steht der Grund in
    // fwCheckError. Blockiert bis zu etwa 20 s.

    // Check for a newer release: true = the request worked (fwRemoteBuild set), otherwise the reason is in
    // fwCheckError. Blocks for up to about 20 s.

    bool checkFirmwareUpdate() {
#if HAS_OTA
        String build, sha, error;
        fwLastCheckMillis = millis();
        struct tm now;
        fwCheckedDay = getLocalTime(&now, 0) ? now.tm_yday : -1;
        bool ok = githubReadInfo(build, sha, error);
        if (ok) {
            fwRemoteBuild = build;
            fwCheckError = "";
        }
        else {
            fwCheckError = error;
        }
        DEBUG_PRINTLN("[UPDATE] GitHub check: " + (ok ? "release " + build + " (installed " + String(version) + ", " + String(fwBuildMarker) + ")" : "failed - " + error));
        return ok;
#else
        return false;
#endif
    }

    // 1 = GitHub hat eine neuere Version, 0 = gleich, -1 = installierte ist neuer, 2 = unbekannt
    // 1 = GitHub has a newer version, 0 = same, -1 = installed one is newer, 2 = unknown

    int firmwareUpdateState() {
        if (fwRemoteBuild.length() == 0) return 2;
        int c = strcmp(fwRemoteBuild.c_str(), version);
        return c > 0 ? 1 : (c == 0 ? 0 : -1);
    }

    // Aus loop(): einmal kurz nach dem Start, danach jede Nacht um 3 Uhr (Zeit gueltig, WLAN verbunden). Ein Fehlschlag
    // wird fruehestens nach einer Stunde wiederholt - die Abfrage haelt die Uhr bis zu einigen Sekunden an.

    // From loop(): once shortly after boot, then every night at 3 o'clock (valid time, WiFi connected). A failure is
    // retried after an hour at the earliest - the request stops the clock for up to a few seconds.

    void firmwareUpdateTick() {
#if HAS_OTA
        if (otaInProgress || WiFi.status() != WL_CONNECTED || WiFi.getMode() == WIFI_AP) return;
        if (fwLastCheckMillis != 0 && millis() - fwLastCheckMillis < WAIT_1h) return;
        if (fwLastCheckMillis == 0 && millis() < 3 * WAIT_1m) return;
        struct tm now;
        if (!getLocalTime(&now, 0)) return;
        bool firstCheck = fwLastCheckMillis == 0;
        bool nightly = now.tm_hour == 3 && now.tm_yday != fwCheckedDay;
        if (firstCheck || nightly) checkFirmwareUpdate();
#endif
    }

    // Firmware von GitHub einspielen: nur bei neuerer Build-Zeit (force: auch gleiche/aeltere, nur ueber die Adresse
    // /firmware/github?force=1), .bin in die zweite App-Partition, Kopf und SHA-256 (uhr4-s3.txt) pruefen, dann
    // aktivieren. Der Aufrufer startet neu. Blockiert, bis alles geladen ist.

    // Install firmware from GitHub: only for a newer build time (force: also same/older, only via the address
    // /firmware/github?force=1), .bin to the second app partition, check header and SHA-256 (uhr4-s3.txt), then
    // activate. The caller restarts. Blocks until everything is downloaded.

    bool installFirmwareFromGithub(String& error, bool force) {
#if HAS_OTA
        if (otaInProgress) {
            error = "an update is already running";
            return false;
        }
        String build, sha;
        if (!githubReadInfo(build, sha, error)) return false;
        fwRemoteBuild = build;
        if (sha.length() != 64) {
            error = "checksum missing in uhr4-s3.txt";
            return false;
        }
        if (!force && strcmp(build.c_str(), version) <= 0) {
            error = "no newer firmware on GitHub";
            return false;
        }

        std::unique_ptr<NetworkClientSecure> client;
        HTTPClient http;
        if (!githubOpen(http, client, ".bin", error)) return false;
        int total = http.getSize();
        if (total < 200000) {
            error = "unexpected file size";
            http.end();
            return false;
        }
        if (!Update.begin(total, U_FLASH)) {
            error = Update.errorString();
            http.end();
            return false;
        }

        otaInProgress = true;
        otaStartMillis = millis();
        otaShownPercent = -1;
        DEBUG_PRINTLN("[UPDATE] Installing " + build + " from GitHub, " + String(total) + " bytes");
        showOtaStatus("GitHub 0 %");

        mbedtls_sha256_context sum;
        mbedtls_sha256_init(&sum);
        mbedtls_sha256_starts(&sum, 0);
        uint8_t* buffer = (uint8_t*)malloc(2048);
        WiFiClient* stream = http.getStreamPtr();
        int done = 0;
        unsigned long lastData = millis();
        error = buffer ? "" : "out of memory";
        while (buffer && error.length() == 0 && done < total) {
            int avail = stream->available();
            if (avail <= 0) {
                if (!stream->connected() || millis() - lastData > 15000) {
                    error = "download interrupted";
                    break;
                }
                delay(5);
                continue;
            }
            int got = stream->readBytes(buffer, min(avail, 2048));
            if (got <= 0) continue;
            lastData = millis();
            if (done == 0) {
                uint16_t chip = (uint16_t)(buffer[12] | (buffer[13] << 8));
                if (buffer[0] != 0xE9 || chip != CONFIG_IDF_FIRMWARE_CHIP_ID) {
                    error = "not a firmware for this chip (ESP32-S3)";
                    break;
                }
            }
            mbedtls_sha256_update(&sum, buffer, got);
            if (Update.write(buffer, got) != (size_t)got) {
                error = Update.errorString();
                break;
            }
            done += got;
            showOtaProgress(done, total);
        }
        free(buffer);
        http.end();

        unsigned char hash[32];
        mbedtls_sha256_finish(&sum, hash);
        mbedtls_sha256_free(&sum);
        if (error.length() == 0) {
            char hex[65];
            for (int i = 0; i < 32; i++) snprintf(hex + 2 * i, 3, "%02x", hash[i]);
            if (sha != String(hex)) error = "checksum does not match";
        }
        if (error.length() == 0 && !Update.end(true)) error = Update.errorString();
        if (error.length() > 0) {
            if (Update.isRunning()) Update.abort();
            otaInProgress = false;
            DEBUG_PRINTLN("[UPDATE] Install failed: " + error);
            return false;
        }
        DEBUG_PRINTLN("[UPDATE] Firmware from GitHub installed");
        return true;
#else
        error = "not available on this chip";
        return false;
#endif
    }

    void setupOtaRoutes() {
#if HAS_OTA

        // Abfrage des Stands: ohne Parameter der zuletzt gemerkte Stand, mit force=1 neu von GitHub holen
        // Query the state: the last remembered state without a parameter, force=1 fetches it again from GitHub

        webserver.on("/firmware/check", HTTP_GET, []() {
            if (!isPrivateNetworkIp(webserver.client().remoteIP())) {
                webserver.send(403, "text/plain", "Only available from a private network");
                return;
            }
            if (webserver.arg("force") == "1") checkFirmwareUpdate();
            static const char* names[] = { "older", "same", "newer", "unknown" };
            int state = firmwareUpdateState();
            String error = fwCheckError;
            error.replace("\\", "/");
            error.replace("\"", "'");
            webserver.send(200, "application/json", String("{\"state\":\"") + names[state == 2 ? 3 : state + 1] +
                "\",\"remote\":\"" + fwRemoteBuild + "\",\"installed\":\"" + String(version) + "\",\"error\":\"" + error + "\"}");
            });

        webserver.on("/firmware/github", HTTP_POST, []() {
            if (!isPrivateNetworkIp(webserver.client().remoteIP())) {
                webserver.send(403, "text/plain", "Only available from a private network");
                return;
            }
            String error;
            if (!installFirmwareFromGithub(error, webserver.arg("force") == "1")) {
                webserver.send(400, "text/plain", error);
                return;
            }
            webserver.send(200, "text/plain", "OK");
            showOtaStatus("Neustart / restart");
            otaInProgress = false;
            delay(500);
            espReboot();
            });

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
