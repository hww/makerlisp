; Open and read a file, and return the contents as a symbol
(define fcat
  (lambda (fnam)
    (define f (fopen fnam 'r))
    (if (zerop f)
      (error (cats '"? Couldn't open file '" fnam '"'")))
    (define text '"")
    (define c)
    (while (not (= (setq c (fgetc f)) -1))
      (setq text (cats text (int2sym c))))
    (fclose f)
    text))
