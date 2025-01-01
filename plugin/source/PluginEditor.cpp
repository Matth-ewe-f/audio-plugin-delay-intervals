#include "PluginProcessor.h"
#include "PluginEditor.h"

// === Layout Constants ===================================================
const int PluginEditor::col1Width = 112;
const int PluginEditor::col1KnobW = 72;
const int PluginEditor::col1KnobH = 80;
const int PluginEditor::toggleW = 34;
const int PluginEditor::toggleH = 22;
const int PluginEditor::togglePadX = 2;
const int PluginEditor::togglePadY = 8;
const int PluginEditor::col2Width = 411;
const int PluginEditor::col2Margin = 16;
const int PluginEditor::delayAmpsAreaHeight = 64;
const int PluginEditor::delayAmpsMarginX = 14;
const int PluginEditor::delayAmpsMarginY = 12;
const int PluginEditor::filterKnobW = 58;
const int PluginEditor::filterKnobH = 68;
const int PluginEditor::filterMixMargin = 20;
const int PluginEditor::col3Width = 112;
const int PluginEditor::col3KnobW = 72;
const int PluginEditor::col3KnobH = 80;
const int PluginEditor::height = 352;
const int PluginEditor::paddingY = 8;

// === Lifecycle ==============================================================
PluginEditor::PluginEditor (PluginProcessor &p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setWantsKeyboardFocus(true);
    setLookAndFeel(&lookAndFeel);
    processorRef.tree.addParameterListener("num-intervals", this);
    processorRef.tree.addParameterListener("dry-wet", this);
    processorRef.tree.addParameterListener("falloff", this);
    // setup components
    setupLeftSideGlobals();
    setupChannels();
    setupRightSideGlobals();
    // set size
    int w = col1Width + col2Width + col3Width;
    int h = height + (paddingY * 2);
    setSize(w, h);
}

PluginEditor::~PluginEditor()
{
    processorRef.tree.removeParameterListener("num-intervals", this);
    processorRef.tree.removeParameterListener("dry-wet", this);
    processorRef.tree.removeParameterListener("falloff", this);
    setLookAndFeel(nullptr);
}

// === Initialization Functions ===============================================
void PluginEditor::setupLeftSideGlobals()
{
    delayTime.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    delayTime.setTitleText("Delay Time");
    delayTime.attachToParameter(&processorRef.tree, "delay-time");
    addParameterControl(&delayTime);
    noTempoSync.toggle.setText("MS");
    noTempoSync.toggle.setRadioGroupId(1, juce::dontSendNotification);
    addAndMakeVisible(noTempoSync.toggle);
    tempoSync.toggle.setText("TP");
    tempoSync.toggle.setRadioGroupId(1, juce::dontSendNotification);
    tempoSync.attachToParameter(&processorRef.tree, "tempo-sync");
    ParameterToggle* noTempoSyncPtr = &noTempoSync;
    tempoSync.addOnToggleFunction([noTempoSyncPtr] (bool b)
    {
        noTempoSyncPtr->toggle.setToggleState(!b, juce::sendNotification);
    });
    addAndMakeVisible(tempoSync.toggle);
    numIntervals.attachToParameter(&processorRef.tree, "num-intervals");
    numIntervals.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    numIntervals.setTitleText("Intervals");
    addParameterControl(&numIntervals);
    loopButton.toggle.setText("LOOP");
    loopButton.attachToParameter(&processorRef.tree, "loop");
    addAndMakeVisible(&loopButton.toggle);
}

