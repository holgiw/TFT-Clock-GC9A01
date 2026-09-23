#pragma once
    // Alle NVS/Preferences-Keys an EINER Stelle statt verstreuter String-Literale -
    // verhindert Tippfehler-Bugs, die sonst zu stillen Laufzeitfehlern statt Compile-Fehlern wuerden.

    // All NVS/Preferences keys in ONE place instead of scattered string literals -
    // turns typo bugs into compile errors instead of silent runtime bugs.

    // Allgemein / System
    // General / System
    constexpr const char* PK_VERSION           = "version";
    constexpr const char* PK_FIRST_START       = "firstStart";
    constexpr const char* PK_MIGRATIONS_DONE   = "migrDone"; // Flag: RLE-Migration + Eckenmaskierung bereits abgeschlossen (siehe setup())
                                                             // Flag: RLE migration + corner masking already done (see setup())
    constexpr const char* PK_LANGUAGE          = "language";
    constexpr const char* PK_LOGGING_ENABLED   = "loggingEnabled";
    constexpr const char* PK_LOG_FILE_NUMBER   = "logFileNumber";
    constexpr const char* PK_PREVIEW_SIZE      = "previewSize"; // Groesse der Web-Vorschau (/preview) in px,
                                                                // vom Nutzer per Schieberegler eingestellt (siehe webserver_routes.h)
                                                                // size of the web preview (/preview) in px, set by the
                                                                // user via a slider (see webserver_routes.h)

    // WLAN
    // WiFi
    constexpr const char* PK_WIFI_ACTIVE       = "wifiActive";
    constexpr const char* PK_LAST_WLAN         = "lastWLan";   // <- einzige Quelle der Wahrheit
                                                               // <- single source of truth

    // Zeit / NTP
    // Time / NTP
    constexpr const char* PK_TIMEZONE          = "timezone";
    constexpr const char* PK_DCF_SYNC_LED      = "dcfSyncLed"; // LED blitzt pro DCF77-Impuls waehrend der Sync-Phase (Default true)
                                                               // LED flashes per DCF77 pulse during the sync phase (default true)

    // Zifferblatt / Darstellung
    // Clock Face / Display
    constexpr const char* PK_TFT_ROTATION1     = "tftRotation1"; // Rotation Display 1 (0-3, 4 = n.a.) - hiess vor Display-2-Support "tftRotation" (siehe LEGACY unten)
                                                                 // rotation of Display 1 (0-3, 4 = n.a.) - was called "tftRotation" before Display 2 support (see LEGACY below)
    constexpr const char* PK_TFT_ROTATION_LEGACY = "tftRotation"; // Alter Key-Name - NUR fuer die einmalige Migration in uhr3.ino verwenden
                                                                   // old key name - use ONLY for the one-time migration in uhr3.ino
    constexpr const char* PK_TFT_ROTATION2     = "tftRotation2"; // Rotation von Display 2 (CS2), 4 = n.a. (nicht angeschlossen, Standard)
                                                                 // rotation of Display 2 (CS2), 4 = n.a. (not connected, default)
    constexpr const char* PK_HOSTNAME          = "hostname"; // leer = automatisch aus MAC-Adresse generiert
                                                             // empty = auto-generated from MAC address
    constexpr const char* PK_HANDSET           = "handset";
    constexpr const char* PK_BACKGROUND        = "background";
    // PK_USE_CS2 entfernt: Display 2 (CS2-Pin) wird ueber die Rotation "n.a." (TFT_ROTATION_NA) abgeschaltet, kein eigener Preferences-Schalter (siehe config.h/globals.h/uhr3.ino).
    // PK_USE_CS2 removed: Display 2 (CS2 pin) is switched off via the rotation value "n.a." (TFT_ROTATION_NA), no separate preferences toggle (see config.h/globals.h/uhr3.ino).
    constexpr const char* PK_STATION_MODE      = "stationMode"; // "wartet auf 12" (siehe globals.h) - "waits at 12" (see globals.h)
    constexpr const char* PK_SMOOTH_SECOND     = "smoothSecond"; // Darstellungsstil des Sekundenzeigers (siehe globals.h)
                                                                 // second hand rendering style (see globals.h)
    // WICHTIG: einziger gueltiger Key fuer Sekundenzeiger-Sichtbarkeit (abweichender
    // Key "secondHand" im ILI9341-Codepfad war ein Bug, siehe git-Historie).

    // IMPORTANT: only valid key for second-hand visibility (the differing
    // key "secondHand" in the ILI9341 code path was a bug, see git history).
    constexpr const char* PK_SHOW_SECOND_HAND  = "showSecondHand";
    constexpr const char* PK_SMOOTH_MINUTE     = "smoothMinute";
    constexpr const char* PK_CENTER_COLOR      = "centerColor";
    constexpr const char* PK_CENTER_SIZE       = "centerSize";
    constexpr const char* PK_CURRENT_PRESET    = "currentPreset";

    // Helligkeit
    // Brightness
    constexpr const char* PK_MIN_BRIGHTNESS    = "minBrightness";
    constexpr const char* PK_MAX_BRIGHTNESS    = "maxBrightness";
    constexpr const char* PK_GAMMA_BRIGHTNESS  = "gammaBrightness";
    constexpr const char* PK_LOW_THRESHOLD     = "lowThreshold";
    constexpr const char* PK_HIGH_THRESHOLD    = "highThreshold";
    constexpr const char* PK_ADC_INVERTED      = "adcInverted";
    constexpr const char* PK_USE_ADC           = "use_adc";
    constexpr const char* PK_BRIGHT_START_HOUR = "brightStart";
    constexpr const char* PK_BRIGHT_END_HOUR   = "brightEnd";

    // Touch
    // Touch
    constexpr const char* PK_USE_TOUCH         = "useTouch";

    // Rocrail-Modellzeit (siehe rocrail_client.h)
    // Rocrail model time (see rocrail_client.h)
    constexpr const char* PK_ROCRAIL_ENABLED   = "rocrailEnabled";

    // PK_ROCRAIL_SERVER/PK_ROCRAIL_SRV_PORT: der aktuell aktive Server -
    // wird beim Speichern aus der Liste unten (dem angehakten Eintrag)
    // uebernommen. rocrail_client.h kennt nur diese zwei Werte, nicht die Liste.

    // PK_ROCRAIL_SERVER/PK_ROCRAIL_SRV_PORT: the currently active server -
    // taken over from the list below (the checked entry) when saving.
    // rocrail_client.h only knows these two values, not the list.
    constexpr const char* PK_ROCRAIL_SERVER    = "rocrailServer";
    constexpr const char* PK_ROCRAIL_SRV_PORT  = "rocrailSrvPort";

    // Liste moeglicher Rocrail-Server (bis zu MAX_WLAN Eintraege) - wie bei
    // den NTP-Servern immer ein leerer Platz nach dem letzten befuellten.
    // PK_ROCRAIL_ACTIVE_SRV: 0-basierter Index des angehakten Eintrags, -1 = keiner.

    // List of possible Rocrail servers (up to MAX_WLAN entries) - like the
    // NTP servers, always one empty slot after the last filled one.
    // PK_ROCRAIL_ACTIVE_SRV: 0-based index of the checked entry, -1 = none.
    constexpr const char* PK_ROCRAIL_ACTIVE_SRV = "rocSrvActive";

    // Liefert den Preferences-Key fuer den Hostnamen/die IP des Rocrail-Server-Eintrags an Index i
    // Returns the preferences key for the hostname/IP of the Rocrail server entry at index i

    inline String pkRocrailServerHost(int i) { return "rocSrvH" + String(i + 1); }


    // Liefert den Preferences-Key fuer den Port des Rocrail-Server-Eintrags an Index i
    // Returns the preferences key for the port of the Rocrail server entry at index i

    inline String pkRocrailServerPort(int i) { return "rocSrvP" + String(i + 1); }


    // Liefert den Preferences-Key fuer den (vom Nutzer editierbaren, siehe
    // rocrailServerNameList[] in globals.h) Anlagennamen des Rocrail-Server-
    // Eintrags an Index i

    // Returns the preferences key for the (user-editable, see
    // rocrailServerNameList[] in globals.h) layout name of the Rocrail
    // server entry at index i

    inline String pkRocrailServerName(int i) { return "rocSrvN" + String(i + 1); }

    // Wartung
    // Maintenance
    constexpr const char* PK_LAST_RESET_WEEK   = "last_reset_week";

    // PK_STATION_MODE: Default ueberall `true`, ausser einer Stelle mit `false`.
    // PK_BRIGHT_START_HOUR/END_HOUR: Ladefunktion nutzt 7/21, Status-Seite zeigt 8/20 - rein kosmetisch.
    // PK_MIN_BRIGHTNESS/PK_LOW_THRESHOLD/PK_HIGH_THRESHOLD/PK_CENTER_SIZE: Erststart schreibt board-abhaengige Werte, spaetere Fallbacks nutzen aber immer die Nicht-Backlight-Defaults.

    // PK_STATION_MODE: default `true` everywhere, except one spot with `false`.
    // PK_BRIGHT_START_HOUR/END_HOUR: load function uses 7/21, status page shows 8/20 - cosmetic only.
    // PK_MIN_BRIGHTNESS/PK_LOW_THRESHOLD/PK_HIGH_THRESHOLD/PK_CENTER_SIZE: first run writes board-dependent values, but later fallbacks always use the non-backlight defaults.


    // Indizierte Keys (WLAN-Slots, NTP-Server, Presets)
    // Statt "ssid" + String(i+1) ueberall von Hand zu bauen, zentrale Helper verwenden.

    // Indexed Keys (WiFi Slots, NTP Servers, Presets)
    // Instead of building "ssid" + String(i+1) by hand everywhere, use these central helpers.


    // Liefert den Preferences-Key fuer das WLAN-SSID-Feld an Index i
    // Returns the preferences key for the WiFi SSID field at index i

    inline String pkSsid(int i)         { return "ssid" + String(i + 1); }


    // Liefert den Preferences-Key fuer das WLAN-Passwort-Feld an Index i
    // Returns the preferences key for the WiFi password field at index i

    inline String pkPass(int i)         { return "pass" + String(i + 1); }


    // Liefert den Preferences-Key fuer den NTP-Server an Index i
    // Returns the preferences key for the NTP server at index i

    inline String pkNtpServer(int i)    { return "ntpServer" + String(i + 1); }


    // Liefert den Preferences-Key fuer den Namen des Presets an Index i
    // Returns the preferences key for the preset name at index i

    inline String pkPresetName(int i)   { return "preset" + String(i) + "_name"; }


    // Liefert den Preferences-Key fuer die URL des Presets an Index i
    // Returns the preferences key for the preset URL at index i

    inline String pkPresetUrl(int i)    { return "preset" + String(i) + "_url"; }

    // HINWEIS: putStringVerified() ist absichtlich nicht hier definiert, da
    // prefs_keys.h in uhr3.ino vor globals.h ("preferences") und config.h
    // (DEBUG_PRINTLN) eingebunden wird; Implementierung siehe wifi_manager.h.

    // NOTE: putStringVerified() is deliberately not defined here, since
    // prefs_keys.h is included in uhr3.ino before globals.h ("preferences") and
    // config.h (DEBUG_PRINTLN); see wifi_manager.h for the implementation.

