; Reference nth item in list
(define nth
  (macro (l n)

    (forget 'nth)

    (define _nth
      (lambda (l n)

        (if (not (listp l))
          (error '"? First argument to \"nth\" must be a list"))
        (if (or (not (integerp n)) (< n 0))
          (error '"? Second argument to \"nth\" must be a positive integer"))

        (if (zerop n)
          (car l)
          (_nth (cdr l) (- n 1)))))

   `(',_nth ,l ,n)))
