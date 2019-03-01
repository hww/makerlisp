(define fgetc
  (macro args
    (forget 'fgetc)
   `(ffcall 'fgetc '(int (void*)) (list ,@args))))
