/*
_____________________________________________________________________________

                       All Association
_____________________________________________________________________________



 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
 ; Date: 2012-Nov-18, S. Razi Alavizadeh, v0.1                            ;
 ; Some Codes are based on code taken from:                               ;
 ; http://nsis.sourceforge.net/File_Association                           ;
 ; Ability to register protocol and extention associations.               ;
 ; It supports old association method and new method by using             ;
 ; IApplicationAssociationRegistration APIs.                              ;
 ; that needed 'AppAssocReg' plugins, downloadable from:                  ;
 ; http://nsis.sourceforge.net/SetVistaDefaultApp_plug-in                 ;
 ; and also add support for set as default backuped association when      ;
 ; uninstalling. This needed 'Registery' plugins.                         ;
 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

 Requirment:
 1. 'AppAssocReg' plugins.
 2. 'Registery' plugins.

 Usage in script:
 1. !include "AllAssociation.nsh"
 2. [Section|Function]
      ${AllAssociationFunction} "Param1" "Param2" "..." $var
    [SectionEnd|FunctionEnd]

 AllAssociationFunction=[RegisterAssociation|UnRegisterAssociation|CreateProgId|UpdateSystemIcons|SetAsDefaultNoAppName]

_____________________________________________________________________________

 ${RegisterAssociation} "[assoc_name]" "[executable]" "[prog_id]" "[description]" "[icon]" "[type]"

"[assoc_name]"     ; assoc_name, which is file format's extention if type is "file" and is protocol's name if type is "protocol".
                   ;
"[executable]"     ; executable which opens the file format or is protocol handler.
                   ;
"[prog_id]"        ; registery internal id for assoc_name, on Vista+ if type is "protocol" this is our registered ProgId for assoc_name and is used for comparison.
                   ;
"[description]"    ; description for the assoc_name. This will be display in Windows Explorer.
                   ;
"[icon]"           ; icon path for this association.
                   ;
"[type]"           ; type of association. "file" for extention and "protocol" for protocol handler.
                   ;

 ${UnRegisterAssociation} "[assoc_name]" "[prog_id]" "[executable]" "[type]"

"[assoc_name]"     ; assoc_name, which is file format's extention if type is "file" and is protocol's name if type is "protocol".
                   ;
"[prog_id]"        ; registery internal id for assoc_name
                   ;
"[executable]"     ; executable which opens the file format or is protocol handler.
                   ;
"[type]"           ; type of association. "file" for extention and "protocol" for protocol handler.
                   ;

 ${CreateProgId} "[prog_id]" "[executable]" "[description]" "[icon]"

"[prog_id]" 	   ; registery internal id for assoc_name
                   ;
"[executable]"     ; executable which opens the file format or is protocol handler.
                   ;
"[description]"    ; description for the assoc_name. This will be display in Windows Explorer.
                   ;
"[icon]"           ; icon path for this prog_id.
                   ;


 ${SetAsDefaultNoAppName} "[prog_id]" "[assoc_name]" "[type]"

"[prog_id]" 	   ; registery internal id for assoc_name
                   ;
"[assoc_name]"     ; assoc_name, which is file format's extention if type is "file" and is protocol's name if type is "protocol".
                   ;
"[type]"           ; type of association. "file" for extention and "protocol" for protocol handler.
                   ;

 ${UpdateSystemIcons} ; it has not arguments and just notifies OS for updating icons for new associations.

_____________________________________________________________________________

                         Macros
_____________________________________________________________________________

 Change log window verbosity (default: 3=no script)

 Example:
 !include "AllAssociation.nsh"
 !insertmacro RegisterAssociation
 ${AllAssociation_VERBOSE} 4   # all verbosity
 !insertmacro UnRegisterAssociation
 ${AllAssociation_VERBOSE} 3   # no script
*/


!ifndef AllAssociation_INCLUDED
!define AllAssociation_INCLUDED

!include Util.nsh

!verbose push
!verbose 3
!ifndef _AllAssociation_VERBOSE
  !define _AllAssociation_VERBOSE 3
!endif
!verbose ${_AllAssociation_VERBOSE}
!define AllAssociation_VERBOSE `!insertmacro AllAssociation_VERBOSE`
!verbose pop

!macro AllAssociation_VERBOSE _VERBOSE
  !verbose push
  !verbose 3
  !undef _AllAssociation_VERBOSE
  !define _AllAssociation_VERBOSE ${_VERBOSE}
  !verbose pop
!macroend


