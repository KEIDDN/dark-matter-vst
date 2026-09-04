#pragma once

#include <array>
#include <juce_core/juce_core.h>

namespace dm
{

/**
    The 30 factory presets, transcribed verbatim from the Dark Matter UI
    prototype (Dark Matter v4.dc.html, FACTORY array).

    Values are normalised [0,1] knob positions in the order:
    predelay, decay, size, damping, diffusion, mix, lowcut, highcut, modulation
    — the same order and the same normalised values the prototype uses, so
    they can be applied directly via AudioProcessorParameter::setValueNotifyingHost.
*/
struct FactoryPreset
{
    const char* name;
    std::array<float, 9> v;
};

inline const std::array<FactoryPreset, 30>& getFactoryPresets()
{
    static const std::array<FactoryPreset, 30> presets { {
        { "Dark Nebula",     { .125f, .45f, .68f, .55f, .72f, .35f, .46f, .75f, .42f } },
        { "Event Horizon",   { .2f,   .62f, .85f, .4f,  .8f,  .5f,  .3f,  .8f,  .25f } },
        { "Black Void",      { .05f,  .9f,  1.f,  .7f,  .95f, .65f, .4f,  .55f, .15f } },
        { "Deep Space",      { .3f,   .7f,  .8f,  .35f, .6f,  .45f, .35f, .9f,  .3f  } },
        { "Gravitational",   { .15f,  .55f, .6f,  .5f,  .5f,  .4f,  .55f, .7f,  .85f } },
        { "Singularity",     { .02f,  .95f, .95f, .6f,  1.f,  .7f,  .45f, .5f,  .1f  } },
        { "Solar Drift",     { .25f,  .5f,  .55f, .3f,  .55f, .38f, .3f,  .95f, .6f  } },
        { "Dark Orbit",      { .1f,   .6f,  .7f,  .5f,  .7f,  .42f, .5f,  .65f, .5f  } },
        { "Red Giant",       { .35f,  .75f, .9f,  .8f,  .65f, .5f,  .2f,  .4f,  .2f  } },
        { "Blue Shift",      { .1f,   .4f,  .5f,  .15f, .6f,  .35f, .5f,  1.f,  .45f } },
        { "Cosmic Dust",     { .2f,   .35f, .45f, .45f, .9f,  .3f,  .4f,  .8f,  .35f } },
        { "Supernova",       { 0.f,   .85f, 1.f,  .3f,  .4f,  .6f,  .25f, .95f, .7f  } },
        { "Eclipse",         { .4f,   .55f, .65f, .75f, .8f,  .4f,  .35f, .5f,  .2f  } },
        { "Zero Gravity",    { .3f,   .65f, .75f, .35f, .5f,  .45f, .45f, .85f, .95f } },
        { "Cold Vacuum",     { .15f,  .3f,  .4f,  .2f,  .85f, .28f, .6f,  .9f,  .1f  } },
        { "Deep Field",      { .5f,   .8f,  .9f,  .5f,  .7f,  .55f, .3f,  .7f,  .3f  } },
        { "Lost Signal",     { .6f,   .5f,  .6f,  .6f,  .3f,  .4f,  .55f, .6f,  .65f } },
        { "Outer Rim",       { .2f,   .7f,  .85f, .45f, .55f, .45f, .35f, .75f, .4f  } },
        { "Starfall",        { .35f,  .4f,  .5f,  .25f, .75f, .35f, .4f,  .95f, .55f } },
        { "Photon Trail",    { .1f,   .45f, .55f, .1f,  .45f, .4f,  .5f,  1.f,  .5f  } },
        { "Quantum Drift",   { .25f,  .6f,  .65f, .4f,  .6f,  .42f, .45f, .8f,  1.f  } },
        { "Infinite Depth",  { .3f,   1.f,  1.f,  .55f, .9f,  .6f,  .35f, .6f,  .2f  } },
        { "Void Chamber",    { .5f,   .22f, .3f,  .85f, .9f,  .22f, .55f, .45f, .1f  } },
        { "Gravity Well",    { .05f,  .7f,  .8f,  .65f, .85f, .5f,  .6f,  .55f, .3f  } },
        { "Dark Energy",     { .15f,  .8f,  .75f, .5f,  .4f,  .55f, .4f,  .7f,  .75f } },
        { "Celestial",       { .3f,   .55f, .7f,  .2f,  .7f,  .4f,  .3f,  .95f, .35f } },
        { "Time Dilation",   { .7f,   .75f, .8f,  .45f, .6f,  .5f,  .4f,  .7f,  .6f  } },
        { "Beyond Light",    { .2f,   .65f, .9f,  .15f, .8f,  .55f, .25f, 1.f,  .3f  } },
        { "Collapse",        { 0.f,   .3f,  .35f, .7f,  .95f, .45f, .65f, .5f,  .15f } },
        { "Afterglow",       { .5f,   .3f,  .35f, .8f,  .9f,  .25f, .25f, .5f,  .12f } },
    } };
    return presets;
}

} // namespace dm
