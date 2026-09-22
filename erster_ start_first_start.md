*English version below.*

---

# Anleitung: Erste Inbetriebnahme, Zifferblätter/Zeiger hochladen, Uhren-Sets erstellen

Diese Anleitung beschreibt den Ersteinrichtungs-Assistenten Schritt für Schritt: WLAN-Einrichtung per WPS oder Accesspoint, das Hochladen eigener Zifferblätter und Zeigersätze sowie das Anlegen und Verwalten von Uhren-Sets (Presets).

---

## 1. Erste Inbetriebnahme – WLAN einrichten

Beim allerersten Start (noch kein WLAN gespeichert) versucht die Uhr automatisch zunächst **WPS**, bevor sie einen eigenen **Accesspoint** öffnet.

### 1.1 Schritt 1: WPS versuchen lassen

1. WLAN-Router griffbereit haben.
2. Uhr mit Strom versorgen. Auf dem Display erscheint **„check for WPS.."**.
3. Innerhalb der nächsten **2 Minuten** am Router die **WPS-Taste** drücken (Bezeichnung je nach Router z.B. „WPS", manchmal kombiniert mit der WLAN-Taste). Die Uhr zeigt währenddessen einen Countdown **„AP mode in …s"** an.
4. Findet die Uhr den Router per WPS, übernimmt sie SSID und Passwort automatisch, zeigt kurz den gefundenen Netzwerknamen und **„found WPS… reboot"** an und startet neu. Die Einrichtung ist damit abgeschlossen.

Reagiert der Router nicht (kein WPS, WPS nicht aktiviert oder Zeit abgelaufen), geht die Uhr automatisch in den Accesspoint-Modus über (Schritt 1.2) – es ist keine weitere Aktion nötig.

### 1.2 Schritt 2: Falls WPS nicht klappt – der Accesspoint-Modus (AP)

Die Uhr spannt jetzt ihr eigenes WLAN auf und zeigt auf dem Display **SSID und Passwort** an:

- **SSID:** `clock123` (bei jeder Uhr gleich)
- **Passwort:** ein 8-stelliger Code (Kleinbuchstaben/Ziffern), der **individuell pro Gerät** aus der MAC-Adresse gebildet wird und **nur auf dem Display dieser Uhr** angezeigt wird

> **Wichtig:** Das Passwort ist von Gerät zu Gerät unterschiedlich – es steht ausschließlich auf dem Display der jeweiligen Uhr. Es gibt kein festes, überall gleiches Passwort.

**Verbinden:**

1. Am Smartphone/PC mit dem WLAN `clock123` verbinden, Passwort wie auf dem Display eingeben.
2. In der Regel öffnet sich automatisch ein Browserfenster (Captive Portal). Passiert das nicht, im Browser manuell `http://192.168.4.1` aufrufen.
3. **Wichtig:** Nur **HTTP** verwenden, **HTTPS funktioniert nicht**.

### 1.3 Schritt 3: Heim-WLAN eintragen

1. Auf der geöffneten Seite (WLAN-Tab) das eigene Heim-WLAN auswählen bzw. SSID und Passwort eintragen.
2. Auf **„Speichern"** klicken.
3. Die Uhr zeigt eine Bestätigung, dass die Einstellungen gespeichert wurden, und **startet automatisch neu** – ein zusätzlicher Klick auf einen separaten „Reset"-Knopf ist dafür nicht nötig.
4. Nach dem Neustart verbindet sich die Uhr mit dem Heim-WLAN. Die neue IP-Adresse lässt sich am einfachsten über `http://<hostname>.local` erreichen (der Hostname steht z.B. im Router oder wird beim ersten Start automatisch aus der MAC-Adresse gebildet).

### 1.4 Später ein weiteres Netzwerk hinzufügen oder WLAN zurücksetzen

Ist die Uhr bereits mit dem Heim-WLAN verbunden, lässt sich jederzeit über die Weboberfläche (Tab **WLAN**) ein **weiteres** Netzwerk per WPS ergänzen, ohne das bisherige zu verlieren:

- Button **„Netzwerk per WPS hinzufügen"** klicken, danach am Router die WPS-Taste drücken. Die Verbindung der Uhr kann dabei für bis zu ca. 2 Minuten kurz unterbrochen sein.

Soll die Uhr komplett neu eingerichtet werden (z.B. Umzug, neuer Router), gibt es zwei Wege:

- **Über die Weboberfläche (empfohlen):** Seite **„Werksreset"** → Option **„gespeicherte WLAN-Netzwerke zurücksetzen"** auswählen. Bei Zugriff aus dem eigenen (privaten) Heimnetz – der normale Fall bei dieser Anleitung – wird sofort zurückgesetzt, ohne weitere Bestätigung. Nur bei Zugriff von außerhalb des Heimnetzes (z. B. über eine Port-Weiterleitung/DMZ) erscheint stattdessen groß und mittig ein 3-stelliger Bestätigungscode auf dem Display der Uhr (Form `_123_`), der zur Bestätigung auf der sich öffnenden Seite eingegeben werden muss (Schutz davor, dass diese Aktion aus der Ferne ohne physischen Zugriff auf die Uhr ausgelöst wird). Löscht nur die WLAN-Zugangsdaten, alle Zifferblätter/Zeigersätze/Presets bleiben erhalten. Die Uhr geht danach wieder in den WPS-/Accesspoint-Modus (siehe 1.1).
- **Über den Taster am Gerät (BUTTON bzw. eingebauter BOOT-Taster):** Kurz gedrückt zeigt er den Namen des aktuell verbundenen WLANs an. Wird er **länger als 10 Sekunden** gehalten, erscheint auf dem Display ein roter **„Factory Reset in N secs"**-Countdown – bis zu diesem Punkt passiert noch nichts, loslassen bricht harmlos ab. Wird er **länger als 15 Sekunden** durchgehend gehalten, löst das einen **vollständigen Werksreset** aus (WLAN **und** alle hochgeladenen Zifferblätter/Zeigersätze/Presets werden gelöscht).

  ⚠️ **Der Taster hat keine Zwischenstufe „nur WLAN löschen"** – für ein reines WLAN-Reset ohne Verlust der eigenen Zifferblätter/Zeigersätze/Presets bitte den Weg über die Weboberfläche (siehe oben) nutzen, sofern die Uhr noch erreichbar ist.

---

## 2. Eigene Zifferblätter hochladen

1. In der Navigation den Punkt **„Zifferblatt"** öffnen (Seite „Zifferblätter verwalten").
2. Anforderungen an die Datei:
   - Größe: **240 x 240 Pixel** (je nach Displaytyp ggf. 160 x 160 – die Seite zeigt die für dein Gerät gültige Größe in der Überschrift an)
   - Format: **16-Bit-BMP (RGB565)**
   - Dateiname muss mit **`face_`** beginnen, z.B. `face_meinmotiv.bmp`
