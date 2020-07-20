; Read flash rom and compare with file
(define romcmp
  (lambda (fnam)

    (define SIZE (macro () 262144))

    ; Open file for reading
    (define f (fopen fnam 'r))
    (if (zerop f)
      (error (cats '"? Couldn't open file \"" fnam '"\"")))

    ; Set frequency divider for 50 MHz
    (while (not (= 255 (readreg (FLASH_FDIV))))
      (writereg (FLASH_KEY) 0xB6)
      (writereg (FLASH_KEY) 0x49)
      (writereg (FLASH_FDIV) 255))

    ; Reset flash I/O access counter
    (writereg (FLASH_PAGE) 0)
    (writereg (FLASH_ROW) 0)
    (writereg (FLASH_COL) 0)

    ; Read flash and file a byte at a time, and compare
    (print '"Reading flash and file ")
    (define c)
    (define i 0)
    (while (< i (SIZE))
      (if
        (or (= (setq c (fgetc f)) -1) (not (= c (readreg (FLASH_DATA)))))
        (break))
      (setq i (+ i 1))
      (if (zerop (% i 1024))
        (print '".")))
    (fclose f)
    (print '" ")
    i))
