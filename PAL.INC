; PAL Unit for file lister                      <C> 1992 Created by Dàrk Shàde

LoadPAL:   mov  cx, 4000
           call ReadFile
           cmp  ax, 768
           jbe  Check4
           mov  si, ax

           mov  ah, 07
           int  21h
           ret

Check4:    cmp  BYTE PTR ds:[si], 64
           ja   PalOK
           dec  si
           jnz  Check4

PALOK:     or   si, si
           jz   IsNot4

           mov  si, 768
FourLoop:  shr  BYTE PTR ds:[si], 2
           dec  si
           jnz  FourLoop

IsNot4:
           mov  ax, 13h
           int  10h

           mov  dx, 03C8h
           xor  al, al
           out  dx, al
           inc  dx

           mov  cx, 768
           xor  si, si
           rep  outsb

           mov  ax, VGASEG
           mov  es, ax
           xor  ax, ax

XLoop:     mov  di, ax
           add  di, 25 * 320 + 32
           mov  dx, 150

YLoop:     mov  es:[di], al
           add  di, 320
           dec  dx
           jnz  YLoop

           inc  ax
           cmp  ax, 256
           jne  XLoop

           mov  ah, 07
           int  21h
           mov  ax, 03
           int  10h
           xor  di, di
           ret