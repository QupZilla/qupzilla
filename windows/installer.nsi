; QupZilla Windows Installer NSIS Script
; Copyright (C) 2010-2014  David Rosca <nowrep@gmail.com>
;               2012-2013  S. Razi Alavizadeh <s.r.alavizadeh@gmail.com>
;
; For compiling this script you need following plugins:
; FindProcDLL_plug-in, KillProcDLL_plug-in and 'AllAssociation.nsh' needs
; Registry_plug-in, Application_Association_Registration_plug-in
; Unicode version of them can be downloaded from:
; http://sourceforge.net/projects/findkillprocuni/files/bin/
; http://nsis.sourceforge.net/Application_Association_Registration_plug-in
; http://nsis.sourceforge.net/Registry_plug-in

RequestExecutionLevel admin

; WinVer.nsh was added in the same release that RequestExecutionLevel so check
; if ___WINVER__NSH___ is defined to determine if RequestExecutionLevel is
; available.
!include /NONFATAL WinVer.nsh

!addplugindir "wininstall\"

!include "FileFunc.nsh"
!include "wininstall\AllAssociation.nsh"
SetCompressor /SOLID /FINAL lzma

!define PRODUCT_NAME "QupZilla"
!define /date PRODUCT_VERSION "2.1.2"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\qupzilla.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"
!define PRODUCT_CAPABILITIES_KEY "Software\${PRODUCT_NAME}\Capabilities"

!include "MUI.nsh"
!define MUI_ABORTWARNING
!define MUI_ICON "wininstall\install.ico"
!define MUI_UNICON "wininstall\uninstall.ico"
!define MUI_WELCOMEFINISHPAGE_BITMAP "wininstall\welcome.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "wininstall\welcome.bmp"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE COPYRIGHT.txt
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_RUN "$INSTDIR\qupzilla.exe"
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Czech"
!insertmacro MUI_LANGUAGE "Slovak"
!insertmacro MUI_LANGUAGE "German"
!insertmacro MUI_LANGUAGE "Dutch"
!insertmacro MUI_LANGUAGE "Portuguese"
!insertmacro MUI_LANGUAGE "Greek"
!insertmacro MUI_LANGUAGE "French"
!insertmacro MUI_LANGUAGE "Italian"
!insertmacro MUI_LANGUAGE "Romanian"
!insertmacro MUI_LANGUAGE "Tradchinese"
!insertmacro MUI_LANGUAGE "Simpchinese"
!insertmacro MUI_LANGUAGE "Indonesian"
!insertmacro MUI_LANGUAGE "Georgian"
!insertmacro MUI_LANGUAGE "Japanese"
!insertmacro MUI_LANGUAGE "Swedish"
!insertmacro MUI_LANGUAGE "Polish"
!insertmacro MUI_LANGUAGE "Ukrainian"
!insertmacro MUI_LANGUAGE "Catalan"
!insertmacro MUI_LANGUAGE "Serbian"
!insertmacro MUI_LANGUAGE "SerbianLatin"
!insertmacro MUI_LANGUAGE "Farsi"
!insertmacro MUI_LANGUAGE "Hebrew"
!insertmacro MUI_LANGUAGE "Spanish"
!insertmacro MUI_LANGUAGE "Arabic"
!insertmacro MUI_LANGUAGE "Basque"
!insertmacro MUI_LANGUAGE "Danish"

!insertmacro MUI_RESERVEFILE_LANGDLL

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "${PRODUCT_NAME} ${PRODUCT_VERSION} Installer.exe"
InstallDir "$PROGRAMFILES\${PRODUCT_NAME}\"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails show
ShowUnInstDetails show

!include "wininstall\languages.nsh"

Section !$(TITLE_SecMain) SecMain
  SectionIn RO
  FindProcDLL::FindProc "qupzilla.exe"
  IntCmp $R0 1 0 notRunning
  MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION "$(MSG_RunningInstance)" /SD IDOK IDCANCEL AbortInstallation
    KillProcDLL::KillProc "qupzilla.exe"
	Sleep 100
	Goto notRunning
AbortInstallation:
  Abort "$(MSG_InstallationCanceled)"

