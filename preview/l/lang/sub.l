; Difference of n arguments
(define sub
  (macro args
    ;(forget 'sub)
   `(n-ary sub _sub 0 ,@args)))
