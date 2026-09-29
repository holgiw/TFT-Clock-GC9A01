#pragma once
    // LovyanGFX-Konfiguration: Bus, Panel und Init-Sequenz. Pins kommen aus
    // config.h, alles liegt im Projekt, an der Bibliothek muss nichts
    // angepasst werden.

    // LovyanGFX configuration: bus, panel and init sequence. Pins come from
    // config.h, everything lives in the project, nothing has to be changed
    // inside the library.

#define LGFX_USE_V1
#include <LovyanGFX.hpp>


    // GC9A01 mit eigener, erprobter Init-Sequenz (gegenueber der LovyanGFX-
    // Vorgabe: 0x74 mit 0x85 statt 0x68, Invertierung an). Das GC9D01 hat
    // einen eigenen Treiber (Panel_UhrGC9D01 weiter unten).

    // GC9A01 with its own proven init sequence (compared to the LovyanGFX
    // default: 0x74 with 0x85 instead of 0x68, inversion on). The GC9D01 has
    // its own driver (Panel_UhrGC9D01 further below).

class Panel_UhrGC9A01 : public lgfx::Panel_GC9A01 {
protected:
    const uint8_t* getInitCommands(uint8_t listno) const override {
        static constexpr uint8_t list0[] = {
            0xEF, 0,
            0xEB, 1, 0x14,
            0xFE, 0,
            0xEF, 0,
            0xEB, 1, 0x14,
            0x84, 1, 0x40,
            0x85, 1, 0xFF,
            0x86, 1, 0xFF,
            0x87, 1, 0xFF,
            0x88, 1, 0x0A,
            0x89, 1, 0x21,
            0x8A, 1, 0x00,
            0x8B, 1, 0x80,
            0x8C, 1, 0x01,
            0x8D, 1, 0x01,
            0x8E, 1, 0xFF,
            0x8F, 1, 0xFF,
            0xB6, 2, 0x00, 0x20,
            0x36, 1, 0x48,
            0x3A, 1, 0x05,
            0x90, 4, 0x08, 0x08, 0x08, 0x08,
            0xBD, 1, 0x06,
            0xBC, 1, 0x00,
            0xFF, 3, 0x60, 0x01, 0x04,
            0xC3, 1, 0x13,
            0xC4, 1, 0x13,
            0xC9, 1, 0x22,
            0xBE, 1, 0x11,
            0xE1, 2, 0x10, 0x0E,
            0xDF, 3, 0x21, 0x0C, 0x02,
            0xF0, 6, 0x45, 0x09, 0x08, 0x08, 0x26, 0x2A,
            0xF1, 6, 0x43, 0x70, 0x72, 0x36, 0x37, 0x6F,
            0xF2, 6, 0x45, 0x09, 0x08, 0x08, 0x26, 0x2A,
            0xF3, 6, 0x43, 0x70, 0x72, 0x36, 0x37, 0x6F,
            0xED, 2, 0x1B, 0x0B,
            0xAE, 1, 0x77,
            0xCD, 1, 0x63,
            0x70, 9, 0x07, 0x07, 0x04, 0x0E, 0x0F, 0x09, 0x07, 0x08, 0x03,
            0xE8, 1, 0x34,
            0x62, 12, 0x18, 0x0D, 0x71, 0xED, 0x70, 0x70, 0x18, 0x0F, 0x71, 0xEF, 0x70, 0x70,
            0x63, 12, 0x18, 0x11, 0x71, 0xF1, 0x70, 0x70, 0x18, 0x13, 0x71, 0xF3, 0x70, 0x70,
            0x64, 7, 0x28, 0x29, 0xF1, 0x01, 0xF1, 0x00, 0x07,
            0x66, 10, 0x3C, 0x00, 0xCD, 0x67, 0x45, 0x45, 0x10, 0x00, 0x00, 0x00,
            0x67, 10, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x01, 0x54, 0x10, 0x32, 0x98,
            0x74, 7, 0x10, 0x85, 0x80, 0x00, 0x00, 0x4E, 0x00,
            0x98, 2, 0x3E, 0x07,
            0x35, 0,
            0x21, 0,
            0x11, 0 + CMD_INIT_DELAY, 120,
            0x29, 0 + CMD_INIT_DELAY, 20,
            0xFF, 0xFF, // Ende / end
        };
        return (listno == 0) ? list0 : nullptr;
    }
};


    // GC9D01 (160x160): eigener Chip mit eigener Startsequenz - dieselbe,
    // mit der uhr3 auf dem GC9D01 lief. Mit der GC9A01-Sequenz blieb das
    // GC9D01 schwarz (Beleuchtung an, kein Bild). Endzustand: MADCTL 0x00
    // (RGB, keine Hardware-Rotation - uhr4 dreht beim GC9D01 per Software),
    // keine Invertierung, Farbtiefe 0x05.

    // GC9D01 (160x160): a chip of its own with its own init sequence - the
    // same one uhr3 ran with on the GC9D01. With the GC9A01 sequence the
    // GC9D01 stayed black (backlight on, no image). Final state: MADCTL 0x00
    // (RGB, no hardware rotation - uhr4 rotates in software on the GC9D01),
    // no inversion, color depth 0x05.

