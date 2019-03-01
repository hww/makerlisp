(define cdaar
  (macro (x)
    (forget 'cdaar)
   `(cdr (caar ,x))))
