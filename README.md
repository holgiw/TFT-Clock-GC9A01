1. Support for Multiple TFT Displays
•	The clock supports various TFT displays:
•	GC9A01
•	GC9D01
•	ILI9341
•	Display resolution and size are dynamically adjusted (e.g., 240x240 or 160x160 pixels).
---
2. Customizable Clock Hands and Faces
•	Clock Hands:
•	Custom hour, minute, and second hands can be uploaded as BMP files.
•	Default hands are included as a fallback.
•	Clock Faces:
•	Custom clock faces can be uploaded and selected.
•	A built-in default clock face is available.
---
3. Smooth Minute and Train Station Modes
•	Smooth Minute Mode:
•	The minute hand moves smoothly instead of jumping in 1-minute increments.
•	Train Station Mode:
•	The second hand stops briefly at 60 seconds, mimicking a classic train station clock.
---
4. Brightness Control
•	Automatic brightness adjustment using a photoresistor.
•	Configurable thresholds for minimum and maximum brightness.
•	Manual brightness control is available if no photoresistor is detected.
---
5. WiFi and NTP Integration
•	WiFi:
•	Supports two WiFi networks (primary and secondary).
•	Automatically reconnects if the connection is lost.
•	NTP (Network Time Protocol):
•	Automatically synchronizes time with an NTP server.
•	Nightly time synchronization at 02:00 and 03:00.
---
6. Web Interface
•	A built-in web interface allows:
•	Uploading and managing clock faces and hands.
•	Adjusting brightness, time zone, and display rotation.
•	Enabling/disabling Smooth Minute and Train Station modes.
•	Viewing system status (e.g., WiFi details, storage usage, uptime).
---
7. File Management with LittleFS
•	Uses the LittleFS filesystem to:
•	Store custom clock faces and hands.
•	Manage files (e.g., upload, rename, delete).
---
8. Time Zone Customization
•	Supports various time zones:
•	Automatic daylight saving time (e.g., CET/CEST).
•	Permanent summer or winter time.
---
9. Hardware Integration
•	Compatible only with ESP32-S2.
•	Photoresistor for brightness measurement.
---
10. Advanced Features
•	Uptime Display: Shows the clock's runtime since the last restart.
•	Reboot Function: Allows restarting the clock via the web interface.
•	BMP Scaling: Uploaded BMP files can be scaled to fit the display size.
