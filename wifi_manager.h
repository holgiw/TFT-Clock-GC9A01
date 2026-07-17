#pragma once
// ####################################################################
// ### WLAN: Verbindungsaufbau, Access-Point, Scan, Reconnect
// ####################################################################
// Benoetigt globals.h, config.h, prefs_keys.h und declarations.h (werden
// zentral in uhr3.ino VOR dieser Datei eingebunden).

// WPS-Typ definieren (Push-Button-Methode)
#define ESP_WPS_MODE WPS_TYPE_PBC

// WPS-Initialisierung
esp_wps_config_t wps_config = WPS_CONFIG_INIT_DEFAULT(ESP_WPS_MODE);

// Aktiviert WPS (Push-Button-Methode) am ESP32 und startet den Verbindungsversuch
void startWPS() {
    if (esp_wifi_wps_enable(&wps_config) == ESP_OK) {
        if (esp_wifi_wps_start(0) == ESP_OK) {           
            DEBUG_PRINTLN("[WPS] WPS started. Please press the WPS button on the router");
        }
        else {
            DEBUG_PRINTLN("[WPS] WPS could not be started");
        }
    }
    else {
        DEBUG_PRINTLN("[WPS] WPS could not be activated");
    }
}


// Startet WPS und wartet wiederholt auf eine erfolgreiche Verbindung; gibt die
// per WPS empfangenen WLAN-Zugangsdaten (SSID/Passwort) auf der seriellen Konsole aus
void scanWPS() {
    Serial.println("[WPS] ");
    startWPS(); // WPS starten  

    for (int i = 0; i < 1000000; i++) {

        Serial.print("");
        long wpsWaitMillis = millis();
        while (WiFi.status() != WL_CONNECTED && (millis() - wpsWaitMillis) <= WAIT_10m) {
            delay(100);
        }
        
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("");
            Serial.println("SSID: " + WiFi.SSID());
            Serial.println("PASS: " + WiFi.psk());
            delay(5000); // 5 Sekunden warten, damit der Nutzer die Daten notieren kann
            WiFi.disconnect(); // Verbindung trennen, damit der normale Verbindungsprozess mit den gespeicherten SSIDs funktioniert
            startWPS(); // WPS erneut starten, damit es für zukünftige Verbindungsversuche bereit ist
        }
    }
}


// überpüft die WiFi-Verbindung und versucht, sie alle x Minuten wiederherzustellen, wenn sie getrennt ist.
bool checkWiFiReconnect() {
    static unsigned long lastAttempt = 0;
    
    unsigned long now = millis();
    if (now - lastAttempt < WAIT_1h) return true;
    lastAttempt = now;

    String pingServer = preferences.getString(PK_PING_SERVER, "");

    if (WiFi.status() == WL_CONNECTED) {
        if (pingServer == "") {
            return true; // Kein Ping-Server konfiguriert, Verbindung als in Ordnung betrachten
        }
        if (isInternetReachable(pingServer)) {
            return true; // Verbindung ist in Ordnung
        }
        else {
            // DEBUG_PRINTLN("[WiFi] Connected to WiFi but no internet. Attempting reconnect..");
            // return false; // Verbindung hat kein Internet, Reconnect versuchen
        }
    }
     
    DEBUG_PRINTLN("[WiFi] Disconnected. Attempting reconnect..");
    WiFi.disconnect();
    connectWiFi(preferences.getInt(PK_LAST_WLAN,0), false);    
    return WiFi.status() == WL_CONNECTED;
}


