# Lizenzen verwendeter Fremdsoftware / Third-party licenses

uhr4 selbst steht unter der GNU General Public License v3.0 (siehe [LICENSE](LICENSE)). Die Firmware
(`build_uhr4/*.bin`) enthält außerdem die folgenden Bibliotheken und Schriften mit eigenen Lizenzen. Ihre
Copyright-Hinweise und Lizenztexte stehen unten.

uhr4 itself is licensed under the GNU General Public License v3.0 (see [LICENSE](LICENSE)). The firmware
(`build_uhr4/*.bin`) also contains the following libraries and fonts under their own licenses. Their copyright
notices and license texts follow below.

| Komponente / Component | Lizenz / License | Quelle / Source |
|---|---|---|
| LovyanGFX (enthält Teile von / includes parts of Adafruit_GFX, Adafruit_ILI9341, TFT_eSPI) | FreeBSD (BSD-2-Clause), BSD, MIT | https://github.com/lovyan03/LovyanGFX |
| RTClib (Adafruit) | MIT | https://github.com/adafruit/RTClib |
| Arduino core for ESP32 (arduino-esp32) | LGPL-2.1 | https://github.com/espressif/arduino-esp32 |
| ESP-IDF (Espressif) | Apache-2.0 | https://github.com/espressif/esp-idf |
| Schrift / font DejaVu Sans (DejaVu18, DejaVu40) | Bitstream Vera License, Public Domain | https://dejavu-fonts.github.io/License.html |
| Schrift / font FreeSans Bold (GNU FreeFont) | GPL-3.0 mit Font-Ausnahme / with font exception | https://www.gnu.org/software/freefont/license.html |
| Schrift / font Orbitron Light (The Orbitron Project Authors) | SIL Open Font License 1.1 | https://github.com/theleagueof/orbitron |

Die Schriften sind als Bitmap-Daten über LovyanGFX in die Firmware eingebunden. Schriften, die im
Zifferblatt-Designer hochgeladen werden (`font_*`, `stripfont_*.vlw`), liegen nur auf der jeweiligen Uhr und
gehören nicht zu diesem Projekt – für sie gilt die Lizenz der hochgeladenen Schrift.

The fonts are compiled into the firmware as bitmap data via LovyanGFX. Fonts uploaded in the clock face designer
(`font_*`, `stripfont_*.vlw`) only live on the respective clock and are not part of this project – the license
of the uploaded font applies to them.

Arduino-esp32 (LGPL-2.1) wird statisch gelinkt. Der vollständige Quellcode von uhr4 liegt in diesem Repository,
die Firmware lässt sich also mit einer geänderten Version der Bibliothek neu bauen (siehe
[CONTRIBUTING.md](CONTRIBUTING.md)).

Arduino-esp32 (LGPL-2.1) is linked statically. The complete source code of uhr4 is in this repository, so the
firmware can be rebuilt with a modified version of the library (see [CONTRIBUTING.md](CONTRIBUTING.md)).

---

## LovyanGFX

Lizenztext aus / license text from `LovyanGFX/license.txt`:

```
Adafruit_ILI9341 ORIGINAL LIBRARY HEADER:
vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvStartvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
  This is our library for the Adafruit  ILI9341 Breakout and Shield
  ----> http://www.adafruit.com/products/1651

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
  
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^End^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Adafruit_GFX ORIGINAL LIBRARY LICENSE:
vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvStartvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

Software License Agreement (BSD License)

Copyright (c) 2012 Adafruit Industries.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^End^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

TFT_eSPI ORIGINAL LIBRARY LICENSE:
vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvStartvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
Software License Agreement (FreeBSD License)

Copyright (c) 2020 Bodmer (https://github.com/Bodmer)

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^End^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

LovyanGFX ORIGINAL LIBRARY LICENSE:
vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvStartvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
Software License Agreement (FreeBSD License)

Copyright (c) 2020 lovyan03 (https://github.com/lovyan03)

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^End^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
```

---

## RTClib

```
MIT License

Copyright (c) 2019 Adafruit Industries

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

---

## Arduino core for ESP32, ESP-IDF

Die vollständigen Lizenztexte stehen in den jeweiligen Repositories. / The full license texts are in the
respective repositories:

- LGPL-2.1: https://github.com/espressif/arduino-esp32/blob/master/LICENSE.md
- Apache-2.0: https://github.com/espressif/esp-idf/blob/master/LICENSE

---

## Schriften / Fonts

- DejaVu: "Fonts are (c) Bitstream (see below). DejaVu changes are in public domain." Vollständiger Text /
  full text: https://dejavu-fonts.github.io/License.html
- GNU FreeFont (FreeSans): GPL-3.0 or later with the font exception ("As a special exception, if you create a
  document which uses this font, and embed this font or unaltered portions of this font into the document, this
  font does not by itself cause the resulting document to be covered by the GNU General Public License.").
  https://www.gnu.org/software/freefont/license.html
- Orbitron: Copyright 2018 The Orbitron Project Authors (https://github.com/theleagueof/orbitron), with Reserved
  Font Name "Orbitron". Licensed under the SIL Open Font License, Version 1.1:
  https://openfontlicense.org/open-font-license-official-text/
