(define cddr
  (macro (x)
    ;(forget 'cddr)
   `(cdr (cdr ,x))))
