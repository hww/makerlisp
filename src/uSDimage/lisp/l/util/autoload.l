; Load on demand support
(if (boundp '_AUTOLOAD_L_) (eof) (define _AUTOLOAD_L_))

(include '<backquote.l>)

; call/cc, with a guaranteed new continuation to hold current environment 
(define call/ce
  (macro (f)
   `(let ((v (call/cc ,f))) v)))

; Autoload evaluates in the top environment
(define autoload
  (lambda (x)

    (define _load
      (lambda (k)

        (define *error-continue* k)
        (define reval
          (lambda ()
            (if (_equal l (inclevel))
              '()
              (progn
                (eval (read) *topk*)
                (reval)))))
        
        (define l (inclevel))
        (include x)
        (reval)
        '()))

    (if (define e (call/cc _load)) (error e))))

; Load a lambda or macro definition and apply it
(define loadapply
  (macro (file func . args)
    (autoload file)
   `(,func ,@args)))

; Capture top continuation
(call/cc
  (lambda (k)
    (eval `(define *topk* ',k) k)))
