#pragma once
    // Zifferblatt-Designer (Seite /facedesigner): Editor-HTML, -CSS und -Skript,
    // direkt aus dem Flash gestreamt (ohne Heap-Kopie), nach dem Seitenkopf.
    // Die Konfiguration kommt aus dem vorher gesendeten JS-Objekt FD.

    // Clock face designer (page /facedesigner): editor HTML, CSS and script,
    // streamed straight from flash (no heap copy), after the page header.
    // Configuration comes from the FD JS object sent right before.

static const char FACE_DESIGNER_HTML[] PROGMEM = R"FDRAW(
<style>
.hd{max-width:1500px;margin:0 auto;text-align:left}
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
.hd .num{width:72px;padding:6px 4px 6px 8px}
.hd .num2{width:64px;padding:6px 4px 6px 8px}
.hd .clr{width:34px;height:28px;padding:0;vertical-align:middle}
.hd label{white-space:nowrap;display:inline-flex;align-items:center;gap:4px;margin:3px 6px 3px 0}
.hd .grid2{display:grid;grid-template-columns:auto auto;gap:2px 8px;align-items:center}
.hd .edcard{flex:0 1 auto;min-width:200px}
.hd .edwrap{overflow:auto;max-width:100%;max-height:calc(100vh - var(--hdTop,70px) - 110px)}
@media (min-width:900px){.hd .row.main{flex-wrap:nowrap}}
.hd.stickon .stick{position:sticky;top:var(--hdTop,70px);max-height:calc(100vh - var(--hdTop,70px) - 8px);overflow-y:auto}
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
      <span id="tName"></span><input type="text" id="faceName" maxlength="20" spellcheck="false" style="width:130px">
      <label><input type="checkbox" id="activate" checked><span id="tActivate"></span></label>
      <button type="button" id="saveBtn"></button>
    </div>
    <small id="spaceHint"></small>
    <div id="msg"></div>
  </div>

  <div class="row main">
    <div class="card stick" style="max-width:250px">
      <h3 id="tTools"></h3>
      <div id="toolBtns"></div>
      <small id="toolHint"></small>
      <span id="tBw"></span> <input type="number" class="num2" id="bw" min="1" max="15" value="1"><br>
      <span id="tSym"></span> <select id="symSel"></select>
      <div id="textOpts" style="display:none">
        <input type="text" id="txtVal" value="12" style="width:200px"><br>
        <span id="tTxtSize"></span> <input type="number" class="num2" id="txtSize" min="4" max="120">
        <select id="txtFont"><option value="sans-serif">Sans</option><option value="serif">Serif</option><option value="monospace">Mono</option></select>
        <label><input type="checkbox" id="txtBold" checked>B</label><br>
        <span id="tTxtMode"></span> <select id="txtMode"></select>
        <span id="rotRow"><br><span id="tTxtRot"></span> <input type="number" class="num2" id="txtRot" min="-360" max="360" value="0"> &deg;</span>
      </div>
      <h3 id="tColor"></h3>
      <input type="color" id="color" value="#000000" style="padding:0;height:32px;width:48px;vertical-align:middle">
      <input type="text" id="colorHex" class="hex" maxlength="7" spellcheck="false" style="vertical-align:middle">
      <button type="button" class="tb" id="pickBtn" style="vertical-align:middle"></button>
      <small id="tStd"></small>
      <div id="swatches"></div>
      <small id="tPal"></small>
      <div id="palette" class="pal"></div>
      <h3 id="tEdit"></h3>
      <button type="button" class="tb" id="undoBtn"></button>
      <button type="button" class="tb" id="redoBtn"></button>
      <button type="button" class="tb" id="clearBtn"></button>
      <h3 id="tImport"></h3>
      <select id="fitSel"></select>
      <input type="file" id="imgFile" accept="image/*,.bmp" style="max-width:220px">
      <h3 id="tLogo"></h3>
      <input type="file" id="logoFile" accept="image/*,.bmp" style="max-width:220px"><br>
      <span id="tLogoW"></span> <input type="number" class="num2" id="logoW" min="2"> px
      <small id="logoHint"></small>
      <h3 id="tFonts"></h3>
      <input type="text" id="fontName" spellcheck="false" style="width:140px"><button type="button" class="tb" id="fontAddBtn"></button>
      <small id="fontFileHint"></small>
      <input type="file" id="fontFile" accept=".ttf,.otf,.woff,.woff2" style="max-width:220px">
    </div>

    <div class="card edcard">
      <div style="text-align:center">
        <button type="button" class="tb" id="zoomOut">&minus;</button>
        <span id="zoomInfo" style="display:inline-block;min-width:70px"></span>
        <button type="button" class="tb" id="zoomIn">+</button>
      </div>
      <div class="edwrap"><canvas id="ed"></canvas></div>
      <small id="posInfo" style="text-align:center"></small>
    </div>

    <div class="card stick" style="max-width:340px">
      <h3 id="tGen"></h3>
      <div class="grid2">
        <span id="tBg"></span><span><input type="color" class="clr" id="gBg"></span>
        <span id="tRing"></span><span><input type="number" class="num2" id="gRing" min="0"><input type="color" class="clr" id="gRingC"></span>
        <span id="tHourM"></span><span><input type="number" class="num2" id="gHLen" min="0"><input type="number" class="num2" id="gHW" min="0"><input type="color" class="clr" id="gHC"></span>
        <span id="tMinM"></span><span><input type="number" class="num2" id="gMLen" min="0"><input type="number" class="num2" id="gMW" min="0"><input type="color" class="clr" id="gMC"></span>
        <span id="tNum"></span><span><select id="gNum"></select></span>
        <span id="tNSize"></span><span><input type="number" class="num2" id="gNSize" min="4"><input type="color" class="clr" id="gNC"></span>
        <span id="tNDist"></span><span><input type="number" class="num2" id="gNDist" min="0"></span>
        <span id="tNFont"></span><span><select id="gNFont"><option value="sans-serif">Sans</option><option value="serif">Serif</option><option value="monospace">Mono</option></select><label><input type="checkbox" id="gNBold" checked>B</label></span>
      </div>
      <small id="genHint"></small>
      <button type="button" id="genBtn"></button>
      <h3 id="tPreview"></h3>
      <canvas id="pv"></canvas>
      <label><input type="checkbox" id="live" checked><span id="tLive"></span></label>
      <label><input type="checkbox" id="showHands" checked><span id="tShowHands"></span></label>
    </div>
  </div>
</div>

