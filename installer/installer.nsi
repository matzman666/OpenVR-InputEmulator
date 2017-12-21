;--------------------------------
;Include Modern UI

	!include "MUI2.nsh"

;--------------------------------
;General

	!define OVERLAY_BASEDIR "..\client_overlay\bin\win64"
	!define DRIVER_BASEDIR "..\driver_vrinputemulator"

	;Name and file
	Name "OpenVR Input Emulator"
	OutFile "OpenVR-InputEmulator.exe"
	
	;Default installation folder
	InstallDir "$PROGRAMFILES64\OpenVR-InputEmulator"
	
	;Get installation folder from registry if available
	InstallDirRegKey HKLM "Software\OpenVR-InputEmulator\Overlay" ""
	
	;Request application privileges for Windows Vista
	RequestExecutionLevel admin
	
;--------------------------------
;Variables

VAR upgradeInstallation

;--------------------------------
;Interface Settings

	!define MUI_ABORTWARNING

;--------------------------------
;Pages

	!insertmacro MUI_PAGE_LICENSE "..\LICENSE"
	!define MUI_PAGE_CUSTOMFUNCTION_PRE dirPre
	!insertmacro MUI_PAGE_DIRECTORY
	!insertmacro MUI_PAGE_INSTFILES
  
	!insertmacro MUI_UNPAGE_CONFIRM
	!insertmacro MUI_UNPAGE_INSTFILES
  
;--------------------------------
;Languages
 
	!insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Macros

;--------------------------------
;Functions

Function dirPre
	StrCmp $upgradeInstallation "true" 0 +2 
		Abort
FunctionEnd

Function .onInit
	StrCpy $upgradeInstallation "false"
 
	ReadRegStr $R0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\OpenVRInputEmulator" "UninstallString"
	StrCmp $R0 "" done
	
	
	; If SteamVR is already running, display a warning message and exit
	FindWindow $0 "Qt5QWindowIcon" "SteamVR Status"
	StrCmp $0 0 +3
		MessageBox MB_OK|MB_ICONEXCLAMATION \
			"SteamVR is still running. Cannot install this software.$\nPlease close SteamVR and try again."
		Abort
 
	
	MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION \
		"OpenVR Input Emulator is already installed. $\n$\nClick `OK` to upgrade the \
		existing installation or `Cancel` to cancel this upgrade." \
		IDOK upgrade
	Abort
 
	upgrade:
		StrCpy $upgradeInstallation "true"
	done:
FunctionEnd

;--------------------------------
;Installer Sections

Section "Install" SecInstall
	
	StrCmp $upgradeInstallation "true" 0 noupgrade 
		DetailPrint "Uninstall previous version..."
		ExecWait '"$INSTDIR\Uninstall.exe" /S _?=$INSTDIR'
		Delete $INSTDIR\Uninstall.exe
		Goto afterupgrade
		
	noupgrade:

	afterupgrade:

	SetOutPath "$INSTDIR"

	;ADD YOUR OWN FILES HERE...
	File "${OVERLAY_BASEDIR}\LICENSE"
	File "${OVERLAY_BASEDIR}\*.exe"
	File "${OVERLAY_BASEDIR}\*.dll"
	File "${OVERLAY_BASEDIR}\*.bat"
	File "${OVERLAY_BASEDIR}\*.vrmanifest"
	File "${OVERLAY_BASEDIR}\*.conf"
	File /r "${OVERLAY_BASEDIR}\res"
	File /r "${OVERLAY_BASEDIR}\qtdata"

	; Install redistributable
	ExecWait '"$INSTDIR\vcredist_x64.exe" /install /quiet'
	
	Var /GLOBAL vrRuntimePath
	nsExec::ExecToStack '"$INSTDIR\OpenVR-InputEmulatorOverlay.exe" -openvrpath'
	Pop $0
	Pop $vrRuntimePath
	DetailPrint "VR runtime path: $vrRuntimePath"

	SetOutPath "$vrRuntimePath\drivers\00vrinputemulator"
	File "${DRIVER_BASEDIR}\driver.vrdrivermanifest"
	SetOutPath "$vrRuntimePath\drivers\00vrinputemulator\resources"
	File "${DRIVER_BASEDIR}\resources\driver.vrresources"
	SetOutPath "$vrRuntimePath\drivers\00vrinputemulator\resources\settings"
	File "${DRIVER_BASEDIR}\resources\settings\default.vrsettings"
	SetOutPath "$vrRuntimePath\drivers\00vrinputemulator\resources\sounds"
	File "${DRIVER_BASEDIR}\resources\sounds\audiocue.wav"
	File "${DRIVER_BASEDIR}\resources\sounds\License.txt"
	SetOutPath "$vrRuntimePath\drivers\00vrinputemulator\bin\win64"
	File "${DRIVER_BASEDIR}\bin\x64\driver_00vrinputemulator.dll"
	
	; Install the vrmanifest
	nsExec::ExecToLog '"$INSTDIR\OpenVR-InputEmulatorOverlay.exe" -installmanifest'
	
	; Post-installation step
	nsExec::ExecToLog '"$INSTDIR\OpenVR-InputEmulatorOverlay.exe" -postinstallationstep'
  
	;Store installation folder
	WriteRegStr HKLM "Software\OpenVR-InputEmulator\Overlay" "" $INSTDIR
	WriteRegStr HKLM "Software\OpenVR-InputEmulator\Driver" "" $vrRuntimePath
  
	;Create uninstaller
	WriteUninstaller "$INSTDIR\Uninstall.exe"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\OpenVRInputEmulator" "DisplayName" "OpenVR Input Emulator"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\OpenVRInputEmulator" "UninstallString" "$\"$INSTDIR\Uninstall.exe$\""

