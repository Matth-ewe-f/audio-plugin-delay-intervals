#include "PluginProcessor.h"
#include "PluginEditor.h"

// === Layout Constants ===================================================
const int PluginEditor::col1Width = 112;
const int PluginEditor::col1KnobW = 72;
const int PluginEditor::col1KnobH = 80;
const int PluginEditor::col1ToggleW = 34;
const int PluginEditor::col1ToggleH = 22;
const int PluginEditor::col1TogglePadX = 2;
const int PluginEditor::col1TogglePadY = 8;
const int PluginEditor::col2Width = 415;
const int PluginEditor::col2Margin = 16;
const int PluginEditor::delayAmpsAreaHeight = 72;
const int PluginEditor::delayAmpsMarginX = 16;
const int PluginEditor::delayAmpsMarginY = 14;
const int PluginEditor::col3Width = 64;
const int PluginEditor::col3Margin = 16;
const int PluginEditor::height = 320;
const int PluginEditor::paddingY = 8;

// === Lifecycle ==============================================================
PluginEditor::PluginEditor (PluginProcessor &p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    juce::ignoreUnused(processorRef);
    setLookAndFeel(&lookAndFeel);
    // setup components
    setupLeftSideGlobals();
    setupChannels();
    setupRightSideGlobals();
    // set size
    int w = col1Width + col2Width + col3Width;
    int h = height + (paddingY * 2);
    setSize(w, h);
}

PluginEditor::~PluginEditor() { }

// === Initialization Functions ===============================================
void PluginEditor::setupLeftSideGlobals()
{
    delayTime.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    delayTime.setTitleText("Delay Time");
    addParameterControl(&delayTime);
    noTempoSync.toggle.setText("MS");
    addAndMakeVisible(noTempoSync.toggle);
    tempoSync.toggle.setText("TP");
    addAndMakeVisible(tempoSync.toggle);
    numIntervals.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    numIntervals.setTitleText("Intervals");
    addParameterControl(&numIntervals);
}

void PluginEditor::setupChannels()
{
    addParameterControl(&leftFilterFirstLow);
    addParameterControl(&leftFilterFirstHigh);
    addParameterControl(&leftFilterSecondLow);
    addParameterControl(&leftFilterSecondHigh);
    addParameterControl(&rightFilterFirstLow);
    addParameterControl(&rightFilterFirstHigh);
    addParameterControl(&rightFilterSecondLow);
    addParameterControl(&rightFilterSecondHigh);
    for (int i = 0;i < leftDelayAmpsLength;i++)
    {
        leftDelayAmps[i].setShowLabel(false);
        leftDelayAmps[i].setSliderStyle(juce::Slider::LinearBarVertical);
        addParameterControl(&leftDelayAmps[i]);
    }
    for (int i = 0;i < rightDelayAmpsLength;i++)
    {
        rightDelayAmps[i].setShowLabel(false);
        rightDelayAmps[i].setSliderStyle(juce::Slider::LinearBarVertical);
        addParameterControl(&rightDelayAmps[i]);
    }
}

void PluginEditor::setupRightSideGlobals()
{
    addParameterControl(&falloff);
    addParameterControl(&wetDry);
}

// === Graphics ===============================================================
void PluginEditor::paint(juce::Graphics &g)
{
    // draw background
    g.fillAll(findColour(CtmColourIds::normalBgColourId));
    // draw the components of each ui section that need to be drawn
    drawLeftSideGlobals(g);
    drawChannels(g);
    drawRightSideGlobals(g);
    // draw lines separating sections
    g.setColour(juce::Colours::white);
    int x1 = col1Width;
    int x2 = x1 + col2Width;
    g.drawLine(x1, 0, x1, getHeight());
    g.drawLine(x2, 0, x2, getHeight());
    g.drawLine(x1, getHeight() / 2, x2, getHeight() / 2);
    g.drawRect(0, 0, getWidth(), getHeight(), 1);
}

void PluginEditor::resized()
{
    layoutLeftSideGlobals();
    layoutChannels();
    layoutRightSideGlobals();
}

// === Layout Functions =======================================================
void PluginEditor::layoutLeftSideGlobals()
{
    int halfHeight = (getHeight() - (paddingY * 2)) / 2;
    int delaySectionH = col1KnobH + col1TogglePadY + col1ToggleH;
    int delaySectionY = paddingY + (halfHeight - delaySectionH) / 2;
    int x = (col1Width - col1KnobW) / 2;
    delayTime.setBounds(x, delaySectionY, col1KnobW, col1KnobH);
    int toggleX = (col1Width - (col1ToggleW * 2) - col1TogglePadX) / 2;
    int toggleY = delaySectionY + col1KnobH + col1TogglePadY;
    noTempoSync.setBounds(toggleX, toggleY, col1ToggleW, col1ToggleH);
    int toggleX2 = toggleX + col1ToggleW + col1TogglePadX;
    tempoSync.setBounds(toggleX2, toggleY, col1ToggleW, col1ToggleH);
    int intervalsY = paddingY + halfHeight + ((halfHeight - col1KnobH) / 2);
    numIntervals.setBounds(x, intervalsY, col1KnobW, col1KnobH);
}

