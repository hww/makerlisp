; Second-level read-eval-print loop
(include '<makerlisp.l>)

(define repl
  (lambda announce

    ; Loop until end of input
    (define _repl
      (lambda (krepl)

        (define r)
        (define v)

        (define getline
          (lambda (k)
            (global *error-continue* k)
            (read '">>> ")))

        (define evaluate
          (lambda (k)
            (global *error-continue* k)
            (eval r krepl)))

        (while 't

          (setq r (call/cc getline))
          (if (eq r '"") (progn (nlprint '"") (continue)))
          (if (eq r '"^D") (break))
          (if (eq r '"\^C") (setq r (list 'quote r)))

          (setq v (call/cc evaluate))
          (if (eq v '"") (continue))

          (nlprint v))

        (global *error-continue* '())
        '""))

    (if announce (nlprint (car announce)))
    (call/cc _repl)
))
