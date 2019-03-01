; Simplest, lisp-iest sieve ever
(include '<makerlisp.l>)

; Generate list of integers for input to lsieve
(define ilist
  (lambda (a n)
    (if (< n 2)
      a
      (ilist (cons n a) (- n 1)))))

; Copy a list, excluding multiples of the specified integer
(define copynm
  (lambda (l n)
    (if l
      (if (zerop (% (car l) n))
        (copynm (cdr l) n)
        (cons (car l) (copynm (cdr l) n))))))

; Lsieve takes a list of integers and returns the list of primes within
(define lsieve
  (lambda (l)
    (if l
      (cons (car l) (lsieve (copynm (cdr l) (car l)))))))

; Sieve finds list of primes up to and including specified number
(define sieve
  (lambda (n)
    (lsieve (ilist '() n))))
