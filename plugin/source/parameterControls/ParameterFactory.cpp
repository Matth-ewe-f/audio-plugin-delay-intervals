#include "ParameterFactory.h"
#include <algorithm>
#include <format>
#include <functional>

namespace ParameterFactory
{

std::unique_ptr<juce::AudioParameterChoice> createBoolParameter(std::string id,
    std::string name, std::string onText, std::string offText,
    float defaultVal)
{
    juce::AudioParameterChoiceAttributes attr;

    auto strFromValue = [onText, offText] (float value, int length)
    {
        juce::ignoreUnused(length);
        return value <= 0 ? offText : onText;
    };
    attr = attr.withStringFromValueFunction(strFromValue);

    auto valueFromStr = [onText] (const juce::String& text)
    {
        juce::String compare(onText);
        compare = compare.toLowerCase().trim();
        return text.toLowerCase().trim().compare(compare) == 0;
    };
    attr = attr.withValueFromStringFunction(valueFromStr);
    
    return std::make_unique<juce::AudioParameterChoice>(id, name,
        juce::StringArray(onText, offText), defaultVal, attr);
}

std::unique_ptr<juce::AudioParameterFloat> createIntParameter(std::string id,
    std::string name, int min, int max, int defaultVal)
{
    juce::NormalisableRange<float> range(min, max, 1);
    return std::make_unique<juce::AudioParameterFloat>(id, name, range,
        defaultVal);
}

std::unique_ptr<juce::AudioParameterFloat> createPercentageParameter
    (std::string id, std::string name, float defaultVal)
{
    juce::NormalisableRange<float> range(0, 100, 0.1f);
    juce::AudioParameterFloatAttributes attr;

    auto strFromValue = [] (float value, int len) 
    {
        juce::ignoreUnused(len);
        return std::format("{:.1f}", value) + "%";
    };
    attr = attr.withStringFromValueFunction(strFromValue);

    auto valueFromStr = [] (const juce::String& text)
    {
        juce::String s = text.trim();
        if (s.endsWith("%"))
            s = s.substring(0, s.length() - 1).trim();
        return attemptStringConvert(s, 0);
    };
    attr = attr.withValueFromStringFunction(valueFromStr);

    return std::make_unique<juce::AudioParameterFloat>(id, name, range,
        defaultVal, attr);
}

// warning can safely be ignored - float comparison involving no arithmetic
// is perfectly safe
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
std::unique_ptr<juce::AudioParameterFloat> createFreqParameter(std::string id,
    std::string name, float defaultVal)
{   
    juce::AudioParameterFloatAttributes attr;

    auto strFromValue = [] (float value, int len)
    {
        juce::ignoreUnused(len);
        if (value >= 1000)
        {
            return std::format("{:.1f}", value / 1000) + " kHz";
        }
        else if (value >= 100)
        {
            return std::format("{:.0f}", value) + " Hz";
        }
        else
        {
            return std::format("{:.01f}", value) + " Hz";
        }
    };
    attr = attr.withStringFromValueFunction(strFromValue);

    auto valueFromStr = [defaultVal] (const juce::String& text)
    {
        juce::String s = text.trim();
        if (s.endsWithIgnoreCase("Hz"))
        {
            s = s.substring(0, s.length() - 2).trim();
        }

        bool k = false;
        if (s.endsWithIgnoreCase("k"))
        {
            k = true;
            s = s.substring(0, s.length() - 1).trim();
        }

        float result = attemptStringConvert(s, -1);
        if (result == -1)
        {
            return defaultVal;
        }
        else
        {
            return k ? result * 1000 : result;
        }
    };
    attr = attr.withValueFromStringFunction(valueFromStr);

    auto denormal = [] (float min, float max, float value)
    {
        return min * pow(2.0f, value * log2(max / min));
    };
    auto normal = [] (float min, float max, float value)
    {
        return log2(value / min) / log2(max / min);
    };
    auto snap = [] (float min, float max, float value)
    {
        juce::ignoreUnused(min, max);
        return std::round(value * 10.0f) / 10.0f;
    };
    juce::NormalisableRange<float> range(20, 20000, denormal, normal, snap);

    return std::make_unique<juce::AudioParameterFloat>(id, name, range,
        defaultVal, attr);
}
#pragma GCC diagnostic pop

std::unique_ptr<juce::AudioParameterFloat> createDelayAmpParameter
    (std::string id, std::string name, float defaultVal)
{
    auto normalize = [] (float min, float max, float value)
    {
        juce::ignoreUnused(min, max);
        return 1.55f - (0.8525f / (0.55f + value));
    };
    auto denormalize = [] (float min, float max, float value)
    {
        juce::ignoreUnused(min, max);
        return (0.8525f / (1.55f - value)) - 0.55f;
    };
    juce::NormalisableRange<float> range(0, 1, denormalize, normalize);

    juce::AudioParameterFloatAttributes attr;
    auto strFromValue = [] (float value, int len)
    {
        juce::ignoreUnused(len);
        return std::format("{:.1f}", value * 100) + "%";
    };
    attr = attr.withStringFromValueFunction(strFromValue);

    return std::make_unique<juce::AudioParameterFloat>(id, name, range,
        defaultVal, attr);
}

std::unique_ptr<juce::AudioParameterFloat> createTimeParameter(std::string id,
    std::string name, float min, float max, float val)
{
    return createTimeParameter(id, name, min, max, 1, val);
}

std::unique_ptr<juce::AudioParameterFloat> createTimeParameter
(std::string id, std::string name, float min, float max, float step, float val)
{
    bool msDecimals = step < 1;
    juce::AudioParameterFloatAttributes attr;

    auto strFromValue = [msDecimals] (float value, int len) {
        juce::ignoreUnused(len);
        if (value < 1000 && msDecimals)
        {
            return std::format("{:.1f}", value) + "ms";
        }
        else if (value < 1000 && !msDecimals)
        {
            return std::format("{:.0f}", value) + "ms";
        }
        else
        {
            return std::format("{:.2f}", value / 1000) + "s";
        }
    };
    attr = attr.withStringFromValueFunction(strFromValue);

    auto valueFromStr = [] (const juce::String& text)
    {
        juce::String s = text.trim();

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
    };
    attr = attr.withValueFromStringFunction(valueFromStr);

    juce::NormalisableRange<float> range(min, max, step);
    return std::make_unique<juce::AudioParameterFloat>(id, name, range, val,
        attr);
}

std::unique_ptr<juce::AudioParameterChoice> createChoiceParameter
    (std::string id, std::string name, juce::StringArray& options,
    int defaultIndex)
{
    return std::make_unique<juce::AudioParameterChoice>(id, name, options,
        defaultIndex);
}

std::unique_ptr<juce::AudioParameterChoice> createIntChoiceParameter
    (std::string id, std::string name, juce::Array<int> options,
    int defaultIndex)
{
    juce::AudioParameterChoiceAttributes attr;
    auto valueFromStr = [options] (const juce::String& s)
    {
        int result = 0;

        // attempt string to integer conversion
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
            // if failure, no handling needed, just return default value
        }
        return result;
    };
    attr = attr.withValueFromStringFunction(valueFromStr);

    juce::StringArray strOptions;
    for (int option : options)
    {
        strOptions.add(std::to_string(option));
    }

    return std::make_unique<juce::AudioParameterChoice>(id, name, strOptions,
        defaultIndex, attr);
}


float attemptStringConvert(const juce::String& text, float valueOnFailure)
{
    try
    {
        juce::String s = text.trim();
        size_t len;
        float attempt = std::stof(s.toStdString(), &len);
        
        if (*(s.toStdString().c_str() + len) == '\0')
        {
            return attempt;
        }
        else
        {
            return valueOnFailure;
        }
    }
    catch (...)
    {
        // no handling necessary, just return failure value
        return valueOnFailure;
    }
}

}