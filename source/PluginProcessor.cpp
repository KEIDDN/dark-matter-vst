#include "PluginProcessor.h"
#include "PluginEditor.h"

DarkMatterProcessor::DarkMatterProcessor()
    : juce::AudioProcessor(BusesProperties()
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMETERS", dm::createParameterLayout())
{
}

void DarkMatterProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (juce::uint32) samplesPerBlock;
    spec.numChannels = 2;
    reverb.prepare(spec);
}

void DarkMatterProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    for (int ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear(ch, 0, buffer.getNumSamples());

    dm::ReverbEngine::Params p;
    p.mixPercent = apvts.getRawParameterValue(dm::ParamID::mix)->load();
    p.sizePercent = apvts.getRawParameterValue(dm::ParamID::size)->load();
    p.decaySeconds = apvts.getRawParameterValue(dm::ParamID::decay)->load();
    p.predelayMs = apvts.getRawParameterValue(dm::ParamID::predelay)->load();
    p.dampingHz = apvts.getRawParameterValue(dm::ParamID::damping)->load();
    p.diffusionPercent = apvts.getRawParameterValue(dm::ParamID::diffusion)->load();
    p.modulationPercent = apvts.getRawParameterValue(dm::ParamID::modulation)->load();
    p.lowCutHz = apvts.getRawParameterValue(dm::ParamID::lowcut)->load();
    p.highCutHz = apvts.getRawParameterValue(dm::ParamID::highcut)->load();
    reverb.setParameters(p);

    juce::dsp::AudioBlock<float> block(buffer);
    reverb.process(block);
}

bool DarkMatterProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
        && layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo();
}

void DarkMatterProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    if (auto xml = state.createXml())
        copyXmlToBinary(*xml, destData);
}

void DarkMatterProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessorEditor* DarkMatterProcessor::createEditor()
{
    return new DarkMatterEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DarkMatterProcessor();
}
