; Inno Setup script -> builds dist\HADOC-KeyBoard-Setup.exe
#define MyAppName "HADOC KeyBoard"
#define MyAppExe "HadocKeyboard.exe"
#ifndef MyAppVersion
  #define MyAppVersion "1.0.3"
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
PrivilegesRequired=admin
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
Source: "..\publish\HadocKeyboardTIP64.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\publish\HadocKeyboardTIP32.dll"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\HADOC KeyBoard"; Filename: "{app}\{#MyAppExe}"
Name: "{autodesktop}\HADOC KeyBoard"; Filename: "{app}\{#MyAppExe}"; Tasks: desktopicon

[Registry]
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; ValueType: string; ValueName: "HADOCKeyBoard"; ValueData: """{app}\{#MyAppExe}"""; Flags: uninsdeletevalue; Tasks: startup

[Run]
Filename: "{sys}\regsvr32.exe"; Parameters: "/s ""{app}\HadocKeyboardTIP64.dll"""; Flags: runhidden waituntilterminated 64bit
Filename: "{syswow64}\regsvr32.exe"; Parameters: "/s ""{app}\HadocKeyboardTIP32.dll"""; Flags: runhidden waituntilterminated 32bit
Filename: "{app}\{#MyAppExe}"; Description: "Launch HADOC KeyBoard now"; Flags: nowait postinstall skipifsilent

[UninstallRun]
Filename: "{syswow64}\regsvr32.exe"; Parameters: "/u /s ""{app}\HadocKeyboardTIP32.dll"""; Flags: runhidden waituntilterminated 32bit
Filename: "{sys}\regsvr32.exe"; Parameters: "/u /s ""{app}\HadocKeyboardTIP64.dll"""; Flags: runhidden waituntilterminated 64bit
Filename: "{sys}\taskkill.exe"; Parameters: "/F /IM {#MyAppExe}"; Flags: runhidden; RunOnceId: "KillHadocKeyboard"
