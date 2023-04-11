; XMS Unit for file lister                      <C> 1992 Created by Dàrk Shàde
.DATA?

XMSLib     dd ?                     ; XMS Function dispatcher

; The Move buffer for XMS
_xmsMemMove STRUC
    xmsLength  dd ?
    xmsSrcHand dw ?
    xmsSrcOffs dd ?
    xmsDstHand dw ?
    xmsDstOffs dd ?
_xmsMemMove ENDS   

XMSMoveStruc _xmsMemMove ?          ; Instance to transfer memory



.CODE

; Procedure which leeches the XMS functions address
; @Output
;   al = 1 (Found), 0 = (Not Found)
; @Modifies
;   ax, bx, es
XMSInit PROC
           mov  ax, 4300h
           int  2Fh
           shr  al, 7
           push ax
           or   al, al
           jz   NoXMSFound
           
           mov  ax, 4301h
           int  2Fh
           mov  WORD PTR [XMSLib], bx
           mov  WORD PTR [XMSLib + 2], es

NoXMSFound:pop  ax
           ret
XMSInit ENDP



; Return the space available in XMS
; @Output
;   ax = Largest free extended memory block in Kb
;   dx = Total free extended memory block in Kb
;   bl = Error codes
XMSAvail PROC
           mov  ah, 08h
           call [XMSLib]
           ret
XMSAvail ENDP



; Procedure to allocate a block of XMS memory
; @Input
;   dx = Memory to allocate in Kb
; @Output
;   ax = 1 (Allocated), 0 (Fail)
;   dx = Handle
XMSAlloc PROC
           mov  ah, 09h
           call [XMSLib]
           ret
XMSAlloc ENDP



; Procedure to free a XMS memory block
; @Input
;   dx = Handle
; @Output
;   dl = 0 Error code
XMSFree PROC
           mov  ah, 0Ah
           call [XMSLib]
           ret
XMSFree ENDP



; Procedure to move a XMS memory block
; @Input
;   ax  = Source Handle
;   esi = Source Address
;   ecx = Length
;   dx  = Destination Handle
;   edi = Destination Address
; @Return
;   bl = Error code
; @Modified
;   ax, bx, si
XMSMove PROC
           mov  XMSMoveStruc.xmsLength, ecx
           mov  XMSMoveStruc.xmsSrcHand, ax
           mov  XMSMoveStruc.xmsSrcOffs, esi
           mov  XMSMoveStruc.xmsDstOffs, edi 
           mov  XMSMoveStruc.xmsDstHand, dx
           mov  XMSMoveStruc.xmsDstOffs, esi
           mov  ah, 0Bh
           mov  si, OFFSET XMSMoveStruc
           call [XMSLib]

           ret
XMSMove ENDP



; Transfer an XMS block of memory to lower memory
; @Input
;   ax  = Source Handle
;   esi = Source Offset
;   ecx = Length
;   edi = Destination Offset
; @Return
;   bl = Error code
; @Modified
;   ax, bx, si
XMS2Real PROC
           xor  dx, dx
           call XMSMove
           ret
XMS2Real ENDP



; Transfer a lower memory block to XMS
; @Input
;   esi = Source Address
;   ecx = Length
;   dx  = Destination Handle
;   edi = Destination Address
; @Return
;   bl = Error code
; @Modified
;   ax, bx, si
Real2XMS PROC
           xor  ax, ax
           call XMSMove
           ret
Real2XMS ENDP
