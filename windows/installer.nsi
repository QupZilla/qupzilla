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
!define /date PRODUCT_VERSION "1.8.6"
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
  File "libhunspell.dll"
  File "libeay32.dll"
  File "ssleay32.dll"
  File "sqlite3.dll"
  File "Microsoft.VC90.CRT.manifest"
  File "msvcm90.dll"
  File "msvcp90.dll"
  File "msvcr90.dll"
  File "QtCore4.dll"
  File "QtGui4.dll"
  File "QtNetwork4.dll"
  File "QtScript4.dll"
  File "QtSql4.dll"
  File "QtWebKit4.dll"
  File "QtXmlPatterns4.dll"

  SetOutPath "$INSTDIR\imageformats"
  File "imageformats\qico4.dll"
  File "imageformats\qsvg4.dll"
  File "imageformats\qgif4.dll"
  File "imageformats\qjpeg4.dll"
  File "imageformats\qmng4.dll"
  File "imageformats\qtiff4.dll"
  File "imageformats\qtga4.dll"

  SetOutPath "$INSTDIR\codecs"
  File "codecs\qcncodecs4.dll"
  File "codecs\qjpcodecs4.dll"
  File "codecs\qkrcodecs4.dll"
  File "codecs\qtwcodecs4.dll"

  SetOutPath "$INSTDIR\iconengines"
  File "iconengines\qsvgicon4.dll"

  SetOutPath "$INSTDIR\hunspell\doc"
  File "wininstall\hunspell\doc\*"

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

  Section Old SecOld
  SetOutPath "$INSTDIR\themes\default"
  File "themes\default\*"
  SetOutPath "$INSTDIR\themes\default\images"
  File "themes\default\images\*"
  SectionEnd

SectionGroupEnd

