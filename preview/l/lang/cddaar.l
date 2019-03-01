(define cddaar
  (macro (x)
    (forget 'cddaar)
   `(cdr (cdaar ,x))))
