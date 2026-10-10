# Flashen und Einrichten der Uhr per USB, Aufruf aus flashESP.bat.
#   -Flash [-Port <n>] [-Display <n>]
#        Ohne -Port (Doppelklick auf flashESP.bat): sucht zuerst die Uhr
#        (port.ps1 -NoSwitch), fragt eine laufende Uhr nach ihrem Displaytyp,
#        fragt Displaytyp und WLAN ab, bringt die Uhr dann in den Download-Modus,
#        flasht sie (esptool) und sendet danach die Einstellungen und die
#        Uhrzeit des PCs ueber USB. Danach bleibt ein serieller Monitor offen und
#        zeigt die Ausgabe der Uhr (Beenden mit Q oder Esc; -NoMonitor = ohne).
#        Mit -Port ohne Rueckfragen, das WLAN wird dann nicht abgefragt.
#   -Send <n>
#        Nur senden (ohne Flashen): Displaytyp an eine laufende Uhr.
#   -Time [-Port <n>]
#        Nur die Uhrzeit des PCs an eine laufende Uhr senden (setTime.bat).
# Gesendet wird "UHR4 WIFI <Name-Hex> <Passwort-Hex>", "UHR4 DISPLAY <Name>",
# bei Bedarf "UHR4 RESTART", danach "UHR4 TIME <Unix-Sekunden>" (siehe
# handleSerialCommands() in display.h).
# Displaytyp ESP32-S2: 1/GC9A01, 2/GC9A01_WITH_BACKLIGHT, 3/GC9D01, 4/ILI9341;
# ESP32-C6 (Waveshare): 1/ST7789 (1,47"), 2/ST7789_240 (1,3"); ESP32-S3
# (Waveshare ESP32-S3-LCD-1.28): fest GC9A01, keine Abfrage; 0 = unveraendert.
# Das Board erkennt das Skript am USB-Port (303A:1001 = ESP32-C6, 1A86:55D3 =
# ESP32-S3 am CH343P) und LAEDT den Build dieses Chips vom GitHub-Release (uhr4-esp32s2,
# -esp32c6 bzw. -esp32s3 als Zip samt SHA-256, Pruefung vor dem Flashen). Dafuer braucht
# der PC beim Flashen Internet; einen Rueckfall auf lokale Dateien gibt es nicht.
#   -Local
#        Nur fuer die Entwicklung: statt zu laden den Build aus dem Unterordner esp32s2,
#        esp32c6 bzw. esp32s3 neben diesem Skript flashen (auch UHR4_LOCAL=1).
#        Die Umgebungsvariable UHR4_BUILD_URL ersetzt die Adresse des Releases (zum Testen).
# Das WLAN-Passwort wird verdeckt eingegeben, bleibt nur im Speicher dieses
# Skripts (keine Datei, keine Umgebungsvariable) und geht nur per USB an die Uhr.
#
# Flashing and setting up the clock via USB, called from flashESP.bat.
#   -Flash [-Port <n>] [-Display <n>]
#        Without -Port (double-click on flashESP.bat): first finds the clock
#        (port.ps1 -NoSwitch), asks a running clock for its display type, asks
#        for display type and WiFi, then switches the clock into download mode,
#        flashes it (esptool) and afterwards sends the settings and the PC's
#        time via USB. With -Port without questions, the WiFi is not asked
#        for then.
#   -Send <n>
#        Send only (without flashing): display type to a running clock.
#   -Time [-Port <n>]
#        Only send the PC's time to a running clock (setTime.bat).
#   -NoMonitor
#        With -Flash: no serial monitor afterwards (it stays open otherwise).
# Sent are "UHR4 WIFI <name hex> <password hex>", "UHR4 DISPLAY <name>",
# "UHR4 RESTART" if needed, then "UHR4 TIME <unix seconds>" (see
# handleSerialCommands() in display.h).
# Display type ESP32-S2: 1/GC9A01, 2/GC9A01_WITH_BACKLIGHT, 3/GC9D01, 4/ILI9341;
# ESP32-C6 (Waveshare): 1/ST7789 (1.47"), 2/ST7789_240 (1.3"); ESP32-S3
# (Waveshare ESP32-S3-LCD-1.28): fixed GC9A01, no question; 0 = unchanged.
# The script recognizes the board by the USB port (303A:1001 = ESP32-C6,
# 1A86:55D3 = ESP32-S3 on the CH343P) and DOWNLOADS the build of that chip from the GitHub
# release (uhr4-esp32s2, -esp32c6 or -esp32s3 as a zip with SHA-256, checked before
# flashing). The PC needs internet while flashing; there is no fallback to local files.
#   -Local
#        For development only: instead of downloading, flash the build from the subfolder
#        esp32s2, esp32c6 or esp32s3 next to this script (also UHR4_LOCAL=1).
#        The environment variable UHR4_BUILD_URL replaces the release address (for testing).
# The WiFi password is entered hidden, stays only in this script's memory (no
# file, no environment variable) and only goes to the clock via USB.
param([switch]$Flash, [string]$Port, [string]$Display, [string]$Send, [switch]$Time, [switch]$NoMonitor, [switch]$Local)

# Board und Displaytypen: 's2' (Lolin S2 Pico, wechselbares Display), 'c6' (Waveshare ESP32-C6-LCD mit fest
# verbautem ST7789) oder 's3' (Waveshare ESP32-S3-LCD-1.28 mit fest verbautem GC9A01). Set-Board stellt Namen
# und Beschriftungen um.
# Board and display types: 's2' (Lolin S2 Pico, exchangeable display), 'c6' (Waveshare ESP32-C6-LCD with a
# built-in ST7789) or 's3' (Waveshare ESP32-S3-LCD-1.28 with a built-in GC9A01). Set-Board switches names and
# labels.
$board = 's2'
$names = @{}
$labels = @{}
function Set-Board([string]$b) {
    $script:board = $b
    if ($b -eq 'c6') {
        $script:names = @{ 1 = 'ST7789'; 2 = 'ST7789_240' }
        $script:labels = @{ 1 = 'ST7789 (172x320, Waveshare ESP32-C6-LCD-1.47) mit Streifen fuer Uhrzeit und Datum / with time and date strip'; 2 = 'ST7789 (240x240, Waveshare ESP32-C6-LCD-1.3)' }
    } elseif ($b -eq 's3') {
        $script:names = @{ 1 = 'GC9A01'; 2 = 'GC9A01_WITH_BACKLIGHT' }
        $script:labels = @{ 1 = 'GC9A01 (240x240) ohne Helligkeitsregelung ueber die Beleuchtung / without backlight dimming'; 2 = 'GC9A01 (240x240) mit Helligkeitsregelung ueber die Beleuchtung (Pin 40) / with backlight dimming (pin 40)' }
    } else {
        $script:names = @{ 1 = 'GC9A01'; 2 = 'GC9A01_WITH_BACKLIGHT'; 3 = 'GC9D01'; 4 = 'ILI9341' }
        $script:labels = @{ 1 = 'GC9A01 (240x240) ohne Hintergrundbeleuchtung (BL) / without backlight (BL)'; 2 = 'GC9A01 (240x240) mit Hintergrundbeleuchtung (BL) an Pin 3 / with backlight (BL) on pin 3'; 3 = 'GC9D01 (160x160)'; 4 = 'ILI9341 (240x320) mit Streifen fuer Uhrzeit und Datum / with time and date strip' }
    }
}
Set-Board 's2'

