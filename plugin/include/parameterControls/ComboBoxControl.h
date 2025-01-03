#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "CtmComboBox.h"

using ComboBoxAttachment
    = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

class ComboBoxControl
{
public:
    std::string parameterId;
    CtmComboBox comboBox;
    juce::Label title;
    std::unique_ptr<ComboBoxAttachment> attachment;

    // === Lifecycle ==========================================================
    ComboBoxControl();
    ~ComboBoxControl();

    // === Settings ===========================================================
    void setBounds(juce::Rectangle<int>);
    void setBounds(int x, int y, int width, int height);
    inline juce::Rectangle<int> getBounds() { return bounds; }
    void attachToParameter
    (juce::AudioProcessorValueTreeState*, std::string);
    void setTitleText(const std::string&);

private:
    juce::Rectangle<int> bounds;
};