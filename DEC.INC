; DEC Unit for file lister                      <C> 1992 Created by Dàrk Shàde
.CODE
; Procedure to show the content of a file in DEC format
ShowDec PROC
           jcxz DecSetup
           
           mov  ah, 07h
           
DecRead:   mov  al, fs:[si]
           inc  si
           push ax
           push ax

           aam
           xchg al, ah
           aam
           xchg al, ah
           add  al, 48
           mov  ah, 07
           stosw

           pop  ax
           aam
           xchg al, ah
           aam
           add  al, 48
           mov  ah, 07
           stosw

           pop  ax
           aam
           add  al, 48
           mov  ah, 07
           stosw                        ; Write the 3 values
           mov  al, 32
           stosw

           dec  cx
           jz   DecDone
           
;           cmp  dx, di
           jmp  DecRead
           
DecDone:   ret

DecSetup:  mov  cl, 3
           ret
ShowDec ENDP
