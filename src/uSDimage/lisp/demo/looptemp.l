; Temperature sensor comparison - TE/MS TSYS01, TI HDC1080, TMP125, TMP101
(define looptemp
  (lambda ()

    ; TSYS01 PROM register array
    (define pv [0 1 2 3 4 5 6 7])

    ; TSYS01 (I2C) setup
    (define tsys01setup
      (lambda (a)

        ; Read a 16 bit TSYS01 PROM register
        (define promread
          (lambda (i)
            (define d (defer 't))
            (starti2c2)
            (writei2c2 (<< a 1))
            (writei2c2 (+ 0xA0 (* i 2)))
            (stopi2c2)
            (starti2c2)
            (writei2c2 ("|" (<< a 1) 1))
            (define h (readi2c2))
            (define l (readi2c2))
            (stopi2c2)
            (defer d)
            ("|" (<< h 8) l)))

        ; Reset the TSYS01
        (define reset
          (lambda ()
            (define d (defer 't))
            (starti2c2)
            (writei2c2 (<< a 1))
            (writei2c2 0x1E)
            (stopi2c2)
            (defer d)))

        ; Reset sensor, get the PROM data
        (reset)
        (define i 0)
        (while (< i 8)
          (vset pv i (promread i))
          (setq i (+ i 1)))))

    ; TSYS01 (I2C) temperature read
    (define tsys01read
      (lambda (a)

        ; Accumulate
        (define addto
          (macro (total adder)
           `(setq ,total (+ ,total ,adder))))

        ; Calculate temperature from ADC value and calibration coefficients
        (define calctemp
          (lambda (adc16)
            (define rt (int2real adc16))
            (define sum 0.0)
            (addto sum (* -2.0 (vref pv 1) (pow 10.0 -21.0) (pow rt 4.0)))
            (addto sum (*  4.0 (vref pv 2) (pow 10.0 -16.0) (pow rt 3.0)))
            (addto sum (* -2.0 (vref pv 3) (pow 10.0 -11.0) (pow rt 2.0)))
            (addto sum (*  1.0 (vref pv 4) (pow 10.0 -6.0) rt))
            (addto sum (* -1.5 (vref pv 5) (pow 10.0 -2.0)))))

        ; Read the temperature register
        (define readadc
          (lambda ()
            (define d (defer 't))
            (starti2c2)
            (writei2c2 (<< a 1))
            (writei2c2 0x00)
            (stopi2c2)
            (starti2c2)
            (writei2c2 ("|" (<< a 1) 1))
            (define h (readi2c2))
            (define l (readi2c2))
            (readi2c2)
            (stopi2c2)

            ; Shouldn't have to do this to "deselect" TSYS01
            (starti2c2)
            (writei2c2 0x00)
            (writei2c2 0x00)
            (stopi2c2)
            (defer d)

            ("|" (<< h 8) l)))

        ; Start a temperature conversion
        (define startadc
          (lambda ()
            (define d (defer 't))
            (starti2c2)
            (writei2c2 (<< a 1))
            (writei2c2 0x48)
            (stopi2c2)
            (defer d)))

        ; Get ADC value, calculate and return the temperature
        (startadc)
        (calctemp (readadc))))

    ; TMP125 (SPI) temperature read
    (define tmp125
      (lambda ()
        (define d (defer 't))
        (t12nacq)
        (define h (spixchg 0))
        (define l (spixchg 0))
        (t12nrel)
        (defer d)
        (/ (>> (<< ("|" (<< h 3) (>> l 5)) 14) 14) 4.0)))

    ; Set TMP101 (I2C) resolution to 10 bits
    (define setup101
      (lambda (a)
        (define d (defer 't))
        (starti2c)
        (writei2c (<< a 1))
        (writei2c 1)
        (writei2c 0x20)
        (starti2c)
        (writei2c (<< a 1))
        (writei2c 0)
        (stopi2c)
        (defer d)))

    ; TMP101 (I2C) temperature read
    (define tmp101
      (lambda (a)
        (define d (defer 't))
        (starti2c)
        (writei2c ("|" (<< a 1) 1))
        (define h (readi2c))
        (define l (readi2c))
        (stopi2c)
        (defer d)
        (/ (>> (<< ("|" (<< h 2) (>> l 6)) 14) 14) 4.0)))

    ; Set HDC1080 I2C) to 14 bit either-or measurements
    (define setup1080
      (lambda (a)
        (define d (defer 't))
        (starti2c)
        (writei2c (<< a 1))
        (writei2c 2)
        (writei2c 0)
        (writei2c 0)
        (stopi2c)
        (defer d)))

    ; HDC1080 (I2C) temperature read
    (define hdc1080
      (lambda (a)
        (define d (defer 't))
        (starti2c)
        (writei2c (<< a 1))
        (writei2c 0)
        (define ack 1)
        (define t 0)
        (while (and (< t 10) (not (zerop ack)))
          (starti2c)
          (setq ack (writei2c ("|" (<< a 1) 1)))
          (setq t (+ t 1)))
        (define h (readi2c))
        (define l (readi2c))
        (stopi2c)
        (defer d)
        (- (/ (* ("|" (<< h 8) l) 165.0) 65536.0) 40.0)))

    ; Accumulate a string
    (define sapp
      (macro (str sym)
       `(setq ,str (cats ,str ,sym))))

    ; Loop on reading and displaying sensors
    (define _looptemp
      (lambda ()
        (define s '"")

        ; Comment out, or adjust the next four statements, to address devices
        (sapp s (sprintf '"TSYS01 %.1f " (tsys01read 0x76)))
        (sapp s (sprintf '"HDC1080 %.1f " (hdc1080 0x40)))
        (sapp s (sprintf '"TMP125 %.2f " (tmp125)))
        (sapp s (sprintf '"TMP101 %.2f " (tmp101 0x48)))

        (nlprint s)
        (delay 5000)
        (_looptemp)))

    ; Set up SPI device on PD7 for TMP125, CPOL 0/CPHA 0, 5.0 MHz
    ; e.g. MakerLisp TemperSure 125
    (define t12nspi (spidev (PD_DR) 7 0x30 5))
    (define t12nacq (t12nspi 'spiacq))
    (define t12nrel (t12nspi 'spirel))

    ; Set up I2C bus on PA7 (CLK) and PA6 (DAT)
    (define i2cobj (i2cbus (PA_DR) 7 (PA_DR) 6))
    (define readi2c (i2cobj 'i2cread))
    (define starti2c (i2cobj 'i2cstart))
    (define stopi2c (i2cobj 'i2cstop))
    (define writei2c (i2cobj 'i2cwrite))

    ; Set up another I2C bus on PA5 (CLK) and PA4 (DAT)
    (define i2cobj2 (i2cbus (PA_DR) 5 (PA_DR) 4))
    (define readi2c2 (i2cobj2 'i2cread))
    (define starti2c2 (i2cobj2 'i2cstart))
    (define stopi2c2 (i2cobj2 'i2cstop))
    (define writei2c2 (i2cobj2 'i2cwrite))

    ; Set up TMP101 for 10 bit temperature
    ; e.g. MakerLisp TemperSure 101
    (setup101 0x48)

    ; Set up HDC1080 for 14 bit temperature/humidity
    ; e.g MakerLisp TemperSure 1080, or Closed Cube HDC1080
    (setup1080 0x40)

    ; Set up TSYS01
    ; e.g. Closed Cube TSYS01, or Elecrow TSYS01 in I2C mode
    (tsys01setup 0x76)

    (_looptemp)))
