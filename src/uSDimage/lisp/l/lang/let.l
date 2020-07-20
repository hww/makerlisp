; 'let' using lambda, bind all at once
(define let
  (macro (bind . body)

    (forget 'let)

    (define vars
      (lambda (b)
        (if (not b)
          '()
          (cons (caar b) (vars (cdr b))))))

    (define vals
      (lambda (b)
        (if (not b)
          '()
          (cons (cadar b) (vals (cdr b))))))

    `((lambda ,(vars bind) ,@body) ,@(vals bind))))
