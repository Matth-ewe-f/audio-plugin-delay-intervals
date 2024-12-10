#include "CtmToggle.h"
#include "CtmLookAndFeel.h"

// === Lifecycle ==============================================================
CtmToggle::CtmToggle()
    : colorOverriden(false), colorGradient(false), toggledText(""),
    untoggledText(""), fontSize(-1), alwaysUp(false), untoggledColor(false)
 { }

void CtmToggle::parentHierarchyChanged()
{
    if (!colorOverriden && !colorGradient)
        fillColor = findColour(CtmColourIds::toggledColourId);
}

// === Setters ================================================================
void CtmToggle::setText(std::string s)
{
    toggledText = s;
    untoggledText = s;
}

void CtmToggle::setText(std::string toggleText, std::string untoggleText)
{
    toggledText = toggleText;
    untoggledText = untoggleText;
}

void CtmToggle::setFixedFontSize(float size)
{
    fontSize = size;
}

void CtmToggle::setColorOverride(juce::Colour color)
{
    fillColor = color;
    colorOverriden = true;
    colorGradient = false;
    repaint();
}

void CtmToggle::setColorGradient(juce::Colour color1, juce::Colour color2)
{
    fillColor = color1;
    gradColor = color2;
    colorOverriden = false;
    colorGradient = true;
    repaint();
}

void CtmToggle::setDisplayAlwaysUp(bool isAlwaysUp)
{
    alwaysUp = isAlwaysUp;
    repaint();
}

void CtmToggle::setColorAsUntoggled(bool showUntoggled)
{
    untoggledColor = showUntoggled;
    repaint();
}

// === Graphics ===============================================================
// warning can safely be ignored - float comparison involving no arithmetic
// is perfectly safe
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
void CtmToggle::paintButton(juce::Graphics& g, bool hover, bool click)
{
    juce::ignoreUnused(hover);
    bool toggle = getToggleState();
    // get the bounds of the button
    juce::Rectangle<int> bounds = getLocalBounds();
    int x = bounds.getX() + 1;
    int y = bounds.getY() + 1;
    int w = bounds.getWidth() - 2;
    int h = bounds.getHeight() - 2;
    // draw the button background
    if (!(untoggledColor) && (toggle || alwaysUp))
    {
        if (colorGradient)
        {
            g.setGradientFill(juce::ColourGradient(
                fillColor, x, y, gradColor, x, y + h, false
            ));
        }
        else
            g.setColour(fillColor);
    }
    else
        g.setColour(findColour(CtmColourIds::untoggledColourId));
    g.fillRoundedRectangle(x, y, w, h, 4);
    // draw the highlights on the top of the button
    juce::Colour light;
    if (click || (!toggle && !alwaysUp))
        light = getShadowColor();
    else 
        light = getHighlightColor();
    g.setColour(light);
    g.drawRoundedRectangle(x, y + 1, w, h - 1, 4, 1);
    light = light.withMultipliedAlpha(0.7f);
    g.setColour(light);
    g.drawRoundedRectangle(x, y + 2, w, h - 3, 4, 1);
    // draw the outline of the button
    g.setColour(findColour(CtmColourIds::darkOutlineColourId));
    g.drawRoundedRectangle(x, y, w, h, 4, 1);
    // draw the button text
    g.setColour(juce::Colours::white);
    g.setFont(fontSize == -1 ? (h / 2) + 2 : fontSize);
    auto justify = juce::Justification::centred;
    std::string s = toggle ? toggledText : untoggledText;
    int textY = !(toggle || alwaysUp) || (alwaysUp && click) ? y + 4 : y + 3;
    g.drawText(s, x + 2, textY, w - 4, h - 8, justify, false);
    // darken the button it it's been clicked
    if (click)
    {
        g.setColour(juce::Colours::black.withAlpha(0.08f));
        g.fillRoundedRectangle(x + 1, y + 1, w - 2, h - 2, 4);
    }
}
#pragma GCC diagnostic pop

// === Private Helper =========================================================
juce::Colour CtmToggle::getHighlightColor()
{
    juce::Colour outline = findColour(CtmColourIds::brightOutlineColourId);
    if (colorOverriden && fillColor.getLightness() < 0.3f)
        return outline.withAlpha(0.2f);
    else
        return outline.withAlpha(0.3f);
}

juce::Colour CtmToggle::getShadowColor()
{
    juce::Colour outline = findColour(CtmColourIds::darkOutlineColourId);
    return outline.withAlpha(0.15f);
}