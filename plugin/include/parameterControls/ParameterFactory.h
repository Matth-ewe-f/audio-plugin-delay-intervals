#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace ParameterFactory
{

std::unique_ptr<juce::AudioParameterChoice> createBoolParameter(std::string id,
    std::string name, std::string onText, std::string offText,
    float defaultVal);

std::unique_ptr<juce::AudioParameterFloat> createIntParameter(std::string id,
    std::string name, int min, int max, int defaultVal);

std::unique_ptr<juce::AudioParameterFloat> createPercentageParameter
    (std::string id, std::string name, float defaultVal);

std::unique_ptr<juce::AudioParameterFloat> createFreqParameter
    (std::string id, std::string name, float defaultVal);

std::unique_ptr<juce::AudioParameterFloat> createDelayAmpParameter
    (std::string id, std::string name, float defaultVal);

std::unique_ptr<juce::AudioParameterFloat> createTimeParameter(std::string id,
    std::string name, float min, float max, float defaultVal);

std::unique_ptr<juce::AudioParameterFloat> createTimeParameter(std::string id,
    std::string name, float min, float max, float step, float defaultVal);

std::unique_ptr<juce::AudioParameterChoice> createChoiceParameter
    (std::string id, std::string name, juce::StringArray&, int defaultIdx);

std::unique_ptr<juce::AudioParameterChoice> createIntChoiceParameter
    (std::string id, std::string name, juce::Array<int> options,
    int defaultIndex);

float attemptStringConvert(const juce::String& text, float valueOnFailure);

}