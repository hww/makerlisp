; Two argument logical or
(define _lor
  (macro (x y)
    (forget '_lor)
    (define temp (gensym))
     `(let ((,temp ,x))
        (if ,temp ,temp ,y))))
