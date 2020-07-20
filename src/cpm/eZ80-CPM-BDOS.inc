
; BDOS - Basic Disk Operating System.

ramstart:       equ     00000h

		ORG	bdos					; Origin of BDOS.

pattrn2:        DB      00h,16h,00h,00h,00h,00h                 ; (* serial number bytes *).

fbase:		jp	fbase1

; BDOS error table.

badsctr:	DW	error1					; Bad sector on read or write.
badslct:	DW	error2					; Bad disk select.
rodisk:		DW	error3					; Disk is read only.
rofile:		DW	error4					; File is read only.

; Entry into BDOS. DE or E are the parameters passed. The function number desired is in register C.

fbase1:		ex	de,hl					; Save the DE parameters.
		ld	(params),hl
		ex	de,hl
		ld	a,e					; And save register E in particular.
		ld	(eparam),a
		ld	hl,0000h
		ld	(status),hl				; Clear return status.
		add	hl,sp
		ld	(usrstack),hl				; Save users stack pointer.
		ld	sp,stkarea				; And set our own.
		xor	a					; Clear auto select storage space.
		ld	(autoflag),a
		ld	(auto),a
		ld	hl,goback				; Set return address.
		push	hl
		ld	a,c					; Get function number.
		cp	nfuncts					; Valid function number?
		ret	nc
		ld	c,e					; Keep single register function here.
		ld	hl,functns				; Now look through the function table.
		ld	e,a
		ld	d,00h					; DE = function number.
		add	hl,de
		add	hl,de					; HL = (start of table) + 2 * (function number).
		ld	e,(hl)
		inc	hl
		ld	d,(hl)					; Now DE = address for this function.
		ld	hl,(params)				; Retrieve parameters.
		ex	de,hl					; Now DE has the original parameters.
		jp	(hl)					; Execute desired function.

; BDOS function jump table.

nfuncts:	EQU	29h					; Number of functions in following table.

functns:	DW	wboot,getcon,outcon,getrdr,punch,listd,dircio,getiob
		DW	setiob,prtstr,rdbuff,getcsts,getver,rstdsk,setdsk,openfil
		DW	closefil,getfst,getnxt,delfile,readseq,wrtseq,fcreate
		DW	renfile,getlog,getcrnt,putdma,getaloc,wrtprtd,getrov,setattr
		DW	getparm,getuser,rdrandom,wtrandom,filesize,setran,logoff,rtn
		DW	rtn,wtspecl

; BDOS error message section.

error1:		ld	hl,badsec				; Bad sector message.
		jp	error5

error2:		ld	hl,badsel				; Bad drive selected.
		call	prterr					; Print it and get a 1 character response.
		jp	rstdsk

error3:		ld	hl,diskro				; Disk is read only.
		jp	error5

error4:		ld	hl,filero				; File is read only.

error5:		call	prterr
                jp      ramstart                                ; Always reboot on these errors.

bdoserr:	ASCII	"Bdos Err On "
bdosdrv:	ASCII	" : $"
badsec:		ASCII	"Bad Sector$"
badsel:		ASCII	"Select$"
filero:		ASCII	"File "
diskro:		ASCII	"R/O$"

; Print BDOS error message.

prterr:		push	hl					; Save second message pointer.
		call	outcrlf					; Send (CR)(LF).
		ld	a,(active)				; Get active drive.
		add	a,'A'					; Make ASCII.
		ld	(bdosdrv),a				; And put in message.
		ld	bc,bdoserr				; And print it.
		call	prtmesg
		pop	bc					; Print second message line now.
		call	prtmesg

; Get an input character. We will check our 1 character buffer first. This may be set by the
; console status routine.

getchar:	ld	hl,charbuf				; Check character buffer.
		ld	a,(hl)					; Anything present already?
		ld	(hl),00h				; ...either case clear it.
		or	a
		ret	nz					; Yes, use it.
		jp	conin					; Nope, go get a character response.

; Input and echo a character.

getecho:	call	getchar					; Input a character.
		call	chkchar					; Carriage control?
		ret	c					; No, a regular control char so don't echo.
		push	af					; OK, save character now.
		ld	c,a
		call	outcon					; And echo it.
		pop	af					; Get character and return.
		ret

; Check character in A. Set the zero flag on a carriage control character and the carry flag on
; any other control character.

chkchar:	cp	cr					; Check for carriage return, line feed, backspace,
		ret	z					; or a tab.
		cp	lf
		ret	z
		cp	tab
		ret	z
		cp	bs
		ret	z
		cp	' '					; Other control char? Set carry flag.
		ret

; Check the console during output. Halt on a Control-S, then reboot on a Control-C. If anything
; else is ready, clear the zero flag and return (the calling routine may want to do something).

ckconsol:	ld	a,(charbuf)				; Check buffer.
		or	a					; If anything, just return without checking.
		jp	nz,ckcon2
		call	const					; Nothing in buffer. check console.
		and	01h					; Look at bit 0.
		ret	z					; Return if nothing.
		call	conin					; OK, get it.
		cp	cntrls					; If not Control-S, return with zero cleared.
		jp	nz,ckcon1
		call	conin					; Halt processing until another character
		cp	cntrlc					; is typed. Control-C?
		jp	z,ramstart				; Yes, reboot now.
		xor	a					; No, just pretend nothing was ever ready.
		ret
ckcon1:		ld	(charbuf),a				; Save character in buffer for later processing.
ckcon2:		ld	a,01h					; Set A to non zero to mean something is ready.
		ret

; Output C to the screen. If the printer flip-flop flag is set, we will send character to printer
; also. The console will be checked in the process.

outchar:	ld	a,(outflag)				; Check output flag.
		or	a					; Anything and we won't generate output.
		jp	nz,outchr1
		push	bc
		call	ckconsol				; Check console (we don't care what's there).
		pop	bc
		push	bc
		call	conout					; Output C to the screen.
		pop	bc
		push	bc
		ld	a,(prtflag)				; Check printer flip-flop flag.
		or	a
		call	nz,listd				; Print it also if non-zero.
		pop	bc
outchr1:	ld	a,c					; Update cursors position.
		ld	hl,curpos
		cp	del					; Rub outs don't do anything here.
		ret	z
		inc	(hl)					; Bump line pointer.
		cp	' '					; And return if a normal character.
		ret	nc
		dec	(hl)					; Restore and check for the start of the line.
		ld	a,(hl)
		or	a
		ret	z					; Ignore control characters at the start of the line.
		ld	a,c
		cp	bs					; Is it a backspace?
		jp	nz,outchr2
		dec	(hl)					; Yes, backup pointer.
		ret
outchr2:	cp	lf					; Is it a line feed?
		ret	nz					; Ignore anything else.
		ld	(hl),00h				; Reset pointer to start of line.
		ret

; Output A to the screen. If it is a control character (other than carriage control), use ^X format.

showit:		ld	a,c
		call	chkchar					; Check character.
		jp	nc,outcon				; Not a control, use normal output.
		push	af
		ld	c,'^'					; For a control character, precede it with '^'.
		call	outchar
		pop	af
		or	'@'					; And then use the letter equivalent.
		ld	c,a

; Function to output C to the console device and expand tabs if necessary.

outcon:		ld	a,c
		cp	tab					; Is it a Tab?
		jp	nz,outchar				; Use regular output.
outcon1:	ld	c,' '					; Yes it is, use spaces instead.
		call	outchar
		ld	a,(curpos)				; Go until the cursor is at a multiple of 8
		and	07h					; position.
		jp	nz,outcon1
		ret

; Echo a backspace character. Erase the previous character on the screen.

backup:		call	backup1					; Backup the screen 1 place.
		ld	c,' '					; Then blank that character.
		call	conout
backup1:	ld	c,bs					; Then back space once more.
		jp	conout

; Signal a deleted line. Print a '#' at the end and start over.

newline:	ld	c,'#'
		call	outchar					; Print this.
		call	outcrlf					; Start new line.
newln1:		ld	a,(curpos)				; Move the cursor to the starting position.
		ld	hl,starting
		cp	(hl)
		ret	nc					; There yet?
		ld	c,' '
		call	outchar					; Nope, keep going.
		jp	newln1

; Output a (CR) (LF) to the console device (screen).

outcrlf:	ld	c,cr
		call	outchar
		ld	c,lf
		jp	outchar

; Print message pointed to by BC. It will end with a '$'.

prtmesg:	ld	a,(bc)					; Check for terminating character.
		cp	'$'
		ret	z
		inc	bc
		push	bc					; Otherwise, bump pointer and print it.
		ld	c,a
		call	outcon
		pop	bc
		jp	prtmesg

; Function to execute a buffered read.

rdbuff:		ld	a,(curpos)				; Use present location as starting one.
		ld	(starting),a
		ld	hl,(params)				; Get the maximum buffer space.
		ld	c,(hl)
		inc	hl					; Point to first available space.
		push	hl					; And save.
		ld	b,00h					; Keep a character count.
rdbuf1:		push	bc
		push	hl
rdbuf2:		call	getchar					; Get the next input character.
		and	7Fh					; Strip bit 7.
		pop	hl					; Reset registers.
		pop	bc
		cp	cr					; End of the line?
		jp	z,rdbuf17
		cp	lf
		jp	z,rdbuf17
		cp	bs					; How about a backspace?
		jp	nz,rdbuf3
		ld	a,b					; Yes, but ignore at the beginning of the line.
		or	a
		jp	z,rdbuf1
		dec	b					; OK, update counter.
		ld	a,(curpos)				; If we backspace to the start of the line,
		ld	(outflag),a				; treat as a cancel (Control-X).
		jp	rdbuf10
rdbuf3:		cp	del					; User typed a rubout?
		jp	nz,rdbuf4
		ld	a,b					; Ignore at the start of the line.
		or	a
		jp	z,rdbuf1
		ld	a,(hl)					; OK, echo the previous character.
		dec	b					; And reset pointers (counters).
		dec	hl
		jp	rdbuf15
rdbuf4:		cp	cntrle					; Physical end of line?
		jp	nz,rdbuf5
		push	bc					; Yes, do it.
		push	hl
		call	outcrlf
		xor	a					; And update starting position.
		ld	(starting),a
		jp	rdbuf2
