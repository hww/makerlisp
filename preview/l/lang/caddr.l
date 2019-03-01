(define caddr
  (macro (x)
    ;(forget 'caddr)
   `(car (cddr ,x))))
