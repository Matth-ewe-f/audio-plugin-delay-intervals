#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

struct DelayAmp : public juce::AudioProcessorValueTreeState::Listener
{
public:
    juce::SmoothedValue<float> smoothAmplitude;

    ~DelayAmp() override;

    void listenTo(juce::AudioProcessorValueTreeState*, std::string);
    void parameterChanged(const juce::String&, float) override;
    float getAmplitude();

    void reset(int samplesPerBlock);

private:
    juce::AudioProcessorValueTreeState* tree;
    std::string parameterId;
};