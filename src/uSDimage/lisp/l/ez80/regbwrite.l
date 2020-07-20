; Write a bit in a register
(define regbwrite
  (macro (r b d)
    (forget 'regbwrite)
   `(writereg ,r ("|" (& (readreg ,r) (~ (<< 1 ,b))) (<< (& ,d 1) ,b)))))
