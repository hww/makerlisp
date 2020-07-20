; Loop on (sieve 750)
(define loopsieve
  (lambda ()
    (define i 1)
    (define s)
    (while 't
      (nlprint i)
      (setq s (sieve 750))
      (nlprint s)
      (setq i (+ i 1)))))
