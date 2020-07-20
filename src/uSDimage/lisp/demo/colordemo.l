; This is the MakerLisp program that is running now
(define colordemo
  (lambda ()
    (define _colordemo
      (lambda (n)
        (let ((b (>> (& n 0xE0) 5)) (f (>> (& n 0x1E) 1)) (a (& n 1)))
          (if (not (= f b))
            (progn
              (color f b) (rv (not (zerop a)))
              (print '"FG/BG color ") (print f) (print '"/") (print b)
              (if (not (zerop a)) (print '" (reversed)")) (print '\n\n)
              (print csym) (print '\n\n) (print fsym) (delay 10000))))
        (_colordemo (+ n 1))))
    (define csym
      (if (zerop (& (readreg (PB_DR)) 0x02)) (chartest 32) (chartest)))
    (define fsym (fcat '"/demo/colordemo.l"))
    (_colordemo 0)))
