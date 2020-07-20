(define caadr
  (macro (x)
    (forget 'caadr)
   `(car (cadr ,x))))
