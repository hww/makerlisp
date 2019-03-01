; Monotonic ascending order of n arguments
(define lessorequal
  (macro args
    ;(forget 'lessorequal)
   `(order-nary lessorequal _lessorequal ,@args)))
