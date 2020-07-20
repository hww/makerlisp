; --------------------------------------------------------------------------
; BIOS
; --------------------------------------------------------------------------
                ORG             bios

; --------------------------------------------------------------------------
; bios jump table for app, ccp, and bdos use
; --------------------------------------------------------------------------
boot:           jp              biosboot
wboot:          jp              bioswboot
const:          jp              biosconst
conin:          jp              biosconin
conout:         jp              biosconout
listd:          jp              bioslist
punch:          jp              biospunch
reader:         jp              biosreader
home:           jp              bioshome
seldsk:         jp              biosseldsk
settrk:         jp              biossettrk
setsec:         jp              biossetsec
setdma:         jp              biossetdma
read:           jp              biosread
write:          jp              bioswrite
listst:         jp              bioslistst
sectrn:         jp              biossectrn
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
; The following is a list of function numbers for the bios function dispatch.
; --------------------------------------------------------------------------
reload          equ             0
concharinst     equ             1
concharin       equ             2
concharoutst    equ             3
concharout      equ             4
constringout    equ             5
listcharoutst   equ             6
listcharout     equ             7
punchcharoutst  equ             8
punchcharout    equ             9
readercharinst  equ             10
readercharin    equ             11
selectdisk      equ             12
homeHead        equ             13
settrack        equ             14
setsector       equ             15
setaddress      equ             16
readsector      equ             17
writesector     equ             18
translate       equ             19
setrambase      equ             20
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
; boot defaults
; --------------------------------------------------------------------------
bootdma         equ             00080h          ; default dma for starting ccp
locstk          equ             00000h          ; stack used by bios when calling out
bootiob         equ             000h            ; default iobyte for starting ccp
bootudv         equ             000h            ; default user and drive for starting ccp
; --------------------------------------------------------------------------
; jump instruction op code value for insertion into jump instructions
; that are placed at start of ram for cpm
; --------------------------------------------------------------------------
jpinst          equ             0C3h            ; jp instruction
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
; coold boot service tail - must survive disk table load
; --------------------------------------------------------------------------
biosboot1:      ld              bc,ramStart     ; tell the cbios about free ram
                ld              l,setrambase
                call            sentry

                jr              gocpm           ; finish with common init
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
; warm boot service
; --------------------------------------------------------------------------
bioswboot:      ld              sp,locstk       ; stack during init

                ld              l,reload        ; get ccp and bdos back in ram
                call            sentry

gocpm:          ld              a,jpinst        ; set up CP/M jumps to bdos and wboot

                ld              hl,bios+3       ; wboot entry point
                ld              (biosjp),a
                ld              (biosjp+1),hl

                ld              hl,bdos+6       ; entry point to bdos
                ld              (bdosen),a
                ld              (bdosen+1),hl

                ld              bc,bootdma      ; set default dma address
                ld              l,setaddress
                call            sentry

                ld              a,(usrdrv)      ; get current disk number
                ld              c,a             ; send to the ccp

                ei                              ; finally!!!
                jp              ccp             ; start command processor
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
; character io services - iobyte is fetched and passed to middle layer
; --------------------------------------------------------------------------
biosconst:      ld              l,concharinst
                jr              charentry
; --------------------------------------------------------------------------
biosconin:      ld              l,concharin
                jr              charentry
; --------------------------------------------------------------------------
biosconout:     ld              l,concharout
                jr              charentry
; --------------------------------------------------------------------------
bioslistst:     ld              l,listcharoutst
                jr              charentry
; --------------------------------------------------------------------------
bioslist:       ld              l,listcharout
                jr              charentry
; --------------------------------------------------------------------------
biospunch:      ld              l,punchcharout
                jr              charentry
; --------------------------------------------------------------------------
biosreader:     ld              l,readercharin
                jr              charentry
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
; middle layer entry points:
;  charentry - character io services - passes iobyte
;  diskentry - disk io services
;  sentry - long jump to middle layer services which are in adl mode
; --------------------------------------------------------------------------
charentry:      ld              a,(iobyte)      ; get iobyte for char functions
                ld              h,a
diskentry:      ld              (stack),sp      ; save app stack
                ld              sp,locstk
                call            sentry          ; cross over
                ld              sp,(stack)      ; restore app stack
                ret
; --------------------------------------------------------------------------
; The 0's will be replaced by the long address of the middle layer services
; Yes, this is self modifying code.
; Get over it.
; Its the most effecient method.
; --------------------------------------------------------------------------
sentry:         call.lil        0 ; middle layer service entry
                ret
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
; disk io services
; --------------------------------------------------------------------------
biosseldsk:     ld              l,selectdisk
                jr              diskentry
; --------------------------------------------------------------------------
bioshome:       ld              l,homeHead
                jr              diskentry
; --------------------------------------------------------------------------
biossettrk:     ld              l,settrack
                jr              diskentry
; --------------------------------------------------------------------------
biossetsec:     ld              l,setsector
                jr              diskentry
; --------------------------------------------------------------------------
biossetdma:     ld              l,setaddress
                jr              diskentry
; --------------------------------------------------------------------------
biosread:       ld              l,readsector
                jr              diskentry
; --------------------------------------------------------------------------
bioswrite:      ld              l,writesector
                jr              diskentry
; --------------------------------------------------------------------------
biossectrn:     ld              l,translate
                jr              diskentry
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
; local variable storage
; --------------------------------------------------------------------------
stack:          dw              0
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
; disk tables will occupy the sigonon message space after boot
; --------------------------------------------------------------------------
ramStart        equ             $
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
; cold boot service - runs once and replaces its self with the warm boot
; This code is overwritten by the bios disk tables.
; --------------------------------------------------------------------------
biosboot:       ld              sp,locstk       ; stack during init

                ld              hl,(0)          ; move middle layer bios address to build
                ld              (sentry+2),hl   ; a jump to the middle layer
                ld              a,(2)
                ld              (sentry+2+2),a

                ld              a,(iobyte)      ; sign on
                ld              bc,signon
                ld              l,constringout
                call            sentry

                ld              a,bootiob       ; set default iobyte
                ld              (iobyte),a

                ld              a,bootudv       ; set default user and disk
                ld              (usrdrv),a      ; select disk zero

                ld              hl,bioswboot    ; replace cold boot vector
                ld              (boot+1),hl     ; with warm boot vector

                jp              biosboot1       ; jump out of way of disk tables
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
; boot message - displayed once
; This signon is overwritten by bios disk tables.
; --------------------------------------------------------------------------
signon:         db              escape,"[2J"
                db              escape,"[H"
                ASCII           "MakerLisp "
                db              cpmsize / 10 + '0'
                db              cpmsize mod 10 + '0'
                ASCII           "k CP/M version 2.2 "
                ASCII           "\r\nCopyright (c) 1979 by Digital Research."
                ASCIZ           "\r\n"
; --------------------------------------------------------------------------



