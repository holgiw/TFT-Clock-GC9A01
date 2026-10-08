(function () {

  // Mit Streifen (ILI9341) ist die Zeichenflaeche das ganze Display: Zifferblatt (FH hoch) plus Streifen (SH),
  // der Streifen oben (OY = SH) oder unten (OY = 0). CX/CY ist die Uhrmitte, nicht die Bildmitte.

  // With a strip (ILI9341) the drawing area is the whole display: clock face (FH high) plus strip (SH), the
  // strip on top (OY = SH) or at the bottom (OY = 0). CX/CY is the clock centre, not the image centre.

  var FH = FD.h, SH = FD.strip ? FD.strip.h : 0, OY = (FD.strip && FD.strip.before) ? SH : 0;
  var W = FD.w, H = FH + SH, N = W * H, CX = W / 2, CY = OY + FH / 2, R = Math.min(W, FH) / 2;
  var DEF = '/face_default.bmp';

  var TX = {
    de: { base: 'Basis:', active: 'aktiv', reset: '\u00c4nderungen verwerfen',
      saveBtn: 'Als neues Zifferblatt speichern', name: 'Name:',
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
      sTime: 'Uhrzeit', sDate: 'Datum', sWday: 'Wochentag (ausgeschrieben)', sShow: 'anzeigen', sSize: 'Gr\u00f6\u00dfe:', sXY: 'Position:', sAuto: 'automatisch', sBlink: 'Doppelpunkt blinkt (ohne Sekunden)', sDef: 'Standard', sSave: 'Streifen speichern',
      stripHint: 'Der Streifen geh\u00f6rt zum Zifferblatt: Grafik (strip_Name.bmp) und diese Einstellungen (stripcfg_Name.txt). '
      + '\u00c4nderungen hier zeigt die Uhr sofort an, gespeichert werden sie mit \u201eStreifen speichern\u201c oder mit dem Zifferblatt. '
      + 'Ein Zifferblatt ohne eigene Einstellungen bekommt den Standard. ' + 'X/Y = Mitte der Zeile im Streifen (hochkant); quer stehen die Zeilen automatisch untereinander.',
      stripSaved: 'Streifen gespeichert.', stripErr: 'Streifen konnte nicht an die Uhr gesendet werden.',
      hub: 'Nabe', hubSize: 'Radius:', hubColor: 'Farbe:', hubSave: 'Nabe speichern',
      hubHint: 'Mittelpunkt \u00fcber den Zeigern, 0 = keine Nabe. \u00c4nderungen zeigt die Uhr sofort an, gespeichert werden sie mit \u201eNabe speichern\u201c.',
      hubSaved: 'Nabe gespeichert.', hubErr: 'Nabe konnte nicht an die Uhr gesendet werden.',
      sTFmt: 'Zeitformat:', sSec: 'Sekunden:', sec0: 'automatisch (ohne Sekundenzeiger)', sec1: 'ohne Sekunden', sec2: 'mit Sekunden',
      sDFmt: 'Datumsformat:', tf0: '24 Stunden',
      tf1: '12 Stunden mit AM/PM', tf2: '12 Stunden', df0: 'T.MM.JJJJ', df1: 'TT.MM.JJJJ', df2: 'TT.MM.JJ', df3: 'MM/TT/JJJJ',
      df4: 'JJJJ-MM-TT', df5: 'TT.MM.', df6: 'T.MM.JJ', vlwCur: ' (auf der Uhr)', vlwErr: 'Schrift f\u00fcr den Streifen konnte nicht erzeugt werden.',
      pos: 'Pixel', center: 'Mitte' },
    en: { base: 'Based on:', active: 'active', reset: 'Discard changes',
      saveBtn: 'Save as new clock face', name: 'Name:',
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
      sTime: 'Time', sDate: 'Date', sWday: 'Weekday (written out)', sShow: 'show', sSize: 'Size:', sXY: 'Position:', sAuto: 'automatic', sBlink: 'Colon blinks (without seconds)', sDef: 'Default', sSave: 'Save strip',
      stripHint: 'The strip belongs to the clock face: graphic (strip_name.bmp) and these settings (stripcfg_name.txt). '
      + 'The clock shows changes here right away, they are stored with "Save strip" or with the clock face. '
      + 'A clock face without its own settings gets the default. ' + 'X/Y = centre of the line in the strip (portrait); in landscape the lines are arranged below each other automatically.',
      stripSaved: 'Strip saved.', stripErr: 'Could not send the strip to the clock.',
      sTFmt: 'Time format:', sSec: 'Seconds:', sec0: 'automatic (without second hand)', sec1: 'without seconds', sec2: 'with seconds',
      sDFmt: 'Date format:', tf0: '24 hours',
      tf1: '12 hours with AM/PM', tf2: '12 hours', df0: 'D.MM.YYYY', df1: 'DD.MM.YYYY', df2: 'DD.MM.YY', df3: 'MM/DD/YYYY',
      df4: 'YYYY-MM-DD', df5: 'DD.MM.', df6: 'D.MM.YY', vlwCur: ' (on the clock)', vlwErr: 'Could not create the font for the strip.',
      hub: 'Hub', hubSize: 'Radius:', hubColor: 'Colour:', hubSave: 'Save hub',
      hubHint: 'Centre over the hands, 0 = no hub. The clock shows changes right away, they are stored with "Save hub".',
      hubSaved: 'Hub saved.', hubErr: 'Could not send the hub to the clock.',
      pos: 'Pixel', center: 'Centre' },
  };
  var L = TX[FD.lang] ? FD.lang : 'en';
  function t(k, a, b) {
    var s = TX[L][k] || TX.en[k] || k;
    if (a !== undefined) s = s.replace('{0}', a);
    return b === undefined ? s : s.replace('{1}', b);
  }

  // Zustand
  // State

  var pix = new Uint16Array(N).fill(0xFFFF), undoSt = [], redoSt = [];
  var tool = 'pen', color = 0x0000, dirty = false, base = DEF;

  // Ueberschreiben nur fuer Dateien, deren Name /upload auch annimmt
  // Overwriting only for files whose name /upload accepts as well

  function canOverwrite() { return /^\/face_[A-Za-z0-9_.-]+\.bmp$/.test(base); }
  // Beide Knoepfe sind immer bedienbar, auch ohne Aenderung
  // Both buttons are always usable, even without a change

  function setDirty(v) {
    dirty = v; $('saveBtn').disabled = false;
    $('saveCurBtn').disabled = !canOverwrite();
  }

  // Farben (RGB565) - im Zifferblatt ist jede Farbe erlaubt, auch Weiss
  // Colours (RGB565) - every colour is allowed in the clock face, white included

  function rgb565(r, g, b) { return (Math.round(r * 31 / 255) << 11) | (Math.round(g * 63 / 255) << 5) | Math.round(b * 31 / 255); }
  function to565(hex) { return rgb565(parseInt(hex.substr(1, 2), 16), parseInt(hex.substr(3, 2), 16), parseInt(hex.substr(5, 2), 16)); }

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

  // Fuellen bleibt im Bereich des Klickpunkts areaY (Zifferblatt oder Streifen) - beide werden getrennt
  // gespeichert. Gedrehte Symmetrie-Startpunkte ausserhalb dieses Bereichs fuellen nichts.

  // Filling stays within the area of the click point areaY (clock face or strip) - both are saved separately.
  // Rotated symmetry start points outside this area fill nothing.

  function floodOn(buf, x, y, v, areaY) {
    if (x < 0 || y < 0 || x >= W || y >= H) return;
    var y0 = 0, y1 = H;
    if (SH) {
      var sy = stripY(), ay = areaY === undefined ? y : areaY;
      if (ay >= sy && ay < sy + SH) { y0 = sy; y1 = sy + SH; } else { y0 = OY; y1 = OY + FH; }
      if (y < y0 || y >= y1) return;
    }
    var target = buf[y * W + x];
    if (target === v) return;
    var stack = [x, y];
    while (stack.length) {
      var py = stack.pop(), px = stack.pop();
      if (px < 0 || py < y0 || px >= W || py >= y1 || buf[py * W + px] !== target) continue;
      buf[py * W + px] = v;
      stack.push(px + 1, py, px - 1, py, px, py + 1, px, py - 1);
    }
  }

  // Text mit Kantenglaettung, mit dem Untergrund gemischt. Im Bogen: Radius und Bogenmitte kommen aus dem
  // Klickpunkt, oben laeuft der Text im Uhrzeigersinn, unten gegen ihn - so steht er an beiden Stellen
  // aufrecht.

  // Anti-aliased text, blended with the underlying pixels. On an arc: radius and arc centre come from the
  // click point, at the top the text runs clockwise, at the bottom anticlockwise - so it stands upright in
  // both places.

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
      seeds(c.x, c.y).forEach(function (p) { floodOn(fb, p[0], p[1], color, c.y); });
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

  // Eigene Schriften: installierte per Name (Test ueber abweichende Textbreite gegen die Ersatzschriften)
  // oder als Datei ueber FontFace - beide in Text und Generator. fontFamilies: alle Schriften des Designers
  // (auch fuer den Streifen), onFontAdded meldet neue an die Streifen-Karte.

  // Own fonts: installed ones by name (checked via a text width differing from the fallback fonts) or as a
  // file via FontFace - both in text and generator. fontFamilies: all designer fonts (also for the strip),
  // onFontAdded reports new ones to the strip card.

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
    var label = f.name.replace(/\.[^.]+$/, '').replace(/^font_/i, '').replace(/[^A-Za-z0-9_-]/g, '_').slice(0, 30) || 'font';
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

  function makeVlw(css, px, extra) {
    var chars = (' -./0123456789:AMP' + (extra || '')).split(''), pad = Math.ceil(px * 0.5);
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
    if (!SH || !facePath) return Promise.resolve(null);
    var sp = '/strip_' + facePath.replace(/^\/?face_/, '');
    return fetch('/file?name=' + encodeURIComponent(sp), { cache: 'no-store' }).then(function (r) {
      if (!r.ok) return null;
      return r.arrayBuffer().then(function (ab) { return decodeImg(ab, W, SH); });
    }).catch(function () { return null; });
  }

  // Laden und Speichern
  // Load and save

  function faceLabel(path) { return path.replace(/^\/?face_/, '').replace(/\.bmp$/, ''); }
  function showBase() {
    $('baseInfo').textContent = t('base') + ' ' + faceLabel(base) + ' (' + t('active') + ')';
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
    var note = '';
    showMsg(t('loading'));
    base = FD.active || DEF;
    fetchFace('/file?name=' + encodeURIComponent(base)).then(null, function () {
      note = t('missing'); base = DEF;
      return fetchFace('/file?name=' + encodeURIComponent(DEF));
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
  // Weiterleitung NICHT folgen; danach aktivieren - /setbackground laedt neu.

  // Upload via /upload (RLE-compressed there, masked on round displays),
  // do NOT follow the redirect; then activate - /setbackground reloads.

  // Streifen-Einstellungen zu einem Zifferblatt speichern (gesetzt im Streifen-Bereich, nur Displays mit Streifen)
  // Store strip settings for a clock face (set in the strip section, only displays with a strip)

  var stripSaveFor = null;

  function saveFace(file, isNew) {
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
      if (stripSaveFor) return stripSaveFor(file);
    }).then(function () {
      return fetch('/setbackground?file=' + encodeURIComponent(file), { redirect: 'manual' }).then(function () {
        FD.active = '/' + file; base = FD.active;
      });
    }).then(function () {
      if (FD.faces.indexOf(file) < 0) FD.faces.push(file);
      setDirty(false);
      showBase();
      $('faceName').value = nextName();
      showMsg(isNew ? t('saved', faceLabel(file)) + t('activated') : t('savedCur', faceLabel(file)), true);
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
    saveFace(file, true);
  };
  $('saveCurBtn').onclick = function () {
    if (canOverwrite()) saveFace(base.substring(1), false);
  };
  window.addEventListener('beforeunload', function (ev) { if (dirty) { ev.preventDefault(); ev.returnValue = ''; } });

  // Vorschau mit den Zeigern des aktiven Satzes
  // Preview with the hands of the active set

  var HW = FD.hand.w, HH = FD.hand.h, HPY = FD.hand.py, PARTS = ['hour', 'minute', 'second'];
  var pv = $('pv'), pctx = pv.getContext('2d'), S = W;
  pv.width = W; pv.height = H;

  // Mit Streifen (ILI9341) zeigt die Vorschau das ganze Display. Uhrzeit und Datum zeichnet die Uhr zweimal
  // (/api/stripimg?text=2, RGB565 big-endian, auf Schwarz und auf Weiss) - aus dem Unterschied folgt die Deckkraft
  // je Pixel, kantengeglaettete Schrift liegt so ohne Saum ueber der Zeichnung.

  // With a strip (ILI9341) the preview shows the whole display. The clock draws time and date twice
  // (/api/stripimg?text=2, RGB565 big-endian, on black and on white) - the difference gives the opacity per
  // pixel, so anti-aliased text lies over the drawing without a fringe.

  var stripImg = null;
  function loadStripImg() {
    var sp = FD.strip;
    if (!sp) return Promise.resolve();
    return fetch('/api/stripimg?text=2' + ($('live').checked ? '' : '&h=10&m=8&s=37'), { cache: 'no-store' }).then(function (r) {
      if (!r.ok) throw new Error(r.status);
      return r.arrayBuffer();
    }).then(function (ab) {
      var b = new Uint8Array(ab), n = sp.w * sp.h;
      if (b.length < n * 4) return;
      var c = stripImg || newCanvas(sp.w, sp.h), x = c.getContext('2d'), id = x.createImageData(sp.w, sp.h);
      for (var i = 0; i < n; i++) {
        var k = rgbOf((b[2 * i] << 8) | b[2 * i + 1]), w = rgbOf((b[2 * (n + i)] << 8) | b[2 * (n + i) + 1]);
        var a = 1 - ((w[0] - k[0]) + (w[1] - k[1]) + (w[2] - k[2])) / 765;
        if (a < 0.03) continue;
        a = Math.min(1, a);
        for (var j = 0; j < 3; j++) id.data[i * 4 + j] = Math.min(255, Math.round(k[j] / a));
        id.data[i * 4 + 3] = Math.round(a * 255);
      }
      x.putImageData(id, 0, 0);
      stripImg = c;
      schedulePreview();
    }).catch(function () {});
  }

  // Neu laden, sobald sich die angezeigte Uhrzeit aendert (ohne Sekunden einmal pro Minute; ohne Live-Uhrzeit
  // die Zeit der Zeiger), hoechstens einmal pro Sekunde - jeder Abruf haelt die Uhr kurz an.

  // Reload as soon as the displayed time changes (without seconds once a minute; without live time the time of
  // the hands), at most once per second - each fetch briefly stalls the clock.

  var stripKey = null, stripBusy = false, stripLastAt = 0;
  function syncStrip() {
    var d = new Date(), sec = +$('sSec').value, withSec = sec === 2 || (sec === 0 && !FD.showSec);
    var key = $('live').checked ? (withSec ? d.getMinutes() * 60 + d.getSeconds() : d.getMinutes()) : 'fix';
    if (key === stripKey || stripBusy || performance.now() - stripLastAt < 900) return;
    stripKey = key; stripBusy = true; stripLastAt = performance.now();
    setTimeout(function () { loadStripImg().then(function () { stripBusy = false; }); }, 200);
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
    var def = '/file?name=' + encodeURIComponent('/hand_set0_' + p + '.bmp');
    var url = '/file?name=' + encodeURIComponent('/hand_set' + (FD.hand.set || '0') + '_' + p + '.bmp');
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
    if (FD.strip && !document.hidden) syncStrip();
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

  var labels = { tName: 'name', tTools: 'tools', tBw: 'bw', tSym: 'sym', tTxtSize: 'txtSize',
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

  // Nabe: Aenderungen gehen sofort live an die Uhr (save=0), "Nabe speichern" legt sie dauerhaft ab

  // Hub: changes go live to the clock right away (save=0), "Save hub" stores them permanently

  var hubTimer = null;
  function hubSend(save) {
    var p = new URLSearchParams();
    p.set('size', FD.hub); p.set('color', FD.hubColor.slice(1)); p.set('save', save ? '1' : '0');
    return fetch('/setcenter', { method: 'POST', body: p }).then(function (r) { return r.json(); }).then(function (j) {
      if (!j.ok) throw new Error();
      if (save) showMsg(t('hubSaved'), true);
    }).catch(function () { showMsg(t('hubErr'), false); });
  }
  function hubLive() {
    var s = parseInt($('hubSize').value, 10);
    FD.hub = isNaN(s) ? 0 : Math.max(0, Math.min(100, s));
    FD.hubColor = $('hubColor').value;
    renderPreview();
    clearTimeout(hubTimer);
    hubTimer = setTimeout(function () { hubSend(false); }, 250);
  }
  $('hubSize').value = FD.hub; $('hubColor').value = FD.hubColor;
  ['hubSize', 'hubColor'].forEach(function (id) { $(id).addEventListener('input', hubLive); });
  $('hubSaveBtn').onclick = function () { clearTimeout(hubTimer); hubSend(true); };
  $('tHub').textContent = t('hub'); $('tHubSize').textContent = t('hubSize'); $('tHubColor').textContent = t('hubColor');
  $('hubHint').textContent = t('hubHint'); $('hubSaveBtn').textContent = t('hubSave');

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
    [0, 1, 6, 2, 3, 4, 5].forEach(function (i) { opt('sDFmt', String(i), t('df' + i)); });

    // Die drei Zeilen des Streifens: Kennbuchstabe der Element-IDs, Groesse in % (SP-Feld), VLW-Groesse in px
    // (SP-Feld, Vorgabe), Anzeige, Position und automatische Hoehe

    // The three lines of the strip: letter of the element IDs, size in % (SP field), VLW size in px (SP field,
    // default), show, position and automatic height

    var LINES = [
      { k: 'T', scale: 'ts', vlw: 'vt', vlwDef: 44, show: 'st', x: 'tx', y: 'ty', auto: 'aty', showDef: 1 },
      { k: 'D', scale: 'ds', vlw: 'vd', vlwDef: 22, show: 'sd', x: 'dx', y: 'dy', auto: 'ady', showDef: 1 },
      { k: 'W', scale: 'ws', vlw: 'vw', vlwDef: 22, show: 'sw', x: 'wx', y: 'wy', auto: 'awy', showDef: 0 }
    ];

    // Schluessel der erzeugten VLW-Dateien - der Wochentag zaehlt nur, wenn er angezeigt wird. Fehlt seine Datei
    // auf der Uhr (SP.vwok), wird beim ersten Senden neu erzeugt.

    // Key of the created VLW files - the weekday only counts if it is shown. If its file is missing on the clock
    // (SP.vwok), they are created again on the first send.

    var vlwKey = function (family, vt, vd, vw, sw, bold) { return family + '|' + vt + '|' + vd + '|' + (sw ? vw : '-') + '|' + bold; };

    // Designer-Schriften als VLW anbieten; die auf der Uhr aktive (SP.vlw, " B" = fett) wird wiedererkannt
    // Offer designer fonts as VLW; the one active on the clock (SP.vlw, " B" = bold) is recognised

    onFontAdded = function (i) {
      var f = fontFamilies[i];
      opt('sFont', 'v:' + i, f.label);
      if (SP.font === 255 && $('sFont').value === 'v:cur' && SP.vlw.replace(/ B$/, '') === f.label) {
        $('sFont').value = 'v:' + i;
        $('sVB').checked = / B$/.test(SP.vlw);
        lastVlwKey = (SP.sw && !SP.vwok) ? null : vlwKey(f.family, SP.vt, SP.vd, SP.vw, SP.sw, $('sVB').checked);
        vlwRows();
      }
    };

    // Groessen-Regler: eingebaute Schrift in % (sTS/sDS/sWS), Designer-Schrift (VLW) in px (sVT/sVD/sVW)
    // Size sliders: built-in font in % (sTS/sDS/sWS), designer font (VLW) in px (sVT/sVD/sVW)

    var sizeLabels = function () {
      var v = $('sFont').value.indexOf('v:') === 0;
      LINES.forEach(function (L) {
        $('s' + L.k + 'SVal').textContent = v ? $('sV' + L.k).value + ' px' : $('s' + L.k + 'S').value + ' %';
      });
    };
    var vlwRows = function () {
      var v = $('sFont').value.indexOf('v:') === 0, cur = $('sFont').value === 'v:cur';
      document.querySelectorAll('#stripCard .vlwRow').forEach(function (e) { e.style.display = v ? '' : 'none'; });
      document.querySelectorAll('#stripCard .fontScale').forEach(function (e) { e.style.display = v ? 'none' : ''; });
      LINES.forEach(function (L) { $('sV' + L.k).disabled = cur; });
      $('sVB').disabled = cur;
      sizeLabels();
    };

    // Dateiname wie stripVlwPath() in display.h: stripfont_<Name>_<Groesse>.vlw, Leerzeichen -> '-'; der
    // Wochentag mit "_wd" und den Buchstaben der Wochentage (deutsch und englisch)

    // File name like stripVlwPath() in display.h: stripfont_<name>_<size>.vlw, spaces -> '-'; the weekday with
    // "_wd" and the letters of the weekdays (German and English)

    var WDAY_CHARS = 'SonntagMontagDienstagMittwochDonnerstagFreitagSamstagSundayMondayTuesdayWednesdayThursdayFridaySaturday'
      .split('').filter(function (c, i, a) { return a.indexOf(c) === i; }).join('');
    var vlwFile = function (name, px, wd) {
      return 'stripfont_' + name.replace(/ /g, '-').replace(/[^A-Za-z0-9_-]/g, '') + '_' + px + (wd ? '_wd' : '') + '.vlw';
    };
    var uploadVlw = function (family, name, vt, vd, vw, sw, bold) {
      var css = function (px) { return (bold ? 'bold ' : '') + px + 'px ' + family; };
      var up = function (px, wd) {
        var fd = new FormData();
        fd.append('upload', makeVlw(css(px), px, wd ? WDAY_CHARS : ''), vlwFile(name, px, wd));
        return fetch('/upload', { method: 'POST', body: fd, redirect: 'manual' });
      };
      var loads = [document.fonts.load(css(vt), '0123456789'), document.fonts.load(css(vd), '0123456789')];
      if (sw) loads.push(document.fonts.load(css(vw), WDAY_CHARS));
      return Promise.all(loads).then(function () {
        return up(vt, false);
      }).then(function () { if (vd !== vt) return up(vd, false); }).then(function () { if (sw) return up(vw, true); });
    };
    fontFamilies.forEach(function (f, i) { onFontAdded(i); });
    LINES.forEach(function (L) { $('s' + L.k + 'X').max = SP.w; $('s' + L.k + 'Y').max = SP.h; });
    var stripSync = function () {
      LINES.forEach(function (L) {
        var auto = $('s' + L.k + 'Auto').checked;
        $('s' + L.k + 'X').disabled = $('s' + L.k + 'Y').disabled = auto;
        if (auto) { $('s' + L.k + 'X').value = SP.w >> 1; $('s' + L.k + 'Y').value = SP[L.auto]; }
      });
    };
    var stripFill = function () {
      $('sBefore').value = SP.before ? '1' : '0'; $('sBlink').checked = !!SP.blink; $('sBg').value = SP.bg; $('sFg').value = SP.fg;
      if (SP.font !== 255) $('sFont').value = String(SP.font);
      else if ($('sFont').value.indexOf('v:') !== 0) $('sFont').value = 'v:cur';
      $('sTFmt').value = String(SP.tfmt || 0); $('sSec').value = String(SP.sec || 0); $('sDFmt').value = String(SP.dfmt || 0);
      LINES.forEach(function (L) {
        $('sV' + L.k).value = SP[L.vlw] || L.vlwDef;
        $('s' + L.k + 'S').value = SP[L.scale] || 100;
        $('s' + L.k + 'Show').checked = SP[L.show] === undefined ? !!L.showDef : !!SP[L.show];
        $('s' + L.k + 'Auto').checked = !(SP[L.x] >= 0);
        $('s' + L.k + 'X').value = SP[L.x] >= 0 ? SP[L.x] : SP.w >> 1;
        $('s' + L.k + 'Y').value = SP[L.y] >= 0 ? SP[L.y] : SP[L.auto];
      });
      vlwRows();
      stripSync();
    };
    var stripSend = function (save, face) {
      var p = new URLSearchParams();
      if (face) p.set('face', face);
      SP.before = +$('sBefore').value; SP.bg = $('sBg').value;
      p.set('before', $('sBefore').value); p.set('blink', $('sBlink').checked ? '1' : '0'); p.set('bg', $('sBg').value); p.set('fg', $('sFg').value);
      p.set('tfmt', $('sTFmt').value); p.set('sec', $('sSec').value); p.set('dfmt', $('sDFmt').value);
      LINES.forEach(function (L) {
        var auto = $('s' + L.k + 'Auto').checked;
        SP[L.scale] = +$('s' + L.k + 'S').value;
        SP[L.show] = $('s' + L.k + 'Show').checked ? 1 : 0;
        p.set(L.scale, SP[L.scale]); p.set(L.show, SP[L.show]);
        p.set(L.x, auto ? -1 : $('s' + L.k + 'X').value); p.set(L.y, auto ? -1 : $('s' + L.k + 'Y').value);
      });

      // Designer-Schrift: VLW-Dateien nur neu erzeugen, wenn sich Schrift, Groesse oder Fett geaendert haben
      // Designer font: only create new VLW files if font, size or bold changed

      var fv = $('sFont').value, prep = Promise.resolve();
      if (fv.indexOf('v:') === 0) {
        p.set('font', 255);
        if (fv !== 'v:cur') {
          var ff = fontFamilies[+fv.slice(2)], bold = $('sVB').checked;
          var vt = +$('sVT').value || 44, vd = +$('sVD').value || 22, vw = +$('sVW').value || 22;
          var key = vlwKey(ff.family, vt, vd, vw, SP.sw, bold);
          p.set('vlw', ff.label + (bold ? ' B' : '')); p.set('vt', vt); p.set('vd', vd); p.set('vw', vw);
          SP.vlw = ff.label + (bold ? ' B' : ''); SP.vt = vt; SP.vd = vd; SP.vw = vw;
          if (key !== lastVlwKey) prep = uploadVlw(ff.family, SP.vlw, vt, vd, vw, SP.sw, bold).then(function () { lastVlwKey = key; });
        }
      }
      else p.set('font', fv);
      SP.font = fv.indexOf('v:') === 0 ? 255 : +fv;
      p.set('save', save ? '1' : '0');
      return prep.then(function () {
        return fetch('/save_strip', { method: 'POST', body: p });
      }, function (e) { showMsg(t('vlwErr'), false); throw e; }).then(function (r) { return r.json(); }).then(function (j) {
        if (!j.ok) throw new Error();
        SP.aty = j.aty; SP.ady = j.ady; SP.awy = j.awy;
        stripSync();
        loadStripImg();
        if (save && !face) showMsg(t('stripSaved'), true);
      }).catch(function () { showMsg(t('stripErr'), false); });
    };
    stripSaveFor = function (face) { clearTimeout(stripTimer); return stripSend(true, face); };
    var stripLive = function () {
      stripSync();
      clearTimeout(stripTimer);
      stripTimer = setTimeout(function () { stripSend(false); }, 250);
    };
    $('sFont').addEventListener('change', vlwRows);
    var liveIds = ['sBefore', 'sBlink', 'sBg', 'sFg', 'sFont', 'sVB', 'sTFmt', 'sSec', 'sDFmt'];
    LINES.forEach(function (L) { ['S', 'Show', 'Auto', 'X', 'Y'].forEach(function (s) { liveIds.push('s' + L.k + s); }); });
    liveIds.forEach(function (id) {
      $(id).addEventListener('input', stripLive);
      $(id).addEventListener('change', stripLive);
    });

    // VLW-Groesse erst beim Loslassen senden - jede Groesse ist eine eigene Datei auf der Uhr
    // Send the VLW size only on release - every size is a file of its own on the clock

    LINES.forEach(function (L) {
      $('s' + L.k + 'S').addEventListener('input', sizeLabels);
      $('sV' + L.k).addEventListener('input', sizeLabels);
      $('sV' + L.k).addEventListener('change', stripLive);
    });
    $('sBefore').addEventListener('change', function () { setStripBefore(this.value === '1'); });
    $('sDefBtn').onclick = function () {
      setStripBefore(false);
      SP.before = 0; SP.blink = 1; SP.bg = '#ffffff'; SP.fg = '#000000'; SP.font = Math.max(0, SP.fonts.indexOf('FreeSans Bold'));
      SP.tfmt = 0; SP.sec = 0; SP.dfmt = 0; SP.vt = 44; SP.vd = SP.vw = 22;
      LINES.forEach(function (L) { SP[L.scale] = 100; SP[L.show] = L.showDef; SP[L.x] = SP[L.y] = -1; });
      stripFill(); stripLive();
    };
    $('sSaveBtn').onclick = function () { clearTimeout(stripTimer); stripSend(true); };
    var stripLabels = { tStrip: 'strip', tSPos: 'sPos', tSBg: 'sBg', tSFg: 'sFg', tSFont: 'sFont', tSTime: 'sTime', tSDate: 'sDate',
      tSWday: 'sWday', stripHint: 'stripHint', tSTFmt: 'sTFmt', tSSec: 'sSec', tSDFmt: 'sDFmt' };
    Object.keys(stripLabels).forEach(function (id) { $(id).textContent = t(stripLabels[id]); });
    [['tSAuto', 'sAuto'], ['tSShow', 'sShow'], ['tSSize', 'sSize'], ['tSXY', 'sXY']].forEach(function (c) {
      document.querySelectorAll('#stripCard .' + c[0]).forEach(function (e) { e.textContent = t(c[1]); });
    });
    $('tSBlink').textContent = t('sBlink');
    $('sDefBtn').textContent = t('sDef'); $('sSaveBtn').textContent = t('sSave');
    stripFill();
  }

  // Start
  // Start

  selectTool('pen');
  setColor(color);
  loadActive();
})();
