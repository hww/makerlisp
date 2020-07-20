; Get character from stdin
(define getchar
  (macro ()
    (forget 'getchar)
   `(fgetc (stdin))))
