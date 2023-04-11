; Modification Unit for file lister             <C> 1992 Created by Dàrk Shàde
.CODE

; Handler to modify a file
ModifyFile PROC
           call TellFileName

           mov  ah, 07h
           mov  si, Command
           add  di, 2
           call [si + _cmd.CmdSubHand]

           mov  ah, 07
           call TellStr
           call AdjustDI
           call CheckMore

           ret
ModifyFile ENDP



