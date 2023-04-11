; MCB Unit for file lister                      <C> 1992 Created by Dàrk Shàde
.CONST

; MCB Header  '00         00         00         00         00         '
MCBHeader  db '\F ID  Segm   Length    PSP     Name\n \n', 0
MCBEntry   db '\7 %2h  %4h  %9z  %4h  ', 0

MCBFree	   db ' Free', 0
MCBDOS	   db 'DOS', 0
MCBData    db ' Data', 0

; Memory Control Block
_mcb STRUC
	mcbLast		db ?		; 'M' => member, 'Z' => last
	mcbPSPSeg	dw ?		; Segment address of the owner's PSP
	mcbSize		dw ?		; Size of the mcb allocation in paras (*10h)
	mcbRes1		db 3 dup(?)	; Reserved
	mcbName		db 8 dup(?)	; Process name
_mcb ENDS



.CODE

LoadMCB PROC
           LOCAL MCBCount: WORD, MCBExt: WORD, MCBAppName: BYTE: 9 = AUTO_SIZE

           push bp
           mov  bp, sp
           sub  sp, AUTO_SIZE

           mov  MCBCount, 0
           mov  MCBExt, 0

           xor  di, di
           mov  si, OFFSET MCBHeader
		   call TellFStr

           mov  ah, 52h
           int  21h
           sub  bx, 2
           mov  fs, es:[bx]             ; es = First MCB segment!
           mov  es, ScrSeg
		   mov  MCBCount, 0

ShowMCB:
           mov  ax, fs:[_mcb.mcbPSPSeg]
		   xor  edx, edx
           mov  dx, fs:[_mcb.mcbSize]
           shl  edx, 4

           push fs
           push edx
           push ax
           push MCBCount
           mov  bx, sp

           mov  si, OFFSET MCBEntry
           call TellFStr
           mov  sp, bx

; Is the memory block free?
		   cmp  WORD PTR fs:[_mcb.mcbPSPSeg], 0
		   jne  MCBChckDOS

           mov  ah, 9
           push OFFSET MCBFree
           push ds
		   jmp  MCBSegWrite

; MCB Block is owned by DOS? 
MCBChckDOS:cmp  fs:[_mcb.mcbPSPSeg], 8
		   jne  MCBChckApp
            
           push OFFSET MCBDOS
           push ds
		   jmp  MCBSegWrite

; MCB Block is an app?
MCBChckApp:mov  dx, fs
           inc  dx
		   cmp  dx, fs:[_mcb.mcbPSPSeg]
           jne  MCBIsData

; Output the name
           mov  ah, 0Ah
           lea  si, [MCBAppName]
           push si
           push ss

           mov  edx, DWORD PTR fs:[_mcb.mcbName]
           mov  ss:[si], edx
           mov  edx, DWORD PTR fs:[_mcb.mcbName + 4]
           mov  ss:[si + 4], edx
           mov  BYTE PTR ss:[si + 8], 0
           jmp  MCBSegWrite

; MCB block is data!
MCBIsData: mov  ah, 0Fh
           push OFFSET MCBData
           push ds

MCBSegWrite:
           mov  si, OFFSET FStr
           mov  bx, sp
		   call TellFStr
           mov  sp, bx

		   cmp  BYTE PTR fs:[_mcb.mcbLast], 'M'
           je   MCBValid
		   cmp  BYTE PTR fs:[_mcb.mcbLast], 'Z'
		   jne  MCBDone
		   cmp  MCBExt, 0
		   jne  MCBDone

		   mov  MCBExt, 1

MCBValid:  inc  WORD PTR MCBCount

           call CheckMore

           mov  dx, fs
           mov  ax, fs:[_mcb.mcbSize]
           add  ax, dx
		   inc  ax
           mov  fs, ax

           jmp  ShowMCB

MCBDone:   mov  sp, bp
           pop  bp
           jmp  Quit           
LoadMCB ENDP