#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "CtmLookAndFeel.h"

class CtmSlider : public juce::Slider
{
public:
    CtmSlider() : useColorOverride(false), enabled(true) { }

    void paint(juce::Graphics& g) override 
    {
        if (useColorOverride || !enabled)
        {
            auto id = CtmColourIds::meterFillColourId;
            juce::Colour oldColour = getLookAndFeel().findColour(id);
            juce::Colour newColor;

            if (enabled)
            {
                newColor = colorOverride;
            }
            else
            {
                newColor = findColour(CtmColourIds::untoggledColourId);
            }

            getLookAndFeel().setColour(id, newColor);
            Slider::paint(g);
            getLookAndFeel().setColour(id, oldColour);
        }
        else
        {
            Slider::paint(g);
        }
    }

    void setColorOverride(juce::Colour newColor)
    {
        useColorOverride = true;
        colorOverride = newColor;
    }

    void setEnabled(bool shouldEnable = true)
    {
        enabled = shouldEnable;
        repaint();
    }

private:
    juce::Colour colorOverride;
    bool useColorOverride;
    bool enabled;
};