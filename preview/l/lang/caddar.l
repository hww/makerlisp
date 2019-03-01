(define caddar
  (macro (x)
    (forget 'caddar)
   `(car (cddar ,x))))
