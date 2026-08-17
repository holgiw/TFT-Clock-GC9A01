1. Support for Multiple TFT Displays
•	The clock supports various TFT displays:
•	GC9A01
•	GC9D01
•	ILI9341 (deprecated - no longer actively maintained)
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
•	Supports up to 15 WiFi networks.
•	Scanning for networks shows signal strength and whether each network is open or secured.
•	Hostname and all WiFi network settings live on their own WLAN tab on the main settings page - on first boot or whenever the last known network is unavailable, that tab opens automatically.
•	Automatically reconnects if the connection is lost (configurable).
•	Individual networks can be deleted directly from the web interface - remaining entries automatically move up to close the gap.
•	Add a new network via WPS with a single button on the web interface - no need to know the SSID/password in advance.
•	Customizable hostname (invalid characters are automatically cleaned up); falls back to an automatically generated name based on the MAC address if left empty.
•	NTP (Network Time Protocol):
•	Automatically synchronizes time with an NTP server.
•	Nightly time synchronization at 02:00 and 03:00.
---
6. Web Interface
•	Redesigned dark-themed, single-page settings hub: Status, WLAN, Clock Setup, Brightness and Time/NTP are now tabs on one page (no more clicking through separate pages for basic settings).
•	File-heavy management screens (Presets, Clock Face, Hand Set, File Manager, Live Preview, Factory Reset) remain separate pages, reachable from the navigation bar.
•	A built-in web interface allows:
•	Uploading, downloading, and managing clock faces and hands.
•	Adjusting brightness, time zone, and display rotation.
•	Enabling/disabling Smooth Minute and Train Station modes.
•	Viewing system status (e.g., WiFi details, storage usage, uptime).
•	Multi-language interface (German, English, French).
•	Factory reset page with separate options: reset everything, delete clock faces (except default), delete hand sets (except default), delete presets, or reset saved WiFi networks - each with its own confirmation.
•	Download additional clock faces, hand sets, and presets directly from GitHub via the web interface - missing dependencies (e.g. a face/hand set referenced by a preset) are fetched automatically.
•	Live preview (top-left corner on every page) always shows the currently active clock face/hands/hub color - click it (with confirmation) to save the current look as a new preset.
---
7. File Management with LittleFS
•	Uses the LittleFS filesystem to:
•	Store custom clock faces and hands, RLE-compressed to save flash space.
•	Manage files (e.g., upload, download, rename, delete) with a compact icon-based interface.
• Logging
---
8. Time Zone Customization
•	Supports various time zones:
•	Automatic daylight saving time (e.g. CET/CEST).
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
• API Interface
• Up to 50 presets (face, hand set, hub color/size, second-hand display) - individually renameable and deletable, sorted alphabetically in the list; back up all presets to a file and restore them later; a warning is shown once all 50 slots are full.
• Deleting a clock face or hand set automatically removes any presets that referenced it; deleting the currently active hand set automatically falls back to the built-in default.
• DCF77 supported



