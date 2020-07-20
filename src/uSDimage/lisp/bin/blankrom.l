; Blank, mass erase flash rom
(define blankrom
  (lambda ()

    ; Warning
    (while (keyp) (getchar))
    (print '"Warning ! This will erase the flash rom COMPLETELY !\n")
    (print '"Use this utility ONLY if you can reprogram the flash\n")
    (print '"afterwards, with the Zilog debugger/programmer tool.\n")
    (print '"Press any key to continue, or \^C to abort ... ")
    (getchar)
    (print '\n)

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
    '""))
