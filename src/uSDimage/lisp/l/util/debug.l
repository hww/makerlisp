; Set a breakpoint on a function
(define debug
  (macro (f)
    (forget 'debug)

    (define _debug
      (lambda (k)

        ; Loader which catches errors
	(define loadfun
	  (lambda (kload)
	    (define df (defer 't))
	    (global "*error-continue*" kload)
	    (defer df)
            (define fobj (eval f k))
	    (if (eq (car fobj) (car (macro ())))
	      (load (cats f '".l") k))))

        ; Attempt to load function if necessary
	(define old-error-handler
	  (if (assignedp '"*error-continue*") "*error-continue*"))
	(call/cc loadfun)
	(define df (defer 't))
	(global "*error-continue*" old-error-handler)
	(defer df)

        ; Macro ?
        (define fobj (eval f k))
        (if (eq (car fobj) (car (macro ())))
          (error (cats '"? Can't set breakpoint on macro '" f '"'")))

        ; Already being debugged ?
        (if (eq (car debugger) (caaddr fobj))
          fobj

          ; Debugger primitive wraps body
          (progn
            (define newf
              (list (car fobj) (cadr fobj)
                (append (list (car debugger) f) (cddr fobj))))
            (eval `(define ,f ',newf) k)))))

    `(call/cc ',_debug)))
