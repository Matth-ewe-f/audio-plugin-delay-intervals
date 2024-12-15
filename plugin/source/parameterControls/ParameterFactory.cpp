#include "ParameterFactory.h"
#include <format>
#include <functional>

namespace ParameterFactory
{

std::unique_ptr<juce::AudioParameterFloat> createPercentageParameter
(std::string id, std::string name, float defaultVal)
{
    juce::AudioParameterFloatAttributes attr;
    attr = attr.withStringFromValueFunction([] (float value, int len) 
    {
        juce::ignoreUnused(len);
        return std::format("{:.1f}", value) + "%";
    });
    attr = attr.withValueFromStringFunction([] (const juce::String& text)
    {
        juce::String s = text.trim();
        if (s.endsWith("%"))
            s = s.substring(0, s.length() - 1).trim();
        return attemptStringConvert(s, 0);
    });
    juce::NormalisableRange<float> range(0, 100, 0.1f);
    return std::make_unique<juce::AudioParameterFloat>(
        id, name, range, defaultVal, attr
    );
}

std::unique_ptr<juce::AudioParameterFloat> createDelayAmpParameter
(std::string id, std::string name, float defaultVal)
{
    auto normalize = [] (float min, float max, float value)
    {
        juce::ignoreUnused(min, max);
        return juce::jmax(0.0f, 1.75f - (1.3022f / (value + 0.7363f)));
    };
    auto denormalize = [] (float min, float max, float value)
    {
        juce::ignoreUnused(min, max);
        return value <= 0 ? 0 : (1.3022f / (1.75f - value)) - 0.7363f;
    };
    juce::NormalisableRange<float> range(0, 1, denormalize, normalize);
    return std::make_unique<juce::AudioParameterFloat>(
        id, name, range, defaultVal
    );
}

std::unique_ptr<juce::AudioParameterFloat> createTimeParameter
(std::string id, std::string name, float min, float max, float val)
{
    return createTimeParameter(id, name, min, max, 1, val);
}

std::unique_ptr<juce::AudioParameterFloat> createTimeParameter
(std::string id, std::string name, float min, float max, float step, float val)
{
    bool msDecimals = step < 1;
    juce::AudioParameterFloatAttributes attr;
    attr = attr.withStringFromValueFunction([msDecimals] (float value, int len)
    {
        juce::ignoreUnused(len);
        if (value < 1000 && msDecimals)
            return std::format("{:.1f}", value) + "ms";
        else if (value < 1000 && !msDecimals)
            return std::format("{:.0f}", value) + "ms";
        else
            return std::format("{:.2f}", value / 1000) + "s";
    });
    attr = attr.withValueFromStringFunction([] (const juce::String& text)
    {
        juce::String s = text.trim();
        // check for units
        bool seconds = false;
        if (s.endsWithIgnoreCase("ms"))
        {
            s = s.substring(0, s.length() - 2).trim();
        }
        else if (s.endsWithIgnoreCase("s"))
        {
            s = s.substring(0, s.length() - 1).trim();
            seconds = true;
        }
        float result = attemptStringConvert(s, 0);
        return seconds ? result * 1000 : result;
    });
    juce::NormalisableRange<float> range(min, max, step);
    return std::make_unique<juce::AudioParameterFloat>(
        id, name, range, val, attr
    );
}

std::unique_ptr<juce::AudioParameterChoice> createIntChoiceParameter
(std::string id, std::string name, juce::Array<int> options, int defaultIndex)
{
    juce::AudioParameterChoiceAttributes attr;
    attr = attr.withValueFromStringFunction([options] (const juce::String& s)
    {
        int result = 0;
        // attempt conversion
        try {
            size_t len;
            int entered = std::stoi(s.trim().toStdString(), &len);
            if (*(s.toStdString().c_str() + len) == '\0')
            {
                // if success, return option the entered int is closest to
                int closest = INT32_MAX;
                int closestIndex = 0;
                for (int i = 0;i < options.size();i++)
                {
                    int d = abs(options[i] - entered);
                    if (d < closest)
                    {
                        closest = d;
                        closestIndex = i;
                    }
                }
                result = closestIndex;
            }
        }
        catch (...)
        {
            // no handling needed, just return result with default value
        }
        return result;
    });
    juce::StringArray strOptions;
    for (int option : options)
        strOptions.add(std::to_string(option));
    return std::make_unique<juce::AudioParameterChoice>(
        id, name, strOptions, defaultIndex, attr
    );
}


float attemptStringConvert(const juce::String& text, float valueOnFailure)
{
    try
    {
        juce::String s = text.trim();
        size_t len;
        float attempt = std::stof(s.toStdString(), &len);
        if (*(s.toStdString().c_str() + len) == '\0')
            return attempt;
        else
            return valueOnFailure;
    }
    catch (...)
    {
        // no handling necessary, just return failure value
        return valueOnFailure;
    }
}

}