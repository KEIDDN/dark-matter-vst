#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <atomic>

namespace dm::ui
{

/** A thin horizontal level bar, filling from the given edge, reading a
    shared atomic level value updated by the audio thread. */
class LevelMeter : public juce::Component, private juce::Timer
{
public:
    enum class FillFrom { Left, Right };

    LevelMeter(const std::atomic<float>& levelSource, FillFrom direction);
    ~LevelMeter() override;

    void paint(juce::Graphics&) override;

private:
    const std::atomic<float>& level;
    FillFrom fillFrom;
    float displayed = 0.0f;

    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeter)
};

} // namespace dm::ui
