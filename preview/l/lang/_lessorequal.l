; Two argument less than or equal
(define _lessorequal
  (macro (x y)
    ;(forget '_lessorequal)
   `(not (_greater ,x ,y))))
