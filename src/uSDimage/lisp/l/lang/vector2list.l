; Vector to list
(define vector2list
  (macro (v)

    (forget 'vector2list)

    (define _v2l
      (lambda (i v)

        (if (not (vectorp v))
          (error '"? Argument to \"vector2list\" must be a vector"))

        (define n (vlen v))
        (if (not (< i n))
          '()
          (cons (vref v i) (_v2l (+ i 1) v)))))

   `(',_v2l 0 ,v)))
