cmake_minimum_required(VERSION 3.22)
project(Delay-Intervals VERSION 0.1.0)

juce_add_plugin(
    ${PROJECT_NAME}
    COMPANY_NAME Matthewe
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE Mthe
    PLUGIN_CODE DLYI
    FORMATS VST3 AU Standalone
    PRODUCT_NAME ${PROJECT_NAME}
)

target_sources(
    ${PROJECT_NAME}
    PRIVATE
        source/dsp/CircularBuffer.cpp
        source/dsp/DelayAmp.cpp
        source/dsp/Filter.cpp
        source/ui/CtmLookAndFeel.cpp
        source/ui/CtmSlider.cpp
        source/ui/SliderLabel.cpp
        source/ui/CtmToggle.cpp
        source/parameterControls/ParameterControl.cpp
        source/parameterControls/ParameterToggle.cpp
        source/parameterControls/ComboBoxControl.cpp
        source/parameterControls/ParameterFactory.cpp
        source/PluginProcessor.cpp
        source/PluginEditor.cpp
)

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
        include/dsp
        include/ui
        include/parameterControls
        include
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

target_compile_definitions(
    ${PROJECT_NAME}
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)