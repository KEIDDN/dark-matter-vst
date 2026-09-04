#include "Knob.h"
#include "Colors.h"
#include "../dsp/Parameters.h"

namespace dm::ui
{

namespace
{
    juce::String formatForParam(const juce::String& paramId, double value)
    {
        using namespace dm;
        if (paramId == ParamID::mix || paramId == ParamID::size
            || paramId == ParamID::diffusion || paramId == ParamID::modulation)
            return juce::String((int) std::round(value)) + " %";

        if (paramId == ParamID::decay)
            return juce::String(value, 1) + " s";

        if (paramId == ParamID::predelay)
            return juce::String((int) std::round(value)) + " ms";

        if (paramId == ParamID::damping || paramId == ParamID::highcut || paramId == ParamID::lowcut)
            return value >= 1000.0 ? juce::String(value / 1000.0, 1) + " kHz"
                                    : juce::String((int) std::round(value)) + " Hz";

        return juce::String(value);
    }
}

Knob::Knob(juce::AudioProcessorValueTreeState& apvts, const juce::String& id,
           const juce::String& displayLabel, DarkMatterLookAndFeel& laf)
    : state(apvts), paramId(id)
{
    // JUCE requires non-negative rotary angles, so -135deg/+135deg become 225deg/495deg
    // (same absolute sweep, expressed in JUCE's [0, 4pi) convention).
    slider.setRotaryParameters(juce::degreesToRadians(225.0f), juce::degreesToRadians(495.0f), true);
    slider.setMouseDragSensitivity(240);
    slider.setLookAndFeel(&laf);
    slider.onValueChange = [this] { updateValueLabel(); };
    addAndMakeVisible(slider);

    nameLabel.setText(displayLabel, juce::dontSendNotification);
    nameLabel.setJustificationType(juce::Justification::centred);
    nameLabel.setFont(laf.getGeist(10.5f, 400));
    nameLabel.setColour(juce::Label::textColourId, textDim);
    addAndMakeVisible(nameLabel);

    valueLabel.setJustificationType(juce::Justification::centred);
    valueLabel.setFont(laf.getGeistMono(12.5f, 300));
    valueLabel.setColour(juce::Label::textColourId, textBright.withAlpha(0.8f));
    addAndMakeVisible(valueLabel);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, paramId, slider);
    updateValueLabel();
}

void Knob::updateValueLabel()
{
    valueLabel.setText(formatForParam(paramId, slider.getValue()), juce::dontSendNotification);
}

void Knob::setResetNormalisedValue(float normalised)
{
    if (auto* p = state.getParameter(paramId))
        slider.setDoubleClickReturnValue(true, p->convertFrom0to1(normalised));
}

void Knob::resized()
{
    auto b = getLocalBounds();
    nameLabel.setBounds(b.removeFromTop(16));
    valueLabel.setBounds(b.removeFromBottom(18));
    slider.setBounds(b.reduced((b.getWidth() - juce::jmin(b.getWidth(), b.getHeight())) / 2, 0));
}

} // namespace dm::ui
