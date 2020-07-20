; Apply f to each element in l, return list of results
(define mapcar
  (lambda (f l)
    (if (not l)
      '()
      (cons (f (car l)) (mapcar f (cdr l))))))
