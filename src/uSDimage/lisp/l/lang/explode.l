; Explode a symbol into a list of one character symbols
(define explode
  (lambda (s)

    (if (not (symbolp s))
      (error '"? Argument to \"explode\" must be a symbol"))

    (if (eq s '"")
      '()
      (cons (car s) (explode (cdr s))))))
