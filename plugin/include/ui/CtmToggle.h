#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

class CtmToggle : public juce::ToggleButton
{
public:
    CtmToggle();

    void parentHierarchyChanged() override;

    void setText(std::string newText);
    void setText(std::string toggleText, std::string untoggleText);
    void setFixedFontSize(float font);

    void setColorOverride(juce::Colour newColour);
    void setColorGradient(juce::Colour color1, juce::Colour color2);

    void setDisplayAlwaysUp(bool isAlwaysUp);
    void setColorAsUntoggled(bool showAsUntoggled);

    void paintButton(juce::Graphics& g, bool hover, bool click) override;

private:
    juce::Colour fillColor;
    bool colorOverriden;
    juce::Colour gradColor;
    bool colorGradient;
    std::string toggledText;
    std::string untoggledText;
    float fontSize;
    bool alwaysUp;
    bool useUntoggledColor;

    juce::Colour getHighlightColor();
    juce::Colour getShadowColor();
};