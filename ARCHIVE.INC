; Archive Unit for file lister                  <C> 1992 Created by Dàrk Shàde
.DATA?

FPointer   dd   ?                       ; File read pointers



.CODE

; Archive handler
; @Input
;   si = Handler
TellArchive PROC
           LOCAL Handler: WORD = AUTO_SIZE
           
           push bp
           mov  bp, sp
           sub  sp, AUTO_SIZE
           
           mov  Handler, si

           xor  eax, eax 
           mov  FPointer, eax
           mov  TotUsed, eax
           mov  TotFiles, ax
           mov  TotDirs, ax
           push bx
           xor  dx, dx
           call TellHead
           
           mov  ax, 0700h + '\'
           stosw
           mov  si, OFFSET DTA.dtaName
           call TellStr
           call AdjustDI
           pop  bx
           
           add  di, 160
           call CheckMore
           
TellALoop: mov  ax, 4200h
           mov  cx, WORD PTR [OFFSET FPointer + 2]
           mov  dx, WORD PTR [OFFSET FPointer]
           int  21h                     ; Set file pointer!

           mov  ah, 3Fh
           xor  dx, dx
           
           push di
           call Handler
           pop  di

           or   al, al
           jz   DoneArch
           
           push bx
           
           call TellFile
           call AdjustDI
           call CheckMore
           
           pop  bx

           jmp  TellALoop
           
DoneArch:  mov  sp, bp
           pop  bp
  
           ret
TellArchive ENDP



