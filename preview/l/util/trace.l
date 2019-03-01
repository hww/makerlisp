; Create tracing form of object, bind name to it
(define trace
  (macro (fnam . rest)

    (forget 'trace)

    ; Execute function to load it
    (define loadfobj
      (lambda (fobj)

        ; Executor which catches errors
        (define exefun
          (lambda (k)
            (global *error-continue* k)
            (eval (eval `(',fobj)))))

        ; Call executor
        (define old-error-handler
          (if (assignedp '*error-continue*) *error-continue*))
        (call/cc exefun)
        (global *error-continue* old-error-handler)))

    ; Traceable ?
    (if (eq fnam '_append) (error '"? Can't trace \"_append\""))
    (if (eq fnam '_backquote) (error '"? Can't trace \"_backquote\""))
    (if (eq fnam 'backquote) (error '"? Can't trace \"backquote\""))
    (define fobj (eval fnam))
    (if (not (pairp fobj)) (error '"? Can't trace an atom"))

    ; Load and execute function once
    (define oldforget (if (assignedp '"*don't-forget*") "*don't-forget*"))
    (global "*don't-forget*" 't)
    (eval `(loadfobj ,fnam))
    (global "*don't-forget*" oldforget)

    ; Get old applicable object, assign to new symbol
    (setq fobj (eval fnam))
    (define oldf (gensym))
    (eval `(global ,oldf fobj))

    ; Function to call on trace
    (define tfun (car rest))
    (if (not tfun) (setq tfun 'nlprint))
    (if (eq fnam tfun) (error (cats '"? Can't trace \"" tfun '"\"")))

    ; Object name will be re-assigned, save old assignment to restore, later.
    (if (not (assignedp '*traced-objects*))
      (global *traced-objects* '()))

    ; Replace with macro or function that traces and applies old object
    (if (eq (car fobj) (car (macro ())))

      (progn
        (setq *traced-objects* (cons (cons fnam fobj) *traced-objects*))

       `(define ,fnam
          (macro args
            (,tfun (cons ',fnam args))
            (cons ',oldf args))))

     (progn

       ; Quote the list of evaluated arguments
       (define quoteargs
         (lambda (args)
           (if (not args)
             '()
             (cons (list 'quote (car args)) (quoteargs (cdr args))))))

       `(progn

          (define ,fnam
            (lambda args
              (,tfun (cons ',fnam args))
              (eval (cons ',oldf (',quoteargs args)))))

          ; This is done last in case assignment above errors
          (setq *traced-objects* (cons (cons ',fnam ',fobj) *traced-objects*))

          ,fnam)))))
