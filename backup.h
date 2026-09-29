#pragma once
    // Komplettsicherung: alle Einstellungen (NVS-Namespace "clock", vollstaendig
    // per nvs_entry_find() aufgezaehlt - auch kuenftige Schluessel ohne Liste)
    // plus alle hochgeladenen Zifferblaetter/Zeigersaetze als TAR-Archiv.
    // TAR, weil es sich ohne Kompression Datei fuer Datei streamen laesst und
    // am PC mit jedem Packprogramm (z.B. 7-Zip) einsehbar ist.
    // Inhalt: settings.txt (immer zuerst) + face_*.bmp + hand_set*.bmp.
    // Benoetigt globals.h, prefs_keys.h, declarations.h (vorher eingebunden).

    // Full backup: all settings (NVS namespace "clock", fully enumerated via
    // nvs_entry_find() - future keys too, without a list) plus all uploaded
    // clock faces/hand sets as a TAR archive. TAR because it can be streamed
    // file by file without compression and can be inspected on a PC with any
    // archiver (e.g. 7-Zip).
    // Contents: settings.txt (always first) + face_*.bmp + hand_set*.bmp.
    // Requires globals.h, prefs_keys.h, declarations.h (included before).

    // WLAN-Zugangsdaten (optional) stehen nur verschluesselt in settings.txt:
    // Zeile "wifienc", AES-256-GCM mit dem internen Schluessel BACKUP_WIFI_KEY
    // (config.h) - gleich in jeder uhr4-Firmware, damit die Sicherung auch
    // auf einer anderen Uhr wiederhergestellt werden kann.

    // WiFi credentials (optional) are stored in settings.txt only encrypted:
    // line "wifienc", AES-256-GCM with the internal key BACKUP_WIFI_KEY
    // (config.h) - the same in every uhr4 firmware, so the backup can also be
    // restored on another clock.

#include <nvs.h>
#include <mbedtls/gcm.h>
#include <esp_random.h>

#define BACKUP_SETTINGS_NAME   "settings.txt"
#define BACKUP_SETTINGS_MAGIC  "# uhr4-backup"
#define BACKUP_FORMAT_VERSION  2            // 2: WLAN verschluesselt (wifienc), 1: WLAN im Klartext (nur noch lesen)
                                            // 2: WiFi encrypted (wifienc), 1: WiFi in plain text (read only)
#define BACKUP_SETTINGS_MAX    (128 * 1024) // Obergrenze fuer settings.txt beim Wiederherstellen
                                            // upper limit for settings.txt when restoring
