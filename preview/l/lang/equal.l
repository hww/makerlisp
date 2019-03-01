; Equality of n arguments
(define equal
  (macro args
    ;(forget 'equal)
   `(order-nary equal _equal ,@args)))
