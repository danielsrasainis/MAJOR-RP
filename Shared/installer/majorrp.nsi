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
    !define OUT_FILE "MRP_installer.exe"
!endif
!ifndef VERSION
    !define VERSION "1.0.0.0"
!endif

!define PRODUCT_NAME   "MAJOR RP"
!define CLIENT_EXE     "Launcher.exe"  ; MAJOR RP: was "MAJOR RP.exe" (renamed 2026-11-28)
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

; Look (Danex, 2026-11-28): dark theme with orange accents, Danex's banner over the inner pages,
; the side picture on Welcome/Finish, Anton titles and Inter text (both OFL, majorrp\fonts-LICENSE.txt).
; majorrp\mrp_ui.exe is NSIS's modern_headerbmpr.exe with a taller window for the banner
; (made by the scratch tool mkui.py: only control positions changed).
Caption "MRP Setup"
UninstallCaption "MRP Uninstall"
BrandingText " "
SetFont "Inter" 8
!define MUI_UI_HEADERIMAGE_RIGHT "majorrp\mrp_ui.exe"
!define MUI_ICON   "mta.ico"
!define MUI_UNICON "uninstall.ico"  ; greyed MAJOR RP icon (Danex, 2026-11-26)
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_RIGHT
!define MUI_HEADERIMAGE_BITMAP          "majorrp\banner.bmp"
!define MUI_HEADERIMAGE_UNBITMAP        "majorrp\banner.bmp"
!define MUI_WELCOMEFINISHPAGE_BITMAP    "majorrp\welcome.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP  "majorrp\welcome.bmp"
!define MUI_BGCOLOR   "101013"
!define MUI_TEXTCOLOR "F0F0F4"
!define MUI_ABORTWARNING
!define MUI_CUSTOMFUNCTION_GUIINIT   MrpGuiInit
!define MUI_CUSTOMFUNCTION_UNGUIINIT un.MrpGuiInit

; Pages
!define MUI_WELCOMEPAGE_TITLE "WELCOME TO MAJOR ROLEPLAY"
!define MUI_WELCOMEPAGE_TEXT "This will install the MAJOR RP game client on your computer.$\r$\n$\r$\nAll what you need is GTA San Andreas game installed. Check it before you continue this installation."
!define MUI_PAGE_CUSTOMFUNCTION_SHOW MrpWelcomeShow
!insertmacro MUI_PAGE_WELCOME

!define MUI_PAGE_HEADER_TEXT "CHOOSE COMPONENTS"
!define MUI_PAGE_HEADER_SUBTEXT "Choose which features of MAJOR RP you want to install."
!define MUI_COMPONENTSPAGE_NODESC
!define MUI_PAGE_CUSTOMFUNCTION_SHOW MrpShowNext
!insertmacro MUI_PAGE_COMPONENTS

!define MUI_PAGE_HEADER_TEXT "INSTALL LOCATION"
!define MUI_PAGE_HEADER_SUBTEXT "Choose where to install MAJOR RP."
!define MUI_DIRECTORYPAGE_TEXT_TOP "MAJOR RP will be installed in this folder. To use a different folder, click Browse."
!define MUI_PAGE_CUSTOMFUNCTION_SHOW MrpShowNext
!insertmacro MUI_PAGE_DIRECTORY

; Second directory page: where GTA San Andreas is. The client reads this from the registry.
!define MUI_PAGE_HEADER_TEXT "GTA SAN ANDREAS FOLDER"
!define MUI_PAGE_HEADER_SUBTEXT "Show MAJOR RP where GTA San Andreas is installed."
!define MUI_DIRECTORYPAGE_TEXT_TOP "Choose the folder that contains gta_sa.exe."
!define MUI_DIRECTORYPAGE_TEXT_DESTINATION "GTA San Andreas folder"
!define MUI_DIRECTORYPAGE_VARIABLE $GTA_DIR
!define MUI_DIRECTORYPAGE_VERIFYONLEAVE
!define MUI_PAGE_CUSTOMFUNCTION_SHOW MrpShowInstall
!define MUI_PAGE_CUSTOMFUNCTION_LEAVE GtaDirLeave
!insertmacro MUI_PAGE_DIRECTORY

