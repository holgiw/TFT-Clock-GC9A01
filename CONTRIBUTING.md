# Contributing to TFT-Clock-GC9A01 (uhr4)

Thanks for your interest in this project! It's a hobby project maintained
in spare time, so please keep pull requests focused and reasonably small -
that makes them much easier to review and merge.

## Hardware & Build Environment

- Target: **ESP32-S2** only (limited RAM - see "Memory constraints" below)
- Board: **LOLIN S2 PICO** (esp32 Arduino core, built with 3.3.12), partition
  scheme: **"No OTA, 2MB APP / 2MB SPIFFS"**. PSRAM is always on for this
  board (`BOARD_HAS_PSRAM` in the board definition, no menu option).
- Compiler flag `-mtext-section-literals` is required: uhr4 is one large translation unit, without it the
  linker fails with "dangerous relocation: l32r: literal target out of range". Visual Micro picks it up from
  `board.txt` in the project folder; for Arduino IDE/arduino-cli add
  `compiler.cpp.extra_flags=-mtext-section-literals` to a `platform.local.txt` next to the ESP32 core's
  `platform.txt` (or pass it via `--build-property`).
- Release builds (`uhr4.ino.bin`, `.bootloader.bin`, `.partitions.bin`,
  `.merged.bin`) go directly into `build_uhr4/` - `flashESP.bat`/`flashESP.sh`
  flash exactly those files.
- Displays: GC9A01 (240x240, without or with backlight on pin 3), GC9D01
  (160x160) or ILI9341 (240x320, 240 clock on top, time/date strip below; fixed backlight) -
  one firmware for all, the type is a setting in the web UI ("Clock Setup" tab, takes effect after a restart).
  Dimensions per type live in `DISPLAY_GEOMETRY` (`config.h`); `CLOCK_WIDTH`,
  `HAND_WIDTH` etc. read the active type at runtime, so never use them in array
  sizes, `static_assert` or `#if`. `TFT_WIDTH`/`TFT_HEIGHT` are the panel size, which is only larger
  than the clock on the ILI9341 (`drawInfoStrips()` in `display.h`).
- Graphics library: **LovyanGFX** (tested with 1.2.30), installed unchanged from
  the Library Manager - bus, panel and init sequence live in `lgfx_config.h`

## Project Structure

The firmware is split into `uhr4.ino` plus several headers, each with a
clear responsibility:

| File | Responsibility |
|---|---|
| `config.h` | Display geometry per type, hardware pins, constants |
| `lgfx_config.h` | LovyanGFX device: SPI bus, GC9A01 and GC9D01 panels (own init sequences), ILI9341 (LovyanGFX default driver) |
| `globals.h` | Global variables and shared state |
| `declarations.h` | Forward declarations for every function (see below) |
| `translation.h` | UI translations (DE) |
| `wifi_manager.h` | WiFi connection, AP mode, scanning, WPS |
| `time_sync.h` | NTP and DCF77 time synchronization |
| `display.h` | Rendering, BMP handling, RLE compression, USB setup commands |
| `presets_manager.h` | Preset storage/retrieval logic |
| `backup.h` | Full backup/restore (TAR), WiFi encryption |
| `webserver_routes.h` | All HTTP routes and generated HTML |
| `face_designer_html.h` | Clock face designer (browser page) - **generated**, see below |
| `hand_designer_html.h` | Hand designer (browser page) - **generated**, see below |
| `web/` | Sources of both designers (`*.css`, `*.html`, `*.js`) and the generator `build_web.py` |
| `rocrail_client.h` | Rocrail model time connection |
| `system_utils.h` | Heap monitoring, misc helpers |
| `prefs_keys.h` | `Preferences` (NVS) key name constants |
| `build_defs.h` | Build date/year from `__DATE__`/`__TIME__` |

**Any new function needs a matching forward declaration in
`declarations.h`** - the project relies on this instead of reordering
`#include`s.

### Designer pages (`web/`)

The clock face and hand designers are edited in `web/face_designer.*` and `web/hand_designer.*`, never in the
generated headers. After a change, run from the project folder:

```bash
python web/build_web.py
```

It writes `face_designer_html.h` and `hand_designer_html.h`: the markup stays text, CSS and script are stored
gzip-compressed without comment lines (served at `/facedesigner.css|.js` and `/handdesigner.css|.js`, unpacked by
the browser). This saves ~90 KB of flash. Commit the regenerated headers together with the `web/` sources - building
the firmware itself needs no extra step.

