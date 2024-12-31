#include "Filter.h"

using Coefficients = juce::dsp::IIR::Coefficients<float>;

// === Lifecycle ==============================================================
Filter::Filter() : Filter(44100, 20, 20000) { }

Filter::Filter(double sampleRate, float low, float high) : smoothGrain(10)
{
    highPass.coefficients = Coefficients::makeHighPass(sampleRate, low);
    highPassFreq.setCurrentAndTargetValue(low);
    lowPass.coefficients = Coefficients::makeLowPass(sampleRate, high);
    lowPassFreq.setCurrentAndTargetValue(high);
}

Filter::~Filter()
{
    tree->removeParameterListener(highPassParam, this);
    tree->removeParameterListener(lowPassParam, this);
}

// === Parameters =============================================================
void Filter::attachToParameters
(juce::AudioProcessorValueTreeState* state, const std::string& highPassParamId,
const std::string& lowPassParamId)
{
    state->addParameterListener(highPassParamId, this);
    state->addParameterListener(lowPassParamId, this);
    highPassFreq.setTargetValue(*state->getRawParameterValue(highPassParamId));
    lowPassFreq.setTargetValue(*state->getRawParameterValue(lowPassParamId));
    tree = state;
    highPassParam = highPassParamId;
    lowPassParam = lowPassParamId;
}

void Filter::parameterChanged(const juce::String& param, float value)
{
    if (param.compare(highPassParam) == 0)
        highPassFreq.setTargetValue(value);
    else if (param.compare(lowPassParam) == 0)
        lowPassFreq.setTargetValue(value);
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
    lastSampleRate = spec.sampleRate;
}

float Filter::processSample(float sample)
{
    if (highPassFreq.isSmoothing())
    {
        float f = highPassFreq.getNextValue();
        double s = lastSampleRate;
        highPass.coefficients = Coefficients::makeFirstOrderHighPass(s, f);
    }
    if (lowPassFreq.isSmoothing())
    {
        float f = lowPassFreq.getNextValue();
        double s = lastSampleRate;
        lowPass.coefficients = Coefficients::makeFirstOrderLowPass(s, f);
    }
    float wet = lowPass.processSample(highPass.processSample(sample));
    return wet;
}

void Filter::processSamples(float* samples, size_t length)
{
    // if either filter is changing its frequency, smooth that change
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