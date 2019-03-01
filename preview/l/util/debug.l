; Set a breakpoint on a function
(define debug
  (macro (f)
    (forget 'debug)

    ; Macro ?
    (define fobj (eval f))
    (if (eq (car fobj) (car (macro ())))
      (error (cats '"? Can't set breakpoint on macro '" f '"'")))

    ; Already being debugged ?
    (if (eq (car debugger) (caaddr fobj))
      `,f

      ; Debugger primitive wraps body
      (progn
        (define newf
          (list (car fobj) (cadr fobj)
            (append (list (car debugger) f) (cddr fobj))))
        `(define ,f ',newf)))))
