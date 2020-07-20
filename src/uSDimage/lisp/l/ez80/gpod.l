; Set GPIO pin mode to open drain
(define gpod
  (lambda (r b)            ; r: P?_DR
    (regbclr (+ r 1) b)    ; 1: P?_DDR
    (regbset (+ r 2) b)    ; 2: P?_ALT1
    (regbclr (+ r 3) b)))  ; 3: P?_ALT2
