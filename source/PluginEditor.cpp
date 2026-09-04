#include "PluginEditor.h"
#include "ui/Colors.h"
#include "dsp/Parameters.h"

using namespace dm::ui;

namespace
{
    constexpr int kWindowWidth = 1200;
    constexpr int kWindowHeight = 800;
}

DarkMatterEditor::DarkMatterEditor(DarkMatterProcessor& p)
    : juce::AudioProcessorEditor(&p), audioProcessor(p),
      presetBar(p.presetManager, lookAndFeel, [this] { syncKnobResetValues(); }),
      orb(p.apvts),
      lurking(p.apvts, lookAndFeel),
      inMeter(p.inputLevel, LevelMeter::FillFrom::Left),
      outMeter(p.outputLevel, LevelMeter::FillFrom::Right)
{
    setLookAndFeel(&lookAndFeel);

    titleLabel.setText("DARK MATTER", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setFont(lookAndFeel.getSyncopate(21.0f).withExtraKerningFactor(0.36f));
    titleLabel.setColour(juce::Label::textColourId, headline);
    titleLabel.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(titleLabel);

    subtitleLabel.setText("Space Reverb", juce::dontSendNotification);
    subtitleLabel.setJustificationType(juce::Justification::centred);
    subtitleLabel.setFont(lookAndFeel.getGeist(9.0f, 400).withExtraKerningFactor(0.52f));
    subtitleLabel.setColour(juce::Label::textColourId, textFaint);
    subtitleLabel.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(subtitleLabel);

    bypassButton.setClickingTogglesState(true);
    bypassButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    bypassButton.setColour(juce::TextButton::buttonOnColourId, accent.withAlpha(0.12f));
    bypassButton.setColour(juce::TextButton::textColourOffId, textDim);
    bypassButton.setColour(juce::TextButton::textColourOnId, juce::Colour(0xffe6dcff));
    addAndMakeVisible(bypassButton);
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        p.apvts, dm::ParamID::bypass, bypassButton);

    addAndMakeVisible(presetBar);
    addAndMakeVisible(orb);

    struct KnobSpec { const char* id; const char* label; };
    static const KnobSpec leftSpecs[] { { dm::ParamID::predelay, "PRE-DELAY" },
                                        { dm::ParamID::decay, "DECAY" },
                                        { dm::ParamID::size, "SIZE" } };
    static const KnobSpec rightSpecs[] { { dm::ParamID::damping, "DAMPING" },
                                         { dm::ParamID::diffusion, "DIFFUSION" },
                                         { dm::ParamID::mix, "MIX" } };
    for (auto& s : leftSpecs)
    {
        auto k = std::make_unique<Knob>(p.apvts, s.id, s.label, lookAndFeel);
        addAndMakeVisible(*k);
        leftKnobs.push_back(std::move(k));
    }
    for (auto& s : rightSpecs)
    {
        auto k = std::make_unique<Knob>(p.apvts, s.id, s.label, lookAndFeel);
        addAndMakeVisible(*k);
        rightKnobs.push_back(std::move(k));
    }

    lowCutKnob = std::make_unique<Knob>(p.apvts, dm::ParamID::lowcut, "LOW CUT", lookAndFeel);
    highCutKnob = std::make_unique<Knob>(p.apvts, dm::ParamID::highcut, "HIGH CUT", lookAndFeel);
    addAndMakeVisible(*lowCutKnob);
    addAndMakeVisible(*highCutKnob);
    addAndMakeVisible(lurking);

    addAndMakeVisible(inMeter);
    addAndMakeVisible(outMeter);

    inLabel.setText("IN", juce::dontSendNotification);
    outLabel.setText("OUT", juce::dontSendNotification);
    for (auto* l : { &inLabel, &outLabel })
    {
        l->setJustificationType(juce::Justification::centred);
        l->setFont(lookAndFeel.getGeist(10.0f, 400).withExtraKerningFactor(0.2f));
        l->setColour(juce::Label::textColourId, textDim);
        addAndMakeVisible(*l);
    }

    signatureLabel.setText("by Keiddn", juce::dontSendNotification);
    signatureLabel.setJustificationType(juce::Justification::centred);
    signatureLabel.setFont(lookAndFeel.getGeist(12.0f, 300).withExtraKerningFactor(0.14f));
    signatureLabel.setColour(juce::Label::textColourId, textDim);
    addAndMakeVisible(signatureLabel);

    p.presetManager.addChangeListener(this);
    syncKnobResetValues();

    setSize(kWindowWidth, kWindowHeight);
}

DarkMatterEditor::~DarkMatterEditor()
{
    audioProcessor.presetManager.removeChangeListener(this);
    setLookAndFeel(nullptr);
}

