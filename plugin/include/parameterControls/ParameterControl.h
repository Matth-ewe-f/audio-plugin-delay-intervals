#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "CtmSlider.h"
#include "SliderLabel.h"

using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

class ParameterControl
{
public:
    std::string parameterName;
    CtmSlider slider;
    SliderLabel label;
    juce::Label title;
    std::unique_ptr<SliderAttachment> attachment;

    // === Lifecycle ==========================================================
    ParameterControl();
    ~ParameterControl();

    // === Settings ===========================================================
    void setBounds(juce::Rectangle<int>);
    void setBounds(int x, int y, int width, int height);
    inline juce::Rectangle<int> getBounds() { return bounds; }
    void attachToParameter
    (juce::AudioProcessorValueTreeState*, std::string);
    void setSliderStyle(juce::Slider::SliderStyle);
    void setShowLabel(bool = true);
    void setTitleText(std::string);
    void setTightText(bool = true);

private:
    juce::Rectangle<int> bounds;
    bool showLabel;
    std::string titleText;
    bool tightText;
    bool everAttached;

};