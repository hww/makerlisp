; Set fg, bg color from ANSI palette
(define color
  (lambda (f b)
    (print '"\e[X")
    (putchar f)
    (putchar b)
    (cls)))