void PluginEditor::setupChannels()
{
    juce::AudioProcessorValueTreeState* tree = &processorRef.tree;
    // get processor parameters that influence layout
    int n = (int)*tree->getRawParameterValue("num-intervals");
    numDelayAmps = n == 0 ? 8 : (n == 1 ? 16 : 32);
    wetRatio = *tree->getRawParameterValue("dry-wet") / 100;
    float f = *tree->getRawParameterValue("falloff");
    autoFalloffRate = 1 - (f / 100);
    // setup filter controls
    leftFilterLow.setTitleText("Low");
    leftFilterLow.setTightText();
    leftFilterLow.attachToParameter(&processorRef.tree, "left-high-pass");
    addParameterControl(&leftFilterLow);
    leftFilterHigh.setTitleText("High");
    leftFilterHigh.setTightText();
    leftFilterHigh.attachToParameter(&processorRef.tree, "left-low-pass");
    addParameterControl(&leftFilterHigh);
    leftFilterMix.setTitleText("Mix");
    leftFilterMix.setTightText();
    leftFilterMix.attachToParameter(&processorRef.tree, "left-filter-mix");
    addParameterControl(&leftFilterMix);
    rightFilterLow.setTitleText("Low");
    rightFilterLow.setTightText();
    rightFilterLow.attachToParameter(&processorRef.tree, "right-high-pass");
    addParameterControl(&rightFilterLow);
    rightFilterHigh.setTitleText("High");
    rightFilterHigh.setTightText();
    rightFilterHigh.attachToParameter(&processorRef.tree, "right-low-pass");
    addParameterControl(&rightFilterHigh);
    rightFilterMix.setTitleText("Mix");
    rightFilterMix.setTightText();
    rightFilterMix.attachToParameter(&processorRef.tree, "right-filter-mix");
    addParameterControl(&rightFilterMix);
    // setup delay amplitude sliders
    for (int i = 0;i < leftDelayAmpsLength;i++)
    {
        leftDelayAmps[i].setShowLabel(false);
        leftDelayAmps[i].setSliderStyle(juce::Slider::LinearBarVertical);
        std::string id = processorRef.getIdForLeftIntervalAmp(i);
        leftDelayAmps[i].attachToParameter(tree, id);
        addParameterControl(&leftDelayAmps[i]);
    }
    for (int i = 0;i < rightDelayAmpsLength;i++)
    {
        rightDelayAmps[i].setShowLabel(false);
        rightDelayAmps[i].setSliderStyle(juce::Slider::LinearBarVertical);
        std::string id = processorRef.getIdForRightIntervalAmp(i);
        rightDelayAmps[i].attachToParameter(tree, id);
        addParameterControl(&rightDelayAmps[i]);
    }
}

void PluginEditor::setupRightSideGlobals()
{
    falloff.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    falloff.setTitleText("Auto-Falloff");
    falloff.attachToParameter(&processorRef.tree, "falloff");
    addParameterControl(&falloff);
    wetDry.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    wetDry.setTitleText("Mix");
    wetDry.attachToParameter(&processorRef.tree, "dry-wet");
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
    drawChannelLabels(g);
    drawRightSideGlobals(g);
    // draw lines separating sections
    g.setColour(findColour(CtmColourIds::brightOutlineColourId));
    int x1 = col1Width;
    int x2 = x1 + col2Width;
    g.drawLine(x1, 0, x1, getHeight());
    g.drawLine(x2, 0, x2, getHeight());
    g.drawLine(x1, getHeight() / 2, x2, getHeight() / 2);
    g.drawRect(0, 0, getWidth(), getHeight(), 1);
}

void PluginEditor::paintOverChildren(juce::Graphics& g)
{
    drawDelayAmpBeatMarkers(g);
}

void PluginEditor::resized()
{
    layoutLeftSideGlobals();
    layoutChannelFilters();
    layoutDelayAmps();
    layoutRightSideGlobals();
}

// === Listener ===============================================================
void PluginEditor::parameterChanged(const juce::String& param, float value)
{
    int v = (int) value;
    if (param.compare("num-intervals") == 0)
        numDelayAmps = v == 0 ? 8 : (v == 1 ? 16 : 32);
    if (param.compare("dry-wet") == 0)
        wetRatio = value / 100;
    if (param.compare("falloff") == 0)
        autoFalloffRate = 1 - (value / 100);
    triggerAsyncUpdate();
}

void PluginEditor::handleAsyncUpdate()
{
    layoutDelayAmps();
    repaint();
}

// === Layout Functions =======================================================
void PluginEditor::layoutLeftSideGlobals()
{
    // delay time controls
    int halfHeight = (getHeight() - (paddingY * 2)) / 2;
    int delaySectionH = col1KnobH + togglePadY + toggleH;
    int delaySectionY = paddingY + 2 * ((halfHeight - delaySectionH) / 3);
    int x = (col1Width - col1KnobW) / 2;
    delayTime.setBounds(x, delaySectionY, col1KnobW, col1KnobH);
    int noTempoX = (col1Width - (toggleW * 2) - togglePadX) / 2;
    int delayToggleY = delaySectionY + col1KnobH + togglePadY;
    noTempoSync.setBounds(noTempoX, delayToggleY, toggleW, toggleH);
    int tempoX = noTempoX + toggleW + togglePadX;
    tempoSync.setBounds(tempoX, delayToggleY, toggleW, toggleH);
    // number of intervals controls
    int intervalsH = col1KnobH + togglePadY + toggleH;
    int intervalsY = paddingY + halfHeight + ((halfHeight - intervalsH) / 3);
    numIntervals.setBounds(x, intervalsY, col1KnobW, col1KnobH);
    int loopX = (col1Width - toggleW) / 2;
    int loopY = intervalsY + col1KnobH + togglePadY;
    loopButton.setBounds(loopX, loopY, toggleW, toggleH);
}

