; Set fg, bg color from ANSI palette
(define color
  (lambda (f b)
    (print
      (cats
        '"\e["
        (if (zerop (& f 0x08)) '"22" '"1")
        '";"
        (sprintf '"%d" (+ (& f 0x07) 30))
        '";"
        (sprintf '"%d" (+ (& b 0x07) 40))
        '"m"))
    (cls)))
