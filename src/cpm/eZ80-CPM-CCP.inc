
; CCP - Console Command Processor.

		ORG	ccp					; Origin of CCP.

		jp	command					; Execute command processor CCP.
		jp	clearbuf				; Entry to empty input buffer before starting CCP.

; Standard CP/M CCP input buffer. Format is (max length), (actual length), (char #1), (char #2), (char #3), etc.

inbuff:		DB	7Fh					; Length of input buffer.
		DB	00h					; Current length of contents.
		ASCII	"Copyright"
		ASCII	" 1979 (c) by Digital Research      "
		DB	00h,00h,00h,00h,00h,00h,00h,00h
		DB	00h,00h,00h,00h,00h,00h,00h,00h
		DB	00h,00h,00h,00h,00h,00h,00h,00h
		DB	00h,00h,00h,00h,00h,00h,00h,00h
		DB	00h,00h,00h,00h,00h,00h,00h,00h
		DB	00h,00h,00h,00h,00h,00h,00h,00h
		DB	00h,00h,00h,00h,00h,00h,00h,00h
		DB	00h,00h,00h,00h,00h,00h,00h,00h
		DB	00h,00h,00h,00h,00h,00h,00h,00h
		DB	00h,00h,00h,00h,00h,00h,00h,00h
inpoint:	DW	inbuff + 02h				; Input line pointer
namepnt:	DW	0000h					; Input line pointer used for error message. Points to start of name in error.

; Routine to print A on the console. All registers used.

printd:		ld	e,a					; Setup BDOS call.
		ld	c,02h
		jp	bdosen

; Routine to print A on the console and to save BC.

printb:		push	bc
		call	printd
		pop	bc
		ret

; Routine to send a carriage return, line feed combination to the console.

crlf:		ld	a,cr
		call	printb
		ld	a,lf
		jp	printb

; Routine to send one space to the console and save BC.

spacec:		ld	a,' '
		jp	printb

; Routine to print character string pointed to by BC on the console. It must terminate with a null byte.

pline:		push	bc
		call	crlf
		pop	hl
pline2:		ld	a,(hl)
		or	a
		ret	z
		inc	hl
		push	hl
		call	printd
		pop	hl
		jp	pline2

; Routine to reset the disk system.

resdsk:		ld	c,0Dh
		jp	bdosen

; Routine to select disk A.

dsksel:		ld	e,a
		ld	c,0Eh
		jp	bdosen

; Routine to call BDOS and save the return code. The zero flag is set on a return of 0ffh.

entry1:		call	bdosen
		ld	(rtncode),a				; Save return code.
		inc	a					; Set zero if 0ffh returned.
		ret

; Routine to open a file. DE must point to the FCB.

open:		ld	c,0Fh
		jp	entry1

; Routine to open file at FCB.

openfcb:	xor	a					; Clear the record number byte at FCB + 32
		ld	(fcbb + 20h),a
		ld	de,fcbb
		jp	open

; Routine to close a file. DE points to FCB.

close:		ld	c,10h
		jp	entry1

; Routine to search for the first file with ambiguous name DE.

srchfst:	ld	c,11h
		jp	entry1

; Search for the next ambiguous file name.

srchnxt:	ld	c,12h
		jp	entry1

; Search for file at FCB.

srchfcb:	ld	de,fcbb
		jp	srchfst

; Routine to delete a file pointed to by DE.

delete:		ld	c,13h
		jp	bdosen

; Routine to call the BDOS and set the zero flag if a zero status is returned.

entry2:		call	bdosen
		or	a					; Set zero flag if appropriate.
		ret

; Routine to read the next record from a sequential file. DE points to the FCB.

rdrec:		ld	c,14h
		jp	entry2

; Routine to read file at FCB.

readfcb:	ld	de,fcbb
		jp	rdrec

; Routine to write the next record of a sequential file. DE points to the FCB.

wrtrec:		ld	c,15h
		jp	entry2

; Routine to create the file pointed to by DE.

create:		ld	c,16h
		jp	entry1

; Routine to rename the file pointed to by DE. Note that the new name starts at DE + 16.

renam:		ld	c,17h
		jp	bdosen

; Get the current user code.

getusr:		ld	e,0FFh

; Routine to get or set the current user code. If E is 0ffH then this is a get, else it is a set.

getsetuc:	ld	c,20h
		jp	bdosen

; Routine to set the current drive byte at (usrdrv).

setcdrv:	call	getusr					; Get user number
		add	a,a					; and shift into the upper 4 bits.
		add	a,a
		add	a,a
		add	a,a
		ld	hl,cdrive				; Now add in the current drive number.
		or	(hl)
		ld	(usrdrv),a				; And save.
		ret

; Move currently active drive down to (usrdrv).

movecd:		ld	a,(cdrive)
		ld	(usrdrv),a
		ret

; Routine to convert A into upper case ASCII. Only letters are affected.

upper:		cp	'a'					; Check for letters in the range of 'a' to 'z'.
		ret	c
		cp	'{'
		ret	nc
		and	5Fh					; Convert it if found.
		ret

; Routine to get a line of input. We must check to see if the user is in (batch) mode. If so,
; then read the input from file ($$$.SUB). At the end, reset to console input.

getinp:		ld	a,(batch)				; If A = 0, then use console input.
		or	a
		jp	z,getinp1

; Use the submit file ($$$.SUB) which is prepared by a submit run. It must be on drive A and
; it will be deleted if and error occurs (like EOF).

		ld	a,(cdrive)				; Select drive 0 if need be.
		or	a
		ld	a,00h					; Always use drive a for submit.
		call	nz,dsksel				; Select it if required.
		ld	de,batchfcb
		call	open					; Look for it.
		jp	z,getinp1				; If not there, use normal input.
		ld	a,(batchfcb + 0Fh)			; Get last record number + 1.
		dec	a
		ld	(batchfcb + 20h),a
		ld	de,batchfcb
		call	rdrec					; Read last record.
		jp	nz,getinp1				; Quit on end of file.

; Move this record into input buffer.

		ld	de,inbuff + 01h
		ld	hl,tpabuf				; Data was read into buffer here.
		ld	b,80h					; All 128 characters may be used.
		call	hl2de					; HL to DE, B bytes.
		ld	hl,batchfcb + 0Eh
		ld	(hl),00h				; Zero out the 'S2' byte.
		inc	hl					; And decrement the record count.
		dec	(hl)
		ld	de,batchfcb				; Close the batch file now.
		call	close
		jp	z,getinp1				; Quit on an error.
		ld	a,(cdrive)				; Re-select previous drive if need be.
		or	a
		call	nz,dsksel				; Don't do needless selects.

; Print line just read on console.

		ld	hl,inbuff + 02h
		call	pline2
		call	chkcon					; Check console, quit on a key.
		jp	z,getinp2				; Jump if no key is pressed.

; Terminate the submit job on any keyboard input. Delete this file such that it is not
; re-started and jump to normal keyboard input section.

		call	delbatch				; Delete the batch file.
		jp	cmmnd1					; And restart command input.

; Get here for normal keyboard input. Delete the submit file incase there was one.

getinp1:	call	delbatch				; Delete file ($$$.SUB).
		call	setcdrv					; Reset active disk.
		ld	c,0Ah					; Get line from console device.
		ld	de,inbuff
		call	bdosen
		call	movecd					; Reset current drive (again).

; Convert input line to upper case.

getinp2:	ld	hl,inbuff + 01h
		ld	b,(hl)					; B = character counter.
getinp3:	inc	hl
		ld	a,b					; End of the line?
		or	a
		jp	z,getinp4
		ld	a,(hl)					; Convert to upper case.
		call	upper
		ld	(hl),a
		dec	b					; Adjust character count.
		jp	getinp3
getinp4:	ld	(hl),a					; Add trailing null.
		ld	hl,inbuff + 02h
		ld	(inpoint),hl				; Reset input line pointer.
		ret

; Routine to check the console for a key pressed. The zero flag is set is none, else the
; character is returned in A.

chkcon:		ld	c,0Bh					; Check console.
		call	bdosen
		or	a
		ret	z					; Return if nothing.
		ld	c,01h					; Else get character.
		call	bdosen
		or	a					; Clear zero flag and return.
		ret

; Routine to get the currently active drive number.

getdsk:		ld	c,19h
		jp	bdosen

; Set the standard DMA address.

stddma:		ld	de,tpabuf

; Routine to set the DMA address to DE.

dmaset:		ld	c,1Ah
		jp	bdosen

;  Delete the batch file created by submit.

delbatch:	ld	hl,batch				; Is batch active?
		ld	a,(hl)
		or	a
		ret	z
		ld	(hl),00h				; Yes, de-activate it.
		xor	a
		call	dsksel					; Select drive 0 for sure.
		ld	de,batchfcb				; And delete this file.
		call	delete
		ld	a,(cdrive)				; Reset current drive.
		jp	dsksel

; Check to two strings at (pattrn1) and (pattrn2). They must be the same or we halt....

verify:		ld	de,pattrn1				; These are the serial number bytes.
                ld      hl,bdos                                 ; Ditto, but how could they be different?
		ld	b,06h					; 6 bytes each.
verify1:	ld	a,(de)
		cp	(hl)
		jp	nz,haltc				; Jump to halt routine.
		inc	de
		inc	hl
		dec	b
		jp	nz,verify1
		ret

; Print back file name with a '?' to indicate a syntax error.

synerr:		call	crlf					; End current line.
		ld	hl,(namepnt)				; This points to name in error.
synerr1:	ld	a,(hl)					; Print it until a space or null is found.
		cp	' '
		jp	z,synerr2
		or	a
		jp	z,synerr2
		push	hl
		call	printd
		pop	hl
		inc	hl
		jp	synerr1
synerr2:	ld	a,'?'					; Add trailing '?'.
		call	printd
		call	crlf
		call	delbatch				; Delete any batch file.
		jp	cmmnd1					; And restart from console input.

; Check character at DE for legal command input. Note that the zero flag is set if the character is a delimiter.

check:		ld	a,(de)
		or	a
		ret	z
		cp	' '					; Control characters are not legal here.
		jp	c,synerr
		ret	z					; Check for valid delimiter.
		cp	'='
		ret	z
		cp	'_'
		ret	z
		cp	'.'
		ret	z
		cp	':'
		ret	z
		cp	';'
		ret	z
		cp	'<'
		ret	z
		cp	'>'
		ret

; Get the next non-blank character from DE.

nonblank:	ld	a,(de)
		or	a					; String ends with a null.
		ret	z
		cp	' '
		ret	nz
		inc	de
		jp	nonblank

; Add HL = HL + A

addhl:		add	a,l
		ld	l,a
		ret	nc					; Take care of any carry.
		inc	h
		ret

; Convert the first name in FCB.

convfst:	ld	a,00h

; Format a file name (convert * to '?', etc.). On return, A = 0 is an unambiguous name was specified. Enter with
; A equal to the position within the FCB for the name (either 0 or 16).

convert:	ld	hl,fcbb
		call	addhl
		push	hl
		push	hl
		xor	a
		ld	(chgdrv),a				; Initialize drive change flag.
		ld	hl,(inpoint)				; Set HL as pointer into input line.
		ex	de,hl
		call	nonblank				; Get next non-blank character.
		ex	de,hl
		ld	(namepnt),hl				; Save pointer here for any error message.
		ex	de,hl
		pop	hl
		ld	a,(de)					; Get first character.
		or	a
		jp	z,convrt1
		sbc	a,'A' - 1				; Might be a drive name, convert to binary.
		ld	b,a					; And save.
		inc	de					; Check next character for a ':'.
		ld	a,(de)
		cp	':'
		jp	z,convrt2
		dec	de					; Nope, move pointer back to the start of the line.
convrt1:	ld	a,(cdrive)
		ld	(hl),a
		jp	convrt3
convrt2:	ld	a,b
		ld	(chgdrv),a				; Set change in drives flag.
		ld	(hl),b
		inc	de

; Convert the basic file name.

convrt3:	ld	b,08h
convrt4:	call	check
		jp	z,convrt8
		inc	hl
		cp	'*'					; Note that an '*' will fill the remaining
		jp	nz,convrt5				; field with '?'.
		ld	(hl),'?'
		jp	convrt6
convrt5:	ld	(hl),a
		inc	de
convrt6:	dec	b
		jp	nz,convrt4
convrt7:	call	check					; Get next delimiter.
		jp	z,getext
		inc	de
		jp	convrt7
convrt8:	inc	hl					; Blank fill the file name.
		ld	(hl),' '
		dec	b
		jp	nz,convrt8

; Get the extension and convert it.

getext:		ld	b,03h
		cp	'.'
		jp	nz,getext5
		inc	de
getext1:	call	check
		jp	z,getext5
		inc	hl
		cp	'*'
		jp	nz,getext2
		ld	(hl),'?'
		jp	getext3
getext2:	ld	(hl),a
		inc	de
getext3:	dec	b
		jp	nz,getext1
getext4:	call	check
		jp	z,getext6
		inc	de
		jp	getext4
getext5:	inc	hl
		ld	(hl),' '
		dec	b
		jp	nz,getext5
getext6:	ld	b,03h
getext7:	inc	hl
		ld	(hl),00h
		dec	b
		jp	nz,getext7
		ex	de,hl
		ld	(inpoint),hl				; Save input line pointer.
		pop	hl

; Check to see if this is an ambiguous file name specification. Set the A register to non zero if it is.

		ld	bc,000Bh				; Set name length.
getext8:	inc	hl
		ld	a,(hl)
		cp	'?'					; Any question marks?
		jp	nz,getext9
		inc	b					; Count them.
getext9:	dec	c
		jp	nz,getext8
		ld	a,b
		or	a
		ret

; CP/M command table. Note commands can be either 3 or 4 characters long.

numcmds:	EQU	06h					; Number of commands
cmdtbl:		ASCII	"DIR ERA TYPESAVEREN USER"

; The following six bytes must agree with those at (pattrn2) or CP/M will halt. Why?

pattrn1:	DB	00h,16h,00h,00h,00h,00h			; (* Serial number bytes *).

; Search the command table for a match with what has just been entered.
; If a match is found, then we jump to the proper section. Else jump to (unknown).
; On return, the C register is set to the command number that matched (or numcmds + 1 if no match).

search:		ld	hl,cmdtbl
		ld	c,00h
search1:	ld	a,c
		cp	numcmds					; This commands exists.
		ret	nc
		ld	de,fcbb + 01h				; Check this one.
		ld	b,04h					; Max command length.
search2:	ld	a,(de)
		cp	(hl)
		jp	nz,search3				; Not a match.
		inc	de
		inc	hl
		dec	b
		jp	nz,search2
		ld	a,(de)					; Allow a 3 character command to match.
		cp	' '
		jp	nz,search4
		ld	a,c					; Set return register for this command.
		ret
search3:	inc	hl
		dec	b
		jp	nz,search3
search4:	inc	c
		jp	search1

; Set the input buffer to empty and then start the command processor CCP.

clearbuf:	xor	a
		ld	(inbuff + 01h),a				; Second byte is actual length.

; CCP - Console Command Processor.

command:	ld	sp,ccpstack				; Setup stack area.
		push	bc					; Note that C should be equal to:
		ld	a,c					; (uuuudddd) where 'uuuu' is the user number
		rra						; and 'dddd' is the drive number.
		rra
		rra
		rra
		and	0Fh					; Isolate the user number.
		ld	e,a
		call	getsetuc				; And set it.
		call	resdsk					; Reset the disk system.
		ld	(batch),a				; Clear batch mode flag.
		pop	bc
		ld	a,c
		and	0Fh					; Isolate the drive number.
		ld	(cdrive),a				; And save.
		call	dsksel					; ...and select.
		ld	a,(inbuff + 01h)
		or	a					; Anything in input buffer already?
		jp	nz,cmmnd2				; Yes, we just process it.

; Entry point to get a command line from the console.

cmmnd1:		ld	sp,ccpstack				; Set stack straight.
		call	crlf					; Start a new line on the screen.
		call	getdsk					; Get current drive.
		add	a,'A'
		call	printd					; Print current drive.
		ld	a,'>'
		call	printd					; Print prompt.
		call	getinp					; Get line from user.

; Process command line here.

cmmnd2:		ld	de,tpabuf
		call	dmaset					; Set standard DMA address.
		call	getdsk
		ld	(cdrive),a				; Set current drive.
		call	convfst					; Convert name typed in.
		call	nz,synerr				; Wild cards are not allowed.
		ld	a,(chgdrv)				; If a change in drives was indicated,
		or	a					; then treat this as an unknown command
		jp	nz,unknown				; which gets executed.
		call	search					; Else search command table for a match.

; Note that an unknown command returns with A pointing to the last address in our table which is (unknown).

		ld	hl,cmdadr				; Now, look through our address table for command A.
		ld	e,a					; Set DE to command number.
		ld	d,00h
		add	hl,de
		add	hl,de					; HL = (cmdadr) + 2 * (command number).
		ld	a,(hl)					; Now pick out this address.
		inc	hl
		ld	h,(hl)
		ld	l,a
		jp	(hl)					; Now execute it.

; CP/M command address table.

cmdadr:		DW	direct,erase,typec,save,rename,user,unknown

; Halt the system. Reason for this is unknown at present.

haltc:		ld	hl,76F3h				; 'di hlt' instructions.
		ld	(ccp),hl
		ld	hl,ccp
		jp	(hl)

; Read error while typing a file.

rderror:	ld	bc,rderr
		jp	pline

rderr:		ASCIZ	"Read error"

; Required file was not located.

none:		ld	bc,nofile
		jp	pline

nofile:		ASCIZ	"No file"

; Decode a command of the form 'A>filename number{ filename}.
; Note that a drive specifier is not allowed on the first filename. On return, the number is
; in register A. Any error causes 'filename?' to be printed and the command is aborted.

decode:		call	convfst					; Convert filename.
		ld	a,(chgdrv)				; Do not allow a drive to be specified.
		or	a
		jp	nz,synerr
		ld	hl,fcbb + 01h				; Convert number now.
		ld	bc,000Bh				; B = sum register, C = max digit count.
decode1:	ld	a,(hl)
		cp	' '					; A space terminates the numeral.
		jp	z,decode3
		inc	hl
		sub	'0'					; Make binary from ASCII.
		cp	0Ah					; Legal digit?
		jp	nc,synerr
		ld	d,a					; Yes, save it in D.
		ld	a,b					; Compute B = B * 10 and check for overflow.
		and	0E0h
		jp	nz,synerr
		ld	a,b
		rlca
		rlca
		rlca						; A = B * 8
		add	a,b					; ......* 9
		jp	c,synerr
		add	a,b					; ......* 10
		jp	c,synerr
		add	a,d					; Add in new digit now.
decode2:	jp	c,synerr
		ld	b,a					; And save result.
		dec	c					; Only look at 11 digits.
		jp	nz,decode1
		ret
decode3:	ld	a,(hl)					; Spaces must follow (why?).
		cp	' '
		jp	nz,synerr
		inc	hl
decode4:	dec	c
		jp	nz,decode3
		ld	a,b					; Set A = the numeric value entered.
		ret

; Move 3 bytes from HL to DE. Note that there is only one reference to this at (a2d5h).

move3:		ld	b,03h

; Move B bytes from HL to DE.

hl2de:		ld	a,(hl)
		ld	(de),a
		inc	hl
		inc	de
		dec	b
		jp	nz,hl2de
		ret

; Compute HL = (tpabuf) + A + C and get the byte that's there.

extract:	ld	hl,tpabuf
		add	a,c
		call	addhl
		ld	a,(hl)
		ret

; Check drive specified. If it means a change, then the new drive will be selected. In any case,
; the drive byte of the FCB will be set to null (means use current drive).

dselect:	xor	a					; Null out first byte of FCB.
		ld	(fcbb),a
		ld	a,(chgdrv)				; A drive change indicated?
		or	a
		ret	z
		dec	a					; Yes, is it the same as the current drive?
		ld	hl,cdrive
		cp	(hl)
		ret	z
		jp	dsksel					; No. select it then.

; Check the drive selection and reset it to the previous drive if it was changed for the preceding command.

resetdr:	ld	a,(chgdrv)				; Drive change indicated?
		or	a
		ret	z
		dec	a					; Yes, was it a different drive?
		ld	hl,cdrive
		cp	(hl)
		ret	z
		ld	a,(cdrive)				; Yes, re-select our old drive.
		jp	dsksel

; Directory Command.

direct:		call	convfst					; Convert file name.
		call	dselect					; Select indicated drive.
		ld	hl,fcbb + 01h				; Was any file indicated?
		ld	a,(hl)
		cp	' '
		jp	nz,direct2
		ld	b,0Bh					; No. fill field with '?' - same as *.*.
direct1:	ld	(hl),'?'
		inc	hl
		dec	b
		jp	nz,direct1
direct2:	ld	e,00h					; Set initial cursor position.
		push	de
		call	srchfcb					; Get first file name.
		call	z,none					; None found at all?
direct3:	jp	z,direct9				; Terminate if no more names.
		ld	a,(rtncode)				; Get file's position in segment (0 - 3).
		rrca
		rrca
		rrca
		and	60h					; A = position * 32
		ld	c,a
		ld	a,0Ah
		call	extract					; Extract the tenth entry in FCB.
		rla						; Check system file status bit.
		jp	c,direct8				; We don't list them.
		pop	de
		ld	a,e					; Bump name count.
		inc	e
		push	de
		and	03h					; At end of line? Change to 01h for 40 characters wide.
		push	af
		jp	nz,direct4
		call	crlf					; Yes, end this line and start another.
		push	bc
		call	getdsk					; Start line with ('A:').
		pop	bc
		add	a,'A'
		call	printb
		ld	a,':'
		call	printb
		jp	direct5
direct4:	call	spacec					; Add separator between file names.
		ld	a,':'
		call	printb
direct5:	call	spacec
		ld	b,01h					; 'extract' each file name character at a time.
direct6:	ld	a,b
		call	extract
		and	7Fh					; Strip bit 7 (status bit).
		cp	' '					; Are we at the end of the name?
		jp	nz,drect65
		pop	af					; Yes, don't print spaces at the end of a line.
		push	af
		cp	03h
		jp	nz,drect63
		ld	a,09h					; First check for no extension.
		call	extract
		and	7Fh
		cp	' '
		jp	z,direct7				; Don't print spaces.
drect63:	ld	a,' '					; Else print them.
drect65:	call	printb
		inc	b					; Bump to next character position.
		ld	a,b
		cp	0Ch					; End of the name?
		jp	nc,direct7
		cp	09h					; Nope, starting extension?
		jp	nz,direct6
		call	spacec					; Yes, add separating space.
		jp	direct6
direct7:	pop	af					; Get the next file name.
direct8:	call	chkcon					; First check console, quit on anything.
		jp	nz,direct9
		call	srchnxt					; Get next name.
		jp	direct3					; And continue with our list.
direct9:	pop	de					; Restore the stack and return to command level.
		jp	getback

; Erase Command.

erase:		call	convfst					; Convert file name.
		cp	0Bh					; Was '*.*' entered?
		jp	nz,erase1
		ld	bc,yesno				; Yes, ask for confirmation.
		call	pline
		call	getinp
		ld	hl,inbuff + 01h
		dec	(hl)					; Must be exactly 'Y'.
		jp	nz,cmmnd1
		inc	hl
		ld	a,(hl)
		cp	'Y'
		jp	nz,cmmnd1
		inc	hl
		ld	(inpoint),hl				; Save input line pointer.
erase1:		call	dselect					; Select desired disk.
		ld	de,fcbb
		call	delete					; Delete the file.
		inc	a
		call	z,none					; Not there?
		jp	getback					; Return to command level now.

yesno:		ASCIZ	"All (y/n)?"

; Type Command.

typec:		call	convfst					; Convert file name.
		jp	nz,synerr				; Wild cards not allowed.
		call	dselect					; Select indicated drive.
		call	openfcb					; Open the file.
		jp	z,type5					; Not there?
		call	crlf					; OK, start a new line on the screen.
		ld	hl,nbytes				; Initialize byte counter.
		ld	(hl),0FFh				; Set to read first sector.
type1:		ld	hl,nbytes
type2:		ld	a,(hl)					; Have we written the entire sector?
		cp	80h
		jp	c,type3
		push	hl					; Yes, read in the next one.
		call	readfcb
		pop	hl
		jp	nz,type4				; End or error?
		xor	a					; OK, clear byte counter.
		ld	(hl),a
type3:		inc	(hl)					; Count this byte.
		ld	hl,tpabuf				; And get the (a)th one from the buffer (tpabuf).
		call	addhl
		ld	a,(hl)
		cp	cntrlz					; End Of File mark?
		jp	z,getback
		call	printd					; No, print it.
		call	chkcon					; Check console, quit if anything ready.
		jp	nz,getback
		jp	type1

; Get here on an End Of File or read error.

type4:		dec	a					; Read error?
		jp	z,getback
		call	rderror					; Yes, print message.
type5:		call	resetdr					; And reset proper drive
		jp	synerr					; Now print file name with problem.

; Save Command.

save:		call	decode					; Get numeric number that follows save.
		push	af					; Save number of pages to write.
		call	convfst					; Convert file name.
		jp	nz,synerr				; Wild cards not allowed.
		call	dselect					; Select specified drive.
		ld	de,fcbb					; Now delete this file.
		push	de
		call	delete
		pop	de
		call	create					; And create it again.
		jp	z,save3					; Can't create?
		xor	a					; Clear record number byte.
		ld	(fcbb + 20h),a
		pop	af					; Convert pages to sectors.
		ld	l,a
		ld	h,00h
		add	hl,hl					; HL = number of sectors to write.
		ld	de,tpa					; And we start from here.
save1:		ld	a,h					; Done yet?
		or	l
		jp	z,save2
		dec	hl					; Nope, count this and compute the start
		push	hl					; Of the next 128 byte sector.
		ld	hl,0080h
		add	hl,de
		push	hl					; Save it and set the transfer address.
		call	dmaset
		ld	de,fcbb					; Write out this sector now.
		call	wrtrec
		pop	de					; Reset DE to the start of the last sector.
		pop	hl					; Restore sector count.
		jp	nz,save3				; Write error?
		jp	save1

; Get here after writing all of the file.

save2:		ld	de,fcbb					; Now close the file.
		call	close
		inc	a					; Did it close OK?
		jp	nz,save4

; Print out error message (no space).

save3:		ld	bc,nospace
		call	pline
save4:		call	stddma					; Reset the standard DMA address.
		jp	getback

nospace:	ASCIZ	"No space"

; Rename Command.

rename:		call	convfst					; Convert first file name.
		jp	nz,synerr				; Wild cards not allowed.
		ld	a,(chgdrv)				; Remember any change in drives specified.
		push	af
		call	dselect					; And select this drive.
		call	srchfcb					; Is this file present?
		jp	nz,rename6				; Yes, print error message.
		ld	hl,fcbb					; Yes, move this name into second slot.
		ld	de,fcbb + 10h
		ld	b,10h
		call	hl2de
		ld	hl,(inpoint)				; Get input pointer.
		ex	de,hl
		call	nonblank				; Get next non blank character.
		cp	'='					; Only allow an '=' or '_' separator.
		jp	z,rename1
		cp	'_'
		jp	nz,rename5
rename1:	ex	de,hl
		inc	hl					; OK, skip separator.
		ld	(inpoint),hl				; Save input line pointer.
		call	convfst					; Convert this second file name now.
		jp	nz,rename5				; Again, no wild cards.
		pop	af					; If a drive was specified, then it
		ld	b,a					; Must be the same as before.
		ld	hl,chgdrv
		ld	a,(hl)
		or	a
		jp	z,rename2
		cp	b
		ld	(hl),b
		jp	nz,rename5				; They were different, error.
rename2:	ld	(hl),b					; Reset as per the first file specification.
		xor	a
		ld	(fcbb),a					; Clear the drive byte of the FCB.
rename3:	call	srchfcb					; And go look for second file.
		jp	z,rename4				; Doesn't exist?
		ld	de,fcbb
		call	renam					; OK, rename the file.
		jp	getback

; Process rename errors here.

rename4:	call	none					; File not there.
		jp	getback
rename5:	call	resetdr					; Bad command format.
		jp	synerr
rename6:	ld	bc,exists				; Destination file already exists.
		call	pline
		jp	getback

exists:		ASCIZ	"File exists"

; User Command.

user:		call	decode					; Get numeric value following command.
		cp	10h					; Legal user number?
		jp	nc,synerr
		ld	e,a					; Yes but is there anything else?
		ld	a,(fcbb + 01h)
		cp	' '
		jp	z,synerr				; Yes, that is not allowed.
		call	getsetuc				; OK, set user code.
		jp	getback1

; Transient Program Command.

unknown:	call	verify					; Check for valid system (why?).
		ld	a,(fcbb + 01h)				; Anything to execute?
		cp	' '
		jp	nz,unkwn1
		ld	a,(chgdrv)				; Nope, only a drive change?
		or	a
		jp	z,getback1				; Neither???
		dec	a
		ld	(cdrive),a				; OK, store new drive.
		call	movecd					; Set (usrdrv) also.
		call	dsksel					; And select this drive.
		jp	getback1				; Then return.

; Here a file name was typed. prepare to execute it.

unkwn1:		ld	de,fcbb + 09h				; An extension specified?
		ld	a,(de)
		cp	' '
		jp	nz,synerr				; Yes, not allowed.
unkwn2:		push	de
		call	dselect					; Select specified drive.
		pop	de
		ld	hl,comfile				; Set the extension to 'COM'.
		call	move3
		call	openfcb					; And open this file.
		jp	z,unkwn9				; Not present?

; Load in the program.

		ld	hl,tpa					; Store the program starting here.
unkwn3:		push	hl
		ex	de,hl
		call	dmaset					; Set transfer address.
		ld	de,fcbb					; And read the next record.
		call	rdrec
		jp	nz,unkwn4				; End of file or read error?
		pop	hl					; Nope, bump pointer for next sector.
		ld	de,0080h
		add	hl,de
		ld	de,ccp					; Enough room for the whole file?
		ld	a,l
		sub	e
		ld	a,h
		sbc	a,d
		jp	nc,unkwn0				; No, it can't fit.
		jp	unkwn3

; Get here after finished reading.

unkwn4:		pop	hl
		dec	a					; Normal end of file?
		jp	nz,unkwn0
		call	resetdr					; Yes, reset previous drive.
		call	convfst					; Convert the first file name that follows
		ld	hl,chgdrv				; command name.
		push	hl
		ld	a,(hl)					; Set drive code in default FCB.
		ld	(fcbb),a
		ld	a,10h					; Put second name 16 bytes later.
		call	convert					; Convert second file name.
		pop	hl
		ld	a,(hl)					; And set the drive for this second file.
		ld	(fcbb + 10h),a
		xor	a					; Clear record byte in FCB.
		ld	(fcbb + 20h),a
		ld	de,tpafcb				; Move it into place at tpafcb.
		ld	hl,fcbb
		ld	b,21h
		call	hl2de
		ld	hl,inbuff + 02h				; Now move the remainder of the input
unkwn5:		ld	a,(hl)					; line down to tpabuf. Look for a non blank.
		or	a					; Or a null.
		jp	z,unkwn6
		cp	' '
		jp	z,unkwn6
		inc	hl
		jp	unkwn5

; Do the line move now. It ends in a null byte.

unkwn6:		ld	b,00h					; Keep a character count.
		ld	de,tpabuf + 01h				; Data gets put here.
unkwn7:		ld	a,(hl)					; Move it now.
		ld	(de),a
		or	a
		jp	z,unkwn8
		inc	b
		inc	hl
		inc	de
		jp	unkwn7
unkwn8:		ld	a,b					; Now store the character count.
		ld	(tpabuf),a
		call	crlf					; Clean up the screen.
		call	stddma					; Set standard transfer address.
		call	setcdrv					; Reset current drive.
		call	tpa					; And execute the program.

; Transient programs return here (or reboot).

		ld	sp,batch				; Set stack first off.
		call	movecd					; Move current drive into place (usrdrv).
		call	dsksel					; And reselect it.
		jp	cmmnd1					; Back to command mode.

; Get here if some error occurred.

unkwn9:		call	resetdr					; Improper format.
		jp	synerr
unkwn0:		ld	bc,badload				; Read error or won't fit.
		call	pline
		jp	getback

badload:	ASCIZ	"Bad load"
comfile:	ASCII	"COM"					; Command file extension.

; Get here to return to command level. We will reset the previous active drive and then either
; return to command level directly or print error message and then return.

getback:	call	resetdr					; Reset previous drive.
getback1:	call	convfst					; Convert first name in FCB.
		ld	a,(fcbb + 01h)				; If this was just a drive change request.
		sub	' '					; make sure it was valid.
		ld	hl,chgdrv
		or	(hl)
		jp	nz,synerr
		jp	cmmnd1					; OK, return to command level.

; CCP stack area.

		DS	10h
ccpstack:	EQU	$					; 16 byte stack space.

; Batch (or submit) processing information storage.

batch:		DB	00h					; Batch mode flag (0 = not active).
batchfcb:	ASCII	00h,"$$$     SUB"
		DB	00h,00h,00h,00h,00h,00h,00h,00h
		DB	00h,00h,00h,00h,00h,00h,00h,00h
		DB	00h,00h,00h,00h,00h

; File Control Block setup by the CCP.

fcbb:		DB	00h
		ASCII	"           ",00h,00h,00h,00h,00h
		ASCII	"           ",00h,00h,00h,00h,00h
rtncode:	DB	00h					; Status returned from BDOS call.
cdrive:		DB	00h					; Currently active drive.
chgdrv:		DB	00h					; Change in drives flag (0 = no change).
nbytes:		DW	0000h					; Byte counter used by Type.

		DS	0Dh					; Room for expansion?

; End of CCP.
