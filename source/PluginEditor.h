#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "ui/DarkMatterLookAndFeel.h"
#include "ui/Knob.h"
#include "ui/LurkingModule.h"
#include "ui/BlackHoleOrb.h"
#include "ui/LevelMeter.h"
#include "ui/PresetBar.h"

class DarkMatterEditor : public juce::AudioProcessorEditor, private juce::ChangeListener
{
public:
    explicit DarkMatterEditor(DarkMatterProcessor&);
    ~DarkMatterEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    DarkMatterProcessor& audioProcessor;
    dm::ui::DarkMatterLookAndFeel lookAndFeel;

    juce::Label titleLabel, subtitleLabel, signatureLabel, inLabel, outLabel;
    juce::TextButton bypassButton { "BYPASS" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;

    dm::ui::PresetBar presetBar;
    dm::ui::BlackHoleOrb orb;

    std::vector<std::unique_ptr<dm::ui::Knob>> leftKnobs, rightKnobs;
    std::unique_ptr<dm::ui::Knob> lowCutKnob, highCutKnob;
    dm::ui::LurkingModule lurking;

    dm::ui::LevelMeter inMeter, outMeter;

    void changeListenerCallback(juce::ChangeBroadcaster*) override;
    void syncKnobResetValues();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DarkMatterEditor)
};
