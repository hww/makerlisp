; Un-trace object, restore original assignment
(define untrace
  (macro (fnam)

    (forget 'untrace)

    (define oldval
      (lambda (otraced fnam)
        (if (not otraced)
          (error '"? untrace: object not traced")
        (if (eq (car (car otraced)) fnam)
          (cdr (car otraced))
          (oldval (cdr otraced) fnam)))))

    (define newtrace
      (lambda (otraced fnam)
        (if (not otraced)
          '()
          (if (eq (car (car otraced)) fnam)
            (cdr otraced)
            (cons (car otraced) (newtrace (cdr otraced) fnam))))))

    (if (not (assignedp '*traced-objects*))
      (error '"? untrace: no objects traced"))

   `(progn
      (setq ,fnam (',oldval *traced-objects* ',fnam))
      (setq *traced-objects* (',newtrace *traced-objects* ',fnam))
      ,fnam)))
