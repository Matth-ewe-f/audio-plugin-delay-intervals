#pragma once
#include <functional>
#include <juce_audio_processors/juce_audio_processors.h>
#include "CtmToggle.h"
#include "ParameterToggle.h"

class ParameterToggle : public juce::AudioProcessorValueTreeState::Listener
{
public:
    std::string parameterName;
    CtmToggle toggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>
        attachment;
    std::vector<std::function<void(bool)>> onToggle;

    ParameterToggle();
    ~ParameterToggle() override;

    void setBounds(int x, int y, int width, int height);
    void attachToParameter(juce::AudioProcessorValueTreeState* state,
        std::string newParam);

    void addOnToggleFunction(std::function<void(bool)> function);
    void removeOnToggleFunctions();

    void parameterChanged(const juce::String&, float) override;

private:
    juce::AudioProcessorValueTreeState* treeState;
};