# Release, aus dem die Builds geladen werden: fest zur Version dieses Tools (die seriellen Befehle muessen zur Firmware
# passen). UHR4_BUILD_URL ersetzt die Adresse, UHR4_LOCAL=1 bzw. -Local nimmt die Ordner neben dem Skript.
# Release the builds are downloaded from: fixed to the version of this tool (the serial commands must match the firmware).
# UHR4_BUILD_URL replaces the address, UHR4_LOCAL=1 or -Local uses the folders next to the script.
$releaseTag = 'v4'
$buildUrl = if ($env:UHR4_BUILD_URL) { $env:UHR4_BUILD_URL.TrimEnd('/') } else { "https://github.com/holgiw/ESP32-Station-Clock/releases/download/$releaseTag" }
$useLocal = $Local -or ($env:UHR4_LOCAL -eq '1')

# Ordner mit den Flash-Dateien des Boards: lokal (nur Entwicklung) oder aus dem Release geladen, per SHA-256 geprueft
# und entpackt. Bei einem Fehler endet das Skript - kein Rueckfall auf lokale Dateien.
# Folder with the board's flash files: local (development only) or downloaded from the release, checked by SHA-256
# and unpacked. On an error the script ends - no fallback to local files.
function Get-BuildDir([string]$b) {
    if ($useLocal) { return Join-Path $PSScriptRoot "esp32$b" }
    $name = "uhr4-esp32$b"
    $work = Join-Path ([IO.Path]::GetTempPath()) "uhr4-flash\$name"
    if (Test-Path $work) { Remove-Item $work -Recurse -Force -ErrorAction SilentlyContinue }
    New-Item -ItemType Directory -Path $work -Force | Out-Null
    $zip = Join-Path $work "$name.zip"
    Write-Host "Build wird geladen / downloading the build: $buildUrl/$name.zip"
    try {
        [Net.ServicePointManager]::SecurityProtocol = [Net.ServicePointManager]::SecurityProtocol -bor [Net.SecurityProtocolType]::Tls12
        $wc = New-Object System.Net.WebClient
        $wc.Headers.Add('User-Agent', 'uhr4-flash')
        $wc.Proxy = [Net.WebRequest]::GetSystemWebProxy()
        $wc.Proxy.Credentials = [Net.CredentialCache]::DefaultCredentials
        $shaText = $wc.DownloadString("$buildUrl/$name.sha256")
        $wc.DownloadFile("$buildUrl/$name.zip", $zip)
    }
    catch {
        Write-Host "Download fehlgeschlagen / download failed: $($_.Exception.Message)"
        Write-Host 'Internetverbindung des PCs pruefen (GitHub erreichbar?), dann flashESP.bat erneut starten.'
        Write-Host 'Check the PC internet connection (is GitHub reachable?), then run flashESP.bat again.'
        exit 1
    }
    $want = ($shaText.Trim() -split '\s+')[0].ToLower()
    $have = (Get-FileHash -Algorithm SHA256 $zip).Hash.ToLower()
    if ($want -ne $have) {
        Write-Host 'Pruefsumme der geladenen Datei stimmt nicht - nichts wird geflasht. Spaeter erneut versuchen.'
        Write-Host 'Checksum of the downloaded file does not match - nothing is flashed. Try again later.'
        exit 1
    }
    Expand-Archive -Path $zip -DestinationPath (Join-Path $work 'files') -Force
    return (Join-Path $work 'files')
}

# Board am COM-Port: USB-Serial-JTAG (303A:1001) = ESP32-C6, CH343P (1A86:55D3) = ESP32-S3, sonst ESP32-S2
# Board on the COM port: USB serial JTAG (303A:1001) = ESP32-C6, CH343P (1A86:55D3) = ESP32-S3, else ESP32-S2
function Get-PortBoard([int]$num) {
    $dev = Get-CimInstance Win32_PnPEntity | Where-Object { $_.Name -match "\(COM$num\)" } | Select-Object -First 1
    if ($dev -and $dev.DeviceID -match 'VID_303A&PID_1001') { return 'c6' }
    if ($dev -and $dev.DeviceID -match 'VID_1A86&PID_55D3') { return 's3' }
    return 's2'
}

# Serielle Schnittstelle zur laufenden Uhr. ESP32-S2 (USB-CDC): DTR und RTS an - erst mit DTR sendet die
# Uhr ihre Antworten. ESP32-C6 (USB-Serial-JTAG) und ESP32-S3 (CH343P mit Reset-Schaltung an DTR/RTS):
# beide aus - ein Umschalten setzt den Chip sonst zurueck.
# Serial port to the running clock. ESP32-S2 (USB CDC): DTR and RTS on - the clock only sends its replies
# with DTR. ESP32-C6 (USB serial JTAG) and ESP32-S3 (CH343P with a reset circuit on DTR/RTS): both off -
# switching them would otherwise reset the chip.
function Open-ClockPort([int]$num) {
    $sp = New-Object System.IO.Ports.SerialPort ("COM$num", 115200)
    $sp.DtrEnable = ($board -eq 's2')
    $sp.RtsEnable = ($board -eq 's2')
    $sp.ReadTimeout = 500

    # Zeitlimit auch beim Schreiben - nimmt die Uhr nichts ab (z.B. Firmware ohne USB-Seriell), haengt
    # Write() sonst endlos
    # Timeout for writing too - if the clock accepts nothing (e.g. firmware without USB serial), Write()
    # would otherwise hang forever

    $sp.WriteTimeout = 1000
    $sp.NewLine = "`n"
    $sp.Open()
    return $sp
}

