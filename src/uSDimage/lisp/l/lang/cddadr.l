(define cddadr
  (macro (x)
    (forget 'cddadr)
   `(cdr (cdadr ,x))))
