; Blink user LED
(define blinky
  (lambda ()
    (led '()) (delay 5000) (led 't) (delay 5000) (blinky)))
