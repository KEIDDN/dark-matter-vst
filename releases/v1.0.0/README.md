# DARK MATTER v1.0.0

Prebuilt binaries for this version, committed directly to the repo so
they're available without running the build scripts. Published by KDDN.

- `macOS/Dark Matter Installer.pkg` — recommended: installs Standalone,
  VST3 and AU in one go. See [docs/PACKAGING.md](../../docs/PACKAGING.md)
  for what it does and how to install manually instead.
- `macOS/Dark Matter.app`, `Dark Matter.vst3`, `Dark Matter.component` —
  the same builds, unpacked, if you'd rather copy them into place yourself.
- **Windows**: not built yet — this repo has no Windows machine to build
  on. Building it needs either a Windows machine or the `build-windows`
  GitHub Actions job (`.github/workflows/build.yml`); once that's run,
  `Windows/Dark Matter Installer.exe` will be added here too.

Unsigned/ad-hoc only (see docs/PACKAGING.md) - macOS Gatekeeper will warn
on first open; right-click → Open to bypass.
