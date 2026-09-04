#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../presets/PresetManager.h"
#include "DarkMatterLookAndFeel.h"

namespace dm::ui
{

class PresetBar : public juce::Component,
                   private juce::ChangeListener,
                   private juce::ListBoxModel
{
public:
    PresetBar(PresetManager& presetManager, DarkMatterLookAndFeel& laf, std::function<void()> onPresetLoaded);
    ~PresetBar() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseUp(const juce::MouseEvent&) override;
    bool hitTest(int x, int y) override;

private:
    PresetManager& presets;
    DarkMatterLookAndFeel& lookAndFeel;
    std::function<void()> onPresetLoaded;

    bool menuOpen = false;
    bool saving = false;
    // PresetBar is both a normal Component (mouseUp fires via the click it
    // received directly) and a Desktop global mouse listener (mouseUp fires
    // again for that same click so outside-clicks can be detected) — this
    // guards against handling the same physical click twice, which used to
    // toggle the menu open then immediately shut on every click on the pill.
    juce::int64 lastHandledMouseUpMs = -1;
    PresetKind activeTab = PresetKind::Factory;

    juce::Label nameLabel;
    juce::TextButton prevButton { "<" }, nextButton { ">" };

    juce::TextButton factoryTabButton { "FACTORY" }, userTabButton { "USER" };
    juce::Label countLabel;
    juce::ListBox listBox { "presets", this };
    juce::TextButton savePresetButton { "SAVE PRESET" }, updateButton { "UPDATE" };
    juce::TextEditor nameEditor;
    juce::TextButton confirmSaveButton { "SAVE" }, cancelSaveButton { juce::String::fromUTF8("\xC3\x97") };

    juce::Rectangle<int> pillBounds, panelBounds;

    // juce::ListBoxModel
    int getNumRows() override;
    void paintListBoxItem(int row, juce::Graphics&, int width, int height, bool selected) override;
    void listBoxItemClicked(int row, const juce::MouseEvent&) override;

    void changeListenerCallback(juce::ChangeBroadcaster*) override;
    void refreshFromState();
    void setMenuOpen(bool open);
    void beginSave();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetBar)
};

} // namespace dm::ui
