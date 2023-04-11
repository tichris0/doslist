.CONST

MoreScr    db '`שת. More .תש''', 0



.CODE

; Procedure to read from a file into fs:ReadBuffer
; @Input
;   cx = Size of the read
; @Modified
;   ax
ReadFile PROC
           push ds
           
           mov  ah, 3Fh
           push fs
           pop  ds
           xor  dx, dx
           int  21h                     ; Read the header!

           pop  ds
           ret
ReadFile ENDP



; Procedure to clear the screen
; @Output
;   di = 0
;   cx = 0
;   ax = 0720h
; @Modifies
;   ax, cx, di
Clear PROC
           xor  di, di
           mov  cx, ScrEnd
           shr  cx, 1
           add  cx, 80
           mov  ax, 0720h
           rep  stosw                   ; Erase the screen, and reset the cur.
           xor  di, di

           ret
Clear ENDP



; Procedure to adjust the cursor to the next line
; @Input
;   di = Screen offset
; @Output
;   di = Adjusted screen offset
; @Modifies
;   cx, di
AdjustDI PROC
           call FixupDI
           call CheckMore

           ret
AdjustDI ENDP



; Procedure to ceil di to the nearest multiple of 160
; @Input
;   di = Screen offset
; @Output
;   di = Adjusted screen offset
; @Modifies
;   ax, cx, di, dx
FixupDI PROC
           mov  cx, 160
           xchg di, ax
           cwd

           add  ax, cx
           dec  ax
           div  cx
           mul  cx
           xchg ax, di
           
           ret
FixupDI ENDP



; Procedure to Pause if needed
; @Input
;   di = Screen coordinate
; @Modifies
;   ax, cx, dx, di
CheckMore PROC
           cmp  di, ScrEnd              ; Are we at the end?
           jb   NoMore

           cmp  More, 0   ; MORE
           jne  ShowMore

           push ds
           xor  cx, cx
           xchg cx, di
           push si                      ; Preserve si
           mov  si, 160
           push es
           pop  ds
           sub  cx, 160                 ; One line will be killed!
           shr  cx, 1
           rep  movsw                   ; Scroll one line up!
           pop  si
           pop  ds
           mov  cx, 80
           mov  ax, 0720h
           rep  stosw                   ; Nuke the last line!
           sub  di, 160

NoMore:    ret
CheckMore ENDP



; Procedure to pause
; @Input
;   di = Screen coordinate
; @Modifies
;   cx, dx, di
ShowMore PROC
           push ax
           push si
           
           mov  ah, 07
           mov  si, OFFSET MoreScr
           call TellStr

           mov  ah, 07h
           int  21h                     ; Wait for a key to be pressed

           cmp  al, 0
           jne  KeyOK
           int  21h                     ; Read the extended key!
KeyOK:     xchg ax, dx

           call Clear

           pop  si
           pop  ax

           cmp  dl, 27
           je   Quit

           ret
ShowMore ENDP



; Procedure to convert String-Numbers to DEC numbers
; @Input
;   ds:si = ASCIIZ String
; @Output
;   eax = Number
; @Modifies
;   eax, edx, cx, si, di 
GetNum PROC
           xor  eax, eax
NextNum:   xor  edx, edx

           mov  dl, [si]
           sub  dl, '0'
           cmp  dl, 9
           ja   NoNum
           
           inc  si
           push dx

           mov  dl, 10
           mul  edx
           pop  dx

           add  eax, edx                ; X * 10 + Y
           jmp  NextNum
           
NoNum:     ret
GetNum ENDP



; Procedure to write a right-justified number to the screen
; @Input
;   ah      = Color
;   bx      = Number
;   cl      = Digit counts
;   es:di   = Screen
; @Modifies
;   al, bx, cx, dx, di 
TellNum PROC
           xor  ch, ch
           add  di, cx
           add  di, cx
           push di
           
           mov  al, cl
           xchg ax, bx          ; ax = Number, bh = Color, bl = Count
           mov  cx, 10
           
