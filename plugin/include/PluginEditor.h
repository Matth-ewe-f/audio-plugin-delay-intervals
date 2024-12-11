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
    static const int col1Width;
    static const int col1KnobW;
    static const int col1KnobH;
    static const int col1ToggleW;
    static const int col1ToggleH;
    static const int col1TogglePadX;
    static const int col1TogglePadY;
    static const int col2Width;
    static const int col2Margin;
    static const int delayAmpsAreaHeight;
    static const int delayAmpsMarginX;
    static const int delayAmpsMarginY;
    static const int col3Width;
    static const int col3Margin;
    static const int height;
    static const int paddingY;

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
