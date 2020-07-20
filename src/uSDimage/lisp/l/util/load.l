; Manual load

; Read and evaluate all expressions in current or specified environment
(define load
  (macro args

    (define _load1
      (lambda (x keval)

        (define _load2
          (lambda (k)

            (define *error-continue* k)
            (define reval
              (lambda ()
                (if (_equal l (inclevel))
                  '()
                  (progn
                    (eval (read) keval)
                    (reval)))))
        
            (define l (inclevel))
            (include x)
            (reval)
            '()))

        (if (define e (call/cc _load2))
          (error e)
          '"")))

    (if (cdr args)
     `(',_load1 ,(car args) ,(car (cdr args)))
     `(call/ce (lambda (k) (',_load1 ,(car args) k))))))