// Startet einen WLAN-Access-Point mit dem Namen und Passwort 'clock123' und zeigt Verbindungsinformationen auf dem Display an.
void startAP() {
#ifdef TFT_Backlight
    ledcWrite(TFT_Backlight, 255);
#endif


    // WLAN im Station-Modus starten
    WiFi.mode(WIFI_MODE_STA);
    WiFi.begin();

    
    // WPS versuchen, wenn möglich
    // leeres oder letztes WLAN finden
    String ssidKey;
    String passKey;

    int lastWlanNr = 0;
    for (lastWlanNr = 0; lastWlanNr < MAX_WLAN; lastWlanNr++) {
        // Dynamisch berechnete Schlüssel
        ssidKey = pkSsid(lastWlanNr);
        passKey = pkPass(lastWlanNr);
                
        if (preferences.getString(ssidKey.c_str(), "") == "") {
            break; // Beende die Schleife, wenn ein leerer  gefunden wird, ansonsten letzter
        }        
    }    
    if (lastWlanNr >= MAX_WLAN) {
        // Alle Slots belegt: letzten gueltigen Slot (MAX_WLAN-1) ueberschreiben,
        // statt eines ungueltigen Index ausserhalb des Arrays (war zuvor ein Bug).
        lastWlanNr = MAX_WLAN - 1;
        ssidKey = pkSsid(lastWlanNr);
        passKey = pkPass(lastWlanNr);
    }
    
    //setCS1(LOW);
    tft.fillRect(0, 0, CLOCK_WIDTH, CLOCK_HEIGHT, TFT_BLACK);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setTextSize(TFT_TEXT_SIZE);
    tft.setCursor(10, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 8));
    tft.println("check for WPS..");
                    
    startWPS(); // WPS starten  
    
    long wpsWaitMillis = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - wpsWaitMillis) <= WAIT_30s) { 
        delay(100);
    }
    if (WiFi.status() == WL_CONNECTED) {
        DEBUG_PRINTLN("[WPS] Verbunden mit dem Netzwerk!");
        DEBUG_PRINTLN("[WPS] SSID: " + WiFi.SSID());
        // Serial.println("Passwort: " + WiFi.psk());

        // Pruefen, ob diese SSID bereits in einem der Slots gespeichert ist.
        bool alreadyStored = false;
        for (int i = 0; i < MAX_WLAN; i++) {
            if (wifiSsid[i] == WiFi.SSID()) {
                alreadyStored = true;
                if (wifiPass[i] == WiFi.psk()) {
                    DEBUG_PRINTLN("[WPS] SSID " + WiFi.SSID() + " already stored unchanged in slot " + String(i + 1) + ", skipping save.");
                }
                else {
                    // Gleiche SSID, aber anderes Passwort - vorhandenen Eintrag aktualisieren.
                    String existingPassKey = pkPass(i);
                    preferences.putString(existingPassKey.c_str(), WiFi.psk());
                    wifiPass[i] = WiFi.psk();
                    preferences.putInt(PK_LAST_WLAN, i);
                    DEBUG_PRINTLN("[WPS] SSID " + WiFi.SSID() + " found in slot " + String(i + 1) + " with a different password - updated.");
                }
                break;
            }
        }

        if (!alreadyStored) {
            preferences.putString(ssidKey.c_str(), WiFi.SSID());
            preferences.putString(passKey.c_str(), WiFi.psk());

            preferences.putInt(PK_LAST_WLAN, lastWlanNr);

            DEBUG_PRINTLN("[WPS] Saved credentials: " + WiFi.SSID() + " in " + ssidKey.c_str());
        }

        tft.setCursor(10, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 4));
        tft.println(WiFi.SSID());

        tft.setCursor(10, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 8));
        tft.println("found WPS... reboot");
        
        delay(WAIT_5s);
        espReboot();
    }

    // WPS deaktivieren, um Speicherplatz zu sparen
    esp_wifi_wps_disable();


    // WLAN-Scan durchführen
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setTextSize(TFT_TEXT_SIZE);
    tft.setCursor(10, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 8));
    tft.println("WLAN-Scan..");
    int networkCount = WiFi.scanNetworks();
    delay(100); // Kurze Pause für Anzeige

    // availableNetworks füllen
    foundNetworkCount = 0;
    for (int i = 0; i < networkCount && i < MAX_WLAN; i++) {
        availableNetworks[i].ssid = WiFi.SSID(i);
        availableNetworks[i].rssi = WiFi.RSSI(i);
        availableNetworks[i].enc = WiFi.encryptionType(i);
        foundNetworkCount++;
    }


    WiFi.softAP("clock123", "clock123");
    DEBUG_PRINTLN("[WiFi] Started Access Point: clock123");

    // Captive portal: leite alle DNS-Anfragen auf die AP-IP um
    dnsServer.start(53, "*", WiFi.softAPIP());

    clearTFT();

    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setTextSize(TFT_TEXT_SIZE);
    tft.setCursor(10, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 8)) ;
    tft.println("AccessPoint active");
    tft.setCursor(10, (CLOCK_HEIGHT / 2));
    tft.println("clock123 clock123");
    tft.setCursor(10, (CLOCK_HEIGHT / 2 ) + (CLOCK_HEIGHT / 8));

    tft.print("http://");
    tft.println(WiFi.softAPIP());

    softAPIP = true;
    softAPIPstart = millis();

    ipAddress = WiFi.softAPIP().toString();
      
}


