#include "Filter.h"

using Coefficients = juce::dsp::IIR::Coefficients<float>;

// === Lifecycle ==============================================================
Filter::Filter() : Filter(44100, 20, 20000) { }

Filter::Filter(double sampleRate, float low, float high)
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
    return lowPass.processSample(highPass.processSample(sample));
}