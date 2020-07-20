#!lisp.sh

; Test coroutines using continuations
(include '<makerlisp.l>)

(verbose 99)

(define resume (macro (t) `(setq ,t (call/cc ,t))))

(define task1
  (lambda (k)
    (let ((t2 task2))
      (print '"Part A\n")
      (print '"task1 starting task2\n")
      (resume t2)
      (print '"Part B\n")
      (print '"Part C\n")
      (print '"task1 resuming task2\n")
      (resume t2)
      (print '"task1 finishing\n")
      (k 0))))

(define task2
  (lambda (k)
    (let ((t1 k))
      (print '"Part D\n")
      (print '"Part E\n")
      (print '"task2 resuming task1\n")
      (resume t1)
      (print '"Part F\n")
      (print '"task2 finishing, resuming task1\n")
      (resume t1))))

(print '"Starting task1\n")
(call/cc task1)
