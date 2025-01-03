#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class CtmComboBox : public juce::ComboBox
{
public:
    void mouseEnter(const juce::MouseEvent&) override { repaint(); }
    void mouseExit(const juce::MouseEvent&) override { repaint(); }
};