// Gemeinsame Funktionen von Zifferblatt- und Zeiger-Designer, als /designer.js vor deren Skripten geladen
// (web/build_web.py). Nur Funktionen ohne Bezug auf Variablen eines Designers.

// Functions shared by the clock face and hand designer, loaded as /designer.js before their scripts
// (web/build_web.py). Only functions without reference to a designer's variables.

// Kurzform fuer document.getElementById()
// Short form for document.getElementById()

function $(id) { return document.getElementById(id); }

// RGB565 als [r, g, b], als #rrggbb und als CSS-Farbe
// RGB565 as [r, g, b], as #rrggbb and as CSS colour

function rgbOf(v) { return [((v >> 11) & 31) * 255 / 31 | 0, ((v >> 5) & 63) * 255 / 63 | 0, (v & 31) * 255 / 31 | 0]; }
function hexOf(v) { var c = rgbOf(v); return '#' + c.map(function (x) { return ('0' + x.toString(16)).slice(-2); }).join(''); }
function cssOf(v) { var c = rgbOf(v); return 'rgb(' + c[0] + ',' + c[1] + ',' + c[2] + ')'; }

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

// Knopf der Werkzeugleiste anlegen
// Create a toolbar button

function button(parent, label, onClick, cls) {
  var b = document.createElement('button');
  b.type = 'button'; b.className = cls || 'tb'; b.textContent = label; b.onclick = onClick;
  parent.appendChild(b); return b;
}

// BMP lesen (16/24/32 bpp, oben oder unten beginnend) - px(x, y) liefert RGB565
// Read a BMP (16/24/32 bpp, top-down or bottom-up) - px(x, y) returns RGB565

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

// Meldung im Designer: ok true = Erfolg, false = Fehler, ohne = neutral
// Message in the designer: ok true = success, false = error, without = neutral

function showMsg(text, ok) { $('msg').className = ok === undefined ? '' : (ok ? 'msg ok' : 'msg err'); $('msg').textContent = text; }

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
