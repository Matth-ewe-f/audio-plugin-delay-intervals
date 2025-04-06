#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "CtmComboBox.h"

class ComboBoxControl
{
public:
    std::string parameterId;
    CtmComboBox comboBox;
    juce::Label title;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>
        attachment;

    ComboBoxControl();
    ~ComboBoxControl();

    inline juce::Rectangle<int> getBounds() { return bounds; }
    void setBounds(juce::Rectangle<int> bounds);
    void setBounds(int x, int y, int width, int height);

    void setTitleText(const std::string& titleText);

    void attachToParameter(juce::AudioProcessorValueTreeState* state,
        std::string newParam);

private:
    juce::Rectangle<int> bounds;
};