; Search a-list for match (eq)
(define assq
  (macro (i l)
    (forget 'assq)

    (define _assq
      (lambda (i l)
        (if (not l)
          l
          (if (eq i (caar l))
            (car l)
            (_assq i (cdr l))))))

    `(',_assq ,i ,l)))
