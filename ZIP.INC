; ZIP Unit for file lister                      <C> 1992 Created by Dàrk Shàde
.CODE

_ziphead STRUC
    zipHeadID   dd ?                ; Local file header signature "PK\3\4"
    zipVersion  dw ?                ; Version needed to extract
    zipBitFlags dw ?                ; General purpose bit flag
    zipMethod   dw ?                ; Compression method (0 = None, 8 = Deflate)
    zipLMTime   dw ?                ; File last modification time
    zipLMDate   dw ?                ; File last modification date
    zipCRC32    dd ?                ; CRC32 of uncompressed data
    zipCompSize dd ?                ; Compressed size
    zipUncpSize dd ?                ; Uncompressed size
    zipFNameLen dw ?                ; Filename length
    zipExtraLen dw ?                ; Extra field length
;   zipFilename db ? dup(?)         ; Filename
_ziphead ENDS



LoadZIP PROC
           mov  cx, 128 + SIZE _ziphead
           push ds
           push fs
           pop  ds
           int  21h                     ; Read the header!
           pop  ds

           xor  al, al                  ; Mark end of search
           cmp  fs:[_ziphead.zipHeadID], 'KP'
           jne  DoneZIP                 ; Quit now, there no file here!

; Jump to the next file
           xor  eax, eax
           mov  ax, fs:[_ziphead.zipExtraLen]
           add  eax, fs:[_ziphead.zipCompSize]
           add  eax, SIZE _ziphead

           xor  edx, edx
           mov  dx, fs:[_ziphead.zipFNameLen]
           add  eax, edx                ; Pointer += Name Length
           add  FPointer, eax
 
; Copy relevant fields
           mov  eax, fs:[_ziphead.zipUncpSize]
           mov  DTA.dtaSize, eax
           mov  eax, DWORD PTR fs:[_ziphead.zipLMTime]
           mov  DTA.dtaDateTime, eax
           mov  DTA.dtaAttr, 80h  ; Don't show the attributes
           
           mov  si, SIZE _ziphead
           mov  cx, fs:[_ziphead.zipFNameLen]
           call CopyLongFilename

           mov  al, 1                   ; Scan for more files
DoneZip:   ret
LoadZIP ENDP
