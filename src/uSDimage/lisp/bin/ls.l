; Directory/file listing utility
(define ls
  (lambda args
    (if (if args (or (cdr args) (not (symbolp (car args)))))
      (error '"? Argument to \"ls\" must be a symbol"))

    (if args
      (if (not (opendir (car args)))
        (error (cats '"? Can't open directory \"" (car args) '"\"")))
      (opendir))

    (define fname)
    (while 't
      (setq fname (lsitem))
      (if (eq fname '"")
        (break))
      (nlprint fname))

    (closedir)
    '""))
