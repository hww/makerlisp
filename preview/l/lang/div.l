; Quotient of n arguments
(define div
  (macro args
    ;(forget 'div)
   `(n-ary div _div 1 ,@args)))
