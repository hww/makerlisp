(define cadadr
  (macro (x)
    (forget 'cadadr)
   `(car (cdadr ,x))))
