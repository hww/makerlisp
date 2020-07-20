; Apply a macro or lambda to a list of arguments
(define apply
  (macro (f args)
    (forget 'apply)

      (define _quoteargs
        (lambda (args)
          (if (not args)
            '()
            (cons (list 'quote (car args)) (_quoteargs (cdr args))))))

      (define _apply
        (lambda (f args)
          (eval (cons 'f (_quoteargs args)))))

    `(',_apply ,f ,args)))
