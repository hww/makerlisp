; Integer and of n arguments
(define iand
  (macro args
    ;(forget 'iand)
   `(n-ary iand _iand -1 ,@args)))