void PluginEditor::layoutChannels()
{
    int pad;
    if (leftDelayAmpsLength == 8)
        pad = 4;
    else if (leftDelayAmpsLength == 16)
        pad = 2;
    else
        pad = 1;
    int leftY = (getHeight() / 2) - delayAmpsAreaHeight + delayAmpsMarginY;
    int rightY = (getHeight() / 2) + 3;
    int usableW = col2Width - (2 * col2Margin) - delayAmpsMarginX;
    int w = (usableW / leftDelayAmpsLength) - pad;
    int h = delayAmpsAreaHeight - delayAmpsMarginY - 3;
    for (int i = 0;i < leftDelayAmpsLength;i++)
    {
        int startX = col1Width + col2Margin + delayAmpsMarginX;
        if (leftDelayAmpsLength == 8)
            startX -= 2;
        int offsetX = (w + pad) * i;
        leftDelayAmps[i].setBounds(startX + offsetX, leftY, w, h);
        rightDelayAmps[i].setBounds(startX + offsetX, rightY, w, h);
    }
}

void PluginEditor::layoutRightSideGlobals()
{

}

// == Drawing Functions ======================================================
void PluginEditor::drawLeftSideGlobals(juce::Graphics& g)
{
    // draw background for delay time controls area
    g.setColour(findColour(CtmColourIds::darkBgColourId));
    int x = (col1Width - (col1ToggleW * 2) - col1TogglePadX) / 2;
    int w = (col1ToggleW * 2) + col1TogglePadX;
    int h = col1KnobH + col1TogglePadY + col1ToggleH;
    int y = paddingY + (((getHeight() - (paddingY * 2)) / 2) - h) / 2;
    g.fillRoundedRectangle(x - 6, y - 6, w + 12, h + 12, 12);
}

void PluginEditor::drawChannels(juce::Graphics& g)
{
    // draw background for delay amplitude sliders
    g.setColour(findColour(CtmColourIds::delayAmpsAreaColourId));
    int x = col1Width + col2Margin;
    int y = (getHeight() / 2) - delayAmpsAreaHeight;
    int w = col2Width - (2 * col2Margin);
    int h = delayAmpsAreaHeight * 2;
    int r = 12;
    g.fillRoundedRectangle(x, y, w, h, r);
    // draw shadow on background
    int s = 6;
    juce::Colour black = juce::Colours::black.withAlpha(0.3f);
    juce::Colour trans = juce::Colours::transparentBlack;
    setVerticalGradient(g, black, y, trans, y + s);
    g.fillRect(x + r, y, w - 2 * r, s);
    setHorizontalGradient(g, black, x, trans, x + s);
    g.fillRect(x, y + r, s, h - 2 * r);
    setVerticalGradient(g, trans, y + h - s, black, y + h);
    g.fillRect(x + r, y + h - s, w - 2 * r, s);
    setHorizontalGradient(g, trans, x + w - s, black, x + w);
    g.fillRect(x + w - s, y + r, s, h - 2 * r);
    float pi_2 = (float)M_PI_2;
    int d = r * 2;
    juce::Path corners;
    corners.addPieSegment(x, y, d, d, 0, -1 * pi_2, 0);
    setRadialGradient(g, trans, x + r, y + r, black, r, r - s);
    g.fillPath(corners);
    corners.clear();
    corners.addPieSegment(x, y + h - d, d, d, 3 * pi_2, 2 * pi_2, 0);
    setRadialGradient(g, trans, x + r, y + h - r, black, r, r - s);
    g.fillPath(corners);
    corners.clear();
    corners.addPieSegment(x + w - d, y + h - d, d, d, 2 * pi_2, pi_2, 0);
    setRadialGradient(g, trans, x + w - r, y + h - r, black, r, r - s);
    g.fillPath(corners);
    corners.clear();
    corners.addPieSegment(x + w - d, y, d, d, pi_2, 0, 0);
    setRadialGradient(g, trans, x + w - r, y + r, black, r, r - s);
    g.fillPath(corners);
}

void PluginEditor::drawRightSideGlobals(juce::Graphics& g)
{
    juce::ignoreUnused(g);
}

// === Helper Functions =======================================================
void PluginEditor::addParameterControl(ParameterControl* control)
{
    addAndMakeVisible(control->slider);
    addAndMakeVisible(control->label);
    addAndMakeVisible(control->title);
}

void PluginEditor::setHorizontalGradient
(juce::Graphics& g, juce::Colour c1, int x1, juce::Colour c2, int x2)
{
    g.setGradientFill(juce::ColourGradient::horizontal(c1, x1, c2, x2));
}

void PluginEditor::setVerticalGradient
(juce::Graphics& g, juce::Colour c1, int y1, juce::Colour c2, int y2)
{
    g.setGradientFill(juce::ColourGradient::vertical(c1, y1, c2, y2));
}

void PluginEditor::setRadialGradient
(juce::Graphics& g, juce::Colour c1, int cx, int cy, juce::Colour c2, int r,
int innerR)
{
    auto grad = juce::ColourGradient(c1, cx, cy, c2, cx, cy + r, true);
    if (innerR == 0 || innerR >= r)
    {
        g.setGradientFill(grad);
    }
    else
    {
        double proportion = (double) innerR / (double) r;
        grad.addColour(proportion, c1);
        g.setGradientFill(grad);
    }
}