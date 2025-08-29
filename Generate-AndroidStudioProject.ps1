param(
    [string]$BuildDir = "build-ide",
    [string]$AndroidBuildDir = "build-android",
    [string]$NdkPath
)

$ErrorActionPreference = "Stop"

$Root = Split-Path -Path $MyInvocation.MyCommand.Path -Parent
$Source = $Root
$Build = Join-Path $Root $BuildDir
$AndroidBuild = Join-Path $Root $AndroidBuildDir

Write-Host "[1/3] Configuring CMake project (host)" -ForegroundColor Cyan
cmake -S $Source -B $Build
if ($LASTEXITCODE -ne 0) {
    throw "CMake configure failed with exit code $LASTEXITCODE"
}

Write-Host "[2/3] Generating Android CMake build (to emit Gradle project)" -ForegroundColor Cyan

# Try to locate the Android NDK
function Get-AndroidNdkPath {
    param([string]$Override)
    if ($Override -and (Test-Path $Override)) { return $Override }
    if ($env:ANDROID_NDK_ROOT -and (Test-Path $env:ANDROID_NDK_ROOT)) { return $env:ANDROID_NDK_ROOT }
    if ($env:ANDROID_NDK_HOME -and (Test-Path $env:ANDROID_NDK_HOME)) { return $env:ANDROID_NDK_HOME }

    $sdkRoots = @()
    foreach ($v in 'ANDROID_SDK_ROOT','ANDROID_HOME') {
        $val = [System.Environment]::GetEnvironmentVariable($v)
        if ($val -and (Test-Path $val)) { $sdkRoots += $val }
    }
    if ($env:LOCALAPPDATA) {
        $sdkRoots += (Join-Path $env:LOCALAPPDATA 'Android/Sdk')
    }
    if ($env:ProgramData) {
        $sdkRoots += (Join-Path $env:ProgramData 'Android/Sdk')
    }
    $sdkRoots = $sdkRoots | Where-Object { $_ -and (Test-Path $_) } | Select-Object -Unique

    foreach ($sdk in $sdkRoots) {
        $ndkDir = Join-Path $sdk 'ndk'
        if (Test-Path $ndkDir) {
            $candidates = Get-ChildItem -Path $ndkDir -Directory -ErrorAction SilentlyContinue | Sort-Object Name -Descending
            if ($candidates -and $candidates.Count -gt 0) { return $candidates[0].FullName }
        }
        $bundle = Join-Path $sdk 'ndk-bundle'
        if (Test-Path $bundle) { return $bundle }
    }
    return $null
}

