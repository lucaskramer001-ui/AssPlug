cmake_minimum_required(VERSION 3.22)

# Universal Binary (Apple Silicon + Intel) auf dem Mac
if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "" FORCE)
endif()

project(AssPlug VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

# Statische C++-Runtime unter Windows: kein Visual-C++-Redistributable nötig
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4
    GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(AssPlug
    COMPANY_NAME             "AssPlug Audio"
    PLUGIN_MANUFACTURER_CODE Aspl
    PLUGIN_CODE              Aplg
    FORMATS                  VST3
    PRODUCT_NAME             "AssPlug"
    IS_SYNTH                 FALSE
    NEEDS_MIDI_INPUT         FALSE
    NEEDS_MIDI_OUTPUT        FALSE
    IS_MIDI_EFFECT           FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD  FALSE
    VST3_CATEGORIES          Fx EQ)

target_sources(AssPlug PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

# Handschrift für den Schriftzug (optional, siehe README)
set(ASSPLUG_FONT ${CMAKE_CURRENT_SOURCE_DIR}/Resources/RockSalt-Regular.ttf)
if(EXISTS ${ASSPLUG_FONT})
    juce_add_binary_data(AssPlugData SOURCES ${ASSPLUG_FONT})
    target_link_libraries(AssPlug PRIVATE AssPlugData)
    target_compile_definitions(AssPlug PRIVATE ASSPLUG_HAS_FONT=1)
else()
    message(WARNING "Resources/RockSalt-Regular.ttf fehlt. Der Schriftzug nutzt eine Ersatzschrift.")
endif()

target_compile_definitions(AssPlug PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

target_link_libraries(AssPlug
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
