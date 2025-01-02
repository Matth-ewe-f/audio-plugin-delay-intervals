#include "PluginProcessor.h"
#include "PluginEditor.h"

// === Layout Constants ===================================================
const int PluginEditor::col1Width = 120;
const int PluginEditor::col1KnobW = 72;
const int PluginEditor::col1KnobH = 80;
const int PluginEditor::syncToggleW = 32;
const int PluginEditor::syncToggleH = 22;
const int PluginEditor::syncTogglePadX = 2;
const int PluginEditor::syncTogglePadY = 8;
const int PluginEditor::loopToggleW = 40;
const int PluginEditor::col2Width = 411;
const int PluginEditor::col2Margin = 16;
const int PluginEditor::delayAmpsAreaHeight = 64;
const int PluginEditor::delayAmpsMarginX = 14;
const int PluginEditor::delayAmpsMarginY = 12;
const int PluginEditor::filterY = 6;
const int PluginEditor::filterMargin = 48;
const int PluginEditor::filterKnobW = 58;
const int PluginEditor::filterKnobH = 68;
const int PluginEditor::filterMixMargin = 20;
const int PluginEditor::col2ButtonW = 50;
const int PluginEditor::col2ButtonH = 24;
const int PluginEditor::col2ButtonPad = 6;
const int PluginEditor::col2ButtonMargin = 8;
const int PluginEditor::col3Width = 120;
const int PluginEditor::col3KnobW = 72;
const int PluginEditor::col3KnobH = 80;
const int PluginEditor::col3KnobMargin = 24;
const int PluginEditor::col3ToggleW = 52;
const int PluginEditor::col3ToggleH = 24;
const int PluginEditor::col3ToggleMargin = 4;
const int PluginEditor::col3ToggleLabelH = 16;
const int PluginEditor::height = 312;
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
    setSize(col1Width + col2Width + col3Width, height);
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
    ParameterToggle* noTempoSyncPtr = &noTempoSync;
    tempoSync.addOnToggleFunction([noTempoSyncPtr] (bool b)
    {
        noTempoSyncPtr->toggle.setToggleState(!b, juce::sendNotification);
    });
    tempoSync.attachToParameter(&processorRef.tree, "tempo-sync");
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
    // setup buttons
    resetLeft.setText("RESET");
    resetLeft.setDisplayAlwaysUp(true);
    resetLeft.setFixedFontSize(13);
    PluginProcessor* processorPtr = &processorRef;
    resetLeft.onClick = [processorPtr] ()
    {
        processorPtr->resetLeftAmps();
        processorPtr->notifyHostOfStateChange();
    };
    addAndMakeVisible(resetLeft);
    resetRight.setText("RESET");
    resetRight.setDisplayAlwaysUp(true);
    resetRight.setFixedFontSize(13);
    resetRight.onClick = [processorPtr] ()
    {
        processorPtr->resetRightAmps();
        processorPtr->notifyHostOfStateChange();
    };
    addAndMakeVisible(resetRight);
    matchLeft.setText("MATCH");
    matchLeft.setDisplayAlwaysUp(true);
    matchLeft.setFixedFontSize(13);
    matchLeft.onClick = [processorPtr] ()
    {
        processorPtr->copyRightAmpsToLeft();
        processorPtr->notifyHostOfStateChange();
    };
    addAndMakeVisible(matchLeft);
    matchRight.setText("MATCH");
    matchRight.setDisplayAlwaysUp(true);
    matchRight.setFixedFontSize(13);
    matchRight.onClick = [processorPtr] ()
    {
        processorPtr->copyLeftAmpsToRight();
        processorPtr->notifyHostOfStateChange();
    };
    addAndMakeVisible(matchRight);
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
    linkAmps.toggle.setText("DELAYS");
    linkAmps.toggle.setFixedFontSize(13);
    linkAmps.attachToParameter(&processorRef.tree, "delays-linked");
    addAndMakeVisible(linkAmps.toggle);
    linkFilters.toggle.setText("FILTERS");
    linkFilters.toggle.setFixedFontSize(13);
    linkFilters.addOnToggleFunction([&] (bool toggled)
    {
        juce::AudioProcessorValueTreeState* tree = &processorRef.tree;
        if (toggled)
        {
            rightFilterLow.attachToParameter(tree, "left-high-pass");
            rightFilterHigh.attachToParameter(tree, "left-low-pass");
            rightFilterMix.attachToParameter(tree, "left-filter-mix");
            processorRef.linkFilters();
        }
        else
        {
            rightFilterLow.attachToParameter(tree, "right-high-pass");
            rightFilterHigh.attachToParameter(tree, "right-low-pass");
            rightFilterMix.attachToParameter(tree, "right-filter-mix");
            processorRef.unlinkFilters();
        }
    });
    linkFilters.attachToParameter(&processorRef.tree, "filters-linked");
    addAndMakeVisible(linkFilters.toggle);
    falloff.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    falloff.setTitleText("Falloff");
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
    int delaySectionH = col1KnobH + syncTogglePadY + syncToggleH;
    int delaySectionY = paddingY + ((halfHeight - delaySectionH) / 2);
    int x = (col1Width - col1KnobW) / 2;
    delayTime.setBounds(x, delaySectionY, col1KnobW, col1KnobH);
    int noTempoX = (col1Width - (syncToggleW * 2) - syncTogglePadX) / 2;
    int delayToggleY = delaySectionY + col1KnobH + syncTogglePadY;
    noTempoSync.setBounds(noTempoX, delayToggleY, syncToggleW, syncToggleH);
    int tempoX = noTempoX + syncToggleW + syncTogglePadX;
    tempoSync.setBounds(tempoX, delayToggleY, syncToggleW, syncToggleH);
    // number of intervals controls
    int intervalsH = col1KnobH + syncTogglePadY + syncToggleH;
    int intervalsY = paddingY + halfHeight + ((halfHeight - intervalsH) / 2);
    numIntervals.setBounds(x, intervalsY, col1KnobW, col1KnobH);
    int loopX = (col1Width - loopToggleW) / 2;
    int loopY = intervalsY + col1KnobH + syncTogglePadY;
    loopButton.setBounds(loopX, loopY, loopToggleW, syncToggleH);
}

