#include "ReverbEngine.h"
#include <cmath>

namespace dm
{

namespace
{
    // Distinct, non-integer-ratio spread (ms) so the 8 lines don't beat/resonate together.
    constexpr std::array<float, 8> kBaseDelayMs { 29.7f, 37.1f, 41.3f, 47.9f, 53.3f, 59.7f, 61.1f, 67.3f };
    // Primary (audible-rate) and secondary (slow-wander) LFO rates per line. Summing
    // two mutually-irregular sines per line (instead of one) turns the modulation
    // from a pure, obviously-periodic wobble into a quasi-periodic, evolving one.
    constexpr std::array<float, 8> kLfoRateHz    { 0.083f, 0.121f, 0.157f, 0.191f, 0.229f, 0.263f, 0.311f, 0.347f };
    constexpr std::array<float, 8> kLfoRateHz2   { 0.019f, 0.026f, 0.031f, 0.037f, 0.041f, 0.048f, 0.053f, 0.059f };
    constexpr std::array<float, 4> kDiffuserMsA  { 5.10f, 7.73f, 10.00f, 12.61f };
    constexpr std::array<float, 4> kDiffuserMsB  { 6.30f, 8.87f, 11.20f, 13.97f };

    constexpr float kMaxModDepthMs = 3.0f;
    // The FDN's feedback matrix (see the Householder mix below) is unitary, i.e.
    // energy-preserving on its own - any individual line's feedback gain <1 is
    // therefore already guaranteed stable, with no risk of runaway feedback. This
    // ceiling is just a numerical safety margin (see computeFeedbackGain()), not a
    // decay-time limiter: it used to be 0.985, which silently capped the real RT60
    // of the short lines/low SIZE well below whatever DECAY asked for.
    constexpr float kMaxFeedbackGain = 0.9995f;
}

ReverbEngine::ReverbEngine()
{
    lfoRateHz = kLfoRateHz;
    baseDelayMs = kBaseDelayMs;

    // Spread each line's starting LFO phase using the golden angle so all 8
    // start decorrelated instead of all wobbling in lockstep from t=0 (which
    // otherwise reads as an obvious, synchronised chorus right after load).
    for (int i = 0; i < numLines; ++i)
        lfoPhase[(size_t) i] = std::fmod((float) i * 2.399963f, juce::MathConstants<float>::twoPi);
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

    for (size_t i = 0; i < diffusersA.size(); ++i)
    {
        diffusersA[i].prepare(juce::jmax(1, (int) (kDiffuserMsA[i] * 0.001 * sampleRate)));
        diffusersB[i].prepare(juce::jmax(1, (int) (kDiffuserMsB[i] * 0.001 * sampleRate)));
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

    const float sizeScale = computeSizeScale();
    smoothedSizeScale.reset(sampleRate, kSmoothingTimeSeconds);
    smoothedSizeScale.setCurrentAndTargetValue(sizeScale);

    smoothedPredelaySamples.reset(sampleRate, kSmoothingTimeSeconds);
    smoothedPredelaySamples.setCurrentAndTargetValue(
        juce::jlimit(0.0f, (float) (0.25 * sampleRate) - 4.0f, params.predelayMs * 0.001f * (float) sampleRate));

    smoothedModDepthSamples.reset(sampleRate, kSmoothingTimeSeconds);
    smoothedModDepthSamples.setCurrentAndTargetValue(
        (params.modulationPercent / 100.0f) * kMaxModDepthMs * 0.001f * (float) sampleRate);

    smoothedDampingCoeff.reset(sampleRate, kSmoothingTimeSeconds);
    smoothedDampingCoeff.setCurrentAndTargetValue(OnePoleLowpass::coeffForCutoff(params.dampingHz, sampleRate));

    for (int i = 0; i < numLines; ++i)
    {
        smoothedFeedbackGain[(size_t) i].reset(sampleRate, kSmoothingTimeSeconds);
        smoothedFeedbackGain[(size_t) i].setCurrentAndTargetValue(computeFeedbackGain(i, sizeScale));
    }

    updateToneFilters();
    reset();
}

void ReverbEngine::reset()
{
    predelayLine.reset();
    for (auto& line : lines)
        line.reset();
    for (auto& d : diffusersA)
        d.reset();
    for (auto& d : diffusersB)
        d.reset();
    for (auto& f : dampingFilters)
        f.state = 0.0f;

    // Same golden-angle spread as the constructor (see there for why) - reapplied
    // here too since reset() can run again later (e.g. on host transport stop).
    for (int i = 0; i < numLines; ++i)
    {
        const float offset = std::fmod((float) i * 2.399963f, juce::MathConstants<float>::twoPi);
        lfoPhase[(size_t) i] = offset;
        lfoPhase2[(size_t) i] = offset * 0.5f;
    }

    lowCutL.reset();
    lowCutR.reset();
    highCutL.reset();
    highCutR.reset();
}

float ReverbEngine::computeSizeScale() const
{
    return 0.35f + 1.75f * (params.sizePercent / 100.0f);
}

float ReverbEngine::computeFeedbackGain(int lineIndex, float sizeScale) const
{
    const float delaySamples = baseDelayMs[(size_t) lineIndex] * sizeScale * 0.001f * (float) sampleRate;
    const float loopSeconds = delaySamples / (float) sampleRate;
    const float gain = std::pow(10.0f, -3.0f * loopSeconds / juce::jmax(0.05f, params.decaySeconds));
    return juce::jmin(kMaxFeedbackGain, gain);
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

    // Cheap per-block target computation (unchanged formulas) - the actual
    // values used in the sample loop below ramp linearly toward these.
    const float diffuserGain = 0.15f + 0.55f * (params.diffusionPercent / 100.0f);
    for (auto& d : diffusersA)
        d.gain = diffuserGain;
    for (auto& d : diffusersB)
        d.gain = diffuserGain;

    const float sizeScale = computeSizeScale();
    smoothedSizeScale.setTargetValue(sizeScale);
    for (int i = 0; i < numLines; ++i)
        smoothedFeedbackGain[(size_t) i].setTargetValue(computeFeedbackGain(i, sizeScale));

    smoothedModDepthSamples.setTargetValue((params.modulationPercent / 100.0f) * kMaxModDepthMs * 0.001f * (float) sampleRate);
    smoothedPredelaySamples.setTargetValue(
        juce::jlimit(0.0f, (float) (0.25 * sampleRate) - 4.0f, params.predelayMs * 0.001f * (float) sampleRate));
    smoothedDampingCoeff.setTargetValue(OnePoleLowpass::coeffForCutoff(params.dampingHz, sampleRate));

    const float mix = juce::jlimit(0.0f, 1.0f, params.mixPercent / 100.0f);
    const float diffuseInject = 1.0f / std::sqrt((float) numLines);
    const float wetTapNorm = 2.0f / (float) numLines;

    for (int n = 0; n < numSamples; ++n)
    {
        const float curSizeScale = smoothedSizeScale.getNextValue();
        const float curPredelaySamples = smoothedPredelaySamples.getNextValue();
        const float curModDepthSamples = smoothedModDepthSamples.getNextValue();
        const float curDampingCoeff = smoothedDampingCoeff.getNextValue();

        const float dryL = block.getSample(0, n);
        const float dryR = numChannels > 1 ? block.getSample(1, n) : dryL;
        const float monoIn = 0.5f * (dryL + dryR);

        predelayLine.pushSample(0, monoIn);
        const float predelayed = predelayLine.popSample(0, curPredelaySamples);

        float diffusedA = predelayed, diffusedB = predelayed;
        for (auto& d : diffusersA)
            diffusedA = d.process(diffusedA);
        for (auto& d : diffusersB)
            diffusedB = d.process(diffusedB);

        std::array<float, numLines> lineOut {};
        for (int i = 0; i < numLines; ++i)
        {
            auto& phase = lfoPhase[(size_t) i];
            phase += (kLfoRateHz[(size_t) i] / (float) sampleRate) * juce::MathConstants<float>::twoPi;
            if (phase > juce::MathConstants<float>::twoPi)
                phase -= juce::MathConstants<float>::twoPi;

            auto& phase2 = lfoPhase2[(size_t) i];
            phase2 += (kLfoRateHz2[(size_t) i] / (float) sampleRate) * juce::MathConstants<float>::twoPi;
            if (phase2 > juce::MathConstants<float>::twoPi)
                phase2 -= juce::MathConstants<float>::twoPi;

            // Two mutually-irregular sines per line instead of one pure tone -
            // reads as an organic, evolving wobble rather than a fixed-rate
            // chorus/vibrato, while keeping the same total max excursion.
            const float modOffset = (0.7f * std::sin(phase) + 0.3f * std::sin(phase2)) * curModDepthSamples;
            const float delaySamples = juce::jmax(1.0f, baseDelayMs[(size_t) i] * curSizeScale * 0.001f * (float) sampleRate + modOffset);

            lineOut[(size_t) i] = lines[(size_t) i].popSample(0, delaySamples, true);
        }

        float sum = 0.0f;
        for (float v : lineOut)
            sum += v;
        const float houseFactor = 2.0f / (float) numLines;

        for (int i = 0; i < numLines; ++i)
        {
            const float feedback = lineOut[(size_t) i] - houseFactor * sum;
            dampingFilters[(size_t) i].coeff = curDampingCoeff;
            const float damped = dampingFilters[(size_t) i].process(feedback);
            const float fed = damped * smoothedFeedbackGain[(size_t) i].getNextValue();
            const float diffused = (i % 2) == 0 ? diffusedA : diffusedB;
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
