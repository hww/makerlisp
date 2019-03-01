; Extend a 2 argument function to an arbitrary number of arguments
(define n-ary

  ; args: macro name, primitive function, base value, argument list
  (macro (mnam pfun bval . args)

    ;(forget 'n-ary)

    (define first (car args))
    (define second (cadr args))
    (define more (cddr args))

    (if (not first)
      bval
      (if (not second)
        `(,pfun ,bval ,first)
        (if (not more)
          `(,pfun ,first ,second)
          `(,mnam (,pfun ,first ,second) ,@more))))))