void PluginEditor::layoutChannelFilters()
{
    // calculate dimensions and shared positions
    int xStart = col1Width + col2Margin;
    int filterAreaW = (col2Width - (2 * col2Margin));
    int filterW = filterKnobW * 3 + filterMixMargin;
    int x1 = xStart + (filterAreaW - filterW) / 2;
    int y1 = (((getHeight() / 2) - delayAmpsAreaHeight) - filterKnobH) / 2 + 9;
    int y2 = (getHeight() / 2) + delayAmpsAreaHeight;
    y2 = y2 + 9 + (getHeight() - y2 - filterKnobH) / 2;
    // lay out filters
    leftFilterLow.setBounds(x1, y1, filterKnobW, filterKnobH);
    rightFilterLow.setBounds(x1, y2, filterKnobW, filterKnobH);
    int x2 = x1 + filterKnobW;
    leftFilterHigh.setBounds(x2, y1, filterKnobW, filterKnobH);
    rightFilterHigh.setBounds(x2, y2, filterKnobW, filterKnobH);
    int x3 = x2 + filterKnobW + filterMixMargin;
    leftFilterMix.setBounds(x3, y1, filterKnobW, filterKnobH);
    rightFilterMix.setBounds(x3, y2, filterKnobW, filterKnobH);
}

void PluginEditor::layoutDelayAmps()
{
    // calculate the dimensions and shared positions
    int pad = 32 / numDelayAmps;
    int leftY = (getHeight() / 2) - delayAmpsAreaHeight + delayAmpsMarginY;
    int rightY = (getHeight() / 2) + 3;
    int ampsUsableW = col2Width - (2 * col2Margin) - delayAmpsMarginX;
    int w = (ampsUsableW / numDelayAmps) - pad;
    int fullH = delayAmpsAreaHeight - delayAmpsMarginY - 3;
    // create parameters for amplitude to slider height map function
    // parameters are for y = a * sqrt(x + 0.02) - b, w points (0, c), (1, 1)
    // I think the above function looks nice!
    float c = numDelayAmps == 8 ? 0.1f : (numDelayAmps == 16 ? 0.075f : 0.06f);
    float a = (1 - c) / 0.87f;
    float b = 0.162f - 1.17f * c;
    // layout as many sliders as are necessary
    for (int i = 0;i < numDelayAmps;i++)
    {
        int startX = col1Width + col2Margin + delayAmpsMarginX;
        if (numDelayAmps == 8)
            startX -= 2;
        int offsetX = (w + pad) * i;
        int x = startX + offsetX;
        // scale heights by the dry-wet ratio and auto-falloff
        float p = juce::jmin((i == 0 ? 1 - wetRatio : wetRatio) * 2, 1.0f);
        p *= pow(autoFalloffRate, (float) i);
        // denormalize the amplitude-to-height mapping a bit for visual appeal
        // uses function described in comment where a and b are defined
        int h = (int) std::round(fullH * (a * pow(p + 0.02f, 0.5f) - b));
        // set the heights
        leftDelayAmps[i].setBounds(x, leftY + (fullH - h), w, h);
        rightDelayAmps[i].setBounds(x, rightY + (fullH - h), w, h);
    }
    // hide any other sliders
    for (int i = numDelayAmps;i < leftDelayAmpsLength;i++)
        leftDelayAmps[i].setBounds(0, 0, 0, 0);
    for (int i = numDelayAmps;i < rightDelayAmpsLength;i++)
        rightDelayAmps[i].setBounds(0, 0, 0, 0);
}

void PluginEditor::layoutRightSideGlobals()
{
    int x = col1Width + col2Width + (col3Width - col3KnobW) / 2;
    int halfHeight = (getHeight() - (paddingY * 2)) / 2;
    int y1 = paddingY + 2 * ((halfHeight - col3KnobH) / 3);
    falloff.setBounds(x, y1, col3KnobW, col3KnobH);
    int y2 = paddingY + halfHeight + (halfHeight - col3KnobH) / 3;
    wetDry.setBounds(x, y2, col3KnobW, col3KnobH);
}

