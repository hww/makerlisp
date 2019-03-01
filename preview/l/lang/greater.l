; Descending order of n arguments
(define greater
  (macro args
    ;(forget 'greater)
   `(order-nary greater _greater ,@args)))