# Auswahl als Zahl oder Name, sonst 0 / choice as number or name, else 0
function Get-Choice([string]$value) {
    $v = $value.Trim().ToUpper()
    if ($v -match '^\d+$' -and $names.ContainsKey([int]$v)) { return [int]$v }
    foreach ($k in $names.Keys) { if ($names[$k] -eq $v) { return $k } }
    return 0
}

# Displaytyp abfragen. $info = Antwort der Uhr auf "UHR4 INFO" (oder $null):
#   Typ gespeichert -> vorgewaehlt, Enter behaelt ihn
#   uhr4 ohne gespeicherten Typ -> Angabe noetig (sonst liefe sie als GC9A01)
#   keine Antwort (neue Uhr, uhr3, aeltere uhr4-Firmware) -> Enter = unveraendert
# Asks for the display type. $info = the clock's reply to "UHR4 INFO" (or $null):
#   type stored -> preselected, Enter keeps it
#   uhr4 without stored type -> answer required (it would run as GC9A01)
#   no reply (new clock, uhr3, older uhr4 firmware) -> Enter = unchanged
function Read-DisplayChoice($info) {
    $current = if ($info) { Get-Choice $info.Name } else { 0 }
    Write-Host ''
    Write-Host 'Welches Display hat die Uhr? / Which display does the clock have?'
    foreach ($k in ($names.Keys | Sort-Object)) {
        $mark = if ($info -and $info.Set -and $k -eq $current) { '   <- eingestellt / configured' } else { '' }
        Write-Host ("  {0} = {1}{2}" -f $k, $labels[$k], $mark)
    }
    if ($info -and $info.Set) {
        Write-Host "  Enter = $($info.Name) beibehalten / keep $($info.Name)"
    }
    elseif ($info) {
        Write-Host '  Die Uhr hat noch keinen Displaytyp eingestellt - bitte angeben.'
        Write-Host '  The clock has no display type configured yet - please choose one.'
    }
    else {
        Write-Host '  Displaytyp der Uhr nicht feststellbar (neue Uhr, uhr3 oder aeltere Firmware).'
        Write-Host '  Enter = unveraendert lassen - bei einer neuen Uhr bitte angeben.'
        Write-Host '  Display type of the clock not detectable (new clock, uhr3 or older firmware).'
        Write-Host '  Enter = keep unchanged - please choose one for a new clock.'
    }
    while ($true) {
        $answer = Read-Host 'Auswahl / choice'
        if (-not $answer) {
            if ($info -and -not $info.Set) { continue }  # Angabe noetig / answer required
            return 0
        }
        $choice = Get-Choice $answer
        if ($choice -gt 0) {
            # Gleicher Typ wie eingestellt: nichts senden / same as configured: send nothing
            if ($info -and $info.Set -and $choice -eq $current) { return 0 }
            return $choice
        }
        $list = ($names.Keys | Sort-Object) -join ', '
        Write-Host "Bitte eine dieser Nummern eingeben / please enter one of these numbers: $list"
    }
}

function ConvertTo-Hex([string]$text) {
    -join ([System.Text.Encoding]::UTF8.GetBytes($text) | ForEach-Object { $_.ToString('x2') })
}

# Frische WLAN-Suche anstossen: netsh zeigt sonst nur das Ergebnis der letzten
# Suche von Windows (die laeuft z.B. erst beim Oeffnen des WLAN-Menues).
# WlanScan() aus wlanapi.dll, ohne Administratorrechte.
# Trigger a fresh WiFi scan: netsh otherwise only shows the result of Windows'
# last scan (which runs e.g. only when the WiFi menu is opened).
# WlanScan() from wlanapi.dll, without administrator rights.
function Start-WifiScan {
    try {
        Add-Type -TypeDefinition @"
using System;
using System.Runtime.InteropServices;
public static class UhrWlanScan {
    [DllImport("wlanapi.dll")] static extern int WlanOpenHandle(uint clientVersion, IntPtr reserved, out uint negotiated, out IntPtr handle);
    [DllImport("wlanapi.dll")] static extern int WlanCloseHandle(IntPtr handle, IntPtr reserved);
    [DllImport("wlanapi.dll")] static extern int WlanEnumInterfaces(IntPtr handle, IntPtr reserved, out IntPtr list);
    [DllImport("wlanapi.dll")] static extern int WlanScan(IntPtr handle, ref Guid iface, IntPtr ssid, IntPtr ie, IntPtr reserved);
    [DllImport("wlanapi.dll")] static extern void WlanFreeMemory(IntPtr memory);
    public static int Scan() {
        uint neg; IntPtr h;
        if (WlanOpenHandle(2, IntPtr.Zero, out neg, out h) != 0) return 0;
        int started = 0;
        IntPtr list;
        if (WlanEnumInterfaces(h, IntPtr.Zero, out list) == 0) {
            // WLAN_INTERFACE_INFO_LIST: 2x DWORD, dann je Eintrag GUID + WCHAR[256] + State = 532 Byte
            int count = Marshal.ReadInt32(list);
            for (int i = 0; i < count; i++) {
                IntPtr entry = new IntPtr(list.ToInt64() + 8 + i * 532);
                Guid g = (Guid)Marshal.PtrToStructure(entry, typeof(Guid));
                if (WlanScan(h, ref g, IntPtr.Zero, IntPtr.Zero, IntPtr.Zero) == 0) started++;
            }
            WlanFreeMemory(list);
        }
        WlanCloseHandle(h, IntPtr.Zero);
        return started;
    }
}
"@
        if ([UhrWlanScan]::Scan() -gt 0) {
            Write-Host 'Suche WLANs ... / scanning for WiFi networks ...'
            Start-Sleep -Seconds 4
        }
    } catch {}
}

