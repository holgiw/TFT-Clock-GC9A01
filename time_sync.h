#pragma once
// ####################################################################
// ### Zeit: RTC, DCF77, NTP-Client & -Server, Zeitzone
// ####################################################################
// Benoetigt globals.h, config.h, prefs_keys.h und declarations.h (werden
// zentral in uhr3.ino VOR dieser Datei eingebunden).

void IRAM_ATTR isr() {
#if defined DCF77_DATAPIN && defined DCF77_INTERRUPT
    DCF77::int0handler();
    if (!dcfTimeFound) toggleLED();
    dcf77Count++;
    if (dcf77Count > 120) dcf77Count = 1;
#endif
}

// Lädt die Zeit vom RTC-Modul und setzt die Systemzeit entsprechend

void loadTimeFromRTC() {
    if (rtcOk == RTC_AVAILABLE) {

        DateTime now = rtc.now(); // DS3231 lesen

        // RTClib liefert Year/Month/Day usw. "normal" (2026, 2, 11, ...)
        struct tm tm_rtc = {};
        tm_rtc.tm_year = now.year() - 1900;
        tm_rtc.tm_mon = now.month() - 1;
        tm_rtc.tm_mday = now.day();
        tm_rtc.tm_hour = now.hour();
        tm_rtc.tm_min = now.minute();
        tm_rtc.tm_sec = now.second();
        tm_rtc.tm_isdst = -1; // Sommerzeit automatisch

        time_t t = mktime(&tm_rtc); // -> Unix time (lokale Interpretation je nach TZ!)

        // ESP32-Systemzeit setzen
        struct timeval tv;
        tv.tv_sec = t;
        tv.tv_usec = 0;
        settimeofday(&tv, nullptr);

        // Optional: globales timeinfo aktualisieren
        time_t now_esp = time(nullptr);
        localtime_r(&now_esp, &timeinfo);

        DEBUG_PRINTLN("[RTC] Time loaded from RTC and system time set");
    }
}


// Initialisiere die NTP-Server

void initializeNtpServers() {
    for (int i = 0; i < MAX_WLAN; i++) {
        String ntpServerKey = pkNtpServer(i);
        String ntpServerValue = preferences.getString(ntpServerKey.c_str(), "");
        if (ntpServerValue.isEmpty()) {
            // Standardwerte setzen, falls keine gespeicherten Werte vorhanden sind
            if (i == 0) {
                strncpy(ntpServers[i], NTP_SERVER_1, sizeof(ntpServers[i]) - 1);
            }
            else if (i == 1) {
                strncpy(ntpServers[i], NTP_SERVER_2, sizeof(ntpServers[i]) - 1);
            }
            else {
                ntpServers[i][0] = '\0'; // Leerer Eintrag
            }
        }
        else {
            // Gespeicherte Werte laden
            strncpy(ntpServers[i], ntpServerValue.c_str(), sizeof(ntpServers[i]) - 1);
            DEBUG_PRINTLN("[NTP] Loaded NTP server " + String(i + 1) + ": " + String(ntpServers[i]));
        }
        ntpServers[i][sizeof(ntpServers[i]) - 1] = '\0'; // Null-terminieren
    }
}



// Funktion, um die DCF77-Zeit abzurufen und die Systemzeit zu setzen 
bool getDCF77Time() {
#if defined DCF77_DATAPIN && defined DCF77_INTERRUPT
    time_t DCFtime;
    if (millis() - lastDCFUpdate > WAIT_1s) {
        // Serial.println(dcf77Count);
       
        lastDCFUpdate = millis(); // Timer zurücksetzen
         // DEBUG_PRINTLN("[DCF77] checking DCF77 time... ");
                
        DCFtime = dcf.getUTCTime(); // DCF-Zeit abrufen 
              
        if (DCFtime != 0) {
            setLedOff(); // LED ausschalten, wenn Zeit gefunden wurde

            if (millis() - lastRTCUpdate > WAIT_6h || dcfTimeFound == false) {

                lastRTCUpdate = millis();

                localtime_r(&DCFtime, &timeinfo); // Konvertiere time_t in struct tm
                setTimeStruct(timeinfo, "[DCF77] set time");         // Übergabe der struct tm an die Funktion
           
                if (rtcOk == RTC_AVAILABLE) {
                    
                    // Setze die RTC mit der synchronisierten Zeit
                    rtc.adjust(DateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                        timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));
                    DEBUG_PRINTLN("[DCF77] RTC updated with DCF77 time");
                }
            }

            dcfTimeFound = true;
        }
    }
    return dcfTimeFound;
 #else
    return false;   
 #endif
}


