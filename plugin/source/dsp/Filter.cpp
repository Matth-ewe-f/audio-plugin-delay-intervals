#include "Filter.h"
typedef juce::dsp::IIR::Coefficients<float> Coefficients; 

Filter::Filter() : Filter(44100, 20, 20000) { }

Filter::Filter(double sampleRate, float low, float high) 
    : lastSampleRate(sampleRate), tempBuffer(128)
{
    setHighPassFrequency(low);
    setLowPassFrequency(high);

    highPassFreq.setCurrentAndTargetValue(low);
    lowPassFreq.setCurrentAndTargetValue(high);
    smoothMix.setCurrentAndTargetValue(1);
}

Filter::~Filter()
{
    tree->removeParameterListener(highPassParam, this);
    tree->removeParameterListener(lowPassParam, this);
    tree->removeParameterListener(mixParam, this);
}

void Filter::attachToParameters(juce::AudioProcessorValueTreeState* state,
    const std::string& highPassParamId, const std::string& lowPassParamId,
    const std::string& mixParamId)
{
    if (highPassParam.compare("") != 0)
    {
        tree->removeParameterListener(highPassParam, this);
    }
    if (lowPassParam.compare("") != 0)
    {
        tree->removeParameterListener(lowPassParam, this);
    }
    if (mixParam.compare("") != 0)
    {
        tree->removeParameterListener(mixParam, this);
    }

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
    {
        highPassFreq.setTargetValue(value);
    }
    else if (param.compare(lowPassParam) == 0)
    {
        lowPassFreq.setTargetValue(value);
    }
    else if (param.compare(mixParam) == 0)
    {
        smoothMix.setTargetValue(value / 100);
    }
}

void Filter::reset()
{
    highPass.reset();
    lowPass.reset();
}

void Filter::prepare(const juce::dsp::ProcessSpec& spec)
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
    {
        setHighPassFrequency(highPassFreq.getNextValue());
    }
    if (lowPassFreq.isSmoothing())
    {
        setLowPassFrequency(lowPassFreq.getNextValue());
    }

    float wet = lowPass.processSample(highPass.processSample(sample));
    float mix = smoothMix.getNextValue();
    return (sample * (1 - mix)) + (wet * mix);
}

void Filter::processSamples(float* samples, size_t length)
{
    if (length == 0)
    {
        return;
    }

    // copy the dry signal to the temporary buffer (for mixing)
    memcpy(tempBuffer.data(), samples, length * sizeof(float));
    
    if (highPassFreq.isSmoothing() || lowPassFreq.isSmoothing())
    {
        size_t processed = 0;
        while (processed < length)
        {
            size_t blockLen = juce::jmin(length - processed, smoothGrain);
            float* ptr = samples + processed;
            juce::dsp::AudioBlock<float> block(&ptr, 1, blockLen);
            juce::dsp::ProcessContextReplacing<float> context(block);
            
            if (highPassFreq.isSmoothing())
            {
                setHighPassFrequency(highPassFreq.skip((int) blockLen));
            }
            if (lowPassFreq.isSmoothing())
            {
                setLowPassFrequency(lowPassFreq.skip((int) blockLen));
            }
            
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

void Filter::setHighPassFrequency(float freq)
{
    highPass.coefficients = Coefficients::makeHighPass(lastSampleRate, freq);
}

void Filter::setLowPassFrequency(float freq)
{
    lowPass.coefficients = Coefficients::makeLowPass(lastSampleRate, freq);
}