; Two argument logical and
(define _land
  (macro (x y)
    (forget '_land)
   `(if ,x ,y)))
