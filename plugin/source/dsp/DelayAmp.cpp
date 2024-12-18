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
    currentValue = value;
    lastValue = value;
    tree = stateTree;
    parameterId = param;
}

void DelayAmp::parameterChanged(const juce::String& param, float value)
{
    juce::ignoreUnused(param);
    currentValue = value;
}

float DelayAmp::getLastValue()
{
    float result = lastValue;
    lastValue = currentValue;
    return result;
}

float DelayAmp::getCurrentValue()
{
    return currentValue;
}

bool DelayAmp::hasNewValue()
{
    return !juce::approximatelyEqual(currentValue, lastValue);
}