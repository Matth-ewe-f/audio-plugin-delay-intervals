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
    explicit PluginEditor(PluginProcessor& p);
    ~PluginEditor() override;

    void paint(juce::Graphics& g) override;
    void paintOverChildren(juce::Graphics& g) override;
    void resized() override;

    void parameterChanged(const juce::String& name, float value) override;

    void handleAsyncUpdate() override;
    void timerCallback() override;

private:
    static constexpr int leftDelayAmpsLength = 16;
    static constexpr int rightDelayAmpsLength = 16;

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
    static const int col3KnobMargin;
    static const int col3ToggleW;
    static const int col3ToggleH;
    static const int col3ToggleMargin;
    static const int col3ToggleLabelH;
    static const int height;
    static const int paddingY;

    PluginProcessor& processorRef;
    CtmLookAndFeel lookAndFeel;
    
    ParameterControl delayTime;
    ComboBoxControl delayTimeSync;
    juce::Label delayTimeSyncLabel;
    juce::Colour normalTextColor;
    ParameterToggle noTempoSync;
    ParameterToggle tempoSync;
    ParameterControl numIntervals;
    ParameterToggle loopButton;
    
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
    
    ParameterToggle linkAmps;
    ParameterToggle linkFilters;
    ParameterControl falloff;
    ParameterControl wetDry;
    
    bool tempoSyncOn;
    int tempoSyncNoteIndex;
    int numDelayAmps;
    float wetRatio;
    float autoFalloffRate;

    void initializeParameters();

    void setupLeftSideGlobals();
    void setupChannelIntervals();
    void setupChannelFilters();
    void setupChannelButtons();
    void setupRightSideGlobals();

    void layoutLeftSideGlobals();
    void layoutChannelIntervals();
    void layoutChannelFilters();
    void layoutChannelButtons();
    void layoutRightSideGlobals();

    void drawLeftSideGlobals(juce::Graphics& g);
    void drawChannelIntervals(juce::Graphics& g);
    void drawChannelFilters(juce::Graphics& g);
    void drawChannelButtons(juce::Graphics& g);
    void drawChannelLabels(juce::Graphics& g);
    void drawDelayAmpBeatMarkers(juce::Graphics& g);
    void drawRightSideGlobals(juce::Graphics& g);

    void addParameterControl(ParameterControl* control);
    void addComboBoxControl(ComboBoxControl* control);

    void setNoteValueDelayLabel();
    void setMillisecondsDisplay(int milliseconds);

    void setHorizontalGradient(juce::Graphics& g, juce::Colour c1, int x1,
        juce::Colour c2, int x2);
    void setVerticalGradient(juce::Graphics& g, juce::Colour c1, int y1,
        juce::Colour c2, int y2);
    void setRadialGradient(juce::Graphics& g, juce::Colour c1, int cx, int cy,
        juce::Colour c2, int r, int innerR);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};
