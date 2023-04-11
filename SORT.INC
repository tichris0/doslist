; Sorting Unit for file lister                  <C> 1992 Created by Dàrk Shàde
.CODE

; QuickSorts an array
; @Input
;   ds:si   = List of elements
;   ds:di   = Last element of list
;   dx      = Element size
; @Modifies
;   ax, bx, cx
QuickSort PROC
           mov  cx, 1
           push si
           push di
           
QSLoop:    
QSInLoop:  cmp  si, di
           jae  QSPop

           push si
           push di                  ; Preserve our range 
           
; Copy the pivot into the stack as a reference
QAPart:    sub  sp, dx
           mov  bp, di              ; Preserve di
           mov  di, sp

           mov  bx, dx
CopyPivot: sub  bx, 2
           mov  ax, ds:[si + bx]
           mov  ss:[di + bx], ax
           jnz  CopyPivot

           
           xchg di, bp              ; bp = &Pivot & Restore di
           sub  si, dx              ; si = &Array[-1]
           add  di, dx              ; di = &Array[n + 1]
           
QSFindLo:  add  si, dx
           call QSCompare
           jb   QSFindLo
           
           xchg si, di
QSFindHi:  sub  si, dx
           call QSCompare
           ja   QSFindHi
           
           xchg si, di
           cmp  si, di
           jae  QSPartDone

; Swap the elements
           mov  bx, dx
SwapPoint: sub  bx, 2
           mov  ax, [si + bx]
           xchg [di + bx], ax
           mov  [si + bx], ax  
           jnz  SwapPoint

           jmp  QSFindLo

QSPartDone:add  sp, dx

           ; bx = pivot, si = Start, di = End
           mov  bx, di
           pop  di
           pop  si

           inc  cx
           mov  ax, bx
           sub  ax, si
           add  ax, bx
           sub  ax, di
           jge  QSLeft                  ; (pi - low) < (high - pi)?
           
           add  bx, dx 
           push bx
           sub  bx, dx
           push di                      ; add       [pi + 1, high]
           mov  di, bx                  ; proces    [low, pi]
           jmp  QSLoop  

QSLeft:    push si
           push bx                      ; add       [low, pi]
           mov  si, bx                  ; process   [pi + 1, high]
           add  si, dx 
           jmp  QSLoop

QSPop:     pop  di
           pop  si
           dec  cx
           jnz  QSLoop

           ret
QuickSort ENDP
