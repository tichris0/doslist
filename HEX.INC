; HEX Unit for file lister                      <C> 1992 Created by Dàrk Shàde
.CODE
; Procedure to show the content of a file in HEX format
ShowHex PROC
           jcxz HexSetup
           
           mov  ah, 07h

HexRead:   mov  ax, 0720h
           stosw

           mov  al, fs:[si]
           inc  si
           push ax
           shr  al, 4
           add  al, '0'
           cmp  al, '9'
           jbe  HexDig_1
           
           add  al, 'A' - '0' - 10

HexDig_1:  mov  ah, 07h
           stosw

           pop  ax
           and  al, 1111b
           add  al, '0'
           cmp  al, '9'
           jbe  HexDig_2
           
           add  al, 'A' - '0' - 10

HexDig_2:  mov  ah, 07h
           stosw

           mov  ax, 0720h
           stosw

           dec  cx
           jz   HexDone
           
           jmp  HexRead
           
HexDone:   ret

HexSetup:  mov  cl, 3
           ret
ShowHex ENDP
