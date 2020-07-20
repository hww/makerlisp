; Get/set a list of symbols from/to, the *PATH* variable
(define path
  (lambda args

    (if (not args)

      ; No arguments, get the path as a list of symbols
      (progn

        (define tpath *PATH*)
        (define c)

        (if (not (symbolp tpath))
          (error '"? *PATH* must be a symbol"))

        ; Pull the first symbol off the path
        (define firstsym
          (lambda (first)
            (setq c (car tpath))
            (setq tpath (cdr tpath))
            (if (or (eq c '"") (eq c '":"))
              first
              (firstsym (cats first c)))))

        ; Build up the list of symbols
        (define getpath
          (lambda ()
            (if (eq tpath '"")
              '()
              (let ((first (firstsym '"")))
                (cons first (getpath))))))

        (getpath))

      ; Or one argument, set the path from a list of symbols
      (progn

        (if (or (cdr args) (not (listp (car args))))
          (error '"? Argument to \"path\" must be a list"))

        ; Walk list concatenating symbols
        (define setpath
          (lambda (l tpath)
            (if (not l)
              tpath
              (let ((s (car l)))
                (if (not (symbolp s))
                  (error '"? Argument to \"path\" must be list of symbols"))
                (setq tpath (cats tpath s))
                (if (cdr l)
                  (setq tpath (cats tpath '":")))
                (setpath (cdr l) tpath)))))

        (global *PATH* (setpath (car args) '""))))))
