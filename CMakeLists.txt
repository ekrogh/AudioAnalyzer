cmake_minimum_required(VERSION 3.19)

# Project
project(AudioAnalyzer VERSION 1.2.2 LANGUAGES C CXX)

# When using Ninja, suppress CMake's auto-regeneration step which triggers
# the slow "Re-checking globbed directories..." phase on each build.
# This keeps android_* helper targets snappy to execute.
if (CMAKE_GENERATOR MATCHES "Ninja")
    set(CMAKE_SUPPRESS_REGENERATION ON)
endif()

# Options derived from .jucer
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# JUCE: add from submodule at ./JUCE (per JUCE CMake API guidance)
if (NOT EXISTS "${CMAKE_SOURCE_DIR}/JUCE/CMakeLists.txt")
    message(FATAL_ERROR "JUCE submodule not found at ${CMAKE_SOURCE_DIR}/JUCE. Initialize submodules with: git submodule update --init --recursive")
endif()

# Keep JUCE build files contained and avoid building extras/tools
set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(JUCE_BUILD_EXTRAS OFF   CACHE BOOL "" FORCE)
set(JUCE_BUILD_HELPER_TOOLS OFF CACHE BOOL "" FORCE)
if (ANDROID)
    set(JUCE_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
endif()

# Place JUCE's binary directory in a small third-party folder, not at ${CMAKE_BINARY_DIR}/JUCE
add_subdirectory("${CMAKE_SOURCE_DIR}/JUCE" "${CMAKE_BINARY_DIR}/_third_party/juce")

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

# # Ensure JUCE generates and exposes the canonical JuceHeader.h for this target
#juce_generate_juce_header(AudioAnalyzer)

# Remove JUCE binary-data target that referenced missing files and instead compile existing JuceLibraryCode/BinaryData.cpp
# juce_add_binary_data(AudioAnalyzerData
#     SOURCES
#     Source/audioAnalyzer_large_icon.png
#     Source/audioAnalyzer_small_icon.png
#     Source/audioanalyzerfirstimage.png
# )

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

    # Existing generated binary data implementation from JuceLibraryCode
    JuceLibraryCode/BinaryData.cpp
)

# Include dirs (headerPath in .jucer points to rnnoise; add Source/and JuceLibraryCode as well)
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

# Define ANDROID_VERSION_CODE by reading from the .jucer if available.
# You can also override via -DANDROID_VERSION_CODE=123 when configuring.
set(ANDROID_VERSION_CODE "${ANDROID_VERSION_CODE}" CACHE STRING "Android versionCode for macro ANDROID_VERSION_CODE (auto-read from .jucer if empty)")

if (NOT ANDROID_VERSION_CODE)
    set(_JUCER_FILE "${CMAKE_SOURCE_DIR}/AudioAnalyzer.jucer")
    if (EXISTS "${_JUCER_FILE}")
        file(READ "${_JUCER_FILE}" _JUCER_XML)
        # Prefer extraDefs entry like: extraDefs="ANDROID_VERSION_CODE=2"
        string(REGEX MATCH "extraDefs=\"([^\"]*)\"" _matchExtra "${_JUCER_XML}")
        if (_matchExtra)
            set(_extraDefs "${CMAKE_MATCH_1}")
            string(REGEX MATCH "ANDROID_VERSION_CODE=([0-9]+)" _matchAvc "${_extraDefs}")
            if (_matchAvc)
                set(ANDROID_VERSION_CODE "${CMAKE_MATCH_1}")
            endif()
        endif()
        # Fallback to exporter attribute: androidVersionCode="2"
        if (NOT ANDROID_VERSION_CODE)
            string(REGEX MATCH "androidVersionCode=\"([0-9]+)\"" _matchVer "${_JUCER_XML}")
            if (_matchVer)
                set(ANDROID_VERSION_CODE "${CMAKE_MATCH_1}")
            endif()
        endif()
    endif()
endif()

if (ANDROID_VERSION_CODE)
    message(STATUS "Defining ANDROID_VERSION_CODE=${ANDROID_VERSION_CODE}")
    target_compile_definitions(AudioAnalyzer PRIVATE "ANDROID_VERSION_CODE=${ANDROID_VERSION_CODE}")
endif()

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
elseif (WIN32)
    # Nothing special for now; JUCE handles Win32 deps
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

# --- Convenience targets for Android SDK/Gradle and device listing ---
# Default ON so the android_* targets are available without extra flags
option(AA_ENABLE_ANDROID_HELPERS "Add convenience android_* targets (adb/logcat/gradle)" ON)

if (AA_ENABLE_ANDROID_HELPERS)
    set(AA_ANDROID_DIR "${CMAKE_SOURCE_DIR}/Builds/Android")
    set(AA_APP_DIR     "${AA_ANDROID_DIR}/app")
    set(AA_ANDROID_APPLICATION_ID "dk.eksit.audioanalyzer" CACHE STRING "Android applicationId used for logcat filtering")

    if (WIN32)
        set(GRADLEW "${AA_ANDROID_DIR}/gradlew.bat")
    else()
        set(GRADLEW "${AA_ANDROID_DIR}/gradlew")
    endif()

    # Try to locate adb even when it's not on PATH
    find_program(ADB_EXECUTABLE NAMES adb
        HINTS "$ENV{ANDROID_SDK_ROOT}" "$ENV{ANDROID_HOME}" "$ENV{LOCALAPPDATA}/Android/Sdk"
        PATH_SUFFIXES platform-tools
    )

    if (ADB_EXECUTABLE)
        add_custom_target(android_devices
            COMMAND ${ADB_EXECUTABLE} devices
            WORKING_DIRECTORY ${AA_ANDROID_DIR}
            USES_TERMINAL
            COMMENT "Listing connected Android devices")

        add_custom_target(android_logcat
            COMMAND ${ADB_EXECUTABLE} logcat
            WORKING_DIRECTORY ${AA_ANDROID_DIR}
            USES_TERMINAL
            COMMENT "Streaming Android logcat (all)")

        add_custom_target(android_logcat_clear
            COMMAND ${ADB_EXECUTABLE} logcat -c
            WORKING_DIRECTORY ${AA_ANDROID_DIR}
            USES_TERMINAL
            COMMENT "Clearing Android logcat buffer")

        # Note: a per-app filtered logcat target is intentionally omitted to avoid shell quoting issues on Windows
    else()
        message(STATUS "adb not found. Set ANDROID_SDK_ROOT or add platform-tools to PATH to enable android_* convenience targets.")
    endif()

    if (EXISTS "${GRADLEW}")
        add_custom_target(android_assemble_debug
            COMMAND ${GRADLEW} -p ${AA_ANDROID_DIR} :app:assembleDebug_Debug --console=plain
            WORKING_DIRECTORY ${AA_ANDROID_DIR}
            USES_TERMINAL
            COMMENT "Assembling Android Debug APKs via Gradle")

        add_custom_target(android_install_debug
            COMMAND ${GRADLEW} -p ${AA_ANDROID_DIR} :app:installDebug_Debug --console=plain
            WORKING_DIRECTORY ${AA_ANDROID_DIR}
            USES_TERMINAL
            COMMENT "Installing Android Debug APK on connected device")

        add_custom_target(android_clean
            COMMAND ${GRADLEW} -p ${AA_ANDROID_DIR} clean --console=plain
            WORKING_DIRECTORY ${AA_ANDROID_DIR}
            USES_TERMINAL
            COMMENT "Cleaning Android Gradle project")
    else()
        message(STATUS "Gradle wrapper not found at ${GRADLEW}. Android Gradle targets will be unavailable.")
    endif()
endif()
