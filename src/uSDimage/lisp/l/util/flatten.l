; Function which expands all macro applications in an expression
(define flatten
  (lambda (x)

    (define cexpand
      (lambda (k)

        (define listquote
          (lambda (l)
            (if (not l)
              '()
              (cons (list 'quote (car l)) (listquote (cdr l))))))

        (define listexpand
          (lambda (l)
            (if (not l)
              '()
              (cons (flatten (car l)) (listexpand (cdr l))))))

        (define xprest
          (lambda ()
            (cons (car x) (listexpand (cdr x)))))

        (if (not (pairp x)) (k x))
        (define y (car x))

        (if (eq y 'lambda) (k x))
        (if (eq y 'macro) (k x))

        (if (not (symbolp y)) (k (xprest)))
        (if (not (assignedp y)) (k (xprest)))
        (define z (eval y))

        (if (not (pairp z)) (k (xprest)))
        (if (not (eq (car z) (car (macro ())))) (k (xprest)))

        (flatten (eval `(',z ,@(listquote (listexpand (cdr x))))))))

    (call/cc cexpand)))
