; Put character to stdout
(define putchar
  (macro (c)
    (forget 'putchar)
   `(fputc ,c (stdout))))
