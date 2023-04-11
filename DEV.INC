; Device Unit for file lister                   <C> 1992 Created by Dàrk Shàde
.CONST
DEVHeader  db '\F ID  Seg   Attr  Strat  Int  Name\n \n', 0
DEVEntry   db '\7 %2h  %4h  %4h  %4h  %4h  ', 0
DEVBlock   db '%c: - %c:', 0



.CODE

; System Device
_sysdev STRUC
    _sysNext        dd ?        ; Pointer to the next driver (ofs = -1 if last)
    _sysAttr        dw ?        ; Device attributes
    _sysStrat       dw ?        ; Device strategy
    _sysInt         dw ?        ; Device interrupt
    UNION
        _sysDrives  dw ?        ; Number of Drivess supported (BLOCK device)
        _sysName    db 8 dup(?) ; Device driver name
    ENDS    
_sysdev ENDS

LoadDEV PROC
           xor  di, di
           mov  si, OFFSET DEVHeader
		   call TellFStr

           mov  ah, 52h
           int  21h
           add  bx, 22h
           push es
           pop  fs                      ; fs = Device Driver segment
           mov  si, bx
           mov  es, ScrSeg

           xor  cx, cx

ShowDEV:   call TellDEV

           ; Load the next device in fs:si
           push DWORD PTR fs:[si + OFFSET _sysdev._sysNext]
           pop  si
           pop  fs

           cmp  si, 0FFFFh
           jne  ShowDEV

           jmp  Quit
LoadDEV ENDP



           
           

; Procedure to write the device driver details
; @Input 
;   fs:si = Device driver header
;   cx    = Device indices (Start at 0)
; @Output
;   cx    = Next device indices
TellDEV PROC
           LOCAL DEVCount: BYTE, BDEVCount: BYTE, DEVName: BYTE: 9 = AUTO_SIZE

           push bp
           mov  bp, sp
           sub  sp, AUTO_SIZE
           
           mov  DEVCount, cl
           mov  BDEVCount, ch
           
           push fs:[si + _sysdev._sysInt]
           push fs:[si + _sysdev._sysStrat]
           push fs:[si + _sysdev._sysAttr]
           push fs
           xor  ch, ch
           push cx
           mov  bx, sp
           
           push si
           mov  si, OFFSET DEVEntry
           call TellFStr 
           pop  si
           mov  sp, bx

           push si
           push ds

           

; Device Name
           test WORD PTR fs:[si + _sysdev._sysAttr], 08000h
           jnz  DevCharDev 

; It's a logical device
; Increase the starting drive
           mov  al, BDEVCount
           add  al, 'A'
           mov  dl, al

           add  al, fs:[si + OFFSET _sysdev._sysDrives]
           push ax
           push dx
           
           sub  al, 'A'
           mov  BDEVCount, al
          
           mov  si, OFFSET DEVBlock 
           jmp  DevNotDev

; It's a character device
DevCharDev:mov  eax, fs:[si + OFFSET _sysdev._sysName]
           mov  edx, fs:[si + OFFSET _sysdev._sysName + 4]

           mov  si, OFFSET DEVName
           mov  DWORD PTR ss:[si], eax
           mov  DWORD PTR ss:[si + 4], edx
           mov  BYTE PTR ss:[si + 8], 0

           mov  ah, 3
           push ss
           pop  ds
           
DevNotDev: mov  bx, sp
           call TellFStr
           mov  sp, bx
           pop  ds
           pop  si 
           call AdjustDI

           mov  cl, DEVCount
           inc  cl
           mov  ch, BDEVCount
           
           mov  sp, bp
           pop  bp

           ret
TellDEV ENDP          
