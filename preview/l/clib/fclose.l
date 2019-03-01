(define fclose
  (macro args
    (forget 'fclose)
   `(ffcall 'fclose '(int (void*)) (list ,@args))))
