; I2C bus object maker
(define i2cbus
  (lambda (clkreg clkbit datreg datbit)

    ; I2C bit read and write
    (define clkrd
      (macro ()
       `(regbread ,clkreg ,clkbit)))

    (define clklo
      (macro ()
       `(regbclr ,clkreg ,clkbit)))

    (define clkhi
      (macro ()
       `(regbset ,clkreg ,clkbit)))

    (define _clkhiw
      (slambda (t)
        (if (zerop t)
          (error '"? Timed out waiting for SCL to go high"))
        (clkhi)
        (if (zerop (clkrd))
          (_clkhiw (- t 1)))))

    (define clkhiw (macro () `(_clkhiw 1000)))

    (define datrd
      (macro ()
       `(regbread ,datreg ,datbit)))

    (define datwr
      (macro (d)
       `(regbwrite ,datreg ,datbit ,d)))

    (define datlo
      (macro ()
       `(regbclr ,datreg ,datbit)))

    (define dathi
      (macro ()
       `(regbset ,datreg ,datbit)))

    ; I2C protocol events
    (define mack
      (slambda ()
        (datlo) (clkhiw) (clklo)))

    (define sack
      (slambda ()
        (clkhiw) (define a (datrd)) (clklo) a))

    (define start
      (slambda ()
        (dathi) (clkhiw) (datlo) (clklo)))

    (define stop
      (slambda ()
        (datlo) (clkhiw) (dathi) (clklo)))

    ; I2C byte read and write
    (define bitread
      (slambda (d n)
        (if (zerop n)
          d
          (progn
            (dathi)
            (clkhiw)
            (setq d ("|" (<< d 1) (datrd)))
            (clklo)
            (bitread d (- n 1))))))

    (define bytread
      (slambda ()
        (define d (bitread 0 8))
        (mack)
        d))

    (define bitwrit
      (slambda (d n)
        (if (zerop n)
          '()
          (progn
            (datwr (>> d 7))
            (clkhiw)
            (clklo)
            (bitwrit (<< d 1) (- n 1))))))

    (define bytwrit
      (slambda (d)
        (bitwrit d 8)
        (sack)))

    ; Return methods specialized by object parameters
    (define mlist
      (list
        (cons 'i2cstart start)
        (cons 'i2cstop stop)
        (cons 'i2cread bytread)
        (cons 'i2cwrite bytwrit)))
    (define f
      (lambda (m)
        (if (define f (cdr (assq m mlist)))
          f
          (error (cats '"? i2cbus: Unknown method: '" m '"'\n")))))

    ; Set CLK and DAT to open drain, initial state
    (gpod clkreg clkbit)
    (gpod datreg datbit)
    (regbclr clkreg clkbit)
    (regbset datreg datbit)

    f))
