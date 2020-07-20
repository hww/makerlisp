; Generate a bit mask, constant expression if possible
(define regbmask
  (macro (b)
    (if (integerp b)
     `(constant (<< 1 ,b))
     `(<< 1 ,b))))
