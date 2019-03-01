; Logical, short-circuiting and of n arguments
(define and
  (macro args
    (forget 'and)
    `(n-ary and _land 't ,@args)))