class Panel_UhrGC9D01 : public lgfx::Panel_GC9A01 {
public:
    Panel_UhrGC9D01() {
        _cfg.panel_width = _cfg.memory_width = 160;
        _cfg.panel_height = _cfg.memory_height = 160;
    }

protected:
    const uint8_t* getInitCommands(uint8_t listno) const override {
        static constexpr uint8_t list0[] = {
            0xFE, 0,
            0xEF, 0,
            0x80, 1, 0xFF,
            0x81, 1, 0xFF,
            0x82, 1, 0xFF,
            0x83, 1, 0xFF,
            0x84, 1, 0xFF,
            0x85, 1, 0xFF,
            0x86, 1, 0xFF,
            0x87, 1, 0xFF,
            0x88, 1, 0xFF,
            0x89, 1, 0xFF,
            0x8A, 1, 0xFF,
            0x8B, 1, 0xFF,
            0x8C, 1, 0xFF,
            0x8D, 1, 0xFF,
            0x8E, 1, 0xFF,
            0x8F, 1, 0xFF,
            0x3A, 1, 0x05,
            0xEC, 1, 0x01,
            0x74, 7, 0x02, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x98, 1, 0x3E,
            0x99, 1, 0x3E,
            0xB5, 2, 0x0D, 0x0D,
            0x60, 4, 0x38, 0x0F, 0x79, 0x67,
            0x61, 4, 0x38, 0x11, 0x79, 0x67,
            0x64, 6, 0x38, 0x17, 0x71, 0x5F, 0x79, 0x67,
            0x65, 6, 0x38, 0x13, 0x71, 0x5B, 0x79, 0x67,
            0x6A, 2, 0x00, 0x00,
            0x6C, 7, 0x22, 0x02, 0x22, 0x02, 0x22, 0x22, 0x50,
            0x6E, 32, 0x03, 0x03, 0x01, 0x01, 0x00, 0x00, 0x0F, 0x0F,
                      0x0D, 0x0D, 0x0B, 0x0B, 0x09, 0x09, 0x00, 0x00,
                      0x00, 0x00, 0x0A, 0x0A, 0x0C, 0x0C, 0x0E, 0x0E,
                      0x10, 0x10, 0x00, 0x00, 0x02, 0x02, 0x04, 0x04,
            0xBF, 1, 0x01,
            0xF9, 1, 0x40,
            0x9B, 1, 0x3B,
            0x93, 3, 0x33, 0x7F, 0x00,
            0x7E, 1, 0x30,
            0x70, 6, 0x0D, 0x02, 0x08, 0x0D, 0x02, 0x08,
            0x71, 3, 0x0D, 0x02, 0x08,
            0x91, 2, 0x0E, 0x09,
            0xC3, 1, 0x19,
            0xC4, 1, 0x19,
            0xC9, 1, 0x3C,
            0xF0, 6, 0x53, 0x15, 0x0A, 0x04, 0x00, 0x3E,
            0xF2, 6, 0x53, 0x15, 0x0A, 0x04, 0x00, 0x3A,
            0xF1, 6, 0x56, 0xA8, 0x7F, 0x33, 0x34, 0x5F,
            0xF3, 6, 0x52, 0xA4, 0x7F, 0x33, 0x34, 0xDF,
            0x36, 1, 0x00,
            0x11, 0 + CMD_INIT_DELAY, 200,
            0x29, 0 + CMD_INIT_DELAY, 20,
            0xFF, 0xFF, // Ende / end
        };
        return (listno == 0) ? list0 : nullptr;
    }

    // Keine Hardware-Rotation (MADCTL immer 0, mit rgb_order = RGB 0x00)
    // No hardware rotation (MADCTL always 0, with rgb_order = RGB 0x00)
    uint8_t getMadCtl(uint8_t) const override { return 0; }

