(define caadar
  (macro (x)
    (forget 'caadar)
   `(car (cadar ,x))))
