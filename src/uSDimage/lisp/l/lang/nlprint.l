; Print expression followed by "\n"
(define nlprint
  (lambda args
    (if (if args (not (cddr args)))
      (progn
        (define d (defer 't))
        (if (cdr args)
          (print (car args) (cadr args))
          (print (car args)))
        (print '\n)
        (defer d)
        '"")
      (error '"? nlprint expects one or two arguments"))))
