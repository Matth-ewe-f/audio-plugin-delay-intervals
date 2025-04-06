#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

class SliderLabel : public juce::TextEditor, public juce::Slider::Listener
{
public:
    SliderLabel();

    void listenTo(juce::Slider* slider);

    void setPrefix(std::string s);
    void setPostfix(std::string s);
    void updateText(juce::Slider* slider);

    void setTypeNegativeValues(bool typeNegatives);
    void setMaxDecimals(int max);
    void setShowPlusForPositive(bool show);
    void setChoicesArrayForChoiceParameter(juce::StringArray choicesArray);

    void setMainFont(const juce::FontOptions& font);
    void setPostfixFont(const juce::FontOptions& font);

    void focusGained(juce::Component::FocusChangeType changeType) override;
    void focusLost(juce::Component::FocusChangeType changeType) override;

private:
    std::string prefix;
    std::string postfix;
    juce::FontOptions mainFont;
    juce::FontOptions postfixFont;
    juce::Slider* attachedSlider;
    int maxDecimals;
    bool showPlus;
    bool typeNegative;
    juce::StringArray choices;

    void sliderValueChanged(juce::Slider* slider) override;
    void onInputReturnKey();
    double convertToSliderValue(const juce::String& string);
    std::string getSliderValueAsString(juce::Slider* slider);
};