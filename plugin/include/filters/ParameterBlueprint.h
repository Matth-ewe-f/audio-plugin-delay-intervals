#pragma once
#include <functional>
#include <juce_audio_processors/juce_audio_processors.h>

typedef struct ParameterBlueprint
{
    // === Fields =============================================================
    std::string idPostfix;
    std::string displayName;
    bool useSecondFilterName;
    juce::NormalisableRange<float> range;
    float defaultValue;
    bool isTwoStepDiscrete;
    bool isDiscrete;            // used if !isTwoStepDiscrete
    std::string units;          // used if !isTwoStepDiscrete
    bool showPlus;              // used if !isTwoStepDiscrete
    bool showAlwaysNegative;    // used if !isTwoStepDiscrete
    int maxDecimals;            // used if !isTwoStepDiscrete
    std::string onText;         // used if isTwoStepDiscrete
    std::string offText;        // used if isTwoStepDiscrete

    // === Lifecycle ==========================================================
    ParameterBlueprint(std::string postfix, std::string display);
    ParameterBlueprint(const ParameterBlueprint& other) = default;

    // === Setters ============================================================
    ParameterBlueprint withUseSecondFilterName(bool = true);
    ParameterBlueprint withRange
    (float min, float max, float step = 1, float skew = 1);
    ParameterBlueprint withDefault(float);
    ParameterBlueprint withTwoStepDiscrete(std::string on, std::string off);
    ParameterBlueprint withDiscrete(bool = true);
    ParameterBlueprint withUnits(std::string);
    ParameterBlueprint withShowPlus(bool = true);
    ParameterBlueprint withAlwaysNegative(bool = true);
    ParameterBlueprint withMaxDecimals(int);

    // === String conversion ==================================================
    std::function<juce::String(float, int)> getStringFromValueFunction();

    // === Parameter conversion ===============================================
    std::unique_ptr<juce::RangedAudioParameter> create();
    std::unique_ptr<juce::RangedAudioParameter> create
    (std::string filterName, std::string filterParamText);
    std::string getIdWithFilterName(std::string filterName) const;

}
ParameterBlueprint;