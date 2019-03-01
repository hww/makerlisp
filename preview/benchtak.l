(define tak 
  (lambda (x y z)
    (if (< y x)
      (tak (tak (- x 1) y z)
           (tak (- y 1) z x)
           (tak (- z 1) x y))
      z)))

(define looptak
  (lambda (n)
    (if (zerop n)
      n
      (progn
        (tak 18 12 6)
        (looptak (- n 1))))))

(define benchtak
  (lambda ()
    (nlprint (time))
    (looptak 10)
    (nlprint (time))))
