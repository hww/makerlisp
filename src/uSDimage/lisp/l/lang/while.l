; 'while' implementation
(define while
  (macro (condition . body)

    (forget 'while)

    (define inner (gensym))
    (define kbreak (gensym))
    (define kcontinue (gensym))

   `(call/cc
      (lambda (,kbreak)
          
        (define break (macro () `(,kbreak '())))
        (define continue (macro () `(,kcontinue ,kcontinue)))

        (define ,inner
          (lambda (,kcontinue)
            (if ,condition (progn ,@body (,inner ,kcontinue)))))
           
        (,inner (call/cc (lambda (k) k)))))))
