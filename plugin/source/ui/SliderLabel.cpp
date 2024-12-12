#include <string>
#include <format>
#include "SliderLabel.h"

// === Lifecycle ==============================================================
SliderLabel::SliderLabel() :
    prefix(""), postfix(""), maxDecimals(0), showPlus(false),
    typeNegative(false)
{
    setJustification(juce::Justification::centred);
    setSelectAllWhenFocused(true);
    onReturnKey = [this]{ onInputReturnKey(); };
}

// === Listener ===============================================================
void SliderLabel::listenTo(juce::Slider* slider)
{
    attachedSlider = slider;
    slider->addListener(this);
}

// === Text Manipulation ======================================================
void SliderLabel::setPrefix(std::string s)
{
    prefix = s;
}

void SliderLabel::setPostfix(std::string s)
{
    postfix = s;
}

void SliderLabel::updateText(juce::Slider* slider)
{
    std::string value = getSliderValueAsString(slider);
    if (slider->getValue() > 0 && showPlus)
    {
        value = '+' + value;
    }
    setFont(mainFont);
    setText(prefix + value, juce::dontSendNotification);
    moveCaretToEnd();
    setFont(postfixFont);
    insertTextAtCaret(postfix);
}

void SliderLabel::setTypeNegativeValues(bool typeNegativeValues)
{
    typeNegative = typeNegativeValues;
}

void SliderLabel::setMaxDecimals(int decimals)
{
    maxDecimals = decimals;
}

void SliderLabel::setShowPlusForPositive(bool show)
{
    showPlus = show;
}

void SliderLabel::setChoicesArrayForChoiceParameter(juce::StringArray arr)
{
    choices = arr;
}

// === Font Setters ===========================================================
void SliderLabel::setMainFont(const juce::FontOptions& font)
{
    mainFont = font;
}

void SliderLabel::setPostfixFont(const juce::FontOptions& font)
{
    postfixFont = font;
}

// === Focus Functions ========================================================
void SliderLabel::focusGained(juce::Component::FocusChangeType changeType)
{
    setFont(mainFont);
    setText(getSliderValueAsString(attachedSlider));
    TextEditor::focusGained(changeType);
}

void SliderLabel::focusLost(juce::Component::FocusChangeType changeType)
{
    juce::ignoreUnused(changeType);
    updateText(attachedSlider);
}

// === Private ================================================================
void SliderLabel::sliderValueChanged(juce::Slider* slider)
{
    updateText(slider);
}

void SliderLabel::onInputReturnKey()
{
    attachedSlider->setValue(convertToSliderValue(getText()));
    getParentComponent()->grabKeyboardFocus();
}

double SliderLabel::convertToSliderValue(const juce::String& text)
{
    return attachedSlider->valueFromTextFunction(text);
}

std::string SliderLabel::getSliderValueAsString(juce::Slider* slider)
{
    double valueAsDbl = slider->getValue();
    std::string value = slider->getTextFromValue(valueAsDbl).toStdString();
    return typeNegative ? '-' + value : value;
}