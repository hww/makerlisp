; cond statement implementation
(define cond
  (macro clauses

    (forget 'cond)

    (define kbreak (gensym))
    (define tstval (gensym))

    (define clause-test
      (lambda (clause)
        (if (not (cdr clause))
         `(if (define ,tstval ,(car clause)) (,kbreak ,tstval))
         `(if ,(car clause) (,kbreak (progn ,@(cdr clause)))))))

    (define tests
      (lambda (clauses)
        (if (not clauses)
          '()
          (cons (clause-test (car clauses)) (tests (cdr clauses))))))

   `(call/cc
      (lambda (,kbreak)
        ,@(tests clauses)))))
