(define caaadr
  (macro (x)
    (forget 'caaadr)
   `(car (caadr ,x))))
