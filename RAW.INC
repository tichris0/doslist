.CODE

; Procedure to view a file
ViewRAW PROC
           LOCAL ReadAt : DWORD, ReadSize : WORD, RowSize : WORD = AUTO_SIZE

           push bp
           mov  bp, sp
           sub  sp, AUTO_SIZE
           
           mov  ax, 3D00h
           mov  dx, OFFSET DTA.dtaName
           int  21h                     ; Open the file for reading only!
           jc   ViewDone 
           xchg ax, bx                  ; bx = File Handle from last int 21h

           mov  ReadAt, 0

           xor  cx, cx                  ; Process 0 byte 
           mov  si, Command
           call [si + _cmd.CmdSubHand]

           mov  dx, 160
           shr  dx, cl
           mov  RowSize, dx             ; Set the ideal row size
           
           mov  dx, ScrEnd
           sub  dx, 160
           shr  dx, cl
           mov  ReadSize, dx            ; Set the ideal read size

           cmp  JumpAddr, 0
           je   ViewPage

           mov  ax, 4200h
           mov  cx, WORD PTR ds:[JumpAddr + 2]
           mov  dx, WORD PTR ds:[JumpAddr]
           int  21h

ViewPage:  push bx
           xor  di, di
           call TellFile
           pop  bx


; Read a page full of data in memory
           xor  si, si
           mov  cx, ReadSize
           call ReadFile
           
           mov  cx, ax                  ; cx = Length
           or   cx, cx
           jz   ViewDone                ; File is 0 byte of length!

           push bx

           mov  bx, Command
           call [bx + _cmd.CmdSubHand]

           call FixupDI
           call ShowMore
           pop  bx

CheckEnter:cmp  dl, 0Dh
           je   ViewDone                    ; Skip entire file
           
CheckHome: cmp  dl, 47h
           jne  CheckEnd                    ; Seek begining of the file    
           
           mov  ReadAt, 0
           jmp  JumpFile

CheckEnd:  cmp  dl, 04Fh
           jne  CheckUp

           xor  edx, edx
           mov  eax, DTA.dtaSize
           push eax
           xor  ecx, ecx
           mov  cx, RowSize
           div  ecx                         ; edx = Remainder           

           pop  eax
           sub  eax, edx                    ; Filesize - Remainder
           add  eax, ecx                    ; += RowSize
           
           mov  cx, ReadSize
           sub  eax, ecx                    ; -= ReadSize
           mov  ReadAt, eax
           jmp  JumpFile

CheckUp:   cmp  dl, 50h
           jne  CheckDown
           
           mov  ax, RowSize 
           add  ReadAt, eax                 ; Move one row up
           jmp  JumpFile
           
CheckDown: cmp  dl, 48h
           jne  CheckPgUp

           mov  ax, RowSize 
           sub  ReadAt, eax                 ; Move one row down
           jmp  JumpFile

CheckPgUp: cmp  dl, 49h
           jne  CheckPgDn
           
           mov  ax, ReadSize 
           sub  ReadAt, eax
           jmp  JumpFile           

; This is the default
CheckPgDn:; cmp  dl, 51h
          ; jne  SkipJump
           
           mov  ax, ReadSize
           add  ReadAt, eax

JumpFile:  jnc  JumpOffset  
           mov  ReadAt, 0

JumpOffset:mov  ax, 4200h
           mov  cx, WORD PTR [ReadAt + 2]
           mov  dx, WORD PTR [ReadAt]
           int  21h

           jmp  ViewPage
           
ViewDone:  mov  ah, 3Eh
           int  21h                     ; Close current file

           mov  sp, bp
           pop  bp
           ret
ViewRAW ENDP
