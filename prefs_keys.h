#pragma once
    // ####################################################################
    // ### Zentrale Preferences-Keys ######################################
    // ####################################################################
    // Alle NVS/Preferences-Keys an EINER Stelle definieren, statt sie als
    // String-Literale über den ganzen Code zu verstreuen. Verhindert
    // Tippfehler wie "lastWlan" vs. "lastWLan" (führte zu einem Bug: der
    // Reconnect griff immer auf Slot 0 statt auf das zuletzt aktive WLAN zu).
    //
    // Vorteil: Tippfehler werden zu Compile-Fehlern (unbekannter Bezeichner)
    // statt zu stillen Laufzeit-Bugs.

    // --- Allgemein / System ---
    constexpr const char* PK_VERSION           = "version";
    constexpr const char* PK_FIRST_START       = "firstStart";
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
    constexpr const char* PK_HANDSET           = "handset";
    constexpr const char* PK_BACKGROUND        = "background";
    constexpr const char* PK_STATION_MODE      = "stationMode";
    // WICHTIG: einziger gültiger Key für die Sekundenzeiger-Sichtbarkeit.
    // Im Original gab es an einer Stelle (ILI9341-Codepfad, aktuell inaktiv)
    // stattdessen den abweichenden Key "secondHand" -> Bug, da der Wert nie
    // mit dem per Webserver/Preset gesetzten "showSecondHand" übereinstimmt.
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

    // ####################################################################
    // ### Bekannte Default-Wert-Inkonsistenzen (im Original-Sketch) ######
    // ####################################################################
    // - PK_STATION_MODE: Default ist überall `true`, außer in einer Stelle
    //   im Webserver-Handler (Zeile ~3280 im Original), wo `false` verwendet
    //   wird. Beim Zentralisieren bitte einheitlich auf `true` setzen.
    // - PK_BRIGHT_START_HOUR / PK_BRIGHT_END_HOUR: Ladefunktion nutzt
    //   Default 7/21, die Status-Seite zeigt bei fehlendem Wert 8/20 an.
    //   Rein kosmetisch (nur relevant falls Key fehlt), sollte aber
    //   vereinheitlicht werden.


    // ####################################################################
    // ### Indizierte Keys (WLAN-Slots, NTP-Server, Presets) ##############
    // ####################################################################
    // Statt an jeder Stelle "ssid" + String(i + 1) von Hand zu bauen,
    // zentrale Helper verwenden. Rückgabe als String, damit .c_str()
    // direkt an preferences.getString()/putString() übergeben werden kann.

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