3. Im Abschnitt **„neue Zifferblätter hochladen"** über **„Datei auswählen"** eine oder mehrere `.bmp`-Dateien wählen (Mehrfachauswahl möglich) und auf **„Hochladen BMP"** klicken.
4. Nach dem Upload erscheint das neue Zifferblatt in der Übersicht als Miniaturbild und kann dort per Klick als aktives Zifferblatt ausgewählt werden.

**Tipp:** Passende Zifferblätter (und Zeigersätze) lassen sich auch direkt als ZIP-Datei aus dem GitHub-Repository des Projekts herunterladen und anschließend über dasselbe Formular hochladen – der Link dazu steht oben auf derselben Seite.

---

## 3. Eigene Zeigersätze hochladen

1. In der Navigation den Punkt **„Zeiger"** öffnen (Seite „Zeigersatz Dateien verwalten").
2. Anforderungen an die Dateien:
   - Größe: **21 x 131 Pixel** (bzw. die für dein Gerät angezeigte Größe)
   - Format: **16-Bit-BMP (RGB565)**
   - Ein Zeigersatz besteht aus **drei** Dateien mit **derselben Nummer**, aber unterschiedlicher Endung:
     - `hand_set<N>_hour.bmp` (Stundenzeiger)
     - `hand_set<N>_minute.bmp` (Minutenzeiger)
     - `hand_set<N>_second.bmp` (Sekundenzeiger)
     - Beispiel für Satz 1: `hand_set1_hour.bmp`, `hand_set1_minute.bmp`, `hand_set1_second.bmp`
   - Der Drehpunkt (Pivot) des Zeigers liegt bei ca. der halben Breite / 77 % der Höhe des Bildes – beim Gestalten des Zeigerbilds darauf achten, damit sich der Zeiger sauber um den Mittelpunkt dreht.
