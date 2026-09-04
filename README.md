# DARK MATTER

### Experimental Reverb Audio Plugin

> **v1.0.0 is ready.** macOS builds (Standalone, VST3, AU) are packaged and
> available under [`releases/v1.0.0/`](releases/v1.0.0/), including a
> one-click `.pkg` installer. Windows builds are prepared but not yet
> produced — see [docs/PACKAGING.md](docs/PACKAGING.md) for how to build
> both platforms and where installables end up. Published by **KDDN**.
>
> ⚠️ This is an unsigned/unnotarized MVP build — macOS will show an
> "unidentified developer" warning on first open. See
> [`releases/v1.0.0/README.md`](releases/v1.0.0/README.md) for the
> one-time bypass steps.

DARK MATTER is an experimental spatial reverb plugin built from scratch, combining real-time audio processing with a highly visual, interaction-driven interface.

Inspired by the behaviour of gravitational fields and black holes, DARK MATTER explores the relationship between **sound, space and visual feedback** through a minimal audio interface centred around a reactive black-hole visualisation.

The project is both a functional audio plugin and an exploration of how audio software can communicate complex processing through visual interaction.

---

## Overview

DARK MATTER is designed around a simple idea:

> **What if reverb could feel like falling into space?**

Instead of presenting reverb as a traditional collection of technical controls, DARK MATTER uses a visual metaphor to represent depth, decay and spatial behaviour.

The interface is deliberately minimal, giving the central visualisation a prominent role while keeping the underlying audio controls accessible and precise.

The current version focuses on building a solid functional foundation rather than competing with commercial reverberation processors.

---

## Features

- Real-time stereo reverb processing
- Adjustable mix and spatial size
- Decay control
- Pre-delay
- Damping
- Diffusion
- Modulation
- Low-cut and high-cut filtering
- Bypass control
- Factory preset system
- User preset support
- Interactive preset browser
- Audio-reactive visualisation
- Responsive native plugin interface

---

## Controls

| Parameter | Description |
|---|---|
| **MIX** | Controls the balance between dry and processed signal |
| **SIZE** | Controls the perceived spatial size of the reverb |
| **DECAY** | Controls the length of the reverb tail |
| **PRE-DELAY** | Controls the delay before the reverb response begins |
| **DAMPING** | Controls the high-frequency absorption of the reverb |
| **DIFFUSION** | Controls the density of the reverb field |
| **MODULATION** | Adds subtle modulation to the reverb network |
| **LOW CUT** | Removes low frequencies from the processed signal |
| **HIGH CUT** | Removes high frequencies from the processed signal |
| **LURKING** | Visual representation of the current filtering behaviour |

---

## Audio Engine

The current reverb engine is based on a lightweight feedback delay network designed to create a dense, evolving reverb response while remaining computationally manageable.

The processing chain combines:

- Multiple parallel delay lines
- Feedback-based reverberation
- Frequency-dependent damping
- Diffusion stages
- Modulation
- Low/high frequency filtering
- Dry/wet signal mixing

The goal is not to reproduce a specific physical acoustic space, but to create a controllable and musical sense of depth.

The DSP architecture is intentionally modular so that individual components can be refined or replaced as the project evolves.

```text
Audio Input
     │
     ▼
Input Filtering
     │
     ▼
Pre-Delay
     │
     ▼
Diffusion / Delay Network
     │
     ├── Damping
     ├── Modulation
     └── Feedback
     │
     ▼
Output Filtering
     │
     ▼
Dry / Wet Mix
     │
     ▼
Audio Output
```

---

## Building & Packaging

For day-to-day development, configure and build the CMake project in
`build/` as usual (see `CMakeLists.txt` - targets `DarkMatter_Standalone`,
`DarkMatter_VST3`, `DarkMatter_AU`).

For distributable macOS/Windows installers, see [docs/PACKAGING.md](docs/PACKAGING.md).
