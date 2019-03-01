; Integer xor of n arguments
(define ixor
  (macro args
    ;(forget 'ixor)
   `(n-ary ixor _ixor 0 ,@args)))
