#pragma once

#include <juce_dsp/juce_dsp.h>
#include <array>

namespace dm
{

/**
    An 8-line feedback delay network (FDN) reverb with input diffusion,
    per-line damping, delay-line modulation, and output tone shaping.

    This is DARK MATTER's sonic core: a mono-in, stereo-out "gravitational
    field" that traps the signal, smears it through diffusion, and lets it
    decay according to `decaySeconds`.
*/
class ReverbEngine
{
public:
    ReverbEngine();

    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();

    struct Params
    {
        float mixPercent = 35.0f;
        float sizePercent = 50.0f;
        float decaySeconds = 2.5f;
        float predelayMs = 20.0f;
        float dampingHz = 8000.0f;
        float diffusionPercent = 65.0f;
        float modulationPercent = 35.0f;
        float lowCutHz = 30.0f;
        float highCutHz = 18000.0f;
    };

    void setParameters(const Params& newParams) { params = newParams; }

    /** Processes a stereo block in place. */
    void process(juce::dsp::AudioBlock<float>& block);

private:
    static constexpr int numLines = 8;
    static constexpr int numDiffusers = 4;

    struct OnePoleLowpass
    {
        float state = 0.0f;
        float coeff = 0.0f;

        void setCutoff(float hz, double rate)
        {
            coeff = std::exp(-2.0f * juce::MathConstants<float>::pi * hz / (float) rate);
        }

        float process(float x)
        {
            state = (1.0f - coeff) * x + coeff * state;
            return state;
        }
    };

    struct AllpassDiffuser
    {
        std::vector<float> buffer;
        int writeIndex = 0;
        int delaySamples = 1;
        float gain = 0.5f;

        void prepare(int delayInSamples)
        {
            delaySamples = juce::jmax(1, delayInSamples);
            buffer.assign((size_t) delaySamples, 0.0f);
            writeIndex = 0;
        }

        void reset() { std::fill(buffer.begin(), buffer.end(), 0.0f); }

        float process(float x)
        {
            const float delayed = buffer[(size_t) writeIndex];
            const float y = -gain * x + delayed;
            buffer[(size_t) writeIndex] = x + gain * y;
            writeIndex = (writeIndex + 1) % delaySamples;
            return y;
        }
    };

    Params params;
    double sampleRate = 44100.0;

    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> predelayLine;
    std::array<AllpassDiffuser, numDiffusers> diffusers;
    std::array<juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd>, numLines> lines;
    std::array<float, numLines> baseDelayMs;
    std::array<OnePoleLowpass, numLines> dampingFilters;
    std::array<float, numLines> lfoPhase {};
    std::array<float, numLines> lfoRateHz;

    juce::dsp::IIR::Filter<float> lowCutL, lowCutR, highCutL, highCutR;

    void updateToneFilters();
};

} // namespace dm
