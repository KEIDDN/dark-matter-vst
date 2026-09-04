#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <cmath>
#include <array>

namespace dm
{
    // Single source of truth for the DECAY parameter's max range, so
    // AudioProcessor::getTailLengthSeconds() can't silently drift out of
    // sync with it (see decayRange below).
    inline constexpr float kMaxDecaySeconds = 20.0f;

namespace ParamID
{
    static constexpr const char* mix = "mix";
    static constexpr const char* size = "size";
    static constexpr const char* decay = "decay";
    static constexpr const char* predelay = "predelay";
    static constexpr const char* damping = "damping";
    static constexpr const char* diffusion = "diffusion";
    static constexpr const char* modulation = "modulation";
    static constexpr const char* lowcut = "lowcut";
    static constexpr const char* highcut = "highcut";
    static constexpr const char* bypass = "bypass";

    // Matches the prototype's KEYS order — also the order factory/user preset
    // value arrays are stored in.
    static constexpr std::array<const char*, 9> orderedIds {
        predelay, decay, size, damping, diffusion, mix, lowcut, highcut, modulation
    };
}

inline juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> params;

    // These curves intentionally match the knob-position -> value formulas from
    // the Dark Matter UI prototype (Dark Matter v4.dc.html, FMT table) exactly,
    // so a knob turned to a given angle always means the same value everywhere,
    // and factory preset data (stored as prototype knob positions) converts
    // losslessly into real parameter units below.
    NormalisableRange<float> decayRange(
        0.2f, kMaxDecaySeconds,
        [](float, float, float v) { return 0.2f + v * v * 19.8f; },
        [](float, float, float value) { return std::sqrt(juce::jmax(0.0f, (value - 0.2f) / 19.8f)); });

    NormalisableRange<float> lowCutRange(
        20.0f, 1000.0f,
        [](float, float, float v) { return 20.0f * std::pow(50.0f, v); },
        [](float, float, float value) { return std::log(value / 20.0f) / std::log(50.0f); });

    NormalisableRange<float> highRange(
        1000.0f, 20000.0f,
        [](float, float, float v) { return 1000.0f * std::pow(20.0f, v); },
        [](float, float, float value) { return std::log(value / 1000.0f) / std::log(20.0f); });

    params.push_back(std::make_unique<AudioParameterFloat>(
        ParameterID{ ParamID::mix, 1 }, "Mix",
        NormalisableRange<float>(0.0f, 100.0f), 35.0f,
        AudioParameterFloatAttributes().withLabel("%")));

    params.push_back(std::make_unique<AudioParameterFloat>(
        ParameterID{ ParamID::size, 1 }, "Size",
        NormalisableRange<float>(0.0f, 100.0f), 50.0f,
        AudioParameterFloatAttributes().withLabel("%")));

    params.push_back(std::make_unique<AudioParameterFloat>(
        ParameterID{ ParamID::decay, 1 }, "Decay",
        decayRange, 2.5f,
        AudioParameterFloatAttributes().withLabel("s")));

    params.push_back(std::make_unique<AudioParameterFloat>(
        ParameterID{ ParamID::predelay, 1 }, "Pre-Delay",
        NormalisableRange<float>(0.0f, 200.0f), 20.0f,
        AudioParameterFloatAttributes().withLabel("ms")));

    params.push_back(std::make_unique<AudioParameterFloat>(
        ParameterID{ ParamID::damping, 1 }, "Damping",
        highRange, 8000.0f,
        AudioParameterFloatAttributes().withLabel("Hz")));

    params.push_back(std::make_unique<AudioParameterFloat>(
        ParameterID{ ParamID::diffusion, 1 }, "Diffusion",
        NormalisableRange<float>(0.0f, 100.0f), 65.0f,
        AudioParameterFloatAttributes().withLabel("%")));

    params.push_back(std::make_unique<AudioParameterFloat>(
        ParameterID{ ParamID::modulation, 1 }, "Modulation",
        NormalisableRange<float>(0.0f, 100.0f), 35.0f,
        AudioParameterFloatAttributes().withLabel("%")));

    params.push_back(std::make_unique<AudioParameterFloat>(
        ParameterID{ ParamID::lowcut, 1 }, "Low Cut",
        lowCutRange, 30.0f,
        AudioParameterFloatAttributes().withLabel("Hz")));

    params.push_back(std::make_unique<AudioParameterFloat>(
        ParameterID{ ParamID::highcut, 1 }, "High Cut",
        highRange, 18000.0f,
        AudioParameterFloatAttributes().withLabel("Hz")));

    params.push_back(std::make_unique<AudioParameterBool>(
        ParameterID{ ParamID::bypass, 1 }, "Bypass", false));

    return { params.begin(), params.end() };
}

} // namespace dm
