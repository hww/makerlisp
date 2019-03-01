(define cdaddr
  (macro (x)
    (forget 'cdaddr)
   `(cdr (caddr ,x))))
