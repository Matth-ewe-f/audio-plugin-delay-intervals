#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace ParameterFactory
{

std::unique_ptr<juce::AudioParameterFloat> createDelayAmpParameter
(std::string id, std::string name, float defaultVal);

std::unique_ptr<juce::AudioParameterFloat> createTimeParameter
(std::string id, std::string name, float min, float max, float defaultVal);

std::unique_ptr<juce::AudioParameterFloat> createTimeParameter
(std::string id, std::string name, float min, float max, float step,
float defaultVal);

std::unique_ptr<juce::AudioParameterChoice> createIntChoiceParameter
(std::string id, std::string name, juce::Array<int> options, int defaultIndex);

}