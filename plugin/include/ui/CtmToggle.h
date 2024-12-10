#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

class CtmToggle : public juce::ToggleButton
{
public:
    // === Lifecycle ==========================================================
    CtmToggle();
    void parentHierarchyChanged() override;

    // === Setters ============================================================
    void setText(std::string);
    void setText(std::string toggleText, std::string untoggleText);
    void setFixedFontSize(float);
    void setColorOverride(juce::Colour);
    void setColorGradient(juce::Colour, juce::Colour);
    void setDisplayAlwaysUp(bool);
    void setColorAsUntoggled(bool);

    // === Graphics ===========================================================
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
    bool untoggledColor;

    // === Private Helper =====================================================
    juce::Colour getHighlightColor();
    juce::Colour getShadowColor();
};