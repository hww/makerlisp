; Ascending order of n arguments
(define less
  (macro args
    ;(forget 'less)
   `(order-nary less _less ,@args)))
