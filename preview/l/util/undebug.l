; Remove a breakpoint on a function
(define undebug
  (macro (f)
    (forget 'undebug)

    ; Macro ?
    (define fobj (eval f))
    (if (eq (car fobj) (car (macro ())))
      (error (cats '"?  No breakpoint on macro '" f '"'")))

    ; Not being debugged ?
    (if (not (eq (car debugger) (caaddr fobj)))
      `,f

      ; Extract body from debugger wrapping
      (progn
        (define oldb (cddr (caddr fobj)))
        (rplacd (cdr fobj) oldb)
       `,f))))
