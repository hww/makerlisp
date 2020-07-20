(define cdadar
  (macro (x)
    (forget 'cdadar)
   `(cdr (cadar ,x))))