3. Dateien über das Upload-Formular auswählen und hochladen.
4. Der neue Zeigersatz erscheint danach in der Übersicht und kann dort ausgewählt werden.

---

## 4. Uhren-Sets (Presets) erstellen und verwalten

Ein „Uhren-Set" (Preset) speichert die **aktuell aktive Kombination** aus Zifferblatt, Zeigersatz, Nabenfarbe/-größe und Sekundenzeiger-Anzeige unter einem Namen, um sie später mit einem Klick wieder abzurufen.

### 4.1 Ein neues Uhren-Set anlegen

1. Zuerst über die anderen Tabs/Seiten (Zifferblatt, Zeiger, Helligkeit-/Zifferblatt-Einstellungen) genau die Kombination einstellen, die gespeichert werden soll.
2. In der Navigation **„Uhren Sets"** öffnen.
3. Im Abschnitt **„Erstelle neues Set"** auf den Button **„Erzeuge ein Set aus den aktuellen Einstellungen"** klicken.
4. Das neue Set erscheint in der Liste (alphabetisch einsortiert) unter einem automatisch vergebenen Namen.
5. Über den jeweiligen Umbenennen-Link lässt sich der Name danach individuell anpassen.

Es sind bis zu **50 Uhren-Sets** möglich. Ist die Liste voll, erscheint beim Anlegen eines weiteren Sets ein Hinweis, dass zuerst ein bestehendes Set gelöscht werden muss.

### 4.2 Ein Uhren-Set aktivieren

In der Liste auf das Vorschaubild bzw. den Namen des gewünschten Sets klicken – die Uhr übernimmt sofort die gespeicherte Kombination.

### 4.3 Umbenennen und Löschen

Jedes Set in der Liste hat eigene Links/Buttons zum Umbenennen und Löschen (mit Sicherheitsabfrage vor dem Löschen).

### 4.4 Sichern und Wiederherstellen

Im Abschnitt **„Sicherung / Wiederherstellung"**:

- **„Presets in Datei speichern"** lädt alle Uhren-Sets als Datei herunter (Backup).
- Über das Datei-Auswahlfeld darunter lässt sich eine zuvor gesicherte Datei wieder hochladen, um die Sets wiederherzustellen bzw. zu ergänzen.

**Hinweis:** Ist die Liste beim allerersten Aufruf der Seite noch komplett leer, bietet die Uhr an, eine Reihe empfohlener Beispiel-Sets direkt von GitHub zu laden – dafür ist eine bestehende Internetverbindung nötig.

---

## Kurzübersicht

| Ziel | Wo |
|---|---|
| WLAN erstmalig einrichten | WPS abwarten → sonst AP `clock123` + Passwort vom Display → `http://192.168.4.1` |
| Weiteres WLAN ergänzen | Web-UI → Tab „WLAN" → „Netzwerk per WPS hinzufügen" |
| Nur WLAN zurücksetzen (Sets bleiben) | Web-UI → „Werksreset" → „gespeicherte WLAN-Netzwerke zurücksetzen" |
| Zifferblatt hochladen | Nav → „Zifferblatt" → `face_*.bmp`, 240×240, RGB565 |
| Zeigersatz hochladen | Nav → „Zeiger" → `hand_set<N>_hour/minute/second.bmp`, 21×131, RGB565 |
| Uhren-Set anlegen | Nav → „Uhren Sets" → „Erzeuge ein Set aus den aktuellen Einstellungen" |

