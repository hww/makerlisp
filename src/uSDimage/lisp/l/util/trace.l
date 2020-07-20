; Trace an applicable object
(include '<apply.l>)

(define trace
  (macro (f . rest)
    (forget 'trace)

    (define _trace
      (lambda (k)

        ; Loader which catches errors
	(define loadobj
	  (lambda (kload)
	    (define df (defer 't))
	    (global "*error-continue*" kload)
	    (defer df)
            (define fobj (eval f k))
	    (if (eq (car fobj) (car (macro ())))
	      (load (cats f '".l") k))))

        ; Traceable ?
        (if (eq f '_append) (error '"? Can't trace \"_append\""))
        (if (eq f '_backquote) (error '"? Can't trace \"_backquote\""))
        (if (eq f 'backquote) (error '"? Can't trace \"backquote\""))
        (define fobj (eval f k))
        (if (not (pairp fobj)) (error '"? Can't trace an atom"))

        ; Function to call on trace
        (define tfun (car rest))
        (if (not tfun) (setq tfun 'nlprint))
        (if (eq f tfun) (error (cats '"? Can't trace \"" tfun '"\"")))

        ; Attempt to load object if necessary
	(define old-error-handler
	  (if (assignedp '"*error-continue*") "*error-continue*"))
	(call/cc loadobj)
	(define df (defer 't))
	(global "*error-continue*" old-error-handler)
	(defer df)

        ; Put applicable object on list to restore
        (setq fobj (eval f k))
        (if (not (assignedp '"*traced-objects*"))
          (global "*traced-objects*" '()))
        (setq "*traced-objects*" (cons (cons f fobj) "*traced-objects*"))

        ; New object traces, then chains to original
        (if (eq (car fobj) (car (macro ())))

          ; Macro
	  (progn

	    ; Need a symbol for macro application of old object
            (define oldf (gensym))
            (eval `(global ,oldf fobj))

            (eval
             `(define ,f
                (macro args
                  (,tfun (cons ',f args))
                  (cons ',oldf args))) k))

          ; Function
          (eval
           `(define ,f
              (lambda args
                (,tfun (cons ',f args))
                 (apply ',fobj args))) k))))

    `(call/cc ',_trace)))
