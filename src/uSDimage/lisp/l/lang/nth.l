; Reference nth item in list
(define nth
  (macro (n l)

    (forget 'nth)

    (define _nth
      (lambda (n l)

        (if (or (not (integerp n)) (< n 0))
          (error '"? First argument to \"nth\" must be a positive integer"))
        (if (not (listp l))
          (error '"? Second argument to \"nth\" must be a list"))

        (if (zerop n)
          (car l)
          (_nth (- n 1) (cdr l)))))

   `(',_nth ,n ,l)))
