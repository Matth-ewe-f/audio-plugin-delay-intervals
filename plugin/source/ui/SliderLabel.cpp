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

// warning can safely be ignored - float comparison involving no arithmetic
// is perfectly safe
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
void SliderLabel::onInputReturnKey()
{
    double newValue = convertToSliderValue(getText().toStdString());
    if (newValue != __DBL_MIN__)
        attachedSlider->setValue(newValue);
    getParentComponent()->grabKeyboardFocus();
}
#pragma GCC diagnostic pop

double SliderLabel::convertToSliderValue(std::string text)
{
    for (size_t i = 0;i < text.length();i++)
    {
        char c = text[i];
        bool first = i == 0;
        bool last = i == text.length() - 1;
        if (!isdigit(c) && c != '.' && !(first && c == '-')
            && !(last && c == 'k'))
        {
            return __DBL_MIN__;
        }
    }
    double result;
    if (text[text.length() - 1] == 'k')
        result = std::stod(text.substr(0, text.length() - 1)) * 1000;
    else
        result = std::stod(text);
    if (typeNegative && result < 0)
        result *= -1;
    return result;
}

std::string SliderLabel::getSliderValueAsString(juce::Slider* slider)
{
    double valueAsDbl = slider->getValue();
    std::string value;
    if (maxDecimals >= 2 && valueAsDbl < 10)
        value = std::format("{:.2f}", slider->getValue());
    else if (maxDecimals >= 1 && valueAsDbl < 1000)
        value = std::format("{:.1f}", slider->getValue());
    else
        value = std::format("{:.0f}", slider->getValue());
    return typeNegative ? '-' + value : value;
}