(define ungetc
  (macro args
    (forget 'ungetc)
   `(ffcall 'ungetc '(int (int void*)) (list ,@args))))
