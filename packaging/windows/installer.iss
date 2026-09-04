; Inno Setup script for DARK MATTER (Standalone .exe + VST3).
; Compile with ISCC.exe (Inno Setup 6+), from this directory:
;   ISCC.exe /DMyAppVersion=1.0.0 installer.iss
; (packaging\windows\build_and_package.ps1 does this for you, version and all.)
;
; Expects the Release build to already exist at ..\..\build-release-windows
; (build_and_package.ps1 builds it first) - this script only packages.

#ifndef MyAppVersion
  #define MyAppVersion "0.0.0"
#endif

[Setup]
; Fixed GUID so Windows/Inno Setup recognise future versions as upgrades of
; the same app rather than a separate install - do not change this.
AppId={{C60AEB8A-2DC6-4FBF-9E96-FE8283A059D6}
AppName=Dark Matter
AppVersion={#MyAppVersion}
AppPublisher=KDDN
DefaultDirName={autopf}\Dark Matter
DefaultGroupName=Dark Matter
ArchitecturesInstallIn64BitMode=x64
DisableProgramGroupPage=yes
OutputDir=..\..\dist\Windows
OutputBaseFilename=Dark Matter Installer
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
; No Authenticode certificate configured - see docs/PACKAGING.md. Windows
; SmartScreen will warn on first run of the installer/app until this is
; signed with a purchased code-signing certificate.

[Types]
Name: "full"; Description: "Full installation"
Name: "custom"; Description: "Custom installation"; Flags: iscustom

[Components]
Name: "standalone"; Description: "Standalone application"; Types: full custom
Name: "vst3"; Description: "VST3 plug-in"; Types: full custom

[Files]
Source: "..\..\build-release-windows\DarkMatter_artefacts\Release\Standalone\Dark Matter.exe"; DestDir: "{app}"; Components: standalone; Flags: ignoreversion
Source: "..\..\build-release-windows\DarkMatter_artefacts\Release\VST3\Dark Matter.vst3\*"; DestDir: "{commoncf64}\VST3\Dark Matter.vst3"; Components: vst3; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\Dark Matter"; Filename: "{app}\Dark Matter.exe"; Components: standalone
Name: "{autodesktop}\Dark Matter"; Filename: "{app}\Dark Matter.exe"; Components: standalone; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "Create a &desktop shortcut"; GroupDescription: "Additional shortcuts:"; Components: standalone
