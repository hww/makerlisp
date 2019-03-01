(define stdout
  (macro args
    (forget 'stdout)
   `(ffcall 'stdout '(void* ()) (list ,@args))))
