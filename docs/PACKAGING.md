# Packaging DARK MATTER

How to build distributable macOS and Windows packages of DARK MATTER.
Published by **KDDN**.

This is separate from day-to-day development builds (`build/`, Debug,
whatever formats you're currently testing) - everything here is a clean
Release build, in its own build directory, that ends up in `dist/`.

```
dist/
├── macOS/
│   ├── Dark Matter.app
│   ├── Dark Matter.vst3
│   ├── Dark Matter.component
│   └── Dark Matter Installer.pkg
└── Windows/
    ├── Dark Matter.exe
    ├── Dark Matter.vst3/
    └── Dark Matter Installer.exe
```

`dist/` and the `build-release-*/` directories it's built from are
git-ignored - nothing here gets committed.

## macOS

Run from the repo root, on a Mac, with Xcode Command Line Tools and CMake
installed:

```sh
packaging/macos/build_and_package.sh
```

This configures a **Release, universal (Apple Silicon + Intel)** build from
scratch, builds Standalone/VST3/AU, ad-hoc code-signs each, and produces
`dist/macOS/Dark Matter Installer.pkg` (a standard macOS installer built
with `pkgbuild`/`productbuild` - no third-party tools).

**Code signing**: only ad-hoc signed (`codesign -s -`), which is enough to
run the app on your own Mac. It is **not notarized**. Other people
downloading it will see Gatekeeper's "cannot verify developer" warning,
which they can bypass with right-click → Open. Real notarization needs a
paid Apple Developer ID Program membership ($99/year) - once you have one,
see the `NOTARIZE` comment in `packaging/macos/build_and_package.sh` for
where the signing identity and `xcrun notarytool`/`stapler` calls slot in.

## Windows

**This can't be built from macOS.** You need either a Windows machine with
Visual Studio 2022 (or the free Build Tools) and CMake, or the GitHub
Actions workflow below.

Locally, from a PowerShell prompt on Windows:

```powershell
packaging\windows\build_and_package.ps1
```

This configures a Release x64 build, builds Standalone + VST3 (AU doesn't
exist on Windows, so it's skipped automatically), and produces
`dist\Windows\Dark Matter Installer.exe` via [Inno
Setup](https://jrsoftware.org/isdl.php) (`ISCC.exe` needs to be on `PATH` -
install it, or `choco install innosetup`, and re-run the script if it warns
that it's missing). Without Inno Setup installed you still get the raw
`Dark Matter.exe` and `.vst3` in `dist\Windows\`, just no installer.

**Code signing**: not signed (no Authenticode certificate configured).
Windows SmartScreen will warn on first run until this is signed with a
purchased code-signing certificate.

### Building Windows via GitHub Actions (no Windows machine needed)

`.github/workflows/build.yml` has a `build-windows` job that runs the exact
same script on a `windows-latest` GitHub-hosted runner. From the repo's
**Actions** tab, run the "Build" workflow manually (`Run workflow` button -
it's `workflow_dispatch` only, nothing runs automatically on push). It also
runs `build-macos` alongside it. When it finishes, both `DarkMatter-macOS`
and `DarkMatter-Windows` are downloadable as workflow artifacts from that
run's summary page - not a GitHub Release, just a zip you can grab, matching
"don't publish anything yet."

## Installing the results

- **macOS**: open `Dark Matter Installer.pkg` and follow the prompts (you
  can deselect Standalone/VST3/AU individually). Or copy `Dark Matter.app`
  to `/Applications`, `Dark Matter.vst3` to
  `/Library/Audio/Plug-Ins/VST3`, and `Dark Matter.component` to
  `/Library/Audio/Plug-Ins/Components` by hand.
- **Windows**: run `Dark Matter Installer.exe`. Or copy `Dark Matter.exe`
  anywhere you like, and the `Dark Matter.vst3` folder to
  `C:\Program Files\Common Files\VST3`.

After installing/copying a plug-in manually (not via the installers above),
rescan plug-ins in your DAW if it doesn't pick it up automatically.

## Plugin identity / metadata

Company name, bundle ID, manufacturer code and plugin code
(`source: CMakeLists.txt`) are unchanged from earlier versions on purpose -
changing any of them would make hosts treat DARK MATTER as a different
plugin, breaking existing saved presets/projects. "KDDN" is used as the
publisher/author name in the installers and this documentation only; it
does not appear in the plugin's own identifiers.

| | |
|---|---|
| Product name | Dark Matter |
| Company (internal ID) | Keiddn |
| Published by | KDDN |
| Bundle ID | com.Keiddn.DarkMatter |
| Manufacturer code | Keid |
| Plugin code | Dkm1 |
| Formats | Standalone, VST3, AU (macOS only) |
