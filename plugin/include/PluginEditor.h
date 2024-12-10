#pragma once
#include "PluginProcessor.h"
#include "CtmLookAndFeel.h"
#include "ParameterControl.h"
#include "ParameterToggle.h"

class PluginEditor final : public juce::AudioProcessorEditor
{
public:
    // === Lifecycle ==========================================================
    explicit PluginEditor (PluginProcessor&);
    ~PluginEditor() override;

    // === Graphics ===========================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // === Global Controls (Left Side) ========================================
    ParameterControl delayTime;
    ParameterToggle noTempoSync;
    ParameterToggle tempoSync;
    ParameterControl numIntervals;
    // === Channel Controls ===================================================
    ParameterControl leftFilterFirstLow;
    ParameterControl leftFilterFirstHigh;
    ParameterControl leftFilterSecondLow;
    ParameterControl leftFilterSecondHigh;
    ParameterControl rightFilterFirstLow;
    ParameterControl rightFilterFirstHigh;
    ParameterControl rightFilterSecondLow;
    ParameterControl rightFilterSecondHigh;
    // === Global Controls (Right Side) =======================================
    ParameterControl falloff;
    ParameterControl wetDry;

    // === Layout Constants ===================================================
    inline static const int col1Width { 112 };
    inline static const int col1KnobW { 72 };
    inline static const int col1KnobH { 80 };
    inline static const int col1ToggleW { 34 };
    inline static const int col1ToggleH { 22 };
    inline static const int col1TogglePadX { 2 };
    inline static const int col1TogglePadY { 8 };
    inline static const int col2Width { 320 };
    inline static const int col2Margin { 8 };
    inline static const int col3Width { 64 };
    inline static const int col3Margin { 16 };
    inline static const int height { 256 };
    inline static const int paddingY { 8 };

    // === Layout Functions ===================================================
    void layoutLeftSideGlobals();
    void layoutChannels();
    void layoutRightSideGlobals();

    // === Helper Functions ===================================================
    void addParameterControl(ParameterControl*);

    PluginProcessor& processorRef;
    CtmLookAndFeel lookAndFeel;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