notRunning:
  SetOverwrite on

  SetOutPath "$INSTDIR"
  File "COPYRIGHT.txt"
  File "qupzilla.exe"
  File "qupzilla.dll"
  File "libeay32.dll"
  File "ssleay32.dll"
  File "qt.conf"
  File "concrt140.dll"
  File "msvcp140.dll"
  File "vccorlib140.dll"
  File "vcruntime140.dll"
  File "icudt54.dll"
  File "icuin54.dll"
  File "icuuc54.dll"
  File "libEGL.dll"
  File "libGLESv2.dll"
  File "opengl32sw.dll"
  File "D3Dcompiler_47.dll"
  File "Qt5Core.dll"
  File "Qt5Gui.dll"
  File "Qt5Network.dll"
  File "Qt5Positioning.dll"
  File "Qt5PrintSupport.dll"
  File "Qt5Qml.dll"
  File "Qt5Quick.dll"
  File "Qt5QuickWidgets.dll"
  File "Qt5Sql.dll"
  File "Qt5Svg.dll"
  File "Qt5WinExtras.dll"
  File "Qt5WebEngine.dll"
  File "Qt5WebEngineCore.dll"
  File "Qt5WebEngineWidgets.dll"
  File "Qt5WebChannel.dll"
  File "Qt5Widgets.dll"
  File "QtWebEngineProcess.exe"

  SetOutPath "$INSTDIR\iconengines"
  File "iconengines\qsvgicon.dll"

  SetOutPath "$INSTDIR\imageformats"
  File "imageformats\*.dll"

  SetOutPath "$INSTDIR\platforms"
  File "platforms\qwindows.dll"

  SetOutPath "$INSTDIR\printsupport"
  File "printsupport\windowsprintersupport.dll"

  SetOutPath "$INSTDIR\qml\QtQuick.2"
  File "qml\QtQuick.2\*"

  SetOutPath "$INSTDIR\qml\QtWebEngine"
  File "qml\QtWebEngine\*"

  SetOutPath "$INSTDIR\resources"
  File "resources\*"

  SetOutPath "$INSTDIR\sqldrivers"
  File "sqldrivers\qsqlite.dll"

  SetOutPath "$INSTDIR\translations\qtwebengine_locales"
  File "translations\qtwebengine_locales\*"

  SetOutPath "$INSTDIR\qtwebengine_dictionaries\doc"
  File "qtwebengine_dictionaries\doc\README-en-US.txt"

  SetOutPath "$INSTDIR\qtwebengine_dictionaries"
  File "qtwebengine_dictionaries\en_US.bdic"

  call RegisterCapabilities
SectionEnd

SectionGroup $(TITLE_SecThemes) SecThemes

  Section Default SecDefault
  SectionIn RO
  SetOutPath "$INSTDIR\themes\windows"
  File "themes\windows\*"
  SetOutPath "$INSTDIR\themes\windows\images"
  File "themes\windows\images\*"
  SectionEnd

  Section Chrome SecChrome
  SetOutPath "$INSTDIR\themes\chrome"
  File "themes\chrome\*"
  SetOutPath "$INSTDIR\themes\chrome\images"
  File "themes\chrome\images\*"
  SectionEnd

  Section Mac SecMac
  SetOutPath "$INSTDIR\themes\mac"
  File "themes\mac\*"
  SetOutPath "$INSTDIR\themes\mac\images"
  File "themes\mac\images\*"
  SectionEnd

  Section Breathe SecBreathe
  SetOutPath "$INSTDIR\themes\breathe"
  File "themes\breathe\*"
  SetOutPath "$INSTDIR\themes\breathe\images"
  File "themes\breathe\images\*"
  SectionEnd
SectionGroupEnd

Section $(TITLE_SecTranslations) SecTranslations
  SetOutPath "$INSTDIR\locale"
  File "locale\*.qm"
  SetOutPath "$INSTDIR\qtwebengine_dictionaries\doc"
  File "qtwebengine_dictionaries\doc\*"
  SetOutPath "$INSTDIR\qtwebengine_dictionaries"
  File "qtwebengine_dictionaries\*.bdic"
SectionEnd

Section $(TITLE_SecPlugins) SecPlugins
  SetOutPath "$INSTDIR\plugins"
  File "plugins\*.dll"
SectionEnd

SectionGroup $(TITLE_SecSetASDefault) SecSetASDefault
	Section $(TITLE_SecExtensions) SecExtensions
	  SetOutPath "$INSTDIR"
	  ${RegisterAssociation} ".htm" "$INSTDIR\qupzilla.exe" "QupZilla.HTM" $(FILE_Htm) "$INSTDIR\qupzilla.exe,1" "file"
	  ${RegisterAssociation} ".html" "$INSTDIR\qupzilla.exe" "QupZilla.HTML" $(FILE_Html) "$INSTDIR\qupzilla.exe,1" "file"
	  ${UpdateSystemIcons}
	SectionEnd

    Section $(TITLE_SecProtocols) SecProtocols
	  ${RegisterAssociation} "http" "$INSTDIR\qupzilla.exe" "QupZilla.HTTP" "URL:HyperText Transfer Protocol" "$INSTDIR\qupzilla.exe,0" "protocol"
	  ${RegisterAssociation} "https" "$INSTDIR\qupzilla.exe" "QupZilla.HTTPS" "URL:HyperText Transfer Protocol with Privacy" "$INSTDIR\qupzilla.exe,0" "protocol"
	  ${RegisterAssociation} "ftp" "$INSTDIR\qupzilla.exe" "QupZilla.FTP" "URL:File Transfer Protocol" "$INSTDIR\qupzilla.exe,0" "protocol"
	  ${UpdateSystemIcons}
    SectionEnd
