; Read rom image file and update flash rom
(include '<cats.l>)

(define romupd
  (lambda (fnam)

    (define SIZE (macro () 262144))

    ; Open file for reading
    (define f (fopen fnam 'r))
    (if (zerop f)
      (error (cats '"? Couldn't open file \"" fnam '"\"")))

    ; Warning
    (while (keyp) (getchar))
    (print '"Warning ! This will erase the flash rom COMPLETELY !\n")
    (print '"A new image will then be loaded to the flash rom.\n")
    (print '"Press any key to continue, or \^C to abort ... ")
    (getchar)
    (print '\n\n)

    ; Set frequency divider for 50 MHz
    (while (not (= 255 (readreg (FLASH_FDIV))))
      (writereg (FLASH_KEY) 0xB6)
      (writereg (FLASH_KEY) 0x49)
      (writereg (FLASH_FDIV) 255))

    ; Unprotect flash rom
    (while (not (= 0 (readreg (FLASH_PROT))))
      (writereg (FLASH_KEY) 0xB6)
      (writereg (FLASH_KEY) 0x49)
      (writereg (FLASH_PROT) 0))

    ; Mass erase
    (writereg (FLASH_PGCTL) 1)

    ; Reset flash I/O access counter
    (writereg (FLASH_PAGE) 0)
    (writereg (FLASH_ROW) 0)
    (writereg (FLASH_COL) 0)

    ; Read file and write to flash, byte by byte
    (print '"Writing flash ")
    (define c)
    (define i 0)
    (while (< i (SIZE))
      (if (= (setq c (fgetc f)) -1)
        (progn
          (print '\n)
          (fclose f)
          (error (cats '"? Error reading file \"" fnam '"\""))))
      (writereg (FLASH_DATA) c)
      (setq i (+ i 1))
      (if (zerop (% i 1024))
        (print '".")))
    (fclose f)
    (print '" ")
    i))
