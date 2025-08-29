param(
    [string]$BuildDir = "build-ide"
)

$ErrorActionPreference = "Stop"

$Root = Split-Path -Path $MyInvocation.MyCommand.Path -Parent
$Source = $Root
$Build = Join-Path $Root $BuildDir

Write-Host "[1/3] Configuring CMake project (host)" -ForegroundColor Cyan
cmake -S $Source -B $Build

Write-Host "[2/3] Building JUCE artefacts to generate Android Studio project" -ForegroundColor Cyan
# Try common target names; fall back to 'help' parsing
$targetsToTry = @(
    "AudioAnalyzer_Android",
    "AudioAnalyzer_AndroidStudio",
    "AudioAnalyzer_artefacts"
)

$selected = $null
foreach ($t in $targetsToTry) {
    try {
        cmake --build $Build --target $t
        $selected = $t
        break
    } catch { }
}

if (-not $selected) {
    Write-Host "Couldn't build a known Android artefact target directly; scanning 'help' for candidates..." -ForegroundColor Yellow
    $help = cmake --build $Build --target help 2>&1 | Out-String
    $androidish = $help -split "`r?`n" | Where-Object { $_ -match "Android|artefact|Artefact" }
    if ($androidish.Count -gt 0) {
        Write-Host ($androidish -join "`n")
    }
    throw "No Android artefact target found. Ensure your JUCE checkout is recent enough."
}

Write-Host "[3/3] Locating generated Gradle project" -ForegroundColor Cyan
$gradle = Get-ChildItem -Path $Build -Recurse -Filter "settings.gradle" -ErrorAction SilentlyContinue | Select-Object -First 1
if ($gradle) {
    $proj = Split-Path -Parent $gradle.FullName
    Write-Host "`nAndroid Studio project generated at:" -ForegroundColor Green
    Write-Host $proj
    Write-Host "`nOpen this folder in Android Studio (File > Open)." -ForegroundColor Green
    exit 0
}

Write-Warning "Could not locate settings.gradle under $Build. Typical path is: $Build/AudioAnalyzer_artefacts/Android/AudioAnalyzer"
exit 1
