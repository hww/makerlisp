(define caaddr
  (macro (x)
    (forget 'caaddr)
   `(car (caddr ,x))))
