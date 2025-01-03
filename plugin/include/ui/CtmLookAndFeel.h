#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
using Colour = juce::Colour;

class CtmLookAndFeel : public juce::LookAndFeel_V4
{
public:
    // === Lifecycle ==========================================================
    CtmLookAndFeel();

    // === Component Layout Overrides =========================================
    juce::Slider::SliderLayout getSliderLayout(juce::Slider&) override;
    // === Component Drawer Overrides =========================================
    void drawRotarySlider
    (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
    const float startAngle, const float endAngle, juce::Slider&) override;
    void drawLinearSlider
    (juce::Graphics&, int x, int y, int w, int h, float pos, float min,
    float max, juce::Slider::SliderStyle, juce::Slider&) override;
    void drawComboBox
    (juce::Graphics&, int w, int h, bool clicked, int buttonX, int buttonY,
    int buttonW, int buttonH, juce::ComboBox&) override;
    void positionComboBoxText(juce::ComboBox&, juce::Label&) override;

private:
    // === Internal Layout Constants ==========================================
    const float rotaryOutlineWidth = 4;

    // === Helper Functions ===================================================
    void drawLinearSliderNoBar
    (juce::Graphics&, int x, int y, int w, int h, float pos, juce::Slider&);
    void drawLinearSliderBar
    (juce::Graphics&, int x, int y, int w, int h, float pos, juce::Slider&);
};

enum CtmColourIds
{
    normalBgColourId,
    darkBgColourId,
    brightBgColourId,
    darkOutlineColourId,
    brightOutlineColourId,
    meterFillColourId,
    BarBackColourId,
    toggledColourId,
    untoggledColourId,
    delayAmpsAreaColourId
};