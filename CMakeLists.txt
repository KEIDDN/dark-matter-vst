cmake_minimum_required(VERSION 3.22)

project(DarkMatter VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS deployment target")

add_subdirectory(extern/juce)

juce_add_plugin(DarkMatter
    COMPANY_NAME "Keiddn"
    PRODUCT_NAME "Dark Matter"
    PLUGIN_MANUFACTURER_CODE Keid
    PLUGIN_CODE Dkm1
    FORMATS Standalone
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    VST3_CATEGORIES Reverb
)

target_sources(DarkMatter PRIVATE
    source/PluginProcessor.cpp
    source/PluginEditor.cpp
    source/dsp/ReverbEngine.cpp
)

target_compile_definitions(DarkMatter PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(DarkMatter PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_opengl
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

# Offline DSP correctness check — not part of the plugin, not built by default.
add_executable(ReverbSanityTest EXCLUDE_FROM_ALL
    tools/reverb_sanity_test/main.cpp
    source/dsp/ReverbEngine.cpp
)
target_link_libraries(ReverbSanityTest PRIVATE
    juce::juce_dsp
    juce::juce_recommended_config_flags
)
