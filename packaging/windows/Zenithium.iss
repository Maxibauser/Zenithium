; Zenithium — Inno Setup script.
;
; Build with:
;     iscc /DAppVersion=0.1.0 /DStageDir=C:\path\to\dist packaging\windows\Zenithium.iss
;
; Or use packaging/windows/build-installer.ps1 which handles staging + iscc
; in one step.

#ifndef AppVersion
  #define AppVersion "0.1.0"
#endif

#ifndef StageDir
  #define StageDir "..\..\dist"
#endif

#define AppName        "Zenithium"
#define AppPublisher   "Zenithium"
#define AppExeName     "Zenithium.exe"
#define AppId          "{{9D18E4E4-9B0D-4DF4-8F71-2E6BB2F4C9A3}"
#define AppUrl         "https://github.com/zenithium/zenithium"

[Setup]
AppId={#AppId}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppName} {#AppVersion}
AppPublisher={#AppPublisher}
AppPublisherURL={#AppUrl}
AppSupportURL={#AppUrl}
AppUpdatesURL={#AppUrl}
DefaultDirName={autopf}\{#AppName}
DefaultGroupName={#AppName}
DisableProgramGroupPage=yes
ArchitecturesInstallIn64BitMode=x64compatible
ArchitecturesAllowed=x64compatible
UninstallDisplayIcon={app}\{#AppExeName}
UninstallDisplayName={#AppName} {#AppVersion}
OutputDir=..\..\dist-installer
OutputBaseFilename=Zenithium-Setup-{#AppVersion}
Compression=lzma2/max
SolidCompression=yes
LicenseFile={#StageDir}\LICENSE
WizardStyle=modern
PrivilegesRequiredOverridesAllowed=dialog
SetupIconFile=
ChangesAssociations=yes
CloseApplications=yes
RestartApplications=no

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "Create a &Desktop shortcut"; \
    GroupDescription: "Additional shortcuts"; Flags: unchecked

Name: "quicklaunchicon"; Description: "Create a &Quick Launch shortcut"; \
    GroupDescription: "Additional shortcuts"; Flags: unchecked; \
    OnlyBelowVersion: 6.1

Name: "associatecpp"; Description: "Associate C / C++ files (.c, .cpp, .cc, .h, .hpp)"; \
    GroupDescription: "File associations"

Name: "associatepy"; Description: "Associate Python files (.py)"; \
    GroupDescription: "File associations"

Name: "associateweb"; Description: "Associate web files (.json, .js, .ts, .html, .css)"; \
    GroupDescription: "File associations"

Name: "associatetext"; Description: "Associate text files (.txt, .md)"; \
    GroupDescription: "File associations"

Name: "openwithfolder"; Description: "Add ""Open with Zenithium"" to folder right-click menu"; \
    GroupDescription: "Explorer integration"

Name: "openwithfile"; Description: "Add ""Open with Zenithium"" to file right-click menu"; \
    GroupDescription: "Explorer integration"

Name: "addtopath"; Description: "Add Zenithium to system PATH"; \
    GroupDescription: "Other"; Flags: unchecked

[Files]
; Everything the CMake install() step staged under {#StageDir}.
Source: "{#StageDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\{#AppName}"; Filename: "{app}\{#AppExeName}"
Name: "{group}\Uninstall {#AppName}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#AppName}"; Filename: "{app}\{#AppExeName}"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\{#AppName}"; \
    Filename: "{app}\{#AppExeName}"; Tasks: quicklaunchicon

[Run]
Filename: "{app}\{#AppExeName}"; Description: "Launch {#AppName}"; \
    Flags: nowait postinstall skipifsilent

[Registry]
; ---------- ProgId — the class Zenithium files are registered under ---------
Root: HKA; Subkey: "Software\Classes\Zenithium.SourceFile"; \
    ValueType: string; ValueName: ""; ValueData: "{#AppName} Source File"; \
    Flags: uninsdeletekey

Root: HKA; Subkey: "Software\Classes\Zenithium.SourceFile\DefaultIcon"; \
    ValueType: string; ValueName: ""; ValueData: "{app}\{#AppExeName},0"

Root: HKA; Subkey: "Software\Classes\Zenithium.SourceFile\shell\open\command"; \
    ValueType: string; ValueName: ""; \
    ValueData: """{app}\{#AppExeName}"" ""%1"""

; ---------- File-extension associations (per Tasks) -------------------------
; C / C++
Root: HKA; Subkey: "Software\Classes\.c";   ValueType: string; ValueName: ""; \
    ValueData: "Zenithium.SourceFile"; Tasks: associatecpp; Flags: uninsdeletevalue
Root: HKA; Subkey: "Software\Classes\.cpp"; ValueType: string; ValueName: ""; \
    ValueData: "Zenithium.SourceFile"; Tasks: associatecpp; Flags: uninsdeletevalue
Root: HKA; Subkey: "Software\Classes\.cc";  ValueType: string; ValueName: ""; \
    ValueData: "Zenithium.SourceFile"; Tasks: associatecpp; Flags: uninsdeletevalue
Root: HKA; Subkey: "Software\Classes\.h";   ValueType: string; ValueName: ""; \
    ValueData: "Zenithium.SourceFile"; Tasks: associatecpp; Flags: uninsdeletevalue
Root: HKA; Subkey: "Software\Classes\.hpp"; ValueType: string; ValueName: ""; \
    ValueData: "Zenithium.SourceFile"; Tasks: associatecpp; Flags: uninsdeletevalue

; Python
Root: HKA; Subkey: "Software\Classes\.py"; ValueType: string; ValueName: ""; \
    ValueData: "Zenithium.SourceFile"; Tasks: associatepy; Flags: uninsdeletevalue

; Web
Root: HKA; Subkey: "Software\Classes\.json"; ValueType: string; ValueName: ""; \
    ValueData: "Zenithium.SourceFile"; Tasks: associateweb; Flags: uninsdeletevalue
Root: HKA; Subkey: "Software\Classes\.js";   ValueType: string; ValueName: ""; \
    ValueData: "Zenithium.SourceFile"; Tasks: associateweb; Flags: uninsdeletevalue
Root: HKA; Subkey: "Software\Classes\.ts";   ValueType: string; ValueName: ""; \
    ValueData: "Zenithium.SourceFile"; Tasks: associateweb; Flags: uninsdeletevalue
Root: HKA; Subkey: "Software\Classes\.html"; ValueType: string; ValueName: ""; \
    ValueData: "Zenithium.SourceFile"; Tasks: associateweb; Flags: uninsdeletevalue
Root: HKA; Subkey: "Software\Classes\.css";  ValueType: string; ValueName: ""; \
    ValueData: "Zenithium.SourceFile"; Tasks: associateweb; Flags: uninsdeletevalue

; Text
Root: HKA; Subkey: "Software\Classes\.txt"; ValueType: string; ValueName: ""; \
    ValueData: "Zenithium.SourceFile"; Tasks: associatetext; Flags: uninsdeletevalue
Root: HKA; Subkey: "Software\Classes\.md";  ValueType: string; ValueName: ""; \
    ValueData: "Zenithium.SourceFile"; Tasks: associatetext; Flags: uninsdeletevalue

; ---------- Shell context menus --------------------------------------------
; "Open with Zenithium" on any file
Root: HKA; Subkey: "Software\Classes\*\shell\OpenWithZenithium"; \
    ValueType: string; ValueName: ""; ValueData: "Open with {#AppName}"; \
    Tasks: openwithfile; Flags: uninsdeletekey
Root: HKA; Subkey: "Software\Classes\*\shell\OpenWithZenithium"; \
    ValueType: string; ValueName: "Icon"; ValueData: "{app}\{#AppExeName},0"; \
    Tasks: openwithfile
Root: HKA; Subkey: "Software\Classes\*\shell\OpenWithZenithium\command"; \
    ValueType: string; ValueName: ""; \
    ValueData: """{app}\{#AppExeName}"" ""%1"""; Tasks: openwithfile

; "Open with Zenithium" on a folder (right-click the folder)
Root: HKA; Subkey: "Software\Classes\Directory\shell\OpenWithZenithium"; \
    ValueType: string; ValueName: ""; ValueData: "Open with {#AppName}"; \
    Tasks: openwithfolder; Flags: uninsdeletekey
Root: HKA; Subkey: "Software\Classes\Directory\shell\OpenWithZenithium"; \
    ValueType: string; ValueName: "Icon"; ValueData: "{app}\{#AppExeName},0"; \
    Tasks: openwithfolder
Root: HKA; Subkey: "Software\Classes\Directory\shell\OpenWithZenithium\command"; \
    ValueType: string; ValueName: ""; \
    ValueData: """{app}\{#AppExeName}"" ""%1"""; Tasks: openwithfolder

; "Open with Zenithium" on the background of a folder (right-click inside)
Root: HKA; Subkey: "Software\Classes\Directory\Background\shell\OpenWithZenithium"; \
    ValueType: string; ValueName: ""; ValueData: "Open with {#AppName}"; \
    Tasks: openwithfolder; Flags: uninsdeletekey
Root: HKA; Subkey: "Software\Classes\Directory\Background\shell\OpenWithZenithium"; \
    ValueType: string; ValueName: "Icon"; ValueData: "{app}\{#AppExeName},0"; \
    Tasks: openwithfolder
Root: HKA; Subkey: "Software\Classes\Directory\Background\shell\OpenWithZenithium\command"; \
    ValueType: string; ValueName: ""; \
    ValueData: """{app}\{#AppExeName}"" ""%V"""; Tasks: openwithfolder

; ---------- Add to PATH ----------------------------------------------------
Root: HKA; Subkey: "Environment"; ValueType: expandsz; ValueName: "Path"; \
    ValueData: "{olddata};{app}"; Tasks: addtopath; \
    Check: NeedsAddPath(ExpandConstant('{app}'))

[Code]
function NeedsAddPath(NewPath: string): Boolean;
var
  ExistingPath: string;
begin
  if not RegQueryStringValue(
       HKEY_CURRENT_USER, 'Environment', 'Path', ExistingPath)
  then
    ExistingPath := '';
  Result := Pos(';' + Uppercase(NewPath) + ';',
                ';' + Uppercase(ExistingPath) + ';') = 0;
end;
