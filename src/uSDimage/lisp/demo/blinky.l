; Blink user LED
(define blinky
  (lambda ()
    (led '()) (delay 10000) (led 't) (delay 10000) (blinky)))
