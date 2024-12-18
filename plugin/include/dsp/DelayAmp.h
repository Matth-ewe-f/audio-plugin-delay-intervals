#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

struct DelayAmp : public juce::AudioProcessorValueTreeState::Listener
{
public:
    ~DelayAmp() override;

    void listenTo(juce::AudioProcessorValueTreeState*, std::string);
    void parameterChanged(const juce::String&, float) override;

    float getLastValue();
    float getCurrentValue();
    bool hasNewValue();

private:
    float lastValue;
    float currentValue;
    juce::AudioProcessorValueTreeState* tree;
    std::string parameterId;
};