; define some registery macros
!define RegistryRead `!insertmacro RegistryRead`

!macro RegistryRead _PATH _VALUE _STRING _TYPE
	registry::_Read /NOUNLOAD `${_PATH}` `${_VALUE}`
	Pop ${_STRING}
	Pop ${_TYPE}
!macroend


!define RegistryWrite `!insertmacro RegistryWrite`

!macro RegistryWrite _PATH _VALUE _STRING _TYPE _ERR
	registry::_Write /NOUNLOAD `${_PATH}` `${_VALUE}` `${_STRING}` `${_TYPE}`
	Pop ${_ERR}
!macroend

!define RegistryFind `!insertmacro RegistryFind`

!macro RegistryFind _HANDLE _PATH _VALUEORKEY _STRING _TYPE
	registry::_Find /NOUNLOAD `${_HANDLE}`
	Pop ${_PATH}
	Pop ${_VALUEORKEY}
	Pop ${_STRING}
	Pop ${_TYPE}
!macroend

!define RegistryOpen `!insertmacro RegistryOpen`

!macro RegistryOpen _PATH _OPTIONS _HANDLE
	registry::_Open /NOUNLOAD `${_PATH}` `${_OPTIONS}`
	Pop ${_HANDLE}
!macroend

!macro RegisterAssociationCall _EXTENSION _EXECUTABLE _PROG_ID _DESCRIPTION _ICON _TYPE
  !verbose push
  !verbose ${_AllAssociation_VERBOSE}

  Push `${_EXECUTABLE}`
  Push `${_PROG_ID}`
  Push `${_DESCRIPTION}`
  Push `${_ICON}`

  ${CallArtificialFunction} CreateProgId_


  Push `${_PROG_ID}`
  Push `${_EXECUTABLE}`
  Push `${_EXTENSION}`
  Push `${_TYPE}`

  ${CallArtificialFunction} RegisterAssociation_

  !verbose pop
!macroend

!macro UnRegisterAssociationCall _EXTENSION _PROG_ID _EXECUTABLE _TYPE
  !verbose push
  !verbose ${_AllAssociation_VERBOSE}
  Push `${_EXTENSION}`
  Push `${_EXECUTABLE}`
  Push `${_PROG_ID}`
  Push `${_TYPE}`

  ${CallArtificialFunction} UnRegisterAssociation_

  ; backuped ProgId was pushed from UnRegisterAssociation_
  Push `${_EXTENSION}`
  Push `${_TYPE}` ; type of association
  ${CallArtificialFunction} SetAsDefaultNoAppName_

  !verbose pop
!macroend

!macro CreateProgIdCall _PROG_ID _EXECUTABLE _DESCRIPTION _ICON
  !verbose push
  !verbose ${_AllAssociation_VERBOSE}
  Push `${_EXECUTABLE}`
  Push `${_PROG_ID}`
  Push `${_DESCRIPTION}`
  Push `${_ICON}`

  ${CallArtificialFunction} CreateProgId_
  !verbose pop
!macroend

!macro SetAsDefaultNoAppNameCall _PROG_ID _EXTENSION _TYPE
  !verbose push
  !verbose ${_AllAssociation_VERBOSE}
  Push `${_PROG_ID}`
  Push `${_EXTENSION}`
  Push `${_TYPE}`

  ${CallArtificialFunction} SetAsDefaultNoAppName_
  !verbose pop
!macroend

!define SetAsDefaultNoAppName `!insertmacro SetAsDefaultNoAppNameCall`
!define un.SetAsDefaultNoAppName `!insertmacro SetAsDefaultNoAppNameCall`

!macro SetAsDefaultNoAppName
!macroend

!macro un.SetAsDefaultNoAppName
!macroend

!macro SetAsDefaultNoAppName_
  !verbose push
  !verbose ${_AllAssociation_VERBOSE}

  Pop $R0 ; TYPE
  Pop $R1 ; EXTENSION
  Pop $R2 ; PROG_ID

  StrCmp $R2 "No ProgId Pushed" NoBackupedProgId 0
  StrCmp $R2 "" NoBackupedProgId 0
  StrCmp $R1 "" NoBackupedProgId 0
    ${RegistryOpen} "HKLM\SOFTWARE\RegisteredApplications" "/K=0 /V=1 /S=0 /B=1" $R8 ; R8 = first registery handle
	StrCmp $R8 0 close 0
