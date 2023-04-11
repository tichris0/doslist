; TXT Unit for file lister                      <C> 1992 Created by Dàrk Shàde
CODE

LoadTXT PROC
           xor  di, di                  ; Destination is at es:0
           push si 
	       push bx 
	       push cx
           call TellFile
           pop  cx
           pop  bx
	       pop  si

           jmp  ReadData

LoopScr:   mov  al, fs:[si]

           cmp  al, 7                   ; Skip the nagging beep!
           jb   WriteChar
           cmp  al, 11
           je   WriteChar
           cmp  al, 12
           je   WriteChar
           cmp  al, 13
           ja   WriteChar

           cmp  al, 10
           jne  Not10                   ; We've found <lf>
           call NewLine
           cmp  BYTE PTR fs:[si + 1], 13
           jne  CheckLn                ; It's actually <lf/cr> "… la Unix"
           inc  si
           dec  cx
           jz   ReadData
           jmp  CheckLn

Not10:     cmp  al, 13
           jne  Not13                   ; We've found <cr>
           call NewLine
           cmp  BYTE PTR fs:[si + 1], 10
           jne  CheckLn                ; Actually <cr/lf> "… la DOS"
           inc  si
           dec  cx
           jz   ReadData
           jmp  CheckLn

Not13:     cmp  al, 08
           jne  CheckNxt
           or   di, di
           jz   CheckNxt
           dec  si

           jmp  CheckNxt


WriteChar: mov  es:[di], ax

CheckNxt:  add  di, 2
CheckLn:   inc  si
           cmp  di, ScrEnd
           jne  NotFull
           push si 
           push bx
           push cx
           call ShowMore
           call TellFile
           pop  cx
           pop  bx 
           pop  si
           mov  ah, 07h

NotFull:   dec  cx
           jz   ReadData
           jmp  LoopScr


NewLine:   push cx
           call FixupDI
           pop  cx
           ret



ReadData:  mov  cx, 16384
           call ReadFile
           or   ax, ax
           jz   PCheckNext              ; There's no more to read =(
           
           mov  cx, ax                  ; cx = Length
           xor  si, si
           mov  ah, 07                  ; ah = Color
           jmp  LoopScr

PCheckNext:call FixupDI
           ret
LoadTXT ENDP
