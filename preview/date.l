; Get/set date in real time clock
(define date
  (lambda args
    
    ; Getting or setting ?
    (if (not args)
      (progn

        ; Convert bcd to binary
        (define bcd2bin
          (lambda (b)
            (+ (* (/ b 16) 10) (% b 16))))

        ; Convert year month day bcd values from RTC to binary
        (define year (+ (* (bcd2bin (readreg (RTC_CEN))) 100)
                           (bcd2bin (readreg (RTC_YR)))))
        (define month (bcd2bin (readreg (RTC_MON))))
        (define day (bcd2bin (readreg (RTC_DOM))))
        (define dow (bcd2bin (readreg (RTC_DOW))))

        ; Get date symbol in familiar format
        (define days (vector 'Sun 'Mon 'Tue 'Wed 'Thu 'Fri 'Sat))
        (cats
         (vref days (% dow 7))
         '" "
         (sprintf '"%d" month)
         '"-"
         (sprintf '"%d" day)
         '"-"
         (sprintf '"%d" year)))

      (progn

        ; Convert binary to bcd
        (define bin2bcd
          (lambda (d)
          (+ (* (/ d 10) 16) (% d 10))))

        ; Compute day of week, given date (Sunday 0)
        ; Tomohiko Sakamoto algorithm
        (define date2dow
          (lambda (y m d)
            (define t (vector 0 3 2 5 0 3 5 1 4 6 2 4))
            (if (< m 3) (setq y (- y 1))) 
            (% (+ y (/ y 4) (/ y -100) (/ y 400) (vref t (- m 1)) d) 7)))

        ; Get a number
        (define digitp (macro (c) `(not (zerop (isdigit (sym2int ,c))))))
        (define getnum
          (lambda (c)
            (define number '"")
            (while (digitp (car dtsym))
              (setq number (cats number (car dtsym)))
              (setq dtsym (cdr dtsym)))
            (if (not (eq (car dtsym) c)) (error fmtmsg))
            (setq dtsym (cdr dtsym))
            (atoi number)))

        ; Single argument must be a symbol
        (define dtsym (car args))
        (if (or (cdr args) (not (symbolp dtsym)))
          (error '"? Arguments to 'date' must be one symbol"))

        ; Parse symbol
        (define fmtmsg '"? Symbol for setting date should be \"MM-DD-YYYY\"")
        (define month (getnum '"-"))
        (if (or (< month 1) (> month 12)) (error fmtmsg))

        (define day (getnum '"-"))
        (if (or (< day 1) (> day 31)) (error fmtmsg))

        (define year (getnum '""))
        (if (or (< year 0) (> year 9999)) (error fmtmsg))

        ; Determine day of week (Monday 1, Sunday 7)
        (define dow (date2dow year month day))
        (if (zerop dow) (setq dow 7))

        ; Good date, now convert to bcd and write to RTC
        (writereg (RTC_CTRL) 0x21)
        (writereg (RTC_DOW) (bin2bcd dow))
        (writereg (RTC_MON) (bin2bcd month))
        (writereg (RTC_DOM) (bin2bcd day))
        (writereg (RTC_YR) (bin2bcd (% year 100)))
        (writereg (RTC_CEN) (bin2bcd (/ year 100)))
        (writereg (RTC_CTRL) 0x20)
        (date)))))
