; SPI device object maker
(define spidev
  (lambda (reg bit mode rate)

    ; Old mode and rate to restore
    (define oldmr)

    ; Select, deselect
    (define select
      (macro ()
       `(writereg ,reg (_iand (readreg ,reg) ,(~ (<< 1 bit))))))

    (define deselect
      (macro ()
       `(writereg ,reg (_ior (readreg ,reg) ,(<< 1 bit)))))

    ; Take control of and relinquish SPI bus
    (define spiacq
      (slambda ()
        (setq oldmr (spimode ("|" (<< rate 8) mode)))
        (select)))

    (define spirel
      (slambda ()
        (deselect)
        (spimode oldmr)))

    ; Return methods specialized by object parameters
    (define mlist
      (list
        (cons 'spiacq spiacq)
        (cons 'spirel spirel)))
    (define f
      (lambda (m)
        (if (define f (cdr (assq m mlist)))
          f
          (error (cats '"? spidev: Unknown method: '" m '"'\n")))))

    ; Set pin used for /CS as GPIO output, initially de-selected
    (regbset reg bit)
    (gpout reg bit)

    f))
