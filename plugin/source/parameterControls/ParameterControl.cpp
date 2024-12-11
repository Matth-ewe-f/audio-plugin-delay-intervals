#include "ParameterControl.h"

// === Lifecycle ==============================================================
ParameterControl::ParameterControl()
    : parameterName(""), showLabel(true), titleText(""), everAttached(false)
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
        title.setBounds(x, y, width, 16);
    else
        title.setBounds(x, y, width, 0);
    int sliderY = hasTitle ? y + 20 : y;
    int sliderH = hasTitle ? height - 38 : (showLabel ? height - 18 : height);
    slider.setBounds(x, sliderY, width, sliderH);
    if (showLabel)
        label.setBounds(x, y + height - 16, width, 16);
    else
        label.setBounds(x, y + height, width, 0);
}

void ParameterControl::attachToParameter
(juce::AudioProcessorValueTreeState* stateTree, std::string param)
{
    SliderAttachment* old = attachment.release();
    if (old != nullptr)
        delete old;
    parameterName = param;
    attachment.reset(new SliderAttachment(*stateTree, param, slider));
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