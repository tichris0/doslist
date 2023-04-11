; RAR Unit for file lister                      <C> 1992 Created by Dàrk Shàde
.CODE

_rarhead STRUC
    rarHeadCRC      dw ?            ; CRC of total block of block part
    rarHeadType     db ?            ; Block type
    rarHeadFlags    dw ?            ; Block flags
    rarHeadSize     dw ?            ; Block size
;   rarAddSize      dd ?            ; added IFF (HeadFlags & 0x8000) == 0
    rarCompSize     dd ?            ; Compressed size
    rarUncompSize   dd ?            ; Uncompressed size
    rarHostOS       db ?            ; Host OS enum
    rarCRC32        dd ?            ; CRC32 of uncompressed data
    rarTime         dw ?            ; File time
    rarDate         dw ?            ; File date
    rarVersion      db ?            ; Unpacker version required
    rarMethod       db ?            ; Unpacking method
    rarFNameLen     dw ?            ; Filename's length
    rarAttrib       dd ?            ; File attributes
;   rarHighPackSize dd ?            ; Only present if LHD_LARGE is set
;   rarHighUnpSize  dd ?            ; Only present if LHD_LARGE is set
;   rarFilename     db ? dup(?)
;   rarSalt         db 8 dup(?)     ; Only present if lHD_SALT is set
;   rarExtTime      _rarExtTime     ; Only present if LHD_exttime is set
_rarhead ENDS



LoadRAR PROC
           mov  cx, 128 + SIZE _rarhead
           push ds
           push fs
           pop  ds
           int  21h                     ; Read the header!
           pop  ds
           or   ax, ax
           jz   DoneRAR

; The first header is special
           cmp  FPointer, 0
           jnz  RARNextHd

           cmp  DWORD PTR fs:[0], '!raR'
           jne  DoneRAR

           mov  DWORD PTR FPointer, 7

RARSkipHd: mov  ax, 4200h
           mov  cx, WORD PTR [OFFSET FPointer + 2]
           mov  dx, WORD PTR [OFFSET FPointer]
           int  21h                     ; Set file pointer!

           mov  ah, 3Fh
           xor  dx, dx
           jmp  LoadRAR

RARNextHd: xor  eax, eax
           mov  ax, fs:[_rarhead.rarHeadSize]
           add  FPointer, eax; Calculate the next file's pos.

           cmp  fs:[_rarhead.rarHeadType], 074h
           jne  RARSkipHd

           mov  eax, fs:[_rarhead.rarCompSize]
           add  FPointer, eax

; Copy relevant fields
           mov  eax, fs:[_rarhead.rarUncompSize] 
           mov  DTA.dtaSize, eax
           mov  eax, DWORD PTR fs:[_rarhead.rarTime]
           mov  DTA.dtaDateTime, eax
           mov  DTA.dtaAttr, 80h ; Don't show the attributes
                                        
           mov  cx, fs:[_rarhead.rarFNameLen]
           mov  si, SIZE _rarhead
           call CopyLongFilename

           mov  al, 1
DoneRAR:   ret
LoadRAR ENDP