---
---

# English Version

# Guide: First-Time Setup, Uploading Clock Faces/Hands, Creating Presets

This guide walks through the first-time setup wizard step by step: WiFi setup via WPS or access point, uploading custom clock faces and hand sets, and creating and managing presets.

---

## 1. First-Time Setup – Configuring WiFi

On the very first start (no WiFi saved yet), the clock automatically tries **WPS** first, before opening its own **access point**.

### 1.1 Step 1: Let it try WPS

1. Have your WiFi router within reach.
2. Power on the clock. The display shows **"check for WPS.."**.
3. Within the next **2 minutes**, press the **WPS button** on your router (labeled "WPS" on most routers, sometimes combined with the WiFi button). Meanwhile the clock shows a countdown, **"AP mode in …s"**.
4. If the clock finds the router via WPS, it takes over the SSID and password automatically, briefly shows the found network name and **"found WPS… reboot"**, then restarts. Setup is complete at this point.

If the router doesn't respond (no WPS, WPS not enabled, or time ran out), the clock automatically switches to access point mode (step 1.2) - no further action is needed.

### 1.2 Step 2: If WPS doesn't work – access point mode (AP)

The clock now opens its own WiFi network and shows **SSID and password** on its display:

- **SSID:** `clock123` (the same on every clock)
- **Password:** an 8-character code (lowercase letters/digits) generated **individually per device** from the MAC address and shown **only on this clock's display**

> **Important:** The password differs from device to device - it appears exclusively on that particular clock's display. There is no fixed password that's the same everywhere.

**Connecting:**

1. On your phone/PC, connect to the WiFi network `clock123`, entering the password shown on the display.
2. A browser window (captive portal) usually opens automatically. If it doesn't, open `http://192.168.4.1` manually in your browser.
3. **Important:** Use **HTTP** only - **HTTPS does not work**.

### 1.3 Step 3: Enter your home WiFi

1. On the page that opens (WLAN tab), select your home WiFi or enter its SSID and password.
2. Click **"Save"**.
3. The clock confirms the settings were saved and **restarts automatically** - no extra click on a separate "Reset" button is needed for this.
4. After restarting, the clock connects to your home WiFi. The easiest way to reach its new IP address is via `http://<hostname>.local` (the hostname can be found e.g. in your router, or is auto-generated from the MAC address on first boot).

### 1.4 Adding another network later, or resetting WiFi

Once the clock is connected to your home WiFi, you can add **another** network via WPS at any time through the web interface (tab **WLAN**), without losing the existing one:

- Click the **"Add Network via WPS"** button, then press the WPS button on your router. The clock's connection may be briefly interrupted for up to about 2 minutes while this happens.

If the clock needs to be set up from scratch (e.g. a move, a new router), there are two ways:

- **Via the web interface (recommended):** Go to the **"Factory Reset"** page → option **"reset saved WiFi networks"**. When accessed from your own (private) home network - the normal case for this guide - it resets immediately, with no further confirmation. Only when accessed from outside the home network (e.g. via a port forward/DMZ) does a 3-digit confirmation code appear instead, large and centered on the clock's display (shown as `_123_`), which must be entered on the page that opens to confirm (protects against this action being triggered remotely without physical access to the clock). This only erases the WiFi credentials; all clock faces/hand sets/presets are kept. The clock then goes back into WPS/access point mode (see 1.1).
- **Via the button on the device (BUTTON, or the built-in Boot button):** A brief press shows the name of the currently connected WiFi network. Holding it for **more than 10 seconds** shows a red **"Factory Reset in N secs"** countdown on the display - up to this point nothing happens yet, releasing it aborts harmlessly. Holding it continuously for **more than 15 seconds** triggers a **full factory reset** (WiFi **and** all uploaded clock faces/hand sets/presets are erased).

  ⚠️ **The button has no in-between "WiFi only" tier** - for a WiFi-only reset without losing your own clock faces/hand sets/presets, use the web interface path described above instead, provided the clock is still reachable.

