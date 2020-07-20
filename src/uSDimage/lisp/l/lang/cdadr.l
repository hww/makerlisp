(define cdadr
  (macro (x)
    (forget 'cdadr)
   `(cdr (cadr ,x))))
