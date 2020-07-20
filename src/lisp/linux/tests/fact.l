#!lisp.sh

; Recursive looping factorial test

; Factorial
(define fact
  (lambda (x)
    (if (zerop x) 1 (* x (fact (- x 1))))))

; Loop a million times
(define loopfact
  (lambda (i)
    (if (< i 1000000)
      (progn
        (fact 42)
        (loopfact (+ i 1)))
      i)))

(loopfact 0)

; Look at the code
fact