SectionGroupEnd

Section -StartMenu
  SetOutPath "$INSTDIR"
  SetShellVarContext all
  CreateDirectory "$SMPROGRAMS\QupZilla"
  CreateShortCut "$SMPROGRAMS\QupZilla\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
  CreateShortCut "$SMPROGRAMS\QupZilla\QupZilla.lnk" "$INSTDIR\qupzilla.exe"
  CreateShortCut "$SMPROGRAMS\QupZilla\License.lnk" "$INSTDIR\COPYRIGHT.txt"
SectionEnd

Section $(TITLE_SecDesktop) SecDesktop
  SetOutPath "$INSTDIR"
  CreateShortCut "$DESKTOP\QupZilla.lnk" "$INSTDIR\qupzilla.exe" ""
SectionEnd

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecMain} $(DESC_SecMain)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecTranslations} $(DESC_SecTranslations)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecPlugins} $(DESC_SecPlugins)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecDesktop} $(DESC_SecDesktop)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecExtensions} $(DESC_SecExtensions)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecThemes} $(DESC_SecThemes)

  !insertmacro MUI_DESCRIPTION_TEXT ${SecSetASDefault} $(DESC_SecSetASDefault)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecProtocols} $(DESC_SecProtocols)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

Section -Uninstaller
  WriteUninstaller "$INSTDIR\uninstall.exe"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\qupzilla.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\Uninstall.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\qupzilla.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "QupZilla Team"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "HelpLink" "https://github.com/QupZilla/qupzilla/wiki"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "InstallLocation" "$INSTDIR"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "InstallSource" "$EXEDIR"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "http://www.qupzilla.com"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLUpdateInfo" "http://blog.qupzilla.com/"
  ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
  WriteRegDWORD ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "EstimatedSize" "$0"
SectionEnd

Section Uninstall
  FindProcDLL::FindProc "qupzilla.exe"
  IntCmp $R0 1 0 notRunning
  MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION "$(MSG_RunningInstance)" /SD IDOK IDCANCEL AbortInstallation
    KillProcDLL::KillProc "qupzilla.exe"
	Sleep 100
	Goto notRunning
AbortInstallation:
  Abort "$(MSG_InstallationCanceled)"

notRunning:
  SetShellVarContext all
  Delete "$DESKTOP\QupZilla.lnk"
  RMDir /r "$INSTDIR"
  RMDir /r "$SMPROGRAMS\QupZilla"
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"

  DeleteRegKey HKLM "Software\${PRODUCT_NAME}"
  DeleteRegValue HKLM "SOFTWARE\RegisteredApplications" "${PRODUCT_NAME}"

  ${UnRegisterAssociation} ".htm" "QupZilla.HTM" "$INSTDIR\qupzilla.exe" "file"
  ${UnRegisterAssociation} ".html" "QupZilla.HTML" "$INSTDIR\qupzilla.exe" "file"
  ${UnRegisterAssociation} "http" "QupZilla.HTTP" "$INSTDIR\qupzilla.exe" "protocol"
  ${UnRegisterAssociation} "https" "QupZilla.HTTPS" "$INSTDIR\qupzilla.exe" "protocol"
  ${UnRegisterAssociation} "ftp" "QupZilla.FTP" "$INSTDIR\qupzilla.exe" "protocol"
  ${UpdateSystemIcons}
SectionEnd

BrandingText "${PRODUCT_NAME} ${PRODUCT_VERSION} Installer"