## Code Style

- **Indentation:** preprocessor directives (`#define`, `#include`, `#if`,
  `#endif`, ...) stay flush left (column 0); everything else follows normal
  nested indentation.
- **Naming:** camelCase for variables and functions.
- Keep related logic together and add a short comment explaining *why*,
  not just *what*, especially for anything non-obvious (timing, memory
  layout, hardware quirks).

## Translations (`translation.h`)

- Translations live in a flash-resident `static const TranslationEntry
  translationTable[]` (a plain array, not `std::map`) - this is
  intentional, see "Memory constraints" below. Add new entries as
  `{ "English key", "German value" }`.
- **Use HTML entities for accented characters** (`&auml;`, `&ouml;`,
  `&uuml;`, `&szlig;`, ...)
  instead of raw UTF-8 umlauts/accents. The HTML head sends no charset
  declaration, so raw UTF-8 bytes get mis-rendered by the browser
  (mojibake) - HTML entities render correctly regardless.
- **Never embed layout markup** (`<br>`, purely-structural `&nbsp;`) inside
  a translation string. If you need a line break or non-breaking space for
  layout, add it in the surrounding C++/HTML code, not the translated
  text. (`&nbsp;` *inside* a translated phrase is fine when it's part of
  keeping that specific phrase from wrapping awkwardly, e.g. narrow nav
  labels.)
- **Every `translate("...")` call needs a German entry - unless the German
  text is identical** (e.g. "Status", "Log", "Version"): `translate()` returns
  the key when there is no entry, so identical entries are deliberately left
  out. Before submitting a PR, verify there are no missing translations
  (extract every `translate("...")` key used across the `.ino`/`.h` files and
  confirm each exists in `translationTable` or reads the same in German), and
  remove entries whose key is no longer used anywhere.
- Keep the key itself in English, matching the fallback text shown when
  the interface language is English.

## Memory Constraints (please read before adding dependencies)

The ESP32-S2 has limited internal RAM, and this project has already hit
real bugs from underestimating that:

- **Avoid `<algorithm>`/`std::sort`.** The project uses small, explicit
  insertion sorts instead (see `naturalSortNames()`) to avoid pulling in
  the dependency for what are always small (file/preset count) lists.
- **Avoid large `std::initializer_list`-based constructions**, especially
  inside functions reachable from HTTP request handlers. A `std::map`
  built from a ~200-entry initializer list once caused a stack overflow
  crash on `/setLanguage` - that's why translations are now a flat array
  instead.
- **Minimize `Preferences` (NVS) writes.** Flash has a limited write
  endurance; only call `preferences.put...()` when a value actually
  changed (see `savePresets()` for the pattern).
- **Don't assume PSRAM helps with TLS/HTTPS.** mbedTLS's internal buffers
  need contiguous *internal* RAM for the handshake and do not
  automatically spill into PSRAM on this chip - a GitHub-fetch-over-HTTPS
  feature was reverted for exactly this reason. If you need HTTPS, budget
  for ~40 KB+ of free internal heap at connection time and test on real
  hardware, not just "should work in theory."

## Before Submitting a Pull Request

Please check:

- [ ] Braces `{`/`}` balance in every file you touched
- [ ] Preprocessor directives (`#if`/`#ifdef`/`#ifndef`/`#endif`) balance
- [ ] Every `translate("...")` call has a matching German entry in
      `translation.h` (unless the German text is identical)
- [ ] No raw non-ASCII characters were introduced into `translation.h`
      (use HTML entities - see above)
- [ ] The sketch still compiles for the ESP32-S2 board/partition scheme
      described above
- [ ] New routes that send/modify state use `HTTP_POST` (not `HTTP_GET`)
      if they have side effects, and confirm destructive actions
      client-side (`onclick="return confirm(...)"`) before submitting,
      matching the existing delete/reset patterns

## Reporting Issues

When reporting a bug, please include:
- Board variant and display type (GC9A01 without backlight, GC9A01 with
  backlight on pin 3, GC9D01 or ILI9341) and the firmware version/build date
- Steps to reproduce
- Serial monitor output if the device crashed or behaved unexpectedly
