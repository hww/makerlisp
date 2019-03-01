(define fputc
  (macro args
    (forget 'fputc)
   `(ffcall 'fputc '(int (int void*)) (list ,@args))))
