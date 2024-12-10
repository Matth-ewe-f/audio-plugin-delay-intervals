#pragma once
#include <functional>
#include <juce_audio_processors/juce_audio_processors.h>
#include "CtmToggle.h"
#include "ParameterToggle.h"

using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

class ParameterToggle : public juce::AudioProcessorValueTreeState::Listener
{
public:
    std::string parameterName;
    CtmToggle toggle;
    std::unique_ptr<ButtonAttachment> attachment;
    std::vector<std::function<void(bool)>> onToggle;

    // === Lifecycle ==========================================================
    ParameterToggle();
    ~ParameterToggle() override;

    // === Settings ===========================================================
    void setBounds(int x, int y, int width, int height);
    void attachToParameter(juce::AudioProcessorValueTreeState*, std::string);
    void addOnToggleFunction(std::function<void(bool)>);
    void removeOnToggleFunctions();

    // === ValueTreeState Listener ============================================
    void parameterChanged(const juce::String&, float) override;

private:
    juce::AudioProcessorValueTreeState* tree;
};