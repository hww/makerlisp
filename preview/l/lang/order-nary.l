; Extend a 2 argument numeric ordering to an arbitrary number of arguments
(define order-nary

  ; args: macro name, primitive function, argument list
  (macro (mnam pfun . args)

    ;(forget 'order-nary)

    (define first (car args))
    (define second (cadr args))
    (define more (cddr args))

    (if (not first)
     `'t
      (if (not second)

       `(progn
          (if (not (numberp ,first))
            (error
              (cats '"? Arguments to '" ',mnam '"' must be numbers"))
          't))
        
        (if (not more)
         `(,pfun ,first ,second)
         `(if (,pfun ,first ,second)
            (,mnam ,second ,@more)))))))