!define MUI_PAGE_HEADER_TEXT "INSTALLING"
!define MUI_PAGE_HEADER_SUBTEXT "Please wait while MAJOR RP is being installed."
!define MUI_INSTFILESPAGE_FINISHHEADER_TEXT "INSTALLATION COMPLETE"
!define MUI_INSTFILESPAGE_FINISHHEADER_SUBTEXT "MAJOR RP was installed successfully."
!define MUI_INSTFILESPAGE_ABORTHEADER_TEXT "INSTALLATION ABORTED"
!define MUI_PAGE_CUSTOMFUNCTION_SHOW MrpShowNext
!insertmacro MUI_PAGE_INSTFILES

; Starting the game from here (still as admin) lets its one-time first-run setup finish without
; a second Windows permission prompt.
!define MUI_FINISHPAGE_TITLE "MAJOR RP IS INSTALLED"
!define MUI_FINISHPAGE_TEXT "MAJOR RP has been installed on your computer.$\r$\n$\r$\nClick Finish to close Setup."
!define MUI_FINISHPAGE_RUN "$INSTDIR\${CLIENT_EXE}"
!define MUI_FINISHPAGE_RUN_TEXT "Start MAJOR RP"
!define MUI_PAGE_CUSTOMFUNCTION_SHOW MrpFinishShow
!insertmacro MUI_PAGE_FINISH

!define MUI_PAGE_HEADER_TEXT "UNINSTALL MAJOR RP"
!define MUI_PAGE_HEADER_SUBTEXT "Remove MAJOR RP from your computer."
!define MUI_PAGE_CUSTOMFUNCTION_SHOW un.MrpShowUninstall
!insertmacro MUI_UNPAGE_CONFIRM
!define MUI_PAGE_HEADER_TEXT "UNINSTALLING"
!define MUI_PAGE_HEADER_SUBTEXT "Please wait while MAJOR RP is being removed."
!define MUI_UNINSTFILESPAGE_FINISHHEADER_TEXT "UNINSTALL COMPLETE"
!define MUI_UNINSTFILESPAGE_FINISHHEADER_SUBTEXT "MAJOR RP was removed from your computer."
!define MUI_UNINSTFILESPAGE_ABORTHEADER_TEXT "UNINSTALL ABORTED"
!define MUI_PAGE_CUSTOMFUNCTION_SHOW un.MrpShowClose
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

;--------------------------------
; Dark theme helpers. Colours are RRGGBB for SetCtlColors and 0x00BBGGRR for Windows messages.
!define MRP_BG      "101013"
!define MRP_PANEL   "18181C"
!define MRP_TEXT    "F0F0F4"
!define MRP_SUB     "AAAAB2"
!define MRP_ORANGE  "FF6E00"
!define MRP_LINE    "34343A"

Var MrpFontTitle
Var MrpFontBig
Var MrpFontMono

