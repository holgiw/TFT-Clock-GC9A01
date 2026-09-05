#pragma once
    // ### Forward-Deklarationen aller Funktionen #########################
    // ### Forward declarations of all functions #########################
    // Ersetzt die automatische Prototyp-Generierung der Arduino-IDE (scannt nur die .ino), da der Sketch auf mehrere .h-Dateien aufgeteilt ist -
    // stellt sicher, dass jede Funktion unabhaengig von der #include-Reihenfolge aufrufbar ist. Sortiert wie die jeweilige .h-Datei.

    // Replaces the Arduino IDE's automatic prototype generation (which only scans the .ino), since the sketch is split across multiple .h files -
    // ensures every function is callable regardless of #include order. Sorted like the respective .h file.


    // --- wifi_manager.h: WLAN: Verbindungsaufbau, Access-Point, Scan, Reconnect ---
    // --- wifi_manager.h: WiFi: connection setup, access point, scan, reconnect ---

    void startWPS() ;
    bool checkWiFiReconnect() ;
    int saveWpsCredentials(const String& ssid, const String& pass) ;
    void onWpsEvent(WiFiEvent_t event) ;
    void startAP() ;
    int connectWiFi(int number, bool verboseMode) ;
    bool isInternetReachable(String pingServer) ;
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


    // --- time_sync.h: Zeit: RTC, DCF77, NTP-Client & -Server, Zeitzone ---
    // --- time_sync.h: Time: RTC, DCF77, NTP client & server, timezone ---

    void IRAM_ATTR isr() ;
    void loadTimeFromRTC() ;
    void initializeNtpServers() ;
    struct tm dcf77DecodedToLocalTm() ;
    bool updateDcf77Status() ;
    bool applyDcf77DecodedTime(String source) ;
    void checkDcf77Health() ;
    void decodeDcf77Telegram() ;
    void processDcf77Bits() ;
    void checkRtcHealth() ;
    String testNtpServer(const String& server) ;
    boolean setupNTP() ;
    void handleNTPFailure() ;
    void setTimeStruct(const struct tm& timeinfo, String source) ;
    uint16_t i2cScan() ;
    void createNtpResponse(byte* packet, time_t currentTime) ;


    // --- display.h: Display: Zifferblatt, Zeiger, Sprites, Helligkeit, Touch ---
    // --- display.h: Display: clock face, hands, sprites, brightness, touch ---

    void* preferPsramMalloc(size_t size) ;
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
    void loadClockFace(uint8_t rotation = tftRotation1) ; // ohne Argument = Rotation von Display 1 (Standardverhalten fuer alle Aufrufer ausserhalb von renderClockFrame())
                                                          // no argument = display 1's rotation (default behaviour for every caller outside renderClockFrame())
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
    void renderClockFrame(uint8_t displayNum, uint8_t rotation, float& lastHourAngleRef, float& lastMinuteAngleRef, bool& firstRunRef) ;
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


    // --- presets_manager.h: Presets: Laden/Speichern/Wechseln vordefinierter Anzeigekonfigurationen ---
    // --- presets_manager.h: Presets: load/save/switch predefined display configurations ---

    String stripRotationParam(const String& url) ;
    void loadPresets() ;
    void savePresets() ;
    bool createPresetFromPreferences(const String& customName = "") ;
    void parsePresetForPreview(const String& url, String& faceOut, String& handSetOut, uint16_t& hubColorOut, uint8_t& hubSizeOut, bool& showSecondOut) ;
    void removeOrphanedPresets(const String& deletedFace, const String& deletedHandSet) ;
    void resetAllPresets() ;
    void switchToNextPreset() ;


    // --- prefs_keys.h / wifi_manager.h: verifiziertes Preferences-Schreiben ---
    // --- prefs_keys.h / wifi_manager.h: verified Preferences writing ---
    // Schreibt einen String in die Preferences und liest ihn sofort wieder aus,
    // um einen fehlgeschlagenen Schreibvorgang (z.B. durch vollen NVS-Namespace) zu erkennen,
    // statt ihn erst nach einem Neustart als "Eintrag verschwunden" zu bemerken.

    // Writes a string to Preferences and immediately reads it back,
    // to detect a failed write (e.g. due to a full NVS namespace)
    // instead of only noticing it as a "missing entry" after a restart.

    bool putStringVerified(const char* key, const String& value) ;


    // --- webserver_routes.h: Webinterface: alle HTTP-Routen & HTML-Generierung ---
    // --- webserver_routes.h: Web interface: all HTTP routes & HTML generation ---

    String generateHtmlHeader(String extraHead = "") ;
    String simpleMessagePage(String heading, String bodyHtml, String extraHead = "") ;
    String generateTopBar() ;
    String getTimeStatus() ;
    String getRtcStatus() ;
    String getDcf77Status() ;
    String dotStatusText(const String& label, const String& state) ;
    String escapeHtmlText(const String& text) ;
    String generateStorageInfo(size_t used, size_t total, bool forceEnglish = false) ;
    String generateFlashMessage() ;
    String generateNavigation() ;
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


    // --- system_utils.h: Systemfunktionen: Tasten, Logging, Reset, Neustart, Hilfsfunktionen ---
    // --- system_utils.h: System functions: buttons, logging, reset, restart, helper functions ---

    void checkButton() ;
    void checkWeeklyRestart() ;
    void eraseAllNVS() ;
    void factoryReset() ;
    void espReboot() ;
    String getCurrentLogFileName() ;
    void deleteAllLogFiles() ;
    void checkHeapWarning(const String& context) ;
    void logToFile(const String& message) ;
    String trim(const String& str) ;


    // --- uhr3.ino: setup() & loop() ---

    void setup() ;
    void loop() ;
