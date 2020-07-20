; Seven segment display clock demo
(define clock7
  (lambda ()

    ; Get an SPI device object on PD7
    ; Modify this if your device /CS is on a different GPIO
    ; or if you want to do SPI transfers with different modes and speeds
    (define seg7 (spidev (PD_DR) 7 0x30 3))
    (define acquire (seg7 'spiacq))
    (define release (seg7 'spirel))

    ; Write data to an address in the LED display controller
    (define write_display
      (lambda (a d)
        (define df (defer 't))
        (acquire)
        (spixchg a)
        (spixchg d)
        (release)
        (defer df)))

    (define _clock7
      (lambda (k)

        ; Get digit corresponding to ascii character
        (define digit (macro (c) `(- (sym2int ,c) 0x30)))

        ; Set exception handler
        (define df (defer 't))
        (global "*error-continue*" k)
        (defer df)

        ; Turn on display test
        (write_display 0x0F 0x01)
        (define dt 't)

        ; Come out of shutdown mode
        (write_display 0x0C 0x01)

        ; Set brightness to half max
        (write_display 0x0A 8)

        ; Set decoding mode for 4 digits
        (write_display 0x09 0x0F)

        ; Set scan for 4 digits, plus 1 for L1,L2,L3
        (write_display 0x0B 4)

        ; Leave display test on for a while
        (delay 10000)

        ; Display time of day
        (define d)
        (define h 't)
        (define i 0)
        (define t)
        (while 't

          ; Check for end or format toggle
          (if (keyp)
            (if (= (getchar) 0x20)
              (setq h (not h))
              (break)))

          ; Get time, HH:MM or MM:SS
          (setq t (time))
          (if (not h)
            (setq t (cdddr t)))

          ; Write the digits
          (setq d (digit (car t)))
          (if (and h (zerop d)) (setq d 0x0F))
          (write_display 4 d)
          (setq t (cdr t))
          (write_display 3 (digit (car t)))
          (setq t (cddr t))
          (write_display 2 (digit (car t)))
          (setq t (cdr t))
          (write_display 1 (digit (car t)))

          ; Blink colon
          (write_display 5 (if (zerop (& i 0x40)) 0x60 0x00))
 
          ; Turn off display test
          (if dt
            (progn
              (write_display 0x0F 0x00)
              (setq dt '())))

          (setq i (+ i 1)))

        '""))

    ; Call worker, which may end by exception (^C)
    (define old-error-handler
      (if (assignedp '"*error-continue*") "*error-continue*"))
    (define r (call/cc _clock7))

    ; Put display in shutdown mode, no display test
    (define df (defer 't))
    (write_display 0x0C 0x00)
    (write_display 0x0F 0x00)
    (global "*error-continue*" old-error-handler)
    (defer df)

    r))
