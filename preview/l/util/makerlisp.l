; MakerLisp start-up file
(if (boundp '_MAKERLISP_L_) (eof) (define _MAKERLISP_L_))

; Arithmetic aliases are not good filenames
(include '<arithmetic.l>)

; All other 'built-in' code is auto-loaded 
(include '<load.l>)

; Announce
(nlprint (version))
