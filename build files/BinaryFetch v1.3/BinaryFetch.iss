; -----------------------------------------------------------
;  BinaryFetch Installer
;  Author: Maruf Hasan
;  Publisher: BinaryFetch-CLI
;  Version: 1.3
;
;  Installs BinaryFetch.exe and adds it to the system PATH.
; -----------------------------------------------------------


[Setup]
AppId={{9F6E2C2A-8A7B-4C6F-9A2D-3F6A0C91E4A1}}

AppName=BinaryFetch
AppVersion=1.3
AppPublisher=BinaryFetch-CLI

AppPublisherURL=https://github.com/BinaryFetch-CLI
AppSupportURL=https://github.com/BinaryFetch-CLI/BinaryFetch
AppUpdatesURL=https://github.com/BinaryFetch-CLI/BinaryFetch

DefaultDirName={autopf}\BinaryFetch
DefaultGroupName=BinaryFetch

UninstallDisplayIcon={app}\BinaryFetch.exe

LicenseFile=H:\programming\git_and_github\BinaryFetch\build files\BinaryFetch v1.3\License.txt
SetupIconFile=H:\programming\git_and_github\BinaryFetch\build files\BinaryFetch v1.3\BinaryFetch.ico

OutputDir=C:\Users\OBITO\Downloads
OutputBaseFilename=BinaryFetch-v1.3-Setup

Compression=lzma
SolidCompression=yes
WizardStyle=modern

PrivilegesRequired=admin
ChangesEnvironment=yes

ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "H:\programming\git_and_github\BinaryFetch\project_binary_fetch\binary_fetch_v1\x64\Release\BinaryFetch.exe"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{autoprograms}\BinaryFetch"; Filename: "{app}\BinaryFetch.exe"
Name: "{autodesktop}\BinaryFetch"; Filename: "{app}\BinaryFetch.exe"

[Code]

const
  WM_SETTINGCHANGE = $001A;
  SMTO_ABORTIFHUNG = $0002;

function SendMessageTimeoutA(
  hWnd: Longint; Msg: Longint; wParam: Longint; lParam: AnsiString;
  fuFlags: Longint; uTimeout: Longint; var lpdwResult: Longint
): Longint;
  external 'SendMessageTimeoutA@user32.dll stdcall';

// Tell Explorer and other running processes to re-read the environment
// block from the registry. Without this, RegWriteStringValue alone
// leaves every already-running process (including Explorer, which is
// what spawns new terminal windows) with a stale PATH, so a "new"
// terminal opened after install still won't find binaryfetch.
procedure RefreshEnvironment();
var
  dwResult: Longint;
begin
  SendMessageTimeoutA(
    HWND_BROADCAST, WM_SETTINGCHANGE, 0, 'Environment',
    SMTO_ABORTIFHUNG, 5000, dwResult
  );
end;

function NeedsAddPath(): Boolean;
var
  Paths: string;
begin
  Result := True;
  if RegQueryStringValue(
      HKLM,
      'SYSTEM\CurrentControlSet\Control\Session Manager\Environment',
      'Path',
      Paths) then
  begin
    Result := Pos(
      Lowercase(ExpandConstant('{app}')),
      Lowercase(Paths)
    ) = 0;
  end;
end;

procedure AddAppPathToSystemPath();
var
  Paths, NewPaths, AppPath: string;
begin
  AppPath := ExpandConstant('{app}');
  if RegQueryStringValue(
      HKLM,
      'SYSTEM\CurrentControlSet\Control\Session Manager\Environment',
      'Path',
      Paths) then
  begin
    if Pos(Lowercase(AppPath), Lowercase(Paths)) = 0 then
    begin
      NewPaths := Paths;
      if (Length(NewPaths) > 0) and (NewPaths[Length(NewPaths)] <> ';') then
        NewPaths := NewPaths + ';';
      NewPaths := NewPaths + AppPath;
      RegWriteStringValue(
        HKLM,
        'SYSTEM\CurrentControlSet\Control\Session Manager\Environment',
        'Path',
        NewPaths
      );
    end;
  end
  else
  begin
    RegWriteStringValue(
      HKLM,
      'SYSTEM\CurrentControlSet\Control\Session Manager\Environment',
      'Path',
      AppPath
    );
  end;

  RefreshEnvironment();
end;

procedure RemoveAppPathFromSystemPath();
var
  Paths, NewPaths, AppPath: string;
begin
  AppPath := ExpandConstant('{app}');
  if not RegQueryStringValue(
      HKLM,
      'SYSTEM\CurrentControlSet\Control\Session Manager\Environment',
      'Path',
      Paths) then
    Exit;

  NewPaths := Paths;

  StringChangeEx(NewPaths, ';' + AppPath, '', True);
  StringChangeEx(NewPaths, AppPath + ';', '', True);
  StringChangeEx(NewPaths, AppPath, '', True);

  while Pos(';;', NewPaths) > 0 do
    StringChangeEx(NewPaths, ';;', ';', True);

  if (Length(NewPaths) > 0) and (NewPaths[1] = ';') then
    Delete(NewPaths, 1, 1);

  if (Length(NewPaths) > 0) and (NewPaths[Length(NewPaths)] = ';') then
    Delete(NewPaths, Length(NewPaths), 1);

  if NewPaths <> Paths then
  begin
    RegWriteStringValue(
      HKLM,
      'SYSTEM\CurrentControlSet\Control\Session Manager\Environment',
      'Path',
      NewPaths
    );
    RefreshEnvironment();
  end;
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  if CurUninstallStep = usUninstall then
    RemoveAppPathFromSystemPath();
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssPostInstall then
  begin
    if NeedsAddPath() then
      AddAppPathToSystemPath();

    MsgBox(
      'BinaryFetch v1.3 installed successfully.'#13#13 +
      'Open a NEW terminal and type:'#13 +
      'binaryfetch'#13#13 +
      'If it still doesn''t work, sign out and back in (or restart).',
      mbInformation,
      MB_OK
    );
  end;
end;
