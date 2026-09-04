#include "PresetBar.h"
#include "Colors.h"

namespace dm::ui
{

namespace
{
    void styleFlatButton(juce::TextButton& b, DarkMatterLookAndFeel& laf, float fontSize = 9.5f)
    {
        b.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        b.setColour(juce::TextButton::buttonOnColourId, juce::Colours::white.withAlpha(0.07f));
        b.setColour(juce::TextButton::textColourOffId, textDim);
        b.setColour(juce::TextButton::textColourOnId, textBright);
        b.setLookAndFeel(&laf);
        juce::ignoreUnused(fontSize);
    }
}

PresetBar::PresetBar(PresetManager& presetManager, DarkMatterLookAndFeel& laf, std::function<void()> callback)
    : presets(presetManager), lookAndFeel(laf), onPresetLoaded(std::move(callback))
{
    nameLabel.setJustificationType(juce::Justification::centredLeft);
    nameLabel.setFont(laf.getGeist(14.0f, 300));
    nameLabel.setColour(juce::Label::textColourId, juce::Colour(0xffe6e3ee));
    nameLabel.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(nameLabel);

    for (auto* b : { &prevButton, &nextButton })
    {
        styleFlatButton(*b, laf, 11.0f);
        addAndMakeVisible(*b);
    }
    prevButton.onClick = [this] { presets.step(-1); };
    nextButton.onClick = [this] { presets.step(1); };

    for (auto* b : { &factoryTabButton, &userTabButton })
    {
        styleFlatButton(*b, laf);
        addChildComponent(*b);
    }
    factoryTabButton.onClick = [this] { activeTab = PresetKind::Factory; refreshFromState(); };
    userTabButton.onClick = [this] { activeTab = PresetKind::User; refreshFromState(); };

    countLabel.setJustificationType(juce::Justification::centredRight);
    countLabel.setFont(laf.getGeistMono(9.0f, 300));
    countLabel.setColour(juce::Label::textColourId, textFaint);
    addChildComponent(countLabel);

    listBox.setRowHeight(32);
    listBox.setColour(juce::ListBox::backgroundColourId, juce::Colours::transparentBlack);
    listBox.setColour(juce::ScrollBar::thumbColourId, juce::Colours::white.withAlpha(0.12f));
    addChildComponent(listBox);

    styleFlatButton(savePresetButton, laf);
    savePresetButton.onClick = [this] { beginSave(); };
    addChildComponent(savePresetButton);

    styleFlatButton(updateButton, laf);
    updateButton.onClick = [this] { presets.updateCurrentUserPreset(); refreshFromState(); };
    addChildComponent(updateButton);

    nameEditor.setFont(laf.getGeist(13.0f, 300));
    nameEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colours::black.withAlpha(0.35f));
    nameEditor.setColour(juce::TextEditor::textColourId, juce::Colour(0xfff0edf7));
    nameEditor.setColour(juce::TextEditor::outlineColourId, accentLight.withAlpha(0.35f));
    nameEditor.setColour(juce::TextEditor::focusedOutlineColourId, accentLight.withAlpha(0.6f));
    nameEditor.setTextToShowWhenEmpty("Preset name", textFaint);
    nameEditor.setInputRestrictions(28);
    nameEditor.onReturnKey = [this]
    {
        presets.saveCurrentAsUserPreset(nameEditor.getText());
        saving = false;
        refreshFromState();
    };
    nameEditor.onEscapeKey = [this] { saving = false; refreshFromState(); };
    addChildComponent(nameEditor);

    styleFlatButton(confirmSaveButton, laf);
    confirmSaveButton.onClick = [this]
    {
        presets.saveCurrentAsUserPreset(nameEditor.getText());
        saving = false;
        refreshFromState();
    };
    addChildComponent(confirmSaveButton);

    styleFlatButton(cancelSaveButton, laf);
    cancelSaveButton.onClick = [this] { saving = false; refreshFromState(); };
    addChildComponent(cancelSaveButton);

