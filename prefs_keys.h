#pragma once
    // ### Zentrale Preferences-Keys ######################################
    // Alle NVS/Preferences-Keys an EINER Stelle statt als verstreute String-Literale -
    // verhindert Tippfehler-Bugs (z.B. "lastWlan" vs "lastWLan"), da sie zu Compile-Fehlern statt stillen Laufzeit-Bugs werden.

    // --- Allgemein / System ---
    constexpr const char* PK_VERSION           = "version";
    constexpr const char* PK_FIRST_START       = "firstStart";
    constexpr const char* PK_MIGRATIONS_DONE   = "migrDone"; // Flag: RLE-Migration + Eckenmaskierung bereits abgeschlossen (siehe setup())
    constexpr const char* PK_LANGUAGE          = "language";
    constexpr const char* PK_LOGGING_ENABLED   = "loggingEnabled";
    constexpr const char* PK_LOG_FILE_NUMBER   = "logFileNumber";

    // --- WLAN ---
    constexpr const char* PK_WIFI_ACTIVE       = "wifiActive";
    constexpr const char* PK_LAST_WLAN         = "lastWLan";   // <- einzige Quelle der Wahrheit
    constexpr const char* PK_PING_SERVER       = "pingServer";

    // --- Zeit / NTP ---
    constexpr const char* PK_TIMEZONE          = "timezone";

    // --- Zifferblatt / Darstellung ---
    constexpr const char* PK_TFT_ROTATION      = "tftRotation";
    constexpr const char* PK_HOSTNAME          = "hostname"; // leer = automatisch aus MAC-Adresse generiert
    constexpr const char* PK_HANDSET           = "handset";
    constexpr const char* PK_BACKGROUND        = "background";
    constexpr const char* PK_STATION_MODE      = "stationMode";
    // WICHTIG: einziger gueltiger Key fuer Sekundenzeiger-Sichtbarkeit (abweichender
    // Key "secondHand" im ILI9341-Codepfad war ein Bug, siehe git-Historie).
    constexpr const char* PK_SHOW_SECOND_HAND  = "showSecondHand";
    constexpr const char* PK_SMOOTH_MINUTE     = "smoothMinute";
    constexpr const char* PK_CENTER_COLOR      = "centerColor";
    constexpr const char* PK_CENTER_SIZE       = "centerSize";
    constexpr const char* PK_CURRENT_PRESET    = "currentPreset";

    // --- Helligkeit ---
    constexpr const char* PK_MIN_BRIGHTNESS    = "minBrightness";
    constexpr const char* PK_MAX_BRIGHTNESS    = "maxBrightness";
    constexpr const char* PK_GAMMA_BRIGHTNESS  = "gammaBrightness";
    constexpr const char* PK_LOW_THRESHOLD     = "lowThreshold";
    constexpr const char* PK_HIGH_THRESHOLD    = "highThreshold";
    constexpr const char* PK_ADC_INVERTED      = "adcInverted";
    constexpr const char* PK_USE_ADC           = "use_adc";
    constexpr const char* PK_BRIGHT_START_HOUR = "brightStart";
    constexpr const char* PK_BRIGHT_END_HOUR   = "brightEnd";

    // --- Touch ---
    constexpr const char* PK_USE_TOUCH         = "useTouch";

    // --- Wartung ---
    constexpr const char* PK_LAST_RESET_WEEK   = "last_reset_week";

    // ### Bekannte Default-Wert-Inkonsistenzen (im Original-Sketch) ######
    // PK_STATION_MODE: Default ueberall `true`, ausser einer Webserver-Handler-Stelle mit `false` (beim Zentralisieren einheitlich auf `true` setzen).
    // PK_BRIGHT_START_HOUR/END_HOUR: Ladefunktion nutzt 7/21, Status-Seite zeigt bei fehlendem Wert 8/20 - rein kosmetisch, sollte vereinheitlicht werden.


    // ### Indizierte Keys (WLAN-Slots, NTP-Server, Presets) ##############
    // Statt "ssid" + String(i+1) ueberall von Hand zu bauen, zentrale Helper
    // verwenden - Rueckgabe als String, .c_str() direkt an preferences.get/putString() uebergebbar.

    // Liefert den Preferences-Key fuer das WLAN-SSID-Feld an Index i
    inline String pkSsid(int i)         { return "ssid" + String(i + 1); }


    // Liefert den Preferences-Key fuer das WLAN-Passwort-Feld an Index i
    inline String pkPass(int i)         { return "pass" + String(i + 1); }


    // Liefert den Preferences-Key fuer den NTP-Server an Index i
    inline String pkNtpServer(int i)    { return "ntpServer" + String(i + 1); }


    // Liefert den Preferences-Key fuer den Namen des Presets an Index i
    inline String pkPresetName(int i)   { return "preset" + String(i) + "_name"; }


    // Liefert den Preferences-Key fuer die URL des Presets an Index i
    inline String pkPresetUrl(int i)    { return "preset" + String(i) + "_url"; }