void PluginEditor::layoutChannelFilters()
{
    // calculate dimensions and shared positions
    int xStart = col1Width + col2Margin;
    int filterAreaW = (col2Width - (2 * col2Margin));
    int filterW = filterKnobW * 3 + filterMixMargin;
    int buttonsW = col2ButtonW;
    int totalW = filterW + filterMargin + buttonsW;
    int x1 = xStart + ((filterAreaW - totalW) / 2);
    int by1 = (getHeight() / 2) - delayAmpsAreaHeight - col2ButtonMargin
        - 2 * col2ButtonH - col2ButtonPad;
    int by2 = (getHeight() / 2) + delayAmpsAreaHeight + col2ButtonMargin;
    int fy1 = filterY;
    int fy2 = getHeight() - filterKnobH - filterY;
    // lay out buttons
    resetLeft.setBounds(x1, by1, col2ButtonW, col2ButtonH);
    resetRight.setBounds(x1, by2, col2ButtonW, col2ButtonH);
    by1 += col2ButtonH + col2ButtonPad;
    by2 += col2ButtonH + col2ButtonPad;
    matchLeft.setBounds(x1, by1, col2ButtonW, col2ButtonH);
    matchRight.setBounds(x1, by2, col2ButtonW, col2ButtonH);
    // lay out filters
    int x2 = x1 + col2ButtonW + filterMargin;
    leftFilterLow.setBounds(x2, fy1, filterKnobW, filterKnobH);
    rightFilterLow.setBounds(x2, fy2, filterKnobW, filterKnobH);
    int x3 = x2 + filterKnobW;
    leftFilterHigh.setBounds(x3, fy1, filterKnobW, filterKnobH);
    rightFilterHigh.setBounds(x3, fy2, filterKnobW, filterKnobH);
    int x4 = x3 + filterKnobW + filterMixMargin;
    leftFilterMix.setBounds(x4, fy1, filterKnobW, filterKnobH);
    rightFilterMix.setBounds(x4, fy2, filterKnobW, filterKnobH);
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
    int toggleX = col1Width + col2Width + (col3Width - col3ToggleW) / 2;
    int knobX = col1Width + col2Width + (col3Width - col3KnobW) / 2;
    int totalH = col3ToggleLabelH + 2 * (col3ToggleH + col3ToggleMargin)
        + 2 * (col3KnobH + col3KnobMargin);
    int y1 = (getHeight() - totalH) / 2 + col3ToggleLabelH + col3ToggleMargin;
    linkFilters.setBounds(toggleX, y1, col3ToggleW, col3ToggleH);
    int y2 = y1 + col3ToggleH + col3ToggleMargin;
    linkAmps.setBounds(toggleX, y2, col3ToggleW, col3ToggleH);
    int y3 = y2 + col3ToggleH + col3KnobMargin;
    falloff.setBounds(knobX, y3, col3KnobW, col3KnobH);
    int y4 = y3 + col3KnobH + col3KnobMargin;
    wetDry.setBounds(knobX, y4, col3KnobW, col3KnobH);
}

