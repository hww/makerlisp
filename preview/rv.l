; Set/clear reverse video attribute
(define rv
  (lambda (r)
    (print '"\e[Z")
    (putchar r)
    (cls)))
