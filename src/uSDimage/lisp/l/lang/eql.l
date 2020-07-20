; Equivalence
(define eql
  (macro (x y)

    (forget 'eql)

    (define _eql
      (lambda (x y)
        (if (and (numberp x) (numberp y))
          (= x y)
          (eq x y))))

   `(',_eql ,x ,y)))
