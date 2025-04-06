#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "CtmSlider.h"
#include "SliderLabel.h"

class ParameterControl
{
public:
    std::string parameterName;
    CtmSlider slider;
    SliderLabel label;
    juce::Label title;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
        attachment;

    ParameterControl();
    ~ParameterControl();

    inline juce::Rectangle<int> getBounds() { return bounds; }
    void setBounds(juce::Rectangle<int> bounds);
    void setBounds(int x, int y, int width, int height);

    void attachToParameter(juce::AudioProcessorValueTreeState* state,
        std::string newParam);
    
    void setSliderStyle(juce::Slider::SliderStyle style);
    void setShowLabel(bool show = true);
    void setTitleText(std::string titleText);
    void setTightText(bool tight = true);

private:
    juce::Rectangle<int> bounds;
    bool showLabel;
    std::string titleText;
    bool tightText;
    bool everAttached;

};