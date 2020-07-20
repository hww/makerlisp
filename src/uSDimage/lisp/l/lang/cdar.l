(define cdar
  (macro (x)
    (forget 'cdar)
   `(cdr (car ,x))))
