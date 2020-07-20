; Stack object maker
(define stack
  (lambda ()
    (define this-stack '())
    (define push
      (lambda (x)
        (setq this-stack (cons x this-stack))))
    (define pop
      (lambda ()
        (let ((v (car this-stack))) (setq this-stack (cdr this-stack)) v)))
    (define get
      (lambda ()
        this-stack))
    (define mlist (list (cons 'push push) (cons 'pop pop) (cons 'get get)))
    (define f
      (lambda (m)
        (if (define f (cdr (assq m mlist)))
          f 
          (error (cats '"? stack: Unknown operation: '" m '"'\n")))))
    f))
