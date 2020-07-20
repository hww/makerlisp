; Set GPIO pin mode to output
(define gpout 
  (lambda (r b)            ; r: P?_DR
    (regbclr (+ r 1) b)    ; 1: P?_DDR
    (regbclr (+ r 2) b)    ; 2: P?_ALT1
    (regbclr (+ r 3) b)))  ; 3: P?_ALT2