void DarkMatterEditor::changeListenerCallback(juce::ChangeBroadcaster*)
{
    syncKnobResetValues();
    orb.triggerPulse();
}

void DarkMatterEditor::syncKnobResetValues()
{
    const auto v = audioProcessor.presetManager.getCurrentPresetValues();
    // Order matches dm::ParamID::orderedIds: predelay, decay, size, damping, diffusion, mix, lowcut, highcut, modulation
    leftKnobs[0]->setResetNormalisedValue(v[0]);
    leftKnobs[1]->setResetNormalisedValue(v[1]);
    leftKnobs[2]->setResetNormalisedValue(v[2]);
    rightKnobs[0]->setResetNormalisedValue(v[3]);
    rightKnobs[1]->setResetNormalisedValue(v[4]);
    rightKnobs[2]->setResetNormalisedValue(v[5]);
    lowCutKnob->setResetNormalisedValue(v[6]);
    highCutKnob->setResetNormalisedValue(v[7]);
    lurking.setResetNormalisedValue(v[8]);
}

void DarkMatterEditor::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    juce::ColourGradient bg(bgCenter, b.getCentreX(), b.getHeight() * 0.45f,
                             bgOuter, b.getCentreX(), 0.0f, true);
    bg.addColour(0.5, bgMid);
    g.setGradientFill(bg);
    g.fillRoundedRectangle(b, 26.0f);

    g.setColour(juce::Colours::white.withAlpha(0.07f));
    g.drawRoundedRectangle(b.reduced(0.5f), 26.0f, 1.0f);

    // Orb outer ring guide.
    juce::Rectangle<float> ring(376.0f, 120.0f, 448.0f, 448.0f);
    g.setColour(juce::Colours::white.withAlpha(0.035f));
    g.drawEllipse(ring, 1.0f);

    // Bottom panel chrome.
    juce::Rectangle<float> panel(352.0f, 596.0f, 496.0f, 120.0f);
    g.setGradientFill(juce::ColourGradient(juce::Colours::white.withAlpha(0.028f), panel.getX(), panel.getY(),
                                            juce::Colours::white.withAlpha(0.006f), panel.getX(), panel.getBottom(), false));
    g.fillRoundedRectangle(panel, 24.0f);
    g.setColour(panelBorder);
    g.drawRoundedRectangle(panel.reduced(0.5f), 24.0f, 1.0f);
    g.drawLine(panel.getX() + panel.getWidth() * 0.333f, panel.getY() + 14.0f,
               panel.getX() + panel.getWidth() * 0.333f, panel.getBottom() - 14.0f, 1.0f);
    g.drawLine(panel.getX() + panel.getWidth() * 0.727f, panel.getY() + 14.0f,
               panel.getX() + panel.getWidth() * 0.727f, panel.getBottom() - 14.0f, 1.0f);
}

void DarkMatterEditor::resized()
{
    presetBar.setBounds(44, 34, 316, 456);

    titleLabel.setBounds(0, 40, kWindowWidth, 26);
    subtitleLabel.setBounds(0, 68, kWindowWidth, 16);

    bypassButton.setBounds(kWindowWidth - 44 - 88, 34, 88, 34);

    auto layoutColumn = std::vector<dm::ui::Knob*> {};
    const int knobW = 130, knobH = 150;
    int y = 158;
    for (size_t i = 0; i < leftKnobs.size(); ++i)
        leftKnobs[i]->setBounds(114, y + (int) i * (knobH + 30) - (knobH - 96) / 2, knobW, knobH);
    for (size_t i = 0; i < rightKnobs.size(); ++i)
        rightKnobs[i]->setBounds(kWindowWidth - 114 - knobW, y + (int) i * (knobH + 30) - (knobH - 96) / 2, knobW, knobH);

    orb.setBounds(392, 136, 416, 416);

    juce::Rectangle<int> panel(352, 596, 496, 120);
    auto lowArea = panel.removeFromLeft(165);
    auto highArea = panel.removeFromRight(165);
    auto lurkArea = panel;
    lowCutKnob->setBounds(lowArea.withSizeKeepingCentre(90, 90));
    highCutKnob->setBounds(highArea.withSizeKeepingCentre(90, 90));
    lurking.setBounds(lurkArea.reduced(6));

    inLabel.setBounds(44, 760, 60, 20);
    inMeter.setBounds(112, 766, 200, 8);
    outMeter.setBounds(kWindowWidth - 44 - 200 - 60, 766, 200, 8);
    outLabel.setBounds(kWindowWidth - 44 - 60, 760, 60, 20);
    signatureLabel.setBounds(0, 762, kWindowWidth, 20);
}
