; Read keyboard controller flash rom image and compare to file
(define kbcmp
  (lambda (fnam)

    (define APPSIZE (macro () 49152))
    (define BOOTSIZ (macro () 0x1800))
    (define ALLSIZE (macro () (+ (APPSIZE) (BOOTSIZ))))
    (define TIMEOUT (macro () 10000))
    (define i)

    (define _kbcmp
      (lambda (k)

        ; Set local exception handler
        (define df (defer 't))
        (global "*error-continue*" k)
        (defer df)

        ; Open file for reading
        (define f (fopen fnam 'r))
        (if (zerop f)
          (error (cats '"? Couldn't open file \"" fnam '"\"")))

        ; Read a byte at a time from keyboard controller, and compare
        (while (keyp) (getchar))
        (print '"Reading keyboard controller flash and file ")
        (define b)
        (define c)
        (define d)
        (define j)
        (define n)
        (setq i 0)
        (while (< i (ALLSIZE))

          ; Request the value
          (setq d (defer 't))
          (print '"\e[")
          (if (zerop i)

            ; Start of APP
            (progn
              (print '"0;0;0;")
              (print 0x9d))

            ; Start of BOOT
            (if (= i (APPSIZE))
              (progn
                (print '"0;0;")
                (print 0xc0)
                (print '";")
                (print 0xbf))))
          (print '"R")
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

    (define e (call/cc _kbcmp))

    (define df (defer 't))
    (global "*error-continue*" old-error-handler)
    (defer df)

    (if e
      (progn
        (delay 500)
        (while (keyp) (getchar))
        (error e))
      i)))
