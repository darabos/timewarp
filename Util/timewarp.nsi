;NSIS Setup Script

!define VER_VERSION 0
!define VER_SUBVERSION 05
!define VER_BUILD 9

!define PRODUCT_NAME "Star Control: TimeWarp"

;--------------------------------
;Configuration

OutFile timewarp-${VER_VERSION}.${VER_SUBVERSION}u${VER_BUILD}.exe
SetCompressor lzma

InstallDir $PROGRAMFILES\TimeWarp
InstallDirRegKey HKLM SOFTWARE\TimeWarp ""

;--------------------------------

;Include Modern UI
!include "MUI.nsh"

;--------------------------------
;Configuration

;Names
Name "TimeWarp"
Caption "${PRODUCT_NAME} ${VER_VERSION}.${VER_SUBVERSION}u${VER_BUILD} Setup"

;Interface Settings
!define MUI_ABORTWARNING
!define MUI_HEADERIMAGE
!define MUI_ICON "..\scpwin.ico"
!define MUI_UNICON "..\scpwin.ico"

!define MUI_COMPONENTSPAGE_SMALLDESC

;Pages
!define MUI_WELCOMEPAGE_TEXT "This wizard will guide you through the installation of ${PRODUCT_NAME}.\r\n\r\n\r\n$_CLICK"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "license.txt"
!insertmacro MUI_PAGE_COMPONENTS
Page custom CreateShortCutF
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_LINK "Visit the ${PRODUCT_NAME} website for the latest news"
!define MUI_FINISHPAGE_LINK_LOCATION "http://timewarp.sf.net/"

!define MUI_FINISHPAGE_RUN "$INSTDIR\twwin.exe"
!define MUI_FINISHPAGE_SHOWREADME $INSTDIR\readme.html

!define MUI_FINISHPAGE_NOREBOOTSUPPORT

!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
;--------------------------------


;Reserve Files
  
  ;These files should be inserted before other files in the data block
  ;Keep these lines before any File command
  ;Only for BZIP2 (solid) compression
  
  ReserveFile "timewarp.ini"
  !insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

;--------------------------------
;Variables

  Var INI_VALUE

;--------------------------------
;Languages

!insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

!define SF_SELECTED 1

; The stuff to install
Section "TimeWarp Core (required)"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File "..\*.dat"
  File "..\*.bat"
  File "..\*.cfg"
  File "..\*.dll"
  File "..\*.dmo"
  File "..\twwin.exe"
  File "..\*.html"
  File "..\*.ini"
  File "..\*.txt"

  SetOutPath $INSTDIR\docs
  File "..\docs\*.*"
  SetOutPath $INSTDIR\fleets
  File /r "..\fleets\*.*"
  SetOutPath $INSTDIR\scrshots
  File /r "..\scrshots\*.*"
  SetOutPath $INSTDIR\ships
  File /r "..\ships\*.*"
  SetOutPath $INSTDIR\gamex
  File /r "..\gamex\*.*"
  SetOutPath $INSTDIR\gamedata
  File /r "..\gamedata\*.*"
  SetOutPath $INSTDIR\interfaces
  File /r "..\interfaces\*.*"
  SetOutPath $INSTDIR\util
  File /r "..\util\*.*"

  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\TimeWarp "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TimeWarp" "DisplayName" "TimeWarp (remove only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TimeWarp" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteUninstaller "uninstall.exe"

  WriteUninstaller "uninstall.exe"

!insertmacro MUI_INSTALLOPTIONS_READ $INI_VALUE "timewarp.ini" "Field 3" "State" 
StrCmp $INI_VALUE "1" "" +2
CreateShortCut "$DESKTOP\TimeWarp.lnk" "$INSTDIR\twwin.exe"
  
!insertmacro MUI_INSTALLOPTIONS_READ $INI_VALUE "timewarp.ini" "Field 2" "State"
 StrCmp $INI_VALUE "1" "" +5
  CreateDirectory "$SMPROGRAMS\TimeWarp"
  CreateShortCut "$SMPROGRAMS\TimeWarp\Uninstall.lnk" "$INSTDIR\uninstall.exe"  "Uninstall"
  CreateShortCut "$SMPROGRAMS\TimeWarp\readme.lnk" "$INSTDIR\readme.html" "Readme.html"
  CreateShortCut "$SMPROGRAMS\TimeWarp\timewarp.lnk" "$INSTDIR\twwin.exe" "Star Control: TimeWarp"

SectionEnd

Section "Source"
  SetOutPath $INSTDIR\source
  File /r "..\source\*.*"
SectionEnd
;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TimeWarp"
  DeleteRegKey HKLM SOFTWARE\TimeWarp


  RMDir  /r "$INSTDIR"

  ; remove group used
  RMDir /r "$SMPROGRAMS\TimeWarp"
  ; remove shortcuts, if any
  Delete "$DESKTOP\TimeWarp.lnk"

SectionEnd

;--------------------------------
;Installer Functions

Function .onInit
  ;Extract InstallOptions INI files
  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "timewarp.ini"
FunctionEnd

Function CreateShortCutF
  !insertmacro MUI_HEADER_TEXT "${PRODUCT_NAME} Option" ""
  !insertmacro MUI_INSTALLOPTIONS_DISPLAY "timewarp.ini"
FunctionEnd