Function .onInit
        ;Prevent Multiple Instances
        System::Call 'kernel32::CreateMutexA(i 0, i 0, t "QupZillaInstaller-4ECB4694-2C39-4f93-9122-A986344C4E7B") i .r1 ?e'
        Pop $R0
        StrCmp $R0 0 +3
          MessageBox MB_OK|MB_ICONEXCLAMATION "QupZilla installer is already running!" /SD IDOK
        Abort

        ;Language selection dialog¨
        ;Return when running silent instalation

        IfSilent 0 +2
          return

        Push ""
        Push ${LANG_ENGLISH}
        Push English
        Push ${LANG_CZECH}
        Push Czech
        Push ${LANG_SLOVAK}
        Push Slovak
        Push ${LANG_GERMAN}
        Push German
        Push ${LANG_DUTCH}
        Push Dutch
        Push ${LANG_PORTUGUESE}
        Push Portuguese
        Push ${LANG_GREEK}
        Push Greek
        Push ${LANG_FRENCH}
        Push French
        Push ${LANG_ITALIAN}
        Push Italian
        Push ${LANG_ROMANIAN}
        Push Romanian
        Push ${LANG_TRADCHINESE}
        Push TraditionalChinese
        Push ${LANG_SIMPCHINESE}
        Push SimplifiedChinese
        Push ${LANG_INDONESIAN}
        Push Indonesian
        Push ${LANG_GEORGIAN}
        Push Georgian
        Push ${LANG_JAPANESE}
        Push Japanese
        Push ${LANG_SWEDISH}
        Push Swedish
        Push ${LANG_POLISH}
        Push Polish
        Push ${LANG_UKRAINIAN}
        Push Ukrainian
        Push ${LANG_CATALAN}
        Push Catalan
        Push ${LANG_SERBIAN}
        Push Serbian
        Push ${LANG_SERBIANLATIN}
        Push SerbianLatin
        Push ${LANG_FARSI}
        Push Persian
        Push ${LANG_HEBREW}
        Push Hebrew
        Push ${LANG_SPANISH}
        Push Spanish
        Push ${LANG_DANISH}
        Push Danish
        Push A ; A means auto count languages
               ; for the auto count to work the first empty push (Push "") must remain
        LangDLL::LangDialog "Installer Language" "Please select the language of the installer"

        Pop $LANGUAGE
        StrCmp $LANGUAGE "cancel" 0 +2
                Abort
FunctionEnd

Function RegisterCapabilities
	!ifdef ___WINVER__NSH___
		${If} ${AtLeastWinVista}
			; even if we don't associate QupZilla as default for ".htm" and ".html"
			; we need to write these ProgIds for future use!
			;(e.g.: user uses "Default Programs" on Win7 or Vista to set QupZilla as default.)
			${CreateProgId} "QupZilla.HTM" "$INSTDIR\qupzilla.exe" $(FILE_Htm) "$INSTDIR\qupzilla.exe,1"
			${CreateProgId} "QupZilla.HTML" "$INSTDIR\qupzilla.exe" $(FILE_Html) "$INSTDIR\qupzilla.exe,1"
			${CreateProgId} "QupZilla.HTTP" "$INSTDIR\qupzilla.exe" "URL:HyperText Transfer Protocol" "$INSTDIR\qupzilla.exe,0"
			${CreateProgId} "QupZilla.HTTPS" "$INSTDIR\qupzilla.exe" "URL:HyperText Transfer Protocol with Privacy" "$INSTDIR\qupzilla.exe,0"
			${CreateProgId} "QupZilla.FTP" "$INSTDIR\qupzilla.exe" "URL:File Transfer Protocol" "$INSTDIR\qupzilla.exe,0"

			; note: these lines just introduce capabilities of QupZilla to OS and don't change defaults!
			WriteRegStr HKLM "${PRODUCT_CAPABILITIES_KEY}" "ApplicationDescription" "$(PRODUCT_DESC)"
			WriteRegStr HKLM "${PRODUCT_CAPABILITIES_KEY}" "ApplicationIcon" "$INSTDIR\qupzilla.exe,0"
			WriteRegStr HKLM "${PRODUCT_CAPABILITIES_KEY}" "ApplicationName" "${PRODUCT_NAME}"
			WriteRegStr HKLM "${PRODUCT_CAPABILITIES_KEY}\FileAssociations" ".htm" "QupZilla.HTM"
			WriteRegStr HKLM "${PRODUCT_CAPABILITIES_KEY}\FileAssociations" ".html" "QupZilla.HTML"
			WriteRegStr HKLM "${PRODUCT_CAPABILITIES_KEY}\URLAssociations" "http" "QupZilla.HTTP"
			WriteRegStr HKLM "${PRODUCT_CAPABILITIES_KEY}\URLAssociations" "https" "QupZilla.HTTPS"
			WriteRegStr HKLM "${PRODUCT_CAPABILITIES_KEY}\URLAssociations" "ftp" "QupZilla.FTP"
			WriteRegStr HKLM "${PRODUCT_CAPABILITIES_KEY}\Startmenu" "StartMenuInternet" "$INSTDIR\qupzilla.exe"
			WriteRegStr HKLM "SOFTWARE\RegisteredApplications" "${PRODUCT_NAME}" "${PRODUCT_CAPABILITIES_KEY}"
		${EndIf}
	!endif
FunctionEnd
