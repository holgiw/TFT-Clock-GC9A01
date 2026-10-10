# Erzeugt die Pakete, die flashESP.bat / flashESP.sh beim Flashen vom GitHub-Release laden:
# je Chip uhr4-esp32s2.zip, uhr4-esp32c6.zip, uhr4-esp32s3.zip (Bootloader, Partitionstabelle,
# App, bei C6 und S3 boot_app0.bin - flach im Zip) und dazu uhr4-<chip>.sha256 ("<SHA-256>  <Zip>").
# Quelle sind die Ordner esp32s2, esp32c6 und esp32s3 neben diesem Skript.
#   .\publish_builds.ps1                      Pakete in einen Ordner schreiben (Standard: Temp), nichts hochladen
#   .\publish_builds.ps1 -OutDir C:\test      Zielordner angeben
#   .\publish_builds.ps1 -Upload              zusaetzlich in das Release hochladen (gh, ersetzt vorhandene Dateien)
# Zusaetzlich entsteht uhr4_flash.zip nur mit den Werkzeugen (ohne Builds und ohne publish_*.ps1) aus dem letzten Commit.
# Zum Testen ohne Release: $env:UHR4_BUILD_URL = 'file:///C:/test' vor flashESP.bat bzw. clocksetup.ps1.
#
# Creates the packages that flashESP.bat / flashESP.sh download from the GitHub release when flashing:
# per chip uhr4-esp32s2.zip, uhr4-esp32c6.zip, uhr4-esp32s3.zip (bootloader, partition table, app,
# boot_app0.bin for C6 and S3 - flat in the zip) and uhr4-<chip>.sha256 ("<SHA-256>  <zip>").
# The source is the folders esp32s2, esp32c6 and esp32s3 next to this script.
#   .\publish_builds.ps1                      write the packages to a folder (default: temp), upload nothing
#   .\publish_builds.ps1 -OutDir C:\test      set the target folder
#   .\publish_builds.ps1 -Upload              also upload to the release (gh, replaces existing files)
# uhr4_flash.zip is created as well, with the tools only (no builds, no publish_*.ps1), from the last commit.
# To test without a release: $env:UHR4_BUILD_URL = 'file:///C:/test' before flashESP.bat or clocksetup.ps1.

param([string]$OutDir, [switch]$Upload, [string]$Tag = 'v4')

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.IO.Compression.FileSystem
if (-not $OutDir) { $OutDir = Join-Path ([System.IO.Path]::GetTempPath()) 'uhr4-packages' }
New-Item -ItemType Directory -Path $OutDir -Force | Out-Null

$files = @()
foreach ($chip in 's2', 'c6', 's3') {
    $src = Join-Path $PSScriptRoot "esp32$chip"
    $parts = @('uhr4.ino.bootloader.bin', 'uhr4.ino.partitions.bin', 'uhr4.ino.bin')
    if ($chip -ne 's2') { $parts += 'boot_app0.bin' }
    $stage = Join-Path ([System.IO.Path]::GetTempPath()) ('uhr4-stage-' + [guid]::NewGuid().ToString('N'))
    New-Item -ItemType Directory -Path $stage | Out-Null
    try {
        foreach ($p in $parts) {
            $f = Join-Path $src $p
            if (-not (Test-Path $f)) { throw "Missing: $f" }
            Copy-Item $f $stage
        }
        $name = "uhr4-esp32$chip"
        $zip = Join-Path $OutDir "$name.zip"
        if (Test-Path $zip) { Remove-Item $zip -Force }
        [System.IO.Compression.ZipFile]::CreateFromDirectory($stage, $zip)
        $hash = (Get-FileHash -Algorithm SHA256 $zip).Hash.ToLower()
        $shaFile = Join-Path $OutDir "$name.sha256"
        [System.IO.File]::WriteAllText($shaFile, "$hash  $name.zip`n")
        Write-Host ("{0}  {1}  {2:N0} KB" -f $name, $hash, ((Get-Item $zip).Length / 1KB))
        $files += $zip, $shaFile
    }
    finally {
        Remove-Item -Recurse -Force $stage
    }
}

# Werkzeug-Zip aus dem letzten Commit (Skripte, esptool.exe, readme.txt; ohne die Builds)
# Tools zip from the last commit (scripts, esptool.exe, readme.txt; without the builds)
$toolsZip = Join-Path $OutDir 'uhr4_flash.zip'
if (Test-Path $toolsZip) { Remove-Item $toolsZip -Force }
Push-Location (Split-Path $PSScriptRoot -Parent)
git archive --format=zip -o $toolsZip HEAD -- build_uhr4 LICENSE THIRD_PARTY_LICENSES.md ':(exclude)build_uhr4/esp32s2' ':(exclude)build_uhr4/esp32c6' ':(exclude)build_uhr4/esp32s3' ':(exclude)build_uhr4/publish_*.ps1'
$gitOk = ($LASTEXITCODE -eq 0)
Pop-Location
if (-not $gitOk) { throw 'git archive failed' }
Write-Host ("uhr4_flash.zip  {0:N0} KB" -f ((Get-Item $toolsZip).Length / 1KB))
$files += $toolsZip
Write-Host "Packages in $OutDir"

if ($Upload) {
    Write-Host "Uploading to release $Tag ..."
    gh release upload $Tag @files --clobber
    if ($LASTEXITCODE -ne 0) { throw 'gh release upload failed' }
    Write-Host 'Done'
}
