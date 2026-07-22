#pragma once
    // ### Forward-Deklarationen aller Funktionen #########################
    // Ersetzt die automatische Prototyp-Generierung der Arduino-IDE (scannt nur die .ino), da der Sketch auf mehrere .h-Dateien aufgeteilt ist -
    // stellt sicher, dass jede Funktion unabhaengig von der #include-Reihenfolge aufrufbar ist. Sortiert wie die jeweilige .h-Datei.

    // --- wifi_manager.h: WLAN: Verbindungsaufbau, Access-Point, Scan, Reconnect ---
    void startWPS() ;
    void scanWPS() ;
    bool checkWiFiReconnect() ;
    int saveWpsCredentials(const String& ssid, const String& pass) ;
    void onWpsEvent(WiFiEvent_t event) ;
    void startAP() ;
    int connectWiFi(int number, bool verboseMode) ;
    bool isInternetReachable(String pingServer) ;
    void animateCursor(TFT_eSPI& tft, int x, int y, int delayMs) ;
    void showWlanCredentials(String wlan) ;
    void eraseWiFiConfig() ;
    void startWiFiScan() ;
    void checkWiFiScan() ;
    void scanAndCacheNetworks() ;

    // --- time_sync.h: Zeit: RTC, DCF77, NTP-Client & -Server, Zeitzone ---
    void IRAM_ATTR isr() ;
    void loadTimeFromRTC() ;
    void initializeNtpServers() ;
    bool getDCF77Time() ;
    bool isDaylightSavingTime(struct tm* timeinfo) ;
    void printTime(time_t rawTime) ;
    void checkNightlyTimeSync() ;
    String testNtpServer(const String& server) ;
    boolean setupNTP() ;
    void handleNTPFailure() ;
    void setTimeStruct(const struct tm& timeinfo, String source) ;
    void scheduleNTPRetry() ;
    void checkNTPRetry() ;
    uint16_t i2cScan() ;
    void createNtpResponse(byte* packet, time_t currentTime) ;

    // --- display.h: Display: Zifferblatt, Zeiger, Sprites, Helligkeit, Touch ---
    void setCS1(bool state) ;
    void setCS2(bool state) ;
    uint16_t setPixelBrightness(uint16_t pixel) ;
    bool isRleFace(const uint8_t* header4) ;
    size_t rleMaxEncodedSize(size_t pixelCount) ;
    size_t rleEncode565(const uint16_t* pixels, size_t count, uint8_t* out) ;
    void rleDecode565(const uint8_t* in, size_t inSize, uint16_t* out, size_t outCount) ;
    void rleDecode565ToBmpRows(const uint8_t* in, size_t inSize, uint8_t* pixelArea, int width, int height, int rowStride) ;
    bool loadFaceBmpInto(const String& path, uint16_t* dest, int32_t expectedW, int32_t expectedH) ;
    void loadClockFace() ;
    void freeClockFaceBuffer() ;
    void resetFacesToDefault() ;
    void resetHandsToDefault() ;
    void loadHandSprites() ;
    bool loadHandBmp(TFT_eSprite* sprite, const char* filename, int width, int height) ;
    float shortestAngleDiff(float from, float to) ;
    void updateClock() ;
    void updateBrightness() ;
    uint16_t getAdjustedAdcValue(int rawValue) ;
    float easeInOutSine(float t) ;
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
    void toggleLedDcf77() ;
    void switchToNextBackground() ;
    void checkTouchInput() ;
    static void validateSelectedBackground() ;
    void updateHandWidths(int newHourWidth, int newMinuteWidth, int newSecondWidth) ;
    void parseBackgroundFilename(const String& filename, int& hourWidth, int& minuteWidth, int& secondWidth) ;
    void enableTouch() ;
    void disableTouch() ;

    // --- presets_manager.h: Presets: Laden/Speichern/Wechseln vordefinierter Anzeigekonfigurationen ---
    String stripRotationParam(const String& url) ;
    void loadPresets() ;
    void savePresets() ;
    bool createPresetFromPreferences(const String& customName = "") ;
    void parsePresetForPreview(const String& url, String& faceOut, String& handSetOut, uint16_t& hubColorOut, uint8_t& hubSizeOut, bool& showSecondOut) ;
    void removeOrphanedPresets(const String& deletedFace, const String& deletedHandSet) ;
    void resetAllPresets() ;
    void switchToNextPreset() ;

    // --- webserver_routes.h: Webinterface: alle HTTP-Routen & HTML-Generierung ---
    String generateHtmlHeader() ;
    String generateHtmlStatus() ;
    String generateFlashMessage() ;
    String generateNavigation() ;
    String generateLanguageSelector() ;
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

    // --- system_utils.h: Systemfunktionen: Tasten, Logging, Reset, Neustart, Hilfsfunktionen ---
    void checkButton() ;
    void checkWeeklyRestart() ;
    void eraseAllNVS() ;
    void factoryReset() ;
    void espReboot() ;
    void deleteAllLogFiles() ;
    void checkHeapWarning(const String& context) ;
    void logToFile(const String& message) ;
    String trim(const String& str) ;

    // --- uhr3.ino: setup() & loop() ---
    void setup() ;
    void loop() ;
