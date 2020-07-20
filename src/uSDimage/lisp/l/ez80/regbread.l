; Read a bit in a register
(define regbread
  (macro (r b)
    (forget 'regbread)
   `(& (>> (readreg ,r) ,b) 1)))
