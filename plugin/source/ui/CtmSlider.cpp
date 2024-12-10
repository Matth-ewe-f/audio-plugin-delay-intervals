#include "CtmSlider.h"
#include "CtmLookAndFeel.h"

CtmSlider::CtmSlider() : useColorOverride(false), enabled(true) { }

void CtmSlider::paint(juce::Graphics& g)
{
    if (useColorOverride || !enabled)
    {
        // temporarily set the look and feel color to the override color
        auto id = CtmColourIds::meterFillColourId;
        juce::Colour oldColour = getLookAndFeel().findColour(id);
        juce::Colour newColor;
        if (enabled)
            newColor = colorOverride;
        else
            newColor = findColour(CtmColourIds::untoggledColourId);
        getLookAndFeel().setColour(id, newColor);
        // paint the component
        Slider::paint(g);
        // reset the look and feel colour
        getLookAndFeel().setColour(id, oldColour);
    }
    else
    {
        Slider::paint(g);
    }
}

void CtmSlider::setColorOverride(juce::Colour color)
{
    useColorOverride = true;
    colorOverride = color;
}

void CtmSlider::setEnabled(bool isEnabled)
{
    enabled = isEnabled;
    repaint();
}