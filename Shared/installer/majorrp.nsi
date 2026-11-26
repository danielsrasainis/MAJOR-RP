; MAJOR RP installer (players): installs the game client, asks for the GTA San Andreas folder,
; creates shortcuts and an uninstaller.
;
; Why a separate script instead of nightly.nsi: the stock MTA installer lists every file by name,
; includes the server, editor and MTA-specific steps, and needs Windows-only plugins. This one
; packs the whole client folder that CI produces, so new files never need to be added here,
; and it compiles with plain makensis on Linux (GitHub Actions job "installer").
;
; Build: makensis -DSRC_DIR=<client folder> -DOUT_FILE=<setup.exe> -DVERSION=1.0.0.<n> majorrp.nsi

Unicode true
SetCompressor /SOLID lzma
RequestExecutionLevel admin   ; writes to Program Files and HKLM, like the stock MTA installer

!ifndef SRC_DIR
    !define SRC_DIR "../../InstallFiles"
!endif
!ifndef OUT_FILE
    !define OUT_FILE "MAJOR RP Setup.exe"
!endif
!ifndef VERSION
    !define VERSION "1.0.0.0"
!endif

!define PRODUCT_NAME   "MAJOR RP"
!define CLIENT_EXE     "MAJOR RP.exe"
; Same keys the client uses (Shared/sdk/majorrp_config.h MAJORRP_REGISTRY_PATH, MTA version 1.7)
!define REG_ROOT_KEY   "Software\MAJOR RP All"
!define REG_COMMON_KEY "Software\MAJOR RP All\Common"
!define REG_VER_KEY    "Software\MAJOR RP All\1.7"
!define UNINST_KEY     "Software\Microsoft\Windows\CurrentVersion\Uninstall\MAJOR RP"

Name "${PRODUCT_NAME}"
OutFile "${OUT_FILE}"
InstallDir "$PROGRAMFILES\MAJOR RP"
InstallDirRegKey HKLM "${REG_VER_KEY}" "Last Install Location"
BrandingText "MAJOR RP Installer"

VIProductVersion "${VERSION}"
VIAddVersionKey "ProductName"     "${PRODUCT_NAME}"
VIAddVersionKey "FileDescription" "MAJOR RP Setup"
VIAddVersionKey "CompanyName"     "MAJOR RP"
VIAddVersionKey "FileVersion"     "${VERSION}"
VIAddVersionKey "ProductVersion"  "${VERSION}"
VIAddVersionKey "LegalCopyright"  "Based on Multi Theft Auto (GPL-3.0)"

!include "MUI2.nsh"
!include "LogicLib.nsh"
!include "FileFunc.nsh"

Var GTA_DIR

; Look: MAJOR RP icon (same file as MAJOR RP.exe) and branded side/header pictures
!define MUI_ICON   "mta.ico"
!define MUI_UNICON "mta.ico"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_RIGHT
!define MUI_HEADERIMAGE_BITMAP          "majorrp\header.bmp"
!define MUI_HEADERIMAGE_UNBITMAP        "majorrp\header.bmp"
!define MUI_WELCOMEFINISHPAGE_BITMAP    "majorrp\welcome.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP  "majorrp\welcome.bmp"
!define MUI_ABORTWARNING

; Pages
!define MUI_WELCOMEPAGE_TITLE "Welcome to MAJOR RP"
!define MUI_WELCOMEPAGE_TEXT "This will install the MAJOR RP game client on your computer.$\r$\n$\r$\nYou need GTA San Andreas (PC) installed. If MAJOR RP is running, close it before you continue.$\r$\n$\r$\nClick Next to continue."
!insertmacro MUI_PAGE_WELCOME

!define MUI_COMPONENTSPAGE_NODESC
!insertmacro MUI_PAGE_COMPONENTS

!define MUI_PAGE_HEADER_TEXT "Install location"
!define MUI_PAGE_HEADER_SUBTEXT "Choose where to install MAJOR RP."
!define MUI_DIRECTORYPAGE_TEXT_TOP "MAJOR RP will be installed in this folder. To use a different folder, click Browse."
!insertmacro MUI_PAGE_DIRECTORY

; Second directory page: where GTA San Andreas is. The client reads this from the registry.
!define MUI_PAGE_HEADER_TEXT "GTA San Andreas folder"
!define MUI_PAGE_HEADER_SUBTEXT "Show MAJOR RP where GTA San Andreas is installed."
!define MUI_DIRECTORYPAGE_TEXT_TOP "Choose the folder that contains gta_sa.exe."
!define MUI_DIRECTORYPAGE_TEXT_DESTINATION "GTA San Andreas folder"
!define MUI_DIRECTORYPAGE_VARIABLE $GTA_DIR
!define MUI_DIRECTORYPAGE_VERIFYONLEAVE
!define MUI_PAGE_CUSTOMFUNCTION_LEAVE GtaDirLeave
!insertmacro MUI_PAGE_DIRECTORY

!insertmacro MUI_PAGE_INSTFILES

; Starting the game from here (still as admin) lets its one-time first-run setup finish without
; a second Windows permission prompt.
!define MUI_FINISHPAGE_RUN "$INSTDIR\${CLIENT_EXE}"
!define MUI_FINISHPAGE_RUN_TEXT "Start MAJOR RP"
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

