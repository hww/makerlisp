; Reverse a list
(define reverse
  (macro (l)

    (forget 'reverse)

    (define _reverser
      (lambda (fwd rev)

        (if (not (listp fwd))
          (error '"? Argument to \"reverse\" must be a list"))

        (if (not fwd)
          rev
          (_reverser (cdr fwd) (cons (car fwd) rev)))))

   `(',_reverser ,l '())))