// Versucht, eine Verbindung zum WLAN herzustellen, basierend auf den gespeicherten SSID- und Passwort-Paaren. 
// Zeigt während des Verbindungsversuchs Informationen auf dem Display an und überprüft die 
// Internet-Konnektivität nach erfolgreicher Verbindung.
int connectWiFi(int number, bool verboseMode) {
#ifdef TFT_Backlight
    if (verboseMode) {
        ledcWrite(TFT_Backlight, 255);
    }
#endif
    if (wifiSsid[number] == "") {
       // DEBUG_PRINTLN("[WiFi] SSID " + String(number + 1) + " is empty, skipping");
        return NOT_CONNECTED;
    }

 

    
    // DEBUG_PRINTLN("[WiFi] Trying SSID" + String(number+1) + ": " + wifiSsid[number]);

    // Wenn verboseMode aktiviert ist, zeige die Verbindungsinformationen auf dem Display an
    if (verboseMode) {
        clearTFT();
        tft.setTextColor(TFT_GREEN, TFT_BLACK);

        tft.setTextSize(TFT_TEXT_SIZE / 2);
#if defined GC9D01
        tft.setCursor(20, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 3));
#else
        tft.setCursor(60, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 3));
#endif


        tft.println(String(version));

        tft.setTextSize(TFT_TEXT_SIZE);
        tft.setCursor(20, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 8));
        tft.println("Connect to SSID" + String(number+1));
        tft.setCursor(20, (CLOCK_HEIGHT / 2));

        if (wifiSsid[number].length() > 15) {
            tft.print(wifiSsid[number].substring(0,15));
            tft.println("..");
        } else tft.println(wifiSsid[number]);
    }

    DEBUG_PRINTLN("[WiFi] Connect to: " + wifiSsid[number]);

    WiFi.disconnect();
    WiFi.mode(WIFI_MODE_NULL);
    
    WiFi.mode(WIFI_STA);
    // MAC-Adresse holen    
    WiFi.macAddress(mac);

    snprintf(hostname, sizeof(hostname), "clock_%02X%02X%02X",
        mac[3], mac[4], mac[5]);
    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
    WiFi.setHostname(hostname);
    DEBUG_PRINTLN("[WiFi] Hostname set to: " + String(hostname));

    uint16_t waitTime = WAIT_30s; // 30 Sekunden
    if (rtcOk == RTC_AVAILABLE) {
        waitTime = WAIT_15s; // 15 Sekunden
    }
    
    
    DEBUG_PRINTLN("[WiFi] Attempting connection with a timeout of " + String(waitTime / 1000) + " seconds..");
    WiFi.begin(wifiSsid[number].c_str(), wifiPass[number].c_str());
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < waitTime) {
         
        if (loggingEnabled) Serial.print("");
        if (verboseMode) {
            animateCursor(tft, 20, (CLOCK_HEIGHT / 2) + (CLOCK_HEIGHT / 8), 100);
        }
        else {
            updateClock();
            delay(10); // kurze Pause, damit der WLAN-Stack und andere Aufgaben Zeit bekommen
        }

    }
    if (loggingEnabled) Serial.println();

    if (WiFi.status() != WL_CONNECTED) {
        DEBUG_PRINTLN("[WiFi] Connection failed or timed out");       
    } else {
        DEBUG_PRINTLN("[WiFi] Connected successfully");
    }
    
    if (WiFi.status() == WL_CONNECTED) {

        // mDNS initialisieren
        if (!MDNS.begin(hostname)) {
            DEBUG_PRINTLN("[WIFI] Error starting mDNS");
        }
        else {
           MDNS.addService("http", "tcp", 80); // Beispiel: HTTP-Dienst auf Port 80  
        }

        DEBUG_PRINTLN("[WiFi] Connected to: " + wifiSsid[number]);
        DEBUG_PRINTLN("[WiFi] IP address: " + WiFi.localIP().toString());
        String fullHostname = String(hostname) + ".local";
        pingHostname = true;
        //   pingHostname = Ping.ping(fullHostname.c_str(),3);
        //   DEBUG_PRINTF("[mDNS] Ping to %s: %s\n", fullHostname.c_str(), pingHostname ? "success" : "failed");


        String pingServer = preferences.getString(PK_PING_SERVER, "");
        if (pingServer == "") {
            DEBUG_PRINTLN("[WiFi] No ping server configured, skipping internet connectivity check");
            return CONNECTED; // Kein Ping-Server konfiguriert, Verbindung als in Ordnung betrachten
        }
        if (!isInternetReachable(pingServer)) {
            // Setze eine Statusvariable oder führe eine Aktion aus, wenn das Internet nicht erreichbar ist
            DEBUG_PRINTLN("[WiFi] Internet not reachable after connection");
            return CONNECTED_NO_INTERNET;
        }

        if (verboseMode) {
            showWlanCredentials(wifiSsid[number]);           
        }

        if (preferences.getInt(PK_LAST_WLAN, -1) != number) {
            preferences.putInt(PK_LAST_WLAN, number);        
            DEBUG_PRINTLN("[WiFi] set lastWLan: " +  (String)(number + 1));
        }
              
      //  if (!WiFi.softAPgetStationNum()) updateClock();

        return CONNECTED;
    }

    return NOT_CONNECTED;
}