rdbuf5:		cp	cntrlp					; Control-P?
		jp	nz,rdbuf6
		push	hl					; Yes, flip the print flag flip-flop byte.
		ld	hl,prtflag
		ld	a,01h					; Prtflag = 1 - prtflag
		sub	(hl)
		ld	(hl),a
		pop	hl
		jp	rdbuf1
rdbuf6:		cp	cntrlx					; Control-X (cancel)?
		jp	nz,rdbuf8
		pop	hl
rdbuf7:		ld	a,(starting)				; Yes, backup the cursor to here.
		ld	hl,curpos
		cp	(hl)
		jp	nc,rdbuff				; Done yet?
		dec	(hl)					; No, decrement pointer and output back up one space.
		call	backup
		jp	rdbuf7
rdbuf8:		cp	cntrlu					; Control-U (cancel line)?
		jp	nz,rdbuf9
		call	newline					; Start a new line.
		pop	hl
		jp	rdbuff
rdbuf9:		cp	cntrlr					; Control-R?
		jp	nz,rdbuf14
rdbuf10:	push	bc					; Yes, start a new line and retype the old one.
		call	newline
		pop	bc
		pop	hl
		push	hl
		push	bc
rdbuf11:	ld	a,b					; Done whole line yet?
		or	a
		jp	z,rdbuf12
		inc	hl					; Nope, get next character.
		ld	c,(hl)
		dec	b					; Count it.
		push	bc
		push	hl
		call	showit					; And display it.
		pop	hl
		pop	bc
		jp	rdbuf11
rdbuf12:	push	hl					; Done with line. If we were displaying
		ld	a,(outflag)				; then update cursor position.
		or	a
		jp	z,rdbuf2
		ld	hl,curpos				; Because this line is shorter, we must
		sub	(hl)					; back up the cursor (not the screen however)
		ld	(outflag),a				; some number of positions.
rdbuf13:	call	backup					; Note that as long as (outflag) is non
		ld	hl,outflag				; zero, the screen will not be changed.
		dec	(hl)
		jp	nz,rdbuf13
		jp	rdbuf2					; Now just get the next character.

; Just a normal character, put this in our buffer and echo.

rdbuf14:	inc	hl
		ld	(hl),a					; Store character.
		inc	b					; And count it.
rdbuf15:	push	bc
		push	hl
		ld	c,a					; Echo it now.
		call	showit
		pop	hl
		pop	bc
		ld	a,(hl)					; Was it an abort request?
		cp	cntrlc					; Control-C abort?
		ld	a,b
		jp	nz,rdbuf16
		cp	01h					; Only if at start of line.
		jp	z,ramstart
rdbuf16:	cp	c					; Nope, have we filled the buffer?
		jp	c,rdbuf1
rdbuf17:	pop	hl					; Yes end the line and return.
		ld	(hl),b
		ld	c,cr
		jp	outchar					; Output (CR) and return.

; Function to get a character from the console device.

getcon:		call	getecho					; Get and echo.
		jp	setstat					; Save status and return.

; Function to get a character from the tape reader device.

getrdr:		call	reader					; Get a character from reader, set status and return.
		jp	setstat

; Function to perform direct console I/O. If C contains (0ffh) then this is an input request.
; If C contains (0feh) then this is a status request. Otherwise we are to output C.

dircio:		ld	a,c					; Test for (0ffh).
		inc	a
		jp	z,dirc1
		inc	a					; Test for (0feh).
		jp	z,const
		jp	conout					; Just output C.
dirc1:		call	const					; This is an input request.
		or	a
		jp	z,goback1				; Not ready? just return (directly).
		call	conin					; Yes, get character.
		jp	setstat					; Set status and return.

; Function to return the I/O byte.

getiob:		ld	a,(iobyte)
		jp	setstat

; Function to set the I/O byte.

setiob:		ld	hl,iobyte
		ld	(hl),c
		ret

; Function to print the character string pointed to by DE on the console device. The string ends with a '$'.

prtstr:		ex	de,hl
		ld	c,l
		ld	b,h					; Now BC points to it.
		jp	prtmesg

; Function to interrogate the console device.

getcsts:	call	ckconsol

; Get here to set the status and return to the cleanup section. Then back to the user.

setstat:	ld	(status),a
rtn:		ret

; Set the status to 1 (read or write error code).

ioerr1:		ld	a,01h
		jp	setstat

outflag:	DB	00h					; Output flag (non zero means no output).
starting:	DB	02h					; Starting position for cursor.
curpos:		DB	00h					; Cursor position (0 = start of line).
prtflag:	DB	00h					; Printer flag (Control-P toggle). List if non zero.
charbuf:	DB	00h					; Single input character buffer.

; Stack area for BDOS calls.

usrstack:	DW	0000h					; Save users stack pointer here.

		DS	30h					; 48 byte stack space.
stkarea:	EQU	$

userno:		DB	00h					; Current user number.
active:		DB	00h					; Currently active drive.
params:		DW	0000h					; Save DE parameters here on entry.
status:		DW	0000h					; Status returned from BDOS function.

; Select error occurred, jump to error routine.

slcterr:	ld	hl,badslct

; Jump to HL indirectly.

jumphl:		ld	e,(hl)
		inc	hl
		ld	d,(hl)					; Now DE contain the desired address.
		ex	de,hl
		jp	(hl)

; Block move. DE to HL, C bytes total.

de2hl:		inc	c					; Is count down to zero?
de2hl1:		dec	c
		ret	z					; Yes, we are done.
		ld	a,(de)					; No, move one more byte.
		ld	(hl),a
		inc	de
		inc	hl
		jp	de2hl1					; And repeat.

; Select the desired drive.

select:		ld	a,(active)				; Get active disk.
		ld	c,a
		call	seldsk					; Select it.
		ld	a,h					; Valid drive?
		or	l					; Valid drive?
		ret	z					; Return if not.

; Here, the BIOS returned the address of the parameter block in HL. We will extract the
; necessary pointers and save them.

		ld	e,(hl)					; Yes, get address of translation table into DE.
		inc	hl
		ld	d,(hl)
		inc	hl
		ld	(scratch1),hl				; Save pointers to scratch areas.
		inc	hl
		inc	hl
		ld	(scratch2),hl				; Ditto.
		inc	hl
		inc	hl
		ld	(scratch3),hl				; Ditto.
		inc	hl
		inc	hl
		ex	de,hl					; Now save the translation table address.
		ld	(xlate),hl
		ld	hl,dirbuf				; Put the next 8 bytes here.
		ld	c,08h					; They consist of the directory buffer
		call	de2hl					; Pointer, parameter block pointer,
		ld	hl,(diskpb)				; check and allocation vectors.
		ex	de,hl
		ld	hl,sectors				; Move parameter block into our RAM.
		ld	c,0Fh					; It is 15 bytes long.
		call	de2hl
		ld	hl,(dsksize)				; Check disk size.
		ld	a,h					; More than 256 blocks on this?
		ld	hl,bigdisk
		ld	(hl),0FFh				; Set to small.
		or	a
		jp	z,select1
		ld	(hl),00h				; Wrong, set to large.
select1:	ld	a,0FFh					; Clear the zero flag.
		or	a
		ret

; Routine to home the disk track head and clear pointers.

homedrv:	call	home					; Home the head.
		xor	a
		ld	hl,(scratch2)				; Set our track pointer also.
		ld	(hl),a
		inc	hl
		ld	(hl),a
		ld	hl,(scratch3)				; And our sector pointer.
		ld	(hl),a
		inc	hl
		ld	(hl),a
		ret

; Do the actual disk read and check the error return status.

doread:		call	read
		jp	ioret

; Do the actual disk write and handle any BIOS error.

dowrite:	call	write
ioret:		or	a
		ret	z					; Return unless an error occurred.
		ld	hl,badsctr				; Bad read/write on this sector.
		jp	jumphl

; Routine to select the track and sector that the desired block number falls in.

trksec:		ld	hl,(filepos)				; Get position of last accessed file
		ld	c,02h					; in directory and compute sector number.
		call	shiftr					; Sector number = file-position / 4.
		ld	(blknmbr),hl				; Save this as the block number of interest.
		ld	(cksumtbl),hl				; What's it doing here too?

; If the sector number has already been set (blknmbr), enter at this point.

trksec1:	ld	hl,blknmbr
		ld	c,(hl)					; Move sector number into BC.
		inc	hl
		ld	b,(hl)
		ld	hl,(scratch3)				; Get current sector number and
		ld	e,(hl)					; move this into DE.
		inc	hl
		ld	d,(hl)
		ld	hl,(scratch2)				; Get current track number.
		ld	a,(hl)					; And this into HL.
		inc	hl
		ld	h,(hl)
		ld	l,a
trksec2:	ld	a,c					; Is desired sector before current one?
		sub	e
		ld	a,b
		sbc	a,d
		jp	nc,trksec3
		push	hl					; Yes, decrement sectors by one track.
		ld	hl,(sectors)				; Get sectors per track.
		ld	a,e
		sub	l
		ld	e,a
		ld	a,d
		sbc	a,h
		ld	d,a					; Now we have backed up one full track.
		pop	hl
		dec	hl					; Adjust track counter.
		jp	trksec2
trksec3:	push	hl					; Desired sector is after current one.
		ld	hl,(sectors)				; Get sectors per track.
		add	hl,de					; Bump sector pointer to next track.
		jp	c,trksec4
		ld	a,c					; Is desired sector now before current one?
		sub	l
		ld	a,b
		sbc	a,h
		jp	c,trksec4
		ex	de,hl					; Not yes, increment track counter
		pop	hl					; and continue until it is.
		inc	hl
		jp	trksec3

; Here we have determined the track number that contains the desired sector.

