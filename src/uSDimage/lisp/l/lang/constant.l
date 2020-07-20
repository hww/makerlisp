; Create a constant expression
(define constant
  (macro (x)
    (forget 'constant)
    (eval `,x)))