#define BACKUP_WIFI_MAX        4096         // Obergrenze entschluesselte WLAN-Daten
                                            // upper limit decrypted WiFi data


    // Schluessel, die NIE gesichert/zurueckgeschrieben werden: geraete- bzw.
    // firmwarespezifische Kennungen. Eine alte "version" wuerde z.B. beim
    // naechsten Start einen Versionswechsel vortaeuschen (Logs geloescht).

    // Keys that are NEVER backed up/restored: device or firmware specific
    // markers. An old "version" would e.g. fake a version change on the next
    // boot (logs deleted).

    // Ausserdem veraltete Schluessel frueherer Firmware (isObsoletePrefKey(),
    // werden beim Start ohnehin geloescht) - "ssid"/"pass" enthielten das WLAN
    // im Klartext und landeten sonst auch ohne WLAN-Haken in der Sicherung.

    // Also obsolete keys of earlier firmware (isObsoletePrefKey(), deleted at
    // boot anyway) - "ssid"/"pass" held the WiFi in plain text and would
    // otherwise end up in the backup even without the WiFi box.

    bool isBackupExcludedKey(const String& key) {
        return key == PK_VERSION || key == PK_FIRST_START || key == PK_MIGRATIONS_DONE ||
               key == PK_LOG_FILE_NUMBER || key == PK_LAST_RESET_WEEK || isObsoletePrefKey(key);
    }


    // WLAN-Gruppe: nur mit Haken "WLAN-Zugangsdaten einschliessen". Neben SSIDs
    // und Passwoertern auch Hostname, zuletzt genutztes Netz und WLAN an/aus -
    // sonst bekaeme eine zweite Uhr beim Wiederherstellen denselben Hostnamen.
    // SSIDs/Passwoerter am Namensanfang erkannt ("ssid..."/"pass..."), nicht
    // nur ssid1..ssidN - so rutscht auch ein kuenftig anders nummerierter oder
    // alter Schluessel nie im Klartext in die Sicherung.

    // WiFi group: only with the "include WiFi credentials" box ticked. Besides
    // SSIDs and passwords also hostname, last used network and WiFi on/off -
    // otherwise a second clock would get the same hostname on restore.
    // SSIDs/passwords are recognized by the name prefix ("ssid..."/"pass..."),
    // not just ssid1..ssidN - so a differently numbered future key or an old
    // one never slips into the backup in plain text.

    bool isBackupWifiKey(const String& key) {
        if (key == PK_HOSTNAME || key == PK_LAST_WLAN || key == PK_WIFI_ACTIVE) return true;
        return key.startsWith("ssid") || key.startsWith("pass");
    }


    // Hardware-Gruppe: haengt an Display und Verdrahtung der jeweiligen Uhr.
    // Wird gesichert (der Displaytyp dient zur Pruefung beim Wiederherstellen),
    // aber nie zurueckgeschrieben - eine andere Uhr behaelt Displaytyp,
    // Rotation, Backlight, Lichtsensor-Richtung und Touch.

    // Hardware group: depends on the display and wiring of the individual
    // clock. Backed up (the display type is used for the check on restore),
    // but never written back - another clock keeps its display type,
    // rotation, backlight, light sensor direction and touch.

    bool isBackupHardwareKey(const String& key) {
        return key == PK_DISPLAY_TYPE || key == PK_TFT_ROTATION1 || key == PK_TFT_ROTATION2 ||
               key == PK_TFT_ROTATION_LEGACY || key == PK_USE_BACKLIGHT || key == PK_ADC_INVERTED ||
               key == PK_USE_TOUCH;
    }


    // Helligkeitswerte, deren sinnvolle Werte vom Backlight-Modus abhaengen
    // (siehe putBrightnessDefaults() in display.h) - beim Wiederherstellen auf
    // eine Uhr mit anderem Backlight-Modus bleiben ihre eigenen Werte.

    // Brightness values whose sensible values depend on the backlight mode
    // (see putBrightnessDefaults() in display.h) - when restoring onto a clock
    // with a different backlight mode, its own values stay.

    bool isBackupBacklightDependentKey(const String& key) {
        return key == PK_MIN_BRIGHTNESS || key == PK_LOW_THRESHOLD || key == PK_HIGH_THRESHOLD;
    }


    // Text-Escaping fuer settings.txt (Tab-getrennt, eine Zeile pro Schluessel)
    // Text escaping for settings.txt (tab-separated, one line per key)

    String backupEscape(const String& s) {
        String out;
        out.reserve(s.length() + 8);
        for (size_t i = 0; i < s.length(); i++) {
            char c = s[i];
            if (c == '\\') out += "\\\\";
            else if (c == '\t') out += "\\t";
            else if (c == '\n') out += "\\n";
            else if (c == '\r') out += "\\r";
            else out += c;
        }
        return out;
    }

    String backupUnescape(const String& s) {
        String out;
        out.reserve(s.length());
        for (size_t i = 0; i < s.length(); i++) {
            char c = s[i];
            if (c == '\\' && i + 1 < s.length()) {
                char n = s[++i];
                if (n == 't') out += '\t';
                else if (n == 'n') out += '\n';
                else if (n == 'r') out += '\r';
                else out += n;
            }
            else {
                out += c;
            }
        }
        return out;
    }


    // Hex-Kodierung fuer Blobs und die verschluesselten WLAN-Daten.
    // backupUnhex() liefert die Byte-Anzahl oder -1 bei Fehler/zu lang.
    // Hex encoding for blobs and the encrypted WiFi data. backupUnhex()
    // returns the byte count or -1 on error/too long.

    String backupHex(const uint8_t* data, size_t len) {
        static const char hex[] = "0123456789abcdef";
        String out;
        out.reserve(len * 2);
        for (size_t i = 0; i < len; i++) {
            out += hex[data[i] >> 4];
            out += hex[data[i] & 0x0F];
        }
        return out;
    }

    int backupHexNibble(char c) {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    }

    int backupUnhex(const String& hex, uint8_t* out, size_t maxLen) {
        if (hex.length() % 2) return -1;
        size_t len = hex.length() / 2;
        if (len > maxLen) return -1;
        for (size_t i = 0; i < len; i++) {
            int hi = backupHexNibble(hex[2 * i]);
            int lo = backupHexNibble(hex[2 * i + 1]);
            if (hi < 0 || lo < 0) return -1;
            out[i] = (uint8_t)((hi << 4) | lo);
        }
        return (int)len;
    }


    // Ueberschreibt einen String mit Nullen (Passwoerter, Klartext-WLAN-Daten)
    // Overwrites a string with zeros (passwords, plain-text WiFi data)

    void backupWipe(String& s) {
        for (size_t i = 0; i < s.length(); i++) s[i] = 0;
        s = "";
    }


    // Ruft fn fuer jede nichtleere Zeile von text auf (ohne Zeilenliste im
    // RAM - settings.txt kann mit vielen Presets einige 10 KB gross sein).
    // Ein '\r' am Zeilenende faellt weg (am PC mit CRLF gespeicherte Datei).
    // Calls fn for every non-empty line of text (without a line list in RAM -
    // settings.txt can be a few 10 KB with many presets). A '\r' at the line
    // end is dropped (file saved with CRLF on a PC).

    template <typename F>
    void forEachBackupLine(const String& text, F fn) {
        int start = 0;
        int n = 0;
        while (start < (int)text.length()) {
            int nl = text.indexOf('\n', start);
            if (nl < 0) nl = text.length();
            int end = (nl > start && text[nl - 1] == '\r') ? nl - 1 : nl;
            if (end > start) fn(text.substring(start, end));
            start = nl + 1;
            if ((++n & 31) == 0) yield();
        }
    }


    // Verschluesselung der WLAN-Daten mit dem internen Schluessel. Format der
    // Zeile "wifienc": "k1:<IV 12 B>:<Tag 16 B>:<Daten>" (alles Hex), IV je
    // Sicherung zufaellig. Der GCM-Tag erkennt eine veraenderte Datei ebenso
    // wie eine Firmware mit anderem BACKUP_WIFI_KEY.

    // Encryption of the WiFi data with the internal key. Format of the
    // "wifienc" line: "k1:<IV 12 B>:<tag 16 B>:<data>" (all hex), IV random
    // per backup. The GCM tag detects a modified file as well as a firmware
    // with a different BACKUP_WIFI_KEY.

    static_assert(sizeof(BACKUP_WIFI_KEY) == 65, "BACKUP_WIFI_KEY (config.h) muss 64 Hex-Zeichen haben / must have 64 hex characters");

    bool backupWifiKey(uint8_t* key) {
        return backupUnhex(String(BACKUP_WIFI_KEY), key, 32) == 32;
    }

    String backupEncrypt(const String& plain) {
        uint8_t iv[12], tag[16], key[32];
        esp_fill_random(iv, sizeof(iv));
        if (!backupWifiKey(key)) return "";

        size_t len = plain.length();
        uint8_t* cipher = (uint8_t*)malloc(len + 1);
        if (!cipher) {
            memset(key, 0, sizeof(key));
            return "";
        }
        mbedtls_gcm_context gcm;
        mbedtls_gcm_init(&gcm);
        int rc = mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, key, 256);
        if (rc == 0) {
            rc = mbedtls_gcm_crypt_and_tag(&gcm, MBEDTLS_GCM_ENCRYPT, len, iv, sizeof(iv), nullptr, 0,
                (const unsigned char*)plain.c_str(), cipher, sizeof(tag), tag);
        }
        mbedtls_gcm_free(&gcm);
        memset(key, 0, sizeof(key));

        String out;
        if (rc == 0) {
            out = "k1:" + backupHex(iv, sizeof(iv)) + ":" + backupHex(tag, sizeof(tag)) + ":" + backupHex(cipher, len);
        }
        free(cipher);
        return out;
    }

    bool backupDecrypt(const String& blob, String& plain, String& error) {
        String field[4];
        int start = 0;
        for (int i = 0; i < 4; i++) {
            int sep = (i < 3) ? blob.indexOf(':', start) : (int)blob.length();
            if (sep < 0) {
                error = "encrypted WiFi data damaged";
                return false;
            }
            field[i] = blob.substring(start, sep);
            start = sep + 1;
        }
        if (field[0] != "k1") {
            error = "unsupported WiFi encryption " + field[0];
            return false;
        }
        uint8_t iv[12], tag[16], key[32];
        size_t len = field[3].length() / 2;
        if (backupUnhex(field[1], iv, sizeof(iv)) != (int)sizeof(iv) ||
            backupUnhex(field[2], tag, sizeof(tag)) != (int)sizeof(tag) ||
            len > BACKUP_WIFI_MAX) {
            error = "encrypted WiFi data damaged";
            return false;
        }

        uint8_t* cipher = (uint8_t*)malloc(len + 1);
        uint8_t* clear = (uint8_t*)malloc(len + 1);
        bool ok = cipher && clear && backupUnhex(field[3], cipher, len) == (int)len && backupWifiKey(key);
        if (!ok) error = "encrypted WiFi data damaged";
        if (ok) {
            mbedtls_gcm_context gcm;
            mbedtls_gcm_init(&gcm);
            int rc = mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, key, 256);
            if (rc == 0) rc = mbedtls_gcm_auth_decrypt(&gcm, len, iv, sizeof(iv), nullptr, 0, tag, sizeof(tag), cipher, clear);
            mbedtls_gcm_free(&gcm);
            if (rc != 0) {
                ok = false;
                error = "WiFi data could not be decrypted (file modified or from a firmware with a different key)";
            }
        }
        memset(key, 0, sizeof(key));

        plain = "";
        if (ok) plain.concat((const char*)clear, len);
        if (clear) {
            memset(clear, 0, len);
            free(clear);
        }
        free(cipher);
        return ok;
    }


    // Baut settings.txt: Kopfzeilen, dann "typ<TAB>schluessel<TAB>wert" je
    // NVS-Eintrag. Typ bestimmt beim Wiederherstellen die put-Funktion; bool
    // liegt im NVS als u8, float (putFloat) als blob (hex).

    // Builds settings.txt: header lines, then "type<TAB>key<TAB>value" per NVS
    // entry. The type determines the put function on restore; bool is stored
    // in NVS as u8, float (putFloat) as a blob (hex).
    // WLAN-Gruppe (nur mit includeWifi): dieselben Zeilen, aber gesammelt
    // mit dem internen Schluessel verschluesselt als eine Zeile "wifienc". Leerer
    // Rueckgabewert = Verschluesselung fehlgeschlagen.
    // WiFi group (only with includeWifi): the same lines, but collected and
    // encrypted with the internal key as one "wifienc" line. Empty return value =
    // encryption failed.

    String buildBackupSettings(bool includeWifi) {
        String out = String(BACKUP_SETTINGS_MAGIC) + "\n";
        out += "format\t" + String(BACKUP_FORMAT_VERSION) + "\n";
        out += "build\t" + String(version) + "\n";
        out += "wifi\t" + String(includeWifi ? 1 : 0) + "\n";
        String wifiPlain;

        nvs_iterator_t it = nullptr;
        esp_err_t res = nvs_entry_find("nvs", "clock", NVS_TYPE_ANY, &it);
        int count = 0;
        while (res == ESP_OK) {
            nvs_entry_info_t info;
            nvs_entry_info(it, &info);
            String key = info.key;

            bool wifiKey = isBackupWifiKey(key);
            if (!isBackupExcludedKey(key) && (includeWifi || !wifiKey)) {
                const char* k = info.key;
                String type, value;
                switch (info.type) {
                    case NVS_TYPE_U8:  type = "u8";  value = String((unsigned int)preferences.getUChar(k)); break;
                    case NVS_TYPE_I8:  type = "i8";  value = String((int)preferences.getChar(k)); break;
                    case NVS_TYPE_U16: type = "u16"; value = String((unsigned int)preferences.getUShort(k)); break;
                    case NVS_TYPE_I16: type = "i16"; value = String((int)preferences.getShort(k)); break;
                    case NVS_TYPE_U32: type = "u32"; value = String(preferences.getUInt(k)); break;
                    case NVS_TYPE_I32: type = "i32"; value = String(preferences.getInt(k)); break;
                    case NVS_TYPE_U64: type = "u64"; value = String((unsigned long long)preferences.getULong64(k)); break;
                    case NVS_TYPE_I64: type = "i64"; value = String((long long)preferences.getLong64(k)); break;
                    case NVS_TYPE_STR: type = "str"; value = backupEscape(preferences.getString(k)); break;
                    case NVS_TYPE_BLOB: {
                        type = "blob";
                        size_t len = preferences.getBytesLength(k);
                        if (len > 0 && len <= 512) {
                            uint8_t buf[512];
                            preferences.getBytes(k, buf, len);
                            value = backupHex(buf, len);
                        }
                        break;
                    }
                    default: break;
                }
                if (type.length() > 0) {
                    String line = type + "\t" + key + "\t" + value + "\n";
                    if (wifiKey) wifiPlain += line;
                    else out += line;
                    backupWipe(value);
                    backupWipe(line);
                    count++;
                }
            }
            res = nvs_entry_next(&it);
            if ((count & 15) == 0) yield();
        }
        nvs_release_iterator(it);

        if (includeWifi) {
            String encrypted = backupEncrypt(wifiPlain);
            backupWipe(wifiPlain);
            if (encrypted.length() == 0) {
                DEBUG_PRINTLN("[Backup] WiFi encryption failed");
                return "";
            }
            out += "wifienc\t" + encrypted + "\n";
        }

        DEBUG_PRINTLN("[Backup] " + String(count) + " settings" + (includeWifi ? " (incl. encrypted WiFi)" : ""));
        return out;
    }


    // Liste der zu sichernden Dateien: nur Zifferblaetter und Zeigersaetze
    // (keine Logs, keine Temp-Dateien). Namen ohne fuehrenden '/'.
    // List of files to back up: only clock faces and hand sets (no logs, no
    // temp files). Names without a leading '/'.

    bool isBackupFileName(const String& name) {
        if (name.length() == 0 || name.length() > 60) return false;
        if (name.indexOf('/') >= 0 || name.indexOf("..") >= 0) return false;
        if (!name.endsWith(".bmp")) return false;
        return name.startsWith("face_") || name.startsWith("hand_set");
    }

    void collectBackupFiles(std::vector<String>& names, std::vector<size_t>& sizes) {
        File root = LittleFS.open("/");
        File file = root.openNextFile();
        while (file) {
            String name = file.name();
            if (name.startsWith("/")) name = name.substring(1);
            if (!file.isDirectory() && isBackupFileName(name)) {
                names.push_back(name);
                sizes.push_back(file.size());
            }
            file = root.openNextFile();
        }
    }


    // TAR (ustar): 512-Byte-Kopf je Datei, Daten auf 512 aufgefuellt, am Ende
    // zwei Nullbloecke.
    // TAR (ustar): 512-byte header per file, data padded to 512, two zero
    // blocks at the end.

    size_t tarPadding(size_t size) {
        return (512 - (size % 512)) % 512;
    }

    void buildTarHeader(uint8_t* h, const String& name, size_t size) {
        memset(h, 0, 512);
        strncpy((char*)h, name.c_str(), 99);
        memcpy(h + 100, "0000644", 7);                           // mode
        memcpy(h + 108, "0000000", 7);                           // uid
        memcpy(h + 116, "0000000", 7);                           // gid
        snprintf((char*)h + 124, 12, "%011lo", (unsigned long)size);
        snprintf((char*)h + 136, 12, "%011lo", (unsigned long)time(nullptr));
        memset(h + 148, ' ', 8);                                 // Pruefsumme zunaechst Leerzeichen
                                                                 // checksum initially spaces
        h[156] = '0';                                            // regulaere Datei / regular file
        memcpy(h + 257, "ustar", 6);
        memcpy(h + 263, "00", 2);
        unsigned long sum = 0;
        for (int i = 0; i < 512; i++) sum += h[i];
        snprintf((char*)h + 148, 7, "%06lo", sum);
        h[155] = ' ';
    }

    bool tarHeaderChecksumOk(const uint8_t* h) {
        char field[9];
        memcpy(field, h + 148, 8);
        field[8] = 0; // strtoul() ueberspringt fuehrende Leerzeichen und endet an NUL/Leerzeichen
                      // strtoul() skips leading spaces and stops at NUL/space
        unsigned long stored = strtoul(field, nullptr, 8);
        unsigned long sum = 0;
        for (int i = 0; i < 512; i++) sum += (i >= 148 && i < 156) ? ' ' : h[i];
        return sum == stored;
    }


    // Sicherung als TAR direkt in die HTTP-Antwort streamen - Dateien werden in
    // 1-KB-Stuecken gelesen, es liegt nie das ganze Archiv im RAM.
    // Stream the backup as TAR straight into the HTTP response - files are read
    // in 1 KB pieces, the whole archive is never held in RAM.

    void streamBackup(bool includeWifi) {
        String settings = buildBackupSettings(includeWifi);
        if (settings.length() == 0) {
            webserver.send(500, "text/plain", "Backup failed: WiFi credentials could not be encrypted");
            return;
        }
        std::vector<String> names;
        std::vector<size_t> sizes;
        collectBackupFiles(names, sizes);

        size_t total = 512 + settings.length() + tarPadding(settings.length()) + 1024;
        for (size_t i = 0; i < names.size(); i++) total += 512 + sizes[i] + tarPadding(sizes[i]);

        char date[16] = "";
        if (timeinfo.tm_year >= 100) strftime(date, sizeof(date), "-%Y%m%d", &timeinfo);
        String fileName = "uhr4-backup-" + String(hostname) + date + ".tar";

        webserver.sendHeader("Content-Disposition", "attachment; filename=" + fileName);
        webserver.sendHeader("Cache-Control", "no-store");
        webserver.setContentLength(total);
        webserver.send(200, "application/x-tar", "");

        static const uint8_t zeros[512] = { 0 };
        uint8_t header[512];

        buildTarHeader(header, BACKUP_SETTINGS_NAME, settings.length());
        webserver.sendContent((const char*)header, 512);
        webserver.sendContent(settings.c_str(), settings.length());
        webserver.sendContent((const char*)zeros, tarPadding(settings.length()));

        uint8_t buf[1024];
        for (size_t i = 0; i < names.size(); i++) {
            File f = LittleFS.open("/" + names[i], "r");
            size_t size = sizes[i];
            buildTarHeader(header, names[i], size);
            webserver.sendContent((const char*)header, 512);
            size_t sent = 0;
            while (sent < size) {
                size_t n = f ? f.read(buf, min(sizeof(buf), size - sent)) : 0;
                if (n == 0) {
                    // Lesefehler: mit Nullen auffuellen, damit die angekuendigte
                    // Laenge stimmt und das Archiv lesbar bleibt.
                    // Read error: pad with zeros so the announced length is right
                    // and the archive stays readable.
                    memset(buf, 0, sizeof(buf));
                    n = min(sizeof(buf), size - sent);
                }
                webserver.sendContent((const char*)buf, n);
                sent += n;
                yield();
            }
            if (f) f.close();
            if (tarPadding(size)) webserver.sendContent((const char*)zeros, tarPadding(size));
        }
        webserver.sendContent((const char*)zeros, 512);
        webserver.sendContent((const char*)zeros, 512);

        DEBUG_PRINTLN("[Backup] Download: " + String(names.size()) + " files, " + String(total) + " bytes (from " + webserver.client().remoteIP().toString() + ")");
    }


    // Prueft settings.txt, BEVOR an der Uhr etwas geaendert wird: Kennung,
    // Format, Displaytyp (muss zu dieser Uhr passen, sonst passen Zifferblaetter
    // und Zeiger nicht), Backlight-Modus (weicht er ab: keepBrightness) und -
    // falls gewuenscht - die WLAN-Daten (Format 2: Entschluesselung mit dem
    // internen Schluessel, Format 1: Klartext). Liefert in applyWifi/
    // keepBrightness/wifiPlain, was applyBackupSettings() spaeter schreibt.

    // Checks settings.txt BEFORE anything is changed on the clock: marker,
    // format, display type (must match this clock, otherwise clock faces and
    // hands don't fit), backlight mode (if it differs: keepBrightness) and -
    // if requested - the WiFi data (format 2: decryption with the internal
    // key, format 1: plain text). Returns in applyWifi/keepBrightness/
    // wifiPlain what applyBackupSettings() writes later.

    bool checkBackupSettings(const String& settings, bool restoreWifi, bool& applyWifi, bool& keepBrightness, String& wifiPlain, String& error) {
        applyWifi = false;
        keepBrightness = false;
        wifiPlain = "";
        if (!settings.startsWith(BACKUP_SETTINGS_MAGIC)) {
            error = "settings.txt is not an uhr4 backup";
            return false;
        }

        // Fehlt ein Schluessel in der Sicherung, galt dort der Standardwert
        // If a key is missing in the backup, the default applied there
        const String typePrefix = "u8\t" + String(PK_DISPLAY_TYPE) + "\t";
        const String backlightPrefix = "u8\t" + String(PK_USE_BACKLIGHT) + "\t";
        int format = 0;
        bool backupHasWifi = false;
        int backupType = DISPLAY_TYPE_DEFAULT;
        int backupBacklight = -1;
        String wifiEnc;
        forEachBackupLine(settings, [&](const String& line) {
            if (line.startsWith("format\t")) format = line.substring(7).toInt();
            else if (line.startsWith("wifi\t")) backupHasWifi = line.substring(5).toInt() == 1;
            else if (line.startsWith("wifienc\t")) wifiEnc = line.substring(8);
            else if (line.startsWith(typePrefix)) backupType = line.substring(typePrefix.length()).toInt();
            else if (line.startsWith(backlightPrefix)) backupBacklight = line.substring(backlightPrefix.length()).toInt() ? 1 : 0;
        });
        if (format < 1 || format > BACKUP_FORMAT_VERSION) {
            error = "unsupported backup format " + String(format);
            return false;
        }
        if (backupType < 0 || backupType >= DISPLAY_TYPE_COUNT) backupType = DISPLAY_TYPE_DEFAULT;
        if (backupType != displayType) {
            const DisplayGeometry& from = DISPLAY_GEOMETRY[backupType];
            error = "the backup is from a " + String(from.name) + " clock (" + String(from.clock) + "x" + String(from.clock) +
                    "), this clock is set to " + String(displayGeom->name) + " (" + String(displayGeom->clock) + "x" +
                    String(displayGeom->clock) + ") - clock faces and hands would not fit";
            return false;
        }
        if (backupBacklight < 0) backupBacklight = DISPLAY_GEOMETRY[backupType].backlightDefault ? 1 : 0;
        keepBrightness = (backupBacklight == 1) != useBacklight;

        if (!restoreWifi) return true;

        if (!backupHasWifi) {
            error = "the backup contains no WiFi credentials - restore it without the WiFi option";
            return false;
        }
        if (format == 1) { // alte Sicherung, WLAN im Klartext / old backup, WiFi in plain text
            applyWifi = true;
            return true;
        }
        if (!backupDecrypt(wifiEnc, wifiPlain, error)) return false;
        applyWifi = true;
        return true;
    }


    // Schluessel, die die Uhr beim Wiederherstellen behaelt (weder entfernen
    // noch ueberschreiben): nie gesicherte, Hardware, WLAN ohne applyWifi,
    // backlight-abhaengige Helligkeit mit keepBrightness.

    // Keys the clock keeps on restore (neither removed nor overwritten):
    // never backed up, hardware, WiFi without applyWifi, backlight-dependent
    // brightness with keepBrightness.

    bool isBackupKeptKey(const String& key, bool applyWifi, bool keepBrightness) {
        return isBackupExcludedKey(key) || isBackupHardwareKey(key) ||
               (!applyWifi && isBackupWifiKey(key)) ||
               (keepBrightness && isBackupBacklightDependentKey(key));
    }


    // Typen der Schluessel, wie sie VOR dem Wiederherstellen auf dieser Uhr
    // liegen - die aktuelle Firmware liest sie mit genau diesem Typ.
    // Types of the keys as they are on this clock BEFORE the restore - the
    // current firmware reads them with exactly this type.

    struct BackupKeyType {
        char key[16];
        nvs_type_t type;
    };

    const char* backupTypeName(nvs_type_t type) {
        switch (type) {
            case NVS_TYPE_U8:  return "u8";
            case NVS_TYPE_I8:  return "i8";
            case NVS_TYPE_U16: return "u16";
            case NVS_TYPE_I16: return "i16";
            case NVS_TYPE_U32: return "u32";
            case NVS_TYPE_I32: return "i32";
            case NVS_TYPE_U64: return "u64";
            case NVS_TYPE_I64: return "i64";
            case NVS_TYPE_STR: return "str";
            case NVS_TYPE_BLOB: return "blob";
            default: return "";
        }
    }

    bool isBackupIntType(const String& type) {
        return type == "u8" || type == "i8" || type == "u16" || type == "i16" ||
               type == "u32" || type == "i32" || type == "u64" || type == "i64";
    }


    // Aeltere Sicherung: hat eine neuere Firmware den Zahlentyp eines
    // Schluessels geaendert (z.B. u8 -> i32), liest sie den alten Typ nicht
    // (Preferences liefert dann still den Standardwert). Deshalb wird eine
    // Ganzzahl in den Typ umgewandelt, den der Schluessel auf dieser Uhr hat -
    // auf den Wertebereich begrenzt. Unbekannte Schluessel behalten ihren Typ.

    // Older backup: if a newer firmware changed the integer type of a key
    // (e.g. u8 -> i32), it does not read the old type (Preferences then
    // silently returns the default). So an integer is converted into the type
    // the key has on this clock - clamped to its value range. Unknown keys
    // keep their type.

    String backupTargetType(const String& key, const String& type, const std::vector<BackupKeyType>& targetTypes) {
        if (!isBackupIntType(type)) return type;
        for (const BackupKeyType& t : targetTypes) {
            if (key == t.key) {
                String target = backupTypeName(t.type);
                return isBackupIntType(target) ? target : type;
            }
        }
        return type;
    }

    long long backupClamp(long long v, long long lo, long long hi) {
        return v < lo ? lo : (v > hi ? hi : v);
    }

    int backupIntBits(const String& type) {
        if (type == "u8" || type == "i8") return 8;
        if (type == "u16" || type == "i16") return 16;
        if (type == "u32" || type == "i32") return 32;
        if (type == "u64" || type == "i64") return 64;
        return 0;
    }


    // Schreibt eine Zeile "typ<TAB>schluessel<TAB>wert" in den NVS. Nie:
    // isBackupKeptKey(); mit onlyWifi ausschliesslich WLAN-Schluessel (Zeilen
    // aus dem entschluesselten Block).

    // Writes a line "type<TAB>key<TAB>value" to NVS. Never:
    // isBackupKeptKey(); with onlyWifi exclusively WiFi keys (lines from the
    // decrypted block).

    bool writeBackupLine(const String& line, bool allowWifi, bool keepBrightness, bool onlyWifi, const std::vector<BackupKeyType>& targetTypes) {
        int t1 = line.indexOf('\t');
        int t2 = (t1 < 0) ? -1 : line.indexOf('\t', t1 + 1);
        if (t1 < 0 || t2 < 0) return false; // Kopfzeilen (format, build, wifi, wifienc) / header lines
        String backupType = line.substring(0, t1);
        String key = line.substring(t1 + 1, t2);
        String value = line.substring(t2 + 1);
        if (key.length() == 0 || key.length() > 15) return false; // NVS-Schluessel max. 15 Zeichen
                                                                   // NVS keys max. 15 chars
        if (isBackupKeptKey(key, allowWifi, keepBrightness)) return false;
        bool wifiKey = isBackupWifiKey(key);
        if (!wifiKey && onlyWifi) return false;
        const char* k = key.c_str();
        String type = backupTargetType(key, backupType, targetTypes);
        long long v = strtoll(value.c_str(), nullptr, 10);

        // Gleiche Breite (auch signed <-> unsigned, z.B. Farbe i32 -> u32):
        // Bitmuster uebernehmen. Andere Breite: auf den Wertebereich begrenzen.
        // Same width (also signed <-> unsigned, e.g. color i32 -> u32): keep
        // the bit pattern. Different width: clamp to the value range.
        bool sameWidth = backupIntBits(type) == backupIntBits(backupType);
        auto fit = [&](long long lo, long long hi) { return sameWidth ? v : backupClamp(v, lo, hi); };

        if (type == "u8") preferences.putUChar(k, (uint8_t)fit(0, UINT8_MAX));
        else if (type == "i8") preferences.putChar(k, (int8_t)fit(INT8_MIN, INT8_MAX));
        else if (type == "u16") preferences.putUShort(k, (uint16_t)fit(0, UINT16_MAX));
        else if (type == "i16") preferences.putShort(k, (int16_t)fit(INT16_MIN, INT16_MAX));
        else if (type == "u32") preferences.putUInt(k, (uint32_t)fit(0, UINT32_MAX));
        else if (type == "i32") preferences.putInt(k, (int32_t)fit(INT32_MIN, INT32_MAX));
        else if (type == "u64") preferences.putULong64(k, (backupType == "u64") ? strtoull(value.c_str(), nullptr, 10) : (uint64_t)fit(0, INT64_MAX));
        else if (type == "i64") preferences.putLong64(k, v);
        else if (type == "str") {
            String text = backupUnescape(value);
            preferences.putString(k, text);
            backupWipe(text);
        }
        else if (type == "blob") {
            uint8_t buf[512];
            int len = backupUnhex(value, buf, sizeof(buf));
            if (len <= 0) return false;
            preferences.putBytes(k, buf, len);
        }
        else return false;
        if (wifiKey) backupWipe(value);
        return true;
    }


    // settings.txt anwenden (nach checkBackupSettings()): zuerst alle
    // Schluessel entfernen, die die Sicherung ersetzt (so verschwinden auch
    // z.B. ueberzaehlige Presets), dann die gesicherten Werte mit ihrem Typ
    // zurueckschreiben. Was isBackupKeptKey() nennt, bleibt unberuehrt - u.a.
    // Hardware und (ohne applyWifi) WLAN und Hostname.

    // Apply settings.txt (after checkBackupSettings()): first remove all keys
    // the backup replaces (so e.g. surplus presets disappear too), then write
    // the backed-up values back with their type. What isBackupKeptKey() names
    // stays untouched - among others hardware and (without applyWifi) WiFi
    // and hostname.

    void applyBackupSettings(const String& settings, bool applyWifi, bool keepBrightness, const String& wifiPlain) {

        // 1) Zu ersetzende Schluessel entfernen (Liste erst sammeln - waehrend
        // der Aufzaehlung zu loeschen wuerde den Iterator ungueltig machen)
        // 1) Remove the keys to be replaced (collect the list first - deleting
        // while enumerating would invalidate the iterator)
        // Dabei die aktuellen Typen merken (siehe backupTargetType()).
        // Remember the current types along the way (see backupTargetType()).
        std::vector<BackupKeyType> targetTypes;
        nvs_iterator_t it = nullptr;
        esp_err_t res = nvs_entry_find("nvs", "clock", NVS_TYPE_ANY, &it);
        while (res == ESP_OK) {
            nvs_entry_info_t info;
            nvs_entry_info(it, &info);
            BackupKeyType t;
            strlcpy(t.key, info.key, sizeof(t.key));
            t.type = info.type;
            targetTypes.push_back(t);
            res = nvs_entry_next(&it);
        }
        nvs_release_iterator(it);
        for (size_t i = 0; i < targetTypes.size(); i++) {
            if (!isBackupKeptKey(targetTypes[i].key, applyWifi, keepBrightness)) preferences.remove(targetTypes[i].key);
            if ((i & 15) == 0) yield();
        }

        // 2) Gesicherte Werte zurueckschreiben - WLAN im Klartext nur aus
        // alten Sicherungen (Format 1), sonst aus dem entschluesselten Block
        // 2) Write the backed-up values back - WiFi in plain text only from
        // old backups (format 1), otherwise from the decrypted block
        int restored = 0;
        forEachBackupLine(settings, [&](const String& line) {
            if (writeBackupLine(line, applyWifi, keepBrightness, false, targetTypes)) restored++;
        });
        if (applyWifi) {
            forEachBackupLine(wifiPlain, [&](const String& line) {
                if (writeBackupLine(line, true, keepBrightness, true, targetTypes)) restored++;
            });
        }

        // 3) Datei-Migration beim naechsten Start erneut laufen lassen (setup()):
        // eine aeltere Sicherung kann Zifferblaetter/Zeiger im alten Format
        // (unkomprimiert, ohne Eckenmaskierung) enthalten. Die Schritte
        // erkennen bereits umgestellte Dateien selbst und ueberspringen sie.
        // 3) Run the file migration again on the next boot (setup()): an older
        // backup may contain faces/hands in the old format (uncompressed,
        // without corner masking). The steps detect already converted files
        // themselves and skip them.
        preferences.remove(PK_MIGRATIONS_DONE);

        DEBUG_PRINTLN("[Backup] Restored " + String(restored) + " settings" + (applyWifi ? " (incl. WiFi)" : " (WiFi kept)") +
                      (keepBrightness ? ", brightness kept (other backlight mode)" : ""));
    }


    // Wiederherstellen: Zustand ueber die Upload-Stuecke hinweg

    // Restore: state across the upload chunks

    struct BackupRestoreState {
        enum Phase { HEADER, DATA, PAD, END, FAILED } phase = HEADER;
        uint8_t header[512];
        size_t headerFill = 0;
        size_t remaining = 0;
        size_t padding = 0;
        bool firstHeader = true;   // der erste Kopf muss settings.txt sein
                                   // the first header must be settings.txt
        bool toSettings = false;
        String settings;
        File out;
        std::vector<String> restoredFiles;
        String error;

        // Formularfelder (stehen im Formular VOR der Datei, damit sie beim
        // Upload-Start schon vorliegen), Ergebnis der Pruefung von settings.txt
        // Form fields (placed BEFORE the file in the form so they are already
        // available at upload start), result of checking settings.txt
        bool restoreWifi = false;
        bool settingsChecked = false; // ab hier wurde an der Uhr etwas geaendert
                                      // from here on something was changed on the clock
        bool applyWifi = false;
        bool keepBrightness = false;  // anderer Backlight-Modus / other backlight mode
        String wifiPlain;             // entschluesselte WLAN-Zeilen / decrypted WiFi lines

        ~BackupRestoreState() {
            backupWipe(wifiPlain);
        }
    };
    BackupRestoreState* backupRestore = nullptr;

    void backupRestoreFail(const String& error) {
        if (!backupRestore) return;
        if (backupRestore->out) backupRestore->out.close();
        backupRestore->phase = BackupRestoreState::FAILED;
        backupRestore->error = error;
        DEBUG_PRINTLN("[Backup] Restore failed: " + error);
    }

    // Loescht alle Zifferblaetter und Zeigersaetze - erst NACHDEM settings.txt
    // vollstaendig geprueft ist (siehe backupRestoreSettingsDone()), damit eine
    // falsch gewaehlte Datei oder nicht entschluesselbare WLAN-Daten nichts zerstoert.
    // Deletes all clock faces and hand sets - only AFTER settings.txt has been
    // fully checked (see backupRestoreSettingsDone()), so a wrongly chosen file
    // or undecryptable WiFi data doesn't destroy anything.
    void deleteAllFacesAndHands() {
        std::vector<String> toDelete;
        std::vector<size_t> unused;
        collectBackupFiles(toDelete, unused);
        for (const String& name : toDelete) LittleFS.remove("/" + name);
        DEBUG_PRINTLN("[Backup] Removed " + String(toDelete.size()) + " existing faces/hand sets");
    }

    // settings.txt ist vollstaendig: pruefen (inkl. WLAN-Entschluesselung), erst dann
    // die vorhandenen Zifferblaetter/Zeiger loeschen - ab hier wird geaendert.
    // settings.txt is complete: check it (incl. WiFi decryption), only then
    // delete the existing faces/hands - from here on things get changed.
    bool backupRestoreSettingsDone() {
        BackupRestoreState& s = *backupRestore;
        s.toSettings = false;
        String error;
        bool ok = checkBackupSettings(s.settings, s.restoreWifi, s.applyWifi, s.keepBrightness, s.wifiPlain, error);
        if (!ok) {
            backupRestoreFail(error);
            return false;
        }
        deleteAllFacesAndHands();
        s.settingsChecked = true;
        return true;
    }

    void backupRestoreHeader() {
        BackupRestoreState& s = *backupRestore;
        bool allZero = true;
        for (int i = 0; i < 512 && allZero; i++) allZero = (s.header[i] == 0);
        if (allZero) {
            s.phase = s.firstHeader ? BackupRestoreState::FAILED : BackupRestoreState::END;
            if (s.firstHeader) backupRestoreFail("empty archive");
            return;
        }
        if (!tarHeaderChecksumOk(s.header)) {
            backupRestoreFail("not a valid backup file (TAR header checksum)");
            return;
        }

        char nameBuf[101];
        memcpy(nameBuf, s.header, 100);
        nameBuf[100] = 0;
        String name = nameBuf;
        while (name.startsWith("./")) name = name.substring(2);
        while (name.startsWith("/")) name = name.substring(1);

        char sizeBuf[13];
        memcpy(sizeBuf, s.header + 124, 12);
        sizeBuf[12] = 0;
        size_t size = strtoul(sizeBuf, nullptr, 8);
        char type = (char)s.header[156];

        s.remaining = size;
        s.padding = tarPadding(size);
        s.toSettings = false;

        if (s.firstHeader) {
            s.firstHeader = false;
            if (name != BACKUP_SETTINGS_NAME || size == 0 || size > BACKUP_SETTINGS_MAX) {
                backupRestoreFail("not an uhr4 backup (settings.txt missing)");
                return;
            }
            // settings.txt sammeln - geprueft und vorhandene Dateien geloescht
            // wird erst, wenn sie vollstaendig ist (backupRestoreSettingsDone())
            // Collect settings.txt - checking and deleting existing files only
            // happens once it is complete (backupRestoreSettingsDone())
            s.toSettings = true;
            s.settings.reserve(size);
        }
        else if ((type == '0' || type == 0) && isBackupFileName(name)) {
            s.out = LittleFS.open("/" + name, "w");
            if (!s.out) {
                backupRestoreFail("cannot write " + name);
                return;
            }
            s.restoredFiles.push_back(name);
        }
        // Alles andere (Verzeichnisse, fremde Dateien) wird uebersprungen
        // Everything else (directories, foreign files) is skipped

        s.phase = (s.remaining > 0) ? BackupRestoreState::DATA
                : (s.padding > 0) ? BackupRestoreState::PAD
                : BackupRestoreState::HEADER;
        if (s.phase != BackupRestoreState::DATA && s.out) s.out.close();
    }

    void backupRestoreConsume(const uint8_t* data, size_t len) {
        BackupRestoreState& s = *backupRestore;
        while (len > 0 && s.phase != BackupRestoreState::FAILED && s.phase != BackupRestoreState::END) {
            if (s.phase == BackupRestoreState::HEADER) {
                size_t n = min(len, (size_t)512 - s.headerFill);
                memcpy(s.header + s.headerFill, data, n);
                s.headerFill += n; data += n; len -= n;
                if (s.headerFill == 512) {
                    s.headerFill = 0;
                    backupRestoreHeader();
                }
            }
            else if (s.phase == BackupRestoreState::DATA) {
                size_t n = min(len, s.remaining);
                if (s.toSettings) {
                    s.settings.concat((const char*)data, n);
                }
                else if (s.out) {
                    if (s.out.write(data, n) != n) {
                        backupRestoreFail("clock storage full while restoring files");
                        return;
                    }
                }
                s.remaining -= n; data += n; len -= n;
                if (s.remaining == 0) {
                    if (s.out) s.out.close();
                    s.phase = (s.padding > 0) ? BackupRestoreState::PAD : BackupRestoreState::HEADER;
                    if (s.toSettings && !backupRestoreSettingsDone()) return;
                }
            }
            else if (s.phase == BackupRestoreState::PAD) {
                size_t n = min(len, s.padding);
                s.padding -= n; data += n; len -= n;
                if (s.padding == 0) s.phase = BackupRestoreState::HEADER;
            }
        }
    }


    // Upload-Rueckruf fuer /backup/restore - verarbeitet das Archiv beim
    // Eintreffen, schreibt Dateien direkt und wendet am Ende settings.txt an.
    // Nur aus einem privaten Netz (Pruefung schon beim Start des Uploads,
    // der Handler danach kaeme zu spaet - geschrieben wird waehrenddessen).

    // Upload callback for /backup/restore - processes the archive as it
    // arrives, writes files directly and applies settings.txt at the end.
    // Only from a private network (checked right at upload start, the handler
    // afterwards would come too late - writing happens during the upload).

    void handleBackupRestoreUpload() {
        HTTPUpload& upload = webserver.upload();

        if (upload.status == UPLOAD_FILE_START) {
            delete backupRestore;
            backupRestore = new (std::nothrow) BackupRestoreState();
            if (!backupRestore) return;
            if (!isPrivateNetworkIp(webserver.client().remoteIP())) {
                backupRestoreFail("only allowed from a private network");
                return;
            }
            // Formularfelder vor der Datei - liegen beim Upload-Start schon vor
            // Form fields before the file - already available at upload start
            backupRestore->restoreWifi = webserver.arg("restoreWifi") == "1";
            DEBUG_PRINTLN("[Backup] Restore started: " + upload.filename + (backupRestore->restoreWifi ? " (incl. WiFi)" : "") + " (from " + webserver.client().remoteIP().toString() + ")");
        }
        else if (upload.status == UPLOAD_FILE_WRITE) {
            if (backupRestore) backupRestoreConsume(upload.buf, upload.currentSize);
        }
        else if (upload.status == UPLOAD_FILE_END) {
            if (!backupRestore || backupRestore->phase == BackupRestoreState::FAILED) return;
            if (backupRestore->out) backupRestore->out.close();
            if (!backupRestore->settingsChecked) {
                backupRestoreFail("backup contains no settings");
                return;
            }
            applyBackupSettings(backupRestore->settings, backupRestore->applyWifi, backupRestore->keepBrightness, backupRestore->wifiPlain);
            backupWipe(backupRestore->wifiPlain);
            backupRestore->phase = BackupRestoreState::END;
            DEBUG_PRINTLN("[Backup] Restore complete: " + String(backupRestore->restoredFiles.size()) + " files");
        }
        else if (upload.status == UPLOAD_FILE_ABORTED) {
            backupRestoreFail("upload aborted");
        }
    }
