#include "PluginProcessor.h"
#include "PluginEditor.h"

DarkMatterProcessor::DarkMatterProcessor()
    : juce::AudioProcessor(BusesProperties()
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

void DarkMatterProcessor::prepareToPlay(double, int)
{
}

void DarkMatterProcessor::processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&)
{
}

bool DarkMatterProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
        && layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo();
}

juce::AudioProcessorEditor* DarkMatterProcessor::createEditor()
{
    return new DarkMatterEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DarkMatterProcessor();
}