    // Farbtiefe 0x05 (wie uhr3) statt LovyanGFX-Standard 0x55
    // Color depth 0x05 (as in uhr3) instead of the LovyanGFX default 0x55
    uint8_t getColMod(uint8_t bpp) const override { return (bpp > 16) ? RGB888_3BYTE : 0x05; }
};


    // Ein Geraet fuer BEIDE Displays: CS wird manuell umgeschaltet (setCS1()/
    // setCS2() in display.h), daher pin_cs = -1. Zwei Panel-Treiber, gewaehlt
    // nach dem Displaytyp per selectPanel() VOR tft.init() (setup()).

    // One device for BOTH displays: CS is switched manually (setCS1()/
    // setCS2() in display.h), hence pin_cs = -1. Two panel drivers, chosen by
    // the display type via selectPanel() BEFORE tft.init() (setup()).

class UhrLGFX : public lgfx::LGFX_Device {
    Panel_UhrGC9A01 _panel_gc9a01;
    Panel_UhrGC9D01 _panel_gc9d01;
    lgfx::Bus_SPI _bus_instance;

    void configPanel(lgfx::Panel_LCD& panel, uint16_t size, bool invert, bool rgbOrder) {
        panel.setBus(&_bus_instance);
        auto cfg = panel.config();
        cfg.pin_cs = -1;              // manuell, siehe CS_1/CS_2 in config.h
                                      // manual, see CS_1/CS_2 in config.h
        // Kein Reset durch LovyanGFX (nur 8 ms Puls, 64 ms Wartezeit). Den
        // Reset macht setup() selbst mit laengeren Zeiten (resetPanels() in
        // display.h).
        // No reset by LovyanGFX (only an 8 ms pulse, 64 ms wait). setup()
        // does the reset itself with longer timing (resetPanels() in
        // display.h).
        cfg.pin_rst = -1;
        cfg.pin_busy = -1;
        cfg.panel_width = size;
        cfg.panel_height = size;
        cfg.memory_width = size;
        cfg.memory_height = size;
        cfg.offset_x = 0;
        cfg.offset_y = 0;
        cfg.offset_rotation = 0;
        cfg.readable = false;
        cfg.invert = invert;
        cfg.rgb_order = rgbOrder;
        cfg.dlen_16bit = false;
        cfg.bus_shared = false;
        panel.config(cfg);
    }

public:
    UhrLGFX() {
        {
            auto cfg = _bus_instance.config();
            cfg.spi_host = SPI3_HOST;     // HSPI wie bisher (USE_HSPI_PORT)
                                          // HSPI as before (USE_HSPI_PORT)
            cfg.spi_mode = 0;
            cfg.freq_write = TFT_SPI_FREQUENCY;
            cfg.freq_read = 16000000;
            cfg.spi_3wire = false;
            cfg.use_lock = true;
            // DMA AUS: LovyanGFX 1.2.x hat in Bus_SPI::writeBytes()/execDMAQueue()
            // keinen DMA-Zweig fuer den ESP32-S2 (nur GDMA-Chips und ESP32) -
            // DMA-Uebertragungen werden dort still verworfen. Betroffen sind
            // auch Nicht-DMA-Uebertragungen von 65..1023 Byte, die intern ueber
            // einen Puffer auf DMA umgeleitet werden, z. B. jede Zeile eines
            // Teilbilds. Ohne DMA-Kanal sendet LovyanGFX immer per CPU.

            // DMA OFF: LovyanGFX 1.2.x has no DMA branch for the ESP32-S2 in
            // Bus_SPI::writeBytes()/execDMAQueue() (only GDMA chips and ESP32) -
            // DMA transfers are silently dropped there. Also affected are
            // non-DMA transfers of 65..1023 bytes, which are internally rerouted
            // to DMA via a buffer, e.g. every row of a partial frame. Without a
            // DMA channel LovyanGFX always sends via the CPU.
            cfg.dma_channel = 0;
            cfg.pin_sclk = TFT_SCLK;
            cfg.pin_mosi = TFT_MOSI;
            cfg.pin_miso = -1;
            cfg.pin_dc = TFT_DC;
            _bus_instance.config(cfg);
        }
        // GC9A01: INVON und BGR wie in der bisherigen Init-Sequenz
        // GC9A01: INVON and BGR as in the previous init sequence
        configPanel(_panel_gc9a01, 240, true, false);
        // GC9D01: keine Invertierung, RGB (Endzustand der Startsequenz)
        // GC9D01: no inversion, RGB (final state of the init sequence)
        configPanel(_panel_gc9d01, 160, false, true);
        setPanel(&_panel_gc9a01);
    }

    // Panel-Treiber zum Displaytyp waehlen - nur VOR tft.init()
    // Choose the panel driver for the display type - only BEFORE tft.init()
    void selectPanel(bool gc9d01) {
        if (gc9d01) setPanel(&_panel_gc9d01);
        else setPanel(&_panel_gc9a01);
    }
};