loop1:
	${RegistryFind} "$R8" $1 $R3 $3 $4 ; $R3 = AppRegisteredName
	StrCmp $1 "" close 0

	${RegistryOpen} "HKLM\$3" "/K=0 /V=1 /S=0 /B=1 /N='$R1'" $R9 ; R9 = second registery handle
	StrCmp $R9 0 loop1 0
loop2:
	${RegistryFind} "$R9" $1 $2 $R4 $4 ; $R4 = ProgId registered for EXTENSION
	StrCmp $1 "" loop1 0
	StrCmp $R4 "$R2" 0 loop2

	AppAssocReg::SetAppAsDefault "$R3" "$R1" "$R0"

close:
	registry::_Close /NOUNLOAD "$R9"
	registry::_Close /NOUNLOAD "$R8"
	registry::_Unload

NoBackupedProgId:
  !verbose pop
!macroend

!define CreateProgId `!insertmacro CreateProgIdCall`
!define un.CreateProgId `!insertmacro CreateProgIdCall`

!macro CreateProgId
!macroend

!macro un.CreateProgId
!macroend

!macro CreateProgId_
  !verbose push
  !verbose ${_AllAssociation_VERBOSE}

  Pop $R0 ;ICON
  Pop $R1 ;DESCRIPTION
  Pop $R2 ;PROG_ID
  Pop $R3 ;EXECUTABLE

  ReadRegStr $0 HKCR $R2 ""
;  StrCmp $0 "" 0 JustSkip ; if icon and description are present then just skip
    WriteRegStr HKCR "$R2" "" "$R1"
    WriteRegStr HKCR "$R2\shell" "" "open"
    WriteRegStr HKCR "$R2\DefaultIcon" "" "$R0"
;JustSkip:
  WriteRegStr HKCR "$R2\shell\open\command" "" '"$R3" "%1"'
  WriteRegStr HKCR "$R2\shell\edit" "" "Edit $R1"
  WriteRegStr HKCR "$R2\shell\edit\command" "" '"$R3" "%1"'

  !verbose pop
!macroend

!define RegisterAssociation `!insertmacro RegisterAssociationCall`
!define un.RegisterAssociation `!insertmacro RegisterAssociationCall`

!macro RegisterAssociation
!macroend

!macro un.RegisterAssociation
!macroend

!macro RegisterAssociation_
  !verbose push
  !verbose ${_AllAssociation_VERBOSE}

  Pop $R0 ;type = file or protocol
  Pop $R1 ;association name = ext or protocol
  Pop $R2 ;exe
  Pop $R3 ;prog_id


  StrCmp "$R0" "file" 0 NoFile
; file
    ReadRegStr $1 HKCR $R1 ""  ; read current file association
    StrCmp "$1" "" NoFileExtBackup  ; is it empty
    StrCmp "$1" "$R3" NoFileExtBackup  ; it is our own
      WriteRegStr HKCR $R1 "backup_val" "$1"  ; backup current value
NoFileExtBackup:
    WriteRegStr HKCR $R1 "" "$R3"  ; set our file association
Goto VistaPlus

NoFile:
; Protocol
  StrCmp "$R0" "protocol" 0 NotSupported
    ; write protocol flag
    WriteRegStr HKCR $R1 "URL Protocol" ""
	;get prog_id of current default

	AppAssocReg::QueryCurrentDefault $R1 "protocol" "user"
    Pop $1
	StrCmp "$1" "method failed" NoProgID 0
	StrCmp "$1" "method not available" NoProgID 0
    StrCmp "$1" "$R3" NoProgID 0   ; it is our own
      WriteRegStr HKCR "$R1\shell\open\command" "backup_progid" "$1"  ; backup current progid
NoProgID:
;    ReadRegStr $1 HKCR "$R1\shell\open\command" ""  ; read current protocol association
	; some applications write this as REG_EXPAND_SZ and don't work with REG_SZ (e.g.: some version of Opera)
	${RegistryRead} "HKCR\$R1\shell\open\command" "" '$1' '$0' ; read current protocol association and its registery type
    StrCmp "$1" "" NoProtocolBackup  ; is it empty
    StrCmp "$1" '"$R2" "%1"'  NoProtocolBackup  ; it is our own
      ;WriteRegStr HKCR "$R1\shell\open\command" "backup_val" "$1"  ; backup current value
	  ${RegistryWrite} "HKCR\$R1\shell\open\command" "backup_val" '$1' '$0' '$2'; backup current value and its registery type
NoProtocolBackup:
    WriteRegStr HKCR "$R1\shell\open\command" "" '"$R2" "%1"'  ; set our file association
