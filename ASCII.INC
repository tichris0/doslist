; ASCII Unit for file lister                    <C> 1992 Created by Dàrk Shàde
.CODE

; Procedure to show the content of a file in ASCII format
ShowASCII PROC
           jcxz ASCIISetup
           
           mov  ah, 07h
           
ASCIIRead: mov  al, fs:[si]
           inc  si
           stosw

           dec  cx
           jz   ASCIIDone
           
;           cmp  dx, di
           jmp  ASCIIRead

ASCIIDone: ret

ASCIISetup:mov  cl, 1
           ret


ShowASCII ENDP
