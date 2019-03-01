; List length
(define length
  (macro (l)

    (forget 'length)

    (define _length
      (lambda (l)

        (if (not l)
          0
          (progn
            (if (atom l)
              (error '"? Argument to \"length\" must be a list"))
            (+ 1 (_length (cdr l)))))))

   `(',_length ,l)))
