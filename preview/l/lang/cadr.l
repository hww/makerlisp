(define cadr
  (macro (x)
    ;(forget 'cadr)
   `(car (cdr ,x))))
