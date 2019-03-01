; This is the MakerLisp program that is running now
(define colordemo
  (lambda ()

    (define _colordemo
      (lambda (n)
        (let ((b (>> (& n 0x1E0) 5)) (f (>> (& n 0x1E) 1)) (a (& n 1)))
          (if (not (= f b))
            (progn
              (color f b) (rv a)
              (print '"FG/BG color ") (print f) (print '"/") (print b)
              (if (not (zerop a)) (print '" (reversed)"))
              (print '\n\n) (print s) (delay 10000))))
        (_colordemo (+ n 1))))

    (define s (cats (chartest) '\n '\n (fcat 'colordemo.l)))
    (_colordemo 0)))
