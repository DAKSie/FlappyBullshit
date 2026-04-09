; Flappy Bird NSIS Installer Script

!include "MUI2.nsh"

; Basic settings
Name "Flappy Bird"
OutFile "FlappyBird-Installer.exe"
InstallDir "$PROGRAMFILES\Flappy Bird"
InstallDirRegKey HKCU "Software\Flappy Bird" ""

; MUI Settings
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

; Installer sections
Section "Install"
  SetOutPath "$INSTDIR"
  
  ; Copy executable
  File "bin\jump.exe"
  
  ; Copy all DLL dependencies from MSYS2
  File "bin\*.dll"
  
  ; Create Start Menu shortcuts
  SetShellVarContext all
  CreateDirectory "$SMPROGRAMS\Flappy Bird"
  CreateShortCut "$SMPROGRAMS\Flappy Bird\Flappy Bird.lnk" "$INSTDIR\jump.exe"
  CreateShortCut "$SMPROGRAMS\Flappy Bird\Uninstall.lnk" "$INSTDIR\uninstall.exe"
  
  ; Create Desktop shortcut
  CreateShortCut "$DESKTOP\Flappy Bird.lnk" "$INSTDIR\jump.exe"
  
  ; Store install folder in registry
  WriteRegStr HKCU "Software\Flappy Bird" "" $INSTDIR
  
  ; Create uninstaller
  WriteUninstaller "$INSTDIR\uninstall.exe"
  
  ; Add uninstall info to Windows Control Panel
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Flappy Bird" \
    "DisplayName" "Flappy Bird"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Flappy Bird" \
    "UninstallString" "$INSTDIR\uninstall.exe"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Flappy Bird" \
    "DisplayIcon" "$INSTDIR\jump.exe"
SectionEnd

; Uninstaller section
Section "Uninstall"
  SetShellVarContext all
  
  ; Remove shortcuts
  RMDir /r "$SMPROGRAMS\Flappy Bird"
  Delete "$DESKTOP\Flappy Bird.lnk"
  
  ; Remove all files from install directory
  Delete "$INSTDIR\jump.exe"
  Delete "$INSTDIR\*.dll"
  Delete "$INSTDIR\uninstall.exe"
  
  RMDir "$INSTDIR"
  
  ; Remove registry entries
  DeleteRegKey HKCU "Software\Flappy Bird"
  DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Flappy Bird"
SectionEnd
