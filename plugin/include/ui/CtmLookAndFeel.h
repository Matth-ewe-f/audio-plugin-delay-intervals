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

private:
    // === Internal Layout Constants ==========================================
    const float rotaryOutlineWidth = 4;
    inline static const Colour darkBgColor { Colour::fromRGB(36, 46, 52) };
    inline static const Colour normalBgColor { Colour::fromRGB(44, 54, 60) };
    inline static const Colour brightBgColor { Colour::fromRGB(54, 72, 82) };
    inline static const Colour darkOutlineColor
        { Colour::fromRGB(22, 28, 30) };
    inline static const Colour brightOutlineColor
        { Colour::fromRGB(200, 200, 200) };
    inline static const Colour rotaryColor { Colour::fromRGB(40, 80, 100) };
    inline static const Colour meterFillColor
        { Colour::fromRGB(57, 125, 227) };
    inline static const Colour toggledColor { Colour::fromRGB(15, 87, 150) };
    inline static const Colour untoggledColor
        { Colour::fromRGB(150, 150, 150) };
};

enum CtmColourIds
{
    normalBgColourId,
    darkBgColourId,
    brightBgColourId,
    darkOutlineColourId,
    brightOutlineColourId,
    meterFillColourId,
    toggledColourId,
    untoggledColourId
};