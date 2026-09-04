#include "DarkMatterLookAndFeel.h"
#include "Colors.h"
#include "BinaryData.h"

namespace dm::ui
{

DarkMatterLookAndFeel::DarkMatterLookAndFeel()
{
    geist200 = juce::Typeface::createSystemTypefaceFor(BinaryData::Geist200_ttf, (size_t) BinaryData::Geist200_ttfSize);
    geist300 = juce::Typeface::createSystemTypefaceFor(BinaryData::Geist300_ttf, (size_t) BinaryData::Geist300_ttfSize);
    geist400 = juce::Typeface::createSystemTypefaceFor(BinaryData::Geist400_ttf, (size_t) BinaryData::Geist400_ttfSize);
    geist500 = juce::Typeface::createSystemTypefaceFor(BinaryData::Geist500_ttf, (size_t) BinaryData::Geist500_ttfSize);
    geistMono300 = juce::Typeface::createSystemTypefaceFor(BinaryData::GeistMono300_ttf, (size_t) BinaryData::GeistMono300_ttfSize);
    geistMono400 = juce::Typeface::createSystemTypefaceFor(BinaryData::GeistMono400_ttf, (size_t) BinaryData::GeistMono400_ttfSize);
    syncopate400 = juce::Typeface::createSystemTypefaceFor(BinaryData::Syncopate400_ttf, (size_t) BinaryData::Syncopate400_ttfSize);

    setColour(juce::ResizableWindow::backgroundColourId, bgOuter);
}

juce::Font DarkMatterLookAndFeel::getGeist(float size, int weight) const
{
    juce::Typeface::Ptr tf = geist300;
    if (weight <= 220) tf = geist200;
    else if (weight <= 320) tf = geist300;
    else if (weight <= 420) tf = geist400;
    else tf = geist500;
    return juce::Font(tf).withHeight(size);
}

juce::Font DarkMatterLookAndFeel::getGeistMono(float size, int weight) const
{
    return juce::Font(weight <= 320 ? geistMono300 : geistMono400).withHeight(size);
}

juce::Font DarkMatterLookAndFeel::getSyncopate(float size) const
{
    return juce::Font(syncopate400).withHeight(size);
}

void DarkMatterLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                              float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                              juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float>((float) x, (float) y, (float) width, (float) height);
    auto centre = bounds.getCentre();
    const float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    const bool active = slider.isMouseButtonDown();
    const bool hover = slider.isMouseOverOrDragging();

    if (active || hover)
    {
        const float glowAlpha = active ? 0.85f : 0.32f;
        juce::ColourGradient glow(accent.withAlpha(0.24f * glowAlpha), centre,
                                   accent.withAlpha(0.0f), centre.translated(radius * 1.35f, 0.0f), true);
        g.setGradientFill(glow);
        g.fillEllipse(bounds.expanded(radius * 0.3f));
    }

    g.setColour(juce::Colours::white.withAlpha(0.045f));
    g.drawEllipse(bounds.reduced(radius * 0.06f), 1.0f);

    juce::Path arc;
    arc.addCentredArc(centre.x, centre.y, radius * 0.97f, radius * 0.97f, 0.0f,
                       rotaryStartAngle, angle, true);
    g.setColour(accent.withAlpha(0.85f));
    g.strokePath(arc, juce::PathStrokeType(juce::jmax(1.6f, radius * 0.06f),
                                            juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    auto body = bounds.reduced(radius * 0.14f);
    juce::ColourGradient bodyGrad(juce::Colour(0xff2a2930),
                                   body.getTopLeft().translated(body.getWidth() * 0.38f, body.getHeight() * 0.28f),
                                   juce::Colour(0xff0a0a0d), body.getCentre().translated(0, body.getHeight() * 0.6f), true);
    g.setGradientFill(bodyGrad);
    g.fillEllipse(body);
    g.setColour(juce::Colours::white.withAlpha(0.1f));
    g.drawEllipse(body, 1.0f);

    {
        juce::Graphics::ScopedSaveState save(g);
        juce::Path clip;
        clip.addEllipse(body);
        g.reduceClipRegion(clip);
        juce::ColourGradient hl(juce::Colours::white.withAlpha(0.13f), body.getTopLeft(),
                                 juce::Colours::white.withAlpha(0.0f), body.getCentre(), false);
        g.setGradientFill(hl);
        g.fillRect(body);
    }

    const auto dot = centre.getPointOnCircumference(radius * 0.72f, angle);
    g.setColour(accentHot);
    g.fillEllipse(juce::Rectangle<float>(radius * 0.09f, radius * 0.09f).withCentre(dot));
}

} // namespace dm::ui
