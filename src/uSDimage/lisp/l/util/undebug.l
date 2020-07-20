; Remove a breakpoint on a function
(define undebug
  (macro (f)
    (forget 'undebug)

    (define _undebug
      (lambda (k)

        ; Macro ?
	(define fobj (eval f k))
	(if (eq (car fobj) (car (macro ())))
	  (error (cats '"?  No breakpoint on macro '" f '"'")))

        ; Not being debugged ?
	(if (not (eq (car debugger) (caaddr fobj)))
	  fobj

          ; Extract body from debugger wrapping
	  (progn
	    (define oldb (cddr (caddr fobj)))
	    (rplacd (cdr fobj) oldb)
	    fobj))))

   `(call/cc ',_undebug)))
