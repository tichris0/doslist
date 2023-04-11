; FILES Unit for file lister                    <C> 1992 Created by Dàrk Shàde
.CONST

VolumeID   db '\7 Volume in current drive is %s\n', 0
DirID      db '\7 Directory of %c:\\', 0
TotSubStr  db '\7%-14z file(s)%18z byte(s)\n', 0
           db '%-12z directory%18z byte(s) free\n', 0

DirStr     db '\F%-32 <DIR>%-52 ', 0



.CODE

; Copies a long filename into the DTA
; @Input
;   fs:si   = Input String (optional ASCIIZ)
;   cl      = Length (Optional if ASCIIZ)
; @Modified
;   al, cl, di 
CopyLongFilename PROC
           mov  di, OFFSET DTA.dtaName

           cmp  cl, 24 + 13
           jbe  CLFLenOK
           mov  cl, 24 + 13

CLFLenOK:  mov  al, fs:[si]
           inc  si
           or   al, al
           jz   CLFDone
           mov  ds:[di], al
           inc  di
           dec  cl
           jnz  CLFLenOK                ; Copy the filename
           
CLFDone:   mov  BYTE PTR ds:[di], 0     ; ASCIIZ
           ret
CopyLongFilename ENDP 

           

; Procedure to choose the file's color based on extension
; @Input
;   DTA
; @Ouput
;   ah = Color
; @Modified
;   eax, cx, si
ColorExt PROC
; Directories get bright white
           test DTA.dtaAttr, 10h
           jz   ColorFiles

           mov  ah, 0Fh
           ret

; Files depend on extension
ColorFiles:mov  si, OFFSET DTA.dtaName
           
FindExtDot:inc  si
           cmp  BYTE PTR ds:[si], '.'
           je   FoundExtDt
           cmp  BYTE PTR ds:[si], 0
           jne  FindExtDot
           
ExtNotFnd: mov  ah, 7
           ret
           
FoundExtDt:mov  eax, [si]
           shr  eax, 8
           mov  cx, ExtColorCount
           mov  si, OFFSET ExtColors - SIZE _ext_color
           
CheckCExt: add  si, SIZE _ext_color
           mov  edx, DWORD PTR [si + _ext_color.ExtString]
           and  edx, 0FFFFFFh
           cmp  eax, edx
           je   FoundExtCo
           loop CheckCExt

           jmp  ExtNotFnd

FoundExtCo:mov  ah, [si + _ext_color.ExtColor]
           ret
ColorExt ENDP



; Procedure to write a filename on the screen
; @Input
;   ah      = Color
;   es:di   = Screen location 
;   DTA
; @Modifies
;   al, cx, si, di
TellFileName PROC
           call ColorExt
           mov  si, OFFSET DTA.dtaName
           mov  cx, si
           
TFNNext:   inc  si
           cmp  BYTE PTR ds:[si], 0
           je   TFScanDone
           cmp  BYTE PTR ds:[si], '\'
           je   TFNProcess
           cmp  BYTE PTR ds:[si], '/'
           jne  TFNNext
           
           mov  BYTE PTR ds:[si], '\'
           
TFNProcess:mov  cx, si
           jmp  TFNNext
           
TFScanDone:cmp  cx, OFFSET DTA.DTAName
           je   TFScanFile

           xchg si, cx
           inc  si
           xor  cl, cl
           xchg cl, ds:[si]
           mov  si, OFFSET DTA.dtaName           
           call TellStr
           xchg ds:[si], cl

           jmp  TFScanShrt

TFScanFile:mov  si, OFFSET DTA.dtaName
TFScanShrt:mov  cx, '.' * 256 + 9
           call TellStrN
           
           dec  si
           mov  cx, 4           
           call TellStrN
           add  di, 2 * 3

           call CheckMore
           
           ret
TellFileName ENDP



