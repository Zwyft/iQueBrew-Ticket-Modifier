; iQue Tools Installer Script for Inno Setup
; Download Inno Setup from: https://jrsoftware.org/isinfo.php
; Then compile this script to create the installer

#define MyAppName "iQue Tools"
#define MyAppVersion "2.0.0"
#define MyAppPublisher "iQue Community"
#define MyAppURL "https://github.com/your-repo/ique-tools"
#define MyAppExeName "iQue_Tools.exe"

[Setup]
; App identification
AppId={{A1B2C3D4-E5F6-7890-ABCD-EF1234567890}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}

; Installation settings
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
LicenseFile=
OutputDir=installer_output
OutputBaseFilename=iQue_Tools_Setup_v{#MyAppVersion}
SetupIconFile=
Compression=lzma2
SolidCompression=yes
WizardStyle=modern

; Windows version requirements
MinVersion=10.0

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
; Main executable
Source: "dist\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion

; Required tool directories
Source: "aulon\*"; DestDir: "{app}\aulon"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "iquecrypt\*"; DestDir: "{app}\iquecrypt"; Flags: ignoreversion recursesubdirs createallsubdirs

; Create empty folders for ROMs
; (These will be created automatically by the app, but included for clarity)

; Optional: Include iQueBrew files if present
; Source: "iQueBrew\*"; DestDir: "{app}\iQueBrew"; Flags: ignoreversion recursesubdirs createallsubdirs skipifsourcedoesntexist

[Dirs]
; Create ROM directories
Name: "{app}\roms"
Name: "{app}\custom_roms"
Name: "{app}\working"

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[Code]
// Optional: Add custom installation checks here
// For example, check if WinUSB driver is installed

procedure InitializeWizard;
begin
  // Show a helpful message about WinUSB driver
end;

[Messages]
WelcomeLabel2=This will install [name/ver] on your computer.%n%nIMPORTANT: You will need to install the WinUSB driver using Zadig before connecting your iQue Player.%n%nClick Next to continue.

[UninstallDelete]
; Clean up any generated files on uninstall
Type: filesandordirs; Name: "{app}\roms"
Type: filesandordirs; Name: "{app}\custom_roms"
Type: filesandordirs; Name: "{app}\working"
