; Is this symbol reachable at the current or specified level or higher, and assigned ?
(define isloaded
  (macro args

    (define _isloaded
      (lambda (s keval)

        (if (not (symbolp s))
          (error '"? \"isloaded\" expects symbol as first argument"))

        (define old-autoload
          (eval `(if (assignedp '"*don't-autoload*") "*don't-autoload*") keval))
        (eval `(define "*don't-autoload*" 't) keval)
        (define there (eval `(boundp ',s) keval))
        (eval `(define "*don't-autoload*" ',old-autoload) keval)
        there))

    (if (cdr args)
     `(',_isloaded ,(car args) ,(cadr args))
     `(call/ce (lambda (k) (',_isloaded ,(car args) k))))))