; Procedure to convert the BIOS-DT info to DEC time/date
; @Input
;   ah      = Color
;   es:di   = Screen location 
;   DTA
; @Modifies
;   bx, cx, dx, di
TellDate PROC
           push ax

           mov  cl, 2
           mov  bx, DTA.dtaDate
           and  bx, 11111b              ; al = Day
           call TellNum

           mov  al, '-'
           stosw

           mov  cl, 2
           mov  bx, DTA.dtaDate
           shr  bx, 5                   ; al = Month
           and  bx, 1111b
           call TellNum

           mov  al, '-'
           stosw
           
           mov  cl, 4
           mov  bx, DTA.dtaDate
           shr  bx, 9
           add  bx, 1980
           call TellNum

           add  di, 2 * 2

           mov  cl, 2
           mov  bx, DTA.dtaTime
           shr  bx, 11                   ; ax = Hour
           call TellNum

           mov  al, ':'
           stosw

           mov  cl, 2
           mov  bx, DTA.dtaTime
           shr  bx, 5
           and  bx, 111111b
           call TellNum

           mov  al, ':'
           stosw

           mov  cl, 2
           mov  bx, DTA.dtaTime
           and  bx, 11111b
           add  bx, bx                  ; Sec * 2
           call TellNum

           pop  ax       
           ret
TellDate ENDP



; Procedure to convert DOS attributes to Uniletters
; @Input
;   es:di   = Screen location 
;   DTA
; @Modifies
;   cl 
TellAttr PROC
           push ax
           mov  cl, DTA.dtaAttr
           
           xor  ax, ax
           test cl, 1
           je   NotR
           mov  ax, 0E00h + 'R' 
           
NotR:      stosw
           xor  ax, ax
           test cl, 20h
           je   NotA
           mov  ax, 0B00h + 'A' 

NotA:      stosw
           xor  ax, ax
           test cl, 4
           je   NotS
           mov  ax, 0A00h + 'S' 

NotS:      stosw
           xor  ax, ax
           test cl, 2
           je   NotH
           mov  ax, 0D00h + 'H'
           
NotH:      stosw

           pop  ax
           ret
TellAttr ENDP



; Procedure to write the header : Volume and current directory
; @Input
;   dx = 0 (Skip the extra line feeds), =/= 0 (Display the line feed)
; @Modifies
;   ax, dl, si, di
TellHead PROC
           push dx
           push OFFSET VolumeName
           mov  bx, sp

           add  di, 160
           call CheckMore
           mov  si, OFFSET VolumeID
           call TellFStr                ; 'Volume is...'
           mov  sp, bx

           mov  ah, 19h
           int  21h
           add  al, 'A'
           push ax
           mov  bx, sp

           mov  si, OFFSET DirID
           call TellFStr
           mov  sp, bx

           sub  sp, 64
           mov  si, sp

           push ds
           push ss
           pop  ds
           mov  ah, 47h
           xor  dl, dl
           int  21h                     ; Get current directory

           mov  ah, 07
           call TellStr
           pop  ds
           add  sp, 64

           pop  si
           or   si, si
           jnz  AddLF

           ret
           
AddLF:     call AdjustDI
           add  di, 160
           call CheckMore

           ret
TellHead ENDP



; Procedure to write the long file detailed from the DTA
; Attrib == 80h => Skip the parameter
; @Input DTA
; @Modified
;   ax, bx, cx, edx, si, di   
TellFile PROC
           call ColorExt
           call TellFileName

           mov  ah, 7
           call FixupDI
           add  di, (16 + 15 + 21) * 2 - 160
           cmp  DTA.dtaAttr, 080h
           jne  TellRgAttr

           add  di, 2 * 6

TellRgAttr:test DTA.dtaAttr, 10h
           jz   TellFLSize              ; Check if it's a directory!
                                        ; We won't write the size           
           mov  si, OFFSET DirStr  
           call TellFStr
           jmp  TellFDate           

TellFLSize:mov  esi, DTA.dtaSize
           call TellSize    

TellFDate: add  di, 2 * 2
           call TellDate

           cmp  DTA.dtaAttr, 080h
           je   SkipAttr
           
           add  di, 2 * 2           
           call TellAttr

SkipAttr:  call AdjustDI
           
           ret
TellFile ENDP



; Procedure to write the total disk space information
; @Modifies
;   ax, bx, cx, dx, si, di   
TellSub PROC
;           call FixupDI
           add  di, 160
           
; Get total unused space!
           xor  ecx, ecx
           mov  ah, 36h
           xor  dl, dl
           int  21h

           mul  bx
           shl  edx, 16
           mov  dx, ax
           mov  eax, edx
           mul  ecx                     ; DX:AX = AX * BX * CX
           push eax 
           
           mov  dx, TotDirs
           push edx

           push TotUsed            

           mov  dx, TotFiles
           push edx
           mov  bx, sp

           mov  si, OFFSET TotSubStr
           call TellFStr
           call CheckMore
           add  di, 160
           call TellFStr

           add  sp, 4 * 4
           ret
TellSub ENDP
