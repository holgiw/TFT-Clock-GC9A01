#pragma once
    // Komplettsicherung: alle Einstellungen (NVS-Namespace "clock", vollstaendig
    // per nvs_entry_find() aufgezaehlt - auch kuenftige Schluessel ohne Liste)
    // plus alle hochgeladenen Zifferblaetter/Zeigersaetze als TAR-Archiv.
    // TAR, weil es sich ohne Kompression Datei fuer Datei streamen laesst und
    // am PC mit jedem Packprogramm (z.B. 7-Zip) einsehbar ist.
    // Inhalt: settings.txt (immer zuerst) + face_*.bmp + hand_set*.bmp + strip_*.bmp + font_* + stripfont_*.vlw,
    // zur Fehlersuche am Ende status.txt und alle log_*.log - die werden nie wiederhergestellt.
    // Benoetigt globals.h, prefs_keys.h, declarations.h (vorher eingebunden).

    // Full backup: all settings (NVS namespace "clock", fully enumerated via
    // nvs_entry_find() - future keys too, without a list) plus all uploaded
    // clock faces/hand sets as a TAR archive. TAR because it can be streamed
    // file by file without compression and can be inspected on a PC with any
    // archiver (e.g. 7-Zip).
    // Contents: settings.txt (always first) + face_*.bmp + hand_set*.bmp + strip_*.bmp + font_* + stripfont_*.vlw,
    // for troubleshooting status.txt and all log_*.log at the end - those are never restored.
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
#define BACKUP_TMP_PATH        "/backup_restore.tmp" // BMP aus der Sicherung, bis es RLE-komprimiert abgelegt ist
                                                     // BMP from the backup until it is stored RLE-compressed
#define BACKUP_SETTINGS_MAGIC  "# uhr4-backup"
#define BACKUP_FORMAT_VERSION  2            // 2: WLAN verschluesselt (wifienc), 1: WLAN im Klartext (nur noch lesen)
                                            // 2: WiFi encrypted (wifienc), 1: WiFi in plain text (read only)
#define BACKUP_SETTINGS_MAX    (128 * 1024) // Obergrenze fuer settings.txt beim Wiederherstellen
                                            // upper limit for settings.txt when restoring
