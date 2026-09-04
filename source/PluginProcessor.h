#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include "dsp/Parameters.h"
#include "dsp/ReverbEngine.h"
#include "presets/PresetManager.h"

class DarkMatterProcessor : public juce::AudioProcessor
{
public:
    DarkMatterProcessor();
    ~DarkMatterProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Dark Matter"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    // Was hardcoded to 4.0, unrelated to the actual DECAY parameter (up to
    // dm::kMaxDecaySeconds = 20s) - hosts could cut the tail short on
    // bounce/export or after transport stop. +0.3s covers predelay/diffusion
    // latency ahead of the tail itself.
    double getTailLengthSeconds() const override { return (double) dm::kMaxDecaySeconds + 0.3; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    // Lets a host's own bypass control (distinct from automating our "bypass"
    // parameter directly, e.g. a mixer-strip bypass button) route through the
    // same parameter processBlock() already checks.
    juce::AudioProcessorParameter* getBypassParameter() const override
    {
        return apvts.getParameter(dm::ParamID::bypass);
    }

    juce::AudioProcessorValueTreeState apvts;
    dm::PresetManager presetManager { apvts };

    std::atomic<float> inputLevel { 0.0f };
    std::atomic<float> outputLevel { 0.0f };

private:
    dm::ReverbEngine reverb;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DarkMatterProcessor)
};
