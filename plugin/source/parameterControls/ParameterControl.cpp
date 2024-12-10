#include "ParameterControl.h"

// === Lifecycle ==============================================================
ParameterControl::ParameterControl()
    : parameterName(""), everAttached(false)
{
    bounds = juce::Rectangle<int>(0, 0, 0, 0);
    setSliderStyle(juce::Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    label.listenTo(&slider);
    juce::FontOptions mainFont(12);
    label.setMainFont(mainFont);
    juce::FontOptions postfixFont(9);
    label.setPostfixFont(postfixFont);
    label.updateText(&slider);
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
    slider.setBounds(x, y, width, height - 13);
    label.setBounds(x, y + height - 12, width, 12);
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