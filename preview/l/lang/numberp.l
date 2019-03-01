; Number predicate
(define numberp
  (macro (x)
    (forget 'numberp)
   `(or (integerp ,x) (realp ,x))))
