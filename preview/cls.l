; Clear screen
(define cls
  (lambda () (print '"\e[2J\e[H")))
