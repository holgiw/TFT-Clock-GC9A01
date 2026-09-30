# Flashen und Einrichten der Uhr per USB, Aufruf aus flashESP.bat.
#   -Flash [-Port <n>] [-Display <n>]
#        Ohne -Port (Doppelklick auf flashESP.bat): sucht zuerst die Uhr
#        (port.ps1 -NoSwitch), fragt eine laufende Uhr nach ihrem Displaytyp,
#        fragt Displaytyp und WLAN ab, bringt die Uhr dann in den Download-Modus,
#        flasht sie (esptool) und sendet danach die Einstellungen und die
#        Uhrzeit des PCs ueber USB.
#        Mit -Port ohne Rueckfragen, das WLAN wird dann nicht abgefragt.
#   -Send <n>
#        Nur senden (ohne Flashen): Displaytyp an eine laufende Uhr.
#   -Time [-Port <n>]
#        Nur die Uhrzeit des PCs an eine laufende Uhr senden (setTime.bat).
# Gesendet wird "UHR4 WIFI <Name-Hex> <Passwort-Hex>", "UHR4 DISPLAY <Name>",
# bei Bedarf "UHR4 RESTART", danach "UHR4 TIME <Unix-Sekunden>" (siehe
# handleSerialCommands() in display.h).
# Displaytyp: 1/GC9A01, 2/GC9A01_WITH_BACKLIGHT, 3/GC9D01, 0 = unveraendert.
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
# Sent are "UHR4 WIFI <name hex> <password hex>", "UHR4 DISPLAY <name>",
# "UHR4 RESTART" if needed, then "UHR4 TIME <unix seconds>" (see
# handleSerialCommands() in display.h).
# Display type: 1/GC9A01, 2/GC9A01_WITH_BACKLIGHT, 3/GC9D01, 0 = unchanged.
# The WiFi password is entered hidden, stays only in this script's memory (no
# file, no environment variable) and only goes to the clock via USB.
param([switch]$Flash, [string]$Port, [string]$Display, [string]$Send, [switch]$Time)

$names = @{ 1 = 'GC9A01'; 2 = 'GC9A01_WITH_BACKLIGHT'; 3 = 'GC9D01' }

