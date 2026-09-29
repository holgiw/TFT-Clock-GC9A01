# Ermittelt den COM-Port zum Flashen der Uhr, Aufruf aus clocksetup.ps1 (zuerst mit
# -NoSwitch nur zum Suchen, danach zum Umschalten in den Download-Modus).
# Rueckgabe als Exit-Code: Portnummer (3 = COM3) oder 0 = abbrechen.
#
# Der ESP32-S2 meldet sich mit ZWEI verschiedenen COM-Ports: laufend (Arduino
# USB-CDC) und im Download-Modus (ROM, USB-Kennung 303A:0002). Laeuft die Uhr,
# wird sie wie in der Arduino IDE per 1200-Baud-Signal in den Download-Modus
# neu gestartet und anschliessend der dann neu erscheinende Port verwendet.
#
# Determines the COM port for flashing the clock, called from clocksetup.ps1 (first
# with -NoSwitch only to find it, then to switch into download mode).
# Returned as exit code: port number (3 = COM3) or 0 = abort.
#
# The ESP32-S2 shows up with TWO different COM ports: running (Arduino
# USB-CDC) and in download mode (ROM, USB id 303A:0002). If the clock is
# running, it is restarted into download mode via the 1200 baud signal like
# the Arduino IDE does, and the port that then appears is used.
# -NoSwitch: nur suchen/auswaehlen, eine laufende Uhr NICHT in den Download-
# Modus bringen (clocksetup.ps1 fragt sie vorher noch nach ihrem Displaytyp).
# -NoSwitch: only find/select, do NOT switch a running clock into download
# mode (clocksetup.ps1 still asks it for its display type beforehand).
param([string]$PortArg, [switch]$NoSwitch)

function Get-ComPorts {
    Get-CimInstance Win32_PnPEntity | Where-Object { $_.Name -match '\(COM(\d+)\)' } | ForEach-Object {
        $num = [int]([regex]::Match($_.Name, '\(COM(\d+)\)').Groups[1].Value)
        $state = 'other'
        if ($_.DeviceID -match 'VID_303A&PID_0002') { $state = 'download' }
        elseif ($_.DeviceID -match 'VID_303A') { $state = 'running' }
        [pscustomobject]@{ Num = $num; State = $state; Name = $_.Name }
    } | Sort-Object Num
}

function Get-StateText($state) {
    switch ($state) {
        'download' { 'UHR - Download-Modus / download mode' }
        'running'  { 'UHR - laeuft / running' }
        default    { '-' }
    }
}

$ports = @(Get-ComPorts)

if (-not $PortArg) {
    Write-Host 'COM-Schnittstellen / COM ports:'
    if ($ports.Count -eq 0) { Write-Host '  keine / none' }
    foreach ($p in $ports) { Write-Host ('  COM{0}  {1}  [{2}]' -f $p.Num, (Get-StateText $p.State), $p.Name) }
    Write-Host ''
    # Genau eine Uhr: ohne Rueckfrage verwenden. Sonst nach der Nummer fragen.
    # Exactly one clock: use it without asking. Otherwise ask for the number.
    $clocks = @($ports | Where-Object { $_.State -ne 'other' })
    if ($clocks.Count -eq 1) {
        $PortArg = [string]$clocks[0].Num
        Write-Host "Uhr auf / clock on COM$PortArg"
    } else {
        if ($clocks.Count -gt 1) {
            Write-Host 'Mehrere Uhren gefunden - bitte nur eine anschliessen oder die Nummer angeben.'
            Write-Host 'Several clocks found - connect only one or enter the number.'
        } else {
            Write-Host 'Keine Uhr erkannt. Bitte pruefen:'
            Write-Host '  * Taucht die Uhr im Geraetemanager (devmgmt.msc) unter "Anschluesse (COM & LPT)" auf'
            Write-Host '    und hat sie dort einen COM-Port bekommen?'
            Write-Host '  * Ist das verwendete USB-Kabel ein Datenkabel oder nur ein Ladekabel?'
            Write-Host 'No clock detected. Please check:'
            Write-Host '  * Does the clock show up in Device Manager (devmgmt.msc) under "Ports (COM & LPT)"'
            Write-Host '    and has it been assigned a COM port?'
            Write-Host '  * Is the USB cable a data cable or just a charging cable?'
            Write-Host ''
            Write-Host 'Nummer angeben oder Enter zum Abbrechen / enter the number or press Enter to abort.'
        }
        $PortArg = Read-Host 'COM-Port Nummer / number (z.B./e.g. 3 = COM3)'
        if (-not $PortArg) { exit 0 }
    }
}

$digits = $PortArg -replace '[^0-9]', ''
if (-not $digits) { Write-Host "Ungueltige Eingabe / invalid input: $PortArg"; exit 0 }
$num = [int]$digits
$port = $ports | Where-Object { $_.Num -eq $num } | Select-Object -First 1

if (-not $port) {
    Write-Host "COM$num nicht vorhanden / not present."
    exit 0
}

if ($port.State -eq 'other') {
    Write-Host "Hinweis: an COM$num wurde keine Uhr erkannt - es wird trotzdem versucht."
    Write-Host "Note: no clock detected on COM$num - trying anyway."
}

if ($NoSwitch) { exit $num }

if ($port.State -eq 'running') {
    Write-Host "Uhr laeuft auf COM$num - Neustart in den Download-Modus ..."
    Write-Host "Clock running on COM$num - restarting into download mode ..."
    try {
        $sp = New-Object System.IO.Ports.SerialPort ("COM$num", 1200)
        $sp.Open()
        $sp.DtrEnable = $false
        Start-Sleep -Milliseconds 100
        $sp.Close()
    } catch {
        # Nicht abbrechen: verschwindet der Port gerade (Uhr startet schon neu),
        # erscheint gleich der Download-Port. Sonst greift unten die Zeitgrenze.

        # Do not abort: if the port is just disappearing (clock already
        # restarting), the download port shows up shortly. Otherwise the
        # timeout below applies.
    }
    $found = $null
    for ($i = 0; $i -lt 30 -and -not $found; $i++) {
        Start-Sleep -Milliseconds 500
        $found = Get-ComPorts | Where-Object { $_.State -eq 'download' } | Select-Object -First 1
    }
    if (-not $found) {
        Write-Host 'Kein Download-Modus erkannt. Programme mit offenem COM-Port (z.B. seriellen Monitor) schliessen'
        Write-Host 'oder Boot-Taste halten, USB neu einstecken und erneut starten.'
        Write-Host 'No download mode detected. Close programs using the COM port (e.g. serial monitor)'
        Write-Host 'or hold the Boot button, replug USB and start again.'
        exit 0
    }
    Write-Host "Download-Modus auf / download mode on COM$($found.Num)"
    $num = $found.Num
}

exit $num
