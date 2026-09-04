#include "LurkingModule.h"
#include "Colors.h"
#include "../dsp/Parameters.h"
#include <cmath>

namespace dm::ui
{

LurkingModule::LurkingModule(juce::AudioProcessorValueTreeState& apvts, DarkMatterLookAndFeel& laf)
    : state(apvts), lookAndFeel(laf)
{
    dragSlider.setMouseDragSensitivity(240);
    dragSlider.setAlpha(0.0f);
    addAndMakeVisible(dragSlider);
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, ParamID::modulation, dragSlider);
    dragSlider.onValueChange = [this] { repaint(); };

    nameLabel.setText("LURKING", juce::dontSendNotification);
    nameLabel.setJustificationType(juce::Justification::centred);
    nameLabel.setFont(laf.getGeist(9.5f, 400));
    nameLabel.setColour(juce::Label::textColourId, textDim);
    nameLabel.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(nameLabel);

    valueLabel.setJustificationType(juce::Justification::centred);
    valueLabel.setFont(laf.getGeistMono(11.0f, 300));
    valueLabel.setColour(juce::Label::textColourId, textBright.withAlpha(0.72f));
    valueLabel.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(valueLabel);

    startTimerHz(45);
}

LurkingModule::~LurkingModule()
{
    stopTimer();
}

void LurkingModule::setResetNormalisedValue(float normalised)
{
    if (auto* p = state.getParameter(ParamID::modulation))
        dragSlider.setDoubleClickReturnValue(true, p->convertFrom0to1(normalised));
}

void LurkingModule::timerCallback()
{
    t += 0.06f;
    valueLabel.setText(juce::String((int) std::round(dragSlider.getValue())) + " %", juce::dontSendNotification);
    repaint();
}

void LurkingModule::resized()
{
    dragSlider.setBounds(getLocalBounds());
    auto b = getLocalBounds();
    nameLabel.setBounds(b.removeFromBottom(24).withTrimmedBottom(2));
    valueLabel.setBounds(b.removeFromBottom(16));
}

void LurkingModule::paint(juce::Graphics& g)
{
    const float lowCutHz = state.getRawParameterValue(ParamID::lowcut)->load();
    const float highCutHz = state.getRawParameterValue(ParamID::highcut)->load();
    const float mod = state.getRawParameterValue(ParamID::modulation)->load() / 100.0f;
    const bool bypassed = state.getRawParameterValue(ParamID::bypass)->load() > 0.5f;
    const float pw = bypassed ? 0.0f : 1.0f;

    auto area = getLocalBounds().toFloat();
    auto canvas = area.withTrimmedBottom(40.0f).reduced(4.0f, 6.0f);
    const float top = canvas.getY();
    const float base = canvas.getBottom();
    const float W = canvas.getWidth();

    juce::Path curve;
    juce::Path fillPath;
    const int steps = (int) W / 2;
    for (int i = 0; i <= steps; ++i)
    {
        const float p = (float) i / (float) steps;
        const float f = 20.0f * std::pow(1000.0f, p);
        float gVal = 1.0f / std::sqrt(1.0f + std::pow(lowCutHz / f, 3.4f))
                   / std::sqrt(1.0f + std::pow(f / highCutHz, 3.4f));
        gVal *= 1.0f + 0.03f * mod * std::sin(p * 16.0f + t * 0.7f) + 0.015f * std::sin(p * 6.0f - t * 0.4f);

        const float x = canvas.getX() + p * W;
        const float y = base - (base - top) * gVal;
        if (i == 0) { curve.startNewSubPath(x, y); fillPath.startNewSubPath(x, base); fillPath.lineTo(x, y); }
        else { curve.lineTo(x, y); fillPath.lineTo(x, y); }
    }
    fillPath.lineTo(canvas.getRight(), base);
    fillPath.closeSubPath();

    const float alpha = 0.35f + 0.65f * pw;

    g.setGradientFill(juce::ColourGradient(accent.withAlpha(0.16f * alpha), 0, top,
                                            accent.withAlpha(0.0f), 0, base, false));
    g.fillPath(fillPath);

    g.setColour(accent.withAlpha(0.28f * alpha));
    g.strokePath(curve, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour(accentLight.withAlpha(0.85f * alpha));
    g.strokePath(curve, juce::PathStrokeType(1.4f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    auto markerX = [&](float hz)
    {
        const float p = std::log(hz / 20.0f) / std::log(1000.0f);
        return canvas.getX() + juce::jlimit(0.0f, 1.0f, p) * W;
    };
    const float yMarker = base - (base - top) * 0.707f;
    for (float hz : { lowCutHz, highCutHz })
    {
        const float x = markerX(hz);
        g.setColour(accentHot.withAlpha(alpha));
        g.fillEllipse(juce::Rectangle<float>(4.4f, 4.4f).withCentre({ x, yMarker }));
        g.setColour(accentLight.withAlpha(0.14f * alpha));
        g.drawLine(x, yMarker + 6.0f, x, base, 1.0f);
    }
}

} // namespace dm::ui
