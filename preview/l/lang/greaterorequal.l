; Monotonic descending order of n arguments
(define greaterorequal
  (macro args
    ;(forget 'greaterorequal)
   `(order-nary greaterorequal _greaterorequal ,@args)))
