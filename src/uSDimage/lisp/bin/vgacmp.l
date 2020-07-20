; Read display controller eeprom image and compare to file
(define vgacmp
  (lambda (fnam)

    (define ROMSIZE (macro () 32768))
    (define TIMEOUT (macro () 10000))
    (define i)

    (define _vgacmp
      (lambda (k)

        ; Set local exception handler
        (define df (defer 't))
        (global "*error-continue*" k)
        (defer df)

        ; Open file for reading
        (define f (fopen fnam 'r))
        (if (zerop f)
          (error (cats '"? Couldn't open file \"" fnam '"\"")))

        ; Read a byte at a time from display controller, and compare
        (while (keyp) (getchar))
        (print '"Reading display controller eeprom and file ")
        (define b)
        (define c)
        (define d)
        (define j)
        (define n)
        (setq i 0)
        (while (< i (ROMSIZE))

          ; Request the value
          (setq d (defer 't))
          (print '"\e[")
          (if (zerop i) (print '"0;0"))
          (print '"U")
          (defer d)

          ; Wait for it
          (setq j 0)
          (while (< j (TIMEOUT))
            (if (keyp) (break))
            (setq j (+ j 1)))
          
          ; If timeout, try again
          (if (not (< j (TIMEOUT))) (continue))

          ; Compute the value from the digits       
          (setq b 100)
          (setq n 0)
          (while (not (zerop b))
            (setq d (- (getchar) 0x30))
            (setq n (+ n (* b d)))
            (setq b (/ b 10)))

          ; Compare with file
          (if (or (= (setq c (fgetc f)) -1) (not (= c n)))
            (break))

          ; Show progress
          (setq i (+ i 1))
          (if (zerop (% i 256))
            (print '".")))

        ; Close up shop
        (fclose f)
        (print '" ")
        '()))

    (define old-error-handler
      (if (assignedp '"*error-continue*") "*error-continue*"))

    (define e (call/cc _vgacmp))

    (define df (defer 't))
    (global "*error-continue*" old-error-handler)
    (defer df)

    (if e
      (progn
        (delay 500)
        (while (keyp) (getchar))
        (error e))
      i)))
