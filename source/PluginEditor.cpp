#include "PluginEditor.h"

DarkMatterEditor::DarkMatterEditor(DarkMatterProcessor& p)
    : juce::AudioProcessorEditor(&p), processor(p)
{
    setSize(1200, 800);
}

void DarkMatterEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(0x05, 0x05, 0x06));
    g.setColour(juce::Colours::white.withAlpha(0.6f));
    g.setFont(20.0f);
    g.drawFittedText("DARK MATTER — toolchain OK", getLocalBounds(), juce::Justification::centred, 1);
}
