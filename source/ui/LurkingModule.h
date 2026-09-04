#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "DarkMatterLookAndFeel.h"

namespace dm::ui
{

/**
    DARK MATTER's visual interpretation of the filtering behaviour: an animated
    low-cut/high-cut frequency-response curve whose wobble amount is the
    MODULATION parameter. Dragging the module (like a knob) changes MODULATION.
*/
class LurkingModule : public juce::Component, private juce::Timer
{
public:
    LurkingModule(juce::AudioProcessorValueTreeState& apvts, DarkMatterLookAndFeel& laf);
    ~LurkingModule() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    void setResetNormalisedValue(float normalised);

private:
    juce::AudioProcessorValueTreeState& state;
    DarkMatterLookAndFeel& lookAndFeel;
    juce::Slider dragSlider { juce::Slider::LinearVertical, juce::Slider::NoTextBox };
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    juce::Label nameLabel, valueLabel;
    float t = 0.0f;

    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LurkingModule)
};

} // namespace dm::ui
