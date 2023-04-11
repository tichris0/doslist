; File lister&viewer                            <C> 1992 Created by Dàrk Shàde
; Last reviewed                                                          04-23

.model compact
.386
.STACK 100h
.DATA?

; Data parsing buffer
SEGMENT FRB USE16
ORG 0h
    ReadBuffer db 65536 dup(?)
ENDS FRB 



SEGMENT FR USE16
ORG 0h
    FileRecords db 65534 dup(?)
    FRCount     dw ?            ; Number of file records in buffer 
ENDS FR



_cmd STRUC
    CmdText     db 3 dup(?)     ; String command
    CmdPrepare  dw ?            ; Parameter parsing handler
    CmdMainHand dw ?            ; Command's main handler (called per file)
    CmdSubHand  dw ?            ; Command's sub handler (optional per handler)
    CmdPreHand  dw ?            ; Command Pre-handler (called at start of dir) 
    CmdPostHand dw ?            ; Command Post-handler (called at end of dir) 
_cmd ENDS



_ext_type ENUM {
    FileMetadata                        ; File Metadata view
    FileArchive                         ; File Archive
}



_ext STRUC
    Text        db 3 dup(0)             ; Extension end
    ExtType     _ext_type ?             ; Extension type
    ExtHandler  dw 0                    ; Command handler address
_ext ENDS



; File Data
_fileData STRUC
	fileAttr		    db ?		; Found attribute
    UNION
        STRUC
        	fileTime    dw ?        ; File Time
    	    fileDate	dw ?        ; File Date
        ENDS
        fileDateTime    dd ?        ; File Time & Date
    ENDS
	fileSize		    dd ?		; File Size
	fileName	        db 13 dup(?); Packed filename
_fileData ENDS



; Disk Transfer Area
_dta STRUC
    ; Search criteria used
	dtaSAttr	db ?		; Search Attribute
	dtaSDrive	db ?		; Search Drive
	dtaSName	db 11 dup(?); Search Name
    ; Disk associations 
	dtaDirEntry dd ?		; Directory entry number
    dtaRes1     dw ?        ; Reserved		  
	dtaDirStart	dw ?		; DirStart
    ; File parameters
	dtaAttr		    db ?		; Found attribute
    UNION
        STRUC
        	dtaTime	dw ?        ; File Time
    	    dtaDate	dw ?        ; File Date
        ENDS
        dtaDateTime dd ?        ; File Time & Date
    ENDS
	dtaSize		    dd ?		; File Size
	dtaName	        db 13 dup(?); Packed filename
_dta ENDS



; File record structure
_fileRec STRUC
    ; File attribute
	fileAttr	        db ?		; Found attribute
    UNION
        STRUC
        	fileTime	dw ?        ; File Time
    	    fileDate	dw ?        ; File Date
        ENDS
        fileDateTime    dd ?        ; File Time & Date
    ENDS
	fileSize		    dd ?		; File Size
	fileName	        db 13 dup(?); Packed filename
_fileRec ENDS



; Constants
.CONST

ScrSeg     dw 0B800h



; Array of commands (Must be sorted by longest strings first)
Commands equ $

; System viewers
BootCmd    _cmd < 'MBR', LoadBoot >
MCBCmd     _cmd < 'MCB', LoadMCB >
DeviceCmd  _cmd < 'DEV', LoadDEV >
ViewCmd    _cmd < 'V',   ViewHelp >
HelpCmd    _cmd < '?',   TellHelp >

; Command line parsers/modifiers
JumpCmd    _cmd < 'J', SetJump >
RecurseCmd _cmd < 'S', SetRecurse >

; File viewers
SpecialCmd _cmd < 'X', SetCB, ShowExt,      NoOp,        NoOp, NoOp >
ASCIICmd   _cmd < 'I', SetCB, ViewRAW,      ShowASCII,   NoOp, NoOp >
NumberCmd  _cmd < 'C', SetCB, ViewRAW,      ShowDec,     NoOp, NoOp >
HexCmd     _cmd < 'H', SetCB, ViewRAW,      ShowHex,     NoOp, NoOp >

; File lister
WideCmd    _cmd < 'W', SetCB, TellFileName, NoOp, TellHead, TellSub >
TreeCmd    _cmd < 'R', SetCBRecurse, NoOp,  NoOp, ShowTree, NoOp >