# Auswahl als Zahl 1-3 oder Name, sonst 0 / choice as number 1-3 or name, else 0
function Get-Choice([string]$value) {
    $v = $value.Trim().ToUpper()
    if ($v -match '^[1-3]$') { return [int]$v }
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
    $labels = @{ 1 = 'GC9A01 (240x240) ohne Hintergrundbeleuchtung (BL) / without backlight (BL)'; 2 = 'GC9A01 (240x240) mit Hintergrundbeleuchtung (BL) an Pin 3 / with backlight (BL) on pin 3'; 3 = 'GC9D01 (160x160)' }
    foreach ($k in 1, 2, 3) {
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
        Write-Host 'Bitte 1, 2 oder 3 eingeben / please enter 1, 2 or 3'
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

function Get-RunningClockPort {
    Get-CimInstance Win32_PnPEntity | Where-Object { $_.Name -match '\(COM(\d+)\)' -and $_.DeviceID -match 'VID_303A' -and $_.DeviceID -notmatch 'PID_0002|PID_1001' } |
        ForEach-Object { [int]([regex]::Match($_.Name, '\(COM(\d+)\)').Groups[1].Value) } | Select-Object -First 1
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
        $sp = New-Object System.IO.Ports.SerialPort ("COM$num", 115200)
        $sp.DtrEnable = $true
        $sp.RtsEnable = $true
        $sp.ReadTimeout = 500
        $sp.NewLine = "`n"
        $sp.Open()
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
            # DTR/RTS an - siehe Einstellungen senden weiter unten
            # DTR/RTS on - see sending the settings further below
            $sp = New-Object System.IO.Ports.SerialPort ("COM$n", 115200)
            $sp.DtrEnable = $true
            $sp.RtsEnable = $true
            $sp.ReadTimeout = 500
            $sp.NewLine = "`n"
            $sp.Open()
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
    }
    elseif (-not (Get-RunningClockPort)) {
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

    # Alle Dateien da? Fehlen sie, wurde meist direkt aus dem Zip gestartet oder der Virenscanner hat
    # esptool.exe entfernt.
    # All files present? If not, it was usually started directly from the zip or the virus scanner
    # removed esptool.exe.
    $missing = @('esptool.exe', 'port.ps1', 'uhr4.ino.bootloader.bin', 'uhr4.ino.partitions.bin', 'uhr4.ino.bin') |
        Where-Object { -not (Test-Path (Join-Path $PSScriptRoot $_)) }
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

    # 2) Fragen (Displaytyp mit Vorwahl aus der Uhr, WLAN) - danach laufen
    #    Flashen und Einrichten ohne Eingriff
    # 2) Questions (display type preselected from the clock, WiFi) - flashing
    #    and setup then run without intervention
    if ($interactive) {
        $running = Get-CimInstance Win32_PnPEntity | Where-Object { $_.Name -match "\(COM$selPort\)" -and $_.DeviceID -match 'VID_303A' -and $_.DeviceID -notmatch 'PID_0002|PID_1001' }
        $info = if ($running) { Get-ClockInfo $selPort } else { $null }
        $c = Read-DisplayChoice $info
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
    Push-Location $PSScriptRoot

    # Vorbelegen: startet esptool.exe gar nicht (z.B. vom Virenscanner blockiert), bliebe sonst der
    # Exit-Code von port.ps1 stehen.
    # Preset: if esptool.exe does not start at all (e.g. blocked by the virus scanner), the exit code
    # of port.ps1 would remain otherwise.
    $global:LASTEXITCODE = 1
    try { & .\esptool.exe --chip esp32-S2 --port "COM$comPort" --baud 921600 --before default_reset --after hard_reset write_flash -z --flash_mode keep --flash_freq keep --flash_size keep 0x1000 uhr4.ino.bootloader.bin 0x8000 uhr4.ino.partitions.bin 0x10000 uhr4.ino.bin }
    catch { Write-Host "esptool.exe: $($_.Exception.Message)" }
    $flashOk = ($LASTEXITCODE -eq 0)
    Pop-Location
    if (-not $flashOk) {
        $script:wifiPass = $null
        Write-Host ''
        Write-Host 'Flashen fehlgeschlagen - siehe Meldung von esptool oben. Haeufige Ursachen:'
        Write-Host '  * ESP nicht im Bootmodus: Boot-Taste druecken und halten, erst DANACH den USB anstecken'
        Write-Host '    ODER bei angestecktem USB: Reset und Boot druecken, Reset loslassen, Boot kurz danach loslassen.'
        Write-Host '  * "Wrong --chip" / "This chip is ...": kein ESP32-S2 - uhr4 laeuft nur auf dem ESP32-S2 (Lolin S2 Pico).'
        Write-Host '  * "could not open port" / Zugriff verweigert: Port belegt - seriellen Monitor schliessen.'
        Write-Host '  * USB-Hub oder Frontanschluss: die Uhr direkt an einen USB-Anschluss am PC stecken.'
        Write-Host '  * esptool.exe startet nicht: vom Virenscanner blockiert - Ausnahme fuer diesen Ordner einrichten.'
        Write-Host 'Danach flashESP.bat erneut starten.'
        Write-Host 'Flashing failed - see the esptool message above. Common causes:'
        Write-Host '  * ESP not in boot mode: press and hold the Boot button, only THEN plug in USB'
        Write-Host '    OR with USB connected: press Reset and Boot, release Reset, release Boot shortly after.'
        Write-Host '  * "Wrong --chip" / "This chip is ...": not an ESP32-S2 - uhr4 only runs on the ESP32-S2 (Lolin S2 Pico).'
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
            # DTR an: erst dann sendet die Uhr (USB-CDC) ihre Antworten. RTS ebenfalls
            # an: .NET setzt die Leitungen beim Oeffnen nacheinander, mit RTS aus
            # entstand dabei die esptool-Folge (DTR/RTS), auf die der ESP32-S2 in den
            # Download-Modus springt. Die Firmware ignoriert diese Folge inzwischen
            # ohnehin (Serial.enableReboot(false) in uhr4.ino).
            # DTR on: only then does the clock (USB CDC) send its replies. RTS on as
            # well: .NET sets the lines one after another when opening, with RTS off
            # this produced the esptool sequence (DTR/RTS) on which the ESP32-S2
            # jumps into download mode. The firmware ignores that sequence by now
            # anyway (Serial.enableReboot(false) in uhr4.ino).
            $sp = New-Object System.IO.Ports.SerialPort ("COM$num", 115200)
            $sp.DtrEnable = $true
            $sp.RtsEnable = $true
            $sp.ReadTimeout = 500
            $sp.NewLine = "`n"
            $sp.Open()

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
exit $result
