#!/usr/bin/env bash
# Builds DARK MATTER (Standalone + VST3 + AU) in Release, universal
# (Apple Silicon + Intel), and assembles a distributable .pkg installer.
#
# Usage:
#   packaging/macos/build_and_package.sh
#
# Output:
#   dist/macOS/Dark Matter.app
#   dist/macOS/Dark Matter.vst3
#   dist/macOS/Dark Matter.component
#   dist/macOS/Dark Matter Installer.pkg
#
# This is a clean, from-scratch configure+build in its own build directory
# (build-release-macos/) so it never touches your everyday Debug dev build/.
#
# Code signing: everything here is ad-hoc signed (codesign -s -), which is
# enough to run on your own Mac. It is NOT notarized - macOS Gatekeeper will
# still warn other people who download it ("cannot verify developer"), which
# they can bypass with right-click > Open. Real notarization requires a paid
# Apple Developer ID and is intentionally left out of this script; search
# for "NOTARIZE" below for where it would slot in once you have one.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
BUILD_DIR="$REPO_ROOT/build-release-macos"
DIST_DIR="$REPO_ROOT/dist/macOS"
STAGING_DIR="$BUILD_DIR/pkg-staging"
PKG_OUT_DIR="$BUILD_DIR/pkg-components"

VERSION="$(grep -m1 'project(DarkMatter VERSION' "$REPO_ROOT/CMakeLists.txt" | sed -E 's/.*VERSION ([0-9.]+).*/\1/')"
echo "== DARK MATTER macOS packaging — version $VERSION =="

echo "-- Configuring (Release, universal binary)"
cmake -S "$REPO_ROOT" -B "$BUILD_DIR" \
    -G "Ninja" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"

echo "-- Building Standalone, VST3, AU"
cmake --build "$BUILD_DIR" --target DarkMatter_Standalone --target DarkMatter_VST3 --target DarkMatter_AU -j "$(sysctl -n hw.ncpu)"

ARTEFACTS="$BUILD_DIR/DarkMatter_artefacts/Release"
APP="$ARTEFACTS/Standalone/Dark Matter.app"
VST3="$ARTEFACTS/VST3/Dark Matter.vst3"
AU="$ARTEFACTS/AU/Dark Matter.component"

for f in "$APP" "$VST3" "$AU"; do
    if [ ! -e "$f" ]; then
        echo "ERROR: expected build output missing: $f" >&2
        exit 1
    fi
done

echo "-- Ad-hoc signing"
codesign --force --deep --sign - "$APP"
codesign --force --deep --sign - "$VST3"
codesign --force --deep --sign - "$AU"
# NOTARIZE: with a Developer ID, this is where you'd instead run
#   codesign --force --deep --options runtime --timestamp --sign "Developer ID Application: ..." ...
# followed by `xcrun notarytool submit` + `xcrun stapler staple` on the .pkg below.

echo "-- Assembling dist/macOS/"
rm -rf "$DIST_DIR"
mkdir -p "$DIST_DIR"
cp -R "$APP" "$DIST_DIR/"
cp -R "$VST3" "$DIST_DIR/"
cp -R "$AU" "$DIST_DIR/"

echo "-- Staging installer payload"
rm -rf "$STAGING_DIR" "$PKG_OUT_DIR"
mkdir -p "$STAGING_DIR/Applications" \
         "$STAGING_DIR/Library/Audio/Plug-Ins/VST3" \
         "$STAGING_DIR/Library/Audio/Plug-Ins/Components" \
         "$PKG_OUT_DIR"
cp -R "$APP" "$STAGING_DIR/Applications/"
cp -R "$VST3" "$STAGING_DIR/Library/Audio/Plug-Ins/VST3/"
cp -R "$AU" "$STAGING_DIR/Library/Audio/Plug-Ins/Components/"

echo "-- Building component packages"
pkgbuild --root "$STAGING_DIR/Applications" \
         --install-location "/Applications" \
         --identifier "com.kddn.darkmatter.standalone" \
         --version "$VERSION" \
         "$PKG_OUT_DIR/standalone.pkg"

pkgbuild --root "$STAGING_DIR/Library/Audio/Plug-Ins/VST3" \
         --install-location "/Library/Audio/Plug-Ins/VST3" \
         --identifier "com.kddn.darkmatter.vst3" \
         --version "$VERSION" \
         "$PKG_OUT_DIR/vst3.pkg"

pkgbuild --root "$STAGING_DIR/Library/Audio/Plug-Ins/Components" \
         --install-location "/Library/Audio/Plug-Ins/Components" \
         --identifier "com.kddn.darkmatter.au" \
         --version "$VERSION" \
         "$PKG_OUT_DIR/au.pkg"

echo "-- Building final installer"
productbuild --distribution "$SCRIPT_DIR/Distribution.xml" \
             --package-path "$PKG_OUT_DIR" \
             --resources "$SCRIPT_DIR/resources" \
             "$DIST_DIR/Dark Matter Installer.pkg"

echo
echo "== Done =="
echo "dist/macOS/ contains:"
ls -1 "$DIST_DIR"
