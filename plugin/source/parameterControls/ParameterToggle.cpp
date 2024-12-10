#include "ParameterToggle.h"

// === Lifecycle ==============================================================
ParameterToggle::ParameterToggle() : parameterName(""), tree(nullptr) { }

ParameterToggle::~ParameterToggle()
{
    if (tree != nullptr)
        tree->removeParameterListener(parameterName, this);
}

// === Settings ===============================================================
void ParameterToggle::setBounds(int x, int y, int width, int height)
{
    toggle.setBounds(x, y, width, height);
}

void ParameterToggle::attachToParameter
(juce::AudioProcessorValueTreeState* stateTree, std::string parameter)
{
    ButtonAttachment* old = attachment.release();
    if (old != nullptr)
        delete old;
    if (tree != nullptr)
        tree->removeParameterListener(parameterName, this);
    tree = stateTree;
    parameterName = parameter;
    tree->addParameterListener(parameterName, this);
    attachment.reset(new ButtonAttachment(*stateTree, parameter, toggle));
    for (std::function<void(bool)> func : onToggle)
    {
        func(*stateTree->getRawParameterValue(parameter) >= 1);
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

// === ValueTreeState Listener ================================================
void ParameterToggle::parameterChanged(const juce::String& param, float value)
{
    juce::ignoreUnused(param);
    toggle.setToggleState(value >= 1, juce::sendNotification);
    for (std::function<void(bool)> func : onToggle)
    {
        func(value >= 1);
    }
}