; Modifiers
TimeCmd    _cmd < 'T', PrepTime, ModifyFile, CastDateTime,  NoOp, NoOp >
DateCmd    _cmd < 'D', PrepDate, ModifyFile, CastDateTime,  NoOp, NoOp >
AttribCmd  _cmd < 'N', PrepAttr, ModifyFile, CastAttr,      NoOp, NoOp >

CommandCount equ ($ - Commands) / SIZE _cmd

AllCmd     _cmd < 'A', SetCB, TellFile, NoOp, TellHead, TellSub >



; Extension colors
_ext_color STRUC
    ExtColor  db ?    
    ExtString db 3 dup(?)
_ext_color ENDS

ExtColors equ $
; Executable files
_ext_color < 8+2, 'BAT' >
_ext_color < 8+2, 'COM' >
_ext_color < 8+2, 'EXE' >
; System files
_ext_color < 8+3, 'OBJ' >
_ext_color < 8+3, 'OVL' >
_ext_color < 8+3, 'SYS' >
_ext_color < 8+3, 'DRV' >
_ext_color < 8+3, '386' >
_ext_color < 8+3, 'DLL' >
; Archives
_ext_color < 8+1, 'ZIP' >
_ext_color < 8+1, 'ARJ' >
_ext_color < 8+1, 'LHA' >
_ext_color < 8+1, 'LZH' >
_ext_color < 8+1, 'RAR' >
_ext_color < 8+1, 'ACE' >
ExtColorCount equ ($ - ExtColors) / SIZE _ext_color



; Extention strings!
;RARNumExt  db '.R'
;ACENumExt  db '.C'

; Graphics viewers
;PALExt     db '.PAL'
;PCXExt     db '.PCX'



FStr       db '%S\n', 0
BackDir    db '..', 0
AllFiles   db '*.*', 0



; Data patching area
.DATA
; Variables are here!
Recurse    db   0                       ; Recurse over directories?
More       db   1                       ; Show more content?
JumpAddr   dd   0                       ; Number given by "/J"

; Grand totals for recursive functions
GTotUsed   dd   0
GTotFiles  dw   0
GTotDirs   dw   0

NameFile   db '????????.???', 0         ; FileName - Search String
Zero       equ $ - 1                    ; Pointer to a NULL ASCIIZ

Command    dw AllCmd                    ; Command to execute



.DATA?

VolumeName db 13 dup(?)                 ; Name of the current volume

TotUsed    dd ?                         ; Total memory used
TotFiles   dw ?                         ; Total files
TotDirs    dw ?                         ; Total directories (must be after Files)

ScrEnd     dw ?

DTA        _dta ?
           db   25 dup(?)               ; Used for long filenames (must follow the DTA)

CmdLine    db   80h dup(?)              ; Copy of the command line arguments
OldDrive   db   ?                       ; The startup drive   
OldDir     dd   128 dup(?)              ; The startup directory

Spill64K   db   ?
ResumeDTA  dd   ?


.CODE
ASSUME GS:FR
ASSUME FS:FRB

; First, setup the data segment
Start:     mov  bx, seg Commands
           mov  ds, bx                  ; ds = Const + Variables
           mov  bx, seg ReadBuffer
           mov  fs, bx                  ; fs = Memory reading segment
           mov  bx, seg FileRecords
           mov  gs, bx                  ; gs = Memory for file records

           xor  ax, ax
           mov  es, ax
           mov  si, 484h                ; Screen rows
           mov  al, es:[si]
           mov  cx, 160
           mul  cx
           mov  ScrEnd, ax              ; Calculate the screen size
           mov  es, ScrSeg              ; es = screen segment
           
           call Clear

           mov  ah, 1Ah
           mov  dx, OFFSET DTA
           int  21h                     ; Set our new DTA!

           push ds
           pop  es
           mov  ah, 19h
           int  21h                     ; Get current drive
           mov  OldDrive, al
           


; Prepare the registers to consume the arguments
           mov  ah, 62h
           int  21h
           mov  es, bx                  ; es = PSP segment
           mov  di, 80h
           mov  cl, BYTE PTR es:[di]    ; dx = Parameter length
           or   cl, cl
           jz   StartList
           inc  di



