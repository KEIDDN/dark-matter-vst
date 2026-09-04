#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "DarkMatterLookAndFeel.h"

namespace dm::ui
{

/** Label + rotary slider + value readout, matching the prototype's knob module. */
class Knob : public juce::Component
{
public:
    Knob(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId,
         const juce::String& displayLabel, DarkMatterLookAndFeel& laf);

    void resized() override;

    /** Updates the double-click reset value to the given normalised [0,1] position,
        called whenever a preset is loaded so double-click resets to "this preset's value"
        rather than a fixed default — matching the prototype's onReset behaviour. */
    void setResetNormalisedValue(float normalised);

    juce::Slider slider { juce::Slider::RotaryVerticalDrag, juce::Slider::NoTextBox };

private:
    juce::AudioProcessorValueTreeState& state;
    juce::String paramId;
    juce::Label nameLabel, valueLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;

    void updateValueLabel();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Knob)
};

} // namespace dm::ui
