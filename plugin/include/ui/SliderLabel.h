#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

class SliderLabel :
    public juce::TextEditor,
    public juce::Slider::Listener
{
public:
    // === Lifecycle ==========================================================
    SliderLabel();

    // === Listener ===========================================================
    void listenTo(juce::Slider*);

    // === Text Manipulation ==================================================
    void setPrefix(std::string s);
    void setPostfix(std::string s);
    void updateText(juce::Slider*);
    void setTypeNegativeValues(bool);
    void setMaxDecimals(int);
    void setShowPlusForPositive(bool);

    // === Focus Functions ====================================================
    void focusGained(juce::Component::FocusChangeType) override;
    void focusLost(juce::Component::FocusChangeType) override;

    // === Font Setters =======================================================
    void setMainFont(const juce::FontOptions&);
    void setPostfixFont(const juce::FontOptions&);
private:
    std::string prefix;
    std::string postfix;
    juce::FontOptions mainFont;
    juce::FontOptions postfixFont;
    juce::Slider* attachedSlider;
    int maxDecimals;
    bool showPlus;
    bool typeNegative;

    // === Private Helper =====================================================
    void sliderValueChanged(juce::Slider*) override;
    void onInputReturnKey();
    double convertToSliderValue(std::string);
    std::string getSliderValueAsString(juce::Slider*);
};