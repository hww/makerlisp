; Apply a macro or lambda to a list of arguments
(define apply
  (macro (f args)
    (forget 'apply)

    (define _apply
      (lambda (f args)
        (eval (cons 'f args))))

   `(',_apply ,f ,args)))
