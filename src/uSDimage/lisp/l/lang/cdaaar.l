(define cdaaar
  (macro (x)
    (forget 'cdaaar)
   `(cdr (caaar ,x))))