// Prüft die Internet-Konnektivität durch Verbindungsversuch zu einem konfigurierten Server.    
bool isInternetReachable(String pingServer) {

    if (WiFi.status() != WL_CONNECTED) {
        DEBUG_PRINTLN("[PING] WiFi not connected, skipping ping");
        return false;
    }

    WiFiClient client;    
    
    int pingPort = 80; // Standardport

if (pingServer.indexOf(':') != -1) { 
    pingPort = pingServer.substring(pingServer.indexOf(':') + 1).toInt();
    pingServer = pingServer.substring(0, pingServer.indexOf(':'));    
} 
// DEBUG_PRINTLN("[PING] Checking internet connectivity by connect to " + pingServer + ":" + pingPort); client.setTimeout(1);
bool connected = client.connect(pingServer.c_str(), pingPort);
    if (connected) {
        DEBUG_PRINTLN("[PING] Successfully connected to " + pingServer + ":" + pingPort + " internet is reachable");
    } else {
        DEBUG_PRINTLN("[PING] Failed to connect to " + pingServer + ":" + pingPort);
    }

    client.stop();
    return connected;
}


// Animation während Verbindungsversuchen
void animateCursor(TFT_eSPI& tft, int x, int y, int delayMs) {
    tft.setCursor(x, y);    tft.print("/");    delay(delayMs);
    tft.setCursor(x, y);    tft.print("-");    delay(delayMs);
    tft.setCursor(x, y);    tft.print("\\");   delay(delayMs);
    tft.setCursor(x, y);    tft.print("-");    delay(delayMs);
}


// Anzeige WLAN Parameter auf dem TFT
void showWlanCredentials(String wlan) {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);

    tft.setTextSize(TFT_TEXT_SIZE/2);
#if defined(GC9D01)
    tft.setCursor(20, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 3));
#else
    tft.setCursor(60, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 3));
#endif
    tft.println(String(version));

    tft.setTextSize(TFT_TEXT_SIZE);
    tft.setCursor(14, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 4));
    if (WiFi.status() == WL_CONNECTED) {
        tft.println("Connected to SSID" + String(preferences.getInt(PK_LAST_WLAN, -1) + 1));
        tft.setCursor(20, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 8));
        if (wlan.length() > 15) {
            tft.print(wlan.substring(0, 15));
            tft.println("..");
        }
        else tft.println(wlan);
        tft.setCursor(20, (CLOCK_HEIGHT / 2));
        tft.println(WiFi.localIP());

        if (pingHostname) {
            tft.setCursor(20, (CLOCK_HEIGHT / 2) + (CLOCK_HEIGHT / 8));
            tft.println(String(hostname) + ".local");
        }       
    }
    else {
        tft.println("Not connected");
    }
}


// --- Funktion: Löscht die gespeicherten WiFi-Konfigurationen ---
void eraseWiFiConfig() {
    // WLAN trennen und komplett deaktivieren
    WiFi.disconnect(true, true);  // true,true => auch gespeicherte Daten löschen    
    delay(100);
    WiFi.mode(WIFI_OFF);
    delay(WAIT_1s);

    for (int i = 0; i < MAX_WLAN; i++) {
        // Dynamisch berechnete Schlüssel
        String ssidKey = pkSsid(i);
        String passKey = pkPass(i);

        preferences.remove(ssidKey.c_str());
        preferences.remove(passKey.c_str());
    }

    // Zusätzlich: Manuell NVS-Einträge für WiFi löschen
    nvs_handle_t nvsHandle;
    esp_err_t err = nvs_open("wifi", NVS_READWRITE, &nvsHandle);
    if (err == ESP_OK) {
        nvs_erase_all(nvsHandle);
        nvs_commit(nvsHandle);
        nvs_close(nvsHandle);
        DEBUG_PRINTLN("WiFi NVS entries erased");
    }
    else {
        DEBUG_PRINTF("Error opening the NVS WiFi handle: %s\n", esp_err_to_name(err));
    }
}


