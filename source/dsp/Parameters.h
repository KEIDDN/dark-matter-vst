#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace dm
{
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
}

inline juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> params;

    auto logRange = [](float lo, float hi)
    {
        NormalisableRange<float> range(lo, hi);
        range.setSkewForCentre(std::sqrt(lo * hi));
        return range;
    };

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
        logRange(0.2f, 20.0f), 2.5f,
        AudioParameterFloatAttributes().withLabel("s")));

    params.push_back(std::make_unique<AudioParameterFloat>(
        ParameterID{ ParamID::predelay, 1 }, "Pre-Delay",
        NormalisableRange<float>(0.0f, 200.0f), 20.0f,
        AudioParameterFloatAttributes().withLabel("ms")));

    params.push_back(std::make_unique<AudioParameterFloat>(
        ParameterID{ ParamID::damping, 1 }, "Damping",
        logRange(1000.0f, 20000.0f), 8000.0f,
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
        logRange(20.0f, 1000.0f), 30.0f,
        AudioParameterFloatAttributes().withLabel("Hz")));

    params.push_back(std::make_unique<AudioParameterFloat>(
        ParameterID{ ParamID::highcut, 1 }, "High Cut",
        logRange(1000.0f, 20000.0f), 18000.0f,
        AudioParameterFloatAttributes().withLabel("Hz")));

    return { params.begin(), params.end() };
}

} // namespace dm
