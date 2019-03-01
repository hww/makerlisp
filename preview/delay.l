; Delay
(define delay
  (lambda (n)
    (if (not (zerop n)) (delay (- n 1)))))
