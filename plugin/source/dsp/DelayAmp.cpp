#include "DelayAmp.h"

DelayAmp::DelayAmp()
    : lastValue(0), currentValue(0), tree(nullptr), parameterId("")
{ }

DelayAmp::~DelayAmp()
{
    tree->removeParameterListener(parameterId, this);
}

void DelayAmp::listenTo
(juce::AudioProcessorValueTreeState* stateTree, std::string param)
{
    stateTree->addParameterListener(param, this);
    currentValue = *stateTree->getRawParameterValue(param);

    if (tree == nullptr && parameterId.compare("") != 0)
    {
        tree->removeParameterListener(parameterId, this);
    }
    else
    {
        lastValue = currentValue;
    }

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