trksec4:	pop	hl					; Get track number HL.
		push	bc
		push	de
		push	hl
		ex	de,hl
		ld	hl,(offset)				; Adjust for first track offset.
		add	hl,de
		ld	b,h
		ld	c,l
		call	settrk					; Select this track.
		pop	de					; Reset current track pointer.
		ld	hl,(scratch2)
		ld	(hl),e
		inc	hl
		ld	(hl),d
		pop	de
		ld	hl,(scratch3)				; Reset the first sector on this track.
		ld	(hl),e
		inc	hl
		ld	(hl),d
		pop	bc
		ld	a,c					; Now subtract the desired one.
		sub	e					; To make it relative (1 - # sectors / track).
		ld	c,a
		ld	a,b
		sbc	a,d
		ld	b,a
		ld	hl,(xlate)				; Translate this sector according to this table.
		ex	de,hl
		call	sectrn					; Let the BIOS translate it.
		ld	c,l
		ld	b,h
		jp	setsec					; And select it.

; Compute block number from record number (savnrec) and extent number (savext).

getblock:	ld	hl,blkshft				; Get logical to physical conversion.
		ld	c,(hl)					; Note that this is base 2 log of ratio.
		ld	a,(savnrec)				; Get record number.
getblk1:	or	a					; Compute A = A / 2^blkshft.
		rra
		dec	c
		jp	nz,getblk1
		ld	b,a					; Save result in B.
		ld	a,08h
		sub	(hl)
		ld	c,a					; Compute C = 8 - blkshft.
		ld	a,(savext)
getblk2:	dec	c					; Compute A = savext * 2^(8 - blkshft).
		jp	z,getblk3
		or	a
		rla
		jp	getblk2
getblk3:	add	a,b
		ret

; Routine to extract the BX block byte from the FCB pointed to by (params).
; If this is a big-disk, then these are 16 bit block numbers, else they are 8 bit numbers.
; Number is returned in HL.

extblk:		ld	hl,(params)				; Get FCB address.
		ld	de,0010h				; Block numbers start 16 bytes into FCB.
		add	hl,de
		add	hl,bc
		ld	a,(bigdisk)				; Are we using a big-disk?
		or	a
		jp	z,extblk1
		ld	l,(hl)					; No, extract an 8 bit number from the FCB.
		ld	h,00h
		ret
extblk1:	add	hl,bc					; Yes, extract a 16 bit number.
		ld	e,(hl)
		inc	hl
		ld	d,(hl)
		ex	de,hl					; Return in HL.
		ret

; Compute block number.

comblk:		call	getblock
		ld	c,a
		ld	b,00h
		call	extblk
		ld	(blknmbr),hl
		ret

; Check for a zero block number (unused).

chkblk:		ld	hl,(blknmbr)
		ld	a,l					; Is it zero?
		or	h
		ret

; Adjust physical block (blknmbr) and convert to logical sector (logsect). this is the starting
; sector of this block. The actual sector of interest is then added to this and the resulting
; sector number is stored back in (blknmbr). This will still have to be adjusted for the track number.

logical:	ld	a,(blkshft)				; Get log2(physical/logical sectors).
		ld	hl,(blknmbr)				; Get physical sector desired.
logicl1:	add	hl,hl					; Compute logical sector number.
		dec	a					; Note logical sectors are 128 bytes long.
		jp	nz,logicl1
		ld	(logsect),hl				; Save logical sector.
		ld	a,(blkmask)				; Get block mask.
		ld	c,a
		ld	a,(savnrec)				; Get next sector to access.
		and	c					; Extract the relative position within physical block.
		or	l					; And add it to logical sector.
		ld	l,a
		ld	(blknmbr),hl				; And store.
		ret

; Set HL to point to extent byte in FCB.

setext:		ld	hl,(params)
		ld	de,000Ch				; It is the twelfth byte.
		add	hl,de
		ret

; Set HL to point to record count byte in FCB and DE to next record number byte.

sethlde:	ld	hl,(params)
		ld	de,000Fh				; Record count byte (#15).
		add	hl,de
		ex	de,hl
		ld	hl,0011h				; Next record number (#32).
		add	hl,de
		ret

; Save current file data from FCB.

strdata:	call	sethlde
		ld	a,(hl)					; Get and store record count byte.
		ld	(savnrec),a
		ex	de,hl
		ld	a,(hl)					; Get and store next record number byte.
		ld	(savnxt),a
		call	setext					; Point to extent byte.
		ld	a,(extmask)				; Get extent mask.
		and	(hl)
		ld	(savext),a				; And save extent here.
		ret

; Set the next record to access. If (mode) is set to 2, then the last record byte (savnrec)
; has the correct number to access. For sequential access, (mode) will be equal to 1.

setnrec:	call	sethlde
		ld	a,(mode)				; Get sequential flag (A = 1).
		cp	02h					; A = 2 indicates that no adder is needed.
		jp	nz,stnrec1
		xor	a					; Clear adder (random access?).
stnrec1:	ld	c,a
		ld	a,(savnrec)				; Get last record number.
		add	a,c					; Increment record count.
		ld	(hl),a					; And set FCB's next record byte.
		ex	de,hl
		ld	a,(savnxt)				; Get next record byte from storage.
		ld	(hl),a					; And put this into FCB as number of records used.
		ret

; Shift HL right C bits.

shiftr:		inc	c
shiftr1:	dec	c
		ret	z
		ld	a,h
		or	a
		rra
		ld	h,a
		ld	a,l
		rra
		ld	l,a
		jp	shiftr1

; Compute the checksum for the directory buffer. return integer sum in A.

checksum:	ld	c,80h					; Length of buffer.
		ld	hl,(dirbuf)				; Get its location.
		xor	a					; Clear summation byte.
chksum1:	add	a,(hl)					; And compute sum ignoring carries.
		inc	hl
		dec	c
		jp	nz,chksum1
		ret

; Shift HL left C bits.

shiftl:		inc	c
shiftl1:	dec	c
		ret	z
		add	hl,hl					; Shift left 1 bit.
		jp	shiftl1

; Routine to set a bit in a 16 bit value contained in BC. The bit set depends on the current drive selection.

setbit:		push	bc					; Save 16 bit word.
		ld	a,(active)				; Get active drive.
		ld	c,a
		ld	hl,0001h
		call	shiftl					; Shift bit 0 into place.
		pop	bc					; Now 'or' this with the original word.
		ld	a,c
		or	l
		ld	l,a					; Low byte done, do high byte.
		ld	a,b
		or	h
		ld	h,a
		ret

; Extract the write protect status bit for the current drive. The result is returned in A, bit 0.

getwprt:	ld	hl,(wrtprt)				; Get status bytes.
		ld	a,(active)				; Which drive is current?
		ld	c,a
		call	shiftr					; Shift status such that bit 0 is the
		ld	a,l					; One of interest for this drive.
		and	01h					; And isolate it.
		ret

; Function to write protect the current disk.

wrtprtd:	ld	hl,wrtprt				; Point to status word.
		ld	c,(hl)					; Set BC equal to the status.
		inc	hl
		ld	b,(hl)
		call	setbit					; And set this bit according to current drive.
		ld	(wrtprt),hl				; Then save.
		ld	hl,(dirsize)				; Now save directory size limit.
		inc	hl					; Remember the last one.
		ex	de,hl
		ld	hl,(scratch1)				; And store it here.
		ld	(hl),e					; Put low byte.
		inc	hl
		ld	(hl),d					; Then high byte.
		ret

; Check for a read only file.

chkrofl:	call	fcb2hl					; Set HL to file entry in directory buffer.
ckrof1:		ld	de,0009h				; Look at bit 7 of the ninth byte.
		add	hl,de
		ld	a,(hl)
		rla
		ret	nc					; Return if OK.
		ld	hl,rofile				; Else, print error message and terminate.
		jp	jumphl

; Check the write protect status of the active disk.

chkwprt:	call	getwprt
		ret	z					; Return if OK.
		ld	hl,rodisk				; Else print message and terminate.
		jp	jumphl

; Routine to set HL pointing to the proper entry in the directory buffer.

fcb2hl:		ld	hl,(dirbuf)				; Get address of buffer.
		ld	a,(fcbpos)				; Relative position of file.

; Routine to add A to HL.

adda2hl:	add	a,l
		ld	l,a
		ret	nc
		inc	h					; Take care of any carry.
		ret

; Routine to get the 'S2' byte from the FCB supplied in the initial parameter specification.

gets2:		ld	hl,(params)				; Get address of FCB.
		ld	de,000Eh				; Relative position of 'S2'.
		add	hl,de
		ld	a,(hl)					; Extract this byte.
		ret

; Clear the 'S2' byte in the FCB.

clears2:	call	gets2					; This sets HL pointing to it.
		ld	(hl),00h				; Now clear it.
		ret

; Set bit 7 in the 'S2' byte of the FCB.

sets2b7:	call	gets2					; Get the byte.
		or	80h					; And set bit 7.
		ld	(hl),a					; Then store.
		ret

; Compare (filepos) with (scratch1) and set flags based on the difference. This checks to see
; if there are more file names in the directory. We are at (filepos) and there are (scratch1)
; of them to check.

morefls:	ld	hl,(filepos)				; We are here.
		ex	de,hl
		ld	hl,(scratch1)				; And don't go past here.
		ld	a,e					; Compute difference but don't keep.
		sub	(hl)
		inc	hl
		ld	a,d
		sbc	a,(hl)					; Set carry if no more names.
		ret

; Call this routine to prevent (scratch1) from being greater than (filepos).

chknmbr:	call	morefls					; Scratch1 too big?
		ret	c
		inc	de					; Yes, reset it to (filepos).
		ld	(hl),d
		dec	hl
		ld	(hl),e
		ret

; Compute HL = DE - HL

subhl:		ld	a,e					; Compute difference.
		sub	l
		ld	l,a					; Store low byte.
		ld	a,d
		sbc	a,h
		ld	h,a					; And then high byte.
		ret

; Set the directory checksum byte.

setdir:		ld	c,0FFh

; Routine to set or compare the directory checksum byte. If C = 0ffh, then this will set the
; checksum byte. Else the byte will be checked. If the check fails (the disk has been changed),
; then this disk will be write protected.

checkdir:	ld	hl,(cksumtbl)
		ex	de,hl
		ld	hl,(alloc1)
		call	subhl
		ret	nc					; OK, if (cksumtbl) > (alloc1), so return.
		push	bc
		call	checksum				; Else compute checksum.
		ld	hl,(chkvect)				; Get address of checksum table.
		ex	de,hl
		ld	hl,(cksumtbl)
		add	hl,de					; Set HL to point to byte for this drive.
		pop	bc
		inc	c					; Set or check ?
		jp	z,chkdir1
		cp	(hl)					; Check them.
		ret	z					; Return if they are the same.
		call	morefls					; Not the same, do we care?
		ret	nc
		call	wrtprtd					; Yes, mark this as write protected.
		ret
chkdir1:	ld	(hl),a					; Just set the byte.
		ret

; Do a write to the directory of the current disk.

dirwrite:	call	setdir					; Set checksum byte.
		call	dirdma					; Set directory DMA address.
		ld	c,01h					; Tell the BIOS to actually write.
		call	dowrite					; Then do the write.
		jp	defdma

; Read from the directory.

dirread:	call	dirdma					; Set the directory DMA address.
		call	doread					; And read it.

; Routine to set the DMA address to the users choice.

defdma:		ld	hl,userdma				; Reset the default DMA address and return.
		jp	dirdma1

; Routine to set the DMA address for directory work.

dirdma:		ld	hl,dirbuf

; Set the DMA address. On entry, HL points to word containing the desired DMA address.

dirdma1:	ld	c,(hl)
		inc	hl
		ld	b,(hl)					; Setup BC and go to the BIOS to set it.
		jp	setdma

; Move the directory buffer into user's DMA space.

movedir:	ld	hl,(dirbuf)				; Buffer is located here, and
		ex	de,hl
		ld	hl,(userdma)				; put it here.
		ld	c,80h					; This is its length.
		jp	de2hl					; Move it now and return.

; Check (filepos) and set the zero flag if it equals 0ffffh.

ckfilpos:	ld	hl,filepos
		ld	a,(hl)
		inc	hl
		cp	(hl)					; Are both bytes the same?
		ret	nz
		inc	a					; Yes, but are they each 0ffh?
		ret

; Set location (filepos) to ffffh.

stfilpos:	ld	hl,0FFFFh
		ld	(filepos),hl
		ret

; Move on to the next file position within the current directory buffer. If no more exist, set
; pointer to 0ffffh and the calling routine will check for this. Enter with C equal to 0ffh to
; cause the checksum byte to be set, else we will check this disk and set write protect if checksums
; are not the same (applies only if another directory sector must be read).

nxentry:	ld	hl,(dirsize)				; Get directory entry size limit.
		ex	de,hl
		ld	hl,(filepos)				; Get current count.
		inc	hl					; Go on to the next one.
		ld	(filepos),hl
		call	subhl					; HL = (dirsize) - (filepos)
		jp	nc,nxent1				; Is there more room left?
		jp	stfilpos				; No. set this flag and return.
nxent1:		ld	a,(filepos)				; Get file position within directory.
		and	03h					; Only look within this sector (only 4 entries fit).
		ld	b,05h					; Convert to relative position (32 bytes each).
nxent2:		add	a,a					; Note that this is not efficient code.
		dec	b					; 5 'add a's would be better.
		jp	nz,nxent2
		ld	(fcbpos),a				; Save it as position of FCB.
		or	a
		ret	nz					; Return if we are within buffer.
		push	bc
		call	trksec					; We need the next directory sector.
		call	dirread
		pop	bc
		jp	checkdir

; Routine to get a bit from the disk space allocation map. It is returned in A, bit position
; 0. On entry to here, set BC to the block number on the disk to check. On return, D will contain
; the original bit position for this block number and HL will point to the address for it.

ckbitmap:	ld	a,c					; Determine bit number of interest.
		and	07h					; Compute D = E = (C and 7) + 1.
		inc	a
		ld	e,a					; Save particular bit number.
		ld	d,a

; Compute BC = BC / 8.

		ld	a,c
		rrca						; Now shift right 3 bits.
		rrca
		rrca
		and	1Fh					; And clear bits 7,6,5.
		ld	c,a
		ld	a,b
		add	a,a					; Now shift B into bits 7,6,5.
		add	a,a
		add	a,a
		add	a,a
		add	a,a
		or	c					; And add in C.
		ld	c,a					; OK, C ha been completed.
		ld	a,b					; Is there a better way of doing this?
		rrca
		rrca
		rrca
		and	1Fh
		ld	b,a					; And now B is completed.

; Use this as an offset into the disk space allocation table.

		ld	hl,(alocvect)
		add	hl,bc
		ld	a,(hl)					; Now get correct byte.
ckbmap1:	rlca						; Get correct bit into position 0.
		dec	e
		jp	nz,ckbmap1
		ret

; Set or clear the bit map such that block number BC will be marked as used. On entry, if E = 0
; then this bit will be cleared, if it equals 1 then it will be set (don't use any other values).

stbitmap:	push	de
		call	ckbitmap				; Get the byte of interest.
		and	0FEh					; Clear the affected bit.
		pop	bc
		or	c					; And now set it according to C.

; Entry to restore the original bit position and then store in table. A contains the value, D
; contains the bit position (1 - 8), and HL points to the address within the space allocation
; table for this byte.

stbmap1:	rrca						; Restore original bit position.
		dec	d
		jp	nz,stbmap1
		ld	(hl),a					; And store byte in table.
		ret

; Set/clear space used bits in allocation map for this file.
; On entry, C = 1 to set the map and C = 0 to clear it.

setfile:	call	fcb2hl					; Get address of FCB
		ld	de,0010h
		add	hl,de					; Get to block number bytes.
		push	bc
		ld	c,11h					; Check all 17 bytes (max) of table.
setfl1:		pop	de
		dec	c					; Done all bytes yet?
		ret	z
		push	de
		ld	a,(bigdisk)				; Check disk size for 16 bit block numbers.
		or	a
		jp	z,setfl2
		push	bc					; Only 8 bit numbers. set BC to this one.
		push	hl
		ld	c,(hl)					; Get low byte from table, always
		ld	b,00h					; Set high byte to zero.
		jp	setfl3
setfl2:		dec	c					; For 16 bit block numbers, adjust counter.
		push	bc
		ld	c,(hl)					; Now get both the low and high bytes.
		inc	hl
		ld	b,(hl)
		push	hl
setfl3:		ld	a,c					; Block used?
		or	b
		jp	z,setfl4
		ld	hl,(dsksize)				; Is this block number within the
		ld	a,l					; space on the disk?
		sub	c
		ld	a,h
		sbc	a,b
		call	nc,stbitmap				; Yes, set the proper bit.
setfl4:		pop	hl					; Point to next block number in FCB.
		inc	hl
		pop	bc
		jp	setfl1

; Construct the space used allocation bit map for the active drive. If a file name starts with
; '$' and it is under the current user number, then (status) is set to minus 1. Otherwise it is
; not set at all.

bitmap:		ld	hl,(dsksize)				; Compute size of allocation table.
		ld	c,03h
		call	shiftr					; HL = HL / 8.
		inc	hl					; At lease 1 byte.
		ld	b,h
		ld	c,l					; Set BC to the allocation table length.

; Initialize the bitmap for this drive. Right now, the first two bytes are specified by the Disk
; Parameter Block. However a patch could be entered here if it were necessary to setup this table
; in a special manor. For example, the BIOS could determine locations of 'bad blocks' and set
; them as already 'used' in the map.

		ld	hl,(alocvect)				; Now zero out the table now.
bitmap1:	ld	(hl),00h
		inc	hl
		dec	bc
		ld	a,b
		or	c
		jp	nz,bitmap1
		ld	hl,(alloc0)				; Get initial space used by directory.
		ex	de,hl
		ld	hl,(alocvect)				; And put this into map.
		ld	(hl),e
		inc	hl
		ld	(hl),d

; End of initialization portion.

		call	homedrv					; Now home the drive.
		ld	hl,(scratch1)
		ld	(hl),03h				; Force next directory request to read
		inc	hl					; in a sector.
		ld	(hl),00h
		call	stfilpos				; Clear initial file position also.
bitmap2:	ld	c,0FFh					; Read next file name in directory
		call	nxentry					; and set checksum byte.
		call	ckfilpos				; Is there another file?
		ret	z
		call	fcb2hl					; Yes, get its address.
		ld	a,0E5h
		cp	(hl)					; Empty file entry?
		jp	z,bitmap2
		ld	a,(userno)				; No, correct user number?
		cp	(hl)
		jp	nz,bitmap3
		inc	hl
		ld	a,(hl)					; Yes, does name start with a '$'?
		sub	'$'
		jp	nz,bitmap3
		dec	a					; Yes, set status to minus one.
		ld	(status),a
bitmap3:	ld	c,01h					; Now set this file's space as used in bit map.
		call	setfile
		call	chknmbr					; Keep (scratch1) in bounds.
		jp	bitmap2

; Set the status (status) and return.

ststatus:	ld	a,(fndstat)
		jp	setstat

; Check extents in A and C. Set the zero flag if they are the same. The number of 16k chunks
; of disk space that the directory extent covers is expressed as (extmask + 1).
; No registers are modified.

samext:		push	bc
		push	af
		ld	a,(extmask)				; Get extent mask and use it to
		cpl						; compare both extent numbers.
		ld	b,a					; Save resulting mask here.
		ld	a,c					; Mask first extent and save in C.
		and	b
		ld	c,a
		pop	af					; Now mask second extent and compare
		and	b					; with the first one.
		sub	c
		and	1Fh					; (* only check bits 0 - 4 *)
		pop	bc					; The zero flag is set if they are the same.
		ret						; Restore BC and return.

; Search for the first occurrence of a file name. On entry, register C should contain the number
; of bytes of the FCB that must match.

findfst:	ld	a,0FFh
		ld	(fndstat),a
		ld	hl,counter				; Save character count.
		ld	(hl),c
		ld	hl,(params)				; Get filename to match.
		ld	(savefcb),hl				; And save.
		call	stfilpos				; Clear initial file position (set to 0ffffh).
		call	homedrv					; Home the drive.

; Entry to locate the next occurrence of a filename within the directory. The disk is not expected
; to have been changed. If it was, then it will be write protected.

findnxt:	ld	c,00h					; Write protect the disk if changed.
		call	nxentry					; Get next filename entry in directory.
		call	ckfilpos				; Is file position = 0ffffh?
		jp	z,fndnxt6				; Yes, exit now then.
		ld	hl,(savefcb)				; Set DE pointing to filename to match.
		ex	de,hl
		ld	a,(de)
		cp	0E5h					; Empty directory entry?
		jp	z,fndnxt1				; (* are we trying to reerect erased entries? *)
		push	de
		call	morefls					; More files in directory?
		pop	de
		jp	nc,fndnxt6				; No more. Exit now.
fndnxt1:	call	fcb2hl					; Get address of this FCB in directory.
		ld	a,(counter)				; Get number of bytes (characters) to check.
		ld	c,a
		ld	b,00h					; Initialize byte position counter.
fndnxt2:	ld	a,c					; Are we done with the compare?
		or	a
		jp	z,fndnxt5
		ld	a,(de)					; No, check next byte.
		cp	'?'					; Don't care about this character?
		jp	z,fndnxt4
		ld	a,b					; Get bytes position in FCB.
		cp	0Dh					; Don't care about the thirteenth byte either.
		jp	z,fndnxt4
		cp	0Ch					; Extent byte?
		ld	a,(de)
		jp	z,fndnxt3
		sub	(hl)					; Otherwise compare characters.
		and	7Fh
		jp	nz,findnxt				; Not the same, check next entry.
		jp	fndnxt4					; So far so good, keep checking.
fndnxt3:	push	bc					; Check the extent byte here.
		ld	c,(hl)
		call	samext
		pop	bc
		jp	nz,findnxt				; Not the same, look some more.

; So far the names compare. Bump pointers to the next byte and continue until all C characters have been checked.

fndnxt4:	inc	de					; Bump pointers.
		inc	hl
		inc	b
		dec	c					; Adjust character counter.
		jp	fndnxt2
fndnxt5:	ld	a,(filepos)				; Return the position of this entry.
		and	03h
		ld	(status),a
		ld	hl,fndstat
		ld	a,(hl)
		rla
		ret	nc
		xor	a
		ld	(hl),a
		ret

; Filename was not found. Set appropriate status.

fndnxt6:	call	stfilpos				; Set (filepos) to 0ffffh.
		ld	a,0FFh					; Say not located.
		jp	setstat

; Erase files from the directory. Only the first byte of the FCB will be affected. It is set to (0e5h).

erafile:	call	chkwprt					; Is disk write protected?
		ld	c,0Ch					; Only compare file names.
		call	findfst					; Get first file name.
erafil1:	call	ckfilpos				; Any found?
		ret	z					; Nope, we must be done.
		call	chkrofl					; Is file read only?
		call	fcb2hl					; Nope, get address of FCB and
		ld	(hl),0E5h				; set first byte to 'empty'.
		ld	c,00h					; Clear the space from the bit map.
		call	setfile
		call	dirwrite				; Now write the directory sector back out.
		call	findnxt					; Find the next file name.
		jp	erafil1					; And repeat process.

; Look through the space allocation map (bit map) for the next available block. Start searching
; at block number (BC - 1). The search procedure is to look for an empty block that is before the
; starting block. If not empty, look at a later block number. In this way, we return the closest
; empty block on either side of the 'target' block number. This will speed access on random devices.
; For serial devices, this should be changed to look in the forward direction first and then start
; at the front and search some more.

; On return, DE = block number that is empty and HL = 0 if no empty block was found.

fndspace:	ld	d,b					; Set DE as the block that is checked.
		ld	e,c

; Look before target block. Registers BC are used as the lower pointer and DE as the upper pointer.

fndspa1:	ld	a,c					; Is block 0 specified?
		or	b
		jp	z,fndspa2
		dec	bc					; Nope, check previous block.
		push	de
		push	bc
		call	ckbitmap
		rra						; Is this block empty?
		jp	nc,fndspa3				; Yes, use this.

; Note that the above logic gets the first block that it finds that is empty. Thus a file could be
; written 'backward' making it very slow to access. This could be changed to look for the first empty
; block and then continue until the start of this empty space is located and then used that starting
; block. This should help speed up access to some files especially on a well used disk with lots of
; fairly small 'holes'.

		pop	bc					; Nope, check some more.
		pop	de

; Now look after target block.

fndspa2:	ld	hl,(dsksize)				; Is block DE within disk limits?
		ld	a,e
		sub	l
		ld	a,d
		sbc	a,h
		jp	nc,fndspa4
		inc	de					; Yes, move on to next one.
		push	bc
		push	de
		ld	b,d
		ld	c,e
		call	ckbitmap				; Check it.
		rra						; Empty?
		jp	nc,fndspa3
		pop	de					; Nope, continue searching.
		pop	bc
		jp	fndspa1

; Empty block found. Set it as used and return with HL pointing to it (true?).

fndspa3:	rla						; Reset byte.
		inc	a					; And set bit 0.
		call	stbmap1					; Update bit map.
		pop	hl					; Set return registers.
		pop	de
		ret

; Free block was not found. If BC is not zero, then we have not checked all of the disk space.

fndspa4:	ld	a,c
		or	b
		jp	nz,fndspa1
		ld	hl,0000h				; Set 'not found' status.
		ret

; Move a complete FCB entry into the directory and write it.

fcbset:		ld	c,00h
		ld	e,20h					; Length of each entry.

; Move E bytes from the FCB pointed to by (params) into FCB in directory starting at relative
; byte C. This updated directory buffer is then written to the disk.

update:		push	de
		ld	b,00h					; Set BC to relative byte position.
		ld	hl,(params)				; Get address of FCB.
		add	hl,bc					; Compute starting byte.
		ex	de,hl
		call	fcb2hl					; Get address of FCB to update in directory.
		pop	bc					; Set C to number of bytes to change.
		call	de2hl
update1:	call	trksec					; Determine the track and sector affected.
		jp	dirwrite				; Then write this sector out.

; Routine to change the name of all files on the disk with a specified name. The FCB contains the
; current name as the first 12 characters and the new name 16 bytes into the FCB.

chgnames:	call	chkwprt					; Check for a write protected disk.
		ld	c,0Ch					; Match first 12 bytes of FCB only.
		call	findfst					; Get first name.
		ld	hl,(params)				; Get address of FCB.
		ld	a,(hl)					; Get user number.
		ld	de,0010h				; Move over to desired name.
		add	hl,de
		ld	(hl),a					; Keep same user number.
chgnam1:	call	ckfilpos				; Any matching file found?
		ret	z					; No, we must be done.
		call	chkrofl					; Check for read only file.
		ld	c,10h					; Start 16 bytes into FCB.
		ld	e,0Ch					; And update the first 12 bytes of directory.
		call	update
		call	findnxt					; Get the next file name.
		jp	chgnam1					; And continue.

; Update a files attributes. The procedure is to search for every file with the same name as shown
; in FCB (ignoring bit 7) and then to update it (which includes bit 7). No other changes are made.

saveattr:	ld	c,0Ch					; Match first 12 bytes.
		call	findfst					; Look for first filename.
savatr1:	call	ckfilpos				; Was one found?
		ret	z					; Nope, we must be done.
		ld	c,00h					; Yes, update the first 12 bytes now.
		ld	e,0Ch
		call	update					; Update filename and write directory.
		call	findnxt					; And get the next file.
		jp	savatr1					; Then continue until done.

;  Open a file (name specified in FCB).

openit:		ld	c,0Fh					; Compare the first 15 bytes.
		call	findfst					; Get the first one in directory.
		call	ckfilpos				; Any at all?
		ret	z
openit1:	call	setext					; Point to extent byte within users FCB.
		ld	a,(hl)					; And get it.
		push	af					; Save it and address.
		push	hl
		call	fcb2hl					; Point to FCB in directory.
		ex	de,hl
		ld	hl,(params)				; This is the users copy.
		ld	c,20h					; Move it into users space.
		push	de
		call	de2hl
		call	sets2b7					; Set bit 7 in 'S2' byte (unmodified).
		pop	de					; Now get the extent byte from this FCB.
		ld	hl,000Ch
		add	hl,de
		ld	c,(hl)					; Into C.
		ld	hl,000Fh				; Now get the record count byte into B.
		add	hl,de
		ld	b,(hl)
		pop	hl					; Keep the same extent as the user had originally.
		pop	af
		ld	(hl),a
		ld	a,c					; Is it the same as in the directory FCB?
		cp	(hl)
		ld	a,b					; If yes, then use the same record count.
		jp	z,openit2
		ld	a,00h					; If the user specified an extent greater than
		jp	c,openit2				; the one in the directory, then set record count to 0.
		ld	a,80h					; Otherwise set to maximum.
openit2:	ld	hl,(params)				; Set record count in users FCB to A.
		ld	de,000Fh
		add	hl,de					; Compute relative position.
		ld	(hl),a					; And set the record count.
		ret

; Move two bytes from DE to HL if (and only if) HL point to a zero value (16 bit).
; Return with zero flag set it DE was moved. Registers DE and HL are not changed, however A is.

moveword:	ld	a,(hl)					; Check for a zero word.
		inc	hl
		or	(hl)					; Both bytes zero?
		dec	hl
		ret	nz					; Nope, just return.
		ld	a,(de)					; Yes, move two bytes from DE into
		ld	(hl),a					; This zero space.
		inc	de
		inc	hl
		ld	a,(de)
		ld	(hl),a
		dec	de					; Don't disturb these registers.
		dec	hl
		ret

; Get here to close a file specified by FCB.

closeit:	xor	a					; Clear status and file position bytes.
		ld	(status),a
		ld	(filepos),a
		ld	(filepos + 01h),a
		call	getwprt					; Get write protect bit for this drive.
		ret	nz					; Just return if it is set.
		call	gets2					; Else get the 'S2' byte.
		and	80h					; And look at bit 7 (file unmodified?).
		ret	nz					; Just return if set.
		ld	c,0Fh					; Else look up this file in directory.
		call	findfst
		call	ckfilpos				; Was it found?
		ret	z					; Just return if not.
		ld	bc,0010h				; Set HL pointing to records used section.
		call	fcb2hl
		add	hl,bc
		ex	de,hl
		ld	hl,(params)				; Do the same for users specified FCB.
		add	hl,bc
		ld	c,10h					; This many bytes are present in this extent.
closeit1:	ld	a,(bigdisk)				; 8 or 16 bit record numbers?
		or	a
		jp	z,closeit4
		ld	a,(hl)					; Just 8 bit. get one from users FCB.
		or	a
		ld	a,(de)					; Now get one from directory FCB.
		jp	nz,closeit2
		ld	(hl),a					; Users byte was zero. update from directory.
closeit2:	or	a
		jp	nz,closeit3
		ld	a,(hl)					; Directories byte was zero, update from users FCB.
		ld	(de),a
closeit3:	cp	(hl)					; If neither one of these bytes were zero,
		jp	nz,closeit7				; then close error if they are not the same.
		jp	closeit5				; OK so far, get to next byte in FCBs.
closeit4:	call	moveword				; Update users FCB if it is zero.
		ex	de,hl
		call	moveword				; Update directories FCB if it is zero.
		ex	de,hl
		ld	a,(de)					; If these two values are no different,
		cp	(hl)					; then a close error occurred.
		jp	nz,closeit7
		inc	de					; Check second byte.
		inc	hl
		ld	a,(de)
		cp	(hl)
		jp	nz,closeit7
		dec	c					; Remember 16 bit values.
closeit5:	inc	de					; Bump to next item in table.
		inc	hl
		dec	c					; There are 16 entries only.
		jp	nz,closeit1				; Continue if more to do.
		ld	bc,0FFECh				; Backup 20 places (extent byte).
		add	hl,bc
		ex	de,hl
		add	hl,bc
		ld	a,(de)
		cp	(hl)					; Directory's extent already greater than the
		jp	c,closeit6				; users extent?
		ld	(hl),a					; No, update directory extent.
		ld	bc,0003h				; And update the record count byte in
		add	hl,bc					; directories FCB.
		ex	de,hl
		add	hl,bc
		ld	a,(hl)					; Get from user.
		ld	(de),a					; And put in directory.
closeit6:	ld	a,0FFh					; Set 'was open and is now closed' byte.
		ld	(closeflg),a
		jp	update1					; Update the directory now.
closeit7:	ld	hl,status				; Set return status and then return.
		dec	(hl)
		ret

; Routine to get the next empty space in the directory. It will then be cleared for use.

getempty:	call	chkwprt					; Make sure disk is not write protected.
		ld	hl,(params)				; Save current parameters (FCB).
		push	hl
		ld	hl,emptyfcb				; Use special one for empty space.
		ld	(params),hl
		ld	c,01h					; Search for first empty spot in directory.
		call	findfst					; (* only check first byte *)
		call	ckfilpos				; None?
		pop	hl
		ld	(params),hl				; Restore original FCB address.
		ret	z					; Return if no more space.
		ex	de,hl
		ld	hl,000Fh				; Point to number of records for this file.
		add	hl,de
		ld	c,11h					; And clear all of this space.
		xor	a
getmt1:		ld	(hl),a
		inc	hl
		dec	c
		jp	nz,getmt1
		ld	hl,000Dh				; Clear the 'S1' byte also.
		add	hl,de
		ld	(hl),a
		call	chknmbr					; Keep (scratch1) within bounds.
		call	fcbset					; Write out this FCB entry to directory.
		jp	sets2b7					; Set 'S2' byte bit 7 (unmodified at present).

; Routine to close the current extent and open the next one for reading.

getnext:	xor	a
		ld	(closeflg),a				; Clear close flag.
		call	closeit					; Close this extent.
		call	ckfilpos
		ret	z					; Not there???
		ld	hl,(params)				; Get extent byte.
		ld	bc,000Ch
		add	hl,bc
		ld	a,(hl)					; And increment it.
		inc	a
		and	1Fh					; Keep within range 0 - 31.
		ld	(hl),a
		jp	z,gtnext1				; Overflow?
		ld	b,a					; Mask extent byte.
		ld	a,(extmask)
		and	b
		ld	hl,closeflg				; Check close flag (0ffh is OK).
		and	(hl)
		jp	z,gtnext2				; If zero, we must read in next extent.
		jp	gtnext3					; Else, it is already in memory.
gtnext1:	ld	bc,0002h				; Point to the 'S2' byte.
		add	hl,bc
		inc	(hl)					; And bump it.
		ld	a,(hl)					; Too many extents?
		and	0Fh
		jp	z,gtnext5				; Yes, set error code.

; Get here to open the next extent.

gtnext2:	ld	c,0Fh					; Set to check first 15 bytes of FCB.
		call	findfst					; Find the first one.
		call	ckfilpos				; None available?
		jp	nz,gtnext3
		ld	a,(rdwrtflg)				; No extent present. Can we open an empty one?
		inc	a					; 0ffh means reading (so not possible).
		jp	z,gtnext5				; Or an error.
		call	getempty				; We are writing, get an empty entry.
		call	ckfilpos				; None?
		jp	z,gtnext5				; Error if true.
		jp	gtnext4					; Else we are almost done.
gtnext3:	call	openit1					; Open this extent.
gtnext4:	call	strdata					; Move in updated data (rec #, extent #, etc.)
		xor	a					; Clear status and return.
		jp	setstat

; Error in extending the file. Too many extents were needed or not enough space on the disk.

gtnext5:	call	ioerr1					; Set error code, clear bit 7 of 'S2'
		jp	sets2b7					; So this is not written on a close.

; Read a sequential file.

rdseq:		ld	a,01h					; Set sequential access mode.
		ld	(mode),a
rdseq1:		ld	a,0FFh					; Don't allow reading unwritten space.
		ld	(rdwrtflg),a
		call	strdata					; Put rec# and ext# into FCB.
		ld	a,(savnrec)				; Get next record to read.
		ld	hl,savnxt				; Get number of records in extent.
		cp	(hl)					; Within this extent?
		jp	c,rdseq2
		cp	80h					; No. is this extent fully used?
		jp	nz,rdseq3				; No. End-Of-File.
		call	getnext					; Yes, open the next one.
		xor	a					; Reset next record to read.
		ld	(savnrec),a
		ld	a,(status)				; Check on open, successful?
		or	a
		jp	nz,rdseq3				; No, error.
rdseq2:		call	comblk					; OK. Compute block number to read.
		call	chkblk					; Check it. Within bounds?
		jp	z,rdseq3				; No, error.
		call	logical					; Convert (blknmbr) to logical sector (128 byte).
		call	trksec1					; Set the track and sector for this block #.
		call	doread					; And read it.
		jp	setnrec					; And set the next record to be accessed.

; Read error occurred. Set status and return.

rdseq3:		jp	ioerr1

; Write the next sequential record.

wtseq:		ld	a,01h					; Set sequential access mode.
		ld	(mode),a
wtseq1:		ld	a,00h					; Allow an addition empty extent to be opened.
		ld	(rdwrtflg),a
		call	chkwprt					; Check write protect status.
		ld	hl,(params)
		call	ckrof1					; Check for read only file, HL already set to FCB.
		call	strdata					; Put updated data into FCB.
		ld	a,(savnrec)				; Get record number to write.
		cp	80h					; Within range?
		jp	nc,ioerr1				; No, error(?).
		call	comblk					; Compute block number.
		call	chkblk					; Check number.
		ld	c,00h					; Is there one to write to?
		jp	nz,wtseq6				; Yes, go do it.
		call	getblock				; Get next block number within FCB to use.
		ld	(relblock),a				; And save.
		ld	bc,0000h				; Start looking for space from the start
		or	a					; if none allocated as yet.
		jp	z,wtseq2
		ld	c,a					; Extract previous block number from FCB
		dec	bc					; so we can be closest to it.
		call	extblk
		ld	b,h
		ld	c,l
wtseq2:		call	fndspace				; Find the next empty block nearest number BC.
		ld	a,l					; Check for a zero number.
		or	h
		jp	nz,wtseq3
		ld	a,02h					; No more space?
		jp	setstat
wtseq3:		ld	(blknmbr),hl				; Save block number to access.
		ex	de,hl					; Put block number into DE.
		ld	hl,(params)				; Now we must update the FCB for this
		ld	bc,0010h				; newly allocated block.
		add	hl,bc
		ld	a,(bigdisk)				; 8 or 16 bit block numbers?
		or	a
		ld	a,(relblock)				; (* update this entry *)
		jp	z,wtseq4				; Zero means 16 bit ones.
		call	adda2hl					; HL = HL + A
		ld	(hl),e					; Store new block number.
		jp	wtseq5
wtseq4:		ld	c,a					; Compute spot in this 16 bit table.
		ld	b,00h
		add	hl,bc
		add	hl,bc
		ld	(hl),e					; Stuff block number DE there.
		inc	hl
		ld	(hl),d
wtseq5:		ld	c,02h					; Set C to indicate writing to unused disk space.
wtseq6:		ld	a,(status)				; Are we OK so far?
		or	a
		ret	nz
		push	bc					; Yes, save write flag for BIOS (register C).
		call	logical					; Convert (blknmbr) over to logical sectors.
		ld	a,(mode)				; Get access mode flag (1 = sequential,
		dec	a					; 0 = random, 2 = special?).
		dec	a
		jp	nz,wtseq9

; Special random I/O from function #40. Maybe for M/PM, but the current block, if it has not been
; written to, will be zeroed out and then written (reason?).

		pop	bc
		push	bc
		ld	a,c					; Get write status flag (2 = writing unused space).
		dec	a
		dec	a
		jp	nz,wtseq9
		push	hl
		ld	hl,(dirbuf)				; Zero out the directory buffer.
		ld	d,a					; Note that A is zero here.
wtseq7:		ld	(hl),a
		inc	hl
		inc	d					; Do 128 bytes.
		jp	p,wtseq7
		call	dirdma					; Tell the BIOS the DMA address for directory access.
		ld	hl,(logsect)				; Get sector that starts current block.
		ld	c,02h					; Set 'writing to unused space' flag.
wtseq8:		ld	(blknmbr),hl				; Save sector to write.
		push	bc
		call	trksec1					; Determine its track and sector numbers.
		pop	bc
		call	dowrite					; Now write out 128 bytes of zeros.
		ld	hl,(blknmbr)				; Get sector number.
		ld	c,00h					; Set normal write flag.
		ld	a,(blkmask)				; Determine if we have written the entire
		ld	b,a					; physical block.
		and	l
		cp	b
		inc	hl					; Prepare for the next one.
		jp	nz,wtseq8				; Continue until (blkmask + 1) sectors written.
		pop	hl					; Reset next sector number.
		ld	(blknmbr),hl
		call	defdma					; And reset DMA address.

; Normal disk write. Set the desired track and sector then do the actual write.

wtseq9:		call	trksec1					; Determine track and sector for this write.
		pop	bc					; Get write status flag.
		push	bc
		call	dowrite					; And write this out.
		pop	bc
		ld	a,(savnrec)				; Get number of records in file.
		ld	hl,savnxt				; Get last record written.
		cp	(hl)
		jp	c,wtseq10
		ld	(hl),a					; We have to update record count.
		inc	(hl)
		ld	c,02h

;* This area has been patched to correct disk update problem when using blocking and de-blocking in the BIOS.

wtseq10:	nop						; Was 'dcr c'
		nop						; Was 'dcr c'
		ld	hl,0000h				; Was 'jnz wtseq99'

; * End of patch.

		push	af
		call	gets2					; Set 'extent written to' flag.
		and	7Fh					; (* clear bit 7 *)
		ld	(hl),a
		pop	af					; Get record count for this extent.
wtseq99:	cp	7Fh					; Is it full?
		jp	nz,wtseq12
		ld	a,(mode)				; Yes, are we in sequential mode?
		cp	01h
		jp	nz,wtseq12
		call	setnrec					; Yes, set next record number.
		call	getnext					; And get next empty space in directory.
		ld	hl,status				; OK?
		ld	a,(hl)
		or	a
		jp	nz,wtseq11
		dec	a					; Yes, set record count to -1.
		ld	(savnrec),a
wtseq11:	ld	(hl),00h				; Clear status.
wtseq12:	jp	setnrec					; Set next record to access.

; For random I/O, set the FCB for the desired record number based on the 'R0,R1,R2' bytes.
; These bytes in the FCB are used as follows:

;       FCB+35            FCB+34            FCB+33
;  |     'R-2'      |      'R-1'      |      'R-0'     |
;  |7             0 | 7             0 | 7             0|
;  |0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0|
;  |    overflow   | | extra |  extent   |   record #  |
;  | ______________| |_extent|__number___|_____________|
;                     also 'S2'

; On entry, register C contains 0ffh if this is a read and thus we can not access unwritten
; disk space. Otherwise, another extent will be opened (for writing) if required.

position:	xor	a					; Set random I/O flag.
		ld	(mode),a

; Special entry (function #40). M/PM?

positn1:	push	bc					; Save read/write flag.
		ld	hl,(params)				; Get address of FCB.
		ex	de,hl
		ld	hl,0021h				; Now get byte 'R0'.
		add	hl,de
		ld	a,(hl)
		and	7Fh					; Keep bits 0 - 6 for the record number to access.
		push	af
		ld	a,(hl)					; Now get bit 7 of 'R0' and bits 0 - 3 of 'R1'.
		rla
		inc	hl
		ld	a,(hl)
		rla
		and	1Fh					; And save this in bits 0 - 4 of C.
		ld	c,a					; This is the extent byte.
		ld	a,(hl)					; Now get the extra extent byte.
		rra
		rra
		rra
		rra
		and	0Fh
		ld	b,a					; And save it in B.
		pop	af					; Get record number back to A.
		inc	hl					; Check overflow byte 'R2'.
		ld	l,(hl)
		inc	l
		dec	l
		ld	l,06h					; Prepare for error.
		jp	nz,positn5				; Out of disk space error.
		ld	hl,0020h				; Store record number into FCB.
		add	hl,de
		ld	(hl),a
		ld	hl,000Ch				; And now check the extent byte.
		add	hl,de
		ld	a,c
		sub	(hl)					; Same extent as before?
		jp	nz,positn2
		ld	hl,000Eh				; Yes, check extra extent byte 'S2' also.
		add	hl,de
		ld	a,b
		sub	(hl)
		and	7Fh
		jp	z,positn3				; Same, we are almost done then.

;  Get here when another extent is required.

positn2:	push	bc
		push	de
		call	closeit					; Close current extent.
		pop	de
		pop	bc
		ld	l,03h					; Prepare for error.
		ld	a,(status)
		inc	a
		jp	z,positn4				; Close error.
		ld	hl,000Ch				; Put desired extent into FCB now.
		add	hl,de
		ld	(hl),c
		ld	hl,000Eh				; And store extra extent byte 'S2'.
		add	hl,de
		ld	(hl),b
		call	openit					; Try and get this extent.
		ld	a,(status)				; Was it there?
		inc	a
		jp	nz,positn3
		pop	bc					; No. can we create a new one (writing?).
		push	bc
		ld	l,04h					; Prepare for error.
		inc	c
		jp	z,positn4				; Nope, reading unwritten space error.
		call	getempty				; Yes we can, try to find space.
		ld	l,05h					; Prepare for error.
		ld	a,(status)
		inc	a
		jp	z,positn4				; Out of space?

; Normal return location. Clear error code and return.

positn3:	pop	bc					; Restore stack.
		xor	a					; And clear error code byte.
		jp	setstat

; Error. Set the 'S2' byte to indicate this (why?).

positn4:	push	hl
		call	gets2
		ld	(hl),0C0h
		pop	hl

; Return with error code (presently in l).

positn5:	pop	bc
		ld	a,l					; Get error code.
		ld	(status),a
		jp	sets2b7

; Read a random record.

readran:	ld	c,0FFh					; Set 'read' status.
		call	position				; Position the file to proper record.
		call	z,rdseq1				; And read it as usual (if no errors).
		ret

; Write to a random record.

writeran:	ld	c,00h					; Set 'writing' flag.
		call	position				; Position the file to proper record.
		call	z,wtseq1				; And write as usual (if no errors).
		ret

; Compute the random record number.
; Enter with HL pointing to a FCB and DE contains a relative location of a record number.
; On exit, C contains the 'R0' byte, B the 'R1' byte, and A the 'R2' byte.

; On return, the zero flag is set if the record is within bounds, otherwise, an overflow occurred.

comprand:	ex	de,hl					; Save FCB pointer in DE.
		add	hl,de					; Compute relative position of record #.
		ld	c,(hl)					; Get record number into BC.
		ld	b,00h
		ld	hl,000ch				; Now get extent.
		add	hl,de
		ld	a,(hl)					; Compute BC = (record #) + (extent) * 128.
		rrca						; Move lower bit into bit 7.
		and	80h					; And ignore all other bits.
		add	a,c					; Add to our record number.
		ld	c,a
		ld	a,00h					; Take care of any carry.
		adc	a,b
		ld	b,a
		ld	a,(hl)					; Now get the upper bits of extent into
		rrca						; Bit positions 0 - 3.
		and	0Fh					; And ignore all others.
		add	a,b					; Add this in to 'R1' byte.
		ld	b,a
		ld	hl,000Eh				; Get the 'S2' byte (extra extent).
		add	hl,de
		ld	a,(hl)
		add	a,a					; And shift it left 4 bits (bits 4 - 7).
		add	a,a
		add	a,a
		add	a,a
		push	af					; Save carry flag (bit 0 of flag byte).
		add	a,b					; Now add extra extent into 'R1'.
		ld	b,a
		push	af					; And save carry (overflow byte 'R2').
		pop	hl					; Bit 0 of L is the overflow indicator.
		ld	a,l
		pop	hl					; And same for first carry flag.
		or	l					; Either one of these set?
		and	01h					; Only check the carry flags.
		ret

; Routine to setup the FCB (bytes 'R0', 'R1', 'R2') to reflect the last record used for a
; random (or other) file. This reads the directory and looks at all extents computing the
; largest record number for each and keeping the maximum value only. Then 'R0', 'R1', and
; 'R2' will reflect this maximum record number. This is used to compute the space used by
; a random file.

ransize:	ld	c,0Ch					; Look through directory for first entry with
		call	findfst					; this name.
		ld	hl,(params)				; Zero out the 'R0, R1, R2' bytes.
		ld	de,0021h
		add	hl,de
		push	hl
		ld	(hl),d					; Note that D = 0.
		inc	hl
		ld	(hl),d
		inc	hl
		ld	(hl),d
ransiz1:	call	ckfilpos				; Is there an extent to process?
		jp	z,ransiz3				; No, we are done.
		call	fcb2hl					; Set HL pointing to proper FCB in dir.
		ld	de,000Fh				; Point to last record in extent.
		call	comprand				; And compute random parameters.
		pop	hl
		push	hl					; Now check these values against those
		ld	e,a					; already in FCB.
		ld	a,c					; The carry flag will be set if those
		sub	(hl)					; in the FCB represent a larger size than
		inc	hl					; this extent does.
		ld	a,b
		sbc	a,(hl)
		inc	hl
		ld	a,e
		sbc	a,(hl)
		jp	c,ransiz2
		ld	(hl),e					; We found a larger (in size) extent.
		dec	hl					; Stuff these values into FCB.
		ld	(hl),b
		dec	hl
		ld	(hl),c
ransiz2:	call	findnxt					; Now get the next extent.
		jp	ransiz1					; Continue till all done.
ransiz3:	pop	hl					; We are done, restore the stack and
		ret						; return.

; Function to return the random record position of a given file which has been read in sequential mode up to now.

setran:		ld	hl,(params)				; Point to FCB.
		ld	de,0020h				; And to last used record.
		call	comprand				; Compute random position.
		ld	hl,0021h				; Now stuff these values into FCB.
		add	hl,de
		ld	(hl),c					; Move 'R0'.
		inc	hl
		ld	(hl),b					; And 'R1'.
		inc	hl
		ld	(hl),a					; And lastly 'R2'.
		ret

; This routine select the drive specified in (active) and update the login vector and bitmap table
; if this drive was not already active.

logindrv:	ld	hl,(login)				; Get the login vector.
		ld	a,(active)				; Get the default drive.
		ld	c,a
		call	shiftr					; Position active bit for this drive
		push	hl					; into bit 0.
		ex	de,hl
		call	select					; Select this drive.
		pop	hl
		call	z,slcterr				; Valid drive?
		ld	a,l					; Is this a newly activated drive?
		rra
		ret	c
		ld	hl,(login)				; Yes, update the login vector.
		ld	c,l
		ld	b,h
		call	setbit
		ld	(login),hl				; And save.
		jp	bitmap					; Now update the bitmap.

; Function to set the active disk number.

setdsk:		ld	a,(eparam)				; Get parameter passed and see if this
		ld	hl,active				; represents a change in drives.
		cp	(hl)
		ret	z
		ld	(hl),a					; Yes it does, log it in.
		jp	logindrv

; This is the 'auto disk select' routine. The first byte of the FCB is examined for a drive
; specification. If non zero then the drive will be selected and logged in.

autosel:	ld	a,0FFh					; Say 'auto-select activated'.
		ld	(auto),a
		ld	hl,(params)				; Get drive specified.
		ld	a,(hl)
		and	1Fh					; Look at lower 5 bits.
		dec	a					; Adjust for (1 = a, 2 = b) etc.
		ld	(eparam),a				; And save for the select routine.
		cp	1Eh					; Check for 'no change' condition.
		jp	nc,autosl1				; Yes, don't change.
		ld	a,(active)				; We must change, save currently active
		ld	(olddrv),a				; drive.
		ld	a,(hl)					; And save first byte of FCB also.
		ld	(autoflag),a				; This must be non-zero.
		and	0E0h					; What's this for (bits 6,7 are used for
		ld	(hl),a					; something)?
		call	setdsk					; Select and log in this drive.
autosl1:	ld	a,(userno)				; Move user number into FCB.
		ld	hl,(params)				; (* upper half of first byte *)
		or	(hl)
		ld	(hl),a
		ret						; And return (all done).

; Function to return the current CP/M version number.

getver:		ld	a,22h					; Version 2.2
		jp	setstat

; Function to reset the disk system.

rstdsk:		ld	hl,0000h				; Clear write protect status and log
		ld	(wrtprt),hl				; in vector.
		ld	(login),hl
		xor	a					; Select drive 'A'.
		ld	(active),a
		ld	hl,tpabuf				; Setup default DMA address.
		ld	(userdma),hl
		call	defdma
		jp	logindrv				; Now log in drive 'A'.

; Function to open a specified file.

openfil:	call	clears2					; Clear 'S2' byte.
		call	autosel					; Select proper disk.
		jp	openit					; And open the file.

; Function to close a specified file.

closefil:	call	autosel					; Select proper disk.
		jp	closeit					; And close the file.

; Function to return the first occurrence of a specified file name. If the first byte of the FCB
; is '?' then the name will not be checked (get the first entry no matter what).

getfst:		ld	c,00h					; Prepare for special search.
		ex	de,hl
		ld	a,(hl)					; Is first byte a '?'?
		cp	'?'
		jp	z,getfst1				; Yes, just get very first entry (zero length match).
		call	setext					; Get the extension byte from FCB.
		ld	a,(hl)					; Is it '?'? if yes, then we want
		cp	'?'					; an entry with a specific 'S2' byte.
		call	nz,clears2				; Otherwise, look for a zero 'S2' byte.
		call	autosel					; Select proper drive.
		ld	c,0Fh					; Compare bytes 0 - 14 in FCB (12 & 13 excluded).
getfst1:	call	findfst					; Find an entry and then move it into
		jp	movedir					; the users DMA space.

; Function to return the next occurrence of a file name.

getnxt:		ld	hl,(savefcb)				; Restore pointers. Note that no
		ld	(params),hl				; other BDOS calls are allowed.
		call	autosel					; No error will be returned, but the
		call	findnxt					; results will be wrong.
		jp	movedir

; Function to delete a file by name.

delfile:	call	autosel					; Select proper drive.
		call	erafile					; Erase the file.
		jp	ststatus				; Set status and return.

; Function to execute a sequential read of the specified record number.

readseq:	call	autosel					; Select proper drive then read.
		jp	rdseq

; Function to write the net sequential record.

wrtseq:		call	autosel					; Select proper drive then write.
		jp	wtseq

; Create a file function.

fcreate:	call	clears2					; Clear the 'S2' byte on all creates.
		call	autosel					; Select proper drive and get the next
		jp	getempty				; empty directory space.

; Function to rename a file.

renfile:	call	autosel					; Select proper drive and then switch
		call	chgnames				; file names.
		jp	ststatus

; Function to return the login vector.

getlog:		ld	hl,(login)
		jp	getprm1

; Function to return the current disk assignment.

getcrnt:	ld	a,(active)
		jp	setstat

; Function to set the DMA address.

putdma:		ex	de,hl
		ld	(userdma),hl				; Save in our space and then get to
		jp	defdma					; the BIOS with this also.

; Function to return the allocation vector.

getaloc:	ld	hl,(alocvect)
		jp	getprm1

; Function to return the read-only status vector.

getrov:		ld	hl,(wrtprt)
		jp	getprm1

; Function to set the file attributes (read-only, system).

setattr:	call	autosel					; Select proper drive then save attributes.
		call	saveattr
		jp	ststatus

; Function to return the address of the disk parameter block for the current drive.

getparm:	ld	hl,(diskpb)
getprm1:	ld	(status),hl
		ret

; Function to get or set the user number. If E was 0ffh then this is a request to return the
; current user number. Else set the user number from E.

getuser:	ld	a,(eparam)				; Get parameter.
		cp	0FFh					; Get user number?
		jp	nz,setuser
		ld	a,(userno)				; Yes, just do it.
		jp	setstat
setuser:	and	1Fh					; No, we should set it instead. Keep low
		ld	(userno),a				; bits (0 - 4) only.
		ret

; Function to read a random record from a file.

rdrandom:	call	autosel					; Select proper drive and read.
		jp	readran

; Function to compute the file size for random files.

wtrandom:	call	autosel					; Select proper drive and write.
		jp	writeran

; Function to compute the size of a random file.

filesize:	call	autosel					; Select proper drive and check file length
		jp	ransize

; Function #37.
; This allows a program to log off any drives.
; On entry, set DE to contain a word with bits set for those drives that are to be logged off. The
; log-in vector and the write protect vector will be updated. This must be a M/PM special function.

logoff:		ld	hl,(params)				; Get drives to log off.
		ld	a,l					; For each bit that is set, we want
		cpl						; to clear that bit in (login)
		ld	e,a					; and (wrtprt).
		ld	a,h
		cpl
		ld	hl,(login)				; Reset the login vector.
		and	h
		ld	d,a
		ld	a,l
		and	e
		ld	e,a
		ld	hl,(wrtprt)
		ex	de,hl
		ld	(login),hl				; And save.
		ld	a,l					; Now do the write protect vector.
		and	e
		ld	l,a
		ld	a,h
		and	d
		ld	h,a
		ld	(wrtprt),hl				; And save. All done.
		ret

; Get here to return to the user.

goback:		ld	a,(auto)				; Was auto select activated?
		or	a
		jp	z,goback1
		ld	hl,(params)				; Yes, but was a change made?
		ld	(hl),00h				; (* reset first byte of FCB *)
		ld	a,(autoflag)
		or	a
		jp	z,goback1
		ld	(hl),a					; Yes, reset first byte properly.
		ld	a,(olddrv)				; And get the old drive and select it.
		ld	(eparam),a
		call	setdsk
goback1:	ld	hl,(usrstack)				; Reset the users stack pointer.
		ld	sp,hl
		ld	hl,(status)				; Get return status.
		ld	a,l					; Force version 1.4 compatibility.
		ld	b,h
		ret						; And go back to user.

; Function #40.
; This is a special entry to do random I/O.
; For the case where we are writing to unused disk space, this space will be zeroed out first.
; This must be a M/PM special purpose function, because why would any normal program even care
; about the previous contents of a sector about to be written over.

wtspecl:	call	autosel					; Select proper drive.
		ld	a,02h					; Use special write mode.
		ld	(mode),a
		ld	c,00h					; Set write indicator.
		call	positn1					; Position the file.
		call	z,wtseq1				; And write (if no errors).
		ret

; BDOS data storage.

emptyfcb:	DB	0E5h					; Empty directory segment indicator.
wrtprt:		DW	0000h					; Write protect status for all 16 drives.
login:		DW	0000h					; Drive active word (1 bit per drive).
userdma:	DW	0080h					; User's DMA address (defaults to 0080h).

; Scratch areas from parameter block.

scratch1:	DW	0000h					; Relative position within directory segment for file (0 - 3).
scratch2:	DW	0000h					; Last selected track number.
scratch3:	DW	0000h					; Last selected sector number.

; Disk storage areas from parameter block.

dirbuf:		DW	0000h					; Address of directory buffer to use.
diskpb:		DW	0000h					; Contains address of Disk Parameter Block.
chkvect:	DW	0000h					; Address of check vector.
alocvect:	DW	0000h					; Address of allocation vector (bit map).

; Parameter block returned from the BIOS.

sectors:	DW	0000h					; Sectors per track from BIOS.
blkshft:	DB	00h					; Block shift.
blkmask:	DB	00h					; Block mask.
extmask:	DB	00h					; Extent mask.
dsksize:	DW	0000h					; Disk size from BIOS (number of blocks - 1).
dirsize:	DW	0000h					; Directory size.
alloc0:		DW	0000h					; Storage for first bytes of bit map (directory space used).
alloc1:		DW	0000h
offset:		DW	0000h					; First usable track number.
xlate:		DW	0000h					; Sector translation table address.

closeflg:	DB	00h					; Close flag (= 0ffh is extent written OK).
rdwrtflg:	DB	00h					; Read/write flag (0ffh = read, 0 = write).
fndstat:	DB	00h					; Filename found status (0 = found first entry).
mode:		DB	00h					; I/O mode select (0 = random, 1 = sequential, 2 = special random).
eparam:		DB	00h					; Storage for register E on entry to BDOS.
relblock:	DB	00h					; Relative position within FCB of block number written.
counter:	DB	00h					; Byte counter for directory name searches.
savefcb:	DW	0000h,0000h				; Save space for address of FCB (for directory searches).
bigdisk:	DB	00h					; If = 0 then disk is > 256 blocks long.
auto:		DB	00h					; If non-zero, then auto select activated.
olddrv:		DB	00h					; On auto select, storage for previous drive.
autoflag:	DB	00h					; If non-zero, then auto select changed drives.
savnxt:		DB	00h					; Storage for next record number to access.
savext:		DB	00h					; Storage for extent number of file.
savnrec:	DW	0000h					; Storage for number of records in file.
blknmbr:	DW	0000h					; Block number (physical sector) used within a file or logical sect
logsect:	DW	0000h					; Starting logical (128 byte) sector of block (physical sector).
fcbpos:		DB	00h					; Relative position within buffer for FCB of file of interest.
filepos:	DW	0000h					; Files position within directory (0 to max entries - 1).
cksumtbl:	DB	00h,00h,00h,00h,00h,00h,00h,00h		; Disk directory buffer checksum bytes. One for each possible drive.
		DB	00h,00h,00h,00h,00h,00h,00h,00h

;   Extra space ?

		DB	00h,00h,00h,00h,00h,00h,00h

; End of BDOS.

