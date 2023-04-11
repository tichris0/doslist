; LHA Unit for file lister                      <C> 1992 Created by Dàrk Shàde
.CODE

_lhahead STRUC
    lhaHeadSize     db ?            ; Size of the file header
    lhaChecksum     db ?            ; Checksum of remaining bytes
    lhaID           db 3 dup(?)     ; '-lz' or '-lh'
    lhaCompression  db ?            ; Compression method
    lhaID2          db ?            ; '-'
    lhaCompSize     dd ?            ; Compressed size
    lhaUncompSize   dd ?            ; Uncompressed size
    lhaTime         dw ?            ; File time
    lhaDate         dw ?            ; File date
    lhaAttrib       dw ?            ; File attribute
    lhaFNameLen     db ?            ; Filename length
;   lhaCRC16        dw ?            ; CRC-16 of original file
;   lhaFilename     db ? dup(?)     ; Filename
_lhahead ENDS



LoadLHA PROC
           mov  cx, 128 + SIZE _lhahead + 2
           push ds
           push fs
           pop  ds
           int  21h                     ; Read the header!
           pop  ds

           xor  al, al
           cmp  BYTE PTR fs:[_lhahead.lhaHeadSize], 0
           je   DoneLHA                  ; No more files!

; Jump to the next file
           xor  eax, eax
           mov  al, fs:[_lhahead.lhaHeadSize]
           add  ax, 2
           add  eax, fs:[_lhahead.lhaCompSize]
           add  FPointer, eax; Calculate the next file's pos.

; Copy relevant fields
           mov  eax, fs:[_lhahead.lhaUncompSize]
           mov  DTA.dtaSize, eax
           mov  eax, DWORD PTR fs:[_lhahead.lhaTime]
           mov  DTA.dtaDateTime, eax
           mov  DTA.dtaAttr, 80h ; Don't show the attributes

           mov  cl, fs:[_lhahead.lhaFNameLen]
           mov  si, SIZE _lhahead
           call CopyLongFilename

           mov  al, 1                   ; Scan for more files
DoneLHA:   ret
LoadLHA ENDP