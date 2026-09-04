# DARK MATTER v1.0.0

Prebuilt binaries for this version, committed directly to the repo so
they're available without running the build scripts. Published by KDDN.

## ⚠️ Before you install (macOS)

This build is **not notarized by Apple** (that requires a paid Apple
Developer ID account, which this project doesn't have yet — it's an MVP).
macOS Gatekeeper will block it on first launch with a message like *"Dark
Matter" cannot be opened because it is from an unidentified developer* (or
similar). This is expected, not a broken download. To open it anyway:

1. **Don't double-click it.** Instead, right-click (or Control-click) the
   `.pkg` (or, after installing, the app) and choose **Open**.
2. Click **Open** again in the warning dialog that appears.
3. If it's still refused: open **System Settings → Privacy & Security**,
   scroll down, and click **Open Anyway** next to the message about Dark
   Matter, then try opening it again.

You only need to do this once per component (the installer, the app, the
plug-in) — after the first approval, it opens normally.

## What's in here

- `macOS/Dark Matter Installer.pkg` — recommended: installs Standalone,
  VST3 and AU in one go (lets you deselect any of the three). See
  [docs/PACKAGING.md](../../docs/PACKAGING.md) for what it does.
- `macOS/Dark Matter.app`, `Dark Matter.vst3`, `Dark Matter.component` —
  the same builds, unpacked, if you'd rather copy them into place
  yourself: `Dark Matter.app` → `/Applications`, `Dark Matter.vst3` →
  `/Library/Audio/Plug-Ins/VST3`, `Dark Matter.component` →
  `/Library/Audio/Plug-Ins/Components`.
- **Windows**: not built yet — this repo has no Windows machine to build
  on. Building it needs either a Windows machine or the `build-windows`
  GitHub Actions job (`.github/workflows/build.yml`); once that's run,
  `Windows/Dark Matter Installer.exe` will be added here too. (Windows
  builds will also be unsigned for the same reason as above — expect a
  SmartScreen warning there, with an "More info → Run anyway" bypass.)

After installing the plug-in, rescan plug-ins in your DAW if it doesn't
show up automatically.
