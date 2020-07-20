
; CP/M Defines - Memory and associated defines.

; Set CP/M memory starting address here.

cpmsize:        EQU     62                                      ; k

tpa:            EQU     00100h                                  ; Base of TPA.
ccp:            EQU     (cpmsize-7)*1024                        ; Base of CCP.
bdos:           EQU     ccp + 00800h                            ; Base of BDOS.
bios:           EQU     ccp + 01600h                            ; Base of BIOS.

; Set CP/M low memory data, vector and buffer addresses.

biosjp:         EQU     00000h                                  ; bios jump
iobyte:         EQU     00003h                                  ; Intel standard I/O definition byte.
usrdrv:         EQU     00004h                                  ; Current user number and drive.
bdosen:         EQU     00005h                                  ; Entry point for the BDOS.
tpafcb:         EQU     0005Ch                                  ; Default File Control Block.
tpabuf:         EQU     00080h                                  ; Default I/O buffer and command line storage.

; Set control character equates.

cntrlc:       EQU 03h         ; Control-C (Reboots CP/M system).
cntrle:       EQU 05h         ; Control-E (Physical EOL, line not sent until CR pressed).
bs:           EQU 08h         ; Control-H (Backspace).
tab:          EQU 09h         ; Control-I (Tab).
lf:           EQU 0Ah         ; Control-J (Line feed).
ff:           EQU 0Ch         ; Control-L (Form feed).
cr:           EQU 0Dh         ; Control-M (Carriage return).
cntrlp:       EQU 10h         ; Control-P (Toggle I/O to current list device).
cntrlr:       EQU 12h         ; Control-R (Retypes current command line).
cntrls:       EQU 13h         ; Control-S (Stops console output temporarily).
cntrlu:       EQU 15h         ; Control-U (Deletes the entire line typed).
cntrlx:       EQU 18h         ; Control-X (Same as Control-U).
cntrlz:       EQU 1Ah         ; Control-Z (End-Of-File mark).
escape:       EQU 1Bh         ; escape
del:          EQU 7Fh         ; Rubout.

; End of CP/M Defines.
