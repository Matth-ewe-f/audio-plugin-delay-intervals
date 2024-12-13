#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace ParameterFactory
{

std::unique_ptr<juce::AudioParameterFloat> createBasicFloatParameter
(std::string id, std::string name, float min, float max, float step,
float skew, float defaultVal);

std::unique_ptr<juce::AudioParameterFloat> createTimeParameter
(std::string id, std::string name, float min, float max, float defaultVal);

std::unique_ptr<juce::AudioParameterFloat> createTimeParameter
(std::string id, std::string name, float min, float max, float step,
float defaultVal);

std::unique_ptr<juce::AudioParameterChoice> createIntChoiceParameter
(std::string id, std::string name, juce::Array<int> options, int defaultIndex);

}