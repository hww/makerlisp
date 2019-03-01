; Sum of n arguments
(define add
  (macro args
    ;(forget 'add)
   `(n-ary add _add 0 ,@args)))
