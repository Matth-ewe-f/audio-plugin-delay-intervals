#include "CtmLookAndFeel.h"

CtmLookAndFeel::CtmLookAndFeel()
{
    // add custom colors
    setColour(CtmColourIds::normalBgColourId, Colour::fromRGB(44, 54, 60));
    setColour(CtmColourIds::darkBgColourId, Colour::fromRGB(34, 43, 49));
    setColour(CtmColourIds::brightBgColourId, Colour::fromRGB(54, 72, 82));
    setColour(CtmColourIds::darkOutlineColourId, Colour::fromRGB(22, 28, 30));
    setColour(
        CtmColourIds::brightOutlineColourId, Colour::fromRGB(200, 200, 200)
    );
    setColour(CtmColourIds::meterFillColourId, Colour::fromRGB(52, 230, 38));
    setColour(CtmColourIds::BarBackColourId, Colour::fromRGB(27, 71, 24));
    setColour(CtmColourIds::toggledColourId, Colour::fromRGB(28, 190, 14));
    setColour(CtmColourIds::untoggledColourId, Colour::fromRGB(150, 150, 150));
    setColour(
        CtmColourIds::delayAmpsAreaColourId, Colour::fromRGB(13, 23, 13)
    );
    // override default colors
    setColour(
        juce::ResizableWindow::backgroundColourId,
        findColour(CtmColourIds::normalBgColourId)
    );
    setColour(
        juce::TextEditor::backgroundColourId,
        Colour::fromRGBA(255, 255, 255, 0)
    );
    setColour(
        juce::TextEditor::outlineColourId, 
        Colour::fromRGBA(255, 255, 255, 0)
    );
    setColour(
        juce::TextEditor::focusedOutlineColourId,
        Colour::fromRGBA(255, 255, 255, 0)
    );
    setColour(
        juce::Slider::backgroundColourId,
        Colour::fromRGB(40, 80, 100)
    );
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
    juce::ignoreUnused(min, max);
    // this function only draws vertical sliders. If I need a horizontal one
    // for this application, I'll implemented it when I need it
    if (style == juce::Slider::SliderStyle::LinearVertical)
        drawLinearSliderNoBar(g, x, y, w, h, pos, slider);
    else if (style == juce::Slider::SliderStyle::LinearBarVertical)
        drawLinearSliderBar(g, x, y, w, h, pos, slider);
}

// === Private Helper =========================================================
void CtmLookAndFeel::drawLinearSliderNoBar
(juce::Graphics& g, int x, int y, int w, int h, float p, juce::Slider& slider)
{
    int cx = x + (w / 2);
    // draw the background
    g.setColour(slider.findColour(CtmColourIds::darkOutlineColourId));
    g.fillRoundedRectangle(cx - 4, y, 8, h, 4);
    // draw the fill
    g.setColour(slider.findColour(CtmColourIds::meterFillColourId));
    g.fillRoundedRectangle(cx - 3, p, 6, h - (p - y) - 1, 3);
    // draw the shadow of the tick
    juce::Colour shadow = slider.findColour(CtmColourIds::darkOutlineColourId);
    shadow = shadow.withAlpha(0.3f);
    g.setColour(shadow);
    g.fillRoundedRectangle(cx - 7, p - 2, 16, 6, 3);
    // draw the tick
    g.setColour(slider.findColour(CtmColourIds::brightOutlineColourId));
    g.fillRoundedRectangle(cx - 8, p - 3, 16, 6, 3);
}

void CtmLookAndFeel::drawLinearSliderBar
(juce::Graphics& g, int x, int y, int w, int h, float p, juce::Slider& slider)
{
    // draw background
    g.setColour(slider.findColour(CtmColourIds::BarBackColourId));
    g.fillRoundedRectangle(x, y, w, h, juce::jmin(w / 4, h / 2, 6));
    // draw fill
    g.reduceClipRegion(x, (int)p, w, h - (int)(p - y));
    g.setColour(slider.findColour(CtmColourIds::meterFillColourId));
    g.fillRoundedRectangle(x, y, w, h, juce::jmin(w / 4, h / 2, 6));
}