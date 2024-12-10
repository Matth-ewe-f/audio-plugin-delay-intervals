#pragma once
#include "PluginProcessor.h"

class PluginEditor final : public juce::AudioProcessorEditor
{
public:
    // === Lifecycle ==========================================================
    explicit PluginEditor (PluginProcessor&);
    ~PluginEditor() override;

    // === Graphics ===========================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    PluginProcessor& processorRef;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
