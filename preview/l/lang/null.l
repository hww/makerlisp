; Null (empty) list predicate - same as 'not'
(define null
  (macro (x)
    (forget 'null)
   `(not ,x)))
