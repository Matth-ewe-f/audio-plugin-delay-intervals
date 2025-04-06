#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class CtmLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CtmLookAndFeel();

    juce::Slider::SliderLayout getSliderLayout(juce::Slider&) override;

    void drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
        float sliderPos, const float startAngle, const float endAngle,
        juce::Slider&) override;
    void drawLinearSlider(juce::Graphics&, int x, int y, int w, int h,
        float pos, float min, float max, juce::Slider::SliderStyle,
        juce::Slider&) override;
    void drawComboBox(juce::Graphics&, int w, int h, bool clicked,
        int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox&)
        override;
    void positionComboBoxText(juce::ComboBox&, juce::Label&) override;

private:
    static constexpr float rotaryOutlineWidth = 4;

    void drawLinearSliderNoBar(juce::Graphics&, int x, int y, int w, int h,
        float pos, juce::Slider&);
    void drawLinearSliderBar(juce::Graphics&, int x, int y, int w, int h,
        float pos, juce::Slider&);

    juce::Colour makeColour(juce::uint8 r, juce::uint8 g, juce::uint8 b) const;
    juce::Colour makeColour(juce::uint8 r, juce::uint8 g, juce::uint8 b,
        juce::uint8 a) const;
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