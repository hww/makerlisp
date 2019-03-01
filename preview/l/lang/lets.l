; 'lets' using nested lambdas, sequential binding
(define lets
  (macro (bind . body)

    (forget 'lets)

    (define bind1 
      (lambda (bind body)
        (if (not bind)
          `(progn ,@body)
          `((lambda ,(list (caar bind))
            ,(bind1 (cdr bind) body)) ,(cadar bind)))))

    (bind1 bind body)))
