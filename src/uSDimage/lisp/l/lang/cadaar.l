(define cadaar
  (macro (x)
    (forget 'cadaar)
   `(car (cdaar ,x))))
