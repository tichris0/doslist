; OBJ Unit for file lister                      <C> 1992 Created by Dàrk Shàde
; Uncomplete
.CONST

; OBJ strings
TitleStr   db 'Title : ', 0
CommStr    db 'Comment : ', 0
OBJStr     db 'Includes file : ', 0



.CODE
; Not even near to finished!

LoadOBJ PROC
           mov  cx, 12000
           call ReadFile
           mov  cx, ax

           xor  di, di

           call TellFile

           mov  di, 320
           xor  bx, bx

ScanOBJ:   call AdjustDI
           call CheckMore
           mov  al, fs:[bx]
           mov  cx, fs:[bx + 1]         ; cx = Length
           add  bx, 3                   ; Bypass record Type & Length

           cmp  al, 80h
           je   ScanTitle               ; Borland Title

           cmp  al, 88h
           je   ScanComm                ; Borland comments

           cmp  al, 0A0h
           je   ObjDone

           add  bx, cx                  ; Fixup the length and keep on!
           jmp  ScanOBJ


WriteStr:  mov  ah, 07
           call TellStr
           mov  si, fs:[bx]             ; al = Length
           and  si, 0FFh
           mov  BYTE PTR fs:[bx + 1 + si], 0
           mov  si, bx
           inc  si
           call TellStr
           mov  bx, si
           jmp  ScanOBJ


ScanTitle: mov  si, OFFSET TitleStr
           jmp WriteStr


ScanComm:  mov  ax, fs:[bx]             ; al = Type, ah = Class

           cmp  cx, 6
           jbe  NotComm

           or   ax, ax
           jnz  NotPlain

           add  bx, 2
           mov  si, OFFSET CommStr
           jmp  WriteStr

NotPlain:  cmp  ax, 0E940h
           jne  NotBor

           add  bx, 6
           mov  si, OFFSET OBJStr
           jmp  WriteStr

NotBor:    ;int  3h
           cmp  ax, 0E900h
           jne  NotBC

           add  bx, 6
           mov  si, OFFSET OBJStr
           jmp  WriteStr

NotBC:

NotComm:   add  bx, cx                  ; Nothing we know!  Simply bypass it!
           jmp  ScanOBJ
     
ObjDone:   pop  bx
           ret
LoadOBJ ENDP