TellDigit: xor  dx, dx
           div  cx
           mov  dh, bh
           add  dl, '0'
           sub  di, 2
           mov  WORD PTR es:[di], dx

           dec  bl
           jz   Skip0Pad
           or   ax, ax
           jnz  TellDigit

Pad0:      mov  al, '0'
           mov  ah, bh
           sub  di, 2
           mov  WORD PTR es:[di], ax
           dec  bl
           jnz  Pad0

Skip0Pad:  pop  di
           mov  ah, bh
           ret
TellNum ENDP



; Procedure to write HEX to the screen
; @Input
;   cx      = Length
;   ah      = Color
;   si      = Number
;   es:di   = Screen
; @Modifies
;   al, cx, dx, di, si  
TellHEX PROC
           xor  bh, bh
           
		   add  di, cx
           add  di, cx
           push di
		   sub  di, 2
           std
           mov  dx, si

THexLoop:  mov  al, dl
           and  al, 0Fh
           add  al, '0'
           cmp  al, '9'
           jbe  THexDig
           
           add  al, 'A' - '0' - 10

THexDig:   stosw                        ; Write the character
           shr  dx, 4
           dec  cl
           jnz  THexLoop

           cld
           pop  di
		   ret
TellHEX ENDP



; Procedure to write a comma delimited number in descending order
; @Input
;   ah      = Color
;   esi     = Number
;   es:di   = Screen
; @Output
;   edx     = 0
; @Modifies
;   al, ecx, edx, di, si 
TellSize PROC
           push di
           
           mov  edx, esi

           mov  ch, ah          ; ch  = Color
           mov  esi, 10         ; ecx = Divisor
		   sub  di, 2
           std
           
Tell3Digs: mov  cl, 3           ; cl = Count
           
Tell3Digit:xchg eax, edx
           xor  edx, edx        ; edx:eax = #
           div  esi
           xchg eax, edx        ; edx = Number, al = Digit
           
           mov  ah, ch
           add  al, '0'
           stosw

           or   edx, edx
           jz   TDDone
           
           dec  cl
           jnz  Tell3Digit
           
           mov  al, ','
           stosw
           jmp  Tell3Digs

TDDone:    pop  di
           cld
           ret
TellSize ENDP



; Procedure to write an ASCIIZ string on the screen
; @Input
;   ah      = Color
;   ds:si   = ASCIIZ String
;   es:di   = Screen
; @Modifies
;   al, si, di
TellStr PROC
           lodsb
           or   al, al
           jz   DontWrite
           stosw
           jmp  TellStr

DontWrite: ret
TellStr ENDP



; Procedure to write a right justified string optionally terminated by a char
; @Input
;   ds:si   = String (ASCIIZ or length-based) 
;   es:di   = Screen
;   ah      = Color
;   cl      = Max string length
;   ch      = Terminal character
; @Modifies
;   al, cx, si, di
TellStrN PROC
           lodsb
           cmp  al, ch
           je   SkipTerm
           or   al, al
           je   SkipTerm
           
           dec cl
           stosw
           jmp  TellStrN
           
SkipTerm:  xor  ch, ch
           add  di, cx
           add  di, cx

           ret
TellStrN ENDP



; Print a formatted string to the screen
; @Input
;   ah      = Color
;   es:di   = Screen
;   ds:si   = String
;   bx      = Parameter list
;     Parameters in the string can be:
;       '%# ' : Insert # spaces
;       '%#s' : String
;           Param: dw OFFSET String
;       '%#h' : Hexadecimal 
;           Param: dw Number
;       '%#z' : Comma delimited decimal 
;           Param: dd Number
;   # : Number of digits to allocate to the operation
;       If the number is negative, it will treat it as an absolute value
; @Modifies
;   eax, bx, cx, edx, si, di
FStrLoop:  pop  si              ; Called for each loop iteration

TellFStr PROC
           lodsb
           or   al, al
           jz   TellFEnd



; Check for replace codes
           cmp  al, '%'
           jne  TellFormat

           xor  cx, cx      ; Default parameter to 0
; See if it is paired with a number (allows for 2 digits)
           lodsb
           cmp  al, '-'
           jne  TellFPos
           
           mov  ch, 1
           lodsb
           
