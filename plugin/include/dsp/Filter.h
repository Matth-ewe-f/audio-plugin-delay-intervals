#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

namespace dsp = juce::dsp;

class Filter : public juce::AudioProcessorValueTreeState::Listener
{
public:
    // === Lifecycle ==========================================================
    Filter();
    Filter(double sampleRate, float low, float high);
    ~Filter();

    // === Parameters =========================================================
    void attachToParameters
    (juce::AudioProcessorValueTreeState*, const std::string& highPass,
    const std::string& lowPass);
    void parameterChanged(const juce::String&, float);

    // === Process Audio ======================================================
    void reset();
    void prepare(const dsp::ProcessSpec&);
    float processSample(float);
    void processSamples(float*, size_t);

private:
    dsp::IIR::Filter<float> highPass;
    dsp::IIR::Filter<float> lowPass;
    juce::SmoothedValue<float> highPassFreq;
    juce::SmoothedValue<float> lowPassFreq;
    double lastSampleRate;
    juce::AudioProcessorValueTreeState* tree;
    std::string highPassParam;
    std::string lowPassParam;
};