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
      <label><span id="tName"></span><input type="text" id="faceName" maxlength="20" spellcheck="false" style="width:130px"></label>
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
      <small id="modeInfo"></small>
      <div id="stripCard" style="display:none">
        <h3 id="tStrip"></h3>
        <div class="grid2">
          <span id="tSPos"></span><span><select id="sBefore"></select></span>
          <span id="tSBg"></span><span><input type="color" class="clr" id="sBg"></span>
          <span id="tSFg"></span><span><input type="color" class="clr" id="sFg"></span>
          <span id="tSFont"></span><span><select id="sFont"></select></span>
          <span class="vlwRow" id="tSVSize"></span><span class="vlwRow"><input type="number" class="num2" id="sVT" min="8" max="120"> / <input type="number" class="num2" id="sVD" min="8" max="120"> px <label><input type="checkbox" id="sVB">B</label></span>
          <span id="tSTFmt"></span><span><select id="sTFmt"></select></span>
          <span id="tSSec"></span><span><select id="sSec"></select></span>
          <span id="tSDFmt"></span><span><select id="sDFmt"></select></span>
          <span></span><span><label><input type="checkbox" id="sBlink"><span id="tSBlink"></span></label></span>
          <span id="tSTime" style="align-self:start;margin-top:4px"></span><span><label><input type="checkbox" id="sTAuto"><span class="tSAuto"></span></label><br>X <input type="range" id="sTX" min="0" style="width:150px;margin:2px 0;vertical-align:middle"><br>Y <input type="range" id="sTY" min="0" style="width:150px;margin:2px 0;vertical-align:middle"></span>
          <span id="tSDate" style="align-self:start;margin-top:4px"></span><span><label><input type="checkbox" id="sDAuto"><span class="tSAuto"></span></label><br>X <input type="range" id="sDX" min="0" style="width:150px;margin:2px 0;vertical-align:middle"><br>Y <input type="range" id="sDY" min="0" style="width:150px;margin:2px 0;vertical-align:middle"></span>
        </div>
        <small id="stripHint"></small>
        <button type="button" class="tb" id="sDefBtn"></button>
        <button type="button" id="sSaveBtn"></button>
      </div>
    </div>
  </div>
</div>

