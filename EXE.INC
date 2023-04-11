; EXE Unit for file lister                      <C> 1992 Created by Dàrk Shàde
.CONST

; EXE Header
EXEHeader  db '\7 [\F%s \7Header]\n \n', 0
EXEMZHead  db '\7  Overlay Number : %-38z    CheckSum:%-73 %-4h\n'
           db '  File Size : %-38z    Header Size : %-78z\n'
           db '  Minimum memory : %-38z    Maximum memory : %-78z\n'
           db '  CS:IP :%-28 %3h0:%4h    SS:SP :%-68 %3h0:%4h\n'
           db '  Relocation Entries :%-38z    Relocation Offset : %-78z\n', 0

EXERelocHeader db ' \n\7 [Relocation Table Offsets]\n \n', 0

EXERelocStr    db '\7 %2h%4h ', 0

ID         db 'MZ', 0



.CODE

_exehead STRUC
	exeHeadID	dw ?		; Always 'MZ' for DOS apps
	exeSizeLo	dw ?		; File size (% 512)
	exeSizeHi 	dw ? 		; File size (/ 512)
	exeReloc	dw ? 		; Number of relocation items
	exeHeadSize	dw ? 		; Size of the header in segment-space (*10h)
	exeMinPara	dw ? 		; Minimum number of pagraph needed to run the program
	exeMaxPara	dw ? 		; Maximum number of paragraph the program would like
	exeSSOffset	dw ?		; Offset of stack segments in para
	exeSP		dw ?		; Initial value for SP
	exeChecksum dw ? 		; Checksum used by exec load pgm
	exeIP		dw ?		; Initial value for IP
	exeCSOffset dw ?		; Offset of the code segment in para
	exeRlOffset dw ? 		; Offset in .EXE of first relocation item
	exeOVLNum 	dw ?		; Over number (0 => Root)
_exehead ENDS



LoadEXE PROC
           push bx

           push bx
           call TellFile
           add  di, 160
           pop  bx

           mov  cx, 512
           call ReadFile

; ID String
           push OFFSET ID
           mov  bx, sp

           mov  si, OFFSET EXEHeader
           call TellFStr
           mov  sp, bx

; Relocation Offset
           xor  eax, eax
           mov  ax, WORD PTR fs:[OFFSET ReadBuffer + _exehead.exeRlOffset]
           push eax
; Relocation Count
           mov  ax, WORD PTR fs:[OFFSET ReadBuffer + _exehead.exeReloc]
           push eax
; SS:SP & CS:IP           
           push WORD PTR fs:[OFFSET ReadBuffer + _exehead.exeSP]
           push WORD PTR fs:[OFFSET ReadBuffer + _exehead.exeSSOffset]
           push WORD PTR fs:[OFFSET ReadBuffer + _exehead.exeIP]
           push WORD PTR fs:[OFFSET ReadBuffer + _exehead.exeCSOffset]
; Min & Max Memory           
           mov  ax, WORD PTR fs:[OFFSET ReadBuffer + _exehead.exeMaxPara]
           shl  eax, 4
           push eax
           
           xor  eax, eax
           mov  ax, WORD PTR fs:[OFFSET ReadBuffer + _exehead.exeMinPara]
           shl  eax, 4
           push eax
; File & Head sizes
           xor  eax, eax
           mov  ax, WORD PTR fs:[OFFSET ReadBuffer + _exehead.exeHeadSize]
           shl  eax, 4
           push eax
           
           xor  eax, eax
           mov  ax, WORD PTR fs:[OFFSET ReadBuffer + _exehead.exeSizeHi]
           dec  ax
           shl  eax, 9
           xor  edx, edx
           mov  dx, WORD PTR fs:[OFFSET ReadBuffer + _exehead.exeSizeLo]
           add  eax, edx  
           push eax
; Overlay & checksum
           xor  eax, eax
           mov  ax, WORD PTR fs:[OFFSET ReadBuffer + _exehead.exeOVLNum]
           push WORD PTR fs:[OFFSET ReadBuffer + _exehead.exeChecksum]
           
           push eax
           
; Print it out           
           mov  bx, sp
           call TellFStr
           mov  sp, bx          ; Restore the stack

; Print the relocation table
           mov  cx, WORD PTR fs:[OFFSET ReadBuffer + _exehead.exeReloc]
           jcxz EXEDone

           mov  si, OFFSET EXERelocHeader
           push cx
           call TellFStr
           pop  cx
           
           xor  esi, esi

ShowReloc: xor  eax, eax
           mov  si, WORD PTR fs:[OFFSET ReadBuffer + _exehead.exeRlOffset]
           mov  ax, fs:[esi + ecx * 4 - 2]
           shl  eax, 4
           add  ax, fs:[esi + ecx * 4 - 4]
           ror  eax, 16
           
           push eax
           mov  bx, sp

           mov  si, OFFSET EXERelocStr
           push cx
           call TellFStr
           pop  cx
           pop  eax
           loop ShowReloc

           call AdjustDI
           
EXEDone:   pop  bx
           ret
LoadEXE ENDP