// == Drawing Functions ======================================================
void PluginEditor::drawLeftSideGlobals(juce::Graphics& g)
{
    // draw background for delay time controls area
    g.setColour(findColour(CtmColourIds::darkBgColourId));
    int x = (col1Width - (toggleW * 2) - togglePadX) / 2;
    int w = (toggleW * 2) + togglePadX;
    int h = col1KnobH + togglePadY + toggleH;
    int halfHeight = (getHeight() - (paddingY * 2)) / 2;
    int y = paddingY + 2 * ((halfHeight - h) / 3);
    g.fillRoundedRectangle(x - 6, y - 6, w + 12, h + 12, 12);
    // draw background for number of intervals controls area
    int y2 = paddingY + halfHeight + ((halfHeight - h) / 3);
    g.fillRoundedRectangle(x - 6, y2 - 6, w + 12, h + 12, 12);
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
    // draw backgrounds for filters
    int xStart = col1Width + col2Margin;
    int filterAreaW = (col2Width - (2 * col2Margin));
    int fw = filterKnobW * 3 + filterMixMargin;
    int x1 = xStart + (filterAreaW - fw) / 2;
    int y1 = (((getHeight() / 2) - delayAmpsAreaHeight) - filterKnobH) / 2 - 9;
    int y2 = (getHeight() / 2) + delayAmpsAreaHeight;
    y2 = y2 - 9 + (getHeight() - y2 - filterKnobH) / 2;
    int fh = filterKnobH + 18;
    g.setColour(findColour(CtmColourIds::darkBgColourId));
    g.fillRoundedRectangle(x1 - 6, y1 - 6, fw + 12, fh + 12, 12);
    g.fillRoundedRectangle(x1 - 6, y2 - 6, fw + 12, fh + 12, 12);
    // draw filter text
    g.setColour(juce::Colours::white);
    g.setFont(14);
    auto centered = juce::Justification::centred;
    g.drawText("Delay Filter", x1 - 6, y1, fw + 12, 14, centered);
    g.drawText("Delay Filter", x1 - 6, y2, fw + 12, 14, centered);
}

void PluginEditor::drawChannelLabels(juce::Graphics& g)
{
    // define dimensions
    int x = col1Width;
    int y1 = 0;
    int y2 = getHeight();
    int h = 20;
    int wr = 45;
    int wl = 37;
    int edge = 8;
    // create paths for the label backgrounds
    juce::Path left;
    left.startNewSubPath(x, y1);
    left.lineTo(x + wl, y1);
    left.lineTo(x + wl - edge, y1 + h);
    left.lineTo(x, y1 + h);
    left.closeSubPath();
    juce::Path right;
    right.startNewSubPath(x, y2);
    right.lineTo(x + wr, y2);
    right.lineTo(x + wr - edge, y2 - h);
    right.lineTo(x, y2 - h);
    right.closeSubPath();
    // draw the label backgrounds
    g.setColour(findColour(CtmColourIds::brightBgColourId));
    g.fillPath(left);
    g.fillPath(right);
    g.setColour(findColour(CtmColourIds::brightOutlineColourId));
    g.strokePath(left, juce::PathStrokeType(1));
    g.strokePath(right, juce::PathStrokeType(1));
    // draw the text
    auto center = juce::Justification::centred;
    g.setColour(juce::Colours::white);
    g.drawText("Left", x, y1 + 2, wl - (2 * edge / 3), h - 4, center);
    g.drawText("Right", x, y2 - h + 1, wr - (2 * edge / 3), h - 4, center);
}

void PluginEditor::drawDelayAmpBeatMarkers(juce::Graphics& g)
{
    // get dimension info
    int numDividers = (numDelayAmps / 4) - 1;
    int ampsUsableW = col2Width - (2 * col2Margin) - delayAmpsMarginX;
    int dividerW = ampsUsableW / numDelayAmps;
    int xStart = col1Width + col2Margin + delayAmpsMarginX;
    int cy = getHeight() / 2;
    juce::Colour opaque = findColour(CtmColourIds::brightOutlineColourId);
    opaque = opaque.withAlpha(0.7f);
    juce::Colour trans = opaque.withAlpha(0.0f);
    // draw each divider
    for (int i = 0;i < numDividers;i++)
    {
        int x = xStart + (dividerW * (i + 1) * 4) - 1;
        if (numDelayAmps == 8)
            x -= 3;
        int lineH;
        if (numDividers == 1)
            lineH = 24;
        else if (numDividers == 3)
            lineH = i == 1 ? 24 : 18;
        else if (numDividers == 7)
            lineH = i == 3 ? 24 : (i % 2 == 1) ? 18 : 8;
        else
            lineH = 0;
        int y1 = cy - lineH;
        int y2 = cy + lineH;
        auto grad = juce::ColourGradient::vertical(trans, y1, trans, y2);
        double p = 2.0 / lineH; // numerator * 2 is length of color fade
        grad.addColour(p, opaque);
        grad.addColour(1 - p, opaque);
        g.setGradientFill(grad);
        g.drawLine(x, y1, x, y2);
    }
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