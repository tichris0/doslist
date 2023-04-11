.CODE

; Procedure to show the full directory tree
ShowTree PROC
           mov  si, dx                  ; si = Directory
           
           mov  dx, di                  ; Preserve di
           add  dx, 160

           mov  bh, bl
           or   bh, bh
           jz   NotTree
           
           dec  bh
           jz   PrintTree   
           
           dec  bh
           jz   OneBranch
           
TreeChar:  mov  eax, 072007B3h
           stosd
           mov  al, 20h                 ; Write "³   "
           stosd
           dec  bh
           jnz  TreeChar

OneBranch: mov  eax, 07C407C3h
           stosd                        ; Write "ÃÄÄÄ"
           inc  al
           stosd

PrintTree: mov  ah, 07
           call TellStr
           call AdjustDI

NotTree:   ret
ShowTree ENDP