<script>
(function () {
  var W = FD.w, H = FD.h, N = W * H, CX = W / 2, CY = H / 2, R = Math.min(W, H) / 2;
  var DEF = '/face_default.bmp';

  var TX = {
    de: { base: 'Basis:', builtin: 'Standard (eingebaut)', active: 'aktiv', reset: 'Aenderungen verwerfen',
      activate: 'neues Zifferblatt aktivieren', saveBtn: 'Als neues Zifferblatt speichern', name: 'Name:',
      saveCurBtn: 'Aktuelles Zifferblatt speichern und anwenden', tools: 'Werkzeug', bw: 'Stiftbreite:',
      pen: 'Stift', line: 'Linie', rect: 'Rahmen', rectf: 'Rechteck', ell: 'Ellipse', ellf: 'Ellipse gefuellt',
      poly: 'Polygon', fill: 'Fuellen', text: 'Text', pick: 'Pipette', sym: 'Symmetrie:', txtSize: 'Groesse:',
      none: 'keine', mx: 'links/rechts gespiegelt', mxy: '4-fach gespiegelt', r4: '4-fach gedreht',
      r12: '12-fach gedreht (Stunden)', r60: '60-fach gedreht (Minuten)',
      pickBtn: 'Aufnehmen', pickTip: 'Farbe aus einem Pixel aufnehmen - auch per Rechtsklick in die Zeichenflaeche oder Klick in die Vorschau',
      color: 'Farbe', std: 'Standardfarben', pal: 'Palette', free: 'Beliebige Farbe', hexHint: '#RRGGBB oder RGB565 (0xFFFF)',
      edit: 'Bearbeiten', undo: 'Rueckgaengig', redo: 'Wiederholen', clear: 'Alles fuellen',
      import: 'Bild laden', cover: 'Flaeche fuellen', contain: 'ganz zeigen',
      gen: 'Zifferblatt erzeugen', bg: 'Hintergrund', ring: 'Rand', hourM: 'Stundenstriche', minM: 'Minutenstriche',
      num: 'Ziffern', nSize: 'Ziffern-Groesse', nDist: 'Ziffern Abstand', nFont: 'Schrift',
      numNone: 'keine', numArabic: '1 - 12', numQuarter: '12, 3, 6, 9', numRoman: 'I - XII',
      genHint: 'Striche: Laenge und Breite in Pixeln. Abstand: Ziffernmitte bis Rand.', genBtn: 'Erzeugen',
      preview: 'Vorschau', live: 'Live-Uhrzeit', showHands: 'Zeiger',
      saving: 'Speichere...', saved: 'Als neues Zifferblatt "{0}" gespeichert', activated: ' und aktiviert',
      savedCur: '"{0}" gespeichert und angewendet', loading: 'Lade...', loaded: 'Aktives Zifferblatt geladen',
      failed: 'Fehler: ', missing: 'Aktives Zifferblatt nicht lesbar - Standard verwendet',
      builtinRO: 'Das eingebaute Standard-Zifferblatt kann nicht ueberschrieben werden - bitte als neues Zifferblatt speichern.',
      badName: 'Name: nur Buchstaben, Ziffern, _ und -, hoechstens 20 Zeichen.',
      confirmExists: 'Das Zifferblatt "{0}" gibt es schon - ueberschreiben?',
      confirmReset: 'Alle Aenderungen verwerfen und das aktive Zifferblatt neu laden?',
      confirmClear: 'Das ganze Zifferblatt mit der aktuellen Farbe fuellen?',
      confirmGen: 'Aktuelles Zifferblatt durch das erzeugte ersetzen?',
      imgErr: 'Bild konnte nicht gelesen werden.',
      lowSpace: 'Wenig freier Speicher ({0} KB) - ein neues Zifferblatt braucht bis zu {1} KB.',
      h_pen: 'Stift: Pixel einzeln setzen oder freihand zeichnen, in der eingestellten Stiftbreite.',
      h_line: 'Linie: vom Anfangs- zum Endpunkt ziehen.', h_rect: 'Rahmen: Rechteck-Umriss aufziehen.',
      h_rectf: 'Rechteck: gefuelltes Rechteck aufziehen.',
      h_ell: 'Ellipse: Umriss aufziehen - ein Quadrat ergibt einen Kreis.',
      h_ellf: 'Ellipse gefuellt: gefuellte Ellipse oder Kreis aufziehen.',
      h_fill: 'Fuellen: faerbt die zusammenhaengende gleichfarbige Flaeche um.',
      h_pick: 'Pipette: Klick uebernimmt die Farbe des Pixels.',
      h_poly: 'Polygon: Punkte anklicken, Doppelklick oder Klick auf den ersten Punkt schliesst, Esc bricht ab.',
      h_text: 'Text: gerade - Klick setzt den Text mittig an die Stelle; im Bogen - Klick legt Radius und Mitte des Bogens fest. Einstellungen darunter.',
      h_stamp: 'Logo: unter "Logo" ein Bild laden und die Breite einstellen, dann per Klick mittig an die Stelle setzen.',
      stamp: 'Logo', txtMode: 'Anordnung:', straight: 'gerade', arcTop: 'Bogen oben', arcBottom: 'Bogen unten', txtRot: 'Drehung:',
      logo: 'Logo', logoW: 'Breite:', logoHint: 'Ideal ist ein PNG mit transparentem Hintergrund. Das Bild bleibt fuer weitere Klicks geladen.',
      noLogo: 'Zuerst unter "Logo" ein Bild laden.', fonts: 'Eigene Schrift', fontAdd: 'Hinzufuegen', fontPh: 'installierte Schrift',
      fontFileHint: 'oder Schriftdatei (.ttf, .otf, .woff) laden - gilt bis zum Neuladen der Seite, auch fuer die Ziffern im Generator.',
      fontAdded: 'Schrift "{0}" hinzugefuegt und ausgewaehlt', fontMissing: 'Die Schrift "{0}" ist auf diesem PC nicht installiert.',
      fontErr: 'Schriftdatei konnte nicht gelesen werden.',
      roundHint: 'Rundes Display: der abgedunkelte Bereich ist auf der Uhr nicht sichtbar.',
      pos: 'Pixel', center: 'Mitte' },
    en: { base: 'Based on:', builtin: 'Default (built-in)', active: 'active', reset: 'Discard changes',
      activate: 'activate new clock face', saveBtn: 'Save as new clock face', name: 'Name:',
      saveCurBtn: 'Save and apply current clock face', tools: 'Tool', bw: 'Pen width:',
      pen: 'Pen', line: 'Line', rect: 'Frame', rectf: 'Rectangle', ell: 'Ellipse', ellf: 'Filled ellipse',
      poly: 'Polygon', fill: 'Fill', text: 'Text', pick: 'Picker', sym: 'Symmetry:', txtSize: 'Size:',
      none: 'none', mx: 'mirrored left/right', mxy: 'mirrored 4 ways', r4: 'rotated 4 times',
      r12: 'rotated 12 times (hours)', r60: 'rotated 60 times (minutes)',
      pickBtn: 'Pick', pickTip: 'Pick the colour of a pixel - also by right-clicking the drawing area or clicking the preview',
      color: 'Colour', std: 'Standard colours', pal: 'Palette', free: 'Any colour', hexHint: '#RRGGBB or RGB565 (0xFFFF)',
      edit: 'Edit', undo: 'Undo', redo: 'Redo', clear: 'Fill all',
      import: 'Load image', cover: 'fill area', contain: 'show whole',
      gen: 'Generate clock face', bg: 'Background', ring: 'Rim', hourM: 'Hour marks', minM: 'Minute marks',
      num: 'Numerals', nSize: 'Numeral size', nDist: 'Numeral distance', nFont: 'Font',
      numNone: 'none', numArabic: '1 - 12', numQuarter: '12, 3, 6, 9', numRoman: 'I - XII',
      genHint: 'Marks: length and width in pixels. Distance: numeral centre to edge.', genBtn: 'Generate',
      preview: 'Preview', live: 'Live time', showHands: 'Hands',
      saving: 'Saving...', saved: 'Saved as new clock face "{0}"', activated: ' and activated',
      savedCur: '"{0}" saved and applied', loading: 'Loading...', loaded: 'Active clock face loaded',
      failed: 'Error: ', missing: 'Active clock face not readable - default used',
      builtinRO: 'The built-in default clock face cannot be overwritten - please save as a new clock face.',
      badName: 'Name: letters, digits, _ and - only, at most 20 characters.',
      confirmExists: 'The clock face "{0}" already exists - overwrite?',
      confirmReset: 'Discard all changes and reload the active clock face?',
      confirmClear: 'Fill the whole clock face with the current colour?',
      confirmGen: 'Replace the current clock face with the generated one?',
      imgErr: 'Image could not be read.',
      lowSpace: 'Little free space ({0} KB) - a new clock face needs up to {1} KB.',
      h_pen: 'Pen: set single pixels or draw freehand, at the chosen pen width.',
      h_line: 'Line: drag from the start to the end point.', h_rect: 'Frame: drag out a rectangle outline.',
      h_rectf: 'Rectangle: drag out a filled rectangle.',
      h_ell: 'Ellipse: drag out an outline - a square gives a circle.',
      h_ellf: 'Filled ellipse: drag out a filled ellipse or circle.',
      h_fill: 'Fill: recolours the connected area of the same colour.',
      h_pick: 'Picker: a click takes over the colour of the pixel.',
      h_poly: 'Polygon: click points, double-click or click the first point to close, Esc cancels.',
      h_text: 'Text: straight - a click places the text centred on that spot; on an arc - a click sets radius and centre of the arc. Settings below.',
      h_stamp: 'Logo: load an image under "Logo" and set its width, then a click places it centred on that spot.',
      stamp: 'Logo', txtMode: 'Layout:', straight: 'straight', arcTop: 'arc at the top', arcBottom: 'arc at the bottom', txtRot: 'Rotation:',
      logo: 'Logo', logoW: 'Width:', logoHint: 'A PNG with a transparent background works best. The image stays loaded for further clicks.',
      noLogo: 'Load an image under "Logo" first.', fonts: 'Own font', fontAdd: 'Add', fontPh: 'installed font',
      fontFileHint: 'or load a font file (.ttf, .otf, .woff) - valid until the page is reloaded, also for the generator numerals.',
      fontAdded: 'Font "{0}" added and selected', fontMissing: 'The font "{0}" is not installed on this PC.',
      fontErr: 'Font file could not be read.',
      roundHint: 'Round display: the darkened area is not visible on the clock.',
      pos: 'Pixel', center: 'Centre' }
  };
  var L = (FD.lang === 'de') ? 'de' : 'en';
  function t(k, a, b) {
    var s = TX[L][k] || TX.en[k] || k;
    if (a !== undefined) s = s.replace('{0}', a);
    return b === undefined ? s : s.replace('{1}', b);
  }
  function $(id) { return document.getElementById(id); }

  // Zustand
  // State
  var pix = new Uint16Array(N).fill(0xFFFF), undoSt = [], redoSt = [];
  var tool = 'pen', color = 0x0000, dirty = false, base = DEF;
  // Ueberschreiben nur fuer Dateien, deren Name /upload auch annimmt
  // Overwriting only for files whose name /upload accepts as well
  function canOverwrite() { return base !== DEF && /^\/face_[A-Za-z0-9_.-]+\.bmp$/.test(base); }
  function setDirty(v) {
    dirty = v; $('saveBtn').disabled = !v;
    var ok = canOverwrite();
    $('saveCurBtn').disabled = !v || !ok;
    $('saveCurBtn').title = ok ? '' : t('builtinRO');
  }

  // Farben (RGB565) - im Zifferblatt ist jede Farbe erlaubt, auch Weiss
  // Colours (RGB565) - every colour is allowed in the clock face, white included
  function rgb565(r, g, b) { return (Math.round(r * 31 / 255) << 11) | (Math.round(g * 63 / 255) << 5) | Math.round(b * 31 / 255); }
  function to565(hex) { return rgb565(parseInt(hex.substr(1, 2), 16), parseInt(hex.substr(3, 2), 16), parseInt(hex.substr(5, 2), 16)); }
  function rgbOf(v) { return [((v >> 11) & 31) * 255 / 31 | 0, ((v >> 5) & 63) * 255 / 63 | 0, (v & 31) * 255 / 31 | 0]; }
  function hexOf(v) { var c = rgbOf(v); return '#' + c.map(function (x) { return ('0' + x.toString(16)).slice(-2); }).join(''); }
  function cssOf(v) { var c = rgbOf(v); return 'rgb(' + c[0] + ',' + c[1] + ',' + c[2] + ')'; }

  // Puffer <-> Canvas
  // Buffer <-> canvas
  function bufToImageData(buf, id) {
    var d = id.data;
    for (var i = 0; i < N; i++) {
      var v = buf[i], j = i * 4;
      d[j] = ((v >> 11) & 31) * 255 / 31; d[j + 1] = ((v >> 5) & 63) * 255 / 63; d[j + 2] = (v & 31) * 255 / 31; d[j + 3] = 255;
    }
    return id;
  }
  function newCanvas(w, h) { var c = document.createElement('canvas'); c.width = w; c.height = h; return c; }
  function bufCanvas(buf) {
    var c = newCanvas(W, H), cx = c.getContext('2d');
    cx.putImageData(bufToImageData(buf, cx.createImageData(W, H)), 0, 0);
    return c;
  }
  function canvasBuf(c) {
    var d = c.getContext('2d').getImageData(0, 0, W, H).data, out = new Uint16Array(N);
    for (var i = 0; i < N; i++) out[i] = rgb565(d[i * 4], d[i * 4 + 1], d[i * 4 + 2]);
    return out;
  }

  // Editor-Canvas
  // Editor canvas
  var ed = $('ed'), ectx = ed.getContext('2d');
  var off = newCanvas(W, H), octx = off.getContext('2d'), oimg = octx.createImageData(W, H);
  // Zoom: Vorgabe fuellt die Fensterhoehe, mindestens aber 2 px je Pixel,
  // mit -/+ oder Strg+Mausrad von 1 bis 16 einstellbar und im Browser gemerkt.

  // Zoom: the default fills the window height, but at least 2 px per pixel,
  // adjustable with -/+ or Ctrl+wheel from 1 to 16 and remembered in the browser.
  var ZMIN = 1, ZMAX = 16, Z = Math.max(2, Math.min(ZMAX, Math.floor((window.innerHeight - 140) / H)));
  try { var zs = +localStorage.getItem('uhr3FdZoom'); if (zs >= ZMIN && zs <= ZMAX) Z = zs; } catch (e) { }
  function applyZoom() {
    ed.width = W * Z; ed.height = H * Z;
    $('zoomInfo').textContent = 'Zoom ' + Z + 'x';
    $('zoomOut').disabled = Z <= ZMIN; $('zoomIn').disabled = Z >= ZMAX;
  }
  function setZoom(z) {
    Z = Math.max(ZMIN, Math.min(ZMAX, z));
    try { localStorage.setItem('uhr3FdZoom', Z); } catch (e) { }
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
  ed.addEventListener('wheel', function (ev) {
    if (!ev.ctrlKey) return;
    ev.preventDefault();
    setZoom(Z + (ev.deltaY < 0 ? 1 : -1));
  }, { passive: false });

  function drawEditor(buf) {
    octx.putImageData(bufToImageData(buf || pix, oimg), 0, 0);
    ectx.imageSmoothingEnabled = false;
    ectx.drawImage(off, 0, 0, W * Z, H * Z);
    if (Z >= 6) {
      ectx.strokeStyle = 'rgba(128,128,128,0.18)'; ectx.lineWidth = 1; ectx.beginPath();
      for (var gx = 1; gx < W; gx++) { ectx.moveTo(gx * Z + 0.5, 0); ectx.lineTo(gx * Z + 0.5, H * Z); }
      for (var gy = 1; gy < H; gy++) { ectx.moveTo(0, gy * Z + 0.5); ectx.lineTo(W * Z, gy * Z + 0.5); }
      ectx.stroke();
    }
    if (FD.round) {
      ectx.fillStyle = 'rgba(0,0,0,0.55)'; ectx.beginPath();
      ectx.rect(0, 0, W * Z, H * Z); ectx.arc(CX * Z, CY * Z, R * Z, 0, Math.PI * 2);
      ectx.fill('evenodd');
    }
    ectx.strokeStyle = 'rgba(245,166,35,0.45)'; ectx.beginPath();
    ectx.moveTo(CX * Z, 0); ectx.lineTo(CX * Z, H * Z);
    ectx.moveTo(0, CY * Z); ectx.lineTo(W * Z, CY * Z); ectx.stroke();
    if (polyPts.length) {
      ectx.strokeStyle = '#f5a623'; ectx.beginPath();
      polyPts.forEach(function (p, i) { if (i) ectx.lineTo(p.x * Z, p.y * Z); else ectx.moveTo(p.x * Z, p.y * Z); });
      ectx.stroke();
      polyPts.forEach(function (p) { ectx.fillStyle = '#f5a623'; ectx.fillRect(p.x * Z - 2, p.y * Z - 2, 4, 4); });
    }
  }

  // Formen werden erst in eine Maske gezeichnet, dann mit Symmetrie in den
  // Puffer uebertragen - Drehungen tasten rueckwaerts ab, dadurch ohne Luecken.

  // Shapes are drawn into a mask first, then copied into the buffer with
  // symmetry - rotations sample backwards, which leaves no gaps.
  function Mask() { this.m = new Uint8Array(N); this.x0 = W; this.y0 = H; this.x1 = -1; this.y1 = -1; }
  Mask.prototype.set = function (x, y) {
    if (x < 0 || y < 0 || x >= W || y >= H) return;
    this.m[y * W + x] = 1;
    if (x < this.x0) this.x0 = x; if (x > this.x1) this.x1 = x;
    if (y < this.y0) this.y0 = y; if (y > this.y1) this.y1 = y;
  };
  // Umrisspunkt in Stiftbreite
  // Outline point at pen width
  var penW = 1;
  $('bw').oninput = function () { penW = Math.max(1, Math.min(15, +this.value || 1)); };
  Mask.prototype.dot = function (x, y) {
    if (penW === 1) { this.set(x, y); return; }
    var r2 = penW * penW / 4 + 0.25, n = Math.ceil(penW / 2);
    for (var dy = -n; dy <= n; dy++)
      for (var dx = -n; dx <= n; dx++) if (dx * dx + dy * dy <= r2) this.set(x + dx, y + dy);
  };
  function symRot() { var s = $('symSel').value; return s.charAt(0) === 'r' ? +s.slice(1) : 1; }
  function paint(buf, mk, v) {
    if (mk.x1 < 0) return;
    var s = $('symSel').value, m = mk.m;
    for (var y = mk.y0; y <= mk.y1; y++) for (var x = mk.x0; x <= mk.x1; x++) {
      if (!m[y * W + x]) continue;
      buf[y * W + x] = v;
      if (s === 'mx' || s === 'mxy') buf[y * W + (W - 1 - x)] = v;
      if (s === 'mxy') { buf[(H - 1 - y) * W + x] = v; buf[(H - 1 - y) * W + (W - 1 - x)] = v; }
    }
    var n = symRot();
    for (var k = 1; k < n; k++) {
      var a = k * 2 * Math.PI / n, ca = Math.cos(a), sa = Math.sin(a);
      var xs = [], ys = [];
      [[mk.x0, mk.y0], [mk.x1 + 1, mk.y0], [mk.x0, mk.y1 + 1], [mk.x1 + 1, mk.y1 + 1]].forEach(function (p) {
        var u = p[0] - CX, w = p[1] - CY;
        xs.push(CX + u * ca - w * sa); ys.push(CY + u * sa + w * ca);
      });
      var dx0 = Math.max(0, Math.floor(Math.min.apply(null, xs))), dx1 = Math.min(W - 1, Math.ceil(Math.max.apply(null, xs)));
      var dy0 = Math.max(0, Math.floor(Math.min.apply(null, ys))), dy1 = Math.min(H - 1, Math.ceil(Math.max.apply(null, ys)));
      for (var dy = dy0; dy <= dy1; dy++) for (var dx = dx0; dx <= dx1; dx++) {
        var uu = dx + 0.5 - CX, ww = dy + 0.5 - CY;
        var sx = Math.floor(CX + uu * ca + ww * sa), sy = Math.floor(CY - uu * sa + ww * ca);
        if (sx >= mk.x0 && sx <= mk.x1 && sy >= mk.y0 && sy <= mk.y1 && m[sy * W + sx]) buf[dy * W + dx] = v;
      }
    }
  }
  // Startpunkte fuer Fuellen mit Symmetrie
  // Seed points for filling with symmetry
  function seeds(x, y) {
    var s = $('symSel').value, out = [[x, y]];
    if (s === 'mx' || s === 'mxy') out.push([W - 1 - x, y]);
    if (s === 'mxy') out.push([x, H - 1 - y], [W - 1 - x, H - 1 - y]);
    var n = symRot();
    for (var k = 1; k < n; k++) {
      var a = k * 2 * Math.PI / n, u = x + 0.5 - CX, w = y + 0.5 - CY;
      out.push([Math.floor(CX + u * Math.cos(a) - w * Math.sin(a)), Math.floor(CY + u * Math.sin(a) + w * Math.cos(a))]);
    }
    return out;
  }

  // Rasterisierung mit harten Kanten ueber Pixelmitten
  // Rasterising with hard edges via pixel centres
  function lineOn(mk, x0, y0, x1, y1) {
    var dx = Math.abs(x1 - x0), sx = x0 < x1 ? 1 : -1, dy = -Math.abs(y1 - y0), sy = y0 < y1 ? 1 : -1, e = dx + dy;
    for (;;) {
      mk.dot(x0, y0);
      if (x0 === x1 && y0 === y1) break;
      var e2 = 2 * e;
      if (e2 >= dy) { e += dy; x0 += sx; }
      if (e2 <= dx) { e += dx; y0 += sy; }
    }
  }
  function rectOn(mk, a, b, filled) {
    var x0 = Math.min(a.x, b.x), x1 = Math.max(a.x, b.x), y0 = Math.min(a.y, b.y), y1 = Math.max(a.y, b.y);
    for (var y = y0; y <= y1; y++) for (var x = x0; x <= x1; x++) {
      if (filled) mk.set(x, y);
      else if (x === x0 || x === x1 || y === y0 || y === y1) mk.dot(x, y);
    }
  }
  function ellOn(mk, a, b, filled) {
    var x0 = Math.min(a.x, b.x), x1 = Math.max(a.x, b.x), y0 = Math.min(a.y, b.y), y1 = Math.max(a.y, b.y);
    var cx = (x0 + x1 + 1) / 2, cy = (y0 + y1 + 1) / 2, rx = (x1 - x0 + 1) / 2, ry = (y1 - y0 + 1) / 2;
    function inside(x, y) { var u = (x + 0.5 - cx) / rx, w = (y + 0.5 - cy) / ry; return u * u + w * w <= 1; }
    for (var y = y0; y <= y1; y++) for (var x = x0; x <= x1; x++) {
      if (!inside(x, y)) continue;
      if (filled) mk.set(x, y);
      else if (!inside(x - 1, y) || !inside(x + 1, y) || !inside(x, y - 1) || !inside(x, y + 1)) mk.dot(x, y);
    }
  }
  function polyOn(mk, pts) {
    var minY = H, maxY = 0;
    pts.forEach(function (p) { minY = Math.min(minY, p.y); maxY = Math.max(maxY, p.y); });
    for (var y = Math.max(0, Math.floor(minY)); y <= Math.min(H - 1, Math.ceil(maxY)); y++) {
      for (var x = 0; x < W; x++) {
        var px = x + 0.5, py = y + 0.5, inside = false;
        for (var i = 0, j = pts.length - 1; i < pts.length; j = i++) {
          var a = pts[i], b = pts[j];
          if ((a.y > py) !== (b.y > py) && px < (b.x - a.x) * (py - a.y) / (b.y - a.y) + a.x) inside = !inside;
        }
        if (inside) mk.set(x, y);
      }
    }
  }
  function floodOn(buf, x, y, v) {
    if (x < 0 || y < 0 || x >= W || y >= H) return;
    var target = buf[y * W + x];
    if (target === v) return;
    var stack = [x, y];
    while (stack.length) {
      var py = stack.pop(), px = stack.pop();
      if (px < 0 || py < 0 || px >= W || py >= H || buf[py * W + px] !== target) continue;
      buf[py * W + px] = v;
      stack.push(px + 1, py, px - 1, py, px, py + 1, px, py - 1);
    }
  }
  // Text mit Kantenglaettung, mit dem Untergrund gemischt
  // Anti-aliased text, blended with the underlying pixels
  // Im Bogen: Radius und Bogenmitte kommen aus dem Klickpunkt, oben laeuft der Text
  // im Uhrzeigersinn, unten gegen ihn - so steht er an beiden Stellen aufrecht.

  // On an arc: radius and arc centre come from the click point, at the top the text
  // runs clockwise, at the bottom anticlockwise - so it stands upright in both places.
  function textOn(buf, x, y) {
    var txt = $('txtVal').value;
    if (!txt) return;
    var c = newCanvas(W, H), cx = c.getContext('2d'), mode = $('txtMode').value;
    cx.font = ($('txtBold').checked ? 'bold ' : '') + Math.max(4, +$('txtSize').value || 12) + 'px ' + $('txtFont').value;
    cx.textAlign = 'center'; cx.textBaseline = 'middle'; cx.fillStyle = '#000';
    if (mode === 'straight') {
      cx.translate(x + 0.5, y + 0.5);
      cx.rotate((+$('txtRot').value || 0) * Math.PI / 180);
      cx.fillText(txt, 0, 0);
    }
    else {
      var dx = x + 0.5 - CX, dy = y + 0.5 - CY, r = Math.max(1, Math.sqrt(dx * dx + dy * dy)), mid = Math.atan2(dx, -dy);
      var chars = Array.from(txt), widths = chars.map(function (ch) { return cx.measureText(ch).width; });
      var total = widths.reduce(function (a, b) { return a + b; }, 0), dir = mode === 'arcTop' ? 1 : -1, acc = 0;
      chars.forEach(function (ch, i) {
        var a = mid + dir * (acc + widths[i] / 2 - total / 2) / r;
        acc += widths[i];
        cx.save();
        cx.translate(CX + Math.sin(a) * r, CY - Math.cos(a) * r);
        cx.rotate(dir > 0 ? a : a - Math.PI);
        cx.fillText(ch, 0, 0);
        cx.restore();
      });
    }
    blendAlpha(buf, cx.getImageData(0, 0, W, H).data, color);
  }

  // Logo-Stempel: Bild in der eingestellten Breite mittig auf den Klickpunkt, Seitenverhaeltnis bleibt
  // Logo stamp: image at the set width centred on the click point, aspect ratio is kept
  var logoImg = null;
  function stampOn(buf, x, y) {
    var iw = logoImg.naturalWidth || logoImg.width, ih = logoImg.naturalHeight || logoImg.height;
    var lw = Math.max(2, +$('logoW').value || 2), lh = lw * ih / iw;
    var c = bufCanvas(buf), cx = c.getContext('2d');
    cx.imageSmoothingQuality = 'high';
    cx.drawImage(logoImg, x + 0.5 - lw / 2, y + 0.5 - lh / 2, lw, lh);
    return canvasBuf(c);
  }
  function blendAlpha(buf, d, v) {
    var col = rgbOf(v);
    for (var i = 0; i < N; i++) {
      var a = d[i * 4 + 3] / 255;
      if (a < 0.06) continue;
      if (a > 0.94) { buf[i] = v; continue; }
      var u = rgbOf(buf[i]);
      buf[i] = rgb565(u[0] + (col[0] - u[0]) * a, u[1] + (col[1] - u[1]) * a, u[2] + (col[2] - u[2]) * a);
    }
  }

  // Rueckgaengig
  // Undo
  function pushUndo() {
    undoSt.push(pix.slice());
    if (undoSt.length > 40) undoSt.shift();
    redoSt = [];
  }
  function commit(buf) { pushUndo(); pix = buf; setDirty(true); drawEditor(); schedulePreview(); }

  // Maus-/Stifteingabe
  // Pointer input
  var drag = null, polyPts = [];
  function cell(ev) {
    var r = ed.getBoundingClientRect();
    return { x: Math.floor((ev.clientX - r.left) * W / r.width), y: Math.floor((ev.clientY - r.top) * H / r.height) };
  }
  function inGrid(c) { return c.x >= 0 && c.y >= 0 && c.x < W && c.y < H; }

  ed.addEventListener('pointerdown', function (ev) {
    if (ev.button === 2) return;
    var c = cell(ev);
    if (!inGrid(c)) return;
    ed.setPointerCapture(ev.pointerId);
    if (tool === 'pick') { setColor(pix[c.y * W + c.x]); pickDone(); return; }
    if (tool === 'fill') {
      var fb = pix.slice();
      seeds(c.x, c.y).forEach(function (p) { floodOn(fb, p[0], p[1], color); });
      commit(fb);
      return;
    }
    if (tool === 'text') {
      var tb = pix.slice();
      textOn(tb, c.x, c.y);
      commit(tb);
      return;
    }
    if (tool === 'stamp') {
      if (logoImg) commit(stampOn(pix, c.x, c.y));
      else showMsg(t('noLogo'), false);
      return;
    }
    if (tool === 'poly') {
      var pt = { x: c.x + 0.5, y: c.y + 0.5 };
      if (polyPts.length >= 3 && Math.abs(pt.x - polyPts[0].x) < 1 && Math.abs(pt.y - polyPts[0].y) < 1) { closePoly(); return; }
      polyPts.push(pt);
      drawEditor();
      return;
    }
    if (tool === 'pen') {
      pushUndo();
      var mk = new Mask(); mk.dot(c.x, c.y); paint(pix, mk, color);
      setDirty(true);
      drag = { start: c, last: c };
      drawEditor();
      return;
    }
    drag = { start: c, last: c };
  });
  ed.addEventListener('pointermove', function (ev) {
    var c = cell(ev);
    $('posInfo').textContent = inGrid(c) ? t('pos') + ' ' + c.x + ' / ' + c.y + '   (' + t('center') + ' ' + CX + ' / ' + CY + ')' : '';
    if (!drag) {
      if (tool === 'text' && inGrid(c)) { var pb = pix.slice(); textOn(pb, c.x, c.y); drawEditor(pb); }
      if (tool === 'stamp' && logoImg && inGrid(c)) drawEditor(stampOn(pix, c.x, c.y));
      return;
    }
    c.x = Math.max(0, Math.min(W - 1, c.x)); c.y = Math.max(0, Math.min(H - 1, c.y));
    if (tool === 'pen') {
      var mk = new Mask(); lineOn(mk, drag.last.x, drag.last.y, c.x, c.y); paint(pix, mk, color);
      drag.last = c;
      drawEditor();
      return;
    }
    drag.last = c;
    drawEditor(shapeBuf(drag.start, c));
  });
  ed.addEventListener('pointerup', function () {
    if (!drag) return;
    if (tool === 'pen') { drag = null; schedulePreview(); return; }
    var b = shapeBuf(drag.start, drag.last);
    drag = null;
    commit(b);
  });
  ed.addEventListener('pointerleave', function () { if ((tool === 'text' || tool === 'stamp') && !drag) drawEditor(); });
  ed.addEventListener('dblclick', function () { if (tool === 'poly' && polyPts.length >= 3) closePoly(); });
  document.addEventListener('keydown', function (ev) {
    if (tool !== 'poly') return;
    if (ev.key === 'Escape') { polyPts = []; drawEditor(); }
    if (ev.key === 'Enter' && polyPts.length >= 3) closePoly();
  });

  function shapeBuf(a, b) {
    var buf = pix.slice(), mk = new Mask();
    if (tool === 'line') lineOn(mk, a.x, a.y, b.x, b.y);
    else if (tool === 'rect') rectOn(mk, a, b, false);
    else if (tool === 'rectf') rectOn(mk, a, b, true);
    else if (tool === 'ell') ellOn(mk, a, b, false);
    else if (tool === 'ellf') ellOn(mk, a, b, true);
    paint(buf, mk, color);
    return buf;
  }
  function closePoly() {
    var buf = pix.slice(), mk = new Mask();
    polyOn(mk, polyPts);
    paint(buf, mk, color);
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
  function options(sel, keys) {
    keys.forEach(function (k) { var o = document.createElement('option'); o.value = k[0]; o.textContent = t(k[1]); sel.appendChild(o); });
  }
  var toolBtn = {};
  ['pen', 'line', 'rect', 'rectf', 'ell', 'ellf', 'poly', 'fill', 'text', 'stamp', 'pick'].forEach(function (k) {
    toolBtn[k] = button($('toolBtns'), t(k), function () { pickReturn = null; selectTool(k); });
    toolBtn[k].title = t('h_' + k);
  });
  options($('symSel'), ['none', 'mx', 'mxy', 'r4', 'r12', 'r60'].map(function (k) { return [k, k]; }));
  options($('fitSel'), [['cover', 'cover'], ['contain', 'contain']]);
  options($('txtMode'), [['straight', 'straight'], ['arcTop', 'arcTop'], ['arcBottom', 'arcBottom']]);
  $('txtMode').onchange = function () { $('rotRow').style.display = this.value === 'straight' ? '' : 'none'; };
  function selectTool(k) {
    tool = k; polyPts = [];
    Object.keys(toolBtn).forEach(function (q) { toolBtn[q].className = 'tb' + (q === k ? ' on' : ''); });
    $('pickBtn').className = 'tb' + (k === 'pick' ? ' on' : '');
    $('textOpts').style.display = k === 'text' ? '' : 'none';
    $('toolHint').textContent = t('h_' + k);
    drawEditor();
  }
  // src = das Eingabefeld, aus dem die Farbe kommt - dessen Wert bleibt beim Tippen/Ziehen unangetastet
  // src = the input the colour comes from - its value stays untouched while typing/dragging
  function setColor(v, src) {
    color = v;
    if (src !== 'picker') $('color').value = hexOf(color);
    if (src !== 'hex') $('colorHex').value = hexOf(color);
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
  [0x0000, 0x4208, 0x8410, 0xF800, 0xB000, 0xFD20, 0xC618, 0xFFFF, 0x001F, 0x07E0].forEach(function (v) { addCell($('swatches'), 'sw', v); });

  // Palette: Graustufen, 16 Farbtoene in 7 Helligkeiten, 2 Reihen gedeckte Toene - alles
  // bereits auf RGB565 gerundet, damit die Palette genau die Farben der Uhr zeigt.

  // Palette: greyscale, 16 hues in 7 lightness levels, 2 rows of muted tones - all already
  // rounded to RGB565, so the palette shows exactly the colours of the clock.
  function hsl565(h, sat, l) {
    var a = sat * Math.min(l, 1 - l);
    function f(n) { var k = (n + h / 30) % 12; return Math.round(255 * (l - a * Math.max(-1, Math.min(k - 3, 9 - k, 1)))); }
    return rgb565(f(0), f(8), f(4));
  }
  var palRows = [[0, 0]].concat([.2, .3, .4, .5, .6, .7, .8].map(function (l) { return [1, l]; }), [[.4, .35], [.4, .65]]);
  palRows.forEach(function (r, ri) {
    for (var i = 0; i < 16; i++) addCell($('palette'), '', ri === 0 ? hsl565(0, 0, i / 15) : hsl565(i * 22.5, r[0], r[1]));
  });

  $('undoBtn').onclick = function () {
    if (!undoSt.length) return;
    redoSt.push(pix); pix = undoSt.pop(); setDirty(true); drawEditor(); schedulePreview();
  };
  $('redoBtn').onclick = function () {
    if (!redoSt.length) return;
    undoSt.push(pix); pix = redoSt.pop(); setDirty(true); drawEditor(); schedulePreview();
  };
  $('clearBtn').onclick = function () {
    if (confirm(t('confirmClear'))) commit(new Uint16Array(N).fill(color));
  };

  // Bild laden: 16-Bit-BMPs selbst dekodieren (nicht jeder Browser zeigt sie an),
  // alles andere ueber <img>; Transparenz liegt ueber dem bisherigen Zifferblatt.

  // Load image: decode 16-bit BMPs ourselves (not every browser displays them),
  // everything else via <img>; transparency lies over the current clock face.
  function imageSource(file) {
    if (/\.bmp$/i.test(file.name)) {
      return file.arrayBuffer().then(function (ab) {
        var img = parseBmp(ab), c = newCanvas(img.w, img.h), cx = c.getContext('2d'), id = cx.createImageData(img.w, img.h);
        for (var y = 0; y < img.h; y++) for (var x = 0; x < img.w; x++) {
          var rgb = rgbOf(img.px(x, y)), i = (y * img.w + x) * 4;
          id.data[i] = rgb[0]; id.data[i + 1] = rgb[1]; id.data[i + 2] = rgb[2]; id.data[i + 3] = 255;
        }
        cx.putImageData(id, 0, 0);
        return c;
      });
    }
    return new Promise(function (ok, fail) {
      var url = URL.createObjectURL(file), img = new Image();
      img.onload = function () { URL.revokeObjectURL(url); ok(img); };
      img.onerror = function () { URL.revokeObjectURL(url); fail(new Error('img')); };
      img.src = url;
    });
  }
  $('logoFile').onchange = function () {
    var f = this.files[0], input = this;
    if (!f) return;
    imageSource(f).then(function (img) {
      logoImg = img;
      selectTool('stamp');
    }).catch(function () { showMsg(t('imgErr'), false); }).then(function () { input.value = ''; });
  };

  // Eigene Schriften: installierte per Name (Test ueber abweichende Textbreite gegen
  // die Ersatzschriften) oder als Datei ueber FontFace - beide in Text und Generator.

  // Own fonts: installed ones by name (checked via a text width differing from the
  // fallback fonts) or as a file via FontFace - both in text and generator.
  var fontCount = 0;
  function addFont(family, label) {
    ['txtFont', 'gNFont'].forEach(function (id) {
      var o = document.createElement('option'); o.value = family; o.textContent = label; $(id).appendChild(o);
    });
    $('txtFont').value = family;
    showMsg(t('fontAdded', label), true);
  }
  function fontInstalled(name) {
    var cx = newCanvas(10, 10).getContext('2d'), probe = 'mmmmmmmmmmlli10OQW';
    return ['monospace', 'serif', 'sans-serif'].some(function (f) {
      cx.font = '72px ' + f; var ref = cx.measureText(probe).width;
      cx.font = '72px "' + name + '", ' + f; return cx.measureText(probe).width !== ref;
    });
  }
  $('fontAddBtn').onclick = function () {
    var n = $('fontName').value.trim().replace(/["\\]/g, '');
    if (!n) return;
    if (!fontInstalled(n)) { showMsg(t('fontMissing', n), false); return; }
    addFont('"' + n + '"', n);
  };
  $('fontFile').onchange = function () {
    var f = this.files[0], input = this, name = 'UserFont' + (++fontCount);
    if (!f) return;
    f.arrayBuffer().then(function (ab) { return new FontFace(name, ab).load(); }).then(function (face) {
      document.fonts.add(face);
      addFont('"' + name + '"', f.name.replace(/\.[^.]+$/, ''));
    }).catch(function () { showMsg(t('fontErr'), false); }).then(function () { input.value = ''; });
  };

  $('imgFile').onchange = function () {
    var f = this.files[0], input = this;
    if (!f) return;
    imageSource(f).then(function (img) {
      var iw = img.naturalWidth || img.width, ih = img.naturalHeight || img.height;
      var s = $('fitSel').value === 'cover' ? Math.max(W / iw, H / ih) : Math.min(W / iw, H / ih);
      var c = bufCanvas(pix), cx = c.getContext('2d'), dw = iw * s, dh = ih * s;
      cx.imageSmoothingQuality = 'high';
      cx.drawImage(img, (W - dw) / 2, (H - dh) / 2, dw, dh);
      commit(canvasBuf(c));
    }).catch(function () { showMsg(t('imgErr'), false); }).then(function () { input.value = ''; });
  };

  // Zifferblatt-Generator: mit Kantenglaettung gezeichnet, danach auf RGB565 gerundet
  // Clock face generator: drawn anti-aliased, then rounded to RGB565
  var genDef = {
    gBg: '#ffffff', gRing: Math.round(W * 0.02), gRingC: '#000000',
    gHLen: Math.round(W * 0.08), gHW: Math.round(W * 0.025), gHC: '#000000',
    gMLen: Math.round(W * 0.03), gMW: Math.max(1, Math.round(W * 0.008)), gMC: '#000000',
    gNum: 'arabic', gNSize: Math.round(W * 0.09), gNC: '#000000', gNDist: Math.round(W * 0.2), gNFont: 'sans-serif'
  };
  options($('gNum'), [['none', 'numNone'], ['arabic', 'numArabic'], ['quarter', 'numQuarter'], ['roman', 'numRoman']]);
  Object.keys(genDef).forEach(function (id) { $(id).value = genDef[id]; });
  function num(id) { return Math.max(0, +$(id).value || 0); }
  $('genBtn').onclick = function () {
    if (!confirm(t('confirmGen'))) return;
    var c = newCanvas(W, H), x = c.getContext('2d');
    x.fillStyle = $('gBg').value; x.fillRect(0, 0, W, H);
    var ring = num('gRing');
    if (ring > 0) {
      x.strokeStyle = $('gRingC').value; x.lineWidth = ring;
      x.beginPath(); x.arc(CX, CY, R - ring / 2, 0, Math.PI * 2); x.stroke();
    }
    var outer = R - ring - Math.round(R * 0.03);
    function marks(count, skip, len, width, col) {
      if (len <= 0 || width <= 0) return;
      x.strokeStyle = col; x.lineWidth = width; x.lineCap = 'butt'; x.beginPath();
      for (var i = 0; i < count; i++) {
        if (skip && i % skip === 0) continue;
        var a = i * 2 * Math.PI / count, s = Math.sin(a), co = -Math.cos(a);
        x.moveTo(CX + s * outer, CY + co * outer); x.lineTo(CX + s * (outer - len), CY + co * (outer - len));
      }
      x.stroke();
    }
    marks(60, 5, num('gMLen'), num('gMW'), $('gMC').value);
    marks(12, 0, num('gHLen'), num('gHW'), $('gHC').value);
    var mode = $('gNum').value;
    if (mode !== 'none') {
      var roman = ['XII', 'I', 'II', 'III', 'IV', 'V', 'VI', 'VII', 'VIII', 'IX', 'X', 'XI'];
      var rn = R - num('gNDist');
      x.fillStyle = $('gNC').value; x.textAlign = 'center'; x.textBaseline = 'middle';
      x.font = ($('gNBold').checked ? 'bold ' : '') + Math.max(4, num('gNSize')) + 'px ' + $('gNFont').value;
      for (var h = 0; h < 12; h++) {
        if (mode === 'quarter' && h % 3) continue;
        var a = h * Math.PI / 6;
        x.fillText(mode === 'roman' ? roman[h] : String(h || 12), CX + Math.sin(a) * rn, CY - Math.cos(a) * rn);
      }
    }
    commit(canvasBuf(c));
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
  function decodeFace(ab) {
    var img = parseBmp(ab), out = new Uint16Array(N);
    for (var ty = 0; ty < H; ty++) for (var tx = 0; tx < W; tx++)
      out[ty * W + tx] = img.px(Math.floor(tx * img.w / W), Math.floor(ty * img.h / H));
    return out;
  }
  function encodeBmp(buf) {
    var rs = Math.floor((W * 2 + 3) / 4) * 4, hs = 66, size = hs + rs * H;
    var ab = new ArrayBuffer(size), dv = new DataView(ab);
    dv.setUint8(0, 66); dv.setUint8(1, 77); dv.setUint32(2, size, true); dv.setUint32(10, hs, true);
    dv.setUint32(14, 40, true); dv.setInt32(18, W, true); dv.setInt32(22, -H, true); dv.setUint16(26, 1, true);
    dv.setUint16(28, 16, true); dv.setUint32(30, 3, true); dv.setUint32(34, rs * H, true);
    dv.setUint32(54, 0xF800, true); dv.setUint32(58, 0x07E0, true); dv.setUint32(62, 0x001F, true);
    for (var y = 0; y < H; y++) for (var x = 0; x < W; x++) dv.setUint16(hs + y * rs + x * 2, buf[y * W + x], true);
    return new Blob([ab], { type: 'image/bmp' });
  }

  // Laden und Speichern
  // Load and save
  function showMsg(text, ok) { $('msg').className = ok === undefined ? '' : (ok ? 'ok' : 'err'); $('msg').textContent = text; }
  function faceLabel(path) { return path.replace(/^\/?face_/, '').replace(/\.bmp$/, ''); }
  function showBase() {
    $('baseInfo').textContent = t('base') + ' ' + (base === DEF ? t('builtin') : faceLabel(base)) + ' (' + t('active') + ')';
  }
  function nextName() {
    var max = 0;
    FD.faces.forEach(function (f) { var m = /^face_design(\d+)\.bmp$/.exec(f); if (m) max = Math.max(max, +m[1]); });
    return 'design' + (max + 1);
  }
  function fetchFace(url) {
    return fetch(url, { cache: 'no-store' }).then(function (r) {
      if (!r.ok) throw new Error(r.status);
      return r.arrayBuffer();
    }).then(decodeFace);
  }
  function loadActive() {
    var isDef = !FD.active || FD.active === DEF, note = '';
    showMsg(t('loading'));
    fetchFace(isDef ? '/api/defaultface' : '/file?name=' + encodeURIComponent(FD.active)).then(function (b) {
      base = isDef ? DEF : FD.active;
      return b;
    }, function () {
      note = t('missing'); base = DEF;
      return fetchFace('/api/defaultface');
    }).then(function (b) { pix = b; }, function () { pix = new Uint16Array(N).fill(0xFFFF); base = DEF; }).then(function () {
      undoSt = []; redoSt = [];
      setDirty(false);
      showBase(); drawEditor(); schedulePreview();
      showMsg(note || t('loaded'), !note);
    });
  }
  $('resetBtn').onclick = function () {
    if (dirty && !confirm(t('confirmReset'))) return;
    loadActive();
  };
  // Hochladen ueber /upload (wird dort RLE-komprimiert, bei runden Displays maskiert),
  // Weiterleitung NICHT folgen; optional aktivieren - /setbackground laedt neu.

  // Upload via /upload (RLE-compressed there, masked on round displays),
  // do NOT follow the redirect; optionally activate - /setbackground reloads.
  function saveFace(file, activate, isNew) {
    $('saveBtn').disabled = true; $('saveCurBtn').disabled = true;
    showMsg(t('saving'));
    var fd = new FormData();
    fd.append('upload', encodeBmp(pix), file);
    fetch('/upload', { method: 'POST', body: fd, redirect: 'manual' }).then(function (r) {
      if (!(r.ok || r.type === 'opaqueredirect')) throw new Error('HTTP ' + r.status);
      if (!activate) return;
      return fetch('/setbackground?file=' + encodeURIComponent(file), { redirect: 'manual' }).then(function () {
        FD.active = '/' + file; base = FD.active;
      });
    }).then(function () {
      if (FD.faces.indexOf(file) < 0) FD.faces.push(file);
      setDirty(false);
      showBase();
      $('faceName').value = nextName();
      showMsg(isNew ? t('saved', faceLabel(file)) + (activate ? t('activated') : '') : t('savedCur', faceLabel(file)), true);
    }).catch(function (e) {
      showMsg(t('failed') + e.message, false);
      setDirty(true);
    });
  }
  $('saveBtn').onclick = function () {
    var name = $('faceName').value.trim();
    if (!/^[A-Za-z0-9_-]{1,20}$/.test(name)) { showMsg(t('badName'), false); return; }
    var file = 'face_' + name + '.bmp';
    if (FD.faces.indexOf(file) >= 0 && !confirm(t('confirmExists', name))) return;
    saveFace(file, $('activate').checked, true);
  };
  $('saveCurBtn').onclick = function () {
    if (canOverwrite()) saveFace(base.substring(1), true, false);
  };
  window.addEventListener('beforeunload', function (ev) { if (dirty) { ev.preventDefault(); ev.returnValue = ''; } });

  // Vorschau mit den Zeigern des aktiven Satzes
  // Preview with the hands of the active set
  var HW = FD.hand.w, HH = FD.hand.h, HPY = FD.hand.py, PARTS = ['hour', 'minute', 'second'];
  var pv = $('pv'), pctx = pv.getContext('2d'), S = W;
  pv.width = S; pv.height = S;
  var hands = {}, previewTimer = null;
  $('live').onchange = schedulePreview;
  $('showHands').onchange = schedulePreview;

  // Zeiger-BMP: Weiss und 0x0120 sind transparent (wie loadHandSprites() im Geraet)
  // Hand BMP: white and 0x0120 are transparent (like loadHandSprites() on the device)
  // Alte Zeiger (FD.hand.lh hoch) unten buendig, wie loadHandPixels() im Geraet
  // Old hands (FD.hand.lh high) flush at the bottom, like loadHandPixels() on the device
  function decodeHand(ab) {
    var img = parseBmp(ab), out = new Int32Array(HW * HH).fill(-1);
    var off = (img.w === HW && img.h === FD.hand.lh && FD.hand.lh < HH) ? HH - FD.hand.lh : 0, sh = HH - off;
    for (var y = 0; y < sh; y++) for (var x = 0; x < HW; x++) {
      var v = img.px(Math.floor(x * img.w / HW), Math.floor(y * img.h / sh));
      out[(y + off) * HW + x] = (v === 0xFFFF || v === 0x0120) ? -1 : v;
    }
    return out;
  }
  PARTS.forEach(function (p) {
    var def = '/api/defaulthand?part=' + p, set = FD.hand.set;
    var url = (!set || set === 'default') ? def : '/file?name=' + encodeURIComponent('/hand_set' + set + '_' + p + '.bmp');
    function get(u) { return fetch(u, { cache: 'no-store' }).then(function (r) { if (!r.ok) throw new Error(r.status); return r.arrayBuffer(); }); }
    get(url).catch(function () { return get(def); }).then(function (ab) {
      hands[p] = handCanvas(decodeHand(ab), p);
      schedulePreview();
    }).catch(function () {});
  });
  // Sprite in Zeigerbreite des Zifferblatts, Bitmap mittig zugeschnitten
  // Sprite at the face's hand width, bitmap cropped in the centre
  function spriteWidth(p) { var w = FD.hand.widths && FD.hand.widths[p]; return w > 0 ? w : HW; }
  function handCanvas(buf, p) {
    var sw = spriteWidth(p), offx = sw < HW ? (HW - sw) >> 1 : 0;
    var c = newCanvas(sw, HH), cx = c.getContext('2d'), id = cx.createImageData(sw, HH);
    for (var y = 0; y < HH; y++) for (var x = 0; x < sw; x++) {
      var v = buf[y * HW + x + offx];
      if (v < 0) continue;
      var rgb = rgbOf(v), i = (y * sw + x) * 4;
      id.data[i] = rgb[0]; id.data[i + 1] = rgb[1]; id.data[i + 2] = rgb[2]; id.data[i + 3] = 255;
    }
    cx.putImageData(id, 0, 0);
    return c;
  }
  function renderPreview() {
    pctx.save();
    pctx.clearRect(0, 0, S, S);
    if (FD.round) { pctx.beginPath(); pctx.arc(S / 2, S / 2, S / 2, 0, Math.PI * 2); pctx.clip(); }
    pctx.drawImage(bufCanvas(pix), 0, 0, S, S);
    if ($('showHands').checked) {
      var d = $('live').checked ? new Date() : new Date(2000, 0, 1, 10, 8, 37);
      var hh = d.getHours() % 12, mm = d.getMinutes(), ss = d.getSeconds();
      var ang = { hour: (hh + mm / 60 + ss / 3600) * 30, minute: (mm + ss / 60) * 6, second: ss * 6 };
      PARTS.forEach(function (p) {
        if (!hands[p] || (p === 'second' && !FD.showSec)) return;
        pctx.save();
        pctx.translate(S / 2, S / 2);
        pctx.rotate(ang[p] * Math.PI / 180);
        pctx.drawImage(hands[p], -((spriteWidth(p) >> 1) + 0.5), -(HPY + 0.5));
        pctx.restore();
      });
      if (FD.hub > 0) {
        pctx.fillStyle = FD.hubColor;
        pctx.beginPath(); pctx.arc(S / 2, S / 2, FD.hub, 0, Math.PI * 2); pctx.fill();
      }
    }
    pctx.restore();
  }
  function schedulePreview() {
    if (previewTimer) clearTimeout(previewTimer);
    previewTimer = setTimeout(renderPreview, 60);
  }
  setInterval(function () { if ($('live').checked && $('showHands').checked && !document.hidden) renderPreview(); }, 1000);

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
    if (inGrid(c)) setColor(pix[c.y * W + c.x]);
  });
  pv.style.cursor = 'crosshair';
  pv.addEventListener('click', function (ev) {
    var r = pv.getBoundingClientRect();
    var d = pctx.getImageData(Math.floor((ev.clientX - r.left) * pv.width / r.width), Math.floor((ev.clientY - r.top) * pv.height / r.height), 1, 1).data;
    setColor(rgb565(d[0], d[1], d[2]));
    pickDone();
  });

  // Beschriftungen
  // Labels
  var labels = { tActivate: 'activate', tName: 'name', tTools: 'tools', tBw: 'bw', tSym: 'sym', tTxtSize: 'txtSize',
    tColor: 'color', tStd: 'std', tPal: 'pal', tEdit: 'edit', tImport: 'import', tGen: 'gen', tBg: 'bg', tRing: 'ring',
    tHourM: 'hourM', tMinM: 'minM', tNum: 'num', tNSize: 'nSize', tNDist: 'nDist', tNFont: 'nFont', genHint: 'genHint',
    tPreview: 'preview', tLive: 'live', tShowHands: 'showHands', tTxtMode: 'txtMode', tTxtRot: 'txtRot',
    tLogo: 'logo', tLogoW: 'logoW', logoHint: 'logoHint', tFonts: 'fonts', fontFileHint: 'fontFileHint' };
  Object.keys(labels).forEach(function (id) { $(id).textContent = t(labels[id]); });
  $('resetBtn').textContent = t('reset'); $('saveBtn').textContent = t('saveBtn'); $('saveCurBtn').textContent = t('saveCurBtn');
  $('pickBtn').textContent = t('pickBtn'); $('pickBtn').title = t('pickTip');
  $('color').title = t('free'); $('colorHex').placeholder = '#RRGGBB'; $('colorHex').title = t('hexHint');
  $('undoBtn').textContent = t('undo'); $('redoBtn').textContent = t('redo'); $('clearBtn').textContent = t('clear');
  $('genBtn').textContent = t('genBtn');
  $('txtSize').value = Math.round(W * 0.09);
  $('logoW').value = Math.round(W * 0.3);
  $('fontAddBtn').textContent = t('fontAdd'); $('fontName').placeholder = t('fontPh');
  $('faceName').value = nextName();
  var need = Math.ceil((N * 2 + 66) / 1024);
  $('spaceHint').textContent = (FD.free >= 0 && FD.free / 1024 < need) ? t('lowSpace', Math.floor(FD.free / 1024), need) : (FD.round ? t('roundHint') : '');

  // Start
  // Start
  selectTool('pen');
  setColor(color);
  loadActive();
})();
</script>
)FDRAW";
