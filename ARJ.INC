; ARJ Unit for file lister                      <C> 1992 Created by Dàrk Shàde
.CODE

_arjhead STRUC
    arjHeadID           dw ?        ; Local file header signature 0EA60h
    arjBasicHeadSize    dw ?        ; Basic header size till end of comment (0 for the end)
    arjHeadSize         db ?        ; Header size
    arjArchVersion      db ?        ; Archiver version number
    arjMinVersion       db ?        ; Minmum version required
    arjHost             db ?        ; Host OS enum
    arjFlags            db ?        ; Internal flag bitmap
    arjCompression      db ?        ; Comprsesion method enum
    arjFiletype         db ?        ; Filetype enum
    arjRes1             db ?        ; Reserved
    arjTime             dw ?        ; File time
    arjDate             dw ?        ; File date
    arjCompSize         dd ?        ; Compressed size
    arjUncompSize       dd ?        ; Uncompressed size
    arjFileCRC32        dd ?        ; CRC32 of uncompressed data
    arjFilespec         dw ?        ; Filespec position in filename
    arjAttrib           dw ?        ; File attributes
    arjHostData         dw ?        ; Host data
;   arjHostExtraData    dd          ; Present when EXTFILE_FLAG is set
;   arjFilename         db ? dup(?) ; File Starting position
;   arjComment          db ? dup(?) ; Comment related to the file
_arjhead ENDS
; following the below header
_arjsuffix STRUC
    arjHeaderCRC32      dd ?        ; Header's CRC32
    arjExtHeaderSize    dw ?        ; Extended header size (never used, = 0)
;   arjExtHeader        db ? dup(?) ; Extended header (never used & only if used)
;   arjExtHeaderCRC32   dd ?        ; Extended header's CRC (never used & only if used)    
_arjsuffix ENDS


LoadARJ PROC
           mov  cx, 128 + SIZE _arjhead + SIZE _arjsuffix + OFFSET _arjhead.arjHeadSize
           push ds
           push fs
           pop  ds
           int  21h                     ; Read the header!
           pop  ds

           xor  al, al

           cmp  WORD PTR fs:[_arjhead.arjHeadID], 0EA60h 
           jne  DoneARJ                 ; Quit now, there's no file here!

           cmp  WORD PTR fs:[_arjhead.arjBasicHeadSize], 0
           je   DoneARJ

           xor  eax, eax
           mov  ax, WORD PTR fs:[_arjhead.arjBasicHeadSize]
           add  eax, SIZE _arjsuffix + OFFSET _arjhead.arjHeadSize
           
; The main header is unique
           cmp  FPointer, 0
           jnz  ARJSkipHd

           add  eax, fs:[_arjhead.arjUncompSize]
           mov  FPointer, eax

           mov  ax, 4200h
           mov  cx, WORD PTR [OFFSET FPointer + 2]
           mov  dx, WORD PTR [OFFSET FPointer]
           int  21h                     ; Set file pointer!

           mov  ah, 3Fh
           xor  dx, dx
           jmp  LoadARJ

; Handle the local headers
ARJSkipHd: ; Jump to the next file
           add  eax, fs:[_arjhead.arjCompSize]
           add  DWORD PTR FPointer, eax
           
; Copy relevant fields
           mov  eax, fs:[_arjhead.arjUncompSize]
           mov  DTA.dtaSize, eax
           mov  eax, DWORD PTR fs:[_arjhead.arjTime]
           mov  DTA.dtaDateTime, eax
           mov  DTA.dtaAttr, 80h ; Don't show the attributes
 
           mov  cl, fs:[_arjhead.arjHeadSize] 
           mov  si, cx
           add  si, OFFSET _arjhead.arjHeadSize
           mov  cl, 0FFh
           call CopyLongFilename

           mov  al, 1
DoneARJ:   ret
LoadARJ ENDP