SectionGroup $(TITLE_SecTranslations) SecTranslations

  Section "English"
  SectionIn RO
  SetOutPath "$INSTDIR\hunspell"
  File "wininstall\hunspell\en_US.aff"
  File "wininstall\hunspell\en_US.dic"
  SectionEnd

  Section "Arabic"
  SetOutPath "$INSTDIR\locale"
  File "locale\ar_SA.qm"
  File "locale\qt_ar.qm"
  SectionEnd

  Section "Bulgarian"
  SetOutPath "$INSTDIR\locale"
  File "locale\bg_BG.qm"
  SectionEnd

  Section "Catalan"
  SetOutPath "$INSTDIR\locale"
  File "locale\ca_ES.qm"
  File "locale\qt_ca.qm"
  SetOutPath "$INSTDIR\hunspell"
  File "wininstall\hunspell\ca_ES.aff"
  File "wininstall\hunspell\ca_ES.dic"
  SectionEnd

  Section "Czech"
  SetOutPath "$INSTDIR\locale"
  File "locale\cs_CZ.qm"
  File "locale\qt_cs.qm"
  SetOutPath "$INSTDIR\hunspell"
  File "wininstall\hunspell\cs_CZ.aff"
  File "wininstall\hunspell\cs_CZ.dic"
  SectionEnd

  Section "German"
  SetOutPath "$INSTDIR\locale"
  File "locale\de_DE.qm"
  File "locale\qt_de.qm"
  SetOutPath "$INSTDIR\hunspell"
  File "wininstall\hunspell\de_DE.aff"
  File "wininstall\hunspell\de_DE.dic"
  SectionEnd

  Section "Greek"
  SetOutPath "$INSTDIR\locale"
  File "locale\el_GR.qm"
  File "locale\qt_el.qm"
  SetOutPath "$INSTDIR\hunspell"
  File "wininstall\hunspell\el_GR.aff"
  File "wininstall\hunspell\el_GR.dic"
  SectionEnd

  Section "Spanish"
  SetOutPath "$INSTDIR\locale"
  File "locale\es_ES.qm"
  File "locale\es_VE.qm"
  File "locale\qt_es.qm"
  SetOutPath "$INSTDIR\hunspell"
  File "wininstall\hunspell\es_ES.aff"
  File "wininstall\hunspell\es_ES.dic"
  File "wininstall\hunspell\es_VE.aff"
  File "wininstall\hunspell\es_VE.dic"
  SectionEnd

  Section "Basque"
  SetOutPath "$INSTDIR\locale"
  File "locale\eu_ES.qm"
  File "locale\qt_eu.qm"
  SectionEnd

  Section "Persian"
  SetOutPath "$INSTDIR\locale"
  File "locale\fa_IR.qm"
  File "locale\qt_fa.qm"
  SetOutPath "$INSTDIR\hunspell"
  File "wininstall\hunspell\fa_IR.aff"
  File "wininstall\hunspell\fa_IR.dic"
  SectionEnd

  Section "French"
  SetOutPath "$INSTDIR\locale"
  File "locale\fr_FR.qm"
  File "locale\qt_fr.qm"
  SetOutPath "$INSTDIR\hunspell"
  File "wininstall\hunspell\fr_FR.aff"
  File "wininstall\hunspell\fr_FR.dic"
  SectionEnd

  Section "Galician"
  SetOutPath "$INSTDIR\locale"
  File "locale\gl_ES.qm"
  File "locale\qt_gl.qm"
  SectionEnd

  Section "Hebrew"
  SetOutPath "$INSTDIR\locale"
  File "locale\he_IL.qm"
  File "locale\qt_he.qm"
  SectionEnd

  Section "Hungarian"
  SetOutPath "$INSTDIR\locale"
  File "locale\hu_HU.qm"
  File "locale\qt_hu.qm"
  SetOutPath "$INSTDIR\hunspell"
  File "wininstall\hunspell\hu_HU.aff"
  File "wininstall\hunspell\hu_HU.dic"
  SectionEnd

  Section "Indonesian"
  SetOutPath "$INSTDIR\locale"
  File "locale\id_ID.qm"
  SetOutPath "$INSTDIR\hunspell"
  File "wininstall\hunspell\id_ID.aff"
  File "wininstall\hunspell\id_ID.dic"
  SectionEnd

  Section "Italian"
  SetOutPath "$INSTDIR\locale"
  File "locale\it_IT.qm"
  File "locale\qt_it.qm"
  SetOutPath "$INSTDIR\hunspell"
  File "wininstall\hunspell\it_IT.aff"
  File "wininstall\hunspell\it_IT.dic"
  SectionEnd

  Section "Japanese"
  SetOutPath "$INSTDIR\locale"
  File "locale\ja_JP.qm"
  File "locale\qt_ja.qm"
  SectionEnd

  Section "Georgian"
  SetOutPath "$INSTDIR\locale"
  File "locale\ka_GE.qm"
  SectionEnd

  Section "Ganda"
  SetOutPath "$INSTDIR\locale"
  File "locale\lg.qm"
  SectionEnd

  Section "Dutch"
  SetOutPath "$INSTDIR\locale"
  File "locale\nl_NL.qm"
  File "locale\qt_nl.qm"
  SetOutPath "$INSTDIR\hunspell"
  File "wininstall\hunspell\nl_NL.aff"
  File "wininstall\hunspell\nl_NL.dic"
  SectionEnd

  Section "N'ko"
  SetOutPath "$INSTDIR\locale"
  File "locale\nqo.qm"
  SectionEnd

  Section "Polish"
  SetOutPath "$INSTDIR\locale"
  File "locale\pl_PL.qm"
  File "locale\qt_pl.qm"
  SectionEnd

  Section "Portuguese"
  SetOutPath "$INSTDIR\locale"
  File "locale\pt_PT.qm"
  File "locale\pt_BR.qm"
  File "locale\qt_pt.qm"
  SetOutPath "$INSTDIR\hunspell"
  File "wininstall\hunspell\pt_PT.aff"
  File "wininstall\hunspell\pt_PT.dic"
  File "wininstall\hunspell\pt_BR.aff"
  File "wininstall\hunspell\pt_BR.dic"
  SectionEnd

  Section "Romanian"
  SetOutPath "$INSTDIR\locale"
  File "locale\ro_RO.qm"
  SetOutPath "$INSTDIR\hunspell"
  File "wininstall\hunspell\ro_RO.aff"
  File "wininstall\hunspell\ro_RO.dic"
  SectionEnd

  Section "Russian"
  SetOutPath "$INSTDIR\locale"
  File "locale\ru_RU.qm"
  File "locale\qt_ru.qm"
  SetOutPath "$INSTDIR\hunspell"
  File "wininstall\hunspell\ru_RU.aff"
  File "wininstall\hunspell\ru_RU.dic"
  SectionEnd

  Section "Slovak"
  SetOutPath "$INSTDIR\locale"
  File "locale\sk_SK.qm"
  File "locale\qt_sk.qm"
  SetOutPath "$INSTDIR\hunspell"
  File "wininstall\hunspell\sk_SK.aff"
  File "wininstall\hunspell\sk_SK.dic"
  SectionEnd

  Section "Serbian"
  SetOutPath "$INSTDIR\locale"
  File "locale\sr_BA.qm"
  File "locale\sr_RS.qm"
  File "locale\qt_sr_BA.qm"
  File "locale\qt_sr_RS.qm"
  File "locale\sr_BA@latin.qm"
  File "locale\sr_RS@latin.qm"
  File "locale\qt_sr_BA@latin.qm"
  File "locale\qt_sr_RS@latin.qm"
  SetOutPath "$INSTDIR\hunspell"
  File "wininstall\hunspell\sr.aff"
  File "wininstall\hunspell\sr.dic"
  File "wininstall\hunspell\sh.aff"
  File "wininstall\hunspell\sh.dic"
  SectionEnd

  Section "Swedish"
  SetOutPath "$INSTDIR\locale"
  File "locale\sv_SE.qm"
  File "locale\qt_sv.qm"
  SectionEnd

  Section "Ukrainian"
  SetOutPath "$INSTDIR\locale"
  File "locale\uk_UA.qm"
  File "locale\qt_uk.qm"
  SetOutPath "$INSTDIR\hunspell"
  File "wininstall\hunspell\uk_UA.aff"
  File "wininstall\hunspell\uk_UA.dic"
  SectionEnd

  Section "Uzbek"
  SetOutPath "$INSTDIR\locale"
  File "locale\uz@Latn.qm"
  SectionEnd

SectionGroupEnd

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

Section -MSVC
  InitPluginsDir
  SetOutPath $PLUGINSDIR
  File "wininstall\vcredist_x86.exe"
  DetailPrint "Installing Visual C++ 2008 Libraries"
  ExecWait '"$PLUGINSDIR\vcredist_x86.exe" /passive /Q:a /c:"msiexec /qb /i vcredist.msi"'
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