; Capitalize every letter in the command & copy in ReadBuffer temporarilly
NextLet:   mov  al, es:[di]
           cmp  al, 97
           jb   NotLet
           cmp  al, 122
           ja   NotLet
           sub  al, 32
           mov  BYTE PTR es:[di], al    ; Adjust for all capital letters!

NotLet:    mov  ds:[di - 81h + CmdLine], al  ; Copy in CmdLine
           inc  di
           dec  cl
           jnz  NextLet

           mov  WORD PTR ds:[di - 81h + CmdLine], 0020h   ; Convert to ASCIIZ

           mov  es, ScrSeg


           
; Parse the arguments
           mov  si, OFFSET CmdLine
ParseParam:lodsb
           or   al, al
           je   StartList

           cmp  al, ' '
           je   ParseParam              ; Skip spaces
           
           cmp  al, '/'                 ; Issuing a command?
           jne  ParsePath
           
           mov  di, OFFSET Commands
           mov  cx, CommandCount
           lodsb



; Check for a command "/." or "/..."
CheckCmd:  cmp  al, [di + OFFSET _cmd.Text]
           jne  CheckNextCmd                ; Does the first byte match?                  
           
           mov  dx, [di + OFFSET _cmd.Text + 1]
           or   dx, dx
           jz   CmdFound
         
           cmp  dx, [si]  
           jne  CheckNextCmd                ; Does the following 2 bytes match?

           inc  si
           inc  si                          ; Skip the command



; We found the command, so let's execute it's prepare function
CmdFound:  call [di + OFFSET _cmd.CmdPrepare]
           
           jmp  ParseParam

CheckNextCmd:
           add  di, SIZE _cmd
           dec  cx
           jnz  CheckCmd



; At this point, we're parsing a path.  Seek the full filepath 
ParsePath: lea  dx, [si - 1]      ; dx = Start of File or Directory

StartPath: lea bx, [si - 1]

SeekDir:   lodsb
           cmp  al, '\'
           je   StartPath
           cmp  al, ':'
           jne  CheckEOD            ; Should we process it as a drive?
           
           mov  dl, ds:[si - 2]
           sub  dl, 'A'
           mov  ah, 0Eh
           int  21h                 ; Change the current drive
           mov  dx, si

CheckEOD:  cmp  al, ' '
           jne  SeekDir



; Preserve the existing path
           push si
           push dx
           mov  ah, 47h
           xor  dl, dl
           mov  si, OFFSET OldDir + 1
           mov  BYTE PTR ds:[si - 1], '\'   ; Start the directory with a '\'
           int  21h                     ; Save the current directory name
           pop  dx
           pop  si


; Attempt to change the directory
           xor  cl, cl 
           mov  ds:[si - 1], cl     ; NULL termiante this string
           
           mov  ah, 3Bh
           int  21h                 ; Change directory to the full file spec
           jnc  ParseParam          ; the parameter was the entire string
           
           cmp  dx, bx
           jge  ParseFile
           
           mov  ds:[bx], cl         ; Convert to ASCIIZ = ASCII + '0'
           mov  ah, 3Bh
           int  21h                 ; Try the sub part of the filespec
           jc   Quit                ; We couldn't find a valid path
           
           lea  dx, [bx + 1]



; Now that we have the directory figured out, seek the filename
ParseFile: push es
           push ds
           pop  es

           mov  cx, si
           sub  cx, dx
           cmp  cx, 1
           jbe  ParseParam
           sub  si, cx
           mov  di, OFFSET NameFile
           rep  movsb               ; Copy filename into NameFile
           pop  es

           jmp  ParseParam



; Start listing disk content
StartList: mov  ah, 4Eh                 ; Start the listing
           mov  cx, 8                   ; 8 = Volume
           mov  dx, OFFSET AllFiles
           int  21h                     ; Get Drive label!

           or   al, al
           jnz  NoLabel
           push ds
           pop  es
           mov  si, OFFSET DTA.dtaName
           mov  di, OFFSET VolumeName
           movsd
           movsd
           movsw
           movsb                        ; Save it now!
           stosb                        ; ASCIIZ Zero
           