; Find GTA San Andreas so most people can just click Next
Function .onInit
    ReadRegStr $GTA_DIR HKLM "${REG_COMMON_KEY}" "GTA:SA Path"
    ${If} $GTA_DIR == ""
        ReadRegStr $GTA_DIR HKLM "Software\Multi Theft Auto: San Andreas All\Common" "GTA:SA Path"
    ${EndIf}
    ${If} $GTA_DIR == ""
        ReadRegStr $0 HKCU "Software\Valve\Steam" "SteamPath"
        ${If} $0 != ""
        ${AndIf} ${FileExists} "$0\steamapps\common\Grand Theft Auto San Andreas\gta_sa.exe"
            StrCpy $GTA_DIR "$0\steamapps\common\Grand Theft Auto San Andreas"
        ${EndIf}
    ${EndIf}
    ${If} $GTA_DIR == ""
        StrCpy $GTA_DIR "$PROGRAMFILES\Rockstar Games\GTA San Andreas"
    ${EndIf}
FunctionEnd

Function GtaDirLeave
    ${IfNot} ${FileExists} "$GTA_DIR\gta_sa.exe"
        MessageBox MB_YESNO|MB_ICONEXCLAMATION "gta_sa.exe was not found in this folder:$\r$\n$GTA_DIR$\r$\n$\r$\nMAJOR RP needs GTA San Andreas to run. Continue anyway?" IDYES +2
        Abort
    ${EndIf}
FunctionEnd

Section "MAJOR RP (required)" SecCore
    SectionIn RO
    SetOutPath "$INSTDIR"
    ; Whole client folder from the CI build (MAJOR RP.exe, MTA, mods, skins, ...). Server not included.
    File /r "${SRC_DIR}/*.*"

    ; The game writes settings, logs and downloaded server files into its own folder,
    ; so normal users (S-1-5-32-545) must be able to change it, as the stock MTA installer does.
    nsExec::ExecToLog 'icacls "$INSTDIR" /grant *S-1-5-32-545:(OI)(CI)M /T /C /Q'

    WriteRegStr HKLM "${REG_COMMON_KEY}" "GTA:SA Path" "$GTA_DIR"
    WriteRegStr HKLM "${REG_VER_KEY}" "Last Install Location" "$INSTDIR"

    WriteUninstaller "$INSTDIR\Uninstall MAJOR RP.exe"
    WriteRegStr HKLM "${UNINST_KEY}" "DisplayName" "${PRODUCT_NAME}"
    WriteRegStr HKLM "${UNINST_KEY}" "DisplayIcon" "$INSTDIR\${CLIENT_EXE}"
    WriteRegStr HKLM "${UNINST_KEY}" "DisplayVersion" "${VERSION}"
    WriteRegStr HKLM "${UNINST_KEY}" "Publisher" "MAJOR RP"
    WriteRegStr HKLM "${UNINST_KEY}" "InstallLocation" "$INSTDIR"
    WriteRegStr HKLM "${UNINST_KEY}" "UninstallString" '"$INSTDIR\Uninstall MAJOR RP.exe"'
    WriteRegDWORD HKLM "${UNINST_KEY}" "NoModify" 1
    WriteRegDWORD HKLM "${UNINST_KEY}" "NoRepair" 1
    ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
    WriteRegDWORD HKLM "${UNINST_KEY}" "EstimatedSize" $0
SectionEnd

Section "Desktop shortcut" SecDesktop
    SetShellVarContext all
    CreateShortcut "$DESKTOP\MAJOR RP.lnk" "$INSTDIR\${CLIENT_EXE}" "" "$INSTDIR\${CLIENT_EXE}" 0
SectionEnd

Section "Start menu shortcuts" SecStartMenu
    SetShellVarContext all
    CreateDirectory "$SMPROGRAMS\MAJOR RP"
    CreateShortcut "$SMPROGRAMS\MAJOR RP\MAJOR RP.lnk" "$INSTDIR\${CLIENT_EXE}" "" "$INSTDIR\${CLIENT_EXE}" 0
    CreateShortcut "$SMPROGRAMS\MAJOR RP\Uninstall MAJOR RP.lnk" "$INSTDIR\Uninstall MAJOR RP.exe"
SectionEnd

; Off by default: GTA SA players almost always have DirectX 9 already, and this needs internet.
Section /o "DirectX 9 runtime (only if the game will not start)" SecDirectX
    ${If} ${FileExists} "$INSTDIR\redist\dxwebsetup.exe"
        DetailPrint "Installing DirectX 9 runtime..."
        ExecWait '"$INSTDIR\redist\dxwebsetup.exe" /Q'
    ${EndIf}
SectionEnd

Section "Uninstall"
    SetShellVarContext all
    Delete "$DESKTOP\MAJOR RP.lnk"
    RMDir /r "$SMPROGRAMS\MAJOR RP"

    ; Only remove what this installer put there, in case someone installed into a shared folder
    RMDir /r "$INSTDIR\MTA"
    RMDir /r "$INSTDIR\mods"
    RMDir /r "$INSTDIR\skins"
    RMDir /r "$INSTDIR\redist"
    RMDir /r "$INSTDIR\tests"   ; shipped by the first installer build by mistake
    Delete "$INSTDIR\*.exe"
    Delete "$INSTDIR\*.dll"
    RMDir "$INSTDIR"

    DeleteRegKey HKLM "${UNINST_KEY}"
    DeleteRegValue HKLM "${REG_VER_KEY}" "Last Install Location"
SectionEnd