// Berechnet manuell, ob mitteleuropäische Sommerzeit (CEST) fuer den gegebenen
// Zeitpunkt gilt (aktuell ungenutzt, da die Zeitzone ueber TIMEZONE_DEFAULT/
// configTzTime bereits automatisch die Sommerzeit beruecksichtigt)
bool isDaylightSavingTime(struct tm* timeinfo) {
    // Prüfen, ob Sommerzeit aktiv ist (für Mitteleuropa)
    if (timeinfo->tm_mon < 2 || timeinfo->tm_mon > 9) {
        return false; // Vor März oder nach Oktober: keine Sommerzeit
    }
    if (timeinfo->tm_mon > 2 && timeinfo->tm_mon < 9) {
        return true; // April bis September: Sommerzeit
    }
    // März und Oktober: Übergang prüfen
    int lastSunday = timeinfo->tm_mday - timeinfo->tm_wday; // Letzter Sonntag
    if (timeinfo->tm_mon == 2) { // März
        return lastSunday >= 25; // Sommerzeit beginnt am letzten Sonntag im März
    }
    if (timeinfo->tm_mon == 9) { // Oktober
        return lastSunday < 25; // Sommerzeit endet am letzten Sonntag im Oktober
    }
    return false;
}


// Gibt die uebergebene Unix-Zeit lesbar (lokale Zeit und UTC) auf der seriellen
// Konsole aus - reine Debug-/Diagnosehilfe
void printTime(time_t rawTime) {
    // Konvertiere time_t in eine struct tm (lokale Zeit)
    struct tm* timeInfo = localtime(&rawTime);

    // Alternativ: GMT/UTC-Zeit verwenden
    // struct tm *timeInfo = gmtime(&rawTime);

    // Extrahiere Stunden, Minuten, Sekunden, usw.
    int hours = timeInfo->tm_hour;
    int minutes = timeInfo->tm_min;
    int seconds = timeInfo->tm_sec;
    int day = timeInfo->tm_mday;
    int month = timeInfo->tm_mon + 1; // Monate beginnen bei 0
    int year = timeInfo->tm_year + 1900; // Jahre seit 1900

    // Zeit und Datum ausgeben
    Serial.printf("Time: %02d:%02d:%02d\n", hours, minutes, seconds);
    Serial.printf("Date: %02d.%02d.%04d\n", day, month, year);
}



//
// NTP-Zeitsynchronisation um 02:00:05 und 03:00:05
//
void checkNightlyTimeSync() {
    static bool triggered2 = false;
    static bool triggered3 = false;

    if (timeinfo.tm_hour == 2 && timeinfo.tm_min == 0 && timeinfo.tm_sec == 5 && !triggered2) {
        DEBUG_PRINTLN("[TIME SYNC] Triggered at 02:00:05");        
        lastCheck = millis();
        triggered2 = true;    
        setupNTP();
    }

    if (timeinfo.tm_hour == 3 && timeinfo.tm_min == 0 && timeinfo.tm_sec == 5 && !triggered3) {
        DEBUG_PRINTLN("[TIME SYNC] Triggered at 03:00:05");         
        lastCheck = millis();
        triggered3 = true;
        setupNTP();
    }

    if (timeinfo.tm_hour > 3 && (triggered2 || triggered3)) {       
        triggered2 = false;
        triggered3 = false;
    }
}



// Initialisiert die Zeitsynchronisierung über NTP und stellt die Zeitzone ein.
// Im Fehlerfall wird die zuletzt bekannte Zeit verwendet.
boolean setupNTP() {

    if (loggingEnabled) Serial.println("[NTP] Setting up NTP..");
    if (WiFi.getMode() != WIFI_STA || !WiFi.isConnected()) {
        //DEBUG_PRINTLN("[NTP] Skipping NTP setup: Not in STA mode or WiFi not connected");
        return true;
    }

    timezone = preferences.getString(PK_TIMEZONE, TIMEZONE_DEFAULT);

    for (int i = 0; i < MAX_WLAN; i++) {
        String ntpServer = ntpServers[i];
        if (ntpServer.length() == 0) continue;
        //DEBUG_PRINTLN("[NTP] Trying server: " + String(ntpServers[i]));
        configTzTime(timezone.c_str(), ntpServers[i]);
        //  struct tm timeinfo;
        if (getLocalTime(&timeinfo, 500)) {

            DEBUG_PRINTLN("[NTP] Time synchronized successfully with " + ntpServer);

            if (rtcOk == RTC_AVAILABLE || rtcOk == RTC_AVAILABLE_BUT_INVALID) {

                // Setze die RTC mit der synchronisierten Zeit
                rtc.adjust(DateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                    timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));
                DEBUG_PRINTLN("[RTC] RTC updated with NTP time");

            }            
            return true;
        }
        DEBUG_PRINTLN("[NTP] Failed to synchronize with server: " + ntpServer);
    }
    handleNTPFailure();
    return false;
}

// Behandelt NTP-Synchronisierungsfehler, indem die letzte bekannte Zeit verwendet oder auf 12:00 Uhr gesetzt wird.