NoLabel:   mov  es, ScrSeg
           xor  di, di
           mov  bx, 1
           mov  dx, OFFSET Zero         ; Tree requires these 2 parameters
           
           call ScanFile
           
           ; Now tell it what main routines to load!
           cmp  Recurse, 0   ; /S
           je   Quit

           call DoCurse
           jmp  Quit
; Main ENDS



; Simple comparison function for the sort algorithm
; @Input
;   ds:si = Source comparator
;   ss:bp = Destination comparator
; @Modified
;   ax
QSCompare PROC
; First sort the directories            
           mov  al, BYTE PTR ss:[bp + OFFSET _fileData.fileAttr]
           mov  ah, BYTE PTR ds:[si + OFFSET _fileData.fileAttr]
           and  ax, 1010h
           cmp  al, ah
           jne  QSDone
           
; Next, compare the filenames
           mov  al, BYTE PTR ss:[bp + OFFSET _fileData.fileName]
           cmp  BYTE PTR ds:[si + OFFSET _fileData.fileName], al
           jne  QSDone

           push bp
           push si
           
           add  bp, OFFSET _fileData.fileName + 1
           add  si, OFFSET _fileData.fileName + 1
           
QCCompName:mov  al, BYTE PTR ss:[bp]
           cmp  BYTE PTR ds:[si], al
           jne  QSCompDone

           or   al, al
           jz   QSCompDone
           
           inc  si
           inc  bp
           jmp  QCCompName
           
QSCompDone:pop  si
           pop  bp
           
QSDone:    ret
QSCompare ENDP



; Procedure to recurse the commands over multiple directories
DoCurse PROC
           cmp  More, 0 ; FIXME
           jz   NotFMore
           
;           call ShowMore
           jmp  DoStart
           
NotFMore:  ;call Clear
           
DoStart:   mov  bx, 1                   ; Directory level = 1 = first level
NewScan:   mov  ah, 4Eh
           mov  cx, 37h
