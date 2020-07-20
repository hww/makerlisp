(define caar
  (macro (x)
    (forget 'caar)
   `(car (car ,x))))
