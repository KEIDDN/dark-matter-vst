#include "PresetManager.h"
#include "../dsp/Parameters.h"

namespace dm
{

PresetManager::PresetManager(juce::AudioProcessorValueTreeState& state) : apvts(state)
{
    userPresetsFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                           .getChildFile("DarkMatter")
                           .getChildFile("UserPresets.xml");
    loadFromDisk();
    loadPreset(PresetKind::Factory, 0);
}

int PresetManager::getNumPresets(PresetKind kind) const
{
    return kind == PresetKind::Factory ? (int) getFactoryPresets().size() : (int) userPresets.size();
}

juce::String PresetManager::getName(PresetKind kind, int index) const
{
    if (kind == PresetKind::Factory)
    {
        const auto& f = getFactoryPresets();
        if (juce::isPositiveAndBelow(index, (int) f.size()))
            return f[(size_t) index].name;
    }
    else if (juce::isPositiveAndBelow(index, (int) userPresets.size()))
    {
        return userPresets[(size_t) index].name;
    }
    return {};
}

std::array<float, 9> PresetManager::captureCurrentNormalized() const
{
    std::array<float, 9> v {};
    for (size_t i = 0; i < ParamID::orderedIds.size(); ++i)
        v[i] = apvts.getParameter(ParamID::orderedIds[i])->getValue();
    return v;
}

void PresetManager::applyNormalized(const std::array<float, 9>& v)
{
    for (size_t i = 0; i < ParamID::orderedIds.size(); ++i)
        apvts.getParameter(ParamID::orderedIds[i])->setValueNotifyingHost(v[i]);
}

void PresetManager::loadPreset(PresetKind kind, int index)
{
    const int count = getNumPresets(kind);
    if (count <= 0)
        return;
    index = ((index % count) + count) % count;

    const auto v = kind == PresetKind::Factory ? getFactoryPresets()[(size_t) index].v
                                                : userPresets[(size_t) index].v;
    applyNormalized(v);
    currentKind = kind;
    currentIndex = index;
    sendChangeMessage();
}

void PresetManager::step(int direction)
{
    auto kind = currentKind;
    int index = currentIndex;
    if (kind == PresetKind::User && userPresets.empty())
    {
        kind = PresetKind::Factory;
        index = -1;
    }
    loadPreset(kind, index + direction);
}

juce::String PresetManager::getCurrentName() const
{
    return getName(currentKind, currentIndex);
}

std::array<float, 9> PresetManager::getCurrentPresetValues() const
{
    if (currentKind == PresetKind::Factory)
    {
        const auto& f = getFactoryPresets();
        if (juce::isPositiveAndBelow(currentIndex, (int) f.size()))
            return f[(size_t) currentIndex].v;
    }
    else if (juce::isPositiveAndBelow(currentIndex, (int) userPresets.size()))
    {
        return userPresets[(size_t) currentIndex].v;
    }
    return {};
}

bool PresetManager::isDirty() const
{
    const auto current = captureCurrentNormalized();
    const auto stored = getCurrentPresetValues();
    for (size_t i = 0; i < current.size(); ++i)
        if (std::abs(current[i] - stored[i]) > 0.004f)
            return true;
    return false;
}

void PresetManager::saveCurrentAsUserPreset(const juce::String& name)
{
    if (name.trim().isEmpty())
        return;

    userPresetNames.push_back(name.trim());
    userPresetValues.push_back(captureCurrentNormalized());
    rebuildUserPresetViews();

    currentKind = PresetKind::User;
    currentIndex = (int) userPresets.size() - 1;
    saveToDisk();
    sendChangeMessage();
}

void PresetManager::updateCurrentUserPreset()
{
    if (! canUpdateCurrent())
        return;
    userPresetValues[(size_t) currentIndex] = captureCurrentNormalized();
    rebuildUserPresetViews();
    saveToDisk();
    sendChangeMessage();
}

void PresetManager::rebuildUserPresetViews()
{
    userPresets.clear();
    for (size_t i = 0; i < userPresetNames.size(); ++i)
        userPresets.push_back({ userPresetNames[i].toRawUTF8(), userPresetValues[i] });
}

void PresetManager::loadFromDisk()
{
    userPresetNames.clear();
    userPresetValues.clear();

    if (auto xml = juce::XmlDocument::parse(userPresetsFile))
    {
        for (auto* preset : xml->getChildWithTagNameIterator("Preset"))
        {
            std::array<float, 9> v {};
            for (size_t i = 0; i < ParamID::orderedIds.size(); ++i)
                v[i] = (float) preset->getDoubleAttribute(ParamID::orderedIds[i], 0.0);
            userPresetNames.push_back(preset->getStringAttribute("name"));
            userPresetValues.push_back(v);
        }
    }
    rebuildUserPresetViews();
}

void PresetManager::saveToDisk()
{
    juce::XmlElement root("UserPresets");
    for (size_t i = 0; i < userPresetNames.size(); ++i)
    {
        auto* preset = root.createNewChildElement("Preset");
        preset->setAttribute("name", userPresetNames[i]);
        for (size_t k = 0; k < ParamID::orderedIds.size(); ++k)
            preset->setAttribute(ParamID::orderedIds[k], (double) userPresetValues[i][k]);
    }
    userPresetsFile.getParentDirectory().createDirectory();
    root.writeTo(userPresetsFile);
}

} // namespace dm
