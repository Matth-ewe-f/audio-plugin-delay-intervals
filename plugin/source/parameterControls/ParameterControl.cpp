#include "ParameterControl.h"

// === Lifecycle ==============================================================
ParameterControl::ParameterControl()
    : parameterName(""), showLabel(true), titleText(""), tightText(false),
    everAttached(false)
{
    bounds = juce::Rectangle<int>(0, 0, 0, 0);
    setSliderStyle(juce::Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    label.listenTo(&slider);
    juce::FontOptions mainFont(14);
    label.setMainFont(mainFont);
    juce::FontOptions postfixFont(11);
    label.setPostfixFont(postfixFont);
    label.updateText(&slider);
    title.setFont(mainFont);
    title.setJustificationType(juce::Justification::centred);
}

ParameterControl::~ParameterControl() { }

// === Settings ===============================================================
void ParameterControl::setBounds(juce::Rectangle<int> b)
{
    setBounds(b.getX(), b.getY(), b.getWidth(), b.getHeight());
}

void ParameterControl::setBounds(int x, int y, int width, int height)
{
    bounds = juce::Rectangle<int>(x, y, width, height);
    bool hasTitle = titleText.compare("") != 0;
    if (hasTitle)
        title.setBounds(x, y, width, tightText ? 14 : 16);
    else
        title.setBounds(x, y, width, 0);
    int sliderY = hasTitle ? (tightText ? y + 17 : y + 20) : y;
    int sliderH = (showLabel ? height - (tightText ? 15 : 18) : height);
    sliderH -= sliderY - y;
    slider.setBounds(x, sliderY, width, sliderH);
    int labelH = showLabel ? (tightText ? 14 : 16) : 0;
    label.setBounds(x, y + height - labelH, width, labelH);
}

void ParameterControl::attachToParameter
(juce::AudioProcessorValueTreeState* stateTree, std::string param)
{
    // delete the old attachment
    SliderAttachment* old = attachment.release();
    if (old != nullptr)
        delete old;
    // attach to the new parameter
    parameterName = param;
    attachment.reset(new SliderAttachment(*stateTree, param, slider));
    juce::RangedAudioParameter* p = stateTree->getParameter(param);
    if (auto choice = dynamic_cast<juce::AudioParameterChoice*>(p))
        label.setChoicesArrayForChoiceParameter(choice->getAllValueStrings());
    // handle first attachment flag
    if (!everAttached)
        label.updateText(&slider);
    everAttached = true;
}

void ParameterControl::setSliderStyle(juce::Slider::SliderStyle style)
{
    slider.setSliderStyle(style);
}

void ParameterControl::setShowLabel(bool show)
{
    showLabel = show;
    setBounds(bounds);
}

void ParameterControl::setTitleText(std::string s)
{
    titleText = s;
    title.setText(s, juce::NotificationType::sendNotificationAsync);
    setBounds(bounds);
}

void ParameterControl::setTightText(bool tight)
{
    tightText = tight;
    juce::FontOptions mainFont(12);
    label.setMainFont(mainFont);
    title.setFont(mainFont);
    juce::FontOptions postfixFont(9);
    label.setPostfixFont(postfixFont);
    setBounds(bounds);
}