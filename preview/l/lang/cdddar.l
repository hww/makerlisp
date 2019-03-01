(define cdddar
  (macro (x)
    (forget 'cdddar)
   `(cdr (cddar ,x))))
