; Search a-list for match (eql)
(define assoc
  (macro (i l)
    (forget 'assoc)

    (define _assoc
      (lambda (i l)
        (if (not l)
          l
          (if (eql i (caar l))
            (car l)
            (_assoc i (cdr l))))))

    `(',_assoc ,i ,l)))
