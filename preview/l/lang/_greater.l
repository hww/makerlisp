; Two argument greater than
(define _greater
  (macro (x y)
    ;(forget '_greater)
   `(_less ,y ,x)))
