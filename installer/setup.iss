; Inno Setup script -> builds dist\HADOC-KeyBoard-Setup.exe
#define MyAppName "HADOC KeyBoard"
#define MyAppExe "HadocKeyboard.exe"
#ifndef MyAppVersion
  #define MyAppVersion "1.0.1"
#endif

[Setup]
AppId={{B5C1A3D2-6E0F-4B7A-9A53-1F6D2C8E4A10}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher=KyZer-HADOC
DefaultDirName={autopf}\HADOC KeyBoard
DefaultGroupName=HADOC KeyBoard
OutputDir=..\dist
OutputBaseFilename=HADOC-KeyBoard-Setup
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
CloseApplications=yes
CloseApplicationsFilter=HadocKeyboard.exe
RestartApplications=no
AppMutex=HADOC-KeyBoard-SingleInstance
UninstallDisplayName={#MyAppName}
UninstallDisplayIcon={app}\{#MyAppExe}

[Tasks]
Name: "startup"; Description: "Start HADOC KeyBoard when Windows starts"
Name: "desktopicon"; Description: "Create a desktop shortcut"; Flags: unchecked

[Files]
Source: "..\publish\{#MyAppExe}"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\HADOC KeyBoard"; Filename: "{app}\{#MyAppExe}"
Name: "{autodesktop}\HADOC KeyBoard"; Filename: "{app}\{#MyAppExe}"; Tasks: desktopicon

[Registry]
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; ValueType: string; ValueName: "HADOCKeyBoard"; ValueData: """{app}\{#MyAppExe}"""; Flags: uninsdeletevalue; Tasks: startup

[Run]
Filename: "{app}\{#MyAppExe}"; Description: "Launch HADOC KeyBoard now"; Flags: nowait postinstall skipifsilent; Check: FileExists(ExpandConstant('{app}\{#MyAppExe}'))

[UninstallRun]
Filename: "{sys}\taskkill.exe"; Parameters: "/F /IM {#MyAppExe}"; Flags: runhidden; RunOnceId: "KillHadocKeyboard"
