## 1. Support for Multiple TFT Displays

- The clock supports various TFT displays:
  - GC9A01
  - GC9D01
  - ILI9341 (deprecated - no longer actively maintained)
- Second, identical display always active: drives a second display of the same type via its own chip-select pin (CS2) on the shared SPI bus - each display gets its own, independently configurable rotation (Rotation Display 1 / Rotation Display 2), so the two can be mounted in different orientations. Status/boot messages (startup, reboot, SSID, WPS, factory reset, ...) are shown on both displays, each correctly rotated to match that display's own orientation.

---

## 2. Customizable Clock Hands and Faces

- Clock Hands:
  - Custom hour, minute, and second hands can be uploaded as BMP files.
  - Default hands are included as a fallback.
  - Anti-aliased hour and minute hands: both are rendered with 3x3 supersampling and per-pixel coverage blending instead of the library's nearest-neighbour rotation, which removes the stair-stepping on their edges. They are drawn into a cached composite image that is only rebuilt when an angle actually changed, so this costs no time in the frames where only the sweeping second hand moves - the per-frame work is in fact lower than before.
- Clock Faces:
  - Custom clock faces can be uploaded and selected.
  - A built-in default clock face is available.

---

## 3. Smooth Minute and Train Station Modes

- Smooth Minute Mode:
  - The minute hand moves smoothly instead of jumping in 1-minute increments.
- Train Station Mode:
  - The second hand steps around in 60 steps, accelerating and braking within each step the way older station clocks did, completes its round in 58.5 seconds and then rests at the top on the 12 for the remaining ~1.5 seconds of the real minute. At the minute change the minute hand jumps forward and the next round starts.

---

## 4. Brightness Control

- Automatic brightness adjustment using a photoresistor.
- Configurable thresholds for minimum and maximum brightness.
- Manual brightness control is available if no photoresistor is detected.

---

## 5. WiFi and NTP Integration

- WiFi:
  - Supports up to 15 WiFi networks.
  - Scanning for networks shows signal strength and whether each network is open or secured.
  - Hostname and all WiFi network settings live on their own WLAN tab on the main settings page - on first boot or whenever the last known network is unavailable, that tab opens automatically.
  - Automatically reconnects if the connection is lost (configurable).
  - Individual networks can be deleted directly from the web interface - remaining entries automatically move up to close the gap.
  - Add a new network via WPS with a single button on the web interface - no need to know the SSID/password in advance.
  - Customizable hostname (invalid characters are automatically cleaned up); falls back to an automatically generated name based on the MAC address if left empty.
- NTP (Network Time Protocol):
  - NTP has priority and is checked hourly; each configured server gets a same-server retry before the clock moves on to the next one, so a single lost UDP response doesn't count as a failed server.
  - If NTP is unavailable, the clock automatically falls back to the last valid DCF77 telegram (if one has been decoded, its parity checks out and it is no older than 10 minutes - the time elapsed since it was decoded is added back on, so an older telegram is just as accurate).
  - The RTC is set both at boot and hourly from whichever source (NTP or DCF77) actually succeeded.
  - DCF77 reception is monitored independently of which source currently drives the clock, and a prolonged DCF77 outage is shown in the status bar even while NTP is currently in charge.

---

## 6. Web Interface

- Redesigned dark-themed, single-page settings hub: Status, WLAN, Clock Setup, Brightness and Time/NTP are now tabs on one page (no more clicking through separate pages for basic settings).
- File-heavy management screens (Presets, Clock Face, Hand Set, File Manager, Live Preview, Factory Reset) remain separate pages, reachable from the navigation bar.
- A built-in web interface allows:
  - Uploading, downloading, and managing clock faces and hands.
  - Adjusting brightness, time zone, and display rotation.
  - Enabling/disabling Smooth Minute and Train Station modes.
  - Viewing system status (e.g., WiFi details, storage usage, uptime) - including whether PSRAM was detected and which rotation mode is active (hardware MADCTL register vs. software pixel remapping on the GC9D01 with PSRAM), so it is visible from the outside how the configured rotation values are actually applied.
- Multi-language interface (German, English, French).
- Factory reset page with separate options: reset everything, delete clock faces (except default), delete hand sets (except default), delete presets, or reset saved WiFi networks - each with its own confirmation.
- Download additional clock faces, hand sets, and presets directly from GitHub via the web interface - missing dependencies (e.g. a face/hand set referenced by a preset) are fetched automatically.
- Live preview (top-left corner on every page) always shows the currently active clock face/hands/hub color - click it (with confirmation) to save the current look as a new preset.

