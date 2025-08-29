cmake_minimum_required(VERSION 3.19)

# Project
project(AudioAnalyzer VERSION 1.2.2 LANGUAGES C CXX)

# Options derived from .jucer
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Android Studio project configuration (used by JUCE's Android exporter)
set(JUCE_ANDROID_MIN_SDK_VERSION 24 CACHE STRING "Android minSdkVersion for JUCE-generated project")
set(JUCE_ANDROID_TARGET_SDK_VERSION 34 CACHE STRING "Android targetSdkVersion for JUCE-generated project")

# Option: run Android Studio generator script at configure/generate time
option(ANDROID_STUDIO_GENERATE_AT_CONFIGURE "Run Generate-AndroidStudioProject.ps1 during CMake configure/generate" OFF)

# Allow overriding local JUCE source path; default to sibling JUCE directory (../JUCE)
set(JUCE_SOURCE_DIR "${CMAKE_SOURCE_DIR}/../JUCE" CACHE PATH "Path to local JUCE checkout (contains CMakeLists.txt and modules/)")

# Try to locate JUCE (prefer a package install, else fall back to subdirectory)
find_package(JUCE CONFIG QUIET)
if (NOT JUCE_FOUND)
    if (EXISTS "${JUCE_SOURCE_DIR}/CMakeLists.txt")
        message(STATUS "JUCE package not found; adding JUCE from ${JUCE_SOURCE_DIR}")
        add_subdirectory("${JUCE_SOURCE_DIR}" JUCE)
    else()
        message(FATAL_ERROR "JUCE not found. Either install JUCE and ensure JUCEConfig.cmake is discoverable, or set JUCE_SOURCE_DIR to your JUCE checkout (path containing CMakeLists.txt). Current JUCE_SOURCE_DIR='${JUCE_SOURCE_DIR}'.")
    endif()
endif()

# rnnoise: now included as a git submodule within this repo
set(RNNOISE_DIR "${CMAKE_SOURCE_DIR}/rnnoise" CACHE PATH "Path to rnnoise project root (defaults to submodule at ./rnnoise)")
if (NOT EXISTS "${RNNOISE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "rnnoise CMakeLists.txt not found at ${RNNOISE_DIR}. Please adjust RNNOISE_DIR.")
endif()
add_subdirectory("${RNNOISE_DIR}" "${CMAKE_BINARY_DIR}/rnnoise_build")

# JUCE GUI application target
juce_add_gui_app(AudioAnalyzer
    COMPANY_NAME "eksit.dk"
    COMPANY_WEBSITE "www.eksit.dk"
    BUNDLE_ID "dk.eksit.AudioAnalyzer"
    PRODUCT_NAME "AudioAnalyzer"
)

# Ensure JUCE generates and exposes the canonical JuceHeader.h for this target
juce_generate_juce_header(AudioAnalyzer)

# Embed resources with JUCE's binary-data target and link to the app
juce_add_binary_data(AudioAnalyzerData
    SOURCES
    Source/audioAnalyzer_large_icon.png
    Source/audioAnalyzer_small_icon.png
        Source/audioanalyzerfirstimage.png
)

# Sources from the .jucer (compile="1")
target_sources(AudioAnalyzer PRIVATE
    # CustomMatPlot module sources
    Source/cmp_downsampler.cpp
    Source/cmp_frame.cpp
    Source/cmp_graph_area.cpp
    Source/cmp_graph_line.cpp
    Source/cmp_grid.cpp
    Source/cmp_legend.cpp
    Source/cmp_label.cpp
    Source/cmp_lookandfeel.cpp
    Source/cmp_plot.cpp
    Source/cmp_trace.cpp

    # App sources
    Source/AudioSettingsModule.cpp
    Source/FFTCtrl.cpp
    Source/FFTModule.cpp
    Source/freqPlotModule.cpp
    Source/helpPage.cpp
    Source/Main.cpp
    Source/MainComponent.cpp
    Source/microphoneAccessPermissionAlert.cpp
    Source/NotchFilter.cpp
    Source/PlotModule.cpp
    Source/SoundProcessorModule.cpp
    Source/SpectrogramComponent.cpp
)

# Include dirs (headerPath in .jucer points to rnnoise; add Source/ and JuceLibraryCode as well)
target_include_directories(AudioAnalyzer PRIVATE
    ${CMAKE_SOURCE_DIR}/Source
    ${CMAKE_SOURCE_DIR}/JuceLibraryCode
    # rnnoise headers: prefer subdir include/, keep root for backward compatibility
    ${RNNOISE_DIR}
    ${RNNOISE_DIR}/include
)

# JUCE compile definitions from .jucer
target_compile_definitions(AudioAnalyzer PRIVATE
    JUCE_STRICT_REFCOUNTEDPOINTER=1
    JUCE_WEB_BROWSER=0
    JUCE_USE_MP3AUDIOFORMAT=1
    JUCE_USE_LAME_AUDIO_FORMAT=1
)

# Link JUCE modules (match modules in .jucer)
target_link_libraries(AudioAnalyzer
    PRIVATE
    AudioAnalyzerData
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_dsp
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
)

# Link rnnoise; handle common target names defensively
if (TARGET rnnoise)
    target_link_libraries(AudioAnalyzer PRIVATE rnnoise)
elseif(TARGET rnnoise_static)
    target_link_libraries(AudioAnalyzer PRIVATE rnnoise_static)
elseif(TARGET RNNoise)
    target_link_libraries(AudioAnalyzer PRIVATE RNNoise)
else()
    message(FATAL_ERROR "rnnoise target was not created by its CMakeLists. Known names not found (rnnoise, rnnoise_static, RNNoise). Please check ${RNNOISE_DIR}.")
endif()

# Platform tweaks
if (APPLE)
    # Example: additional frameworks if needed
    target_link_libraries(AudioAnalyzer PRIVATE "-framework AVFoundation")

    # macOS app icon (.icns) wiring
    set(APP_ICNS_PATH "${CMAKE_SOURCE_DIR}/Resources/AudioAnalyzer.icns")
    if (EXISTS "${APP_ICNS_PATH}")
        set_source_files_properties("${APP_ICNS_PATH}" PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
        target_sources(AudioAnalyzer PRIVATE "${APP_ICNS_PATH}")
        set_target_properties(AudioAnalyzer PROPERTIES MACOSX_BUNDLE_ICON_FILE "AudioAnalyzer.icns")
        message(STATUS "macOS: Using app icon at ${APP_ICNS_PATH}")
    else()
        # Attempt to generate .icns from the large PNG using sips + iconutil
        set(_AA_SRC_ICON_PNG "${CMAKE_SOURCE_DIR}/Source/audioAnalyzer_large_icon.png")
        if (EXISTS "${_AA_SRC_ICON_PNG}")
            find_program(SIPS_EXECUTABLE sips)
            find_program(ICONUTIL_EXECUTABLE iconutil)
            if (SIPS_EXECUTABLE AND ICONUTIL_EXECUTABLE)
                set(_AA_ICONSET_DIR "${CMAKE_BINARY_DIR}/AudioAnalyzer.iconset")
                file(MAKE_DIRECTORY "${_AA_ICONSET_DIR}")
                # Generate standard iconset sizes
                execute_process(COMMAND "${SIPS_EXECUTABLE}" -z 16 16   "${_AA_SRC_ICON_PNG}" --out "${_AA_ICONSET_DIR}/icon_16x16.png")
                execute_process(COMMAND "${SIPS_EXECUTABLE}" -z 32 32   "${_AA_SRC_ICON_PNG}" --out "${_AA_ICONSET_DIR}/icon_16x16@2x.png")
                execute_process(COMMAND "${SIPS_EXECUTABLE}" -z 32 32   "${_AA_SRC_ICON_PNG}" --out "${_AA_ICONSET_DIR}/icon_32x32.png")
                execute_process(COMMAND "${SIPS_EXECUTABLE}" -z 64 64   "${_AA_SRC_ICON_PNG}" --out "${_AA_ICONSET_DIR}/icon_32x32@2x.png")
                execute_process(COMMAND "${SIPS_EXECUTABLE}" -z 128 128 "${_AA_SRC_ICON_PNG}" --out "${_AA_ICONSET_DIR}/icon_128x128.png")
                execute_process(COMMAND "${SIPS_EXECUTABLE}" -z 256 256 "${_AA_SRC_ICON_PNG}" --out "${_AA_ICONSET_DIR}/icon_128x128@2x.png")
                execute_process(COMMAND "${SIPS_EXECUTABLE}" -z 256 256 "${_AA_SRC_ICON_PNG}" --out "${_AA_ICONSET_DIR}/icon_256x256.png")
                execute_process(COMMAND "${SIPS_EXECUTABLE}" -z 512 512 "${_AA_SRC_ICON_PNG}" --out "${_AA_ICONSET_DIR}/icon_256x256@2x.png")
                execute_process(COMMAND "${SIPS_EXECUTABLE}" -z 512 512 "${_AA_SRC_ICON_PNG}" --out "${_AA_ICONSET_DIR}/icon_512x512.png")
                execute_process(COMMAND "${SIPS_EXECUTABLE}" -z 1024 1024 "${_AA_SRC_ICON_PNG}" --out "${_AA_ICONSET_DIR}/icon_512x512@2x.png")
                set(_AA_GEN_ICNS "${CMAKE_BINARY_DIR}/AudioAnalyzer.icns")
                execute_process(
                    COMMAND "${ICONUTIL_EXECUTABLE}" -c icns -o "${_AA_GEN_ICNS}" "${_AA_ICONSET_DIR}"
                    RESULT_VARIABLE _AA_ICNS_RV
                )
                if (_AA_ICNS_RV EQUAL 0 AND EXISTS "${_AA_GEN_ICNS}")
                    set_source_files_properties("${_AA_GEN_ICNS}" PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
                    target_sources(AudioAnalyzer PRIVATE "${_AA_GEN_ICNS}")
                    set_target_properties(AudioAnalyzer PROPERTIES MACOSX_BUNDLE_ICON_FILE "AudioAnalyzer.icns")
                    message(STATUS "macOS: Generated .icns from ${_AA_SRC_ICON_PNG} -> ${_AA_GEN_ICNS}")
                else()
                    message(WARNING "macOS: Failed to generate .icns from ${_AA_SRC_ICON_PNG}")
                endif()
            else()
                message(STATUS "macOS: sips/iconutil not available; cannot auto-generate .icns. Default icon will be used.")
            endif()
        else()
            message(STATUS "macOS: Resources/AudioAnalyzer.icns not found and no Source/audioAnalyzer_large_icon.png; default icon will be used")
        endif()
    endif()
elseif (WIN32)
    # Windows app icon via .rc resource script (with override and on-the-fly generation from PNG)
    # If Resources/appicon.ico is missing, generate it from Source/audioAnalyzer_large_icon.png at configure-time.
    set(AUDIOANALYZER_WIN_ICON "${CMAKE_SOURCE_DIR}/Resources/appicon.ico" CACHE FILEPATH "Path to Windows .ico icon for the app")
    set(AUDIOANALYZER_WIN_RC   "${CMAKE_SOURCE_DIR}/Resources/AudioAnalyzer.rc" CACHE FILEPATH "Path to Windows .rc resource script (optional)")

    # Try to generate an .ico if none exists in Resources/
    if (NOT EXISTS "${AUDIOANALYZER_WIN_ICON}")
        set(_AA_SRC_ICON_PNG "${CMAKE_SOURCE_DIR}/Source/audioAnalyzer_large_icon.png")
        if (EXISTS "${_AA_SRC_ICON_PNG}")
            file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/Resources")
            set(_AA_GEN_ICO "${CMAKE_BINARY_DIR}/Resources/appicon.ico")
            # Locate PowerShell to run a tiny script that wraps the PNG into a single-entry .ico
            if (NOT DEFINED PWSH_EXECUTABLE)
                find_program(PWSH_EXECUTABLE NAMES pwsh powershell)
            endif()
            if (PWSH_EXECUTABLE)
                set(_AA_GEN_ICO_PS "${CMAKE_BINARY_DIR}/generate_ico.ps1")
                file(WRITE "${_AA_GEN_ICO_PS}" "Param([string]$Png,[string]$Ico)\n$ErrorActionPreference='Stop'\nif(-not (Test-Path $Png)){ throw \"PNG not found: $Png\" }\n$png=[IO.File]::ReadAllBytes($Png)\n$ms=New-Object IO.MemoryStream\n$bw=New-Object IO.BinaryWriter($ms)\n# ICONDIR\n$bw.Write([UInt16]0)  # reserved\n$bw.Write([UInt16]1)  # type=1 icon\n$bw.Write([UInt16]1)  # count=1\n# ICONDIRENTRY\n$bw.Write([Byte]0)    # width 0=256\n$bw.Write([Byte]0)    # height 0=256\n$bw.Write([Byte]0)    # color count\n$bw.Write([Byte]0)    # reserved\n$bw.Write([UInt16]0)  # planes\n$bw.Write([UInt16]0)  # bitcount (0 for PNG)\n$bw.Write([UInt32]$png.Length) # bytes in res\n$bw.Write([UInt32]22) # image offset (6+16)\n$bw.Write($png)\n$bw.Flush()\n[IO.File]::WriteAllBytes($Ico,$ms.ToArray())\n")
                execute_process(
                    COMMAND "${PWSH_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass -File "${_AA_GEN_ICO_PS}" -Png "${_AA_SRC_ICON_PNG}" -Ico "${_AA_GEN_ICO}"
                    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
                    RESULT_VARIABLE _AA_GEN_ICO_RV
                )
                if (_AA_GEN_ICO_RV EQUAL 0 AND EXISTS "${_AA_GEN_ICO}")
                    set(AUDIOANALYZER_WIN_ICON "${_AA_GEN_ICO}" CACHE FILEPATH "Path to Windows .ico icon for the app" FORCE)
                    message(STATUS "Windows: Generated .ico from ${_AA_SRC_ICON_PNG} -> ${_AA_GEN_ICO}")
                else()
                    message(WARNING "Windows: Failed to generate .ico from ${_AA_SRC_ICON_PNG}; result=${_AA_GEN_ICO_RV}")
                endif()
            else()
                message(WARNING "Windows: PowerShell not found; cannot generate .ico from PNG")
            endif()
        else()
            message(STATUS "Windows: No Resources/appicon.ico and no Source/audioAnalyzer_large_icon.png; default icon will be used")
        endif()
    endif()

    # Now wire the .rc, generating a minimal one if needed
    if (EXISTS "${AUDIOANALYZER_WIN_ICON}" AND EXISTS "${AUDIOANALYZER_WIN_RC}")
        target_sources(AudioAnalyzer PRIVATE "${AUDIOANALYZER_WIN_RC}")
        message(STATUS "Windows: Using app icon at ${AUDIOANALYZER_WIN_ICON} via ${AUDIOANALYZER_WIN_RC}")
    elseif (EXISTS "${AUDIOANALYZER_WIN_ICON}")
        # Write RC using forward slashes to avoid C-style escape sequences (e.g., \b, \a) in quoted strings
        set(_AA_ICO_NATIVE "${AUDIOANALYZER_WIN_ICON}")
        string(REPLACE "\\" "/" _AA_ICO_NATIVE "${_AA_ICO_NATIVE}")
        set(_AA_GEN_RC "${CMAKE_BINARY_DIR}/AudioAnalyzer_icon.rc")
        file(WRITE "${_AA_GEN_RC}" "IDI_APP_ICON ICON \"${_AA_ICO_NATIVE}\"\n")
        target_sources(AudioAnalyzer PRIVATE "${_AA_GEN_RC}")
        message(STATUS "Windows: Using app icon at ${AUDIOANALYZER_WIN_ICON} via generated RC ${_AA_GEN_RC}")
    else()
        message(STATUS "Windows: app icon not configured; app will use default icon")
    endif()
elseif (UNIX)
    # Ensure pthread is available if toolchain requires explicit linkage
    find_package(Threads REQUIRED)
    target_link_libraries(AudioAnalyzer PRIVATE Threads::Threads)

    # Link libcurl for JUCE URL functionality if available; otherwise disable CURL usage
    find_package(CURL QUIET)
    if (CURL_FOUND)
        target_link_libraries(AudioAnalyzer PRIVATE CURL::libcurl)
    else()
        message(STATUS "libcurl not found; disabling JUCE_USE_CURL to avoid link errors")
        target_compile_definitions(AudioAnalyzer PRIVATE JUCE_USE_CURL=0)
    endif()
endif()

# Install rules (optional)
include(GNUInstallDirs)
install(TARGETS AudioAnalyzer RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} BUNDLE DESTINATION .)

# Convenience target: generate Android Studio (Gradle) project
# This lets you trigger the Android project generation from CMake or CMake GUI.
# If a helper script exists at the repo root, we'll run it; otherwise, we'll
# depend on the JUCE-provided Android artefact target directly.

set(GENERATE_ANDROID_SCRIPT "${CMAKE_SOURCE_DIR}/Generate-AndroidStudioProject.ps1" CACHE FILEPATH "Helper script to generate Android Studio project")
find_program(PWSH_EXECUTABLE NAMES pwsh powershell)

# Try to detect Android NDK path from environment to avoid spurious failures
set(_AA_ANDROID_NDK "")
if (DEFINED ENV{ANDROID_NDK_ROOT} AND EXISTS $ENV{ANDROID_NDK_ROOT})
    file(TO_CMAKE_PATH "$ENV{ANDROID_NDK_ROOT}" _AA_ANDROID_NDK)
elseif (DEFINED ENV{ANDROID_NDK_HOME} AND EXISTS $ENV{ANDROID_NDK_HOME})
    file(TO_CMAKE_PATH "$ENV{ANDROID_NDK_HOME}" _AA_ANDROID_NDK)
elseif (DEFINED ENV{ANDROID_SDK_ROOT} AND EXISTS $ENV{ANDROID_SDK_ROOT})
    # Best-effort: if SDK is present but NDK root not specified, let the script discover it
    set(_AA_ANDROID_NDK "")
endif()

# Optional: run the helper script during configure/generate (CMake GUI 'Generate')
if (ANDROID_STUDIO_GENERATE_AT_CONFIGURE)
    if (EXISTS "${GENERATE_ANDROID_SCRIPT}" AND PWSH_EXECUTABLE)
        if (DEFINED _AA_ANDROID_NDK AND NOT _AA_ANDROID_NDK STREQUAL "")
            execute_process(
                COMMAND "${PWSH_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass -File "${GENERATE_ANDROID_SCRIPT}" -NdkPath "${_AA_ANDROID_NDK}"
                WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
                RESULT_VARIABLE _AA_ANDROID_GEN_RESULT
            )
        else()
            message(STATUS "Android NDK not detected via environment (ANDROID_NDK_ROOT/ANDROID_NDK_HOME). Skipping automatic Android Studio generation at configure.")
            set(_AA_ANDROID_GEN_RESULT 0)
        endif()
        if (NOT _AA_ANDROID_GEN_RESULT EQUAL 0)
            message(WARNING "Android Studio generation script failed with code ${_AA_ANDROID_GEN_RESULT}")
        elseif(NOT _AA_ANDROID_NDK STREQUAL "")
            message(STATUS "Android Studio project generated by ${GENERATE_ANDROID_SCRIPT}")
        endif()
    else()
        message(WARNING "ANDROID_STUDIO_GENERATE_AT_CONFIGURE=ON but script not found or PowerShell not available")
    endif()
endif()

if (EXISTS "${GENERATE_ANDROID_SCRIPT}" AND PWSH_EXECUTABLE)
    if (DEFINED _AA_ANDROID_NDK AND NOT _AA_ANDROID_NDK STREQUAL "")
        add_custom_target(android_studio
            COMMAND "${PWSH_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass -File "${GENERATE_ANDROID_SCRIPT}" -NdkPath "${_AA_ANDROID_NDK}"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            COMMENT "Generating Android Studio project (using detected NDK at ${_AA_ANDROID_NDK})"
            VERBATIM
        )
    else()
        add_custom_target(android_studio
            COMMAND "${PWSH_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass -File "${GENERATE_ANDROID_SCRIPT}"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            COMMENT "Generating Android Studio project (NDK path will be auto-detected by the script)"
            VERBATIM
        )
    endif()
else()
    # Fallback: wire the target to whichever JUCE Android artefact target exists
    if (TARGET AudioAnalyzer_Android)
        add_custom_target(android_studio DEPENDS AudioAnalyzer_Android)
    elseif (TARGET AudioAnalyzer_AndroidStudio)
        add_custom_target(android_studio DEPENDS AudioAnalyzer_AndroidStudio)
    elseif (TARGET AudioAnalyzer_artefacts)
        add_custom_target(android_studio DEPENDS AudioAnalyzer_artefacts)
    else()
        message(STATUS "android_studio target not created: no helper script found and no known JUCE Android artefact target detected at configure time.")
    endif()
endif()
