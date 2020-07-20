(define cadddr
  (macro (x)
    (forget 'cadddr)
   `(car (cdddr ,x))))
