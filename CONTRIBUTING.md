# Contributing to TFT-Clock-GC9A01 (uhr3)

Thanks for your interest in this project! It's a hobby project maintained
in spare time, so please keep pull requests focused and reasonably small -
that makes them much easier to review and merge.

## Hardware & Build Environment

- Target: **ESP32-S2** only (limited RAM - see "Memory constraints" below)
- Arduino IDE, partition scheme: **"No OTA, 2MB APP / 2MB SPIFFS"**
- **PSRAM: Enabled** in board settings
- Displays: GC9A01 (240x240), GC9D01 (160x160), or ILI9341 (240x240) -
  selected via `config.h`

## Project Structure

The firmware is split into `uhr3.ino` plus several headers, each with a
clear responsibility:

| File | Responsibility |
|---|---|
| `config.h` | Board/display selection, hardware pins, constants |
| `globals.h` | Global variables and shared state |
| `declarations.h` | Forward declarations for every function (see below) |
| `translation.h` | UI translations (DE/FR) |
| `wifi_manager.h` | WiFi connection, AP mode, scanning, WPS |
| `time_sync.h` | NTP and DCF77 time synchronization |
| `display.h` | Rendering, BMP handling, RLE compression |
| `presets_manager.h` | Preset storage/retrieval logic |
| `webserver_routes.h` | All HTTP routes and generated HTML |
| `system_utils.h` | Heap monitoring, misc helpers |
| `prefs_keys.h` | `Preferences` (NVS) key name constants |

**Any new function needs a matching forward declaration in
`declarations.h`** - the project relies on this instead of reordering
`#include`s.

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
  `{ "English key", "German value", "French value" }`.
- **Use HTML entities for accented characters** (`&auml;`, `&ouml;`,
  `&uuml;`, `&szlig;`, `&eacute;`, `&egrave;`, `&ccedil;`, `&agrave;`, ...)
  instead of raw UTF-8 umlauts/accents. The HTML head sends no charset
  declaration, so raw UTF-8 bytes get mis-rendered by the browser
  (mojibake) - HTML entities render correctly regardless.
- **Never embed layout markup** (`<br>`, purely-structural `&nbsp;`) inside
  a translation string. If you need a line break or non-breaking space for
  layout, add it in the surrounding C++/HTML code, not the translated
  text. (`&nbsp;` *inside* a translated phrase is fine when it's part of
  keeping that specific phrase from wrapping awkwardly, e.g. narrow nav
  labels.)
- **Every `translate("...")` call needs both a German and a French
  entry.** Before submitting a PR, verify there are no missing
  translations (extract every `translate("...")` key used across the
  `.ino`/`.h` files and confirm each exists in `translationTable`).
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
- [ ] Every `translate("...")` call has a matching German *and* French
      entry in `translation.h`
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
- Board variant and display (GC9A01/GC9D01/ILI9341)
- Steps to reproduce
- Serial monitor output if the device crashed or behaved unexpectedly