# Sichtbare 2,4-GHz-WLANs (Kanal 1-14 bei mindestens einem Zugangspunkt), nach
# Signalstaerke sortiert. netsh-Beschriftung deutsch ("Kanal") oder englisch
# ("Channel"); "SSID"/"Signal" heissen in beiden Sprachen gleich.
# Visible 2.4 GHz WiFis (channel 1-14 on at least one access point), sorted
# by signal strength. netsh labels German ("Kanal") or English ("Channel");
# "SSID"/"Signal" are the same in both languages.
function Get-WifiNetworks {
    $nets = @{}
    $ssid = $null
    $signal = 0
    try {
        foreach ($l in (netsh wlan show networks mode=bssid 2>$null)) {
            if ($l -match '^SSID \d+\s*:\s*(.*)$') {
                $ssid = $Matches[1].Trim()
                if ($ssid -and -not $nets.ContainsKey($ssid)) { $nets[$ssid] = -1 }
            }
            elseif ($ssid -and $l -match '^\s+Signal\s*:\s*(\d+)') { $signal = [int]$Matches[1] }
            elseif ($ssid -and $l -match '^\s+(Kanal|Channel)\s*:\s*(\d+)\s*$') {
                if ([int]$Matches[2] -le 14 -and $signal -gt $nets[$ssid]) { $nets[$ssid] = $signal }
            }
        }
    } catch {}
    return @($nets.Keys | Where-Object { $nets[$_] -ge 0 } | Sort-Object { -$nets[$_] })
}
function Get-CurrentWifi {
    try {
        foreach ($l in (netsh wlan show interfaces 2>$null)) {
            if ($l -match '^\s+SSID\s*:\s*(.+)$') { return $Matches[1].Trim() }
        }
    } catch {}
    return ''
}

# Gespeichertes WLAN-Passwort des PCs fuer ein Netz (netsh ... key=clear, ohne
# Administratorrechte fuer eigene Profile), leer wenn keins/nicht lesbar.
# Stored WiFi password of the PC for a network (netsh ... key=clear, without
# administrator rights for own profiles), empty if none/not readable.
function Get-WifiProfileKey([string]$ssid) {
    try {
        foreach ($l in (netsh wlan show profile "name=$ssid" key=clear 2>$null)) {
            if ($l -match '^\s*(Schl.sselinhalt|Key Content)\s*:\s*(.*)$') { return $Matches[2].Trim() }
        }
    } catch {}
    return ''
}

# Fragt das WLAN fuer die Uhr ab, Ergebnis in $script:wifiSsid/$script:wifiPass
# Asks for the WiFi for the clock, result in $script:wifiSsid/$script:wifiPass
$wifiSsid = ''
$wifiPass = ''
function Read-Wifi {
    Write-Host ''
    Write-Host 'WLAN fuer die Uhr einrichten? Die Uhr kann nur 2,4-GHz-Netze, nur diese werden gezeigt.'
    Write-Host 'Set up WiFi for the clock? The clock only supports 2.4 GHz networks, only those are shown.'
    Start-WifiScan
    $nets = @(Get-WifiNetworks)
    $current = Get-CurrentWifi
    for ($i = 0; $i -lt $nets.Count; $i++) {
        $mark = if ($nets[$i] -eq $current) { '  (PC ist verbunden / PC connected)' } else { '' }
        Write-Host ('  {0} = {1}{2}' -f ($i + 1), $nets[$i], $mark)
    }

    # PC haengt an einem 2,4-GHz-Netz: SSID und gespeichertes Passwort anbieten
    # PC is on a 2.4 GHz network: offer its SSID and stored password
    $answer = ''
    if ($current -and ($nets -contains $current)) {
        $take = Read-Host "Der PC ist mit '$current' (2,4 GHz) verbunden - WLAN-Name und Passwort uebernehmen? / PC is connected to '$current' (2.4 GHz) - take over WiFi name and password? [J/n, Y/n]"
        if (-not $take -or $take -match '^[JjYy]') {
            $key = Get-WifiProfileKey $current
            if ($key -and $key.Length -ge 8 -and $key.Length -le 63) {
                $script:wifiSsid = $current
                $script:wifiPass = $key
                Write-Host "WLAN '$current' mit dem gespeicherten Passwort des PCs uebernommen / WiFi '$current' taken over with the PC's stored password."
                return
            }
            Write-Host 'Kein gespeichertes Passwort lesbar - bitte eingeben / no stored password readable - please enter it.'
            $answer = $current
        }
    }

    if (-not $answer) {
        if ($nets.Count -eq 0) {
            # Keine Liste (kein WLAN-Adapter am PC, WLAN aus oder keine Rechte): Name eintippen
            # No list (no WiFi adapter on the PC, WiFi off or no permission): type the name
            Write-Host '  Keine WLAN-Liste verfuegbar - WLAN-Name bitte eintippen oder Enter = ueberspringen'
            Write-Host '  No WiFi list available - please type the WiFi name or Enter = skip'
        } else {
            Write-Host '  Nummer, WLAN-Name eingeben oder Enter = ueberspringen (Einrichtung spaeter per WPS/Access Point)'
            Write-Host '  number, WiFi name or Enter = skip (set up later via WPS/access point)'
        }
        $answer = Read-Host 'WLAN / WiFi'
    }
    if (-not $answer) { return }

    $ssid = if ($answer -match '^\d+$' -and [int]$answer -ge 1 -and [int]$answer -le $nets.Count) { $nets[[int]$answer - 1] } else { $answer }
    while ($true) {
        $p1 = Read-Host "Passwort fuer / password for '$ssid' (Enter = offenes Netz / open network)" -AsSecureString
        $p2 = Read-Host 'Passwort wiederholen / repeat password' -AsSecureString
        $t1 = [Runtime.InteropServices.Marshal]::PtrToStringUni([Runtime.InteropServices.Marshal]::SecureStringToBSTR($p1))
        $t2 = [Runtime.InteropServices.Marshal]::PtrToStringUni([Runtime.InteropServices.Marshal]::SecureStringToBSTR($p2))
        if ($t1 -ne $t2) { Write-Host 'Die Passwoerter stimmen nicht ueberein / passwords do not match'; continue }
        if ($t1.Length -gt 0 -and ($t1.Length -lt 8 -or $t1.Length -gt 63)) { Write-Host 'WLAN-Passwoerter haben 8-63 Zeichen / WiFi passwords have 8-63 characters'; continue }
        $script:wifiSsid = $ssid
        $script:wifiPass = $t1
        return
    }
}

