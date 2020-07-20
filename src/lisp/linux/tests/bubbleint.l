#!lisp.sh
(verbose 99)

; Bubble sort of integers
(define bubble
  (lambda (unsorted)

    ; Check for list of integers
    (define listint
      (lambda (x)
        (if (null x)
          't
          (and (pairp x) (integerp (car x)) (listint (cdr x))))))

    (define bubble1
      (lambda (k)
        
        (if (not (listint unsorted))
          (k '"bubble expects list of integers"))

        ; Now bubble sort
        (let ((change 't) (result unsorted))

          ; Return a list, bubbled once on each adjacent pair
          (define sort1
            (lambda (x)
              (if (or (null x) (null (cdr x)))
                x
                (if (> (car x) (car (cdr x)))
                  (progn
                    (setq change 't)
                    (cons (car (cdr x)) (sort1 (cons (car x) (cdr (cdr x))))))
                  (cons (car x) (sort1 (cdr x)))))))

          ; Bubble until no change
          (while change
            (setq change '())
            (setq result (sort1 result)))

          result)))

    (call/cc bubble1)))

; Now test it
(bubble 0)
(bubble '())
(bubble '(9 8 7 6 5 4 3 2 1 a))
(bubble '(0xa 9 8 7 6 5 4 3 2 1))
