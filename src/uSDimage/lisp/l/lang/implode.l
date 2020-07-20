; Implode a list of (one or more character) symbols into a single symbol
(define implode
  (lambda (l)

    (if (not (listp l))
      (error '"? Argument to \"implode\" must be a list"))

    (if (not l)
      '""
      (cats (car l) (implode (cdr l))))))
