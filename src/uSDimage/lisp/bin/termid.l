; Get terminal identification
(define termid
  (lambda ()
    (putchar (sym2int '"^E"))
    (delay 100)
    (define s '"")
    (while (keyp) (setq s (cats s (int2sym (getchar)))))
    s))
