#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "FactoryPresets.h"

namespace dm
{

enum class PresetKind { Factory, User };

/**
    Owns the factory preset list, the user preset list (persisted to disk),
    and "current preset" tracking (which one is loaded, whether the user has
    since edited it) — mirroring the UI prototype's preset browser logic.
*/
class PresetManager : public juce::ChangeBroadcaster
{
public:
    explicit PresetManager(juce::AudioProcessorValueTreeState& state);

    int getNumPresets(PresetKind kind) const;
    juce::String getName(PresetKind kind, int index) const;

    void loadPreset(PresetKind kind, int index);
    void step(int direction);

    void saveCurrentAsUserPreset(const juce::String& name);
    void updateCurrentUserPreset();

    PresetKind getCurrentKind() const { return currentKind; }
    int getCurrentIndex() const { return currentIndex; }
    juce::String getCurrentName() const;
    std::array<float, 9> getCurrentPresetValues() const;

    bool isDirty() const;
    bool canUpdateCurrent() const { return currentKind == PresetKind::User && ! userPresets.empty(); }

private:
    juce::AudioProcessorValueTreeState& apvts;
    PresetKind currentKind = PresetKind::Factory;
    int currentIndex = 0;

    // userPresets holds raw const char* names that point into userPresetNames'
    // owned juce::String storage — rebuildUserPresetViews() keeps them in sync.
    std::vector<juce::String> userPresetNames;
    std::vector<std::array<float, 9>> userPresetValues;
    std::vector<FactoryPreset> userPresets;
    juce::File userPresetsFile;

    void loadFromDisk();
    void saveToDisk();
    void rebuildUserPresetViews();
    std::array<float, 9> captureCurrentNormalized() const;
    void applyNormalized(const std::array<float, 9>& v);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetManager)
};

} // namespace dm
