; Clear or set PA7 for 'blinky' LED
(define led
  (lambda (on)
    (if on (regbset (PA_DR) 7) (regbclr (PA_DR) 7))
    (gpout (PA_DR) 7)))