// == Drawing Functions ======================================================
void PluginEditor::drawLeftSideGlobals(juce::Graphics& g)
{
    // draw background for delay time controls area
    g.setColour(findColour(CtmColourIds::darkBgColourId));
    int x = (col1Width - (syncToggleW * 2) - syncTogglePadX) / 2;
    int w = (syncToggleW * 2) + syncTogglePadX;
    int h = col1KnobH + syncTogglePadY + syncToggleH;
    int halfHeight = (getHeight() - (paddingY * 2)) / 2;
    int y = paddingY + ((halfHeight - h) / 2);
    g.fillRoundedRectangle(x - 10, y - 6, w + 20, h + 14, 14);
    // draw background for number of intervals controls area
    int y2 = paddingY + halfHeight + ((halfHeight - h) / 2);
    g.fillRoundedRectangle(x - 10, y2 - 6, w + 20, h + 14, 14);
}

void PluginEditor::drawChannels(juce::Graphics& g)
{
    // draw backgrounds for buttons
    int usableW = (col2Width - (2 * col2Margin));
    int fw = filterKnobW * 3 + filterMixMargin;
    int buttonsW = col2ButtonW;
    int totalW = fw + filterMargin + buttonsW;
    int bx = col1Width + col2Margin + ((usableW - totalW) / 2);
    int yExtra = 20;
    int by1 = (getHeight() / 2) - delayAmpsAreaHeight - col2ButtonMargin
        - 2 * col2ButtonH - col2ButtonPad;
    int by2 = (getHeight() / 2) + delayAmpsAreaHeight + col2ButtonMargin;
    int bw = col2ButtonW;
    int bh = 2 * col2ButtonH + col2ButtonPad + yExtra;
    g.setColour(findColour(CtmColourIds::darkBgColourId));
    g.fillRoundedRectangle(bx - 15, by1 - 9, bw + 30, bh + 18, 12);
    g.fillRoundedRectangle(bx - 15, by2 - yExtra, bw + 30, bh + 9, 12);
    // draw backgrounds for filters
    int fx = bx + buttonsW + filterMargin;
    int fy1 = filterY;
    int fy2 = getHeight() - filterKnobH - filterY;
    int fh = filterKnobH + yExtra;
    g.fillRoundedRectangle(fx - 6, fy1 - yExtra, fw + 12, fh + 6, 12);
    g.fillRoundedRectangle(fx - 6, fy2 - 6, fw + 12, fh + 6, 12);
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
    // draw backgrounds for controls
    int x = col1Width + col2Width + (col3Width - col3KnobW) / 2;
    int totalH = col3ToggleLabelH + 2 * (col3ToggleH + col3ToggleMargin)
        + 2 * (col3KnobH + col3KnobMargin);
    int y1 = (getHeight() - totalH) / 2;
    g.setColour(findColour(CtmColourIds::darkBgColourId));
    int toggleAreaH = col3ToggleLabelH + 2 * (col3ToggleH + col3ToggleMargin);
    int r = 12;
    g.fillRoundedRectangle(x - 6, y1 - 6, col3KnobW + 12, toggleAreaH + 12, r);
    int y2 = y1 + toggleAreaH + col3KnobMargin;
    g.fillRoundedRectangle(x - 6, y2 - 6, col3KnobW + 12, col3KnobH + 12, r);
    int y3 = y2 + col3KnobH + col3KnobMargin;
    g.fillRoundedRectangle(x - 6, y3 - 6, col3KnobW + 12, col3KnobH + 12, r);
    // draw text label for the link buttons
    auto center = juce::Justification::centred;
    g.setColour(juce::Colours::white);
    g.setFont(14);
    juce::Rectangle<int> textRext(x, y1, col3KnobW, col3ToggleLabelH);
    g.drawFittedText("Mirror", textRext, center, 1);
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