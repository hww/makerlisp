; Read display controller eeprom image and save to file
(include '<cats.l>)
(include '<delay.l>)

(define vgaimg
  (lambda (fnam)

    (define ROMSIZE (macro () 32768))
    (define TIMEOUT (macro () 10000))
    (define i)

    (define _vgaimg
      (lambda (k)

        ; Set local exception handler
        (define df (defer 't))
        (global "*error-continue*" k)
        (defer df)

        ; Open file for writing
        (define f (fopen fnam 'w))
        (if (zerop f)
          (error (cats '"? Couldn't open file \"" fnam '"\"")))

        ; Read a byte at a time from display controller, and write it out
        (while (keyp) (getchar))
        (print '"Reading display controller eeprom ")
        (define b)
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

          ; Write to file
          (if (= (fputc n f) -1)
            (progn
              (print '\n)
              (fclose f)
              (error (cats '"? Error writing to file \"" fnam '"\""))))

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

    (define e (call/cc _vgaimg))

    (define df (defer 't))
    (global "*error-continue*" old-error-handler)
    (defer df)

    (if e
      (progn
        (delay 500)
        (while (keyp) (getchar))
        (error e))
      i)))