$ndk = Get-AndroidNdkPath -Override $NdkPath
if ($ndk) {
    Write-Host "Using Android NDK: $ndk" -ForegroundColor DarkGray
    $toolchain = Join-Path $ndk "build/cmake/android.toolchain.cmake"
    if (-not (Test-Path $toolchain)) { throw "Android toolchain not found at $toolchain" }

    # Prefer Ninja; locate it via PATH or Android SDK cmake packages
    function Find-NinjaPath {
        try { $v = & ninja --version 2>$null; if ($LASTEXITCODE -eq 0) { return 'ninja' } } catch {}
        $sdkRoots = @()
        foreach ($v in 'ANDROID_SDK_ROOT','ANDROID_HOME') {
            $val = [System.Environment]::GetEnvironmentVariable($v)
            if ($val -and (Test-Path $val)) { $sdkRoots += $val }
        }
        if ($env:LOCALAPPDATA) { $sdkRoots += (Join-Path $env:LOCALAPPDATA 'Android/Sdk') }
        $sdkRoots = $sdkRoots | Where-Object { $_ -and (Test-Path $_) } | Select-Object -Unique
        foreach ($sdk in $sdkRoots) {
            $cmakeDir = Join-Path $sdk 'cmake'
            if (Test-Path $cmakeDir) {
                $versions = Get-ChildItem -Path $cmakeDir -Directory -ErrorAction SilentlyContinue | Sort-Object Name -Descending
                foreach ($ver in $versions) {
                    $ninja = Join-Path $ver.FullName 'bin/ninja.exe'
                    if (Test-Path $ninja) { return $ninja }
                }
            }
        }
        return $null
    }

    $ninjaPath = Find-NinjaPath
    $generatorArgs = @()
    if ($ninjaPath) {
        $ninjaDir = Split-Path -Parent $ninjaPath
        if ($ninjaPath -ne 'ninja') { $env:PATH = "$ninjaDir;$env:PATH" }
        $generatorArgs = @('-G','Ninja', "-DCMAKE_MAKE_PROGRAM=$ninjaPath")
        Write-Host "Using Ninja at: $ninjaPath" -ForegroundColor DarkGray
    } else {
        Write-Host "Ninja not found. Install Android SDK 'CMake' component (includes Ninja), or add Ninja to PATH. Falling back may select Visual Studio generator and fail." -ForegroundColor Yellow
    }

    $androidConfigureArgs = @(
        '-S', $Source,
        '-B', $AndroidBuild,
        "-DCMAKE_TOOLCHAIN_FILE=$toolchain",
        '-DANDROID_ABI=arm64-v8a',
        '-DANDROID_PLATFORM=24',
        '-DCMAKE_BUILD_TYPE=Release',
        "-DJUCE_ANDROID_MIN_SDK_VERSION=24",
        "-DJUCE_ANDROID_TARGET_SDK_VERSION=34"
    ) + $generatorArgs

    Write-Host "Configuring Android build folder '$AndroidBuildDir'" -ForegroundColor DarkGray
    # If an existing build dir was configured with a different generator (e.g., Visual Studio), clean it
    $androidCache = Join-Path $AndroidBuild 'CMakeCache.txt'
    if (Test-Path $androidCache) {
        try {
            $cacheContent = Get-Content -Raw $androidCache
            if ($cacheContent -match 'CMAKE_GENERATOR[^=]*=([^\r\n]+)') {
                $prevGen = $Matches[1].Trim()
                if ($prevGen -notmatch 'Ninja') {
                    Write-Host "Detected previous generator '$prevGen' in $AndroidBuild; cleaning directory due to generator change to Ninja" -ForegroundColor Yellow
                    Remove-Item -Recurse -Force $AndroidBuild
                }
            }
        } catch { }
    }

    & cmake @androidConfigureArgs
    if ($LASTEXITCODE -ne 0) { throw "Android CMake configure failed with exit code $LASTEXITCODE" }

    # A no-op build often finalises file generation for some generators
    & cmake --build $AndroidBuild
    # ignore non-zero here; we'll rely on Gradle discovery below

    $gradle = Get-ChildItem -Path $AndroidBuild -Recurse -Filter "settings.gradle" -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($gradle) {
        $proj = Split-Path -Parent $gradle.FullName
        Write-Host "`nAndroid Studio project generated at:" -ForegroundColor Green
        Write-Host $proj
        Write-Host "`nOpen this folder in Android Studio (File > Open)." -ForegroundColor Green
        exit 0
    } else {
        Write-Host "Android build configured, but settings.gradle not found yet. Falling back to host-target probes..." -ForegroundColor Yellow
    }
} else {
    Write-Host "Android NDK not detected (ANDROID_NDK_ROOT/ANDROID_SDK_ROOT). Falling back to host-target probes..." -ForegroundColor Yellow
}

# Fallback: Try host-side JUCE artefact targets (some JUCE versions emit Android project this way)
# Determine if the generator is multi-config to pass a config explicitly (defaults to Release).
$configArg = @()
$defaultConfig = "Release"
$cachePath = Join-Path $Build "CMakeCache.txt"
if (Test-Path $cachePath) {
    $cache = Get-Content -Raw $cachePath
    if ($cache -match "CMAKE_CONFIGURATION_TYPES") {
        $cfg = $env:CONFIGURATION
        if ([string]::IsNullOrWhiteSpace($cfg)) { $cfg = $defaultConfig }
        $configArg = @("--config", $cfg)
        Write-Host "Detected multi-config generator; using configuration '$cfg'" -ForegroundColor DarkGray
    }
}

# Important: do NOT attempt to build the custom 'android_studio' target from this script to avoid recursion
$targetsToTry = @(
    "AudioAnalyzer_Android",
    "AudioAnalyzer_AndroidStudio",
    "AudioAnalyzer_artefacts"
)

$selected = $null
foreach ($t in $targetsToTry) {
    Write-Host "Trying target '$t'..." -ForegroundColor DarkGray
    & cmake --build $Build --target $t @configArg
    if ($LASTEXITCODE -eq 0) {
        $selected = $t
        Write-Host "Built target '$t' successfully" -ForegroundColor DarkGray
        break
    } else {
        Write-Host "Target '$t' failed with exit code $LASTEXITCODE; trying next..." -ForegroundColor Yellow
    }
}

if (-not $selected) {
    Write-Host "Couldn't build a known Android artefact target directly. If you have the Android NDK installed, rerun this script after setting ANDROID_NDK_ROOT or ANDROID_SDK_ROOT." -ForegroundColor Yellow
    throw "No Android artefact target built successfully. Ensure your JUCE checkout supports Android via CMake, or use the Android NDK path flow."
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
