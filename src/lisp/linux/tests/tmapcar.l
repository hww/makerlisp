; Test mapcar and other appropriately-aggressive JIT situations
(define foo (lambda (f x) (f x x)))
(foo + 1)
(foo - 1)
(foo cats 'a)
foo

(mapcar car '((a 1) (b 2) (c 3)))
(mapcar cdr '((a 1) (b 2) (c 3)))
(mapcar cadr '((a 1) (b 2) (c 3)))
mapcar

(define reduce
  (lambda (function initial-value lst)
    (if (null lst)
      initial-value
      (reduce function
        (apply function (list initial-value (car lst)))
        (cdr lst)))))

(reduce + 0 '(1 2 3 4 5))
(reduce + 999 '(1 2 3 4))

reduce
