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
    ParameterControl leftDelayAmps[16];
    int leftDelayAmpsLength = 16;
    ParameterControl rightDelayAmps[16];
    int rightDelayAmpsLength = 16;
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
    inline static const int col2Width { 415 };
    inline static const int col2Margin { 16 };
    inline static const int delayAmpsAreaHeight { 72 };
    inline static const int delayAmpsMarginX { 16 };
    inline static const int delayAmpsMarginY { 14 };
    inline static const int col3Width { 64 };
    inline static const int col3Margin { 16 };
    inline static const int height { 320 };
    inline static const int paddingY { 8 };

    // === Initialization Functions ===========================================
    void setupLeftSideGlobals();
    void setupChannels();
    void setupRightSideGlobals();

    // === Layout Functions ===================================================
    void layoutLeftSideGlobals();
    void layoutChannels();
    void layoutRightSideGlobals();

    // == Drawing Functions ===================================================
    void drawLeftSideGlobals(juce::Graphics&);
    void drawChannels(juce::Graphics&);
    void drawRightSideGlobals(juce::Graphics&);

    // === Helper Functions ===================================================
    void addParameterControl(ParameterControl*);
    void setHorizontalGradient
    (juce::Graphics&, juce::Colour c1, int x1, juce::Colour c2, int x2);
    void setVerticalGradient
    (juce::Graphics&, juce::Colour c1, int y1, juce::Colour c2, int y2);
    void setRadialGradient
    (juce::Graphics&, juce::Colour c1, int cx, int cy, juce::Colour c2, int r,
    int innerR);

    PluginProcessor& processorRef;
    CtmLookAndFeel lookAndFeel;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