<script>
(function () {
  // Mit Streifen (ILI9341) ist die Zeichenflaeche das ganze Display: Zifferblatt (FH hoch) plus Streifen (SH),
  // der Streifen oben (OY = SH) oder unten (OY = 0). CX/CY ist die Uhrmitte, nicht die Bildmitte.
  // With a strip (ILI9341) the drawing area is the whole display: clock face (FH high) plus strip (SH), the
  // strip on top (OY = SH) or at the bottom (OY = 0). CX/CY is the clock centre, not the image centre.
  var FH = FD.h, SH = FD.strip ? FD.strip.h : 0, OY = (FD.strip && FD.strip.before) ? SH : 0;
  var W = FD.w, H = FH + SH, N = W * H, CX = W / 2, CY = OY + FH / 2, R = Math.min(W, FH) / 2;
  var DEF = '/face_default.bmp';

  var TX = {
    de: { base: 'Basis:', builtin: 'Standard (eingebaut)', active: 'aktiv', reset: '\u00c4nderungen verwerfen',
      activate: 'neues Zifferblatt aktivieren', saveBtn: 'Als neues Zifferblatt speichern', name: 'Name:',
      saveCurBtn: 'Aktuelles Zifferblatt speichern und anwenden', tools: 'Werkzeug', bw: 'Stiftbreite:',
      pen: 'Stift', line: 'Linie', rect: 'Rahmen', rectf: 'Rechteck', ell: 'Ellipse', ellf: 'Ellipse gef\u00fcllt', circ: 'Kreis', circf: 'Kreis gef\u00fcllt', radius: 'Radius',
      poly: 'Polygon', fill: 'F\u00fcllen', text: 'Text', pick: 'Pipette', sym: 'Symmetrie:', txtSize: 'Gr\u00f6\u00dfe:',
      none: 'keine', mx: 'links/rechts gespiegelt', mxy: '4-fach gespiegelt', r4: '4-fach gedreht',
      r12: '12-fach gedreht (Stunden)', r60: '60-fach gedreht (Minuten)',
      pickBtn: 'Aufnehmen', pickTip: 'Farbe aus einem Pixel aufnehmen - auch per Rechtsklick in die Zeichenfl\u00e4che oder Klick in die Vorschau',
      color: 'Farbe', std: 'Standardfarben', pal: 'Palette', free: 'Beliebige Farbe', hexHint: '#RRGGBB oder RGB565 (0xFFFF)',
      edit: 'Bearbeiten', undo: 'R\u00fcckg\u00e4ngig', redo: 'Wiederholen', clear: 'Alles f\u00fcllen',
      import: 'Bild laden', cover: 'Fl\u00e4che f\u00fcllen', contain: 'ganz zeigen',
      gen: 'Zifferblatt erzeugen', bg: 'Hintergrund', ring: 'Rand', hourM: 'Stundenstriche', minM: 'Minutenstriche',
      num: 'Ziffern', nSize: 'Ziffern-Gr\u00f6\u00dfe', nDist: 'Ziffern Abstand', nFont: 'Schrift',
      numNone: 'keine', numArabic: '1 - 12', numQuarter: '12, 3, 6, 9', numRoman: 'I - XII',
      genHint: 'Striche: L\u00e4nge und Breite in Pixeln. Abstand: Ziffernmitte bis Rand.', genBtn: 'Erzeugen',
      preview: 'Vorschau', live: 'Live-Uhrzeit', showHands: 'Zeiger',
      saving: 'Speichere...', saved: 'Als neues Zifferblatt "{0}" gespeichert', activated: ' und aktiviert',
      savedCur: '"{0}" gespeichert und angewendet', loading: 'Lade...', loaded: 'Aktives Zifferblatt geladen',
      failed: 'Fehler: ', missing: 'Aktives Zifferblatt nicht lesbar - Standard verwendet',
      builtinRO: 'Das eingebaute Standard-Zifferblatt kann nicht \u00fcberschrieben werden - bitte als neues Zifferblatt speichern.',
      badName: 'Name: nur Buchstaben, Ziffern, _ und -, h\u00f6chstens 20 Zeichen.',
      confirmExists: 'Das Zifferblatt "{0}" gibt es schon - \u00fcberschreiben?',
      confirmReset: 'Alle \u00c4nderungen verwerfen und das aktive Zifferblatt neu laden?',
      confirmClear: 'Das ganze Zifferblatt mit der aktuellen Farbe f\u00fcllen?',
      imgErr: 'Bild konnte nicht gelesen werden.',
      lowSpace: 'Wenig freier Speicher ({0} KB) - ein neues Zifferblatt braucht bis zu {1} KB.',
      h_pen: 'Stift: Pixel einzeln setzen oder freihand zeichnen, in der eingestellten Stiftbreite.',
      h_line: 'Linie: vom Anfangs- zum Endpunkt ziehen.', h_rect: 'Rahmen: Rechteck-Umriss aufziehen.',
      h_rectf: 'Rechteck: gef\u00fclltes Rechteck aufziehen.',
      h_ell: 'Ellipse: im Mittelpunkt ansetzen und ziehen - waagerechter und senkrechter Abstand sind die Halbachsen.',
      h_ellf: 'Ellipse gef\u00fcllt: im Mittelpunkt ansetzen und ziehen, wie Ellipse.',
      h_circ: 'Kreis: im Mittelpunkt ansetzen und ziehen - der Abstand zur Maus ist der Radius, der Kreis ist exakt rund.',
      h_circf: 'Kreis gef\u00fcllt: im Mittelpunkt ansetzen und ziehen, wie Kreis.',
      h_fill: 'F\u00fcllen: f\u00e4rbt die zusammenh\u00e4ngende gleichfarbige Fl\u00e4che um.',
      h_pick: 'Pipette: Klick \u00fcbernimmt die Farbe des Pixels.',
      h_poly: 'Polygon: Punkte anklicken, Doppelklick oder Klick auf den ersten Punkt schlie\u00dft, Esc bricht ab.',
      h_text: 'Text: gerade - Klick setzt den Text mittig an die Stelle; im Bogen - Klick legt Radius und Mitte des Bogens fest. Einstellungen darunter.',
      h_stamp: 'Logo: unter "Logo" ein Bild laden und die Breite einstellen, dann per Klick mittig an die Stelle setzen.',
      stamp: 'Logo', txtMode: 'Anordnung:', straight: 'gerade', arcTop: 'Bogen oben', arcBottom: 'Bogen unten', txtRot: 'Drehung:',
      logo: 'Logo', logoW: 'Breite:', logoHint: 'Ideal ist ein PNG mit transparentem Hintergrund. Das Bild bleibt f\u00fcr weitere Klicks geladen.',
      noLogo: 'Zuerst unter "Logo" ein Bild laden.', fonts: 'Eigene Schrift', fontAdd: 'Hinzuf\u00fcgen', fontPh: 'installierte Schrift',
      fontFileHint: 'oder Schriftdatei (.ttf, .otf, .woff) laden - sie wird auf der Uhr gespeichert und steht f\u00fcr Text, die Ziffern im Generator und den Streifen (Uhrzeit/Datum) zur Verf\u00fcgung.',
      fontAdded: 'Schrift "{0}" hinzugef\u00fcgt und ausgew\u00e4hlt', fontMissing: 'Die Schrift "{0}" ist auf diesem PC nicht installiert.',
      fontErr: 'Schriftdatei konnte nicht gelesen werden.',
      roundHint: 'Rundes Display: der abgedunkelte Bereich ist auf der Uhr nicht sichtbar.',
      modeAs: 'Wie auf der Uhr:', mStation: 'Sekunde wartet auf 12', mSecSmooth: 'Sekunde schleichend', mSecTick: 'Sekunde tickend',
      mMinSmooth: 'Minute schleichend', mMinJump: 'Minute springt',
      strip: 'Streifen Uhrzeit/Datum', sPos: 'Lage:', sBelow: 'unter der Uhr (quer: rechts)', sAbove: '\u00fcber der Uhr (quer: links)', sBg: 'Hintergrund:', sFg: 'Schriftfarbe:', sFont: 'Schriftart:',
      sTime: 'Uhrzeit:', sDate: 'Datum:', sAuto: 'automatisch', sBlink: 'Doppelpunkt blinkt (ohne Sekunden)', sDef: 'Standard', sSave: 'Streifen speichern',
      stripHint: 'Der Streifen ist Teil der Zeichenfl\u00e4che und wird mit dem Zifferblatt gespeichert (strip_Name.bmp). '
      + '\u00c4nderungen hier zeigt die Uhr sofort an, gespeichert werden sie mit \u201eStreifen speichern\u201c. ' + 'X/Y = Mitte der Zeile im Streifen (hochkant); quer stehen die Zeilen automatisch untereinander.',
      stripSaved: 'Streifen gespeichert.', stripErr: 'Streifen konnte nicht an die Uhr gesendet werden.',
      sTFmt: 'Zeitformat:', sSec: 'Sekunden:', sec0: 'automatisch (ohne Sekundenzeiger)', sec1: 'ohne Sekunden', sec2: 'mit Sekunden',
      sDFmt: 'Datumsformat:', sVSize: 'Gr\u00f6\u00dfe Uhrzeit / Datum:', tf0: '24 Stunden',
      tf1: '12 Stunden mit AM/PM', tf2: '12 Stunden', df0: 'T.MM.JJJJ', df1: 'TT.MM.JJJJ', df2: 'TT.MM.JJ', df3: 'MM/TT/JJJJ',
      df4: 'JJJJ-MM-TT', df5: 'TT.MM.', vlwCur: ' (auf der Uhr)', vlwErr: 'Schrift f\u00fcr den Streifen konnte nicht erzeugt werden.',
      pos: 'Pixel', center: 'Mitte' },
    en: { base: 'Based on:', builtin: 'Default (built-in)', active: 'active', reset: 'Discard changes',
      activate: 'activate new clock face', saveBtn: 'Save as new clock face', name: 'Name:',
      saveCurBtn: 'Save and apply current clock face', tools: 'Tool', bw: 'Pen width:',
      pen: 'Pen', line: 'Line', rect: 'Frame', rectf: 'Rectangle', ell: 'Ellipse', ellf: 'Filled ellipse', circ: 'Circle', circf: 'Filled circle', radius: 'Radius',
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
      imgErr: 'Image could not be read.',
      lowSpace: 'Little free space ({0} KB) - a new clock face needs up to {1} KB.',
      h_pen: 'Pen: set single pixels or draw freehand, at the chosen pen width.',
      h_line: 'Line: drag from the start to the end point.', h_rect: 'Frame: drag out a rectangle outline.',
      h_rectf: 'Rectangle: drag out a filled rectangle.',
      h_ell: 'Ellipse: start at the centre and drag - horizontal and vertical distance are the semi-axes.',
      h_ellf: 'Filled ellipse: start at the centre and drag, like ellipse.',
      h_circ: 'Circle: start at the centre and drag - the distance to the mouse is the radius, the circle is exactly round.',
      h_circf: 'Filled circle: start at the centre and drag, like circle.',
      h_fill: 'Fill: recolours the connected area of the same colour.',
      h_pick: 'Picker: a click takes over the colour of the pixel.',
      h_poly: 'Polygon: click points, double-click or click the first point to close, Esc cancels.',
      h_text: 'Text: straight - a click places the text centred on that spot; on an arc - a click sets radius and centre of the arc. Settings below.',
      h_stamp: 'Logo: load an image under "Logo" and set its width, then a click places it centred on that spot.',
      stamp: 'Logo', txtMode: 'Layout:', straight: 'straight', arcTop: 'arc at the top', arcBottom: 'arc at the bottom', txtRot: 'Rotation:',
      logo: 'Logo', logoW: 'Width:', logoHint: 'A PNG with a transparent background works best. The image stays loaded for further clicks.',
      noLogo: 'Load an image under "Logo" first.', fonts: 'Own font', fontAdd: 'Add', fontPh: 'installed font',
      fontFileHint: 'or load a font file (.ttf, .otf, .woff) - it is stored on the clock and available for text, the generator numerals and the strip (time/date).',
      fontAdded: 'Font "{0}" added and selected', fontMissing: 'The font "{0}" is not installed on this PC.',
      fontErr: 'Font file could not be read.',
      roundHint: 'Round display: the darkened area is not visible on the clock.',
      modeAs: 'As on the clock:', mStation: 'second waits at 12', mSecSmooth: 'smooth second', mSecTick: 'ticking second',
      mMinSmooth: 'smooth minute', mMinJump: 'minute jumps',
      strip: 'Time/date strip', sPos: 'Placement:', sBelow: 'below the clock (landscape: right)', sAbove: 'above the clock (landscape: left)', sBg: 'Background:', sFg: 'Text colour:', sFont: 'Font:',
      sTime: 'Time:', sDate: 'Date:', sAuto: 'automatic', sBlink: 'Colon blinks (without seconds)', sDef: 'Default', sSave: 'Save strip',
      stripHint: 'The strip is part of the drawing area and is saved with the clock face (strip_name.bmp). '
      + 'The clock shows changes here right away, they are stored with "Save strip". ' + 'X/Y = centre of the line in the strip (portrait); in landscape the lines are arranged below each other automatically.',
      stripSaved: 'Strip saved.', stripErr: 'Could not send the strip to the clock.',
      sTFmt: 'Time format:', sSec: 'Seconds:', sec0: 'automatic (without second hand)', sec1: 'without seconds', sec2: 'with seconds',
      sDFmt: 'Date format:', sVSize: 'Size time / date:', tf0: '24 hours',
      tf1: '12 hours with AM/PM', tf2: '12 hours', df0: 'D.MM.YYYY', df1: 'DD.MM.YYYY', df2: 'DD.MM.YY', df3: 'MM/DD/YYYY',
      df4: 'YYYY-MM-DD', df5: 'DD.MM.', vlwCur: ' (on the clock)', vlwErr: 'Could not create the font for the strip.',
      pos: 'Pixel', center: 'Centre' },
  };
  var L = TX[FD.lang] ? FD.lang : 'en';
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
  try { var zs = +localStorage.getItem('uhr4FdZoom'); if (zs >= ZMIN && zs <= ZMAX) Z = zs; } catch (e) { }
  function applyZoom() {
    ed.width = W * Z; ed.height = H * Z;
    $('zoomInfo').textContent = 'Zoom ' + Z + 'x';
    $('zoomOut').disabled = Z <= ZMIN; $('zoomIn').disabled = Z >= ZMAX;
  }
  function setZoom(z) {
    Z = Math.max(ZMIN, Math.min(ZMAX, z));
    try { localStorage.setItem('uhr4FdZoom', Z); } catch (e) { }
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
      if (s === 'mxy') {
        var my = 2 * CY - 1 - y; // an der Uhrmitte gespiegelt / mirrored at the clock centre
        if (my >= 0 && my < H) { buf[my * W + x] = v; buf[my * W + (W - 1 - x)] = v; }
      }
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
    if (s === 'mxy') out.push([x, 2 * CY - 1 - y], [W - 1 - x, 2 * CY - 1 - y]);
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
  // Halbachsen fuer Ellipse/Kreis ab dem Mittelpunkt a (Kreis mit ganzzahligem Radius, wie im Zeiger-Designer)
  // Semi-axes for ellipse/circle from the centre a (circle with an integer radius, as in the hand designer)
  function shapeRadii(a, b) {
    var dx = b.x - a.x, dy = b.y - a.y;
    if (tool === 'circ' || tool === 'circf') { var r = Math.round(Math.sqrt(dx * dx + dy * dy)) + 0.5; return { rx: r, ry: r }; }
    return { rx: Math.abs(dx) + 0.5, ry: Math.abs(dy) + 0.5 };
  }
  // Ellipse/Kreis um die Mitte des Pixels c mit den Halbachsen rx/ry (mind. 0.5 = nur dieses Pixel)
  // Ellipse/circle around the centre of pixel c with the semi-axes rx/ry (at least 0.5 = only this pixel)
  function ellOn(mk, c, rx, ry, filled) {
    var cx = c.x + 0.5, cy = c.y + 0.5;
    function inside(x, y) { var u = (x + 0.5 - cx) / rx, w = (y + 0.5 - cy) / ry; return u * u + w * w <= 1; }
    var x0 = Math.floor(cx - rx), x1 = Math.ceil(cx + rx), y0 = Math.floor(cy - ry), y1 = Math.ceil(cy + ry);
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
    var fromCenter = /^(ell|circ)/.test(tool);
    if (!fromCenter) { c.x = Math.max(0, Math.min(W - 1, c.x)); c.y = Math.max(0, Math.min(H - 1, c.y)); }
    if (tool === 'pen') {
      var mk = new Mask(); lineOn(mk, drag.last.x, drag.last.y, c.x, c.y); paint(pix, mk, color);
      drag.last = c;
      drawEditor();
      return;
    }
    drag.last = c;
    if (fromCenter) {
      var rr = shapeRadii(drag.start, c);
      $('posInfo').textContent += '   ' + t('radius') + ' ' + (rr.rx === rr.ry ? rr.rx - 0.5 : (rr.rx - 0.5) + ' / ' + (rr.ry - 0.5));
    }
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
    else if (/^(ell|circ)f?$/.test(tool)) { var rr = shapeRadii(a, b); ellOn(mk, a, rr.rx, rr.ry, /f$/.test(tool)); }
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
  ['pen', 'line', 'rect', 'rectf', 'ell', 'ellf', 'circ', 'circf', 'poly', 'fill', 'text', 'stamp', 'pick'].forEach(function (k) {
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
  // fontFamilies: alle Schriften des Designers (auch fuer den Streifen), onFontAdded meldet neue an die Streifen-Karte
  // fontFamilies: all designer fonts (also for the strip), onFontAdded reports new ones to the strip card
  var fontFamilies = [{ family: 'sans-serif', label: 'Sans' }, { family: 'serif', label: 'Serif' }, { family: 'monospace', label: 'Mono' }];
  var onFontAdded = null;
  function addFont(family, label, quiet) {
    ['txtFont', 'gNFont'].forEach(function (id) {
      var o = document.createElement('option'); o.value = family; o.textContent = label; $(id).appendChild(o);
    });
    fontFamilies.push({ family: family, label: label });
    if (onFontAdded) onFontAdded(fontFamilies.length - 1);
    if (quiet) return;
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
  // Schriftdatei: im Browser anmelden und als font_<Name>.<ext> auf der Uhr speichern
  // Font file: register it in the browser and store it on the clock as font_<name>.<ext>
  $('fontFile').onchange = function () {
    var f = this.files[0], input = this;
    if (!f) return;
    var ext = (/\.(ttf|otf|woff2?)$/i.exec(f.name) || [])[1];
    var label = f.name.replace(/\.[^.]+$/, '').replace(/[^A-Za-z0-9_-]/g, '_').slice(0, 30) || 'font';
    f.arrayBuffer().then(function (ab) {
      return new FontFace('UF_' + label, ab).load().then(function (face) {
        document.fonts.add(face);
        if (!ext) return;
        var fd = new FormData();
        fd.append('upload', new Blob([ab]), 'font_' + label + '.' + ext.toLowerCase());
        return fetch('/upload', { method: 'POST', body: fd, redirect: 'manual' });
      });
    }).then(function () { addFont('"UF_' + label + '"', label); })
      .catch(function () { showMsg(t('fontErr'), false); }).then(function () { input.value = ''; });
  };
  // Auf der Uhr gespeicherte Schriften laden
  // Load the fonts stored on the clock
  (FD.fonts || []).forEach(function (fn) {
    var label = fn.replace(/^font_/, '').replace(/\.[^.]+$/, '');
    fetch('/file?name=' + encodeURIComponent('/' + fn), { cache: 'no-store' }).then(function (r) {
      if (!r.ok) throw new Error(r.status);
      return r.arrayBuffer();
    }).then(function (ab) { return new FontFace('UF_' + label, ab).load(); }).then(function (face) {
      document.fonts.add(face);
      addFont('"UF_' + label + '"', label, true);
    }).catch(function () {});
  });

  // VLW-Schrift (kantengeglaettet, wie LovyanGFX sie laedt) fuer die Streifen-Zeichen aus einer Browser-Schrift:
  // Kopf 6 x int32, je Zeichen 7 x int32 (Unicode, Hoehe, Breite, Vorschub, Oberkante ueber Grundlinie, linker
  // Versatz, 0), danach die Alpha-Bitmaps. Alle Zahlen big-endian, Zeichen nach Unicode sortiert.
  // VLW font (anti-aliased, as LovyanGFX loads it) for the strip characters from a browser font: header 6 x
  // int32, per character 7 x int32 (unicode, height, width, advance, top above baseline, left offset, 0), then
  // the alpha bitmaps. All numbers big-endian, characters sorted by unicode.
  function makeVlw(css, px) {
    var chars = ' -./0123456789:AMP'.split(''), pad = Math.ceil(px * 0.5);
    var c = newCanvas(px * 4, px * 3), x = c.getContext('2d');
    x.font = css; x.textBaseline = 'alphabetic';
    var ref = x.measureText('0');
    var ascent = Math.ceil(ref.fontBoundingBoxAscent || ref.actualBoundingBoxAscent || px * 0.8);
    var descent = Math.ceil(ref.fontBoundingBoxDescent || px * 0.2);
    var glyphs = chars.map(function (ch) {
      var m = x.measureText(ch);
      var l = Math.ceil(m.actualBoundingBoxLeft || 0), r = Math.ceil(m.actualBoundingBoxRight || 0);
      var a = Math.ceil(m.actualBoundingBoxAscent || 0), d = Math.ceil(m.actualBoundingBoxDescent || 0);
      var w = Math.min(255, Math.max(0, l + r)), h = Math.max(0, a + d), bits = new Uint8Array(0);
      if (ch === ' ' || !w || !h) { w = 0; h = 0; }
      else {
        x.clearRect(0, 0, c.width, c.height); x.fillStyle = '#fff';
        x.fillText(ch, pad + l, pad + a);
        var id = x.getImageData(pad, pad, w, h).data;
        bits = new Uint8Array(w * h);
        for (var i = 0; i < w * h; i++) bits[i] = id[i * 4 + 3];
      }
      return { u: ch.charCodeAt(0), w: w, h: h, adv: Math.min(255, Math.round(m.width)), dy: a, dx: -l, bits: bits };
    });
    var size = 24 + glyphs.length * 28;
    glyphs.forEach(function (g) { size += g.bits.length; });
    var ab = new ArrayBuffer(size), dv = new DataView(ab), bytes = new Uint8Array(ab), o = 24;
    [glyphs.length, 11, px, 0, ascent, descent].forEach(function (v, i) { dv.setInt32(i * 4, v); });
    glyphs.forEach(function (g) {
      [g.u, g.h, g.w, g.adv, g.dy, g.dx, 0].forEach(function (v, i) { dv.setInt32(o + i * 4, v); });
      o += 28;
    });
    glyphs.forEach(function (g) { bytes.set(g.bits, o); o += g.bits.length; });
    return new Blob([ab], { type: 'application/octet-stream' });
  }

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
    commit(keepStrip(canvasBuf(c)));
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
  function decodeImg(ab, w, h) {
    var img = parseBmp(ab), out = new Uint16Array(w * h);
    for (var ty = 0; ty < h; ty++) for (var tx = 0; tx < w; tx++)
      out[ty * w + tx] = img.px(Math.floor(tx * img.w / w), Math.floor(ty * img.h / h));
    return out;
  }
  function decodeFace(ab) { return decodeImg(ab, W, FH); }
  function encodeBmp(buf, w, h) {
    var rs = Math.floor((w * 2 + 3) / 4) * 4, hs = 66, size = hs + rs * h;
    var ab = new ArrayBuffer(size), dv = new DataView(ab);
    dv.setUint8(0, 66); dv.setUint8(1, 77); dv.setUint32(2, size, true); dv.setUint32(10, hs, true);
    dv.setUint32(14, 40, true); dv.setInt32(18, w, true); dv.setInt32(22, -h, true); dv.setUint16(26, 1, true);
    dv.setUint16(28, 16, true); dv.setUint32(30, 3, true); dv.setUint32(34, rs * h, true);
    dv.setUint32(54, 0xF800, true); dv.setUint32(58, 0x07E0, true); dv.setUint32(62, 0x001F, true);
    for (var y = 0; y < h; y++) for (var x = 0; x < w; x++) dv.setUint16(hs + y * rs + x * 2, buf[y * w + x], true);
    return new Blob([ab], { type: 'image/bmp' });
  }

  // Zifferblatt und Streifen liegen gemeinsam in pix: Zifferblatt ab Zeile OY, Streifen ab stripY()
  // Clock face and strip share pix: clock face from row OY, strip from stripY()
  function stripY() { return OY ? 0 : FH; }
  function rows(buf, y0, h) { return buf.slice(y0 * W, (y0 + h) * W); }
  function hexTo565(h) { var n = parseInt(h.slice(1), 16); return rgb565((n >> 16) & 255, (n >> 8) & 255, n & 255); }
  function compose(face, strip) {
    var out = new Uint16Array(N);
    out.set(face.subarray(0, W * FH), OY * W);
    if (SH) {
      if (strip) out.set(strip.subarray(0, W * SH), stripY() * W);
      else out.fill(hexTo565(FD.strip.bg), stripY() * W, (stripY() + SH) * W);
    }
    return out;
  }
  // Ergebnis des Generators nur fuers Zifferblatt - der Streifen bleibt, wie er ist
  // Generator result for the clock face only - the strip stays as it is
  function keepStrip(buf) {
    if (SH) buf.set(rows(pix, stripY(), SH), stripY() * W);
    return buf;
  }
  // Lage des Streifens umschalten: Zifferblatt und Streifen tauschen die Plaetze (Rueckgaengig wird geleert)
  // Switch the strip placement: clock face and strip swap places (undo is cleared)
  function setStripBefore(before) {
    var oy = (SH && before) ? SH : 0;
    if (!SH || oy === OY) return;
    var face = rows(pix, OY, FH), strip = rows(pix, stripY(), SH);
    OY = oy; CY = OY + FH / 2;
    pix = compose(face, strip);
    undoSt = []; redoSt = [];
    drawEditor(); schedulePreview();
  }
  // Streifen-Grafik zum Zifferblatt (strip_<Name>.bmp), null wenn keine da ist
  // Strip graphic of the clock face (strip_<name>.bmp), null if there is none
  function fetchStrip(facePath) {
    if (!SH || !facePath || facePath === DEF) return Promise.resolve(null);
    var sp = '/strip_' + facePath.replace(/^\/?face_/, '');
    return fetch('/file?name=' + encodeURIComponent(sp), { cache: 'no-store' }).then(function (r) {
      if (!r.ok) return null;
      return r.arrayBuffer().then(function (ab) { return decodeImg(ab, W, SH); });
    }).catch(function () { return null; });
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
    }).then(function (b) {
      return fetchStrip(base).then(function (st) { pix = compose(b, st); });
    }, function () { pix = compose(new Uint16Array(W * FH).fill(0xFFFF), null); base = DEF; }).then(function () {
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
    function upload(blob, name) {
      var fd = new FormData();
      fd.append('upload', blob, name);
      return fetch('/upload', { method: 'POST', body: fd, redirect: 'manual' }).then(function (r) {
        if (!(r.ok || r.type === 'opaqueredirect')) throw new Error('HTTP ' + r.status);
      });
    }
    upload(encodeBmp(rows(pix, OY, FH), W, FH), file).then(function () {
      if (SH) return upload(encodeBmp(rows(pix, stripY(), SH), W, SH), 'strip_' + file.replace(/^face_/, ''));
    }).then(function () {
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
  pv.width = W; pv.height = H;

  // Mit Streifen (ILI9341) zeigt die Vorschau das ganze Display. Uhrzeit und Datum zeichnet die Uhr
  // (/api/stripimg?text=1, RGB565 big-endian, Hintergrund 0x0120 = transparent) - sie liegen ueber der Zeichnung.
  // With a strip (ILI9341) the preview shows the whole display. The clock draws time and date
  // (/api/stripimg?text=1, RGB565 big-endian, background 0x0120 = transparent) - they lie over the drawing.
  var stripImg = null;
  function loadStripImg() {
    var sp = FD.strip;
    if (!sp) return;
    fetch('/api/stripimg?text=1', { cache: 'no-store' }).then(function (r) {
      if (!r.ok) throw new Error(r.status);
      return r.arrayBuffer();
    }).then(function (ab) {
      var b = new Uint8Array(ab);
      if (b.length < sp.w * sp.h * 2) return;
      var c = stripImg || newCanvas(sp.w, sp.h), x = c.getContext('2d'), id = x.createImageData(sp.w, sp.h);
      for (var i = 0; i < sp.w * sp.h; i++) {
        var v = (b[2 * i] << 8) | b[2 * i + 1], rgb = rgbOf(v);
        id.data[i * 4] = rgb[0]; id.data[i * 4 + 1] = rgb[1]; id.data[i * 4 + 2] = rgb[2]; id.data[i * 4 + 3] = v === 0x0120 ? 0 : 255;
      }
      x.putImageData(id, 0, 0);
      stripImg = c;
      schedulePreview();
    }).catch(function () {});
  }
  var hands = {}, previewTimer = null;
  $('live').onchange = schedulePreview;
  $('showHands').onchange = schedulePreview;

  // Zeiger-BMP: Weiss und 0x0120 sind transparent (wie loadHandSprites() im Geraet)
  // Hand BMP: white and 0x0120 are transparent (like loadHandSprites() on the device)
  // Gueltige Formate mittig und unten buendig, wie placeHand() im Geraet
  // Valid formats centred and flush at the bottom, like placeHand() on the device
  function decodeHand(ab) {
    var img = parseBmp(ab), out = new Int32Array(HW * HH).fill(-1);
    var known = (img.w === HW || img.w === FD.hand.lw) && (img.h === HH || img.h === FD.hand.lh);
    var sw = known ? img.w : HW, sh = known ? img.h : HH, ox = (HW - sw) >> 1, oy = HH - sh;
    for (var y = 0; y < sh; y++) for (var x = 0; x < sw; x++) {
      var v = img.px(Math.floor(x * img.w / sw), Math.floor(y * img.h / sh));
      out[(y + oy) * HW + x + ox] = (v === 0xFFFF || v === 0x0120) ? -1 : v;
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
    pctx.clearRect(0, 0, pv.width, pv.height);
    if (FD.round) { pctx.beginPath(); pctx.arc(CX, CY, R, 0, Math.PI * 2); pctx.clip(); }
    if (!pvFace) pvFace = bufCanvas(pix);
    pctx.drawImage(pvFace, 0, 0);
    if (SH && stripImg) pctx.drawImage(stripImg, 0, stripY());
    if ($('showHands').checked) {
      var ang = handAngles($('live').checked ? new Date() : new Date(2000, 0, 1, 10, 8, 37), FD.mode);
      PARTS.forEach(function (p) {
        if (!hands[p] || (p === 'second' && !FD.showSec)) return;
        pctx.save();
        pctx.translate(CX, CY);
        pctx.rotate(ang[p] * Math.PI / 180);
        pctx.drawImage(hands[p], -((spriteWidth(p) >> 1) + 0.5), -(HPY + 0.5));
        pctx.restore();
      });
      if (FD.hub > 0) {
        pctx.fillStyle = FD.hubColor;
        pctx.beginPath(); pctx.arc(CX, CY, FD.hub, 0, Math.PI * 2); pctx.fill();
      }
    }
    pctx.restore();
  }
  // Zeigerwinkel wie renderClockFrame() im Geraet: Bahnhofsuhr (Sekunde eilt
  // und wartet auf der 12), schleichende oder springende Minute und Sekunde.

  // Hand angles like renderClockFrame() on the device: station clock (second
  // races and waits at 12), smooth or jumping minute and second.
  function handAngles(d, M) {
    var h = d.getHours() % 12, m = d.getMinutes(), s = d.getSeconds(), ms = d.getMilliseconds(), sec;
    M = M || {};
    if (M.station) {
      var el = s * 1000 + ms, fast = M.fastMs || 975, tick = Math.floor(el / fast), sub = (el % fast) / fast;
      sec = Math.min(M.smoothSec ? tick + (1 - Math.cos(Math.PI * Math.sqrt(sub))) / 2 : tick, 60);
    }
    else sec = M.smoothSec ? s + ms / 1000 : s;
    return { hour: (h + m / 60 + s / 3600) * 30, minute: (M.smoothMin && !M.station) ? (m + s / 60) * 6 : m * 6, second: sec * 6 };
  }
  function modeText(M) {
    M = M || {};
    return t('modeAs') + ' ' + [M.station ? t('mStation') : '', M.smoothSec ? t('mSecSmooth') : t('mSecTick'),
      (M.smoothMin && !M.station) ? t('mMinSmooth') : t('mMinJump')].filter(Boolean).join(', ');
  }
  var pvFace = null;
  function schedulePreview() {
    pvFace = null;
    if (previewTimer) clearTimeout(previewTimer);
    previewTimer = setTimeout(renderPreview, 60);
  }
  // Live-Uhrzeit fluessig pro Bildschirmbild, pausiert bei verstecktem Tab
  // Live time smoothly per display frame, paused while the tab is hidden
  (function animate() {
    if ($('live').checked && $('showHands').checked && !document.hidden) renderPreview();
    requestAnimationFrame(animate);
  })();
  $('modeInfo').textContent = modeText(FD.mode);

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

  // Streifen Uhrzeit/Datum (nur Displays mit Streifen wie ILI9341): Aenderungen gehen sofort live an die Uhr
  // (save=0), "Streifen speichern" legt sie dauerhaft ab.
  // Time/date strip (only displays with a strip like the ILI9341): changes go live to the clock right away
  // (save=0), "Save strip" stores them permanently.
  if (FD.strip) {
    var SP = FD.strip, stripTimer = null;
    $('stripCard').style.display = '';
    [['0', 'sBelow'], ['1', 'sAbove']].forEach(function (k) {
      var o = document.createElement('option'); o.value = k[0]; o.textContent = t(k[1]); $('sBefore').appendChild(o);
    });
    var lastVlwKey = null;
    var opt = function (sel, value, text) {
      var o = document.createElement('option'); o.value = value; o.textContent = text; $(sel).appendChild(o); return o;
    };
    SP.fonts.forEach(function (n, i) { opt('sFont', String(i), n); });
    if (SP.font === 255) opt('sFont', 'v:cur', (SP.vlw || 'VLW') + t('vlwCur'));
    [0, 1, 2].forEach(function (i) { opt('sTFmt', String(i), t('tf' + i)); });
    [0, 1, 2].forEach(function (i) { opt('sSec', String(i), t('sec' + i)); });
    [0, 1, 2, 3, 4, 5].forEach(function (i) { opt('sDFmt', String(i), t('df' + i)); });

    // Designer-Schriften als VLW anbieten; die auf der Uhr aktive (SP.vlw, " B" = fett) wird wiedererkannt
    // Offer designer fonts as VLW; the one active on the clock (SP.vlw, " B" = bold) is recognised
    onFontAdded = function (i) {
      var f = fontFamilies[i];
      opt('sFont', 'v:' + i, f.label);
      if (SP.font === 255 && $('sFont').value === 'v:cur' && SP.vlw.replace(/ B$/, '') === f.label) {
        $('sFont').value = 'v:' + i;
        $('sVB').checked = / B$/.test(SP.vlw);
        lastVlwKey = f.family + '|' + SP.vt + '|' + SP.vd + '|' + $('sVB').checked;
        vlwRows();
      }
    };
    var vlwRows = function () {
      var v = $('sFont').value.indexOf('v:') === 0;
      document.querySelectorAll('#stripCard .vlwRow').forEach(function (e) { e.style.display = v ? '' : 'none'; });
      $('sVT').disabled = $('sVD').disabled = $('sVB').disabled = $('sFont').value === 'v:cur';
    };
    // Dateiname wie stripVlwPath() in display.h: stripfont_<Name>_<Groesse>.vlw, Leerzeichen -> '-'
    // File name like stripVlwPath() in display.h: stripfont_<name>_<size>.vlw, spaces -> '-'
    var vlwFile = function (name, px) { return 'stripfont_' + name.replace(/ /g, '-').replace(/[^A-Za-z0-9_-]/g, '') + '_' + px + '.vlw'; };
    var uploadVlw = function (family, name, vt, vd, bold) {
      var css = function (px) { return (bold ? 'bold ' : '') + px + 'px ' + family; };
      var up = function (px, name) {
        var fd = new FormData();
        fd.append('upload', makeVlw(css(px), px), name);
        return fetch('/upload', { method: 'POST', body: fd, redirect: 'manual' });
      };
      return Promise.all([document.fonts.load(css(vt), '0123456789'), document.fonts.load(css(vd), '0123456789')]).then(function () {
        return up(vt, vlwFile(name, vt));
      }).then(function () { if (vd !== vt) return up(vd, vlwFile(name, vd)); });
    };
    fontFamilies.forEach(function (f, i) { onFontAdded(i); });
    $('sTX').max = $('sDX').max = SP.w; $('sTY').max = $('sDY').max = SP.h;
    var stripSync = function () {
      [['T', SP.aty], ['D', SP.ady]].forEach(function (k) {
        var auto = $('s' + k[0] + 'Auto').checked;
        $('s' + k[0] + 'X').disabled = $('s' + k[0] + 'Y').disabled = auto;
        if (auto) { $('s' + k[0] + 'X').value = SP.w >> 1; $('s' + k[0] + 'Y').value = k[1]; }
      });
    };
    var stripFill = function () {
      $('sBefore').value = SP.before ? '1' : '0'; $('sBlink').checked = !!SP.blink; $('sBg').value = SP.bg; $('sFg').value = SP.fg;
      if (SP.font !== 255) $('sFont').value = String(SP.font);
      else if ($('sFont').value.indexOf('v:') !== 0) $('sFont').value = 'v:cur';
      $('sTFmt').value = String(SP.tfmt || 0); $('sSec').value = String(SP.sec || 0); $('sDFmt').value = String(SP.dfmt || 0);
      $('sVT').value = SP.vt || 44; $('sVD').value = SP.vd || 22;
      vlwRows();
      $('sTAuto').checked = SP.tx < 0; $('sDAuto').checked = SP.dx < 0;
      $('sTX').value = SP.tx < 0 ? SP.w >> 1 : SP.tx; $('sTY').value = SP.ty < 0 ? SP.aty : SP.ty;
      $('sDX').value = SP.dx < 0 ? SP.w >> 1 : SP.dx; $('sDY').value = SP.dy < 0 ? SP.ady : SP.dy;
      stripSync();
    };
    var stripSend = function (save) {
      var p = new URLSearchParams();
      SP.before = +$('sBefore').value; SP.bg = $('sBg').value;
      p.set('before', $('sBefore').value); p.set('blink', $('sBlink').checked ? '1' : '0'); p.set('bg', $('sBg').value); p.set('fg', $('sFg').value);
      p.set('tfmt', $('sTFmt').value); p.set('sec', $('sSec').value); p.set('dfmt', $('sDFmt').value);

      // Designer-Schrift: VLW-Dateien nur neu erzeugen, wenn sich Schrift, Groesse oder Fett geaendert haben
      // Designer font: only create new VLW files if font, size or bold changed
      var fv = $('sFont').value, prep = Promise.resolve();
      if (fv.indexOf('v:') === 0) {
        p.set('font', 255);
        if (fv !== 'v:cur') {
          var ff = fontFamilies[+fv.slice(2)], vt = +$('sVT').value || 44, vd = +$('sVD').value || 22, bold = $('sVB').checked;
          var key = ff.family + '|' + vt + '|' + vd + '|' + bold;
          p.set('vlw', ff.label + (bold ? ' B' : '')); p.set('vt', vt); p.set('vd', vd);
          SP.vlw = ff.label + (bold ? ' B' : ''); SP.vt = vt; SP.vd = vd;
          if (key !== lastVlwKey) prep = uploadVlw(ff.family, SP.vlw, vt, vd, bold).then(function () { lastVlwKey = key; });
        }
      }
      else p.set('font', fv);
      SP.font = fv.indexOf('v:') === 0 ? 255 : +fv;
      p.set('tx', $('sTAuto').checked ? -1 : $('sTX').value); p.set('ty', $('sTAuto').checked ? -1 : $('sTY').value);
      p.set('dx', $('sDAuto').checked ? -1 : $('sDX').value); p.set('dy', $('sDAuto').checked ? -1 : $('sDY').value);
      p.set('save', save ? '1' : '0');
      return prep.then(function () {
        return fetch('/save_strip', { method: 'POST', body: p });
      }, function (e) { showMsg(t('vlwErr'), false); throw e; }).then(function (r) { return r.json(); }).then(function (j) {
        if (!j.ok) throw new Error();
        SP.aty = j.aty; SP.ady = j.ady;
        stripSync();
        loadStripImg();
        if (save) showMsg(t('stripSaved'), true);
      }).catch(function () { showMsg(t('stripErr'), false); });
    };
    var stripLive = function () {
      stripSync();
      clearTimeout(stripTimer);
      stripTimer = setTimeout(function () { stripSend(false); }, 250);
    };
    $('sFont').addEventListener('change', vlwRows);
    ['sBefore', 'sBlink', 'sBg', 'sFg', 'sFont', 'sVT', 'sVD', 'sVB', 'sTFmt', 'sSec', 'sDFmt', 'sTAuto', 'sDAuto', 'sTX', 'sTY', 'sDX', 'sDY'].forEach(function (id) {
      $(id).addEventListener('input', stripLive);
      $(id).addEventListener('change', stripLive);
    });
    $('sBefore').addEventListener('change', function () { setStripBefore(this.value === '1'); });
    $('sDefBtn').onclick = function () {
      setStripBefore(false);
      SP.before = 0; SP.blink = 1; SP.bg = '#000000'; SP.fg = '#ffffff'; SP.font = 0; SP.tfmt = 0; SP.sec = 0; SP.dfmt = 0;
      SP.tx = SP.ty = SP.dx = SP.dy = -1;
      stripFill(); stripLive();
    };
    $('sSaveBtn').onclick = function () { clearTimeout(stripTimer); stripSend(true); };
    var stripLabels = { tStrip: 'strip', tSPos: 'sPos', tSBg: 'sBg', tSFg: 'sFg', tSFont: 'sFont', tSTime: 'sTime', tSDate: 'sDate',
      stripHint: 'stripHint', tSVSize: 'sVSize', tSTFmt: 'sTFmt', tSSec: 'sSec', tSDFmt: 'sDFmt' };
    Object.keys(stripLabels).forEach(function (id) { $(id).textContent = t(stripLabels[id]); });
    document.querySelectorAll('#stripCard .tSAuto').forEach(function (e) { e.textContent = t('sAuto'); });
    $('tSBlink').textContent = t('sBlink');
    $('sDefBtn').textContent = t('sDef'); $('sSaveBtn').textContent = t('sSave');
    stripFill();
    loadStripImg();
    setInterval(loadStripImg, 15000); // Uhrzeit im Streifen der Vorschau aktuell halten
                                      // keep the time in the preview's strip current
  }

  // Start
  // Start
  selectTool('pen');
  setColor(color);
  loadActive();
})();
</script>
)FDRAW";
