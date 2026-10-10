# Veroeffentlicht die ESP32-S3-Firmware fuer die Update-Funktion der Uhr ("Firmware von GitHub", ota_update.h):
# laedt uhr4-s3.bin und uhr4-s3.txt (Zeile 1: Build-Zeit, Zeile 2: SHA-256) in das neueste GitHub-Release.
# Die Build-Zeit steht als Kennung "UHR4_FW_BUILD=..." in der .bin. Aufruf: .\publish_s3_update.ps1 [-DryRun]
#
# Publishes the ESP32-S3 firmware for the clock's update function ("Firmware from GitHub", ota_update.h):
# uploads uhr4-s3.bin and uhr4-s3.txt (line 1: build time, line 2: SHA-256) to the latest GitHub release.
# The build time is in the .bin as marker "UHR4_FW_BUILD=...". Usage: .\publish_s3_update.ps1 [-DryRun]

param([switch]$DryRun)

$ErrorActionPreference = 'Stop'
$bin = Join-Path $PSScriptRoot 'esp32s3\uhr4.ino.bin'
if (-not (Test-Path $bin)) { throw "Not found: $bin" }

$bytes = [System.IO.File]::ReadAllBytes($bin)
$text = [System.Text.Encoding]::Latin1.GetString($bytes)
$at = $text.IndexOf('UHR4_FW_BUILD=')
if ($at -lt 0) { throw 'Marker UHR4_FW_BUILD= not found - the .bin was built without the update function' }
$end = $text.IndexOf([char]0, $at)
$stamp = ($text.Substring($at + 14, $end - $at - 14) -replace '\s+', ' ').Trim()
$built = [datetime]::ParseExact($stamp, 'MMM d yyyy HH:mm:ss', [System.Globalization.CultureInfo]::InvariantCulture)
$iso = $built.ToString('yyyy-MM-dd HH:mm:ss')
$sha = (Get-FileHash -Algorithm SHA256 $bin).Hash.ToLower()

$work = Join-Path ([System.IO.Path]::GetTempPath()) ('uhr4-s3-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $work | Out-Null
try {
    Copy-Item $bin (Join-Path $work 'uhr4-s3.bin')
    [System.IO.File]::WriteAllText((Join-Path $work 'uhr4-s3.txt'), "$iso`n$sha`n")
    Write-Host "Build $iso  SHA-256 $sha  ($([math]::Round($bytes.Length / 1KB)) KB)"
    if ($DryRun) { Write-Host 'Dry run - nothing uploaded'; return }
    $tag = (gh release view --json tagName -q .tagName).Trim()
    Write-Host "Uploading to release $tag ..."
    gh release upload $tag (Join-Path $work 'uhr4-s3.bin') (Join-Path $work 'uhr4-s3.txt') --clobber
    if ($LASTEXITCODE -ne 0) { throw 'gh release upload failed' }
    Write-Host 'Done'
}
finally {
    Remove-Item -Recurse -Force $work
}
