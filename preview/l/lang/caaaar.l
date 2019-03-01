(define caaaar 
  (macro (x)
    (forget 'caaaar)
   `(car (caaar ,x))))
