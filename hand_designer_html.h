#pragma once
    // Zeiger-Designer (Seite /handdesigner): Editor-HTML, -CSS und -Skript,
    // direkt aus dem Flash gestreamt (ohne Heap-Kopie), nach dem Seitenkopf.
    // Die Konfiguration kommt aus dem vorher gesendeten JS-Objekt HD.

    // Hand designer (page /handdesigner): editor HTML, CSS and script,
    // streamed straight from flash (no heap copy), after the page header.
    // Configuration comes from the HD JS object sent right before.

static const char HAND_DESIGNER_HTML[] PROGMEM = R"HDRAW(
<style>
.hd{max-width:1150px;margin:0 auto;text-align:left}
.hd .row{display:flex;flex-wrap:wrap;gap:14px;justify-content:center;align-items:flex-start}
.hd .card{background:var(--panel);border:1px solid var(--panel-border);border-radius:10px;padding:10px 12px}
.hd h3{margin:4px 0 8px;font-size:1rem}
.hd button,.hd select,.hd input{width:auto;margin:3px;padding:6px 10px}
.hd .tb{background:var(--panel);border:1px solid var(--panel-border);color:var(--text);font-weight:normal}
.hd .tb.on{background:var(--accent);border-color:var(--accent);color:#1a1200;font-weight:bold}
.hd button:disabled{opacity:.4;cursor:default}
.hd canvas{touch-action:none;border:1px solid var(--panel-border);display:block;margin:0 auto}
.hd .sw{width:22px;height:22px;border-radius:4px;border:1px solid var(--panel-border);display:inline-block;cursor:pointer;margin:2px;vertical-align:middle}
.hd .pal{display:grid;grid-template-columns:repeat(16,1fr);gap:1px;max-width:224px;margin:2px 0 4px}
.hd .pal span{aspect-ratio:1;cursor:pointer;border-radius:2px}
.hd .sw.on,.hd .pal span.on{outline:2px solid var(--accent);outline-offset:1px}
.hd .hex{width:86px;font-family:monospace}
.hd.stickon .stick{position:sticky;top:var(--hdTop,70px);max-height:calc(100vh - var(--hdTop,70px) - 8px);overflow-y:auto}
.hd .num{width:72px;padding:6px 4px 6px 8px}
.hd label{white-space:nowrap;display:inline-flex;align-items:center;gap:4px;margin:3px 6px 3px 0}
.hd .grid2{display:grid;grid-template-columns:auto auto;gap:2px 8px;align-items:center}
.hd .ok{background:#d4edda;color:#155724;border:1px solid #c3e6cb;border-radius:6px;padding:8px 12px;margin:8px 0}
.hd .err{background:#f8d7da;color:#721c24;border:1px solid #f5c6cb;border-radius:6px;padding:8px 12px;margin:8px 0}
.hd small{display:block;margin:4px 0}
</style>

<div class="hd" id="hdRoot">
  <div class="card" style="margin-bottom:14px">
    <div class="row" style="justify-content:flex-start;align-items:center">
      <span id="baseInfo"></span>
      <button type="button" class="tb" id="resetBtn"></button>
      <span style="flex:1"></span>
      <button type="button" id="saveCurBtn"></button>
      <label><input type="checkbox" id="activate" checked><span id="tActivate"></span></label>
      <button type="button" id="saveBtn"></button>
    </div>
    <div id="msg"></div>
  </div>

  <div class="row">
    <div class="card stick" style="max-width:250px">
      <h3 id="tPart"></h3>
      <div id="partBtns"></div>
      <h3 id="tTools"></h3>
      <div id="toolBtns"></div>
      <small id="toolHint"></small>
      <label><input type="checkbox" id="sym" checked><span id="tSym"></span></label>
      <h3 id="tColor"></h3>
      <input type="color" id="color" value="#000000" style="padding:0;height:32px;width:48px;vertical-align:middle">
      <input type="text" id="colorHex" class="hex" maxlength="7" spellcheck="false" style="vertical-align:middle">
      <button type="button" class="tb" id="pickBtn" style="vertical-align:middle"></button>
      <small id="colorHint"></small>
      <small id="tStd"></small>
      <div id="swatches"></div>
      <small id="tPal"></small>
      <div id="palette" class="pal"></div>
      <h3 id="tEdit"></h3>
      <button type="button" class="tb" id="undoBtn"></button>
      <button type="button" class="tb" id="redoBtn"></button>
      <button type="button" class="tb" id="clearBtn"></button><br>
      <span id="tShift"></span>
      <button type="button" class="tb" data-shift="0,-1">&#8593;</button>
      <button type="button" class="tb" data-shift="0,1">&#8595;</button>
      <button type="button" class="tb" data-shift="-1,0">&#8592;</button>
      <button type="button" class="tb" data-shift="1,0">&#8594;</button><br>
      <span id="tCopy"></span>
      <select id="copySel"></select>
      <button type="button" class="tb" id="copyBtn">OK</button>
    </div>

    <div class="card">
      <div style="text-align:center">
        <button type="button" class="tb" id="zoomOut">&minus;</button>
        <span id="zoomInfo" style="display:inline-block;min-width:70px"></span>
        <button type="button" class="tb" id="zoomIn">+</button>
      </div>
      <canvas id="ed"></canvas>
      <small id="posInfo" style="text-align:center"></small>
      <small id="widthHint" style="text-align:center;max-width:220px;margin:4px auto"></small>
    </div>

    <div class="card stick" style="max-width:300px">
      <h3 id="tGen"></h3>
      <div class="grid2">
        <span id="tLen"></span><input type="number" class="num" id="gLen" min="1">
        <span id="tTipW"></span><input type="number" class="num" id="gTipW" min="0">
        <span id="tBaseW"></span><input type="number" class="num" id="gBaseW" min="1">
        <span id="tTail"></span><input type="number" class="num" id="gTail" min="0">
        <span id="tTailW"></span><input type="number" class="num" id="gTailW" min="0">
        <span id="tDisc"></span><input type="number" class="num" id="gDisc" min="0">
        <span id="tDiscPos"></span><input type="number" class="num" id="gDiscPos" min="0">
      </div>
      <button type="button" id="genBtn"></button>
      <h3 id="tPreview"></h3>
      <canvas id="pv"></canvas>
      <label><input type="checkbox" id="live" checked><span id="tLive"></span></label>
      <select id="bgSel"></select>
    </div>
  </div>
</div>

<script>
(function () {
  var W = HD.w, H = HD.h, PX = HD.px, PY = HD.py, N = W * H;
  var PARTS = ['hour', 'minute', 'second'];
  var TRANSPARENT = -1;

  var TX = {
    de: { base: 'Basis:', builtin: 'Standard (eingebaut)', set: 'Satz', active: 'aktiv', reset: 'Aenderungen verwerfen',
      activate: 'neues Design aktivieren', saveBtn: 'Als neues Design speichern', part: 'Zeiger', hour: 'Stunde', minute: 'Minute', second: 'Sekunde', tools: 'Werkzeug',
      pen: 'Stift', erase: 'Radierer', line: 'Linie', rect: 'Rahmen', rectf: 'Rechteck', ell: 'Ellipse',
      ellf: 'Ellipse gefuellt', poly: 'Polygon', fill: 'Fuellen', pick: 'Pipette', sym: 'Spiegeln an der Mittelachse',
      color: 'Farbe', edit: 'Bearbeiten', undo: 'Rueckgaengig', redo: 'Wiederholen', clear: 'Leeren', shift: 'Verschieben:',
      copy: 'Kopieren nach:', gen: 'Form erzeugen', len: 'Laenge', tipW: 'Spitze breit', baseW: 'Breite am Drehpunkt',
      tail: 'Gegengewicht', tailW: 'Gegengewicht breit', disc: 'Scheibe (Durchm.)', discPos: 'Scheibe Abstand',
      genBtn: 'Erzeugen', preview: 'Vorschau', live: 'Live-Uhrzeit', bgFace: 'Zifferblatt', bgDark: 'dunkel',
      bgLight: 'hell', saving: 'Speichere...', saved: 'Als neues Design Satz {0} gespeichert', activated: ' und aktiviert',
      saveCurBtn: 'Aktuelles Design speichern und anwenden', savedCur: 'Satz {0} gespeichert und angewendet',
      builtinRO: 'Das eingebaute Standard-Design kann nicht ueberschrieben werden - bitte als neues Design speichern.',
      loading: 'Lade...', loaded: 'Aktives Design geladen', failed: 'Fehler: ', missing: 'fehlt - Standard verwendet',
      confirmReset: 'Alle Aenderungen verwerfen und das aktive Design neu laden?', confirmClear: 'Diesen Zeiger komplett leeren?',
      confirmGen: 'Aktuellen Zeiger durch die erzeugte Form ersetzen?',
      whiteHint: 'Reines Weiss ist transparent - wird automatisch zu Fast-Weiss.',
      pickBtn: 'Aufnehmen', pickTip: 'Farbe aus einem Pixel aufnehmen - auch per Rechtsklick in die Zeichenflaeche oder Klick in die Vorschau',
      std: 'Standardfarben', pal: 'Palette', free: 'Beliebige Farbe', hexHint: '#RRGGBB oder RGB565 (0xFFFF)',
      h_pen: 'Stift: Pixel einzeln setzen oder freihand zeichnen.',
      h_erase: 'Radierer: macht Pixel wieder transparent.', h_line: 'Linie: vom Anfangs- zum Endpunkt ziehen.',
      h_rect: 'Rahmen: Rechteck-Umriss aufziehen.', h_rectf: 'Rechteck: gefuelltes Rechteck aufziehen.',
      h_ell: 'Ellipse: Umriss aufziehen - ein Quadrat ergibt einen Kreis.',
      h_ellf: 'Ellipse gefuellt: gefuellte Ellipse oder Kreis aufziehen.',
      h_fill: 'Fuellen: faerbt die zusammenhaengende gleichfarbige Flaeche um.',
      h_pick: 'Pipette: Klick uebernimmt die Farbe des Pixels.',
      h_poly: 'Polygon: Punkte anklicken, Doppelklick oder Klick auf den ersten Punkt schliesst, Esc bricht ab.',
      pos: 'Pixel', pivot: 'Drehpunkt',
      widthHint: 'Das aktuelle Zifferblatt zeigt diesen Zeiger nur {0} px breit - ausgegraute Spalten werden auf der Uhr abgeschnitten.' },
    en: { base: 'Based on:', builtin: 'Default (built-in)', set: 'Set', active: 'active', reset: 'Discard changes',
      activate: 'activate new design', saveBtn: 'Save as new design', part: 'Hand', hour: 'Hour', minute: 'Minute', second: 'Second', tools: 'Tool',
      pen: 'Pen', erase: 'Eraser', line: 'Line', rect: 'Frame', rectf: 'Rectangle', ell: 'Ellipse',
      ellf: 'Filled ellipse', poly: 'Polygon', fill: 'Fill', pick: 'Picker', sym: 'Mirror at the centre axis',
      color: 'Colour', edit: 'Edit', undo: 'Undo', redo: 'Redo', clear: 'Clear', shift: 'Move:',
      copy: 'Copy to:', gen: 'Generate shape', len: 'Length', tipW: 'Tip width', baseW: 'Width at pivot',
      tail: 'Counterweight', tailW: 'Counterweight width', disc: 'Disc (diameter)', discPos: 'Disc distance',
      genBtn: 'Generate', preview: 'Preview', live: 'Live time', bgFace: 'Clock face', bgDark: 'dark',
      bgLight: 'light', saving: 'Saving...', saved: 'Saved as new design, set {0}', activated: ' and activated',
      saveCurBtn: 'Save and apply current design', savedCur: 'Set {0} saved and applied',
      builtinRO: 'The built-in default design cannot be overwritten - please save as a new design.',
      loading: 'Loading...', loaded: 'Active design loaded', failed: 'Error: ', missing: 'missing - default used',
      confirmReset: 'Discard all changes and reload the active design?', confirmClear: 'Clear this hand completely?',
      confirmGen: 'Replace the current hand with the generated shape?',
      whiteHint: 'Pure white is transparent - automatically becomes near-white.',
      pickBtn: 'Pick', pickTip: 'Pick the colour of a pixel - also by right-clicking the drawing area or clicking the preview',
      std: 'Standard colours', pal: 'Palette', free: 'Any colour', hexHint: '#RRGGBB or RGB565 (0xFFFF)',
      h_pen: 'Pen: set single pixels or draw freehand.', h_erase: 'Eraser: makes pixels transparent again.',
      h_line: 'Line: drag from the start to the end point.', h_rect: 'Frame: drag out a rectangle outline.',
      h_rectf: 'Rectangle: drag out a filled rectangle.',
      h_ell: 'Ellipse: drag out an outline - a square gives a circle.',
      h_ellf: 'Filled ellipse: drag out a filled ellipse or circle.',
      h_fill: 'Fill: recolours the connected area of the same colour.',
      h_pick: 'Picker: a click takes over the colour of the pixel.',
      h_poly: 'Polygon: click points, double-click or click the first point to close, Esc cancels.',
      pos: 'Pixel', pivot: 'Pivot',
      widthHint: 'The current clock face shows this hand only {0} px wide - greyed-out columns are cut off on the clock.' }
  };
  var L = (HD.lang === 'de') ? 'de' : 'en';
  function t(k, a) { var s = TX[L][k] || TX.en[k] || k; return a === undefined ? s : s.replace('{0}', a); }
  function $(id) { return document.getElementById(id); }

  // Zustand
  // State
  var pix = {}, undoSt = {}, redoSt = {};
  PARTS.forEach(function (p) { pix[p] = new Int32Array(N).fill(TRANSPARENT); undoSt[p] = []; redoSt[p] = []; });
  var part = 'hour', tool = 'pen', color = 0x0000, dirty = false;
  function setDirty(v) {
    dirty = v; $('saveBtn').disabled = !v;
    // Das eingebaute Standard-Design liegt im Flash und laesst sich nicht ueberschreiben
    // The built-in default design lives in flash and cannot be overwritten
    var builtin = activeBase() === 'default';
    $('saveCurBtn').disabled = !v || builtin;
    $('saveCurBtn').title = builtin ? t('builtinRO') : '';
  }

  // Formparameter je Zeiger (Startwerte aus den Displaymassen)
  // shape parameters per hand (defaults derived from the display size)
  var tailMax = H - PY - 1;
  var gen = {
    hour:   { len: Math.round(PY * 0.62), tipW: Math.max(1, Math.round(W * 0.2)), baseW: Math.round(W * 0.45), tail: Math.round(tailMax * 0.3), tailW: Math.round(W * 0.3), disc: 0, discPos: 0 },
    minute: { len: Math.round(PY * 0.95), tipW: Math.max(1, Math.round(W * 0.12)), baseW: Math.round(W * 0.35), tail: Math.round(tailMax * 0.3), tailW: Math.round(W * 0.25), disc: 0, discPos: 0 },
    second: { len: Math.round(PY * 0.95), tipW: 1, baseW: Math.max(1, Math.round(W * 0.12)), tail: Math.round(tailMax * 0.8), tailW: Math.max(1, Math.round(W * 0.12)), disc: Math.round(W * 0.45), discPos: Math.round(PY * 0.62) }
  };

  // Farben (RGB565)
  // Colours (RGB565)
  function to565(hex) {
    var r = parseInt(hex.substr(1, 2), 16), g = parseInt(hex.substr(3, 2), 16), b = parseInt(hex.substr(5, 2), 16);
    return (Math.round(r * 31 / 255) << 11) | (Math.round(g * 63 / 255) << 5) | Math.round(b * 31 / 255);
  }
  // 0xFFFF (Weiss) und 0x0120 (TRANSPARENT_COLOR) gelten auf dem Geraet als transparent
  // 0xFFFF (white) and 0x0120 (TRANSPARENT_COLOR) count as transparent on the device
  function fix565(v) { if (v === 0xFFFF) return 0xFFDF; if (v === 0x0120) return 0x0100; return v; }
  function rgbOf(v) { return [((v >> 11) & 31) * 255 / 31 | 0, ((v >> 5) & 63) * 255 / 63 | 0, (v & 31) * 255 / 31 | 0]; }
  function hexOf(v) { var c = rgbOf(v); return '#' + c.map(function (x) { return ('0' + x.toString(16)).slice(-2); }).join(''); }
  function cssOf(v) { var c = rgbOf(v); return 'rgb(' + c[0] + ',' + c[1] + ',' + c[2] + ')'; }

  // Editor-Canvas
  // Editor canvas
  var ed = $('ed'), ectx = ed.getContext('2d');
  // Zoom: Vorgabe fuellt die Fensterhoehe, mindestens aber 8 px je Pixel,
  // mit -/+ von 3 bis 24 einstellbar und im Browser gemerkt.

  // Zoom: the default fills the window height, but at least 8 px per pixel,
  // adjustable with -/+ from 3 to 24 and remembered in the browser.
  var ZMIN = 3, ZMAX = 24, Z = Math.max(8, Math.min(ZMAX, Math.floor((window.innerHeight - 40) / H)));
  try { var zs = +localStorage.getItem('uhr3HdZoom'); if (zs >= ZMIN && zs <= ZMAX) Z = zs; } catch (e) { }
  function applyZoom() {
    ed.width = W * Z; ed.height = H * Z;
    $('zoomInfo').textContent = 'Zoom ' + Z + 'x';
    $('zoomOut').disabled = Z <= ZMIN; $('zoomIn').disabled = Z >= ZMAX;
  }
  function setZoom(z) {
    Z = Math.max(ZMIN, Math.min(ZMAX, z));
    try { localStorage.setItem('uhr3HdZoom', Z); } catch (e) { }
    applyZoom(); drawEditor(); updateSticky();
  }
  applyZoom();

  // Seitliche Karten bleiben beim Scrollen unter der Topbar stehen - nur solange
  // alle drei Karten nebeneinander passen, sonst wuerden sie den Editor verdecken.

  // Side cards stay put below the topbar while scrolling - only while all three
  // cards fit side by side, otherwise they would cover the editor.
  function updateSticky() {
    var root = $('hdRoot'), cards = root.querySelectorAll('.row > .card');
    var tb = document.querySelector('.topbar');
    root.style.setProperty('--hdTop', ((tb ? tb.offsetHeight : 0) + 8) + 'px');
    root.classList.remove('stickon');
    var top = cards[0].offsetTop, oneLine = true;
    for (var i = 1; i < cards.length; i++) if (cards[i].offsetTop !== top) oneLine = false;
    root.classList.toggle('stickon', oneLine);
  }
  window.addEventListener('resize', updateSticky);
  updateSticky();
  $('zoomOut').onclick = function () { setZoom(Z - 1); };
  $('zoomIn').onclick = function () { setZoom(Z + 1); };

  // Sichtbare Spalten auf der Uhr: das Zifferblatt kann schmalere Zeiger
  // vorgeben (Dateiname face_x!h!m!s.bmp), die Uhr schneidet dann mittig zu
  // (siehe pushHandRowCentered() im Geraet).

  // Columns visible on the clock: the clock face can specify narrower hands
  // (filename face_x!h!m!s.bmp), the clock then crops them in the centre
  // (see pushHandRowCentered() on the device).
  function spriteWidth(p) { var w = HD.widths && HD.widths[p]; return w > 0 ? w : W; }
  function cropOffset(p) { var sw = spriteWidth(p); return sw < W ? (W - sw) >> 1 : 0; }

  function drawEditor(buf) {
    buf = buf || pix[part];
    for (var y = 0; y < H; y++) {
      for (var x = 0; x < W; x++) {
        var v = buf[y * W + x];
        ectx.fillStyle = v < 0 ? (((x + y) & 1) ? '#2b3138' : '#1f252b') : cssOf(v);
        ectx.fillRect(x * Z, y * Z, Z, Z);
      }
    }
    if (Z >= 5) {
      ectx.strokeStyle = 'rgba(255,255,255,0.07)'; ectx.lineWidth = 1; ectx.beginPath();
      for (var gx = 1; gx < W; gx++) { ectx.moveTo(gx * Z + 0.5, 0); ectx.lineTo(gx * Z + 0.5, H * Z); }
      for (var gy = 1; gy < H; gy++) { ectx.moveTo(0, gy * Z + 0.5); ectx.lineTo(W * Z, gy * Z + 0.5); }
      ectx.stroke();
    }
    var sw = spriteWidth(part);
    if (sw < W) {
      var off = cropOffset(part);
      ectx.fillStyle = 'rgba(0,0,0,0.6)';
      ectx.fillRect(0, 0, off * Z, H * Z);
      ectx.fillRect((off + sw) * Z, 0, (W - off - sw) * Z, H * Z);
    }
    ectx.strokeStyle = 'rgba(245,166,35,0.55)'; ectx.beginPath();
    ectx.moveTo((PX + 0.5) * Z, 0); ectx.lineTo((PX + 0.5) * Z, H * Z);
    ectx.moveTo(0, (PY + 0.5) * Z); ectx.lineTo(W * Z, (PY + 0.5) * Z); ectx.stroke();
    ectx.beginPath(); ectx.arc((PX + 0.5) * Z, (PY + 0.5) * Z, Z * 0.8, 0, Math.PI * 2); ectx.stroke();
    if (polyPts.length) {
      ectx.strokeStyle = '#f5a623'; ectx.beginPath();
      polyPts.forEach(function (p, i) { if (i) ectx.lineTo(p.x * Z, p.y * Z); else ectx.moveTo(p.x * Z, p.y * Z); });
      ectx.stroke();
      polyPts.forEach(function (p) { ectx.fillStyle = '#f5a623'; ectx.fillRect(p.x * Z - 2, p.y * Z - 2, 4, 4); });
    }
  }

  // Rasterisierung mit harten Kanten ueber Pixelmitten
  // Rasterising with hard edges via pixel centres
  function plot(buf, x, y, v) {
    if (x < 0 || y < 0 || x >= W || y >= H) return;
    buf[y * W + x] = v;
    if ($('sym').checked) buf[y * W + (W - 1 - x)] = v;
  }
  function lineOn(buf, x0, y0, x1, y1, v) {
    var dx = Math.abs(x1 - x0), sx = x0 < x1 ? 1 : -1, dy = -Math.abs(y1 - y0), sy = y0 < y1 ? 1 : -1, e = dx + dy;
    for (;;) {
      plot(buf, x0, y0, v);
      if (x0 === x1 && y0 === y1) break;
      var e2 = 2 * e;
      if (e2 >= dy) { e += dy; x0 += sx; }
      if (e2 <= dx) { e += dx; y0 += sy; }
    }
  }
  function rectOn(buf, a, b, v, filled) {
    var x0 = Math.min(a.x, b.x), x1 = Math.max(a.x, b.x), y0 = Math.min(a.y, b.y), y1 = Math.max(a.y, b.y);
    for (var y = y0; y <= y1; y++) for (var x = x0; x <= x1; x++)
      if (filled || x === x0 || x === x1 || y === y0 || y === y1) plot(buf, x, y, v);
  }
  function ellOn(buf, a, b, v, filled) {
    var x0 = Math.min(a.x, b.x), x1 = Math.max(a.x, b.x), y0 = Math.min(a.y, b.y), y1 = Math.max(a.y, b.y);
    var cx = (x0 + x1 + 1) / 2, cy = (y0 + y1 + 1) / 2, rx = (x1 - x0 + 1) / 2, ry = (y1 - y0 + 1) / 2;
    function inside(x, y) { var u = (x + 0.5 - cx) / rx, w = (y + 0.5 - cy) / ry; return u * u + w * w <= 1; }
    for (var y = y0; y <= y1; y++) for (var x = x0; x <= x1; x++) {
      if (!inside(x, y)) continue;
      if (filled || !inside(x - 1, y) || !inside(x + 1, y) || !inside(x, y - 1) || !inside(x, y + 1)) plot(buf, x, y, v);
    }
  }
  function discOn(buf, cx, cy, d, v) {
    var r = d / 2;
    for (var y = Math.floor(cy - r); y <= Math.ceil(cy + r); y++)
      for (var x = Math.floor(cx - r); x <= Math.ceil(cx + r); x++) {
        var u = x + 0.5 - cx, w = y + 0.5 - cy;
        if (u * u + w * w <= r * r) plot(buf, x, y, v);
      }
  }
  function polyOn(buf, pts, v) {
    var minY = H, maxY = 0;
    pts.forEach(function (p) { minY = Math.min(minY, p.y); maxY = Math.max(maxY, p.y); });
    for (var y = Math.max(0, Math.floor(minY)); y <= Math.min(H - 1, Math.ceil(maxY)); y++) {
      for (var x = 0; x < W; x++) {
        var px = x + 0.5, py = y + 0.5, inside = false;
        for (var i = 0, j = pts.length - 1; i < pts.length; j = i++) {
          var a = pts[i], b = pts[j];
          if ((a.y > py) !== (b.y > py) && px < (b.x - a.x) * (py - a.y) / (b.y - a.y) + a.x) inside = !inside;
        }
        if (inside) plot(buf, x, y, v);
      }
    }
  }
  function floodOn(buf, x, y, v) {
    var target = buf[y * W + x];
    if (target === v) return;
    var stack = [[x, y]];
    while (stack.length) {
      var p = stack.pop(), px = p[0], py = p[1];
      if (px < 0 || py < 0 || px >= W || py >= H || buf[py * W + px] !== target) continue;
      buf[py * W + px] = v;
      stack.push([px + 1, py], [px - 1, py], [px, py + 1], [px, py - 1]);
    }
  }

  // Rueckgaengig
  // Undo
  function pushUndo() {
    undoSt[part].push(pix[part].slice());
    if (undoSt[part].length > 40) undoSt[part].shift();
    redoSt[part] = [];
  }
  function commit(buf) { pushUndo(); pix[part] = buf; setDirty(true); drawEditor(); schedulePreview(); }

  // Maus-/Stifteingabe
  // Pointer input
  var drag = null, polyPts = [];
  function cell(ev) {
    var r = ed.getBoundingClientRect();
    return { x: Math.floor((ev.clientX - r.left) * W / r.width), y: Math.floor((ev.clientY - r.top) * H / r.height) };
  }
  function inGrid(c) { return c.x >= 0 && c.y >= 0 && c.x < W && c.y < H; }
  function paintValue() { return tool === 'erase' ? TRANSPARENT : color; }

  ed.addEventListener('pointerdown', function (ev) {
    if (ev.button === 2) return;
    var c = cell(ev);
    if (!inGrid(c)) return;
    ed.setPointerCapture(ev.pointerId);
    if (tool === 'pick') {
      var pv = pix[part][c.y * W + c.x];
      if (pv >= 0) { setColor(pv); }
      pickDone();
      return;
    }
    if (tool === 'fill') {
      var fb = pix[part].slice();
      floodOn(fb, c.x, c.y, color);
      if ($('sym').checked) floodOn(fb, W - 1 - c.x, c.y, color);
      commit(fb);
      return;
    }
    if (tool === 'poly') {
      var pt = { x: c.x + 0.5, y: c.y + 0.5 };
      if (polyPts.length >= 3 && Math.abs(pt.x - polyPts[0].x) < 1 && Math.abs(pt.y - polyPts[0].y) < 1) { closePoly(); return; }
      polyPts.push(pt);
      drawEditor();
      return;
    }
    if (tool === 'pen' || tool === 'erase') {
      pushUndo();
      plot(pix[part], c.x, c.y, paintValue());
      setDirty(true);
      drag = { start: c, last: c };
      drawEditor();
      return;
    }
    drag = { start: c, last: c };
  });
  ed.addEventListener('pointermove', function (ev) {
    var c = cell(ev);
    $('posInfo').textContent = inGrid(c) ? t('pos') + ' ' + c.x + ' / ' + c.y + '   (' + t('pivot') + ' ' + PX + ' / ' + PY + ')' : '';
    if (!drag) return;
    c.x = Math.max(0, Math.min(W - 1, c.x)); c.y = Math.max(0, Math.min(H - 1, c.y));
    if (tool === 'pen' || tool === 'erase') {
      lineOn(pix[part], drag.last.x, drag.last.y, c.x, c.y, paintValue());
      drag.last = c;
      drawEditor();
      return;
    }
    drag.last = c;
    drawEditor(shapeBuf(drag.start, c));
  });
  ed.addEventListener('pointerup', function () {
    if (!drag) return;
    if (tool === 'pen' || tool === 'erase') { drag = null; schedulePreview(); return; }
    var b = shapeBuf(drag.start, drag.last);
    drag = null;
    commit(b);
  });
  ed.addEventListener('dblclick', function () { if (tool === 'poly' && polyPts.length >= 3) closePoly(); });
  document.addEventListener('keydown', function (ev) {
    if (tool !== 'poly') return;
    if (ev.key === 'Escape') { polyPts = []; drawEditor(); }
    if (ev.key === 'Enter' && polyPts.length >= 3) closePoly();
  });

  function shapeBuf(a, b) {
    var buf = pix[part].slice(), v = color;
    if (tool === 'line') lineOn(buf, a.x, a.y, b.x, b.y, v);
    else if (tool === 'rect') rectOn(buf, a, b, v, false);
    else if (tool === 'rectf') rectOn(buf, a, b, v, true);
    else if (tool === 'ell') ellOn(buf, a, b, v, false);
    else if (tool === 'ellf') ellOn(buf, a, b, v, true);
    return buf;
  }
  function closePoly() {
    var buf = pix[part].slice();
    polyOn(buf, polyPts, color);
    polyPts = [];
    commit(buf);
  }

  // Werkzeugleiste
  // Toolbar
  function button(parent, label, onClick, cls) {
    var b = document.createElement('button');
    b.type = 'button'; b.className = cls || 'tb'; b.textContent = label; b.onclick = onClick;
    parent.appendChild(b); return b;
  }
  var partBtn = {}, toolBtn = {};
  PARTS.forEach(function (p) { partBtn[p] = button($('partBtns'), t(p), function () { selectPart(p); }); });
  ['pen', 'erase', 'line', 'rect', 'rectf', 'ell', 'ellf', 'poly', 'fill', 'pick'].forEach(function (k) {
    toolBtn[k] = button($('toolBtns'), t(k), function () { pickReturn = null; selectTool(k); });
    toolBtn[k].title = t('h_' + k);
  });
  function selectPart(p) {
    part = p; polyPts = [];
    PARTS.forEach(function (q) { partBtn[q].className = 'tb' + (q === p ? ' on' : ''); });
    $('widthHint').textContent = spriteWidth(p) < W ? t('widthHint', spriteWidth(p)) : '';
    fillGenForm(); fillCopySel(); drawEditor();
  }
  function selectTool(k) {
    tool = k; polyPts = [];
    Object.keys(toolBtn).forEach(function (q) { toolBtn[q].className = 'tb' + (q === k ? ' on' : ''); });
    $('pickBtn').className = 'tb' + (k === 'pick' ? ' on' : '');
    $('toolHint').textContent = t('h_' + k);
    drawEditor();
  }
  // src = das Eingabefeld, aus dem die Farbe kommt - dessen Wert bleibt beim Tippen/Ziehen unangetastet
  // src = the input the colour comes from - its value stays untouched while typing/dragging
  function setColor(v, src) {
    color = fix565(v);
    if (src !== 'picker') $('color').value = hexOf(color);
    if (src !== 'hex') $('colorHex').value = hexOf(color);
    $('colorHint').textContent = (v === 0xFFFF) ? t('whiteHint') : '';
    colorCells.forEach(function (c) { c.classList.toggle('on', +c.dataset.v === color); });
  }
  $('color').addEventListener('input', function () { setColor(to565(this.value), 'picker'); });
  $('colorHex').addEventListener('input', function () {
    var h = this.value.trim();
    if (/^#?[0-9a-f]{6}$/i.test(h)) setColor(to565(h.charAt(0) === '#' ? h : '#' + h), 'hex');
    else if (/^0x[0-9a-f]{1,4}$/i.test(h)) setColor(parseInt(h, 16), 'hex');
  });
  $('colorHex').addEventListener('change', function () { this.value = hexOf(color); });
  var colorCells = [];
  function addCell(parent, cls, v) {
    var c = document.createElement('span');
    if (cls) c.className = cls;
    c.style.background = cssOf(v); c.title = hexOf(v); c.dataset.v = v;
    c.onclick = function () { setColor(v); };
    parent.appendChild(c); colorCells.push(c);
  }
  [0x0000, 0x4208, 0x8410, 0xF800, 0xB000, 0xFD20, 0xC618, 0xFFDF, 0x001F, 0x07E0].forEach(function (v) { addCell($('swatches'), 'sw', v); });

  // Palette: Graustufen, 16 Farbtoene in 7 Helligkeiten, 2 Reihen gedeckte Toene - alles
  // bereits auf RGB565 gerundet, damit die Palette genau die Farben der Uhr zeigt.

  // Palette: greyscale, 16 hues in 7 lightness levels, 2 rows of muted tones - all already
  // rounded to RGB565, so the palette shows exactly the colours of the clock.
  function hsl565(h, sat, l) {
    var a = sat * Math.min(l, 1 - l);
    function f(n) { var k = (n + h / 30) % 12; return Math.round(255 * (l - a * Math.max(-1, Math.min(k - 3, 9 - k, 1)))); }
    return fix565(to565('#' + [f(0), f(8), f(4)].map(function (x) { return ('0' + x.toString(16)).slice(-2); }).join('')));
  }
  var palRows = [[0, 0]].concat([.2, .3, .4, .5, .6, .7, .8].map(function (l) { return [1, l]; }), [[.4, .35], [.4, .65]]);
  palRows.forEach(function (r, ri) {
    for (var i = 0; i < 16; i++) addCell($('palette'), '', ri === 0 ? hsl565(0, 0, i / 15) : hsl565(i * 22.5, r[0], r[1]));
  });

  $('undoBtn').onclick = function () {
    if (!undoSt[part].length) return;
    redoSt[part].push(pix[part]); pix[part] = undoSt[part].pop(); setDirty(true); drawEditor(); schedulePreview();
  };
  $('redoBtn').onclick = function () {
    if (!redoSt[part].length) return;
    undoSt[part].push(pix[part]); pix[part] = redoSt[part].pop(); setDirty(true); drawEditor(); schedulePreview();
  };
  $('clearBtn').onclick = function () {
    if (confirm(t('confirmClear'))) commit(new Int32Array(N).fill(TRANSPARENT));
  };
  Array.prototype.forEach.call(document.querySelectorAll('.hd [data-shift]'), function (b) {
    b.onclick = function () {
      var d = b.getAttribute('data-shift').split(','), dx = +d[0], dy = +d[1];
      var src = pix[part], buf = new Int32Array(N).fill(TRANSPARENT);
      for (var y = 0; y < H; y++) for (var x = 0; x < W; x++) {
        var nx = x + dx, ny = y + dy;
        if (nx >= 0 && ny >= 0 && nx < W && ny < H) buf[ny * W + nx] = src[y * W + x];
      }
      commit(buf);
    };
  });
  function fillCopySel() {
    var s = $('copySel'); s.innerHTML = '';
    PARTS.forEach(function (p) { if (p !== part) { var o = document.createElement('option'); o.value = p; o.textContent = t(p); s.appendChild(o); } });
  }
  $('copyBtn').onclick = function () {
    var target = $('copySel').value, keep = part;
    part = target; commit(pix[keep].slice()); part = keep; drawEditor();
  };

  // Formgenerator
  // Shape generator
  var genFields = { gLen: 'len', gTipW: 'tipW', gBaseW: 'baseW', gTail: 'tail', gTailW: 'tailW', gDisc: 'disc', gDiscPos: 'discPos' };
  function fillGenForm() {
    var g = gen[part];
    Object.keys(genFields).forEach(function (id) { $(id).value = g[genFields[id]]; });
    $('gLen').max = PY; $('gTail').max = tailMax; $('gTipW').max = W; $('gBaseW').max = W; $('gTailW').max = W; $('gDisc').max = W; $('gDiscPos').max = PY;
  }
  $('genBtn').onclick = function () {
    var g = gen[part];
    Object.keys(genFields).forEach(function (id) { g[genFields[id]] = Math.max(0, +$(id).value || 0); });
    g.len = Math.min(g.len, PY + 0.5); g.tail = Math.min(g.tail, tailMax + 0.5);
    if (pix[part].some(function (v) { return v >= 0; }) && !confirm(t('confirmGen'))) return;
    var cx = PX + 0.5, cy = PY + 0.5, buf = new Int32Array(N).fill(TRANSPARENT);
    polyOn(buf, [
      { x: cx - g.tipW / 2, y: cy - g.len }, { x: cx + g.tipW / 2, y: cy - g.len },
      { x: cx + g.baseW / 2, y: cy }, { x: cx + g.tailW / 2, y: cy + g.tail },
      { x: cx - g.tailW / 2, y: cy + g.tail }, { x: cx - g.baseW / 2, y: cy }
    ], color);
    if (g.disc > 0) discOn(buf, cx, cy - g.discPos, g.disc, color);
    commit(buf);
  };

  // BMP lesen/schreiben (Format wie encodeBmpToBytes() im Geraet)
  // BMP read/write (format like encodeBmpToBytes() on the device)
  function parseBmp(ab) {
    var dv = new DataView(ab);
    if (dv.getUint8(0) !== 66 || dv.getUint8(1) !== 77) throw new Error('no BMP');
    var off = dv.getUint32(10, true), w = dv.getInt32(18, true), h = dv.getInt32(22, true), bpp = dv.getUint16(28, true);
    if ([16, 24, 32].indexOf(bpp) < 0) throw new Error(bpp + ' bpp');
    var topDown = h < 0, ah = Math.abs(h), rs = Math.floor((w * bpp / 8 + 3) / 4) * 4;
    return { w: w, h: ah, px: function (x, y) {
      var p = off + (topDown ? y : ah - 1 - y) * rs + x * (bpp / 8);
      if (bpp === 16) return dv.getUint16(p, true);
      return ((dv.getUint8(p + 2) >> 3) << 11) | ((dv.getUint8(p + 1) >> 2) << 5) | (dv.getUint8(p) >> 3);
    } };
  }
  // Alte Zeiger (HD.lh hoch) unten buendig einsetzen - Drehpunkt und Stueck
  // darunter liegen dann wie im neuen Format, oben bleibt es transparent.

  // Place old hands (HD.lh high) flush at the bottom - pivot and the part
  // below it then sit like in the new format, the top stays transparent.
  function decodeBmp(ab) {
    var img = parseBmp(ab), out = new Int32Array(N).fill(TRANSPARENT);
    var off = (img.w === W && img.h === HD.lh && HD.lh < H) ? H - HD.lh : 0, sh = H - off;
    for (var ty = 0; ty < sh; ty++) for (var tx = 0; tx < W; tx++) {
      var v = img.px(Math.floor(tx * img.w / W), Math.floor(ty * img.h / sh));
      out[(ty + off) * W + tx] = (v === 0xFFFF || v === 0x0120) ? TRANSPARENT : v;
    }
    return out;
  }
  // Gespeichert wird im alten Format (HD.lh hoch), solange der Zeiger nicht in
  // die zusaetzlichen Zeilen oben ragt - so laeuft er auch auf aelterer Firmware.

  // Saved in the old format (HD.lh high) as long as the hand does not reach into
  // the extra rows at the top - so it also runs on older firmware.
  function encodeBmp(buf) {
    var top = H > HD.lh ? H - HD.lh : 0;
    for (var i = 0; i < top * W; i++) if (buf[i] >= 0) { top = 0; break; }
    var oh = H - top, rs = Math.floor((W * 2 + 3) / 4) * 4, hs = 66, size = hs + rs * oh;
    var ab = new ArrayBuffer(size), dv = new DataView(ab);
    dv.setUint8(0, 66); dv.setUint8(1, 77); dv.setUint32(2, size, true); dv.setUint32(10, hs, true);
    dv.setUint32(14, 40, true); dv.setInt32(18, W, true); dv.setInt32(22, -oh, true); dv.setUint16(26, 1, true);
    dv.setUint16(28, 16, true); dv.setUint32(30, 3, true); dv.setUint32(34, rs * oh, true);
    dv.setUint32(54, 0xF800, true); dv.setUint32(58, 0x07E0, true); dv.setUint32(62, 0x001F, true);
    for (var y = 0; y < oh; y++) for (var x = 0; x < W; x++) {
      var v = buf[(y + top) * W + x];
      dv.setUint16(hs + y * rs + x * 2, v < 0 ? 0xFFFF : fix565(v), true);
    }
    return new Blob([ab], { type: 'image/bmp' });
  }

  // Laden und Speichern
  // Load and save
  function showMsg(text, ok) { $('msg').className = ok === undefined ? '' : (ok ? 'ok' : 'err'); $('msg').textContent = text; }
  // Basis ist immer das aktive Design - fehlt dort ein Zeiger, nimmt die Uhr
  // den eingebauten Standard, daher hier genauso.

  // The basis is always the active design - if a hand is missing there, the
  // clock uses the built-in default, so the same happens here.
  function activeBase() { return (HD.active && HD.sets.indexOf(HD.active) >= 0) ? HD.active : 'default'; }
  function showBase() {
    var b = activeBase();
    $('baseInfo').textContent = t('base') + ' ' + (b === 'default' ? t('builtin') : t('set') + ' ' + b) + ' (' + t('active') + ')';
  }
  function nextId() {
    var max = 0;
    HD.sets.forEach(function (id) { if (/^\d+$/.test(id)) max = Math.max(max, +id); });
    return String(max + 1);
  }
  function fetchHand(url) {
    return fetch(url, { cache: 'no-store' }).then(function (r) {
      if (!r.ok) throw new Error(r.status);
      return r.arrayBuffer();
    }).then(decodeBmp);
  }
  function loadActive() {
    var id = activeBase(), notes = [];
    showMsg(t('loading'));
    Promise.all(PARTS.map(function (p) {
      var def = '/api/defaulthand?part=' + p;
      var url = id === 'default' ? def : '/file?name=' + encodeURIComponent('/hand_set' + id + '_' + p + '.bmp');
      return fetchHand(url).catch(function () {
        if (id !== 'default') notes.push(t(p) + ' ' + t('missing'));
        return fetchHand(def);
      }).then(function (buf) { pix[p] = buf; }).catch(function () { pix[p] = new Int32Array(N).fill(TRANSPARENT); });
    })).then(function () {
      PARTS.forEach(function (p) { undoSt[p] = []; redoSt[p] = []; });
      setDirty(false);
      showBase(); drawEditor(); schedulePreview();
      showMsg(t('loaded') + (notes.length ? ' - ' + notes.join(', ') : ''), true);
    });
  }
  $('resetBtn').onclick = function () {
    if (dirty && !confirm(t('confirmReset'))) return;
    loadActive();
  };
  // Alle drei Zeiger als Satz id hochladen (vorhandene Dateien werden ueberschrieben),
  // optional aktivieren - /sethandset laedt die Zeiger auf der Uhr neu.

  // Upload all three hands as set id (existing files are overwritten),
  // optionally activate - /sethandset reloads the hands on the clock.
  function saveSet(id, activate, isNew) {
    $('saveBtn').disabled = true; $('saveCurBtn').disabled = true;
    showMsg(t('saving'));
    // Nacheinander hochladen und Weiterleitung NICHT folgen (sonst baut das
    // Geraet jedes Mal die komplette /handsets-Seite auf).

    // Upload one after another and do NOT follow the redirect (otherwise the
    // device builds the complete /handsets page every time).
    var chain = Promise.resolve();
    PARTS.forEach(function (p) {
      chain = chain.then(function () {
        var fd = new FormData();
        fd.append('upload', encodeBmp(pix[p]), 'hand_set' + id + '_' + p + '.bmp');
        return fetch('/uploadhandset', { method: 'POST', body: fd, redirect: 'manual' }).then(function (r) {
          if (!(r.ok || r.type === 'opaqueredirect')) throw new Error(t(p) + ' (' + r.status + ')');
        });
      });
    });
    chain.then(function () {
      if (!activate) return;
      return fetch('/sethandset?set=' + encodeURIComponent(id), { redirect: 'manual' }).then(function () { HD.active = id; });
    }).then(function () {
      if (isNew) HD.sets.push(id);
      setDirty(false);
      showBase();
      showMsg(isNew ? t('saved', id) + (activate ? t('activated') : '') : t('savedCur', id), true);
    }).catch(function (e) {
      showMsg(t('failed') + e.message, false);
      setDirty(true);
    });
  }
  $('saveBtn').onclick = function () { saveSet(nextId(), $('activate').checked, true); };
  $('saveCurBtn').onclick = function () {
    var id = activeBase();
    if (id === 'default') return;
    saveSet(id, true, false);
  };
  window.addEventListener('beforeunload', function (ev) { if (dirty) { ev.preventDefault(); ev.returnValue = ''; } });

  // Vorschau
  // Preview
  var pv = $('pv'), pctx = pv.getContext('2d'), S = HD.cw;
  pv.width = S; pv.height = S;
  var faceCanvas = null, previewTimer = null;
  ['bgFace', 'bgDark', 'bgLight'].forEach(function (k) {
    var o = document.createElement('option'); o.value = k; o.textContent = t(k); $('bgSel').appendChild(o);
  });
  $('bgSel').onchange = schedulePreview;
  $('live').onchange = schedulePreview;

  // Zifferblatt selbst dekodieren statt per <img> - nicht jeder Browser
  // zeigt 16-Bit-BMPs an; das Standard-Zifferblatt liegt in der Firmware.

  // Decode the clock face ourselves instead of via <img> - not every browser
  // displays 16-bit BMPs; the default clock face lives in the firmware.
  if (HD.face) {
    var faceUrl = HD.face === '/face_default.bmp' ? '/api/defaultface' : '/file?name=' + encodeURIComponent(HD.face);
    fetch(faceUrl, { cache: 'no-store' }).then(function (r) {
      if (!r.ok) throw new Error(r.status);
      return r.arrayBuffer();
    }).then(function (ab) {
      var img = parseBmp(ab), c = document.createElement('canvas');
      c.width = img.w; c.height = img.h;
      var cx = c.getContext('2d'), id = cx.createImageData(img.w, img.h);
      for (var y = 0; y < img.h; y++) for (var x = 0; x < img.w; x++) {
        var rgb = rgbOf(img.px(x, y)), i = (y * img.w + x) * 4;
        id.data[i] = rgb[0]; id.data[i + 1] = rgb[1]; id.data[i + 2] = rgb[2]; id.data[i + 3] = 255;
      }
      cx.putImageData(id, 0, 0);
      faceCanvas = c;
      schedulePreview();
    }).catch(function () {});
  }

  // Zeiger wie auf der Uhr: Sprite in Zeigerbreite des Zifferblatts, Bitmap
  // mittig zugeschnitten, Drehpunkt bei halber Sprite-Breite.

  // Hand like on the clock: sprite at the face's hand width, bitmap cropped
  // in the centre, pivot at half the sprite width.
  function handCanvas(buf, p) {
    var sw = spriteWidth(p), off = cropOffset(p);
    var c = document.createElement('canvas'); c.width = sw; c.height = H;
    var cx = c.getContext('2d'), id = cx.createImageData(sw, H);
    for (var y = 0; y < H; y++) for (var x = 0; x < sw; x++) {
      var sx = x + off;
      if (sx >= W) continue;
      var v = buf[y * W + sx];
      if (v < 0) continue;
      var rgb = rgbOf(v), i = (y * sw + x) * 4;
      id.data[i] = rgb[0]; id.data[i + 1] = rgb[1]; id.data[i + 2] = rgb[2]; id.data[i + 3] = 255;
    }
    cx.putImageData(id, 0, 0);
    return c;
  }
  function renderPreview() {
    var bg = $('bgSel').value;
    pctx.imageSmoothingEnabled = true;
    pctx.fillStyle = bg === 'bgLight' ? '#f2f2f2' : '#111';
    pctx.fillRect(0, 0, S, S);
    if (bg === 'bgFace' && faceCanvas) pctx.drawImage(faceCanvas, 0, 0, S, S);
    var d = $('live').checked ? new Date() : new Date(2000, 0, 1, 10, 8, 37);
    var hh = d.getHours() % 12, mm = d.getMinutes(), ss = d.getSeconds();
    var ang = { hour: (hh + mm / 60) * 30, minute: (mm + ss / 60) * 6, second: ss * 6 };
    PARTS.forEach(function (p) {
      pctx.save();
      pctx.translate(S / 2, S / 2);
      pctx.rotate(ang[p] * Math.PI / 180);
      pctx.drawImage(handCanvas(pix[p], p), -((spriteWidth(p) >> 1) + 0.5), -(PY + 0.5));
      pctx.restore();
    });
    if (HD.hub > 0) {
      pctx.fillStyle = HD.hubColor;
      pctx.beginPath(); pctx.arc(S / 2, S / 2, HD.hub, 0, Math.PI * 2); pctx.fill();
    }
  }
  function schedulePreview() {
    if (previewTimer) clearTimeout(previewTimer);
    previewTimer = setTimeout(renderPreview, 60);
  }
  setInterval(function () { if ($('live').checked && !document.hidden) renderPreview(); }, 1000);

  // Farbe aufnehmen: der Button schaltet einmalig auf die Pipette und danach zurueck,
  // Rechtsklick in die Zeichenflaeche und Klick in die Vorschau nehmen direkt auf.

  // Pick colour: the button switches to the picker once and back afterwards,
  // right-click in the drawing area and click in the preview pick directly.
  var pickReturn = null;
  function pickDone() { if (pickReturn) { var k = pickReturn; pickReturn = null; selectTool(k); } }
  $('pickBtn').onclick = function () {
    if (tool === 'pick') { var k = pickReturn || 'pen'; pickReturn = null; selectTool(k); return; }
    pickReturn = tool; selectTool('pick');
  };
  ed.addEventListener('contextmenu', function (ev) {
    ev.preventDefault();
    var c = cell(ev);
    if (inGrid(c)) { var v = pix[part][c.y * W + c.x]; if (v >= 0) setColor(v); };
  });
  pv.style.cursor = 'crosshair';
  pv.addEventListener('click', function (ev) {
    var r = pv.getBoundingClientRect();
    var d = pctx.getImageData(Math.floor((ev.clientX - r.left) * pv.width / r.width), Math.floor((ev.clientY - r.top) * pv.height / r.height), 1, 1).data;
    setColor(to565('#' + [d[0], d[1], d[2]].map(function (x) { return ('0' + x.toString(16)).slice(-2); }).join('')));
    pickDone();
  });

  // Beschriftungen
  // Labels
  var labels = { tActivate: 'activate', tPart: 'part', tTools: 'tools', tSym: 'sym',
    tColor: 'color', tStd: 'std', tPal: 'pal', tEdit: 'edit', tShift: 'shift', tCopy: 'copy', tGen: 'gen', tLen: 'len', tTipW: 'tipW',
    tBaseW: 'baseW', tTail: 'tail', tTailW: 'tailW', tDisc: 'disc', tDiscPos: 'discPos', tPreview: 'preview', tLive: 'live' };
  Object.keys(labels).forEach(function (id) { $(id).textContent = t(labels[id]); });
  $('resetBtn').textContent = t('reset'); $('saveBtn').textContent = t('saveBtn'); $('saveCurBtn').textContent = t('saveCurBtn');
  $('pickBtn').textContent = t('pickBtn'); $('pickBtn').title = t('pickTip');
  $('color').title = t('free'); $('colorHex').placeholder = '#RRGGBB'; $('colorHex').title = t('hexHint');
  $('undoBtn').textContent = t('undo'); $('redoBtn').textContent = t('redo'); $('clearBtn').textContent = t('clear');
  $('genBtn').textContent = t('genBtn');

  // Start
  // Start
  selectPart('hour');
  selectTool('pen');
  setColor(color);
  loadActive();
})();
</script>
)HDRAW";