# Laufende Uhr des eingestellten Boards: ESP32-S2 mit Arduino-USB-CDC (nicht der Download-Port 303A:0002),
# ESP32-C6 am USB-Serial-JTAG (303A:1001), ESP32-S3 am CH343P (1A86:55D3)
# Running clock of the selected board: ESP32-S2 with Arduino USB CDC (not the download port 303A:0002),
# ESP32-C6 on the USB serial JTAG (303A:1001), ESP32-S3 on the CH343P (1A86:55D3)
function Get-RunningClockPort {
    Get-CimInstance Win32_PnPEntity | Where-Object {
        $_.Name -match '\(COM(\d+)\)' -and
        $(if ($board -eq 's3') { $_.DeviceID -match 'VID_1A86&PID_55D3' }
          elseif ($board -eq 'c6') { $_.DeviceID -match 'VID_303A&PID_1001' }
          else { $_.DeviceID -match 'VID_303A' -and $_.DeviceID -notmatch 'PID_0002|PID_1001' })
    } | ForEach-Object { [int]([regex]::Match($_.Name, '\(COM(\d+)\)').Groups[1].Value) } | Select-Object -First 1
}

# Sendet einen Befehl und wartet bis 3 s auf "UHR4 OK ..." bzw. "UHR4 ERROR ..."
# Sends a command and waits up to 3 s for "UHR4 OK ..." or "UHR4 ERROR ..."
function Send-Command($sp, [string]$cmd) {
    $sp.Write("$cmd`n")
    $until = (Get-Date).AddSeconds(3)
    while ((Get-Date) -lt $until) {
        try { $line = $sp.ReadLine() } catch { continue }
        if ($line -match '(UHR4 (OK|ERROR).*)') { return $Matches[1].Trim() }
    }
    return ''
}


# Fragt eine laufende Uhr VOR dem Flashen nach ihrem Displaytyp ("UHR4 INFO",
# siehe handleSerialInfo() in display.h). $null, wenn keine laufende Uhr da ist
# oder sie nicht antwortet (uhr3, aeltere uhr4-Firmware).
# Asks a running clock BEFORE flashing for its display type ("UHR4 INFO", see
# handleSerialInfo() in display.h). $null if there is no running clock or it
# does not reply (uhr3, older uhr4 firmware).
function Get-ClockInfo([int]$num = 0) {
    if (-not $num) { $num = Get-RunningClockPort }
    if (-not $num) { return $null }
    Write-Host "Frage die Uhr auf COM$num nach ihrem Displaytyp ... / asking the clock on COM$num for its display type ..."
    $sp = $null
    try {
        $sp = Open-ClockPort $num
        $reply = Send-Command $sp 'UHR4 INFO'
        $sp.Close()
        if ($reply -match '^UHR4 OK INFO (\S+) ([01])') {
            return [pscustomobject]@{ Name = $Matches[1]; Set = ($Matches[2] -eq '1') }
        }
    } catch {
    } finally {
        if ($sp -and $sp.IsOpen) { try { $sp.Close() } catch {} }
    }
    return $null
}

# Sendet die Uhrzeit des PCs ("UHR4 TIME <Unix-Sekunden UTC>.<ms>", erst
# direkt vor dem Senden bestimmt) an die laufende Uhr - auf COM$num oder die
# gefundene. Wartet bis $waitSec Sekunden auf die Uhr (z.B. waehrend sie nach
# dem Flashen startet). $true bei Erfolg.
# Sends the PC's time ("UHR4 TIME <unix seconds UTC>.<ms>", determined only
# right before sending) to the running clock - on COM$num or the one found.
# Waits up to $waitSec seconds for the clock (e.g. while it starts after
# flashing). $true on success.
function Send-ClockTime([int]$num = 0, [int]$waitSec = 60) {
    $start = Get-Date
    $portSeen = $null
    $hinted = $false
    $busyPort = $null
    while (((Get-Date) - $start).TotalSeconds -lt $waitSec) {
        $n = if ($num) { $num } else { Get-RunningClockPort }
        if (-not $n) {
            if (-not $hinted -and ((Get-Date) - $start).TotalSeconds -gt 20) {
                Write-Host 'Keine laufende Uhr gefunden - startet sie nicht von selbst? Einmal die Reset-Taste druecken.'
                Write-Host 'No running clock found - does it not start by itself? Press the Reset button once.'
                $hinted = $true
            }
            Start-Sleep -Milliseconds 500
            continue
        }
        # Uhr da, antwortet aber 20 s lang nicht: Firmware ohne "UHR4 TIME"
        # Clock present but no reply for 20 s: firmware without "UHR4 TIME"
        if (-not $portSeen) { $portSeen = Get-Date }
        elseif (-not $busyPort -and ((Get-Date) - $portSeen).TotalSeconds -gt 20) { break }

        $sp = $null
        try {
            # DTR/RTS je nach Board - siehe Open-ClockPort
            # DTR/RTS depending on the board - see Open-ClockPort
            $sp = Open-ClockPort $n
            $ms = [DateTimeOffset]::UtcNow.ToUnixTimeMilliseconds()
            $cmd = 'UHR4 TIME {0}.{1:D3}' -f [math]::Floor($ms / 1000), [int]($ms % 1000)
            $reply = Send-Command $sp $cmd
            $sp.Close()
            if ($reply -match '^UHR4 OK TIME (.*)$') {
                Write-Host "Uhrzeit gesetzt / time set: $($Matches[1])"
                return $true
            }
            if ($reply) {
                Write-Host "Die Uhr meldet einen Fehler / the clock reports an error: $reply"
                return $false
            }
        } catch {
            # Port gerade weg (Uhr startet) oder belegt - erneut versuchen.
            # Belegt ("Zugriff verweigert"): sofort einmal melden, damit der
            # serielle Monitor noch waehrend der Wartezeit geschlossen werden kann.
            # Port just gone (clock starting) or in use - try again. In use
            # ("access denied"): report once right away, so the serial monitor
            # can still be closed during the wait.
            if ("$($_.Exception)" -match 'UnauthorizedAccess|denied|verweigert') {
                if (-not $busyPort) {
                    Write-Host "COM$n ist belegt - ein anderes Programm (z.B. serieller Monitor in Visual Micro/Arduino IDE) hat die Schnittstelle offen. Bitte schliessen, warte ..."
                    Write-Host "COM$n is in use - another program (e.g. serial monitor in Visual Micro/Arduino IDE) has the port open. Please close it, waiting ..."
                }
                $busyPort = "COM$n"
            }
            else { $busyPort = $null }
        } finally {
            if ($sp -and $sp.IsOpen) { try { $sp.Close() } catch {} }
        }
        Start-Sleep -Milliseconds 500
    }
    if ($busyPort) {
        Write-Host "Uhrzeit nicht gesetzt: $busyPort ist weiterhin belegt. Seriellen Monitor schliessen und erneut starten."
        Write-Host "Time not set: $busyPort is still in use. Close the serial monitor and start again."
        return $false
    }
    Write-Host 'Uhrzeit nicht gesetzt: keine Antwort (Firmware ohne diese Funktion?). Programme mit offenem COM-Port schliessen.'
    Write-Host 'Time not set: no reply (firmware without this function?). Close programs using the COM port.'
    return $false
}


