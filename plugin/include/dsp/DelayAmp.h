#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

struct DelayAmp : public juce::AudioProcessorValueTreeState::Listener
{
public:
    juce::SmoothedValue<float> smoothAmplitude;
    juce::SmoothedValue<float> smoothFalloffValue;

    ~DelayAmp() override;

    void listenTo(juce::AudioProcessorValueTreeState*, std::string);
    void parameterChanged(const juce::String&, float) override;
    void setFalloffValue(float value);
    float getAmplitude();

    void reset(int samplesPerBlock);

private:
    juce::AudioProcessorValueTreeState* tree;
    std::string parameterId;
};