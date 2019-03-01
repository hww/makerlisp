; Create a constant expression
(define constant
  (forget 'constant)
  (macro (x) (eval `,x)))
