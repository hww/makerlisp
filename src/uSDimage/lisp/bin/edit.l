; Simple editor
(define edit
  (lambda args

    ; Draw the frame
    (define frame
      (lambda ()

        (define d)
        (define i)
        (define j)

        ; Turn cursor off
        (setq d (defer 't))
        (print '"\e[?25l")
        (print '"\e[?7l")

        ; Clear screen
        (cls)

        ; Mode line
        (print '"\e[29;1H")
        (setq i 1)
        (while (< i 81)
          (putchar 196)
          (setq i (+ i 1)))

        ; Restore cursor
        (print '"\e[H")

        ; Turn cursor back on
        (print '"\e[?25h")
        (defer d)))

    ; Fill buffer display area with text
    (define text
      (lambda ()

        (define d)
        (define i)
        (define j)

        ; Turn cursor off
        (setq d (defer 't))
        (print '"\e[?25l")

        ; 28 lines of 80 characters
        (setq j 1)
        (while (< j 29)
          (print '"\e[")
          (print j)
          (print '";1H")
          (setq i 1)
          (while (< i 81)
             (print '".")
             (setq i (+ i 1)))
          (setq j (+ j 1)))
        (print '"\e[H")

        ; Turn cursor back on
        (print '"\e[?25h")
        (defer d)))

    (define _edit
      (lambda (k)

      ; Set exception handler
      (define df (defer 't))
      (global "*error-continue*" k)
      (defer df)
      
      ; Draw frame
      (frame)

      ; Fill buffer display area with text
      (text)

      ; Wait
      (while 't)
      '""))

    (define old-error-handler
      (if (assignedp '"*error-continue*") "*error-continue*"))

    (define r (call/cc _edit))

    (define df (defer 't))
    (global "*error-continue*" old-error-handler)
    (defer df)

    (cls)
    (if (not (eq r '"")) (nlprint r))))
