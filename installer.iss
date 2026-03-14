[Setup]
AppName=GoPieMenu
AppVersion=1.0.0
AppPublisher=RyuuMeow
DefaultDirName={autopf}\GoPieMenu
DefaultGroupName=GoPieMenu

OutputDir=Output
OutputBaseFilename=GoPieMenu_Setup
Compression=lzma
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64

[Files]
Source: "deploy\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\GoPieMenu"; Filename: "{app}\GoPieMenu.exe"
Name: "{autodesktop}\GoPieMenu"; Filename: "{app}\GoPieMenu.exe"; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked