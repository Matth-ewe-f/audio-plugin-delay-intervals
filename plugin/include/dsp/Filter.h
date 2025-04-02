#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

class Filter : public juce::AudioProcessorValueTreeState::Listener
{
public:
    // Lifecycle
    Filter();
    Filter(double sampleRate, float low, float high);
    ~Filter();

    // Parameters
    void attachToParameters(juce::AudioProcessorValueTreeState* treeState,
        const std::string& highPass, const std::string& lowPass,
        const std::string& mix);
    void parameterChanged(const juce::String&, float);

    // Process Audio
    void reset();
    void prepare(const juce::dsp::ProcessSpec&);
    float processSample(float sample);
    void processSamples(float* samples, size_t length);

private:
    juce::dsp::IIR::Filter<float> highPass;
    juce::dsp::IIR::Filter<float> lowPass;

    juce::SmoothedValue<float> highPassFreq;
    juce::SmoothedValue<float> lowPassFreq;
    juce::SmoothedValue<float> smoothMix;
    static constexpr size_t smoothGrain = 10;

    juce::AudioProcessorValueTreeState* tree;
    std::string highPassParam;
    std::string lowPassParam;
    std::string mixParam;

    double lastSampleRate;
    std::vector<float> tempBuffer;

    // Helper Functions
    void setHighPassFrequency(float freq);
    void setLowPassFrequency(float freq);

};