#include "ParameterToggle.h"
typedef juce::AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;

ParameterToggle::ParameterToggle() : parameterName(""), treeState(nullptr) { }

ParameterToggle::~ParameterToggle()
{
    if (treeState != nullptr)
    {
        treeState->removeParameterListener(parameterName, this);
    }
}

void ParameterToggle::setBounds(int x, int y, int width, int height)
{
    toggle.setBounds(x, y, width, height);
}

void ParameterToggle::attachToParameter
(juce::AudioProcessorValueTreeState* newState, std::string newParam)
{
    ButtonAttachment* old = attachment.release();
    if (old != nullptr)
    {
        delete old;
    }
    if (treeState != nullptr)
    {
        treeState->removeParameterListener(parameterName, this);
    }

    newState->addParameterListener(newParam, this);
    attachment.reset(new ButtonAttachment(*newState, newParam, toggle));
    treeState = newState;
    parameterName = newParam;

    for (std::function<void(bool)> func : onToggle)
    {
        func(*newState->getRawParameterValue(newParam) >= 1);
    }
}

void ParameterToggle::addOnToggleFunction(std::function<void(bool)> func)
{
    onToggle.push_back(func);
}

void ParameterToggle::removeOnToggleFunctions()
{
    onToggle.clear();
}

void ParameterToggle::parameterChanged(const juce::String& param, float value)
{
    juce::ignoreUnused(param);
    toggle.setToggleState(value >= 1, juce::sendNotification);
    
    for (std::function<void(bool)> func : onToggle)
    {
        func(value >= 1);
    }
}