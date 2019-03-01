(define fact (lambda (n) (if (zerop n) 1 (* n (fact (- n 1))))))
