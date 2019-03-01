(define cddddr
  (macro (x)
    (forget 'cddddr)
   `(cdr (cdddr ,x))))
