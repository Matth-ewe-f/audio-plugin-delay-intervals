#include "PluginProcessor.h"
#include "PluginEditor.h"

// === Lifecycle ==============================================================
PluginEditor::PluginEditor (PluginProcessor &p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setSize(400, 300);
}

PluginEditor::~PluginEditor() { }

// === Graphics ===============================================================
void PluginEditor::paint(juce::Graphics &g)
{
    auto colorId = juce::ResizableWindow::backgroundColourId;
    g.fillAll(getLookAndFeel().findColour(colorId));
    g.setColour(juce::Colours::white);
    g.setFont(15.0f);
    auto justify = juce::Justification::centred;
    g.drawFittedText("Hello World!", getLocalBounds(), justify, 1);
}

void PluginEditor::resized()
{
    // control layout here
}