# ---------------------------------------------------------------------------
# Ablauf / flow

$displayName = ''
if ($Time) {
    # Nur Uhrzeit setzen (setTime.bat) / only set the time (setTime.bat)
    $num = 0
    if ($Port) {
        $num = [int]($Port -replace '\D', '')
        Set-Board (Get-PortBoard $num)
    }
    else {
        # Keine laufende ESP32-S2-Uhr: nach einer ESP32-C6-, dann ESP32-S3-Uhr suchen
        # No running ESP32-S2 clock: look for an ESP32-C6, then an ESP32-S3 clock
        foreach ($b in 'c6', 's3') {
            if (Get-RunningClockPort) { break }
            Set-Board $b
        }
    }
    if (-not $Port -and -not (Get-RunningClockPort)) {
        Write-Host 'Keine laufende Uhr gefunden (USB-Kabel? Uhr im Download-Modus: Reset druecken).'
        Write-Host 'No running clock found (USB cable? clock in download mode: press Reset).'
        exit 1
    }
    if (Send-ClockTime $num 30) { exit 0 }
    exit 1
}
elseif ($Send) {
    $c = Get-Choice $Send
    if ($c -gt 0) { $displayName = $names[$c] }
}
elseif ($Flash) {
    $interactive = -not $Port

    # Werkzeuge da? Fehlen sie, wurde meist direkt aus dem Zip gestartet oder der Virenscanner hat
    # esptool.exe entfernt.
    # Tools present? If not, it was usually started directly from the zip or the virus scanner
    # removed esptool.exe.
    $missing = @('esptool.exe', 'port.ps1') | Where-Object { -not (Test-Path (Join-Path $PSScriptRoot $_)) }
    if ($missing) {
        Write-Host "Fehlende Dateien / missing files: $($missing -join ', ')"
        Write-Host 'Das Zip zuerst komplett in einen Ordner auspacken und flashESP.bat dort starten. Fehlt nur'
        Write-Host 'esptool.exe, hat es meist der Virenscanner entfernt (Fehlalarm) - Ausnahme einrichten, neu auspacken.'
        Write-Host 'First unpack the whole zip into a folder and start flashESP.bat there. If only esptool.exe is'
        Write-Host 'missing, the virus scanner usually removed it (false alarm) - add an exception, unpack again.'
        exit 1
    }
    $portScript = Join-Path $PSScriptRoot 'port.ps1'

    # 1) Uhr suchen und COM-Port bestimmen - noch NICHT in den Download-Modus,
    #    damit eine laufende Uhr ihren Displaytyp melden kann
    # 1) Find the clock and determine the COM port - NOT yet into download
    #    mode, so a running clock can report its display type
    & powershell -NoProfile -ExecutionPolicy Bypass -File $portScript "$Port" -NoSwitch
    $selPort = $LASTEXITCODE
    if ($selPort -le 0) { exit 1 }

    # Board am Port erkennen und den Build dieses Chips laden (Get-BuildDir)
    # Recognize the board on the port and download the build of that chip (Get-BuildDir)
    Set-Board (Get-PortBoard $selPort)
    Write-Host "ESP32-$($board.ToUpper()) an / on COM$selPort"
    $binDir = Get-BuildDir $board
    $bins = @('uhr4.ino.bootloader.bin', 'uhr4.ino.partitions.bin', 'uhr4.ino.bin')
    if ($board -ne 's2') {
        $bins += 'boot_app0.bin'
    }
    $missing = $bins | Where-Object { -not (Test-Path (Join-Path $binDir $_)) }
    if ($missing) {
        Write-Host "Fehlende Dateien in / missing files in ${binDir}: $($missing -join ', ')"
        Write-Host 'Das Paket im Release ist unvollstaendig - bitte melden. / The package in the release is incomplete - please report.'
        exit 1
    }

    # 2) Fragen (Displaytyp mit Vorwahl aus der Uhr, WLAN) - danach laufen
    #    Flashen und Einrichten ohne Eingriff
    # 2) Questions (display type preselected from the clock, WiFi) - flashing
    #    and setup then run without intervention
    if ($interactive) {
        # ESP32-S2: nur der laufende Port antwortet; ESP32-C6: einfach fragen - eine fremde oder leere Firmware
        # antwortet nicht, dann bleibt es bei $null
        # ESP32-S2: only the running port replies; ESP32-C6: just ask - a foreign or empty firmware does not
        # reply, then it stays $null
        $running = Get-CimInstance Win32_PnPEntity | Where-Object { $_.Name -match "\(COM$selPort\)" -and $_.DeviceID -match 'VID_303A|VID_1A86' -and $_.DeviceID -notmatch 'PID_0002' }
        $info = if ($running) { Get-ClockInfo $selPort } else { $null }

        # ESP32-S3: das Display ist fest verbaut - keine Abfrage. Antwortet am CH343P keine uhr4, nachfragen:
        # der Wandler sitzt auch auf fremden Geraeten.
        # ESP32-S3: the display is built in - no question. If no uhr4 replies on the CH343P, ask: the converter
        # is also found on other devices.
        if ($board -eq 's3') {
            $c = 0
            if (-not $info) {
                $ok = Read-Host "An COM$selPort antwortet keine uhr4 - trotzdem den ESP32-S3-Build flashen? / no uhr4 replies on COM$selPort - flash the ESP32-S3 build anyway? [j/N, y/N]"
                if ($ok -notmatch '^[JjYy]') { exit 1 }
            }
        }
        else { $c = Read-DisplayChoice $info }
        Read-Wifi
    }
    else {
        $c = Get-Choice ([string]$Display)
    }
    if ($c -gt 0) { $displayName = $names[$c] }

    # 3) Laufende Uhr in den Download-Modus bringen (port.ps1, liefert den
    #    dann neuen COM-Port; eine Uhr im Download-Modus bleibt, wie sie ist)
    # 3) Switch a running clock into download mode (port.ps1, returns the then
    #    new COM port; a clock in download mode stays as it is)
    Write-Host ''
    & powershell -NoProfile -ExecutionPolicy Bypass -File $portScript "$selPort"
    $comPort = $LASTEXITCODE
    if ($comPort -le 0) { $script:wifiPass = $null; exit 1 }

    # 4) Flashen / flash
    Push-Location $binDir

    # Vorbelegen: startet esptool.exe gar nicht (z.B. vom Virenscanner blockiert), bliebe sonst der
    # Exit-Code von port.ps1 stehen.
    # Preset: if esptool.exe does not start at all (e.g. blocked by the virus scanner), the exit code
    # of port.ps1 would remain otherwise.
    $global:LASTEXITCODE = 1
    $esptool = Join-Path $PSScriptRoot 'esptool.exe'

    # ESP32-C6 und -S3: Bootloader an 0x0, dazu boot_app0.bin an 0xE000 - setzt eine Startauswahl einer
    # vorherigen Firmware (z.B. Waveshare-Demo) zurueck. ESP32-S2: Bootloader an 0x1000.
    # ESP32-C6 and -S3: bootloader at 0x0, plus boot_app0.bin at 0xE000 - resets a boot selection of a
    # previous firmware (e.g. Waveshare demo). ESP32-S2: bootloader at 0x1000.
    try {
        if ($board -eq 'c6' -or $board -eq 's3') {
            & $esptool --chip "esp32$board" --port "COM$comPort" --baud 921600 --before default_reset --after hard_reset write_flash -z --flash_mode keep --flash_freq keep --flash_size keep 0x0 uhr4.ino.bootloader.bin 0x8000 uhr4.ino.partitions.bin 0xe000 boot_app0.bin 0x10000 uhr4.ino.bin
        } else {
            & $esptool --chip esp32-S2 --port "COM$comPort" --baud 921600 --before default_reset --after hard_reset write_flash -z --flash_mode keep --flash_freq keep --flash_size keep 0x1000 uhr4.ino.bootloader.bin 0x8000 uhr4.ino.partitions.bin 0x10000 uhr4.ino.bin
        }
    }
    catch { Write-Host "esptool.exe: $($_.Exception.Message)" }
    $flashOk = ($LASTEXITCODE -eq 0)
    Pop-Location
    if (-not $flashOk) {
        $script:wifiPass = $null
        Write-Host ''
        Write-Host 'Flashen fehlgeschlagen - siehe Meldung von esptool oben. Haeufige Ursachen:'
        Write-Host '  * ESP nicht im Bootmodus: Boot-Taste druecken und halten, erst DANACH den USB anstecken'
        Write-Host '    ODER bei angestecktem USB: Reset und Boot druecken, Reset loslassen, Boot kurz danach loslassen.'
        Write-Host '  * "Wrong --chip" / "This chip is ...": falscher Chip - uhr4 gibt es fuer ESP32-S2 (Lolin S2 Pico), ESP32-C6 und ESP32-S3 (Waveshare).'
        Write-Host '  * "could not open port" / Zugriff verweigert: Port belegt - seriellen Monitor schliessen.'
        Write-Host '  * USB-Hub oder Frontanschluss: die Uhr direkt an einen USB-Anschluss am PC stecken.'
        Write-Host '  * esptool.exe startet nicht: vom Virenscanner blockiert - Ausnahme fuer diesen Ordner einrichten.'
        Write-Host 'Danach flashESP.bat erneut starten.'
        Write-Host 'Flashing failed - see the esptool message above. Common causes:'
        Write-Host '  * ESP not in boot mode: press and hold the Boot button, only THEN plug in USB'
        Write-Host '    OR with USB connected: press Reset and Boot, release Reset, release Boot shortly after.'
        Write-Host '  * "Wrong --chip" / "This chip is ...": wrong chip - uhr4 exists for ESP32-S2 (Lolin S2 Pico), ESP32-C6 and ESP32-S3 (Waveshare).'
        Write-Host '  * "could not open port" / access denied: port in use - close the serial monitor.'
        Write-Host '  * USB hub or front port: plug the clock directly into a USB port of the PC.'
        Write-Host '  * esptool.exe does not start: blocked by the virus scanner - add an exception for this folder.'
        Write-Host 'Then run flashESP.bat again.'
        exit 1
    }
}
else {
    Write-Host 'Aufruf / usage: clocksetup.ps1 -Flash [-Port <n>] [-Display <n>]  |  clocksetup.ps1 -Send <n>  |  clocksetup.ps1 -Time [-Port <n>]'
    exit 1
}