; Load the embedded fonts for this process only (FR_PRIVATE), before any window is created
!macro MRP_LOAD_FONTS
    InitPluginsDir
    File "/oname=$PLUGINSDIR\inter.otf" "majorrp\inter.otf"
    File "/oname=$PLUGINSDIR\anton.ttf" "majorrp\anton.ttf"
    System::Call 'gdi32::AddFontResourceEx(t "$PLUGINSDIR\inter.otf", i 0x10, p 0) i'
    System::Call 'gdi32::AddFontResourceEx(t "$PLUGINSDIR\anton.ttf", i 0x10, p 0) i'
    ; button pictures (Windows push buttons can't be coloured, so they show a picture of an orange button)
    SetOutPath "$PLUGINSDIR\buttons"
    File "majorrp\buttons\*.bmp"
    SetOutPath "$PLUGINSDIR"
!macroend

; Put a button picture on one of the main buttons: BS_BITMAP | BS_FLAT, classic look, picture at the button's size
!macro MRP_BUTTON_IMAGE ID NAME
    GetDlgItem $0 $HWNDPARENT ${ID}
    System::Call 'user32::GetWindowLong(p r0, i -16) i .r1'
    IntOp $1 $1 | 0x8080
    System::Call 'user32::SetWindowLong(p r0, i -16, i r1) i'
    !insertmacro MRP_PLAIN_CONTROL $0
    System::Call '*(i,i,i,i) p .r1'
    System::Call 'user32::GetClientRect(p r0, p r1)'
    System::Call '*$1(i,i,i .r2,i .r3)'
    System::Free $1
    System::Call 'user32::LoadImage(p 0, t "$PLUGINSDIR\buttons\${NAME}.bmp", i 0, i r2, i r3, i 0x10) p .r1'
    SendMessage $0 0xF7 0 $1 $1   ; BM_SETIMAGE, returns the old picture
    ${If} $1 != 0
        System::Call 'gdi32::DeleteObject(p r1)'
    ${EndIf}
    System::Call 'user32::InvalidateRect(p r0, p 0, i 1)'
!macroend

; Dark Windows look for a button / checkbox / scroll bar (Windows 10 1809+; older Windows ignores it)
!macro MRP_DARK_CONTROL HWND
    System::Call 'uxtheme::SetWindowTheme(p ${HWND}, w "DarkMode_Explorer", p 0)'
!macroend

; Classic (unthemed) look, so the control uses our colours instead of the Windows theme
!macro MRP_PLAIN_CONTROL HWND
    System::Call 'uxtheme::SetWindowTheme(p ${HWND}, w "", w "")'
!macroend

!macro MRP_FUNCTIONS UN
Function ${UN}MrpGuiInit
    CreateFont $MrpFontTitle "Anton" 12 400
    CreateFont $MrpFontBig   "Anton" 18 400
    CreateFont $MrpFontMono  "Consolas" 9 400

    ; window, dark title bar (Windows 10 2004+ uses 20, older Windows 10 used 19), Windows 11 caption colour
    SetCtlColors $HWNDPARENT ${MRP_TEXT} ${MRP_BG}
    System::Call 'dwmapi::DwmSetWindowAttribute(p $HWNDPARENT, i 19, *i 1, i 4)'
    System::Call 'dwmapi::DwmSetWindowAttribute(p $HWNDPARENT, i 20, *i 1, i 4)'
    System::Call 'dwmapi::DwmSetWindowAttribute(p $HWNDPARENT, i 35, *i 0x00131010, i 4)'

    ; page title (Anton) and subtitle, orange bar next to the title, lines
    SendMessage $mui.Header.Text ${WM_SETFONT} $MrpFontTitle 1
    SetCtlColors $mui.Header.Text ${MRP_TEXT} ${MRP_BG}
    SetCtlColors $mui.Header.SubText ${MRP_SUB} ${MRP_BG}
    GetDlgItem $0 $HWNDPARENT 1036
    SetCtlColors $0 "" ${MRP_ORANGE}
    SetCtlColors $mui.Line.Standard "" ${MRP_LINE}
    SetCtlColors $mui.Line.FullWindow "" ${MRP_LINE}
    SetCtlColors $mui.Branding.Background ${MRP_BG} ${MRP_BG}
    SetCtlColors $mui.Branding.Text ${MRP_BG} ${MRP_BG}

FunctionEnd

; Back / Next / Cancel pictures; $R9 = picture for the Next button (next, install, finish, uninstall, close)
Function ${UN}MrpButtons
    !insertmacro MRP_BUTTON_IMAGE 3 "back"
    !insertmacro MRP_BUTTON_IMAGE 1 $R9
    !insertmacro MRP_BUTTON_IMAGE 2 "cancel"
FunctionEnd

; Colours every control of the inner page that is showing (components, folders, progress, uninstall)
Function ${UN}MrpPageShow
    FindWindow $1 "#32770" "" $HWNDPARENT
    SetCtlColors $1 ${MRP_TEXT} ${MRP_BG}

    ; plain texts
    ${ForEach} $2 1006 1029 + 1
        GetDlgItem $0 $1 $2
        ${If} $0 != 0
            SetCtlColors $0 ${MRP_TEXT} ${MRP_BG}
        ${EndIf}
    ${Next}
    ; components page: space required / "select components" labels
    ${ForEach} $2 1021 1023 + 1
        GetDlgItem $0 $1 $2
        ${If} $0 != 0
            SetCtlColors $0 ${MRP_SUB} ${MRP_BG}
        ${EndIf}
    ${Next}
    GetDlgItem $0 $1 1024   ; space available
    ${If} $0 != 0
        SetCtlColors $0 ${MRP_SUB} ${MRP_BG}
    ${EndIf}

    ; folder group box: orange caption
    GetDlgItem $0 $1 1020
    ${If} $0 != 0
        !insertmacro MRP_PLAIN_CONTROL $0
        SetCtlColors $0 ${MRP_ORANGE} ${MRP_BG}
    ${EndIf}

    ; folder edit boxes (directory page 1019, uninstall page 1000)
    ${ForEach} $2 1019 1000 - 19
        GetDlgItem $0 $1 $2
        ${If} $0 != 0
            SetCtlColors $0 ${MRP_TEXT} ${MRP_PANEL}
        ${EndIf}
    ${Next}

    ; Browse... and Show details buttons
    GetDlgItem $0 $1 1001
    ${If} $0 != 0
        !insertmacro MRP_DARK_CONTROL $0
    ${EndIf}
    GetDlgItem $0 $1 1027
    ${If} $0 != 0
        !insertmacro MRP_DARK_CONTROL $0
    ${EndIf}

    ; components tree: dark background, light text (TVM_SETBKCOLOR / TVM_SETTEXTCOLOR)
    GetDlgItem $0 $1 1032
    ${If} $0 != 0
        !insertmacro MRP_DARK_CONTROL $0
        SendMessage $0 0x111D 0 0x00131010
        SendMessage $0 0x111E 0 0x00F4F0F0
    ${EndIf}

    ; progress bar: flat orange (PBM_SETBARCOLOR / PBM_SETBKCOLOR need the classic look)
    GetDlgItem $0 $1 1004
    ${If} $0 != 0
        !insertmacro MRP_PLAIN_CONTROL $0
        SendMessage $0 0x0409 0 0x00006EFF
        SendMessage $0 0x2001 0 0x002E2828
    ${EndIf}

    ; install log: dark box, grey code-style text (LVM_SETBKCOLOR / LVM_SETTEXTBKCOLOR / LVM_SETTEXTCOLOR)
    GetDlgItem $0 $1 1016
    ${If} $0 != 0
        !insertmacro MRP_DARK_CONTROL $0
        SendMessage $0 0x1001 0 0x001C1818
        SendMessage $0 0x1026 0 0x001C1818
        SendMessage $0 0x1024 0 0x00B2AAAA
        SendMessage $0 ${WM_SETFONT} $MrpFontMono 1
    ${EndIf}
FunctionEnd
Function ${UN}MrpShowNext
    StrCpy $R9 "next"
    Call ${UN}MrpButtons
    Call ${UN}MrpPageShow
FunctionEnd
!macroend
!insertmacro MRP_FUNCTIONS ""
!insertmacro MRP_FUNCTIONS "un."

Function MrpShowInstall
    StrCpy $R9 "install"
    Call MrpButtons
    Call MrpPageShow
FunctionEnd
Function un.MrpShowUninstall
    StrCpy $R9 "uninstall"
    Call un.MrpButtons
    Call un.MrpPageShow
FunctionEnd
Function un.MrpShowClose
    StrCpy $R9 "close"
    Call un.MrpButtons
    Call un.MrpPageShow
FunctionEnd

; Welcome / Finish: side picture over the full (taller) page height, Anton title
!macro MRP_FULLPAGE PAGE IMAGE BITMAP TITLE
    System::Call '*(i,i,i,i) p .r1'
    System::Call 'user32::GetClientRect(p ${PAGE}, p r1)'
    System::Call '*$1(i,i,i,i .r3)'
    System::Call 'user32::GetClientRect(p ${IMAGE}, p r1)'
    System::Call '*$1(i,i,i .r2,i)'
    System::Free $1
    System::Call 'user32::SetWindowPos(p ${IMAGE}, p 0, i 0, i 0, i r2, i r3, i 0x16)'
    ${NSD_FreeImage} ${BITMAP}
    ${NSD_SetStretchedImage} ${IMAGE} "$PLUGINSDIR\modern-wizard.bmp" ${BITMAP}
    SendMessage ${TITLE} ${WM_SETFONT} $MrpFontBig 1
!macroend

Function MrpWelcomeShow
    StrCpy $R9 "next"
    Call MrpButtons
    !insertmacro MRP_FULLPAGE $mui.WelcomePage $mui.WelcomePage.Image $mui.WelcomePage.Image.Bitmap $mui.WelcomePage.Title
    SetCtlColors $mui.WelcomePage.Text ${MRP_SUB} ${MRP_BG}
FunctionEnd

Function MrpFinishShow
    StrCpy $R9 "finish"
    Call MrpButtons
    !insertmacro MRP_FULLPAGE $mui.FinishPage $mui.FinishPage.Image $mui.FinishPage.Image.Bitmap $mui.FinishPage.Title
    SetCtlColors $mui.FinishPage.Text ${MRP_SUB} ${MRP_BG}
    ; themed check boxes ignore text colours (black text on dark), so use the classic look
    !insertmacro MRP_PLAIN_CONTROL $mui.FinishPage.Run
    SetCtlColors $mui.FinishPage.Run ${MRP_TEXT} ${MRP_BG}
FunctionEnd

; Find GTA San Andreas so most people can just click Next
Function .onInit
    !insertmacro MRP_LOAD_FONTS
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

Function un.onInit
    !insertmacro MRP_LOAD_FONTS
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
    ; Browser languages: the build ships English only, so drop the 52 languages an older install left behind
    RMDir /r "$INSTDIR\MTA\cef\locales"
    ; Whole client folder from the CI build (Launcher.exe, MTA, mods, skins, ...). Server not included.
    File /r "${SRC_DIR}/*.*"

    ; The game writes settings, logs and downloaded server files into its own folder,
    ; so normal users (S-1-5-32-545) must be able to change it, as the stock MTA installer does.
    nsExec::ExecToLog 'icacls "$INSTDIR" /grant *S-1-5-32-545:(OI)(CI)M /T /C /Q'

    WriteRegStr HKLM "${REG_COMMON_KEY}" "GTA:SA Path" "$GTA_DIR"
    WriteRegStr HKLM "${REG_VER_KEY}" "Last Install Location" "$INSTDIR"

    Delete "$INSTDIR\Uninstall MAJOR RP.exe"   ; old name from the first installer build
    Delete "$INSTDIR\MAJOR RP.exe"             ; game exe before it was renamed to Launcher.exe

    ; Leftovers from older builds (installing over an old version does not remove them by itself)
    Delete "$INSTDIR\MTA\netc_d.dll"
    Delete "$INSTDIR\MTA\cgui\images\chat.png"
    Delete "$INSTDIR\MTA\cgui\images\chatinput.png"
    RMDir /r "$INSTDIR\MTA\cgui\images\transferset"
    Delete "$INSTDIR\MTA\locale\en_US\menu_browse_servers.png"
    Delete "$INSTDIR\MTA\locale\en_US\menu_host_game.png"
    Delete "$INSTDIR\MTA\locale\en_US\menu_map_editor.png"
    Delete "$INSTDIR\MTA\locale\en_US\menu_quick_connect.png"
    Delete "$INSTDIR\MTA\locale\en_US\menu_about.png"            ; ABOUT button removed (Danex, 2026-11-28)
    Delete "$INSTDIR\MTA\locale\en_US\menu_about_hover.png"
    Delete "$INSTDIR\MTA\locale\en_US\menu_connecting_hover.png"
    Delete "$INSTDIR\MTA\cgui\images\majorrp\tb3_cancel.png"   ; download box v5: no cancel button / stat boxes
    Delete "$INSTDIR\MTA\cgui\images\majorrp\tb3_stat.png"
    WriteUninstaller "$INSTDIR\Uninstall.exe"
    WriteRegStr HKLM "${UNINST_KEY}" "DisplayName" "${PRODUCT_NAME}"
    WriteRegStr HKLM "${UNINST_KEY}" "DisplayIcon" "$INSTDIR\${CLIENT_EXE}"
    WriteRegStr HKLM "${UNINST_KEY}" "DisplayVersion" "${VERSION}"
    WriteRegStr HKLM "${UNINST_KEY}" "Publisher" "MAJOR RP"
    WriteRegStr HKLM "${UNINST_KEY}" "InstallLocation" "$INSTDIR"
    WriteRegStr HKLM "${UNINST_KEY}" "UninstallString" '"$INSTDIR\Uninstall.exe"'
    WriteRegDWORD HKLM "${UNINST_KEY}" "NoModify" 1
    WriteRegDWORD HKLM "${UNINST_KEY}" "NoRepair" 1
    ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
    WriteRegDWORD HKLM "${UNINST_KEY}" "EstimatedSize" $0
SectionEnd

Section "Desktop shortcut" SecDesktop
    ; On the installing user's own desktop (also when it is moved to OneDrive). The all-users
    ; desktop (C:\Users\Public\Desktop) didn't show the shortcut on some PCs.
    SetShellVarContext current
    CreateShortcut "$DESKTOP\MAJOR RP.lnk" "$INSTDIR\${CLIENT_EXE}" "" "$INSTDIR\${CLIENT_EXE}" 0
    ${IfNot} ${FileExists} "$DESKTOP\MAJOR RP.lnk"
        DetailPrint "Could not create the desktop shortcut in $DESKTOP"
    ${EndIf}
    ; Tell Explorer to redraw the desktop so the new icon appears right away
    System::Call 'shell32::SHChangeNotify(i 0x08000000, i 0, p 0, p 0)'
SectionEnd

Section "Start menu shortcuts" SecStartMenu
    SetShellVarContext all
    CreateDirectory "$SMPROGRAMS\MAJOR RP"
    CreateShortcut "$SMPROGRAMS\MAJOR RP\MAJOR RP.lnk" "$INSTDIR\${CLIENT_EXE}" "" "$INSTDIR\${CLIENT_EXE}" 0
    Delete "$SMPROGRAMS\MAJOR RP\Uninstall MAJOR RP.lnk"   ; old name from the first installer build
    CreateShortcut "$SMPROGRAMS\MAJOR RP\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
SectionEnd

; Off by default: GTA SA players almost always have DirectX 9 already, and this needs internet.
Section /o "DirectX 9 runtime (only if the game will not start)" SecDirectX
    ${If} ${FileExists} "$INSTDIR\redist\dxwebsetup.exe"
        DetailPrint "Installing DirectX 9 runtime..."
        ExecWait '"$INSTDIR\redist\dxwebsetup.exe" /Q'
    ${EndIf}
SectionEnd

Section "Uninstall"
    ; Shortcuts may be in the current user's or the all-users folders (first build used all-users)
    SetShellVarContext current
    Delete "$DESKTOP\MAJOR RP.lnk"
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
    System::Call 'shell32::SHChangeNotify(i 0x08000000, i 0, p 0, p 0)'
SectionEnd
