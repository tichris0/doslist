; SYS Unit for file lister                      <C> 1992 Created by Dàrk Shàde
CODE

LoadSYS PROC
           mov  cx, SIZE _sysdev
           call ReadFile

           mov  ax, WORD PTR fs:[0] 
           cmp  ax, 'ZM'
           je   SYSisEXE

           cmp  ax, 0FFFFh
           jne  InvalidSYS

           push bx
           call TellFile
           add  di, 160

           mov  si, OFFSET DEVHeader
		   call TellFStr
           pop  bx
           
           xor  cx, cx

ShowSYS:   push bx
           xor  si, si
           call TellDev
           pop  bx

           cmp  WORD PTR fs:[OFFSET _sysdev._sysNext], 0FFFFh
           jne  InvalidSYS

           mov  si, WORD PTR fs:[OFFSET _sysdev._sysNext]
           cmp  si, 0FFFFh
           je   InvalidSYS

           push cx
           mov  ax, 4200h
           xor  cx, cx
           mov  dx, si
           int  21h                     ; Seek the next device
           
           push ds
           push fs
           pop  ds
           mov  ah, 3Fh
           mov  cx, SIZE _sysdev
           xor  dx, dx
           int  21h                     ; Read the header!
           pop  ds
           pop  cx
           jmp  ShowSYS             
           
           
SYSisEXE:  mov  ax, 4200h
           xor  cx, cx
           xor  dx, dx
           int  21h                     ; Reset the read pointer

           push bx
           call LoadEXE
           pop  bx
InvalidSYS:call ShowMore
           ret
LoadSYS ENDP
