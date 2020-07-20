; case statement implementation
(define case
  (macro (casexpr . cases)

    (forget 'case)

    (define kbreak (gensym))
    (define keyval (gensym))

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
        (if (or (eq (car acase) 'otherwise) (eq (car acase) 't))
         `(,kbreak (progn ,@(cdr acase)))
         `(if (',_eql ,keyval ',(car acase))
            (,kbreak (progn ,@(cdr acase)))))))

    (define tests
      (lambda (cases)
        (if (not cases)
          '()
          (if (pairp (caar cases))
            (append (tests (vmult (car cases))) (tests (cdr cases)))
            (cons (case-test (car cases)) (tests (cdr cases)))))))
    
   `(call/cc
      (lambda (,kbreak)
        (define ,keyval ,casexpr)
        ,@(tests cases)))))
