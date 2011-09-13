RequestExecutionLevel admin
!include "wininstall\FileAssociation.nsh"
SetCompressor /SOLID /FINAL lzma

!define PRODUCT_NAME "QupZilla"
!define /date PRODUCT_VERSION "1.0.0-b4"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\qupzilla.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

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
!insertmacro MUI_LANGUAGE "Dutch"
!insertmacro MUI_LANGUAGE "French"
!insertmacro MUI_LANGUAGE "German"
!insertmacro MUI_LANGUAGE "Korean"
!insertmacro MUI_LANGUAGE "Russian"
!insertmacro MUI_LANGUAGE "Spanish"
!insertmacro MUI_LANGUAGE "Swedish"
!insertmacro MUI_LANGUAGE "Tradchinese"
!insertmacro MUI_LANGUAGE "Simpchinese"

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
  KillProcDLL::KillProc "qupzilla.exe"
  Sleep 100
  SetOverwrite on

  SetOutPath "$INSTDIR"
  File "COPYRIGHT.txt"
  File "qupzilla.exe"
  File "libeay32.dll"
  File "ssleay32.dll"
  File "msvcp100.dll"
  File "msvcr100.dll"
  File "phonon4.dll"
  File "QtCore4.dll"
  File "QtGui4.dll"
  File "QtNetwork4.dll"
  File "QtSql4.dll"
  File "QtWebKit4.dll"

  SetOutPath "$INSTDIR\data\default\profiles"
  File "data\default\profiles\profiles.ini"
  
  SetOutPath "$INSTDIR\data\default\profiles\default"
  File "data\default\profiles\default\background.png"
  File "data\default\profiles\default\browsedata.db"

  SetOutPath "$INSTDIR\imageformats"
  File "imageformats\qico4.dll"
  File "imageformats\qsvg4.dll"
  File "imageformats\qgif4.dll"
  File "imageformats\qjpeg4.dll"
  File "imageformats\qtiff4.dll"
  File "imageformats\qmng4.dll"

  SetOutPath "$INSTDIR\sqldrivers"
  File "sqldrivers\qsqlite4.dll"
  File "sqldrivers\qsqlodbc4.dll"

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

  Section Old SecOld
  SetOutPath "$INSTDIR\themes\default"
  File "themes\default\*"
  SetOutPath "$INSTDIR\themes\default\images"
  File "themes\default\images\*"
  SectionEnd

SectionGroupEnd

SectionGroup $(TITLE_SecTranslations) SecTranslations
  Section $(TITLE_SecEnglish) SecEnglish
  SectionIn RO  
  SectionEnd

  Section $(TITLE_SecCzech) SecCzech
  SetOutPath "$INSTDIR\locale"
  File "locale\cs_CZ.qm"
  File "locale\qt_cs.qm"
  SectionEnd
  
  Section $(TITLE_SecSlovak) SecSlovak
  SetOutPath "$INSTDIR\locale"
  File "locale\sk_SK.qm"
  File "locale\qt_sk.qm"
  SectionEnd

SectionGroupEnd

SectionGroup $(TITLE_SecPlugins) SecPlugins
  Section $(TITLE_SecExamplePlugin) SecExamplePlugin
  SetOutPath "$INSTDIR\plugins"
  File "plugins\ExamplePlugin.dll"
  SectionEnd
SectionGroupEnd


Section $(TITLE_SecExtensions) SecExtensions
  SetOutPath "$INSTDIR"
  ${registerExtension} "$INSTDIR\qupzilla.exe" ".htm" $(FILE_Htm)
  ${registerExtension} "$INSTDIR\qupzilla.exe" ".html" $(FILE_Html)
SectionEnd

Section "-StartMenu"
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
  !insertmacro MUI_DESCRIPTION_TEXT ${SecEnglish} $(DESC_SecEnglish)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecCzech} $(DESC_SecCzech)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecSlovak} $(DESC_SecSlovak)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecPlugins} $(DESC_SecPlugins)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecExamplePlugin} $(DESC_SecExamplePlugin)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecDesktop} $(DESC_SecDesktop)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecExtensions} $(DESC_SecExtensions)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecThemes} $(DESC_SecThemes)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

Section "-Uninstaller"
  WriteUninstaller "$INSTDIR\uninstall.exe"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\qupzilla.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\Uninstall.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\qupzilla.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
SectionEnd

Section Uninstall
  KillProcDLL::KillProc "qupzilla.exe"
  Sleep 100

  SetShellVarContext all
  Delete "$DESKTOP\QupZilla.lnk"
  RMDir /r "$INSTDIR"
  RMDir /r "$SMPROGRAMS\QupZilla"
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  ${unregisterExtension} ".htm" $(FILE_Htm)
  ${unregisterExtension} ".html" $(FILE_Html)
SectionEnd

BrandingText "${PRODUCT_NAME} ${PRODUCT_VERSION} Installer"

Function .onInit
	;Language selection dialog

	Push ""
    Push ${LANG_ENGLISH}
	Push English
	Push ${LANG_CZECH}
	Push Czech
	Push ${LANG_DUTCH}
	Push Dutch
	Push ${LANG_FRENCH}
	Push French
	Push ${LANG_GERMAN}
	Push German
	Push ${LANG_KOREAN}
	Push Korean
	Push ${LANG_RUSSIAN}
	Push Russian
	Push ${LANG_SPANISH}
	Push Spanish
	Push ${LANG_SWEDISH}
	Push Swedish
	Push ${LANG_TRADCHINESE}
	Push "Traditional Chinese"
	Push ${LANG_SIMPCHINESE}
	Push "Simplified Chinese"
	Push ${LANG_SLOVAK}
	Push Slovak
	Push A ; A means auto count languages
	       ; for the auto count to work the first empty push (Push "") must remain
	LangDLL::LangDialog "Installer Language" "Please select the language of the installer"

	Pop $LANGUAGE
	StrCmp $LANGUAGE "cancel" 0 +2
		Abort
FunctionEnd
