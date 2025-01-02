#include "Filter.h"

using Coefficients = juce::dsp::IIR::Coefficients<float>;

// === Lifecycle ==============================================================
Filter::Filter() : Filter(44100, 20, 20000) { }

Filter::Filter(double sampleRate, float low, float high)
    : smoothGrain(10), tempBuffer(128)
{
    highPass.coefficients = Coefficients::makeHighPass(sampleRate, low);
    highPassFreq.setCurrentAndTargetValue(low);
    lowPass.coefficients = Coefficients::makeLowPass(sampleRate, high);
    lowPassFreq.setCurrentAndTargetValue(high);
    smoothMix.setCurrentAndTargetValue(1);
}

Filter::~Filter()
{
    tree->removeParameterListener(highPassParam, this);
    tree->removeParameterListener(lowPassParam, this);
    tree->removeParameterListener(mixParam, this);
}

// === Parameters =============================================================
void Filter::attachToParameters
(juce::AudioProcessorValueTreeState* state, const std::string& highPassParamId,
const std::string& lowPassParamId, const std::string& mixParamId)
{
    if (highPassParam.compare("") != 0)
        tree->removeParameterListener(highPassParam, this);
    if (lowPassParam.compare("") != 0)
        tree->removeParameterListener(highPassParam, this);
    if (mixParam.compare("") != 0)
        tree->removeParameterListener(highPassParam, this);
    state->addParameterListener(highPassParamId, this);
    state->addParameterListener(lowPassParamId, this);
    state->addParameterListener(mixParamId, this);
    highPassFreq.setTargetValue(*state->getRawParameterValue(highPassParamId));
    lowPassFreq.setTargetValue(*state->getRawParameterValue(lowPassParamId));
    smoothMix.setTargetValue(*state->getRawParameterValue(mixParamId) / 100);
    tree = state;
    highPassParam = highPassParamId;
    lowPassParam = lowPassParamId;
    mixParam = mixParamId;
}

void Filter::parameterChanged(const juce::String& param, float value)
{
    if (param.compare(highPassParam) == 0)
        highPassFreq.setTargetValue(value);
    else if (param.compare(lowPassParam) == 0)
        lowPassFreq.setTargetValue(value);
    else if (param.compare(mixParam) == 0)
        smoothMix.setTargetValue(value / 100);
}

// === Process Audio ==========================================================
void Filter::reset()
{
    highPass.reset();
    lowPass.reset();
}

void Filter::prepare(const dsp::ProcessSpec& spec)
{
    highPass.prepare(spec);
    lowPass.prepare(spec);
    highPassFreq.reset((int) spec.maximumBlockSize);
    lowPassFreq.reset((int) spec.maximumBlockSize);
    tempBuffer.resize(spec.maximumBlockSize);
    lastSampleRate = spec.sampleRate;
}

float Filter::processSample(float sample)
{
    if (highPassFreq.isSmoothing())
        highPass.coefficients = makeHighPass(highPassFreq.getNextValue());
    if (lowPassFreq.isSmoothing())
        lowPass.coefficients = makeLowPass(lowPassFreq.getNextValue());
    float wet = lowPass.processSample(highPass.processSample(sample));
    float mix = smoothMix.getNextValue();
    return (sample * (1 - mix)) + (wet * mix);
}

void Filter::processSamples(float* samples, size_t length)
{
    // copy the dry signal to the temporary buffer (for mixing)
    for (size_t i = 0;i < length;i++)
        tempBuffer[i] = samples[i];
    // process the signal, smoothing frequency changes if necessary
    if (highPassFreq.isSmoothing() || lowPassFreq.isSmoothing())
    {
        size_t processed = 0;
        while (processed < length)
        {
            // create the block of samples to process
            size_t blockLen = juce::jmin(length - processed, smoothGrain);
            float* ptr = samples + processed;
            juce::dsp::AudioBlock<float> block(&ptr, 1, blockLen);
            juce::dsp::ProcessContextReplacing<float> context(block);
            // handle frequency smoothing
            if (highPassFreq.isSmoothing())
            {
                float f = highPassFreq.skip((int) blockLen);
                highPass.coefficients = makeHighPass(f);
            }
            if (lowPassFreq.isSmoothing())
            {
                float f = lowPassFreq.skip((int) blockLen);
                lowPass.coefficients = makeLowPass(f);
            }
            // process the samples
            lowPass.process(context);
            highPass.process(context);
            processed += blockLen;
        }
    }
    else
    {
        juce::dsp::AudioBlock<float> block(&samples, 1, length);
        juce::dsp::ProcessContextReplacing<float> context(block);
        lowPass.process(context);
        highPass.process(context);
    }
    // mix the dry and wet signal
    for (size_t i = 0;i < length;i++)
    {
        float mix = smoothMix.getNextValue();
        samples[i] = (samples[i] * mix) + (tempBuffer[i] * (1 - mix));
    }
}

// === Private ================================================================
juce::ReferenceCountedObjectPtr<dsp::IIR::Coefficients<float>>
Filter::makeHighPass(float freq)
{
    return Coefficients::makeHighPass(lastSampleRate, freq);
}

juce::ReferenceCountedObjectPtr<dsp::IIR::Coefficients<float>>
Filter::makeLowPass(float freq)
{
    return Coefficients::makeLowPass(lastSampleRate, freq);
}