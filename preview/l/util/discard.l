; Discard value
(define discard
  (macro (x)
    (forget 'discard)
    `((lambda (x) '"") ,x)))