#define BACKUP_WIFI_MAX        4096         // Obergrenze entschluesselte WLAN-Daten
                                            // upper limit decrypted WiFi data


    // Nie gesicherte/zurueckgeschriebene Schluessel: geraete-/firmwarespezifische Kennungen (eine alte
    // "version" taeuschte einen Versionswechsel vor) und veraltete Schluessel (isObsoletePrefKey()) -
    // "ssid"/"pass" enthielten das WLAN im Klartext.

    // Keys never backed up/restored: device/firmware specific markers (an old "version" would fake a version
    // change) and obsolete keys (isObsoletePrefKey()) - "ssid"/"pass" held the WiFi in plain text.

    bool isBackupExcludedKey(const String& key) {
        return key == PK_VERSION || key == PK_FIRST_START || key == PK_MIGRATIONS_DONE ||
               key == PK_LOG_FILE_NUMBER || key == PK_LAST_RESET_WEEK || isObsoletePrefKey(key);
    }


    // WLAN-Gruppe (nur mit Haken "WLAN-Zugangsdaten einschliessen"): SSIDs, Passwoerter, Hostname, letztes
    // Netz, WLAN an/aus - sonst bekaeme eine zweite Uhr denselben Hostnamen. Erkannt am Namensanfang
    // "ssid..."/"pass...", so rutschen auch alte oder kuenftige Schluessel nie im Klartext in die Sicherung.

    // WiFi group (only with "include WiFi credentials" ticked): SSIDs, passwords, hostname, last network,
    // WiFi on/off - otherwise a second clock would get the same hostname. Recognized by the name prefix
    // "ssid..."/"pass...", so old or future keys never slip into the backup in plain text.

    bool isBackupWifiKey(const String& key) {
        if (key == PK_HOSTNAME || key == PK_LAST_WLAN || key == PK_WIFI_ACTIVE) return true;
        return key.startsWith("ssid") || key.startsWith("pass");
    }


    // Hardware-Gruppe (Display und Verdrahtung der jeweiligen Uhr): wird gesichert (Displaytyp zur Pruefung
    // beim Wiederherstellen), aber nie zurueckgeschrieben - eine andere Uhr behaelt Displaytyp, Rotation,
    // Backlight, Lichtsensor.

    // Hardware group (display and wiring of the individual clock): backed up (display type for the check on
    // restore), but never written back - another clock keeps display type, rotation, backlight, light sensor.

    bool isBackupHardwareKey(const String& key) {
        return key == PK_DISPLAY_TYPE || key == PK_TFT_ROTATION1 || key == PK_TFT_ROTATION2 ||
               key == PK_TFT_ROTATION_LEGACY || key == PK_USE_BACKLIGHT || key == PK_ADC_INVERTED;
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


    // WLAN-Daten mit dem internen Schluessel verschluesseln. Zeile "wifienc": "k1:<IV 12 B>:<Tag 16
    // B>:<Daten>" (Hex), IV je Sicherung zufaellig. Der GCM-Tag erkennt eine veraenderte Datei und eine
    // Firmware mit anderem BACKUP_WIFI_KEY.

    // Encrypt the WiFi data with the internal key. Line "wifienc": "k1:<IV 12 B>:<tag 16 B>:<data>" (hex),
    // random IV per backup. The GCM tag detects a modified file and a firmware with a different
    // BACKUP_WIFI_KEY.

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


    // Baut settings.txt: Kopfzeilen, dann "typ<TAB>schluessel<TAB>wert" je NVS-Eintrag (bool als u8, float
    // als Hex-Blob). WLAN-Gruppe (nur mit includeWifi) verschluesselt als eine Zeile "wifienc"; leer =
    // Verschluesselung fehlgeschlagen.

    // Builds settings.txt: header lines, then "type<TAB>key<TAB>value" per NVS entry (bool as u8, float as
    // hex blob). WiFi group (only with includeWifi) encrypted as one "wifienc" line; empty = encryption
    // failed.

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
        if (name.startsWith("font_")) {
            return name.endsWith(".ttf") || name.endsWith(".otf") || name.endsWith(".woff") || name.endsWith(".woff2");
        }
        if (name.startsWith("stripfont_")) return name.endsWith(".vlw");
        if (!name.endsWith(".bmp")) return false;
        return name.startsWith("face_") || name.startsWith("hand_set") || name.startsWith("strip_");
    }

    // Logdateien fuer die Sicherung (nur zur Fehlersuche, isBackupFileName() laesst sie beim Wiederherstellen aus)
    // Log files for the backup (troubleshooting only, isBackupFileName() skips them on restore)

    void collectLogFiles(std::vector<String>& names, std::vector<size_t>& sizes) {
        File root = LittleFS.open("/");
        File file = root.openNextFile();
        while (file) {
            String name = file.name();
            if (name.startsWith("/")) name = name.substring(1);
            if (!file.isDirectory() && name.endsWith(".log") && name.indexOf('/') < 0 && name.length() <= 60) {
                names.push_back(name);
                sizes.push_back(file.size());
            }
            file = root.openNextFile();
        }
    }


    // HTML der Statusseite als lesbarer Text: <li> wird "- ", <h3> eine Zwischenzeile, Zeilenende bei </li>, <br>
    // und </ul>, Entities (&auml;, &deg;, &#9888; ...) werden zu UTF-8 (appendUtf8()).

    // Status page HTML as readable text: <li> becomes "- ", <h3> a heading line, line end at </li>, <br> and </ul>,
    // entities (&auml;, &deg;, &#9888; ...) become UTF-8 (appendUtf8()).

    void appendUtf8(String& out, uint32_t cp) {
        if (cp < 0x80) out += (char)cp;
        else if (cp < 0x800) { out += (char)(0xC0 | (cp >> 6)); out += (char)(0x80 | (cp & 0x3F)); }
        else if (cp < 0x10000) { out += (char)(0xE0 | (cp >> 12)); out += (char)(0x80 | ((cp >> 6) & 0x3F)); out += (char)(0x80 | (cp & 0x3F)); }
        else { out += (char)(0xF0 | (cp >> 18)); out += (char)(0x80 | ((cp >> 12) & 0x3F)); out += (char)(0x80 | ((cp >> 6) & 0x3F)); out += (char)(0x80 | (cp & 0x3F)); }
    }

    String htmlToText(const String& html) {
        static const struct { const char* name; uint16_t cp; } entities[] = {
            { "amp", '&' }, { "lt", '<' }, { "gt", '>' }, { "quot", '"' }, { "apos", '\'' }, { "nbsp", ' ' },
            { "deg", 0xB0 }, { "auml", 0xE4 }, { "ouml", 0xF6 }, { "uuml", 0xFC }, { "Auml", 0xC4 }, { "Ouml", 0xD6 },
            { "Uuml", 0xDC }, { "szlig", 0xDF }, { "times", 0xD7 }, { "hellip", 0x2026 }, { "ndash", 0x2013 },
        };
        String out;
        out.reserve(html.length());
        for (int i = 0; i < (int)html.length(); i++) {
            char c = html[i];
            if (c == '<') {
                int end = html.indexOf('>', i);
                if (end < 0) break;
                String tag = html.substring(i + 1, end);
                tag.toLowerCase();
                if (tag == "li" || tag.startsWith("li ")) out += "- ";
                else if (tag == "h3") {
                    if (out.endsWith("- ")) out.remove(out.length() - 2);
                    out += "\n";
                }
                else if ((tag == "/li" || tag.startsWith("br") || tag == "/br" || tag == "/ul" || tag == "/h3") && !out.endsWith("\n")) out += "\n";
                i = end;
                continue;
            }
            if (c == '&') {
                int end = html.indexOf(';', i);
                if (end > i + 1 && end - i <= 9) {
                    String ent = html.substring(i + 1, end);
                    long cp = -1;
                    if (ent.startsWith("#x") || ent.startsWith("#X")) cp = strtol(ent.c_str() + 2, nullptr, 16);
                    else if (ent.startsWith("#")) cp = ent.substring(1).toInt();
                    else {
                        for (const auto& e : entities) {
                            if (ent == e.name) { cp = e.cp; break; }
                        }
                    }
                    if (cp > 0) {
                        appendUtf8(out, (uint32_t)cp);
                        i = end;
                        continue;
                    }
                }
            }
            out += c;
        }
        return out;
    }


    // status.txt: Kopfzeile mit Uhr, Displaytyp und Zeitpunkt, dann die Statusseite als Text
    // status.txt: header line with clock, display type and time, then the status page as text

    String buildStatusText() {
        char now[24] = "";
        if (timeinfo.tm_year >= 100) strftime(now, sizeof(now), "%Y-%m-%d %H:%M:%S", &timeinfo);
        String text = "uhr4 status - " + String(hostname) + " - " + displayChoiceName(displayType, useBacklight) + " - " + now + "\n\n";
        String chunk;
        generateStatusItems(chunk, [&text](String& part) { text += htmlToText(part); part = ""; });
        return text;
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


    // Sicherung als TAR direkt in die HTTP-Antwort streamen - es liegt nie das ganze Archiv im RAM. Bilder
    // (Zifferblaetter, Zeiger, Streifen) als Standard-BMP statt im RLEB-Format der Uhr, damit sie sich mit
    // jedem Bildprogramm oeffnen lassen; Wiederherstellen packt sie wieder (backupRestoreFinishBmp()).

    // Stream the backup as TAR straight into the HTTP response - the whole archive is never held in RAM.
    // Images (clock faces, hands, strips) as standard BMP instead of the clock's RLEB format, so any image
    // program can open them; restoring packs them again (backupRestoreFinishBmp()).

    void streamBackup(bool includeWifi) {
        String settings = buildBackupSettings(includeWifi);
        if (settings.length() == 0) {
            webserver.send(500, "text/plain", "Backup failed: WiFi credentials could not be encrypted");
            return;
        }
        std::vector<String> names;
        std::vector<size_t> sizes;
        collectBackupFiles(names, sizes);
        std::vector<bool> asBmp(names.size(), false);
        for (size_t i = 0; i < names.size(); i++) {
            int32_t w, h;
            if (names[i].endsWith(".bmp") && readRleSize("/" + names[i], w, h)) {
                asBmp[i] = true;
                sizes[i] = bmpFileSize565(w, h);
            }
        }

        // Zur Fehlersuche: gepufferte Logzeilen zuerst schreiben, dann Statusseite und Logs ans Ende
        // For troubleshooting: write buffered log lines first, then status page and logs at the end

        flushLogBuffer();
        String status = buildStatusText();
        std::vector<String> logNames;
        std::vector<size_t> logSizes;
        collectLogFiles(logNames, logSizes);

        size_t total = 512 + settings.length() + tarPadding(settings.length()) + 1024;
        for (size_t i = 0; i < names.size(); i++) total += 512 + sizes[i] + tarPadding(sizes[i]);
        total += 512 + status.length() + tarPadding(status.length());
        for (size_t i = 0; i < logNames.size(); i++) total += 512 + logSizes[i] + tarPadding(logSizes[i]);

        // Dateiname mit Hostname, Displaytyp (wie in flashESP, z.B. GC9A01_WITH_BACKLIGHT) und Datum
        // File name with host name, display type (as in flashESP, e.g. GC9A01_WITH_BACKLIGHT) and date

        char date[16] = "";
        if (timeinfo.tm_year >= 100) strftime(date, sizeof(date), "-%Y%m%d", &timeinfo);
        String fileName = "uhr4-backup-" + String(hostname) + "-" + displayChoiceName(displayType, useBacklight) + date + ".tar";

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
        auto sendFile = [&](const String& name, size_t size, bool bmp) {
            buildTarHeader(header, name, size);
            webserver.sendContent((const char*)header, 512);
            size_t sent = 0;
            if (bmp) {
                writeRleAsBmp("/" + name, [&](const uint8_t* data, size_t n) {
                    n = min(n, size - sent);
                    webserver.sendContent((const char*)data, n);
                    sent += n;
                });
            }
            File f = (sent < size && !bmp) ? LittleFS.open("/" + name, "r") : File();
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
        };
        for (size_t i = 0; i < names.size(); i++) sendFile(names[i], sizes[i], asBmp[i]);

        buildTarHeader(header, "status.txt", status.length());
        webserver.sendContent((const char*)header, 512);
        webserver.sendContent(status.c_str(), status.length());
        if (tarPadding(status.length())) webserver.sendContent((const char*)zeros, tarPadding(status.length()));
        for (size_t i = 0; i < logNames.size(); i++) sendFile(logNames[i], logSizes[i], false);

        webserver.sendContent((const char*)zeros, 512);
        webserver.sendContent((const char*)zeros, 512);

        DEBUG_PRINTLN("[Backup] Download: " + String(names.size()) + " files + status + " + String(logNames.size()) + " logs, " + String(total) + " bytes (from " + webserver.client().remoteIP().toString() + ")");
    }


    // Prueft settings.txt, BEVOR etwas geaendert wird: Kennung, Format, Displaytyp (muss zur Uhr passen),
    // Backlight-Modus (abweichend: keepBrightness) und ggf. WLAN-Daten (Format 2 verschluesselt, Format 1
    // Klartext). Liefert, was applyBackupSettings() spaeter schreibt.

    // Checks settings.txt BEFORE anything is changed: marker, format, display type (must match the clock),
    // backlight mode (differing: keepBrightness) and optionally the WiFi data (format 2 encrypted, format 1
    // plain text). Returns what applyBackupSettings() writes later.

    bool checkBackupSettings(const String& settings, bool restoreWifi, bool& applyWifi, bool& keepBrightness, String& wifiPlain, String& backupDisplay, String& error) {
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
        if (backupBacklight < 0) backupBacklight = DISPLAY_GEOMETRY[backupType].backlightDefault ? 1 : 0;
        backupDisplay = displayChoiceName(backupType, backupBacklight == 1);

        // Nur die Uhrgroesse muss passen - GC9A01 und ILI9341 teilen sich die 240er Zifferblaetter und Zeiger.
        // Only the clock size has to match - GC9A01 and ILI9341 share the 240 clock faces and hands.

        const DisplayGeometry& from = DISPLAY_GEOMETRY[backupType];
        if (from.clock != displayGeom->clock) {
            error = "the backup is from a " + String(from.name) + " clock (" + String(from.clock) + "x" + String(from.clock) +
                    "), this clock is set to " + String(displayGeom->name) + " (" + String(displayGeom->clock) + "x" +
                    String(displayGeom->clock) + ") - clock faces and hands would not fit";
            return false;
        }
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


    // Aeltere Sicherung: aendert eine neuere Firmware den Zahlentyp eines Schluessels (z.B. u8 -> i32), liest
    // sie den alten nicht. Daher in den Typ auf dieser Uhr umwandeln, begrenzt auf dessen Wertebereich;
    // Unbekannte bleiben.

    // Older backup: if a newer firmware changed a key's integer type (e.g. u8 -> i32), it cannot read the old
    // one. So convert to the type on this clock, clamped to its range; unknown keys keep their type.

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


    // settings.txt anwenden (nach checkBackupSettings()): erst alle ersetzten Schluessel entfernen (so
    // verschwinden auch ueberzaehlige Presets), dann die Werte mit ihrem Typ schreiben. isBackupKeptKey()
    // (Hardware, ohne applyWifi auch WLAN/Hostname) bleibt unberuehrt.

    // Apply settings.txt (after checkBackupSettings()): first remove all replaced keys (so surplus presets
    // disappear too), then write the values with their type. isBackupKeptKey() (hardware, without applyWifi
    // also WiFi/hostname) stays untouched.

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

        // 3) Datei-Migration beim naechsten Start erneut laufen lassen: eine aeltere Sicherung kann
        // Zifferblaetter/Zeiger im alten Format enthalten. Bereits umgestellte Dateien werden uebersprungen.

        // 3) Run the file migration again on the next boot: an older backup may contain faces/hands in the
        // old format. Already converted files are skipped.

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
        String bmpTarget;          // Bild, das gerade nach BACKUP_TMP_PATH geschrieben wird
                                   // image currently being written to BACKUP_TMP_PATH
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
        String backupDisplay;         // Displaytyp der Sicherung wie im Dateinamen / backup's display type as in the file name
        String wifiPlain;             // entschluesselte WLAN-Zeilen / decrypted WiFi lines

        ~BackupRestoreState() {
            backupWipe(wifiPlain);
        }
    };
    BackupRestoreState* backupRestore = nullptr;

    void backupRestoreFail(const String& error) {
        if (!backupRestore) return;
        if (backupRestore->out) backupRestore->out.close();
        LittleFS.remove(BACKUP_TMP_PATH);
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
        bool ok = checkBackupSettings(s.settings, s.restoreWifi, s.applyWifi, s.keepBrightness, s.wifiPlain, s.backupDisplay, error);
        if (!ok) {
            backupRestoreFail(error);
            return false;
        }
        deleteAllFacesAndHands();
        s.settingsChecked = true;
        return true;
    }

    // Bild aus der Sicherung ablegen: Standard-BMP wie beim Hochladen RLE-komprimiert (scaleAndSaveBmp() in
    // gleicher Groesse), RLEB aus aelteren Sicherungen unveraendert.

    // Store an image from the backup: standard BMP RLE-compressed like on upload (scaleAndSaveBmp() at the
    // same size), RLEB from older backups unchanged.

    void backupRestoreFinishBmp() {
        BackupRestoreState& s = *backupRestore;
        if (s.bmpTarget.isEmpty()) return;
        String target = "/" + s.bmpTarget;
        s.bmpTarget = "";
        int32_t w = 0, h = 0;
        LittleFS.remove(target);
        bool ok = readRleSize(BACKUP_TMP_PATH, w, h)
                ? LittleFS.rename(BACKUP_TMP_PATH, target)
                : readImageSize(BACKUP_TMP_PATH, w, h) && scaleAndSaveBmp(BACKUP_TMP_PATH, target.c_str(), w, h);
        LittleFS.remove(BACKUP_TMP_PATH);
        if (!ok) backupRestoreFail("cannot store " + target.substring(1));
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
        else if ((type == '0' || type == 0) && isBackupFileName(name) && size > 0) {
            bool bmp = name.endsWith(".bmp");
            s.bmpTarget = bmp ? name : String();
            String path = bmp ? String(BACKUP_TMP_PATH) : String("/" + name);
            s.out = LittleFS.open(path, "w");
            if (!s.out) {
                backupRestoreFail("cannot write " + name);
                return;
            }
            s.restoredFiles.push_back(name);
        }

        // Alles andere (Verzeichnisse, status.txt, Logdateien, fremde Dateien) wird uebersprungen
        // Everything else (directories, status.txt, log files, foreign files) is skipped

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
                    backupRestoreFinishBmp();
                    if (s.phase == BackupRestoreState::FAILED) return;
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


    // Upload-Rueckruf fuer /backup/restore: verarbeitet das Archiv beim Eintreffen, schreibt Dateien direkt
    // und wendet am Ende settings.txt an. Pruefung auf privates Netz schon beim Upload-Start - geschrieben
    // wird waehrenddessen.

    // Upload callback for /backup/restore: processes the archive as it arrives, writes files directly and
    // applies settings.txt at the end. Private network check right at upload start - writing happens during
    // the upload.

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
