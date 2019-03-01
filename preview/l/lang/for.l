; 'for' implementation
(define for
  (macro (var list . body)

    (forget 'for)

    (define inner (gensym))
    (define kbreak (gensym))
    (define kcontinue (gensym))
    (define nlist (gensym))

   `(call/cc
      (lambda (,kbreak)
          
        (define break (macro () `(,kbreak '())))
        (define continue (macro () `(,kcontinue ,kcontinue)))

        (define ,inner
          (lambda (,kcontinue)
            (if ,nlist
              (progn
                (setq ,var (car ,nlist))
                ,@body
                (setq ,nlist (cdr ,nlist))
                (,inner ,kcontinue)))))

        (define ,var)
        (define ,nlist ,list)
        (,inner (call/cc (lambda (k) k)))))))