SectionEnd

;--------------------------------
;Uninstaller Section

Section "Uninstall"
	; If SteamVR is already running, display a warning message and exit
	FindWindow $0 "Qt5QWindowIcon" "SteamVR Status"
	StrCmp $0 0 +3
		MessageBox MB_OK|MB_ICONEXCLAMATION \
			"SteamVR is still running. Cannot uninstall this software.$\nPlease close SteamVR and try again."
		Abort

	; Remove the vrmanifest
	nsExec::ExecToLog '"$INSTDIR\OpenVR-InputEmulatorOverlay.exe" -removemanifest'

	; Delete installed files
	Var /GLOBAL vrRuntimePath2
	ReadRegStr $vrRuntimePath2 HKLM "Software\OpenVR-InputEmulator\Driver" ""
	DetailPrint "VR runtime path: $vrRuntimePath2"
	Delete "$vrRuntimePath2\drivers\00vrinputemulator\driver.vrdrivermanifest"
	Delete "$vrRuntimePath2\drivers\00vrinputemulator\resources\driver.vrresources"
	Delete "$vrRuntimePath2\drivers\00vrinputemulator\resources\settings\default.vrsettings"
	Delete "$vrRuntimePath2\drivers\00vrinputemulator\resources\sounds\audiocue.wav"
	Delete "$vrRuntimePath2\drivers\00vrinputemulator\resources\sounds\License.txt"
	Delete "$vrRuntimePath2\drivers\00vrinputemulator\bin\win64\driver_00vrinputemulator.dll"
	Delete "$vrRuntimePath2\drivers\00vrinputemulator\bin\win64\driver_vrinputemulator.log"
	RMdir "$vrRuntimePath2\drivers\00vrinputemulator\resources\settings"
	RMdir "$vrRuntimePath2\drivers\00vrinputemulator\resources\sounds"
	RMdir "$vrRuntimePath2\drivers\00vrinputemulator\resources\"
	RMdir "$vrRuntimePath2\drivers\00vrinputemulator\bin\win64\"
	RMdir "$vrRuntimePath2\drivers\00vrinputemulator\bin\"
	RMdir "$vrRuntimePath2\drivers\00vrinputemulator\"
	
	!include uninstallFiles.list

	DeleteRegKey HKLM "Software\OpenVR-InputEmulator\Overlay"
	DeleteRegKey HKLM "Software\OpenVR-InputEmulator\Driver"
	DeleteRegKey HKLM "Software\OpenVR-InputEmulator"
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\OpenVRInputEmulator"
SectionEnd

