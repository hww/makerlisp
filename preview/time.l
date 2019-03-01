; Get/set time in real time clock
(define time
  (lambda args
    
    ; Getting or setting ?
    (if (not args)
      (progn

        ; Convert bcd to binary
        (define bcd2bin
          (lambda (b)
            (+ (* (/ b 16) 10) (% b 16))))

        ; Convert hours minutes seconds bcd values from RTC to binary
        (define hours (bcd2bin (readreg (RTC_HRS))))
        (define minutes (bcd2bin (readreg (RTC_MIN))))
        (define seconds (bcd2bin (readreg (RTC_SEC))))

        ; Get time symbol in familiar format
        (cats
         (sprintf '"%02d" hours)
         '":"
         (sprintf '"%02d" minutes)
         '":"
         (sprintf '"%02d" seconds)))

      (progn

        ; Convert binary to bcd
        (define bin2bcd
          (lambda (d)
          (+ (* (/ d 10) 16) (% d 10))))

        ; Get a number
        (define digitp (macro (c) `(not (zerop (isdigit (sym2int ,c))))))
        (define getnum
          (lambda (c)
            (define number '"")
            (while (digitp (car tmsym))
              (setq number (cats number (car tmsym)))
              (setq tmsym (cdr tmsym)))
            (if (not (eq (car tmsym) c)) (error fmtmsg))
            (setq tmsym (cdr tmsym))
            (atoi number)))

        ; Single argument must be a symbol
        (define tmsym (car args))
        (if (or (cdr args) (not (symbolp tmsym)))
          (error '"? Arguments to 'time' must be one symbol"))

        ; Parse symbol
        (define fmtmsg '"? Symbol for setting time should be \"HH:MM:SS\"")
        (define hours (getnum '":"))
        (if (or (< hours 0) (> hours 23)) (error fmtmsg))

        (define minutes (getnum '":"))
        (if (or (< minutes 0) (> minutes 59)) (error fmtmsg))

        (define seconds (getnum '""))
        (if (or (< seconds 0) (> seconds 59)) (error fmtmsg))

        ; Good time, now convert to bcd and write to RTC
        (writereg (RTC_CTRL) 0x21)
        (writereg (RTC_HRS) (bin2bcd hours))
        (writereg (RTC_MIN) (bin2bcd minutes))
        (writereg (RTC_SEC) (bin2bcd seconds))
        (writereg (RTC_CTRL) 0x20)
        (time)))))