void handleNTPFailure() {
    DEBUG_PRINTLN("[NTP] Handling NTP synchronization failure..");

    // Versuche, die letzte bekannte Zeit zu verwenden
   // struct tm timeinfo;
    if (getLocalTime(&timeinfo, 100)) {
        char timeStr[32];
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
        DEBUG_PRINTLN("[NTP] Using last known time: " + String(timeStr));
    }
    else {
        // Wenn keine gültige Zeit verfügbar ist, auf 12:00 Uhr setzen
        DEBUG_PRINTLN("[NTP] No valid time available. Setting time to 12:00");
        timeinfo.tm_hour = 12;
        timeinfo.tm_min = 0;
        timeinfo.tm_sec = 0;
        timeinfo.tm_year = 126; // Jahr 2026 (1900 + 126)
        timeinfo.tm_mon = 0;    // Januar
        timeinfo.tm_mday = 1;   // 1. Tag des Monats
        setTimeStruct(timeinfo, "[NTP]"); // Funktion, um die Zeit zu setzen
    }
    // Optionally, schedule a retry later
    scheduleNTPRetry();
}

// Setzt die Systemzeit manuell anhand einer `tm`-Struktur. 

void setTimeStruct(const struct tm& timeinfo, String source) {

    // Konvertiere struct tm in time_t (unter Berücksichtigung der Zeitzone)
    time_t t = mktime(const_cast<struct tm*>(&timeinfo));

    // Setze die Systemzeit
    timeval tv = { t, 0 }; // Sekunden und Mikrosekunden
    settimeofday(&tv, nullptr);

    char buffer[64];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S %Z", &timeinfo);
    DEBUG_PRINT(source + " ");
    DEBUG_PRINTLN(buffer); // Gibt die lokale Zeit und die Zeitzone aus
}

// Plant einen NTP-Wiederholungsversuch in 30 Minuten.

void scheduleNTPRetry() {
    lastNTPRetry = millis();
    DEBUG_PRINTLN("[NTP] Scheduled retry in 30 minutes");
}

// Überprüft, ob es Zeit für einen NTP-Wiederholungsversuch ist, und führt diesen gegebenenfalls durch.

void checkNTPRetry() {
    if (WiFi.status() != WL_CONNECTED) {
        //DEBUG_PRINTLN("[NTP] Skipping retry: Not connected to WiFi");
        lastNTPRetry = millis();
        return;
    }
    if (lastNTPRetry > 0 && millis() - lastNTPRetry >= WAIT_1h) {
        // DEBUG_PRINTLN("[NTP] Retrying NTP synchronization..");
        setupNTP();
        lastNTPRetry = millis();
    }
}



// --- Funktion: Scannt den I2C-Bus nach Geräten und gibt die Anzahl der gefundenen Geräte zurück ---
uint16_t i2cScan() {
    byte error, address;
    int nDevices = 0;
    
    int rtc3231Addr = 0x68;
    
    i2cAddr = "";

    // teste direkt auf 0x68
    Wire.beginTransmission(rtc3231Addr);
    error = Wire.endTransmission();
    if (error == 0) {        
        i2cAddr = "0x68 (RTC DS3231)";
        DEBUG_PRINTLN("[I2C] " + i2cAddr);
        return 1;
    }

    DEBUG_PRINTLN("[I2C] Scanning I2C..");
    
    for (address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
        if (error == 0) {
            DEBUG_PRINT("[I2C]   I2C device found at address 0x");
            if (address < 16) {
                DEBUG_PRINT("0");
            }
            DEBUG_PRINTLN(String(address, HEX) + " ");            
        }
        else if (error == 4) {
            DEBUG_PRINT("[I2C] Unknow error at address 0x");
            if (address < 16) {
                DEBUG_PRINT("0");
            }
            DEBUG_PRINTLN(String(address, HEX) + " ");
        }
    }
    if (nDevices == 0) {
        DEBUG_PRINTLN("[I2C] No I2C devices found");
    }
    else {
        DEBUG_PRINTLN("[I2C] done");
    }

    return nDevices;
}


// Funktion, um ein NTP-Paket zu erstellen

void createNtpResponse(byte* packet, time_t currentTime) {
    memset(packet, 0, NTP_PACKET_SIZE);

    // Flags und Stratum
    packet[0] = 0b00100100; // LI, Version, Mode
    packet[1] = 1;          // Stratum
    packet[2] = 6;          // Poll Interval
    packet[3] = 0xEC;       // Precision

    // Root Delay und Root Dispersion
    packet[4] = 0;
    packet[5] = 0;
    packet[6] = 0;
    packet[7] = 0;

    // Reference Identifier
    packet[12] = 'L';
    packet[13] = 'O';
    packet[14] = 'C';
    packet[15] = 'L';

    // Reference Timestamp
    time_t refTime = currentTime - 1; // Referenzzeit (1 Sekunde vorher)
    uint32_t refSeconds = htonl((uint32_t)(refTime + 2208988800UL));
    memcpy(&packet[16], &refSeconds, 4);

    // Origin Timestamp (kann leer bleiben)
    // Transmit Timestamp
    uint32_t transmitSeconds = htonl((uint32_t)(currentTime + 2208988800UL));
    memcpy(&packet[40], &transmitSeconds, 4);
}
