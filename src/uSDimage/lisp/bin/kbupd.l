; Read file and update keyboard controller flash rom
(include '<cats.l>)

(define kbupd
  (lambda (fnam)

    (define APPSIZE (macro () 49152))
    (define APPDWORDS (macro () (/ (APPSIZE) 8)))
    (define BOOTSIZ (macro () 0x1800))
    (define ALLDWORDS (macro () (/ (+ (APPSIZE) (BOOTSIZ)) 8)))
    (define TIMEOUT (macro () 10000))

    ; Open file for reading
    (define f (fopen fnam 'r))
    (if (zerop f)
      (error (cats '"? Couldn't open file \"" fnam '"\"")))

    ; Warning
    (while (keyp) (getchar))
    (print '"Warning ! Updating keyboard controller flash rom !\n")
    (print '"Press any key to continue, or \^C to abort ... ")
    (getchar)
    (print '\n\n)

    ; Start session, wait for first request for data, retry until we get it
    (while (keyp) (getchar))
    (define j)
    (while 't
      (print '"\e[W")
      (setq j 0)
      (while (< j (TIMEOUT))
        (if (keyp) (break))
          (setq j (+ j 1)))
      (if (< j (TIMEOUT)) (break)))

    ; Read file and write to keyboard controller, 8 bytes at a time
    (print '"Writing keyboard controller flash ")
    (define b)
    (define d)
    (define n)
    (define z)
    (define i 0)
    (while (< i (ALLDWORDS))

      ; Wait for a request for data
      (getchar)

      ; Done with APP section ?
      (if (= i (APPDWORDS))
        (progn
          (print '"\e[W")
          (getchar)))

      ; Start an 8 byte write command
      (print '"\e[")

      ; Read and send eight bytes as parameters
      (setq j 0)
      (while (< j 8)

        ; Get a byte 
        (if (= (setq n (fgetc f)) -1)
          (progn
            (print '"W")
            (getchar)
            (print '\n)
            (fclose f)
            (error (cats '"? Error reading file \"" fnam '"\""))))

        ; Separate parameters
        (if (not (zerop j))
          (print '";"))

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
      (print '"W")

      (setq i (+ i 1))
      (if (zerop (% i 32))
        (print '".")))

    ; End the session
    (getchar)
    (print '"\e[W")
    (getchar)

    ; Close up
    (fclose f)
    (print '" ")
    i))
