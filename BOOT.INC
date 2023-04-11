; BOOT Unit for file lister                     <C> 1992 Created by Dàrk Shàde
.CONST

; Partition Table Header
MBRPart    db '\7  Status :%-35 %2h    Partition type :%-75 %2h\n'
           db '  Starting head :%-38z    Ending head :%-78z\n'
           db '  Starting sector : %-38z    Ending sector : %-78z\n'
           db '  Starting cylinder : %-38z    Ending cylinder : %-78z\n'
           db '  Starting block : %-38z    Block count : %-78z\n \n', 0

BootSec    db '\7 [Partition %2h: \F%s\7]\n'
           db '  OEM Name :%-37S    File System :%-77S\n' 
           db '  Reserved sectors :%-38z    Hidden sectors :%-78z\n'
           db '  FATs count :%-38z    Roots count :%-78z\n'
           db '  Head count :%-38z    Sector count :%-78z\n'
           db '  Sectors per Cluster :%-38z    Sectors per Track :%-78z\n'
           db '  Sectors per FAT :%-38z    Bytes per Sector :%-78z\n'    
           db '  Drive Number :%-35 %2h    Serial Number :%-68 %4h-%4h\n \n', 0 



.CODE

_mbrpartition STRUC
    mbrStatus       db ?            ; Status of or physical drive
    mbrFirstHead    db ?            ; Head of first sector (CHS format)
    mbrFirstSector  dw ?            ; Sector & Cylinder of first sector (CHS)
    mbrType         db ?            ; Partition type
    mbrLastHead     db ?            ; Head of the last sector (CHS format)
    mbrLastSector   dw ?            ; Sector & Cylinder of last sector (CHS)
    mbrOffset       dd ?            ; LAB address of first sector
    mbrCount        dd ?            ; Sector count in partition
_mbrpartition ENDS

mbrFirstPart    equ 01BEh
mbrBootID       equ 01FFh

_bpb STRUC
    bpbJumpInstr    db 3 dup(?)     ; Jump instruction to rest of code
    bpbOEMName      db 8 dup(?)     ; OEM name
    bpbBytesPerSec  dw ?            ; Bytes per sector
    bpbSecPerClust  db ?            ; Sectors per cluster
    bpbResSec       dw ?            ; Number of reserved sectors
    bpbFATCount     db ?            ; Number of FAT tables
    bpbRootDirCount dw ?            ; Number of root directory entries
    bpbSecCount     dw ?            ; Total number of sectors
    bpbMediaDesc    db ?            ; Media descriptor
    bpbSecPerFAT    dw ?            ; Number of sectors per FAT
    bpbSecPerTrack  dw ?            ; Number of setors per track
    bpbHeadCount    dw ?            ; Number of heads
    bpbHideSecCount dd ?            ; Number of hidden sectors
    bpbfsSecCount   dd ?            ; Total number of sectors in the filesystem
    bpbLogicDriveNo db ?            ; Logical drive number
    bpbRes1         db ?            ; Reserved
    bpbExtSignature db ?            ; Extended signature 0x29
    bpbSerialNo     dd ?            ; Serial number
    bpbVolumeLabel  db 11 dup(?)    ; Volume label
    bpbFSType       db 8 dup(?)     ; File system type
_bpb ENDS


LoadBOOT PROC
           LOCAL PartAt : BYTE, PartDrive : BYTE, PartOEM : BYTE : 9, PartFS : BYTE : 9 = AUTO_SIZE

           push bp
           mov  bp, sp
           sub  sp, AUTO_SIZE

           mov  PartAt, 0 

           xor  di, di
           
NextBoot:  cmp  PartAt, 4
           jae  BootDone
           
           mov  ah, 19h
           int  21h

           xor  dh, dh
           mov  dl, al
           cmp  al, 2
           jb   ReadSec0
           add  dl, 80h - 2

ReadSec0:  xor  ax, ax
           int  13h                     ; Reset disk
           xor  bx, bx
           mov  ax, 0201h               ; Command/1 Sector read
           mov  cx, 1                   ; Starting Sector Number
           mov  PartDrive, dl
           push es
           push fs
           pop  es
           int  13h                     ; Read the first sector
           pop  es
           or   ah, ah
           jnz  BootDone

           cmp  BYTE PTR fs:[0], 0EBh           ; JMP SHORT ?
           je   MBRFloppy

           mov  si, mbrFirstPart
           xor  ah, ah
           mov  al, PartAt
           shl  al, 4
           add  si, ax
           inc  PartAt
           
           cmp  BYTE PTR fs:[si + OFFSET _mbrpartition.mbrStatus], 0
           je   NextBoot