ScanRMore: mov  dx, OFFSET NameFile
           int  21h

           or   al, al
           jnz  DownRList               ; No more :(
           
           mov  ah, 4Fh
           test DTA.dtaAttr, 10h        ; Is it a directory?
           jz   ScanRMore
           
           cmp  DTA.dtaName, '.'
           je   ScanRMore               ; Don't show "." & ".." devices

           push DTA.dtaDirEntry 

           mov  ah, 03Bh
           mov  dx, OFFSET DTA.dtaName  ; Directorie's name offset
           int  21h                     ; Change directory
           jc   Quit

           inc  bx                      ; One branch up!
           push bx

           call ScanFile
           call AdjustDI

StartNew:  pop  bx
           jmp  NewScan                 ; Scan for some more!



; Unwind the stack
DownRList: dec  bx
           jz   GTot                    ; Outta here!

           pop  DTA.dtaDirEntry

           mov  ah, 03Bh
           mov  dx, OFFSET BackDir
           int  21h
           
           mov  ah, 4Fh
           jmp  ScanRMore


; We're officially done
GTot:      mov  si, Command

           xor  bl, bl
           call [si + _cmd.CmdPreHand]

           mov  eax, GTotUsed
           mov  TotUsed, eax
           mov  ax, GTotFiles
           mov  TotFiles, ax
           mov  ax, GTotDirs
           mov  TotDirs, ax
           
           mov  si, Command
           call [si + _cmd.CmdPostHand]

           ret
DoCurse ENDP



; Function to dispatch to file listing functions
ScanFile PROC
           mov  si, Command
           call [si + _cmd.CmdPreHand]

; Are we supposed to do any work per file?
           mov  si, Command           
           cmp  WORD PTR [si + _cmd.CmdMainHand], OFFSET NoOp
           je   NoOp

           xor  eax, eax 
           mov  TotUsed, eax
           mov  TotFiles, ax
           mov  TotDirs, ax
           mov  FRCount, ax
           
           push es
           push di
           push gs
           pop  es
           mov  di, ax

           mov  Spill64K, 1     ; By default, assume that we have too many files
           mov  ah, 4Eh
ScanWMore: mov  dx, OFFSET NameFile
           mov  cx, 37h
           int  21h

           or   al, al
           jnz  DoneList

           cmp  DTA.dtaName, '.'
           je   SkipFile

; Copy the DTA's relevant info to the FRB
           push es
           push gs
           pop  es
           mov  si, OFFSET DTA + (SIZE _dta) - (SIZE _fileData)
           mov  cx, (SIZE _fileData) / 2
           rep  movsw
           pop  es
           inc  FRCount

; Increment the File or Dir count
           xor  eax, eax
           test DTA.dtaAttr, 10h
           setnz al
           inc  WORD PTR [eax * 2 + OFFSET TotFiles]

; Increase total space used
           mov  eax, DTA.dtaSize
           add  TotUsed, eax
      
;           cmp  di, 44
           cmp  di, (65536 / SIZE _fileData) * (SIZE _fileData)
           jne  SkipFile

; Save state for later
           mov  eax, DTA.dtaDirEntry
           mov  ResumeDTA, eax

           jmp  NextList           

SkipFile:  mov  ah, 4Fh
           jmp  ScanWMore
           
DoneList:  mov  Spill64K, 0
           
NextList:  cmp  FRCount, 0
           je   CleanScan
           
           xor  si, si
           mov  dx, SIZE _fileData
           sub  di, dx
           push ds
           push gs
           pop  ds
           call QuickSort
           pop  ds
           
           pop  di
           pop  es
           xor  si, si              ; bx = Address of the file data

; Call the main handler
ProcFile:  push si

; Copy file info into the DTA
           push di
           push ds
           push es

           push ds
           pop  es
           push gs
           pop  ds

           mov  di, OFFSET DTA + (SIZE _dta) - (SIZE _fileData)
           mov  cx, (SIZE _fileData) / 2
           rep  movsw
           
           pop  es
           pop  ds
           pop  di

; Process the command normaly
           mov  si, Command
           call [si + _cmd.CmdMainHand]
           
           pop  si
           add  si, SIZE _fileData

           dec  FRCount
           jnz  ProcFile

           cmp  Spill64K, 0
           je   ScanPost           

           mov  eax, ResumeDTA
           mov  DTA.dtaDirEntry, eax
           
           push es
           push di
           push gs
           pop  es
           xor  di, di
           
           jmp  SkipFile  

CleanScan: pop  di
           pop  es

; Call the post handler
ScanPost:  mov  si, Command
           call [si + _cmd.CmdPostHand]

           mov  eax, TotUsed
           add  GTotUsed, eax
           mov  ax, TotFiles
           add  GTotFiles, ax
           mov  ax, TotDirs
           add  GTotDirs, ax
   
NoOp:      ret
ScanFile ENDP



; Sets the active command            
SetCB PROC
           mov  Command, di
           ret
SetCB ENDP



; Enables recursion & Sets the active command
SetCBRecurse PROC
           mov  Command, di
           mov  Recurse, 1
           ret
SetCBRecurse ENDP



; Enables recursion
SetRecurse PROC
           mov  Recurse, 1
           ret
SetRecurse ENDP

           

; Sets the read file jump location 
SetJump PROC
           call GetNum
           mov  JumpAddr, eax
           ret
SetJump ENDP



;  Procedure called upon quit
Quit PROC      
           xor  dx, dx
           mov  es, dx
           mov  ax, di
           mov  cx, 160d
           div  cx
           xchg al, ah
           mov  es:[450h], ax           ; Set the cursor properly!

           mov  ah, 3Bh
           mov  dx, OFFSET OldDir
           int  21h                     ; Restore the old directory

           mov  ah, 0Eh
           mov  dl, OldDrive
           int  21h                     ; Restore the current drive

           mov  ax, 4C00h
           int  21h                     ; Resume to caller!
Quit ENDP



; Here we bunch up all the external files
include    UTILS.inc
include    SORT.inc
include    HELP.inc
include    FILES.inc
include    ARCHIVE.inc
include    MODIFY.inc
include    EXT.inc



; Data viewers
include    RAW.INC
include    ASCII.INC
include    HEX.INC
include    DEC.INC
include    TREE.INC
; Data setters
include    ATTR.INC
include    DATETIME.INC
; Memory manager
include    XMS.INC
; Memory formats
include    BOOT.INC
include    MCB.INC
include    DEV.INC



END        Start