    presets.addChangeListener(this);
    juce::Desktop::getInstance().addGlobalMouseListener(this);

    activeTab = presets.getCurrentKind();
    refreshFromState();
}

PresetBar::~PresetBar()
{
    juce::Desktop::getInstance().removeGlobalMouseListener(this);
    presets.removeChangeListener(this);
}

void PresetBar::changeListenerCallback(juce::ChangeBroadcaster*)
{
    activeTab = presets.getCurrentKind();
    refreshFromState();
    if (onPresetLoaded)
        onPresetLoaded();
}

void PresetBar::beginSave()
{
    saving = true;
    const bool suggestIteration = ! presets.isDirty() && activeTab == PresetKind::Factory;
    nameEditor.setText(suggestIteration ? presets.getCurrentName() + " II" : juce::String(), juce::dontSendNotification);
    refreshFromState();
    nameEditor.grabKeyboardFocus();
}

void PresetBar::setMenuOpen(bool open)
{
    if (menuOpen == open)
        return;
    menuOpen = open;
    if (open)
        activeTab = presets.getCurrentKind();
    else
        saving = false;
    refreshFromState();
}

void PresetBar::mouseUp(const juce::MouseEvent& e)
{
    if (auto* comp = e.originalComponent)
    {
        if (comp == this || comp == &nameLabel)
        {
            if (pillBounds.contains(e.getEventRelativeTo(this).getPosition()))
            {
                setMenuOpen(! menuOpen);
                return;
            }
        }
    }

    if (menuOpen)
    {
        const auto posInThis = e.getEventRelativeTo(this).getPosition();
        const bool inside = getLocalBounds().contains(posInThis);
        if (! inside)
            setMenuOpen(false);
    }
}

void PresetBar::refreshFromState()
{
    const bool dirty = presets.isDirty();
    nameLabel.setText(presets.getCurrentName(), juce::dontSendNotification);

    factoryTabButton.setVisible(menuOpen);
    userTabButton.setVisible(menuOpen);
    countLabel.setVisible(menuOpen);
    listBox.setVisible(menuOpen);
    savePresetButton.setVisible(menuOpen && ! saving);
    updateButton.setVisible(menuOpen && ! saving && presets.canUpdateCurrent() && dirty);
    nameEditor.setVisible(menuOpen && saving);
    confirmSaveButton.setVisible(menuOpen && saving);
    cancelSaveButton.setVisible(menuOpen && saving);

    factoryTabButton.setToggleState(activeTab == PresetKind::Factory, juce::dontSendNotification);
    userTabButton.setToggleState(activeTab == PresetKind::User, juce::dontSendNotification);

    const int total = presets.getNumPresets(activeTab);
    const bool onActiveTab = presets.getCurrentKind() == activeTab;
    countLabel.setText((onActiveTab ? juce::String(presets.getCurrentIndex() + 1).paddedLeft('0', 2)
                                     : juce::String(total).paddedLeft('0', 2))
                            + " / " + juce::String(total).paddedLeft('0', 2),
                        juce::dontSendNotification);

    listBox.updateContent();
    listBox.repaint();
    repaint();
}

int PresetBar::getNumRows()
{
    return presets.getNumPresets(activeTab);
}

void PresetBar::paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool)
{
    const bool selected = presets.getCurrentKind() == activeTab && row == presets.getCurrentIndex();
    if (selected)
    {
        g.setColour(accent.withAlpha(0.09f));
        g.fillRoundedRectangle(1.0f, 1.0f, (float) width - 2.0f, (float) height - 2.0f, 8.0f);
    }

    g.setColour(textFaint);
    g.setFont(lookAndFeel.getGeistMono(9.5f, 300));
    g.drawText(juce::String(row + 1).paddedLeft('0', 2), 12, 0, 26, height, juce::Justification::centredLeft);

    g.setColour(selected ? juce::Colour(0xfff0edf7) : textDim);
    g.setFont(lookAndFeel.getGeist(13.0f, 300));
    g.drawText(presets.getName(activeTab, row), 40, 0, width - 60, height, juce::Justification::centredLeft);

    if (selected)
    {
        g.setColour(accentDot);
        g.fillEllipse((float) width - 20.0f, height / 2.0f - 2.5f, 5.0f, 5.0f);
    }
}

