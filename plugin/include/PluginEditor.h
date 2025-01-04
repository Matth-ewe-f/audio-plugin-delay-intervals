#pragma once
#include "PluginProcessor.h"
#include "CtmLookAndFeel.h"
#include "CtmToggle.h"
#include "ParameterControl.h"
#include "ParameterToggle.h"
#include "ComboBoxControl.h"

class PluginEditor final :
    public juce::AudioProcessorEditor,
    public juce::AudioProcessorValueTreeState::Listener,
    public juce::AsyncUpdater,
    private juce::Timer
{
public:
    // === Lifecycle ==========================================================
    explicit PluginEditor (PluginProcessor&);
    ~PluginEditor() override;

    // === Graphics ===========================================================
    void paint(juce::Graphics&) override;
    void paintOverChildren(juce::Graphics&) override;
    void resized() override;

    // === Superclass Functions ===============================================
    void parameterChanged(const juce::String&, float) override;
    void handleAsyncUpdate() override;
    void timerCallback() override;

private:
    // === Static Constants ===================================================
    static constexpr int leftDelayAmpsLength = 16;
    static constexpr int rightDelayAmpsLength = 16;
    // === Variables ==========================================================
    PluginProcessor& processorRef;
    CtmLookAndFeel lookAndFeel;
    // === Global Controls (Left Side) ========================================
    ParameterControl delayTime;
    ComboBoxControl delayTimeSync;
    juce::Label delayTimeSyncLabel;
    juce::Colour normalTextColor;
    ParameterToggle noTempoSync;
    ParameterToggle tempoSync;
    ParameterControl numIntervals;
    ParameterToggle loopButton;
    // === Channel Controls ===================================================
    ParameterControl leftFilterLow;
    ParameterControl leftFilterHigh;
    ParameterControl leftFilterMix;
    ParameterControl rightFilterLow;
    ParameterControl rightFilterHigh;
    ParameterControl rightFilterMix;
    ParameterControl leftDelayAmps[leftDelayAmpsLength];
    ParameterControl rightDelayAmps[rightDelayAmpsLength];
    CtmToggle resetLeft;
    CtmToggle resetRight;
    CtmToggle matchLeft;
    CtmToggle matchRight;
    // === Global Controls (Right Side) =======================================
    ParameterToggle linkAmps;
    ParameterToggle linkFilters;
    ParameterControl falloff;
    ParameterControl dryMix;
    ParameterControl wetMix;
    // === Info for Drawing Controls ==========================================
    bool tempoSyncOn;
    int tempoSyncNoteIndex;
    int numDelayAmps;
    float wetRatio;
    float autoFalloffRate;

    // === Layout Constants ===================================================
    static const int col1Width;
    static const int col1KnobW;
    static const int col1KnobH;
    static const int syncToggleW;
    static const int syncToggleH;
    static const int syncTogglePadX;
    static const int syncTogglePadY;
    static const int loopToggleW;
    static const int col2Width;
    static const int col2Margin;
    static const int delayAmpsAreaHeight;
    static const int delayAmpsMarginX;
    static const int delayAmpsMarginY;
    static const int filterY;
    static const int filterMargin;
    static const int filterKnobW;
    static const int filterKnobH;
    static const int filterMixMargin;
    static const int col2ButtonW;
    static const int col2ButtonH;
    static const int col2ButtonPad;
    static const int col2ButtonMargin;
    static const int col3Width;
    static const int col3KnobW;
    static const int col3KnobH;
    static const int col3WetDryH;
    static const int col3KnobMargin;
    static const int col3ToggleW;
    static const int col3ToggleH;
    static const int col3ToggleMargin;
    static const int col3ToggleLabelH;
    static const int height;
    static const int paddingY;

    // === Initialization Functions ===========================================
    void setupLeftSideGlobals();
    void setupChannels();
    void setupRightSideGlobals();

    // === Layout Functions ===================================================
    void layoutLeftSideGlobals();
    void layoutChannelFilters();
    void layoutDelayAmps();
    void layoutRightSideGlobals();

    // == Drawing Functions ===================================================
    void drawLeftSideGlobals(juce::Graphics&);
    void drawChannels(juce::Graphics&);
    void drawChannelLabels(juce::Graphics&);
    void drawDelayAmpBeatMarkers(juce::Graphics&);
    void drawRightSideGlobals(juce::Graphics&);

    // === Helper Functions ===================================================
    void addParameterControl(ParameterControl*);
    void addComboBoxControl(ComboBoxControl*);
    void setNoteValueDelayLabel(int index);
    void setMillisecondsDisplay(int milliseconds);
    void setHorizontalGradient
    (juce::Graphics&, juce::Colour c1, int x1, juce::Colour c2, int x2);
    void setVerticalGradient
    (juce::Graphics&, juce::Colour c1, int y1, juce::Colour c2, int y2);
    void setRadialGradient
    (juce::Graphics&, juce::Colour c1, int cx, int cy, juce::Colour c2, int r,
    int innerR);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
