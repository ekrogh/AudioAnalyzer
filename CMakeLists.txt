cmake_minimum_required(VERSION 3.19)

# Project
project(AudioAnalyzer VERSION 1.2.2 LANGUAGES C CXX)

# Options derived from .jucer
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

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
