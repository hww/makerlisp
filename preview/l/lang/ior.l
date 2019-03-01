; Integer or of n arguments
(define ior
  (macro args
    ;(forget 'ior)
   `(n-ary ior _ior 0 ,@args)))
