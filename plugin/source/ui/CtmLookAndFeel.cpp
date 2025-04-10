#include "CtmLookAndFeel.h"

CtmLookAndFeel::CtmLookAndFeel()
{
    // add custom colors
    setColour(CtmColourIds::normalBgColourId, makeColour(44, 54, 60));
    setColour(CtmColourIds::darkBgColourId, makeColour(34, 43, 49));
    setColour(CtmColourIds::brightBgColourId, makeColour(54, 72, 82));
    setColour(CtmColourIds::darkOutlineColourId, makeColour(22, 28, 30));
    setColour(CtmColourIds::brightOutlineColourId, makeColour(200, 200, 200));
    setColour(CtmColourIds::meterFillColourId, makeColour(52, 230, 38));
    setColour(CtmColourIds::BarBackColourId, makeColour(27, 71, 24));
    setColour(CtmColourIds::toggledColourId, makeColour(28, 190, 14));
    setColour(CtmColourIds::untoggledColourId, makeColour(150, 150, 150));
    setColour(CtmColourIds::delayAmpsAreaColourId, makeColour(13, 23, 13));
    // override default colors
    setColour(
        juce::ResizableWindow::backgroundColourId,
        findColour(CtmColourIds::normalBgColourId)
    );
    setColour(
        juce::TextEditor::backgroundColourId,
        makeColour(255, 255, 255, 0)
    );
    setColour(
        juce::TextEditor::outlineColourId, 
        makeColour(255, 255, 255, 0)
    );
    setColour(
        juce::TextEditor::focusedOutlineColourId,
        makeColour(255, 255, 255, 0)
    );
    setColour(
        juce::Slider::backgroundColourId,
        makeColour(40, 80, 100)
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

void CtmLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int w,
    int h, float sliderPos, const float startAngle, const float endAngle,
    juce::Slider& slider)
{
    // position information for drawing circles
    float cx = x + (w / 2.0f);
    float cy = y + (h / 2.0f);
    float r = (juce::jmin(w, h) / 2.0f) - (rotaryOutlineWidth / 2);
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

void CtmLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int w,
    int h, float pos, float min, float max, juce::Slider::SliderStyle style,
    juce::Slider& slider)
{
    juce::ignoreUnused(min, max);

    // this function only draws vertical sliders. If I need a horizontal one
    // for this application, I'll implement it when that need arises
    if (style == juce::Slider::SliderStyle::LinearVertical)
    {
        drawLinearSliderNoBar(g, x, y, w, h, pos, slider);
    }
    else if (style == juce::Slider::SliderStyle::LinearBarVertical)
    {
        drawLinearSliderBar(g, x, y, w, h, pos, slider);
    }
}

void CtmLookAndFeel::drawComboBox(juce::Graphics& g, int w, int h,
    bool clicked, int buttonX, int buttonY, int buttonW, int buttonH,
    juce::ComboBox& comboBox)
{
    juce::ignoreUnused(clicked, buttonX, buttonY, buttonW, buttonH);

    if (comboBox.isMouseOver(true))
    {
        g.setColour(comboBox.findColour(CtmColourIds::darkBgColourId));
    }
    else
    {
        g.setColour(comboBox.findColour(CtmColourIds::normalBgColourId));
    }
    g.fillRoundedRectangle(1, 1, w - 2, h - 2, 6);

    g.setColour(comboBox.findColour(CtmColourIds::brightOutlineColourId));
    g.drawRoundedRectangle(1, 1, w - 2, h - 2, 6, 1);
}

void CtmLookAndFeel::positionComboBoxText(juce::ComboBox& comboBox,
    juce::Label& label)
{
    label.setBounds(1, 1, comboBox.getWidth() - 2, comboBox.getHeight() - 2);
}

void CtmLookAndFeel::drawLinearSliderNoBar(juce::Graphics& g, int x, int y,
    int w, int h, float p, juce::Slider& slider)
{
    // position information
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

void CtmLookAndFeel::drawLinearSliderBar(juce::Graphics& g, int x, int y,
    int w, int h, float p, juce::Slider& slider)
{
    // draw background
    g.setColour(slider.findColour(CtmColourIds::BarBackColourId));
    g.fillRoundedRectangle(x, y, w, h, juce::jmin(w / 4, h / 2, 6));
    
    // draw fill
    g.reduceClipRegion(x, (int)p, w, h - (int)(p - y));
    g.setColour(slider.findColour(CtmColourIds::meterFillColourId));
    g.fillRoundedRectangle(x, y, w, h, juce::jmin(w / 4, h / 2, 6));
}

juce::Colour CtmLookAndFeel::makeColour(juce::uint8 r, juce::uint8 g,
    juce::uint8 b) const
{
    return juce::Colour::fromRGB(r, g, b);
}

juce::Colour CtmLookAndFeel::makeColour(juce::uint8 r, juce::uint8 g,
    juce::uint8 b, juce::uint8 a) const
{
    return juce::Colour::fromRGBA(r, g, b, a);
}