---

## 2. Uploading Custom Clock Faces

1. Open **"Clock Face"** in the navigation (the "Manage Clock Face Files" page).
2. File requirements:
   - Size: **240 x 240 pixels** (possibly 160 x 160 depending on the display type - the page shows the size valid for your device in its heading)
   - Format: **16-bit BMP (RGB565)**
   - The filename must start with **`face_`**, e.g. `face_mydesign.bmp`
3. In the **"Upload New Clock Face"** section, choose one or more `.bmp` files (multiple selection is supported) and click **"Upload BMP"**.
4. After uploading, the new face appears in the overview as a thumbnail and can be selected there as the active clock face by clicking it.

**Tip:** Matching clock faces (and hand sets) can also be downloaded directly as a ZIP file from the project's GitHub repository and then uploaded via the same form - the link for this is at the top of the same page.

---

## 3. Uploading Custom Hand Sets

1. Open **"Hand Set"** in the navigation (the "Manage Hand Set Files" page).
2. File requirements:
   - Size: **21 x 131 pixels** (or the size shown for your device)
   - Format: **16-bit BMP (RGB565)**
   - A hand set consists of **three** files sharing the **same number**, but with different suffixes:
     - `hand_set<N>_hour.bmp` (hour hand)
     - `hand_set<N>_minute.bmp` (minute hand)
     - `hand_set<N>_second.bmp` (second hand)
     - Example for set 1: `hand_set1_hour.bmp`, `hand_set1_minute.bmp`, `hand_set1_second.bmp`
   - The hand's pivot point sits at roughly half the image width / 77% of its height - keep this in mind when designing the hand graphic, so it rotates cleanly around the center.
3. Select and upload the files via the upload form.
4. The new hand set then appears in the overview and can be selected there.

---

## 4. Creating and Managing Presets

A preset stores the **currently active combination** of clock face, hand set, hub color/size, and second-hand visibility under a name, so it can be recalled later with a single click.

### 4.1 Creating a new preset

1. First, set up exactly the combination you want to save using the other tabs/pages (Clock Face, Hand Set, Brightness/Clock Setup settings).
2. Open **"Presets"** in the navigation.
3. In the **"Create New Preset"** section, click the **"Create Preset from Current Settings"** button.
4. The new preset appears in the list (sorted alphabetically) under an automatically assigned name.
5. Its name can then be customized via the corresponding rename link.

Up to **50 presets** are possible. Once the list is full, creating another preset shows a notice that an existing one must be deleted first.

### 4.2 Activating a preset

Click the thumbnail or name of the desired preset in the list - the clock immediately applies the saved combination.

### 4.3 Renaming and deleting

Each preset in the list has its own links/buttons for renaming and deleting (with a confirmation prompt before deletion).

### 4.4 Backup and restore

In the **"Backup / Restore Presets"** section:

- **"Save Presets to File"** downloads all presets as a file (backup).
- The file picker below it lets you upload a previously saved file again, to restore or add to the presets.

**Note:** If the list is still completely empty the very first time the page is opened, the clock offers to load a set of recommended sample presets directly from GitHub - this requires an existing internet connection.

---

## Quick Reference

| Goal | Where |
|---|---|
| Set up WiFi for the first time | Wait for WPS → otherwise AP `clock123` + password from the display → `http://192.168.4.1` |
| Add another WiFi network | Web UI → "WLAN" tab → "Add Network via WPS" |
| Reset WiFi only (presets kept) | Web UI → "Factory Reset" → "reset saved WiFi networks" |
| Upload a clock face | Nav → "Clock Face" → `face_*.bmp`, 240×240, RGB565 |
| Upload a hand set | Nav → "Hand Set" → `hand_set<N>_hour/minute/second.bmp`, 21×131, RGB565 |
| Create a preset | Nav → "Presets" → "Create Preset from Current Settings" |
