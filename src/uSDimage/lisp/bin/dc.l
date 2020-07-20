; Set fg, bg color directly
(define dc
  (lambda (f b)
    (print '"\e[Y")
    (putchar f)
    (putchar b)
    (cls)))
