cmake_minimum_required(VERSION 3.22)
project(VocKitWindows VERSION 0.3.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
include(FetchContent)
FetchContent_Declare(juce GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.10 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(juce)
juce_add_plugin(VocKitFX
    COMPANY_NAME "Beats of Things"
    BUNDLE_ID "com.beatsofthings.vockitfx"
    PLUGIN_MANUFACTURER_CODE BtsT
    PLUGIN_CODE VcFX
    FORMATS VST3
    PRODUCT_NAME "VocKit FX"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE)
juce_generate_juce_header(VocKitFX)
target_sources(VocKitFX PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_include_directories(VocKitFX PRIVATE ../src)
target_compile_definitions(VocKitFX PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(VocKitFX PRIVATE juce::juce_audio_utils juce::juce_dsp juce::juce_recommended_config_flags juce::juce_recommended_warning_flags juce::juce_recommended_lto_flags)
