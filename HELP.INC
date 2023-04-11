; Help Unit for file lister                     <C> 1992 Created by D‡rk Sh‡de
.CONST

; Help file strings!
Help       db 'LIST [Drive:][Path][FileName] [/?] [/V] [/W] [/I] [/C] [/H] [/X] [/J(Position)]\n'
           db '%5 [/D (*) ; (Year, Month, Day)] [/T (*) ; (Hour, Min, Sec) ] [/N Ò(RASH)]\n'
           db '%5 [/R] [/S] [/MBC] [/MCB] [/DEV] [MORE]                    Version 0.9\n'
           db ' \n'
           db '  [Drive:][Path][FileName]%-32 Specifies drive, directory and/or files to list\n'
           db '  /?%-32 Give general help on LIST comands\n'
           db '  /V%-32 Give all supported special files [/X]\n'
           db '  /W%-32 Uses wide format listing\n'
           db '  /I%-32 Show ASCII characters in the file(s)\n'
           db '  /C%-32 Show ASCII Dec value in the file(s)\n'
           db '  /H%-32 Show ASCII Hex value in the file(s)\n'
           db '  /X%-32 Special Vieweing (ANS/PCX/ZIP...)\n'
           db '  /R%-32 Show directory tree\n'
           db '  /S%-32 Recurse subdirectories\n'
           db '  /MBR%-32 Show BOOT record of specified drive\n'
           db '  /MCB%-32 Show the Memory Control Block chain\n'
           db '  /DEV%-32 Show the installed devices\n'
           db '  /J(File Position)%-32 Start viewing file at (File Position)\n'
           db '  /D [Year] [Month] [Day] [*]%-32 Modifies the file''s date (* = Current date)\n'
           db '  /T [Hour] [Min] [Sec] [*]%-32 Modifies the file''s time (* = ^^   ^^ time)\n'
           db '  /N [ÒR] [ÒA] [ÒS] [ÒH]%-32 Adds or removes file attributes\n'
           db '  MORE%-32 Pauses after each screenful of information\n', 0

TypeHelp   db '   Base Ext%4 Company%5 Initial Host  Accepted extentions\n'
           db ' \n'
           db '  .ACE%9 Marcel L.   ACE/WinAce%4 *.A##\n'
           db '  .ARJ%9 LA Soft%5 ARJ/WinArj%4 *.ARJ\n'
           db '  .LHA & .LZH  Haruyasu Y. LHA%11 *.LHA & *.LZH\n'
           db '  .RAR%9 Eugene R.   RAR%11 *.R##\n'
           db '  .ZIP%9 PKWARE%6 PKZIP/WinZip  *.ZIP\n'
           db ' \n'
           db '  .EXE%9 Varies%6 Compiler%6 *.EXE\n'
           db '  .OVL%9 Varies%6 Compiler%6 *.OVL\n'
           db '  .OBJ%9 Varies%6 Compiler%6 *.OBJ\n'
           db '  .SYS%9 Varies%6 Compiler%6 *.SYS\n'
           db '  .TXT%9 Varies%6 Text Editor   any ASCII readable text\n'
           db ' \n'
           db '  .PAL%9 Varies%6 Unknown       *.PAL (6bit & 8bit modes)\n'
           db '  .PCX%9 ZSoft%7 Paint Brush   *.PCX\n', 0



.CODE

; Procedure to show the help content on the screen
TellHelp PROC
           xor  di, di
           mov  ah, 07
           mov  si, OFFSET Help

           call TellfStr                ; Write on the screen!
           jmp  Quit
TellHelp ENDP



; Procedure to show the filetype help on the screen
ViewHelp PROC
           xor  di, di
           mov  ah, 07
           mov  si, OFFSET TypeHelp

           call TellfStr                ; Write on the screen!
           jmp  Quit
ViewHelp ENDP
