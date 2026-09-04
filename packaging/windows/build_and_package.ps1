#Requires -Version 5.1
<#
.SYNOPSIS
    Builds DARK MATTER (Standalone .exe + VST3) in Release for Windows and
    packages an Inno Setup installer.

.DESCRIPTION
    Must run on Windows, with CMake and a Visual Studio 2022 (or compatible)
    C++ toolchain available - either locally, or via the windows-latest
    GitHub Actions runner (see .github/workflows/build.yml).

    This is a clean, from-scratch configure+build in its own build
    directory (build-release-windows\), so it never touches an existing
    dev build directory.

.EXAMPLE
    From a PowerShell prompt with cmake and MSVC on PATH:
        packaging\windows\build_and_package.ps1
#>

$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = Resolve-Path (Join-Path $scriptDir "..\..")
$buildDir = Join-Path $repoRoot "build-release-windows"
$distDir = Join-Path $repoRoot "dist\Windows"

$versionMatch = Select-String -Path (Join-Path $repoRoot "CMakeLists.txt") -Pattern "project\(DarkMatter VERSION ([0-9.]+)\)"
$version = $versionMatch.Matches[0].Groups[1].Value
Write-Host "== DARK MATTER Windows packaging - version $version =="

Write-Host "-- Configuring (Release, x64)"
cmake -S "$repoRoot" -B "$buildDir" -G "Visual Studio 17 2022" -A x64
if ($LASTEXITCODE -ne 0) { throw "CMake configure failed" }

Write-Host "-- Building Standalone and VST3"
cmake --build "$buildDir" --config Release --target DarkMatter_Standalone --target DarkMatter_VST3 --parallel
if ($LASTEXITCODE -ne 0) { throw "Build failed" }

$artefacts = Join-Path $buildDir "DarkMatter_artefacts\Release"
$exe = Join-Path $artefacts "Standalone\Dark Matter.exe"
$vst3 = Join-Path $artefacts "VST3\Dark Matter.vst3"

foreach ($f in @($exe, $vst3)) {
    if (-not (Test-Path $f)) { throw "Expected build output missing: $f" }
}

Write-Host "-- Assembling dist\Windows\"
if (Test-Path $distDir) { Remove-Item $distDir -Recurse -Force }
New-Item -ItemType Directory -Path $distDir | Out-Null
Copy-Item $exe $distDir
Copy-Item $vst3 $distDir -Recurse

Write-Host "-- Code signing: skipped (no Authenticode certificate configured)."
Write-Host "   Windows SmartScreen will warn users until this is signed with a"
Write-Host "   purchased code-signing certificate - see docs/PACKAGING.md."

$iscc = Get-Command "ISCC.exe" -ErrorAction SilentlyContinue
if ($null -eq $iscc) {
    Write-Warning "Inno Setup (ISCC.exe) not found on PATH - skipping installer build."
    Write-Warning "Install it from https://jrsoftware.org/isdl.php (or 'choco install innosetup') and re-run."
} else {
    Write-Host "-- Building installer"
    & $iscc.Path "/DMyAppVersion=$version" (Join-Path $scriptDir "installer.iss")
    if ($LASTEXITCODE -ne 0) { throw "Inno Setup compile failed" }
}

Write-Host ""
Write-Host "== Done =="
Write-Host "dist\Windows\ contains:"
Get-ChildItem $distDir | ForEach-Object { Write-Host " - $($_.Name)" }
