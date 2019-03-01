; List predicate
(define listp
  (macro (x)
    (forget 'listp)
   `(or (null ,x) (pairp ,x))))
