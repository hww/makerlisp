; Set/clear reverse video attribute
(define rv
  (lambda (r)
    (print (cats '"\e[" (if r '"7" '"27") '"m"))
    (cls)))
