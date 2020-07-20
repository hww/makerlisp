(define cadar
  (macro (x)
    (forget 'cadar)
   `(car (cdar ,x))))
