; Product of n arguments
(define mul
  (macro args
    ;(forget 'mul)
   `(n-ary mul _mul 1 ,@args)))
