; VESA Unit for file lister                     <C> 1992 Created by Dàrk Shàde
.CONST

NoVESA     db 'VESA compliancy not found', 0
NotMode    db 'VESA Mode not supported by your VGA card', 0

X8Bit      dw 1024, 800, 640, 640, 320
Y8Bit      dw  768, 600, 480, 400, 200
VGASeg     dw 0A000h



.DATA?

Index      db ?
CurPage    dw ?
Header     db 256 dup(?)
MaxX       dw ?
MaxY       dw ?



.CODE

GetVESANFO:mov  ax, 4F00h
           push ds
           pop  es
           mov  di, OFFSET Header
           int  10h                     ; Returns AL = 4Fh if no error
           cmp  al, 4Fh
           je   IsVESA
           mov  si, OFFSET NoVESA
           call TellStr
           mov  ax, 4c00h
           int  21h

IsVESA:    ret


SetMode:   mov  ax, 4F02h               ; In bx = Mode
           mov  cx, bx
           int  10h
           cmp  al, 4Fh
           je   GoodMode

           mov  si, OFFSET NotMode
           call TellStr
           mov  ax, 4c00h
           int  21h

GoodMode:  mov  CurPage, 0

           mov  ax, 4F01h
           push ds
           pop  es
           mov  di, OFFSET Header
           int  10h                     ; Get Mode info
           mov  es, VGASeg
           ret


SetPage:   xor  bx, bx                  ; In DX = Page
           call DWORD PTR [Header+12]
           ret


SetRGB:    mov  dx, 3dah                ; In al = Color, cl;bx = Color
           out  dx, al

           mov  al, cl
           out  dx, al
           mov  al, bh
           out  dx, al
           mov  al, bl
           out  dx, al
           ret


; Draw pixel by pixel to the virtual screen!
PutPixel8: mov  edx, edi                ; In es = ScrSeg, al = Color, edi = Offs
           shr  edx, 16                 ; Put the page number in dx
           cmp  dx, CurPage
           je   DoPixel8
           call SetPage
DoPixel8:  mov  es:[di], al
           ret


PutPixel16:mov  edx, edi
           shr  edx, 16
           cmp  dx, CurPage
           je   DoPixel16
           call SetPage
DoPixel16: mov  es:[di], ax
           ret


PutPixel24:mov  edx, edi
           shr  edx, 16
           cmp  dx, CurPage
           je   DoPixel24
           call SetPage
DoPixel24: mov  es:[di], ax
           shr  eax, 16
           mov  es:[di + 2], al
           ret


; Determine the optimum 8bit screen mode
Optimize8: cmp  cx, 1024
           jbe  DeSize81
           cmp  dx, 768
           jbe  DeSize81
           mov  ax, 0

DeSize81:  cmp  cx, 800
           jbe  DeSize82
           cmp  dx, 600
           jbe  DeSize82
           mov  ax, 1

DeSize82:  cmp  cx, 640
           jbe  DeSize83
           cmp  dx, 480
           jbe  DeSize83
           mov  ax, 2

DeSize83:  cmp  cx, 640
           jbe  DeSize84
           cmp  dx, 400
           jbe  DeSize84
           mov  ax, 3

DeSize84:  cmp  cx, 320
           jbe  SetupI8
           cmp  dx, 200
           jbe  SetupI8
           mov  ax, 4

SetupI8:   ;mov  Index, ax
           add  ax, ax
           mov  si, ax
           mov  cx, cs:[X8Bit + si]
           mov  dx, cs:[Y8Bit + si]
           mov  MaxX, cx
           mov  MaxY, dx

           call SetMode

           ret
