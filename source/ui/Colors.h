#pragma once

#include <juce_graphics/juce_graphics.h>

namespace dm::ui
{
    inline const juce::Colour bgOuter   { 0xff050506 };
    inline const juce::Colour bgMid     { 0xff0a090c };
    inline const juce::Colour bgCenter  { 0xff111013 };

    inline const juce::Colour text        = juce::Colour::fromFloatRGBA(207 / 255.0f, 203 / 255.0f, 216 / 255.0f, 1.0f);
    inline const juce::Colour textDim     = text.withAlpha(0.55f);
    inline const juce::Colour textFaint   = text.withAlpha(0.35f);
    inline const juce::Colour textBright  = juce::Colour::fromFloatRGBA(240 / 255.0f, 237 / 255.0f, 247 / 255.0f, 1.0f);
    inline const juce::Colour headline    = juce::Colour::fromFloatRGBA(235 / 255.0f, 232 / 255.0f, 242 / 255.0f, 1.0f);

    inline const juce::Colour accent      = juce::Colour::fromFloatRGBA(150 / 255.0f, 120 / 255.0f, 255 / 255.0f, 1.0f);
    inline const juce::Colour accentLight = juce::Colour::fromFloatRGBA(183 / 255.0f, 156 / 255.0f, 255 / 255.0f, 1.0f);
    inline const juce::Colour accentDot   = juce::Colour::fromFloatRGBA(201 / 255.0f, 184 / 255.0f, 255 / 255.0f, 1.0f);
    inline const juce::Colour accentHot   = juce::Colour::fromFloatRGBA(233 / 255.0f, 227 / 255.0f, 255 / 255.0f, 1.0f);

    inline const juce::Colour panelBorder = juce::Colours::white.withAlpha(0.07f);
    inline const juce::Colour panelFillHi = juce::Colours::white.withAlpha(0.035f);
    inline const juce::Colour panelFillLo = juce::Colours::white.withAlpha(0.01f);

    inline const juce::Colour browserBgHi = juce::Colour::fromFloatRGBA(26 / 255.0f, 25 / 255.0f, 31 / 255.0f, 0.92f);
    inline const juce::Colour browserBgLo = juce::Colour::fromFloatRGBA(12 / 255.0f, 12 / 255.0f, 16 / 255.0f, 0.96f);
}
