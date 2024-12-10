#include "PluginProcessor.h"
#include "PluginEditor.h"

// === Lifecycle ==============================================================
PluginEditor::PluginEditor (PluginProcessor &p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    juce::ignoreUnused(processorRef);
    setLookAndFeel(&lookAndFeel);
    // Setup left-side global controls
    delayTime.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    delayTime.setTitleText("Delay Time");
    addParameterControl(&delayTime);
    noTempoSync.toggle.setText("MS");
    addAndMakeVisible(noTempoSync.toggle);
    tempoSync.toggle.setText("TP");
    addAndMakeVisible(tempoSync.toggle);
    numIntervals.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    numIntervals.setTitleText("Intervals");
    addParameterControl(&numIntervals);
    // Setup channel controls
    addParameterControl(&leftFilterFirstLow);
    addParameterControl(&leftFilterFirstHigh);
    addParameterControl(&leftFilterSecondLow);
    addParameterControl(&leftFilterSecondHigh);
    addParameterControl(&rightFilterFirstLow);
    addParameterControl(&rightFilterFirstHigh);
    addParameterControl(&rightFilterSecondLow);
    addParameterControl(&rightFilterSecondHigh);
    // Setup right-side global controls
    addParameterControl(&falloff);
    addParameterControl(&wetDry);
    // Setup size
    int w = col1Width + col2Width + (col2Margin * 2) + col3Width
        + (col3Margin * 2);
    int h = height + (paddingY * 2);
    setSize(w, h);
}

PluginEditor::~PluginEditor() { }

// === Graphics ===============================================================
void PluginEditor::paint(juce::Graphics &g)
{
    // draw background
    g.fillAll(findColour(CtmColourIds::normalBgColourId));
    // draw lines separating sections
    g.setColour(juce::Colours::white);
    int x1 = col1Width;
    int x2 = x1 + col2Width + (col2Margin * 2);
    g.drawLine(x1, 0, x1, getHeight());
    g.drawLine(x2, 0, x2, getHeight());
    g.drawLine(x1, getHeight() / 2, x2, getHeight() / 2);
    g.drawRect(0, 0, getWidth(), getHeight(), 1);
    // draw background for delay time controls area
    g.setColour(findColour(CtmColourIds::darkBgColourId));
    int x = (col1Width - (col1ToggleW * 2) - col1TogglePadX) / 2;
    int w = (col1ToggleW * 2) + col1TogglePadX;
    int h = col1KnobH + col1TogglePadY + col1ToggleH;
    int y = paddingY + (((getHeight() - (paddingY * 2)) / 2) - h) / 2;
    g.fillRoundedRectangle(x - 6, y - 6, w + 12, h + 12, 12);
}

void PluginEditor::resized()
{
    layoutLeftSideGlobals();
    layoutChannels();
    layoutRightSideGlobals();
}

// === Layout Functions =======================================================
void PluginEditor::layoutLeftSideGlobals()
{
    int halfHeight = (getHeight() - (paddingY * 2)) / 2;
    int delaySectionH = col1KnobH + col1TogglePadY + col1ToggleH;
    int delaySectionY = paddingY + (halfHeight - delaySectionH) / 2;
    int x = (col1Width - col1KnobW) / 2;
    delayTime.setBounds(x, delaySectionY, col1KnobW, col1KnobH);
    int toggleX = (col1Width - (col1ToggleW * 2) - col1TogglePadX) / 2;
    int toggleY = delaySectionY + col1KnobH + col1TogglePadY;
    noTempoSync.setBounds(toggleX, toggleY, col1ToggleW, col1ToggleH);
    int toggleX2 = toggleX + col1ToggleW + col1TogglePadX;
    tempoSync.setBounds(toggleX2, toggleY, col1ToggleW, col1ToggleH);
    int intervalsY = paddingY + halfHeight + ((halfHeight - col1KnobH) / 2);
    numIntervals.setBounds(x, intervalsY, col1KnobW, col1KnobH);
}

void PluginEditor::layoutChannels()
{

}

void PluginEditor::layoutRightSideGlobals()
{

}

// === Helper Functions =======================================================
void PluginEditor::addParameterControl(ParameterControl* control)
{
    addAndMakeVisible(control->slider);
    addAndMakeVisible(control->label);
    addAndMakeVisible(control->title);
}