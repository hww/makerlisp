; Two argument greater than or equal
(define _greaterorequal
  (macro (x y)
    ;(forget '_greaterorequal)
   `(not (_less ,x ,y))))