# 5) Einstellungen senden / send the settings
$result = 0
$restarting = $false
if ($displayName -or $wifiSsid) {
    Write-Host ''
    Write-Host 'Einstellungen werden an die Uhr gesendet - warte auf die gestartete Uhr ...'
    Write-Host 'Sending settings to the clock - waiting for the started clock ...'

    $start = Get-Date
    $hinted = $false
    $wifiDone = -not $wifiSsid
    $result = 1
    $portSeen = $null    # erster Zeitpunkt mit laufender Uhr / first time a running clock was seen
    $noReply = $false
    while (((Get-Date) - $start).TotalSeconds -lt 150) {
        $num = Get-RunningClockPort
        if (-not $num) {
            if (-not $hinted -and ((Get-Date) - $start).TotalSeconds -gt 20) {
                Write-Host 'Die Uhr startet nicht von selbst? Einmal die Reset-Taste druecken.'
                Write-Host 'Clock does not start by itself? Press the Reset button once.'
                $hinted = $true
            }
            Start-Sleep -Milliseconds 500
            continue
        }
        # Uhr da, antwortet aber 45 s lang nicht: Firmware ohne Einrichtung per USB
        # (aelter als diese flashESP-Version) - aufgeben statt die Schnittstelle
        # weiter zu belegen.
        # Clock present but no reply for 45 s: firmware without USB setup (older
        # than this flashESP version) - give up instead of keeping the port busy.
        if (-not $portSeen) { $portSeen = Get-Date }
        elseif (((Get-Date) - $portSeen).TotalSeconds -gt 45) { $noReply = $true; break }

        $sp = $null
        try {
            # ESP32-S2: DTR an, erst dann sendet die Uhr (USB-CDC) ihre Antworten. RTS ebenfalls an: .NET
            # setzt die Leitungen beim Oeffnen nacheinander, mit RTS aus entstand dabei die esptool-Folge,
            # auf die der ESP32-S2 in den Download-Modus springt. ESP32-C6: beide aus (Open-ClockPort).
            # ESP32-S2: DTR on, only then does the clock (USB CDC) send its replies. RTS on as well: .NET
            # sets the lines one after another when opening, with RTS off this produced the esptool
            # sequence on which the ESP32-S2 jumps into download mode. ESP32-C6: both off (Open-ClockPort).
            $sp = Open-ClockPort $num

            if (-not $wifiDone) {
                $reply = Send-Command $sp ("UHR4 WIFI {0} {1}" -f (ConvertTo-Hex $wifiSsid), (ConvertTo-Hex $wifiPass))
                if (-not $reply) { throw 'no reply' }
                if ($reply -notmatch '^UHR4 OK') { Write-Host "Die Uhr meldet einen Fehler / the clock reports an error: $reply"; break }
                Write-Host "WLAN '$wifiSsid' gespeichert / WiFi '$wifiSsid' saved."
                $wifiDone = $true
            }
            $restarting = $false
            if ($displayName) {
                $reply = Send-Command $sp "UHR4 DISPLAY $displayName"
                if (-not $reply) { throw 'no reply' }
                if ($reply -notmatch '^UHR4 OK') { Write-Host "Die Uhr meldet einen Fehler / the clock reports an error: $reply"; break }
                $restarting = $reply -match 'RESTART'
                if ($restarting) { Write-Host "Displaytyp $displayName eingestellt / display type $displayName set." }
                else { Write-Host "Displaytyp $displayName war bereits eingestellt / display type $displayName was already set." }
            }
            if (-not $restarting -and $wifiSsid) {
                # WLAN wird erst nach einem Neustart verwendet / WiFi is used only after a restart
                [void](Send-Command $sp 'UHR4 RESTART')
                $restarting = $true
            }
            # Sofort schliessen - die Uhr startet erst 1 s nach ihrer Antwort neu
            # Close right away - the clock restarts only 1 s after its reply
            $sp.Close()
            if ($restarting) {
                Write-Host 'Die Uhr startet neu / the clock restarts.'
            }
            $result = 0
            break
        } catch {
            # Port gerade weg (Uhr startet) oder belegt - erneut versuchen
            # port just gone (clock starting) or in use - try again
        } finally {
            if ($sp -and $sp.IsOpen) { try { $sp.Close() } catch {} }
        }
        Start-Sleep -Milliseconds 500
    }
    $wifiPass = $null

    if ($noReply) {
        Write-Host 'Die Uhr antwortet nicht - ihre Firmware kennt die Einrichtung per USB noch nicht.'
        Write-Host 'Displaytyp und WLAN dann in der Weboberflaeche einstellen.'
        Write-Host 'The clock does not reply - its firmware does not know the setup via USB yet.'
        Write-Host 'Set display type and WiFi in the web interface then.'
    }
    elseif ($result -ne 0 -and ((Get-Date) - $start).TotalSeconds -ge 150) {
        Write-Host 'Keine Antwort von der Uhr. Displaytyp und WLAN dann in der Weboberflaeche einstellen'
        Write-Host 'oder flashESP.bat erneut starten. Programme mit offenem COM-Port (z.B. seriellen Monitor) schliessen.'
        Write-Host 'No reply from the clock. Set display type and WiFi in the web interface'
        Write-Host 'or run flashESP.bat again. Close programs using the COM port (e.g. serial monitor).'
    }
}

