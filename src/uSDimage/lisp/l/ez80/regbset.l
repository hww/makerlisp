; Set a bit in a register
(define regbset
  (macro (r b)
    (forget 'regbset)
   `(writereg ,r ("|" (readreg ,r) (<< 1 ,b)))))