;Goto VistaPlus

; TODO: type = startMenu or type = mime

VistaPlus:
  ;Vista and newer need some more works
  AppAssocReg::SetAppAsDefault "${PRODUCT_NAME}" "$R1" "$R0"

NotSupported:
  registry::_Unload
  !verbose pop
!macroend

!define UnRegisterAssociation `!insertmacro UnRegisterAssociationCall`
!define un.UnRegisterAssociation `!insertmacro UnRegisterAssociationCall`

!macro UnRegisterAssociation
!macroend

!macro un.UnRegisterAssociation
!macroend

!macro UnRegisterAssociation_
  !verbose push
  !verbose ${_AllAssociation_VERBOSE}

  Pop $R3 ;type = file or protocol
  Pop $R2 ;prog_id
  Pop $R1 ;exe
  Pop $R0 ;association name = ext or protocol

  StrCmp "$R3" "file" 0 NoFile
; file
	  ReadRegStr $1 HKCR $R0 ""
	  StrCmp $1 $R2 0 NoOwn ; only do this if we own it
	  ReadRegStr $1 HKCR $R0 "backup_val"
	  StrCmp $1 "" 0 RestoreFile ; if backup="" then delete the whole key
	  Push `"No ProgId Pushed"`
	  DeleteRegKey HKCR $R0
	  Goto NoOwn

	RestoreFile:
	  Push `$1`
	  WriteRegStr HKCR $R0 "" $1
	  DeleteRegValue HKCR $R0 "backup_val"
	  DeleteRegKey HKCR $R2 ;Delete key with association name settings
	  Goto NoOwn

NoFile:
; Protocol
  StrCmp "$R3" "protocol" 0 NoOwn
	  ReadRegStr $1 HKCR "$R0\shell\open\command" ""
	  StrCmp $1 '"$R1" "%1"' 0 NoOwn ; only do this if we own it
	  ; ReadRegStr $1 HKCR "$R0\shell\open\command" "backup_val"
	  ${RegistryRead} "HKEY_CLASSES_ROOT\$R0\shell\open\command" "backup_val" '$1' '$0' ; read current protocol association and its registery type
	  StrCmp $1 "" 0 RestoreProtocol ; if backup="" then delete the whole key
	  Push `"No ProgId Pushed"`
	  DeleteRegKey HKCR "$R0\shell\open\command"
	  Goto NoOwn

	RestoreProtocol:
	  ReadRegStr $2 HKCR "$R0\shell\open\command" "backup_progid"
	  StrCmp $2 "" 0 HasProgId
	    Push `"No ProgId Pushed"`
	    Goto NoProgID
	  HasProgId:
	    Push `$2`
	NoProgID:
	  ;WriteRegStr HKCR "$R0\shell\open\command" "" $1
	  ${RegistryWrite} "HKEY_CLASSES_ROOT\$R0\shell\open\command" "" '$1' '$0' '$2'
	  DeleteRegValue HKCR "$R0\shell\open\command" "backup_val"
	  DeleteRegValue HKCR "$R0\shell\open\command" "backup_progid"
      StrCmp $R2 "" NoOwn 0 ;Delete protocol association if it's present
	    DeleteRegKey HKCR $R2 ;Delete key with association name settings
;Goto NoOwn

; TODO: type = startMenu or type = mime

 NoOwn:
  registry::_Unload
  !verbose pop
!macroend

!define UpdateSystemIcons `!insertmacro UpdateSystemIcons_`
!define un.UpdateSystemIcons `!insertmacro UpdateSystemIcons_`

!macro UpdateSystemIcons
!macroend

!macro un.UpdateSystemIcons
!macroend

 ; !defines for use with SHChangeNotify
!ifdef SHCNE_ASSOCCHANGED
  !undef SHCNE_ASSOCCHANGED
!endif
!define SHCNE_ASSOCCHANGED 0x08000000

!ifdef SHCNF_FLUSHNOWAIT
  !undef SHCNF_FLUSHNOWAIT
!endif
!define SHCNF_FLUSHNOWAIT        0x3000

!macro UpdateSystemIcons_
; Using the system.dll plugin to call the SHChangeNotify Win32 API function so we
; can update the shell.
  System::Call "shell32::SHChangeNotify(i,i,i,i) (${SHCNE_ASSOCCHANGED}, ${SHCNF_FLUSHNOWAIT}, 0, 0)"
!macroend
!endif # !AllAssociation_INCLUDED
