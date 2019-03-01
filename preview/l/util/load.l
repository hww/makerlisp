; Load on demand support
(if (boundp '_LOAD_L_) (eof) (define _LOAD_L_))

(include '<backquote.l>)

; Read and evaluate all the expressions in a lisp file
(define load
  (lambda (x)

    (define reval
      (lambda ()
        (if (_equal l (inclevel))
          '""
          (progn
            (define r (read))
            (if (eq r '"\^C")
              (error '"\^C"))
            (setq r (eval r *topk*))
            (if (symbolp r)
              (if (eq (car r) '"?")
                (error r)))
              (reval)))))

    (define l (inclevel))
    (include x)
    (reval)))

; Load a lambda or macro definition and apply it
(define loadapply
  (macro (file func . args)
    (load file)
   `(,func ,@args)))

; Capture top continuation
(call/cc
  (lambda (k)
    (eval `(define *topk* ',k) k)))
