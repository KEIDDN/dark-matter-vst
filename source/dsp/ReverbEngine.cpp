#include "ReverbEngine.h"
#include <cmath>

namespace dm
{

namespace
{
    // Distinct, non-integer-ratio spread (ms) so the 8 lines don't beat/resonate together.
    constexpr std::array<float, 8> kBaseDelayMs { 29.7f, 37.1f, 41.3f, 47.9f, 53.3f, 59.7f, 61.1f, 67.3f };
    constexpr std::array<float, 8> kLfoRateHz    { 0.083f, 0.121f, 0.157f, 0.191f, 0.229f, 0.263f, 0.311f, 0.347f };
    constexpr std::array<float, 4> kDiffuserMs   { 5.10f, 7.73f, 10.00f, 12.61f };

    constexpr float kMaxModDepthMs = 3.0f;
    constexpr float kMaxFeedbackGain = 0.985f;
}

ReverbEngine::ReverbEngine()
{
    lfoRateHz = kLfoRateHz;
    baseDelayMs = kBaseDelayMs;
}

void ReverbEngine::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    juce::dsp::ProcessSpec monoSpec = spec;
    monoSpec.numChannels = 1;

    predelayLine.prepare(monoSpec);
    predelayLine.setMaximumDelayInSamples((int) (0.25 * sampleRate));

    const int maxLineDelay = (int) (0.35 * sampleRate);
    for (auto& line : lines)
    {
        line.prepare(monoSpec);
        line.setMaximumDelayInSamples(maxLineDelay);
    }

    for (size_t i = 0; i < diffusers.size(); ++i)
    {
        const int delaySamples = juce::jmax(1, (int) (kDiffuserMs[i] * 0.001 * sampleRate));
        diffusers[i].prepare(delaySamples);
    }

    for (auto& f : dampingFilters)
    {
        f.state = 0.0f;
        f.setCutoff(params.dampingHz, sampleRate);
    }

    lowCutL.prepare(monoSpec);
    lowCutR.prepare(monoSpec);
    highCutL.prepare(monoSpec);
    highCutR.prepare(monoSpec);

    updateToneFilters();
    reset();
}

void ReverbEngine::reset()
{
    predelayLine.reset();
    for (auto& line : lines)
        line.reset();
    for (auto& d : diffusers)
        d.reset();
    for (auto& f : dampingFilters)
        f.state = 0.0f;
    lfoPhase.fill(0.0f);

    lowCutL.reset();
    lowCutR.reset();
    highCutL.reset();
    highCutR.reset();
}

void ReverbEngine::updateToneFilters()
{
    *lowCutL.coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, juce::jlimit(10.0f, (float) sampleRate * 0.49f, params.lowCutHz));
    *lowCutR.coefficients = *lowCutL.coefficients;
    *highCutL.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, juce::jlimit(10.0f, (float) sampleRate * 0.49f, params.highCutHz));
    *highCutR.coefficients = *highCutL.coefficients;
}

void ReverbEngine::process(juce::dsp::AudioBlock<float>& block)
{
    const int numChannels = (int) block.getNumChannels();
    const int numSamples = (int) block.getNumSamples();
    if (numChannels < 1 || numSamples <= 0)
        return;

    updateToneFilters();

    for (auto& f : dampingFilters)
        f.setCutoff(params.dampingHz, sampleRate);

    const float diffuserGain = 0.15f + 0.55f * (params.diffusionPercent / 100.0f);
    for (auto& d : diffusers)
        d.gain = diffuserGain;

    const float sizeScale = 0.35f + 1.75f * (params.sizePercent / 100.0f);
    std::array<float, numLines> baseDelaySamples {};
    std::array<float, numLines> feedbackGain {};
    for (int i = 0; i < numLines; ++i)
    {
        baseDelaySamples[(size_t) i] = baseDelayMs[(size_t) i] * sizeScale * 0.001f * (float) sampleRate;
        const float loopSeconds = baseDelaySamples[(size_t) i] / (float) sampleRate;
        const float gain = std::pow(10.0f, -3.0f * loopSeconds / juce::jmax(0.05f, params.decaySeconds));
        feedbackGain[(size_t) i] = juce::jmin(kMaxFeedbackGain, gain);
    }

    const float modDepthSamples = (params.modulationPercent / 100.0f) * kMaxModDepthMs * 0.001f * (float) sampleRate;
    const float predelaySamples = juce::jlimit(0.0f, (float) (0.25 * sampleRate) - 4.0f, params.predelayMs * 0.001f * (float) sampleRate);
    const float mix = juce::jlimit(0.0f, 1.0f, params.mixPercent / 100.0f);
    const float diffuseInject = 1.0f / std::sqrt((float) numLines);
    const float wetTapNorm = 2.0f / (float) numLines;

    for (int n = 0; n < numSamples; ++n)
    {
        const float dryL = block.getSample(0, n);
        const float dryR = numChannels > 1 ? block.getSample(1, n) : dryL;
        const float monoIn = 0.5f * (dryL + dryR);

        predelayLine.pushSample(0, monoIn);
        float diffused = predelayLine.popSample(0, predelaySamples);

        for (auto& d : diffusers)
            diffused = d.process(diffused);

        std::array<float, numLines> lineOut {};
        for (int i = 0; i < numLines; ++i)
        {
            auto& phase = lfoPhase[(size_t) i];
            phase += (kLfoRateHz[(size_t) i] / (float) sampleRate) * juce::MathConstants<float>::twoPi;
            if (phase > juce::MathConstants<float>::twoPi)
                phase -= juce::MathConstants<float>::twoPi;

            const float modOffset = std::sin(phase) * modDepthSamples;
            const float delaySamples = juce::jmax(1.0f, baseDelaySamples[(size_t) i] + modOffset);

            lineOut[(size_t) i] = lines[(size_t) i].popSample(0, delaySamples, true);
        }

        float sum = 0.0f;
        for (float v : lineOut)
            sum += v;
        const float houseFactor = 2.0f / (float) numLines;

        for (int i = 0; i < numLines; ++i)
        {
            const float feedback = lineOut[(size_t) i] - houseFactor * sum;
            const float damped = dampingFilters[(size_t) i].process(feedback);
            const float fed = damped * feedbackGain[(size_t) i];
            const float inputToLine = fed + diffused * diffuseInject;
            lines[(size_t) i].pushSample(0, inputToLine);
        }

        float wetL = 0.0f, wetR = 0.0f;
        for (int i = 0; i < numLines; ++i)
        {
            if ((i % 2) == 0)
                wetL += lineOut[(size_t) i];
            else
                wetR += lineOut[(size_t) i];
        }
        wetL *= wetTapNorm;
        wetR *= wetTapNorm;

        wetL = lowCutL.processSample(wetL);
        wetL = highCutL.processSample(wetL);
        wetR = lowCutR.processSample(wetR);
        wetR = highCutR.processSample(wetR);

        const float outL = dryL * (1.0f - mix) + wetL * mix;
        const float outR = dryR * (1.0f - mix) + wetR * mix;

        block.setSample(0, n, outL);
        if (numChannels > 1)
            block.setSample(1, n, outR);
    }
}

} // namespace dm