---

## 7. File Management with LittleFS

- Uses the LittleFS filesystem to:
  - Store custom clock faces and hands, RLE-compressed to save flash space.
  - Manage files (e.g., upload, download, rename, delete) with a compact icon-based interface.
- Logging

---

## 8. Time Zone Customization

- Supports various time zones:
  - Automatic daylight saving time (e.g. CET/CEST).
  - Permanent summer or winter time.

---

## 9. Hardware Integration

- Compatible only with ESP32-S2.
- Photoresistor for brightness measurement.

---

## 10. Advanced Features

- Uptime Display: Shows the clock's runtime since the last restart.
- Reboot Function: Allows restarting the clock via the web interface.
- BMP Scaling: Uploaded BMP files can be scaled to fit the display size.
- API Interface
- Up to 50 presets (face, hand set, hub color/size, second-hand display) - individually renameable and deletable, sorted alphabetically in the list; back up all presets to a file and restore them later; a warning is shown once all 50 slots are full.
- Deleting a clock face or hand set automatically removes any presets that referenced it; deleting the currently active hand set automatically falls back to the built-in default.
- DCF77 supported
- DCF77 decoder works on a one-second grid: pulses are placed by the distance between their start edges (always a whole number of seconds), not by counting pulses. A second that is received too weakly therefore only leaves a hole at its own position instead of shifting every following bit, and spurious edges caused by interference are discarded instead of being counted as extra bits. The grid is carried across dropouts of up to a minute.
- The bit progress is collected by grid position rather than by second of the minute, so it runs from the very first pulse - the live page no longer stays empty during the minutes it takes to identify the minute marker (it says so while that mapping is still missing). The grid is cleared on every wrap while the marker is unknown, so the display shows the last minute of actual reception rather than accumulating bits from several minutes.
- The 59th second (the minute marker, the only second without a pulse) is identified statistically rather than from a single two-second gap: with imperfect reception every lost second produces exactly the same gap, so the decoder used to lock onto the wrong position again and again. Instead, the decoder counts for each of the 60 grid positions how often a pulse was missing there - the marker is the one position that is missing in every single minute, while dropouts scatter randomly. It is usually identified within about three minutes.
- Missing bits are reconstructed where the protocol allows it: the fixed bits 0 and 20, the mutually inverse summer/winter time bits, and one missing bit per parity group. Since parity can no longer verify a group completed this way, such a telegram is only accepted once it matches the previous telegram plus the minutes elapsed since - two independently received minutes exactly one minute apart. Fully received telegrams are accepted immediately, as before.
- A telegram is only accepted when the protocol's fixed bits (bit 0 = 0, bit 20 = 1), all three parities and the value ranges (month 1-12, hour 0-23, ...) fit - so a disturbed telegram can no longer set a wrong time.
- The 14 unused weather/special-function bits no longer have to be received: a gap there used to discard an otherwise complete and correct time telegram.
- The DCF77 interrupt handler no longer calls anything that lives in flash (neither the DCF77 library's handler nor digitalRead), and its ring buffer holds 64 instead of 16 edges. While the flash cache is disabled - during every LittleFS write, so also every log line - such calls cost incoming edges; the enlarged buffer additionally bridges blocking operations such as an NTP attempt or a large web request.
- DCF77 live page (/dcf77): shows the bit-by-bit reception progress of the current telegram (59-box grid, updated every second) plus the last fully decoded telegram (date, time, weekday, summer/winter time, call bit, parity checks, number of reconstructed bits). Seconds that were lost are shown with a dashed border. Diagnostics cover dropped edges (ring buffer overflow), whether the minute marker has been identified and at which grid position, counters for pulses seen/missing/grid losses, and the raw width and spacing in milliseconds of the last twelve pulses - which shows without an oscilloscope whether the receiver delivers a usable signal at all (expected: widths around 100 or 200 ms, spacing close to a multiple of 1000 ms)
- DCF77 sync LED blink can be turned off via a checkbox (default: on) - while the clock is still acquiring the time signal, the onboard LED flashes briefly for every received DCF77 pulse. The flash switches itself off again after a fixed time, and the LED is additionally switched off once per minute as a safety net, so it can never stay lit permanently
