#include "ComboBoxControl.h"
typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment
    ComboBoxAttachment;

ComboBoxControl::ComboBoxControl() : parameterId(""), bounds(0, 0, 0, 0)
{
    title.setFont(juce::FontOptions(14));
    title.setJustificationType(juce::Justification::centred);
}

ComboBoxControl::~ComboBoxControl() { }

void ComboBoxControl::setBounds(juce::Rectangle<int> rect)
{
    setBounds(rect.getX(), rect.getY(), rect.getWidth(), rect.getHeight());
}

void ComboBoxControl::setBounds(int x, int y, int width, int height)
{
    bounds = juce::Rectangle<int>(x, y, width, height);

    if (title.getText().compare("") != 0)
    {
        title.setBounds(x, y, width, 16);
        comboBox.setBounds(x, y + 4 + ((height - 16) / 2), width, 24);
    }
    else
    {
        title.setBounds(x, y, width, 0);
        comboBox.setBounds(x, y + (height / 2) - 12, width, 24);
    }
}

void ComboBoxControl::attachToParameter
    (juce::AudioProcessorValueTreeState* treeState, std::string newParam)
{
    ComboBoxAttachment* old = attachment.release();
    if (old != nullptr)
    {
        delete old;
    }
    
    comboBox.clear();
    auto* param = treeState->getParameter(newParam);
    comboBox.addItemList(param->getAllValueStrings(), 1);
    
    attachment.reset(new ComboBoxAttachment(*treeState, newParam, comboBox));
    parameterId = newParam;
}

void ComboBoxControl::setTitleText(const std::string& str)
{
    title.setText(str, juce::sendNotificationAsync);
    setBounds(bounds);
}