# Serieller Monitor: zeigt die Ausgabe der Uhr, damit Fehler beim Start (WLAN, Zeit) sichtbar bleiben. Verbindet sich
# nach einem Neustart der Uhr selbst wieder. Beenden mit Q oder Esc (oder Strg+C). Die Uhr schreibt ins Log, wenn
# "Logging aktivieren" in ihren Einstellungen an ist.
# Serial monitor: shows the clock's output so errors at start (WiFi, time) stay visible. Reconnects by itself after
# the clock restarts. Quit with Q or Esc (or Ctrl+C). The clock writes to the log if "Enable Logging" is on in its
# settings.
function Start-SerialMonitor {
    Write-Host ''
    Write-Host 'Serieller Monitor - Beenden mit Q oder Esc. Ausgaben erscheinen nur, wenn "Logging aktivieren" in der Uhr an ist.'
    Write-Host 'Serial monitor - quit with Q or Esc. Output appears only if "Enable Logging" is on in the clock.'
    Write-Host '------------------------------------------------------------'
    $sp = $null
    $keys = $true
    while ($true) {
        if ($keys) {
            try {
                if ([Console]::KeyAvailable) {
                    $k = [Console]::ReadKey($true)
                    if ($k.Key -eq 'Q' -or $k.Key -eq 'Escape') { break }
                }
            } catch { $keys = $false }   # keine Konsole (Eingabe umgeleitet) / no console (input redirected)
        }
        if (-not $sp -or -not $sp.IsOpen) {
            $n = Get-RunningClockPort
            if (-not $n) { Start-Sleep -Milliseconds 500; continue }
            try {
                $sp = Open-ClockPort $n
                Write-Host "--- COM$n verbunden / connected ---"
            } catch {
                $sp = $null
                Start-Sleep -Milliseconds 500
                continue
            }
        }
        try {
            $text = $sp.ReadExisting()
            if ($text) { Write-Host -NoNewline $text }
        } catch {
            # Port weg (Uhr startet neu): schliessen und neu verbinden / port gone (clock restarts): close and reconnect
            try { $sp.Close() } catch {}
            $sp = $null
            Write-Host '--- Verbindung getrennt / disconnected ---'
            continue
        }
        Start-Sleep -Milliseconds 50
    }
    if ($sp -and $sp.IsOpen) { try { $sp.Close() } catch {} }
    Write-Host ''
}

# 6) Uhrzeit des PCs senden - nach einem Neustart erst, wenn die Uhr wieder
#    laeuft (die Wartezeit ueberbrueckt die 1 s bis zum Neustart). Nur wenn
#    die Einstellungen angekommen sind; sonst antwortet die Uhr ohnehin nicht.
# 6) Send the PC's time - after a restart only once the clock runs again (the
#    wait bridges the 1 s until the restart). Only if the settings arrived;
#    otherwise the clock does not reply anyway.
if ($result -eq 0) {
    Write-Host ''
    Write-Host 'Uhrzeit des PCs wird an die Uhr gesendet ... / sending the PC time to the clock ...'
    if ($restarting) { Start-Sleep -Seconds 3 }
    [void](Send-ClockTime 0 60)
}

# 7) Serieller Monitor (nur nach dem Flashen) / serial monitor (only after flashing)
if ($Flash -and -not $NoMonitor) { Start-SerialMonitor }
exit $result
