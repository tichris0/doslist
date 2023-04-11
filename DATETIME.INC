; DATETIME Unit for file lister                 <C> 1992 Created by Dàrk Shàde
; Functions to modify a file's date/time
.DATA?

NewDate    dw   ?
NewTime    dw   ?



.CODE

; Procedure to set the Parse the date arguments
PrepDate PROC
           mov  Command, di

           inc  si
           cmp  BYTE PTR [si], '*'
           jne  ParseDate
           inc  si
           
           mov  ah, 2Ah
           int  21h

           sub  cx, 1980
           shl  cx, 9

           xor  al, al
           mov  ah, dh
           shr  ax, 3
           
           or   al, dl
           add  ax, cx
           mov  NewDate, ax
           ret

           
           
ParseDate: call GetNum
           sub  ax, 1980
           shl  ax, 9
           mov  cx, ax                  ; cx = Date

           inc  si
           call GetNum
           shl  ax, 5
           or   cx, ax

           inc  si
           call GetNum
           or   cx, ax     
           mov  NewDate, cx             ; Setup the new date in Y:M:D format!
           ret
PrepDate ENDP           

           

; Procedure to parse the time arguments
PrepTime PROC
           mov  Command, di
           mov  NewDate, 0

           inc  si
           cmp  BYTE PTR [si], '*'
           jne  ParseTime  
           inc  si

           mov  ah, 2Ch
           int  21h

           shl  ch, 3
           xor  ah, ah
           mov  al, cl
           
           shl  ax, 5
           shr  dh, 1
           or   al, dh
           or   ah, ch
           mov  NewTime, ax
           ret



ParseTime: call GetNum
           shl  ax, 0Bh
           mov  cx, ax

           inc  si
           call GetNum
           shl  ax, 5
           or   cx, ax

           inc  si
           call GetNum
           shr  ax, 1                   ; Sec / 2
           
           or   cx, ax                  ; Setup the new date in Y:M:D format!
           mov  NewTime, cx
           ret
PrepTime ENDP



; Procedure to set the date & time (date = 0   =>   setTime, else setDate) 
CastDateTime PROC
           mov  si, OFFSET AttrStr      ; Default => Failed to change
           
           mov  ax, 3D00h
           mov  dx, OFFSET DTA.dtaName
           int  21h                     ; Open file using handle
           xchg ax, bx
           jc   NotChg

           mov  ax, 5701h
           mov  cx, DTA.dtaTime
           mov  dx, DTA.dtaDate
           
           cmp  NewDate, 0
           jne  ChangeDate

           mov  cx, NewTime
           jmp  ChangeDT

ChangeDate:mov  dx, NewDate

ChangeDT:  int  21h
           jc   NotChg
           add  si, 4

NotChg:    mov  ah, 3Eh
           int  21h                     ; Close File
           
           ret
CastDateTime ENDP
