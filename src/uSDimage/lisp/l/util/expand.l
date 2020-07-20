; Macro which expands all macro applications in an expression
(define expand
  (macro (x)
    (forget 'expand)
   `',(flatten x)))