void PresetBar::listBoxItemClicked(int row, const juce::MouseEvent&)
{
    presets.loadPreset(activeTab, row);
}

void PresetBar::resized()
{
    pillBounds = { 0, 0, 270, 44 };
    panelBounds = { 0, 54, 316, juce::jmax(0, getHeight() - 54) };

    auto pill = pillBounds;
    auto arrows = pill.removeFromRight(64).reduced(2);
    prevButton.setBounds(arrows.removeFromLeft(30));
    nextButton.setBounds(arrows.removeFromLeft(30));
    nameLabel.setBounds(pill.reduced(16, 0));

    auto panel = panelBounds;
    auto tabsRow = panel.removeFromTop(40).reduced(12, 8);
    countLabel.setBounds(tabsRow.removeFromRight(70));
    factoryTabButton.setBounds(tabsRow.removeFromLeft(70));
    userTabButton.setBounds(tabsRow.removeFromLeft(70));

    auto footer = panel.removeFromBottom(48).reduced(8);
    listBox.setBounds(panel.reduced(6));

    if (saving)
    {
        auto row = footer;
        cancelSaveButton.setBounds(row.removeFromRight(32));
        row.removeFromRight(6);
        confirmSaveButton.setBounds(row.removeFromRight(56));
        row.removeFromRight(6);
        nameEditor.setBounds(row);
    }
    else
    {
        auto row = footer;
        if (updateButton.isVisible())
        {
            updateButton.setBounds(row.removeFromRight(70));
            row.removeFromRight(6);
        }
        savePresetButton.setBounds(row);
    }
}

void PresetBar::paint(juce::Graphics& g)
{
    // Pill chrome.
    {
        auto r = pillBounds.toFloat();
        g.setGradientFill(juce::ColourGradient(juce::Colours::white.withAlpha(0.035f), r.getX(), r.getY(),
                                                juce::Colours::white.withAlpha(0.01f), r.getX(), r.getBottom(), false));
        g.fillRoundedRectangle(r, 22.0f);
        g.setColour(panelBorder);
        g.drawRoundedRectangle(r.reduced(0.5f), 22.0f, 1.0f);

        if (presets.isDirty())
        {
            g.setColour(accentDot);
            const float dotX = nameLabel.getRight() + 8.0f;
            g.fillEllipse(dotX, r.getCentreY() - 2.0f, 4.0f, 4.0f);
        }
    }

    if (! menuOpen)
        return;

    // Browser panel chrome.
    auto r = panelBounds.toFloat();
    g.setGradientFill(juce::ColourGradient(browserBgHi, r.getX(), r.getY(), browserBgLo, r.getX(), r.getBottom(), false));
    g.fillRoundedRectangle(r, 20.0f);
    g.setColour(panelBorder);
    g.drawRoundedRectangle(r.reduced(0.5f), 20.0f, 1.0f);

    if (activeTab == PresetKind::User && getNumRows() == 0 && ! saving)
    {
        g.setColour(textDim);
        g.setFont(lookAndFeel.getGeist(12.0f, 300));
        g.drawText("No user presets yet", panelBounds.withHeight(220).withY(80), juce::Justification::centred);
        g.setColour(textFaint);
        g.setFont(lookAndFeel.getGeist(10.0f, 300));
        g.drawText("Shape a sound, then save it below.", panelBounds.withHeight(240).withY(100), juce::Justification::centred);
    }
}

} // namespace dm::ui
