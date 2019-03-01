; Load palette entry for fg, bg with rgb color values
(define cload
  (lambda (f b fc bc)
    (print '\e[Y)
    (putchar f)
    (putchar b)
    (putchar fc)
    (putchar bc)
    (cls)))
