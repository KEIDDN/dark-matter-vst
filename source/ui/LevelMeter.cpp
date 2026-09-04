#include "LevelMeter.h"
#include "Colors.h"

namespace dm::ui
{

LevelMeter::LevelMeter(const std::atomic<float>& levelSource, FillFrom direction)
    : level(levelSource), fillFrom(direction)
{
    startTimerHz(30);
}

LevelMeter::~LevelMeter()
{
    stopTimer();
}

void LevelMeter::timerCallback()
{
    const float target = juce::jlimit(0.0f, 1.0f, level.load());
    displayed += (target - displayed) * 0.35f;
    repaint();
}

void LevelMeter::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    g.setColour(juce::Colours::black.withAlpha(0.4f));
    g.fillRoundedRectangle(b, b.getHeight() * 0.5f);
    g.setColour(juce::Colours::white.withAlpha(0.06f));
    g.drawRoundedRectangle(b.reduced(0.5f), b.getHeight() * 0.5f, 1.0f);

    auto inner = b.reduced(1.0f);
    const float w = juce::jmax(4.0f, inner.getWidth() * juce::jmax(displayed, 0.04f));
    auto fillRect = fillFrom == FillFrom::Left
        ? inner.withWidth(w)
        : inner.withLeft(inner.getRight() - w);

    g.setGradientFill(juce::ColourGradient(
        accent.withAlpha(0.55f), fillFrom == FillFrom::Left ? fillRect.getX() : fillRect.getRight(), 0,
        accentLight.withAlpha(0.9f), fillFrom == FillFrom::Left ? fillRect.getRight() : fillRect.getX(), 0, false));
    g.fillRoundedRectangle(fillRect, inner.getHeight() * 0.5f);
}

} // namespace dm::ui
