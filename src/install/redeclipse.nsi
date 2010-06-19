Name "RedEclipse"

OutFile "redeclipse-B2-setup.exe"

InstallDir $PROGRAMFILES\RedEclipse

InstallDirRegKey HKLM "Software\RedEclipse" "Install_Dir"

SetCompressor /SOLID lzma
XPStyle on

Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

Section "RedEclipse (required)"

  SectionIn RO
  
  SetOutPath $INSTDIR
  
  File /r "..\..\*.*"
  
  WriteRegStr HKLM SOFTWARE\RedEclipse "Install_Dir" "$INSTDIR"
  
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\RedEclipse" "DisplayName" "RedEclipse"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\RedEclipse" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\RedEclipse" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\RedEclipse" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd

Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\RedEclipse"
  
  SetOutPath "$INSTDIR"
  
  CreateShortCut "$INSTDIR\RedEclipse.lnk"                "$INSTDIR\redeclipse.bat" "" "$INSTDIR\bin\bfclient.exe" 0
  CreateShortCut "$SMPROGRAMS\RedEclipse\RedEclipse.lnk" "$INSTDIR\redeclipse.bat" "" "$INSTDIR\bin\bfclient.exe" 0
  CreateShortCut "$SMPROGRAMS\RedEclipse\Uninstall.lnk"   "$INSTDIR\uninstall.exe"   "" "$INSTDIR\uninstall.exe" 0
  
SectionEnd

Section "Uninstall"
  
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\RedEclipse"
  DeleteRegKey HKLM SOFTWARE\RedEclipse

  RMDir /r "$SMPROGRAMS\RedEclipse"
  RMDir /r "$INSTDIR"

SectionEnd
