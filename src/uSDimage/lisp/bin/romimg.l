; Read flash rom and save to file
(include '<cats.l>)

(define romimg
  (lambda (fnam)

    (define SIZE (macro () 262144))

    ; Open file for writing
    (define f (fopen fnam 'w))
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

    ; Read flash a byte at a time, and write it out
    (print '"Reading flash ")
    (define i 0)
    (while (< i (SIZE))
      (if (= (fputc (readreg (FLASH_DATA)) f) -1)
        (progn
          (print '\n)
          (fclose f)
          (error (cats '"? Error writing to file \"" fnam '"\""))))
      (setq i (+ i 1))
      (if (zerop (% i 1024))
        (print '".")))
    (fclose f)
    (print '" ")
    i))
