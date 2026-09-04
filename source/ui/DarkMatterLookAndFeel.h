#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace dm::ui
{

/** Fonts + the custom rotary-knob painting shared across the whole plugin UI. */
class DarkMatterLookAndFeel : public juce::LookAndFeel_V4
{
public:
    DarkMatterLookAndFeel();

    juce::Font getGeist(float size, int weight = 300) const;
    juce::Font getGeistMono(float size, int weight = 400) const;
    juce::Font getSyncopate(float size) const;

    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider&) override;

private:
    juce::Typeface::Ptr geist200, geist300, geist400, geist500;
    juce::Typeface::Ptr geistMono300, geistMono400;
    juce::Typeface::Ptr syncopate400;
};

} // namespace dm::ui
