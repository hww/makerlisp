(define cddar
  (macro (x)
    (forget 'cddar)
   `(cdr (cdar ,x))))