// --- Funktion: Startet einen asynchronen WiFi-Scan ---
void startWiFiScan() {
    if (!isScanning) {
      
        isScanning = true;

        WiFi.mode(WIFI_STA);
        WiFi.disconnect();
        delay(10);
        DEBUG_PRINTLN("[WiFi] Starting asynchronous scan..");
        WiFi.scanNetworks(true); // Asynchroner Scan
        //delay(250);
    }
}


// --- Funktion: Überprüft den Status des WiFi-Scans und verarbeitet die Ergebnisse ---
void checkWiFiScan() {
    if (isScanning) {
        int scanStatus = WiFi.scanComplete();
        if (scanStatus == WIFI_SCAN_RUNNING) {
            // Scan läuft noch
            // DEBUG_PRINTLN("[WiFi] Scan in progress..");
        }
        else if (scanStatus >= 0) {
            // Scan abgeschlossen

            // Vorherige Ergebnisse löschen
            for (int i = 0; i < MAX_WLAN; i++) {
                availableNetworks[i].ssid = "";  
                availableNetworks[i].rssi = 0;
                availableNetworks[i].enc = 0;
            }

            int16_t n = scanStatus; 
            DEBUG_PRINTLN("[WiFi] found " + String(n) + " WiFi networks");
            if (n > MAX_WLAN) {
                if (loggingEnabled) DEBUG_PRINTLN("[WiFi] here the best " + String(MAX_WLAN) + " networks:");            
                n = MAX_WLAN;
            }
           
            for (int i = 0; i < n; i++) {
                availableNetworks[i].ssid = WiFi.SSID(i);
                availableNetworks[i].rssi = WiFi.RSSI(i);
                availableNetworks[i].enc = WiFi.encryptionType(i);
                foundNetworkCount++;
                if (loggingEnabled) {
                    DEBUG_PRINTLN("  [WiFi] " +
                        availableNetworks[i].ssid + " (" +
                        String(availableNetworks[i].rssi) + " dBm) " +
                        (availableNetworks[i].enc == WIFI_AUTH_OPEN ? "Open" : "Secured"));
                }

            }
            WiFi.scanDelete(); // Ergebnisse löschen
            isScanning = false;
            DEBUG_PRINTLN("[WiFi] done");
        }
        else {
            // Fehler beim Scan
            DEBUG_PRINTLN("[WiFi] Scan failed with error: " + String(scanStatus));
            isScanning = false;
            //scanAndCacheNetworks();
        }
    }
}


// --- Funktion: Scannt verfügbare WLANs und speichert sie im Cache ---
void scanAndCacheNetworks() {
    
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextSize(TFT_TEXT_SIZE);
    tft.setCursor(10, (CLOCK_HEIGHT / 2) - (CLOCK_HEIGHT / 8));
    tft.println("WLAN-Scan..");

    DEBUG_PRINTLN("[WiFi] Scanning for WiFi networks..");
#ifdef LED_BOARD
 
#endif
    int networkCount = WiFi.scanNetworks();
    DEBUG_PRINTLN("[WiFi] found " + String(networkCount) + " WiFi networks:");    
    if (networkCount > MAX_WLAN) {
        DEBUG_PRINTLN("[WiFi] here the best " + String(MAX_WLAN) + " networks:");
        networkCount = MAX_WLAN;
    }

    foundNetworkCount = 0;
    for (int i = 0; i < networkCount; i++) {
        availableNetworks[i].ssid = WiFi.SSID(i);
        availableNetworks[i].rssi = WiFi.RSSI(i);
        availableNetworks[i].enc = WiFi.encryptionType(i);
        foundNetworkCount++;

        DEBUG_PRINTLN("  [WiFi] " + availableNetworks[i].ssid +
                      " (" + String(availableNetworks[i].rssi) + " dBm) " + 
                      (availableNetworks[i].enc == WIFI_AUTH_OPEN ? "Open" : "Secured"));
    }    

    WiFi.scanDelete(); // Ergebnisse löschen
    DEBUG_PRINTLN("[WiFi] done");
  
}


