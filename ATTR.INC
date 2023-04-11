; Attributes Unit for file lister               <C> 1992 Created by Dàrk Shàde
.CONST

AttrStr    db 'NOT changed', 0



.DATA

AttribMask db   3Fh
AttribAdd  db   0



.CODE

; Procedure to set the attributes
PrepAttr PROC
           lodsb
           cmp  al, ' '
           je   PrepAttr
           cmp  al, '+'
           je   AddFlag
           cmp  al, '-'
           je   SubFlag

           mov  Command, di
           dec  si
           ret



; Check for addition attrributes 
AddFlag:   lodsb                        ; Step1 : Remove the filter bit
           cmp  al, 'R'
           jne  NotRA
           or   AttribAdd, 1

NotRA:     cmp  al, 'A'
           jne  NotAA
           or   AttribAdd, 20h

NotAA:     cmp  al, 'S'
           jne  NotSA
           or   AttribAdd, 4

NotSA:     cmp  al, 'H'
           jne  PrepAttr
           or   AttribAdd, 2
           jmp  PrepAttr



; Check for subtraction attributes
SubFlag:   lodsb                        ; Step1 : Remove the filter bit
           cmp  al, 'R'
           jne  NotRS
           and  AttribMask, 0FFh - 1

NotRS:     cmp  al, 'A'
           jne  NotAS
           and  AttribMask, 0FFh - 20h

NotAS:     cmp  al, 'S'
           jne  NotSS
           and  AttribMask, 0FFh - 4

NotSS:     cmp  al, 'H'
           jne  PrepAttr
           and  AttribMask, 0FFh - 2
           jmp  PrepAttr
PrepAttr ENDP



CastAttr PROC
           mov  si, OFFSET AttrStr

           xor  ch, ch
           mov  cl, DTA.dtaAttr
           and  cl, AttribMask
           add  cl, AttribAdd
           
           mov  ax, 4301h
           mov  dx, OFFSET DTA.dtaName
           int  21h
           jc   IsB21
           add  si, 4

IsB21:     ret
CastAttr ENDP
