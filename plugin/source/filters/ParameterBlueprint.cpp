#include "ParameterBlueprint.h"
#include <format>

using Parameter = juce::AudioProcessorValueTreeState::Parameter;
using Attributes = juce::AudioProcessorValueTreeStateParameterAttributes;

// === Lifecycle ==============================================================
ParameterBlueprint::ParameterBlueprint(std::string post, std::string display)
    : idPostfix(post), displayName(display), useSecondFilterName(false),
    range(0, 1, 1), defaultValue(0), isTwoStepDiscrete(false),
    isDiscrete(false), units(""), showPlus(false), showAlwaysNegative(false),
    maxDecimals(0), onText(""), offText("")
{ }

// === Setters ================================================================
ParameterBlueprint ParameterBlueprint::withUseSecondFilterName(bool use)
{
    useSecondFilterName = use;
    return *this;
}

ParameterBlueprint ParameterBlueprint::withRange
(float min, float max, float step, float skew)
{
    range = juce::NormalisableRange<float>(min, max, step, skew);
    return *this;
}

ParameterBlueprint ParameterBlueprint::withDefault(float defaultVal)
{
    defaultValue = defaultVal;
    return *this;
}

ParameterBlueprint ParameterBlueprint::withTwoStepDiscrete
(std::string on, std::string off)
{
    isTwoStepDiscrete = true;
    onText = on;
    offText = off;
    return *this;
}

ParameterBlueprint ParameterBlueprint::withDiscrete(bool discrete)
{
    isDiscrete = discrete;
    return *this;
}

ParameterBlueprint ParameterBlueprint::withUnits(std::string unitText)
{
    units = unitText;
    return *this;
}

ParameterBlueprint ParameterBlueprint::withShowPlus(bool show)
{
    showPlus = show;
    return *this;
}

ParameterBlueprint ParameterBlueprint::withAlwaysNegative(bool negative)
{
    showAlwaysNegative = negative;
    return *this;
}

ParameterBlueprint ParameterBlueprint::withMaxDecimals(int max)
{
    maxDecimals = max;
    return *this;
}

// === string conversion ======================================================
std::function<juce::String(float, int)>
ParameterBlueprint::getStringFromValueFunction()
{
    if (isTwoStepDiscrete)
    {
        // capture single variables rather than entire struct
        std::string on = onText;
        std::string off = offText;
        return [on, off] (float value, int length)
        {
            juce::ignoreUnused(length);
            return value <= 0 ? off : on;
        };
    }
    else
    {
        // capture single variables rather than entire struct
        std::string unitText = units;
        int decimals = maxDecimals;
        bool plus = showPlus;
        bool negative = showAlwaysNegative;
        return [unitText, decimals, plus, negative] (float value, int length)
        {
            juce::ignoreUnused(length);
            std::string valueString;
            if (decimals >= 2 && value < 10)
                valueString = std::format("{:.2f}", value);
            else if (decimals >= 1 && value < 1000)
                valueString = std::format("{:.1f}", value);
            else
                valueString = std::format("{:.0f}", value);
            if (negative)
                valueString = "-" + valueString;
            else if (value > 0 && plus)
                valueString = "+" + valueString;
            return valueString + " " + unitText;
        };
    }
}

// === Parameter conversion ===================================================
std::unique_ptr<juce::RangedAudioParameter> ParameterBlueprint::create()
{
    Attributes attr;
    attr = attr
        .withDiscrete(isDiscrete || isTwoStepDiscrete)
        .withStringFromValueFunction(getStringFromValueFunction());
    return std::make_unique<Parameter>(
        idPostfix, displayName, range, defaultValue, attr
    );
}

std::unique_ptr<juce::RangedAudioParameter> ParameterBlueprint::create
(std::string filterName, std::string filterParamText)
{
    Attributes attr;
    attr = attr
        .withDiscrete(isDiscrete || isTwoStepDiscrete)
        .withStringFromValueFunction(getStringFromValueFunction());
    return std::make_unique<Parameter>(
        filterName + "-" + idPostfix,
        std::vformat(filterParamText, std::make_format_args(displayName)),
        range,
        defaultValue,
        attr
    );
}

std::string ParameterBlueprint::getIdWithFilterName
(std::string filterName) const
{
    return filterName + "-" + idPostfix;
}