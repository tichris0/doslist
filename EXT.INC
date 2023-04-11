; EXT Unit for file lister                      <C> 1992 Created by Dàrk Shàde
; Compression formats
include    ZIP.INC                      ; Few bugs with some ZIP files!
include    ARJ.INC
include    RAR.INC                      ; Must include *.R##
include    LHA.INC
include    ACE.INC                      ; Must include *.C##
; File formats
include    EXE.INC
include    SYS.INC
include    OBJ.INC
include    TXT.INC
; Graphic formats
;include    PAL.INC
;include    PCX.INC
;include    VESA.INC



.CONST

; Extension Handlers
Extensions equ $
; File archive
ZIPExt     _ext < 'ZIP', FileArchive, LoadZIP >
ARJExt     _ext < 'ARJ', FileArchive, LoadARJ >
LHAExt     _ext < 'LHA', FileArchive, LoadLHA >
LZHExt     _ext < 'LZH', FileArchive, LoadLHA >
ACEExt     _ext < 'ACE', FileArchive, LoadACE >
RARExt     _ext < 'RAR', FileArchive, LoadRAR >

; Metadata viewers
EXEExt     _ext < 'EXE', FileMetadata, LoadEXE >
OBJExt     _ext < 'OBJ', FileMetadata, LoadOBJ >
OVLExt     _ext < 'OVL', FileMetadata, LoadEXE >
SYSExt     _ext < 'SYS', FileMetadata, LoadSYS >

ExtensionCount equ ($ - Extensions) / SIZE _ext

DefaultExt _ext < 'TXT', FileMetadata, LoadTXT >




.CODE

; Procedure to manage the special viewing system
ShowExt PROC
           mov  si, OFFSET DTA.dtaName
SeekDot:   lodsb
           or   al, al
           jz   SpecialDone
           cmp  al, '.'
           jnz  SeekDot

           mov  bx, OFFSET Extensions
           mov  cx, ExtensionCount
           lodsb

CheckExt:  cmp  al, [bx + OFFSET _ext.Text]
           jne  CheckNextExt                ; Does the first byte match?                  
           
           mov  dx, [bx + OFFSET _ext.Text + 1]
           or   dx, dx
           jz   ExtFound
         
           cmp  dx, [si]  
           jne  CheckNextExt                ; Does the following 2 bytes match?


; We found the command, so let's execute it's handler function
ExtFound:  mov  si, [bx + OFFSET _ext.ExtHandler]
           mov  cl, [bx + OFFSET _ext.ExtType] 
  
           mov  ax, 03D00h
           mov  dx, OFFSET DTA.dtaName
           int  21h
           jc   Back
           xchg ax, bx
           
           cmp  cl, FileArchive 
           jne  SpxHandler

           xor  dx, dx ; TEMP
           call TellArchive
           jmp  SkipSpxH

SpxHandler:call si

SkipSpxH:  mov  ah, 3Eh
           int  21h

           call AdjustDI
;           call ShowMore
Back:      ret



CheckNextExt:
           add  bx, SIZE _ext
           dec  cx
           jnz  CheckExt

           ; Extension doesn't match anything known.  Use default
           mov  bx, OFFSET DefaultExt
           jmp  ExtFound
           
SpecialDone:ret
ShowExt ENDP           
