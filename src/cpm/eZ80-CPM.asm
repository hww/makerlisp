; CP/M v2.2 Copyright 1979 (c) by Digital Research
; Ported to the eZ80F91 platform by
; Jean-Michel Howland
; Christopher D. Farrar

		CPU	= EZ80F91

                .ASSUME ADL=0

                INCLUDE "eZ80-CPM-MDEF.inc"                     ; CP/M memory defines.

                ORG     ccp                                     ; start addr of cpm binary blob

; Start of CP/M code.

		INCLUDE	"eZ80-CPM-CCP.inc"			; CP/M Console Command Processor code.
		INCLUDE	"eZ80-CPM-BDOS.inc"			; CP/M Basic Disk Operating System code.
		INCLUDE	"eZ80-CPM-BIOS.inc"			; CP/M Basic Input/Output System code.

; End of CP/M code.

		END
