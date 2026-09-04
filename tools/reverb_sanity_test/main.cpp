// Offline correctness check for dm::ReverbEngine — not shipped in the plugin.
// Feeds an impulse through the engine and verifies:
//   1. output stays finite (no NaN/Inf, no runaway feedback)
//   2. the tail actually decays over time (it is a reverb, not an oscillator)
//   3. the measured decay time is in a sane ballpark of the requested decay

#include <juce_dsp/juce_dsp.h>
#include "../../source/dsp/ReverbEngine.h"
#include <cstdio>
#include <cmath>
#include <vector>

int main()
{
    constexpr double sampleRate = 48000.0;
    constexpr int blockSize = 512;
    constexpr double lengthSeconds = 10.0;
    constexpr float requestedDecay = 3.0f;

    dm::ReverbEngine reverb;
    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) blockSize, 2 };
    reverb.prepare(spec);

    dm::ReverbEngine::Params p;
    p.mixPercent = 100.0f; // full wet, so we're measuring the tail cleanly
    p.sizePercent = 50.0f;
    p.decaySeconds = requestedDecay;
    p.predelayMs = 0.0f;
    p.dampingHz = 8000.0f;
    p.diffusionPercent = 65.0f;
    p.modulationPercent = 35.0f;
    p.lowCutHz = 30.0f;
    p.highCutHz = 18000.0f;
    reverb.setParameters(p);

    const int totalSamples = (int) (lengthSeconds * sampleRate);
    juce::AudioBuffer<float> buffer(2, totalSamples);
    buffer.clear();
    buffer.setSample(0, 0, 1.0f);
    buffer.setSample(1, 0, 1.0f);

    bool finite = true;
    std::vector<float> envelope; // RMS per analysis window
    constexpr int windowSize = 2048;

    for (int start = 0; start < totalSamples; start += blockSize)
    {
        const int n = juce::jmin(blockSize, totalSamples - start);
        juce::AudioBuffer<float> block(2, n);
        for (int ch = 0; ch < 2; ++ch)
            block.copyFrom(ch, 0, buffer, ch, start, n);

        juce::dsp::AudioBlock<float> ab(block);
        reverb.process(ab);

        for (int ch = 0; ch < 2; ++ch)
            buffer.copyFrom(ch, start, block, ch, 0, n);
    }

    for (int start = 0; start < totalSamples; start += windowSize)
    {
        const int n = juce::jmin(windowSize, totalSamples - start);
        double sumSq = 0.0;
        for (int ch = 0; ch < 2; ++ch)
        {
            const float* d = buffer.getReadPointer(ch, start);
            for (int i = 0; i < n; ++i)
            {
                const float v = d[i];
                if (!std::isfinite(v))
                    finite = false;
                sumSq += (double) v * (double) v;
            }
        }
        envelope.push_back((float) std::sqrt(sumSq / (2.0 * n)));
    }

    if (!finite)
    {
        std::printf("FAIL: non-finite output detected (NaN/Inf) — feedback network is unstable.\n");
        return 1;
    }

    float peak = 0.0f;
    int peakIdx = 0;
    for (int i = 0; i < (int) envelope.size(); ++i)
        if (envelope[(size_t) i] > peak) { peak = envelope[(size_t) i]; peakIdx = i; }

    if (peak <= 0.0f)
    {
        std::printf("FAIL: output is silent — reverb is not producing a tail.\n");
        return 1;
    }

    const float target = peak * (float) std::pow(10.0, -60.0 / 20.0);
    int rt60Idx = -1;
    for (int i = peakIdx; i < (int) envelope.size(); ++i)
    {
        if (envelope[(size_t) i] < target)
        {
            rt60Idx = i;
            break;
        }
    }

    const double windowSeconds = windowSize / sampleRate;
    const double peakTime = peakIdx * windowSeconds;

    std::printf("Peak RMS: %.6f at t=%.3fs\n", (double) peak, peakTime);

    if (rt60Idx < 0)
    {
        std::printf("FAIL: tail never decayed 60dB within %.1fs — decay is not working (or requested decay too long for this test window).\n", lengthSeconds);
        return 1;
    }

    const double measuredRT60 = rt60Idx * windowSeconds - peakTime;
    std::printf("Measured RT60: %.3fs (requested decay parameter: %.3fs)\n", measuredRT60, (double) requestedDecay);

    // Damping/diffusion/Householder losses mean actual RT60 will run somewhat
    // shorter than the per-line nominal target — allow a generous but bounded range.
    if (measuredRT60 < requestedDecay * 0.25 || measuredRT60 > requestedDecay * 2.0)
    {
        std::printf("FAIL: measured RT60 is far outside a sane range of the requested decay.\n");
        return 1;
    }

    std::printf("PASS: reverb is stable and decays plausibly.\n");
    return 0;
}