TellFPos:  cmp  al, '0'
           jb   CheckTWild
           
           cmp  al, '9'
           ja   CheckTWild
           
           sub  al, '0'
           mov  cl, al      ; cl = Digit
           lodsb

           cmp  al, '0'
           jb   CheckTWild  
           cmp  al, '9'
           ja   CheckTWild
           
           sub  al, '0'
           add  al, cl
           add  al, cl
           
           shl  cl, 3       ; al = 8 * al
           add  cl, al      ; bl = (2 * al) + (8 * al) = 10 * al 
           
           lodsb
           
; Allowed replace codes            
CheckTWild:push si

           cmp  al, 'h'
           je   TellFHex

           cmp  al, 'z'
           je   TellFSize

           cmp  al, 's'
           je   TellFString

           cmp  al, 'S'
           je   TellF2String

           cmp  al, 'c'
           je   TellFChar

           cmp  al, ' '
           je   TellFSpace

           jmp  TellFStr    ; Illegal command, just keep going



; Check for formatting codes
TellFormat:cmp  al, '\'
           jne  TellRaw

           lodsb
           cmp  al, '0'
           jb   TellRaw
           
           cmp  al, '9'
           ja   TellExtCol



; Process the color formatter
           sub  al, '0'
           mov  ah, al
           jmp  TellFStr
           
TellExtCol:cmp  al, 'A'
           jb   TellRaw
           
           cmp  al, 'F'
           ja   TellEscape
           
           sub  al, 'A' - 10
           mov  ah, al
           jmp  TellFStr



; Line feed formatter
TellEscape:cmp  al, 'n'
           jne  TellTab
           
           call AdjustDI
           jmp  TellFStr

TellTab:   cmp  al, 't'
           jne  TellRaw
           
TellRaw:   stosw
           jmp  TellFStr



; Character print
TellFChar: mov  al, ss:[bx]
           stosw
           add  bx, 2
           
           jmp  FStrLoop



; Hexadecimal print %#h, #: Number of nibbles to display
TellFHex:  xor  ch, ch

           mov  si, WORD PTR ss:[bx]
           add  bx, 2

           call TellHex
           
           jmp  FStrLoop



; Size print %#z, #: Number of digits to reserve for printing
TellFSize: or   ch, ch
           jz   TellSJump   

           push cx
           call FixupDI
           pop  cx
           sub  di, 160 
           xor  ch, ch
           dec  cl

TellSJump: add  di, cx
           add  di, cx

           mov  esi, DWORD PTR ss:[bx]
           add  bx, 4
           call TellSize

           jmp  FStrLoop



; Insert a bunch of spaces
TellFSpace:mov  si, OFFSET Zero
           push ds
           jmp  TellRegStr          ; Use the general string function to do it


TellF2String: 
           push ds
           int 3h
           mov  ds, WORD PTR ss:[bx]
           add  bx, 2
           jmp  SkipDS
                       
           
; String print %#s, #: Total number of 
TellFString:
           push ds
SkipDS:    mov  si, WORD PTR ss:[bx]
           add  bx, 2
          
TellRegStr:mov  dx, si

FindStrEnd:mov  al, [si]
           inc  si           
           or   al, al
           jnz  FindStrEnd

           dec  si
           sub  si, dx
           xchg si, dx              ; dx = Length
           
           xchg dl, cl  
           sub  dl, cl
           jnc  SkipClip

           call TellStr
           jmp  TellFSEnd


PrePadSpc: add  di, dx
           add  di, dx

           call TellStr
           jmp  TellFSEnd


SkipClip:  or   ch, ch
           jz   PrePadSpc     ; Pre-pend or Post-pend the spaces?

           push cx
           push dx
           call FixupDI
           pop  dx
           pop  cx
           sub  di, 160
           add  di, dx
           add  di, dx          ; Set to the appropriate col
           
           call TellStr
           
           or   ch, ch
           jnz  TellFSEnd       ; Should we post-pend spaces?  
           
           add  di, dx
           add  di, dx                      
           
TellFSEnd: pop  ds
           jmp  FStrLoop

TellFEnd:  ret
TellFStr ENDP
