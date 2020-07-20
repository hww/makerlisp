; Create symbol containing all 256 CP 437 characters
(define chartest
  (lambda args
    (let ((i (if (integerp (car args)) (car args) 0)) (s '"\e[11m"))
      (while (< i 256)
        (if (< i 32) (setq s (cats s '\e)))
        (if (= i 32) (setq s (cats s '"\e[10m")))
        (setq s (cats s (int2sym i))) 
        (setq i (+ i 1)))
      s)))
