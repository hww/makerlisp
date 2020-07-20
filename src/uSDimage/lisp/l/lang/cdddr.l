(define cdddr
  (macro (x)
    (forget 'cdddr)
   `(cdr (cddr ,x))))
