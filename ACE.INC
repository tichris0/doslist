; ACE Unit for file lister                      <C> 1992 Created by Dàrk Shàde
.CODE

_acehead STRUC
    aceHeadCRC      dw ?            ; CRC of total block up to HEAD_TYPE
    aceHeadSize     dw ?            ; Block size
    aceHeadType     db ?            ; Type of block
    aceHeadFlags    dw ?            ; Flags related to the block and its content
    aceCompSize     dd ?            ; Compressed size
    aceUncompSize   dd ?            ; Uncompressed size
    aceTime         dw ?            ; File time
    aceDate         dw ?            ; File date
    aceAttrib       dd ?            ; File attributes
    aceCRC32        dd ?            ; CRC of file
    aceTechInfo     dd ?            ; file info
    aceRes1         dw ?            ; Reserved
    aceFNameLen     dw ?            ; Filename length
_acehead ENDS



LoadACE PROC
           mov  cx, 128 + SIZE _acehead
           push ds
           push fs
           pop  ds
           int  21h                     ; Read the header!
           pop  ds
           or   ax, ax
           jz   DoneACE

; The first header is special
           cmp  FPointer, 0
           jnz  ACENextHd

           cmp  DWORD PTR fs:[8], 'ECA*'
           jne  DoneACE

           jmp  ACENextHd

ACESkipHd: mov  ax, 4200h
           mov  cx, WORD PTR [OFFSET FPointer + 2]
           mov  dx, WORD PTR [OFFSET FPointer]
           int  21h                     ; Set file pointer!

           mov  ah, 3Fh
           xor  dx, dx
           jmp  LoadACE

ACENextHd: xor  eax, eax 
           mov  ax, fs:[_acehead.aceHeadSize]
           add  eax, OFFSET _acehead.aceHeadType
           add  DWORD PTR FPointer, eax; Calculate the next file's pos.

           cmp  BYTE PTR fs:[_acehead.aceHeadType], 01h
           jne  ACESkipHd

           mov  eax, fs:[_acehead.aceCompSize]
           add  DWORD PTR FPointer, eax

; Copy relevant fields
           mov  eax, fs:[_acehead.aceUncompSize] 
           mov  DTA.dtaSize, eax
           mov  eax, DWORD PTR fs:[_acehead.aceTime]
           mov  DTA.dtaDateTime, eax
           mov  DTA.dtaAttr, 80h ; Don't show the attributes
                                        
           mov  cx, fs:[_acehead.aceFNameLen]
           mov  si, SIZE _acehead
           call CopyLongFilename

           mov  al, 1
DoneACE:   ret
LoadACE ENDP