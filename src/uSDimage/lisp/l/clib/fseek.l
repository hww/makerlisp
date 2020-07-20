(define fseek
  (macro args
    (forget 'fseek)
   `(ffcall 'fseek '(int (void* double int)) (list ,@args))))
