(define caaar
  (macro (x)
    (forget 'caaar)
   `(car (caar ,x))))
