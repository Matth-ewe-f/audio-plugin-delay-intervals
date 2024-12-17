#include "DelayAmp.h"

DelayAmp::~DelayAmp()
{
    tree->removeParameterListener(parameterId, this);
}

void DelayAmp::listenTo
(juce::AudioProcessorValueTreeState* stateTree, std::string param)
{
    stateTree->addParameterListener(param, this);
    float value = *stateTree->getRawParameterValue(param);
    smoothAmplitude.setCurrentAndTargetValue(value);
    tree = stateTree;
    parameterId = param;
}

void DelayAmp::parameterChanged(const juce::String& param, float value)
{
    juce::ignoreUnused(param);
    smoothAmplitude.setTargetValue(value);
}

float DelayAmp::getAmplitude()
{
    return smoothAmplitude.getNextValue();
}

void DelayAmp::reset(int samplesPerBlock)
{
    smoothAmplitude.reset(samplesPerBlock);
}