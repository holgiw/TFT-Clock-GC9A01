#pragma once
    // Forward-Deklarationen aller Funktionen - ersetzt Arduino IDEs automatische Prototyp-Generierung
    // (scannt nur die .ino), da der Sketch auf mehrere .h-Dateien aufgeteilt ist.

    // Forward declarations of all functions - replaces the Arduino IDE's automatic prototype generation
    // (which only scans the .ino), since the sketch is split across multiple .h files.


    // wifi_manager.h: WLAN: Verbindungsaufbau, Access-Point, Scan, Reconnect
    // wifi_manager.h: WiFi: connection setup, access point, scan, reconnect

    void startWPS() ;
    bool checkWiFiReconnect() ;
    int saveWpsCredentials(const String& ssid, const String& pass) ;
    void onWpsEvent(WiFiEvent_t event) ;
    void restorePreviousWpsConnection() ;
    void startAP() ;
    int connectWiFi(int number, bool verboseMode) ;
    int connectWiFiWithRetries(int number, const String& label, bool verboseMode = true) ; // ruft connectWiFi() bis zu WIFI_CONNECT_ATTEMPTS mal auf (siehe wifi_manager.h)
                                                                                            // calls connectWiFi() up to WIFI_CONNECT_ATTEMPTS times (see wifi_manager.h)
    void animateCursor(int x, int y, int delayMs) ;
    void showWlanCredentials(String wlan) ;
    void eraseWiFiConfig() ;
    void startWiFiScan() ;
    void collectStrongestNetworks(int totalFound) ;
    void checkWiFiScan() ;
    void scanAndCacheNetworks() ;

    // In uhr3.ino definiert (nicht in wifi_manager.h), da sie den kompletten
    // Boot-Ablauf des WLAN-Aufbaus kapselt - siehe Kommentar dort.

    // Defined in uhr3.ino (not in wifi_manager.h), since it encapsulates the
    // whole boot-time WiFi setup flow - see the comment there.
    void connectWiFiAtBoot() ;


    // time_sync.h: Zeit: RTC, DCF77, NTP-Client & -Server, Zeitzone
    // time_sync.h: Time: RTC, DCF77, NTP client & server, timezone

    void IRAM_ATTR isr() ;
    void logTimeSyncDifference(const String& source, const struct timeval& oldTime, unsigned long oldTimeMillis) ; // loggt Alt-/Neu-Zeitdifferenz nach einer Synchronisation (siehe time_sync.h)
                                                                                                                  // logs old/new time difference after a sync (see time_sync.h)
    void loadTimeFromRTC() ;
    void applyNtpServerDefaultsIfNoneConfigured() ; // Fallback auf NTP_SERVER_1/2, wenn ntpServers[] komplett leer ist (siehe time_sync.h)
                                                    // fallback to NTP_SERVER_1/2 when ntpServers[] is completely empty (see time_sync.h)
    void applyTimezoneDefaultIfInvalid() ; // Fallback auf TIMEZONE_DEFAULT, wenn `timezone` leer oder ungueltig ist (siehe time_sync.h)
                                           // fallback to TIMEZONE_DEFAULT when `timezone` is empty or invalid (see time_sync.h)
    bool isValidPosixTimezone(const String& tz) ; // Grammatik-/Wertebereichspruefung eines POSIX-TZ-Strings (siehe time_sync.h)
                                                  // grammar/range check of a POSIX TZ string (see time_sync.h)
    void initializeNtpServers() ;
    struct tm dcf77DecodedToLocalTm() ;
    long rtcDriftSec(struct tm newLocal) ; // Abweichung der RTC-Zeit von newLocal in Sekunden (siehe time_sync.h)
                                           // deviation of the RTC time from newLocal in seconds (see time_sync.h)
    bool updateDcf77Status() ;
    bool applyDcf77DecodedTime(String source) ;
    void checkDcf77Health() ;
    bool decodeDcf77Telegram(unsigned long decodedAtMillis) ; // false = Telegramm strukturell unmoeglich, Synchronisation verwerfen (siehe processDcf77Bits())
                                                              // false = telegram structurally impossible, drop synchronization (see processDcf77Bits())
    void resetDcf77MarkerStats() ;
    void evaluateDcf77Marker() ;
    void processDcf77Bits() ;
    void checkRtcHealth() ;
    String testNtpServer(const String& server) ;
    boolean setupNTP() ; // blockierender Worker - nicht direkt aufrufen, siehe startNtpSyncTask()
                        // blocking worker - do not call directly, see startNtpSyncTask()
    void ntpSyncTaskFunc(void* param) ;
    void startNtpSyncTask(String label) ;
    void pollNtpSyncTask() ;
    void stopNtpSyncTaskIfRunning() ; // hartes Beenden vor Reboot/Werksreset - siehe system_utils.h
                                      // hard stop before reboot/factory reset - see system_utils.h
    void handleNTPFailure() ;
    void setTimeStruct(const struct tm& timeinfo, String source) ;
    uint16_t i2cScan() ;
    bool startNtpServer() ;
    void createNtpResponse(byte* packet, const struct timeval& receivedAt) ;


    // display.h: Display: Zifferblatt, Zeiger, Sprites, Helligkeit, Touch
    // display.h: Display: clock face, hands, sprites, brightness, touch

    void* preferPsramMalloc(size_t size) ;
    bool isDisplayConnected(uint8_t displayNum) ;
    uint8_t primaryDisplayRotation() ;
    uint8_t effectiveRotation(uint8_t displayNum) ;
    void setCSIdle() ;
    void applyDisplayRotation(uint8_t displayNum, uint8_t newRotation) ;
    void setCS1(bool state) ;
    void setCS2(bool state) ;
    TFT_eSPI& beginStatusDraw(uint8_t displayNum) ;
    void endStatusDraw(uint8_t displayNum) ;
    uint16_t setPixelBrightness(uint16_t pixel) ;
    bool isRleFace(const uint8_t* header4) ;
    size_t rleMaxEncodedSize(size_t pixelCount) ;
    size_t rleEncode565(const uint16_t* pixels, size_t count, uint8_t* out) ;
    void rleDecode565(const uint8_t* in, size_t inSize, uint16_t* out, size_t outCount) ;
    void rleDecode565ToBmpRows(const uint8_t* in, size_t inSize, uint8_t* pixelArea, int width, int height, int rowStride) ;
    bool loadFaceBmpInto(const String& path, uint16_t* dest, int32_t expectedW, int32_t expectedH) ;
    void loadClockFace(uint8_t rotation = primaryDisplayRotation()) ; // ohne Argument = Rotation des ersten angeschlossenen Displays (Standardverhalten fuer alle Aufrufer ausserhalb von renderClockFrame())
                                                                      // no argument = rotation of the first connected display (default behaviour for every caller outside renderClockFrame())
    void freeClockFaceBuffer() ;
    void resetFacesToDefault() ;
    void resetHandsToDefault() ;
    void pushHandRowCentered(TFT_eSprite* sprite, int row, uint16_t* rowPixels, int srcWidth, const uint8_t* transparentColor) ;
    void loadHandSprites() ;
    bool loadHandPixelsForPreview(const char* filename, uint16_t* outBuffer, int width, int height) ;
    bool loadHandBmp(TFT_eSprite* sprite, const char* filename, int width, int height) ;
    float shortestAngleDiff(float from, float to) ;
    int prepareClockFaceCache() ;
    int faceOrientationFor(uint8_t rotation) ;
    bool blitFaceIntoBuffer(uint16_t* dest, uint8_t rotation) ;
    void blitHandAntiAliased(uint16_t* canvas, TFT_eSprite* handSprite, float angleDeg) ;
    bool buildHandComposite(HandComposite& comp, uint8_t rotation, float hourAngle, float minuteAngle) ;
    bool drawCompositeInto(uint8_t displayNum, uint8_t rotation, float hourAngle, float minuteAngle) ;
    void renderClockFrame(uint8_t displayNum, uint8_t rotation, float& lastHourAngleRef, float& lastMinuteAngleRef, float& lastSecondAngleRef, bool& firstRunRef) ;
    void updateClock() ;
    void updateBrightness() ;
    uint16_t getAdjustedAdcValue(int rawValue) ;
    float easeInOutSine(float t) ;
    uint32_t crc32Update(uint32_t crc, const uint8_t* buf, size_t len) ;
    uint32_t adler32(const uint8_t* data, size_t len) ;
    void appendPngChunk(std::vector<uint8_t>& out, const char* type, const uint8_t* data, uint32_t len) ;
    String encodePngToBase64(const uint16_t* data, int width, int height) ;
    uint8_t* encodeBmpToBytes(const uint16_t* data, int width, int height, size_t* outSize) ;
    String encodeBmpToBase64(const uint16_t* data, int width, int height) ;
    void clearTFT() ;
    float rotatedAngle(float angle, int orientation) ;
    bool checkBmpFormat(const String& filename, int expectedWidth = CLOCK_WIDTH, int expectedHeight = CLOCK_HEIGHT) ;
    String getBmpInfo(const String& filename) ;
    bool scaleAndSaveBmp(const char* sourcePath, const char* targetPath, int outW, int outH) ;
    void migrateFaceBmpsToRLE() ;
    void migrateHandBmpsToRLE() ;
    bool peekFirstPixelIsWhite(const String& path) ;
    void remaskExistingFaceCorners() ;
    void sendScaledBmpPreview(const String& sourcePath, int outW, int outH) ;
    bool streamRleFaceAsStandardBmp(const String& path, const char* contentType = "image/bmp") ;
    void blitRotatedHand(uint16_t* canvas, int canvasW, int canvasH, const uint16_t* hand, int handW, int handH, float pivotX, float pivotY, float cx, float cy, float angleDeg, float scale) ;
    bool generatePresetPreviewBmp(const String& faceFile, const String& handSetName, uint16_t hubColorRgb565, uint8_t hubSize, bool showSecond, uint8_t** outBytes, size_t& outSize) ;
    void setLedOff() ;
    void setLedOn() ;
    void toggleLED() ;
    void checkTouchInput() ;
    static void validateSelectedBackground() ;
    void updateHandWidths(int newHourWidth, int newMinuteWidth, int newSecondWidth) ;
    void parseBackgroundFilename(const String& filename, int& hourWidth, int& minuteWidth, int& secondWidth) ;
    void enableTouch() ;
    void disableTouch() ;


    // rocrail_client.h: Rocrail-Modellzeit: TCP-Verbindung (Serveradresse
    // verpflichtend), XML-Auswertung des <clock/>-Events, Fortschreiben der
    // (ggf. beschleunigten) Modellzeit zwischen zwei Server-Updates.

    // rocrail_client.h: Rocrail model time: TCP connection (server address
    // mandatory), XML parsing of the <clock/> event, advancing the
    // (possibly accelerated) model time between two server updates.

    void pollRocrailClient() ;
    void connectRocrailClient() ;
    void triggerRocrailConnectNow() ;
    void startRocrailConnectTask() ;
    void loadRocrailServerList() ;
    void rocrailConnectTaskFunc(void* param) ;
    void pollRocrailConnectTask() ;
    void processRocrailBuffer() ;
    void processRocrailClockPayload(const String& payload) ;
    void advanceRocrailTime() ;
    int rocrailXmlAttrInt(const String& tag, const char* attr, int fallback) ;
    String rocrailXmlAttrString(const String& tag, const char* attr) ;
    bool shouldLogThrottled(uint8_t& counter, bool& isLastLogged, uint8_t limit = ROCRAIL_LOG_THROTTLE_LIMIT) ; // Drosselung wiederkehrender Log-Zeilen (siehe rocrail_client.h)
                                                                                                                 // throttling for recurring log lines (see rocrail_client.h)

    // R2RNet-Multicast-Diagnose (siehe rocrail_client.h) - keine echte
    // Discovery, nur Logging eingehender Pakete zur Formatanalyse.
    // R2RNet multicast diagnostics (see rocrail_client.h) - not real
    // discovery, just logging incoming packets for format analysis.
    bool startR2rnetDebugListener() ;
    void pollR2rnetDebugListener() ;


    // presets_manager.h: Presets: Laden/Speichern/Wechseln vordefinierter Anzeigekonfigurationen
    // presets_manager.h: Presets: load/save/switch predefined display configurations

    String stripRotationParam(const String& url) ;
    void loadPresets() ;
    void savePresets() ;
    bool createPresetFromPreferences(const String& customName = "") ;
    void parsePresetForPreview(const String& url, String& faceOut, String& handSetOut, uint16_t& hubColorOut, uint8_t& hubSizeOut, bool& showSecondOut) ;
    void removeOrphanedPresets(const String& deletedFace, const String& deletedHandSet) ;
    void resetAllPresets() ;
    void switchToNextPreset() ;


    // prefs_keys.h / wifi_manager.h: verifiziertes Preferences-Schreiben
    // prefs_keys.h / wifi_manager.h: verified Preferences writing

    // Schreibt einen String in die Preferences und liest ihn sofort wieder aus,
    // um einen fehlgeschlagenen Schreibvorgang (z.B. durch vollen NVS-Namespace) zu erkennen,
    // statt ihn erst nach einem Neustart als "Eintrag verschwunden" zu bemerken.

    // Writes a string to Preferences and immediately reads it back,
    // to detect a failed write (e.g. due to a full NVS namespace)
    // instead of only noticing it as a "missing entry" after a restart.

    bool putStringVerified(const char* key, const String& value) ;


    // webserver_routes.h: Webinterface: alle HTTP-Routen & HTML-Generierung
    // webserver_routes.h: Web interface: all HTTP routes & HTML generation

    void applyWlanList(String newSsid[MAX_WLAN], String newPass[MAX_WLAN]) ; // schreibt eine komplette WLAN-Liste in Preferences + RAM (siehe webserver_routes.h)
    void executePendingAction(String action) ; // fuehrt eine der acht bestaetigten/direkt erlaubten Aktionen aus (siehe webserver_routes.h)
                                               // executes one of the eight confirmed/directly-allowed actions (see webserver_routes.h)
                                                                             // writes a complete WiFi list to preferences + RAM (see webserver_routes.h)
    String generateHtmlHeader(String extraHead = "") ;
    String simpleMessagePage(String heading, String bodyHtml, String extraHead = "") ;
    String generateTopBar() ;
    String getTimeStatus() ;
    String getRtcStatus() ;
    String getDcf77Status() ;
    String dotStatusText(const String& label, const String& state) ;
    bool isSameSubnet(IPAddress ip, IPAddress ownIp, IPAddress mask) ; // byteweiser Subnetzvergleich (siehe webserver_routes.h)
                                                                       // byte-wise subnet comparison (see webserver_routes.h)
    bool isPrivateNetworkIp(IPAddress ip) ; // im selben Netz wie die Uhr (STA oder AP) oder RFC1918-Rueckfall? (siehe webserver_routes.h) - blendet den Status-Tab bei externem Zugriff aus
                                            // in the same network as the clock (STA or AP) or RFC1918 fallback? (see webserver_routes.h) - hides the Status tab on external access
                                            // RFC1918 range? (see webserver_routes.h) - hides the Status tab on external access
    String escapeHtmlText(const String& text) ;
    String fileManagerReturnTarget(const String& from) ; // /delete- und /rename-Rueckspring-Ziel anhand des "from"-Parameters (siehe webserver_routes.h)
                                                         // /delete and /rename return target based on the "from" parameter (see webserver_routes.h)
    String escapeJsonText(const String& text) ;
    String generateStorageInfo(size_t used, size_t total, bool forceEnglish = false) ;
    String generateFlashMessage() ;
    String generateNavigation() ;
    String currentPreviewSignature() ;
    String generateSettingsTabNav() ;
    String generateLanguageSelector() ;
    String resetReasonToString(esp_reset_reason_t reason) ;
    String rtcStatusToString(int status) ;
    String formatDurationMs(unsigned long ms) ;
    bool naturalLess(const String& a, const String& b) ;
    void naturalSortNames(std::vector<String>& names) ;
    void redirectTo(const String& location, const String& body = "") ;
    String beginPage() ;
    void updateNtpServersFromRequest() ;
    String sanitizeHostname(String input) ;
    void setupWebServer() ;
    void handleFileUpload() ;
    bool validateAndFixPresetFace(String& url, const std::vector<String>& existingFaces) ;
    void handlePresetImportUpload() ;
    void handlePresetMergeUpload() ;


    // system_utils.h: Systemfunktionen: Tasten, Logging, Reset, Neustart, Hilfsfunktionen
    // system_utils.h: System functions: buttons, logging, reset, restart, helper functions

    void checkButton() ;
    String factoryResetActionLabel(const String& action) ; // kurze Beschriftung fuer Display + Web-Eingabeseite (siehe system_utils.h)
                                                            // short label for display + web entry page (see system_utils.h)
    void requestConfirmationCode(const String& action) ; // erzeugt/zeigt einen neuen Bestaetigungscode (siehe system_utils.h)
    bool rejectIfConfirmationPending() ; // true + Hinweisseite, wenn schon ein anderer Code aussteht - vor pendingWifi*-Zuweisungen aufrufen (siehe system_utils.h)
                                        // true + hint page if a different code is already pending - call before pendingWifi* assignments (see system_utils.h)
                                                          // generates/shows a new confirmation code (see system_utils.h)
    bool checkFactoryResetCodePending() ; // siehe system_utils.h - true, waehrend ein Bestaetigungscode angezeigt wird
                                         // see system_utils.h - true while a confirmation code is being displayed
    void checkWeeklyRestart() ;
    void eraseAllNVS() ;
    void factoryReset() ;
    void espReboot() ;
    String getCurrentLogFileName() ;
    void deleteAllLogFiles() ;
    void checkHeapWarning(const String& context) ;
    void logToFile(const String& message) ;
    void flushLogBuffer() ;
    void checkLogFlush() ;
    String trim(const String& str) ;
    bool getSmoothSecondPref(bool stationModeFallback) ; // liest PK_SMOOTH_SECOND mit Migrations-Fallback auf stationMode (siehe system_utils.h)
                                                          // reads PK_SMOOTH_SECOND with a migration fallback to stationMode (see system_utils.h)


    // uhr3.ino: setup() & loop()

    void setup() ;
    void loop() ;

