; Clear a bit in a register
(define regbclr
  (macro (r b)
    (forget 'regbclr)
   `(writereg ,r (& (readreg ,r) (~ (<< 1 ,b))))))