Master:    push DWORD PTR fs:[si + OFFSET _mbrpartition.mbrCount]

           push DWORD PTR fs:[si + OFFSET _mbrpartition.mbrOffset]

           xor  eax, eax
           mov  ax, fs:[si + OFFSET _mbrpartition.mbrLastSector]
           xchg al, ah
           shr  ah, 6
           push eax

           mov  ax, fs:[si + OFFSET _mbrpartition.mbrFirstSector]
           xchg al, ah
           shr  ah, 6
           push eax

           mov  al, fs:[si + OFFSET _mbrpartition.mbrLastSector]
           and  ax, 03Fh
           push eax

           mov  al, fs:[si + OFFSET _mbrpartition.mbrFirstSector]
           and  al, 03Fh
           push eax

           mov  al, fs:[si + OFFSET _mbrpartition.mbrLastHead]
           push eax

           mov  al, fs:[si + OFFSET _mbrpartition.mbrFirstHead]
           push eax

           mov  al, fs:[si + OFFSET _mbrpartition.mbrType]
           push ax
           
           mov  al, fs:[si + OFFSET _mbrpartition.mbrStatus]
           push ax
           mov  bx, sp

           push si
           mov  si, OFFSET MBRPart
           call TellFStr
           pop  si
           mov  sp, bx

           
; Read the partition table           
           xor  bx, bx
           mov  ax, 0201h
           mov  cx, fs:[si + OFFSET _mbrpartition.mbrFirstSector]
           mov  dh, fs:[si + OFFSET _mbrpartition.mbrFirstHead]
           mov  dl, PartDrive           ; Drive #
           push es
           push fs
           pop  es
           int  13h                     ; Read the first sector
           pop  es
           or   ah, ah
           jnz  BootDone

           jmp  MBRNoFlop

MBRFloppy: mov  PartAt, 0FFh    

MBRNoFlop: push WORD PTR fs:[_bpb.bpbSerialNo]
           push WORD PTR fs:[_bpb.bpbSerialNo + 2]

           xor  eax, eax
           mov  al, fs:[_bpb.bpbLogicDriveNo]
           push ax
           
           mov  ax, fs:[_bpb.bpbBytesPerSec]
           push eax

           mov  ax, fs:[_bpb.bpbSecPerFAT]
           push eax

           mov  ax, fs:[_bpb.bpbSecPerTrack]
           push eax
           
           xor  ah, ah
           mov  al, fs:[_bpb.bpbSecPerClust]
           push eax

           mov  eax, fs:[_bpb.bpbfsSecCount]
           add  ax, fs:[_bpb.bpbSecCount]
           push eax

           xor eax, eax
           mov  ax, fs:[_bpb.bpbHeadCount]
           push eax

           mov  ax, fs:[_bpb.bpbRootDirCount]
           push eax

           xor  ah, ah
           mov  al, fs:[_bpb.bpbFATCount]
           push eax

           push DWORD PTR fs:[_bpb.bpbHideSecCount]
           
           mov  ax, fs:[_bpb.bpbResSec]
           push eax

           lea  ax, [PartFS]
           push ax
           push ss

           lea ax, [PartOEM]
           push ax
           push ss

           push OFFSET VolumeName

           mov  ax, 1
           push ax
           mov  bx, sp


           push di
           push ds
           push es

           push ds
           pop  es
           push fs
           pop  ds
           xor  al, al

           mov  si, OFFSET _bpb.bpbVolumeLabel
           mov  di, OFFSET VolumeName
           mov  cx, 11
           call CopyNoSpace

           push ss
           pop  es
           
           mov  si, OFFSET _bpb.bpbOEMName
           lea  di, [PartOEM]
           mov  cx, 8
           call CopyNoSpace

           mov  si, OFFSET _bpb.bpbFSType
           lea  di, [PartFS]
           mov  cx, 8
           call CopyNoSpace
           
           pop  es
           pop  ds
           pop  di

           mov  si, OFFSET BootSec
           call TellFStr
           mov  sp, bx

           jmp  NextBoot
           
BootDone:  mov  sp, bp
           pop  bp
           jmp  Quit
LoadBoot ENDP



CopyNoSpace PROC
           lodsb
           cmp  al, ' '
           je   CopyHasSpc
           
           mov  dx, di
CopyHasSpc:stosb
           loop CopyNoSpace

           mov  di, dx
           mov  BYTE PTR es:[di + 1], cl    ; ASCIIZ

           ret
CopyNoSpace ENDP
