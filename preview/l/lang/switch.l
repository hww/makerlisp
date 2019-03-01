; switch statement implementation
(define switch
  (macro (swexpr . cases)

    (forget 'switch)

    (define kbreak (gensym))
    (define keyval (gensym))
    (define match (gensym))

    (define _eql
      (lambda (x y)
        (if (and (numberp x) (numberp y))
          (= x y)
          (eq x y))))

    (define vmult
      (lambda (acase)
        (if (not (caar acase))
          '()
          (cons (cons (caar acase) (cdr acase))
            (vmult (cons (cdar acase) (cdr acase)))))))

    (define case-test
      (lambda (acase)
        (if (eq (car acase) 'default)
         `(progn ,@(cdr acase))
         `(if (or ,match (',_eql ,keyval ',(car acase)))
            (progn (setq ,match 't) ,@(cdr acase))))))

    (define tests
      (lambda (cases)
        (if (not cases)
          '()
          (if (pairp (caar cases))
            (append (tests (vmult (car cases))) (tests (cdr cases)))
            (cons (case-test (car cases)) (tests (cdr cases)))))))
    
   `(call/cc
      (lambda (,kbreak)
        (define ,match '())
        (define ,keyval ,swexpr)
        (define break (macro () `(,kbreak '())))
        ,@(tests cases)))))
