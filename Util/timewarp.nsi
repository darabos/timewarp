; timewarp.nsi
;
; This script is based on example2.nsi.
;
; It will install TimeWarp into a directory that the user selects.

;--------------------------------

!define VER_MAJOR 0.05
!define VER_MINOR 8

; The name of the installer
Name "TimeWarp"
SetCompressor bzip2

; The file to write
OutFile "timewarp-${VER_MAJOR}u${VER_MINOR}.exe"

; The default installation directory
InstallDir $PROGRAMFILES\TimeWarp

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM SOFTWARE\TimeWarp "Install_Dir"

; The text to prompt the user to enter a directory
ComponentText "This will install Star Control: Time Warp ${VER_MAJOR}u${VER_MINOR} on your computer. Select which optional things you want installed."

; The text to prompt the user to enter a directory
DirText "Choose a directory to install in to:"

;--------------------------------

; The stuff to install
Section "Time Warp Core (required)"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File "..\*.dat"
  File "..\*.bat"
  File "..\*.cfg"
  File "..\*.dll"
  File "..\*.dmo"
  File "..\timewarp.exe"
  File "..\*.html"
  File "..\*.ini"
  File "..\*.txt"

  SetOutPath $INSTDIR\docs
  File "..\docs\*.*"

  SetOutPath $INSTDIR\fleets
  File "..\fleets\*.scf"

  SetOutPath $INSTDIR\scrshots
  File "..\scrshots\*.*"
  
  SetOutPath $INSTDIR\ships
  File "..\ships\*.*"

  SetOutPath $INSTDIR\ships\sc1
  File "..\ships\sc1\*.*"

  SetOutPath $INSTDIR\ships\sc2
  File "..\ships\sc2\*.*"

  SetOutPath $INSTDIR\ships\sc3
  File "..\ships\sc3\*.*"

  SetOutPath $INSTDIR\Util
  File "..\Util\*.*"
    

  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\TimeWarp "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TimeWarp" "DisplayName" "TimeWarp (remove only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TimeWarp" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteUninstaller "uninstall.exe"
  CreateDirectory "$SMPROGRAMS\TimeWarp"
  CreateShortCut "$SMPROGRAMS\TimeWarp\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\TimeWarp\readme.lnk" "$INSTDIR\readme.html" "" "$INSTDIR\readme.html" 0
  CreateShortCut "$SMPROGRAMS\TimeWarp\timewarp.lnk" "$INSTDIR\timewarp.exe" "" "$INSTDIR\timewarp.exe" 0
SectionEnd

; optional section (can be disabled by the user)
Section "Sources"
  SetOutPath $INSTDIR
  File "..\*."
  File "..\*.ico"
  File "..\*.rc"
  

  SetOutPath $INSTDIR\source
  File "..\source\*.*"
  SetOutPath $INSTDIR\source\ais
  File "..\source\ais\*.*"
  SetOutPath $INSTDIR\source\games
  File "..\source\games\*.*"
  SetOutPath $INSTDIR\source\melee
  File "..\source\melee\*.*"
  SetOutPath $INSTDIR\source\newships
  File "..\source\newships\*.*"
  SetOutPath $INSTDIR\source\other
  File "..\source\other\*.*"
  SetOutPath $INSTDIR\source\sc1ships
  File "..\source\sc1ships\*.*"
  SetOutPath $INSTDIR\source\sc2ships
  File "..\source\sc2ships\*.*"
  SetOutPath $INSTDIR\source\sc3ships
  File "..\source\sc3ships\*.*"
  SetOutPath $INSTDIR\source\twgui
  File "..\source\twgui\*.*"
  SetOutPath $INSTDIR\source\util
  File "..\source\util\*.*"

SectionEnd

;--------------------------------

; Uninstaller

UninstallText "This will uninstall Star Control: Time Warp. Hit next to continue."

; Uninstall section

Section "Uninstall"
  
  ; remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TimeWarp"
  DeleteRegKey HKLM SOFTWARE\TimeWarp

  ; remove shortcuts, if any
  Delete "$SMPROGRAMS\TimeWarp\*.*"

  Delete "$INSTDIR\docs\*.*"  
  RMDir  "$INSTDIR\docs"

  Delete "$INSTDIR\fleets\*.*"
  RMDir  "$INSTDIR\fleets"

  Delete "$INSTDIR\scrshots\*.*"
  RMDir  "$INSTDIR\scrshots"

  Delete "$INSTDIR\ships\sc1\*.*"
  RMDir  "$INSTDIR\ships\sc1"

  Delete "$INSTDIR\ships\sc2\*.*"
  RMDir  "$INSTDIR\ships\sc2"

  Delete "$INSTDIR\ships\sc3\*.*"
  RMDir  "$INSTDIR\ships\sc3"

  Delete "$INSTDIR\ships\*.*"
  RMDir  "$INSTDIR\ships"

  Delete "$INSTDIR\source\ais\*.*"
  RMDir  "$INSTDIR\source\ais"

  Delete "$INSTDIR\source\games\*.*"
  RMDir  "$INSTDIR\source\games"

  Delete "$INSTDIR\source\melee\*.*"
  RMDir  "$INSTDIR\source\melee"

  Delete "$INSTDIR\source\newships\*.*"
  RMDir  "$INSTDIR\source\newships"

  Delete "$INSTDIR\source\other\*.*"
  RMDir  "$INSTDIR\source\other"

  Delete "$INSTDIR\source\sc1ships\*.*"
  RMDir  "$INSTDIR\source\sc1ships"

  Delete "$INSTDIR\source\sc2ships\*.*"
  RMDir  "$INSTDIR\source\sc2ships"

  Delete "$INSTDIR\source\sc3ships\*.*"
  RMDir  "$INSTDIR\source\sc3ships"

  Delete "$INSTDIR\source\twgui\*.*"
  RMDir  "$INSTDIR\source\twgui"

  Delete "$INSTDIR\source\util\*.*"
  RMDir  "$INSTDIR\source\util"

  Delete "$INSTDIR\source\*.*"
  RMDir  "$INSTDIR\source"

  Delete "$INSTDIR\util\*.*"
  RMDir  "$INSTDIR\util"

  Delete "$INSTDIR\*.*"
  RMDir  "$INSTDIR"

  ; remove directories used
  RMDir "$SMPROGRAMS\TimeWarp"

SectionEnd
