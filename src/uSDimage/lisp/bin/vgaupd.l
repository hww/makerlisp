; Read file and update display controller eeprom
(include '<cats.l>)
(include '<delay.l>)

(define vgaupd
  (lambda (fnam)

    (define ROMLONGS (macro () (/ 32768 4)))
    (define TIMEOUT (macro () 10000))
    (define i)

    (define _vgaupd
      (lambda (k)

        ; Set local exception handler
        (define df (defer 't))
        (global "*error-continue*" k)
        (defer df)

        ; Open file for reading
        (define f (fopen fnam 'r))
        (if (zerop f)
          (error (cats '"? Couldn't open file \"" fnam '"\"")))

        ; Warning
        (while (keyp) (getchar))
        (print '"Warning ! Updating display controller eeprom !\n")
        (print '"Press any key to continue, or \^C to abort ... ")
        (getchar)
        (print '\n\n)

        ; Read file and write to display controller, 4 bytes at a time
        (while (keyp) (getchar))
        (print '"Writing display controller eeprom ")
        (define b)
        (define d)
        (define j)
        (define n)
        (define o)
        (define z)
        (setq i 0)
        (while (< i (ROMLONGS))

          ; Start a 4 byte write command
          (setq o (defer 't))
          (print '"\e[")
          (if (zerop i) (print '"0;0;"))
          (setq j 0)
          (while (< j 4)

            ; Get a byte 
            (if (= (setq n (fgetc f)) -1)
              (progn
                (print '"V")
                (print '\n)
                (fclose f)
                (error (cats '"? Error reading file \"" fnam '"\""))))

            ; Separate parameters
            (if (not (zerop j)) (print '";"))

            ; Convert to decimal ASCII
            (setq b 100)
            (setq z 't)
            (while (not (zerop b))
              (setq d (/ n b))
              (setq n (- n (* d b)))
              (if (not (zerop d))
                (setq z '()))
              (if (or (not z) (= b 1))
                (putchar (+ d 0x30)))
              (setq b (/ b 10)))

            (setq j (+ j 1)))

          ; End the write command
          (print '"V")
          (defer o)

          ; Wait for response
          (setq j 0)
          (while (< j (TIMEOUT))
            (if (keyp) (break))
            (setq j (+ j 1)))
          
          ; If timeout, try again
          (if (not (< j (TIMEOUT)))
            (progn
              (fseek f -4.0 1)
              (continue)))
          (getchar)

          (setq i (+ i 1))
          (if (zerop (% i 64))
            (print '".")))

        ; Close up
        (fclose f)
        (print '" ")
        '()))

    (define old-error-handler
      (if (assignedp '"*error-continue*") "*error-continue*"))

    (define e (call/cc _vgaupd))

    (define df (defer 't))
    (global "*error-continue*" old-error-handler)
    (defer df)

    (if e
      (progn
        (delay 500)
        (while (keyp) (getchar))
        (error e))
      i)))
