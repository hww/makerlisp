; --------------------------------------------------------------------------
; Copyright (c) 2019, Christopher D. Farrar
; --------------------------------------------------------------------------
; I here grant permission to any and all to copy and use this software for
; any purpose as long as my copyright message is retained.
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
; Boot and warm boot functions
; --------------------------------------------------------------------------
                TITLE   "CPM mid level interface"
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
; some basic init stuff
; --------------------------------------------------------------------------
                CPU = EZ80F91

                segment         CODE
                .assume         adl=1
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
; Entry points to this level for the c monitor
; --------------------------------------------------------------------------
                xdef            _adl_startCpm
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
; Entry points to this level for the cpm bios
; --------------------------------------------------------------------------
                xdef            _adl_biosEntry
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
; c bios external refs - memory reload
; --------------------------------------------------------------------------
                xref            _adl_biosDispatch
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
; start cpm
; --------------------------------------------------------------------------
                SEGMENT         CODE

;
; On entry to the _adl_startCpm the following will be true:
;
; 0..3 Will contain adl callback address of bios dispatch entry
;
; 100h... Will contain boot code that will prepare the
;         cpm system and enter the bios cold boot
;
; The ccp, bdos, and bios will already be in the appropreate locations.

_adl_startCpm:  ld              a,(_mbregister)         ; set the mbase register for cpm
                ld              mb,a
                stmix                                   ; need mixed mode to switch modes
                ei
                call.lis        100h                    ; location of boot code
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
; main entry point for cpm bios.
; --------------------------------------------------------------------------
_adl_biosEntry:
                ld              (_hlregister),hl        ; save registers passed in by cpm
                ld              (_deregister),de
                ld              (_bcregister),bc
                push            af
                pop             hl
                ld              (_afregister),hl
                call            _adl_biosDispatch       ; go execute the specified service
                ld              hl,(_afregister)        ; get return values into register before cpm return
                push            hl
                pop             af
                ld              bc,(_bcregister)
                ld              de,(_deregister)
                ld              hl,(_hlregister)
                ret.l                                   ; return to cpm and return to z80 mode
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
; the following register must be accessable to the c code
; --------------------------------------------------------------------------
                xdef            _mbregister
                xdef            _afregister
                xdef            _bcregister
                xdef            _deregister
                xdef            _hlregister
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
; storage for i register before crossing over to z80 only mode
; --------------------------------------------------------------------------
                SEGMENT         DATA
_mbregister:    ds              1
_afregister:    ds              3
_bcregister:    ds              3
_deregister:    ds              3
_hlregister:    ds              3
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
                end
; --------------------------------------------------------------------------
