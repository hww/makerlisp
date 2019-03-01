; Create symbol containing all 256 CP 437 characters
(define chartest
  (lambda ()
    (let ((i 0) (s '""))
      (while (< i 256)
        (if (< i 32) (setq s (cats s '\e)))
        (setq s (cats s (int2sym i))) 
        (setq i (+ i 1)))
      s)))
