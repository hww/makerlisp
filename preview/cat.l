; Open, read a file, and print it out
(define cat
  (lambda (fnam)
    (define f (fopen fnam 'r))
    (if (zerop f)
      (error (cats '"? Couldn't open file \"" fnam '"\"")))
    (define c)
    (while (not (= (setq c (fgetc f)) -1))
      (putchar c))
    (fclose f)
    '""))
