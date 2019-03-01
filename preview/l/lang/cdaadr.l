(define cdaadr
  (macro (x)
    (forget 'cdaadr)
   `(cdr (caadr ,x))))
