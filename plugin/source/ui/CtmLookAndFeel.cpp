#include "CtmLookAndFeel.h"

CtmLookAndFeel::CtmLookAndFeel()
{
    // override default colors
    setColour(juce::ResizableWindow::backgroundColourId, normalBgColor);
    juce::Colour trans = juce::Colour::fromRGBA(255, 255, 255, 0);
    setColour(juce::TextEditor::backgroundColourId, trans);
    setColour(juce::TextEditor::outlineColourId, trans);
    setColour(juce::TextEditor::focusedOutlineColourId, trans);
    setColour(juce::Slider::backgroundColourId, rotaryColor);
    // add custom colors
    setColour(CtmColourIds::normalBgColourId, normalBgColor);
    setColour(CtmColourIds::darkBgColourId, darkBgColor);
    setColour(CtmColourIds::brightBgColourId, brightBgColor);
    setColour(CtmColourIds::darkOutlineColourId, darkOutlineColor);
    setColour(CtmColourIds::brightOutlineColourId, brightOutlineColor);
    setColour(CtmColourIds::meterFillColourId, meterFillColor);
    setColour(CtmColourIds::toggledColourId, toggledColor);
    setColour(CtmColourIds::untoggledColourId, untoggledColor);
}

juce::Slider::SliderLayout CtmLookAndFeel::getSliderLayout
(juce::Slider& slider)
{
    juce::Rectangle<int> b = slider.getLocalBounds();
    juce::Slider::SliderLayout layout;
    if (slider.getSliderStyle() == juce::Slider::LinearVertical)
    {
        b.setY(b.getY() + 6);
        b.setHeight(b.getHeight() - 12);
    }
    layout.sliderBounds = b;
    return layout;
}

void CtmLookAndFeel::drawRotarySlider
(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
const float startAngle, const float endAngle, juce::Slider& slider)
{
    // position information for drawing circles
    float cx = x + (width / 2.0f);
    float cy = y + (height / 2.0f);
    float r = (juce::jmin(width, height) / 2.0f) - (rotaryOutlineWidth / 2);
    float rx = cx - r;
    float ry = cy - r;
    float innerR = r - (rotaryOutlineWidth / 2);
    float innerRx = cx - innerR;
    float innerRy = cy - innerR;
    // draw the body of the rotary
    g.setColour(slider.findColour(juce::Slider::backgroundColourId));
    g.fillEllipse(innerRx, innerRy, innerR * 2, innerR * 2);
    // draw the back of the fill meter
    juce::Path meter;
    meter.addArc(rx, ry, r * 2, r * 2, startAngle, endAngle, true);
    juce::PathStrokeType pathStroke(rotaryOutlineWidth);
    g.setColour(slider.findColour(CtmColourIds::darkOutlineColourId));
    g.strokePath(meter, pathStroke);
    // draw the front of the fill meter
    juce::Path meterFill;
    float fillAngle = startAngle + ((endAngle - startAngle) * sliderPos);
    meterFill.addArc(rx, ry, r * 2, r * 2, startAngle, fillAngle, true);
    g.setColour(slider.findColour(CtmColourIds::meterFillColourId));
    g.strokePath(meterFill, pathStroke);
    // draw a tick and shadow at the point to which the fill meter is filled
    juce::Line<float> l1(cx, y, cx, y + 8);
    juce::Path tick;
    tick.addLineSegment(l1, 3);
    juce::Line<float> l2(cx, y - 1, cx, y + 9);
    juce::Path shadow;
    shadow.addLineSegment(l2, 5);
    tick.applyTransform(juce::AffineTransform::rotation(fillAngle, cx, cy));
    shadow.applyTransform(juce::AffineTransform::rotation(fillAngle, cx, cy));
    juce::Colour shadowColor
        = slider.findColour(CtmColourIds::darkOutlineColourId);
    shadowColor = shadowColor.withAlpha(0.25f);
    g.setColour(shadowColor);
    g.fillPath(shadow);
    g.setColour(slider.findColour(CtmColourIds::brightOutlineColourId));
    g.fillPath(tick);
}

void CtmLookAndFeel::drawLinearSlider
(juce::Graphics& g, int x, int y, int w, int h, float pos, float min,
float max, juce::Slider::SliderStyle style, juce::Slider& slider)
{
    // this function draws all sliders as vertical. If I need a horizontal
    // slider for this application, I'll implemented it too
    juce::ignoreUnused(min, max, style);
    int cx = x + (w / 2);
    // draw the background
    g.setColour(findColour(CtmColourIds::darkOutlineColourId));
    g.fillRoundedRectangle(cx - 4, y, 8, h, 4);
    // draw the fill
    g.setColour(slider.findColour(CtmColourIds::meterFillColourId));
    g.fillRoundedRectangle(cx - 3, pos, 6, h - (pos - y) - 1, 3);
    // draw the shadow of the tick
    juce::Colour shadow = slider.findColour(CtmColourIds::darkOutlineColourId);
    shadow = shadow.withAlpha(0.3f);
    g.setColour(shadow);
    g.fillRoundedRectangle(cx - 7, pos - 2, 16, 6, 3);
    // draw the tick
    g.setColour(slider.findColour(CtmColourIds::brightOutlineColourId));
    g.fillRoundedRectangle(cx - 8, pos - 3, 16, 6, 3);
}