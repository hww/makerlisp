; Second-level read-eval-print loop
(define repl
  (lambda announce

    ; Loop until end of input
    (define _repl
      (lambda (krepl)

        (define e)
        (define r)
        (define v)

        (define getexpr
          (lambda (k)
            (define df (defer 't))
            (global "*error-continue*" k)
            (defer df)
            (setq r (read '">>> "))
            '()))

        (define evaluate
          (lambda (k)
            (define df (defer 't))
            (global "*error-continue*" k)
            (defer df)
            (eval r krepl)))

        (eval `(define exit (lambda (n) (',krepl n))) krepl)

        (while 't

          (if (setq e (call/cc getexpr))
            (setq r (list 'quote e))
            (progn
              (if (eq r '"") (progn (nlprint '"") (continue)))
              (if (eq r '"^D") (break))))

          (setq v (call/cc evaluate))
          (if (eq v '"") (continue))

          (nlprint v))))

    ; Start the repl with a continuation holding its environment
    (if announce (nlprint (car announce)))
    (define old-error-continue (if (assignedp '"*error-continue*") "*error-continue*"))
    (call/ce _repl)
    (define df (defer 't))
    (global "*error-continue*" old-error-continue)
    (defer df)
    '""))
